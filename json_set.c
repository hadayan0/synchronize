// json_set.c
// Author: Hada Osamu
// Last modified: 2013/8/26 ( int means long

#include "json.h"

// return: added child pointer

//======================================
// call json_new_ function and set it to 'data's child.
json_object_ptr	json_set_object(
		json_data_ptr data, json_type_t type, char *string )
{
	json_type_t *child_type;
	json_data_ptr *child_value;

	if ( getchild( &child_value, &child_type, data, type ) )
	{
		return NULL;
	}

	if ( *child_type == JSON_NONE ) // no child.
	{
		*child_type = JSON_OBJECT;
		return *child_value = json_new_object( string );
	}
	else if ( *child_type == JSON_OBJECT ) // child data type matches.
	{
		return json_select_object_last( *child_value )->next
			= json_new_object( string );
	}
	// child data type doesn't match. so can't have child.
	return NULL;
}

//======================================
json_array_ptr	json_set_array(
		json_data_ptr data, json_type_t type )
{
	json_type_t *child_type;
	json_data_ptr *child_value;

	if ( getchild( &child_value, &child_type, data, type ) )
	{
		return NULL;
	}

	if ( *child_type == JSON_NONE ) // no child.
	{
		*child_type = JSON_ARRAY;
		return *child_value = json_new_array();
	}
	else if ( *child_type == JSON_ARRAY ) // child data type matches.
	{
		return json_select_array_last( *child_value )->next
			= json_new_array();
	}
	// child data type doesn't match. so can't have child.
	return NULL;
}

//======================================
json_string_ptr	json_set_string(
		json_data_ptr data, json_type_t type, char *string )
{
	json_type_t *child_type;
	json_data_ptr *child_value;

	if ( getchild( &child_value, &child_type, data, type ) )
	{
		return NULL;
	}

	if ( *child_type == JSON_NONE ) // no child.
	{
		*child_type = JSON_STRING;
		return *child_value = json_new_string( string );
	}
	// child data type doesn't match. so can't have child.
	return NULL;
}

//======================================
json_int_ptr	json_set_int(
		json_data_ptr data, json_type_t type, long value )
{
	json_type_t *child_type;
	json_data_ptr *child_value;

	if ( getchild( &child_value, &child_type, data, type ) )
	{
		return NULL;
	}

	if ( *child_type == JSON_NONE ) // no child.
	{
		*child_type = JSON_INT;
		return *child_value = json_new_int( value );
	}
	// child data type doesn't match. so can't have child.
	return NULL;
}

//======================================
json_double_ptr	json_set_double(
		json_data_ptr data, json_type_t type, double value )
{
	json_type_t *child_type;
	json_data_ptr *child_value;

	if ( getchild( &child_value, &child_type, data, type ) )
	{
		return NULL;
	}

	if ( *child_type == JSON_NONE ) // no child.
	{
		*child_type = JSON_DOUBLE;
		return *child_value = json_new_double( value );
	}
	// child data type doesn't match. so can't have child.
	return NULL;
}

//======================================
json_bool_ptr	json_set_bool(
		json_data_ptr data, json_type_t type, _Bool value )
{
	json_type_t *child_type;
	json_data_ptr *child_value;

	if ( getchild( &child_value, &child_type, data, type ) )
	{
		return NULL;
	}

	if ( *child_type == JSON_NONE ) // no child.
	{
		*child_type = JSON_BOOL;
		return *child_value = json_new_bool( value );
	}
	// child data type doesn't match. so can't have child.
	return NULL;
}

//======================================
_Bool			json_set_null( json_data_ptr data, json_type_t type )
{
	json_type_t *child_type;
	json_data_ptr *child_value;

	if ( getchild( &child_value, &child_type, data, type ) )
	{
		return false;
	}

	if ( *child_type == JSON_NONE ) // no child.
	{
		*child_type = JSON_NULL;
		return true;
	}
	// child data type doesn't match. so can't have child.
	return false;
}

//======================================
// call json_new_ function and set it to 'data's next pointer.
json_object_ptr	json_add_object( json_object_ptr data, char *string )
{
	if ( data == NULL )
	{
		return NULL;
	}
	return json_select_object_last( data )->next = json_new_object( string );
}

//======================================
json_array_ptr	json_add_array( json_array_ptr data )
{
	if ( data == NULL )
	{
		return NULL;
	}
	return json_select_array_last( data )->next = json_new_array();
}
