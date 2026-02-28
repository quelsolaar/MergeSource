#include "la_includes.h"

#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_tool.h"
#include "forge.h"
#include "la_particle_fx.h"
#include "la_draw_overlay.h"



struct{
	float		*active_vertex;
	float		*active_vertex_shadow;
	float		*active_vertex_color;
	float		*move_vertex;
	float		*move_vertex_shadow;
	float		*move_vertex_color;
	float		*tag_select;
	float		*tag_select_shadow;
	float		*tag_select_color;
	float		*tag_unselect;
	float		*tag_unselect_shadow;
	float		*tag_unselect_color;
	float		*delete_array;
	uint		delete_array_length;
	uint		delete_array_allocation;
	RShader		*delete_shader;
	float		delete_execute_timer;
	RShader		*poly_shader;
	RShader		*vertex_shader;
	RShader		*line_shader;
	RShader		*base_shader;
	LoOverlay	current_overlay;
}GlobalOverlay;



char *la_poly_shader_vertex = 
"attribute vec4 vertex;"
"attribute vec4 normal;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 NormalMatrix;"
"uniform mat4 transform;"
"uniform vec4 color_a;"
"uniform vec4 color_b;"
"uniform vec4 select_a;"
"uniform vec4 select_b;"
"uniform float distance;"
"varying vec4 color;"
"void main()"
"{"
"	vec4 n, fog, v;"
"	n = NormalMatrix * vec4(normalize(normal.xyz), 1.0);"
"	v = vec4(vertex.xyz, 1.0);"
"	gl_Position = ModelViewProjectionMatrix * mix(v, transform * v, vertex.a);"
"   fog = vec4(2.0) - gl_Position.zzzz / vec4(distance);"
"	color = mix(mix(color_a * fog, select_a, normal.a), mix(color_b * fog, select_b, normal.a), 1.0 - abs(n.z));"
"}";

char *la_poly_shader_fragment = 
"varying vec4 color;"
"void main()"
"{"
"	gl_FragColor = color;"
"}";


char *la_line_shader_vertex = 
"attribute vec3 vertex;"
"attribute vec4 select;"
"varying vec4 col;"
"varying vec4 bcol;"
"varying vec2 fade;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 transform;"

"uniform vec4 select_color;"
"uniform vec4 crease_color;"
"uniform vec4 base_color;"
"uniform vec4 fog_color;"
"uniform float distance;"
"void main()"
"{"
"	float f;"
"	vec4 v;"
"	fade = select.gb;"
"	col = select_color * select.rrrr;"
"	v = vec4(vertex.xyz, 1.0);"
"	gl_Position = ModelViewProjectionMatrix * mix(v, transform * v, select.r);"
//"   bcol = mix(fog_color, base_color, 1.0 / (1.0 + gl_Position.z * distance));"
"	f = 4.0 * gl_Position.z / distance;"
"   bcol = (base_color + crease_color * select.aaaa) * vec4(1.0 / (1.0 + f * f));"
//"   bcol = vec4(f);"
"}";

char *la_line_shader_fragment = 

"varying vec4 col;"
"varying vec4 bcol;"
"varying vec2 fade;"
"void main()"
"{"
"	gl_FragColor = (bcol * vec4(0.45 - (fade.x * fade.y)) + col);"
//"	gl_FragColor = bcol;"
"}";

char *la_delete_shader_vertex = 
"attribute vec3 vertex;"
"attribute vec3 merge;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform float animate;"
"void main()"
"{"
"	vec4 n, fog;"
"	gl_Position = ModelViewProjectionMatrix * vec4(mix(vertex, merge, animate), 1.0);"
"}";

char *la_delete_shader_fragment = 
"uniform vec4 color;"
"void main()"
"{"
"   if(mod(gl_FragCoord.x + gl_FragCoord.y, 10.0) > 3.0)"
"		discard;"
"	gl_FragColor = color;"
"}";


char *la_base_shader_vertex = 
"attribute vec4 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform float maxlengt;"
"varying float color;"
"void main()"
"{"
"	color = vertex.a * maxlengt;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);"
"}";

char *la_base_shader_fragment = 
"varying float color;"
"void main()"
"{"
"	gl_FragColor = vec4(color, color * color, 0.2, 1.0) * vec4(0.4, 0.2, 1.0, 1.0);"
"}";

/*
char *la_vertex_shader_vertex = 
"attribute vec3 vertex;"
"attribute vec4 params;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 NormalMatrix;"
"uniform vec2 size;"
"uniform vec2 variance;"
"uniform float distance;"
"uniform float time;"
"varying vec4 color;"
"varying vec2 uv;"
"void main()"
"{"
"	vec4 v;"
"	vec2 s;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);"
"	s = params.xy * (size + variance * vec2(sin(time + params.b)));"
"	gl_Position += gl_Position.zzzz * vec4(s, 0, 0);"
"	uv = params.xy;"
"	color = mix(vec4(0, 0, 1, 0), vec4(1, 0, 0, 0), 1.0 - abs(gl_Position.z * distance));"
"}";*/

char *la_vertex_shader_vertex_old = 
"attribute vec3 vertex;"
"attribute vec4 params;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 NormalMatrix;"
"uniform vec2 size;"
"uniform vec2 variance;"
"uniform float distance;"
"uniform float time;"
"uniform vec4 color_a;"
"uniform vec4 color_b;"
"uniform vec4 select_a;"
"uniform vec4 select_b;"
"uniform vec3 light;"
"varying vec4 color;"
"varying vec2 uv;"
"void main()"
"{"
"	vec4 v;"
"	vec2 s;"
"   vec3 l;"
"	l = (vertex - light) / vec3(distance * 0.02);"
"	s = params.xy * (size * vec2(0.8 + params.a) + variance * vec2(sin(time + params.b))) * vec2(distance);"
//"	s = params.xy * vec2(1000.0) * vec2(distance);"
"	v = vec4(vertex, 1.0) + vec4(0, 0, 0, 0) * NormalMatrix;"
"	gl_Position = ModelViewProjectionMatrix * v;"
"	uv = params.xy;"
"	color = vec4(1, 0, 0, 1) + mix(mix(color_a, select_b, params.a), mix(color_b, select_a, params.a), 1.5 - abs(gl_Position.z / distance)) + vec4(l, 0.0) / vec4(1.0 + dot(l, l));"
"}";


char *la_vertex_shader_vertex = 
"attribute vec3 vertex;"
"attribute vec4 params;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 NormalMatrix;"
"uniform vec2 size;"
"uniform vec2 variance;"
"uniform float distance;"
"uniform float time;"
"uniform vec4 color_a;"
"uniform vec4 color_b;"
"uniform vec4 select_a;"
"uniform vec4 select_b;"
"uniform vec3 light;"
"varying vec4 color;"
"varying vec2 uv;"
"void main()"
"{"
"	vec4 v;"
"	vec2 s;"
"   vec3 l;"
"	s = params.xy * (size * vec2(0.8 + params.a) + variance * vec2(sin(time + params.b))) * vec2(distance);"
"	v = vec4(vertex, 1.0) + vec4(s, 0, 0) * NormalMatrix;"
"	gl_Position = ModelViewProjectionMatrix * v;"
"	uv = params.xy;"
"	color = mix(mix(color_a, select_b, params.a), mix(color_b, select_a, params.a), 1.5 - abs(gl_Position.z / distance)) + vec4(l, 1.0) / vec4(1.0 + dot(l, l));"
"}";


char *la_vertex_shader_fragment = 
"varying vec4 color;"
"varying vec2 uv;"
"void main()"
"{"
"	if(dot(uv, uv) > 1.0)"
"		discard;"	
"	gl_FragColor = color;"
//"	if(dot(uv, uv) > 1.0)"
//"		gl_FragColor = vec4(1.0);"
"}";


void la_do_mesh_clear(LoOverlay *overlay)
{
	overlay->poly_pool = NULL;
	overlay->line_pool = NULL;
	overlay->vertex_pool = NULL;
	overlay->base_pool = NULL;
	overlay->surface_version = -1;
}

void la_do_mesh_free(LoOverlay *overlay)
{
	r_array_free(overlay->poly_pool);
	r_array_free(overlay->line_pool);
	r_array_free(overlay->vertex_pool);
	if(overlay->base_pool != NULL)
		r_array_free(overlay->base_pool);
	la_do_mesh_clear(overlay);
}

#define LA_SHADOW 0.22

void la_draw_set_vec2(float *array, uint pos, float a, float b)
{
	array += pos * 2;
	*(array++) = a;
	*(array) = b;
}

void la_draw_set_vec3(float *array, uint pos, float a, float b, float c)
{
	array += pos * 3;
	*(array++) = a;
	*(array++) = b;
	*(array) = c;
}

void la_draw_set_vec4(float *array, uint pos, float a, float b, float c, float d)
{
	array += pos * 4;
	*(array++) = a;
	*(array++) = b;
	*(array++) = c;
	*(array) = d;
}

void la_create_shadow_edge(float size, uint count, float *shadow, float *color, float *square)
{
/*	uint i;
	float pos_a[2], pos_b[2], temp[2], r;
	pos_a[1] = -(square[(count - 1) * 2] - square[0]);
	pos_a[0] = (square[count * 2 - 1] - square[1]);
	r  = sqrt(pos_a[0] * pos_a[0] + pos_a[1] * pos_a[1]);
	pos_a[1] /= r;
	pos_a[0] /= r;
	for(i = 0; i < count; i++)
	{
		pos_b[1] = -(square[i * 2] - square[((i + 1) % count) * 2]);
		pos_b[0] = (square[i * 2 + 1] - square[((i + 1) % count) * 2 + 1]);
		r = sqrt(pos_b[0] * pos_b[0] + pos_b[1] * pos_b[1]);
		pos_b[1] /= r;
		pos_b[0] /= r;
		r = (pos_a[0] + pos_b[0]) * pos_b[0] + (pos_a[1] + pos_b[1]) * pos_b[1];
		temp[0] = size * (pos_a[0] + pos_b[0]) / r;
		temp[1] = size * (pos_a[1] + pos_b[1]) / r;
		la_draw_set_vec2(shadow, i * 4 + 2, square[i * 2], square[i * 2 + 1]);
		la_draw_set_vec4(color, i * 4 + 2, 0, 0, 0, LA_SHADOW);
		la_draw_set_vec2(shadow, i * 4 + 3, square[i * 2] + temp[0], square[i * 2 + 1] + temp[1]);
		la_draw_set_vec4(color, i * 4 + 3, 0.3, 0.3, 0.3, 0);
		la_draw_set_vec2(shadow, ((i + 1) % count) * 4 + 0, square[i * 2] + temp[0], square[i * 2 + 1] + temp[1]);
		la_draw_set_vec4(color, ((i + 1) % count) * 4 + 0, 0.3, 0.3, 0.3, 0);
		la_draw_set_vec2(shadow, ((i + 1) % count) * 4 + 1, square[i * 2], square[i * 2 + 1]);
		la_draw_set_vec4(color, ((i + 1) % count) * 4 + 1, 0, 0, 0, LA_SHADOW);
		pos_a[0] = pos_b[0];
		pos_a[1] = pos_b[1];
	}*/
}


