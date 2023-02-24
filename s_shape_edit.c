#include <math.h>
#include <stdlib.h>
#include <stdio.h> 
#include "betray.h"
#include "imagine.h"
#include "seduce.h"
#include "originate.h"
#include "s_draw_3d.h"

#define SEDUCE_SHAPE_MIN_EDGE_SIZE 0.02
#define SEDUCE_SHAPE_EDITOR_SNAP 0.00001
#define SEDUCE_SHAPE_COS_30_DEGREES 0.86602540378443864676372317075294

typedef enum{
	MO_EMM_STRETCH,
	MO_EMM_SCALE,
	MO_EMM_ROTATE,
	MO_EMM_ROTATE_AND_STRETCH,
	MO_EMM_ROTATE_AND_SCALE,
	MO_EMM_MIRROR,
	MO_EMM_RULER,
	MO_EMM_GRID,
	MO_EMM_CIRCLE,
	MO_EMM_OFF,
	MO_EMM_COUNT
}SShapeEditManipulatorMode;

typedef struct{
	float *loop;
	uint loop_size;
	uint loop_allocated;
	boolean *selection;
	float matrix[16];
	float rotate;
	float scale;
	uint user_type;
	void *user_pointer;
	float *triangle_array;
	void *pool;
}SShapeLoop;

typedef struct{
	SShapeLoop *loops;
	uint loop_count;
	uint loop_allocated;
	uint loop_selected;
	SShapeEditManipulatorMode manip_mode;
	float manip_start[2];
	float manip_end[2];
	float manip_divisions;
	RMatrix matrix;
}SShape;


SShape *seduce_shape_create()
{
	SShape *shape;
	shape = malloc(sizeof *shape);
	shape->loops = NULL;
	shape->loop_count = 0;
	shape->loop_allocated = 0;
	shape->loop_selected = 0;
	shape->manip_mode = MO_EMM_OFF;
	shape->manip_start[0] = -0.5;
	shape->manip_start[1] = 0;
	shape->manip_end[0] = 0.5;
	shape->manip_end[1] = 0;
	shape->manip_divisions = 10;
	return shape;
}

void seduce_shape_destroy(SShape *shape)
{
	uint i;
	for(i = 0; i < shape->loop_count; i++)
		free(shape->loops[i].loop);
	free(shape->loops);
	free(shape);
}

void seduce_shape_loop_count(SShape *shape)
{
	return shape->loop_count;
}


void seduce_shape_unselect(SShape *shape)
{
	uint i, j;
	for(i = 0; i < shape->loop_count; i++)
		for(j = 0; j < shape->loops[i].loop_size; j++)
			shape->loops[i].selection[j] = FALSE;
}


boolean seduce_shape_line_segment_intersect_test(float *a, float *a2, float *b, float *b2)
{
	if(((a[1] - a2[1]) * (b[0] - a2[0]) - (a[0] - a2[0]) * (b[1] - a2[1]) > 0) !=
		((a[1] - a2[1]) * (b2[0] - a2[0]) - (a[0] - a2[0]) * (b2[1] - a2[1]) > 0))
		if(((b[1] - b2[1]) * (a[0] - b2[0]) - (b[0] - b2[0]) * (a[1] - b2[1]) > 0) !=
			((b[1] - b2[1]) * (a2[0] - b2[0]) - (b[0] - b2[0]) * (a2[1] - b2[1]) > 0))
			return TRUE;
	return FALSE;
}


boolean seduce_shape_line_segment_colission_test(float *a, float *a2, float *pos)
{
	float vec[2], f;
	vec[0] = a[0] - a2[0];
	vec[1] = a[1] - a2[1];
	if(vec[0] * (pos[0] - a[0]) + vec[1] * (pos[1] - a[1]) < 0 &&
		vec[0] * (pos[0] - a2[0]) + vec[1] * (pos[1] - a2[1]) > 0)
	{
		f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		vec[0] /= f;
		vec[1] /= f;
		
		f = (pos[0] - a2[0]) * vec[1] - (pos[1] - a2[1]) * vec[0];
		if(f < SEDUCE_SHAPE_MIN_EDGE_SIZE && f > -SEDUCE_SHAPE_MIN_EDGE_SIZE)
		{
			return TRUE;	
	//	f = (pos[0] - a[0]) * vec[0] + (pos[0] - a[0]) * vec[0];
		}
	}
	return FALSE;
}

boolean seduce_shape_polygon_backface_test(float *v_a, float *v_b, float *v_c)
{
	float vec[2];
	vec[0] = v_a[0] - v_b[0];
	vec[1] = v_a[1] - v_b[1];
	if(vec[1] * (v_c[0] - v_a[0]) - vec[0] * (v_c[1] - v_a[1]) < SEDUCE_SHAPE_EDITOR_SNAP)
		return TRUE;
	return FALSE;
}

boolean seduce_shape_polygon_colission_test(float *v_a, float *v_b, float *v_c, float *pos)
{
	float vec[2];
	vec[0] = v_a[0] - v_b[0];
	vec[1] = v_a[1] - v_b[1];
	f_normalize2f(vec);
	if(vec[1] * (pos[0] - v_a[0]) - vec[0] * (pos[1] - v_a[1]) < SEDUCE_SHAPE_EDITOR_SNAP)
		return TRUE;
	vec[0] = v_b[0] - v_c[0];
	vec[1] = v_b[1] - v_c[1];
	f_normalize2f(vec);
	if(vec[1] * (pos[0] - v_b[0]) - vec[0] * (pos[1] - v_b[1]) < SEDUCE_SHAPE_EDITOR_SNAP)
		return TRUE;
	vec[0] = v_c[0] - v_a[0];
	vec[1] = v_c[1] - v_a[1];
	f_normalize2f(vec);
	if(vec[1] * (pos[0] - v_c[0]) - vec[0] * (pos[1] - v_c[1]) < SEDUCE_SHAPE_EDITOR_SNAP)
		return TRUE;
	return FALSE;
}

boolean seduce_shape_loop_colission_test(float *loop, uint loop_count, float *matrix, float *pos)
{
	float a[2], b[2];
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

//void mo_edit_loop_polygonize(MoEditProcess *edit, uint *loop, uint size, uint *ref)

void seduce_shape_loop_polygonize(float *array, uint size, float *polygons)
{
	uint i = 0, a = 0, b = 1, c = 2, vertex, count = 2, found[3], output = 0, ref_length = 0, save[2];
	float vec[2], *v, *base, *back, sides[4], f, dist, best, *used;
	
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
				f_normalize2f(vec);

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

		polygons[(count - 2) * 9 + 0] = array[a * 2];
		polygons[(count - 2) * 9 + 1] = 0.0;
		polygons[(count - 2) * 9 + 2] = array[a * 2 + 1];
		polygons[(count - 2) * 9 + 3] = array[b * 2 + 0];
		polygons[(count - 2) * 9 + 4] = 0.0;
		polygons[(count - 2) * 9 + 5] = array[b * 2 + 1];
		polygons[(count - 2) * 9 + 6] = array[c * 2 + 0];
		polygons[(count - 2) * 9 + 7] = 0.0;
		polygons[(count - 2) * 9 + 8] = array[c * 2 + 1];

		used[b] = TRUE;
		b = c;
		c = (c + 1) % size;
		while(used[c])
			c = (c + 1) % size;
		count++;
	}
	free(used);
}

