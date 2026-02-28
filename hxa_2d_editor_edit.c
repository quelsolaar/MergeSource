#include <math.h>
#include <stdlib.h>
#include <stdio.h> 
#include "betray.h"
#include "imagine.h"
#include "seduce.h"
#include "s_draw_3d.h"
#include "hxa.h"
#include "hxa_utils.h"
#include "hxa_2d_editor_internal.h"

HxA2DEditorShape mo_level_edit;

RMatrix *io_camera_matrix = NULL;

#define MO_MENU_EDIT_COS_30_DEGREES 0.86602540378443864676372317075294

void mo_menu_edit_unselect(HxA2DEditorInstance *level)
{
	uint i, j;
	for(i = 0; i < level->loop_count; i++)
		for(j = 0; j < level->loops[i].loop_size; j++)
			level->loops[i].selection[j] = FALSE;	
	for(i = 0; i < level->entity_count; i++)
		level->entity[i].selected = FALSE;
}



boolean mo_line_segment_intersect_test(double *a, double *a2, double *b, double *b2)
{
	if(((a[1] - a2[1]) * (b[0] - a2[0]) - (a[0] - a2[0]) * (b[1] - a2[1]) > 0) !=
		((a[1] - a2[1]) * (b2[0] - a2[0]) - (a[0] - a2[0]) * (b2[1] - a2[1]) > 0))
		if(((b[1] - b2[1]) * (a[0] - b2[0]) - (b[0] - b2[0]) * (a[1] - b2[1]) > 0) !=
			((b[1] - b2[1]) * (a2[0] - b2[0]) - (b[0] - b2[0]) * (a2[1] - b2[1]) > 0))
			return TRUE;
	return FALSE;
}


boolean mo_line_segment_colission_test(double *a, double *a2, double *pos)
{
	double vec[2], f;
	vec[0] = a[0] - a2[0];
	vec[1] = a[1] - a2[1];
	if(vec[0] * (pos[0] - a[0]) + vec[1] * (pos[1] - a[1]) < 0 &&
		vec[0] * (pos[0] - a2[0]) + vec[1] * (pos[1] - a2[1]) > 0)
	{
		f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		vec[0] /= f;
		vec[1] /= f;
		
		f = (pos[0] - a2[0]) * vec[1] - (pos[1] - a2[1]) * vec[0];
		if(f < HXA_2D_EDITOR_MIN_EDGE_SIZE && f > -HXA_2D_EDITOR_MIN_EDGE_SIZE)
		{
			return TRUE;	
	//	f = (pos[0] - a[0]) * vec[0] + (pos[0] - a[0]) * vec[0];
		}
	}
	return FALSE;
}

boolean mo_polygon_backface_test(double *v_a, double *v_b, double *v_c)
{
	double vec[2];
	vec[0] = v_a[0] - v_b[0];
	vec[1] = v_a[1] - v_b[1];
	if(vec[1] * (v_c[0] - v_a[0]) - vec[0] * (v_c[1] - v_a[1]) < MO_EDITOR_SNAP)
		return TRUE;
	return FALSE;
}

boolean mo_polygon_colission_test(double *v_a, double *v_b, double *v_c, double *pos)
{
	double vec[2];
	vec[0] = v_a[0] - v_b[0];
	vec[1] = v_a[1] - v_b[1];
	f_normalize2d(vec);
	if(vec[1] * (pos[0] - v_a[0]) - vec[0] * (pos[1] - v_a[1]) < MO_EDITOR_SNAP)
		return TRUE;
	vec[0] = v_b[0] - v_c[0];
	vec[1] = v_b[1] - v_c[1];
	f_normalize2d(vec);
	if(vec[1] * (pos[0] - v_b[0]) - vec[0] * (pos[1] - v_b[1]) < MO_EDITOR_SNAP)
		return TRUE;
	vec[0] = v_c[0] - v_a[0];
	vec[1] = v_c[1] - v_a[1];
	f_normalize2d(vec);
	if(vec[1] * (pos[0] - v_c[0]) - vec[0] * (pos[1] - v_c[1]) < MO_EDITOR_SNAP)
		return TRUE;
	return FALSE;
}

void o_menu_edit_loop_polygonize_colission_func(uint id, double *a, double *b, double *c, void *user_a, void *user_b)
{
	float af[2], bf[2], cf[2];
	af[0] = (float)a[0];
	af[1] = (float)a[1];
	bf[0] = (float)b[0];
	bf[1] = (float)b[1];
	cf[0] = (float)c[0];
	cf[1] = (float)c[1];
	seduce_element_add_triangle(user_a, user_b, 0, af, bf, cf);
}
/*
	x = loop->matrix[0] * loop->loop[i * 2 + 0] + loop->matrix[8] * loop->loop[i * 2 + 1] + loop->matrix[12] - center[0];
	y = loop->matrix[2] * loop->loop[i * 2 + 0] + loop->matrix[10] * loop->loop[i * 2 + 1] + loop->matrix[14] - center[1];
*/
boolean mo_menu_edit_loop_colission_test(double *loop, uint loop_count, double *matrix, double *pos)
{
	double a[2], b[2];
	uint i, ii, count = 0;

	a[0] = matrix[0] * loop[0 * 2 + 0] + matrix[8] * loop[0 * 2 + 1] + matrix[12];
	for(i = 0; i < loop_count; i++)
	{
		ii = (i + 1) % loop_count;
		b[0] = matrix[0] * loop[ii * 2 + 0] + matrix[8] * loop[ii * 2 + 1] + matrix[12]; 
		if((a[0] > pos[0]) != (b[0] > pos[0]))
		{
			a[1] = matrix[2] * loop[i * 2 + 0] + matrix[10] * loop[i * 2 + 1] + matrix[14]; 
			b[1] = matrix[2] * loop[ii * 2 + 0] + matrix[10] * loop[ii * 2 + 1] + matrix[14]; 

			if((a[0] > pos[0]) != ((a[1] - b[1]) * (pos[0] - b[0]) + (b[0] - a[0]) * (pos[1] - b[1]) > 0))
				count++;
		}
		a[0] = b[0];
	}
	return count % 2;
}
/*
typedef struct{
	float *loop;
	uint loop_size;
	uint loop_allocated;
	MOLevelType	type;
	float matrix[16];
	void *pool;
}HxA2DEditorLoop;*/


void o_menu_edit_loop_polygonize_draw_func(uint id, double *a, double *b, double *c, void *user_a, void *user_b)
{
	r_primitive_line_3d(a[0] * 0.98 + b[0] * 0.01 + c[0] * 0.01, 
						0.0, 
						a[1] * 0.98 + b[1] * 0.01 + c[1] * 0.01, 
						a[0] * 0.01 + b[0] * 0.98 + c[0] * 0.01, 
						0.0, 
						a[1] * 0.01 + b[1] * 0.98 + c[1] * 0.01, 
						0.93, 0.93, 0.93, 1.0);
	r_primitive_line_3d(a[0] * 0.01 + b[0] * 0.98 + c[0] * 0.01, 
						0.0, 
						a[1] * 0.01 + b[1] * 0.98 + c[1] * 0.01, 
						a[0] * 0.01 + b[0] * 0.01 + c[0] * 0.98, 
						0.0, 
						a[1] * 0.01 + b[1] * 0.01 + c[1] * 0.98, 
						0.93, 0.93, 0.93, 1.0);
	r_primitive_line_3d(a[0] * 0.98 + b[0] * 0.01 + c[0] * 0.01, 
						0.0, 
						a[1] * 0.98 + b[1] * 0.01 + c[1] * 0.01, 
						a[0] * 0.01 + b[0] * 0.01 + c[0] * 0.98, 
						0.0, 
						a[1] * 0.01 + b[1] * 0.01 + c[1] * 0.98, 
						0.93, 0.93, 0.93, 1.0);
}

//void mo_edit_loop_polygonize(IOEditProcess *edit, uint *loop, uint size, uint *ref)

void mo_menu_edit_loop_polygonize(double *array, uint size, float *polygons)
{
	uint i = 0, a = 0, b = 1, c = 2, vertex, count = 2, found[3], output = 0, ref_length = 0, save[2];
	double vec[2], *v, *base, *back, sides[4], f, dist, best, *used;
	
	used = malloc((sizeof *used) * size);
	for(i = 0; i < size; i++)
		used[i] = FALSE;

	while(count < size)
	{
		save[1] = -1;
		if(count + 1 < size)
		{
			best = 1000000;
			for(i = 0; i < size; i++)
			{
				a = b;
				b = c;
				c = (c + 1) % size;
				while(used[c])
					c = (c + 1) % size;

				base = &array[a * 2];
				vec[0] = array[c * 2 + 0] - base[0];
				vec[1] = array[c * 2 + 1] - base[1];
				f_normalize2d(vec);

				back = &array[b * 2 + 0];
				dist = (vec[1] * (back[0] - base[0]) - vec[0] * (back[1] - base[1]));

				sides[0] = array[c * 2 + 0] - back[0];
				sides[1] = array[c * 2 + 1] - back[1];
				sides[2] = array[a * 2 + 0] - back[0];
				sides[3] = array[a * 2 + 1] - back[1];
				save[0] = -1;
				for(vertex = (c + 1) % size; vertex != a; vertex = (vertex + 1) % size)
				{
					if(!used[vertex])
					{
						v = &array[vertex * 2];
						if(0 <= sides[1] * (v[0] - back[0]) - sides[0] * (v[1] - back[1]) &&
							0 >= sides[3] * (v[0] - back[0]) - sides[2] * (v[1] - back[1]))
						{
							f = (vec[1] * (base[0] - v[0]) - vec[0] * (base[1] - v[1]));
							if(f > dist)
							{
								if(f < 0.0 || (0 < sides[1] * (v[0] - back[0]) - sides[0] * (v[1] - back[1]) && 0 > sides[3] * (v[0] - back[0]) - sides[2] * (v[1] - back[1])))
								{
									save[0] = vertex;
									dist = f;
								}
							}
						}
					}
				}
				if(dist < best)
				{
					save[1] = save[0];
					best = dist;
					found[0] = a;
					found[1] = b;
					found[2] = c;
				}
			}
			a = found[0];
			b = found[1];
			c = found[2];
		}

		polygons[(count - 2) * 9 + 0] = (float)array[a * 2];
		polygons[(count - 2) * 9 + 1] = (float)array[a * 2 + 1];
		polygons[(count - 2) * 9 + 2] = (float)0.0;
		polygons[(count - 2) * 9 + 3] = (float)array[b * 2 + 0];
		polygons[(count - 2) * 9 + 4] = (float)array[b * 2 + 1];
		polygons[(count - 2) * 9 + 5] = (float)0.0;
		polygons[(count - 2) * 9 + 6] = (float)array[c * 2 + 0];
		polygons[(count - 2) * 9 + 7] = (float)array[c * 2 + 1];
		polygons[(count - 2) * 9 + 8] = (float)0.0;

		used[b] = TRUE;
		b = c;
		c = (c + 1) % size;
		while(used[c])
			c = (c + 1) % size;
		count++;
	}
	free(used);
}

