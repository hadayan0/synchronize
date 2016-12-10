// json_parse.c
// Author: Hada Osamu
// Last modified: 2013/8/26 ( available when type==NULL )

#include "json.h"

//#define DEBUG

//======================================
// parse json data from 'fp' to 'data' and 'type'
// This allocate memories so must be cleared by json_clear()
// return:	0	when succeed
// 			-1	when 'file' can't open
// 			-2	when malloc failed
// 			-3	when syntax error occured
// 			-4	when don't have enough buffer ( Sometime I'll fix it. )
// 			-5	invalid 'data' token
// if error occured, this function don't free the memory.
// so please check return value, and if non-zero, CALL json_clear() FUNCTION.
int json_parse_fp( json_data_ptr *data, json_type_t *type, FILE *fp )
{
	int i;
	char c;
	char buf[BUFSIZE];
	json_type_t type_buf;
	_Bool pointed;

	if ( data == NULL ) { return -5; } // 'data' can't be used

	switch ( c = fgetvalidc( fp ) )
	{
	case EOF:
		return -3; // syntax error

	case 't':	case 'T':	// True of bool
#ifdef DEBUG
		printf("T");
#endif
		fgets( buf, 4, fp );
		if (	strcmp( buf, "rue" ) == 0	||
				strcmp( buf, "RUE" ) == 0	)
		{	// match
			*data = json_new_bool( true );
			if ( *data == NULL ) { return -2; } // malloc failed
			if ( type != NULL ) { *type = JSON_BOOL; }
			return 0; // success
		}
		return -3; // syntax error

	case 'f':	case 'F':
#ifdef DEBUG
		printf("F");
#endif
		fgets( buf, 5, fp );
		if (	strcmp( buf, "alse" ) == 0	||
				strcmp( buf, "ALSE" ) == 0	)
		{	// match
			*data = json_new_bool( false );
			if ( *data == NULL ) { return -2; } // malloc failed
			if ( type != NULL ) { *type = JSON_BOOL; }
			return 0; // success
		}
		return -3; // syntax error

	case 'n':	case 'N':
#ifdef DEBUG
		printf("N");
#endif
		fgets( buf, 4, fp );
		if (	strcmp( buf, "ull" ) == 0	||
				strcmp( buf, "ULL" ) == 0	)
		{	// match
			if ( type != NULL ) { *type = JSON_NULL; }
			return 0; // success
		}
		return -3; // syntax error

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '-':
#ifdef DEBUG
		printf("1");
#endif
		ungetc( c, fp );
		pointed = false;
		for ( i = 0; i < BUFSIZE; i++ )
		{
			buf[i] = fgetc( fp );
			if ( ( buf[i] >= '0' && buf[i] <= '9' ) ||
					buf[i] == '+' || buf[i] == '-' )
			{ // valid character
				continue;
			}
			if ( buf[i] == '.' || buf[i] == 'e' || buf[i] == 'E' )
			{ // valid character meaning floating point
				pointed = true;
				continue;
			}
			// invalid character occured. --> the number ended.
			ungetc( buf[i], fp ); // invalid char should be read again
			buf[i] = '\0';
			if ( pointed )
			{ // floating point
				*data = json_new_double( atof( buf ) );
				if ( type != NULL ) { *type = JSON_DOUBLE; }
			}
			else
			{ // integer
				*data = json_new_int( atoi( buf ) );
				if ( type != NULL ) { *type = JSON_INT; }
			}
			if ( *data == NULL ) { return -2; } // malloc failed
			return 0;	// success
		}
		// buffer is not enough
		return -4;

	case '"':
#ifdef DEBUG
		printf("\"");
#endif
		for ( i = 0; i < BUFSIZE; i++ )
		{
			buf[i] = fgetc( fp );
			if ( buf[i] == '\\' ) // means one '\'
			{ // escape sequence ( control character )
				switch ( c = fgetc( fp ) )
				{
					case '"':	buf[i] = '"';	break;
					case '\\':	buf[i] = '\\';	break;
					case '/':	buf[i] = '/';	break;
					case 'b':	buf[i] = '\b';	break;
					case 'f':	buf[i] = '\f';	break;
					case 'n':	buf[i] = '\n';	break;
					case 'r':	buf[i] = '\r';	break;
					case 't':	buf[i] = '\t';	break;
					//case 'u':	break; // Sometime I'll write...
					default: // NOT JSON RULE: single '\' doesn't escape
						ungetc( c, fp );
						break;
				}
			}
			else if ( buf[i] == '"' )
			{
				buf[i] = '\0';
				*data = json_new_string( buf );
				if ( *data == NULL ) { return -2; } // malloc failed
				if ( type != NULL ) { *type = JSON_STRING; }
				return 0;	// success
			}
			else if ( buf[i] == EOF ) {	return -3; } // syntax error
		}
		// buffer is not enough
		return -4;

	case '{':
#ifdef DEBUG
		printf("{");
#endif
		*data = json_new_object( NULL );
		if ( *data == NULL ) { return -2; } // malloc failed
		if ( type != NULL ) { *type = JSON_OBJECT; }
		while ( 1 ) // list loop. keep loop while ',' occuring only
		{
			// pair: string
			i = json_parse_fp(
					(json_data_ptr*)&( ( (json_object_ptr)*data )->string ),
					&type_buf, fp );
			if ( i != 0 )
			{ // when error occured
				//json_clear( *data, *type );
				return i;
			}
			if ( type_buf != JSON_STRING )
			{ // here allowed string only
				//json_clear( *data, *type );
				return -3; // syntax error
			}
			
			// ':'
			if ( fgetvalidc( fp ) != ':' )
			{
				//json_clear( *data, *type );
				return -3; // syntax error
			}
			// pair: value
			i = json_parse_fp( &(((json_object_ptr)*data)->value),
					&(((json_object_ptr)*data)->type), fp );
			if ( i != 0 )
			{ // when error occured
				//json_clear( *data, *type );
				return i;
			}
			
			// ',' or '}'
			switch( fgetvalidc( fp ) )
			{
			case ',': // continue to next element
				((json_object_ptr)*data)->next = json_new_object( NULL );
				if ( ((json_object_ptr)*data)->next == NULL )
				{
					return -2; // malloc failed
				}
				data = (json_data_ptr*)&( ( (json_object_ptr)*data )->next );
				break;
			case '}': // end of object list
				return 0; // success
			default:
				return -3;	// syntax error
			}
		}
		assert( !"json_parse_fp(): object loop out" );

	case '[':
#ifdef DEBUG
		printf("A");
#endif
		*data = json_new_object( NULL );
		*data = json_new_array();
		if ( *data == NULL ) { return -2; } // malloc failed
		if ( type != NULL ) { *type = JSON_ARRAY; }
		while ( 1 ) // list loop. keep loop while ',' occuring only
		{
			//  value
			i = json_parse_fp( &( ( (json_array_ptr)*data )->value ),
					&( ( (json_array_ptr)*data )->type ), fp );
			if ( i != 0 )
			{ // when error occured
				//json_clear( *data, *type );
				return i;
			}
			
			// ',' or ']'
			switch( fgetvalidc( fp ) )
			{
			case ',': // continue to next element
				((json_array_ptr)*data)->next = json_new_array();
				if ( ((json_array_ptr)*data)->next == NULL )
				{
					return -2; // malloc failed
				}
				data = (json_data_ptr*)&((json_array_ptr)*data)->next;
				break;
			case ']': // end of array list
				return 0; // success
			default:
				return -3;// syntax error
			}
		}
		assert( !"json_parse_fp(): array loop out" );

	default:
		return -3; // syntax error
	}
}

//======================================
int json_parse_file( json_data_ptr *data, json_type_t *type, char *file )
{
	int res;
	FILE *fp;
	
	fp = fopen( file, "r" );
	if ( fp == NULL)
	{
		return -1;
	}

	res = json_parse_fp( data, type, fp );
	if ( res != 0 ) {
		printf("json_parse_file(): %d returned.\n", res);
		json_clear( *data, *type );
		*data = NULL;
		if ( type == NULL ) { *type = JSON_NONE; }
		printf("json_parse_file(): error occured around line %d\n",
				get_linenum( fp ) );
	}

	fclose( fp );

	return res;
}
