
#pragma once

#include <phymac_parser/defines.h>
#include <phymac_parser/logging.h>

#define ASSERT(...) debug_assert(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define ASSERT_WRN(...) debug_assert_wrn(__LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define ASSERT_CALLED_ONCE(...) static int __FUNCTION__##count = 0; __FUNCTION__##count++; ASSERT(DESCRIPTION(__FUNCTION__##count < 2), "The Function \"%s\" has been called more than once", __FUNCTION__)
#define ASSERT_NOT_IMPLEMENTED(...) ASSERT(DESCRIPTION(false), "Function \"%s\" isn't implemented yet but you're still trying to call it", __FUNCTION__)

#define DESCRIPTION(bool_value) (CAST_TO(u64, (bool_value)) | (1ULL << 16))

#ifdef GLOBAL_DEBUG
#	define assert(...) ASSERT(__VA_ARGS__)
#	define assert_wrn(...) ASSERT_WRN(__VA_ARGS__)
#	define assert_called_once(...) ASSERT_CALLED_ONCE(__VA_ARGS__)
#	define assert_not_implemented(...) ASSERT_NOT_IMPLEMENTED(__VA_ARGS__)
#else
#	define assert(...)
#	define assert_wrn(...)
#	define assert_called_once(...)
#	define assert_not_implemented(...)
#endif

BEGIN_CPP_COMPATIBLE

static void debug_assert_wrn(u32 line, const char* function, const char* file, u64 assertion, ...)
{
	if(assertion & 1ULL) return;
	va_list args;
	va_start(args, assertion);
	const char* format = "";
	if(assertion & (1ULL << 16)) format = va_arg(args, const char*);
	debug_logv("[Assertion Failed] ", line, function, file, format, args);
	va_end(args);
}

static void debug_assert(u32 line, const char* function, const char* file, u64 assertion, ...)
{
	if(assertion & 1ULL) return;
	va_list args;
	va_start(args, assertion);
	const char* format = "";
	if(assertion & (1ULL << 16)) format = va_arg(args, const char*);
	debug_logv("[Assertion Failed] ", line, function, file, format, args);
	va_end(args);
	exit(0);
}

END_CPP_COMPATIBLE
