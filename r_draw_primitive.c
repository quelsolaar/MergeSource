#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "betray.h"
#include "relinquish.h"


char *r_shader_color_vertex = 
"attribute vec3 vertex;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"void main()\n"
"{"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);\n"
"}";

char *r_shader_color_fragment = 
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"	gl_FragColor = color;\n"
"}\n";

char *r_shader_vertex_color_vertex = 
"attribute vec3 vertex;"
"attribute vec4 color;"
"varying vec4 col;"
"uniform mat4 ModelViewProjectionMatrix;"
"void main()"
"{"
"	col = color;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);"
"}";

char *r_shader_vertex_color_fragment = 
"varying vec4 col;"
"void main()"
"{"
"	gl_FragColor = col;"
"}";


char *r_shader_texture_vertex = 
"attribute vec4 vertex;"
"attribute vec2 uv;"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec2 mapping;"
"void main()"
"{"
"	mapping = uv;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);"
"}";

char *r_shader_texture_fragment = 
"uniform sampler2D image;"
"varying vec2 mapping;"
"void main()"
"{"
"	gl_FragColor = texture2D(image, mapping).rgrg;"
"}";


char *r_shader_surface_vertex = 
"attribute vec4 vertex;"
"attribute vec4 alpha;"
"uniform vec4 color;"
"uniform vec3 pos;"
"uniform vec2 size;"
"uniform vec2 pixel;"
"uniform mat4 NormalMatrix;"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec4 col;"
"varying vec3 normal;"
"varying vec4 v;"
"void main()"
"{"
"	vec3 expand;"
"	vec3 vec;"
"	col = color * alpha;"
"	normal = normalize((NormalMatrix * vec4(0.0, 0.0, -1.0, 0.0)).xyz);"
"	vec = pos + vec3(vertex.xy * size, 0.0);"
"	expand = vec3((ModelViewProjectionMatrix * vec4(vec, 1.0)).zz * vertex.ba * pixel, 0);"
"	gl_Position = v = ModelViewProjectionMatrix * vec4(vec + expand, 1.0) + vec4(expand, 0);"
"}";

char *r_shader_surface_fragment = 
"uniform sampler2D reflection;"
"varying vec4 col;"
"varying vec3 normal;"
"varying vec4 v;"
"void main()"
"{"
"	vec3 ref;"
"	ref = normalize(reflect(normalize(v.xyz), normal.xyz)) * vec3(0.5) + vec3(0.5);"
"	gl_FragColor = col + vec4(texture2D(reflection, ref.xy).xyz * (vec3(1.0) + (normal.zzz)), 0.0);"
"}";

char *r_shader_image_vertex = 
"attribute vec4 vertex;"
"uniform vec4 uv;"
"uniform vec4 color;"
"uniform mat4 NormalMatrix;"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec4 col;"
"varying vec2 mapping;"
"void main()"
"{"
"	mapping = uv.rg + vertex.xy * uv.ba;"
"	col = color;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);"
"}";

char *r_shader_image_fragment = 
"uniform sampler2D image;"
"varying vec4 col;"
"varying vec2 mapping;"
"void main()"
"{"
"	vec4 tex;"
"	tex = texture2D(image, mapping).rgba;"
"	gl_FragColor = tex * col;"
"}";



char *r_shader_sprite_vertex = 
"attribute vec3 vertex;"
"attribute vec4 color;"
"attribute vec4 size;"
"uniform mat4 NormalMatrix;"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec2 uv;"
"varying vec4 col;"
"void main()"
"{"
"	uv = size.zw;"
"	col = color;"
"	gl_Position = ModelViewProjectionMatrix * (vec4(vertex.xyz, 1.0) + vec4(size.xy, 0.0, 0.0) * NormalMatrix);"
"}";

char *r_shader_sprite_fragment = 
"uniform sampler2D image;"
"varying vec2 uv;"
"varying vec4 col;"
"void main()"
"{"
"	vec2 vec;"
"	vec = uv.xy - vec2(0.5);"
"	if(length(vec) > 0.5)"
"		discard;"
"	gl_FragColor = vec4(1, 1, 1, texture2D(image, uv.xy).a) * col;"
"}";


#define SUI_DRAW_SURFACE_BUFFER (128 * 6)
#define SUI_DRAW_IMAGE_BUFFER (6)
#define SUI_DRAW_LINE_BUFFER (1024)
#define SUI_DRAW_SPRITE_BUFFER (1024)

extern uint sui_3d_texture_shade;
extern uint sui_3d_texture_reflection;

