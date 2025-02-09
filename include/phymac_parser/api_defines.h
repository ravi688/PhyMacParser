#pragma once

#if (defined _WIN32 || defined __CYGWIN__) && defined(__GNUC__)
#	define PPSR_IMPORT_API __declspec(dllimport)
#	define PPSR_EXPORT_API __declspec(dllexport)
#else
#	define PPSR_IMPORT_API __attribute__((visibility("default")))
#	define PPSR_EXPORT_API __attribute__((visibility("default")))
#endif

#ifdef PPSR_BUILD_STATIC_LIBRARY
#	define PPSR_API
#elif defined(PPSR_BUILD_DYNAMIC_LIBRARY)
#	define PPSR_API PPSR_EXPORT_API
#elif defined(PPSR_USE_DYNAMIC_LIBRARY)
#	define PPSR_API PPSR_IMPORT_API
#elif defined(PPSR_USE_STATIC_LIBRARY)
#	define PPSR_API
#else
#	define PPSR_API
#endif
