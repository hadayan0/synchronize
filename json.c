// json.c
// Author: Hada Osamu
// Last modified: 2013/8/23

#include "json.h"

//======================================
// get next vaild character from 'fp'
char fgetvalidc( FILE* fp )
{
	char c;
	do { // get the first character
		c = fgetc( fp );
	} while ( c == ' ' || c == '\t' || c == '\n' );
	return c;
}

//======================================
// get data->value and data->type.
// return -1 when 'data' doesn't have these members.
int getchild(	json_data_ptr **child_value,json_type_t **child_type,
				json_data_ptr data,			json_type_t type )
{
	if ( type == JSON_OBJECT )
	{
		*child_type  = &( (json_object_ptr)data )->type;
		*child_value = &( (json_object_ptr)data )->value;
		return 0;
	}
	else if ( type == JSON_ARRAY )
	{
		*child_type  = &( (json_array_ptr)data )->type;
		*child_value = &( (json_array_ptr)data )->value;
		return 0;
	}
	return -1;
}

//======================================
// get line number from 'fp'
int get_linenum( FILE* fp )
{
	unsigned int num;
	long target, current;

	target = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	num = 0;
	do {
		if ( fgetc( fp ) == '\n' )
		{
			num++;
		}
		current = ftell( fp );
	} while ( target != current );

	return num;
}

//======================================
// print to 'fp' but '"' and '\' will be escape.
void fescape( FILE* fp, char *str )
{
	while ( *str != '\0' ) {
		switch ( *str )
		{
			case '"':
			case '\\':
			case '/':	fputc( '\\', fp ); fputc( *str, fp );	break;
			case '\b':	fputc( '\\', fp ); fputc( 'b', fp );	break;
			case '\f':	fputc( '\\', fp ); fputc( 'f', fp );	break;
			case '\n':	fputc( '\\', fp ); fputc( 'n', fp );	break;
			case '\r':	fputc( '\\', fp ); fputc( 'r', fp );	break;
			case '\t':	fputc( '\\', fp ); fputc( 't', fp );	break;
			default:	fputc( *str, fp );						break;
		}
		str++;
	}
}
