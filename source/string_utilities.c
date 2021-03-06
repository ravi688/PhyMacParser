
#include <phymac_parser/string_utilities.h>
#include <string.h>
#include <ctype.h>

PMC_PARSER_API const char* skip_whitespaces(const char* str, const char* const end)
{
	while((str < end) && isspace(*str)) str++;
	return str;
}

PMC_PARSER_API u32 __get_word_length(const char* str, const char* const end, const char* delimiters)
{
	u32 count = 0;
	while((str < end) && (!isspace(*str)) && (strchr(delimiters, *str) == NULL)) { str++; count++; }
	return count;
}


PMC_PARSER_API u32 get_word_length(const char* string, const char delimiter)
{
	u32 count = 0;
	while((!isspace(*string)) && (delimiter != (*string))) { string++; count++; }
	return count;
}

PMC_PARSER_API bool is_empty(const char* start, const char* const end)
{
	bool empty = true;
	while(start < end)
	{
		if(!isspace(*start))
		{
			empty = false;
			break;
		}
		++start;
	}
	return empty;
}

PMC_PARSER_API void remove_comments(char* start, const char* const end)
{
	bool single_line_comment_begin = false; 
	bool multiple_line_comment_begin = false;

	while(start < end)
	{
		if(!multiple_line_comment_begin && !single_line_comment_begin)
		{
			bool found = true;
			if(strncmp(start, "//", 2) == 0)
				single_line_comment_begin = true;
			else if(strncmp(start, "/*", 2) == 0)
				multiple_line_comment_begin = true;
			else found = false;
			if(found)
			{
				start[0] = ' ';
				start[1] = ' ';
				start++;
			}
			start++;
			continue;
		}
		if(multiple_line_comment_begin)
		{
			if(strncmp(start, "*/", 2) == 0)
				multiple_line_comment_begin = false;
			start[0] = ' ';
			start[1] = ' ';
			start += 2;
			continue;
		}
		else if(single_line_comment_begin)
		{
			const char* ptr = strchr(start, '\n');
			if(ptr == NULL)
				ptr = end;
			if(ptr != start)
				memset(start, ' ', ptr - start);
			start = (char*)(ptr + 1);
			single_line_comment_begin = false;
			continue;
		}
	}
}