boolean hxa_2d_editor_edit_loop_valid_test(double *array, uint size)
{
	uint i, j, next, jnext;
	double vec[2], f;
	boolean keep_going = FALSE;
	size *= 2;
	return TRUE;
	for(i = 0; i < size; i++)
		if(array[i] < -(256 * 1024) || array[i] > (256 * 1024))
			return FALSE;
	for(i = 0; i < size; i += 2)
	{
		for(j = 0; j < size; j += 2)
		{
			if(j != i)
			{
				vec[0] = array[i] - array[j];
				vec[1] = array[i + 1] - array[j + 1];
				f = vec[0] * vec[0] + vec[1] * vec[1];
				if(f < HXA_2D_EDITOR_MIN_EDGE_SIZE * HXA_2D_EDITOR_MIN_EDGE_SIZE)
					return FALSE;
			}
		}
	}

	for(j = 0; j < size; j += 2)
	{
		for(i = 0; i < size; i += 2)
		{
			next = (i + 2) % size;
			if(i != j && next != j)
				if(mo_line_segment_colission_test(&array[i], &array[next], &array[j]))
					return FALSE;
		}
	} 
	for(j = 0; j < size; j += 2)
	{
		jnext = (j + 2) % size;
		for(i = 0; i < size; i += 2)
		{
			next = (i + 2) % size;
			if(i != j && i != jnext && next != jnext && next != j)
			{
				vec[0] = array[i] - array[next];
				vec[1] = array[i + 1] - array[next + 1];
				if(((array[j] - array[i]) * vec[1] - (array[j + 1] - array[i + 1]) * vec[0] < 0) != 
					((array[jnext] - array[i]) * vec[1] - (array[jnext + 1] - array[i + 1]) * vec[0] < 0))
				{
					vec[0] = array[j] - array[jnext];
					vec[1] = array[j + 1] - array[jnext + 1];
					if(((array[i] - array[j]) * vec[1] - (array[i + 1] - array[j + 1]) * vec[0] < 0) != 
			 			((array[next] - array[j]) * vec[1] - (array[next + 1] - array[j + 1]) * vec[0] < 0))
					{
						return FALSE;
					}
				}
			}
		}
	}
	size = size - 2;
	f = (array[0] - array[size]) * (array[1] + array[size + 1]);
	for(i = 0; i < size; i += 2)
		f += (array[i + 2] - array[i]) * (array[i + 2 + 1] + array[i + 1]);
	return f > 0;
}



void hxa_2d_editor_edit_draw_background(HxA2DEditorInstance *level)
{
	RShader *shader;
	uint i, j;
	shader = r_shader_presets_get(P_SP_COLOR_UNIFORM);
	r_shader_set(shader);
	r_shader_vec4_set(NULL, r_shader_uniform_location(shader, "color"), 1, 1, 1, 1);

	for(i = 0; i < level->loop_count; i++)
	{
		r_array_section_draw(level->loops[i].pool, NULL, GL_TRIANGLES, 0, (level->loops[i].loop_size - 2) * 3);
	}
}


uint hxa_2d_editor_edit_guideline_snap_edge(double *pos_a, double *pos_b, double *best, double *vector, double pointer_x, double pointer_y)
{
	double pos1[3], pos2[3], vec[2], f, ff, length;
	vec[0] = pos_a[0] - pos_b[0];
	vec[1] = pos_a[1] - pos_b[1];
	length = f_normalize2d(vec);
	f = (pointer_x - pos_a[0]) * vec[1] - (pointer_y - pos_a[1]) * vec[0];
	f *= f;
	if(f < *best)
	{
		ff = vec[0] * vector[0] + vec[1] * vector[1];
		if(ff > 0.99 || ff < -0.99)
				return FALSE;
		ff = (pointer_x - pos_b[0]) * vec[0] - (pointer_y - pos_b[1]) * vec[1];
		if(ff > length * 11.0 || ff < length * -10.0)
			return FALSE;
		*best = f;
		return TRUE;
	}
	return FALSE;
}



