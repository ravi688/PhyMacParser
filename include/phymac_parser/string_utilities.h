
#include <phymac_parser/defines.h>


BEGIN_CPP_COMPATIBLE

/*
	descriptions: skips the whitespaces (isspace) between str and end (inclusive of *str)
	params:
		str: ptr to the begining
		end: ptr to the end
	returns:
		ptr to the first non-whitespace character or
		end if everything has been a whitespace
 */
PMC_PARSER_API const char* skip_whitespaces(const char* str, const char* const end);

/*
	descriptions: returns the word length
	params:
		str: ptr to the begining
		end: ptr to the end
		delimiters: string of delimiter character for the word
	returns:
		length of the word
 */
PMC_PARSER_API u32 __get_word_length(const char* str, const char* const end, const char* delimiters);
PMC_PARSER_API u32 get_word_length(const char* string, const char delimiter);

/*
	descriptions: returns if the region from start to end (exclusive) is empty or not
	params:
		start: ptr to the begining
		end: ptr to the end
	returns:
		true if the string contains only whitespace or start == end
		false otherwise
 */
PMC_PARSER_API bool is_empty(const char* start, const char* const end);

/*
	descriptions: removes all the comments (multi line or single line)
	params:
		start: ptr to the beginning
		end: ptr to the end
	returns:
		nothing
 */
PMC_PARSER_API void remove_comments(char* start, const char* const end);

END_CPP_COMPATIBLE
