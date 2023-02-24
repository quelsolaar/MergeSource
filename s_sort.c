#include <math.h>
#include <stdlib.h>

#include "seduce.h"
#include "s_draw_3d.h"

typedef enum{
	SEDUCE_SET_POINT,
	SEDUCE_SET_LINE,
	SEDUCE_SET_TRIANGLE,
	SEDUCE_SET_QUAD,
	SEDUCE_SET_COUNT
}SSortElementType;

typedef struct{
	void *id;
	void *id_down_click;
	uint part;
	char *tooltip;
	char *tooltip_description;
	float tooltip_timer;
}SSortAction;

typedef struct{
	SSortElementType type;
	float pos[16];
	void *id;
	uint part;
	uint user_id;
	uint draw_id;
}SSortElement;

typedef struct{
	RMatrix matrix;
	void *id;
}SSortSurface;

typedef struct{
	float pos[3];
	float draw[3];
	void *id;
	uint part;
	char *tooltip;
	char *tooltip_description;
	float tooltip_timer;
	float click_timer;
	float fade_timer;
}SSortSelection;

typedef struct{
	float pos[3];
	float timer;
	float color[3];
}SSortClick;

#define SEDUCE_SORT_CLICK_COUNT 16

struct{
	SSortElement *element;
	uint element_count;
	uint element_allocated;
	
	SSortSurface *surface;
	uint surface_count;
	uint surface_allocated;
	SSortSelection *selected;
	SSortClick clicks[SEDUCE_SORT_CLICK_COUNT];
	uint frame_number;
	SSortAction *action;
	boolean action_computed;
	uint user_id;
	float center[2];
	float scale;
	float bound[2];
	float tooltip_size;
	float tooltip_color[4];
	void *debug_id;
}SSortStorage;


void seduce_viewport_set(float x_start, float y_start, float x_end, float y_end)
{	
	float aspect;
	uint screen_x, screen_y;
	aspect = betray_screen_mode_get(&screen_x, &screen_y, NULL);
	y_start /= aspect;
	y_end /= aspect;
	r_viewport((int)((float)screen_x * (x_start + 1) / 2.0),
               (int)((float)screen_y * (y_start + 1) / 2.0),
               (int)((float)screen_x * (x_end + 1) / 2.0) - (int)((float)screen_x * (x_start + 1) / 2.0),
               (int)((float)screen_y * (y_end + 1) / 2.0) - (int)((float)screen_y * (y_start + 1) / 2.0));
	SSortStorage.center[0] = (x_start + x_end) * 0.5;
	SSortStorage.center[1] = (y_start + y_end) * 0.5 * aspect;
	SSortStorage.scale = (x_end - x_start) * 0.5;
	SSortStorage.bound[0] = 1.0;
	SSortStorage.bound[1] = aspect;
}

void seduce_matrix_projection_screenf(float *output, float x, float y, float z)
{
	float temp[3];
    //	r_matrix_projection_cameraf(r_matrix_get(), output, x, y, z);
	r_matrix_projection_screenf(r_matrix_get(), temp, x, y, z);
	output[0] = (temp[0] * SSortStorage.scale + SSortStorage.center[0]);
	output[1] = (temp[1] * SSortStorage.scale + SSortStorage.center[1]);
	output[2] = temp[2];
}

void seduce_element_init()
{
	static boolean init = TRUE;
	if(init)
	{
		uint i;
		init = FALSE;
		SSortStorage.element = NULL;
		SSortStorage.element_count = 0;
		SSortStorage.element_allocated = 0;
		SSortStorage.surface = NULL;
		SSortStorage.surface_count = 0;
		SSortStorage.surface_allocated = 0;
		SSortStorage.frame_number = 0;
		for(i = 0; i < SEDUCE_SORT_CLICK_COUNT; i++)
		{		
			SSortStorage.clicks[i].color[0] = 0;
			SSortStorage.clicks[i].color[1] = 0;
			SSortStorage.clicks[i].color[2] = 0;
			SSortStorage.clicks[i].pos[0] = 0;
			SSortStorage.clicks[i].pos[1] = 0;
			SSortStorage.clicks[i].pos[2] = 0;
			SSortStorage.clicks[i].timer = 1000;
		}
		SSortStorage.action = malloc((sizeof *SSortStorage.action) * betray_support_functionality(B_SF_POINTER_COUNT_MAX));
		SSortStorage.action_computed = FALSE;
        for(i = 0; i < betray_support_functionality(B_SF_USER_COUNT_MAX); i++)
        {
		    SSortStorage.action[i].tooltip = NULL;
            SSortStorage.action[i].tooltip_description = NULL;
		}
		SSortStorage.user_id = -1;
		SSortStorage.selected = malloc((sizeof *SSortStorage.selected) * betray_support_functionality(B_SF_USER_COUNT_MAX));
		for(i = 0; i < betray_support_functionality(B_SF_USER_COUNT_MAX); i++)
		{
			SSortStorage.selected[i].pos[0] = 0;
			SSortStorage.selected[i].pos[1] = 0;
			SSortStorage.selected[i].pos[2] = 0;
			SSortStorage.selected[i].draw[0] = 0;
			SSortStorage.selected[i].draw[1] = 0;
			SSortStorage.selected[i].draw[2] = 0;
			SSortStorage.selected[i].id = NULL;
			SSortStorage.selected[i].part = 0;
			SSortStorage.selected[i].tooltip = NULL;
			SSortStorage.selected[i].tooltip_timer = 0;
			SSortStorage.selected[i].click_timer = 10.0;
			SSortStorage.selected[i].fade_timer = 40.0;
		}
		SSortStorage.center[0] = 0;
		SSortStorage.center[1] = 0;
		SSortStorage.scale = 1.0;
		SSortStorage.bound[0] = 1.0;
		SSortStorage.bound[1] = 10000;
		SSortStorage.tooltip_size = SEDUCE_T_SIZE * 2.0;
		SSortStorage.tooltip_color[0] = 0.1;
		SSortStorage.tooltip_color[1] = 0.1;
		SSortStorage.tooltip_color[2] = 0.1;
		SSortStorage.tooltip_color[2] = 0.5;
		SSortStorage.debug_id = NULL;
	}
}

void seduce_element_center_get(uint element, float *center)
{
	uint i, count;
	float *pos;
	pos = SSortStorage.element[element].pos;
	switch(SSortStorage.element[element].type)
	{
		case SEDUCE_SET_POINT :
        center[0] = pos[0];
        center[1] = pos[1];
        center[2] = pos[2];
		break;
		case SEDUCE_SET_LINE :
        center[0] = (pos[0] + pos[3]) / 2.0;
        center[1] = (pos[1] + pos[4]) / 2.0;
        center[2] = (pos[2] + pos[5]) / 2.0;
		break;
		case SEDUCE_SET_TRIANGLE :
        center[0] = pos[9];
        center[1] = pos[10];
        center[2] = pos[11];
		break;
		case SEDUCE_SET_QUAD :
        center[0] = pos[12];
        center[1] = pos[13];
        center[2] = pos[14];
		break;
	}
}


boolean seduce_element_position_get(void *id, float *pos)
{
	uint i, count;
	for(i = 0; i < SSortStorage.element_count; i++)
	{
		if(SSortStorage.element[i].id == id)
		{
			seduce_element_center_get(i, pos);
			return TRUE;
		}
	}
	return FALSE;
}


void seduce_element_select(uint user_id, void *id, uint part, SSortElementType type, float *pos)
{
	uint i, count;
	switch(type)
	{
		case SEDUCE_SET_POINT :
        SSortStorage.selected[user_id].pos[0] = pos[0];
        SSortStorage.selected[user_id].pos[1] = pos[1];
        SSortStorage.selected[user_id].pos[2] = pos[2];
		break;
		case SEDUCE_SET_LINE :
        SSortStorage.selected[user_id].pos[0] = (pos[0] + pos[3]) / 2.0;
        SSortStorage.selected[user_id].pos[1] = (pos[1] + pos[4]) / 2.0;
        SSortStorage.selected[user_id].pos[2] = (pos[2] + pos[5]) / 2.0;
		break;
		case SEDUCE_SET_TRIANGLE :
        SSortStorage.selected[user_id].pos[0] = pos[9];
        SSortStorage.selected[user_id].pos[1] = pos[10];
        SSortStorage.selected[user_id].pos[2] = pos[11];
		break;
		case SEDUCE_SET_QUAD :
        SSortStorage.selected[user_id].pos[0] = pos[12];
        SSortStorage.selected[user_id].pos[1] = pos[13];
        SSortStorage.selected[user_id].pos[2] = pos[14];
		break;
	}
	SSortStorage.selected[user_id].part = part;
	if(SSortStorage.selected[user_id].id != id)
	{
		SSortStorage.selected[user_id].tooltip = NULL;
		SSortStorage.selected[user_id].tooltip_timer = 0;
		SSortStorage.selected[user_id].id = id;
	}
}

void seduce_element_add_point(BInputState *input, void *id, uint part, float *pos, float size)
{
	RMatrix	*matrix;
	uint i;
	if(id == NULL)
		return;
	if(id == SSortStorage.debug_id)
	{
		uint *a = NULL;
		*a = 0;
	}
    
	if(input->frame_number != SSortStorage.frame_number)
	{
		SSortStorage.element_count = 0;
		SSortStorage.surface_count = 0;
		SSortStorage.action_computed = FALSE;
	}
	SSortStorage.frame_number = input->frame_number;
    
	if(SSortStorage.element_count == SSortStorage.element_allocated)
	{
		SSortStorage.element_allocated += 256;
		SSortStorage.element = realloc(SSortStorage.element, (sizeof *SSortStorage.element) * (SSortStorage.element_allocated));
	}
	seduce_matrix_projection_screenf(SSortStorage.element[SSortStorage.element_count].pos, pos[0], pos[1], pos[2]);
	SSortStorage.element[SSortStorage.element_count].pos[3] = size / -SSortStorage.element[SSortStorage.element_count].pos[2];
	SSortStorage.element[SSortStorage.element_count].id = id;
	SSortStorage.element[SSortStorage.element_count].part = part;
	SSortStorage.element[SSortStorage.element_count].type = SEDUCE_SET_POINT;
	SSortStorage.element[SSortStorage.element_count].user_id = SSortStorage.user_id;
	SSortStorage.element[SSortStorage.element_count].draw_id = input->draw_id;
	for(i = 0; i < input->user_count; i++)
		if(id == SSortStorage.selected[i].id && part == SSortStorage.selected[i].part)
        seduce_element_select(i, id, part, SEDUCE_SET_POINT, SSortStorage.element[SSortStorage.element_count].pos);
	SSortStorage.element_count++;
}


void seduce_element_add_line(BInputState *input, void *id, uint part, float *a, float *b, float size)
{
	uint i;
	if(id == NULL)
		return;
	if(id == SSortStorage.debug_id)
	{
		uint *a = NULL;
		*a = 0;
	}
	if(input->frame_number != SSortStorage.frame_number)
	{
		SSortStorage.element_count = 0;
		SSortStorage.surface_count = 0;
		SSortStorage.action_computed = FALSE;
	}
	SSortStorage.frame_number = input->frame_number;
    
	if(SSortStorage.element_count == SSortStorage.element_allocated)
	{
		SSortStorage.element_allocated += 256;
		SSortStorage.element = realloc(SSortStorage.element, (sizeof *SSortStorage.element) * (SSortStorage.element_allocated));
	}
	seduce_matrix_projection_screenf(SSortStorage.element[SSortStorage.element_count].pos, a[0], a[1], a[2]);
	seduce_matrix_projection_screenf(&SSortStorage.element[SSortStorage.element_count].pos[3], b[0], b[1], b[2]);
	SSortStorage.element[SSortStorage.element_count].pos[6] = size / -(SSortStorage.element[SSortStorage.element_count].pos[2] + SSortStorage.element[SSortStorage.element_count].pos[5]) * 0.5;
    /*	f_transform3f(SSortStorage.element[SSortStorage.element_count].pos, matrix->matrix[matrix->current], a[0], a[1], a[2]);
        SSortStorage.element[SSortStorage.element_count].pos[0] /= -SSortStorage.element[SSortStorage.element_count].pos[2];
        SSortStorage.element[SSortStorage.element_count].pos[1] /= -SSortStorage.element[SSortStorage.element_count].pos[2];
        f_transform3f(&SSortStorage.element[SSortStorage.element_count].pos[3], matrix->matrix[matrix->current], b[0], b[1], b[2]);
        SSortStorage.element[SSortStorage.element_count].pos[3] /= -SSortStorage.element[SSortStorage.element_count].pos[5];
        SSortStorage.element[SSortStorage.element_count].pos[4] /= -SSortStorage.element[SSortStorage.element_count].pos[5];*/
	SSortStorage.element[SSortStorage.element_count].id = id;
	SSortStorage.element[SSortStorage.element_count].part = part;
	SSortStorage.element[SSortStorage.element_count].type = SEDUCE_SET_LINE;
	SSortStorage.element[SSortStorage.element_count].user_id = SSortStorage.user_id;
	SSortStorage.element[SSortStorage.element_count].draw_id = input->draw_id;
	for(i = 0; i < input->user_count; i++)
		if(id == SSortStorage.selected[i].id && part == SSortStorage.selected[i].part)
        seduce_element_select(i, id, part, SEDUCE_SET_LINE, SSortStorage.element[SSortStorage.element_count].pos);
	SSortStorage.element_count++;
}

