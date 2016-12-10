// readable.c

//==========================================================
// �e���`�Ȃ�
//----------------------------------------------------------
#define VERSION     "2.03"                      // ���̃t�@�C�������ɏڍׂ�
#define SHORTBUF    30                          // ���O�ȂǒZ��������
#define LONGBUF     200                         // �R�}���h�Ȃǒ���������
#define JSONFILE    "_data.json"                // �f�B���N�g���p�X���f�[�^
#define LOGFILE     "log.csv"                   // ����M���O

//==========================================================

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "json.h"

enum direction { cp_up, cp_down };
enum mode { mode_normal, mode_help, mode_all_up, mode_all_down };

void quit( json_object_ptr json, int status, _Bool interactive );
void chop( char *str );
int read_json( json_object_ptr *data );
void write_json( json_object_ptr data );
int select( json_object_ptr *sel, json_object_ptr json, char *ask, _Bool all );
void copy_int( json_object_ptr json, json_object_ptr user, enum direction dir );
void copy_all( json_object_ptr json, json_object_ptr user, enum direction dir );
void copy_file( json_object_ptr user, json_object_ptr game, enum direction dir );
void write_log( char *user, char *game, enum direction dir );
void show_log( json_object_ptr json );

int main( int argc, char **argv )
{
	_Bool interactive = true;
	
	json_object_ptr json = NULL;		// root
	
	printf("syncronize version %s\n", VERSION);

	// json�f�[�^�ǂݍ���-----------------------------------
	if ( read_json( &json ) == -1 ) {
		quit( json, EXIT_FAILURE, interactive );
	}
	
	quit( json, EXIT_SUCCESS, interactive );
	return 0;
}

void quit( json_object_ptr json, int status, _Bool interactive )
{
	char buf[2];

	if ( json != NULL ) {
		write_json( json );
		json_clear( json, JSON_OBJECT );
	}

	if ( interactive || status != EXIT_SUCCESS ) {
		printf("See you.");
		fgets( buf, 2, stdin );
	}

	exit( status );
}

void chop( char *str )
{
	str[ strlen(str) - 1 ] = '\0';
}

void write_json( json_object_ptr data )
{
	FILE *fp;
	fp = fopen( JSONFILE, "w" );
	if ( fp == NULL )
	{
		printf( "write_json error: file '%s' cannot open.", JSONFILE );
	}
	json_show( fp, (json_data_ptr)data, JSON_OBJECT, "\t", showex_ex );
	fclose( fp );
}

int read_json( json_object_ptr *data )
{
	_Bool err = false;
	json_type_t type;

	switch ( json_parse_file( (json_data_ptr)data, &type, JSONFILE ) )
	{
	case -1:
		printf( "read_json error: file '%s' cannot open.\n", JSONFILE );
		err = true;
		break;
	case -2:
		printf( "read_json error: cannot allocate memory for json.\n" );
		err = true;
		break;
	case -3:
		printf( "read_json error: syntax error in file '%s'.\n", JSONFILE );
		err = true;
		break;
	case -4:
		printf( "read_json error: buffer size is not enough.\n" );
		err = true;
		break;
	case -5:
		printf( "read_json error: destination json_xxx_ptr is invalid.\n" );
		err = true;
		break;
	}
	if ( type != JSON_OBJECT )
	{
		printf( "read_json error: file '%s' is invalid to sync.\n", JSONFILE );
		err = true;
	}

	if ( err )	{ return -1; }
	else		{ return 0; }
}

int select( json_object_ptr *sel, json_object_ptr json, char *ask, _Bool all )
{
	char buf[LONGBUF];
	const json_object_ptr json_dir_top = json->value;
	json_object_ptr json_dir;

	assert( json->type == JSON_OBJECT );
	while(1)
	{
		json_dir = json_dir_top;

		printf( "%s [ ", ask );		// ���╶
		while ( json_dir != NULL )
		{
			printf( "%s / ", json_dir->string );
			json_dir = json_dir->next;
		}
		if ( all ) {
			printf("All / ");
		}
		printf("Exit ]");

		fgets( buf, LONGBUF, stdin );
		chop( buf );

		if ( strcasecmp( buf, "exit" ) == 0 || strcasecmp( buf, "e" ) == 0)
		{
			return 1;
		}
		if ( all &&
			( strcasecmp( buf, "all" ) == 0 || strcasecmp( buf, "a" ) == 0 ) ) {
			return 2;
		}
		*sel = json_selecti_object( json_dir_top, buf );

		if ( *sel != NULL ) { break; }
		printf( "Invalid selection.\n" );
	}
	return 0;
}

