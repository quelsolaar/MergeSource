#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "forge.h"
#include "imagine.h"
#include "betray.h"
#include "relinquish.h"
#include "seduce.h"

HXAMeta *hxa_util_meta_add(HXAMeta **meta_data, hxa_uint32 *meta_data_count, char *name, HXAMetaDataType type, void *data, unsigned int length, int copy);
int hxa_util_meta_resize(HXAMeta *meta, unsigned int new_size);
int hxa_util_meta_delete(HXAMeta **meta_data, hxa_uint32 *meta_data_count, HXAMeta *meta);
void hxa_util_layer_stack_change_components(HXALayer *layer, unsigned int length, hxa_uint8 components);
void hxa_util_layer_stack_vertex_count(HXANode *node, unsigned int length);
void hxa_util_layer_stack_face_count(HXANode *node, unsigned int length);
void hxa_util_layer_stack_corner_count(HXANode *node, unsigned int length);


void hxa_seduce_panel_meta_text_done_func(void *user, char *text)
{
	HXAMeta *meta;
	uint i;
	meta = user;
	for(i = 0; text[i] != 0; i++);
	i++;	
	meta->value.text_value = malloc(i);
	memcpy(meta->value.text_value, text, i);
	meta->array_length = i;
}

float hxa_seduce_panel_meta(BInputState *input, HXAMeta **meta, uint *meta_count, void **expand, float text_size, float x, float y, float *y_span, float brightness, float red, float green, float blue)
{
	char *type_names[] = {"INT64", "DOUBLE", "NODE", "TEXT", "BINARY", "META"};
	char *text, text_buffer[64];  
	uint i, j, count, ivalue;
	double dvalue;
	float f;
	if(y <= y_span[0] && y >= y_span[1])
	{
		f = seduce_text_line_draw(NULL, x, y, text_size, SEDUCE_T_SPACE, "Meta :", red, green, blue, 1.0, -1);
		i = seduce_text_button_list(input, meta_count, x + f + text_size * 1, y, 2.0, SEDUCE_TBAS_LEFT, text_size, SEDUCE_T_SPACE, text_size, type_names, HXA_MDT_COUNT, -1, brightness, brightness, brightness, 1.0, red, green, blue, 1); /* A horizontal list of text buttons. "selected" will be highlited. Returns the number of the text beeing clicked. returnes -1 by default. */	
		if(i != ~0)
		{
			hxa_util_meta_add(meta, meta_count, "Unnamed", i, NULL, 1, FALSE);
		}
	}

	y -= text_size * 4.0;

	for(i = 0; i < *meta_count; i++)
	{
		if(y <= y_span[0] && y >= y_span[1])
		{
			if(seduce_widget_button_icon(input, &((*meta)[i].type), seduce_object_3d_object_lookup("close"), x + 1 * text_size, y + text_size * 0.75, text_size * 1.5, 1, NULL))
			{
				hxa_util_meta_delete(meta, meta_count, &((*meta)[i]));
				break;
			}
	
			text = "Name :";
			seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
			seduce_text_edit_line(input, (*meta)[i].name, NULL, (*meta)[i].name, 256, x + 15 * text_size, y, 0.4, text_size, "Name", TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0);
		}
		y -= text_size * 3.0;
		if(y <= y_span[0] && y >= y_span[1])
		{
			text = "Type :";
			seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
			seduce_text_line_draw(NULL, x + 15 * text_size, y, text_size, SEDUCE_T_SPACE, type_names[(*meta)[i].type], brightness, brightness, brightness, 0.7, -1);
		}
		y -= text_size * 3.0;

		if((*meta)[i].type == HXA_MDT_META)
		{
			y = hxa_seduce_panel_meta(input, (HXAMeta **)&(*meta)[i].value.array_of_meta, &(*meta)[i].array_length, expand, text_size, x + 8.5 * text_size, y, y_span, brightness, red, green, blue);
		}else if((*meta)[i].type == HXA_MDT_TEXT)
		{
			if(y <= y_span[0] && y >= y_span[1])
			{
				text = "Text :";
				seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
				seduce_text_edit_line(input, (*meta)[i].value.text_value, NULL, (*meta)[i].value.text_value, 4096, x + 15 * text_size, y, 0.4, text_size, "Name", TRUE, hxa_seduce_panel_meta_text_done_func, &(*meta)[i], brightness, brightness, brightness, 1.0, red, green, blue, 1.0);
			}
			y -= text_size * 3.0;
		}else
		{
			count = (*meta)[i].array_length;
			if(y <= y_span[0] && y >= y_span[1])
			{
				text = "Count :";
				seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);				
				if(S_TIS_DONE == seduce_text_edit_uint(input, &(*meta)[i].array_length, NULL, &count, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0))/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */		
					hxa_util_meta_resize(&(*meta)[i], count);
			}
			y -= text_size * 3.0;
			if(count > 0)
			{
				if(*expand != &(*meta)[i] && count > 1)
				{
					if(y <= y_span[0] && y >= y_span[1])
						if(seduce_text_button(input, &(*meta)[i].value.int64_value, x + 15 * text_size, y, 0, text_size, SEDUCE_T_SPACE, "Expand", brightness, brightness, brightness, 1.0, red, green, blue, 1.0))
							*expand = &(*meta)[i];
					y -= text_size * 3.0;
				}else
				{
					switch((*meta)[i].type)
					{
						case HXA_MDT_INT64 :
							text = "Integer :";
							if(y <= y_span[0] && y >= y_span[1])
								seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
							for(j = 0; j < count; j++)
							{
								if(y <= y_span[0] && y >= y_span[1])
								{
									if(j > 0)
									{
										snprintf(text_buffer, 64, "%u :", j);
										seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text_buffer, -1), y, text_size, SEDUCE_T_SPACE, text_buffer, red, green, blue, 1.0, -1);
									}
									ivalue = (*meta)[i].value.int64_value[j];
									seduce_text_edit_uint(input, &(*meta)[i].value.int64_value[j], NULL, &ivalue, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0);/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
									(*meta)[i].value.int64_value[j] = ivalue;
								}
								y -= text_size * 3.0;
							}
						break;
						case HXA_MDT_DOUBLE :
							text = "Floating Point :";
							if(y <= y_span[0] && y >= y_span[1])
								seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
							for(j = 0; j < count; j++)
							{
								if(y <= y_span[0] && y >= y_span[1])
								{
									if(j > 0)
									{
										snprintf(text_buffer, 64, "%u :", j);
										seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text_buffer, -1), y, text_size, SEDUCE_T_SPACE, text_buffer, red, green, blue, 1.0, -1);
									}
									dvalue = (*meta)[i].value.double_value[j];
									seduce_text_edit_double(input, &(*meta)[i].value.double_value[j], NULL, &dvalue, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0);/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
									(*meta)[i].value.double_value[j] = dvalue;
								}
								y -= text_size * 3.0;
							}
						break;
						case HXA_MDT_NODE :
							text = "Node :";
							if(y <= y_span[0] && y >= y_span[1])
								seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
							for(j = 0; j < count; j++)
							{
								if(y <= y_span[0] && y >= y_span[1])
								{
									if(j > 0)
									{
										snprintf(text_buffer, 64, "%u :", j);
										seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text_buffer, -1), y, text_size, SEDUCE_T_SPACE, text_buffer, red, green, blue, 1.0, -1);
									}
									ivalue = (*meta)[i].value.node_value[j];
									seduce_text_edit_uint(input, &(*meta)[i].value.node_value[j], NULL, &ivalue, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0);/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
									(*meta)[i].value.node_value[j] = ivalue;
								}
								y -= text_size * 3.0;
							}
						break;
						case HXA_MDT_BINARY :
							text = "Binary :";
							if(y <= y_span[0] && y >= y_span[1])
								seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
							for(j = 0; j < count; j++)
							{
								if(y <= y_span[0] && y >= y_span[1])
								{
									if(j > 0)
									{
										snprintf(text_buffer, 64, "%u :", j);
										seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text_buffer, -1), y, text_size, SEDUCE_T_SPACE, text_buffer, red, green, blue, 1.0, -1);
									}
									ivalue = (*meta)[i].value.bin_value[j];
									seduce_text_edit_uint(input, &(*meta)[i].value.bin_value[j], NULL, &ivalue, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0);/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
									(*meta)[i].value.bin_value[j] = ivalue;
								}
								y -= text_size * 3.0;
							}
						break;
						case HXA_MDT_META :
						break;
					}
				}
			}
		}
		y -= text_size;
	}
	return y;
}


