
#pragma once

#include <common/defines.h>

#ifdef PPSR_STATIC_LIBRARY
#	define PPSR_API
#elif PPSR_DYNAMIC_LIBRARY
#	define PPSR_API __declspec(dllimport)
#elif BUILD_DYNAMIC_LIBRARY
#	define PPSR_API __declspec(dllexport)
#else
#	define PPSR_API
#endif
