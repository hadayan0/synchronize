// json_select.c
// Author: Hada Osamu
// Last modified: 2013/8/23

#include "json.h"

//======================================
// select from object or array and return it's pointer.
json_object_ptr	json_select_object( json_object_ptr data, char *string )
{
	while ( data != NULL )
	{
		if ( strcmp( data->string, string ) == 0 )
		{
			break;
		}
		data = data->next;
	}
	return data;
}

//======================================
json_array_ptr	json_select_array( json_array_ptr data, int num )
{
	int i=0;
	while ( data != NULL )
	{
		if ( i == num )
		{
			break;
		}
		data = data->next;
		i++;
	}
	return data;
}

//======================================
json_object_ptr	json_select_object_last( json_object_ptr data )
{
	if ( data == NULL )
	{
		return NULL;
	}
	while ( data->next != NULL )
	{
		data = data->next;
	}
	return data;
}

//======================================
json_array_ptr	json_select_array_last( json_array_ptr data )
{
	if ( data == NULL )
	{
		return NULL;
	}
	while ( data->next != NULL )
	{
		data = data->next;
	}
	return data;
}

//======================================
// select from object or array and return it's pointer.
json_object_ptr	json_selecti_object( json_object_ptr data, char *string )
{
	while ( data != NULL )
	{
		if ( strcasecmp( data->string, string ) == 0 )
		{
			break;
		}
		data = data->next;
	}
	return data;
}