boolean seduce_element_triangle_test(float *pos, float *array, float *vectors)
{
	float vec[2];
	vec[0] = pos[0] - array[0];
	vec[1] = pos[1] - array[1];
	if(vectors[0] * vec[0] + vectors[1] * vec[1] > 0)
	{
		if(vectors[4] * vec[0] + vectors[5] * vec[1] > 0)
		{
			vec[0] = pos[0] - array[3];
			vec[1] = pos[1] - array[4];
			if(vectors[2] * vec[0] + vectors[3] * vec[1] > 0)
				return TRUE;
		}
	}else
	{
		if(vectors[4] * vec[0] + vectors[5] * vec[1] < 0)
		{
			vec[0] = pos[0] - array[3];
			vec[1] = pos[1] - array[4];
			if(vectors[2] * vec[0] + vectors[3] * vec[1] < 0)
				return TRUE;
		}
	}
	return FALSE;
}

void seduce_element_add_triangle(BInputState *input, void *id, uint part, float *a, float *b, float *c)
{
	float vertex[9], vecs[6], vec[2];
	uint i;
	if(id == NULL)
		return;
	if(id == SSortStorage.debug_id)
	{
		uint *a = NULL;
		*a = 0;
	}
    
	seduce_matrix_projection_screenf(&vertex[0], a[0], a[1], a[2]);
	seduce_matrix_projection_screenf(&vertex[3], b[0], b[1], b[2]);
	seduce_matrix_projection_screenf(&vertex[6], c[0], c[1], c[2]);
    
	vecs[0] = vertex[4] - vertex[1];
	vecs[1] = vertex[0] - vertex[3];
	vecs[2] = vertex[7] - vertex[4];
	vecs[3] = vertex[3] - vertex[6];
	vecs[4] = vertex[1] - vertex[7];
	vecs[5] = vertex[6] - vertex[0];
    
	for(i = 0; i < SSortStorage.element_count; i++)
	{
		if(SSortStorage.element[i].type == SEDUCE_SET_POINT &&
           seduce_element_triangle_test(SSortStorage.element[i].pos, vertex, vecs))
		{
			SSortStorage.element[i] = SSortStorage.element[--SSortStorage.element_count];
			i--;
		}
	}
	if(input->frame_number != SSortStorage.frame_number)
	{
		SSortStorage.element_count = 0;
		SSortStorage.surface_count = 0;
		SSortStorage.action_computed = FALSE;
	}
	SSortStorage.frame_number = input->frame_number;
    
	if(SSortStorage.element_count == SSortStorage.element_allocated)
	{
		SSortStorage.element_allocated += 256;
		SSortStorage.element = realloc(SSortStorage.element, (sizeof *SSortStorage.element) * (SSortStorage.element_allocated));
	}
	for(i = 0; i < 9; i++)
		SSortStorage.element[SSortStorage.element_count].pos[i] = vertex[i];
    
	SSortStorage.element[SSortStorage.element_count].pos[9] = 0;
	SSortStorage.element[SSortStorage.element_count].pos[10] = 0;
	for(i = 0; i < 9; i += 3)
	{
		SSortStorage.element[SSortStorage.element_count].pos[9] += vertex[i] / 3.0;
		SSortStorage.element[SSortStorage.element_count].pos[10] += vertex[i + 1] / 3.0;
	}
    
	SSortStorage.element[SSortStorage.element_count].id = id;
	SSortStorage.element[SSortStorage.element_count].part = part;
	SSortStorage.element[SSortStorage.element_count].draw_id = input->draw_id;
    
	SSortStorage.element[SSortStorage.element_count].type = SEDUCE_SET_TRIANGLE;
	SSortStorage.element[SSortStorage.element_count].user_id = SSortStorage.user_id;
	for(i = 0; i < input->user_count; i++)
		if(id == SSortStorage.selected[i].id && part == SSortStorage.selected[i].part)
        seduce_element_select(i, id, part, SEDUCE_SET_TRIANGLE, SSortStorage.element[SSortStorage.element_count].pos);
	SSortStorage.element_count++;
}

boolean seduce_element_quad_test(float *pos, float *array, float *vectors)
{
	float vec[2];
	vec[0] = pos[0] - array[0];
	vec[1] = pos[1] - array[1];
	
	if(vectors[0] * vec[0] + vectors[1] * vec[1] > 0)
	{
		if(vectors[6] * vec[0] + vectors[7] * vec[1] > 0)
		{
			vec[0] = pos[0] - array[6];
			vec[1] = pos[1] - array[7];
			if(vectors[2] * vec[0] + vectors[3] * vec[1] > 0)
				if(vectors[4] * vec[0] + vectors[5] * vec[1] > 0)
                return TRUE;
		}
	}else if(vectors[6] * vec[0] + vectors[7] * vec[1] < 0)
	{
		vec[0] = pos[0] - array[6];
		vec[1] = pos[1] - array[7];
		if(vectors[2] * vec[0] + vectors[3] * vec[1] < 0)
			if(vectors[4] * vec[0] + vectors[5] * vec[1] < 0)
            return TRUE;
	}
	return FALSE;
}

void seduce_element_add_quad(BInputState *input, void *id, uint part, float *a, float *b, float *c, float *d)
{
	float vertex[12], vecs[8], vec[2];
	uint i;
	if(id == NULL)
		return;
	if(id == SSortStorage.debug_id)
	{
		uint *a = NULL;
		*a = 0;
	}
	seduce_matrix_projection_screenf(&vertex[0], a[0], a[1], a[2]);
	seduce_matrix_projection_screenf(&vertex[3], b[0], b[1], b[2]);
	seduce_matrix_projection_screenf(&vertex[6], c[0], c[1], c[2]);
	seduce_matrix_projection_screenf(&vertex[9], d[0], d[1], d[2]);
    
	vecs[0] = vertex[4] - vertex[1];
	vecs[1] = vertex[0] - vertex[3];
	vecs[2] = vertex[7] - vertex[4];
	vecs[3] = vertex[3] - vertex[6];
	vecs[4] = vertex[10] - vertex[7];
	vecs[5] = vertex[6] - vertex[9];
	vecs[6] = vertex[1] - vertex[10];
	vecs[7] = vertex[9] - vertex[0];
    
    /*	for(i = 0; i < SSortStorage.element_count; i++)
        {
            if(SSortStorage.element[i].type == SEDUCE_SET_POINT &&
                seduce_element_quad_test(SSortStorage.element[i].pos, vertex, vecs))
            {
                SSortStorage.element[i] = SSortStorage.element[--SSortStorage.element_count];
                i--;
            }
        }*/
	if(input->frame_number != SSortStorage.frame_number)
	{
		SSortStorage.element_count = 0;
		SSortStorage.surface_count = 0;
		SSortStorage.action_computed = FALSE;
	}
	SSortStorage.frame_number = input->frame_number;
    
	if(SSortStorage.element_count == SSortStorage.element_allocated)
	{
		SSortStorage.element_allocated += 256;
		SSortStorage.element = realloc(SSortStorage.element, (sizeof *SSortStorage.element) * (SSortStorage.element_allocated));
	}
	for(i = 0; i < 12; i++)
		SSortStorage.element[SSortStorage.element_count].pos[i] = vertex[i];
    
	SSortStorage.element[SSortStorage.element_count].pos[12] = 0;
	SSortStorage.element[SSortStorage.element_count].pos[13] = 0;
	for(i = 0; i < 12; i += 3)
	{
		SSortStorage.element[SSortStorage.element_count].pos[12] += vertex[i] / 4.0;
		SSortStorage.element[SSortStorage.element_count].pos[13] += vertex[i + 1] / 4.0;
	}
	SSortStorage.element[SSortStorage.element_count].id = id;
	SSortStorage.element[SSortStorage.element_count].part = part;
	SSortStorage.element[SSortStorage.element_count].draw_id = input->draw_id;
	SSortStorage.element[SSortStorage.element_count].type = SEDUCE_SET_QUAD;
	SSortStorage.element[SSortStorage.element_count].user_id = SSortStorage.user_id;
	for(i = 0; i < input->user_count; i++)
		if(id == SSortStorage.selected[i].id && part == SSortStorage.selected[i].part)
        seduce_element_select(i, id, part, SEDUCE_SET_QUAD, SSortStorage.element[SSortStorage.element_count].pos);
	SSortStorage.element_count++;
}


void seduce_element_add_rectangle(BInputState *input, void *id, uint part, float pos_x, float pos_y, float size_x, float size_y)
{
	float array[12];
	array[0] = pos_x;
	array[1] = pos_y;
	array[2] = 0;
	array[3] = pos_x + size_x;
	array[4] = pos_y;
	array[5] = 0;
	array[6] = pos_x + size_x;
	array[7] = pos_y + size_y;
	array[8] = 0;
	array[9] = pos_x;
	array[10] = pos_y + size_y;
	array[11] = 0;
	seduce_element_add_quad(input, id, part, array, &array[3], &array[6], &array[9]);
}

void seduce_element_add_surface(BInputState *input, void *id)
{
	RMatrix	*matrix;
	uint i;
	if(id == NULL)
		return;
	if(id == SSortStorage.debug_id)
	{
		uint *a = NULL;
		*a = 0;
	}
	if(input->frame_number != SSortStorage.frame_number)
	{
		SSortStorage.element_count = 0;
		SSortStorage.surface_count = 0;
		SSortStorage.action_computed = FALSE;
	}
	SSortStorage.frame_number = input->frame_number;
	if(SSortStorage.surface_count == SSortStorage.surface_allocated)
	{
		SSortStorage.surface_allocated += 256;
		SSortStorage.surface = realloc(SSortStorage.surface, (sizeof *SSortStorage.surface) * (SSortStorage.surface_allocated));
	}
	matrix = r_matrix_get();
	SSortStorage.surface[SSortStorage.surface_count].id = id;
	SSortStorage.surface[SSortStorage.surface_count].matrix = *matrix;
	SSortStorage.surface_count++;
}

void seduce_element_add_center(BInputState *input, void *id, float *center)
{
	float pos[3];
	uint i;
	seduce_matrix_projection_screenf(pos, center[0], center[1], center[2]);
	pos[0] /= -pos[2];
	pos[1] /= -pos[2];
	for(i = 0; i < SSortStorage.element_count; i++)
	{
		if(SSortStorage.element[i].type == SEDUCE_SET_TRIANGLE)
		{
			SSortStorage.element[i].pos[12] = pos[0];
			SSortStorage.element[i].pos[13] = pos[1];
		}
		if(SSortStorage.element[i].type == SEDUCE_SET_QUAD)
		{
			SSortStorage.element[i].pos[12] = pos[0];
			SSortStorage.element[i].pos[13] = pos[1];
		}
	}
}





boolean seduce_element_surface_project(BInputState *input, void *id, float *output, uint axis, float pointer_x, float pointer_y)
{
	float  zero[3] = {0, 0, 0},  out[3];
	uint i;
	for(i = 0; i < SSortStorage.surface_count; i++)
		if(SSortStorage.surface[i].id == id)
        break;
	if(i == SSortStorage.surface_count)
		return FALSE;
	r_matrix_projection_surfacef(&SSortStorage.surface[i].matrix, output, zero, axis, pointer_x, pointer_y);
	return TRUE;
}