void copy_int( json_object_ptr json, json_object_ptr user, enum direction dir )
{
	int sel;
	char buf[SHORTBUF];
	json_object_ptr game;

	if ( dir != cp_down && dir != cp_up ) {
		assert( !"direction unknown" );
	}
	assert ( json_select_object( json, "file" )->type == JSON_OBJECT );

	while (1)
	{
		// Title
		switch ( dir ) {
			case cp_down:	printf("\n< Download Mode >\n");	break;
			case cp_up:		printf("\n< Upload Mode >\n");		break;
		}

		// Choise
		sel = select( &game, user, "Which Title?", true );
		if ( sel == 1 ) { // Exit is selected
			break;
		}
		else if ( sel == 2 ) { // All is selected
			copy_all( json, user, dir );
			continue;
		}

		game = json_select_object(
				(json_object_ptr)json_select_object( json, "file" )->value,
				game->string );
		assert( game != NULL );
		
		// Make sure
		printf("Are you sure?: %s : %s ",
				(char*)game->string, (char*)user->string);
		switch ( dir ) {
			case cp_down:	printf("<-- Box [ Y or other ]");	break;
			case cp_up:		printf("--> Box [ Y or other ]");	break;
		}
		fgets( buf, LONGBUF, stdin );
		chop( buf );
		if ( strcasecmp( buf, "yes" ) != 0 &&
				strcasecmp( buf, "y" ) != 0 )
		{ // If user deny, back to selection of game title.
			continue;
		}

		// �R�s�[���s�̊֐��Ăяo��
		copy_file( user, game, dir );
		// ���O��������
		write_log( user->string, game->string, dir );
	}// while (1) : < xxx Mode > loop
}

void copy_all( json_object_ptr json, json_object_ptr user, enum direction dir )
{
	json_object_ptr game_list, game_dir, file_list;

	if ( dir != cp_down && dir != cp_up ) {
		assert( !"direction unknown" );
	}

	// set game_list to top of the list
	assert( user->type == JSON_OBJECT );
	game_list = (json_object_ptr)user->value;

	// set file_list to omit json_select_object
	assert( json_select_object( json, "file" )->type == JSON_OBJECT );
	file_list = (json_object_ptr)json_select_object( json, "file" )->value;

	while ( game_list != NULL ) // game_list will be changed next to next.
	{
		game_dir = json_select_object( file_list, game_list->string );

		if ( game_dir == NULL ) {
			printf( "ERROR: no file-list exists: \"dir\"/\"%s\"\n",
					game_list->string );
		}
		else {
			// �R�s�[���s�̊֐��Ăяo��
			copy_file( user, game_dir, dir );
			// ���O��������
		}

		// set game_list to next
		game_list = game_list->next;
	} // while ( game_list == NULL ) // game_list will be changed next to next.

	write_log( user->string, "All", dir );
}

void copy_file( json_object_ptr user, json_object_ptr game, enum direction dir )
{
	char buf[LONGBUF];		// command
	json_array_ptr file;	// ���X�g�����ǂ�
	char c_game[SHORTBUF];
	char c_directory[LONGBUF];
	char c_file[LONGBUF];
	char c_option[SHORTBUF];

	assert( user->type == JSON_OBJECT );
	assert( game->type == JSON_ARRAY );
	file = game->value;
	assert( file->type == JSON_ARRAY );

	// set arguments for the command: all file common
	strcpy( c_game, (char*)game->string );
	strcpy( c_directory, (char*)json_select_object(
			(json_object_ptr)user->value, c_game)->value );

	while ( file != NULL )
	{
		// set arguments for the command: each file local
		strcpy( c_file,
			(char*)json_select_array( (json_array_ptr)file->value, 0)->value );
		strcpy( c_option,
			(char*)json_select_array( (json_array_ptr)file->value, 1)->value );
		switch ( dir )
		{
		case cp_up:
			sprintf( buf, "xcopy /d /y /f %s \"%s%s\" \".\\%s\\%s\"",
				c_option, c_directory, c_file, c_game, c_file );
			break;
		case cp_down:
			sprintf( buf, "xcopy /d /y /f %s \".\\%s\\%s\" \"%s%s\"",
				c_option, c_game, c_file, c_directory, c_file );
			break;
		}

		printf( "%s\n", buf );
		system( buf );

		file = file->next;
	}
}

//void write_log(
//		json_object_ptr json, char *user, char *game, enum direction dir )
//{
//	json_object_ptr log;
//	json_array_ptr log_each;
//
//	assert( json_select_object( json, "log" )->type == JSON_OBJECT );
//
//	// ���O��������
//	log = json_select_object(
//			(json_object_ptr)json_select_object( json, "log" )->value, game );
//	if ( log == NULL ) { // If game title doesn't exist, create it.
//		log = json_set_object(
//			json_select_object( json, "log" ), JSON_OBJECT, game );
//	}
//	json_set_array( log, JSON_OBJECT ); // set this log
//	log_each = json_select_array_last( (json_array_ptr)log->value );
//	json_set_array( log_each, JSON_ARRAY ); // [0] Down/Up
//	switch ( dir ) {
//	case cp_down:
//		json_set_bool(
//				json_select_array( (json_array_ptr)log_each->value, 0 ),
//				JSON_ARRAY, true );
//		break;
//	case cp_up:
//		json_set_bool(
//				json_select_array( (json_array_ptr)log_each->value, 0 ),
//				JSON_ARRAY, false );
//		break;
//	}
//	json_set_array( log_each, JSON_ARRAY ); // [1] User
//	json_set_string(
//			json_select_array( (json_array_ptr)log_each->value, 1 ),
//			JSON_ARRAY, user );
//	json_set_array( log_each, JSON_ARRAY ); // [2] Time
//	json_set_int(
//			json_select_array( (json_array_ptr)log_each->value, 2 ),
//			JSON_ARRAY, time( NULL ) );
//}

