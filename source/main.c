
#include <phymac_parser/debug.h>
#include <phymac_parser/assert.h>

#include <phymac_parser/v3d_generic.h>

int main(int argc, char** argv)
{
	debug_log_info("PhyMac parser");

	ppsr_v3d_generic_callbacks_t callbacks =
	{
		.user_data = NULL
	};

	ppsr_result_t result = ppsr_v3d_generic_parse("testfiles/shader.glsl", &callbacks);

	assert(result == PPSR_SUCCESS);

	return 0;
}