/*

boolean seduce_element_surface_project(BInputState *input, void *id, float *output, float pointer_x, float pointer_y)
{
	float center[3], axis_vector[9], zero[3] = {0, 0, 0}, vector[3], out[3], f;
	uint i, axis = 2 * 3;
	for(i = 0; i < SSortStorage.element_count; i++)
		if(SSortStorage.element[i].id == id && SSortStorage.element[i].type == SEDUCE_SET_MATRIX)
			break;
	if(i == SSortStorage.element_count)
		return FALSE;
	f_transform3f(center, SSortStorage.element[i].pos, 0, 0, 0);
	f_transform3f(&axis_vector[0], SSortStorage.element[i].pos, 1, 0, 0);
	f_transform3f(&axis_vector[3], SSortStorage.element[i].pos, 0, 1, 0);
	f_transform3f(&axis_vector[6], SSortStorage.element[i].pos, 0, 0, 1);
	axis_vector[axis + 0] -= center[0];
	axis_vector[axis + 1] -= center[1];
	axis_vector[axis + 2] -= center[2];
	vector[0] = pointer_x;
	vector[1] = pointer_y;
	vector[2] = -1;
	f_normalize3f(vector);
	f_project3f(out, center, &axis_vector[axis], zero, vector);
	out[0] -= center[0];
	out[1] -= center[1];
	out[2] -= center[2];
	axis_vector[axis + 0] -= center[0];
	axis_vector[axis + 1] -= center[1];
	axis_vector[axis + 2] -= center[2];
	axis = (axis + 3) % 9;
	f = axis_vector[axis + 0] * axis_vector[axis + 0] + axis_vector[axis + 1] * axis_vector[axis + 1] + axis_vector[axis + 2] * axis_vector[axis + 2];
	axis_vector[axis + 0] /= f;
	axis_vector[axis + 1] /= f;
	axis_vector[axis + 2] /= f;
	axis_vector[axis + 0] -= center[0];
	axis_vector[axis + 1] -= center[1];
	axis_vector[axis + 2] -= center[2];
	axis = (axis + 3) % 9;
	f = axis_vector[axis + 0] * axis_vector[axis + 0] + axis_vector[axis + 1] * axis_vector[axis + 1] + axis_vector[axis + 2] * axis_vector[axis + 2];
	axis_vector[axis + 0] /= f;
	axis_vector[axis + 1] /= f;
	axis_vector[axis + 2] /= f;
	output[1] = out[0] * axis_vector[axis + 0] + out[1] * axis_vector[axis + 1] + out[2] * axis_vector[axis + 2];
	axis = (axis + 6) % 9;
	output[0] = out[0] * axis_vector[axis + 0] + out[1] * axis_vector[axis + 1] + out[2] * axis_vector[axis + 2];
	return TRUE;
}

*/
boolean seduce_element_active(BInputState *input, void *id, void *part)
{
	uint i; 
	for(i = 0; i < input->pointer_count; i++)
		if(id == seduce_element_pointer_id(input, i, part))
        return TRUE;
	for(i = 0; i < input->user_count; i++)
		if(id == seduce_element_selected_id(i, NULL, part))
        return TRUE;
	return FALSE;
}

uint seduce_element_primary_axis(BInputState *input, uint user_id)
{
	uint axis;
	for(axis = 0; axis < input->axis_count && (input->axis[axis].axis_type != B_AXIS_STICK || input->axis[axis].axis_count == 1 || input->axis[axis].user_id != user_id); axis++);
	if(input->axis_count == axis)
		for(axis = 0; axis < input->axis_count && (input->axis[axis].axis_type != B_AXIS_SUBSTICK || input->axis[axis].axis_count == 1 || input->axis[axis].user_id != user_id); axis++);
	if(input->axis_count == axis)
		return -1;
	return axis;
}

void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float size, uint id, float fade, float *color);


uint seduce_element_direction_test(BInputState *input, uint user_id, float dir_x, float dir_y, float pos_x, float pos_y)
{
	float f, best = 100000000.0, vec[2], center[2];
	uint i, found = -1, axis;
	for(i = 0; i < SSortStorage.element_count; i++)
	{
		if(SSortStorage.element[i].id != SSortStorage.selected[user_id].id || SSortStorage.element[i].part != SSortStorage.selected[user_id].part)
		{
			seduce_element_center_get(i, vec);
			vec[0] -= SSortStorage.selected[user_id].pos[0];
			vec[1] -= SSortStorage.selected[user_id].pos[1];
			if(0.001 < vec[0] * dir_x + vec[1] * dir_y)
			{
				seduce_element_center_get(i, center);
				vec[0] = center[0] - pos_x;
				vec[1] = center[1] - pos_y;
				f = vec[0] * vec[0] + vec[1] * vec[1];
				if(f < best)
				{
					best = f;
					found = i;
				}
			}
		}
	}
	return found;
}


void seduce_element_debug(BInputState *input)
{
	RMatrix	matrix, *m;
	float color[3], *p, aspect, size, vec[2];
	uint i, j, user_count;
    /*	if(input->frame_number == SSortStorage.frame_number)
        {
            BInputState *input;
            uint i, j, count;
            float pos[2];
            input = betray_get_input_state();
            if(!SSortStorage.action_computed)
            {
                float f, best, vec[2];
                for(i = 0; i < input->pointer_count; i++)
                {
                    pos[0] = input->pointers[i].pointer_x;
                    pos[1] = input->pointers[i].pointer_y;
                    SSortStorage.action[i].id = seduce_element_colission_test(pos, &SSortStorage.action[i].part, input->pointers[i].user_id);
                }
                count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
                for(; i < count; i++)
                    SSortStorage.action[i].id = NULL;
                SSortStorage.action_computed = TRUE;
            }
        }*/
    
	m = r_matrix_get();
	r_matrix_identity(&matrix);
	aspect = betray_screen_mode_get(NULL, NULL, NULL);
    r_matrix_frustum(&matrix, -0.05, 0.05, -0.05 * aspect, 0.05 * aspect, 0.05, 10.0);
	r_matrix_translate(&matrix, 0.0, 0.0, -1.0);
	r_matrix_set(&matrix);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	for(i = 0; i < SSortStorage.element_count; i++)
	{
		if(SSortStorage.element[i].draw_id == input->draw_id)
		{
			p = SSortStorage.element[i].pos;
			color[0] = 1;
			color[1] = (float)(1 + i) / (float)SSortStorage.element_count;
			if(SSortStorage.element[i].user_id != -1)
				color[2] = 1.0;
			else
				color[2] = 0.0;
			for(j = 0; j < input->pointer_count; j++)
				if(SSortStorage.action[j].id == SSortStorage.element[i].id && SSortStorage.action[j].part == SSortStorage.element[i].part)
                color[0] = 0.0;
            
			switch(SSortStorage.element[i].type)
			{
				case SEDUCE_SET_POINT:
                size = p[3] * 0.7;
                r_primitive_line_3d(p[0] + size, p[1] + size, 0, p[0] - size, p[1] - size, 0, color[0], color[1], color[2], 1.1);
                r_primitive_line_3d(p[0] - size, p[1] + size, 0, p[0] + size, p[1] - size, 0, color[0], color[1], color[2], 1.1);
                break;
				case SEDUCE_SET_LINE:
                size = SSortStorage.element[i].pos[6];
                vec[0] = p[0] - p[3];
                vec[1] = p[1] - p[4];
                f_normalize2f(vec);
                vec[0] *= size;
                vec[1] *= size;
                r_primitive_line_3d(p[0] + vec[1], p[1] - vec[0], 0, p[3] + vec[1], p[4] - vec[0], 0, color[0], color[1], color[2], 1.1);
                r_primitive_line_3d(p[0] - vec[1], p[1] + vec[0], 0, p[3] - vec[1], p[4] + vec[0], 0, color[0], color[1], color[2], 1.1);
                
                r_primitive_line_3d(p[0] + vec[1], p[1] - vec[0], 0, p[0] - vec[1], p[1] + vec[0], 0, color[0], color[1], color[2], 1.1);
                r_primitive_line_3d(p[3] + vec[1], p[4] - vec[0], 0, p[3] - vec[1], p[4] + vec[0], 0, color[0], color[1], color[2], 1.1);
                break;
				case SEDUCE_SET_TRIANGLE:
                r_primitive_line_3d(p[0], p[1], 0, p[3], p[4], 0, color[0], color[1], color[2], 1.1);
                r_primitive_line_3d(p[3], p[4], 0, p[6], p[7], 0, color[0], color[1], color[2], 1.1);
                r_primitive_line_3d(p[6], p[7], 0, p[0], p[1], 0, color[0], color[1], color[2], 1.1);
                break;
				case SEDUCE_SET_QUAD:
                r_primitive_line_3d(p[0], p[1], 0, p[3], p[4], 0, color[0], color[1], color[2], 1.1);
                r_primitive_line_3d(p[3], p[4], 0, p[6], p[7], 0, color[0], color[1], color[2], 1.1);
                r_primitive_line_3d(p[6], p[7], 0, p[9], p[10], 0, color[0], color[1], color[2], 1.1);
                r_primitive_line_3d(p[9], p[10], 0, p[0], p[1], 0, color[0], color[1], color[2], 1.1);
                break;
			}
		}
	}
	r_primitive_line_flush();
	r_matrix_set(m);
}




void s_sort_draw_tool_tip(BInputState *input, float x, float y, char *text, char *description, float text_size, float timer, float red, float green, float blue, float alpha)
{
	if(input->mode == BAM_DRAW)
	{
		float f, aspect, length, headline_length, center[3], height, full_height, expand_timer, pos[2], arrow_pos, arrow_size, shadow[8 * 2], text_color = 0;
		uint shadow_count = 3;
		SeduceBackgroundObject *object;
		STextBlockMode modes;
        
		if(red + green + blue < 1.5)
			text_color = 1.0;
        
		timer *= 4.0;
		expand_timer = 0.0;
		if(timer > 1.0)
		{
			if(timer > 3.0)
			{
				expand_timer = timer - 3.0;
				if(expand_timer >= 1.0)
					expand_timer = 1.0;
			}
			timer = 1.0;
		}
        
		aspect = betray_screen_mode_get(NULL, NULL, NULL);
		center[0] = x;
		center[1] = y;
		center[2] = 0;
		
		headline_length = length = seduce_text_line_length(NULL, text_size, 0.200, text, -1) + text_size * 4.0;
        
		if(description != NULL)
		{
			modes.character_position = 0;
			modes.font = seduce_font_default_get(); /* Font */
			modes.red = text_color; /* Red color component of text */
			modes.green = text_color; /* Green color component of text */ 
			modes.blue = text_color; /* Red color component of text */
			modes.alpha = 1.0; /* Alpha component of text */
			modes.letter_size = text_size; /* Sice of charcters */
			modes.letter_spacing = 0.200; /* Added spacing between chacters. Use SEDUCE_T_SPACE as default value.*/
			if(headline_length < text_size * 48.0)
				length = text_size * 48.0;	
			f = seduce_text_block_height(length - text_size * 12.0, 3.2, SEDUCE_TBAS_STRETCH, description, 0, &modes, 1, -1) + text_size * 10.0;
			length = headline_length;
			if(headline_length < text_size * 48.0 * expand_timer)
				length = text_size * 48.0 * expand_timer;	
			height = text_size * 3.0 + f * expand_timer;
			full_height = text_size * 3.0 + f;
			arrow_size = text_size * 3.0;
		}else
		{
			expand_timer = 0;
			full_height = height = text_size * 3.0;		
		}
		arrow_size = text_size * (1.5 + expand_timer * 1.5);
		object = seduce_background_object_allocate();
        
        
		if(x * x > (0.9 - length * 0.5) * (0.9 - length * 0.5))
		{
			pos[1] = y + height * 0.5;
			if(pos[1] > aspect - arrow_size)
				pos[1] = aspect - arrow_size;
			if(pos[1] - height < arrow_size - aspect)
				pos[1] = arrow_size - aspect + height;
			arrow_pos = y;
			if(arrow_pos > aspect - arrow_size * 2)
				arrow_pos = aspect - arrow_size * 2;
			if(arrow_pos < -aspect + arrow_size * 2)
				arrow_pos = -aspect + arrow_size * 2;
			if(x > 0.0)
			{
				pos[0] = x - arrow_size - length;
				shadow[0] = x - arrow_size;
				shadow[1] = arrow_pos + arrow_size;
				shadow[2] = x;
				shadow[3] = y;
				shadow[4] = x - arrow_size;
				shadow[5] = arrow_pos - arrow_size;
				shadow_count = 6;
				if(arrow_pos - arrow_size > pos[1] - height)
				{
					shadow[shadow_count++] = pos[0] + length;
					shadow[shadow_count++] = pos[1] - height;
				}
				shadow[shadow_count++] = pos[0];
				shadow[shadow_count++] = pos[1] - height;
				shadow[shadow_count++] = pos[0];
				shadow[shadow_count++] = pos[1];
				if(arrow_pos + arrow_size < pos[1])
				{
					shadow[shadow_count++] = pos[0] + length;
					shadow[shadow_count++] = pos[1];
				}
				shadow_count /= 2;
			}else
			{
				pos[0] = x + arrow_size;
                
				shadow[0] = x + arrow_size;
				shadow[1] = arrow_pos - arrow_size;
				shadow[2] = x;
				shadow[3] = y;
				shadow[4] = x + arrow_size;
				shadow[5] = arrow_pos + arrow_size;
				shadow_count = 6;
				if(arrow_pos + arrow_size < pos[1])
				{
					shadow[shadow_count++] = pos[0];
					shadow[shadow_count++] = pos[1];
				}
				shadow[shadow_count++] = pos[0] + length;
				shadow[shadow_count++] = pos[1];
				shadow[shadow_count++] = pos[0] + length;
				shadow[shadow_count++] = pos[1] - height;
				if(arrow_pos - arrow_size > pos[1] - height)
				{
					shadow[shadow_count++] = pos[0];
					shadow[shadow_count++] = pos[1] - height;
				}
				shadow_count /= 2;
                
			}
		}else if(y + full_height < aspect)
		{
			pos[0] = x - length * 0.5;
			pos[1] = y + height + arrow_size;
			shadow[0] = x + arrow_size;
			shadow[1] = y + arrow_size;
			shadow[2] = x;
			shadow[3] = y;
			shadow[4] = x - arrow_size;
			shadow[5] = y + arrow_size;
            
			shadow[6] = pos[0];
			shadow[7] = pos[1] - height;
			shadow[8] = pos[0];
			shadow[9] = pos[1];
			shadow[10] = pos[0] + length;
			shadow[11] = pos[1];
			shadow[12] = pos[0] + length;
			shadow[13] = pos[1] - height;
			shadow_count = 7;
            
		}else
		{
			pos[0] = x - length * 0.5;
			pos[1] = y - arrow_size;
			shadow[0] = x - arrow_size;
			shadow[1] = y - arrow_size;
			shadow[2] = x;
			shadow[3] = y;
			shadow[4] = x + arrow_size;
			shadow[5] = y - arrow_size;
			shadow[6] = pos[0] + length;
			shadow[7] = pos[1];
			shadow[8] = pos[0] + length;
			shadow[9] = pos[1] - height;
			shadow[10] = pos[0];
			shadow[11] = pos[1] - height;
			shadow[12] = pos[0];
			shadow[13] = pos[1];
			shadow_count = 7;
		}
		seduce_background_tri_add(object, NULL, 0,
                                  shadow[0], shadow[1], 0.000,
                                  shadow[2], shadow[3], 0.000,
                                  shadow[4], shadow[5], 0.000,
                                  red, green, blue, alpha);
		seduce_background_quad_add(object, NULL, 0,
                                   pos[0], pos[1], 0.000,
                                   pos[0] + length, pos[1], 0.000,
                                   pos[0] + length, pos[1] - height, 0.000,
                                   pos[0], pos[1] - height, 0.000,
                                   red, green, blue, alpha);
		seduce_background_shadow_add(object, shadow, shadow_count, TRUE, text_size * 2.0);
		seduce_primitive_surface_draw(input, object, timer);
		seduce_primitive_background_object_free(object);
		seduce_text_line_draw(NULL, pos[0] + text_size * 2.0 + expand_timer * (length - headline_length) * 0.5, pos[1] - text_size * 2.0 - text_size * 3.0 * expand_timer, text_size, 0.200, text, text_color, text_color, text_color, 1.0, -1);
		if(expand_timer > 0.99 && description != NULL)
			seduce_text_block_draw(pos[0] + text_size * 6.0, pos[1] - text_size * 8.0, length - text_size * 12.0, 10000, 3.2, SEDUCE_TBAS_STRETCH, description, 0, &modes, 1);
	}
    
    
}

