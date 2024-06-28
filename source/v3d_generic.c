
#include <phymac_parser/v3d_generic.h>
#include <phymac_parser/debug.h>
#include <phymac_parser/string.h> // for custom string functions
#include <string.h>		// for memory and string functions

// NOTE: earlier this used to be <BufferLib/buffer.h>, since linux is case-sensitive, we need to use <bufferlib/buffer.h> here.
#include <bufferlib/buffer.h>


#define NO_PARSE_ATTRIB "NoParse"

static const char* check(const char* str, const char* const end)
{
	if(str >= end)
		DEBUG_LOG_FETAL_ERROR("Unexpected end of file while parsing, exiting");
	return str;
}

static void expected(const char* exp_str, const char* str, const char* const end)
{
	DEBUG_LOG_FETAL_ERROR("Unexpected \"%.*s\", expected %s", get_token_length(str, end), str, exp_str);
}

static v3d_generic_node_t* node_create(com_allocation_callbacks_t* callbacks)
{
	v3d_generic_node_t* node = (v3d_generic_node_t*)com_call_allocate(callbacks, sizeof(v3d_generic_node_t));
	memset(node, 0, sizeof(v3d_generic_node_t));
	return node;
}

static const char* get_token(const char* str, const char* delimiters, const char* const start, const char* const end, u32_pair_t OUT pair)
{
	u32_pair_t value = { str - start, skip_until(str, delimiters, end) - start };
	str = skip_ws(str + U32_PAIR_DIFF(value), end);	
	pair->start = value.start;
	pair->end = value.end;
	return str;
}

typedef struct attrib_str_pair_t
{
	v3d_generic_attribute_t* attributes;
	u32 attribute_count;
	const char* str;
} attrib_str_pair_t;

static attrib_str_pair_t parse_attributes(com_allocation_callbacks_t* callbacks, const char* str, const char* const start, const char* const end, bool OUT is_parse)
{
	BUFFER attributes = buf_new_with_callbacks(callbacks, v3d_generic_attribute_t);
	while(*str == '[')
	{
		str = skip_ws(str + 1, end);
		v3d_generic_attribute_t attribute;
		const char* _str = get_token(str, "(]\t\n\r ", start, end, &attribute.name);
		if(strncmp(NO_PARSE_ATTRIB, str, U32_PAIR_DIFF(attribute.name)) == 0)
			*is_parse = false;
		str = _str;
		BUFFER parameters = buf_new_with_callbacks(callbacks, u32_pair_t);
		BUFFER arguments = buf_new_with_callbacks(callbacks, u32_pair_t);
		if(*str == '(')
		{
			str++;
			while(*str != ')')
			{
				str = skip_ws(str, end);
				u32_pair_t pair;
				str = get_token(str, ",=)]\t\n\r ", start, end, &pair);
				bool found_param = false;
L1:
				switch(*str)
				{
					case ',':
						str = check(str + 1, end);
					case ')':
						if(!found_param)
						{
							buf_push(&arguments, &pair);
							buf_push_pseudo(&parameters, 1);
							found_param = false;
						}
						continue;
					case '=':
						buf_push(&parameters, &pair);
						str = skip_ws(str + 1, end);
						str = get_token(str, ",)]\t\n\r ", start, end, &pair);
						buf_push(&arguments, &pair);
						found_param = true;
						goto L1;
					default:
						expected("\",\" \"=\" or \")\"", str, end);
				}
			}
			str = skip_ws(str + 1, end);
		}
		if(*str != ']')
			expected("\"]\"", str, end);
		str = skip_ws(str + 1, end);
		attribute.arguments = buf_get_ptr(&arguments);
		attribute.argument_count = buf_get_element_count(&arguments);
		attribute.parameters = buf_get_ptr(&parameters);
		attribute.parameter_count = buf_get_element_count(&parameters);
		buf_push(&attributes, &attribute);
	}
	return (attrib_str_pair_t) { buf_get_ptr(&attributes), buf_get_element_count(&attributes), str };
}

