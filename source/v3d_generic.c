
#include <phymac_parser/v3d_generic.h>
#include <phymac_parser/debug.h>
#include <phymac_parser/string.h> // for custom string functions
#include <string.h>		// for memory and string functions
#include <stdlib.h> 	// for malloc


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

static const char* parse_attributes(const char* str, const char* const start, const char* const end, bool OUT is_parse)
{
	while(*str == '[')
	{
		str = skip_ws(str + 1, end);
		u32_pair_t name = { str - start, skip_until(str, "(]\t\n ", end) - start };
		debug_log_info("Attribute Name: %.*s", U32_PAIR_DIFF(name), str);
		if(strncmp("NoParse", str, U32_PAIR_DIFF(name)) == 0)
			*is_parse = false;
		str = skip_ws(str + U32_PAIR_DIFF(name), end);
		if(*str == '(')
		{
			str++;
			while(*str != ')')
			{
				str = skip_ws(str, end);
				u32_pair_t para_name = { str - start, skip_until(str, ",=)]\t\n ", end) - start };
				debug_log_info("Parameter: %.*s", para_name.end - para_name.start, str);
				str = skip_ws(str + U32_PAIR_DIFF(para_name), end);
L1:
				switch(*str)
				{
					case ',':
						str = check(str + 1, end);
						continue;
					case ')':
						continue;
					case '=':
						str = skip_ws(str + 1, end);
						u32_pair_t arg_name = { str - start, skip_until(str, ",)]\t\n ", end) - start };
						debug_log_info("Value: %.*s", U32_PAIR_DIFF(arg_name), str);
						str = skip_ws(str + U32_PAIR_DIFF(arg_name), end);
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
	}
	return str;
}

static const char* parse(const char* str, const char* const start, const char* const end)
{
	str = skip_ws(str, end);

	bool is_parse = true;
	str = parse_attributes(str, start, end, &is_parse);
	
	while((*str != '{') && (*str != ';') && (*str != ',') && (*str != '}'))
	{
		u32_pair_t name = { str - start, skip_until(str, ",{;\t\n ", end) - start };
		debug_log_info("token: %.*s", U32_PAIR_DIFF(name), str);
		str = skip_ws(str + U32_PAIR_DIFF(name), end);
	}

	switch(*str)
	{
		case '{':
			str++;
			if(is_parse)
			{
				while(*str != '}')
					str = parse(str, start, end);
			}
			else
			{
				u32_pair_t unparsed = { str - start };
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
				unparsed.end = str - start;
			}
		case ',':
		case ';':
			return str + 1;
		case '}':
			return str;
	}
	return str;
}


PPSR_API ppsr_v3d_generic_parse_result_t ppsr_v3d_generic_parse(const char* string, u32 length)
{
	ppsr_v3d_generic_parse_result_t result = { NULL, NULL, PPSR_SUCCESS };
	parse(string, string, string + length);
	return result;
}
