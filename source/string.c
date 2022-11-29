
#include <phymac_parser/string.h>
#include <string.h>
#include <ctype.h>

#include <phymac_parser/debug.h>

static void unexpected_end_of_file()
{
	DEBUG_LOG_FETAL_ERROR("Unexpected end of file while parsing, exiting");
}

#define CHECKED_RETURN(ptr) if((ptr) > end) unexpected_end_of_file(); return CAST_TO(char*, (ptr))

PPSR_API char* skip(const char* str, const char* letters, const char* const end)
{
	u64 count = strspn(str, letters);
	const char* ptr = str + count;
	CHECKED_RETURN(ptr);
}

PPSR_API char* skip_token(const char* str, const char* const end)
{
	while((str < end) && (isalnum(*str) || (*str == '_')))
		++str;
	CHECKED_RETURN(str);
}

PPSR_API char* skip_until(const char* str, const char* delimiters,  const char* const end)
{
	u32 count = strcspn(str, delimiters);
	const char* ptr = str + count;
	CHECKED_RETURN(ptr);
}
