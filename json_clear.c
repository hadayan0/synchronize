// json_clear.c
// Author: Hada Osamu
// Last modified: 2013/8/23

#include "json.h"

//======================================
// clear memory from json data (recursive function)
void json_clear( json_data_ptr data, json_type_t type )
{
	if ( data == NULL )
	{
		return;
	}
	switch ( type )
	{
	case JSON_NONE:
	case JSON_NULL: // these types don't allocate values
		return;
	case JSON_STRING:
	case JSON_INT:
	case JSON_DOUBLE: // these types mean they're leaf nodes
	case JSON_BOOL:
		break;
	case JSON_OBJECT: // call recursive function
		free(			( (json_object_ptr)data )->string );
		json_clear(	( (json_object_ptr)data )->value,
						( (json_object_ptr)data )->type );
		if ( ( (json_object_ptr)data )->next != NULL )
		{
			json_clear(	( (json_object_ptr)data )->next, JSON_OBJECT );
		}
		break;
	case JSON_ARRAY: // call recursive function
		json_clear(	( (json_array_ptr)data )->value,
						( (json_array_ptr)data )->type );
		if ( ( (json_array_ptr)data )->next != NULL )
		{
			json_clear(	( (json_array_ptr)data )->next, JSON_ARRAY );
		}
		break;
	default: // invalid json type
		assert( !"invalid json_type_t type" );
	}
	free( data );
}

//======================================
// clear memory from json data (for root)
//void json_clear( json_root_ptr root )
//{
//	json_clear_re( root->value, root->type );
//	root->type = JSON_NONE;
//}