void la_do_init(void)
{
	char buffer[2048];
	uint i;
	float temp, square[8] = {0.006, 0.01, 0.003, 0.038, -0.003, 0.038, -0.006, 0.01};
	GlobalOverlay.active_vertex = malloc((sizeof * GlobalOverlay.active_vertex) * 4 * 4 * 2);
	GlobalOverlay.active_vertex_shadow = malloc((sizeof * GlobalOverlay.active_vertex_shadow) * 16 * 4 * 2);
	GlobalOverlay.active_vertex_color = malloc((sizeof * GlobalOverlay.active_vertex_color) * 16 * 4 * 4);
	for(i = 0; i < 4; i++)
	{
		square[0] = 0.013 * sin((float)i * 3.14 * 0.5 - 0.4);
		square[1] = 0.013 * cos((float)i * 3.14 * 0.5 - 0.4);
		square[2] = 0.045 * sin((float)i * 3.14 * 0.5 - 0.05);
		square[3] = 0.045 * cos((float)i * 3.14 * 0.5 - 0.05);
		square[4] = 0.045 * sin((float)i * 3.14 * 0.5 + 0.05);
		square[5] = 0.045 * cos((float)i * 3.14 * 0.5 + 0.05);
		square[6] = 0.013 * sin((float)i * 3.14 * 0.5 + 0.4);
		square[7] = 0.013 * cos((float)i * 3.14 * 0.5 + 0.4);
		la_draw_set_vec2(GlobalOverlay.active_vertex, i * 4 + 0, square[0], square[1]);
		la_draw_set_vec2(GlobalOverlay.active_vertex, i * 4 + 1, square[2], square[3]);
		la_draw_set_vec2(GlobalOverlay.active_vertex, i * 4 + 2, square[4], square[5]);
		la_draw_set_vec2(GlobalOverlay.active_vertex, i * 4 + 3, square[6], square[7]);
		la_create_shadow_edge(0.005, 4, &GlobalOverlay.active_vertex_shadow[i * 4 * 2], &GlobalOverlay.active_vertex_color[i * 4 * 4], square);
	}
	GlobalOverlay.move_vertex = malloc((sizeof * GlobalOverlay.move_vertex) * 4 * 8 * 2);
	GlobalOverlay.move_vertex_shadow = malloc((sizeof * GlobalOverlay.move_vertex_shadow) * 16 * 8 * 2);
	GlobalOverlay.move_vertex_color = malloc((sizeof * GlobalOverlay.move_vertex_color) * 16 * 8 * 4);
	for(i = 0; i < 8; i++)
	{
		square[0] = 0.02 * sin((float)i * 3.14 * 0.25);
		square[1] = 0.02 * cos((float)i * 3.14 * 0.25);
		square[2] = 0.025 * sin((float)i * 3.14 * 0.25);
		square[3] = 0.025 * cos((float)i * 3.14 * 0.25);
		square[4] = 0.025 * sin((float)i * 3.14 * 0.25 + 0.3);
		square[5] = 0.025 * cos((float)i * 3.14 * 0.25 + 0.3);
		square[6] = 0.017 * sin((float)i * 3.14 * 0.25 + 0.3);
		square[7] = 0.017 * cos((float)i * 3.14 * 0.25 + 0.3);
		la_draw_set_vec2(GlobalOverlay.move_vertex, i * 4 + 0, square[0], square[1]);
		la_draw_set_vec2(GlobalOverlay.move_vertex, i * 4 + 1, square[2], square[3]);
		la_draw_set_vec2(GlobalOverlay.move_vertex, i * 4 + 2, square[4], square[5]);
		la_draw_set_vec2(GlobalOverlay.move_vertex, i * 4 + 3, square[6], square[7]);
		la_create_shadow_edge(0.005, 4, &GlobalOverlay.move_vertex_shadow[i * 4 * 2], &GlobalOverlay.move_vertex_color[i * 4 * 4], square);
	}

	GlobalOverlay.tag_select = malloc((sizeof * GlobalOverlay.tag_select) * 4 * 6 * 2);
	GlobalOverlay.tag_select_shadow = malloc((sizeof * GlobalOverlay.tag_select_shadow) * 16 * 6 * 2);
	GlobalOverlay.tag_select_color = malloc((sizeof * GlobalOverlay.tag_select_color) * 16 * 6 * 4);
	for(i = 0; i < 6; i++)
	{
		square[0] = 0.01 * sin((float)i * 3.14 * 0.33333333);
		square[1] = 0.01 * cos((float)i * 3.14 * 0.33333333);
		square[2] = 0.02 * sin((float)i * 3.14 * 0.33333333 + 0.2);
		square[3] = 0.02 * cos((float)i * 3.14 * 0.33333333 + 0.2);
		square[4] = 0.02 * sin((float)i * 3.14 * 0.33333333 + 0.3);
		square[5] = 0.02 * cos((float)i * 3.14 * 0.33333333 + 0.3);
		square[6] = 0.01 * sin((float)i * 3.14 * 0.33333333 + 0.5);
		square[7] = 0.01 * cos((float)i * 3.14 * 0.33333333 + 0.5);
		la_draw_set_vec2(GlobalOverlay.tag_select, i * 4 + 0, square[0], square[1]);
		la_draw_set_vec2(GlobalOverlay.tag_select, i * 4 + 1, square[2], square[3]);
		la_draw_set_vec2(GlobalOverlay.tag_select, i * 4 + 2, square[4], square[5]);
		la_draw_set_vec2(GlobalOverlay.tag_select, i * 4 + 3, square[6], square[7]);
		la_create_shadow_edge(0.005, 4, &GlobalOverlay.tag_select_shadow[i * 4 * 2], &GlobalOverlay.tag_select_color[i * 4 * 4], square);
	}

	GlobalOverlay.tag_unselect = malloc((sizeof * GlobalOverlay.tag_unselect) * 4 * 6 * 2);
	GlobalOverlay.tag_unselect_shadow = malloc((sizeof * GlobalOverlay.tag_unselect_shadow) * 16 * 6 * 2);
	GlobalOverlay.tag_unselect_color = malloc((sizeof * GlobalOverlay.tag_unselect_color) * 16 * 6 * 4);
	for(i = 0; i < 6; i++)
	{
		square[0] = 0.01 * sin((float)i * 3.14 * 0.33333333);
		square[1] = 0.01 * cos((float)i * 3.14 * 0.33333333);
		square[2] = 0.013 * sin((float)i * 3.14 * 0.33333333);
		square[3] = 0.013 * cos((float)i * 3.14 * 0.33333333);
		square[4] = 0.013 * sin((float)i * 3.14 * 0.33333333 + 0.5);
		square[5] = 0.013 * cos((float)i * 3.14 * 0.33333333 + 0.5);
		square[6] = 0.01 * sin((float)i * 3.14 * 0.33333333 + 0.5);
		square[7] = 0.01 * cos((float)i * 3.14 * 0.33333333 + 0.5);
		la_draw_set_vec2(GlobalOverlay.tag_unselect, i * 4 + 0, square[0], square[1]);
		la_draw_set_vec2(GlobalOverlay.tag_unselect, i * 4 + 1, square[2], square[3]);
		la_draw_set_vec2(GlobalOverlay.tag_unselect, i * 4 + 2, square[4], square[5]);
		la_draw_set_vec2(GlobalOverlay.tag_unselect, i * 4 + 3, square[6], square[7]);
		la_create_shadow_edge(0.005, 4, &GlobalOverlay.tag_unselect_shadow[i * 4 * 2], &GlobalOverlay.tag_unselect_color[i * 4 * 4], square);
	}
		
	GlobalOverlay.delete_array = NULL;
	GlobalOverlay.delete_array_length = 0;
	GlobalOverlay.delete_execute_timer = 0;



	GlobalOverlay.line_shader = r_shader_create_simple(buffer, 2048, la_line_shader_vertex, la_line_shader_fragment, "color font");
	r_shader_set(GlobalOverlay.line_shader);
	r_shader_state_set_blend_mode(GlobalOverlay.line_shader, R_BM_SRC_ALPHA, R_BM_ONE_MINUS_SRC_ALPHA);
	r_shader_state_set_blend_mode(GlobalOverlay.line_shader, R_BM_ONE, R_BM_ONE);
	r_shader_state_set_depth_test(GlobalOverlay.line_shader, R_DT_ALWAYS);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.line_shader, "base_color"), 0.6, 1.6, 2.0, 0.15);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.line_shader, "fog_color"), 0.1, 1.0, 0.1, 0.15);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.line_shader, "select_color"), 0.6, 0.4, 0.1, 0.5);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.line_shader, "crease_color"), 3.9, 0.0, 0, 0.5);

	GlobalOverlay.poly_shader = r_shader_create_simple(buffer, 2048, la_poly_shader_vertex, la_poly_shader_fragment, "poly shader");
	r_shader_set(GlobalOverlay.poly_shader);
	r_shader_state_set_blend_mode(GlobalOverlay.poly_shader, R_BM_ONE, R_BM_ONE);
	r_shader_state_set_depth_test(GlobalOverlay.poly_shader, R_DT_ALWAYS);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.poly_shader, "color_a"), 0.0, 0.0, 0.01, 0.5);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.poly_shader, "color_b"), 0.01, 0.02, 0.03, 0.3);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.poly_shader, "select_a"), 0.3, 0.1, 0.0, 0.3);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.poly_shader, "select_b"), 0.2, 0.0, 0.1, 0.5);

	
	GlobalOverlay.vertex_shader = r_shader_create_simple(buffer, 2048, la_vertex_shader_vertex, la_vertex_shader_fragment, "vertex shader");
	r_shader_set(GlobalOverlay.vertex_shader);
	r_shader_state_set_blend_mode(GlobalOverlay.vertex_shader, R_BM_ONE, R_BM_ONE);
	r_shader_state_set_depth_test(GlobalOverlay.vertex_shader, R_DT_ALWAYS);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.vertex_shader, "color_a"), 0.04, 0.12, 0.28, 0.5);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.vertex_shader, "color_b"), 0.16, 0.28, 0.4, 0.3);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.vertex_shader, "select_a"), 0.6, 0.2, 0.0, 0.3);
	r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.vertex_shader, "select_b"), 0.4, 0.0, 0.2, 0.5);

	GlobalOverlay.delete_shader = r_shader_create_simple(buffer, 2048, la_delete_shader_vertex, la_delete_shader_fragment, "delete shader");
	
	r_shader_state_set_blend_mode(GlobalOverlay.delete_shader, R_BM_ONE, R_BM_ONE);
	r_shader_state_set_depth_test(GlobalOverlay.delete_shader, R_DT_ALWAYS);
	la_do_mesh_clear(&GlobalOverlay.current_overlay);

	
	GlobalOverlay.base_shader = r_shader_create_simple(buffer, 2048, la_base_shader_vertex, la_base_shader_fragment, "delete shader");

	r_shader_state_set_blend_mode(GlobalOverlay.base_shader, R_BM_ONE, R_BM_ONE);
	r_shader_state_set_depth_test(GlobalOverlay.base_shader, R_DT_ALWAYS);
}

void la_do_edge_select(double *vertex_a, double *vertex_b)
{
	float a[2], b[2];
	a[0] = vertex_a[0];
	a[1] = vertex_a[1];
	b[0] = vertex_b[0] - vertex_a[0];
	b[1] = vertex_b[1] - vertex_a[1];
	r_primitive_line_2d(a[0] + b[1] * 0.04, a[1] - b[0] * 0.04, a[0] - b[1] * 0.04, a[1] + b[0] * 0.04, 0.2, 0.8, 1, 1.0);
	r_primitive_line_2d(a[0] - b[1] * 0.04, a[1] + b[0] * 0.04, a[0] + (b[0] * 0.4) - b[1] * 0.06, a[1] + (b[1] * 0.4) + b[0] * 0.06, 0.2, 0.8, 1, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * 0.4) - b[1] * 0.06, a[1] + (b[1] * 0.4) + b[0] * 0.06, a[0] + (b[0] * 0.4) + b[1] * 0.06, a[1] + (b[1] * 0.4) - b[0] * 0.06, 0.2, 0.8, 1, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * 0.4) + b[1] * 0.06, a[1] + (b[1] * 0.4) - b[0] * 0.06, a[0] + b[1] * 0.04, a[1] - b[0] * 0.04, 0.2, 0.8, 1, 1.0);
	
	r_primitive_line_2d(a[0] + (b[0] * 0.6) + b[1] * 0.06, a[1] + (b[1] * 0.6) - b[0] * 0.06, 
		a[0] + (b[0] * 0.6) - b[1] * 0.06, a[1] + (b[1] * 0.6) + b[0] * 0.06, 
		0.2, 0.8, 1, 1.0);

	r_primitive_line_2d(a[0] + (b[0] * 0.6) - b[1] * 0.06, a[1] + (b[1] * 0.6) + b[0] * 0.06, 
		a[0] + b[0] - b[1] * 0.04, a[1] + b[1] + b[0] * 0.04, 
		0.2, 0.8, 1, 1.0);

	r_primitive_line_2d(a[0] + b[0] - b[1] * 0.04, a[1] + b[1] + b[0] * 0.04, a[0] + b[0] + b[1] * 0.04, a[1] + b[1] - b[0] * 0.04, 0.2, 0.8, 1, 1.0);

	r_primitive_line_2d(a[0] + (b[0] * 0.6) + b[1] * 0.06, a[1] + (b[1] * 0.6) - b[0] * 0.06, 
		a[0] + b[0] + b[1] * 0.04, a[1] + b[1] - b[0] * 0.04, 
		0.2, 0.8, 1, 1.0);

	r_primitive_line_flush();
}

void la_do_edge_delete_air(double *vertex_a, double *vertex_b)
{
	float a[2], b[2];
	a[0] = vertex_a[0];
	a[1] = vertex_a[1];
	b[0] = vertex_b[0] - vertex_a[0];
	b[1] = vertex_b[1] - vertex_a[1];
	r_primitive_line_2d(a[0] + b[0] * 0.1, a[1] + b[1] * 0.1, a[0] + b[0] * 0.2 + b[1] * 0.06, a[1] + b[1] * 0.2 + b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + b[0] * 0.2 + b[1] * 0.06, a[1] + b[1] * 0.2 + b[0] * 0.06, a[0] + b[0] * 0.8 + b[1] * 0.06, a[1] + b[1] * 0.8 + b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + b[0] * 0.8 + b[1] * 0.06, a[1] + b[1] * 0.8 + b[0] * 0.06, a[0] + b[0] * 0.9, a[1] + b[1] * 0.9, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + b[0] * 0.1, a[1] + b[1] * 0.1, a[0] + b[0] * 0.2 - b[1] * 0.06, a[1] + b[1] * 0.2 - b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + b[0] * 0.2 - b[1] * 0.06, a[1] + b[1] * 0.2 - b[0] * 0.06, a[0] + b[0] * 0.8 - b[1] * 0.06, a[1] + b[1] * 0.8 - b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_flush();
}


void la_do_edge_split(double *vertex_a, double *vertex_b, double pos)
{
	float a[2], b[2];
	a[0] = vertex_a[0];
	a[1] = vertex_a[1];
	b[0] = vertex_b[0] - vertex_a[0];
	b[1] = vertex_b[1] - vertex_a[1];
	r_primitive_line_2d(a[0] + (b[0] * -0.03) - b[1] * 0.06, a[1] + (b[1] * -0.03) + b[0] * 0.06, a[0] + (b[0] * -0.03) - b[1] * 0.02, a[1] + (b[1] * -0.03) + b[0] * 0.02, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * -0.03) - b[1] * 0.02, a[1] + (b[1] * -0.03) + b[0] * 0.02, a[0] + (b[0] * (pos - 0.01)) - b[1] * 0.02, a[1] + (b[1] * (pos - 0.01)) + b[0] * 0.02, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * (pos - 0.01)) - b[1] * 0.02, a[1] + (b[1] * (pos - 0.01)) + b[0] * 0.02, a[0] + (b[0] * (pos - 0.03)) - b[1] * 0.06, a[1] + (b[1] * (pos - 0.03)) + b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * (pos - 0.03)) - b[1] * 0.06, a[1] + (b[1] * (pos - 0.03)) + b[0] * 0.06, a[0] + (b[0] * -0.03) - b[1] * 0.06, a[1] + (b[1] * -0.03) + b[0] * 0.06, 1, 0.1, 0.4, 1.0); 

	r_primitive_line_2d(a[0] + (b[0] * -0.03) + b[1] * 0.06, a[1] + (b[1] * -0.03) - b[0] * 0.06, a[0] + (b[0] * -0.03) + b[1] * 0.02, a[1] + (b[1] * -0.03) - b[0] * 0.02, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * -0.03) + b[1] * 0.02, a[1] + (b[1] * -0.03) - b[0] * 0.02, a[0] + (b[0] * (pos - 0.01)) + b[1] * 0.02, a[1] + (b[1] * (pos - 0.01)) - b[0] * 0.02, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * (pos - 0.01)) + b[1] * 0.02, a[1] + (b[1] * (pos - 0.01)) - b[0] * 0.02, a[0] + (b[0] * (pos - 0.03)) + b[1] * 0.06, a[1] + (b[1] * (pos - 0.03)) - b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * (pos - 0.03)) + b[1] * 0.06, a[1] + (b[1] * (pos - 0.03)) - b[0] * 0.06,a[0] + (b[0] * -0.03) + b[1] * 0.06, a[1] + (b[1] * -0.03) - b[0] * 0.06, 1, 0.1, 0.4, 1.0);

	r_primitive_line_2d(a[0] + (b[0] * 1.03) - b[1] * 0.06, a[1] + (b[1] * 1.03) + b[0] * 0.06, a[0] + (b[0] * 1.03) - b[1] * 0.02, a[1] + (b[1] * 1.03) + b[0] * 0.02, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * 1.03) - b[1] * 0.02, a[1] + (b[1] * 1.03) + b[0] * 0.02, a[0] + (b[0] * (pos + 0.01)) - b[1] * 0.02, a[1] + (b[1] * (pos + 0.01)) + b[0] * 0.02, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * (pos + 0.01)) - b[1] * 0.02, a[1] + (b[1] * (pos + 0.01)) + b[0] * 0.02, a[0] + (b[0] * (pos + 0.03)) - b[1] * 0.06, a[1] + (b[1] * (pos + 0.03)) + b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * (pos + 0.03)) - b[1] * 0.06, a[1] + (b[1] * (pos + 0.03)) + b[0] * 0.06, a[0] + (b[0] * 1.03) - b[1] * 0.06, a[1] + (b[1] * 1.03) + b[0] * 0.06, 1, 0.1, 0.4, 1.0);

	r_primitive_line_2d(a[0] + (b[0] * 1.03) + b[1] * 0.06, a[1] + (b[1] * 1.03) - b[0] * 0.06, a[0] + (b[0] * 1.03) + b[1] * 0.02, a[1] + (b[1] * 1.03) - b[0] * 0.02, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * 1.03) + b[1] * 0.02, a[1] + (b[1] * 1.03) - b[0] * 0.02, a[0] + (b[0] * (pos + 0.01)) + b[1] * 0.02, a[1] + (b[1] * (pos + 0.01)) - b[0] * 0.02, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * (pos + 0.01)) + b[1] * 0.02, a[1] + (b[1] * (pos + 0.01)) - b[0] * 0.02, a[0] + (b[0] * (pos + 0.03)) + b[1] * 0.06, a[1] + (b[1] * (pos + 0.03)) - b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] + (b[0] * (pos + 0.03)) + b[1] * 0.06, a[1] + (b[1] * (pos + 0.03)) - b[0] * 0.06, a[0] + (b[0] * 1.03) + b[1] * 0.06, a[1] + (b[1] * 1.03) - b[0] * 0.06, 1, 0.1, 0.4, 1.0);
	r_primitive_line_flush();
}

void la_do_edge_delete(double *a, double *b, double c_x, double c_y, double c_z)
{
	r_primitive_line_2d(a[0] * 0.9 + b[0] * 0.1, 
		a[1] * 0.9 + b[1] * 0.1, 
		a[0] * 0.75 + b[0] * 0.15 + c_x * 0.1, 
		a[1] * 0.75 + b[1] * 0.15 + c_y * 0.1, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(a[0] * 0.75 + b[0] * 0.15 + c_x * 0.1,
		a[1] * 0.75 + b[1] * 0.15 + c_y * 0.1,
		b[0] * 0.75 + a[0] * 0.15 + c_x * 0.1,
		b[1] * 0.75 + a[1] * 0.15 + c_y * 0.1, 1, 0.1, 0.4, 1.0);
	r_primitive_line_2d(b[0] * 0.75 + a[0] * 0.15 + c_x * 0.1, 
		b[1] * 0.75 + a[1] * 0.15 + c_y * 0.1, 
		b[0] * 0.9 + a[0] * 0.1, 
		b[1] * 0.9 + a[1] * 0.1, 1, 0.1, 0.4, 1.0);
}

void la_do_polygon_delete_clear()
{
	GlobalOverlay.delete_execute_timer = 0;
	GlobalOverlay.delete_array_length = 0;
}

void la_do_polygon_delete_execute()
{
	GlobalOverlay.delete_execute_timer = 0.01;
}

void la_do_polygon_delete(uint polygon)
{
	double *vertex, *v[4];
	uint *ref,  length, i, j, k, temp[4], tri_count, quad_count, line_count, *lines, vertex_count;
	float *array, center[3], n[3], a[3], b[3], f;
	udg_get_geometry(&length, NULL, &vertex, &ref, NULL);
	polygon *= 4;
	if(GlobalOverlay.delete_array_length + 1 >= GlobalOverlay.delete_array_allocation)
	{
		GlobalOverlay.delete_array_allocation += 64;
		GlobalOverlay.delete_array = realloc(GlobalOverlay.delete_array, (sizeof *GlobalOverlay.delete_array) * GlobalOverlay.delete_array_allocation * 6 * 3);
	}
	array = &GlobalOverlay.delete_array[GlobalOverlay.delete_array_length * 6 * 3];
	if(ref[polygon + 3] < length)
	{
		v[0] = &vertex[ref[polygon] * 3];
		v[1] = &vertex[ref[polygon + 1] * 3];
		v[2] = &vertex[ref[polygon + 2] * 3];
		v[3] = &vertex[ref[polygon + 3] * 3];
		a[0] = v[0][0] - v[2][0]; 
		a[1] = v[0][1] - v[2][1]; 
		a[2] = v[0][2] - v[2][2]; 
		b[0] = v[1][0] - v[2][0]; 
		b[1] = v[1][1] - v[2][1]; 
		b[2] = v[1][2] - v[2][2]; 
		f_cross3f(n, a, b);
		f_normalize3f(n);
		f = seduce_view_distance_camera_get(NULL) * -0.4;
	/*	center[0] = (v[0][0] + v[1][0] + v[2][0] + v[3][0]) / 4.0 + n[0] * f;
		center[1] = (v[0][1] + v[1][1] + v[2][1] + v[3][1]) / 4.0 + n[1] * f;
		center[2] = (v[0][2] + v[1][2] + v[2][2] + v[3][2]) / 4.0 + n[2] * f;*/
		 n[0] *= f;
		 n[1] *= f;
		 n[2] *= f;
		*array++ = v[0][0];
		*array++ = v[0][1];
		*array++ = v[0][2];
		*array++ = v[0][0] + n[0];
		*array++ = v[0][1] + n[1];
		*array++ = v[0][2] + n[2];
		*array++ = v[1][0];
		*array++ = v[1][1];
		*array++ = v[1][2];
		*array++ = v[1][0] + n[0];
		*array++ = v[1][1] + n[1];
		*array++ = v[1][2] + n[2];
		*array++ = v[2][0];
		*array++ = v[2][1];
		*array++ = v[2][2];
		*array++ = v[2][0] + n[0];
		*array++ = v[2][1] + n[1];
		*array++ = v[2][2] + n[2];

		*array++ = v[0][0];
		*array++ = v[0][1];
		*array++ = v[0][2];
		*array++ = v[0][0] + n[0];
		*array++ = v[0][1] + n[1];
		*array++ = v[0][2] + n[2];
		*array++ = v[2][0];
		*array++ = v[2][1];
		*array++ = v[2][2];
		*array++ = v[2][0] + n[0];
		*array++ = v[2][1] + n[1];
		*array++ = v[2][2] + n[2];
		*array++ = v[3][0];
		*array++ = v[3][1];
		*array++ = v[3][2];
		*array++ = v[3][0] + n[0];
		*array++ = v[3][1] + n[1];
		*array++ = v[3][2] + n[2];
		GlobalOverlay.delete_array_length += 2;
	}else
	{
		v[0] = &vertex[ref[polygon] * 3];
		v[1] = &vertex[ref[polygon + 1] * 3];
		v[2] = &vertex[ref[polygon + 2] * 3];
		a[0] = v[0][0] - v[2][0]; 
		a[1] = v[0][1] - v[2][1]; 
		a[2] = v[0][2] - v[2][2]; 
		b[0] = v[1][0] - v[2][0]; 
		b[1] = v[1][1] - v[2][1]; 
		b[2] = v[1][2] - v[2][2]; 
		f_cross3f(n, a, b);
		f_normalize3f(n);
		f = seduce_view_distance_camera_get(NULL) * 0.4;
		center[0] = (v[0][0] + v[1][0] + v[2][0]) / 3.0 + n[0] * f;
		center[1] = (v[0][1] + v[1][1] + v[2][1]) / 3.0 + n[1] * f;
		center[2] = (v[0][2] + v[1][2] + v[2][2]) / 3.0 + n[2] * f;
		*array++ = v[0][0];
		*array++ = v[0][1];
		*array++ = v[0][2];
		*array++ = center[0];
		*array++ = center[1];
		*array++ = center[2];
		*array++ = v[1][0];
		*array++ = v[1][1];
		*array++ = v[1][2];
		*array++ = center[0];
		*array++ = center[1];
		*array++ = center[2];
		*array++ = v[2][0];
		*array++ = v[2][1];
		*array++ = v[2][2];
		*array++ = center[0];
		*array++ = center[1];
		*array++ = center[2];
		GlobalOverlay.delete_array_length++;
	}
}




void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float size, uint id, float fade, float *color);

void la_do_active_vertex(BInputState *input, double *vertex, boolean move)
{
	float color[4] = {0.2, 0.6, 1.0, 1.0};
	double pos[3];
	seduce_view_projection_screend(NULL, pos, vertex[0], vertex[1], vertex[2]);
	if(move)
		seduce_object_3d_draw(input, pos[0], pos[1], 0, 0.016, SEDUCE_OBJECT_HANDLE_MOVE_3D, 1, color);
	else
		seduce_object_3d_draw(input, pos[0], pos[1], 0, 0.016, SEDUCE_OBJECT_HANDLE_MOVE_3D, 1, color);
}

void la_do_xyz_lines(double *start, boolean snap)
{
	float xyz_lines[24 * 3], xyz_color[24 * 3], m[16];
	double camera[3], r, r2, x, y, z, scale[3] = {0, 0, 0}, start_transformed[3];
	uint i, axis;

	for(i = 0; i < 16; i++)
		m[i] = la_axis_matrix[i];
	r_matrix_push(&la_world_matrix);
	r_matrix_matrix_mult(&la_world_matrix, m);

	f_transforminv3d(start_transformed, la_axis_matrix, start[0], start[1], start[2]);
	seduce_view_camera_getd(NULL, camera);
	f_transforminv3d(camera, la_axis_matrix, camera[0], camera[1], camera[2]);
	camera[0] -= start_transformed[0];
	camera[1] -= start_transformed[1];
	camera[2] -= start_transformed[2];
	r = sqrt(camera[0] * camera[0] + camera[1] * camera[1] + camera[2] * camera[2]);
	r2 = sqrt(camera[1] * camera[1] + camera[2] * camera[2]);
	r *= 0.004;
	x = r * 20;
	y = camera[2] / r2 * r;
	z = camera[1] / r2 * r;
/*
	r_primitive_line_3d(start_transformed[0] + x, start_transformed[1] + y, start_transformed[2] - z, start_transformed[0] + x * 40, start_transformed[1] + y, start_transformed[2] - z, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] + x * 40, start_transformed[1] - y, start_transformed[2] + z, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] + z, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] - x, start_transformed[1] + y, start_transformed[2] - z, start_transformed[0] - x * 40, start_transformed[1] + y, start_transformed[2] - z, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] - x * 40, start_transformed[1] - y, start_transformed[2] + z, start_transformed[0] - x, start_transformed[1] - y, start_transformed[2] + z, 0.05, 0.15, 0.25, 1.0);
*/

	seduce_primitive_line_add_3d(NULL, start_transformed[0] + x, start_transformed[1] + y, start_transformed[2] - z,
							start_transformed[0] + x * 40, start_transformed[1] + y, start_transformed[2] - z,
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] + x * 40, start_transformed[1] - y, start_transformed[2] + z,
							start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] + z,
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] - x, start_transformed[1] + y, start_transformed[2] - z,
							start_transformed[0] - x * 40, start_transformed[1] + y, start_transformed[2] - z,
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] - x * 40, start_transformed[1] - y, start_transformed[2] + z,
							start_transformed[0] - x, start_transformed[1] - y, start_transformed[2] + z,
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);

	r2 = sqrt(camera[0] * camera[0] + camera[2] * camera[2]);
	x = camera[2] / r2 * r;
	y = r * 20;
	z = camera[0] / r2 * r;
/*
	r_primitive_line_3d(start_transformed[0] - x, start_transformed[1] + y, start_transformed[2] + z, start_transformed[0] - x, start_transformed[1] + y * 40, start_transformed[2] + z, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] + x, start_transformed[1] + y * 40, start_transformed[2] - z, start_transformed[0] + x, start_transformed[1] + y, start_transformed[2] - z, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] - x, start_transformed[1] - y, start_transformed[2] + z, start_transformed[0] - x, start_transformed[1] - y * 40, start_transformed[2] + z, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] + x, start_transformed[1] - y * 40, start_transformed[2] - z, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] - z, 0.05, 0.15, 0.25, 1.0);
*/

	seduce_primitive_line_add_3d(NULL, start_transformed[0] - x, start_transformed[1] + y, start_transformed[2] + z, start_transformed[0] - x, start_transformed[1] + y * 40, start_transformed[2] + z, 
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] + x, start_transformed[1] + y * 40, start_transformed[2] - z, start_transformed[0] + x, start_transformed[1] + y, start_transformed[2] - z, 
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] - x, start_transformed[1] - y, start_transformed[2] + z, start_transformed[0] - x, start_transformed[1] - y * 40, start_transformed[2] + z, 
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] + x, start_transformed[1] - y * 40, start_transformed[2] - z, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] - z, 
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);

	r2 = sqrt(camera[1] * camera[1] + camera[0] * camera[0]);
	x = camera[1] / r2 * r;
	y = camera[0] / r2 * r;
	z = r * 20;
/*	r_primitive_line_3d(start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] + z, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] + z * 40, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] - x, start_transformed[1] + y, start_transformed[2] + z * 40, start_transformed[0] + -x, start_transformed[1] + y, start_transformed[2] + z, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] - z, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] - z * 40, 0.05, 0.15, 0.25, 1.0);
	r_primitive_line_3d(start_transformed[0] - x, start_transformed[1] + y, start_transformed[2] - z * 40, start_transformed[0] + -x, start_transformed[1] + y, start_transformed[2] - z, 0.05, 0.15, 0.25, 1.0);
*/	r_primitive_line_flush();

	seduce_primitive_line_add_3d(NULL, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] + z, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] + z * 40,
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] - x, start_transformed[1] + y, start_transformed[2] + z * 40, start_transformed[0] + -x, start_transformed[1] + y, start_transformed[2] + z,
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] - z, start_transformed[0] + x, start_transformed[1] - y, start_transformed[2] - z * 40, 
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);
	seduce_primitive_line_add_3d(NULL, start_transformed[0] - x, start_transformed[1] + y, start_transformed[2] - z * 40, start_transformed[0] + -x, start_transformed[1] + y, start_transformed[2] - z, 
							0.1, 0.3, 0.5, 1.0,
							0.1, 0.3, 0.5, 1.0);

	seduce_primitive_line_animation_set(0, 0, 1, 1, 1);
	seduce_primitive_line_draw(NULL, 1.0, 1.0, 1.0, 1.0);
	r_matrix_pop(&la_world_matrix);
}

void la_do_draw(double *start, double *end, boolean snap, double *closest)
{
	static uint counter = 0;
	double camera[3], r, r2, x, y, z;
	float a[3], b[3];
	RMatrix *reset;
	reset = r_matrix_get();
	r_matrix_set(&la_world_matrix); // la_interface_matrix;
	r_primitive_line_3d((float)start[0], (float)start[1], (float)start[2],
						(float)end[0], (float)end[1], (float)end[2], 1, 1, 1, 1.0);
	la_do_xyz_lines(start, snap);
	r_primitive_line_flush();
	r_matrix_set(reset);
}

void la_do_draw_closest_edge(uint *edge, double x, double y, boolean snap)
{
	egreal *vertex, a[3], b[3];
	if(edge[0] != -1 && edge[1] != -1)
	{
		udg_get_geometry(NULL, NULL, &vertex, NULL, NULL);
		glDisable(GL_DEPTH_TEST);
		seduce_view_projection_screend(NULL, a, vertex[edge[0] * 3], vertex[edge[0] * 3 + 1], vertex[edge[0] * 3 + 2]);
		seduce_view_projection_screend(NULL, b, vertex[edge[1] * 3], vertex[edge[1] * 3 + 1], vertex[edge[1] * 3 + 2]);
		if(snap)
		{
			la_do_edge_select(a, b);
		}else
		{
			if(udg_get_select(edge[0]) + udg_get_select(edge[1]) > 0.99)
				la_do_edge_select(a, b);
			else
			{
				double r;
				r = sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]));
				r = (((a[0] - b[0]) / r) * (a[0] - x) + ((a[1] - b[1]) / r) * (a[1] - y)) / r;
				la_do_edge_split(a, b, r);
			}
		}
		glEnable(GL_DEPTH_TEST);
	}
}

void la_do_active_polygon(double *snap)
{
	static double t = 0;
	double pos[3], matrix[16], vec[3], rand[3] = {2.342, -1.23, 2.6489};
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	vec[0] = snap[0] + snap[3];
	vec[1] = snap[1] + snap[4];
	vec[2] = snap[2] + snap[5];
	f_matrixzxd(matrix, snap, vec, rand);
	glMultMatrixd(matrix);
	seduce_view_projection_screend(NULL, vec, snap[0], snap[1], snap[2]);
	t += betray_time_delta_get() * 60;
	glRotated(t, 0, 0, 1);
	glScaled(vec[2], vec[2], vec[2]);
//	r_gl(GL_QUADS, GlobalOverlay.active_vertex, 16, 2, 0.8, 0.8, 0.8, 0.0);
//	r_gl(GL_QUADS, GlobalOverlay.active_vertex_shadow, 16 * 4, 2, 0.8, 0.8, 0.8, 0.0);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

void la_do_draw_snap_edge(uint *edge)
{
	double a[3], b[3], vertex_a[3], vertex_b[3];
	float draw[16];
	if(edge[0] != -1 && edge[1] != -1)
	{
		udg_get_vertex_pos(a, edge[0]);
		udg_get_vertex_pos(b, edge[1]);
		seduce_view_projection_screend(NULL, vertex_a, a[0], a[1], a[2]);
		seduce_view_projection_screend(NULL, vertex_b, b[0], b[1], b[2]);
		a[0] = -vertex_a[0];
		a[1] = -vertex_a[1];
		b[0] = vertex_a[0] - vertex_b[0];
		b[1] = vertex_a[1] - vertex_b[1];
		la_draw_set_vec2(draw, 0, a[0] + b[1] * 0.04, a[1] - b[0] * 0.04);
		la_draw_set_vec2(draw, 1, a[0] - b[1] * 0.04, a[1] + b[0] * 0.04);
		la_draw_set_vec2(draw, 2, a[0] + (b[0] * 0.4) - b[1] * 0.06, a[1] + (b[1] * 0.4) + b[0] * 0.06);
		la_draw_set_vec2(draw, 3, a[0] + (b[0] * 0.4) + b[1] * 0.06, a[1] + (b[1] * 0.4) - b[0] * 0.06);
		la_draw_set_vec2(draw, 4, a[0] + (b[0] * 0.6) + b[1] * 0.06, a[1] + (b[1] * 0.6) - b[0] * 0.06);
		la_draw_set_vec2(draw, 5, a[0] + (b[0] * 0.6) - b[1] * 0.06, a[1] + (b[1] * 0.6) + b[0] * 0.06);
		la_draw_set_vec2(draw, 6, a[0] + b[0] - b[1] * 0.04, a[1] + b[1] + b[0] * 0.04);
		la_draw_set_vec2(draw, 7, a[0] + b[0] + b[1] * 0.04, a[1] + b[1] - b[0] * 0.04);
		glTranslated(0, 0, -1);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_TEST);
	}
}

boolean p_persuade_force_update = FALSE;

void la_draw_force_update_persuade(void)
{
	p_persuade_force_update = TRUE;
}

/*
void draw_persuade_surface(ENode *node)
{
#ifdef PERSUADE_H
	static ENode *old_node = NULL;
	static PMesh *mesh = NULL, *next = NULL;
	static uint version;
	egreal camera[3];
	double cam[3], *vertex;
	seduce_view_camera_getd(NULL, cam);
	p_lod_set_view_pos(cam);
	camera[0] = cam[0];
	camera[1] = cam[1];
	camera[2] = cam[2];
	if(old_node != NULL && old_node != node && mesh != NULL)
	{
		p_rm_destroy(mesh);
		mesh = NULL;
	}
	old_node = node;
	if(node == NULL)
		return;
	if(mesh == NULL)
		mesh = p_rm_create(node);


//	if(mesh != NULL)
//		mesh = p_rm_service(mesh, NULL, e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node,  0)));
	udg_get_geometry(NULL, NULL, &vertex, NULL, NULL);
	if(mesh != NULL)
	{
		p_rm_set_eay(mesh, camera);
		mesh = p_rm_service(mesh, NULL, vertex);
	}
	if(mesh != NULL && p_rm_drawable(mesh))
	{
		glPushMatrix();

		if(p_persuade_force_update)
			p_rm_unvalidate(mesh);

		glPolygonMode(GL_FRONT, GL_LINE);
		if(version != udg_get_version(FALSE, TRUE, FALSE, FALSE, FALSE))
		{
			udg_get_geometry(NULL, NULL, &vertex, NULL, NULL);
			p_rm_update_shape(mesh, vertex);
			version = udg_get_version(FALSE, TRUE, FALSE, FALSE, FALSE);
		}
//		p_rm_service(mesh, NULL, e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node,  0)));
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_LIGHTING);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glBindTexture(GL_TEXTURE_2D, la_pfx_surface_material());
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_NORMALIZE);
//		glDisable(GL_LIGHTING);
//		glDisable(GL_TEXTURE_2D);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


#ifdef E_GEOMETRY_REAL_PRECISION_64_BIT
		glVertexPointer(3, GL_DOUBLE, 0, p_rm_get_vertex(mesh));
		glNormalPointer(GL_DOUBLE, 0 , p_rm_get_normal(mesh));
//		glNormalPointer(GL_DOUBLE, 0 , p_rm_get_vertex(mesh));
#endif
#ifdef E_GEOMETRY_REAL_PRECISION_32_BIT
		glVertexPointer(3, GL_FLOAT, 0, p_rm_get_vertex(mesh));
		glNormalPointer(GL_FLOAT, 0 , p_rm_get_normal(mesh));
#endif
//		glEnable(GL_LIGHTING);
		glColor4f(1, 1, 1, 1);

		glDrawElements(GL_TRIANGLES, p_rm_get_ref_length(mesh), GL_UNSIGNED_INT, p_rm_get_reference(mesh));
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_NORMALIZE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glPopMatrix();
		p_persuade_force_update = FALSE;
	}
#endif
}*/

void draw_owerlay_tags(void)
{
	double pos[3], f, color;
	UNDOTag	*tag;
	uint count, i;
	tag = udg_get_tags(&count);
	for(i = 0; i < count; i++)
	{
		seduce_view_projection_screend(NULL, pos, tag[i].vec[0], tag[i].vec[1], tag[i].vec[2]);
		glDisable(GL_DEPTH_TEST);
		glTranslated(-pos[0], -pos[1], -1);
	//	glRotated(t++, 0, 0, 1);
		if(tag[i].select < 0.01)
		{
//			sui_set_blend_gl(GL_ADD, GL_ADD);
//			r_gl(GL_QUADS, GlobalOverlay.tag_unselect, 24, 2, 0.8, 0.8, 0.8, 0.0);
		/*	sui_set_blend_gl(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			sui_set_color_array_gl(GlobalOverlay.tag_unselect_color, 24 * 4, 4);
			r_gl(GL_QUADS, GlobalOverlay.tag_unselect_shadow, 24 * 4, 2, 0.8, 0.8, 0.8);*/
		}else
		{
	//		sui_set_blend_gl(GL_ADD, GL_ADD);
	//		r_gl(GL_QUADS, GlobalOverlay.tag_select, 24, 2, 0.8, 0.8, 0.8, 0.0);
		/*	sui_set_blend_gl(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			sui_set_color_array_gl(GlobalOverlay.tag_select_color, 24 * 4, 4);
			r_gl(GL_QUADS, GlobalOverlay.tag_select_shadow, 24 * 4, 2, 0.8, 0.8, 0.8);*/
		}
		color = tag[i].select * 0.5 + 0.25;
	//	r_text(0.03, SEDUCE_T_SIZE * -0.5, SEDUCE_T_SIZE, SEDUCE_T_SPACE, tag[i].group, color, color, color, 1.0);
		seduce_text_line_draw(NULL, 0.03, SEDUCE_T_SIZE * -0.5, SEDUCE_T_SIZE, SEDUCE_T_SPACE, tag[i].group, color, color, color, 1.0, -1);

		f = seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, tag[i].group, -1);
		seduce_text_line_draw(NULL, 0.05 + f, SEDUCE_T_SIZE * -0.5, SEDUCE_T_SIZE, SEDUCE_T_SPACE, tag[i].tag, color, color, color, 1.0, -1);
	//	r_2d_line_gl(0.04 + f, 0.03, 0.04 + f, -0.03, color, color, color, 0.0);
		glEnable(GL_DEPTH_TEST);
	}
}

#define LA_OVERLAY_BASE_LENGTH 0.1

void la_do_mesh_update(LoOverlay *overlay, uint32 length, uint32 ref_length, double *vertex, double *base, uint32 *ref, uint32 *crease, double *select, uint version)
{
	static uint draw;
	uint i, j, k, vertex_count, temp[4], tri_count, quad_count, line_count, *lines;
	float s, c, e, f, *poly = NULL, *line = NULL, *p, *l, *l2;

	if(overlay->surface_version != version)
	{	
		RFormats types[2] = {R_FLOAT, R_FLOAT};
		uint poly_size[2] = {4, 4}, line_size[2] = {3, 4}, vertex_size[2] = {3, 4}, base_size[1] = {4};
		boolean *vertex_deny = NULL;
		tri_count = 0;
		quad_count = 0;
		vertex_count = 0;
		if(overlay->poly_pool != NULL)
			la_do_mesh_free(overlay);
		if(length != 0)
		{
			vertex_deny = malloc((sizeof *vertex_deny) * length);
			for(i = 0; i < length; i++)
				vertex_deny[i] = vertex[i * 3] != V_REAL64_MAX;

			overlay->max[0] = overlay->max[1] = overlay->max[2] = -100000000;
			overlay->min[0] = overlay->min[1] = overlay->min[2] = 100000000;

			for(i = 0; i < ref_length; i++)
			{
				if(ref[i * 4] < length && ref[i * 4 + 1] < length && ref[i * 4 + 2] < length && vertex[ref[i * 4] * 3] != V_REAL64_MAX && vertex[ref[i * 4 + 1] * 3] != V_REAL64_MAX && vertex[ref[i * 4 + 2] * 3] != V_REAL64_MAX)
				{
					vertex_deny[ref[i * 4]] = vertex_deny[ref[i * 4 + 1]] = vertex_deny[ref[i * 4 + 2]] = FALSE;
					if(ref[i * 4 + 3] < length && vertex[ref[i * 4 + 3] * 3] != V_REAL64_MAX)
					{
						vertex_deny[ref[i * 4 + 3]] = FALSE;
						quad_count++;
					}else
						tri_count++;
				}
			}
			for(i = 0; i < length; i++)
				if(vertex_deny[i])
					vertex_count++;



			lines = udg_get_edge_data(&line_count);



			if(vertex_count == 0)		
				overlay->vertex_pool = NULL;
			else
			{
				float uv[6] = {0, 1.7301, 1.7301, -1, -1.7301, -1};
				line = l = malloc((sizeof *line) * vertex_count * 7 * 3);
				overlay->vertex_pool = r_array_allocate(vertex_count * 3, types, vertex_size, 2, 0);
				for(i = 0; i < length; i++)
				{
					if(vertex_deny[i])
					{
						f = f_randf(i) * PI * 2.0; 
						if(select == NULL)
							s = 0;
						else
							s = select[i];
						for(j = 0; j < 6;)
						{
							*l++ = vertex[i * 3];
							*l++ = vertex[i * 3 + 1];
							*l++ = vertex[i * 3 + 2];
							*l++ = uv[j++];
							*l++ = uv[j++];
							*l++ = f;
							*l++ = s;
						}
					}else if(vertex[i * 3] != V_REAL64_MAX)
					{
						if(vertex[i * 3] > overlay->max[0])
							overlay->max[0] = vertex[i * 3];
						if(vertex[i * 3 + 1] > overlay->max[1])
							overlay->max[1] = vertex[i * 3 + 1];
						if(vertex[i * 3 + 2] > overlay->max[2])
							overlay->max[2] = vertex[i * 3 + 2];

						if(vertex[i * 3] < overlay->min[0])
							overlay->min[0] = vertex[i * 3];
						if(vertex[i * 3 + 1] < overlay->min[1])
							overlay->min[1] = vertex[i * 3 + 1];
						if(vertex[i * 3 + 2] < overlay->min[2])
							overlay->min[2] = vertex[i * 3 + 2];
					}
				}
				r_array_load_vertex(overlay->vertex_pool, NULL, line, 0, vertex_count * 3);
				free(line);
			}
			if(vertex_deny != NULL)
				free(vertex_deny);
			if(quad_count + tri_count + line_count == 0)
			{
				overlay->line_pool = NULL;
			}else
			{
				overlay->line_pool = r_array_allocate((quad_count * 8 + tri_count * 6 + line_count * 2), types, line_size, 2, 0);
				line = l = malloc((sizeof *line) * (quad_count * 8 + tri_count * 6 + line_count * 2) * 7);
			}

			if(quad_count + tri_count == 0)
			{
				overlay->poly_pool = NULL;
			}else
			{
				overlay->poly_pool = r_array_allocate((quad_count * 6 + tri_count * 3), types, poly_size, 2, 0);
				poly = p = malloc((sizeof *poly) * (quad_count * 6 + tri_count * 3) * 8);
				for(i = 0; i < ref_length * 4; i += 4)
				{
					if(ref[i] < length && ref[i + 1] < length && ref[i + 2] < length && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
					{
						float a[3], b[3], normal[3];
						temp[0] = ref[i] * 3;
						temp[1] = ref[i + 1] * 3;
						temp[2] = ref[i + 2] * 3;
						temp[3] = ref[i + 3] * 3;
						a[0] = vertex[temp[0] + 0] - vertex[temp[2] + 0];
						a[1] = vertex[temp[0] + 1] - vertex[temp[2] + 1];
						a[2] = vertex[temp[0] + 2] - vertex[temp[2] + 2];
						b[0] = vertex[temp[1] + 0] - vertex[temp[2] + 0];
						b[1] = vertex[temp[1] + 1] - vertex[temp[2] + 1];
						b[2] = vertex[temp[1] + 2] - vertex[temp[2] + 2];
						f_cross3f(normal, a, b);
					
				
						if(temp[3] < length * 3 && vertex[temp[3]] != V_REAL64_MAX)
						{
							e = s = 0;
							if(select != NULL && select[ref[i]] > 0.1 && select[ref[i + 1]] > 0.1 && select[ref[i + 2]] > 0.1 && select[ref[i + 3]] > 0.1)
								s = 1;
							*poly++ = vertex[temp[0] + 0];
							*poly++ = vertex[temp[0] + 1];
							*poly++ = vertex[temp[0] + 2];
							if(select != NULL)
								*poly++ = select[ref[i]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							*poly++ = vertex[temp[1] + 0];
							*poly++ = vertex[temp[1] + 1];
							*poly++ = vertex[temp[1] + 2];	
							if(select != NULL)
								*poly++ = select[ref[i + 1]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							*poly++ = vertex[temp[2] + 0];
							*poly++ = vertex[temp[2] + 1];
							*poly++ = vertex[temp[2] + 2];	
							if(select != NULL)
								*poly++ = select[ref[i + 2]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							*poly++ = vertex[temp[0] + 0];
							*poly++ = vertex[temp[0] + 1];
							*poly++ = vertex[temp[0] + 2];	
							if(select != NULL)
								*poly++ = select[ref[i]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							*poly++ = vertex[temp[2] + 0];
							*poly++ = vertex[temp[2] + 1];
							*poly++ = vertex[temp[2] + 2];		
							if(select != NULL)
								*poly++ = select[ref[i + 2]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							*poly++ = vertex[temp[3] + 0];
							*poly++ = vertex[temp[3] + 1];
							*poly++ = vertex[temp[3] + 2];	
							if(select != NULL)
								*poly++ = select[ref[i + 3]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							s = 1.0 - s;
							if(crease != NULL)
								c = (float)crease[i] / 4294967295.0;
							else
								c = 0;

							*line++ = vertex[temp[0] + 0];
							*line++ = vertex[temp[0] + 1];
							*line++ = vertex[temp[0] + 2];

							if(select != NULL)
								*line++ = select[ref[i]];
							else
								*line++ = 0;
							*line++ = 0;
							*line++ = 1;
							*line++ = c;

							*line++ = vertex[temp[1] + 0];
							*line++ = vertex[temp[1] + 1];
							*line++ = vertex[temp[1] + 2];

							if(select != NULL)
								*line++ = select[ref[i + 1]];
							else
								*line++ = 0;
							*line++ = 1;
							*line++ = 0;
							*line++ = c;

							if(crease != NULL)
								c = (float)crease[i + 1] / 4294967295.0;
							else
								c = 0;
							*line++ = vertex[temp[1] + 0];
							*line++ = vertex[temp[1] + 1];
							*line++ = vertex[temp[1] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 1]];
							else
								*line++ = 0;
							*line++ = 0;
							*line++ = 1;
							*line++ = c;

							*line++ = vertex[temp[2] + 0];
							*line++ = vertex[temp[2] + 1];
							*line++ = vertex[temp[2] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 2]];
							else
								*line++ = 0;
							*line++ = 1;
							*line++ = 0;
							*line++ = c;

							if(crease != NULL)
								c = (float)crease[i + 2] / 4294967295.0;
							else
								c = 0;
							*line++ = vertex[temp[2] + 0];
							*line++ = vertex[temp[2] + 1];
							*line++ = vertex[temp[2] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 2]];
							else
								*line++ = 0;
							*line++ = 0;
							*line++ = 1;
							*line++ = c;

							*line++ = vertex[temp[3] + 0];
							*line++ = vertex[temp[3] + 1];
							*line++ = vertex[temp[3] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 3]];
							else
								*line++ = 0;
							*line++ = 1;
							*line++ = 0;
							*line++ = c;
							if(crease != NULL)
								c = (float)crease[i + 3] / 4294967295.0;
							else
								c = 0;
							*line++ = vertex[temp[3] + 0];
							*line++ = vertex[temp[3] + 1];
							*line++ = vertex[temp[3] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 3]];
							else
								*line++ = 0;
							*line++ = 0;
							*line++ = 1;
							*line++ = c;

							*line++ = vertex[temp[0] + 0];
							*line++ = vertex[temp[0] + 1];
							*line++ = vertex[temp[0] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 0]];
							else
								*line++ = 0;
							*line++ = 1;
							*line++ = 0;
							*line++ = c;
						}else
						{
							e = s = 0;
							if(select != NULL && select[ref[i]] > 0.1 && select[ref[i + 1]] > 0.1 && select[ref[i + 2]] > 0.1)
								s = 1;
							*poly++ = vertex[temp[0] + 0];
							*poly++ = vertex[temp[0] + 1];
							*poly++ = vertex[temp[0] + 2];		
							if(select != NULL)
								*poly++ = select[ref[i]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							*poly++ = vertex[temp[1] + 0];
							*poly++ = vertex[temp[1] + 1];
							*poly++ = vertex[temp[1] + 2];
							if(select != NULL)
								*poly++ = select[ref[i + 1]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							*poly++ = vertex[temp[2] + 0];
							*poly++ = vertex[temp[2] + 1];
							*poly++ = vertex[temp[2] + 2];	
							if(select != NULL)
								*poly++ = select[ref[i + 2]];
							else
								*poly++ = 0;
							*poly++ = normal[0];
							*poly++ = normal[1];
							*poly++ = normal[2];
							*poly++ = s;
							s = 1.0 - s;
							if(crease != NULL)
								c = (float)crease[i] / 4294967295.0;
							else
								c = 0;
							*line++ = vertex[temp[0] + 0];
							*line++ = vertex[temp[0] + 1];
							*line++ = vertex[temp[0] + 2];
							if(select != NULL)
								*line++ = select[ref[i]];
							else
								*line++ = 0;
							*line++ = 0;
							*line++ = 1;
							*line++ = c;
							*line++ = vertex[temp[1] + 0];
							*line++ = vertex[temp[1] + 1];
							*line++ = vertex[temp[1] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 1]];
							else
								*line++ = 0;
							*line++ = 1;
							*line++ = 0;
							*line++ = c;
							if(crease != NULL)
								c = (float)crease[i + 1] / 4294967295.0;
							else
								c = 0;
							*line++ = vertex[temp[1] + 0];
							*line++ = vertex[temp[1] + 1];
							*line++ = vertex[temp[1] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 1]];
							else
								*line++ = 0;
							*line++ = 0;
							*line++ = 1;
							*line++ = c;
							*line++ = vertex[temp[2] + 0];
							*line++ = vertex[temp[2] + 1];
							*line++ = vertex[temp[2] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 2]];
							else
								*line++ = 0;
							*line++ = 1;
							*line++ = 0;
							*line++ = c;
							if(crease != NULL)
								c = (float)crease[i + 2] / 4294967295.0;
							else
								c = 0;
							if(select != NULL)
								f = select[ref[i + 2]] * select[ref[i]] * s;
							else
								f = 0;
							*line++ = vertex[temp[2] + 0];
							*line++ = vertex[temp[2] + 1];
							*line++ = vertex[temp[2] + 2];
							if(select != NULL)
								*line++ = select[ref[i + 2]];
							else
								*line++ = 0;
							*line++ = 0;
							*line++ = 1;
							*line++ = c;
							*line++ = vertex[temp[0] + 0];
							*line++ = vertex[temp[0] + 1];
							*line++ = vertex[temp[0] + 2];
							if(select != NULL)
								*line++ = select[ref[i]];
							else
								*line++ = 0;
							*line++ = 1;
							*line++ = 0;
							*line++ = c;
						}
					}
				}
				r_array_load_vertex(overlay->poly_pool, NULL, p, 0, -1);
				free(p);
			}
			if(quad_count + tri_count + line_count != 0)
			{
				for(i = 0; i < line_count * 2; i += 2)
				{				
					*line++ = vertex[lines[i] * 3 + 0] * 0.97 + vertex[lines[i + 1] * 3 + 0] * 0.03;
					*line++ = vertex[lines[i] * 3 + 1] * 0.97 + vertex[lines[i + 1] * 3 + 1] * 0.03;
					*line++ = vertex[lines[i] * 3 + 2] * 0.97 + vertex[lines[i + 1] * 3 + 2] * 0.03;
					*line++ = udg_get_select(lines[i]);
					*line++ = 0;
					*line++ = 1;
					*line++ = 1;

					*line++ = vertex[lines[i] * 3 + 0] * 0.03 + vertex[lines[i + 1] * 3 + 0] * 0.97;
					*line++ = vertex[lines[i] * 3 + 1] * 0.03 + vertex[lines[i + 1] * 3 + 1] * 0.97;
					*line++ = vertex[lines[i] * 3 + 2] * 0.03 + vertex[lines[i + 1] * 3 + 2] * 0.97;
					*line++ = udg_get_select(lines[i + 1]);
					*line++ = 1;
					*line++ = 0;
					*line++ = 1;
				}
				r_array_load_vertex(overlay->line_pool, NULL, l, 0, -1);
				free(l);
			}

			if(base != NULL)
			{
				float vec[3], f, best = 0;
				uint draw_length = 0;
				line = l = malloc((sizeof *line) * (length * 2) * 4);
				for(i = 0; i < length * 3; i += 3)
				{
					line = l2 = &l[draw_length * 4];
					*line++ = base[i + 0];
					*line++ = base[i + 1];
					*line++ = base[i + 2];
					*line++ = 0;
					*line++ = vertex[i + 0];
					*line++ = vertex[i + 1];
					*line++ = vertex[i + 2];
					vec[0] = l2[0] - l2[4];
					vec[1] = l2[1] - l2[5];
					vec[2] = l2[2] - l2[6];
					f = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];
					if(f != 0.0)
					{
						f = sqrt(f);
						*line = f;
						if(f > best)
							best = f;
						draw_length += 2;
					}
				}
				overlay->base_length = best;
				overlay->base_pool = r_array_allocate(draw_length, types, base_size, 1, 0);
				r_array_load_vertex(overlay->base_pool, NULL, l, 0, draw_length);
			}
		}
		overlay->surface_version = version;
	}
}

void la_do_mesh_surface(LoOverlay *overlay)
{
	if(overlay->poly_pool != NULL)
	{
		r_shader_set(GlobalOverlay.poly_shader);
		r_shader_float_set(GlobalOverlay.poly_shader, r_shader_uniform_location(GlobalOverlay.poly_shader, "distance"), seduce_view_distance_camera_get(NULL));
		r_shader_mat4v_set(GlobalOverlay.poly_shader, r_shader_uniform_location(GlobalOverlay.poly_shader, "transform"), la_t_tm_matrix_get());
		r_array_section_draw(overlay->poly_pool, NULL, GL_TRIANGLES, 0, -1);
	}

	if(overlay->line_pool != NULL)
	{
		r_shader_set(GlobalOverlay.line_shader);
		r_shader_float_set(GlobalOverlay.line_shader, r_shader_uniform_location(GlobalOverlay.line_shader, "distance"), seduce_view_distance_camera_get(NULL));
		r_shader_mat4v_set(GlobalOverlay.line_shader, r_shader_uniform_location(GlobalOverlay.line_shader, "transform"), la_t_tm_matrix_get());
		r_array_section_draw(overlay->line_pool, NULL, GL_LINES, 0, -1);
	}
	if(overlay->vertex_pool != NULL)
	{
		float aspect, pos[3];
		BInputState *input;
		input = betray_get_input_state();
		r_shader_set(GlobalOverlay.vertex_shader);		
		aspect = betray_screen_mode_get(NULL, NULL, NULL);
		seduce_view_projectionf(GlobalOverlay.vertex_shader, pos, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
		r_shader_vec3_set(GlobalOverlay.vertex_shader, r_shader_uniform_location(GlobalOverlay.vertex_shader, "light"), pos[0], pos[1], pos[2]);
		r_shader_vec2_set(GlobalOverlay.vertex_shader, r_shader_uniform_location(GlobalOverlay.vertex_shader, "size"), 0.0015, 0.0015);
		r_shader_vec2_set(GlobalOverlay.vertex_shader, r_shader_uniform_location(GlobalOverlay.vertex_shader, "variance"), 0.0003, 0.0003);
		r_shader_float_set(GlobalOverlay.vertex_shader, r_shader_uniform_location(GlobalOverlay.vertex_shader, "time"), input->minute_time * 20.0 * PI);
		r_shader_float_set(GlobalOverlay.vertex_shader, r_shader_uniform_location(GlobalOverlay.vertex_shader, "distance"), seduce_view_distance_camera_get(NULL));
		r_array_section_draw(overlay->vertex_pool, NULL, GL_TRIANGLES, 0, -1);
	}
/*
	if(overlay->base_pool != NULL)
	{
		r_shader_set(GlobalOverlay.base_shader);
		r_shader_float_set(GlobalOverlay.base_shader,r_shader_uniform_location(GlobalOverlay.base_shader, "maxlengt"), 1.0 / overlay->base_length);
		r_array_section_draw(overlay->base_pool, NULL, GL_LINES, 0, -1);
	}*/
}

void la_do_surface_delete()
{
	if(GlobalOverlay.delete_array_length != 0)
	{
		RFormats types[2] = {R_FLOAT, R_FLOAT};
		uint size[2] = {3, 3};
		void *pool;
		float f;
		pool = r_array_allocate(GlobalOverlay.delete_array_length * 3, types, size, 2, 0);
		r_array_load_vertex(pool, NULL, GlobalOverlay.delete_array, 0, GlobalOverlay.delete_array_length * 3);
		r_shader_set(GlobalOverlay.delete_shader);	
		f = 1.0 - GlobalOverlay.delete_execute_timer;
		f *= f;
		if(GlobalOverlay.delete_execute_timer < 0.01)
			r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.delete_shader, "color"), 0.25, 0, 0.05, 0);
		else
			r_shader_vec4_set(NULL, r_shader_uniform_location(GlobalOverlay.delete_shader, "color"), 2.5 * f, 0.1 * f, 0.5 * f, 0);
		r_shader_float_set(GlobalOverlay.delete_shader, r_shader_uniform_location(GlobalOverlay.delete_shader, "animate"), GlobalOverlay.delete_execute_timer * GlobalOverlay.delete_execute_timer);
		r_array_section_draw(pool, NULL, GL_TRIANGLES, 0, -1);
		r_array_free(pool);

		GlobalOverlay.delete_execute_timer += 0.005;
		if(GlobalOverlay.delete_execute_timer > 1.0)
		{
			GlobalOverlay.delete_execute_timer = 0;
			GlobalOverlay.delete_array_length = 0;
		}
	}
	glEnable(GL_DEPTH_TEST);
}

