#include <phymac_parser/phymac_parser.h>
#include <phymac_parser/string_utilities.h>
#include <phymac_parser/logging.h>

static const char* parse(const char* str, u32 length, pmc_parser_callbacks_t* callbacks, u32 categoryRank);

void pmc_parse(pmc_parser_callbacks_t* callbacks, char* string, u32 length)
{
	remove_comments(string, string + length);
	parse(string, length, callbacks, 0);
}

static const char* parse(const char* str, u32 length, pmc_parser_callbacks_t* callbacks, u32 categoryRank)
{
	const char* origin = str;
	const char* const end = origin + length;
	str = skip_whitespaces(str, end);

	if(str >= end)
	{
		if(categoryRank != 0)
			LOG_FETAL_ERROR("A category is not closed, expected '}' before the end of file");
		return str;
	}

	u32 len = __get_word_length(str, end, "[]{}=,");
	const char* name = str;
	str += len;
	str = skip_whitespaces(str, end);
	switch(*str)
	{
		// start of an attribute
		case '[':
		break;

		// start of a category
		case '{':
			if(callbacks->begin_category != NULL)
				callbacks->begin_category(name, len, callbacks->user_data);
			categoryRank++;
		break;

		// end of a category
		case '}':
			if((categoryRank > 0) && (len != 0))
			{
				if(callbacks->field != NULL)
					callbacks->field(name, len, NULL, 0, callbacks->user_data);
			}

			if(categoryRank > 0)
			{
				if(callbacks->end_category != NULL)
					callbacks->end_category(callbacks->user_data);
				--categoryRank;
			}
			else
				LOG_FETAL_ERROR("Unexpected '}', closing brace '}' must match with opening branch '{'");
		break;

		// assignment to a field
		case '=':
			const char* field_name = name;
			++str;
			str = skip_whitespaces(str, end);
			u32 _len = __get_word_length(str, end, ",]");			// \n is already included
			str += _len;

			// attribute
			if((*str == ']') && (callbacks->attribute != NULL))
				callbacks->attribute(field_name, len, str - _len, _len, callbacks->user_data);
			// field
			else if(callbacks->field != NULL)
				callbacks->field(field_name, len, str - _len, _len, callbacks->user_data);
		break;

		// declaration of a field but not assignment
		case ',':
			if(callbacks->field  != NULL)
				callbacks->field(name, len, NULL, 0, callbacks->user_data);
		break;

		case ']':
			if(callbacks->attribute != NULL)
				callbacks->attribute(name, len, NULL, 0, callbacks->user_data);
		break;

		case 0:
			LOG_FETAL_ERROR("Unexpected end of file");
		break;

		default:
			LOG_FETAL_ERROR("Expected '{', '=' or ',' before \"%.*s\"", len, str);
	}
	++str;
	return parse(str, length - (str - origin), callbacks, categoryRank);
}