float hxa_seduce_panel_meta_size(HXAMeta **meta, uint *meta_count, void **expand, float text_size)
{  
	uint i,  count;
	float y;
	y = text_size * 4.0;
	for(i = 0; i < *meta_count; i++)
	{
		y += text_size * 6.0;
		if((*meta)[i].type == HXA_MDT_META)
		{
			y += hxa_seduce_panel_meta_size((HXAMeta **)&(*meta)[i].value.array_of_meta, &(*meta)[i].array_length, expand, text_size);
		}else if((*meta)[i].type == HXA_MDT_TEXT)
			y += text_size * 3.0;
		else
		{
			count = (*meta)[i].array_length;
			y += text_size * 3.0;
			if(count > 0)
			{
				if(*expand != &(*meta)[i])
				{
					y += text_size * 3.0;
				}else
				{
					switch((*meta)[i].type)
					{
						default :
							y += text_size * 3.0 * (float)count;
						break;
						case HXA_MDT_META :
						break;
					}
				}
			}
		}
		y += text_size;
	}
	return y;
}

float hxa_seduce_panel_stack(BInputState *input, HXALayerStack *stack, uint stack_size, char *name, float text_size, float x, float y, float size_x, float *y_span, float brightness, float red, float green, float blue)
{	
	char *type_names[] = {"UINT8", "INT32", "FLOAT", "DOUBLE"};
	char *text; 
	HXALayer *layer;
	uint i = ~0, components;
	float f;
	if(y <= y_span[0] && y >= y_span[1])
	{
		f = seduce_text_line_draw(NULL, x, y, text_size, SEDUCE_T_SPACE, name, red, green, blue, 1.0, -1);
		i = seduce_text_button_list(input, &stack->layer_count, x + f + text_size * 2, y, size_x, SEDUCE_TBAS_LEFT, text_size, SEDUCE_T_SPACE, text_size, type_names, HXA_LDT_COUNT, -1, brightness, brightness, brightness, 0.7, red, green, blue, 1); /* A horizontal list of text buttons. "selected" will be highlited. Returns the number of the text beeing clicked. returnes -1 by default. */
	}
	if(i != ~0)
	{
		layer = realloc(stack->layers, (sizeof *stack->layers) * (stack->layer_count + 1));
		if(layer != NULL)
		{
			stack->layers = layer;
			layer = &layer[stack->layer_count];
			layer->type = i;
			layer->components = 1;
			snprintf(layer->name, 256, "UnNamed");
			switch(i)
			{
				case HXA_LDT_UINT8 :
					layer->data.uint8_data = malloc((sizeof *layer->data.uint8_data) * stack_size);
					for(i = 0; i < stack_size; i++)
						layer->data.uint8_data[i] = 0;
				break;
				case HXA_LDT_INT32 :
					layer->data.int32_data = malloc((sizeof *layer->data.int32_data) * stack_size);
					for(i = 0; i < stack_size; i++)
						layer->data.int32_data[i] = 0;
				break;
				case HXA_LDT_FLOAT :
					layer->data.float_data = malloc((sizeof *layer->data.float_data) * stack_size);
					for(i = 0; i < stack_size; i++)
						layer->data.float_data[i] = 0;
				break;
				case HXA_LDT_DOUBLE :
					layer->data.double_data = malloc((sizeof *layer->data.double_data) * stack_size);
					for(i = 0; i < stack_size; i++)
						layer->data.double_data[i] = 0;
				break;				
			}
			stack->layer_count++;
		}
	}

	y -= text_size * 3.0;
	for(i = 0; i < stack->layer_count; i++)
	{
		if(y <= y_span[0] && y >= y_span[1])
		{
			text = "Name :";
			seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
			seduce_text_edit_line(input, &stack->layers[i].name, NULL, stack->layers[i].name, 256, x + 15 * text_size, y, 0.4, text_size, "Name", TRUE, NULL, NULL, brightness, brightness, brightness, 0.7, red, green, blue, 1.0);
		//	seduce_text_line_draw(NULL, x + 15 * text_size, y, text_size, SEDUCE_T_SPACE, stack->layers[i].name, red, green, blue, 1.0, -1);
		}
		y -= text_size * 3.0;
		if(y <= y_span[0] && y >= y_span[1])
		{
			text = "Type :";
			seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
			seduce_text_line_draw(NULL, x + 15 * text_size, y, text_size, SEDUCE_T_SPACE, type_names[stack->layers[i].type], brightness, brightness, brightness, 0.7, -1);
		}
		y -= text_size * 3.0;
		if(y <= y_span[0] && y >= y_span[1])
		{
			text = "Components :";
			seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);
			components = stack->layers[i].components;
			if(S_TIS_DONE == seduce_text_edit_uint(input, &stack->layers[i].components, NULL, &components, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 0.7, red, green, blue, 1.0))/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */
				hxa_util_layer_stack_change_components(&stack->layers[i], stack_size, components);
		}
		y -= text_size * 4.0;
	}
	return y;
}



