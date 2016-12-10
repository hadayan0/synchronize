// json_show.c
// Author: Hada Osamu
// Last modified: 2013/8/23

#include "json.h"

//======================================
// show json data to 'fp' (recursive function)
void json_show_re( FILE *fp, json_data_ptr data, json_type_t type, int nest,
		char *indent, enum showex mode )
{
	int i;

	switch ( type )
	{
	case JSON_NONE:
		break;
	case JSON_NULL:
		fprintf( fp, "null" );
		break;
	case JSON_STRING:
		if ( mode == showex_normal ) {
			fprintf( fp, "\"%s\"", (char*)data );
		}
		else {
			fprintf( fp, "\"" );
			fescape( fp, (char*)data );
			fprintf( fp, "\"" );
		}
		break;
	case JSON_INT:
		fprintf( fp, "%d", *(int*)data );
		break;
	case JSON_DOUBLE:
		fprintf( fp, "%f", *(double*)data );
		break;
	case JSON_BOOL:
		if ( *(_Bool*)data )	{ fprintf( fp, "true" ); }
		else					{ fprintf( fp, "false" ); }
		break;

	case JSON_OBJECT: // call recursive function
		// If this is the first element, "{\n" has been placed.
		fprintf( fp, "\"%s\":", ( (json_object_ptr)data )->string );

		// If the child is object or array, it's the first element
		switch ( ( (json_object_ptr)data )->type )
		{
		case JSON_OBJECT:
			fprintf( fp, "{\n" );
			for ( i=0; i<nest+1; i++ ) { fprintf( fp, indent ); }
			break;
		case JSON_ARRAY:
			fprintf( fp, "[\n" );
			for ( i=0; i<nest+1; i++ ) { fprintf( fp, indent ); }
			break;
		default:
			break;
		}
		json_show_re( fp, ( (json_object_ptr)data )->value,
				( (json_object_ptr)data )->type, nest+1, indent, mode );

		if ( ( (json_object_ptr)data )->next == NULL )
		{
			// This is the last element, so close bracket.
			fprintf( fp, "\n" );
			for ( i=0; i<nest-1; i++ ) { fprintf( fp, indent ); }
			fprintf( fp, "}" );
		}
		else
		{
			fprintf( fp, ",\n" );
			for ( i=0; i<nest; i++ ) { fprintf( fp, indent ); }
			json_show_re( fp, ( (json_object_ptr)data )->next,
					JSON_OBJECT, nest, indent, mode );
		}
		break;

	case JSON_ARRAY: // call recursive function
		// If this is the first element, "{\n" has been placed.

		// If the child is object or array, it's the first element
		switch ( ( (json_array_ptr)data )->type )
		{
		case JSON_OBJECT:
			fprintf( fp, "{\n" );
			for ( i=0; i<nest+1; i++ ) { fprintf( fp, indent ); }
			break;
		case JSON_ARRAY:
			fprintf( fp, "[\n" );
			for ( i=0; i<nest+1; i++ ) { fprintf( fp, indent ); }
			break;
		default:
			break;
		}
		json_show_re( fp, ( (json_array_ptr)data )->value,
				( (json_array_ptr)data )->type, nest+1, indent, mode );

		if ( ( (json_array_ptr)data )->next == NULL )
		{
			// This is the last element, so close bracket.
			fprintf( fp, "\n" );
			for ( i=0; i<nest-1; i++ ) { fprintf( fp, indent ); }
			fprintf( fp, "]" );
		}
		else
		{
			fprintf( fp, ",\n" );
			for ( i=0; i<nest; i++ ) { fprintf( fp, indent ); }
			json_show_re( fp, ( (json_array_ptr)data )->next,
					JSON_ARRAY, nest, indent, mode );
		}
		break;

	default: // invalid json type
		assert( !"invalid json_type_t type" );
	}
}

//======================================
// show json data to 'fp' (for root)
void json_show( FILE *fp, json_data_ptr data, json_type_t type,
		char *indent, enum showex mode )
{
	switch ( type )
	{
	case JSON_OBJECT:
		fprintf( fp, "{\n%s", indent );
		break;
	case JSON_ARRAY:
		fprintf( fp, "[\n%s", indent );
		break;
	default:
		break;
	}
	json_show_re( fp, data, type, 1, indent, mode );
	fprintf( fp, "\n" );
}