void *r_surface_pool = NULL;
void *r_surface_section = NULL;
uint r_surface_count = 0;

void *r_line_pool = NULL;
uint r_line_count = 0;

void *r_sprite_pool = NULL;
void *r_sprite_section = NULL;
uint r_sprite_count = 0;
uint r_sprite_texture_id = -1;

void *r_image_pool = NULL;
void *r_image_section = NULL;

RShader *r_vertex_color_shader = NULL;
RShader *r_color_shader = NULL;
uint r_color_location_color;
RShader *r_image_shader = NULL;
uint r_image_location_uv;
uint r_image_location_color;
uint r_image_location_pos;
uint r_image_location_size;
uint r_image_location_pixel;

RShader *r_texture_shader = NULL;
RShader *r_surface_shader = NULL;
uint r_surface_location_color;
uint r_surface_location_pos;
uint r_surface_location_size;
uint r_surface_location_pixel;

RShader *r_sprite_shader = NULL;

void r_primitive_image_aa_set(float *array, float pos_x, float pos_y, float alpha, float dist_x, float dist_y)
{
	array[0] = pos_x;
	array[1] = pos_y;
	array[2] = dist_x;
	array[3] = dist_y;
	array[4] = 1;
	array[5] = 1;
	array[6] = 1;
	array[7] = alpha;
}

void r_primitive_init()
{
	RFormats vertex_format_types[3] = {R_FLOAT, R_FLOAT, R_FLOAT};
	uint vertex_format_size[3] = {3, 4, 4};
	uint vertex_format_size_surface[2] = {4, 4};
	float array[8 * 30], surface[3 * 6] = {0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0};
	char buffer[2048];
	if(r_line_pool != NULL)
		free(r_line_pool);
	r_line_pool = r_array_allocate(SUI_DRAW_LINE_BUFFER, vertex_format_types, vertex_format_size, 2, 0);
	if(r_surface_pool != NULL)
		free(r_surface_pool);
	r_surface_pool = r_array_allocate(6, vertex_format_types, vertex_format_size, 1, 0);
	if(r_image_pool != NULL)
		free(r_image_pool);
	r_image_pool = r_array_allocate(SUI_DRAW_IMAGE_BUFFER, vertex_format_types, vertex_format_size_surface, 1, 0);
	if(r_sprite_pool != NULL)
		free(r_sprite_pool);
	r_sprite_pool = r_array_allocate(SUI_DRAW_SPRITE_BUFFER, vertex_format_types, vertex_format_size, 3, 0);

	r_primitive_image_aa_set(&array[8 * 0], 0, 0, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 1], 1, 0, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 2], 1, 1, 1, 0, 0);

	r_primitive_image_aa_set(&array[8 * 3], 0, 0, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 4], 1, 1, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 5], 0, 1, 1, 0, 0);

	r_primitive_image_aa_set(&array[8 * 6], 0, 0, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 7], 1, 0, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 8], 1, 0, 0, 1, -1);

	r_primitive_image_aa_set(&array[8 * 9], 0, 0, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 10], 1, 0, 0, 1, -1);
	r_primitive_image_aa_set(&array[8 * 11], 0, 0, 0, -1, -1);

	r_primitive_image_aa_set(&array[8 * 12], 1, 0, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 13], 1, 0, 0, 1, -1);
	r_primitive_image_aa_set(&array[8 * 14], 1, 1, 1, 0, 0);

	r_primitive_image_aa_set(&array[8 * 15], 1, 1, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 16], 1, 0, 0, 1, -1);
	r_primitive_image_aa_set(&array[8 * 17], 1, 1, 0, 1, 1);

	r_primitive_image_aa_set(&array[8 * 18], 0, 1, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 19], 1, 1, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 20], 1, 1, 0, 1, 1);

	r_primitive_image_aa_set(&array[8 * 21], 0, 1, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 22], 1, 1, 0, 1, 1);
	r_primitive_image_aa_set(&array[8 * 23], 0, 1, 0, -1, 1);

	r_primitive_image_aa_set(&array[8 * 24], 0, 1, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 25], 0, 1, 0, -1, 1);
	r_primitive_image_aa_set(&array[8 * 26], 0, 0, 1, 0, 0);

	r_primitive_image_aa_set(&array[8 * 27], 0, 0, 1, 0, 0);
	r_primitive_image_aa_set(&array[8 * 28], 0, 1, 0, -1, 1);
	r_primitive_image_aa_set(&array[8 * 29], 0, 0, 0, -1, -1);
	r_array_load_vertex(r_image_pool, NULL, array, 0, 30);

	r_array_load_vertex(r_surface_pool, NULL, surface, 0, 6);
	if(r_color_shader != NULL)
		free(r_color_shader);
	r_color_shader = r_shader_create_simple(NULL, 0, r_shader_color_vertex, r_shader_color_fragment, "color primitive");
	r_color_location_color = r_shader_uniform_location(r_color_shader, "color");
	if(r_vertex_color_shader != NULL)
		free(r_vertex_color_shader);
	r_vertex_color_shader = r_shader_create_simple(NULL, 0, r_shader_vertex_color_vertex, r_shader_vertex_color_fragment, "vertex color primitive");
	r_shader_state_set_blend_mode(r_vertex_color_shader, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);	
	if(r_surface_shader != NULL)
		free(r_surface_shader);	
	r_surface_shader = r_shader_create_simple(buffer, 2048, r_shader_surface_vertex, r_shader_surface_fragment, "color primitive");
	r_surface_location_color = r_shader_uniform_location(r_surface_shader, "color");
	r_surface_location_pos = r_shader_uniform_location(r_surface_shader, "pos");
	r_surface_location_size = r_shader_uniform_location(r_surface_shader, "size");
	r_surface_location_pixel = r_shader_uniform_location(r_surface_shader, "pixel");
	if(r_image_shader != NULL)
		free(r_image_shader);	
	r_image_shader = r_shader_create_simple(NULL, 0, r_shader_image_vertex, r_shader_image_fragment, "image primitive");
	r_shader_state_set_blend_mode(r_image_shader, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//	r_shader_state_set_blend_mode(r_image_shader, GL_ONE, GL_ONE);
	r_shader_state_set_depth_test(r_image_shader, GL_ALWAYS);
	r_image_location_uv = r_shader_uniform_location(r_image_shader, "uv");
	r_image_location_color = r_shader_uniform_location(r_image_shader, "color");
	r_image_location_pos = r_shader_uniform_location(r_image_shader, "pos");
	r_image_location_size = r_shader_uniform_location(r_image_shader, "size");
	r_image_location_pixel = r_shader_uniform_location(r_image_shader, "pixel");
	if(r_texture_shader != NULL)
		free(r_texture_shader);	
	r_texture_shader = r_shader_create_simple(NULL, 0, r_shader_texture_vertex, r_shader_texture_fragment, "texture");
	if(r_sprite_shader != NULL)
		free(r_sprite_shader);	
	r_sprite_shader = r_shader_create_simple(NULL, 0, r_shader_sprite_vertex, r_shader_sprite_fragment, "sprite primitive");
}

