#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "hxa_utils.h"

HXALayer *hxa_util_extrude_find_in_layer_stack(HXALayerStack *stack, HXALayerDataType type, unsigned int components, char *name)
{
	unsigned int i, j;
	for(i = 0; i < stack->layer_count; i++)
	{
		if(stack->layers[i].type == HXA_LDT_INT32 &&
		  (stack->layers[i].components >= components))
		{
			for(j = 0; name[j] != 0 && name[j] == stack->layers[i].name[j]; j++);
			if(name[j] == stack->layers[i].name[j])
				return &stack->layers[i];
		}
	}
	return NULL;
}



 int *hxa_util_extrude_vertex_ids(HXANode *node, unsigned int *edge_height, unsigned int level_count, unsigned int *output_count)
{	
	unsigned int i, j, *section, count, vertex_id;
	int *vertex_id_buffer, test = ~0;
	hxa_int32 *ref, r;
	count = node->content.geometry.vertex_count;
	vertex_id_buffer = malloc((sizeof *vertex_id_buffer) * count * level_count);
	memset(vertex_id_buffer, 0xFF, (sizeof * vertex_id_buffer) * count * level_count);

	ref = node->content.geometry.corner_stack.layers->data.int32_data;
	count = node->content.geometry.edge_corner_count;
	for(i = 0; i < count; i++)
	{
		if((r = *ref++) < 0)
			r = -1 - r;
		vertex_id_buffer[r * level_count + edge_height[i]] = 0;
	}
	vertex_id = count = node->content.geometry.vertex_count;
	section = vertex_id_buffer;
	for(i = 0; i < count; i++)
	{
		for(j = 0; j < level_count && section[j] == ~0; j++);
		if(j < level_count)
		{
			section[j++] = i;
			for(; j < level_count; j++)
				if(section[j] != 0xFFFFFFFF)
					section[j] = vertex_id++;
		}
		section += level_count;
	}
	*output_count = vertex_id;
	return vertex_id_buffer;
}

void hxa_util_extrude_vertices(HXANode *node, hxa_int32 *selector, hxa_int32 *values, double *lengths, unsigned int value_count, double *direction, unsigned int *vertex_id_buffer, unsigned int vertex_count)
{	
	unsigned int i, j, k, l;

	for(i = 0; i < node->content.geometry.vertex_stack.layer_count; i++)
	{

		if(node->content.geometry.vertex_stack.layers[i].components == 3 &&
		   (node->content.geometry.vertex_stack.layers[i].type == HXA_LDT_FLOAT ||
		   node->content.geometry.vertex_stack.layers[i].type == HXA_LDT_DOUBLE))
		{
			if(node->content.geometry.vertex_stack.layers[i].type == HXA_LDT_FLOAT)
			{ 
				float *new_vertex_array, *write, read[3];
				new_vertex_array = node->content.geometry.vertex_stack.layers[i].data.float_data = realloc(node->content.geometry.vertex_stack.layers[i].data.float_data, (sizeof *new_vertex_array) * 3 * vertex_count);
				l = 0;
				for(j = 0; j < node->content.geometry.vertex_count; j++)
				{
					read[0] = new_vertex_array[j * 3];
					read[1] = new_vertex_array[j * 3 + 1];
					read[2] = new_vertex_array[j * 3 + 2];
					l++;
					for(k = 0; k < value_count; k++)
					{ 
						if(vertex_id_buffer[l] != 0xFFFFFFFF)
						{
							write = &new_vertex_array[vertex_id_buffer[l] * 3];
							write[0] = read[0] + direction[0] * (float)lengths[k];
							write[1] = read[1] + direction[1] * (float)lengths[k];
							write[2] = read[2] + direction[2] * (float)lengths[k];	
						}
						l++;
					}
				}
			}else
			{ 
				double *new_vertex_array, *write, read[3];
				new_vertex_array = node->content.geometry.vertex_stack.layers[i].data.double_data = realloc(node->content.geometry.vertex_stack.layers[i].data.double_data, (sizeof *new_vertex_array) * 3 * vertex_count);
				l = 0;
				for(j = 0; j < node->content.geometry.vertex_count; j++)
				{
					read[0] = new_vertex_array[j * 3];
					read[1] = new_vertex_array[j * 3 + 1];
					read[2] = new_vertex_array[j * 3 + 2];
					l++;
					for(k = 0; k < value_count; k++)
					{ 
						if(vertex_id_buffer[l] != 0xFFFFFFFF)
						{
							write = &new_vertex_array[vertex_id_buffer[l] * 3];
							write[0] = read[0] + direction[0] * (double)lengths[k];
							write[1] = read[1] + direction[1] * (double)lengths[k];
							write[2] = read[2] + direction[2] * (double)lengths[k];	
						}
						l++;
					}
				}
			}
		}
		else
		{
			hxa_uint8 *read, *write;
			size_t size;
			switch(node->content.geometry.vertex_stack.layers[i].type)
			{
				case HXA_LDT_UINT8 :
					size = sizeof(hxa_uint8) * node->content.geometry.vertex_stack.layers[i].components;
					read = node->content.geometry.vertex_stack.layers[i].data.uint8_data = realloc(node->content.geometry.vertex_stack.layers[i].data.uint8_data, size * vertex_count);

				break;
				case HXA_LDT_INT32 :
					size = sizeof(hxa_int32) * node->content.geometry.vertex_stack.layers[i].components;
					node->content.geometry.vertex_stack.layers[i].data.int32_data = realloc(node->content.geometry.vertex_stack.layers[i].data.int32_data, size * vertex_count);
					read = (hxa_uint8 *)node->content.geometry.vertex_stack.layers[i].data.int32_data;
				break;
				case HXA_LDT_FLOAT :
					size = sizeof(float) * node->content.geometry.vertex_stack.layers[i].components;
					node->content.geometry.vertex_stack.layers[i].data.float_data = realloc(node->content.geometry.vertex_stack.layers[i].data.float_data, size * vertex_count);
					read = (hxa_uint8 *)node->content.geometry.vertex_stack.layers[i].data.float_data;
				break;
				case HXA_LDT_DOUBLE :
					size = sizeof(double) * node->content.geometry.vertex_stack.layers[i].components;
					node->content.geometry.vertex_stack.layers[i].data.double_data = realloc(node->content.geometry.vertex_stack.layers[i].data.double_data, size * vertex_count);
					read = (hxa_uint8 *)node->content.geometry.vertex_stack.layers[i].data.double_data;
				break;
				default :
				return;
			}
			write = read;
			l = 0;
			for(j = 0; j < node->content.geometry.vertex_count; j++)
			{
				l++;
				for(k = 0; k < value_count; k++)
				{ 
					if(vertex_id_buffer[l] != 0xFFFFFFFF)
						memcpy(&write[vertex_id_buffer[l] * size], &read[j * size], size);
					l++;
				}
			}
		}
	}
}


