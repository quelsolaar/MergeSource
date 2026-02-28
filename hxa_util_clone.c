#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "hxa.h"
#include "hxa_utils.h"

void hxa_clone_geometry_layer(HXALayer *layer, unsigned int old_length)
{
	HXALayer l;
	unsigned int i, length;
	switch(layer->type)
	{
		case HXA_LDT_UINT8 :
			layer->data.uint8_data = realloc(layer->data.uint8_data, (sizeof *layer->data.uint8_data) * layer->components * old_length * 2);
			memcpy(&layer->data.uint8_data[layer->components * old_length], layer->data.uint8_data, (sizeof *layer->data.uint8_data) * layer->components * old_length);
		break;
		case HXA_LDT_INT32 :
			layer->data.int32_data = realloc(layer->data.int32_data, (sizeof *layer->data.int32_data) * layer->components * old_length * 2);
			memcpy(&layer->data.int32_data[layer->components * old_length], layer->data.int32_data, (sizeof *layer->data.int32_data) * layer->components * old_length);
		break;
		case HXA_LDT_FLOAT :
			layer->data.float_data = realloc(layer->data.float_data, (sizeof *layer->data.float_data) * layer->components * old_length * 2);
			memcpy(&layer->data.float_data[layer->components * old_length], layer->data.float_data, (sizeof *layer->data.float_data) * layer->components * old_length);
		break;
		case HXA_LDT_DOUBLE :
			layer->data.double_data = realloc(layer->data.double_data, (sizeof *layer->data.double_data) * layer->components * old_length * 2);
			memcpy(&layer->data.double_data[layer->components * old_length], layer->data.double_data, (sizeof *layer->data.double_data) * layer->components * old_length);
		break;
	}
}

void hxa_clone_geometry_node(HXANode* node, double x_offset, double y_offset, double z_offset)
{
	unsigned int i;
	for(i = 0; i < node->content.geometry.vertex_stack.layer_count; i++)
		hxa_clone_geometry_layer(&node->content.geometry.vertex_stack.layers[i], node->content.geometry.vertex_count);
	for(i = 0; i < node->content.geometry.corner_stack.layer_count; i++)
		hxa_clone_geometry_layer(&node->content.geometry.corner_stack.layers[i], node->content.geometry.edge_corner_count);
	for(i = 0; i < node->content.geometry.edge_stack.layer_count; i++)
		hxa_clone_geometry_layer(&node->content.geometry.edge_stack.layers[i], node->content.geometry.edge_corner_count);
	for(i = 0; i < node->content.geometry.face_stack.layer_count; i++)
		hxa_clone_geometry_layer(&node->content.geometry.face_stack.layers[i], node->content.geometry.face_count);
	if(node->content.geometry.vertex_stack.layers->type == HXA_LDT_FLOAT)
	{
		for(i = node->content.geometry.vertex_count; i < node->content.geometry.vertex_count * 2; i++)
		{
			node->content.geometry.vertex_stack.layers->data.float_data[i * 3 + 0] += (float)x_offset;
			node->content.geometry.vertex_stack.layers->data.float_data[i * 3 + 1] += (float)y_offset;
			node->content.geometry.vertex_stack.layers->data.float_data[i * 3 + 2] += (float)z_offset;
		}
	}else
	{
		for(i = node->content.geometry.vertex_count; i < node->content.geometry.vertex_count * 2; i++)
		{
			node->content.geometry.vertex_stack.layers->data.double_data[i * 3 + 0] += x_offset;
			node->content.geometry.vertex_stack.layers->data.double_data[i * 3 + 1] += y_offset;
			node->content.geometry.vertex_stack.layers->data.double_data[i * 3 + 2] += z_offset;
		}
	}
	for(i = node->content.geometry.edge_corner_count; i < node->content.geometry.edge_corner_count * 2; i++)
	{
		if(node->content.geometry.corner_stack.layers->data.int32_data[i] < 0)
			node->content.geometry.corner_stack.layers->data.int32_data[i] -= (int)node->content.geometry.vertex_count;
		else
			node->content.geometry.corner_stack.layers->data.int32_data[i] += (int)node->content.geometry.vertex_count;

	}
	node->content.geometry.vertex_count *= 2;
	node->content.geometry.edge_corner_count *= 2;
	node->content.geometry.face_count *= 2;
}