uint special_drawcall_active = FALSE;

void r_primitive_image(float pos_x, float pos_y, float pos_z, float size_x, float size_y, float u_start, float v_start, float u_end, float v_end, uint texture_id, float red, float green, float blue, float alpha)
{
	RMatrix *m;
	r_shader_set(r_image_shader);
	r_shader_vec4_set(NULL, r_image_location_uv, u_start, v_start, u_end - u_start, v_end - v_start);
	r_shader_vec4_set(NULL, r_image_location_color, red, green, blue, alpha);
	m = r_matrix_get();
	r_matrix_push(m);
	r_matrix_translate(m, pos_x, pos_y, pos_z);
	r_matrix_scale(m, size_x, size_y, 0);
	r_shader_uniform_texture_set(r_image_shader, r_shader_uniform_location(r_image_shader, "image"), r_shader_uniform_texture_pointer_get(texture_id));
	special_drawcall_active = TRUE;
	r_array_section_draw(r_surface_pool, NULL, GL_TRIANGLES, 0, 30);
	special_drawcall_active = FALSE;
	r_matrix_pop(m);
}

void r_primitive_image_shader(float pos_x, float pos_y, float pos_z, float size_x, float size_y, RShader *shader)
{
	RMatrix *m;
	r_shader_set(shader);
	m = r_matrix_get();
	r_matrix_push(m);
	r_matrix_translate(m, pos_x, pos_y, pos_z);
	r_matrix_scale(m, size_x, size_y, 0);
	special_drawcall_active = TRUE;
	r_array_section_draw(r_surface_pool, NULL, GL_TRIANGLES, 0, 30);
	special_drawcall_active = FALSE;
	r_matrix_pop(m);
}

void r_primitive_surface(float pos_x, float pos_y, float pos_z, float size_x, float size_y, float red, float green, float blue, float alpha)
{
	RMatrix *m;
	r_shader_set(r_color_shader);
	r_shader_vec4_set(NULL, r_color_location_color, red, green, blue, alpha);
	m = r_matrix_get();
	r_matrix_push(m);
	r_matrix_translate(m, pos_x, pos_y, pos_z);
	r_matrix_scale(m, size_x, size_y, 0);
	r_array_section_draw(r_surface_pool, NULL, GL_TRIANGLES, 0, 30);
	r_matrix_pop(m);

}

