
#pragma once

#include <phymac_parser/defines.h>

typedef struct pmc_parser_callbacks_t
{
	void* user_data; 								// pointer to a user data

	// callback when a category is started
	void (*begin_category)(const char* str,  		// str: pointer to the name of the started category (not null terminated)
							u32 length, 			// length: length of the name of the started category
							void* user_data); 		// user_data: pointer to the user data
	
	// callback when a category is ended
	void (*end_category)(void* user_data);  		// user_data: pointer to the user data
	
	// callback when a field is started
	void (*field)(	const char* str,  				// str: pointer to the name of the started field (not null terminated)
					u32 length,  					// length: length of the name of the started field
					const char* value,  			// value: pointer to the string value (field's value) (not null terminated)
					u32 value_length,  				// value_length: length of the string value
					void* user_data); 				// user_data: pointer to the user data

	// callback when an attribute is started
	void (*attribute)(	const char* str,  			// str: pointer to the name of the started attribute (not null terminated)
						u32 length,  				// length: length of the name of the attribute
						const char* value,  		// value: pointer to the string value (attribute's value) (not null terminated)
						u32 value_length,  			// value_length: length of the string value
						void* user_data); 			// user_data: pointer to the user data

} pmc_parser_callbacks_t;

PMC_PARSER_API void pmc_parse(pmc_parser_callbacks_t* callbacks, char* string, u32 length);