float hxa_seduce_panel_stack_size(HXALayerStack *stack, float text_size)
{	
	return text_size * (3.0 + 10.0 * (float)stack->layer_count);
}


boolean hxa_seduce_panel_header(BInputState *input, HXAFile *file, float x, float y, float size_x, float size_y, float text_size, float brightness, float red, float green, float blue)
{
	const char *node_types[] = {"Meta", "Geometry", "Image"};
	HXANode *node;
	HXALayer *layer;
	static void *expand;
	float f = 0;
	uint i;
	f = seduce_text_line_draw(NULL, x, y, text_size, SEDUCE_T_SPACE, "Add Node:", red, green, blue, 1.0, -1);
	i = seduce_text_button_list(input, hxa_seduce_panel_header, x + f + text_size * 2, y, size_x, SEDUCE_TBAS_LEFT, text_size, SEDUCE_T_SPACE, text_size, node_types, 3, -1, brightness, brightness, brightness, 0.7, red, green, blue, 1); /* A horizontal list of text buttons. "selected" will be highlited. Returns the number of the text beeing clicked. returnes -1 by default. */
	if(i != ~0)
	{ 
		node = realloc(file->node_array, (sizeof *node) * (file->node_count + 1));
		if(node != NULL)
		{	
			file->node_array = node;
			node = &node[file->node_count++];
			node->meta_data = NULL;
			node->meta_data_count = 0;
			switch(i)
			{
				case 0 :
					node->type = HXA_NT_META_ONLY;
				break;
				case 1 :
					node->type = HXA_NT_GEOMETRY;
					node->content.geometry.vertex_count = 0;
					node->content.geometry.edge_corner_count = 0;
					node->content.geometry.face_count = 0;

					node->content.geometry.corner_stack.layer_count = 1;
					layer = node->content.geometry.corner_stack.layers = malloc(sizeof *layer);
					layer->components = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_COMPONENTS;
					snprintf(layer->name, 256, "%s", HXA_CONVENTION_HARD_BASE_CORNER_LAYER_NAME);
					layer->type = HXA_CONVENTION_HARD_BASE_CORNER_LAYER_TYPE;
					layer->data.int32_data = NULL;

					node->content.geometry.vertex_stack.layer_count = 1;
					layer = node->content.geometry.vertex_stack.layers = malloc((sizeof *layer));
					layer->components = HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_COMPONENTS;
					snprintf(layer->name, 256, "%s", HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME);
					layer->type = HXA_LDT_FLOAT;
					layer->data.float_data = NULL;

					node->content.geometry.edge_stack.layer_count = 0;
					node->content.geometry.edge_stack.layers = NULL;
					node->content.geometry.face_stack.layer_count = 0;
					node->content.geometry.face_stack.layers = NULL;
				break;
				case 2 :
					node->type = HXA_NT_IMAGE;
					node->content.image.resolution[0] = 16;
					node->content.image.resolution[1] = 16;
					node->content.image.resolution[2] = 1;
					node->content.image.type = HXA_IT_2D_IMAGE;
					node->content.image.image_stack.layer_count = 0;
					node->content.image.image_stack.layers = NULL;
				break;
			}
		}
	}
	return TRUE;
}