extern void seduce_particle_update(BInputState *input);
extern void seduce_surface_click_buffer_update(BInputState *input);
#define SEDUCE_SORT_AXIS_LENGTH 0.1


void seduce_element_endframe(BInputState *input, boolean debug)
{
	RMatrix	matrix, *save;
	float *pos, aspect, axis_length = 0.2, vec[3], axis_pos[2], f, f2, best, color, center[3], m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, radius, fade;
	uint i, j, user_id, axis, found;
    
	if(input->mode == BAM_DRAW)
	{	
		SSortStorage.action_computed = FALSE;
		save = r_matrix_get();
		r_matrix_identity(&matrix);
		aspect = betray_screen_mode_get(&i, &j, NULL);
        r_matrix_frustum(&matrix, -0.05, 0.05, -0.05 * aspect, 0.05 * aspect, 0.05, 10.0);
		r_matrix_translate(&matrix, 0.0, 0.0, -1.0);
		r_matrix_set(&matrix);
		r_viewport(0, 0, i, j);
		if(debug)
		{
			float left, right, top, bottom;
			for(i = 0; i < input->pointer_count; i++)
			{
				r_primitive_line_2d(input->pointers[i].pointer_x - 0.1,
									input->pointers[i].pointer_y,
									input->pointers[i].pointer_x + 0.1,
									input->pointers[i].pointer_y, 1, 1, 1, 1);
				r_primitive_line_2d(input->pointers[i].pointer_x,
									input->pointers[i].pointer_y - 0.1,
									input->pointers[i].pointer_x,
									input->pointers[i].pointer_y + 0.1, 1, 1, 1, 1);
				r_primitive_line_2d(input->pointers[i].pointer_x,
									input->pointers[i].pointer_y,
									input->pointers[i].pointer_x + input->pointers[i].delta_pointer_x,
									input->pointers[i].pointer_y + input->pointers[i].delta_pointer_y, 1, 1, 1, 1);
				for(j = 0; j < input->pointers[i].button_count; j++)
				{
					r_primitive_line_2d(input->pointers[i].click_pointer_x[j] - 0.03,
										input->pointers[i].click_pointer_y[j] - 0.03,
										input->pointers[i].click_pointer_x[j] + 0.03,
										input->pointers[i].click_pointer_y[j] + 0.03, 0.6, 0.6, 0.6, 1);
					r_primitive_line_2d(input->pointers[i].click_pointer_x[j] + 0.03,
										input->pointers[i].click_pointer_y[j] - 0.03,
										input->pointers[i].click_pointer_x[j] - 0.03,
										input->pointers[i].click_pointer_y[j] + 0.03, 0.6, 0.6, 0.6, 1);
				}
			}
			betray_screen_mode_safe_get(BETRAY_SA_ACTION, &left, &right, &top, &bottom);
			r_primitive_line_2d(left, top, right, top, 0.6, 0.6, 0.6, 1);
			r_primitive_line_2d(left, bottom, right, bottom, 0.6, 0.6, 0.6, 1);
			r_primitive_line_2d(left, top, left, bottom, 0.6, 0.6, 0.6, 1);
			r_primitive_line_2d(right, top, right, bottom, 0.6, 0.6, 0.6, 1);
			betray_screen_mode_safe_get(BETRAY_SA_TITLE, &left, &right, &top, &bottom);
			r_primitive_line_2d(left, top, right, top, 0.4, 0.4, 0.4, 1);
			r_primitive_line_2d(left, bottom, right, bottom, 0.4, 0.4, 0.4, 1);
			r_primitive_line_2d(left, top, left, bottom, 0.4, 0.4, 0.4, 1);
			r_primitive_line_2d(right, top, right, bottom, 0.4, 0.4, 0.4, 1);


			r_primitive_line_flush();
			seduce_element_debug(input);
		}
	}
	if(input->mode == BAM_MAIN)
		seduce_surface_click_buffer_update(input);
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->mode == BAM_MAIN)
			SSortStorage.action[i].tooltip_timer += input->delta_time;
		SSortStorage.action[i].tooltip_description;
		if(input->mode == BAM_DRAW && NULL != SSortStorage.action[i].tooltip)
            s_sort_draw_tool_tip(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, SSortStorage.action[i].tooltip, SSortStorage.action[i].tooltip_description, SSortStorage.tooltip_size, SSortStorage.action[i].tooltip_timer, SSortStorage.tooltip_color[0], SSortStorage.tooltip_color[1], SSortStorage.tooltip_color[2], SSortStorage.tooltip_color[3]);
        
	}
	if(input->mode == BAM_MAIN)
		for(i = 0; i < SEDUCE_SORT_CLICK_COUNT; i++)
        SSortStorage.clicks[i].timer += input->delta_time;
	if(input->mode == BAM_MAIN)
	{
		for(i = 0; i < SEDUCE_SORT_CLICK_COUNT; i++)
		{	
			if(SSortStorage.clicks[i].timer < 4.0)	
			{
				f = SSortStorage.clicks[i].timer / 1.0;
				f = f * 0.1 + f * f;
				seduce_primitive_circle_add_3d(NULL,
                                               SSortStorage.clicks[i].pos[0], SSortStorage.clicks[i].pos[1], SSortStorage.clicks[i].pos[2],
                                               1, 0, 0,
                                               0, 0, 1,
                                               0.02 * (f * 10.0 + 1),
                                               0.0, 1.0,
                                               0, 1,
                                               SSortStorage.clicks[i].color[0], SSortStorage.clicks[i].color[1], SSortStorage.clicks[i].color[2], 1 - f,
                                               SSortStorage.clicks[i].color[0], SSortStorage.clicks[i].color[1], SSortStorage.clicks[i].color[2], 1 - f);
			}
		}
	}
    
	for(user_id = 0; user_id < input->user_count; user_id++)
	{
		SSortStorage.selected[user_id].draw[0] = SSortStorage.selected[user_id].draw[0] * (1 - input->delta_time * 10.0) + SSortStorage.selected[user_id].pos[0] * input->delta_time * 10.0;
		SSortStorage.selected[user_id].draw[1] = SSortStorage.selected[user_id].draw[1] * (1 - input->delta_time * 10.0) + SSortStorage.selected[user_id].pos[1] * input->delta_time * 10.0;
		axis = seduce_element_primary_axis(input, user_id);
		found = -1;
		best = 100000;
		if(input->mode == BAM_MAIN)
		{
			SSortStorage.selected[user_id].tooltip_timer += input->delta_time;
			SSortStorage.selected[user_id].click_timer += input->delta_time;
			SSortStorage.selected[user_id].fade_timer += input->delta_time;
		}
        
		if(input->mode == BAM_DRAW && NULL != SSortStorage.selected[user_id].tooltip)
			s_sort_draw_tool_tip(input, SSortStorage.selected[user_id].draw[0], SSortStorage.selected[user_id].draw[1], SSortStorage.selected[user_id].tooltip, SSortStorage.selected[user_id].tooltip_description, SSortStorage.tooltip_size, SSortStorage.selected[user_id].tooltip_timer, SSortStorage.tooltip_color[0], SSortStorage.tooltip_color[1], SSortStorage.tooltip_color[2], SSortStorage.tooltip_color[3]);
        
		if(input->axis_count > axis)
		{
            
            
			radius = 1;
            /*	if(SSortStorage.selected[user_id].fade_timer < 0.5)
                    radius = 1 + 4.0 * (1.0 - SSortStorage.selected[user_id].fade_timer / 0.5);
                else if(SSortStorage.selected[user_id].fade_timer < 0.5)
                    SSortStorage.selected[user_id].fade_timer;
                    */
			if(input->mode == BAM_EVENT && betray_button_get(user_id, BETRAY_BUTTON_FACE_A))
			{
				SSortStorage.selected[user_id].click_timer = 0;
				found = 0;
				for(i = 1; i < SEDUCE_SORT_CLICK_COUNT; i++)		
					if(SSortStorage.clicks[i].timer > SSortStorage.clicks[found].timer)	
                    found = i;
				SSortStorage.clicks[found].timer = 0; 	
				SSortStorage.clicks[found].pos[0] = SSortStorage.selected[user_id].pos[0];
				SSortStorage.clicks[found].pos[1] = SSortStorage.selected[user_id].pos[1];
				SSortStorage.clicks[found].pos[2] = SSortStorage.selected[user_id].pos[2];
				SSortStorage.clicks[found].color[0] = 1;
				SSortStorage.clicks[found].color[1] = 1;
				SSortStorage.clicks[found].color[2] = 1;
                
			}
            
			if(input->mode == BAM_DRAW)
			{
                /*		{
                            char text[256];
                            sprintf(text, "click %f fade %f", SSortStorage.selected[user_id].click_timer, SSortStorage.selected[user_id].fade_timer);
                            seduce_text_line_draw(NULL, 0.0, 0.0, SEDUCE_T_SIZE * 5, SEDUCE_T_SPACE, text, 1.0, 1.0, 1.0, 0.5, -1);
                        }	*/
				if(SSortStorage.selected[user_id].click_timer < 0.5)
					radius *= f_spline(SSortStorage.selected[user_id].click_timer / 0.5, 1, -0.5, 1, 1);
                //		radius *= 1.0 - SSortStorage.selected[user_id].click_timer * (1.0 - SSortStorage.selected[user_id].click_timer) * 4.0;
                //	if(SSortStorage.selected[user_id].click_timer < 1.0)
                //		radius = SSortStorage.selected[user_id].click_timer;
                
                
				fade = 1;
				radius *= 0.02;
                /*		if(input->axis[axis].axis[0] < 0.01 && input->axis[axis].axis[0] > -0.01 && input->axis[axis].axis[1] < 0.01 && input->axis[axis].axis[1] > -0.01)
                        {
                            seduce_primitive_circle_add_3d(NULL,
                                                        SSortStorage.selected[user_id].draw[0], SSortStorage.selected[user_id].draw[1], 0,
                                                        1, 0, 0,
                                                        0, 0, 1,
                                                        radius,
                                                        0.0, 1.0,
                                                        0, 1,
                                                        fade, fade, fade, 1,
                                                        fade, fade, fade, 1);
                            seduce_primitive_line_draw(NULL);
                        }else
                        {
                            f = radius / sqrt(input->axis[axis].axis[0] * input->axis[axis].axis[0] + input->axis[axis].axis[1] * input->axis[axis].axis[1]);
                            seduce_primitive_circle_add_3d(NULL,
                                                        SSortStorage.selected[user_id].draw[0], SSortStorage.selected[user_id].draw[1], 0,
                                                        input->axis[axis].axis[0], input->axis[axis].axis[1], 0,
                                                        0, 0, 1,
                                                        radius,
                                                        0.25, 0.5,
                                                        0, 1,
                                                        fade, fade, fade, 1,
                                                        fade, fade, fade, 1);
                            seduce_primitive_circle_add_3d(NULL,
                                                        SSortStorage.selected[user_id].draw[0] + input->axis[axis].axis[0] * SEDUCE_SORT_AXIS_LENGTH, SSortStorage.selected[user_id].draw[1]  + input->axis[axis].axis[1] * SEDUCE_SORT_AXIS_LENGTH, 0,
                                                        input->axis[axis].axis[0], input->axis[axis].axis[1], 0,
                                                        0, 0, 1,
                                                        radius,
                                                        0.75, 0.5,
                                                        0, 1,
                                                        fade, fade, fade, 1,
                                                        fade, fade, fade, 1);
                            seduce_primitive_line_add_3d(NULL,
                                                        SSortStorage.selected[user_id].draw[0] + input->axis[axis].axis[0] * SEDUCE_SORT_AXIS_LENGTH + f * input->axis[axis].axis[1], SSortStorage.selected[user_id].draw[1] + input->axis[axis].axis[1] * SEDUCE_SORT_AXIS_LENGTH - f * input->axis[axis].axis[0], 0,
                                                        SSortStorage.selected[user_id].draw[0] + f * input->axis[axis].axis[1], SSortStorage.selected[user_id].draw[1] - f * input->axis[axis].axis[0], 0,
                                                        fade, fade, fade, 1,
                                                        fade, fade, fade, 1);
                            seduce_primitive_line_add_3d(NULL,
                                                        SSortStorage.selected[user_id].draw[0] + input->axis[axis].axis[0] * SEDUCE_SORT_AXIS_LENGTH - f * input->axis[axis].axis[1], SSortStorage.selected[user_id].draw[1] + input->axis[axis].axis[1] * SEDUCE_SORT_AXIS_LENGTH + f * input->axis[axis].axis[0], 0,
                                                        SSortStorage.selected[user_id].draw[0] - f * input->axis[axis].axis[1], SSortStorage.selected[user_id].draw[1] + f * input->axis[axis].axis[0], 0,
                                                        fade, fade, fade, 1,
                                                        fade, fade, fade, 1);
                            seduce_primitive_line_draw(NULL);
                        }*/
                
			}
            
            
			if(input->axis[axis].axis[0] > 0.01 || input->axis[axis].axis[0] < -0.01 || input->axis[axis].axis[1] > 0.01 || input->axis[axis].axis[1] < -0.01)
			{
                
                
				if(SSortStorage.selected[user_id].fade_timer > 0.5)
					SSortStorage.selected[user_id].fade_timer = 0.5;
                
                
                /*	if(input->mode == BAM_DRAW)
                    {	
    
                        f = sqrt(input->axis[axis].axis[0] * input->axis[axis].axis[0] + input->axis[axis].axis[1] * input->axis[axis].axis[1]);
                        m[0] = -input->axis[axis].axis[1] / f;
                        m[1] = input->axis[axis].axis[0] / f;
                        m[4] = -m[1];
                        m[5] = m[0];
                        m[10] = -1;
                        m[12] = SSortStorage.selected[user_id].draw[0];
                        m[13] = SSortStorage.selected[user_id].draw[1];
                        r_matrix_push(&matrix);
                        r_matrix_matrix_mult(&matrix, m);
                        seduce_object_3d_draw(input, 0, 0, 0, -0.04, SUI_3D_OBJECT_EXTEND, 1, NULL);
                        r_matrix_pop(&matrix);
                        for(i = 1; i < 8; i++)
                        {
                            r_matrix_push(&matrix);
                            f2 = (float)i * (f * axis_length + 0.025) / 7.0;
                            m[12] = SSortStorage.selected[user_id].draw[0] + input->axis[axis].axis[0] / f * f2;
                            m[13] = SSortStorage.selected[user_id].draw[1] + input->axis[axis].axis[1] / f * f2;
                            r_matrix_matrix_mult(&matrix, m);
                            seduce_object_3d_draw(input, 0, 0, 0, -0.04, SUI_3D_OBJECT_EXTEND_CENTER, 1, NULL);
                            r_matrix_pop(&matrix);
                        }
                        r_matrix_push(&matrix);
                        m[12] = SSortStorage.selected[user_id].draw[0] + input->axis[axis].axis[0] * axis_length;
                        m[13] = SSortStorage.selected[user_id].draw[1] + input->axis[axis].axis[1] * axis_length;
                        r_matrix_matrix_mult(&matrix, m);
                        seduce_object_3d_draw(input, 0, 0, 0, -0.04, SUI_3D_OBJECT_EXTEND_TIP, 1, NULL);
                        r_matrix_pop(&matrix);
    
    
    
                        m[12] = SSortStorage.selected[user_id].draw[0] + input->axis[axis].axis[0] * axis_length;
                        m[13] = SSortStorage.selected[user_id].draw[1] + input->axis[axis].axis[1] * axis_length;
    
                    }*/
                
                
				axis_pos[0] = SSortStorage.selected[user_id].pos[0] + input->axis[axis].axis[0] * axis_length;
				axis_pos[1] = SSortStorage.selected[user_id].pos[1] + input->axis[axis].axis[1] * axis_length;
                
                
                //	seduce_element_direction_test(BInputState *input, uint user_id, float dir_x, float dir_y, float pos_x, float pos_y)
				found = -1;
				for(i = 0; i < SSortStorage.element_count; i++)
				{
					if(SSortStorage.element[i].id != SSortStorage.selected[user_id].id || SSortStorage.element[i].part != SSortStorage.selected[user_id].part)
					{
						seduce_element_center_get(i, vec);
						vec[0] -= SSortStorage.selected[user_id].pos[0];
						vec[1] -= SSortStorage.selected[user_id].pos[1];
						if(0.001 < vec[0] * input->axis[axis].axis[0] + vec[1] * input->axis[axis].axis[1])
						{
							seduce_element_center_get(i, center);
							vec[0] = center[0] - axis_pos[0];
							vec[1] = center[1] - axis_pos[1];
							f = vec[0] * vec[0] + vec[1] * vec[1];
							if(f < best)
							{
								best = f;
								found = i;
							}
						}
					}
				}
				if(found != -1)
				{
					if(input->mode == BAM_DRAW)
					{
						seduce_element_center_get(found, vec);
						seduce_object_3d_draw(input, vec[0], vec[1], 0, -0.1, SUI_3D_OBJECT_SELECTRING, 1, NULL);
					}
					if(input->mode == BAM_EVENT)
						if(betray_button_get(user_id, BETRAY_BUTTON_FACE_B))
                        seduce_element_select(user_id, SSortStorage.element[found].id, SSortStorage.element[found].part, SSortStorage.element[found].type, SSortStorage.element[found].pos);
                    
				}	
			}
		}
	}
    
    //	if(input->mode == BAM_MAIN)
    //		seduce_particle_update(input);
    //	seduce_background_particle(input);
    
	if(input->mode == BAM_DRAW)
	{
		r_matrix_set(save);
	}
	if(input->mode == BAM_EVENT)
	{
		void *id;
		id = seduce_element_pointer_id(input, 0, NULL);
		if(betray_button_get(-1, BETRAY_BUTTON_PAUSE))
			SSortStorage.debug_id = id;
		if(betray_button_get(-1, BETRAY_BUTTON_F1 + 11))
			SSortStorage.debug_id = id;
		seduce_element_pointer(input, id, NULL);
	}
    
}

