#include <math.h>
#include <stdlib.h>
#include <stdio.h> 
#include "betray.h"
#include "imagine.h"
#include "seduce.h"
#include "hxa.h"
#include "hxa_2d_editor_internal.h"


void hxa_2d_editor_hxa_node_save(HxA2DEditorShape *shape, HXANode *node)
{
	HxA2DEditorInstance *instance;
	uint i, j, vertex_count;
	float *v;
	int32 *ref;
	instance = &shape->instances[shape->instance_current];
	for(i = vertex_count = 0; i < instance->loop_count; i++)
		vertex_count += instance->loops[i].loop_size;

	node->type = HXA_NT_GEOMETRY;
	node->meta_data = NULL;
	node->meta_data_count = 0;

	node->content.geometry.vertex_count = vertex_count; // number of vertices
	node->content.geometry.vertex_stack.layers = malloc(sizeof *node->content.geometry.vertex_stack.layers);
	node->content.geometry.vertex_stack.layer_count = 1;
	node->content.geometry.vertex_stack.layers->components = 3;
	sprintf(node->content.geometry.vertex_stack.layers->name, "%s", HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME);
	node->content.geometry.vertex_stack.layers->components = HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_COMPONENTS;
	node->content.geometry.vertex_stack.layers->type = HXA_LDT_FLOAT;
	if(vertex_count != 0)
	{
		node->content.geometry.vertex_stack.layers->data.float_data = v = malloc((sizeof *v) * vertex_count * 3);
		for(i = 0; i < instance->loop_count; i++)
		{
			for(j = 0; j < instance->loops[i].loop_size; j++)
			{
				*v++ = instance->loops[i].loop[j * 2];
				*v++ = instance->loops[i].loop[j * 2 + 1];
				*v++ = 0;
			}
		}
	}else
		node->content.geometry.vertex_stack.layers->data.float_data = NULL;

	node->content.geometry.edge_corner_count = vertex_count; // number of corners
	node->content.geometry.corner_stack.layers = malloc(sizeof *node->content.geometry.corner_stack.layers);
	node->content.geometry.corner_stack.layer_count = 1;
	node->content.geometry.corner_stack.layers->components = 1;
	sprintf(node->content.geometry.corner_stack.layers->name, "%s", HXA_CONVENTION_HARD_BASE_CORNER_LAYER_NAME);
	node->content.geometry.corner_stack.layers->components = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_COMPONENTS;
	node->content.geometry.corner_stack.layers->type = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_TYPE;
	if(vertex_count != 0)
	{
		node->content.geometry.corner_stack.layers->data.int32_data = ref = malloc((sizeof *ref) * vertex_count);

		for(i = vertex_count = 0; i < instance->loop_count; i++)
		{
			for(j = 0; j < instance->loops[i].loop_size - 1; j++)
				*ref++ = (int32)vertex_count++;
			*ref++ = -1 - (int32)(vertex_count++);
		}
	}else
		node->content.geometry.corner_stack.layers->data.int32_data = NULL;


	node->content.geometry.edge_stack.layers = NULL; // stack of edge arrays
	node->content.geometry.edge_stack.layer_count = 0;
	
	node->content.geometry.face_count = instance->loop_count; // number of polygons
	node->content.geometry.face_stack.layers = malloc(sizeof *node->content.geometry.face_stack.layers);
	node->content.geometry.face_stack.layer_count = 1;
	node->content.geometry.face_stack.layers->components = 1;
	sprintf(node->content.geometry.face_stack.layers->name, "%s", HXA_CONVENTION_SOFT_LAYER_MATERIAL_ID);
	node->content.geometry.face_stack.layers->components = 1;
	node->content.geometry.face_stack.layers->type = HXA_LDT_INT32;
	if(vertex_count != 0)
	{
		node->content.geometry.face_stack.layers->data.int32_data = ref = malloc((sizeof *ref) * instance->loop_count);
		for(i = 0; i < instance->loop_count; i++)
			ref[i] = instance->loops[i].material;
	}else
		node->content.geometry.face_stack.layers->data.int32_data = NULL;

}

extern HXALayer *hxa_layer_find_by_name_and_type(HXALayerStack *stack, char *name, HXALayerDataType type);

HxA2DEditorShape *hxa_2d_editor_hxa_node_load(HXANode *node)
{
	HxA2DEditorInstance *instance;
	HxA2DEditorShape *shape;
	HXALayer *layer_material32, *layer_material8 = NULL;
	int32 *ref;
	double *v, *loop;
	char *name = HXA_CONVENTION_SOFT_LAYER_MATERIAL_ID;
	uint i, j, k, side_count, material = 0;

	if(node->type != HXA_NT_GEOMETRY)
		return NULL;
	if(node->content.geometry.face_count == 0 ||
		node->content.geometry.vertex_count == 0 ||
		node->content.geometry.edge_corner_count == 0)
		return NULL;
	if(node->content.geometry.vertex_stack.layers->type == HXA_LDT_FLOAT)
	{
		v = malloc((sizeof *v) * node->content.geometry.vertex_count * 3);
		for(i = 0; i < node->content.geometry.vertex_count * 3; i++)
			v[i] = (double)node->content.geometry.vertex_stack.layers->data.float_data[i];
	}else
	{
		v = node->content.geometry.vertex_stack.layers->data.double_data;
	}

	ref = node->content.geometry.corner_stack.layers->data.int32_data;
	shape = hxa_2d_editor_init_empty();
	instance = &shape->instances[shape->instance_current];	
	if((layer_material32 = hxa_layer_find_by_name_and_type(&node->content.geometry.face_stack, HXA_CONVENTION_SOFT_LAYER_MATERIAL_ID, HXA_LDT_INT32)) == NULL)
		layer_material8 = hxa_layer_find_by_name_and_type(&node->content.geometry.face_stack, HXA_CONVENTION_SOFT_LAYER_MATERIAL_ID, HXA_LDT_UINT8);

	for(i = k = 0; i < node->content.geometry.face_count; i++)
	{
		for(side_count = 0; ref[k + side_count] >= 0; side_count++);

		if(layer_material32 != NULL)
			material = (uint)layer_material32->data.int32_data[i];
		else if(layer_material8 != NULL)
			material = (uint)layer_material8->data.uint8_data[i];
		
		hxa_2d_editor_structure_add_loop(instance, material, 0, 0, 1, side_count + 1);
		loop = instance->loops[i].loop;
		instance->loops[i].loop_size = side_count + 1;
		for(j = 0; j < side_count; j++)
		{
			loop[j * 2 + 0] = v[ref[j + k] * 3];
			loop[j * 2 + 1] = v[ref[j + k] * 3 + 1];
		}
		loop[j * 2 + 0] = v[(-ref[j + k] - 1) * 3];
		loop[j * 2 + 1] = v[(-ref[j + k] - 1) * 3 + 1];
		k += side_count + 1;
	}
	if(node->content.geometry.vertex_stack.layers->type == HXA_LDT_FLOAT)
		free(v);
	return shape;
}