static bool node_is_empty(v3d_generic_node_t* node)
{
	return (node == NULL) || ((node->indexer_count == 0) && (node->child_count == 0) && (node->attribute_count == 0) && (node->qualifier_count == 0) && (node->unparsed.start == node->unparsed.end));
}

typedef struct node_str_pair_t
{
	v3d_generic_node_t* node;
	const char* str;
} node_str_pair_t;

static node_str_pair_t parse(com_allocation_callbacks_t* callbacks, const char* str, const char* const start, const char* const end)
{
	str = skip_ws(str, end);

	v3d_generic_node_t* node = node_create(callbacks);
	bool is_parse = true;
	attrib_str_pair_t attributes = parse_attributes(callbacks, str, start, end, &is_parse);
	str = attributes.str;
	node->attributes = attributes.attributes;
	node->attribute_count = attributes.attribute_count;

	BUFFER list = buf_new_with_callbacks(callbacks, u32_pair_t);
	char buffer[2] = { *str, 0 };
	while(strpbrk("{};,=[", buffer) == NULL)
	{
		u32_pair_t pair;
		str = get_token(str, ",{[;\t\n\r ", start, end, &pair);
		buf_push(&list, &pair);
		buffer[0] = *str;
	}

	node->qualifiers = buf_get_ptr(&list);
	node->qualifier_count = buf_get_element_count(&list);

L2:
	switch(*str)
	{
		case '{':
			node->is_block = true;
			str = skip_ws(str + 1, end);
			if(is_parse)
			{
				BUFFER list = buf_new_with_callbacks(callbacks, v3d_generic_node_t*);
				while(*str != '}')
				{
					node_str_pair_t node = parse(callbacks, str, start, end);
					if(!node_is_empty(node.node))
						buf_push(&list, &node.node);
					else com_call_deallocate(callbacks, node.node);
					str = skip_ws(node.str, end);
				}
				node->childs = buf_get_ptr(&list);
				node->child_count = buf_get_element_count(&list);
			}
			else
			{
				node->unparsed.start =  str - start;
				int depth = 1;
				do
				{
					switch(*str)
					{
						case '{':
							depth++;
							break;
						case '}': 
							depth--;
							break;
					}
					str = check(str + 1, end);
				} while (depth != 0);
				node->unparsed.end = str - start - 1;
			}
		case ',':
		case ';':
			return (node_str_pair_t) { node, str + 1 };
		case '}':
			return (node_str_pair_t) { node, str };
		case '[':
			str = skip_ws(str + 1, end);
			list = buf_new_with_callbacks(callbacks, u32_pair_t);
			do
			{
				u32_pair_t pair;
				str = get_token(str, "]\t\n\r ", start, end, &pair);
				buf_push(&list, &pair);
			} while(*str != ']');
			node->indexers = buf_get_ptr(&list);
			node->indexer_count = buf_get_element_count(&list);
			str = skip_ws(str + 1, end);
			goto L2;
		case '=':
			str = skip_ws(str + 1, end);
			node->has_value = true;
			node_str_pair_t pair = parse(callbacks, str, start, end);
			node->value = pair.node;
			str = pair.str;
			goto L2;
	}
	return (node_str_pair_t) { node, str };
}