void r_primitive_surface2(float pos_x, float pos_y, float pos_z, float size_x, float size_y, float red, float green, float blue, float alpha)
{
	static float time = 0;
	RMatrix *m;
	r_shader_set(r_color_shader);
	r_shader_vec4_set(NULL, r_color_location_color, red, green, blue, alpha);
	m = r_matrix_get();
	r_matrix_push(m);
	r_matrix_translate(m, pos_x, pos_y, 0);
	r_matrix_scale(m, size_x, size_y, 1);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	r_array_section_draw(r_surface_pool, NULL, GL_TRIANGLES, 0, 6);
	time += 0.01f;
	glBlendFunc(GL_ONE, GL_ONE);
	r_shader_vec4_set(NULL, r_color_location_color, 0.5f, 0.5f, 0.5f, 0.1f);
	r_array_section_draw(r_surface_pool, NULL, GL_TRIANGLES, 0, 6);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	r_matrix_pop(m);

}

void r_primitive_line_flush()
{
	r_shader_set(r_vertex_color_shader);
	r_array_section_draw(r_line_pool, NULL, GL_LINES, 0, r_line_count);
	r_line_count = 0;	
}

void r_primitive_line_clear()
{
	r_line_count = 0;	
}


void r_primitive_line_3d(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, float red, float green, float blue, float alpha)
{
	float array[14];
	array[0] = start_x;
	array[1] = start_y;
	array[2] = start_z;
	array[3] = array[10] = red;
	array[4] = array[11] = green;
	array[5] = array[12] = blue;
	array[6] = array[13] = alpha;
	array[7] = end_x;
	array[8] = end_y;
	array[9] = end_z;
	r_array_load_vertex(r_line_pool, NULL, array, r_line_count, 2);
	r_line_count += 2;
	if(r_line_count == SUI_DRAW_LINE_BUFFER)
		r_primitive_line_flush();
}

void r_primitive_line_2d(float start_x, float start_y, float end_x, float end_y, float red, float green, float blue, float alpha)
{
	float array[14];
	array[0] = start_x;
	array[1] = start_y;
	array[2] = 0;
	array[3] = array[10] = red;
	array[4] = array[11] = green;
	array[5] = array[12] = blue;
	array[6] = array[13] = alpha;
	array[7] = end_x;
	array[8] = end_y;
	array[9] = 0;
	r_array_load_vertex(r_line_pool, NULL, array, r_line_count, 2);
	r_line_count += 2;
	if(r_line_count == SUI_DRAW_LINE_BUFFER)
		r_primitive_line_flush();
}


void r_primitive_line_fade_3d(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z, float start_red, float start_green, float start_blue, float start_alpha, float end_red, float end_green, float end_blue, float end_alpha)
{
	float array[14];
	array[0] = start_x;
	array[1] = start_y;
	array[2] = start_z;
	array[3] = start_red;
	array[4] = start_green;
	array[5] = start_blue;
	array[6] = start_alpha;
	array[7] = end_x;
	array[8] = end_y;
	array[9] = end_z;
	array[10] = end_red;
	array[11] = end_green;
	array[12] = end_blue;
	array[13] = end_alpha;
	r_array_load_vertex(r_line_pool, NULL, array, r_line_count, 2);
	r_line_count += 2;
	if(r_line_count == SUI_DRAW_LINE_BUFFER)
		r_primitive_line_flush();
}

void r_primitive_line_fade_2d(float start_x, float start_y, float end_x, float end_y, float start_red, float start_green, float start_blue, float start_alpha, float end_red, float end_green, float end_blue, float end_alpha)
{
	float array[14];
	array[0] = start_x;
	array[1] = start_y;
	array[2] = 0;
	array[3] = start_red;
	array[4] = start_green;
	array[5] = start_blue;
	array[6] = start_alpha;
	array[7] = end_x;
	array[8] = end_y;
	array[9] = 0;
	array[10] = end_red;
	array[11] = end_green;
	array[12] = end_blue;
	array[13] = end_alpha;
	r_array_load_vertex(r_line_pool, NULL, array, r_line_count, 2);
	r_line_count += 2;
	if(r_line_count == SUI_DRAW_LINE_BUFFER)
		r_primitive_line_flush();
}

void r_primitive_sprite_flush()
//FK: There's no GL_QUADS on OpenGL ES
#ifndef BETRAY_CONTEXT_OPENGLES
	r_shader_set(r_sprite_shader);
	r_array_section_draw(r_sprite_pool, r_sprite_section, GL_QUADS, 0, r_sprite_count);
	r_sprite_count = 0;
#endif
}

void r_primitive_sprite(float pos_x, float pos_y, float pos_z, uint texture_id, float size_x, float size_y, float red, float green, float blue, float alpha)
{
	float array[11 * 4];
	if(r_sprite_texture_id != texture_id)
	{
		r_primitive_sprite_flush();
		r_shader_texture_set(r_sprite_shader, 0, texture_id);
		r_sprite_texture_id = texture_id;
	}
	array[0] = pos_x;
	array[1] = pos_y;
	array[2] = pos_z;
	array[3] = red;
	array[4] = green;
	array[5] = blue;
	array[6] = alpha;
	array[7] = -size_x;
	array[8] = -size_y;
	array[9] = 0;
	array[10] = 0;

	array[11] = pos_x;
	array[12] = pos_y;
	array[13] = pos_z;
	array[14] = red;
	array[15] = green;
	array[16] = blue;
	array[17] = alpha;
	array[18] = size_x;
	array[19] = -size_y;
	array[20] = 1;
	array[21] = 0;
	
	array[22] = pos_x;
	array[23] = pos_y;
	array[24] = pos_z;
	array[25] = red;
	array[26] = green;
	array[27] = blue;
	array[28] = alpha;
	array[29] = size_x;
	array[30] = size_y;
	array[31] = 1;
	array[32] = 1;

	array[33] = pos_x;
	array[34] = pos_y;
	array[35] = pos_z;
	array[36] = red;
	array[37] = green;
	array[38] = blue;
	array[39] = alpha;
	array[40] = -size_x;
	array[41] = size_y;
	array[42] = 0;
	array[43] = 1;

	r_array_load_vertex(r_sprite_pool, r_sprite_section, array, r_sprite_count, 4);
	r_sprite_count += 4;
	if(r_sprite_count == SUI_DRAW_SPRITE_BUFFER)
		r_primitive_sprite_flush();
}

void r_primitive_sprite_uv(float pos_x, float pos_y, float pos_z, uint texture_id, float size_x, float size_y, float u_start, float v_start, float u_end, float v_end, float red, float green, float blue, float alpha)
{
	float array[11 * 4];
	if(r_sprite_texture_id != texture_id)
	{
		r_primitive_sprite_flush();
		r_shader_texture_set(r_sprite_shader, 0, texture_id);
		r_sprite_texture_id = texture_id;
	}
	array[0] = pos_x;
	array[1] = pos_y;
	array[2] = pos_z;
	array[3] = red;
	array[4] = green;
	array[5] = blue;
	array[6] = alpha;
	array[7] = -size_x;
	array[8] = -size_y;
	array[9] = u_start;
	array[10] = v_start;

	array[11] = pos_x;
	array[12] = pos_y;
	array[13] = pos_z;
	array[14] = red;
	array[15] = green;
	array[16] = blue;
	array[17] = alpha;
	array[18] = size_x;
	array[19] = -size_y;
	array[20] = u_end;
	array[21] = v_start;
	
	array[22] = pos_x;
	array[23] = pos_y;
	array[24] = pos_z;
	array[25] = red;
	array[26] = green;
	array[27] = blue;
	array[28] = alpha;
	array[29] = size_x;
	array[30] = size_y;
	array[31] = u_end;
	array[32] = v_end;

	array[33] = pos_x;
	array[34] = pos_y;
	array[35] = pos_z;
	array[36] = red;
	array[37] = green;
	array[38] = blue;
	array[39] = alpha;
	array[40] = -size_x;
	array[41] = size_y;
	array[42] = u_start;
	array[43] = v_end;

	r_array_load_vertex(r_sprite_pool, r_sprite_section, array, r_sprite_count, 4);
	r_sprite_count += 4;
	if(r_sprite_count == SUI_DRAW_SPRITE_BUFFER)
		r_primitive_sprite_flush();
}

void	*r_shader_presets_get(RShaderPresets preset)
{
	switch(preset)
	{
		case P_SP_COLOR_UNIFORM :
			return r_color_shader;
		case P_SP_COLOR_VERTEX :
			return r_vertex_color_shader;
		case P_SP_TEXTURE :
			return r_texture_shader;
		case P_SP_COLORED_UNIFORM_TEXTURE :
			return r_image_shader;
	}
	return NULL;

}
