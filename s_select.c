
#include <stdlib.h>
#include <math.h>
#include "betray.h"
#include "seduce.h"

#define RING_SECTIONS 17
#define DRAW_CLOSE_RANGE 0.025
#define DRAW_SEGMENT_LENGTH 0.01


char *seduce_draw_shader_vertex = 
"attribute vec3 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform float scroll;" 
"varying float pos;"
"void main()"
"{"
"	pos = vertex.z + scroll;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xy, 0, 1.0);"
"}";

char *seduce_draw_shader_fragment_old  = 
"varying float pos;"
"uniform vec4 color;" 
"void main()"
"{"
"	if(mod(pos, 0.1) > 0.03)"
"		gl_FragColor = vec4(0.2, 0.2, 0.2, 1.0);"
"	else"
"		gl_FragColor = vec4(color);"
"}";


char *seduce_draw_shader_fragment= 
"varying float pos;"
"uniform vec4 trail;" 
"uniform vec4 color;" 
"void main()"
"{"
"	if(trail.b > -pos)"
"	{"
"		if(trail.g > -pos)"
"		{"
"			gl_FragColor = vec4(1, 0, 0, 1.0);"
"		}else"
"			gl_FragColor = vec4(0, 1, 0, 1.0);"
"	}else"
"	{"
"		if(trail.a > -pos)"
"		{"
"			gl_FragColor = vec4(0, 1, 1, 1.0);"
"		}else"
"			gl_FragColor = vec4(0, 0, 1, 1.0);"

"	}"
"}";

#define SEDUCE_LINE_SELECT_SEGMENTS 32000
#define SEDUCE_LINE_SELECT_SIMULTANIUS_MAX 16


typedef struct{
	uint		user_id;
	uint		pointer_id;
	float		*array;
	uint		array_length;
	uint		array_allocation;
	float		distance;
	float		trail[3];
	boolean		completed;
}SeduceSelectEvent;

typedef struct{
	SeduceSelectEvent *events;
	uint event_count;
	RShader	*line_shader;
	void *pool;
	uint pool_size;
}SeduceSelect;

SeduceSelect seduce_global_select;

void seduce_select_init(void)
{
	uint i;
	seduce_global_select.events = malloc((sizeof *seduce_global_select.events) * SEDUCE_LINE_SELECT_SIMULTANIUS_MAX);
	for(i = 0; i < SEDUCE_LINE_SELECT_SIMULTANIUS_MAX; i++)
	{
		seduce_global_select.events[i].array = NULL;
		seduce_global_select.events[i].array_allocation = 0;
		seduce_global_select.events[i].array_length = 0;
	}
	seduce_global_select.event_count = 0;
	seduce_global_select.line_shader = r_shader_create_simple(NULL, 0, seduce_draw_shader_vertex, seduce_draw_shader_fragment, "Line Shader");
	seduce_global_select.pool = NULL;
	seduce_global_select.pool_size = 0;
}