uint seduce_shape_add_loop(SShape *shape, float x, float y, float size, uint user_type, void *user_pointer)
{
	SShapeLoop *l;
	if(shape->loop_count == shape->loop_allocated)
	{
		shape->loop_allocated += 16;
		shape->loops = realloc(shape->loops, (sizeof *shape->loops) * shape->loop_allocated * 2);
	}
	shape->loop_selected = shape->loop_count;
	l = &shape->loops[shape->loop_count++];
	l->loop_allocated = 16;
	l->loop = malloc((sizeof *l->loop) * l->loop_allocated * 2);
	l->selection = malloc((sizeof *l->selection) * l->loop_allocated);
	l->selection[0] = FALSE;
	l->selection[1] = FALSE;
	l->selection[2] = FALSE;
	l->selection[3] = FALSE;
	l->loop[0] = size + x;
	l->loop[1] = size + y;
	l->loop[2] = size + x;
	l->loop[3] = -size + y;
	l->loop[4] = -size + x;
	l->loop[5] = -size + y;
	l->loop[6] = -size + x;
	l->loop[7] = size + y;
	l->loop_size = 4;
	l->user_type = user_type;
	l->user_pointer = user_pointer;
	l->matrix[0] = 1.0;
	l->matrix[1] = 0.0;
	l->matrix[2] = 0.0;
	l->matrix[3] = 0.0;
	l->matrix[4] = 0.0;
	l->matrix[5] = 1.0;
	l->matrix[6] = 0.0;
	l->matrix[7] = 0.0;
	l->matrix[8] = 0.0;
	l->matrix[9] = 0.0;
	l->matrix[10] = 1.0;
	l->matrix[11] = 0.0;
	l->matrix[12] = 0.0;
	l->matrix[13] = 0.0;
	l->matrix[14] = 0.0;
	l->matrix[15] = 1.0;
	l->rotate = 0.5;
	l->scale = 0.5;
	l->triangle_array = NULL;
	l->pool = NULL;
}


void seduce_shape_remove_loop(SShape *shape, uint loop)
{
	uint i;
	free(shape->loops[loop].loop);
	if(shape->loops[loop].triangle_array != NULL)
		free(shape->loops[loop].triangle_array);
	if(shape->loops[loop].pool != NULL)
		r_array_free(shape->loops[loop].pool);
	for(shape->loop_count--; loop < shape->loop_count; loop++)
		shape->loops[loop] = shape->loops[loop + 1];
}