void hxa_util_extrude_top(HXANode *node, hxa_int32 *poly_height, unsigned int value_count, unsigned int *vertex_id_buffer)
{	
	hxa_int32 *ref, r;
	unsigned int i, j, height; 
	ref = node->content.geometry.corner_stack.layers->data.int32_data;
	for(i = 0; i < node->content.geometry.edge_corner_count; i++)
	{
		r = ref[i];
		if(r < 0)
			ref[i] = -1 - vertex_id_buffer[(-1 - r) * value_count + poly_height[i]];
		else
			ref[i] = vertex_id_buffer[r * value_count + poly_height[i]];
	}
}


void r_primitive_line_3d(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, float red, float green, float blue, float alpha);

void hxa_util_extrude_edge_count(HXANode *node, int *vertex_list_a, int *vertex_list_b, unsigned int height, unsigned int other_hight, unsigned int *tri_count, unsigned int *quad_count)
{
	unsigned int  i;
	for(i = other_hight; i <= height; i++)
	{
		if(vertex_list_a[i] != ~0)
		{
			if(vertex_list_b[i] != ~0)
				(*quad_count)++;
			else
				(*tri_count)++;
		}else
			(*tri_count)++;
	}
}

typedef struct {
	unsigned int corners[2];
	unsigned int poly_id;
	unsigned int edge_id;
}HXAExtrudePolyMapping;