void *seduce_element_colission_test(float *pos, uint *part, uint user_id)
{
	uint i, j, count;
	float best, f, f2, vecs[8], found_pos[2] = {0, 0}, vec[2];
	void *found_id = NULL;
	uint found_part = 0;
	if(part != NULL)
		*part = 0;
	best = 1.0;
	found_pos[0] = pos[0];
	found_pos[1] = pos[1];
    
	
	for(i = 0; i < SSortStorage.element_count && SSortStorage.element[i].user_id != user_id; i++);
	if(i == SSortStorage.element_count)
		user_id = -1;
    
	for(i = SSortStorage.element_count; i != 0;)
	{
		i--;
		if(SSortStorage.element[i].user_id == user_id)
		{
			switch(SSortStorage.element[i].type)
			{
				case SEDUCE_SET_POINT :
                vec[0] = (SSortStorage.element[i].pos[0] - pos[0]) / SSortStorage.element[i].pos[3];
                vec[1] = (SSortStorage.element[i].pos[1] - pos[1]) / SSortStorage.element[i].pos[3];
                f = vec[0] * vec[0] + vec[1] * vec[1];
                if(f < best)
                {
                    best = f;
                    found_id = SSortStorage.element[i].id;
                    found_part = SSortStorage.element[i].part;
                    if(part != NULL)
                        *part = found_part;
                    found_pos[0] = pos[0];
                    found_pos[1] = pos[1];
                }
				break;
				case SEDUCE_SET_LINE :
                vec[0] = SSortStorage.element[i].pos[3] - SSortStorage.element[i].pos[0];
                vec[1] = SSortStorage.element[i].pos[4] - SSortStorage.element[i].pos[1];
                f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
                vec[0] /= f;
                vec[1] /= f;
                f2 = vec[0] * (pos[0] - SSortStorage.element[i].pos[0]) + vec[1] * (pos[1] - SSortStorage.element[i].pos[1]);
                if(f2 > 0)
                {
                    if(f2 < f)
                    {
                        vec[0] = (SSortStorage.element[i].pos[0] + vec[0] * f2 - pos[0]) / SSortStorage.element[i].pos[6];
                        vec[1] = (SSortStorage.element[i].pos[1] + vec[1] * f2 - pos[1]) / SSortStorage.element[i].pos[6];
                        f = vec[0] * vec[0] + vec[1] * vec[1];
                        if(f < best)
                        {
                            best = f;
                            found_id = SSortStorage.element[i].id;
                            found_part = SSortStorage.element[i].part;
                            if(part != NULL)
                                *part = found_part;
                        }
                    }
                }
				break;
				case SEDUCE_SET_TRIANGLE :
                vecs[0] = SSortStorage.element[i].pos[4] - SSortStorage.element[i].pos[1];
                vecs[1] = SSortStorage.element[i].pos[0] - SSortStorage.element[i].pos[3];
                vecs[2] = SSortStorage.element[i].pos[7] - SSortStorage.element[i].pos[4];
                vecs[3] = SSortStorage.element[i].pos[3] - SSortStorage.element[i].pos[6];
                vecs[4] = SSortStorage.element[i].pos[1] - SSortStorage.element[i].pos[7];
                vecs[5] = SSortStorage.element[i].pos[6] - SSortStorage.element[i].pos[0];
                if(seduce_element_triangle_test(pos, SSortStorage.element[i].pos, vecs))
                {
                    if(found_id != NULL)
                    {
                        for(j = 0; j < SSortStorage.element_count; j++)
                        {
                            if(SSortStorage.element[j].id == found_id && SSortStorage.element[j].part == found_part)
                            {
                                if(SSortStorage.element[j].type == SEDUCE_SET_TRIANGLE)
                                {
                                    vecs[0] = SSortStorage.element[j].pos[4] - SSortStorage.element[j].pos[1];
                                    vecs[1] = SSortStorage.element[j].pos[0] - SSortStorage.element[j].pos[3];
                                    vecs[2] = SSortStorage.element[j].pos[7] - SSortStorage.element[j].pos[4];
                                    vecs[3] = SSortStorage.element[j].pos[3] - SSortStorage.element[j].pos[6];
                                    vecs[4] = SSortStorage.element[j].pos[1] - SSortStorage.element[j].pos[7];
                                    vecs[5] = SSortStorage.element[j].pos[6] - SSortStorage.element[j].pos[0];
                                    if(seduce_element_triangle_test(found_pos, SSortStorage.element[i].pos, vecs))
                                        return found_id;
                                }
                                if(SSortStorage.element[j].type == SEDUCE_SET_QUAD)
                                {
                                    vecs[0] = SSortStorage.element[j].pos[4] - SSortStorage.element[j].pos[1];
                                    vecs[1] = SSortStorage.element[j].pos[0] - SSortStorage.element[j].pos[3];
                                    vecs[2] = SSortStorage.element[j].pos[7] - SSortStorage.element[j].pos[4];
                                    vecs[3] = SSortStorage.element[j].pos[3] - SSortStorage.element[j].pos[6];
                                    vecs[4] = SSortStorage.element[j].pos[10] - SSortStorage.element[j].pos[7];
                                    vecs[5] = SSortStorage.element[j].pos[6] - SSortStorage.element[j].pos[9];
                                    vecs[6] = SSortStorage.element[j].pos[1] - SSortStorage.element[j].pos[10];
                                    vecs[7] = SSortStorage.element[j].pos[9] - SSortStorage.element[j].pos[0];
                                    if(seduce_element_quad_test(found_pos, SSortStorage.element[j].pos, vecs))
                                        return found_id;
                                }
                            }
                        }
                        return found_id;
                    }else
                    {
                        if(part != NULL)
                            *part = SSortStorage.element[i].part;
                        return SSortStorage.element[i].id;
                    }
                }
				break;
				case SEDUCE_SET_QUAD :
                vecs[0] = SSortStorage.element[i].pos[4] - SSortStorage.element[i].pos[1];
                vecs[1] = SSortStorage.element[i].pos[0] - SSortStorage.element[i].pos[3];
                vecs[2] = SSortStorage.element[i].pos[7] - SSortStorage.element[i].pos[4];
                vecs[3] = SSortStorage.element[i].pos[3] - SSortStorage.element[i].pos[6];
                vecs[4] = SSortStorage.element[i].pos[10] - SSortStorage.element[i].pos[7];
                vecs[5] = SSortStorage.element[i].pos[6] - SSortStorage.element[i].pos[9];
                vecs[6] = SSortStorage.element[i].pos[1] - SSortStorage.element[i].pos[10];
                vecs[7] = SSortStorage.element[i].pos[9] - SSortStorage.element[i].pos[0];
                if(seduce_element_quad_test(pos, SSortStorage.element[i].pos, vecs))
                {
                    if(found_id != NULL)
                    {
                        for(j = 0; j < SSortStorage.element_count; j++)
                        {
                            if(SSortStorage.element[j].id == found_id && SSortStorage.element[j].part == found_part)
                            {
                                if(SSortStorage.element[j].type == SEDUCE_SET_TRIANGLE)
                                {
                                    vecs[0] = SSortStorage.element[j].pos[4] - SSortStorage.element[j].pos[1];
                                    vecs[1] = SSortStorage.element[j].pos[0] - SSortStorage.element[j].pos[3];
                                    vecs[2] = SSortStorage.element[j].pos[7] - SSortStorage.element[j].pos[4];
                                    vecs[3] = SSortStorage.element[j].pos[3] - SSortStorage.element[j].pos[6];
                                    vecs[4] = SSortStorage.element[j].pos[1] - SSortStorage.element[j].pos[7];
                                    vecs[5] = SSortStorage.element[j].pos[6] - SSortStorage.element[j].pos[0];
                                    if(seduce_element_triangle_test(found_pos, SSortStorage.element[i].pos, vecs))
                                        return found_id;
                                }
                                if(SSortStorage.element[j].type == SEDUCE_SET_QUAD)
                                {
                                    vecs[0] = SSortStorage.element[j].pos[4] - SSortStorage.element[j].pos[1];
                                    vecs[1] = SSortStorage.element[j].pos[0] - SSortStorage.element[j].pos[3];
                                    vecs[2] = SSortStorage.element[j].pos[7] - SSortStorage.element[j].pos[4];
                                    vecs[3] = SSortStorage.element[j].pos[3] - SSortStorage.element[j].pos[6];
                                    vecs[4] = SSortStorage.element[j].pos[10] - SSortStorage.element[j].pos[7];
                                    vecs[5] = SSortStorage.element[j].pos[6] - SSortStorage.element[j].pos[9];
                                    vecs[6] = SSortStorage.element[j].pos[1] - SSortStorage.element[j].pos[10];
                                    vecs[7] = SSortStorage.element[j].pos[9] - SSortStorage.element[j].pos[0];
                                    if(seduce_element_quad_test(found_pos, SSortStorage.element[j].pos, vecs))
                                        return found_id;
                                }
                            }
                        }
                        return found_id;
                    }else
                    {
                        if(part != NULL)
                            *part = SSortStorage.element[i].part;
                        return SSortStorage.element[i].id;
                    }
                }
				break;
			}
		}
	}
	return found_id;
}