boolean seduce_shape_loop_valid_test(float *array, uint size)
{
	uint i, j, next, jnext;
	float vec[2], f;
	boolean keep_going = FALSE;
	size *= 2;
	for(i = 0; i < size; i++)
		if(array[i] < -2.0 || array[i] > 2.0)
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
				if(f < SEDUCE_SHAPE_MIN_EDGE_SIZE * SEDUCE_SHAPE_MIN_EDGE_SIZE)
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
				if(seduce_shape_line_segment_colission_test(&array[i], &array[next], &array[j]))
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

uint seduce_shape_guideline_snap_edge(RMatrix *matrix, float *pos_a, float *pos_b, float *best, float *vector, float pointer_x, float pointer_y)
{
	float pos1[3], pos2[3], vec[2], f, ff;
	r_matrix_projection_screenf(matrix, pos1, pos_a[0], 0.0, pos_a[1]);
	r_matrix_projection_screenf(matrix, pos2, pos_b[0], 0.0, pos_b[1]);
	if(pos1[2] < 0 && pos2[2] < 0)
	{
		vec[0] = pos1[0] - pos2[0];
		vec[1] = pos1[1] - pos2[1];
		f_normalize2f(vec);
		f = (pointer_x - pos1[0]) * vec[1] - (pointer_y - pos1[1]) * vec[0];
		f *= f;
		if(f < *best)
		{
			vec[0] = pos_a[0] - pos_b[0];
			vec[1] = pos_a[1] - pos_b[1];
			f_normalize2f(vec);
			ff = vec[0] * vector[0] + vec[1] * vector[1];
			if(ff > 0.99 || ff < -0.99)
				return FALSE;
			*best = f;
			return TRUE;
		}
	}
	return FALSE;
}



boolean seduce_shape_guideline_snap_corner(RMatrix *matrix, float *output, float *pos_a, float *pos_b, float *vector, float *best, float pointer_x, float pointer_y)
{
	boolean out = FALSE;
	float pos2[2], vec[2];
	vec[0] = pos_a[0] - pos_b[0];
	vec[1] = pos_a[1] - pos_b[1];

	pos2[0] = pos_a[0] - vec[1];
	pos2[1] = pos_a[1] + vec[0];
	if(seduce_shape_guideline_snap_edge(matrix, pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}

	pos2[0] = pos_a[0] - vec[1] + vec[0];
	pos2[1] = pos_a[1] + vec[0] + vec[1];
	if(seduce_shape_guideline_snap_edge(matrix, pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	pos2[0] = pos_a[0] - vec[1] - vec[0];
	pos2[1] = pos_a[1] + vec[0] - vec[1];
	if(seduce_shape_guideline_snap_edge(matrix, pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	
	pos2[0] = pos_a[0] - vec[1] * 0.5 - vec[0] * SEDUCE_SHAPE_COS_30_DEGREES;
	pos2[1] = pos_a[1] + vec[0] * 0.5 - vec[1] * SEDUCE_SHAPE_COS_30_DEGREES;
	if(seduce_shape_guideline_snap_edge(matrix, pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	pos2[0] = pos_a[0] - vec[1] * 0.5 + vec[0] * SEDUCE_SHAPE_COS_30_DEGREES;
	pos2[1] = pos_a[1] + vec[0] * 0.5 + vec[1] * SEDUCE_SHAPE_COS_30_DEGREES;
	if(seduce_shape_guideline_snap_edge(matrix, pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}

	pos2[0] = pos_a[0] - vec[1] * SEDUCE_SHAPE_COS_30_DEGREES - vec[0] * 0.5;
	pos2[1] = pos_a[1] + vec[0] * SEDUCE_SHAPE_COS_30_DEGREES - vec[1] * 0.5;
	if(seduce_shape_guideline_snap_edge(matrix, pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	pos2[0] = pos_a[0] - vec[1] * SEDUCE_SHAPE_COS_30_DEGREES + vec[0] * 0.5;
	pos2[1] = pos_a[1] + vec[0] * SEDUCE_SHAPE_COS_30_DEGREES + vec[1] * 0.5;
	if(seduce_shape_guideline_snap_edge(matrix, pos_a, pos2, best, vector, pointer_x, pointer_y))
	{
		output[0] = pos_a[0];
		output[1] = pos_a[1];
		output[2] = pos2[0];
		output[3] = pos2[1];
		out = TRUE;
	}
	return out;
}


boolean seduce_shape_guideline_snap_pass(SShape *shape, float *output, float *vector, float pointer_x, float pointer_y, uint manip_mode)
{
	boolean out = FALSE;
	float best = 0.006 * 0.006;
	uint i, j, jj;

	if(manip_mode == MO_EMM_RULER)
	{
		float f, length, vec[2], a[2], b[2];
		vec[0] = shape->manip_end[0] - shape->manip_start[0];
		vec[1] = shape->manip_end[1] - shape->manip_start[1];
		length = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		vec[0] /= length;
		vec[1] /= length;

		if(seduce_shape_guideline_snap_edge(&shape->matrix, shape->manip_start, shape->manip_end, &best, vector, pointer_x, pointer_y))
		{
			output[0] = shape->manip_start[0];
			output[1] = shape->manip_start[1];
			output[2] = shape->manip_end[0];
			output[3] = shape->manip_end[1];
			out = TRUE;
		}
		j = shape->manip_divisions;
		for(i = 0; i < j * 3 + 1; i++)
		{
			f = length * (float)i / (float)j - length;
			a[0] = shape->manip_start[0] + vec[0] * f + vec[1];
			a[1] = shape->manip_start[1] + vec[1] * f - vec[0]; 
			b[0] = shape->manip_start[0] + vec[0] * f;
			b[1] = shape->manip_start[1] + vec[1] * f;

			if(seduce_shape_guideline_snap_edge(&shape->matrix, a, b, &best, vector, pointer_x, pointer_y))
			{
				output[0] = a[0];
				output[1] = a[1];
				output[2] = b[0];
				output[3] = b[1];
				out = TRUE;
			}
		}
	}
	if(shape->manip_mode == MO_EMM_GRID)
	{
		float f, length, vec[2], a[2], b[2];
		vec[0] = shape->manip_end[0] - shape->manip_start[0];
		vec[1] = shape->manip_end[1] - shape->manip_start[1];
		length = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		vec[0] /= length;
		vec[1] /= length;
		j = shape->manip_divisions;
		for(i = 0; i < j * 3 + 1; i++)
		{
			f = length * (float)i / (float)j - length;
			a[0] = shape->manip_start[0] + vec[0] * f + vec[1];
			a[1] = shape->manip_start[1] + vec[1] * f - vec[0]; 
			b[0] = shape->manip_start[0] + vec[0] * f;
			b[1] = shape->manip_start[1] + vec[1] * f;

			if(seduce_shape_guideline_snap_edge(&shape->matrix, a, b, &best, vector, pointer_x, pointer_y))
			{
				output[0] = a[0];
				output[1] = a[1];
				output[2] = b[0];
				output[3] = b[1];
				out = TRUE;
			}
		}
		for(i = 0; i < j * 2 + 1; i++)
		{
			f = length * (float)i / j - length;
			a[0] = shape->manip_start[0] - vec[1] * f + vec[0];
			a[1] = shape->manip_start[1] + vec[0] * f + vec[1]; 
			b[0] = shape->manip_start[0] - vec[1] * f;
			b[1] = shape->manip_start[1] + vec[0] * f;
			if(seduce_shape_guideline_snap_edge(&shape->matrix, a, b, &best, vector, pointer_x, pointer_y))
			{
				output[0] = a[0];
				output[1] = a[1];
				output[2] = b[0];
				output[3] = b[1];
				out = TRUE;
			}
		}	
		return out;
	}

	if(shape->manip_mode == MO_EMM_CIRCLE)
	{
		float center[2], vec[2], circle[2], f, x, y;
		center[0] = (shape->manip_end[0] + shape->manip_start[0]) * 0.5;
		center[1] = (shape->manip_end[1] + shape->manip_start[1]) * 0.5;
		vec[0] = (shape->manip_end[0] - shape->manip_start[0]) * 0.5;
		vec[1] = (shape->manip_end[1] - shape->manip_start[1]) * 0.5;
		j = shape->manip_divisions * 2;
		for(i = 0; i < j; i++)
		{
			f = (float)i / (float)j * 2.0 * PI;
			x = sin(f);
			y = cos(f);
			circle[0] = center[0] + vec[0] * x - vec[1] * y;
			circle[1] = center[1] + vec[1] * x + vec[0] * y;
			if(seduce_shape_guideline_snap_edge(&shape->matrix, center, circle, &best, vector, pointer_x, pointer_y))
			{
				output[0] = center[0];
				output[1] = center[1];
				output[2] = circle[0];
				output[3] = circle[1];
				out = TRUE;
			}
		}
	}

	for(i = 0; i < shape->loop_count; i++)
	{
		for(j = 0; j < shape->loops[i].loop_size; j++)
		{
			jj = (j + 1) % shape->loops[i].loop_size;
			if((!shape->loops[i].selection[j] && !shape->loops[i].selection[jj]))
			{
				if(seduce_shape_guideline_snap_edge(&shape->matrix, &shape->loops[i].loop[j * 2], &shape->loops[i].loop[jj * 2], &best, vector, pointer_x, pointer_y))
				{
					output[0] = shape->loops[i].loop[j * 2];
					output[1] = shape->loops[i].loop[j * 2 + 1];
					output[2] = shape->loops[i].loop[jj * 2];
					output[3] = shape->loops[i].loop[jj * 2 + 1];
					out = TRUE;
				}
				if(shape->loops[i].selection[(j + shape->loops[i].loop_size - 1) % shape->loops[i].loop_size])
					if(seduce_shape_guideline_snap_corner(&shape->matrix, output, &shape->loops[i].loop[j * 2], &shape->loops[i].loop[jj * 2], vector, &best, pointer_x, pointer_y))
						out = TRUE;
				if(shape->loops[i].selection[(j + 2) % shape->loops[i].loop_size])
					if(seduce_shape_guideline_snap_corner(&shape->matrix, output, &shape->loops[i].loop[jj * 2], &shape->loops[i].loop[j * 2], vector, &best, pointer_x, pointer_y))
						out = TRUE;
			}
		}
	}
	return out;
}


void seduce_shape_guidecircle_snap(float *output, float *center, float radius)
{
	float vec[2], f;
	vec[0] = output[0] - center[0];
	vec[1] = output[1] - center[1];
	f = vec[0] * vec[0] + vec[1] * vec[1];
	if(f < (radius + 0.01) * (radius + 0.01) && f > (radius - 0.01) * (radius - 0.01))
	{
		f_normalize2f(vec);
		output[0] = center[0] + vec[0] * radius;
		output[1] = center[1] + vec[1] * radius;
	}
}

void seduce_shape_guidecircle_snap_pass(SShape *level, float *output)
{
	boolean out = FALSE;
	float best = 0.006 * 0.006;
	uint i, j, j_pre, j_post;

	if(level->manip_mode == MO_EMM_CIRCLE)
	{
		float center[2];
		center[0] = (level->manip_end[0] + level->manip_start[0]) * 0.5;
		center[1] = (level->manip_end[1] + level->manip_start[1]) * 0.5;
		seduce_shape_guidecircle_snap(output, center, f_distance2f(level->manip_start, level->manip_end) * 0.5);
	}
	
	for(i = 0; i < level->loop_count; i++)
	{
		for(j = 0; j < level->loops[i].loop_size; j++)
		{
			if(!level->loops[i].selection[j])
			{
				j_pre = (j + level->loops[i].loop_size - 1) % level->loops[i].loop_size;
				j_post = (j + 1) % level->loops[i].loop_size;

				if(!level->loops[i].selection[j_pre] && level->loops[i].selection[j_post])
					seduce_shape_guidecircle_snap(output, &level->loops[i].loop[j * 2], f_distance2f(&level->loops[i].loop[j * 2], &level->loops[i].loop[j_pre * 2]));
				if(level->loops[i].selection[j_pre] && !level->loops[i].selection[j_post])
					seduce_shape_guidecircle_snap(output, &level->loops[i].loop[j * 2], f_distance2f(&level->loops[i].loop[j * 2], &level->loops[i].loop[j_post * 2]));

			}
		}
	}
}



void seduce_shape_guideline_snap(SShape *shape, float *pos, float pointer_x, float pointer_y, SShapeEditManipulatorMode manipulator)
{
	float edge[4], other_edge[4], vector[2] = {0, 0};
	float camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 1, 0}, f;

	seduce_view_camera_getf(NULL, camera);
	seduce_view_camera_vector_getf(NULL, vec, pointer_x, pointer_y);
	f_project3f(pos, center, normal, camera, vec);
	pos[1] = pos[2];
	if(seduce_shape_guideline_snap_pass(shape, edge, vector, pointer_x, pointer_y, manipulator))
	{
		vector[0] = edge[0] - edge[2];
		vector[1] = edge[1] - edge[3];
		f_normalize2f(vector);
		if(seduce_shape_guideline_snap_pass(shape, other_edge, vector, pointer_x, pointer_y, manipulator))
		{
			f_intersect2(pos, edge, &edge[2], other_edge, &other_edge[2]);
		}else
		{
			f = vector[1] * (edge[2] - pos[0]) - vector[0] * (edge[3] - pos[1]);
			pos[0] += vector[1] * f;
			pos[1] += -vector[0] * f;
		}
	}
	if(manipulator == MO_EMM_OFF)
		seduce_shape_guidecircle_snap_pass(shape, pos);
}

void seduce_shape_guideline_edge(float *pos, float *vector, float length)
{
	float f;
	r_primitive_line_fade_3d(pos[0] - vector[1] * 0.05, 0, pos[1] + vector[0] * 0.05, pos[0] - vector[1] * 2.0, 0, pos[1] + vector[0] * 2.0, 0, 0, 0, 0.1, 0, 0, 0, 0);
	r_primitive_line_fade_3d(pos[0] + vector[1] * 0.05, 0, pos[1] - vector[0] * 0.05, pos[0] + vector[1] * 2.0, 0, pos[1] - vector[0] * 2.0, 0, 0, 0, 0.1, 0, 0, 0, 0);
	
	r_primitive_line_fade_3d(pos[0] + vector[0] * 0.05 + vector[1] * 0.05, 0,
							pos[1] + vector[1] * 0.05 - vector[0] * 0.05, 
							pos[0] + vector[0] * 2.0 + vector[1] * 2.0, 0, 
							pos[1] + vector[1] * 2.0 - vector[0] * 2.0, 
							0, 0, 0, 0.1, 0, 0, 0, 0);
	r_primitive_line_fade_3d(pos[0] + vector[0] * 0.05 - vector[1] * 0.05, 0,
							pos[1] + vector[1] * 0.05 + vector[0] * 0.05, 
							pos[0] + vector[0] * 2.0 - vector[1] * 2.0, 0, 
							pos[1] + vector[1] * 2.0 + vector[0] * 2.0, 
							0, 0, 0, 0.1, 0, 0, 0, 0);
	r_primitive_line_fade_3d(pos[0] - vector[0] * 0.05 + vector[1] * 0.05, 0,
							pos[1] - vector[1] * 0.05 - vector[0] * 0.05, 
							pos[0] - vector[0] * 2.0 + vector[1] * 2.0, 0, 
							pos[1] - vector[1] * 2.0 - vector[0] * 2.0, 
							0, 0, 0, 0.1, 0, 0, 0, 0);
	r_primitive_line_fade_3d(pos[0] - vector[0] * 0.05 - vector[1] * 0.05, 0,
							pos[1] - vector[1] * 0.05 + vector[0] * 0.05, 
							pos[0] - vector[0] * 2.0 - vector[1] * 2.0, 0, 
							pos[1] - vector[1] * 2.0 + vector[0] * 2.0, 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_3d(pos[0] - vector[0] * 0.025 - vector[1] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 0,
							pos[1] - vector[1] * 0.025 + vector[0] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 
							pos[0] - vector[0] * 1.0 - vector[1] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 0, 
							pos[1] - vector[1] * 1.0 + vector[0] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_3d(pos[0] - vector[0] * 0.025 + vector[1] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 0,
							pos[1] - vector[1] * 0.025 - vector[0] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 
							pos[0] - vector[0] * 1.0 + vector[1] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 0, 
							pos[1] - vector[1] * 1.0 - vector[0] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_3d(pos[0] + vector[0] * 0.025 - vector[1] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 0,
							pos[1] + vector[1] * 0.025 + vector[0] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 
							pos[0] + vector[0] * 1.0 - vector[1] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 0, 
							pos[1] + vector[1] * 1.0 + vector[0] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_3d(pos[0] + vector[0] * 0.025 + vector[1] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 0,
							pos[1] + vector[1] * 0.025 - vector[0] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 
							pos[0] + vector[0] * 1.0 + vector[1] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 0, 
							pos[1] + vector[1] * 1.0 - vector[0] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 
							0, 0, 0, 0.1, 0, 0, 0, 0);
	

	r_primitive_line_fade_3d(pos[0] - vector[1] * 0.025 + vector[0] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 0,
							pos[1] + vector[0] * 0.025 + vector[1] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 
							pos[0] - vector[1] * 1.0 + vector[0] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 0, 
							pos[1] + vector[0] * 1.0 + vector[1] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_3d(pos[0] + vector[1] * 0.025 + vector[0] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 0,
							pos[1] - vector[0] * 0.025 + vector[1] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 
							pos[0] + vector[1] * 1.0 + vector[0] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 0, 
							pos[1] - vector[0] * 1.0 + vector[1] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_3d(pos[0] - vector[1] * 0.025 - vector[0] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 0,
							pos[1] + vector[0] * 0.025 - vector[1] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 
							pos[0] - vector[1] * 1.0 - vector[0] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 0, 
							pos[1] + vector[0] * 1.0 - vector[1] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 
							0, 0, 0, 0.1, 0, 0, 0, 0);

	r_primitive_line_fade_3d(pos[0] + vector[1] * 0.025 - vector[0] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 0,
							pos[1] - vector[0] * 0.025 - vector[1] * 0.05 * SEDUCE_SHAPE_COS_30_DEGREES, 
							pos[0] + vector[1] * 1.0 - vector[0] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 0, 
							pos[1] - vector[0] * 1.0 - vector[1] * 2.0 * SEDUCE_SHAPE_COS_30_DEGREES, 
							0, 0, 0, 0.1, 0, 0, 0, 0);


	for(f = 0; f < PI * 2; f += PI / 32.0)
		r_primitive_line_fade_3d(pos[0] + sin(f) * length, 0, pos[1] + cos(f) * length, 
								pos[0] + sin(f + PI / 32.0) * length, 0, pos[1] + cos(f + PI / 32.0) * length, 0, 0, 0, 0.1, 0, 0, 0, 0.1);
}

void seduce_shape_draw_surface(BInputState *input, SShape *shape, SShapeEditManipulatorMode manipulator_mode)
{
	float  vec[2], *a, *b, length;
	uint i, j;

	for(i = 0; i < shape->loop_count; i++)
	{
		for(j = 0; j < shape->loops[i].loop_size; j++)
		{
			if(!shape->loops[i].selection[j] && !shape->loops[i].selection[(j + shape->loops[i].loop_size - 1) % shape->loops[i].loop_size])
			{
				b = &shape->loops[i].loop[((j + shape->loops[i].loop_size - 1) % shape->loops[i].loop_size) * 2];
				a = &shape->loops[i].loop[j * 2];
				vec[0] = a[0] - b[0];
				vec[1] = a[1] - b[1];
				length = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
				vec[0] /= length;
				vec[1] /= length;
				r_primitive_line_fade_3d(a[0] + vec[0] * 0.05, 0, a[1] + vec[1] * 0.05, a[0] + vec[0] * 2.0, 0, a[1] + vec[1] * 2.0, 0, 0, 0, 0.1, 0, 0, 0, 0);
				r_primitive_line_fade_3d(b[0] - vec[0] * 0.05, 0, b[1] - vec[1] * 0.05, b[0] - vec[0] * 2.0, 0, b[1] - vec[1] * 2.0, 0, 0, 0, 0.1, 0, 0, 0, 0);
				if(shape->loops[i].selection[(j + 1) % shape->loops[i].loop_size])
					seduce_shape_guideline_edge(a, vec, length);
				if(shape->loops[i].selection[(j + shape->loops[i].loop_size - 2) % shape->loops[i].loop_size])
					seduce_shape_guideline_edge(b, vec, length);
			}
		}
	}
	r_primitive_line_flush();
}

boolean seduce_shape_draw_delete(BInputState *input, void *id, float *pos)
{
	uint i;
	if(input->mode == BAM_DRAW)
	{
		seduce_element_add_point(input, id, 0, pos, 0.01);
		r_primitive_line_3d(pos[0] + 0.0, 0.0, pos[2] + 0.005, pos[0] + 0.0075, 0.0, pos[2] + 0.0125, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] + 0.0125, 0.0, pos[2] + 0.0075, pos[0] + 0.0075, 0.0, pos[2] + 0.0125, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] + 0.005, 0.0, pos[2] + 0.0, pos[0] + 0.0125, 0.0, pos[2] + 0.0075, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] - 0.0, 0.0, pos[2] + 0.005, pos[0] - 0.0075, 0.0, pos[2] + 0.0125, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] - 0.0125, 0.0, pos[2] + 0.0075, pos[0] - 0.0075, 0.0, pos[2] + 0.0125, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] - 0.005, 0.0, pos[2] + 0.0, pos[0] - 0.0125, 0.0, pos[2] + 0.0075, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] + 0.0, 0.0, pos[2] - 0.005, pos[0] + 0.0075, 0.0, pos[2] - 0.0125, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] + 0.0125, 0.0, pos[2] - 0.0075, pos[0] + 0.0075, 0.0, pos[2] - 0.0125, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] + 0.005, 0.0, pos[2] - 0.0, pos[0] + 0.0125, 0.0, pos[2] - 0.0075, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] - 0.0, 0.0, pos[2] - 0.005, pos[0] - 0.0075, 0.0, pos[2] - 0.0125, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] - 0.0125, 0.0, pos[2] - 0.0075, pos[0] - 0.0075, 0.0, pos[2] - 0.0125, 0.0, 0.0, 0.0, 0.3);
		r_primitive_line_3d(pos[0] - 0.005, 0.0, pos[2] - 0.0, pos[0] - 0.0125, 0.0, pos[2] - 0.0075, 0.0, 0.0, 0.0, 0.3);
	}
	for(i = 0; i < input->pointer_count; i++)
		if(id == seduce_element_pointer_id(input, i, NULL))
			break;
	if(i < input->pointer_count)
	{
		if(input->mode == BAM_DRAW)
		{
			r_primitive_line_3d(pos[0] + 0.0, 0.005, pos[2] + 0.005, pos[0] + 0.0075, 0.005, pos[2] + 0.0125, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.0125, 0.005, pos[2] + 0.0075, pos[0] + 0.0075, 0.005, pos[2] + 0.0125, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.005, 0.005, pos[2] + 0.0, pos[0] + 0.0125, 0.005, pos[2] + 0.0075, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.0, 0.005, pos[2] + 0.005, pos[0] - 0.0075, 0.005, pos[2] + 0.0125, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.0125, 0.005, pos[2] + 0.0075, pos[0] - 0.0075, 0.005, pos[2] + 0.0125, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.005, 0.005, pos[2] + 0.0, pos[0] - 0.0125, 0.005, pos[2] + 0.0075, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.0, 0.005, pos[2] - 0.005, pos[0] + 0.0075, 0.005, pos[2] - 0.0125, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.0125, 0.005, pos[2] - 0.0075, pos[0] + 0.0075, 0.005, pos[2] - 0.0125, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] + 0.005, 0.005, pos[2] - 0.0, pos[0] + 0.0125, 0.005, pos[2] - 0.0075, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.0, 0.005, pos[2] - 0.005, pos[0] - 0.0075, 0.005, pos[2] - 0.0125, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.0125, 0.005, pos[2] - 0.0075, pos[0] - 0.0075, 0.005, pos[2] - 0.0125, 1.0, 0.0, 0.0, 1.0);
			r_primitive_line_3d(pos[0] - 0.005, 0.005, pos[2] - 0.0, pos[0] - 0.0125, 0.005, pos[2] - 0.0075, 1.0, 0.0, 0.0, 1.0);
		}
		if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			return TRUE;
	}
	return FALSE;
}

typedef enum{
	SEDUCE_ES_IDLE,
	SEDUCE_ES_EMPTY,
	SEDUCE_ES_MANIPULATOR,
	SEDUCE_ES_NOT_SELECTED,
	SEDUCE_ES_SURFACE,
	SEDUCE_ES_SELECTION,
	SEDUCE_ES_SELECT
}SeduceShapeEditState;

void seduce_shape_edit(BInputState *input, SShape *shape, SShapeEditManipulatorMode manipulator)
{
	static SeduceShapeEditState state[16] = {SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE, SEDUCE_ES_IDLE};
	static void *grab[16], *TEST;
	float f, *array, a[3] = {0, 0, 0}, b[3] = {0, 0, 0}, origo[3] = {0, 0, 0}, center[2], selection;
	uint i, j, k, l, m, size;
	static boolean shift = FALSE, shift_last = FALSE;

	betray_button_get_up_down(0, &shift, &shift_last, BETRAY_BUTTON_SHIFT);


	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < input->pointer_count && i < 16; i++)
		{
			if(state[i] == SEDUCE_ES_SELECT)
			{
				float camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 1, 0}, start[3], end[3];
				seduce_view_camera_getf(NULL, camera);
				seduce_view_camera_vector_getf(NULL, vec, input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0]);
				f_project3f(start, center, normal, camera, vec);
				seduce_view_camera_vector_getf(NULL, vec, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
				f_project3f(end, center, normal, camera, vec);
				
				r_primitive_line_3d(start[0], 0.0, start[2], start[0], 0.0, end[2], 0, 0, 0, 0.3);
				r_primitive_line_3d(end[0], 0.0, start[2], end[0], 0.0, end[2], 0, 0, 0, 0.3);
				r_primitive_line_3d(start[0], 0.0, start[2], end[0], 0.0, start[2], 0, 0, 0, 0.3);
				r_primitive_line_3d(start[0], 0.0, end[2], end[0], 0.0, end[2], 0, 0, 0, 0.3);
				r_primitive_line_3d(start[0], 0.02, start[2], start[0], 0.02, end[2], 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(end[0], 0.02, start[2], end[0], 0.02, end[2], 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(start[0], 0.02, start[2], end[0], 0.02, start[2], 0.2, 0.6, 1.0, 1);
				r_primitive_line_3d(start[0], 0.02, end[2], end[0], 0.02, end[2], 0.2, 0.6, 1.0, 1);
			}
		}		

		for(i = 0; i < shape->loop_count; i++)
		{
			if(shape->loops[i].triangle_array == NULL)
			{
				RFormats vertex_format_types = R_FLOAT;
				uint vertex_format_size = 3;
				shape->loops[i].triangle_array = malloc((sizeof *shape->loops[i].triangle_array) * (shape->loops[i].loop_size - 2) * 3 * 3);
				seduce_shape_loop_polygonize(shape->loops[i].loop, shape->loops[i].loop_size, shape->loops[i].triangle_array);
				shape->loops[i].pool = r_array_allocate((shape->loops[i].loop_size - 2) * 3, &vertex_format_types, &vertex_format_size, 1, 0);
				r_array_load_vertex(shape->loops[i].pool, NULL, shape->loops[i].triangle_array, 0, (shape->loops[i].loop_size - 2) * 3);
			}
		}
	//	free(shape->loops[i].triangle_array);
	//	r_array_free(shape->loops[i].pool);
	
	//	seduce_shape_surface(input, shape);
	
		{

		/*	for(j = 0; j < MO_LST_COUNT; j++)
				for(i = 0; i < shape->loop_count; i++)
					if(shape->loops[i].type == depth_order[j])
						for(k = 0; k < shape->loops[i].loop_size - 2; k++)
							seduce_element_add_triangle(input, shape->loops[i].matrix, 0, 
															&shape->loops[i].triangle_array[k * 9], 
															&shape->loops[i].triangle_array[k * 9 + 3], 
															&shape->loops[i].triangle_array[k * 9 + 6]);*/

			for(i = 0; i < shape->loop_count; i++)
			{
				array = shape->loops[i].loop;
				size = shape->loops[i].loop_size * 2;

				if(shape->loop_selected == i)
					selection = 1.0;
				else
					selection = 0.3;

				for(j = 0; j < input->pointer_count; j++)
					if(shape->loops[i].matrix == seduce_element_pointer_id(input, j, NULL))
						selection += 0.3;

			
				for(j = 0; j < input->pointer_count && j < 16; j++)
				{
					if(state[j] == SEDUCE_ES_SELECTION)
					{
						float pointer[3], *p, f;
						seduce_shape_guideline_snap(shape, pointer, input->pointers[j].pointer_x, input->pointers[j].pointer_y, FALSE);
						p = grab[j];
						pointer[0] -= p[0];
						pointer[1] -= p[1];
						for(k = 0; k < shape->loops[i].loop_size * 2; k++)
							if(shape->loops[i].selection[k / 2])
								shape->loops[i].loop[k] += pointer[k % 2];
						if(!seduce_shape_loop_valid_test(shape->loops[j].loop, shape->loops[j].loop_size))
						{
							f = sin(input->minute_time * 720) * 0.3 + 0.7;
							for(k = 0; k < shape->loops[j].loop_size; k++)
								r_primitive_line_3d(shape->loops[j].loop[k * 2], 0.05, 
													shape->loops[j].loop[k * 2 + 1], 
													shape->loops[j].loop[((k + 1) % shape->loops[j].loop_size) * 2], 0.05, 
													shape->loops[j].loop[((k + 1) % shape->loops[j].loop_size) * 2 + 1],
													1.0, 0.0, 0.2, f);

						}
						for(k = 0; k < shape->loops[i].loop_size * 2; k++)
							if(shape->loops[i].selection[k / 2])
								shape->loops[i].loop[k] -= pointer[k % 2];
					}
				}

				center[0] = center[1] = 0;
				for(j = 0; j < size; j += 2)
				{			
					a[0] = array[j];
					a[2] = array[j + 1];
		
					seduce_element_add_point(input, &array[j], 0, a, SEDUCE_SHAPE_MIN_EDGE_SIZE);
					k = (j + 2) % size;
					a[0] = array[j] - array[k];
					a[2] = array[j + 1] - array[k + 1];
					f = sqrt(a[0] * a[0] + a[2] * a[2]) * 2;

					a[0] = array[j] * (1 - SEDUCE_SHAPE_MIN_EDGE_SIZE / f) + array[k] * (SEDUCE_SHAPE_MIN_EDGE_SIZE / f);
					a[2] = array[j + 1] * (1 - SEDUCE_SHAPE_MIN_EDGE_SIZE / f) + array[k + 1] * (SEDUCE_SHAPE_MIN_EDGE_SIZE / f);
					if(&array[j] == seduce_element_pointer_id(input, 0, NULL) || (/*shape->loops[i].selected && */shape->loops[i].selection[j / 2]))
						r_primitive_line_3d(array[j], 0.0, array[j + 1], a[0], 0.0, a[2], 0.2, 0.6, 1.0, 1.0);
					else
						r_primitive_line_3d(array[j], 0.0, array[j + 1], a[0], 0.0, a[2], 0, 0, 0, selection);

					b[0] = array[j] * (SEDUCE_SHAPE_MIN_EDGE_SIZE / f) + array[k] * (1 - SEDUCE_SHAPE_MIN_EDGE_SIZE / f);
					b[2] = array[j + 1] * (SEDUCE_SHAPE_MIN_EDGE_SIZE / f) + array[k + 1] * (1 - SEDUCE_SHAPE_MIN_EDGE_SIZE / f);
					if(f > SEDUCE_SHAPE_MIN_EDGE_SIZE * 4)
						seduce_element_add_line(input, &array[j + 1], 0, a, b, SEDUCE_SHAPE_MIN_EDGE_SIZE);
					if(&array[j + 1] == seduce_element_pointer_id(input, 0, NULL))
						r_primitive_line_3d(a[0], 0.0, a[2], b[0], 0.0, b[2], 0.2, 0.6, 1.0, 1.0);
					else
						r_primitive_line_3d(a[0], 0.0, a[2], b[0], 0.0, b[2], 0, 0, 0, selection);
					if(shape->loops[i].selection[k / 2])
					{
						r_primitive_line_fade_3d(array[k], 0.0, array[k + 1], array[k], 0.1, array[k + 1], 0.2, 0.6, 1.0, 1.0, 0.2, 0.6, 1.0, 0.0);
						r_primitive_line_3d(array[k] + 0.01, 0.01, array[k + 1] + 0.01, array[k] + 0.01, 0.01, array[k + 1] - 0.01, 0.2, 0.6, 1.0, 1.0);
						r_primitive_line_3d(array[k] - 0.01, 0.01, array[k + 1] + 0.01, array[k] - 0.01, 0.01, array[k + 1] - 0.01, 0.2, 0.6, 1.0, 1.0);
						r_primitive_line_3d(array[k] + 0.01, 0.01, array[k + 1] + 0.01, array[k] - 0.01, 0.01, array[k + 1] + 0.01, 0.2, 0.6, 1.0, 1.0);
						r_primitive_line_3d(array[k] + 0.01, 0.01, array[k + 1] - 0.01, array[k] - 0.01, 0.01, array[k + 1] - 0.01, 0.2, 0.6, 1.0, 1.0);
					}
					if(&array[k] == seduce_element_pointer_id(input, 0, NULL) || (shape->loops[i].selection[k / 2]))
						r_primitive_line_3d(array[k], 0.0, array[k + 1], b[0], 0.0, b[2], 0.2, 0.6, 1.0, 1.0);
					else
						r_primitive_line_3d(array[k], 0.0, array[k + 1], b[0], 0.0, b[2], 0, 0, 0, selection);
				}
				r_primitive_line_flush();
			}
		}
	}else
	{
		if(input->mode == BAM_EVENT)
		{
			for(i = 0; i < input->pointer_count && i < 16; i++)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(shape->manip_start == seduce_element_pointer_id(input, i, NULL))
					{				
						seduce_shape_unselect(shape);
						state[i] = SEDUCE_ES_MANIPULATOR;
						grab[i] = shape->manip_start;
					}else if(shape->manip_end == seduce_element_pointer_id(input, i, NULL))
					{
						seduce_shape_unselect(shape);
						state[i] = SEDUCE_ES_MANIPULATOR;
						grab[i] = shape->manip_end;
					}else
					{
						for(j = 0; j < shape->loop_count; j++)
						{
							if(shape->loops[j].matrix == seduce_element_pointer_id(input, i, NULL))
							{
								state[i] = SEDUCE_ES_SURFACE;
								grab[i] = shape->loops[j].matrix;
								break;
							}           
						}
						if(j == shape->loop_count)
						{
							for(j = 0; j < shape->loop_count; j++)
							{
								for(k = 0; k < shape->loops[j].loop_size; k++)
								{
									if(&shape->loops[j].loop[k * 2 + 1] == seduce_element_pointer_id(input, i, NULL))
									{
										size = shape->loops[j].loop_size * 2;
										shape->loops[j].loop_size++;
										if(!shift)
											seduce_shape_unselect(shape);
										for(l = 0; l < size - k * 2; l++)
											shape->loops[j].loop[size - l + 1] = shape->loops[j].loop[size - l - 1];
										shape->loops[j].selection[k + 1] = TRUE;
										shape->loops[j].loop[k * 2 + 2] = (shape->loops[j].loop[k * 2 + 2] + shape->loops[j].loop[((k + 2) % shape->loops[j].loop_size) * 2 + 0]) * 0.5;
										shape->loops[j].loop[k * 2 + 3] = (shape->loops[j].loop[k * 2 + 3] + shape->loops[j].loop[((k + 2) % shape->loops[j].loop_size) * 2 + 1]) * 0.5;
										state[i] = SEDUCE_ES_SELECTION;
										grab[i] = TEST = &shape->loops[j].loop[k * 2 + 2];
									}
									if(&shape->loops[j].loop[k * 2] == seduce_element_pointer_id(input, i, NULL))
									{
										if(!shape->loops[j].selection[k])
										{
											if(!shift)
												seduce_shape_unselect(shape);
											shape->loops[j].selection[k] = TRUE;
											shape->loop_selected = j;
										}
										state[i] = SEDUCE_ES_SELECTION;
										grab[i] = &shape->loops[j].loop[k * 2];
									}
								}
							}
						}
						if(NULL == seduce_element_pointer_id(input, i, NULL))
							state[i] = SEDUCE_ES_EMPTY;
					}
				}
				if(input->pointers[i].button[0])
				{
					if(state[i] == SEDUCE_ES_SURFACE || state[i] == SEDUCE_ES_EMPTY)
					{
						if((input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) * 
							(input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) +
							(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) * 
							(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) > 0.01 * 0.01)
							state[i] = SEDUCE_ES_SELECT;
					}
					if(state[i] == SEDUCE_ES_SELECT)
					{
						float min[2], max[2], camera[3], vec[3], center[3] = {0, 0, 0}, normal[3] = {0, 1, 0}, start[3], end[3];
						seduce_view_camera_getf(NULL, camera);
						seduce_view_camera_vector_getf(NULL, vec, input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0]);
						f_project3f(start, center, normal, camera, vec);
						seduce_view_camera_vector_getf(NULL, vec, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
						f_project3f(end, center, normal, camera, vec);
						if(start[0] > end[0])
						{
							min[0] = end[0];
							max[0] = start[0];
						}else
						{
							min[0] = start[0];
							max[0] = end[0];
						}
						if(start[2] > end[2])
						{
							min[1] = end[2];
							max[1] = start[2];
						}else
						{
							min[1] = start[2];
							max[1] = end[2];
						}
						for(j = 0; j < shape->loop_count; j++)
						{
							for(k = 0; k < shape->loops[j].loop_size; k++)
							{
								if(!shift)
									shape->loops[j].selection[k] = shape->loops[j].loop[k * 2 + 0] > min[0] && 
																	shape->loops[j].loop[k * 2 + 1] > min[1] && 
																	shape->loops[j].loop[k * 2 + 0] < max[0] && 
																	shape->loops[j].loop[k * 2 + 1] < max[1];
								else
									if(shape->loops[j].loop[k * 2 + 0] > min[0] && 
										shape->loops[j].loop[k * 2 + 1] > min[1] && 
										shape->loops[j].loop[k * 2 + 0] < max[0] && 
										shape->loops[j].loop[k * 2 + 1] < max[1])
											shape->loops[j].selection[k] = TRUE;
							}
						}
					}
					if(state[i] == SEDUCE_ES_MANIPULATOR)
					{
						float pos[3];
						seduce_shape_guideline_snap(shape, pos, input->pointers[i].pointer_x, input->pointers[i].pointer_y, TRUE);
						((float *)grab[i])[0] = pos[0];
						((float *)grab[i])[1] = pos[1];
					}
					if(state[i] == SEDUCE_ES_SELECTION)
					{
						boolean update;
						float pointer[3], *p;
						seduce_shape_guideline_snap(shape, pointer, input->pointers[i].pointer_x, input->pointers[i].pointer_y, FALSE);
						p = grab[i];
						pointer[0] -= p[0];
						pointer[1] -= p[1];

						for(j = 0; j < shape->loop_count; j++)
						{
							update = FALSE;
							for(k = 0; k < shape->loops[j].loop_size; k++)
							{
								if(shape->loops[j].selection[k])
								{
									shape->loops[j].loop[k * 2 + 0] += pointer[0];
									shape->loops[j].loop[k * 2 + 1] += pointer[1];
									update = TRUE;
								}
							}
							if(!seduce_shape_loop_valid_test(shape->loops[j].loop, shape->loops[j].loop_size))
							{
								for(k = 0; k < shape->loops[j].loop_size; k++)
								{
									if(shape->loops[j].selection[k])
									{
										shape->loops[j].loop[k * 2 + 0] -= pointer[0];
										shape->loops[j].loop[k * 2 + 1] -= pointer[1];
									}
								}
							}
							if(update)
							{
								if(shape->loops[j].triangle_array != NULL)
									free(shape->loops[j].triangle_array);
								shape->loops[j].triangle_array = NULL;
								if(shape->loops[j].pool != NULL)
									r_array_free(shape->loops[j].pool);
							}
						}
					}
				}
				if(state[i] == SEDUCE_ES_SURFACE && !input->pointers[i].button[0])
				{
					for(j = 0; j < shape->loop_count; j++)
						if(grab[i] == shape->loops[j].matrix)
							shape->loop_selected = j;
					for(j = 0; j < shape->loop_count && grab[i] != shape->loops[j].matrix; j++);
					if(j < shape->loop_count)
					{
						for(k = 0; k < shape->loops[j].loop_size && !shape->loops[j].selection[k]; k++);
						if(k < shape->loops[j].loop_size)
						{
							if(!shift)
								seduce_shape_unselect(shape);
						}else
						{
							if(!shift)
								seduce_shape_unselect(shape);
							for(j = 0; j < shape->loop_count; j++)
							{
								if(!shift)
								{
									for(k = 0; k < shape->loops[j].loop_size; k++)
										shape->loops[j].selection[k] = grab[i] == shape->loops[j].matrix;
								}else for(k = 0; k < shape->loops[j].loop_size; k++)
										if(grab[i] == shape->loops[j].matrix)
											shape->loops[j].selection[k] = TRUE;
							}
						}
					}
				}
				if(state[i] == SEDUCE_ES_EMPTY && !input->pointers[i].button[0])
				{
					shape->loop_selected = -1;
					if(!shift)
						seduce_shape_unselect(shape);

				}
				if(!input->pointers[i].button[0])
					state[i] = SEDUCE_ES_IDLE;
			}
		}
	}

//	seduce_shape_manipulator(input, edit, shape);

	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < shape->loop_count; i++)
		{
			for(j = 0; j < shape->loops[i].loop_size; j++)
			{
				float pos[3], vec1[2], vec2[2];
				vec1[0] = shape->loops[i].loop[j * 2] - shape->loops[i].loop[((j + 1) % shape->loops[i].loop_size) * 2];
				vec1[1] = shape->loops[i].loop[j * 2 + 1] - shape->loops[i].loop[((j + 1) % shape->loops[i].loop_size) * 2 + 1];
				vec2[0] = shape->loops[i].loop[((j + shape->loops[i].loop_size - 1) % shape->loops[i].loop_size) * 2] - shape->loops[i].loop[j * 2];
				vec2[1] = shape->loops[i].loop[((j + shape->loops[i].loop_size - 1) % shape->loops[i].loop_size) * 2 + 1] - shape->loops[i].loop[j * 2 + 1];
				f_normalize2f(vec1);
				f_normalize2f(vec2);
				pos[0] = vec1[1] + vec2[1];
				pos[1] = -vec2[0] + -vec1[0];
				f_normalize2f(pos);
				pos[0] = shape->loops[i].loop[j * 2] + pos[0] * 0.03;
				pos[2] = shape->loops[i].loop[j * 2 + 1] + pos[1] * 0.03;
				pos[1] = 0;		

				seduce_shape_draw_delete(input, &((uint8*)&shape->loops[i].loop[j * 2])[1], pos);
			//	seduce_element_add_point(input, &((uint8*)&shape->loops[i].loop[j * 2])[1], 0, pos, 0.01);			
				seduce_tool_tip(input, &((uint8*)&shape->loops[i].loop[j * 2])[1], seduce_translate("Delete corner."), NULL);
			}
		}
		r_primitive_line_flush();
	}

	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
		{
			for(j = 0; j < shape->loop_count; j++)
			{
				for(k = 0; k < shape->loops[j].loop_size; k++)
				{
					if(&((uint8*)&shape->loops[j].loop[k * 2])[1] == seduce_element_pointer_id(input, i, NULL))
					{
						shape->loops[j].loop_size--;
						for(i = k; i < shape->loops[j].loop_size; i++)
						{
							shape->loops[j].loop[i * 2] = shape->loops[j].loop[i * 2 + 2];
							shape->loops[j].loop[i * 2 + 1] = shape->loops[j].loop[i * 2 + 3];
						}
						if(shape->loops[j].triangle_array != NULL)
							free(shape->loops[j].triangle_array);
						shape->loops[j].triangle_array = NULL;
						if(shape->loops[j].pool != NULL)
							r_array_free(shape->loops[j].pool);
						return;
					}
				}
			}
		}
	}
}


void mo_menu_edit_draw_loop(uint *loop, uint size, float *vertex, float height, float red, float green, float blue)
{
	uint i;
	size--;
	for(i = 0; i < size; i++)
		r_primitive_line_3d(vertex[loop[i] * 2], height + 0.01 * i, vertex[loop[i] * 2 + 1], vertex[loop[i + 1] * 2], height + 0.01 * i, vertex[loop[i + 1] * 2 + 1], red, green, blue, 1.0);
	r_primitive_line_3d(vertex[loop[0] * 2], height + 0.01 * i, vertex[loop[0] * 2 + 1], vertex[loop[size] * 2], height + 0.01 * i, vertex[loop[size] * 2 + 1], red, green, blue, 1.0);
}