void hxa_util_extrude_edge_add_triangles(HXANode *node, int *vertex_list_a, int *vertex_list_b, unsigned int height, unsigned int other_hight, unsigned int polygon_id, unsigned int corner_a, unsigned int corner_b, int *ref, HXAExtrudePolyMapping *mapping, unsigned int *ref_count, unsigned int *mapping_count)
{
	int  i, a, b, ref_used = 0;
	a = vertex_list_a[other_hight];
	b = vertex_list_b[other_hight];
	for(i = other_hight + 1; i <= height; i++)
	{
		if(vertex_list_a[i] != ~0)
		{
			mapping->corners[0] = corner_a;
			mapping->corners[1] = corner_b;
			mapping->poly_id = polygon_id;
			mapping++;
			(*mapping_count)++;
			ref[ref_used++] = a;
			a = ref[ref_used++] = vertex_list_a[i];
			ref[ref_used++] = -1 - b;
			ref_used -= 3;
			if(ref[ref_used + 0] < 0 || ref[ref_used + 1] < 0 || ref[ref_used + 2] >= 0) 
				ref_used += 0;
			ref_used += 3;
		}
		if(vertex_list_b[i] != ~0)
		{
			mapping->corners[0] = corner_b;
			mapping->corners[1] = corner_a;
			mapping->poly_id = polygon_id;
			mapping++;
			(*mapping_count)++;
			ref[ref_used++] = vertex_list_b[i];
			ref[ref_used++] = b;
			b = vertex_list_b[i];
			ref[ref_used++] = -1 - a;
			ref_used -= 3;
			if(ref[ref_used + 0] < 0 || ref[ref_used + 1] < 0 || ref[ref_used + 2] >= 0) 
				ref_used += 0;
			ref_used += 3;
		}
	}
	*ref_count += ref_used;
}

void hxa_util_extrude_edge_add_polygons(HXANode *node, int *vertex_list_a, int *vertex_list_b, unsigned int height, unsigned int other_hight, unsigned int polygon_id, unsigned int corner_a, unsigned int corner_b, int *ref, HXAExtrudePolyMapping *mapping, unsigned int *ref_count, unsigned int *mapping_count)
{
	int i, a, b, ref_used = 0;
	a = vertex_list_a[other_hight];
	b = vertex_list_b[other_hight];
	for(i = other_hight + 1; i <= height; i++)
	{
		if(vertex_list_a[i] != ~0 && vertex_list_b[i] != ~0)
		{
			mapping->corners[0] = corner_a;
			mapping->corners[1] = corner_b;
			mapping->poly_id = polygon_id;
			mapping++;			
			(*mapping_count)++;
			ref[ref_used + 1] = a;
			ref[ref_used] = a = vertex_list_a[i];
			ref_used += 2;
			ref[ref_used++] = b;
			b = vertex_list_b[i];
			ref[ref_used++] = -1 - b;
		}else if(vertex_list_a[i] != ~0)
		{
			mapping->corners[0] = corner_a;
			mapping->corners[1] = corner_b;
			mapping->poly_id = polygon_id;
			mapping++;
			(*mapping_count)++;
			ref[ref_used++] = a;
			a = ref[ref_used++] = vertex_list_a[i];
			ref[ref_used++] = -1 - b;

		}else if(vertex_list_b[i] != ~0)
		{
			mapping->corners[0] = corner_b;
			mapping->corners[1] = corner_a;
			mapping->poly_id = polygon_id;
			mapping++;
			(*mapping_count)++;
			ref[ref_used++] = vertex_list_b[i];
			ref[ref_used++] = b;
			b = vertex_list_b[i];
			ref[ref_used++] = -1 - a;
		}
	}
	*ref_count += ref_used;
}