float hxa_seduce_panel_size(HXAFile *file, float size_x, float size_y, float text_size)
{
	HXANode *node;
	HXALayer *layer;
	char *text, text_buffer[64];
	static void *expand;
	float f = 0, y;
	uint i, count;

	y = text_size * 6.0;
	for(i = 0; i < file->node_count; i++)
	{
		y += text_size * 3.0;
		y += hxa_seduce_panel_meta_size(&file->node_array[i].meta_data, &file->node_array[i].meta_data_count, &expand, text_size);	
		switch(file->node_array[i].type)
		{
			case HXA_NT_GEOMETRY :
				y += text_size * 9.0;				
				y += text_size * (3.0 + 3.0 * (float)file->node_array[i].content.geometry.vertex_stack.layer_count);
				y += text_size * (3.0 + 3.0 * (float)file->node_array[i].content.geometry.edge_stack.layer_count);
				y += text_size * (3.0 + 3.0 * (float)file->node_array[i].content.geometry.corner_stack.layer_count);
				y += text_size * (3.0 + 3.0 * (float)file->node_array[i].content.geometry.face_stack.layer_count);

			break;
			case HXA_NT_IMAGE :
				y += text_size * (3.0 + 3.0 * (float)file->node_array[i].content.image.image_stack.layer_count);
			break;
		}
	}
	return y;
}

