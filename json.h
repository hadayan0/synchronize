// json.h
// Author: Hada Osamu
// Last modified: 2013/8/26 ( int means long )

#ifndef __JSON_H__
#define __JSON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

//======================================
// buffer
//--------------------------------------
#define BUFSIZE 256


//======================================
// data type definition
//--------------------------------------
typedef enum json_type
{
	JSON_NONE,
	JSON_OBJECT, JSON_ARRAY,
	JSON_STRING, JSON_INT, JSON_DOUBLE,
	JSON_BOOL, JSON_NULL
} json_type_t;

typedef void*			json_data_ptr;
typedef char*			json_string_ptr;
typedef long*			json_int_ptr;
typedef double*			json_double_ptr;
typedef _Bool*			json_bool_ptr;
//typedef void*			json_null_ptr;

typedef struct json_object
{
	json_string_ptr		string;
	json_data_ptr		value;
	json_type_t			type;
	struct json_object	*next;
} json_object_t;
typedef json_object_t	*json_object_ptr;

typedef struct json_array
{
	json_data_ptr		value;
	json_type_t		 	type;
	struct json_array	*next;
} json_array_t;
typedef json_array_t	*json_array_ptr;

//typedef struct json_root // root
//{
//	json_data_ptr		value;
//	json_type_t			type;
//} json_root_t;
//typedef json_root_t		*json_root_ptr;
//
//typedef json_root_t		json_t[1];


//======================================
// function definition
//--------------------------------------

// json.c
// get next vaild character from 'fp'
char fgetvalidc( FILE* fp );
// get data->value and data->type.
// return -1 when 'data' doesn't have these members.
int getchild(	json_data_ptr **child_value,json_type_t **child_type,
				json_data_ptr data,			json_type_t type );
// get line number from 'fp'
int get_linenum( FILE* fp );
// print to 'fp' but '"' and '\' will be escape.
void fescape( FILE* fp, char *str );

// json_new.c
// allocate json data and returns it's pointer.
json_object_ptr	json_new_object( char *string );
json_array_ptr	json_new_array( void );
json_string_ptr	json_new_string( char *string );
json_int_ptr	json_new_int( long value );
json_double_ptr	json_new_double( double value );
json_bool_ptr	json_new_bool( _Bool value );

// json_clear.c
// clear memory from json data recursively.
void json_clear( json_data_ptr data, json_type_t type );
//void json_clear( json_t data );

// json_show.c
// show all json data to 'fp'.
enum showex { showex_normal, showex_ex };
void json_show_re( FILE *fp, json_data_ptr data, json_type_t type, int nest,
		char *indent, enum showex mode );
void json_show( FILE *fp, json_data_ptr data, json_type_t type,
		char *indent, enum showex mode );

// json_select.c
// select from object or array and return it's pointer.
// returns NULL when 'string' not found.
// selecti ignores whether big char or small char.
json_object_ptr	json_select_object( json_object_ptr data, char *string );
json_array_ptr	json_select_array( json_array_ptr data, int num );
json_object_ptr	json_select_object_last( json_object_ptr data );
json_array_ptr	json_select_array_last( json_array_ptr data );
json_object_ptr	json_selecti_object( json_object_ptr data, char *string );

// json_set.c
// return: added child pointer
// call json_new_ function and set it to 'data's child.
json_object_ptr	json_set_object(
		json_data_ptr data, json_type_t type, char *string );
json_array_ptr	json_set_array(
		json_data_ptr data, json_type_t type );
json_string_ptr	json_set_string(
		json_data_ptr data, json_type_t type, char *string );
json_int_ptr	json_set_int(
		json_data_ptr data, json_type_t type, long value );
json_double_ptr	json_set_double(
		json_data_ptr data, json_type_t type, double value );
json_bool_ptr	json_set_bool(
		json_data_ptr data, json_type_t type, _Bool value );
_Bool			json_set_null(
		json_data_ptr data, json_type_t type );
json_object_ptr	json_add_object( json_object_ptr data, char *string );
json_array_ptr	json_add_array( json_array_ptr data );

// json_parse.c
// parse json data from 'fp' to 'data' and 'type'
// This allocate memories so must be cleared by json_clear()
// return:	0	when succeed
// 			-1	when 'file' can't open
// 			-2	when malloc failed
// 			-3	when syntax error occured
// 			-4	when don't have enough buffer ( Sometime I'll fix it. )
// 			-5	invalid 'data' token
// if error occured, json_parse_fp() don't free the memory.
// so please check return value, and if non-zero, CALL json_clear() FUNCTION.
// or, call json_parse_file() to free the memory automaticaly.
int json_parse_fp( json_data_ptr *data, json_type_t *type, FILE *fp );
int json_parse_file( json_data_ptr *data, json_type_t *type, char *file );

#endif // #define __JSON_H__