HXAExtrudePolyMapping *hxa_util_extrude_edges(HXANode *node, unsigned int *n, hxa_int32 *edge_height, int *vertex_id_buffer, hxa_int32 *values, unsigned int value_count, int triangles_only, unsigned int *refs_added, unsigned int *polys_added)
{
	HXAExtrudePolyMapping *mapping;
	hxa_int32 a, b, *ref;
	unsigned int i, j, poly_id, height, tri_count = 0, quad_count = 0, ref_position, mapping_position;
	ref = node->content.geometry.corner_stack.layers->data.int32_data;
	for(i = 0; i < node->content.geometry.edge_corner_count; i++)
	{
		height = 0;
		if(n[i] != ~0)	
			height = edge_height[n[i]];
		if(edge_height[i] > height)
		{
			a = ref[i];
			if(a < 0)
			{
				a = -1 - a;
				for(j = i - 1; j != 0 && ref[j] >= 0; j--)
					b = ref[j];
			}else
			{
				b = ref[i + 1];
				if(b < 0)
					b = -1 - b;
			}
			hxa_util_extrude_edge_count(node, &vertex_id_buffer[a * value_count], &vertex_id_buffer[b * value_count], edge_height[i], height, &tri_count, &quad_count);
		}
	}
	if(triangles_only)
	{
		tri_count += quad_count << 1;
		quad_count = 0;
	}
	mapping = malloc((sizeof *mapping) * (tri_count + quad_count));
	
	node->content.geometry.corner_stack.layers->data.int32_data = 
		realloc(node->content.geometry.corner_stack.layers->data.int32_data, 
			(sizeof *node->content.geometry.corner_stack.layers->data.int32_data) * 
			(node->content.geometry.edge_corner_count + tri_count * 3 + quad_count * 4));
	
	ref = node->content.geometry.corner_stack.layers->data.int32_data;
	ref_position = node->content.geometry.edge_corner_count;
	mapping_position = 0;
	for(i = poly_id = 0; i < node->content.geometry.edge_corner_count; i++)
	{
		height = 0;
		if(n[i] != ~0)	
		{
			height = edge_height[n[i]];
			if(edge_height[i] > height)
			{
				a = ref[i];
				if(a < 0)
				{
					a = -1 - a;
					for(j = i - 1; j != 0xFFFFFFFF && ref[j] >= 0; j--);
					b = ref[++j];
				}else
				{
					j = i + 1;
					b = ref[j];
					if(b < 0)
						b = -1 - b;
				}
				if(triangles_only)
					hxa_util_extrude_edge_add_triangles(node, &vertex_id_buffer[a * value_count], &vertex_id_buffer[b * value_count], 
														edge_height[i], height, poly_id, i, j,
														&node->content.geometry.corner_stack.layers->data.int32_data[ref_position],
														&mapping[mapping_position],
														&ref_position, &mapping_position);
				else
					hxa_util_extrude_edge_add_polygons(node, &vertex_id_buffer[a * value_count], &vertex_id_buffer[b * value_count], 
														edge_height[i], height, poly_id, i, j,
														&node->content.geometry.corner_stack.layers->data.int32_data[ref_position],
														&mapping[mapping_position],
														&ref_position, &mapping_position);

				if(n[i] != ~0)
				{
					n[n[i]] = ~0;
					n[i] = ~0;
				}
			}
		}
		if(ref[i] < 0)
			poly_id++;
	}
	*refs_added = ref_position - node->content.geometry.edge_corner_count;
	*polys_added = mapping_position;
	return mapping;
}

unsigned int *hxa_util_extrude_poly_heights(HXANode *node, hxa_int32 *selector, hxa_int32 *values, unsigned int value_count)
{
	hxa_int32 *ref;
	unsigned int i, height, *poly_height, *p; 
	p = poly_height = malloc((sizeof *poly_height) * node->content.geometry.edge_corner_count);
	ref = node->content.geometry.corner_stack.layers->data.int32_data;
	for(i = 0; i < node->content.geometry.face_count; i++)
	{
		for(height = 0; ; height++)
		{
			if(height == value_count)
			{
				height = 0;
				break;
			}
			if(values[height] == selector[i])
			{
				height++;
				break;
			}
		}
		while((*ref++) >= 0)
			*p++ = height;
		*p++ = height;
	}
	return poly_height;
}

