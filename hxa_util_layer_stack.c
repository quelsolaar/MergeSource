#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "hxa_utils.h"

#define FALSE 0
#define TRUE !FALSE

void hxa_util_layer_stack_change_components(HXALayer *layer, unsigned int length, hxa_uint8 components)
{
	unsigned int i, j;
	hxa_uint8 *uint8_data;
	hxa_int32 *int32_data;
	float *float_data;
	double *double_data;
	if(layer->components > components)
	{
		switch(layer->type)
		{
			case HXA_LDT_UINT8 :
				uint8_data = layer->data.uint8_data;
				for(i = 1; i < length; i++)
					for(j = 0; j < components; j++)
						uint8_data[i * components + j] = uint8_data[i * layer->components + j];
				uint8_data = realloc(uint8_data, (sizeof *uint8_data) * length * components);
			break;
			case HXA_LDT_INT32 :
				int32_data = layer->data.int32_data;
				for(i = 1; i < length; i++)
					for(j = 0; j < components; j++)
						int32_data[i * components + j] = int32_data[i * layer->components + j];
				int32_data = realloc(int32_data, (sizeof *int32_data) * length * components);
			break;
			case HXA_LDT_FLOAT :
				float_data = layer->data.float_data;
				for(i = 1; i < length; i++)
					for(j = 0; j < components; j++)
						float_data[i * components + j] = float_data[i * layer->components + j];
				float_data = realloc(float_data, (sizeof *float_data) * length * components);
			break;
			case HXA_LDT_DOUBLE  :
				double_data = layer->data.double_data;
				for(i = 1; i < length; i++)
					for(j = 0; j < components; j++)
						double_data[i * components + j] = double_data[i * layer->components + j];
				double_data = realloc(double_data, (sizeof *double_data) * length * components);
			break;
		}
		layer->components = components;
		return;
	}
	if(layer->components < components)
	{
		switch(layer->type)
		{
			case HXA_LDT_UINT8 :
				uint8_data = malloc((sizeof *uint8_data) * length * components);
				for(i = 0; i < length; i++)
				{
					for(j = 0; j < layer->components; j++)
						uint8_data[i * components + j] = layer->data.uint8_data[i * layer->components + j];
					for(; j < components; j++)
						uint8_data[i * components + j] = 0;
				}
				free(layer->data.uint8_data);
				layer->data.uint8_data = uint8_data;
			break;
			case HXA_LDT_INT32 :
				int32_data = malloc((sizeof *int32_data) * length * components);
				for(i = 0; i < length; i++)
				{
					for(j = 0; j < layer->components; j++)
						int32_data[i * components + j] = layer->data.int32_data[i * layer->components + j];
					for(; j < components; j++)
						int32_data[i * components + j] = 0;
				}
				free(layer->data.int32_data);
				layer->data.int32_data = int32_data;
			break;
			case HXA_LDT_FLOAT :
				float_data = malloc((sizeof *float_data) * length * components);
				for(i = 0; i < length; i++)
				{
					for(j = 0; j < layer->components; j++)
						float_data[i * components + j] = layer->data.float_data[i * layer->components + j];
					for(; j < components; j++)
						float_data[i * components + j] = 0;
				}
				free(layer->data.float_data);
				layer->data.float_data = float_data;
			break;
			case HXA_LDT_DOUBLE  :
				double_data = malloc((sizeof *double_data) * length * components);
				for(i = 0; i < length; i++)
				{
					for(j = 0; j < layer->components; j++)
						double_data[i * components + j] = layer->data.double_data[i * layer->components + j];
					for(; j < components; j++)
						double_data[i * components + j] = 0;
				}
				free(layer->data.double_data);
				layer->data.double_data = double_data;
			break;
		}
		return;
	}
}

void hxa_util_layer_stack_layer_length_set(HXALayer *layer, unsigned int old_length, unsigned int new_length)
{
	switch(layer->type)
	{
		case HXA_LDT_UINT8 :
			layer->data.uint8_data = realloc(layer->data.uint8_data, (sizeof *layer->data.uint8_data) * new_length * layer->components);					
			if(old_length < new_length)
				memset(&layer->data.uint8_data[old_length * layer->components], 0, 
					(new_length - old_length) * layer->components * (sizeof *layer->data.uint8_data));
		break;
		case HXA_LDT_INT32 :
			layer->data.int32_data = realloc(layer->data.int32_data, (sizeof *layer->data.int32_data) * new_length * layer->components);					
			if(old_length < new_length)
				memset(&layer->data.int32_data[old_length * layer->components], 0, 
					(new_length - old_length) * layer->components * (sizeof *layer->data.int32_data));
		break;
		case HXA_LDT_FLOAT :
			layer->data.float_data = realloc(layer->data.float_data, (sizeof *layer->data.float_data) * new_length * layer->components);					
			if(old_length < new_length)
				memset(&layer->data.float_data[old_length * layer->components], 0, 
					(new_length - old_length) * layer->components * (sizeof *layer->data.float_data));
		break;
		case HXA_LDT_DOUBLE :
			layer->data.double_data = realloc(layer->data.double_data, (sizeof *layer->data.double_data) * new_length * layer->components);					
			if(old_length < new_length)
				memset(&layer->data.double_data[old_length * layer->components], 0, 
					(new_length - old_length) * layer->components * (sizeof *layer->data.double_data));
		break;
	}
}

void hxa_util_layer_stack_vertex_count(HXANode *node, unsigned int length)
{
	unsigned int i;
	for(i = 0; i < node->content.geometry.vertex_stack.layer_count; i++)
		hxa_util_layer_stack_layer_length_set(&node->content.geometry.vertex_stack.layers[i], node->content.geometry.vertex_count, length);
	node->content.geometry.vertex_count = length;
}

void hxa_util_layer_stack_face_count(HXANode *node, unsigned int length)
{
	unsigned int i;
	for(i = 0; i < node->content.geometry.face_stack.layer_count; i++)
		hxa_util_layer_stack_layer_length_set(&node->content.geometry.face_stack.layers[i], node->content.geometry.face_count, length);
	node->content.geometry.face_count = length;
}

void hxa_util_layer_stack_corner_count(HXANode *node, unsigned int length)
{
	unsigned int i;
	for(i = 0; i < node->content.geometry.corner_stack.layer_count; i++)
		hxa_util_layer_stack_layer_length_set(&node->content.geometry.corner_stack.layers[i], node->content.geometry.edge_corner_count, length);
	for(i = 0; i < node->content.geometry.edge_stack.layer_count; i++)
		hxa_util_layer_stack_layer_length_set(&node->content.geometry.edge_stack.layers[i], node->content.geometry.edge_corner_count, length);
	node->content.geometry.edge_corner_count = length;
}