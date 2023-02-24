#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "hxa_utils.h"

#define FALSE 0
#define TRUE !FALSE

void hxa_util_primitive_cube(HXAFile *file, double x, double y, double z)
{
	HXANode *node;
	HXALayer *layer;
	char *name;
	unsigned int i;
	file->node_array = realloc(file->node_array, (sizeof *file->node_array) * (file->node_count + 1));
	node = &file->node_array[file->node_count++];
	node->type = HXA_NT_GEOMETRY;
	node->meta_data = NULL;
	node->meta_data_count = 0;
	node->content.geometry.vertex_count = 8;
	node->content.geometry.vertex_stack.layer_count = 1;
	layer = node->content.geometry.vertex_stack.layers = malloc(sizeof *node->content.geometry.vertex_stack.layers);
	layer->components = HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_COMPONENTS;
	layer->type = HXA_LDT_DOUBLE;
	name = HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME;
	for(i = 0; name[i] != 0; i++)
		layer->name[i] = name[i];
	layer->name[i] = 0;
	layer->data.double_data = malloc((sizeof *layer->data.double_data) * 8 * 3);
	layer->data.double_data[0] = x;
	layer->data.double_data[1] = y;
	layer->data.double_data[2] = z;
	layer->data.double_data[3] = -x;
	layer->data.double_data[4] = y;
	layer->data.double_data[5] = z;
	layer->data.double_data[6] = x;
	layer->data.double_data[7] = -y;
	layer->data.double_data[8] = z;
	layer->data.double_data[9] = -x;
	layer->data.double_data[10] = -y;
	layer->data.double_data[11] = z;
	layer->data.double_data[12] = x;
	layer->data.double_data[13] = y;
	layer->data.double_data[14] = -z;
	layer->data.double_data[15] = -x;
	layer->data.double_data[16] = y;
	layer->data.double_data[17] = -z;
	layer->data.double_data[18] = x;
	layer->data.double_data[19] = -y;
	layer->data.double_data[20] = -z;
	layer->data.double_data[21] = -x;
	layer->data.double_data[22] = -y;
	layer->data.double_data[23] = -z;


	node->content.geometry.edge_corner_count = 24;
	node->content.geometry.corner_stack.layer_count = 1;
	layer = node->content.geometry.corner_stack.layers = malloc(sizeof *node->content.geometry.corner_stack.layers);
	layer->components = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_COMPONENTS;
	layer->type = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_TYPE;
	name = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_NAME;
	for(i = 0; name[i] != 0; i++)
		layer->name[i] = name[i];
	layer->name[i] = 0;
	layer->data.int32_data = malloc((sizeof *layer->data.int32_data) * 6 * 4);
	layer->data.int32_data[0] = 0;
	layer->data.int32_data[1] = 1;
	layer->data.int32_data[2] = 3;
	layer->data.int32_data[3] = -2 - 1;
	layer->data.int32_data[4] = 6;
	layer->data.int32_data[5] = 7;
	layer->data.int32_data[6] = 5;
	layer->data.int32_data[7] = -4 - 1;
	layer->data.int32_data[8] = 1;
	layer->data.int32_data[9] = 0;
	layer->data.int32_data[10] = 4;
	layer->data.int32_data[11] = -5 - 1;
	layer->data.int32_data[12] = 2;
	layer->data.int32_data[13] = 3;
	layer->data.int32_data[14] = 7;
	layer->data.int32_data[15] = -6 - 1;
	layer->data.int32_data[16] = 0;
	layer->data.int32_data[17] = 2;
	layer->data.int32_data[18] = 6;
	layer->data.int32_data[19] = -4 - 1;
	layer->data.int32_data[20] = 3;
	layer->data.int32_data[21] = 1;
	layer->data.int32_data[22] = 5;
	layer->data.int32_data[23] = -7 - 1;

	node->content.geometry.edge_stack.layer_count = 0;
	node->content.geometry.edge_stack.layers = NULL;
	
	node->content.geometry.face_count = 6;	
	node->content.geometry.face_stack.layer_count = 0;
	node->content.geometry.face_stack.layers = NULL;
}

void hxa_util_primitive_grid(HXAFile *file, double x_size, double y_size, unsigned int x_grid, unsigned int y_grid, unsigned int u_axis, unsigned int v_axis)
{
	HXANode *node;
	HXALayer *layer;
	double *b, x_start, y_start;
	char *name;
	unsigned int i, j, pos;
	int *ref;
	file->node_array = realloc(file->node_array, (sizeof *file->node_array) * (file->node_count + 1));
	node = &file->node_array[file->node_count++];
	node->type = HXA_NT_GEOMETRY;
	node->meta_data = NULL;
	node->meta_data_count = 0;
	node->content.geometry.vertex_count = (x_grid + 1) * (y_grid + 1);
	node->content.geometry.vertex_stack.layer_count = 1;
	layer = node->content.geometry.vertex_stack.layers = malloc(sizeof *node->content.geometry.vertex_stack.layers);
	layer->components = HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_COMPONENTS;
	layer->type = HXA_LDT_DOUBLE;
	name = HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME;
	for(i = 0; name[i] != 0; i++)
		layer->name[i] = name[i];
	layer->name[i] = 0;
	b = layer->data.double_data = malloc((sizeof *layer->data.double_data) * 8 * 3);
	x_start = x_size * -0.5;
	y_start = y_size * -0.5;
	x_size /= x_grid;
	y_size /= y_grid;
	for(i = 0; i < x_grid + 1; i++)
	{
		for(j = 0; j < y_grid + 1; j++)
		{
			b[2] = b[1] = b[0] = 0;
			b[u_axis] = x_start + x_size * i;
			b[v_axis] = y_start + y_size * j;
			b += 3;
		}
	}
	node->content.geometry.edge_corner_count = x_grid * y_grid * 4;
	node->content.geometry.corner_stack.layer_count = 1;
	layer = node->content.geometry.corner_stack.layers = malloc(sizeof *node->content.geometry.corner_stack.layers);
	layer->components = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_COMPONENTS;
	layer->type = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_TYPE;
	name = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_NAME;
	for(i = 0; name[i] != 0; i++)
		layer->name[i] = name[i];
	layer->name[i] = 0;
	ref = layer->data.int32_data = malloc((sizeof *layer->data.int32_data) * node->content.geometry.edge_corner_count);
	for(i = 0; i < y_grid; i++)
	{
		for(j = 0; j < x_grid; j++)
		{
			pos = i * (x_grid + 1) + j;
			ref[0] = pos;
			ref[1] = pos + 1;
			ref[2] = pos + 1 + x_grid + 1;
			ref[3] = -1 - (pos + x_grid + 1);
			ref += 4;
		}
	}	
	node->content.geometry.edge_stack.layer_count = 0;
	node->content.geometry.edge_stack.layers = NULL;
	node->content.geometry.face_count = x_grid * y_grid;	
	node->content.geometry.face_stack.layer_count = 0;
	node->content.geometry.face_stack.layers = NULL;
}
