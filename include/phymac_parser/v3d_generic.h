
/*

 V3d Generic language structure
 --------------------------------


[AttributeName1(Argument1, Argument2, Parameter3 = Argument3)]
Qualifier1 Qualifer2 
{
	[AttributeName2]
	Qualifier1
	{
		qualifier1 qualifier2[indexer0][indexer1][indexer2];
		qualifier2 = 
		{
			value1,
			value2,
			value3
		};
	}

	[NoParse]
	{
		// Unparsed data
	}
}

 */

#pragma once

#include <phymac_parser/defines.h>
#include <phymac_parser/result.h>
#include <common/allocation_callbacks.h>

typedef struct u32_pair_t
{
	u32 start;
	u32 end;
} u32_pair_t;

#define U32_PAIR_INVALID ((u32_pair_t) { 0UL, 0UL })
#define U32_PAIR_IS_INVALID(pair) (((pair).start == 0UL) && ((pair).end == 0UL))
static FORCE_INLINE u32 U32_PAIR_DIFF(u32_pair_t pair) { return pair.end - pair.start; }

typedef struct v3d_generic_attribute_t
{
	u32_pair_t name; 			// name of the attribute/function
	u32_pair_t* arguments; 		// list of the arguments passed to the function
	u32_pair_t* parameters; 	// list of the possible parameters of the function
	u32 parameter_count; 		// number of possible parameters of the function
	u32 argument_count; 		// number of arguments passed to the function
} v3d_generic_attribute_t;

typedef struct v3d_generic_node_t v3d_generic_node_t;

typedef struct v3d_generic_node_t
{
	v3d_generic_node_t** childs; 			// list of childs of this node
	u32 child_count;	 					// number of childs of this node

	v3d_generic_attribute_t* attributes; 	// list of attributes of this node
	u32 attribute_count;					// number of attributes of this node

	u32_pair_t* qualifiers; 				// list of qualifiers of this node
	u32 qualifier_count; 					// number of qualifiers of this node

	u32_pair_t* indexers; 					// list of indexers
	u32 indexer_count; 						// number of indexers

	u32_pair_t unparsed;	 				// unparsed content if [NoParse] attribute applied to a node

	bool has_value; 						// true if this node has been assigned value, otherwise false
	v3d_generic_node_t* value;				// node contaning the value of this node

	bool is_block; 							// true if '<qualifiers> { }', false if '<qualifiers>;' etc.
} v3d_generic_node_t;

typedef struct ppsr_v3d_generic_parse_result_t
{
	v3d_generic_node_t* root;		// root node for the parsed AST
	char* log_buffer; 				// log buffer if there are any errors, warnings etc.
	ppsr_result_t result; 			// result code
} ppsr_v3d_generic_parse_result_t;


BEGIN_CPP_COMPATIBLE

/*
	description: parses the v3d generic file format
	params:
		1. string: ptr to the v3d generic file text (not necessarily null termianted)
		2. length: length of the v3d generic file text (exclusive of the null termination character)
	returns:
		ppsr_v3d_generic_parse_result_t containing the parsed AST, log buffer and result code
 */
PPSR_API ppsr_v3d_generic_parse_result_t ppsr_v3d_generic_parse(com_allocation_callbacks_t* callbacks, const char* string, u32 length);
PPSR_API void ppsr_v3d_generic_parse_result_destroy(com_allocation_callbacks_t* callbacks, ppsr_v3d_generic_parse_result_t result);
PPSR_API void debug_node(v3d_generic_node_t* node, const char* start);


/* helper functions for the user code */
PPSR_API v3d_generic_attribute_t* node_find_attribute(v3d_generic_node_t* node, const char* start, const char* attrName);
/* description: invokes 'visitor' function for each attribute for which 'predicate' returns true
 * params:
 *	node: pointer to the node object on which the attributes will be searched
 * 	start: pointer to the start of the main string (the original string which was used to parse)
 * 	predicate: function pointer which will be invoked for every attribute, it would return either true or false, if true, then only 'visitor' will be called
 *	visitor: function pointer which will be invoked each time an attribute with a matching name is found
 * 	user_data: pointer to the user data which will be passed to the visitor callback and predicate callback as the last argument
 * NOTE: if 'visitor' returns false then iteration stops and no subsequent calls to 'visitor' are made. */
PPSR_API void node_foreach_attribute(v3d_generic_node_t* node, const char* start, bool (*predicate)(u32_pair_t, const char* start, void* user_data), bool (*visitor)(v3d_generic_attribute_t* attr, const char* start, void* user_data), void* user_data);
/* description: invokes 'visitor' function for each attribute with name matching the supplied name 'attr_name' 
 * params:
 *	node: pointer to the node object on which the attributes will be searched
 * 	start: pointer to the start of the main string (the original string which was used to parse)
 * 	attr_name: c-string, name of the attribute to be searched and invoked the callback 'visitor'
 * 	visitor: function pointer which will be invoked each time an attribute with a matching name is found
 * 	user_data: pointer to the user data which will be passed to the visitor callback as the last argument
 * NOTE: if 'visitor' returns false then iteration stops and no subsequent calls to 'visitor' are made. */
PPSR_API void node_foreach_attribute_name(v3d_generic_node_t* node, const char* start, const char* attr_name, bool (*visitor)(v3d_generic_attribute_t* attr, const char* start, void* user_data), void* user_data);

END_CPP_COMPATIBLE
