
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hxa_upi.h"
#include "unwrap2.h"

extern void hxa_print(HXAFile *file, int data);

HXAFile *hxa_upi_plugin_execute_func(HXAUPIInterfaceParam *params, void *instance, void *user)
{
	UnError errors[128];
	HXAFile *file;
	UnwrapSetting *settings;
	UnOutputIdentical output;
	uint i, count_basic, count_all, error_count;
	settings = unwrap_settings_alloc(&count_basic, &count_all); 
	file = params[0].content.hxa;
	if(file == NULL)
		return NULL;
	for(i = 0; i < file->node_count; i++)
	{
		if(file->node_array[i].type == HXA_NT_GEOMETRY)
		{
			un_unwrap_identical(&output, /* a pointer to the structure where the output will be written */
								UN_VAT_32_BIT_FLOAT, /* sould the output vu data be written as floats of doubles*/
								FALSE, /* should the algorith output a stages array. */
								file->node_array[i].content.geometry.corner_stack.layers[0].data.int32_data, /* an array of referneces. FBX style */
								file->node_array[i].content.geometry.edge_corner_count, /* the length of the ref array */
								file->node_array[i].content.geometry.vertex_stack.layers->data.float_data, /* array of vertex data stored as floats or doubles */
								UN_VAT_32_BIT_FLOAT, /* the type of data given as vertex_array */
								sizeof(float) * 3,  /* the space between each vertex. Example: sizeof(float) * 3. useful for readin interleaved vertex arrays. */
								file->node_array[i].content.geometry.edge_corner_count,  /* the number of vertices available.*/
								NULL, /* a popinter to a UnEdgeData structure. Can be NULL if no good edge data is available. Can be NULL. */
								errors,  /* a pointer to an array of UNError structures, to be written to. */
								&error_count, /* a pointer to a integer value where the number of errors will be written.*/
								128, /* the length of the errors buffer */
								settings, /* an array of settings. must be identical in length and order as returned by unwrap_settings_alloc. */
								NULL); /* A user pointer that is transferd to all callbacks. */
		}
	}
extern void un_unwrap_identical_free(UnOutputIdentical *identical); /* will free all memebers allocated by un_unwrap_identical.*/


	return file;
}

extern UnwrapSetting *unwrap_settings_alloc(uint *count_basic, uint *count_all); 

HxALibExport void hxa_upi_plugin_library_initialize(void (*hxa_upi_library_register_func)(HxAUPIPlugin *plugin))
{
	HxAUPIPlugin p;
	UnwrapSetting *settings;
	uint i, count_basic, count_all;
	settings = unwrap_settings_alloc(&count_basic, &count_all); 
	p.params = malloc(sizeof(HXAUPIInterfaceParam) * 2);	
	p.params[0].type = HXA_UPI_IPT_HXA_CONSUME;
	p.params[0].name = "HxA";
	p.params[0].description = "The HxA structure to unwrapped";
	p.params[0].content.hxa = NULL;	
	for(i = 0; i < count_basic; i++)
	{
		p.params[i + 1].name = settings[i].name;
		p.params[i + 1].description = settings[i].description;
		switch(settings[i].type)
		{	
			case U_ST_BOOLEAN :
				p.params[i + 1].type = HXA_UPI_IPT_BOOLEAN;
				p.params[i + 1].content.boolean = settings[i].value.boolean_value;
			break;
			case U_ST_INTEGER :
				p.params[i + 1].type = HXA_UPI_IPT_BOOLEAN;
				p.params[i + 1].content.signed_integer = settings[i].value.integer;
			break;
			case U_ST_REAL :
				p.params[i + 1].type = HXA_UPI_IPT_DOUBLE_UNBOUND;
				p.params[i + 1].content.double_value = settings[i].value.real;
			break;
			case U_ST_POSITION :
				p.params[i + 1].type = HXA_UPI_IPT_POS_3D;
				p.params[i + 1].content.point_3d[0] = settings[i].value.pos[0];
				p.params[i + 1].content.point_3d[1] = settings[i].value.pos[1];
				p.params[i + 1].content.point_3d[2] = settings[i].value.pos[2];
			break;
		}
	}
	p.param_count = count_basic + 1;
	p.name = "UV Unwrapper";
	p.descrtiption = "Uses the Ministry of flats fully automated UV unwrapper to generate texture coordinates for a mesh.";
	p.has_output = TRUE;
	p.instance_create_func = NULL;
	p.instance_destroy_func = NULL;
	p.instance_update_func = NULL;
	p.execute_func = hxa_upi_plugin_execute_func;
	p.user_data = NULL;
	hxa_upi_library_register_func(&p);
}