PPSR_API void debug_node(v3d_generic_node_t* node, const char* start)
{

	debug_log_info("[Node]: ");
	debug_log_info("Attributes: %lu", node->attribute_count);
	for(u32 i = 0; i < node->attribute_count; i++)
	{
		debug_log_info("\tName: %.*s", U32_PAIR_DIFF(node->attributes[i].name), start + node->attributes[i].name.start);
		debug_log_info("\tParameters: %lu", node->attributes[i].parameter_count);
		for(u32 j = 0; j < node->attributes[i].parameter_count; j++)
			debug_log_info("\t\t%.*s = %.*s", U32_PAIR_DIFF(node->attributes[i].parameters[j]), start + node->attributes[i].parameters[j].start,
				U32_PAIR_DIFF(node->attributes[i].arguments[j]), start + node->attributes[i].arguments[j].start);
	}

	debug_log_info("Qualifiers: %lu", node->qualifier_count);
	for(u32 i = 0; i < node->qualifier_count; i++)
		debug_log_info("\t%.*s", U32_PAIR_DIFF(node->qualifiers[i]), node->qualifiers[i].start + start);

	debug_log_info("Indexers: %lu", node->indexer_count);
	for(u32 i = 0; i < node->indexer_count; i++)
		debug_log_info("\t%.*s", U32_PAIR_DIFF(node->indexers[i]), node->indexers[i].start + start);

	if(node->has_value)
	{
		debug_log_info("Value: ");
		debug_node(node->value, start);
	}

	if(node->unparsed.start != node->unparsed.end)
		debug_log_info("Unparsed: %.*s", U32_PAIR_DIFF(node->unparsed), node->unparsed.start + start);

	debug_log_info("Childs: %u", node->child_count);
	for(u32 i = 0; i < node->child_count; i++)
		debug_node(node->childs[i], start);
}

PPSR_API ppsr_v3d_generic_parse_result_t ppsr_v3d_generic_parse(com_allocation_callbacks_t* callbacks_ptr, const char* start, u32 length)
{
	com_allocation_callbacks_t callbacks = (callbacks_ptr == NULL) ? com_allocation_callbacks_get_std() : *callbacks_ptr;
	const char* str = start;
	const char* end = str + length;

	v3d_generic_node_t* root = node_create(&callbacks);
	BUFFER list = buf_new_with_callbacks(&callbacks, v3d_generic_node_t*);
	
	while(str < end)
	{
		AUTO result = parse(&callbacks, str, start, end);
		buf_push(&list, &result.node);
		str = skip_ws(result.str, end);
		if((str != end) && ((str + 1) == end))
			str = end;
	}
	
	if(buf_get_element_count(&list) == 0)
		return (ppsr_v3d_generic_parse_result_t) { NULL, NULL, PPSR_SUCCESS };

	root->childs = CAST_TO(v3d_generic_node_t**, buf_get_ptr(&list));
	root->child_count = buf_get_element_count(&list);

	ppsr_v3d_generic_parse_result_t result = { NULL, NULL, PPSR_SUCCESS };
	result.root = root;
	DEBUG_BLOCK
	(
		debug_node(result.root, start);
	)
	return result;
}

void destroy_attribute(com_allocation_callbacks_t* callbacks, v3d_generic_attribute_t* attribute)
{
	if(attribute->argument_count > 0)
		com_call_deallocate(callbacks, attribute->arguments);
	if(attribute->parameter_count > 0)
		com_call_deallocate(callbacks, attribute->parameters);
}

void destroy_node(com_allocation_callbacks_t* callbacks, v3d_generic_node_t* node)
{
	for(u32 i = 0; i < node->child_count; i++)
		destroy_node(callbacks, node->childs[i]);
	if(node->child_count > 0)
		com_call_deallocate(callbacks, node->childs);
	for(u32 i = 0; i < node->attribute_count; i++)
		destroy_attribute(callbacks, &node->attributes[i]);
	if(node->attribute_count > 0)
		com_call_deallocate(callbacks, node->attributes);
	if(node->qualifier_count > 0)
		com_call_deallocate(callbacks, node->qualifiers);
	if(node->indexer_count > 0)
		com_call_deallocate(callbacks, node->indexers);
	if(node->value != NULL)
		destroy_node(callbacks, node->value);
	com_call_deallocate(callbacks, node);
}

PPSR_API void ppsr_v3d_generic_parse_result_destroy(com_allocation_callbacks_t* callbacks, ppsr_v3d_generic_parse_result_t result)
{
	if(result.root != NULL)
		destroy_node(callbacks, result.root);
	if(result.log_buffer != NULL)
		com_call_deallocate(callbacks, result.log_buffer);
}
