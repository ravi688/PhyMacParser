
#pragma once

#include <phymac_parser/defines.h>
#include <phymac_parser/result.h>

/* callbacks */
typedef struct ppsr_v3d_generic_callbacks_t
{
	void* user_data;
} ppsr_v3d_generic_callbacks_t;

BEGIN_CPP_COMPATIBLE

PPSR_API ppsr_result_t ppsr_v3d_generic_parse(const char* file_path, ppsr_v3d_generic_callbacks_t* callbacks);

END_CPP_COMPATIBLE
