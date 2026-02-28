#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "hxa_utils.h"

#define FALSE 0
#define TRUE !FALSE

HXANode *hxa_util_default_image(HXAFile *file, unsigned int x, unsigned int y, unsigned int z, HXAImageType image_type, unsigned char components, HXALayerDataType data_type, char *name)
{
	HXANode *node;
	HXALayer *layer;
	unsigned int i, size;
	file->node_array = realloc(file->node_array, (sizeof *file->node_array) * (file->node_count + 1));
	node = &file->node_array[file->node_count++];
	node->meta_data = NULL;
	node->meta_data_count = 0;
	node->type = HXA_NT_IMAGE;
	node->content.image.type = image_type;
	node->content.image.resolution[0] = x;
	node->content.image.resolution[1] = y;
	node->content.image.resolution[2] = z;
	node->content.image.image_stack.layer_count = 1;
	layer = node->content.image.image_stack.layers = malloc(sizeof *node->content.image.image_stack.layers);
	layer->components = components;
	for(i = 0; i < 255 && name[i] != 0; i++)
		layer->name[i] = name[i];
	layer->name[i] = 0;
	layer->type = data_type;
	switch(image_type)
	{
		case HXA_IT_CUBE_IMAGE :
			size = node->content.image.resolution[0] * node->content.image.resolution[1] * 6;
		case HXA_IT_1D_IMAGE :
			size = node->content.image.resolution[0];
		break;
		case HXA_IT_2D_IMAGE :
			size = node->content.image.resolution[0] * node->content.image.resolution[1];
		break;
		case HXA_IT_3D_IMAGE :
			size = node->content.image.resolution[0] * node->content.image.resolution[1] * node->content.image.resolution[2];
		break;
	}	
	size *= components;
	switch(image_type)
	{
		case HXA_LDT_UINT8 :
			layer->data.uint8_data = malloc(size * (sizeof *layer->data.uint8_data));
			for(i = 0; i < size; i++)
				layer->data.uint8_data[i] = 0;
		break;
		case HXA_LDT_INT32 :
			layer->data.int32_data = malloc(size * (sizeof *layer->data.int32_data));
			for(i = 0; i < size; i++)
				layer->data.int32_data[i] = 0;
		break;
		case HXA_LDT_FLOAT :
			layer->data.float_data = malloc(size * (sizeof *layer->data.float_data));
			for(i = 0; i < size; i++)
				layer->data.float_data[i] = 0;
		break;
		case HXA_LDT_DOUBLE :
			layer->data.double_data = malloc(size * (sizeof *layer->data.double_data));
			for(i = 0; i < size; i++)
				layer->data.double_data[i] = 0;
		break;
	}
	return node; 
}
