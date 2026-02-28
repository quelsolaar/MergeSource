#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "hxa_utils.h"

#define FALSE 0
#define TRUE !FALSE

void hxa_util_blur_node_line_uint8(unsigned char *layer, unsigned long long *buffer, unsigned int buffer_length, unsigned long long *weights, unsigned int weight_length, unsigned int stride)
{
	unsigned char *b;
	unsigned int i, j, k, half_length;
	unsigned long long sum, weight_sum;
	half_length = weight_length / 2;

	b = layer;
	for(i = j = 0; i < buffer_length; j += stride)	
		buffer[i++] = layer[j];
	for(i = j = 0; i < half_length; i++)	
	{
		sum = weight_sum = 0;
		for(k = 0; k < half_length + i; k++)
		{
			weight_sum += weights[half_length - i + k];
			sum += buffer[k] * weights[half_length - i + k];
		}
		layer[j] = sum / weight_sum;
		j += stride;
	}
	for(; i < buffer_length - half_length; i++)	
	{
		sum = weight_sum = 0;
		for(k = 0; k < weight_length; k++)
		{
			weight_sum += weights[k];
			sum += buffer[i + k - half_length] * weights[k];
		}
		layer[j] = sum / weight_sum;
		j += stride;
	}
	for(; i < buffer_length; i++)	
	{
		sum = weight_sum = 0;
		for(k = 0; i + k - half_length < buffer_length; k++)
		{
			weight_sum += weights[k];
			sum += buffer[i + k - half_length] * weights[k];
		}
		layer[j] = sum / weight_sum;
		j += stride;
	}
}


void hxa_util_blur_node_line(HXALayer *layer, unsigned long long *buffer, unsigned int start, unsigned int buffer_length, unsigned long long *weights, unsigned int weight_length, unsigned int stride)
{
	switch(layer->type)
	{
		case HXA_LDT_UINT8 :
			hxa_util_blur_node_line_uint8(&layer->data.uint8_data[start], buffer, buffer_length, weights, weight_length, stride);
		break;
		case HXA_LDT_INT32 :
		break;
		case HXA_LDT_FLOAT :
		break;
		case HXA_LDT_DOUBLE :
		break;
	}
}

void *hxa_util_blur_init(HXANode *node, HXALayer *layer, float blur_size, unsigned int *size_output, void **buffer)
{
	unsigned int size, blur_size_half, blur_size_full, i;
	double f, min, *weight_d;
	unsigned long long *weight_l;
	size = node->content.image.resolution[0];
	switch(node->content.image.type)
	{
		case HXA_IT_CUBE_IMAGE :
			if(size < node->content.image.resolution[1])
				size = node->content.image.resolution[1];
		break;
		case HXA_IT_1D_IMAGE :
		break;
		case HXA_IT_2D_IMAGE :
			if(size < node->content.image.resolution[1])
				size = node->content.image.resolution[1];
		break;
		case HXA_IT_3D_IMAGE :
			if(size < node->content.image.resolution[1])
				size = node->content.image.resolution[1];
			if(size < node->content.image.resolution[2])
				size = node->content.image.resolution[2];
		break;
	}
	blur_size_half = 1 + blur_size + 0.999;
	*size_output = blur_size_full = blur_size_half + blur_size_half - 1;
	if(layer->type == HXA_LDT_UINT8 || layer->type == HXA_LDT_INT32)
	{
		*buffer = malloc(sizeof(unsigned long long) * size);
		min = 256.0 / (5.0);
		weight_l = malloc(sizeof(double) * *size_output);
		for(i = 0; i < blur_size_half; i++)
		{
			f = 2.0 * (double)i / blur_size;
			weight_l[blur_size_half - 1 - i] = weight_l[blur_size_half - 1 + i] = (unsigned long long)(256.0 * 256.0 / (1.0 + f * f) - min);
 		}
		return weight_l;
	}else
	{
		*buffer = malloc(sizeof(double) * size);
		min = 256.0 * 256.0 / (5.0);
		weight_d = malloc(sizeof(double) * *size_output);
		for(i = 0; i < blur_size_half; i++)
		{
			f = 2.0 * (double)i / blur_size;
			weight_d[i] = weight_d[blur_size_full - 1 - i] = 256.0 / (1.0 + f * f) - min;
		}
		return weight_d;
	}
}

void hxa_util_blur_layer(HXANode *node, HXALayer *layer, float blur_size)
{
	void *buffer, *weights;
	unsigned int i, j, k, weight_length;
	weights = hxa_util_blur_init(node, layer, blur_size, &weight_length, &buffer);
	switch(node->content.image.type)
	{
		case HXA_IT_CUBE_IMAGE :
			for(k = 0; k < layer->components * 6 * node->content.image.resolution[0] * node->content.image.resolution[1]; k < layer->components * node->content.image.resolution[0] * node->content.image.resolution[1])
			{
				for(i = 0; i < layer->components; i++)
				{
					for(j = 0; j < node->content.image.resolution[0]; j++)
						hxa_util_blur_node_line(layer, buffer, k + i + layer->components * j, node->content.image.resolution[1], weights, weight_length, node->content.image.resolution[0] * (hxa_uint32)layer->components);
					for(j = 0; j < node->content.image.resolution[1]; j++)
						hxa_util_blur_node_line(layer, buffer, k + i + layer->components * j * node->content.image.resolution[0], node->content.image.resolution[0], weights, weight_length,  (hxa_uint32)layer->components);
				}
			}
		break;
		case HXA_IT_1D_IMAGE :
			for(i = 0; i < layer->components; i++)
			{
				hxa_util_blur_node_line(layer, buffer, i, node->content.image.resolution[0], weights, weight_length,  (hxa_uint32)layer->components);
			}
		break;
		case HXA_IT_2D_IMAGE :
			for(i = 0; i < layer->components; i++)
			{
				for(j = 0; j < node->content.image.resolution[0]; j++)
					hxa_util_blur_node_line(layer, buffer, i + layer->components * j, node->content.image.resolution[1], weights, weight_length, node->content.image.resolution[0] * (hxa_uint32)layer->components);
				for(j = 0; j < node->content.image.resolution[1]; j++)
					hxa_util_blur_node_line(layer, buffer, i + layer->components * j * node->content.image.resolution[0], node->content.image.resolution[0], weights, weight_length,  (hxa_uint32)layer->components);
			}
		break;
		case HXA_IT_3D_IMAGE :
		break;
	}
	free(buffer);
	free(weights);
}

void hxa_util_blur_node(HXANode *node, float blur_size)
{
	unsigned int i;
	if(node->type != HXA_NT_IMAGE)
		return;
	for(i = 0; i < node->content.image.image_stack.layer_count; i++)
		hxa_util_blur_layer(node, &node->content.image.image_stack.layers[i], blur_size);
	
}