STypeInState seduce_select_draw(BInputState *input, void *id, uint user, float red, float green, float blue)
{
	STypeInState state = S_TIS_IDLE;
	float x, y, dist;
	uint i, j;
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			{
				for(j = 1; j < input->pointers[i].button_count && !input->pointers[i].button[j]; j++);
				if(j == input->pointers[i].button_count && id == seduce_element_pointer_id(input, i, NULL))
				{
					for(j = 0; j < seduce_global_select.event_count && (seduce_global_select.events[j].completed || seduce_global_select.events[j].user_id != input->pointers[i].user_id); j++);
					if(j == seduce_global_select.event_count)
					{

 						for(j = 0; j < seduce_global_select.event_count; j++)
						{
							if(seduce_global_select.events[j].user_id == input->pointers[i].user_id)
							{
								free(seduce_global_select.events[j].array);
								seduce_global_select.events[j].array = NULL;
								seduce_global_select.events[j--] = seduce_global_select.events[--seduce_global_select.event_count];
								seduce_global_select.events[seduce_global_select.event_count].array = NULL;
							}
						}
					}
					if(seduce_global_select.event_count < SEDUCE_LINE_SELECT_SIMULTANIUS_MAX)
					{
						j = seduce_global_select.event_count;
						if(seduce_global_select.events[j].array == NULL)
						{
							seduce_global_select.events[j].array_allocation = SEDUCE_LINE_SELECT_SEGMENTS;
							seduce_global_select.events[j].array = malloc((sizeof *seduce_global_select.events[j].array) * 6 * seduce_global_select.events[j].array_allocation);
						}
						seduce_global_select.events[j].array[0] = input->pointers[i].pointer_x;
						seduce_global_select.events[j].array[1] = input->pointers[i].pointer_y;
						seduce_global_select.events[j].array[2] = 0;
						seduce_global_select.events[j].user_id = input->pointers[i].user_id;
						seduce_global_select.events[j].pointer_id = i;
						seduce_global_select.events[j].array_length = 1;
						seduce_global_select.events[j].completed = FALSE;
						seduce_global_select.events[j].distance = 0;
						seduce_global_select.events[j].trail[0] = 0.2;
						seduce_global_select.events[j].trail[1] = 0.4;
						seduce_global_select.events[j].trail[2] = 0.6;
						seduce_global_select.event_count++;
					}
				}
			}
		}
		for(i = 0; i < seduce_global_select.event_count; i++)
		{
			if(!seduce_global_select.events[i].completed)
			{
				j = seduce_global_select.events[i].pointer_id;
				x = seduce_global_select.events[i].array[seduce_global_select.events[i].array_length * 6 - 6] - input->pointers[j].pointer_x;
				y = seduce_global_select.events[i].array[seduce_global_select.events[i].array_length * 6 - 5] - input->pointers[j].pointer_y;
				dist = x * x + y * y;
				if(seduce_global_select.events[i].array_length == seduce_global_select.events[i].array_allocation)
				{
					seduce_global_select.events[i].array_allocation += SEDUCE_LINE_SELECT_SEGMENTS;
					seduce_global_select.events[i].array = realloc(seduce_global_select.events[i].array, (sizeof *seduce_global_select.events[i].array) * 6 * seduce_global_select.events[i].array_allocation);
				}
				if(dist > 0.0001 * 0.0001)
				{
					dist = sqrt(dist) + seduce_global_select.events[j].distance;
					seduce_global_select.events[i].distance = dist;
					seduce_global_select.events[i].array[seduce_global_select.events[i].array_length * 6 - 3] = input->pointers[j].pointer_x;
					seduce_global_select.events[i].array[seduce_global_select.events[i].array_length * 6 - 2] = input->pointers[j].pointer_y;
					seduce_global_select.events[i].array[seduce_global_select.events[i].array_length * 6 - 1] = dist;
					seduce_global_select.events[i].array[seduce_global_select.events[i].array_length * 6 + 0] = input->pointers[j].pointer_x;
					seduce_global_select.events[i].array[seduce_global_select.events[i].array_length * 6 + 1] = input->pointers[j].pointer_y;
					seduce_global_select.events[i].array[seduce_global_select.events[i].array_length * 6 + 2] = dist;
					seduce_global_select.events[i].array_length++;
				}
				if(!input->pointers[seduce_global_select.events[i].pointer_id].button[0])
				{
					seduce_global_select.events[i].completed = TRUE;
					if(seduce_global_select.events[j].distance > 0.1)
						state = S_TIS_DONE;
				}
			}
		}
	}
	if(input->mode == BAM_DRAW)
	{
		j = 0;
		for(i = 0; i < seduce_global_select.event_count; i++)
			if(!seduce_global_select.events[i].completed && seduce_global_select.events[i].array_length > j)
				j = seduce_global_select.events[i].array_length;
		if(j > 1)
		{		
			j--;
			if(j > seduce_global_select.pool_size)
			{		
				RFormats vertex_format_types = R_FLOAT;
				uint vertex_format_size[1] = {3};
				seduce_global_select.pool_size = j + SEDUCE_LINE_SELECT_SEGMENTS;
				if(seduce_global_select.pool != NULL)
					r_array_free(seduce_global_select.pool);
				seduce_global_select.pool = r_array_allocate(seduce_global_select.pool_size * 2, &vertex_format_types, vertex_format_size, 1, 0);
			}
			r_shader_set(seduce_global_select.line_shader);
			r_shader_vec4_set(seduce_global_select.line_shader, r_shader_uniform_location(seduce_global_select.line_shader, "color"), red, green, blue, 1.0);

			for(i = 0; i < seduce_global_select.event_count; i++)
			{
				if(!seduce_global_select.events[i].completed && seduce_global_select.events[i].array_length > 1)
				{
					
					seduce_global_select.events[i].trail[0] = seduce_global_select.events[i].trail[0] * 0.9 + 0.1 * (seduce_global_select.events[i].distance + 0.2);
					seduce_global_select.events[i].trail[1] = seduce_global_select.events[i].trail[1] * 0.9 + 0.1 * (seduce_global_select.events[i].trail[0] + 0.2);
					seduce_global_select.events[i].trail[2] = seduce_global_select.events[i].trail[2] * 0.9 + 0.1 * (seduce_global_select.events[i].trail[1] + 0.2);

					r_array_load_vertex(seduce_global_select.pool, NULL, seduce_global_select.events[i].array, 0, (seduce_global_select.events[i].array_length - 1) * 2);
					r_shader_float_set(seduce_global_select.line_shader, r_shader_uniform_location(seduce_global_select.line_shader, "scroll"), -seduce_global_select.events[i].distance);
					r_shader_vec4_set(seduce_global_select.line_shader, r_shader_uniform_location(seduce_global_select.line_shader, "trail"),
						0, 
						seduce_global_select.events[i].trail[0] - seduce_global_select.events[i].distance,
						seduce_global_select.events[i].trail[1] - seduce_global_select.events[i].distance,
						seduce_global_select.events[i].trail[2] - seduce_global_select.events[i].distance);
					r_array_section_draw(seduce_global_select.pool, NULL, GL_LINES, 0, (seduce_global_select.events[i].array_length - 1) * 2);
				}
			}

		/*	for(i = 0; i < seduce_global_select.event_count; i++)
			{
				if(!seduce_global_select.events[i].completed && seduce_global_select.events[i].array_length > 1)
					for(j = 0; j < seduce_global_select.events[i].array_length - 1; j++)
						r_primitive_line_2d(seduce_global_select.events[i].array[6 * j + 0] + 0.01,
											seduce_global_select.events[i].array[6 * j + 1],
											seduce_global_select.events[i].array[6 * j + 3] + 0.01,
											seduce_global_select.events[i].array[6 * j + 4], 1, 0, 0, 1);
				r_primitive_line_flush();
			}*/

		}else if(seduce_global_select.pool != NULL)
		{
			r_array_free(seduce_global_select.pool);
			seduce_global_select.pool = NULL;
			seduce_global_select.pool_size = 0;
		}
	}
	for(i = 0; i < seduce_global_select.event_count; i++)
		if(!seduce_global_select.events[i].completed && seduce_global_select.events[i].distance > 0.1)
			return S_TIS_ACTIVE;
	return state;
}

