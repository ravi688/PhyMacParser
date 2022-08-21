
#include <phymac_parser/debug.h>
#include <phymac_parser/assert.h>
#include <phymac_parser/v3d_generic.h>
#include <disk_manager/file_reader.h>

int main(int argc, char** argv)
{
	BUFFER* text = load_text_from_file("test_files/example.v3dshader");
	ppsr_v3d_generic_parse_result_t result = ppsr_v3d_generic_parse(buf_get_ptr(text), buf_get_element_count(text) - 1);
	assert(result.result == PPSR_SUCCESS);
	buf_free(text);
	return 0;
}