boolean hxa_2d_editor_edit_guideline_snap_corner(double *output, double *pos_a, double *pos_b, double *vector, double *best, double pointer_x, double pointer_y)
{
	boolean out = FALSE;
	double pos2[2], vec[2];
	vec[0] = pos_a[0] - pos_b[0];
	vec[1] = pos_a[1] - pos_b[1];

	pos2[0] = pos_a[0] - vec[1];
	pos2[1] = pos_a[1] + vec[0];
	if(hxa_2d_editor_edit_guideline_snap_edge(pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}

	pos2[0] = pos_a[0] - vec[1] + vec[0];
	pos2[1] = pos_a[1] + vec[0] + vec[1];
	if(hxa_2d_editor_edit_guideline_snap_edge(pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	pos2[0] = pos_a[0] - vec[1] - vec[0];
	pos2[1] = pos_a[1] + vec[0] - vec[1];
	if(hxa_2d_editor_edit_guideline_snap_edge(pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	/*
	pos2[0] = pos_a[0] - vec[1] * 0.5 - vec[0] * MO_MENU_EDIT_COS_30_DEGREES;
	pos2[1] = pos_a[1] + vec[0] * 0.5 - vec[1] * MO_MENU_EDIT_COS_30_DEGREES;
	if(hxa_2d_editor_edit_guideline_snap_edge(pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	pos2[0] = pos_a[0] - vec[1] * 0.5 + vec[0] * MO_MENU_EDIT_COS_30_DEGREES;
	pos2[1] = pos_a[1] + vec[0] * 0.5 + vec[1] * MO_MENU_EDIT_COS_30_DEGREES;
	if(hxa_2d_editor_edit_guideline_snap_edge(pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}

	pos2[0] = pos_a[0] - vec[1] * MO_MENU_EDIT_COS_30_DEGREES - vec[0] * 0.5;
	pos2[1] = pos_a[1] + vec[0] * MO_MENU_EDIT_COS_30_DEGREES - vec[1] * 0.5;
	if(hxa_2d_editor_edit_guideline_snap_edge(pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	pos2[0] = pos_a[0] - vec[1] * MO_MENU_EDIT_COS_30_DEGREES + vec[0] * 0.5;
	pos2[1] = pos_a[1] + vec[0] * MO_MENU_EDIT_COS_30_DEGREES + vec[1] * 0.5;
	if(hxa_2d_editor_edit_guideline_snap_edge(pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}*/
	return out;
}

typedef enum{
	MO_EEST_NONE,
	MO_EEST_EDGE,
	MO_EEST_CORNER,
	MO_EEST_CIRCLE
}MOMenyEditSnapType;


MOMenyEditSnapType hxa_2d_editor_edit_guideline_snap_pass(HxA2DEditorInstance *level, double *output, double *best, double *vector, double pointer_x, double pointer_y, double scale, boolean manipulator)
{
	MOMenyEditSnapType out = MO_EEST_NONE;
	uint i, j, jj;
	double center_a[2], center_b[2];
	*best = 0.006 * scale * 0.006 * scale;
	
	center_a[0] = center_b[0] = 0;
	center_a[1] = -1;
	center_b[1] = 1;
	if(hxa_2d_editor_edit_guideline_snap_edge(center_a, center_b, best, vector, pointer_x, pointer_y))
	{
		output[0] = center_a[0];
		output[1] = center_a[1];
		output[2] = center_b[0];
		output[3] = center_b[1];
		out = MO_EEST_EDGE;
	}
	center_a[1] = center_b[1] = 0;
	center_a[0] = -1;
	center_b[0] = 1;
	if(hxa_2d_editor_edit_guideline_snap_edge(center_a, center_b, best, vector, pointer_x, pointer_y))
	{
		output[0] = center_a[0];
		output[1] = center_a[1];
		output[2] = center_b[0];
		output[3] = center_b[1];
		out = MO_EEST_EDGE;
	}


	if(level->manip_mode == HXA_2DEMM_RULER && !manipulator)
	{
		double f, length, vec[2], a[2], b[2];
		vec[0] = level->manip_end[0] - level->manip_start[0];
		vec[1] = level->manip_end[1] - level->manip_start[1];
		length = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		vec[0] /= length;
		vec[1] /= length;

		if(hxa_2d_editor_edit_guideline_snap_edge(level->manip_start, level->manip_end, best, vector, pointer_x, pointer_y))
		{
			output[0] = level->manip_start[0];
			output[1] = level->manip_start[1];
			output[2] = level->manip_end[0];
			output[3] = level->manip_end[1];
			out = MO_EEST_EDGE;
		}
		j = level->manip_divisions;
		for(i = 0; i < j * 3 + 1; i++)
		{
			f = length * (double)i / (double)j - length;
			a[0] = level->manip_start[0] + vec[0] * f + vec[1];
			a[1] = level->manip_start[1] + vec[1] * f - vec[0]; 
			b[0] = level->manip_start[0] + vec[0] * f;
			b[1] = level->manip_start[1] + vec[1] * f;

			if(hxa_2d_editor_edit_guideline_snap_edge(a, b, best, vector, pointer_x, pointer_y))
			{
				output[0] = a[0];
				output[1] = a[1];
				output[2] = b[0];
				output[3] = b[1];
				out = MO_EEST_EDGE;
			}
		}
	}
	if(level->manip_mode == HXA_2DEMM_GRID && !manipulator)
	{
		double f, length, vec[2], a[2], b[2];
		vec[0] = level->manip_end[0] - level->manip_start[0];
		vec[1] = level->manip_end[1] - level->manip_start[1];
		length = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		vec[0] /= length;
		vec[1] /= length;
		j = level->manip_divisions;
		for(i = 0; i < j * 3 + 1; i++)
		{
			f = length * (double)i / (double)j - length;
			a[0] = level->manip_start[0] + vec[0] * f + vec[1];
			a[1] = level->manip_start[1] + vec[1] * f - vec[0]; 
			b[0] = level->manip_start[0] + vec[0] * f;
			b[1] = level->manip_start[1] + vec[1] * f;

			if(hxa_2d_editor_edit_guideline_snap_edge(a, b, best, vector, pointer_x, pointer_y))
			{
				output[0] = a[0];
				output[1] = a[1];
				output[2] = b[0];
				output[3] = b[1];
				out = MO_EEST_EDGE;
			}
		}
		for(i = 0; i < j * 2 + 1; i++)
		{
			f = length * (double)i / (double)j - length;
			a[0] = level->manip_start[0] - vec[1] * f + vec[0];
			a[1] = level->manip_start[1] + vec[0] * f + vec[1]; 
			b[0] = level->manip_start[0] - vec[1] * f;
			b[1] = level->manip_start[1] + vec[0] * f;
			if(hxa_2d_editor_edit_guideline_snap_edge(a, b, best, vector, pointer_x, pointer_y))
			{
				output[0] = a[0];
				output[1] = a[1];
				output[2] = b[0];
				output[3] = b[1];
				out = MO_EEST_EDGE;
			}
		}	
		return out;
	}

	if(level->manip_mode == HXA_2DEMM_CIRCLE)
	{
		double center[2], vec[2], circle[2], f, x, y;
		center[0] = (level->manip_end[0] + level->manip_start[0]) * 0.5;
		center[1] = (level->manip_end[1] + level->manip_start[1]) * 0.5;
		vec[0] = (level->manip_end[0] - level->manip_start[0]) * 0.5;
		vec[1] = (level->manip_end[1] - level->manip_start[1]) * 0.5;
		j = level->manip_divisions * 2;
		for(i = 0; i < j; i++)
		{
			f = (double)i / (double)j * 2.0 * PI;
			x = sin(f);
			y = cos(f);
			circle[0] = center[0] + vec[0] * x - vec[1] * y;
			circle[1] = center[1] + vec[1] * x + vec[0] * y;
			if(hxa_2d_editor_edit_guideline_snap_edge(center, circle, best, vector, pointer_x, pointer_y))
			{
				output[0] = center[0];
				output[1] = center[1];
				output[2] = circle[0];
				output[3] = circle[1];
				out = MO_EEST_EDGE;
			}
		}
	}

	for(i = 0; i < level->loop_count; i++)
	{
		for(j = 0; j < level->loops[i].loop_size; j++)
		{
			jj = (j + 1) % level->loops[i].loop_size;
			if((!level->loops[i].selection[j] && !level->loops[i].selection[jj]) || manipulator)
			{
				if(hxa_2d_editor_edit_guideline_snap_edge(&level->loops[i].loop[j * 2], &level->loops[i].loop[jj * 2], best, vector, pointer_x, pointer_y))
				{
					output[0] = level->loops[i].loop[j * 2];
					output[1] = level->loops[i].loop[j * 2 + 1];
					output[2] = level->loops[i].loop[jj * 2];
					output[3] = level->loops[i].loop[jj * 2 + 1];
					out = TRUE;
				}
				if(level->loops[i].selection[(j + level->loops[i].loop_size - 1) % level->loops[i].loop_size])
					if(hxa_2d_editor_edit_guideline_snap_corner(output, &level->loops[i].loop[j * 2], &level->loops[i].loop[jj * 2], vector, best, pointer_x, pointer_y))
						out = MO_EEST_EDGE;
				if(level->loops[i].selection[(j + 2) % level->loops[i].loop_size])
					if(hxa_2d_editor_edit_guideline_snap_corner(output, &level->loops[i].loop[jj * 2], &level->loops[i].loop[j * 2], vector, best, pointer_x, pointer_y))
						out = MO_EEST_EDGE;
			}
		}
	}
	return out;
}


boolean mo_menu_edit_draw_guidecircle_snap(HxA2DEditorInstance *level, double *pos, double *center, double radius, double *output, double *best)
{
	double vec[2], f;
	vec[0] = pos[0] - center[0];
	vec[1] = pos[1] - center[1];
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	if(f < radius)
	{
		f = radius - f;
		if(f < *best)
		{
			*best = f;
			output[0] = center[0];
			output[1] = center[1];
			output[2] = radius;
			return TRUE;
		}
	}
	return FALSE;
}

boolean mo_menu_edit_draw_guidecircle_snap_pass(HxA2DEditorInstance *level, double *pos, double *output, double *ignore, double *best, double scale)
{
	boolean out = FALSE;
	uint i, j, j_pre, j_post;
	*best = 0.006 * scale;

	if(level->manip_mode == HXA_2DEMM_CIRCLE)
	{
		double center[2];
		center[0] = (level->manip_end[0] + level->manip_start[0]) * 0.5;
		center[1] = (level->manip_end[1] + level->manip_start[1]) * 0.5;
		if(ignore[0] != center[0] || ignore[1] != center[1])
			if(mo_menu_edit_draw_guidecircle_snap(level, pos, center, f_distance2d(level->manip_start, level->manip_end) * 0.5, output, best))
				out = TRUE;
	}
	
	for(i = 0; i < level->loop_count; i++)
	{
		for(j = 0; j < level->loops[i].loop_size; j++)
		{
			if(!level->loops[i].selection[j])
			{
				if(ignore[0] != level->loops[i].loop[j * 2] || ignore[1] != level->loops[i].loop[j * 2 + 1])
				{
					j_pre = (j + level->loops[i].loop_size - 1) % level->loops[i].loop_size;
					j_post = (j + 1) % level->loops[i].loop_size;
						
					if(!level->loops[i].selection[j_pre] && level->loops[i].selection[j_post])
						if(mo_menu_edit_draw_guidecircle_snap(level, pos, &level->loops[i].loop[j * 2], f_distance2d(&level->loops[i].loop[j * 2], &level->loops[i].loop[j_pre * 2]), output, best))
							out = TRUE;
					if(level->loops[i].selection[j_pre] && !level->loops[i].selection[j_post])
						if(mo_menu_edit_draw_guidecircle_snap(level, pos, &level->loops[i].loop[j * 2], f_distance2d(&level->loops[i].loop[j * 2], &level->loops[i].loop[j_post * 2]), output, best))
							out = TRUE;
				}

			}
		}
	}
	*best *= *best;
	return out;
}



void hxa_2d_editor_edit_guideline_snap(HxA2DEditorInstance *level, double *pos, double pointer_x, double pointer_y, double scale, boolean manipulator)
{
	double edge[2][4], vector[2] = {0, 0};
	double camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 0, 1}, f, best[3] = {100000, 100000, 100000}, circle[2][3] = {60000, 60000, -1, 0, 0, -1};
	MOMenyEditSnapType output[2] = {MO_EEST_NONE, MO_EEST_NONE};
	uint i, ii;
	boolean circle_output = FALSE;

	seduce_view_camera_getd(NULL, camera);
	seduce_view_camera_vector_getd(NULL, vec, pointer_x, pointer_y);
	f_project3d(pos, center, normal, camera, vec);
	pos[2] = 0;

	for(i = 0; i < 2; i++)
	{
		output[i] = hxa_2d_editor_edit_guideline_snap_pass(level, edge[i], &best[0], vector, pos[0], pos[1], scale, manipulator);
		if(output[i] == MO_EEST_CORNER)
		{
			pos[0] = edge[i][0];
			pos[1] = edge[i][1];
			if(pos[0] < -1 || pos[0] > 1 || pos[1] < -1 || pos[1] > 1 || pos[0] != pos[0] || pos[1] != pos[1])
				i = 0;
			return;
		}
		if(mo_menu_edit_draw_guidecircle_snap_pass(level, pos, circle[i], circle[0], &best[i], scale))
			output[i] = MO_EEST_CIRCLE;
		if(output[i] == MO_EEST_EDGE)
		{
			vector[0] = edge[i][0] - edge[i][2];
			vector[1] = edge[i][1] - edge[i][3];
			f_normalize2d(vector);
		}
		if(output[i] == MO_EEST_NONE)
			break;
	}	
	if(output[0] == MO_EEST_NONE)
	{
		if(pos[0] < -1 || pos[0] > 1 || pos[1] < -1 || pos[1] > 1 || pos[0] != pos[0] || pos[1] != pos[1])
			i = 0;
		return;
	}
	if(output[0] == MO_EEST_EDGE && output[1] == MO_EEST_NONE)
	{
		vector[0] = edge[0][0] - edge[0][2];
		vector[1] = edge[0][1] - edge[0][3];
		f_normalize2d(vector);
		f = vector[1] * (edge[0][2] - pos[0]) - vector[0] * (edge[0][3] - pos[1]);
		pos[0] += vector[1] * f;
		pos[1] += -vector[0] * f;
		if(pos[0] < -1 || pos[0] > 1 || pos[1] < -1 || pos[1] > 1 || pos[0] != pos[0] || pos[1] != pos[1])
			i = 0;
		return;
	}
	if(output[0] == MO_EEST_CIRCLE && output[1] == MO_EEST_NONE)
	{
		vector[0] = pos[0] - circle[0][0];
		vector[1] = pos[1] - circle[0][1];
		f_normalize2d(vector);
		pos[0] = circle[0][0] + vector[0] * circle[0][2];
		pos[1] = circle[0][1] + vector[1] * circle[0][2];
		return;
	}
	if(output[0] == MO_EEST_EDGE && output[1] == MO_EEST_EDGE)
	{
		f_intersect2d(pos, edge[0], &edge[0][2], edge[1], &edge[1][2]);
		if(pos[0] < -1 || pos[0] > 1 || pos[1] < -1 || pos[1] > 1 || pos[0] != pos[0] || pos[1] != pos[1])
			i = 0;
		return;
	}
	if((output[0] == MO_EEST_CIRCLE && output[1] == MO_EEST_EDGE) ||
	   (output[0] == MO_EEST_EDGE && output[1] == MO_EEST_CIRCLE))
	{
		if(output[0] == MO_EEST_EDGE)
		{
			i = 0;
			ii = 1;
		}else
		{
			i = 1;
			ii = 0;
		}
		f_vector_normalized2d(vector, edge[i], &edge[i][2]);
		f = vector[1] * (circle[ii][0] - edge[i][0]) - vector[0] * (circle[ii][1] - edge[i][1]);
		if(f < circle[ii][2] && f > -circle[ii][2])
		{
			vec[0] = circle[ii][0] - vector[1] * f;
			vec[1] = circle[ii][1] + vector[0] * f;
			f = sqrt(circle[ii][2] * circle[ii][2] - f * f);
			if((pos[0] - circle[ii][0]) * vector[0] + (pos[1] - circle[ii][1]) * vector[1] > 0)
			{
				pos[0] = vec[0] + vector[0] * f;
				pos[1] = vec[1] + vector[1] * f;
			}else
			{
				pos[0] = vec[0] - vector[0] * f;
				pos[1] = vec[1] - vector[1] * f;
			}
		}else
		{
			pos[0] = circle[ii][0] - vector[1] * circle[ii][2];
			pos[1] = circle[ii][1] + vector[0] * circle[ii][2];
		}
		if(pos[0] < -1 || pos[0] > 1 || pos[1] < -1 || pos[1] > 1 || pos[0] != pos[0] || pos[1] != pos[1])
			i = 0;
		return;
	}
	if(output[0] == MO_EEST_CIRCLE && output[1] == MO_EEST_CIRCLE)
	{
		for(i = 0; i < 64; i++)
		{
			ii = i % 2;
			vector[0] = pos[0] - circle[ii][0];
			vector[1] = pos[1] - circle[ii][1];
			f_normalize2d(vector);
			pos[0] = circle[ii][0] + vector[0] * circle[ii][2];
			pos[1] = circle[ii][1] + vector[1] * circle[ii][2];
		}
		return;
	}
	i = 0;

	/*

	switch(output)
	{
		case MO_EEST_NONE :
			if(!manipulator)
			{
				if(mo_menu_edit_draw_guidecircle_snap_pass(level, pos, circle, &best[1], scale))
				{
					f_vector_normalized2d(vector, circle, pos);
					pos[0] = circle[0] + vector[0] * circle[2];
					pos[1] = circle[1] + vector[1] * circle[2];
				}
			}
		break;
		case MO_EEST_EDGE :
			vector[0] = edge[0] - edge[2];
			vector[1] = edge[1] - edge[3];
			f_normalize2d(vector);
			if(!manipulator)
				circle_output = mo_menu_edit_draw_guidecircle_snap_pass(level, pos, circle, &best[1], scale);

			if(MO_EEST_EDGE == hxa_2d_editor_edit_guideline_snap_pass(level, other_edge, &best[2], vector, pos[0], pos[1], scale, manipulator) && (!circle_output || best[2] < best[1]))
			{
				f_intersect2d(pos, edge, &edge[2], other_edge, &other_edge[2]);
			}else if(circle_output)
			{
				f_vector_normalized2d(vector, edge, &edge[2]);
				f = vector[1] * (circle[0] - edge[0]) - vector[0] * (circle[1] - edge[1]);
				if(f < circle[2] && f > -circle[2])
				{
					vec[0] = circle[0] - vector[1] * f;
					vec[1] = circle[1] + vector[0] * f;
					f = sqrt(circle[2] * circle[2] - f * f);
					if((pos[0] - circle[0]) * vector[0] + (pos[1] - circle[1]) * vector[1] > 0)
					{
						pos[0] = vec[0] + vector[0] * f;
						pos[1] = vec[1] + vector[1] * f;
					}else
					{
						pos[0] = vec[0] - vector[0] * f;
						pos[1] = vec[1] - vector[1] * f;
					}
				}else
				{
					pos[0] = circle[0] - vector[1] * circle[2];
					pos[1] = circle[1] + vector[0] * circle[2];
				}
			}else
			{
				f = vector[1] * (edge[2] - pos[0]) - vector[0] * (edge[3] - pos[1]);
				pos[0] += vector[1] * f;
				pos[1] += -vector[0] * f;
			//	r_primitive_line_3d(pos[0], 0.1, pos[1], pos[0] + vector[1], 0.1, pos[1] - vector[0], 1.2, 0.6, 1.0, 1.0);
			}
		break;
		case MO_EEST_CORNER : 
			pos[0] = edge[0];
			pos[1] = edge[1];
		break;


	}*/

}

void mo_menu_edit_draw_guideline_edge(double *pos, double *vector, double length, float scale)
{
	double f;
	float vec_short[2], vec_long[2], vec[2], vec_shortest[2];
	vec_short[0] = vector[0] * 0.01 * scale;
	vec_short[1] = vector[1] * 0.01 * scale;
	vec_shortest[0] = vector[0] * 0.025 * scale;
	vec_shortest[1] = vector[1] * 0.025 * scale;
	vec_long[0] = vector[0] * length * 10.0;
	vec_long[1] = vector[1] * length * 10.0;
	r_primitive_line_fade_2d(pos[0] - vec_short[1], pos[1] + vec_short[0], pos[0] - vec_long[1], pos[1] + vec_long[0], 0, 0, 0, 0.1, 0, 0, 0, 0.05);
	r_primitive_line_fade_2d(pos[0] + vec_short[1], pos[1] - vec_short[0], pos[0] + vec_long[1], pos[1] - vec_long[0], 0, 0, 0, 0.1, 0, 0, 0, 0.05);
	
	r_primitive_line_fade_2d(pos[0] + vec_short[0] + vec_short[1],
							pos[1] + vec_short[1] - vec_short[0], 
							pos[0] + vec_long[0] + vec_long[1],
							pos[1] + vec_long[1] - vec_long[0], 
							0, 0, 0, 0.1, 0, 0, 0, 0.05);
	r_primitive_line_fade_2d(pos[0] + vec_short[0] - vec_short[1],
							pos[1] + vec_short[1] + vec_short[0], 
							pos[0] + vec_long[0] - vec_long[1],
							pos[1] + vec_long[1] + vec_long[0], 
							0, 0, 0, 0.1, 0, 0, 0, 0.05);
	r_primitive_line_fade_2d(pos[0] - vec_short[0] + vec_short[1],
							pos[1] - vec_short[1] - vec_short[0], 
							pos[0] - vec_long[0] + vec_long[1],
							pos[1] - vec_long[1] - vec_long[0], 
							0, 0, 0, 0.1, 0, 0, 0, 0.05);
	r_primitive_line_fade_2d(pos[0] - vec_short[0] - vec_short[1],
							pos[1] - vec_short[1] + vec_short[0], 
							pos[0] - vec_long[0] - vec_long[1],
							pos[1] - vec_long[1] + vec_long[0], 
							0, 0, 0, 0.1, 0, 0, 0, 0.05);
	/*
	vec_short[0] *= MO_MENU_EDIT_COS_30_DEGREES;
	vec_short[1] *= MO_MENU_EDIT_COS_30_DEGREES;
	vec_long[0] *= MO_MENU_EDIT_COS_30_DEGREES;
	vec_long[1] *= MO_MENU_EDIT_COS_30_DEGREES;
	vec[0] = vector[0] * scale;
	vec[1] = vector[1] * scale;
	
	r_primitive_line_fade_2d(pos[0] - vec_shortest[0] - vec_short[1],
							pos[1] - vec_shortest[1] + vec_short[0], 
							pos[0] - vec[0] - vec_long[1],
							pos[1] - vec[1] + vec_long[0], 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_2d(pos[0] - vec_shortest[0] + vec_short[1],
							pos[1] - vec_shortest[1] - vec_short[0], 
							pos[0] - vec[0] + vec_long[1],
							pos[1] - vec[1] - vec_long[0], 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_2d(pos[0] + vec_shortest[0] - vec_short[1],
							pos[1] + vec_shortest[1] + vec_short[0], 
							pos[0] + vec[0] - vec_long[1],
							pos[1] + vec[1] + vec_long[0], 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_2d(pos[0] + vec_shortest[0] + vec_short[1],
							pos[1] + vec_shortest[1] - vec_short[0], 
							pos[0] + vec[0] + vec_long[1],
							pos[1] + vec[1] - vec_long[0], 
							0, 0, 0, 0.1, 0, 0, 0, 0);
	

	r_primitive_line_fade_2d(pos[0] - vec_shortest[1] + vec_short[0],
							pos[1] + vec_shortest[0] + vec_short[1], 
							pos[0] - vec[1] + vec_long[0], 
							pos[1] + vec[0] + vec_long[1], 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_2d(pos[0] + vec_shortest[1] + vec_short[0],
							pos[1] - vec_shortest[0] + vec_short[1], 
							pos[0] + vec[1] + vec_long[0], 
							pos[1] - vec[0] + vec_long[1], 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_2d(pos[0] - vec_shortest[1] - vec_short[0],
							pos[1] + vec_shortest[0] - vec_short[1], 
							pos[0] - vec[1] - vec_long[0],
							pos[1] + vec[0] - vec_long[1], 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_2d(pos[0] + vec_shortest[1] - vec_short[0],
							pos[1] - vec_shortest[0] - vec_short[1], 
							pos[0] + vec[1] - vec_long[0],
							pos[1] - vec[0] - vec_long[1], 
							0, 0, 0, 0.1, 0, 0, 0, 0);

*/
	for(f = 0; f < PI * 2; f += PI / 32.0)
		r_primitive_line_fade_2d(pos[0] + sin(f) * length, pos[1] + cos(f) * length, 
								pos[0] + sin(f + PI / 32.0) * length, pos[1] + cos(f + PI / 32.0) * length, 0, 0, 0, 0.1, 0, 0, 0, 0.1);
}

void hxa_2d_editor_draw_surface(BInputState *input, HxA2DEditorShape *edit, char *(*material_func)(uint32 material, float *color, void *user), void *user)
{
	HxA2DEditorInstance *level;
	float colors[] = {1, 1, 1,
					0.5, 0.5, 0.5,
					0.3, 0.3, 0.3,
					0.2, 0.6, 1.0,
					1.0, 1.0, 1.0,
					0.2, 0.6, 1.0};
	uint mirror_count[] = {1, 2, 3, 4, 2, 4};
	double matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, alpha, vec[2], *a, *b, length;
	float mf[16], color[4];
	uint i, j, k;
	static RShader	*shader = NULL;
	static uint location;
	level = &edit->instances[edit->instance_current];
	for(i = 0; i < level->loop_count; i++)
	{
		if(level->loops[i].triangle_array != NULL && level->loops[i].pool == NULL)
		{
			RFormats vertex_format_types = R_FLOAT;
			uint vertex_format_size = 3;
			level->loops[i].pool = r_array_allocate((level->loops[i].loop_size - 2) * 3, &vertex_format_types, &vertex_format_size, 1, 0);
			r_array_load_vertex(level->loops[i].pool, NULL, level->loops[i].triangle_array, 0, (level->loops[i].loop_size - 2) * 3);
		}
	}


	if(shader == NULL)
	{
		char *shader_color_vertex = 
			"attribute vec3 vertex;\n"
			"uniform mat4 ModelViewProjectionMatrix;\n"
			"void main()\n"
			"{"
			"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);\n"
			"}";
		char *shader_color_fragment = 
			"uniform vec4 color;\n"
			"void main()\n"
			"{\n"
			"	gl_FragColor = color;\n"
			"}\n";
		shader = r_shader_create_simple(NULL, 0, shader_color_vertex, shader_color_fragment, "color primitive");
		location = r_shader_uniform_location(shader, "color");
		r_shader_state_set_depth_test(shader, R_DT_ALWAYS); 
	}
	r_shader_set(shader);
	
	for(i = 0; i < level->loop_count; i++)
	{
		if(level->loops[i].pool != NULL)
		{
			material_func(level->loops[i].material, color, user);
			r_shader_vec4_set(NULL, location, color[0], color[1], color[2], color[3]);
			r_array_section_draw(level->loops[i].pool, NULL, GL_TRIANGLES, 0, -1);
		}
	}
							

/*	for(k = 0; k < MO_LST_COUNT; k++)
	{
		alpha = 1.0;
		for(j = 0; j < mirror_count[level->symetry]; j++)
		{
			hxa_2d_editor_structure_process_matrix(matrix, j, level->symetry);
			for(k = 0; k < 16; k++)
				mf[k] = (float)matrix[k];
			r_matrix_push(NULL);
			r_matrix_translate(NULL, level->symetry_pos[0], 0, level->symetry_pos[1]);
			r_matrix_matrix_mult(NULL, mf);
			r_matrix_translate(NULL, -level->symetry_pos[0], 0, -level->symetry_pos[1]);
			for(i = 0; i < level->loop_count; i++)
			{
				r_shader_vec4_set(NULL, location, 1, 0, 1, 1);
				r_array_section_draw(level->loops[i].pool, NULL, GL_TRIANGLES, 0, -1);
			}
			r_matrix_pop(NULL);
			alpha = 0.3;
		}
	}*/
}

void mo_menu_edit_draw_level_guides(BInputState *input, HxA2DEditorInstance *level, float scale)
{
	double matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, vec[2], vec2[2], *a, *b, length;
	float mf[16];
	uint i, j;
	for(i = 0; i < level->loop_count; i++)
	{
		for(j = 0; j < level->loops[i].loop_size; j++)
		{
			if(!level->loops[i].selection[j] && !level->loops[i].selection[(j + level->loops[i].loop_size - 1) % level->loops[i].loop_size])
			{
				b = &level->loops[i].loop[((j + level->loops[i].loop_size - 1) % level->loops[i].loop_size) * 2];
				a = &level->loops[i].loop[j * 2];
				vec[0] = a[0] - b[0];
				vec[1] = a[1] - b[1];
				length = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);			
				vec2[0] = vec[0] * scale / length;
				vec2[1] = vec[1] * scale / length;
				r_primitive_line_fade_2d(a[0] + vec2[0] * 0.01, a[1] + vec2[1] * 0.01, a[0] + vec[0] * 10.0, a[1] + vec[1] * 10.0, 0, 0, 0, 0.1, 0, 0, 0, 0.05);
				r_primitive_line_fade_2d(b[0] - vec2[0] * 0.01, b[1] - vec2[1] * 0.01, b[0] - vec[0] * 10.0, b[1] - vec[1] * 10.0, 0, 0, 0, 0.1, 0, 0, 0, 0.05);

				vec[0] /= length;
				vec[1] /= length;
				if(level->loops[i].selection[(j + 1) % level->loops[i].loop_size])
					mo_menu_edit_draw_guideline_edge(a, vec, length, scale);
				if(level->loops[i].selection[(j + level->loops[i].loop_size - 2) % level->loops[i].loop_size])
					mo_menu_edit_draw_guideline_edge(b, vec, length, scale);
			}
		}
	}

	r_primitive_line_flush();
}


void mo_menu_edit_draw_loop_menu(BInputState *input, HxA2DEditorShape *edit, HxA2DEditorInstance *level, uint loop, float pos_x, float pos_y, float vec_x, float vec_y, char *(*material_func)(uint32 material, float *color, void *user), void *user, float scale)
{
	static float matrix[16] = {1, 0, 0, 0, 
							0, 1, 0, 0, 
							0, 0, 1, 0, 
							0, 0, 0, 1};
	float square[12] = {0.0, 0.002, 0,
						0.1, 0.002, 0,
						0.1, 0.012, 0, 
						0.0, 0.012, 0}, tmp[4];
	uint i, j;
	
	tmp[0] = -vec_x;
	tmp[1] = -vec_y;
	f_normalize2f(tmp);
	if(input->mode == BAM_DRAW)
	{
		matrix[0] = tmp[0];
		matrix[1] = tmp[1];
		matrix[4] = -tmp[1];	
		matrix[5] = tmp[0];
		matrix[12] = pos_x;
		matrix[13] = pos_y;
		r_matrix_push(NULL);
		r_matrix_matrix_mult(NULL, matrix);
	}
	/*
	for(i = 0; i < MO_LST_COUNT - 1; i++)
	{	
		if(input->mode == BAM_DRAW)
		{
			tmp[0] = (float)i * 0.012;
			tmp[1] = 0.002;
			square[0] = square[9] = (float)i * 0.012;
			square[3] = square[6] = (float)i * 0.012 + 0.012;
		//	r_primitive_surface(tmp[0], tmp[1], 0, 0.01, 0.01, mo_menu_edit_surface_colors[i * 3], mo_menu_edit_surface_colors[i * 3 + 1], mo_menu_edit_surface_colors[i * 3 + 2], 1);
			for(j = 0; j < input->pointer_count; j++)
				if(&matrix[i] == seduce_element_pointer_id(input, j, NULL))
					break; 
			if(j < input->pointer_count || level->loops[loop].type == i)
			{
				r_primitive_surface(tmp[0], tmp[1], 0.00, 0.01, 0.01, 0, 0, 0, 0.1);
				r_primitive_surface(tmp[0], tmp[1], 0.01, 0.01, 0.01, mo_menu_edit_surface_colors[i * 3], mo_menu_edit_surface_colors[i * 3 + 1], mo_menu_edit_surface_colors[i * 3 + 2], 1);

			}else
				r_primitive_surface(tmp[0], tmp[1], 0, 0.01, 0.01, mo_menu_edit_surface_colors[i * 3], mo_menu_edit_surface_colors[i * 3 + 1], mo_menu_edit_surface_colors[i * 3 + 2], 1);
			seduce_element_add_quad(input, &matrix[i], 0, &square[0], &square[3], &square[6], &square[9]);
		}
		if(seduce_widget_button_invisible(input, &matrix[i], (float)i * 0.012 + 0.006, 0.002 + 0.006, 0.01, FALSE))
		{
			level = hxa_2d_editor_structure_instance_add(edit);
			level->loops[loop].type = i;
		}
	}*/

	
	if(seduce_widget_button_icon(input, &level->loop_allocated, SEDUCE_OBJECT_ADD, (0.012) * scale, 0.014 * scale, 0.02 * scale, 1, NULL))
	{
		level = hxa_2d_editor_structure_instance_add(edit);
		mo_menu_edit_unselect(level);
		hxa_2d_editor_structure_add_loop(level, level->loops[loop].material, 0, 0, 1, level->loops[loop].loop_allocated);
		level->loops[level->loop_count - 1].loop_size = level->loops[loop].loop_size;
		for(i = 0; i < level->loops[loop].loop_size; i++)
		{
			level->loops[level->loop_count - 1].loop[i * 2] = level->loops[loop].loop[i * 2] + 0.1 * scale;
			level->loops[level->loop_count - 1].loop[i * 2 + 1] = level->loops[loop].loop[i * 2 + 1] + 0.1 * scale;
			level->loops[level->loop_count - 1].selection[i] = TRUE;
			level->loops[loop].selection[i] = FALSE;
		}
		level->loop_selected = level->loop_count - 1;
	}

	if(seduce_widget_button_icon(input, &level->loop_count, SEDUCE_OBJECT_SUBTRACT, (0.012 + 0.024) * scale, 0.014 * scale, 0.02 * scale, 1, NULL))
	{
		level = hxa_2d_editor_structure_instance_add(edit);
		hxa_2d_editor_structure_remove_loop(level, loop);
	}

	if(seduce_widget_button_icon(input, &level->loops[loop].loop_allocated, SEDUCE_OBJECT_UP, (0.012 + 0.024 * 2.0) * scale, 0.014 * scale, 0.02 * scale, 1, NULL))
	{
		hxa_2d_editor_move_up(edit, loop);
		return;
	}

	if(seduce_widget_button_icon(input, &level->loops[loop].loop_size, SEDUCE_OBJECT_DOWN, (0.012 + 0.024 * 3.0) * scale, 0.014 * scale, 0.02 * scale, 1, NULL))
	{
		hxa_2d_editor_move_down(edit, loop);
		return;
	}

	if(input->mode == BAM_DRAW)
	{
		if(material_func != NULL)
			seduce_text_line_draw(NULL, (0.024 * 4.0) * scale, 0.006 * scale, 0.01 * scale, SEDUCE_T_SPACE, material_func(level->loops[loop].material, tmp, user), 0.5, 0.5, 0.5, 1, -1);
		r_matrix_pop(NULL);
	}
}

boolean mo_menu_edit_draw_delete(BInputState *input, void *id, float pos_x, float pos_y, float scale)
{
	float pos[3];
	uint i;
	pos[0] = pos_x;
	pos[1] = pos_y;
	pos[2] = 0;
	if(input->mode == BAM_DRAW)
	{
		seduce_element_add_point(input, id, 0, pos, 0.01 * scale);			

		r_primitive_line_2d(pos[0] + 0.0 * scale, pos[1] + 0.005 * scale, pos[0] + 0.0075 * scale, pos[1] + 0.0125 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] + 0.0125 * scale, pos[1] + 0.0075 * scale, pos[0] + 0.0075 * scale, pos[1] + 0.0125 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] + 0.005 * scale, pos[1] + 0.0 * scale, pos[0] + 0.0125 * scale, pos[1] + 0.0075 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] - 0.0 * scale, pos[1] + 0.005 * scale, pos[0] - 0.0075 * scale, pos[1] + 0.0125 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] - 0.0125 * scale, pos[1] + 0.0075 * scale, pos[0] - 0.0075 * scale, pos[1] + 0.0125 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] - 0.005 * scale, pos[1] + 0.0 * scale, pos[0] - 0.0125 * scale, pos[1] + 0.0075 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] + 0.0 * scale, pos[1] - 0.005 * scale, pos[0] + 0.0075 * scale, pos[1] - 0.0125 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] + 0.0125 * scale, pos[1] - 0.0075 * scale, pos[0] + 0.0075 * scale, pos[1] - 0.0125 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] + 0.005 * scale, pos[1] - 0.0 * scale, pos[0] + 0.0125 * scale, pos[1] - 0.0075 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] - 0.0 * scale, pos[1] - 0.005 * scale, pos[0] - 0.0075 * scale, pos[1] - 0.0125 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] - 0.0125 * scale, pos[1] - 0.0075 * scale, pos[0] - 0.0075 * scale, pos[1] - 0.0125 * scale, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_2d(pos[0] - 0.005 * scale, pos[1] - 0.0 * scale, pos[0] - 0.0125 * scale, pos[1] - 0.0075 * scale, 0.0, 0.0, 0.0, 0.3);
	}
	for(i = 0; i < input->pointer_count; i++)
		if(id == seduce_element_pointer_id(input, i, NULL))
			break;
	if(i < input->pointer_count)
	{
		if(input->mode == BAM_DRAW)
		{
			r_primitive_line_3d(pos[0] + 0.0 * scale, pos[1] + 0.005 * scale, 0.005 * scale, pos[0] + 0.0075 * scale, pos[1] + 0.0125 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.0125 * scale,  pos[1] + 0.0075 * scale, 0.005 * scale, pos[0] + 0.0075 * scale, pos[1] + 0.0125 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.005 * scale, pos[1] + 0.0 * scale, 0.005 * scale, pos[0] + 0.0125 * scale, pos[1] + 0.0075 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.0 * scale, pos[1] + 0.005 * scale, 0.005 * scale, pos[0] - 0.0075 * scale, pos[1] + 0.0125 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.0125 * scale, pos[1] + 0.0075 * scale, 0.005 * scale, pos[0] - 0.0075 * scale, pos[1] + 0.0125 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.005 * scale, pos[1] + 0.0 * scale, 0.005 * scale, pos[0] - 0.0125 * scale, pos[1] + 0.0075 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.0 * scale, pos[1] - 0.005 * scale, 0.005 * scale, pos[0] + 0.0075 * scale, pos[1] - 0.0125 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.0125 * scale, pos[1] - 0.0075 * scale, 0.005 * scale, pos[0] + 0.0075 * scale, pos[1] - 0.0125 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.005 * scale, pos[1] - 0.0 * scale, 0.005 * scale, pos[0] + 0.0125 * scale, pos[1] - 0.0075 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.0 * scale, pos[1] - 0.005 * scale, 0.005 * scale, pos[0] - 0.0075 * scale, pos[1] - 0.0125 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.0125 * scale, pos[1] - 0.0075 * scale, 0.005 * scale, pos[0] - 0.0075 * scale, pos[1] - 0.0125 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.005 * scale, pos[1] - 0.0 * scale, 0.005 * scale, pos[0] - 0.0125 * scale, pos[1] - 0.0075 * scale, 0.005 * scale, 1.0, 0.0, 0.0, 1.0);
		}
		if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			return TRUE;
	}
	return FALSE;
}

