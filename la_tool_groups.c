#include "la_includes.h"
#include "la_tool.h"
#include "la_geometry_undo.h"


boolean la_t_group_compute_outline_test(double *vertex, uint *ref, double *select, uint vertex_count)
{
	if(ref[0] < vertex_count && 
		ref[1] < vertex_count &&  
		ref[2] < vertex_count && 
		vertex[ref[0] * 3] != E_REAL_MAX && 
		vertex[ref[1] * 3] != E_REAL_MAX && 
		vertex[ref[2] * 3] != E_REAL_MAX &&
		select[ref[0]] > 0.01 &&
		select[ref[1]] > 0.01 &&
		select[ref[2]] > 0.01)
	{
		if(ref[3] < vertex_count && vertex[ref[3] * 3] != E_REAL_MAX && select[ref[3]] < 0.01)
			return FALSE;
		return TRUE;
	}
	return FALSE;
}

SeduceLineObject *la_t_group_compute_outline(float *select, uint vertex_count, float offset, double *center)
{
	SeduceLineObject *object = NULL;
	uint32 i, j, poly, polygon_count, *ref, *neighbor;
	double *vertex, size, n[3], sum = 0;
	double *normal;
	center[0] = 0;
	center[1] = 0;
	center[2] = 0;
	udg_get_geometry(&i, &polygon_count, &vertex, &ref, NULL);
	if(vertex_count == 0 || i == 0)
		return NULL;
	neighbor = la_compute_neighbor(ref, polygon_count, i, vertex);
	if(i < vertex_count)
		vertex_count = i;

	normal = malloc((sizeof *normal) * vertex_count * 3);
	for(i = 0; i < vertex_count * 3; i++)
		normal[i] = 0;
	for(i = 0; i < polygon_count * 4 ; i += 4)
	{
		if(la_t_group_compute_outline_test(vertex, &ref[i], select, vertex_count))
		{
			poly = 3;
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				poly = 4;

			f_normal3d(n, &vertex[ref[i] * 3], &vertex[ref[i + 2] * 3], &vertex[ref[i + 1] * 3]);
			for(j = 0; j < poly; j++)
			{
				normal[ref[i + j] * 3] += n[0];
				normal[ref[i + j] * 3 + 1] += n[1];
				normal[ref[i + j] * 3 + 2] += n[2];
			}
		}
	}


	for(i = 0; i < polygon_count * 4 ; i += 4)
	{
		if(la_t_group_compute_outline_test(vertex, &ref[i], select, vertex_count))
		{
			poly = 3;
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				poly = 4;
			for(j = 0; j < poly; j++)
			{
				center[0] += vertex[ref[i + j] * 3];
				center[1] += vertex[ref[i + j] * 3 + 1];
				center[2] += vertex[ref[i + j] * 3 + 2];
				sum++;
				if(neighbor[i + j] >= polygon_count * 4 || !la_t_group_compute_outline_test(vertex, &ref[(neighbor[i + j] / 4) * 4], select, vertex_count))
				{
					if(object == NULL)
						object = seduce_primitive_line_object_allocate();
						
					f_normalize3d(&normal[ref[i + j] * 3]);
					f_normalize3d(&normal[ref[i + (j + 1) % poly] * 3]);

					seduce_primitive_line_add_3d(object,
							vertex[ref[i + j] * 3] + normal[ref[i + j] * 3] * offset, 
							vertex[ref[i + j] * 3 + 1] + normal[ref[i + j] * 3 + 1] * offset, 
							vertex[ref[i + j] * 3 + 2] + normal[ref[i + j] * 3 + 2] * offset,
							vertex[ref[i + (j + 1) % poly] * 3] + normal[ref[i + (j + 1) % poly] * 3] * offset, 
							vertex[ref[i + (j + 1) % poly] * 3 + 1] + normal[ref[i + (j + 1) % poly] * 3 + 1] * offset, 
							vertex[ref[i + (j + 1) % poly] * 3 + 2] + normal[ref[i + (j + 1) % poly] * 3 + 2] * offset,
							1, 1, 1, 0, 
							1, 1, 1, 0);
				}
			}
		}
	}
	center[0] /= sum; 
	center[1] /= sum; 
	center[2] /= sum;
/*	seduce_primitive_circle_add_3d(NULL,
							center[0], 
							center[1], 
							center[2], 
							0, 0, 1,
							0, 1, 0,
							0.1,
							0, 1,
							0, 1,
							la_tools_group_list[i]->timer,
							la_tools_group_list[i]->timer,
							la_tools_group_list[i]->timer,
							la_tools_group_list[i]->timer,
							la_tools_group_list[i]->timer,
							la_tools_group_list[i]->timer,
							la_tools_group_list[i]->timer,
							la_tools_group_list[i]->timer);*/

	free(neighbor);
	free(normal);
	return object;
}