boolean seduce_select_test(float x, float y, float *pos_a, float *pos_b)
{
	if((pos_a[1] > y && pos_b[1] < y) || (pos_a[1] < y && pos_b[1] > y))
         if((((y - pos_b[1]) / (pos_a[1] - pos_b[1])) * (pos_a[0] - pos_b[0])) > x - pos_b[0])
			 return TRUE;
	return FALSE;
}

boolean seduce_select_query_pos(BInputState *input, uint user_id, float x, float y)
{
	uint i, j, count, front;
	for(i = 0; i < seduce_global_select.event_count; i++)
	{
		if((user_id == -1 || user_id == seduce_global_select.events[i].user_id) && seduce_global_select.events[i].array_length > 1)
		{
			front = 0;
			count = seduce_global_select.events[i].array_length - 1;
			for(j = 0; j < count; j++)
				if(seduce_select_test(x, y, &seduce_global_select.events[i].array[j * 6], &seduce_global_select.events[i].array[j * 6 + 3]))
					front++;
			if(seduce_select_test(x, y, &seduce_global_select.events[i].array[count * 6], &seduce_global_select.events[i].array[0]))
				front++;
			if(front % 2 == 1)
				return TRUE;
		}
	}
	return FALSE;
}


boolean seduce_select_query_id(BInputState *input, uint user_id, void *id)
{
	float pos[3];
	if(!seduce_element_position_get(id, pos))
		return FALSE;
	return seduce_select_query_pos(input, user_id, pos[0], pos[1]);
}