extern void mo_edit_process_test(HxA2DEditorInstance *level);


void hxa_2d_editor_draw(BInputState *input, HxA2DEditorShape *edit, boolean draw_shape, char *(*material_func)(uint32 material, float *color, void *user), void *user, double scale)
{
	double d, *array, origo[3] = {0, 0, 0}, center[2], selection;
	float scalef, f, f2, af[3] = {0, 0, 0}, bf[3] = {0, 0, 0};
	uint i, j, k, l, m, size;
	static boolean shift = FALSE, shift_last = FALSE;
	HxA2DEditorInstance *level;
	level = &edit->instances[edit->instance_current];
	scalef = (float)scale;
	betray_button_get_up_down(0, &shift, &shift_last, BETRAY_BUTTON_SHIFT);

	if(betray_button_get(-1, BETRAY_BUTTON_UNDO))
		hxa_2d_editor_undo(edit);
	if(betray_button_get(-1, BETRAY_BUTTON_REDO))
		hxa_2d_editor_redo(edit);
	if(betray_button_get(-1, BETRAY_BUTTON_DELETE))
		hxa_2d_editor_entity_delete_selected(edit);

	if(input->mode == BAM_DRAW)
	{
		r_primitive_line_2d(-8.0, -8.0, 8.0, -8.0, 0.0, 0.0, 0.0, 1.1);
		r_primitive_line_2d(-8.0, 8.0, 8.0,  8.0, 0.0, 0.0, 0.0, 1.1);
		r_primitive_line_2d(-8.0, -8.0, -8.0, 8.0, 0.0, 0.0, 0.0, 1.1);
		r_primitive_line_2d(8.0, -8.0, 8.0, 8.0, 0.0, 0.0, 0.0, 1.1);

		r_primitive_line_2d(-8.0, 0.0, 8.0, 0.0, 0.1, 0.1, 0.1, 0.1);
		r_primitive_line_2d(0.0, -8.0, 0.0, 8.0, 0.1, 0.1, 0.1, 0.1);

		for(i = 0; i < input->pointer_count && i < 16; i++)
		{
			if(edit->state[i] == HXA_2DEES_SELECT)
			{
				double camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 0, 1}, start[3], end[3];
				seduce_view_camera_getd(NULL, camera);
				seduce_view_camera_vector_getd(NULL, vec, (double)input->pointers[i].click_pointer_x[0], (double)input->pointers[i].click_pointer_y[0]);
				f_project3d(start, center, normal, camera, vec);
				seduce_view_camera_vector_getd(NULL, vec, (double)input->pointers[i].pointer_x, (double)input->pointers[i].pointer_y);
				f_project3d(end, center, normal, camera, vec);
		
			//	r_matrix_projection_surfaced(NULL, start, center, 2, (double)input->pointers[i].click_pointer_x[0], (double)input->pointers[i].click_pointer_y[0]);
			//	r_matrix_projection_surfaced(NULL, end, center, 2, (double)input->pointers[i].pointer_x, (double)input->pointers[i].pointer_y);
				
				r_primitive_line_2d(start[0], start[1], start[0], end[1], 0, 0, 0, 0.3);
				r_primitive_line_2d(end[0], start[1], end[0], end[1], 0, 0, 0, 0.3);
				r_primitive_line_2d(start[0], start[1], end[0], start[1], 0, 0, 0, 0.3);
				r_primitive_line_2d(start[0], end[1], end[0], end[1], 0, 0, 0, 0.3);
				r_primitive_line_3d(start[0],start[1], 0.02 * scalef, start[0], end[1], 0.02 * scalef, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(end[0], start[1], 0.02 * scalef, end[0], end[1], 0.02 * scalef, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(start[0], start[1], 0.02 * scalef, end[0], start[1], 0.02 * scalef, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(start[0], end[1], 0.02 * scalef, end[0], end[1], 0.02 * scalef, 0.2, 0.6, 1.0, 1);
			}
			if(edit->state[i] == HXA_2DEES_ADD_SHAPE)
			{
				double camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 0, 1}, placement[3], size;
				seduce_view_camera_getd(NULL, camera);
				seduce_view_camera_vector_getd(NULL, vec, (double)input->pointers[i].pointer_x, (double)input->pointers[i].pointer_y);
				f_project3d(placement, center, normal, camera, vec);
				size = scale * 0.1;
			//	r_matrix_projection_surfaced(NULL, start, center, 2, (double)input->pointers[i].click_pointer_x[0], (double)input->pointers[i].click_pointer_y[0]);
			//	r_matrix_projection_surfaced(NULL, end, center, 2, (double)input->pointers[i].pointer_x, (double)input->pointers[i].pointer_y);
				
				r_primitive_line_2d(placement[0] - size, placement[1] - size, placement[0] - size, placement[1] + size, 0, 0, 0, 0.3);
				r_primitive_line_2d(placement[0] + size, placement[1] - size, placement[0] + size, placement[1] + size, 0, 0, 0, 0.3);
				r_primitive_line_2d(placement[0] - size, placement[1] - size, placement[0] + size, placement[1] - size, 0, 0, 0, 0.3);
				r_primitive_line_2d(placement[0] - size, placement[1] + size, placement[0] + size, placement[1] + size, 0, 0, 0, 0.3);

				r_primitive_line_3d(placement[0] - size, placement[1] - size, 0.02 * scalef, placement[0] - size, placement[1] + size, 0.02 * scalef, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(placement[0] + size, placement[1] - size, 0.02 * scalef, placement[0] + size, placement[1] + size, 0.02 * scalef, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(placement[0] - size, placement[1] - size, 0.02 * scalef, placement[0] + size, placement[1] - size, 0.02 * scalef, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(placement[0] - size, placement[1] + size, 0.02 * scalef, placement[0] + size, placement[1] + size, 0.02 * scalef, 0.2, 0.6, 1.0, 1);
			}
		}		


			

		//	free(level->loops[i].triangle_array);
		//	r_array_free(level->loops[i].pool);
	
		for(i = 0; i < level->loop_count; i++)
		{
			if(level->loops[i].triangle_array == NULL)
			{
				RFormats vertex_format_types = R_FLOAT;
				uint vertex_format_size = 3; // BUG ???
				level->loops[i].triangle_array = malloc((sizeof *level->loops[i].triangle_array) * (level->loops[i].loop_size - 2) * 3 * 3);
				mo_menu_edit_loop_polygonize(level->loops[i].loop, level->loops[i].loop_size, level->loops[i].triangle_array);
			}
		}

		if(draw_shape)
			hxa_2d_editor_draw_surface(input, edit, material_func, user);




		mo_menu_edit_draw_level_guides(input, level, scalef);
	


	// 	for(j = 0; j < MO_LST_COUNT; j++)
			for(i = 0; i < level->loop_count; i++)
	//			if(level->loops[i].type == depth_order[j]/* && !level->loops[i].selected*/)
					for(k = 0; k < level->loops[i].loop_size - 2; k++)
						seduce_element_add_triangle(input, level->loops[i].matrix, 0, 
														&level->loops[i].triangle_array[k * 9], 
														&level->loops[i].triangle_array[k * 9 + 3], 
														&level->loops[i].triangle_array[k * 9 + 6]);
		for(i = 0; i < level->entity_count; i++)
		{
			size = 0.5;
			af[0] = level->entity[i].pos[0];
			af[1] = level->entity[i].pos[1];
			af[2] = 0;
			seduce_element_add_point(input, level->entity[i].pos, 0, af, 0.02);
			if(seduce_element_active(input, level->entity[i].pos, NULL))
				size += 0;
			seduce_tool_tip(input, level->entity[i].pos, level->entity[i].name, NULL);
			if(!level->entity[i].selected)
			{
				r_primitive_line_3d(level->entity[i].pos[0] - 0.01, level->entity[i].pos[1] - 0.01, 0.0, level->entity[i].pos[0] + 0.01, level->entity[i].pos[1] - 0.01, 0.0, 0.1, 0.3, 0.5, 0.5);
				r_primitive_line_3d(level->entity[i].pos[0] - 0.01, level->entity[i].pos[1] + 0.01, 0.0, level->entity[i].pos[0] + 0.01, level->entity[i].pos[1] + 0.01, 0.0, 0.1, 0.3, 0.5, 0.5);
				r_primitive_line_3d(level->entity[i].pos[0] - 0.01, level->entity[i].pos[1] - 0.01, 0.0, level->entity[i].pos[0] - 0.01, level->entity[i].pos[1] + 0.01, 0.0, 0.1, 0.3, 0.5, 0.5);
				r_primitive_line_3d(level->entity[i].pos[0] + 0.01, level->entity[i].pos[1] - 0.01, 0.0, level->entity[i].pos[0] + 0.01, level->entity[i].pos[1] + 0.01, 0.0, 0.1, 0.3, 0.5, 0.5);
			}else
			{
				r_primitive_line_3d(level->entity[i].pos[0] - 0.01, level->entity[i].pos[1] - 0.01, 0.0, level->entity[i].pos[0] + 0.01, level->entity[i].pos[1] - 0.01, 0.0, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(level->entity[i].pos[0] - 0.01, level->entity[i].pos[1] + 0.01, 0.0, level->entity[i].pos[0] + 0.01, level->entity[i].pos[1] + 0.01, 0.0, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(level->entity[i].pos[0] - 0.01, level->entity[i].pos[1] - 0.01, 0.0, level->entity[i].pos[0] - 0.01, level->entity[i].pos[1] + 0.01, 0.0, 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(level->entity[i].pos[0] + 0.01, level->entity[i].pos[1] - 0.01, 0.0, level->entity[i].pos[0] + 0.01, level->entity[i].pos[1] + 0.01, 0.0, 0.2, 0.6, 1.0, 1);


			//	r_primitive_line_3d(level->entity[i].pos[0] - 0.08, level->entity[i].pos[1], 0.0, level->entity[i].pos[0] + 0.08, level->entity[i].pos[1], 0.0, 0.2, 0.6, 1.0, 1);
			//	r_primitive_line_3d(level->entity[i].pos[0], level->entity[i].pos[1] - 0.08, 0.0, level->entity[i].pos[0], level->entity[i].pos[1] + 0.08, 0.0, 0.2, 0.6, 1.0, 1);
			}				
		}


		for(i = 0; i < level->loop_count; i++)
		{
			array = level->loops[i].loop;
			size = level->loops[i].loop_size * 2;

			if(level->loop_selected == i)
				selection = 1.0;
			else
				selection = 0.3;

			for(j = 0; j < input->pointer_count; j++)
				if(level->loops[i].matrix == seduce_element_pointer_id(input, j, NULL))
					selection += 0.3;

			
			for(j = 0; j < input->pointer_count && j < 16; j++)
			{
				if(edit->state[j] == HXA_2DEES_SELECTION)
				{
					double pointer[3], *p, f;
					hxa_2d_editor_edit_guideline_snap(level, pointer, input->pointers[j].pointer_x, input->pointers[j].pointer_y, scale, FALSE);
					p = edit->grab[j];
					pointer[0] -= p[0];
					pointer[1] -= p[1];
					for(k = 0; k < level->loops[i].loop_size * 2; k++)
						if(level->loops[i].selection[k / 2])
							level->loops[i].loop[k] += pointer[k % 2];
					if(!hxa_2d_editor_edit_loop_valid_test(level->loops[j].loop, level->loops[j].loop_size))
					{
						f = sin(input->minute_time * 720) * 0.3 + 0.7;
						for(k = 0; k < level->loops[j].loop_size; k++)
							r_primitive_line_3d(level->loops[j].loop[k * 2], 
												level->loops[j].loop[k * 2 + 1], 0.05, 
												level->loops[j].loop[((k + 1) % level->loops[j].loop_size) * 2], 
												level->loops[j].loop[((k + 1) % level->loops[j].loop_size) * 2 + 1], 0.05,
												1.0, 0.0, 0.2, f);

					}
					for(k = 0; k < level->loops[i].loop_size * 2; k++)
						if(level->loops[i].selection[k / 2])
							level->loops[i].loop[k] -= pointer[k % 2];
				}
			}

			center[0] = center[1] = 0;
			for(j = 0; j < size; j += 2)
			{
				scalef = scale;
				af[0] = (float)array[j];
				af[1] = (float)array[j + 1];
		
				seduce_element_add_point(input, &array[j], 0, af, 0.02 * scalef);
				k = (j + 2) % size;
				af[0] = (float)array[j] - (float)array[k];
				af[1] = (float)array[j + 1] - (float)array[k + 1];
				f = sqrt(af[0] * af[0] + af[1] * af[1]) * 2;

				f2 = (0.02 * scalef) / f;
				if(f2 > 0.5)
					f2 = 0.5;

				af[0] = (float)array[j] * (1 - f2) + (float)array[k] * f2;
				af[1] = (float)array[j + 1] * (1 - f2) + (float)array[k + 1] * f2;
				if(!level->loops[i].selection[j / 2])
				{
					if(&array[j] == seduce_element_pointer_id(input, 0, NULL))
						r_primitive_line_3d((float)array[j], (float)array[j + 1], 0.0, af[0], af[1], 0.0, 0.2, 0.6, 1.0, 1.0);
					else
						r_primitive_line_3d((float)array[j], (float)array[j + 1], 0.0, af[0], af[1], 0.0, 1, 1, 1, selection);
				}
				bf[0] = (float)array[j] * f2 + (float)array[k] * (1 - f2);
				bf[1] = (float)array[j + 1] * f2 + (float)array[k + 1] * (1 - f2);
				if(f > (0.005 * scalef) * 4)
					seduce_element_add_line(input, &array[j + 1], 0, af, bf, 0.02 * scalef);
				if(&array[j + 1] == seduce_element_pointer_id(input, 0, NULL))
					r_primitive_line_3d(af[0], af[1], 0.0, bf[0], bf[1], 0.0, 0.2, 0.6, 1.0, 1.0);
				else
					r_primitive_line_3d(af[0], af[1], 0.0, bf[0], bf[1], 0, 1, 1, 1, selection);
				if(level->loops[i].selection[k / 2])
				{
				//	r_primitive_line_fade_3d(array[k], array[k + 1], 0.0, array[k], array[k + 1], 0.1, 0.2, 0.6, 1.0, 1.0, 0.2, 0.6, 1.0, 0.0);
					f = 0.005 * scalef;
					r_primitive_line_3d((float)array[k] + f, (float)array[k + 1] + f, f, (float)array[k] + f, (float)array[k + 1] - f, f, 0.2, 0.6, 1.0, 1.0);
					r_primitive_line_3d((float)array[k] - f, (float)array[k + 1] + f, f, (float)array[k] - f, (float)array[k + 1] - f, f, 0.2, 0.6, 1.0, 1.0);
					r_primitive_line_3d((float)array[k] + f, (float)array[k + 1] + f, f, (float)array[k] - f, (float)array[k + 1] + f, f, 0.2, 0.6, 1.0, 1.0);
					r_primitive_line_3d((float)array[k] + f, (float)array[k + 1] - f, f, (float)array[k] - f, (float)array[k + 1] - f, f, 0.2, 0.6, 1.0, 1.0);
				}
				if(!level->loops[i].selection[k / 2])
				{
					if(&array[k] == seduce_element_pointer_id(input, 0, NULL))
						r_primitive_line_3d((float)array[k], (float)array[k + 1], 0.0, bf[0], bf[1], 0.0, 0.2, 0.6, 1.0, 1.0);
					else
						r_primitive_line_3d((float)array[k], (float)array[k + 1], 0.0, bf[0], bf[1], 0.0, 1, 1, 1, selection);
				}
			}
			r_primitive_line_flush();
		}
	}else
	{
	/*	for(i = 0; i < level->loop_count; i++)
		{
			if(level->loops[i].loop_allocated <= level->loops[i].loop_size + 1)
			{
				level->loops[i].loop_allocated += 1;
				level->loops[i].loop = realloc(level->loops[i].loop, (sizeof *level->loops[i].loop) * level->loops[i].loop_allocated * 2);
				level->loops[i].selection = realloc(level->loops[i].selection, (sizeof *level->loops[i].loop) * level->loops[i].loop_allocated);
			}
		}*/
		if(input->mode == BAM_EVENT)
		{
			for(i = 0; i < input->pointer_count && i < 16; i++)
			{
				if(edit->state[i] == HXA_2DEES_ADD_SHAPE)
				{
					if(input->pointers[i].button[0] && !input->pointers[i].last_button[0] ||
					   input->pointers[i].button[2] && !input->pointers[i].last_button[2])
					{
						double camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 0, 1}, placement[3], size;
						seduce_view_camera_getd(NULL, camera);
						seduce_view_camera_vector_getd(NULL, vec, (double)input->pointers[i].pointer_x, (double)input->pointers[i].pointer_y);
						f_project3d(placement, center, normal, camera, vec);
						level = hxa_2d_editor_structure_instance_add(edit);
						hxa_2d_editor_structure_add_loop(level, edit->material_create, placement[0], placement[1], scale * 0.1, 4);
					}
					if((input->pointers[i].button[0] && !input->pointers[i].last_button[0]) ||
						(input->pointers[i].button[1] && !input->pointers[i].last_button[1]))
						edit->state[i] = HXA_2DEES_IDLE;

				}else if(edit->state[i] == HXA_2DEES_ADD_ENTITY)
				{
					if(input->pointers[i].button[0] && !input->pointers[i].last_button[0] ||
					   input->pointers[i].button[2] && !input->pointers[i].last_button[2])
					{
						double camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 0, 1}, placement[3], size;
						seduce_view_camera_getd(NULL, camera);
						seduce_view_camera_vector_getd(NULL, vec, (double)input->pointers[i].pointer_x, (double)input->pointers[i].pointer_y);
						f_project3d(placement, center, normal, camera, vec);
						level = hxa_2d_editor_structure_instance_add(edit);
						hxa_2d_editor_structure_entity_add(level, placement[0], placement[1], &edit->node_create, edit->node_name);
					}
					if((input->pointers[i].button[0] && !input->pointers[i].last_button[0]) ||
						(input->pointers[i].button[1] && !input->pointers[i].last_button[1]))
						edit->state[i] = HXA_2DEES_IDLE;

				}else if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(level->manip_start == seduce_element_pointer_id(input, i, NULL))
					{				
						level = hxa_2d_editor_structure_instance_add(edit);
						mo_menu_edit_unselect(level);
						edit->state[i] = HXA_2DEES_MANIPULATOR;
						edit->grab[i] = level->manip_start;
					}else if(level->manip_end == seduce_element_pointer_id(input, i, NULL))
					{
						level = hxa_2d_editor_structure_instance_add(edit);
						mo_menu_edit_unselect(level);
						edit->state[i] = HXA_2DEES_MANIPULATOR;
						edit->grab[i] = level->manip_end;
					}else
					{
						for(j = 0; j < level->loop_count; j++)
						{
							if(level->loops[j].matrix == seduce_element_pointer_id(input, i, NULL))
							{
								level = hxa_2d_editor_structure_instance_add(edit);
								edit->state[i] = HXA_2DEES_SURFACE;
								edit->grab[i] = level->loops[j].matrix;
								seduce_element_pointer_id(input, i, NULL);
								break;
							}           
						}
						if(j == level->loop_count)
						{
							for(j = 0; j < level->entity_count; j++)
							{
								if(level->entity[j].pos == seduce_element_pointer_id(input, i, NULL))
								{
									level = hxa_2d_editor_structure_instance_add(edit);
									edit->state[i] = HXA_2DEES_SELECTION;
									edit->grab[i] = &level->entity[j].pos;
									if(!level->entity[j].selected)
									{
										if(!shift)
											mo_menu_edit_unselect(level);
										level->entity[j].selected = TRUE;
									}
									break;
								}
							}
							if(j == level->entity_count)
							{
								for(j = 0; j < level->loop_count; j++)
								{
									for(k = 0; k < level->loops[j].loop_size; k++)
									{
										if(&level->loops[j].loop[k * 2 + 1] == seduce_element_pointer_id(input, i, NULL))
										{
											level = hxa_2d_editor_structure_instance_add(edit);
											size = level->loops[j].loop_size * 2;
											level->loops[j].loop_size++;
											if(!shift)
												mo_menu_edit_unselect(level);
											for(l = 0; l < size - k * 2; l++)
												level->loops[j].loop[size - l + 1] = level->loops[j].loop[size - l - 1];
											level->loops[j].selection[k + 1] = TRUE;
											level->loops[j].loop[k * 2 + 2] = (level->loops[j].loop[k * 2 + 2] + level->loops[j].loop[((k + 2) % level->loops[j].loop_size) * 2 + 0]) * 0.5;
											level->loops[j].loop[k * 2 + 3] = (level->loops[j].loop[k * 2 + 3] + level->loops[j].loop[((k + 2) % level->loops[j].loop_size) * 2 + 1]) * 0.5;
											edit->state[i] = HXA_2DEES_SELECTION;
											edit->grab[i] = &level->loops[j].loop[k * 2 + 2];
										}
										if(&level->loops[j].loop[k * 2] == seduce_element_pointer_id(input, i, NULL))
										{
											level = hxa_2d_editor_structure_instance_add(edit);
											if(!level->loops[j].selection[k])
											{
												if(!shift)
													mo_menu_edit_unselect(level);
												level->loops[j].selection[k] = TRUE;
												level->loop_selected = j;
											}
											edit->state[i] = HXA_2DEES_SELECTION;
											edit->grab[i] = &level->loops[j].loop[k * 2];
										}
									}
								}
							}
						}
						if(NULL == seduce_element_pointer_id(input, i, NULL))
							edit->state[i] = HXA_2DEES_EMPTY;
					}
				}
				if(input->pointers[i].button[0])
				{
					if(edit->state[i] == HXA_2DEES_SURFACE || edit->state[i] == HXA_2DEES_EMPTY)
					{
						if((input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) * 
							(input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) +
							(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) * 
							(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) > 0.01 * 0.01)
							edit->state[i] = HXA_2DEES_SELECT;
					}
					if(edit->state[i] == HXA_2DEES_SELECT)
					{
						double camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 0, 1}, start[3], end[3];
						double min[2], max[2];
						seduce_view_camera_getd(NULL, camera);
						seduce_view_camera_vector_getd(NULL, vec, input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0]);
						f_project3d(start, center, normal, camera, vec);
						seduce_view_camera_vector_getd(NULL, vec, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
						f_project3d(end, center, normal, camera, vec);
					//	r_matrix_projection_surfacef(NULL, start, center, 1, input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0]);
					//	r_matrix_projection_surfacef(NULL, end, center, 1, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
						if(start[0] > end[0])
						{
							min[0] = end[0];
							max[0] = start[0];
						}else
						{
							min[0] = start[0];
							max[0] = end[0];
						}
						if(start[1] > end[1])
						{
							min[1] = end[1];
							max[1] = start[1];
						}else
						{
							min[1] = start[1];
							max[1] = end[1];
						}
						for(j = 0; j < level->loop_count; j++)
						{
							for(k = 0; k < level->loops[j].loop_size; k++)
							{
								if(!shift)
									level->loops[j].selection[k] = level->loops[j].loop[k * 2 + 0] > min[0] && 
																	level->loops[j].loop[k * 2 + 1] > min[1] && 
																	level->loops[j].loop[k * 2 + 0] < max[0] && 
																	level->loops[j].loop[k * 2 + 1] < max[1];
								else
									if(level->loops[j].loop[k * 2 + 0] > min[0] && 
										level->loops[j].loop[k * 2 + 1] > min[1] && 
										level->loops[j].loop[k * 2 + 0] < max[0] && 
										level->loops[j].loop[k * 2 + 1] < max[1])
											level->loops[j].selection[k] = TRUE;
							}
						}
						
						for(j = 0; j < level->entity_count; j++)
						{
							if(!shift)
								level->entity[j].selected = level->entity[j].pos[0] > min[0] && 
															level->entity[j].pos[1] > min[1] && 
															level->entity[j].pos[0] < max[0] && 
															level->entity[j].pos[1] < max[1];
							else
								if(level->entity[j].pos[0] > min[0] && 
									level->entity[j].pos[1] > min[1] && 
									level->entity[j].pos[0] < max[0] && 
									level->entity[j].pos[1] < max[1])
										level->entity[j].selected = TRUE;
						}
					}
					if(edit->state[i] == HXA_2DEES_MANIPULATOR)
					{
						double pos[3];
						hxa_2d_editor_edit_guideline_snap(level, pos, input->pointers[i].pointer_x, input->pointers[i].pointer_y, scale, TRUE);
						((double *)edit->grab[i])[0] = pos[0];
						((double *)edit->grab[i])[1] = pos[1];
					}


					if(edit->state[i] == HXA_2DEES_SELECTION)
					{
						boolean update;
						double pointer[3], *p;
						hxa_2d_editor_edit_guideline_snap(level, pointer, input->pointers[i].pointer_x, input->pointers[i].pointer_y, scale, FALSE);
						p = edit->grab[i];
						pointer[0] -= p[0];
						pointer[1] -= p[1];
						for(k = 0; k < level->entity_count; k++)
						{
							if(level->entity[k].selected) 
							{
								level->entity[k].pos[0] += pointer[0];
								level->entity[k].pos[1] += pointer[1];
							}
						}

						for(j = 0; j < level->loop_count; j++)
						{
							update = FALSE;
							for(k = 0; k < level->loops[j].loop_size; k++)
							{
								if(level->loops[j].selection[k])
								{
									level->loops[j].loop[k * 2 + 0] += pointer[0];
									level->loops[j].loop[k * 2 + 1] += pointer[1];
									update = TRUE;
								}
							}
							if(!hxa_2d_editor_edit_loop_valid_test(level->loops[j].loop, level->loops[j].loop_size))
							{
								for(k = 0; k < level->loops[j].loop_size; k++)
								{
									if(level->loops[j].selection[k])
									{
										level->loops[j].loop[k * 2 + 0] -= pointer[0];
										level->loops[j].loop[k * 2 + 1] -= pointer[1];
									}
								}
							}
							if(update)
							{
								if(level->loops[j].triangle_array != NULL)
									free(level->loops[j].triangle_array);
								level->loops[j].triangle_array = NULL;
								if(level->loops[j].pool != NULL)
									r_array_free(level->loops[j].pool);
								level->loops[j].pool = NULL;
							}
						}
					}
				}
				if(edit->state[i] == HXA_2DEES_SURFACE && !input->pointers[i].button[0])
				{
					for(j = 0; j < level->loop_count; j++)
						if(edit->grab[i] == level->loops[j].matrix)
							level->loop_selected = j;
					for(j = 0; j < level->loop_count && edit->grab[i] != level->loops[j].matrix; j++);
					if(j < level->loop_count)
					{
						for(k = 0; k < level->loops[j].loop_size && !level->loops[j].selection[k]; k++);
						if(k < level->loops[j].loop_size)
						{
							if(!shift)
								mo_menu_edit_unselect(level);
						}else
						{
							if(!shift)
								mo_menu_edit_unselect(level);
							for(j = 0; j < level->loop_count; j++)
							{
								if(!shift)
								{
									for(k = 0; k < level->loops[j].loop_size; k++)
										level->loops[j].selection[k] = edit->grab[i] == level->loops[j].matrix;
								}else for(k = 0; k < level->loops[j].loop_size; k++)
										if(edit->grab[i] == level->loops[j].matrix)
											level->loops[j].selection[k] = TRUE;
								if(edit->grab[i] == level->loops[j].matrix)
								{
									double matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
									for(k = 0; k < level->entity_count; k++)
										level->entity[k].selected = mo_menu_edit_loop_colission_test(level->loops[j].loop, level->loops[j].loop_size, matrix, level->entity[k].pos);
								}
							}
						}
					}
				}
				if(edit->state[i] == HXA_2DEES_EMPTY && !input->pointers[i].button[0])
				{
					level->loop_selected = -1;
					if(!shift)
						mo_menu_edit_unselect(level);

				}
				if(edit->state[i] == HXA_2DEES_SELECTION && !input->pointers[i].button[0])
				{
				/*	double matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
					for(j = 0; j < level->entity_count; j++)
					{
						for(k = 0; k < level->loop_count; k++)
							if(mo_menu_edit_loop_colission_test(level->loops[k].loop, level->loops[k].loop_size, matrix, level->entity[j].pos))
								break;
						if(k == level->loop_count)
							level->entity[j--] = level->entity[--level->entity_count];
					}*/
				}

				if(!input->pointers[i].button[0] && edit->state[i] != HXA_2DEES_ADD_SHAPE && edit->state[i] != HXA_2DEES_ADD_ENTITY)
					edit->state[i] = HXA_2DEES_IDLE;
			}
		}
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < 16 && edit->state[i] == HXA_2DEES_IDLE; i++);
		if(i < 16)
			edit->updated = TRUE;
	}
	hxa_2d_editor_manipulator(input, edit, level, scale);
	if(level->loop_selected < level->loop_count)
	{
		double best = 100000.0, vec[2], *p, *p2;
		uint found = ~0;
		i = level->loop_selected;
		for(j = 0; j < level->loops[i].loop_size; j++)
		{
			p = &level->loops[i].loop[j * 2];
			p2 = &level->loops[i].loop[((j + 1) % level->loops[i].loop_size) * 2];
			d = p[0] - p2[0]; 
			if(d < best)
			{
				vec[0] = p[0] - p2[0];
				vec[1] = p[1] - p2[1];
				for(k = 0; k < level->loops[i].loop_size; k++)
				{
					p = &level->loops[i].loop[k * 2];
					if((p[0] - p2[0]) * vec[1] - (p[1] - p2[1]) * vec[0] > 0.01)
						if((p[0] - p2[0]) * vec[0] + (p[1] - p2[1]) * vec[1] > 0.01)
							break;
				}
				if(k == level->loops[i].loop_size)
				{
					found = j;
					best = d;
				}
			}
		}

		if(found != ~0)
		{
			p = &level->loops[i].loop[found * 2];
			vec[0] = level->loops[i].loop[found * 2] - level->loops[i].loop[((found + 1) % level->loops[i].loop_size) * 2];
			vec[1] = level->loops[i].loop[found * 2 + 1] - level->loops[i].loop[((found + 1) % level->loops[i].loop_size) * 2 + 1];
			f_normalize2d(vec);
			vec[0] *= scale * 0.4;
			vec[1] *= scale * 0.4;
			mo_menu_edit_draw_loop_menu(input, edit, level, i, (float)p[0], (float)p[1], (float)vec[0], (float)vec[1], material_func, user,  scalef);
		}
	}

	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < level->loop_count; i++)
		{
			for(j = 0; j < level->loops[i].loop_size; j++)
			{
				if(level->loops[i].selection[j])
				{
					double pos[3], vec1[2], vec2[2];
					vec1[0] = level->loops[i].loop[j * 2] - level->loops[i].loop[((j + 1) % level->loops[i].loop_size) * 2];
					vec1[1] = level->loops[i].loop[j * 2 + 1] - level->loops[i].loop[((j + 1) % level->loops[i].loop_size) * 2 + 1];
					vec2[0] = level->loops[i].loop[((j + level->loops[i].loop_size - 1) % level->loops[i].loop_size) * 2] - level->loops[i].loop[j * 2];
					vec2[1] = level->loops[i].loop[((j + level->loops[i].loop_size - 1) % level->loops[i].loop_size) * 2 + 1] - level->loops[i].loop[j * 2 + 1];
					f_normalize2d(vec1);
					f_normalize2d(vec2);
					pos[0] = vec1[1] + vec2[1];
					pos[1] = -vec2[0] + -vec1[0];
					f_normalize2d(pos);
					pos[0] = level->loops[i].loop[j * 2] + pos[0] * 0.03 * scale;
					pos[1] = level->loops[i].loop[j * 2 + 1] + pos[1] * 0.03 * scale;
					mo_menu_edit_draw_delete(input, &((uint8*)&level->loops[i].loop[j * 2])[1], (float)pos[0], (float)pos[1], scale);
				//	seduce_element_add_point(input, &((uint8*)&level->loops[i].loop[j * 2])[1], 0, pos, 0.01);			
					seduce_tool_tip(input, &((uint8*)&level->loops[i].loop[j * 2])[1], seduce_translate("Delete corner."), NULL);
				}
			}
		}
		r_primitive_line_flush();
	}

	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
		{
			for(j = 0; j < level->loop_count; j++)
			{
				for(k = 0; k < level->loops[j].loop_size; k++)
				{
					if(&((uint8*)&level->loops[j].loop[k * 2])[1] == seduce_element_pointer_id(input, i, NULL))
					{
						level = hxa_2d_editor_structure_instance_add(edit);
						level->loops[j].loop_size--;
						for(i = k; i < level->loops[j].loop_size; i++)
						{
							level->loops[j].loop[i * 2] = level->loops[j].loop[i * 2 + 2];
							level->loops[j].loop[i * 2 + 1] = level->loops[j].loop[i * 2 + 3];
						}
						if(level->loops[j].triangle_array != NULL)
							free(level->loops[j].triangle_array);
						level->loops[j].triangle_array = NULL;
						if(level->loops[j].pool != NULL)
							r_array_free(level->loops[j].pool);
						level->loops[j].pool = NULL;
						return;
					}
				}
			}
		}
	}
}