void seduce_action_compute(BInputState *input)
{
	uint i, j, count;
	float pos[2];
	void *id;
	if(!SSortStorage.action_computed && input->mode == BAM_EVENT)
	{
		float f, best, vec[2];
		for(i = 0; i < input->pointer_count; i++)
		{
			pos[0] = input->pointers[i].pointer_x;
			pos[1] = input->pointers[i].pointer_y;
			id = seduce_element_colission_test(pos, &SSortStorage.action[i].part, input->pointers[i].user_id);
			if(SSortStorage.action[i].id != id)
			{
				SSortStorage.action[i].id = id;
				
				SSortStorage.action[i].tooltip = NULL;
				SSortStorage.action[i].tooltip_description = NULL;
				SSortStorage.action[i].tooltip_timer = 0;
			}
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				SSortStorage.action[i].id_down_click = id;
		}
		count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		for(; i < count; i++)
		{
			SSortStorage.action[i].id = NULL;
			SSortStorage.action[i].id_down_click = NULL;
			SSortStorage.action[i].tooltip = NULL;
			SSortStorage.action[i].tooltip_description = NULL;
			SSortStorage.action[i].tooltip_timer = 0;
		}
		SSortStorage.action_computed = TRUE;
	}
}

uint seduce_element_pointer(BInputState *input, void *id, uint *part)
{
	uint i, count;
	count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	seduce_action_compute(input);
	for(i = 0; i < count; i++)
	{
		if(SSortStorage.action[i].id == id)
		{
			if(part != NULL)
				*part = SSortStorage.action[i].part;
			return i;
		}
	}
	return -1;
}

void *seduce_element_pointer_id(BInputState *input, uint pointer, uint *part)
{
	seduce_action_compute(input);
	if(part != NULL)
		*part = SSortStorage.action[pointer].part;
	return SSortStorage.action[pointer].id;
}

void *seduce_element_pointer_down_click_id(BInputState *input, uint pointer, uint *part)
{
	seduce_action_compute(input);
	if(part != NULL)
		*part = SSortStorage.action[pointer].part;
	return SSortStorage.action[pointer].id_down_click;
}

void *seduce_element_selected_id(uint user_id, float *pos, uint *part)
{
	if(pos != NULL)
	{
		pos[0] = SSortStorage.selected[user_id].pos[0];
		pos[1] = SSortStorage.selected[user_id].pos[1];
		pos[2] = SSortStorage.selected[user_id].pos[2];
	}
    
	if(part != NULL)
		*part = SSortStorage.selected[user_id].part;
	return SSortStorage.selected[user_id].id;
}

void seduce_element_user_exclusive_begin(uint user_id)
{
	SSortStorage.user_id = user_id;
}

void seduce_element_user_exclusive_end()
{
	SSortStorage.user_id = -1;
}

void seduce_tool_tip(BInputState *input, void *id, char *tooltip, char *extended)
{
	uint i, count;
	if(input->mode == BAM_DRAW)
	{
		count = betray_support_functionality(B_SF_USER_COUNT_MAX);
		for(i = 0; i < count; i++)
		{
			if(SSortStorage.selected[i].id == id && SSortStorage.action[i].tooltip_timer > 0.1)
			{
				SSortStorage.selected[i].tooltip = tooltip;
				SSortStorage.selected[i].tooltip_description = extended;
			}
		}
		count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		for(i = 0; i < count; i++)
		{
			if(SSortStorage.action[i].id == id && SSortStorage.action[i].tooltip_timer > 0.1)
			{
				SSortStorage.action[i].tooltip = tooltip;
				SSortStorage.action[i].tooltip_description = extended;
			}
		}
	}
}

void seduce_tool_tip_settings(float text_size, float red, float green, float blue, float alpha)
{
	SSortStorage.tooltip_size = text_size;
	SSortStorage.tooltip_color[0] = red;
	SSortStorage.tooltip_color[1] = green;
	SSortStorage.tooltip_color[2] = blue;
	SSortStorage.tooltip_color[3] = alpha;
}

void seduce_draw_tool_tip_one_old(BInputState *input, float x, float y)
{
	static float time = 1.0;
	float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, scale = 1, pos[2];
	float color[4] = {0.2, 0.6, 1.0, 1.0};
	char *text = "HERE IS MY TEXT";
	pos[0] = x;
	pos[1] = y + scale * 0.1;
    //	if(input->mode == BAM_MAIN)
    //		seduce_animate(input, &time, input->pointers[0].button[0], 1.5);
	if(input->mode == BAM_DRAW)
	{
		r_matrix_push(NULL);
		matrix[4] = pos[0] - x;
		matrix[5] = pos[1] - y;
		f_normalize2f(&matrix[4]);
		matrix[0] = -matrix[5];
		matrix[1] = matrix[4];
		matrix[12] = pos[0];
		matrix[13] = pos[1];
		r_matrix_matrix_mult(NULL, matrix);
		if((int)(input->minute_time * 100.0) % 5 == 0)
			r_matrix_rotate(NULL, input->minute_time * 360 * 100.0, 0, 1, 0);
		seduce_object_3d_draw(input, 0, -0.016 + time * -0.16, 0, scale * 0.05, SUI_3D_OBJECT_MARKER, 1, color);
		r_matrix_pop(NULL);
        
		if(matrix[4] > 0.1)
			seduce_text_line_draw(NULL, matrix[12] + scale * 0.025, matrix[13] - scale * SEDUCE_T_SIZE * 0.5, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, 1, 1, 1, 1, -1);
		else if(matrix[4] < -0.1)
			seduce_text_line_draw(NULL, matrix[12] - scale * 0.025 - seduce_text_line_length(NULL, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), matrix[13] - scale * SEDUCE_T_SIZE * 0.5, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, 1, 1, 1, 1, -1);
		else if(matrix[5] < 0.0)
			seduce_text_line_draw(NULL, matrix[12] - seduce_text_line_length(NULL, scale * SEDUCE_T_SIZE * 0.5, SEDUCE_T_SPACE, text, -1), matrix[13] - scale * 0.025, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, 1, 1, 1, 1, -1);
		else
			seduce_text_line_draw(NULL, matrix[12] - seduce_text_line_length(NULL, scale * SEDUCE_T_SIZE * 0.5, SEDUCE_T_SPACE, text, -1), matrix[13] + scale * 0.025, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, 1, 1, 1, 1, -1);
        
	}
}


void seduce_draw_tool_tip_one(BInputState *input, float x, float y)
{
	char *tooltip = "MMMM", text[512], *t = "A text byte stream cannot be losslessly converted to UTF-16, due to the possible presence of errors in the byte stream encoding. This causes unexpected and often severe problems attempting to use existing data in a system that uses UTF-16 as an internal encoding. Results are security bugs, DoS";
	float color[4] = {1.0, 1.0, 1.0, 1};
	float pos[2] = {0, 0}, center[3] = {0, 0, 0};
	uint i, length;
	char character;
	static uint progress = 0;
	static float height = 0, time;
	STextBlockMode mode;
    
    
    
	if(input->mode == BAM_MAIN)
		seduce_animate(input, &time, input->pointers[0].button[0], 1.5);
    
    
	if(input->mode == BAM_DRAW)
	{
        
		pos[0] = x;
		if(pos[0] > 0.85)
			pos[0] = 0.85;
		if(pos[0] < -0.85)
			pos[0] = -0.85;
		pos[1] = y;
		pos[1] += height;
		for(i = 0; t[i] != 0; i++)
			text[i] = t[i];
		text[i] = 0;
		seduce_text_line_draw(NULL, pos[0] - seduce_text_line_length(NULL, SEDUCE_T_SIZE * 0.5, SEDUCE_T_SPACE, tooltip, -1), pos[1], SEDUCE_T_SIZE, SEDUCE_T_SPACE, tooltip, 0.2, 0.6, 1.0, 1.0, -1);
        
        
        /*		seduce_background_quad_draw(input, NULL, 0, pos[0] - 0.115, pos[1] + 0.02, 0, 
                                                        pos[0] + 0, pos[1] + 0.03, 0, 
                                                        pos[0] + 0, pos[1] - 0.33, 0, 
                                                        pos[0] - 0.115, pos[1] - 0.32, 0,
                                                            -0.1, -0.2, 1,
                                                            0, 0, 0, 0.7);
        
                seduce_background_quad_draw(input, NULL, 0, pos[0] - 0, pos[1] + 0.03, 0, 
                                                        pos[0] + 0.115, pos[1] + 0.02, 0, 
                                                        pos[0] + 0.115, pos[1] - 0.32, 0, 
                                                        pos[0] - 0, pos[1] - 0.33, 0,
                                                            -0.1, -0.2, 1,
                                                            0, 0, 0, 0.7);*/
        
        //extern SeduceBackgroundObject *seduce_background_object_allocate();
        //extern void seduce_background_shadow_add(SeduceBackgroundObject *object, float *list, uint count, boolean closed, float size);
        
        
		
		center[0] = pos[0];
		center[1] = pos[1] - 0.13;
		if(input->pointers[0].button[0] && !input->pointers[0].last_button[0] && input->draw_id == 0)
			seduce_background_particle_burst(input, center[0], center[1], 20, 0.2, S_PT_SPLAT_ONE);
        
		seduce_background_polygon_flush(input, center, time);
		color[3] = time * time; 
		
		mode.character_position = 0;
		mode.font = NULL;
		mode.red = 1.0;
		mode.green = 1.0; 
		mode.blue = 1.0;
		mode.alpha = 1.0;
		mode.letter_size = SEDUCE_T_SIZE;
		mode.letter_spacing = SEDUCE_T_SPACE;
        
		seduce_text_block_draw(pos[0] - 0.1, pos[1], 0.2, 1.0, SEDUCE_T_LINE_SPACEING, SEDUCE_TBAS_STRETCH, text, 0, &mode, 1);
        
        //	i = seduce_text_block_draw_old(pos[0] - 0.1, pos[1], SEDUCE_T_SIZE, SEDUCE_T_SPACE, 0.2, SEDUCE_T_LINE_SPACEING, 2, SEDUCE_TBAS_STRETCH, text, 0, color, NULL, NULL, NULL, NULL);
		pos[1] -= SEDUCE_T_SIZE * SEDUCE_T_LINE_SPACEING;
		length = seduce_text_line_length(NULL, 1, SEDUCE_T_SPACE, tooltip, -1);
		pos[1] -= 2.0 * 0.2 / length;
		seduce_text_line_draw(NULL, pos[0] - 0.1, pos[1], 0.2 / length, SEDUCE_T_SPACE, tooltip, 0.2, 0.6, 1.0, 1.0, -1);
		pos[1] -= SEDUCE_T_SIZE * SEDUCE_T_LINE_SPACEING;
        
        //	i = seduce_text_block_draw_old(pos[0] - 0.1, pos[1], SEDUCE_T_SIZE, SEDUCE_T_SPACE, 0.2, SEDUCE_T_LINE_SPACEING, i, SEDUCE_TBAS_STRETCH, &text[i], 0, color, NULL, NULL, NULL, NULL);	
	}
    //	height = (height * 0.99) + (float)i * SEDUCE_T_SIZE * 2.0 * 0.01;
    
    
    
    
    
}


