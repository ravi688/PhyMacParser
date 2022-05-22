
#include <phymac_parser/defines.h>


BEGIN_CPP_COMPATIBLE

PMC_PARSER_API const char* skip_whitespaces(const char* str, const char* const end);
PMC_PARSER_API u32 __get_word_length(const char* str, const char* const end, const char* delimiters);
PMC_PARSER_API u32 get_word_length(const char* string, const char delimiter);
PMC_PARSER_API bool is_empty(const char* start, const char* const end);
PMC_PARSER_API void remove_comments(char* start, u64 length);

END_CPP_COMPATIBLE
