#include <stdlib.h>
#include <string.h>
#include "hxa.h"

#define TRUE 1
#define FALSE 0
#define HXA_RAY_CAST_EPSILON 0.000001

typedef struct{
	float min[3];
	float max[3];
	unsigned int children[2];
}HXABoundingVolume;

extern void r_primitive_line_3d(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, float red, float green, float blue, float alpha);

void hxa_bvh_draw(HXABoundingVolume *v)
{
	r_primitive_line_3d(v->min[0], v->min[1], v->min[2], v->min[0], v->min[1], v->max[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->max[0], v->min[1], v->min[2], v->max[0], v->min[1], v->max[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->max[0], v->max[1], v->min[2], v->max[0], v->max[1], v->max[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->min[0], v->max[1], v->min[2], v->min[0], v->max[1], v->max[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->min[0], v->min[1], v->min[2], v->min[0], v->max[1], v->min[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->max[0], v->min[1], v->min[2], v->max[0], v->max[1], v->min[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->max[0], v->min[1], v->max[2], v->max[0], v->max[1], v->max[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->min[0], v->min[1], v->max[2], v->min[0], v->max[1], v->max[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->min[0], v->min[1], v->min[2], v->max[0], v->min[1], v->min[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->min[0], v->max[1], v->min[2], v->max[0], v->max[1], v->min[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->min[0], v->max[1], v->max[2], v->max[0], v->max[1], v->max[2], 0.3, 0.3, 0.3, 0.3);
	r_primitive_line_3d(v->min[0], v->min[1], v->max[2], v->max[0], v->min[1], v->max[2], 0.3, 0.3, 0.3, 0.3);
}

void hxa_bvh_prep(HXABoundingVolume *poly_volumes, float *vertex, unsigned int *ref, unsigned int count)
{
	HXABoundingVolume *v;
	unsigned int i, next, r;
	v = poly_volumes;
	for(i = next = 0; i < count; i++)
	{
		r = *ref++ * 3;
		v->min[0] = v->max[0] = vertex[r++];
		v->min[1] = v->max[1] = vertex[r++];
		v->min[2] = v->max[2] = vertex[r];
		r = *ref++ * 3;
		if(v->max[0] < vertex[r])
			v->max[0] = vertex[r];
		if(v->min[0] > vertex[r])
			v->min[0] = vertex[r];
		r++;
		if(v->max[1] < vertex[r])
			v->max[1] = vertex[r];
		if(v->min[1] > vertex[r])
			v->min[1] = vertex[r];
		r++;
		if(v->max[2] < vertex[r])
			v->max[2] = vertex[r];
		if(v->min[2] > vertex[r])
			v->min[2] = vertex[r];

		r = (-1 - *ref++) * 3;
		if(v->max[0] < vertex[r])
			v->max[0] = vertex[r];
		if(v->min[0] > vertex[r])
			v->min[0] = vertex[r];
		r++;
		if(v->max[1] < vertex[r])
			v->max[1] = vertex[r];
		if(v->min[1] > vertex[r])
			v->min[1] = vertex[r];
		r++;
		if(v->max[2] < vertex[r])
			v->max[2] = vertex[r];
		if(v->min[2] > vertex[r])
			v->min[2] = vertex[r];			
		v->children[0] = ++next;
		v->children[1] = 0xFFFFFFFF;
		v++;
	}
	v--;
	v->children[0] = 0xFFFFFFFF;
}

void hxa_bvh_expand(HXABoundingVolume *volume, HXABoundingVolume *add, HXABoundingVolume *center)
{
	float f;
	if(volume->min[0] > add->min[0])
		volume->min[0] = add->min[0];
	if(volume->min[1] > add->min[1])
		volume->min[1] = add->min[1];
	if(volume->min[2] > add->min[2])
		volume->min[2] = add->min[2];
	if(volume->max[0] < add->max[0])
		volume->max[0] = add->max[0];
	if(volume->max[1] < add->max[1])
		volume->max[1] = add->max[1];
	if(volume->max[2] < add->max[2])
		volume->max[2] = add->max[2];
	f = add->min[0] + add->max[0];
	if(f > center->max[0])
		center->max[0] = f;
	if(f < center->min[0])
		center->min[0] = f;
	f = add->min[1] + add->max[1];
	if(f > center->max[1])
		center->max[1] = f;
	if(f < center->min[1])
		center->min[1] = f;
	f = add->min[2] + add->max[2];
	if(f > center->max[2])
		center->max[2] = f;
	if(f < center->min[2])
		center->min[2] = f;


}

float hxa_bvh_size(HXABoundingVolume *volume)
{
	return (volume->max[0] - volume->min[0]) * (volume->max[1] - volume->min[1]) * (volume->max[2] - volume->min[2]);
}

unsigned int hxa_bvh_axis(HXABoundingVolume *volume)
{
	unsigned int axis = 0;
	float f, f2;
	f = volume->max[0] - volume->min[0];
	f2 = volume->max[1] - volume->min[1];
	if(f2 > f)
	{
		f = f2;
		axis = 1;
	}
	f2 = volume->max[2] - volume->min[2];
	if(f2 > f)
		axis = 2;
	return axis;
}

void hxa_bvh_split_compute(HXABoundingVolume *poly_volume, unsigned int list_start, unsigned int list_length, HXABoundingVolume *parent, HXABoundingVolume *volume_buffer, unsigned int *volume_buffer_used)
{
	HXABoundingVolume *poly, center_volume;
	unsigned int id, next, axis, counts[2], handles[2];
	float split, x, y, z;

	
	parent->min[0] = parent->min[1] = parent->min[2] = 10000000000;
	parent->max[0] = parent->max[1] = parent->max[2] = -10000000000;
	center_volume.min[0] = center_volume.min[1] = center_volume.min[2] = 10000000000;
	center_volume.max[0] = center_volume.max[1] = center_volume.max[2] = -10000000000;
	parent->children[0] = parent->children[1] = 0;
	for(id = list_start; id != 0xFFFFFFFF; id = poly->children[0])
	{
		poly = &poly_volume[id];
		hxa_bvh_expand(parent, poly, &center_volume);
	}
	if(list_length <= 2)
	{
		parent->children[0] = list_start | 0x80000000;
		if(list_length == 2)
			parent->children[1] = poly_volume[list_start].children[0] | 0x80000000;
		else
			parent->children[1] = 0xFFFFFFFF;
		return;
	}
	axis = hxa_bvh_axis(&center_volume);
	split = (center_volume.max[axis] + center_volume.min[axis]) * 0.5;
	parent->children[0] = (*volume_buffer_used)++;
	parent->children[1] = (*volume_buffer_used)++;
	counts[0] = 0;
	counts[1] = 0;
	handles[0] = 0xFFFFFFFF;
	handles[1] = 0xFFFFFFFF;
	while(counts[0] == 0 || counts[1] == 0)
	{
		counts[0] = counts[1] = 0;
		for(id = list_start; id != 0xFFFFFFFF; id = next)
		{		
			poly = &poly_volume[id];
			next = poly->children[0];
			if(split > poly->max[axis] + poly->min[axis])
			{
				poly->children[0] = handles[0];
				handles[0] = id;
				counts[0]++;
			}else
			{
				poly->children[0] = handles[1];
				handles[1] = id;
				counts[1]++;
			}
		}
		if(counts[0] == 0 || counts[1] == 0)
			axis += 0;
		axis = (axis + 1) % 3;

	}
	hxa_bvh_split_compute(poly_volume, handles[0], counts[0], &volume_buffer[parent->children[0]], volume_buffer, volume_buffer_used);
	hxa_bvh_split_compute(poly_volume, handles[1], counts[1], &volume_buffer[parent->children[1]], volume_buffer, volume_buffer_used);
}


HXABoundingVolume *hxa_volume_compute(HXANode *node)
{
	HXABoundingVolume *poly_volumes, *buffer;
	unsigned int used = 1;
	poly_volumes = malloc((sizeof *poly_volumes) * node->content.geometry.face_count);	
	hxa_bvh_prep(poly_volumes, 
					node->content.geometry.vertex_stack.layers[0].data.float_data, 
					node->content.geometry.corner_stack.layers[0].data.int32_data, 
					node->content.geometry.face_count);
	buffer = malloc((sizeof *buffer) * node->content.geometry.face_count * node->content.geometry.face_count / 2);	
	hxa_bvh_split_compute(poly_volumes, 0, node->content.geometry.face_count, buffer, buffer, &used);
/*	{
		int i;
		for(i = 0; i < used; i++)
			hxa_bvh_draw(&buffer[i]);
	}*/
	free(poly_volumes);
	buffer = realloc(buffer, (sizeof *buffer) * used);
	return buffer;
}


int hxa_bvh_intersection(float start[3], float in_vector[3], float inv_vector[3], float length, float box_min[3], float box_max[3]) 
{
	float center[3], extant[3], mid[3], vector[3], ad[3], f;
	unsigned int i;
	center[0] = (box_min[0] + box_max[0]) * 0.5;
	center[1] = (box_min[1] + box_max[1]) * 0.5;
	center[2] = (box_min[2] + box_max[2]) * 0.5;
	extant[0] = box_max[0] - center[0];
	extant[1] = box_max[1] - center[1];
	extant[2] = box_max[2] - center[2];
/*	mid[0] = (p0[0] + start[0]) * 0.5;
	mid[1] = (p0[1] + start[1]) * 0.5;
	mid[2] = (p0[2] + start[2]) * 0.5;*/
	length *= 0.5;
	mid[0] = in_vector[0] * length + start[0];
	mid[1] = in_vector[1] * length + start[1];
	mid[2] = in_vector[2] * length + start[2];
	vector[0] = start[0] - mid[0];
	vector[1] = start[1] - mid[1];
	vector[2] = start[2] - mid[2];
	mid[0] -= center[0];
	mid[1] -= center[1];
	mid[2] -= center[2];
	for(i = 0; i < 3; i++)
	{
		if(vector[i] < 0)
			ad[i] = -vector[i];
		else
			ad[i] = vector[i];
		if(mid[i] < 0)
		{
			if(-mid[i] > extant[i] + ad[i])
				return FALSE;	
		}else
		{
			if(mid[i] > extant[i] + ad[i])
				return FALSE;	
		}
	}
	ad[0] += HXA_RAY_CAST_EPSILON;
	ad[1] += HXA_RAY_CAST_EPSILON;
	ad[2] += HXA_RAY_CAST_EPSILON;
	f = mid[1] * vector[2] - mid[2] * vector[1];
	if(f < 0)
		f = -f;
	if(f > extant[1] * ad[2] + extant[2] * ad[1])
		return FALSE;
	f = mid[2] * vector[0] - mid[0] * vector[2];
	if(f < 0)
		f = -f;
	if(f > extant[0] * ad[2] + extant[2] * ad[0])
		return FALSE;
	f = mid[0] * vector[1] - mid[1] * vector[0];
	if(f < 0)
		f = -f;
	if(f > extant[0] * ad[1] + extant[1] * ad[0])
		return FALSE;
	return TRUE;
}


int hxa_raycast_intersect_triangle(float orig[3], float dir[3], float vert0[3], float vert1[3], float vert2[3], float *out_uv_depth)
{
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det, inv_det;

	/*
			r_primitive_line_3d(vert0[0], 
								vert0[1], 
								vert0[2], 
								vert1[0], 
								vert1[1], 
								vert1[2], 1, 0, 0, 1);
			r_primitive_line_3d(vert1[0], 
								vert1[1], 
								vert1[2], 
								vert2[0], 
								vert2[1], 
								vert2[2], 1, 0, 0, 1);
			r_primitive_line_3d(vert2[0], 
								vert2[1], 
								vert2[2], 
								vert0[0], 
								vert0[1], 
								vert0[2], 1, 0, 0, 1);
	*/

	edge1[0] = vert1[0] - vert0[0];
	edge1[1] = vert1[1] - vert0[1];
	edge1[2] = vert1[2] - vert0[2]; 
	edge2[0] = vert2[0] - vert0[0];
	edge2[1] = vert2[1] - vert0[1];
	edge2[2] = vert2[2] - vert0[2];
	pvec[0] = dir[1] * edge2[2] - dir[2] * edge2[1];
	pvec[1] = dir[2] * edge2[0] - dir[0] * edge2[2];
	pvec[2] = dir[0] * edge2[1] - dir[1] * edge2[0];
	det = edge1[0] * pvec[0] + edge1[1] * pvec[1] + edge1[2] * pvec[2];

	if(det > -HXA_RAY_CAST_EPSILON && det < HXA_RAY_CAST_EPSILON)
		return FALSE;
	inv_det = 1.0 / det;

	tvec[0] = orig[0] - vert0[0];
	tvec[1] = orig[1] - vert0[1];
	tvec[2] = orig[2] - vert0[2];

	out_uv_depth[0] = (tvec[0] * pvec[0] + tvec[1] * pvec[1] + tvec[2] * pvec[2]) * inv_det;
	if(out_uv_depth[0] < 0.0 || out_uv_depth[0] > 1.0)
		return FALSE;

	qvec[0] = tvec[1] * edge1[2] - tvec[2] * edge1[1];
	qvec[1] = tvec[2] * edge1[0] - tvec[0] * edge1[2];
	qvec[2] = tvec[0] * edge1[1] - tvec[1] * edge1[0];

	out_uv_depth[1] = (dir[0] * qvec[0] + dir[1] * qvec[1] + dir[2] * qvec[2]) * inv_det;
	if(out_uv_depth[1] < 0.0 || out_uv_depth[0] + out_uv_depth[1] > 1.0)
		return FALSE;
	out_uv_depth[2] = (edge2[0] * qvec[0] + edge2[1] * qvec[1] + edge2[2] * qvec[2]) * inv_det;
	return TRUE;
}

void hax_volume_traverse(HXANode *node, HXABoundingVolume *volume_array, unsigned int element, float start[3], float camera_vec[3], float inv_vector[3], float *found, float *out_uv_depth)
{
	HXABoundingVolume *volume;
	float *vertex_array, uv_depth[3];
	unsigned int i, polygon;
	hxa_int32 *ref;
	volume = &volume_array[element];
	if(hxa_bvh_intersection(start, camera_vec, inv_vector, out_uv_depth[2], volume->min, volume->max))
	{
	//	hxa_bvh_draw(volume);
		for(i = 0; i < 2; i++)
		{
			if(volume->children[i] < 0x80000000)
				hax_volume_traverse(node, volume_array, volume->children[i], start, camera_vec, inv_vector,  found, out_uv_depth);	
			else if(volume->children[i] != 0xFFFFFFFF)
			{
				polygon = volume->children[i] & ~0x80000000;
				vertex_array = node->content.geometry.vertex_stack.layers[0].data.float_data;
				ref = &node->content.geometry.corner_stack.layers[0].data.int32_data[polygon * 3];
				if(hxa_raycast_intersect_triangle(start, camera_vec, 
											   &vertex_array[ref[0] * 3], 
											   &vertex_array[ref[1] * 3], 
											   &vertex_array[(- 1 - ref[2]) * 3], uv_depth) && out_uv_depth[2] > uv_depth[2] && uv_depth[2] > 0)
				{
					memcpy(out_uv_depth, uv_depth, (sizeof *out_uv_depth) * 3);
					*found = polygon;
				}
			}
		}			
	}		
}


unsigned int hxa_raycast_slow(HXANode *node, HXABoundingVolume *volume, float start[3], float normalized_vector[3], float length, float *out_uv_depth)
{
	unsigned int polygon, found = ~0;
	int *ref;
	float inv_vector[3], uv_depth[3], *vertex_array;
	inv_vector[0] = 1.0 / normalized_vector[0];
	inv_vector[1] = 1.0 / normalized_vector[1];
	inv_vector[2] = 1.0 / normalized_vector[2];	
	vertex_array = node->content.geometry.vertex_stack.layers[0].data.float_data;
	for(polygon = 0; polygon < node->content.geometry.face_count; polygon++)
	{
		ref = &node->content.geometry.corner_stack.layers[0].data.int32_data[polygon * 3];
		if(hxa_raycast_intersect_triangle(start, normalized_vector, 
										&vertex_array[ref[0] * 3], 
										&vertex_array[ref[1] * 3], 
										&vertex_array[(- 1 - ref[2]) * 3], uv_depth))
			
		{
			if(length > uv_depth[2] && uv_depth[2] > 0)
			{
				length = uv_depth[2];
				memcpy(out_uv_depth, uv_depth, (sizeof *out_uv_depth) * 3);
				found = polygon;
			}
		}
	}
	return found;
}
unsigned int hxa_raycast(HXANode *node, HXABoundingVolume *volume, float start[3], float normalized_vector[3], float length, float *out_uv_depth)
{
	unsigned int found = ~0;
	float inv_vector[3];
	inv_vector[0] = 1.0 / normalized_vector[0];
	inv_vector[1] = 1.0 / normalized_vector[1];
	inv_vector[2] = 1.0 / normalized_vector[2];	
	out_uv_depth[2] = length;
	hax_volume_traverse(node, volume, 0, start, normalized_vector, inv_vector, &found, out_uv_depth);
	return found;
}

void hxa_volume_free(HXABoundingVolume *volume)
{
	free(volume);
}