void seduce_background_circle_draw(BInputState *input, float pos_x, float pos_y, uint splits, float timer, uint selected);

#define SEDUCE_DEMO_TOGGLE 8
boolean seduce_demo_toggle[SEDUCE_DEMO_TOGGLE] = {TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};

void seduce_test_popup_func(BInputState *input, float time, void *user)
{
	static boolean toggle[8] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
	float vec[2];
	uint i;
	char *names[SEDUCE_DEMO_TOGGLE];
	names[0] = seduce_translate("Show Widget Demo");
	names[1] = seduce_translate("Draw Image");
	names[2] = seduce_translate("Draw Icon list");
	names[3] = seduce_translate("Draw Exclamation Star Field");
	names[4] = seduce_translate("Show Betray settings");
	names[5] = seduce_translate("Show Seduce Color settings");
	names[6] = seduce_translate("Draw Vanishing point");
	names[7] = seduce_translate("Draw Background");
	for(i = 0; i < SEDUCE_DEMO_TOGGLE; i++)
	{
		vec[0] = sin((float)i * PI * 2.0 / 8.0) * 0.3;
		vec[1] = cos((float)i * PI * 2.0 / 8.0) * 0.3;
		seduce_widget_toggle_icon(input, &seduce_demo_toggle[i], &seduce_demo_toggle[i], SUI_3D_OBJECT_CHECKBOXCHECKED, vec[0], vec[1], 0.1, time);
        
		seduce_text_line_draw(NULL, vec[0] + seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, names[i], -1) * -0.5, vec[1] - 0.07, SEDUCE_T_SIZE, SEDUCE_T_SPACE, names[i], 1, 1, 1, 0.7 * time, -1);
        
	}
}


extern void seduce_color_settings(BInputState *input, boolean active);
boolean seduce_background_shape_draw2(BInputState *input, void *id, float a_x, float a_y, float b_x, float b_y, float c_x, float c_y, float d_x, float d_y, float timer, float normal_x, float normal_y, float *center);
void r_matrix_projection_screenf_new(RMatrix *matrix, float *output, float x, float y, float z);