void hxa_2d_editor_edit_add_new_shape(HxA2DEditorShape *edit, uint32 material)
{
	HxA2DEditorInstance *instance;
	uint i, j;
	boolean found = FALSE;

	instance = &edit->instances[edit->instance_current];

	for(i = 0; i < instance->loop_count; i++)
	{

		for(j = 0; j < instance->loops[i].loop_size && instance->loops[i].selection[j]; j++);
		if(instance->loop_selected == i || j == instance->loops[i].loop_size)
		{
			if(found == FALSE)
				instance = hxa_2d_editor_structure_instance_add(edit);
			instance->loops[i].material = material;
			found = TRUE;
			
		}
	}
	if(!found)
	{
		edit->state[0] = HXA_2DEES_ADD_SHAPE;
		edit->material_create = material;
	}
}


void hxa_2d_editor_edit_add_new_entity(HxA2DEditorShape *edit, HXANode *node, char *name)
{
	HxA2DEditorInstance *instance;
	uint i;
/*	if(edit->instances[edit->instance_current].loop_selected < edit->instances[edit->instance_current].loop_count)
	{
		instance = hxa_2d_editor_structure_instance_add(edit);
		instance->loops[instance->loop_selected].material = material;
	}else*/
	edit->state[0] = HXA_2DEES_ADD_ENTITY;
	for(i = 0; name[i] != '\0' && i < 32 - 1; i++)		
		edit->node_name[i] = name[i];
	edit->node_name[i] = '\0';
	
	if(edit->node_create.type != ~0)
		hxa_util_free_node_content(&edit->node_create);
	hxa_util_node_clone_content(&edit->node_create, node);
}


void hxa_2d_editor_hxa_node_load_entity(HxA2DEditorShape *edit, HXANode *node, float pos_x, float pos_y, char *name)
{
	HxA2DEditorInstance *level;
	level = hxa_2d_editor_structure_instance_add(edit);
	hxa_2d_editor_structure_entity_add(level, pos_x, pos_y, node, name);
}