boolean hxa_seduce_panel(BInputState *input, HXAFile *file, float x, float y, float size_x, float size_y, float text_size, float brightness, float red, float green, float blue)
{
	const char *node_types[] = {"Meta", "Geometry", "Image"};
	static float scroll = 0;
	HXANode *node;
	HXALayer *layer;
	char *text, text_buffer[64];
	static void *expand;
	float f = 0, y_span[2], full_size, pos_a[3], pos_b[3];
	uint i, count;
	
	full_size = hxa_seduce_panel_size(file, size_x, size_y, text_size);
	
	pos_a[0] = x + text_size * 0.5;
	pos_a[1] = y;
	pos_a[2] = 0;
	pos_b[0] = x + text_size * 0.5;
	pos_b[1] = y - size_y;
	pos_b[2] = 0;
	seduce_manipulator_slider(input, &file->version, &scroll, pos_a, pos_b, full_size, size_y, 1, red, green, blue);

	y_span[0] = y;
	y_span[1] = y - size_y + text_size * 2.0;
	x += text_size;
	y += scroll;
	y -= text_size * 3.0;
	if(y <= y_span[0] && y >= y_span[1])
		hxa_seduce_panel_header(input, file, x, y, size_x, size_y, text_size, brightness, red, green, blue);
	y -= text_size * 3.0;


	for(i = 0; i < file->node_count; i++)
	{
		if(y <= y_span[0] && y >= y_span[1])
			seduce_text_line_draw(NULL, x, y, text_size, SEDUCE_T_SPACE, "Node", red, green, blue, 1.0, -1);
		y -= text_size * 3.0;
		y = hxa_seduce_panel_meta(input, &file->node_array[i].meta_data, &file->node_array[i].meta_data_count, &expand, text_size, x + text_size * 3.0, y, y_span, brightness, red, green, blue);

		switch(file->node_array[i].type)
		{
			case HXA_NT_GEOMETRY :
			
				if(y <= y_span[0] && y >= y_span[1])
				{
					text = "Vetex Count :";					
					seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);	
					count = file->node_array[i].content.geometry.vertex_count;
					if(S_TIS_DONE == seduce_text_edit_uint(input, &file->node_array[i].content.geometry.vertex_count, NULL, &count, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0))/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */				
						hxa_util_layer_stack_vertex_count(&file->node_array[i], count);
				}
				y -= text_size * 3.0;
				if(y <= y_span[0] && y >= y_span[1])
				{
					text = "Corner Count :";
					seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);	
					count = file->node_array[i].content.geometry.edge_corner_count;
					if(S_TIS_DONE == seduce_text_edit_uint(input, &file->node_array[i].content.geometry.edge_corner_count, NULL, &count, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0))/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */				
						hxa_util_layer_stack_corner_count(&file->node_array[i], count);
				}
				y -= text_size * 3.0;
				if(y <= y_span[0] && y >= y_span[1])
				{
					text = "Face Count :";			
					seduce_text_line_draw(NULL, x + 14 * text_size - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), y, text_size, SEDUCE_T_SPACE, text, red, green, blue, 1.0, -1);	
					count = file->node_array[i].content.geometry.face_count;
					if(S_TIS_DONE == seduce_text_edit_uint(input, &file->node_array[i].content.geometry.face_count, NULL, &count, x + 15 * text_size, y, 0.2, text_size, TRUE, NULL, NULL, brightness, brightness, brightness, 1.0, red, green, blue, 1.0))/* Creates an editable text feild for typing in 64bit unsigend integer numbers.  Lable will be printed if the text field is empty. The text striing cna either be modifyed as the user types, or if given a done_func a call back can be called when the user completes the typing with a new string. */				
						hxa_util_layer_stack_face_count(&file->node_array[i], count);
				}
				y -= text_size * 3.0;
				y = hxa_seduce_panel_stack(input, &file->node_array[i].content.geometry.vertex_stack, file->node_array[i].content.geometry.vertex_count, "Vertex stack :", text_size, x, y, size_x, y_span, brightness, red, green, blue);
				y = hxa_seduce_panel_stack(input, &file->node_array[i].content.geometry.corner_stack, file->node_array[i].content.geometry.edge_corner_count, "Corner stack :", text_size, x, y, size_x, y_span, brightness, red, green, blue);
				y = hxa_seduce_panel_stack(input, &file->node_array[i].content.geometry.edge_stack, file->node_array[i].content.geometry.edge_corner_count, "Edge stack :", text_size, x, y, size_x, y_span, brightness, red, green, blue);
				y = hxa_seduce_panel_stack(input, &file->node_array[i].content.geometry.face_stack, file->node_array[i].content.geometry.face_count, "Face stack :", text_size, x, y,  size_x, y_span, brightness, red, green, blue);
			break;
			case HXA_NT_IMAGE :
				count = 10;
				y = hxa_seduce_panel_stack(input, &file->node_array[i].content.image.image_stack, count, "Image stack :", text_size, x, y, size_x, y_span, brightness, red, green, blue);
			break;
		}
	}
	return TRUE;
}