void seduce_element_test(BInputState *input, void *user_pointer)
{
	RMatrix	matrix;
	static float timer = 0, *t, amnimation = 0, background_animation = 0;
	float aspect, pos[3];
	uint i, j;
    
    
	if(input->mode == BAM_MAIN)
	{
		timer += input->delta_time * 0.01;
		if(!seduce_demo_toggle[0])
		{
			amnimation -= input->delta_time * 2.0;
			if(amnimation < 0.0)
				amnimation = 0.0;
		}else
		{
			amnimation += input->delta_time * 2.0;
			if(amnimation > 1.0)
				amnimation = 1.0;
		}
		if(!seduce_demo_toggle[7])
		{
			background_animation -= input->delta_time;
			if(background_animation < 0.0)
				background_animation = 0.0;
		}else
		{
			background_animation += input->delta_time;
			if(background_animation > 1.0)
				background_animation = 1.0;
		}
	}
	t = &timer;
    
	if(betray_button_get(0, BETRAY_BUTTON_Q))
		exit(0);
    
	{
		static float value = 0.5, c[4] = {1, 1, 1, 1};
		static uint selected = 0;
		static boolean toggle[2] = {FALSE, FALSE};
		static uint button;
		char *lables[14] = {"An option", "A selection", "A thing", "Something Else", "Not much", "Slightly more", "7 Hej", "8 Hej", "9 Hopp", "10 Hej", "11 Hopp", "12 Hej", "13 Hopp", "14 Hej"}, *t;
        
		static float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -0.3, 1};
		static float m2[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		static float m3[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		static float center[3] = {0.2, 0.2, 0}, manip_point[3] = {0.5, 0, 0}, manip_point2[3] = {1, 0, 0}, manip_point3[6] = {0.0, 0, 0, 0.5, 0.3, 0}, manip_point4[6] = {-0.5, -0.3, -0.4, 0.5, 0.3, 0.4}, manip_size[3] = {1, 1, 1};
		static char text[32] = {0};
		static char password[32] = {0};
		static double dvalue = 0;
		static int ivalue = 0;
		static uint uvalue = 0;
		float view[3] = {0.0, 0.0, 1};
		uint x, y;
		if(input->mode == BAM_DRAW)
		{
			glClearColor(0.0, 0.0, 0.0, 0);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
		}
		r_matrix_identity(&matrix);
		aspect = betray_screen_mode_get(&x, &y, NULL);
		r_viewport(0, 0, x, y);
        /*	view[0] = input->pointers[0].pointer_x;
            view[1] = input->pointers[0].pointer_y;*/
        //	betray_view_vantage(view);
		r_matrix_frustum(&matrix, -0.01 - view[0] * 0.01, 0.01 - view[0] * 0.01, -0.01 * aspect - view[1] * 0.01, 0.01 * aspect - view[1] * 0.01, 0.01 * view[2], 100.0); /* set frustum */
        
		
		r_matrix_translate(&matrix, -view[0], -view[1], -view[2]); /* move camera bac so we can see Z plane*/
		r_matrix_set(&matrix);
        
        /*		t = seduce_translate("Welcome to the Seduce demo! Press the right mouse button, or put five fingers on the touch screen to activate the Popup Menu where you can toggle on or off all interface elements.");
                for(i = 0; i < 8 && !seduce_demo_toggle[i]; i++);
                if(i == 8)
                    seduce_text_block_draw_old(-0.125, 0.25, SEDUCE_T_SIZE, SEDUCE_T_SPACE, 0.25, 3.0, 10, SEDUCE_TBAS_STRETCH, t, 0, c, NULL, NULL, NULL, NULL);
        */
		if(seduce_demo_toggle[6])
		{
			for(i = 0; i < 21; i++)
			{
				r_primitive_line_3d(1, (float)i * 0.1 - 1.0, -1000.0, 1, (float)i * 0.1 - 1.0, 0, 1, 1, 1, 0.3);
				r_primitive_line_3d(-1, (float)i * 0.1 - 1.0, -1000.0, -1, (float)i * 0.1 - 1.0, 0, 1, 1, 1, 0.3);
			}
			r_primitive_line_flush();
		}
        /*	{
                static uint texture_id = -1;
                if(texture_id == -1)
                    texture_id = gather_file_opengl_texture("image3.tga", G_FORMAT_TARGA_SIMPLE);
                
                if(background_animation > 0.001)
                {
                    r_matrix_push(&matrix);
                    r_matrix_translate(&matrix, 0, 0, -1);
                    seduce_background_image_draw(input, m, -2.2, -1.1, 4.4, 2.2, 0.0, 0.0, 1.0, 1.0, background_animation, center, texture_id);
                    r_matrix_pop(&matrix);
                }
            }*/
        
		r_matrix_push(&matrix);
		r_matrix_matrix_mult(NULL, m);
        
        /*	if(seduce_demo_toggle[1])
            {
                static uint texture_id = -1;
                if(texture_id == -1)
                    texture_id = gather_file_opengl_texture("image.tga", G_FORMAT_TARGA_SIMPLE);
                seduce_background_image_draw(input, m, -0.2, -0.2, 0.4, 0.4, 0.0, 0.0, 1.0, 1.0, 1, center, texture_id);
            }*/
        
		if(seduce_demo_toggle[2])
			for(i = 0; i < SUI_3D_OBJECT_COUNT; i++)
            seduce_object_3d_draw(input, 0.1 * (i / 6) - 0.6, 0.1 * (i % 6) - 0.4, 0, 0.05, i, FALSE, NULL);
        
		if(seduce_demo_toggle[3])
			for(i = 0; i < SUI_3D_OBJECT_COUNT; i++)
            seduce_object_3d_draw(input, f_randnf(i * 3 + 0), f_randnf(i * 3 + 1), f_randnf(i * 3 + 2) * 0.99, 0.05, SUI_3D_OBJECT_MARKER, FALSE, NULL);
        
		seduce_background_shape_matrix_interact(input, m, m, TRUE, TRUE);
		r_matrix_pop(&matrix);
        
		if(amnimation > 0.001)
		{
			float f;
			r_matrix_push(&matrix);
			r_matrix_rotate(&matrix, (value - 0.5) * 360.0, 1, 1, 0.2);
			r_matrix_matrix_mult(&matrix, m3);
			seduce_background_square_draw(input, m3, -0.2, -0.4, 0.4, 0.8, 0.5, 0.1, amnimation);
			seduce_background_shape_matrix_interact(input, m3, m3, TRUE, TRUE);
            /*	{
                    float test[3];
                    r_matrix_projection_surfacef(&matrix, test, 2, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
                //	r_matrix_projection_axisf(&matrix, test, 0, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
                    seduce_object_3d_draw(input, test[0], test[1], test[2], 0.2, SUI_3D_OBJECT_MARKER, FALSE, NULL);
                }*/
            /*		seduce_widget_slider_radial(input, 0.1, 0.15, 0.1, 0.5, amnimation,  &c[0], &c[1], c[0], c[1], c[2], c[0], c[1], c[2]);
                    seduce_widget_slider_radial(input, 0.1, 0.0, 0.1, 0.5, amnimation,  &c[1], &c[2], c[0], c[1], c[2], c[0], c[1], c[2]);
                    seduce_widget_slider_radial(input, 0.1, -0.15, 0.1, 0.5, amnimation,  &c[2], &c[3] ,c[0], c[1], c[2], c[0], c[1], c[2]);
                    seduce_widget_slider_radial(input, &value,  &value, 0.1, -0.3, 0.1, 0.5, 0, 1, amnimation, TRUE, NULL);
        */
            
			
            seduce_popup_detect_icon(input, seduce_test_popup_func, SUI_3D_OBJECT_STOP, 0.5, 0.0, 0.1, amnimation, seduce_test_popup_func, NULL, TRUE, NULL);
            
            
			seduce_widget_color_wheel_radial(input, c, c, -0.1, 0.15, 0.1, 0.5, amnimation);
            //		seduce_widget_button_icon(input, center, SUI_3D_OBJECT_MESSAGE, -0.1, -0.15,  0.1, amnimation);
			seduce_widget_toggle_icon(input, &toggle[1], &toggle[1], SUI_3D_OBJECT_CHECKBOXCHECKED, -0.1, -0.3, 0.1, amnimation);
			seduce_widget_select_radial(input, &selected, &selected, lables, 6, S_PUT_ANGLE, -0.1, 0.0, 0.1, 0.5, amnimation, TRUE);
            
			f = 0.2;
			seduce_background_shape_draw2(input, NULL, -0.01, f + -0.01, 0.2, f + -0.02, 0.2, f + 0.04, -0.01, f + 0.03, amnimation, 0.1, 0.0, center);
			seduce_text_edit_line(input, text, NULL, text, 32, 0.01, f + 0, 0.18, 0.01, "type something", TRUE, NULL, NULL,  1, 1, 1, 0.8, 0.0, 0.0, 0.0, 1.6);	
			
            
			f = 0.1;
			seduce_background_shape_draw2(input, NULL, -0.01, f + -0.01, 0.2, f + -0.02, 0.2, f + 0.04, -0.01, f + 0.03, amnimation, 0.1, 0.0, center);
			seduce_text_edit_obfuscated(input, password, password, 32, 0.01, f + 0, 0.18, 0.01, "Password", TRUE, NULL, NULL, 1, 1, 1, 0.8, 0.0, 0.0, 0.0, 1.6);
            
            
			f = 0.0;
			seduce_background_shape_draw2(input, NULL, -0.01, f + -0.01, 0.2, f + -0.02, 0.2, f + 0.04, -0.01, f + 0.03, amnimation, 0.1, 0.0, center);
			seduce_text_edit_double(input, &dvalue, NULL, &dvalue, 0.01, f + 0, 0.18, 0.01, TRUE, NULL, NULL, 1, 1, 1, 0.8, 0.0, 0.0, 0.0, 1.6);
            
			f = -0.1;
			seduce_background_shape_draw2(input, NULL, -0.01, f + -0.01, 0.2, f + -0.02, 0.2, f + 0.04, -0.01, f + 0.03, amnimation, 0.1, 0.0, center);
			seduce_text_edit_int(input, &ivalue, NULL, &ivalue, 0.01, f + 0, 0.18, 0.01, TRUE, NULL, NULL, 1, 1, 1, 0.8, 0.0, 0.0, 0.0, 1.6);
            
			f = -0.2;
			seduce_background_shape_draw2(input, NULL, -0.01, f + -0.01, 0.2, f + -0.02, 0.2, f + 0.04, -0.01, f + 0.03, amnimation, 0.1, 0.0, center);
			seduce_text_edit_uint(input, &uvalue, NULL, &uvalue, 0.01, f + 0, 0.18, 0.01, TRUE, NULL, NULL, 1, 1, 1, 0.8, 0.0, 0.0, 0.0, 1.6);
            
			
            //	r_matrix_scale(NULL, 0.3, 1, 1);
            
            /*	seduce_manipulator_point(input, NULL, manip_point, manip_point, 1);
                f_normalize3f(manip_point);
                manip_point[0] *= 0.25;
                manip_point[1] *= 0.25;
                manip_point[2] *= 0.25;*/
            
			seduce_manipulator_pos_xyz(input, NULL, manip_point, manip_point, NULL, FALSE, TRUE, TRUE, TRUE, 1, amnimation);
            
            //	seduce_manipulator_point_plane(input, NULL, manip_point2, manip_point2, NULL, FALSE, 2, 1);
            
            //	seduce_manipulator_normal(input, NULL, manip_point, manip_point2, manip_point2, 1);
            
			seduce_manipulator_radius(input, NULL, manip_point, manip_point2, manip_point2, amnimation);
            
            
			seduce_manipulator_scale(input, NULL, manip_point, manip_size, manip_size, NULL, FALSE, TRUE, TRUE, TRUE, 1, amnimation);
            
            //	seduce_manipulator_point_axis(input, NULL, manip_point2, manip_point2, NULL, FALSE, 0, 1);
            
            r_primitive_line_3d(manip_point2[0], manip_point2[1], manip_point2[2], 0, manip_point2[1], manip_point2[2], 1, 1, 1, 1);
            r_primitive_line_3d(0, manip_point2[1], manip_point2[2], 0, 0, manip_point2[2], 1, 1, 1, 1);
            r_primitive_line_3d(manip_point2[0], manip_point2[1], manip_point2[2], manip_point2[0], 0, manip_point2[2], 1, 1, 1, 1);
            r_primitive_line_3d(manip_point2[0], 0, manip_point2[2], 0, 0, manip_point2[2], 1, 1, 1, 1);
            
            
            r_primitive_line_3d(manip_point2[0], manip_point2[1], 0, 0, manip_point2[1], 0, 1, 1, 1, 1);
            r_primitive_line_3d(0, manip_point2[1], 0, 0, 0, 0, 1, 1, 1, 1);
            r_primitive_line_3d(manip_point2[0], manip_point2[1], 0, manip_point2[0], 0, 0, 1, 1, 1, 1);
            r_primitive_line_3d(manip_point2[0], 0, 0, 0, 0, 0, 1, 1, 1, 1);
            
            
            
            r_primitive_line_3d(manip_point2[0], manip_point2[1], manip_point2[2], manip_point2[0], manip_point2[1], 0, 1, 1, 1, 1);
            r_primitive_line_3d(0, manip_point2[1], manip_point2[2], 0, manip_point2[1], 0, 1, 1, 1, 1);
            r_primitive_line_3d(0, 0, manip_point2[2], 0, 0, 0, 1, 1, 1, 1);
            r_primitive_line_3d(manip_point2[0], 0, manip_point2[2], manip_point2[0], 0, 0, 1, 1, 1, 1);
            r_primitive_line_flush();
            
            
            seduce_manipulator_square_cornered(input, NULL, manip_point3, &manip_point3[3], manip_point3, NULL, FALSE, input->pointers[0].button[1], 1, amnimation);
            
            r_primitive_line_3d(manip_point3[0], manip_point3[1], manip_point3[2], 
                                manip_point3[0], manip_point3[4], manip_point3[2], 1, 1, 1, 1);  
            
            r_primitive_line_3d(manip_point3[3], manip_point3[1], manip_point3[2], 
                                manip_point3[3], manip_point3[4], manip_point3[2], 1, 1, 1, 1);  
            
            
            seduce_manipulator_cube_cornered(input, NULL, manip_point4, &manip_point4[3], manip_point4, NULL, FALSE, 0.3, amnimation);
            
            
            r_primitive_line_3d(manip_point4[3], 
                                manip_point4[4], 
                                manip_point4[5], 
                                manip_point4[3], 
                                manip_point4[4], 
                                manip_point4[2], 1, 0, 0, 1);
            
            r_primitive_line_3d(manip_point4[0], 
                                manip_point4[4], 
                                manip_point4[5], 
                                manip_point4[0], 
                                manip_point4[4], 
                                manip_point4[2], 1, 0, 0, 1);
            
            r_primitive_line_3d(manip_point4[0], 
                                manip_point4[1], 
                                manip_point4[5], 
                                manip_point4[0], 
                                manip_point4[1], 
                                manip_point4[2], 1, 0, 0, 1);
            
            r_primitive_line_3d(manip_point4[3], 
                                manip_point4[1], 
                                manip_point4[5], 
                                manip_point4[3], 
                                manip_point4[1], 
                                manip_point4[2], 1, 0, 0, 1);	
            
            r_primitive_line_3d(manip_point[0], 
                                manip_point[1], 
                                manip_point[2], 
                                0, 
                                0, 
                                0, 0, 1, 0, 1);
            
            r_primitive_line_3d(manip_point[0], 
                                0, 
                                0, 
                                0, 
                                0, 
                                0, 0, 1, 0, 1);	
            r_primitive_line_3d(manip_point[0], 
                                manip_point[1], 
                                0, 
                                manip_point[0], 
                                0, 
                                0, 0, 1, 0, 1);	
            r_primitive_line_3d(manip_point[0], 
                                manip_point[1], 
                                manip_point[2], 
                                manip_point[0], 
                                manip_point[1], 
                                0, 0, 1, 0, 1);	
            r_primitive_line_flush();
			if(input->mode == BAM_DRAW)
			{	
                RMatrix	m2;
				float test[3], test2[3], point[3];
				for(i = 0; i < 5; i++)
				{
					point[0] = 0.5;
					point[1] = 0;
					point[2] = (float)i * 0.2;
					r_primitive_line_3d(point[0] + 0.1, point[1], point[2], point[0], point[1], point[2], 1, 0, 0, 1.1);
					r_primitive_line_3d(point[0], point[1] + 0.1, point[2], point[0], point[1], point[2], 0, 1, 0, 1.1);
					r_primitive_line_3d(point[0], point[1], point[2] + 0.1, point[0], point[1], point[2], 0, 0, 1, 1.1);
					
					r_primitive_line_flush();
					
                    //	r_matrix_projection_screenf(NULL, test, point[0], point[1], point[2]);
					r_matrix_projection_screenf(NULL, test2, point[0], point[1], point[2]);
                    /*	r_matrix_set(&m2);
                        r_matrix_identity(&m2);
                        r_matrix_frustum(&m2, -0.01, 0.01, -0.01 * aspect, 0.01 * aspect, 0.01, 10.0);
                        r_matrix_translate(&m2, 0, 0, -1); 
    
                        r_primitive_line_3d(test[0] + 0.1, test[1], 0, test[0], test[1], 0, 0, 1, 1, 1.1);
                        r_primitive_line_3d(test[0], test[1] + 0.1, 0, test[0], test[1], 0, 1, 0, 1, 1.1);
                        r_primitive_line_3d(test[0], test[1],  0.1, test[0], test[1], 0, 1, 1, 0, 1.1);
    
                        r_primitive_line_3d(test2[0], test2[1], 0, test2[0] - 0.2, test2[1], 0, 0, 1, 1, 1.1);
                        r_primitive_line_3d(test2[0], test2[1], 0, test2[0], test2[1] - 0.2, 0, 1, 0, 1, 1.1);
                        r_primitive_line_3d(test2[0], test2[1], 0, test2[0], test2[1], -0.2, 1, 1, 0, 1.1);
                        r_primitive_line_flush();
                        r_matrix_set(&matrix);*/
					r_matrix_projection_worldf(NULL, test2, test2[0], test2[1], test2[2]);
					
					r_primitive_line_3d(test2[0], test2[1], test2[2], test2[0] - 0.1, test2[1], test2[2], 0, 1, 1, 1.1);
					r_primitive_line_3d(test2[0], test2[1], test2[2], test2[0], test2[1] - 0.1, test2[2], 1, 0, 1, 1.1);
					r_primitive_line_3d(test2[0], test2[1], test2[2], test2[0], test2[1], test2[2] - 0.1, 1, 1, 0, 1.1);
					
					r_primitive_line_flush();
					r_matrix_set(&matrix);
				}
			}
			r_matrix_pop(&matrix);
		}
        //	seduce_betray_settings(input, seduce_demo_toggle[4]);
        //	seduce_color_settings(input, seduce_demo_toggle[5]);
        //	seduce_background_shape_draw2(input, NULL, 0, 0, 0.1, 0, 0.1, 0.01, 0, 0.01, 1, 0, 0, NULL);
        
        
        //		t = "Although Hilary Mantel is apparently yet to begin the third volume of her trilogy of novels about Thomas Cromwell, we can be confident of several plot twists that it will not feature. Cromwell will not precipitate a civil war. He will not betray the husband of his foster-sister, with whom he is in love. He will not escape the executioner's block. His downfall is scripted. The history books cannot be cheated. Mantel's Cromwell is as bound to the inevitability of his doom as any prisoner to a rack.";
        //		seduce_text_block_draw_old(0.3, -0.25, 0.01, SEDUCE_T_SPACE, 0.25, 2.0, 10, SEDUCE_TBAS_STRETCH, t, 0, c, NULL, NULL, NULL, NULL);
        
		/*	{
			float output[2];
			seduce_element_add_surface(input, &c[0]);
			seduce_element_surface_project(input, &c[0], output, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
			seduce_widget_button_icon(input, center, SUI_3D_OBJECT_MESSAGE, output[0], output[1],  0.1, amnimation);
		}*/
        
        /*	{
                static float time = 0;
                char *lables[14] = {"Hej", "Hopp", "Hej", "Hopp", "Hej", "Hopp", "Hej", "Hej", "Hopp", "Hej", "Hopp", "Hej", "Hopp", "Hej"};
                seduce_popup_simple(input, 0, input->pointers[0].click_pointer_x[1], input->pointers[0].click_pointer_y[1], lables, 14, &time, input->pointers[0].button[1], 0.1, 0.1, 0.1, 1.0, 1.0, 1.0);
            }*/
        /*	{
                static float value, c[4] = {1, 1, 1, 1};
                s_widget_slider_new(input, -0.1, 0, 0.1, 0, 1, amnimation, &value, &value, c, c, c, c);
            }*/
        
		{
			float red[4] = {1.0, 0.1, 0.3, 1.0};
			if(seduce_widget_button_icon(input, &button, SUI_3D_OBJECT_CLOSE, -0.97, aspect - 0.03,  0.02, 1, red))
			{
				seduce_translate_save("seduce_language_translation.txt");
				exit(0);
			}
		}
	}
    /*	if(input->mode == BAM_DRAW)
            seduce_element_make_sure();*/
    
    
	if(input->mode == BAM_DRAW)
		glEnable(GL_DEPTH_TEST);
	seduce_popup_detect_multitouch(input, NULL, 5, seduce_test_popup_func, NULL);
	seduce_popup_detect_mouse(input, NULL, 1, seduce_test_popup_func, NULL);
	seduce_popup_detect_axis(input, BETRAY_BUTTON_FACE_Y, seduce_test_popup_func, NULL);
    
	seduce_element_endframe(input, FALSE);
	if(input->mode == BAM_EVENT)
	{
		void *id;
		id = seduce_element_pointer_id(input, 0, NULL);
		i = seduce_element_pointer(input, id, NULL);
		SSortStorage.action_computed = FALSE;
	}
    
}
