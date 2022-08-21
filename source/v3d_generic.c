
#include <phymac_parser/v3d_generic.h>
#include <phymac_parser/debug.h>
#include <phymac_parser/string.h> // for custom string functions
#include <string.h>		// for memory and string functions
#include <stdlib.h> 	// for malloc
#include <BufferLib/buffer.h>


#define NO_PARSE_ATTRIB "NoParse"

static const char* check(const char* str, const char* const end)
{
	if(str >= end)
		debug_log_fetal_error("Unexpected end of file while parsing, exiting");
	return str;
}

static void expected(const char* exp_str, const char* str, const char* const end)
{
	debug_log_fetal_error("Unexpected \"%.*s\", expected %s", get_token_length(str, end), str, exp_str);
}

static v3d_generic_node_t* node_create()
{
	v3d_generic_node_t* node = (v3d_generic_node_t*)malloc(sizeof(v3d_generic_node_t));
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

static attrib_str_pair_t parse_attributes(const char* str, const char* const start, const char* const end, bool OUT is_parse)
{
	BUFFER attributes = buf_new(v3d_generic_attribute_t);
	while(*str == '[')
	{
		str = skip_ws(str + 1, end);
		v3d_generic_attribute_t attribute;
		const char* _str = get_token(str, "(]\t\n ", start, end, &attribute.name);
		if(strncmp(NO_PARSE_ATTRIB, str, U32_PAIR_DIFF(attribute.name)) == 0)
			*is_parse = false;
		str = _str;
		BUFFER parameters = buf_new(u32_pair_t);
		BUFFER arguments = buf_new(u32_pair_t);
		if(*str == '(')
		{
			str++;
			while(*str != ')')
			{
				str = skip_ws(str, end);
				u32_pair_t pair;
				str = get_token(str, ",=)]\t\n ", start, end, &pair);
L1:
				switch(*str)
				{
					case ',':
						str = check(str + 1, end);
					case ')':
						buf_push(&arguments, &pair);
						buf_push_pseudo(&parameters, 1);
						continue;
					case '=':
						buf_push(&parameters, &pair);
						str = skip_ws(str + 1, end);
						str = get_token(str, ",)]\t\n ", start, end, &pair);
						buf_push(&arguments, &pair);
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

static node_str_pair_t parse(const char* str, const char* const start, const char* const end)
{
	str = skip_ws(str, end);

	v3d_generic_node_t* node = node_create();
	bool is_parse = true;
	attrib_str_pair_t attributes = parse_attributes(str, start, end, &is_parse);
	str = attributes.str;
	node->attributes = attributes.attributes;
	node->attribute_count = attributes.attribute_count;

	BUFFER list = buf_new(u32_pair_t);
	char buffer[2] = { *str, 0 };
	while(strpbrk("{};,=[", buffer) == NULL)
	{
		u32_pair_t pair;
		str = get_token(str, ",{;\t\n ", start, end, &pair);
		buf_push(&list, &pair);
		buffer[0] = *str;
	}

	node->qualifiers = buf_get_ptr(&list);
	node->qualifier_count = buf_get_element_count(&list);

L2:
	switch(*str)
	{
		case '{':
			str = skip_ws(str + 1, end);
			if(is_parse)
			{
				BUFFER list = buf_new(v3d_generic_node_t*);
				while(*str != '}')
				{
					node_str_pair_t node = parse(str, start, end);
					if(!node_is_empty(node.node))
						buf_push(&list, &node.node);
					else free(node.node);
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
					str++;
					switch(*str)
					{
						case '{':
							depth++;
							break;
						case '}':
							depth--;
							break;
					}
				} while ((depth != 0) || (*str != '}'));
				node->unparsed.end = str - start;
			}
		case ',':
		case ';':
			return (node_str_pair_t) { node, str + 1 };
		case '}':
			return (node_str_pair_t) { node, str };
		case '[':
			str = skip_ws(str + 1, end);
			list = buf_new(u32_pair_t);
			while(*str != ']')
			{
				u32_pair_t pair;
				str = get_token(str, "]\t\n ", start, end, &pair);
				buf_push(&list, &pair);
			}
			node->indexers = buf_get_ptr(&list);
			node->indexer_count = buf_get_element_count(&list);
			str = skip_ws(str + 1, end);
			goto L2;
		case '=':
			str = skip_ws(str + 1, end);
			node->has_value = true;
			node_str_pair_t pair = parse(str, start, end);
			node->value = pair.node;
			str = pair.str;
			goto L2;
	}
	return (node_str_pair_t) { node, str };
}


static void debug_node(v3d_generic_node_t* node, const char* start)
{
	debug_log_info("[Node]: ");
	if(node->attribute_count > 0)
		debug_log_info("Attributes: ");
	for(u32 i = 0; i < node->attribute_count; i++)
	{
		debug_log_info("\tName: %.*s", U32_PAIR_DIFF(node->attributes[i].name), start + node->attributes[i].name.start);
		if(node->attributes[i].parameter_count > 0)
			debug_log_info("\tParameters: ");
		for(u32 j = 0; j < node->attributes[i].parameter_count; j++)
			debug_log_info("\t\t%.*s = %.*s", U32_PAIR_DIFF(node->attributes[i].parameters[j]), start + node->attributes[i].parameters[j].start,
				U32_PAIR_DIFF(node->attributes[i].arguments[j]), start + node->attributes[i].arguments[j].start);
	}

	if(node->qualifier_count > 0)
		debug_log_info("Qualifiers: ");
	for(u32 i = 0; i < node->qualifier_count; i++)
		debug_log_info("\t%.*s", U32_PAIR_DIFF(node->qualifiers[i]), node->qualifiers[i].start + start);

	if(node->indexer_count > 0)
		debug_log_info("Indexers: ");
	for(u32 i = 0; i < node->indexer_count; i++)
		debug_log_info("\t%.*s", U32_PAIR_DIFF(node->indexers[i]), node->indexers[i].start + start);

	if(node->has_value)
	{
		debug_log_info("Value: ");
		debug_node(node->value, start);
	}

	if(node->unparsed.start != node->unparsed.end)
		debug_log_info("Unparsed: %.*s", U32_PAIR_DIFF(node->unparsed), node->unparsed.start + start);

	if(node->child_count > 0)
		debug_log_info("Childs: %u", node->child_count);
	for(u32 i = 0; i < node->child_count; i++)
		debug_node(node->childs[i], start);
}

PPSR_API ppsr_v3d_generic_parse_result_t ppsr_v3d_generic_parse(const char* string, u32 length)
{
	ppsr_v3d_generic_parse_result_t result = { NULL, NULL, PPSR_SUCCESS };
	result.root = parse(string, string, string + length).node;
	#ifdef GLOBAL_DEBUG
	debug_node(result.root, string);
	#endif
	return result;
}