typedef struct{
	SeduceLineObject *object;
	double *select;
	double center[3];
	float timer;
	uint vertex_count;
	uint checksum;
	uint version;
}LATGroup;

LATGroup **la_tools_group_list = NULL;
uint la_tools_group_list_size = 0;


void la_t_group_draw(BInputState *input)
{
	uint i;
	float camera[3], size, color[3] = {0.2, 0.7, 0.6}, c1[3] = {0.8, 1.0, 0.0}, c2[3] = {0.2, 1.0, 0.0}, c3[3] = {0.0, 1.0, 1.0}, c4[3] = {0.0, 0.5, 0.7};
	double tmp[3];
	seduce_view_camera_vector_getf(NULL, camera, 0, 0);
	size = 0.005 * seduce_view_distance_camera_get(NULL);
	for(i = 0; i < la_tools_group_list_size; i++)
	{
		if(la_tools_group_list[i] != NULL && la_tools_group_list[i]->timer > 0.01)
		{
			seduce_view_projection_screend(NULL, tmp, la_tools_group_list[i]->center[0], la_tools_group_list[i]->center[1], la_tools_group_list[i]->center[2]);
			tmp[0] -= input->pointers[0].pointer_x;
			tmp[1] -= input->pointers[0].pointer_y;
			f_spline3df(color, (float)i / (float)la_tools_group_list_size, c1, c2, c3, c4);
			if(tmp[0] * tmp[0] + tmp[1] * tmp[1] < (size / tmp[2]) * (size / tmp[2]))
			{
				color[0] *= 2.0;
				color[1] *= 2.0;
				color[2] *= 2.0;
			}
			seduce_primitive_circle_add_3d(la_tools_group_list[i]->object,
							la_tools_group_list[i]->center[0], 
							la_tools_group_list[i]->center[1], 
							la_tools_group_list[i]->center[2], 
							0, 0, 1,
							camera[0], camera[1], camera[2],
							size,
							0, 1,
							0, 1,
							1, 1, 1, 0, 
							1, 1, 1, 0);
			seduce_primitive_line_draw(la_tools_group_list[i]->object, color[0], color[1], color[2], 0.0);
		}
	}
}

void la_t_group_free(LATGroup *group)
{	
	seduce_primitive_line_object_free(group->object);
	free(group->select);
	free(group);
}

void la_t_group_store()
{
	SeduceLineObject *object;
	double *select, center[3];
	uint vertex_count = 0, i, count, version, checksum;
	count =  imagine_setting_integer_get("GROUP_GOUNT", 12, NULL);
	if(la_tools_group_list_size != count)
	{
		for(i = count; i < la_tools_group_list_size; i++)
			la_t_group_free(la_tools_group_list[i]);
		la_tools_group_list = realloc(la_tools_group_list, (sizeof *la_tools_group_list) * count);
		for(i = la_tools_group_list_size; i < count; i++)
			la_tools_group_list[i] = NULL;
		la_tools_group_list_size = count;
	}
	if(la_tools_group_list_size == 0)
		return;
			

	udg_get_geometry(&vertex_count, NULL, NULL, NULL, NULL);
	if(vertex_count == 0)
		return;
	select = malloc((sizeof *select) * vertex_count);
	checksum = 0;
	for(i = 0; i < vertex_count; i++)
	{
		select[i] = udg_get_select(i);
		if(select[i] > 0.01)
			checksum ^= f_randi(i);
	}
	for(i = 0; i < la_tools_group_list_size; i++)
	{
		if(la_tools_group_list[i] != NULL && la_tools_group_list[i]->checksum == checksum)
		{
			free(select);
			return;
		}
	}
	object = la_t_group_compute_outline(select, vertex_count, 0.01 * seduce_view_distance_camera_get(NULL), center);
	if(object != NULL)
	{
		if(la_tools_group_list[la_tools_group_list_size - 1] != NULL)
			la_t_group_free(la_tools_group_list[la_tools_group_list_size - 1]);
		for(i = la_tools_group_list_size - 1; i != 0; i--)
			la_tools_group_list[i] = la_tools_group_list[i - 1];
		la_tools_group_list[0] = malloc(sizeof *la_tools_group_list[0]);
		la_tools_group_list[0]->object = object;
		la_tools_group_list[0]->select = select;
		la_tools_group_list[0]->center[0] = center[0];
		la_tools_group_list[0]->center[1] = center[1];
		la_tools_group_list[0]->center[2] = center[2];
		la_tools_group_list[0]->vertex_count = vertex_count;
		la_tools_group_list[0]->checksum = checksum;
		la_tools_group_list[0]->version = udg_get_version(TRUE, TRUE, FALSE, TRUE, TRUE);
		la_tools_group_list[0]->timer = 0;
	}
}