void write_log( char *user, char *game, enum direction dir )
{
	FILE *fp;
	time_t t;
	time( &t );
	fp = fopen( LOGFILE, "a+" );
	if ( fp == NULL )
	{
		printf( "write_log error: file '%s' cannot open.", LOGFILE );
	}
	fprintf( fp, "%s,%s,%s,%s",
			game,
			dir == cp_up ? "Up" : "Down",
			user,
			ctime( &t ) );
	fclose( fp );
}

//void show_log( json_object_ptr json )
//{
//	json_object_ptr game;
//	json_array_ptr log_each;
//
//	while (1)
//	{
//		// Title
//		printf("\n< Log View Mode >\n");
//
//		// Choise
//		if ( select( &game, json_select_object( json, "log" ),
//					"Which Title?", false ) == 1 )
//		{
//			break;
//		}
//		
//		// ���O�t�@�C���ǂݍ���-----------------
//		assert( game->type == JSON_ARRAY );
//		log_each = (json_array_ptr)game->value;
//		while ( log_each != NULL ) {
//			assert( log_each->type == JSON_ARRAY );
//			if ( *(json_bool_ptr)json_select_array(
//						(json_array_ptr)log_each->value, 0 )->value ) {
//				printf( "Down / " );
//			} else {
//				printf( "Up   / " );
//			}
//			printf( "%s / %s",
//					(char*)json_select_array(
//						(json_array_ptr)log_each->value, 1 )->value,
//					ctime( (time_t*)(json_int_ptr)json_select_array(
//							(json_array_ptr)log_each->value, 2 )->value )
//				);
//			log_each = log_each -> next;
//		} // while ( log_each != NULL )
//	} // while (1) : < Log View Mode > loop
//}

void show_log( json_object_ptr json )
{
	json_object_ptr game;
	char title[ SHORTBUF ];
	char buf[ LONGBUF ], titlebuf[ SHORTBUF ];
	FILE *fp;
	int sel;

	fp = fopen( LOGFILE, "r" );
	if ( fp == NULL )
	{
		printf( "show_log error: file '%s' cannot open.", LOGFILE );
	}

	while (1)
	{
		// Title
		printf("\n< Log View Mode >\n");

		// Choise
		sel =  select( &game, json_select_object( json, "file" ),
					"Which Title?", true );
		if ( sel == 1 )
		{
			break;
		}
		if ( sel != 2 )
		{
			strcpy( title, game->string );
		}
		
		// ���O�t�@�C���ǂݍ���-----------------
		fseek( fp, 0, SEEK_SET );
		while ( fgets( buf, LONGBUF, fp ) != NULL ) {
			if ( sel == 2 )
			{
				printf( "%s", buf );
				continue;
			}
			
			// like grep
			// "All" match
			strncpy( titlebuf, buf, 4 ); // 3(All) + 1(,)
			if ( titlebuf[ 3 ] == ',' ) {
				titlebuf[ 3 ] = '\0';
				if ( strcmp( "All", titlebuf ) == 0 ) {
					printf( "%s", buf );
				}
			}
			// each game title
			strncpy( titlebuf, buf, strlen(title)+1 );
			if ( titlebuf[ strlen(title) ] == ',' ) {
				titlebuf[ strlen(title) ] = '\0';
				if ( strcmp( title, titlebuf ) == 0 ) {
					printf( "%s", buf );
				}
			}
		}
	} // while (1) : < Log View Mode > loop

	fclose( fp );
}

//
// History
// 2013/11/12   2.03	���O�t�@�C����JSON�I�u�W�F�N�g�ƕ�����CSV��
// 2013/09/08	2.02	--no-interactive(-n,/n)�I�v�V�����ǉ�
//						����I�����A�Ō��See you��\�����Ȃ�
// 2013/08/31	2.01	all-upload/download�@�\�A�R�}���h���C���w��
// 						���O�������ݕ������֐���
// 2013/08/26	2.00	JSON�L�q�Ή��Bbat�s�v�B
// 2013/07/15	1.03	th135�����S�Y�O�Ή��A�ꕔ�t�@�C���͍X�V���t��r
// 2012/11/23	1.02	RoLO�Ή�
// 2012/10/22	1.01	�Г��E��(katamichi)�Ή��Alog�t�@�C�������s�A/nopause
// 2012/10/21	1.00	����
