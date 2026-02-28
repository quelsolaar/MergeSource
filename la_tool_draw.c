#include "la_includes.h"

#include "la_particle_fx.h"

#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_tool.h"
#include "la_draw_overlay.h"
#include <math.h>

#define RING_SECTIONS 17
#define DRAW_CLOSE_RANGE 0.025
#define DRAW_SEGMENT_LENGTH 0.01

struct{
	float		*array;
	uint		array_length;
	uint		array_allocation;
	void		*line_pool;
	void		*ruler_object;
	void		*draw_pool;
	RShader		*line_shader;
	RShader		*draw_shader;
	uint		vertex_tie_length;
	float		last_x;
	float		last_y;
	float		line_length;
	float		*ring;
	uint		*delete_poly_list;
	uint		delete_poly_length;
	uint		*delete_edge_list;
	uint		delete_edge_length;
	uint		*delete_vertex_list;
	uint		delete_vertex_length;
	boolean		select_length;
}GlobalDrawLine;

char *la_draw_shader_vertex = 
"attribute vec3 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform float scroll;" 
"varying float pos;"
"void main()"
"{"
"	pos = vertex.z + scroll;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xy, 0, 1.0);"
"}";

char *la_draw_shader_fragment = 
"varying float pos;"
"void main()"
"{"
"	if(mod(pos, 0.3) > 0.05)"
"		gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);"
"	else"
"		gl_FragColor = vec4(0.2, 0.6, 1.0, 1.0);"
"}";

void la_t_init_circle(float *array, uint sections, float start, float end, float x, float y, float z, float size)
{
	float f;
	uint i;
	for(i = 0; i < sections; i++)
	{
		f = (start + (end - start) * (float)i / (float)sections) * PI * 2.0;
		array[i * 6 + 0] = x + cos(f) * size;
		array[i * 6 + 1] = y + sin(f) * size;
		array[i * 6 + 2] = z;
		f = (start + (end - start) * (float)(1 + i) / (float)sections) * PI * 2.0;
		array[i * 6 + 3] = x + cos(f) * size;
		array[i * 6 + 4] = y + sin(f) * size;
		array[i * 6 + 5] = z;
	}
}

void la_t_init_draw_line(void)
{
	RFormats vertex_format_types[2] = {R_FLOAT, R_FLOAT};
	uint vertex_format_size[2] = {3, 3};
	char buffer[2048];
	float *array, x, y, f;
	uint i = 0, j;
	GlobalDrawLine.array = NULL; 
	GlobalDrawLine.array_length = 0;
	GlobalDrawLine.array_allocation = 0;
	GlobalDrawLine.delete_vertex_length = 0;
	GlobalDrawLine.draw_pool = NULL;
	GlobalDrawLine.line_pool = NULL;
	GlobalDrawLine.draw_shader = r_shader_create_simple(buffer, 2048, la_draw_shader_vertex, la_draw_shader_fragment, "vertex shader");
	GlobalDrawLine.ring = malloc((sizeof *GlobalDrawLine.ring) * RING_SECTIONS * 8 * 2);
	array = malloc((sizeof *array) * 1024 * 1024);
/*	for(x = -0.6; x < 1.65; x += 0.2)
	{
		for(y = -0.6; y < 0.65; y += 0.2)
		{ 
			if(x > 1.01 || x < -0.01 || y > 0.01 || y < -0.01)
			{
				array[i++] = x - 0.0125;
				array[i++] = y;	
				array[i++] = 0;	
				array[i++] = x + 0.0125;
				array[i++] = y;	
				array[i++] = 0;	
				array[i++] = x;
				array[i++] = y - 0.0125;	
				array[i++] = 0;	
				array[i++] = x;
				array[i++] = y + 0.0125;
				array[i++] = 0;		
			}
		}
	}
	for(x = - 4.0; x < 5.05; x += 0.4)
	{
		for(y = - 4.0; y < 4.05; y += 0.4)
		{ 
			if(x > 1.41 || x < -0.41 || y > 0.41 || y < -0.41)
			{
				array[i++] = x - 0.025;
				array[i++] = y;	
				array[i++] = 0;	
				array[i++] = x + 0.025;
				array[i++] = y;	
				array[i++] = 0;	
				array[i++] = x;
				array[i++] = y - 0.025;	
				array[i++] = 0;	
				array[i++] = x;
				array[i++] = y + 0.025;	
				array[i++] = 0;	
			}
		}
	}
	la_t_init_circle(&array[i], 32, 0, 1, 0, 0, 0.2, 0.15);
	i += 6 * 32;
	la_t_init_circle(&array[i], 32, 0, 1, 0, 0, 0.0, 0.25);
	i += 6 * 32;
	la_t_init_circle(&array[i], 32, 0, 1, 0, 0, 0.1, 0.3);
	i += 6 * 32;
	la_t_init_circle(&array[i], 32, 0, 1, 0, 0, -0.2, 0.2);
	i += 6 * 32;
	la_t_init_circle(&array[i], 32, 0, 1, 0, 0, -0.1, 0.4);
	i += 6 * 32;
	la_t_init_circle(&array[i], 32, 0, 1, 0, 0, 0.0, 0.8);
	i += 6 * 32;
	la_t_init_circle(&array[i], 32, 0, 1, 0, 0, 0.05, 0.4);
	i += 6 * 32;

	la_t_init_circle(&array[i], 32, 0.25, 0.75, 0, 0, 0.0, 0.05);
	i += 6 * 32;
	la_t_init_circle(&array[i], 32, 0.75, 1.25, 1, 0, 0.0, 0.05);
	i += 6 * 32;
	array[i++] = 0;
	array[i++] = 0.05;	
	array[i++] = 0;	
	array[i++] = 1;
	array[i++] = 0.05;	
	array[i++] = 0;	

	array[i++] = 0;
	array[i++] = -0.05;	
	array[i++] = 0;	
	array[i++] = 1;
	array[i++] = -0.05;	
	array[i++] = 0;	*/

	array[i++] = -10;
	array[i++] = -0.05;	
	array[i++] = 0;	
	array[i++] = 11;
	array[i++] = -0.05;	
	array[i++] = 0;

	array[i++] = -10;
	array[i++] = 0.05;	
	array[i++] = 0;	
	array[i++] = 11;
	array[i++] = 0.05;	
	array[i++] = 0;
	j = 0;
	for(x = -10; x < 11.001; x += 0.2)
	{
		if(j % 10 == 0)
			f = 0.3;
		else if(j % 5 == 0)
			f = 0.2;
		else if(j % 2 == 0)
			f = 0.15;	
		else
			f = 0.13;
		array[i++] = x;
		array[i++] = 0.1;	
		array[i++] = 0;	
		array[i++] = x;
		array[i++] = f;	
		array[i++] = 0;

		array[i++] = x;
		array[i++] = -0.1;	
		array[i++] = 0;	
		array[i++] = x;
		array[i++] = -f;	
		array[i++] = 0;
		j++;
	}


	GlobalDrawLine.line_pool = r_array_allocate(i / 3, vertex_format_types, vertex_format_size, 1, 0);
	r_array_load_vertex(GlobalDrawLine.line_pool, NULL, array, 0, i / 3);



	GlobalDrawLine.ruler_object = seduce_primitive_line_object_allocate();

	seduce_primitive_line_add_3d(GlobalDrawLine.ruler_object,
							-10, -0.05, 0, 11, -0.05, 0,
							0.6, 0.6, 0.6, 1,
							0.6, 0.6, 0.6, 1);
	seduce_primitive_line_add_3d(GlobalDrawLine.ruler_object,
							-10, 0.05, 0, 11, 0.05, 0,
							0.6, 0.6, 0.6, 1,
							0.6, 0.6, 0.6, 1);
	j = 0;
	for(x = -10; x < 11.001; x += 0.2)
	{
		if(j % 10 == 0)
			f = 0.3;
		else if(j % 5 == 0)
			f = 0.2;
		else if(j % 2 == 0)
			f = 0.15;	
		else
			f = 0.13;
		seduce_primitive_line_add_3d(GlobalDrawLine.ruler_object,
								x, 0.1, 0, x, f, 0, 
								0.6, 0.6, 0.6, 1,
								0.6, 0.6, 0.6, 1);
		seduce_primitive_line_add_3d(GlobalDrawLine.ruler_object,
								x, -0.1, 0, x, -f, 0,
								0.6, 0.6, 0.6, 1,
								0.6, 0.6, 0.6, 1);
		j++;
	}
}
void la_t_new_draw_line(float x, float y)
{
	GlobalDrawLine.array_length = 0;
	GlobalDrawLine.select_length = FALSE;
	GlobalDrawLine.last_x = x;
	GlobalDrawLine.last_y = y;
	GlobalDrawLine.line_length = 0;
}

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

	/*for(i = 0; i < ref_length; i++)
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
	}*/
	r_primitive_line_flush();
	return del;
}


/*

*/
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
         /* pos[0] = pos[0] / -1;
            pos[1] = pos[1] / -1;
            if(pos[2] > 0)*/
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
      /*  pos[0] = pos[0] / -1;
        pos[1] = pos[1] / -1;*/
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
//	shader = r_shader_presets_get(P_SP_COLOR_UNIFORM);
//	r_shader_set(shader);
//	r_shader_vec4_set(NULL, r_shader_uniform_location(shader, "color"), 0.1, 0.15, 0.2, 1);
//	r_array_section_draw(GlobalDrawLine.line_pool, NULL, GL_LINES, 0, -1);
	seduce_primitive_line_draw(GlobalDrawLine.ruler_object, 1.0, 1.0, 1.0, 1.0);
	r_matrix_pop(&la_world_matrix);
	r_matrix_set(reset);
}