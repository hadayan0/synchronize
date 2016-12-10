// sync.c

//==========================================================
// 各種定義など
//----------------------------------------------------------
#define VERSION     "2.05"                      // このファイル末尾に詳細を
#define SHORTBUF    30                          // 名前など短い文字列
#define LONGBUF     200                         // コマンドなど長い文字列
#define JSONFILE    "data.json"                 // ディレクトリパス等データ
#define LOGFILE     "log.csv"                   // 送受信ログ

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
void show_last_log( void );

int main( int argc, char **argv )
{
	int i;
	char buf[SHORTBUF];
	enum mode mode = mode_normal;
	_Bool interactive = true;
	
	json_object_ptr json = NULL;		// root
	json_object_ptr json_user = NULL;	// each user's game path
	
	printf("syncronize version %s\n", VERSION);

	printf("Last log: ");
	show_last_log();
	printf("\n");

	// jsonデータ読み込み-----------------------------------
	if ( read_json( &json ) == -1 ) {
		quit( json, EXIT_FAILURE, interactive );
	}

	// コマンドライン引数解読-------------------------------
	json_user = NULL;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp( argv[i], "-u" ) == 0 ||
		     strcmp( argv[i], "/u" ) == 0 ) {
			if ( ++i == argc ) {
				mode = mode_help;
				break;
			}
			json_user = json_selecti_object(
					(json_object_ptr)json_select_object( json, "dir" )->value,
					argv[i] );
		}
		else if (	strcmp( argv[i], "-U" )					== 0 ||
					strcmp( argv[i], "--all-upload" )		== 0 ||
					strcmp( argv[i], "/U" )					== 0 ) {
			mode = mode_all_up;
		}
		else if (	strcmp( argv[i], "-D" )					== 0 ||
					strcmp( argv[i], "--all-download" )		== 0 ||
					strcmp( argv[i], "/D" )					== 0 ) {
			mode = mode_all_down;
		}
		else if (	strcmp( argv[i], "-n" )					== 0 ||
					strcmp( argv[i], "--no-interactive" )	== 0 ||
					strcmp( argv[i], "/n" )					== 0 ) {
			interactive = false;
		}
		else { // invalid argument
			mode = mode_help;
			break;
		}
	}

	// 特殊モード動作---------------------------------------
	switch ( mode ) {
	case mode_help:
		printf( "usage: sync.exe [-U|-D] [-u username]\n" );
		printf( "-U\n" );
		printf( "--all-upload      all upload mode\n" );
		printf( "-D\n" );
		printf( "--all-download    all download mode\n" );
		printf( "-u username       set user to username\n" );
		printf( "-n\n" );
		printf( "--no_interactive  don't show message \"See you\"\n" );
		printf( "                  when finishes successfly.\n" );
		quit( json, EXIT_FAILURE, interactive );
	case mode_all_up:
		if ( json_user == NULL ) {
			printf( "%s: all-upload mode: no user selected.\n", argv[0] );
			quit( json, EXIT_FAILURE, interactive  );
		}
		else {
			copy_all( json, json_user, cp_up );
			quit( json, EXIT_SUCCESS, interactive  );
		}
	case mode_all_down:
		if ( json_user == NULL ) {
			printf( "%s: all-download mode: no user selected.\n", argv[0] );
			quit( json, EXIT_FAILURE, interactive  );
		}
		else {
			copy_all( json, json_user, cp_down );
			quit( json, EXIT_SUCCESS, interactive  );
		}
	case mode_normal:
		break;
	}
	
	// どのユーザ-------------------------------------------
	if ( json_user == NULL ) {
		if ( select( &json_user, json_select_object( json, "dir" ),
					"Who are you?", false ) == 1 ) {
			quit( json, EXIT_SUCCESS, interactive  );
		}
	}

	// メインループ-----------------------------------------
	while ( 1 ) {
		puts("\n< Main Menu >");
		printf("Hello %s. command? [ Down / Up / Log / Tree / Exit / ? ]", 
				json_user->string);
		fgets( buf, SHORTBUF , stdin );
		chop( buf );
		
		// DropBoxからダウンロード---------------------------
		if ( strcasecmp( buf, "down" ) == 0 || strcasecmp( buf, "d" ) == 0 )
		{
			copy_int( json, json_user, cp_down );
		}
		// DropBoxへアップロード-----------------------------
		else if ( strcasecmp( buf, "up" ) == 0 || strcasecmp( buf, "u" ) == 0 )
		{
			copy_int( json, json_user, cp_up );
		}
		// ログ表示------------------------------------------
		else if ( strcasecmp( buf, "log" ) == 0 || strcasecmp( buf, "l" ) == 0 )
		{
			show_log( json );
		}
		// JSONデータツリー表示------------------------------
		else if ( strcasecmp( buf, "tree" ) == 0 || strcasecmp( buf, "t" ) == 0 )
		{
			json_show( stdout, json, JSON_OBJECT, "    ", showex_ex );
		}
		// 終了----------------------------------------------
		else if ( strcasecmp( buf, "exit" ) == 0 || strcasecmp( buf, "e" ) == 0 )
		{
			break;
		}
		// 説明----------------------------------------------
		else if ( strcmp( buf, "?" ) == 0 )
		{
			puts("commands...");
			puts("Down\tDownload from Dropbox to your computer\t( <-- Box )");
			puts("Up\tUpload from your computer to Dropbox\t( --> Box )");
			puts("Log\tShow log file");
			puts("Tree\tShow JSON tree");
			puts("Exit\tExit");
			puts("?\tShow this help");
		}
		else
		{
			puts("Invalid command.");
			puts("Use '?' command to show help.");
		}
	} // while ( 1 )
	
	quit( json, EXIT_SUCCESS, interactive );
	return 0;
}

void quit( json_object_ptr json, int status, _Bool interactive )
{
	char buf[2];

	if ( json != NULL ) {
		//write_json( json );
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

		printf( "%s [ ", ask );		// 質問文
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

		// コピー実行の関数呼び出し
		copy_file( user, game, dir );
		// ログ書き込み
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
			// コピー実行の関数呼び出し
			copy_file( user, game_dir, dir );
		}

		// set game_list to next
		game_list = game_list->next;
	} // while ( game_list == NULL ) // game_list will be changed next to next.

	// ログ書き込み
	write_log( user->string, "All", dir );
}

void copy_file( json_object_ptr user, json_object_ptr game, enum direction dir )
{
	char buf[LONGBUF];		// command
	json_array_ptr file;	// リストをたどる
	char c_game[SHORTBUF];
	char c_directory[LONGBUF];
	char c_file[LONGBUF];
	char c_option[SHORTBUF];
	int  system_return;

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
		printf( "copying %s\\%s :", c_game, c_file );

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

		//printf( "%s\n", buf );
		if ( (system_return = system( buf )) != 0 )
		{
			printf( "xcopy command return: %d\n", system_return );
		}

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
//	// ログ書き込み
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
//		// ログファイル読み込み-----------------
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
		
		// ログファイル読み込み-----------------
		fseek( fp, 0, SEEK_SET );
		while ( fgets( buf, LONGBUF, fp ) != NULL )
		{
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

void show_last_log( void )
{
	char buf[ LONGBUF ];
	FILE *fp;

	fp = fopen( LOGFILE, "r" );
	if ( fp == NULL )
	{
		printf( "show_log error: file '%s' cannot open.", LOGFILE );
	}

	// seek to the last line
	while ( fgets( buf, LONGBUF, fp ) != NULL );

	printf( "%s", buf );

	fclose( fp );
}