/*
void la_t_draw_line_add(float x, float y, boolean add)
{
	uint i;
	float dist, tie_length;

	if(GlobalDrawLine.array_length + 2 > GlobalDrawLine.array_allocation)
	{
		RFormats vertex_format_types = R_FLOAT;
		uint vertex_format_size[1] = {3};
		GlobalDrawLine.array_allocation += 4096;
		GlobalDrawLine.array = realloc(GlobalDrawLine.array, (sizeof *GlobalDrawLine.array) * GlobalDrawLine.array_allocation * 3 * 2);
		if(GlobalDrawLine.draw_pool != NULL)
			r_array_free(GlobalDrawLine.draw_pool);
		GlobalDrawLine.draw_pool = r_array_allocate(GlobalDrawLine.array_allocation * 2, &vertex_format_types, vertex_format_size, 1, 0);
		r_array_load_vertex(GlobalDrawLine.draw_pool, NULL, GlobalDrawLine.array, 0, GlobalDrawLine.array_length * 2);
	}

	dist = sqrt((GlobalDrawLine.last_x - x) * (GlobalDrawLine.last_x - x) + (GlobalDrawLine.last_y - y) * (GlobalDrawLine.last_y - y));
	if(add && (dist > 0.001 || GlobalDrawLine.array_length != 0))
	{
		i = 6 * GlobalDrawLine.array_length;
		GlobalDrawLine.array[i++] = GlobalDrawLine.last_x;
		GlobalDrawLine.array[i++] = GlobalDrawLine.last_y;
		GlobalDrawLine.array[i++] = GlobalDrawLine.line_length;
		GlobalDrawLine.line_length += dist;
		GlobalDrawLine.array[i++] = x;
		GlobalDrawLine.array[i++] = y;
		GlobalDrawLine.array[i] = GlobalDrawLine.line_length;
		if(GlobalDrawLine.select_length == FALSE)
			if(DRAW_CLOSE_RANGE * DRAW_CLOSE_RANGE < (GlobalDrawLine.last_x - GlobalDrawLine.array[0]) * (GlobalDrawLine.last_x - GlobalDrawLine.array[0]) + (GlobalDrawLine.last_y - GlobalDrawLine.array[1]) * (GlobalDrawLine.last_y - GlobalDrawLine.array[1]))
				GlobalDrawLine.select_length = TRUE;
		GlobalDrawLine.last_x = x;
		GlobalDrawLine.last_y = y;
		r_array_load_vertex(GlobalDrawLine.draw_pool, NULL, &GlobalDrawLine.array[6 * GlobalDrawLine.array_length], 2 * GlobalDrawLine.array_length, 2);
		GlobalDrawLine.array_length++;
	}
	r_shader_set(GlobalDrawLine.draw_shader);
	r_shader_float_set(GlobalDrawLine.draw_shader, r_shader_uniform_location(GlobalDrawLine.draw_shader, "scroll"), -GlobalDrawLine.line_length);
	r_array_section_draw(GlobalDrawLine.draw_pool, NULL, GL_LINES, 0, GlobalDrawLine.array_length * 2);
	r_primitive_line_flush();
}

boolean draw_line_delete_test(double *vertex, double *a, double *b, boolean *del)
{
	double c[2], d[2];
	float start, end;
	if(a[2] > 0 || b[2] > 0 || GlobalDrawLine.array == NULL)
		return FALSE;
	c[0] = GlobalDrawLine.array[0];
	c[1] = GlobalDrawLine.array[1];
	d[0] = GlobalDrawLine.array[GlobalDrawLine.array_length * 6 - 3];
	d[1] = GlobalDrawLine.array[GlobalDrawLine.array_length * 6 - 2];
	start = (a[0] - b[0]) * (c[1] - b[1]) + (a[1] - b[1]) * (b[0] - c[0]);
	end = (a[0] - b[0]) * (d[1] - b[1]) + (a[1] - b[1]) * (b[0] - d[0]);
	if((start > 0 && end < 0) || (start < 0 && end > 0))
	{
		start = (c[0] - d[0]) * (a[1] - d[1]) + (c[1] - d[1]) * (d[0] - a[0]);
		end = (c[0] - d[0]) * (b[1] - d[1]) + (c[1] - d[1]) * (d[0] - b[0]);
		if((start > 0 && end < 0) || (start < 0 && end > 0))
		{
			*del = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

boolean la_t_draw_select_menu_test(void)
{
	if(GlobalDrawLine.select_length)
	{
		uint i;
		i = 6 * GlobalDrawLine.array_length - 3;
		if(DRAW_CLOSE_RANGE * DRAW_CLOSE_RANGE > 
								(GlobalDrawLine.array[i] - GlobalDrawLine.array[0]) *
								(GlobalDrawLine.array[i] - GlobalDrawLine.array[0]) + 
								(GlobalDrawLine.array[i + 1] - GlobalDrawLine.array[1]) * 
								(GlobalDrawLine.array[i + 1] - GlobalDrawLine.array[1]))
			return TRUE;
	}
	return FALSE;
}

boolean la_t_draw_line_test_delete(void)
{
	ENode *node;
	double *vertex, a[3], b[3], c[3], d[3];
	boolean del = FALSE;
	uint vertex_length, *ref, ref_length, i;

	if(!GlobalDrawLine.select_length)
		return FALSE;

	udg_get_geometry(&vertex_length, &ref_length, &vertex, &ref, NULL);
	la_do_polygon_delete_execute();
	for(i = 0; i < ref_length; i++)
	{
		if(ref[i * 4] < vertex_length && ref[i * 4 + 1] < vertex_length && ref[i * 4 + 2] < vertex_length)
		{
			seduce_view_projection_screend(NULL, a, vertex[ref[i * 4] * 3], vertex[ref[i * 4] * 3 + 1], vertex[ref[i * 4] * 3 + 2]);
			seduce_view_projection_screend(NULL, b, vertex[ref[i * 4 + 1] * 3], vertex[ref[i * 4 + 1] * 3 + 1], vertex[ref[i * 4 + 1] * 3 + 2]);
			seduce_view_projection_screend(NULL, c, vertex[ref[i * 4 + 2] * 3], vertex[ref[i * 4 + 2] * 3 + 1], vertex[ref[i * 4 + 2] * 3 + 2]);

			if((a[0] - b[0]) * (c[1] - b[1]) + (a[1] - b[1]) * (b[0] - c[0]) > 0)
			{
				if(draw_line_delete_test(vertex, a, b, &del))
					udg_polygon_delete(i);
				else if(draw_line_delete_test(vertex, b, c, &del))
					udg_polygon_delete(i);
				else if(ref[i * 4 + 3] > vertex_length)
				{
					if(draw_line_delete_test(vertex, c, a, &del))
						udg_polygon_delete(i);
				}
				else
				{	
					seduce_view_projection_screend(NULL, d, vertex[ref[i * 4 + 3] * 3], vertex[ref[i * 4 + 3] * 3 + 1], vertex[ref[i * 4 + 3] * 3 + 2]);
					if(draw_line_delete_test(vertex, c, d, &del))
						udg_polygon_delete(i);
					else if(draw_line_delete_test(vertex, d, a, &del))
						udg_polygon_delete(i);
				}
			}
		}
	}
	ref = udg_get_edge_data(&i);
	while(i != 0)
	{
		i--;
		udg_get_vertex_pos(c, ref[i * 2]);
		seduce_view_projection_screend(NULL, a, c[0], c[1], c[2]);
		udg_get_vertex_pos(d, ref[i * 2 + 1]);
		seduce_view_projection_screend(NULL, b, d[0], d[1], d[2]);
		if(draw_line_delete_test(vertex, a, b, &del))
		{
		//	la_pfx_create_dust_line(d, c);
			udg_destroy_edge(i);
		}
	}
	return del;
}

boolean la_t_draw_line_draw_delete_overlay(void)
{
	ENode *node;
	double *vertex,	a[3], b[3], c[3], d[3];
	boolean del = FALSE;
	uint vertex_length, *ref, ref_length, i, local[4];
	udg_get_geometry(&vertex_length, &ref_length, &vertex, &ref, NULL);
	if(!GlobalDrawLine.select_length)
		return FALSE;
	la_do_polygon_delete_clear();
	del = p_find_line_intersect(GlobalDrawLine.array, &GlobalDrawLine.array[GlobalDrawLine.array_length * 6 - 3], la_do_polygon_delete);

	for(i = 0; i < ref_length; i++)
	{
		local[0] = ref[i * 4];
		local[1] = ref[i * 4 + 1];
		local[2] = ref[i * 4 + 2];
		local[3] = ref[i * 4 + 3];
		if(ref[i * 4] < vertex_length && ref[i * 4 + 1] < vertex_length && ref[i * 4 + 2] < vertex_length)
		{
			seduce_view_projection_screend(NULL, a, vertex[local[0] * 3], vertex[local[0] * 3 + 1], vertex[local[0] * 3 + 2]);
			seduce_view_projection_screend(NULL, b, vertex[local[1] * 3], vertex[local[1] * 3 + 1], vertex[local[1] * 3 + 2]);
			seduce_view_projection_screend(NULL, c, vertex[local[2] * 3], vertex[local[2] * 3 + 1], vertex[local[2] * 3 + 2]);
			if((a[0] - b[0]) * (c[1] - b[1]) + (a[1] - b[1]) * (b[0] - c[0]) > 0)
			{
				if(local[3] > vertex_length)
				{			
					if(draw_line_delete_test(vertex, a, b, &del) ||
						draw_line_delete_test(vertex, b, c, &del) || 
						draw_line_delete_test(vertex, c, a, &del))
						la_do_polygon_delete(i);
				}
				else
				{	
					seduce_view_projection_screend(NULL, d, vertex[local[3] * 3], vertex[local[3] * 3 + 1], vertex[local[3] * 3 + 2]);
					if(draw_line_delete_test(vertex, a, b, &del) ||
						draw_line_delete_test(vertex, b, c, &del) || 
						draw_line_delete_test(vertex, b, d, &del) || 
						draw_line_delete_test(vertex, d, a, &del))
						la_do_polygon_delete(i);
				}
			}
		}
	}
	ref = udg_get_edge_data(&i);
	while(i != 0)
	{
		i--;
		seduce_view_projection_screend(NULL, a, vertex[ref[i * 2] * 3], vertex[ref[i * 2] * 3 + 1], vertex[ref[i * 2] * 3 + 2]);
		seduce_view_projection_screend(NULL, b, vertex[ref[i * 2 + 1] * 3], vertex[ref[i * 2 + 1] * 3 + 1], vertex[ref[i * 2 + 1] * 3 + 2]);
	//	if(draw_line_delete_test(vertex, a, b, &del))
	//		la_do_edge_delete_air(a, b);
	}
	r_primitive_line_flush();
	return del;
}

boolean la_t_draw_line_test_select(SelectionMode mode)
{
	double *vertex, pos[3], center[3] = {0, 0, 0}, sum = 0;
	boolean output = FALSE;
	uint length, i, k, front, *ref;
	UNDOTag	*tag;
	if(!GlobalDrawLine.select_length)
		return FALSE;
    udg_get_geometry(&length, NULL, &vertex, NULL, NULL);
    if(GlobalDrawLine.array != NULL)
		la_t_draw_line_add(GlobalDrawLine.array[0], GlobalDrawLine.array[1], TRUE);
    for(i = 0; i < length; i++)
    {
        if(vertex[i * 3] != V_REAL64_MAX)
        {
            front = 0;
            seduce_view_projection_screend(NULL, pos, vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);

            for(k = 0; k < GlobalDrawLine.array_length * 6; k += 6)
                if((GlobalDrawLine.array[k + 1] > pos[1] && GlobalDrawLine.array[k + 4] < pos[1]) || (GlobalDrawLine.array[k + 1] < pos[1] && GlobalDrawLine.array[k + 4] > pos[1]))
                    if((((pos[1] - GlobalDrawLine.array[k + 4]) / (GlobalDrawLine.array[k + 1] - GlobalDrawLine.array[k + 4])) * (GlobalDrawLine.array[k] - GlobalDrawLine.array[k + 3])) > pos[0] - GlobalDrawLine.array[k + 3])
                        front++;
            if(front % 2 == 1)
            {
                output = TRUE;
				sum++;
				center[0] += vertex[i * 3];
				center[1] += vertex[i * 3 + 1];
				center[2] += vertex[i * 3 + 2];
                switch(mode)
                {
					case SM_SELECT :
						udg_set_select(i, 1);
					break;
					case SM_DESELECT :
						udg_set_select(i, 0);
					break;					
					case SM_SUB :
						if(udg_get_select(i) - 0.25 > 0)
							udg_set_select(i, udg_get_select(i) - 0.25);
						else
							udg_set_select(i, 0);
					break;
					case SM_ADD :
						if(udg_get_select(i) + 0.25 < 1)
							udg_set_select(i, udg_get_select(i) + 0.25);
						else
							udg_set_select(i, 1);
					break;	
				}
			}
		}
	}

	tag = udg_get_tags(&length);
    for(i = 0; i < length; i++)
	{
        front = 0;
        seduce_view_projection_screend(NULL, pos, tag[i].vec[0], tag[i].vec[1], tag[i].vec[2]);

        // if(pos[2] > 0)
        for(k = 0; k < GlobalDrawLine.array_length * 6; k += 6)
            if((GlobalDrawLine.array[k + 1] > pos[1] && GlobalDrawLine.array[k + 4] < pos[1]) || (GlobalDrawLine.array[k + 1] < pos[1] && GlobalDrawLine.array[k + 4] > pos[1]))
                if((((pos[1] - GlobalDrawLine.array[k + 4]) / (GlobalDrawLine.array[k + 1] - GlobalDrawLine.array[k + 4])) * (GlobalDrawLine.array[k] - GlobalDrawLine.array[k + 3])) > pos[0] - GlobalDrawLine.array[k + 3])
                    front++;
        if(front % 2 == 1)
        {
            output = TRUE;
			sum++;
			center[0] += tag[i].vec[0];
			center[1] += tag[i].vec[1];
			center[2] += tag[i].vec[2];
            switch(mode)
            {
				case SM_SELECT :
					udg_select_tag(i, 1);
				break;
				case SM_DESELECT :
					udg_select_tag(i, 0);
				break;					
				case SM_SUB :
					if(tag[i].select - 0.25 > 0)
						udg_select_tag(i, tag[i].select - 0.25);
					else
						udg_set_select(i, 0);
				break;
				case SM_ADD :
					if(tag[i].select + 0.25 < 1)
						udg_select_tag(i, tag[i].select + 0.25);
					else
						udg_select_tag(i, 1);
				break;	
			}
		}
	}

	if(la_t_tm_hiden() && output)
	{
		la_t_tm_hide(FALSE);
		la_t_tm_place(center[0] / sum, center[1] / sum, center[2] / sum);
	}
	return output;
}


void la_t_draw_line_draw_overlay(double *start, double *end)
{
	float s[3], e[3], camera[3], m[16], f;
	RMatrix *reset;
	RShader *shader;
	uint32 vertex_count, i;
	double *vertex;
	seduce_view_camera_getf(NULL, camera);
	
	s[0] = start[0];
	s[1] = start[1];
	s[2] = start[2];
	e[0] = end[0];
	e[1] = end[1];
	e[2] = end[2];
	f = sqrt((start[0] - end[0]) * (start[0] - end[0]) + 
			(start[1] - end[1]) * (start[1] - end[1]) +
			(start[2] - end[2]) * (start[2] - end[2]));
	f = 0.15 * seduce_view_distance_camera_get(NULL);
	f_matrixxzf(m, s, e, camera);
	m[0] *= f; 
	m[1] *= f; 
	m[2] *= f; 
	m[4] *= f; 
	m[5] *= f; 
	m[6] *= f; 
	m[8] *= f; 
	m[9] *= f; 
	m[10] *= f; 
	reset = r_matrix_get();
	r_matrix_set(&la_world_matrix);
	r_matrix_push(&la_world_matrix);
	r_matrix_matrix_mult(&la_world_matrix, m);
	shader = r_shader_presets_get(P_SP_COLOR_UNIFORM);
	r_shader_set(shader);
	r_shader_vec4_set(NULL, r_shader_uniform_location(shader, "color"), 0.1, 0.15, 0.2, 1);
	r_array_section_draw(GlobalDrawLine.line_pool, NULL, GL_LINES, 0, -1);
	r_matrix_pop(&la_world_matrix);
	r_matrix_set(reset);
}*/