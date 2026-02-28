#include "forge.h"
#include "hxa.h"
#include "hxa_utils.h"
#include "relinquish.h"


void hxa_test_draw(HXANode *node)
{
	uint i, j;
	float *v;
	int *ref, first, r0, r1;
/*	v = node->content.geometry.vertex_stack.layers[0].data.float_data;
	for(i = j = 0; i < node->content.geometry.vertex_count; i++)
	{
		r_primitive_line_3d(v[0] - 0.01, v[1], v[2], v[0] + 0.01, v[1], v[2], 1, 1, 0, 1);
		r_primitive_line_3d(v[0], v[1] - 0.01, v[2], v[0], v[1] + 0.01, v[2], 1, 1, 0, 1);
		v += 3;
	}*/
	v = node->content.geometry.vertex_stack.layers[0].data.float_data;

	ref = node->content.geometry.corner_stack.layers[0].data.int32_data;	
	for(i = j = 0; i < node->content.geometry.face_count; i++)
	{
		r0 = first = ((*ref++) * 3);
		while((r1 = (*ref++)) >= 0)
		{
			r1 *= 3;
			r_primitive_line_3d(v[r0], v[r0 + 1], v[r0 + 2], v[r1], v[r1 + 1], v[r1 + 2], 1, 1, 1, 1);
			r0 = r1;
		}
		r1 = -(1 + r1) * 3;
		r_primitive_line_3d(v[r0], v[r0 + 1], v[r0 + 2], v[r1], v[r1 + 1], v[r1 + 2], 1, 1, 1, 1);
		r_primitive_line_3d(v[first], v[first + 1], v[first + 2], v[r1], v[r1 + 1], v[r1 + 2], 1, 1, 1, 1);
	}
}


void hxa_test()
{
	static HXAFile *file = NULL;
	uint i;
	if(file == NULL)
	{
		file = hxa_load("raycast.hxa", TRUE);
		hxa_util_validate(file, FALSE);
	//	file = hxa_load("street.hxa", TRUE);		
		for(i = 0; i < file->node_count; i++)
		{
			hxa_util_node_vertex_purge(&file->node_array[i]);
			hxa_util_convert_node_double_to_float(&file->node_array[i]);
			hxa_util_triangulate_node(&file->node_array[i], 3);
		}
	}
	for(i = 0; i < file->node_count; i++)
		hxa_test_draw(&file->node_array[i]);
	r_primitive_line_flush();
}