void la_t_group_update(BInputState *input, boolean active)
{
	SeduceLineObject *object = NULL;
	double select;
	uint i, version, vertex_count;
	version = udg_get_version(TRUE, TRUE, FALSE, TRUE, TRUE);
	for(i = 0; i < la_tools_group_list_size; i++)
		if(la_tools_group_list[i] != NULL)
			seduce_animate(input, &la_tools_group_list[i]->timer, !active && la_tools_group_list[i]->version == version, 2.0);
	for(i = 0; i < la_tools_group_list_size; i++)
	{
		if(la_tools_group_list[i] != NULL && la_tools_group_list[i]->version != version && la_tools_group_list[i]->timer < 0.01)
		{
			object = la_t_group_compute_outline(la_tools_group_list[i]->select, la_tools_group_list[i]->vertex_count, 0.01 * seduce_view_distance_camera_get(NULL), la_tools_group_list[i]->center);
			if(object != NULL)
			{
				seduce_primitive_line_object_free(la_tools_group_list[i]->object);
				la_tools_group_list[i]->object = object;
				la_tools_group_list[i]->version = version;
			}else
			{

				la_t_group_free(la_tools_group_list[i]);
				la_tools_group_list[i] = NULL;
			}
			return;
		}
	}
}

uint la_t_group_hit_test(BInputState *input)
{	
	double tmp[3], select, size;
	uint i, j, vertex_count;
	size = 0.005 * seduce_view_distance_camera_get(NULL);
	for(i = 0; i < la_tools_group_list_size; i++)
	{
		if(la_tools_group_list[i] != NULL)
		{
			seduce_view_projection_screend(NULL, tmp, la_tools_group_list[i]->center[0], la_tools_group_list[i]->center[1], la_tools_group_list[i]->center[2]);
		
			tmp[0] -= input->pointers[0].pointer_x;
			tmp[1] -= input->pointers[0].pointer_y;
			if(tmp[0] * tmp[0] + tmp[1] * tmp[1] < (size / tmp[2]) * (size / tmp[2]))
				return i;
		}
	}
	return -1;
}					
boolean la_t_group_interact(BInputState *input)
{	
	double tmp[3], select, size;
	uint i, j, vertex_count;
	size = 0.005 * seduce_view_distance_camera_get(NULL);
	if(input->pointers[0].button[0] && !input->pointers[0].last_button[0])
	{
		i = la_t_group_hit_test(input);
		if(i != -1)
		{
			udg_get_geometry(&vertex_count, NULL, NULL, NULL, NULL);
			for(j = 0; j < la_tools_group_list[i]->vertex_count && j < vertex_count; j++)
			{
				select = udg_get_select(i);
				if(select + 0.01 > la_tools_group_list[i]->select[j] ||
					select - 0.01 < la_tools_group_list[i]->select[j])
					udg_set_select(j, la_tools_group_list[i]->select[j]);
			}
			la_t_tm_place(la_tools_group_list[i]->center[0], la_tools_group_list[i]->center[1], la_tools_group_list[i]->center[2]);
			return TRUE;
		}
	}
	if(input->pointers[0].button[1] && !input->pointers[0].last_button[1])
	{
		i = la_t_group_hit_test(input);
		if(i != -1)
		{
			seduce_view_center_set(NULL, la_tools_group_list[i]->center[0], la_tools_group_list[i]->center[1], la_tools_group_list[i]->center[2]);
			return TRUE;
		}
	}
	return FALSE;
}