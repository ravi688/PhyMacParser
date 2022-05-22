
#pragma once

#include <phymac_parser/defines.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_INFO(...) debug_log("[Info] ", __LINE__, __FUNCTION__, __FILE__,  __VA_ARGS__)
#define LOG_WARNING(...) debug_log("[Warning] ", __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define LOG_ERROR(...) debug_log("[Error] ", __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)
#define LOG_FETAL_ERROR(...) debug_log_exit("[Fetal Error] ", __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__)

#ifdef GLOBAL_DEBUG
#	define log_info(...) LOG_INFO(__VA_ARGS__)
#	define log_warning(...) LOG_WARNING(__VA_ARGS__)
#	define log_error(...) LOG_ERROR(__VA_ARGS__)
#	define log_fetal_error(...) LOG_FETAL_ERROR(__VA_ARGS__)
#else
#	define log_info(...)
#	define log_warning(...)
# 	define log_error(...)
#	define log_fetal_error(...)
#endif

BEGIN_CPP_COMPATIBLE

static void debug_logv(const char* description, u32 line, const char* function, const char* file, const char* format, va_list args)
{
	printf("%s", description);
	vprintf(format, args);
	printf(" | %u, %s, %s\n", line, function, file);
}

static void debug_log(const char* description, u32 line, const char* function, const char* file, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	debug_logv(description, line, function, file, format, args);
	va_end(args);
}

static void debug_log_exit(const char* description, u32 line, const char* function, const char* file, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	debug_logv(description, line, function, file, format, args);
	va_end(args);
	exit(0);
}

END_CPP_COMPATIBLE
