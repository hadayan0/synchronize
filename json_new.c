// json_new.c
// Author: Hada Osamu
// Last modified: 2013/8/26 ( int means long )

#include "json.h"

//======================================
// allocate json data and returns it's pointer.
json_object_ptr json_new_object( char* string )
{
	json_object_ptr json;

	json = (json_object_ptr) malloc( sizeof( json_object_t ) );
	if ( json == NULL )
	{
		return NULL;
	}

	if ( string != NULL )
	{
		json->string = json_new_string( string );
		if ( json->string == NULL )
		{
			return NULL;
		}
	}
	else
	{
		json->string = NULL;
	}

	json->value = NULL;
	json->type = JSON_NONE;
	json->next = NULL;

	return json;
}

//======================================
json_array_ptr json_new_array( void )
{
	json_array_ptr json;

	json = (json_array_ptr) malloc( sizeof( json_array_t ) );
	if ( json == NULL )
	{
		return NULL;
	}

	json->value = NULL;
	json->type = JSON_NONE;
	json->next = NULL;

	return json;
}

//======================================
json_string_ptr json_new_string( char* string )
{
	json_string_ptr str;

	str = (json_string_ptr) malloc( sizeof(char) * ( strlen(string) + 1 ) );
	if ( str == NULL )
	{
		return NULL;
	}

	strcpy( str, string );

	return str;
}

//======================================
json_int_ptr json_new_int( long value )
{
	json_int_ptr val;
	val = (json_int_ptr) malloc( sizeof(long) );
	*val = value;
	return val;
}

//======================================
json_double_ptr json_new_double( double value )
{
	json_double_ptr val;
	val = (json_double_ptr) malloc( sizeof(double) );
	*val = value;
	return val;
}

//======================================
json_bool_ptr json_new_bool( _Bool value )
{
	json_bool_ptr val;
	val = (json_bool_ptr) malloc( sizeof(_Bool) );
	*val = value;
	return val;
}