void draw_owerlay_vertex(void)
{
	static float *base_lines = NULL, *vertex_array = NULL, *vertex_color = NULL, *vertex_reference_array = NULL;
	static uint vertex_version = 0, vertex_count = 0, vertex_reference_version = 0, vertex_reference_count = 0;
	static boolean no_select = TRUE;
	egreal *vertex, *base;
	egreal select, manip_pos[3] ={0, 0, 0}, sum = 0;
	uint length, count, i, j;
	boolean manip = TRUE;
	UNDOTag	*tag;
	tag = udg_get_tags(&count);
	udg_get_geometry(&length, NULL, &vertex, NULL, NULL);
	if(vertex_version != udg_get_version(TRUE, TRUE, TRUE, FALSE, TRUE))
	{
		j = 0;
		for(i = 0; i < length; i++)
			if(vertex[i * 3] != V_REAL64_MAX)
				j++;

		if(vertex_array != NULL)
		{
			free(vertex_array);
			free(vertex_color);
		}
		if(base_lines != NULL)
			free(base_lines);

		for(i = 0; i < count && tag[i].select < 0.001; i++);	
		base_lines = NULL;
		if(j == 0 || i == count)
		{
			vertex_array = NULL;
			vertex_color = NULL;
			no_select = TRUE;
		}else
		{
			vertex_count = j;
			vertex_array = malloc((sizeof *base_lines) * vertex_count * 3);
			vertex_color = malloc((sizeof *base_lines) * vertex_count * 3);
			if((base = udg_get_base_layer()) != NULL)
				base_lines = malloc((sizeof *base_lines) * vertex_count * 2 * 3);
			j = 0;
			for(i = 0; i < length; i++)
			{
				if(vertex[i * 3] != E_REAL_MAX)
				{
					la_draw_set_vec3(vertex_array, j, vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);
					select = udg_get_select(i);
					la_draw_set_vec3(vertex_color, j, 0.2 + select * 0.8, 0.6 + select * 0.4, 1);
					j++;
					if(select > 0.0001)
					{
						manip = FALSE;
						sum += select;
						manip_pos[0] += vertex[i * 3] * select;
						manip_pos[1] += vertex[i * 3 + 1] * select;
						manip_pos[2] += vertex[i * 3 + 2] * select;
					}
				}
			}
			for(i = 0; i < count; i++)
			{
				if(tag[i].select > 0.0001)
				{
					manip = FALSE;
					sum += tag[i].select;
					manip_pos[0] += tag[i].vec[0] * tag[i].select;
					manip_pos[1] += tag[i].vec[1] * tag[i].select;
					manip_pos[2] += tag[i].vec[2] * tag[i].select;
				}
			}
			vertex_version = udg_get_version(TRUE, TRUE, TRUE, FALSE, FALSE);
			if(manip == FALSE && no_select == TRUE)
				la_t_tm_place(manip_pos[0] / sum, manip_pos[1] / sum, manip_pos[2] / sum);
			no_select = manip;
			if((base = udg_get_base_layer()) != NULL)
			{
				j = 0;
				for(i = 0; i < length; i++)
				{	
					if(vertex[i * 3] != V_REAL64_MAX)
					{
						la_draw_set_vec3(base_lines, j++, vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);
						la_draw_set_vec3(base_lines, j++, base[i * 3], base[i * 3 + 1], base[i * 3 + 2]);
					}
				}
			}
		}
	}

	if(udg_get_reference_geometry(&length, &vertex))
	{
		if(vertex_version != udg_get_reference_version())
		{
		/*	vertex_version = udg_get_reference_version();
			if(vertex_reference_array != NULL)
				free(vertex_reference_array);
			vertex_reference_count = 0;
			for(i = 0; i < length; i++)
				if(vertex[i * 3] != V_REAL64_MAX)
					vertex_reference_count += 3;
			if(vertex_reference_count > 0)
			{
				vertex_reference_array = malloc((sizeof *vertex_reference_array) * vertex_reference_count);
				vertex_reference_count = 0;
				for(i = 0; i < length; i++)
				{
					if(vertex[i * 3] != V_REAL64_MAX)
					{
						vertex_reference_array[vertex_reference_count++] =
							vertex[i * 3];
						vertex_reference_array[vertex_reference_count++] =
							vertex[i * 3 + 1];
						vertex_reference_array[vertex_reference_count++] =
							vertex[i * 3 + 2];
					}
				}
				vertex_reference_count /= 3;
			}else
				vertex_reference_array = NULL;*/
		}
	}else
	{
		free(vertex_reference_array);
		vertex_reference_array = NULL;
	}
}
void draw_owerlay_edge(void)
{
	static float *edge_array = NULL, *edge_color = NULL;
	static uint *edge_ref = NULL, el = 0, ev = 0;
	double *vertex, *vertex0, *vertex1;
	uint length, edge_length, i, j,  *edge;
return;
	udg_get_geometry(&length, NULL, &vertex, NULL, NULL);
	edge = udg_get_edge_data(&edge_length);
	if(el != edge_length || ev != udg_get_version(TRUE, TRUE, FALSE, TRUE, FALSE))
	{
		if(edge_array != 0)
		{
			free(edge_array);
			free(edge_color);
			free(edge_ref);
			edge_array = 0;
			edge_color = 0;
			edge_ref = 0;
		}
		if(edge_length != 0)
		{
			edge_array = malloc((sizeof *edge_array) * edge_length * 5 * 3);
			edge_color = malloc((sizeof *edge_color) * edge_length * 5 * 3);
			edge_ref = malloc((sizeof *edge_ref) * edge_length * 4 * 2);
			for(i = 0; i < edge_length; i++)
			{
				if(edge[i * 2] < length && edge[i * 2 + 1] < length)
				{
					vertex0 = &vertex[edge[i * 2] * 3];
					vertex1 = &vertex[edge[i * 2 + 1] * 3];
					la_draw_set_vec3(edge_array, i * 5, vertex0[0] * 0.95 + vertex1[0] * 0.05, vertex0[1] * 0.95 + vertex1[1] * 0.05, vertex0[2] * 0.95 + vertex1[2] * 0.05);
					la_draw_set_vec3(edge_array, i * 5 + 1, vertex0[0] * 0.75 + vertex1[0] * 0.25, vertex0[1] * 0.75 + vertex1[1] * 0.25, vertex0[2] * 0.75 + vertex1[2] * 0.25);
					la_draw_set_vec3(edge_array, i * 5 + 2, vertex0[0] * 0.50 + vertex1[0] * 0.50, vertex0[1] * 0.50 + vertex1[1] * 0.50, vertex0[2] * 0.50 + vertex1[2] * 0.50);
					la_draw_set_vec3(edge_array, i * 5 + 3, vertex0[0] * 0.25 + vertex1[0] * 0.75, vertex0[1] * 0.25 + vertex1[1] * 0.75, vertex0[2] * 0.25 + vertex1[2] * 0.75);
					la_draw_set_vec3(edge_array, i * 5 + 4, vertex0[0] * 0.05 + vertex1[0] * 0.95, vertex0[1] * 0.05 + vertex1[1] * 0.95, vertex0[2] * 0.05 + vertex1[2] * 0.95);
					la_draw_set_vec3(edge_color, i * 5 + 0, 0.5, 0.5, 0.5);
					la_draw_set_vec3(edge_color, i * 5 + 1, 0.3, 0.3, 0.3);
					la_draw_set_vec3(edge_color, i * 5 + 2, 0.2, 0.2, 0.2);
					la_draw_set_vec3(edge_color, i * 5 + 3, 0.3, 0.3, 0.3);
					la_draw_set_vec3(edge_color, i * 5 + 4, 0.5, 0.5, 0.5);
				}else for(j = 0; j < 5; j++)
				{
					la_draw_set_vec3(edge_array, i * 5 + j, 0, 0, 0);
					la_draw_set_vec3(edge_color, i * 5 + j, 0, 0, 0);
				}
		/*		la_draw_set_ivec2(edge_ref, i * 4 + 0, i * 5, i * 5 + 1);
				la_draw_set_ivec2(edge_ref, i * 4 + 1, i * 5 + 1, i * 5 + 2);
				la_draw_set_ivec2(edge_ref, i * 4 + 2, i * 5 + 2, i * 5 + 3);
				la_draw_set_ivec2(edge_ref, i * 4 + 3, i * 5 + 3, i * 5 + 4);*/
			}
		}
		el = edge_length;
		ev = udg_get_version(TRUE, TRUE, FALSE, TRUE, FALSE);
	}
	if(el != 0)
	{
	//	sui_set_blend_gl(GL_ONE, GL_ONE);
	//	sui_set_color_array_gl(edge_color, el * 5, 3);
	//	r_elements_gl(GL_LINES, edge_array, edge_ref, el * 8, 3, 1, 1, 1, 0.0);
	}
}


extern void draw_view_cage(void);
extern void draw_persuade_surface(ENode *node);

void la_do_owerlay(BInputState *input)
{
	static uint version, draw = FALSE;
	uint32 vertex_count, ref_length, *ref, *crease;
	double *vertex, *select = NULL;
//	if(sui_test_setting_version(&version))
//		draw = imagine_setting_integer_get("RENDER_AS_SDS", TRUE, "Draw subdivision surfaces");
//	if(draw_view_cage() || draw != TRUE)
	ENode *node;

	udg_get_geometry(&vertex_count, &ref_length, &vertex, &ref, &crease);
	if((node = e_ns_get_node(0, udg_get_modeling_node())) != NULL)
		select = e_nsg_get_layer_data(node, e_nsg_get_layer_by_type(node,  VN_G_LAYER_VERTEX_REAL, "select"));
	draw_persuade_surface(input);
	la_do_mesh_update(&GlobalOverlay.current_overlay, vertex_count, ref_length, vertex, udg_get_base_layer(), ref, crease, select, udg_get_version(TRUE, TRUE, TRUE, TRUE, FALSE));
	la_do_mesh_surface(&GlobalOverlay.current_overlay);
	la_do_surface_delete();
}