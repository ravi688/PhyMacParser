
#pragma once

#include <phymac_parser/defines.h>

BEGIN_CPP_COMPATIBLE

PPSR_API char* skip(const char* str, const char* letters, const char* const end);
static FORCE_INLINE char* skip_ws(const char* str, const char* const end) { return skip(str, "\t\n\r\f\v ", end); }
PPSR_API char* skip_token(const char* str, const char* const end);
static FORCE_INLINE u32 get_token_length(const char* str, const char* const end) { return skip_token(str, end) - str; }
PPSR_API char* skip_until(const char* str, const char* delimiters, const char* const end);

END_CPP_COMPATIBLE