HXALayer *hxa_util_extrude_extend_layers(HXANode *node, HXAExtrudePolyMapping *mapping, unsigned int added_reference, unsigned int added_polygons)
{
	unsigned int type_sizes[HXA_LDT_COUNT] = {sizeof(hxa_uint8), sizeof(hxa_int32), sizeof(float), sizeof(double)};
	unsigned int i, j, pos, size;
	HXALayer *layer = NULL;
	char *name = HXA_CONVENTION_HARD_EDGE_NEIGHBOUR_LAYER_NAME;
	hxa_int32 *ref, *n;
	unsigned char *data;
	for(i = 0; i < node->content.geometry.face_stack.layer_count; i++)
	{
		size = node->content.geometry.face_stack.layers[i].components * type_sizes[node->content.geometry.face_stack.layers[i].type];
		node->content.geometry.face_stack.layers[i].data.uint8_data = realloc(node->content.geometry.face_stack.layers[i].data.uint8_data,
																			  size * (node->content.geometry.face_count + added_polygons));
		data = node->content.geometry.face_stack.layers[i].data.uint8_data;
		for(j = 0; j < added_polygons; j++)
			memcpy(&data[(node->content.geometry.face_count + j) * size], &data[mapping[j].poly_id * size], size);
	}
	ref = node->content.geometry.corner_stack.layers[0].data.int32_data;
	for(i = 1; i < node->content.geometry.corner_stack.layer_count; i++)
	{
		size = node->content.geometry.corner_stack.layers[i].components * type_sizes[node->content.geometry.corner_stack.layers[i].type];
		node->content.geometry.corner_stack.layers[i].data.uint8_data = realloc(node->content.geometry.corner_stack.layers[i].data.uint8_data,
																			  size * (node->content.geometry.edge_corner_count + added_reference));
		data = node->content.geometry.corner_stack.layers[i].data.uint8_data;
		pos = node->content.geometry.edge_corner_count;
		for(j = 0; j < added_reference; j++)
		{
			memcpy(&data[pos++ * size], &data[mapping[j].corners[0] * size], size);
			memcpy(&data[pos++ * size], &data[mapping[j].corners[0] * size], size);		
			memcpy(&data[pos++ * size], &data[mapping[j].corners[1] * size], size);
			if(ref[pos] < 0)				
				memcpy(&data[pos++ * size], &data[mapping[j].corners[1] * size], size);
		}
	}
	for(i = 0; i < node->content.geometry.edge_stack.layer_count; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == node->content.geometry.edge_stack.layers[i].name[j]; j++);
		if(name[j] == node->content.geometry.edge_stack.layers[i].name[j])
		{
			layer = &node->content.geometry.edge_stack.layers[i];
			layer->data.int32_data = realloc(node->content.geometry.edge_stack.layers[i].data.int32_data,
																				  (sizeof *layer->data.int32_data) * 
																					(node->content.geometry.edge_corner_count + added_reference));
			memset(&layer->data.int32_data[node->content.geometry.edge_corner_count], 0xFF, added_reference * sizeof(hxa_int32));
		}else
		{
			size = node->content.geometry.edge_stack.layers[i].components * type_sizes[node->content.geometry.edge_stack.layers[i].type];
			node->content.geometry.edge_stack.layers[i].data.uint8_data = realloc(node->content.geometry.edge_stack.layers[i].data.uint8_data,
																				  size * (node->content.geometry.edge_corner_count + added_reference));
			data = node->content.geometry.edge_stack.layers[i].data.uint8_data;
			pos = node->content.geometry.edge_corner_count;
			for(j = 0; j < added_reference; j++)
			{
				memcpy(&data[pos++ * size], &data[mapping[j].corners[0] * size], size);
				memcpy(&data[pos++ * size], &data[mapping[j].corners[0] * size], size);		
				memcpy(&data[pos++ * size], &data[mapping[j].corners[0] * size], size);
				if(ref[pos] < 0)				
					memcpy(&data[pos++ * size], &data[mapping[j].corners[0] * size], size);
			}
		}
	}
	return layer;
}


void hxa_util_extrude(HXANode *node, char *layer_name, hxa_int32 *values, double *lengths, unsigned int value_count, double *direction, int tris_only)
{	
	HXAExtrudePolyMapping *mapping;
	hxa_int32 *selector;
	HXALayer *layer;
	unsigned int vertex_id_count, *n, *poly_height, poly_added, refs_added;
	int *vertex_id_buffer;

	if((layer = hxa_util_extrude_find_in_layer_stack(&node->content.geometry.face_stack, HXA_LDT_INT32, 1, layer_name)) == NULL)
		return;
	selector = layer->data.int32_data;

	poly_height = hxa_util_extrude_poly_heights(node, selector, values, value_count);
	vertex_id_buffer = hxa_util_extrude_vertex_ids(node, poly_height, value_count + 1, &vertex_id_count);

	if(vertex_id_count == node->content.geometry.vertex_count)
	{
		free(poly_height);
		free(vertex_id_buffer);
		return;
	}
	hxa_util_extrude_vertices(node, selector, values, lengths, value_count, direction, vertex_id_buffer, vertex_id_count);
	node->content.geometry.vertex_count = vertex_id_count;

	if((layer = hxa_util_extrude_find_in_layer_stack(&node->content.geometry.edge_stack, HXA_LDT_INT32, 1, HXA_CONVENTION_HARD_EDGE_NEIGHBOUR_LAYER_NAME)) == NULL)
	{
		n = hxa_neighbour_node(node);
	}else
		n = layer->data.int32_data;
	
	mapping = hxa_util_extrude_edges(node, n, poly_height, vertex_id_buffer, values, value_count + 1, tris_only, &refs_added, &poly_added);
	hxa_util_extrude_top(node, poly_height, value_count + 1, vertex_id_buffer);
	
	layer = hxa_util_extrude_extend_layers(node, mapping, refs_added, poly_added);
	node->content.geometry.face_count += poly_added;
	node->content.geometry.edge_corner_count += refs_added;
	hxa_neighbour_node_repair(node, layer->data.int32_data);
	free(mapping);

}
