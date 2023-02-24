#include <stdlib.h>
#include <stdio.h>

#include "seduce.h"


#include "s_draw_3d_data.h"
#include "s_texture.h"

void *seduce_object_3d_pool = NULL;
RShader *seduce_object_3d_shader = NULL;
uint seduce_object_3d_location_fade_a;
uint seduce_object_3d_location_fade_b;
uint seduce_object_3d_location_fade;
uint seduce_object_3d_location_reflect_color;
uint seduce_object_3d_location_light_color;
uint seduce_object_3d_location_shade_color;
uint seduce_object_3d_texture_shade;
uint seduce_object_3d_texture_reflection;

char *seduce_object_3d_shader_vertex = 
"attribute vec4 vertex;\n"
"attribute vec4 normal;\n"
"varying vec3 n;\n"
"varying vec3 n2;\n"
"varying vec3 n3;\n"
"varying vec3 c;\n"
"varying vec3 surface_pos;"
"varying vec3 reflection_uv;\n"
"varying float fernel;\n"
"varying vec4 temp_color;\n"
"uniform vec3 fade_a;\n"
"uniform vec3 fade_b;\n"
"uniform mat4 ModelViewMatrix;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"void main()\n"
"{\n"
"	c = mix(fade_a, fade_b, normal.aaa);\n"
"	n3 = (NormalMatrix * vec4(normalize(normal.xyz), 0.0)).xyz;"
"	fernel = (1.0 - n3.z) * 2.0;\n"
"	n = n3 * vec3(0.5) + vec3(0.5);\n"
"	n2 = normalize(normal.xyz) * vec3(0.5) + vec3(0.5);\n"
"	surface_pos = vec3((ModelViewMatrix * vec4(vertex.xyz, 1)).xy, 0.1);"
//"	surface_pos /= surface_pos.zzz;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);\n"
"	surface_pos = gl_Position.xyz / gl_Position.zzz;"
"	temp_color = vec4(normal.xyz, 1);\n"
"	reflection_uv = vec3(0.5) + reflect(normalize(surface_pos), n3.xyz) * vec3(0.5);\n"
"}\n";

char *seduce_object_3d_shader_fragment = 
"varying vec3 n;"
"varying vec3 n2;"
"varying vec3 n3;"
"varying vec3 c;"
"varying vec3 reflection_uv;\n"
"varying float fernel;\n"
"varying vec4 temp_color;\n"
"uniform sampler2D light;"
"uniform sampler2D reflection;"
"uniform vec3 reflect_color;"
"uniform float fade;"
"varying vec3 surface_pos;"
"uniform vec3 light_color;"
"uniform vec3 shade_color;"
"uniform vec4 light_pos0;"
"uniform vec4 light_pos1;"
"uniform vec4 light_pos2;"
"uniform vec4 light_pos3;"
"uniform vec4 light_pos4;"
"uniform vec4 light_pos5;"
"void main()"
"{"
"	vec3 temp;"
"	vec4 color;"
"	float f, gray;"
"	temp = vec3(light_pos0.xy - surface_pos.xy, 0.1);"
"	f = max(0.0, light_pos0.a * (1 + dot(normalize(temp), normalize(n3))) / dot(temp, temp));"
"	temp = vec3(light_pos1.xy - surface_pos.xy, 0.1);"
"	f += max(0.0, light_pos1.a * (1 + dot(normalize(temp), normalize(n3))) / dot(temp, temp));"
"	temp = vec3(light_pos2.xy - surface_pos.xy, 0.1);"
"	f += max(0.0, light_pos2.a * (1 + dot(normalize(temp), normalize(n3))) / dot(temp, temp));"
"	temp = vec3(light_pos3.xy - surface_pos.xy, 0.1);"
"	f += max(0.0, light_pos3.a * (1 + dot(normalize(temp), normalize(n3))) / dot(temp, temp));"
"	temp = vec3(light_pos4.xy - surface_pos.xy, 0.1);"
"	f += max(0.0, light_pos4.a * (1 + dot(normalize(temp), normalize(n3))) / dot(temp, temp));"
"	temp = vec3(light_pos5.xy - surface_pos.xy, 0.1);"
"	f += max(0.0, light_pos5.a * (1 + dot(normalize(temp), normalize(n3))) / dot(temp, temp));"
//"   color = texture2D(light, reflection_uv.xy);"
//"   color = vec4(0.8);"
//"	gl_FragColor = mix(vec4(c * (vec3(f * 0.1) *  light_color + color.xyz * color.xyz * vec3(fernel)), 1), vec4(c, 1), vec4(fade));"
//"	gl_FragColor = mix(vec4(c * (light_color + color.xyz + vec3(f * fernel * 0.2)), 1), vec4(c, 1), vec4(fade));"
"	gl_FragColor = vec4(c, 1.0) * vec4(0.7 + fernel * f * 0.02);"
"}";

char *crystal_shader_vertex =
"attribute vec3 vertex;\n"
"attribute vec4 normal;\n"
"varying vec4 col;\n"
"varying vec4 shade;\n"
"varying vec4 pos;\n"
"varying vec3 reflection_uv;\n"
"varying vec3 refraction_uv;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"\n"
"void main()\n"
"{\n"
"	vec4 n, p;\n"
//"	col = vec4(normal.xyz, 1.0);\n"
"	n = NormalMatrix * vec4(normalize(normal.xyz), 0.0);\n"
"	p = ModelViewProjectionMatrix * vec4(vertex, 1.0);\n"
//"	col = vec4(vec3(dot(n.xyz, n.xyz)), 1.0) * vec4(0.5);\n"
"	col = vec4(vec3(1.5) - n.zzz, 1.0);\n"
"	shade = 0.5/* normal.aaaa*/;\n"
"	reflection_uv = vec3(0.5) + reflect(p.xyz, n.xyz) * vec3(0.5);\n"
"	gl_Position = p;\n"
"	pos = p - vec4(0, 0, 0.5, 0);\n"
"	refraction_uv = vec3(0.5) + (p.xyz - n.xyz * vec3(0.4)) * vec3(0.5);\n"
"}\n";

char *crystal_shader_fragment =
"varying vec4 col;\n"
"varying vec4 shade;\n"
"varying vec4 pos;\n"
"varying vec3 reflection_uv;\n"
"varying vec3 refraction_uv;\n"
"uniform sampler2D image;\n"
"uniform vec4 color_a;\n"
"uniform vec4 color_b;\n"
"\n"
"void main()\n"
"{\n"
"	vec4 ref;\n"
"	ref = texture2D(image, reflection_uv.xy);\n"
"	gl_FragColor = col * ref * ref + (vec4(1.0) - texture2D(image, refraction_uv.xy)) * vec4(0.2, 0.4, 0.1, 1.0)/* * mix(color_b, color_a, shade)*/;\n"
"//	gl_FragColor = mix(color_b, color_a, shade);\n"
"//	gl_FragColor = reflection_uv.xyxy;\n"
"//	gl_FragColor = ref + texture2D(image, refraction_uv.xy);\n"
"}\n\n";


/*void *r_3d_sections[50];*/

extern float seduce_background_shader_light_color[4];
extern float seduce_background_shader_shade_color[4];

void seduce_object_3d_color(float col_a_r, float col_a_g, float col_a_b, float col_b_r, float col_b_g, float col_b_b, float reflect_r, float reflect_g, float reflect_b)
{
	r_shader_set(seduce_object_3d_shader);
	r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_fade_a, col_a_r, col_a_g, col_a_b);
	r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_fade_b, col_b_r - col_a_r, col_b_g - col_a_g, col_b_b - col_a_b);
	r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_reflect_color, reflect_r, reflect_g, reflect_b);
	r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_light_color, seduce_background_shader_light_color[0], seduce_background_shader_light_color[1], seduce_background_shader_light_color[2]);
	r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_shade_color, seduce_background_shader_shade_color[0], seduce_background_shader_shade_color[1], seduce_background_shader_shade_color[2]);
}

extern uint r_texture_gen(void *data, uint type, uint x_size, uint y_size, boolean alpha);



void sui_3d_export();

void seduce_object_3d_geometry_init()
{
	RFormats vertex_format_types[2] = {R_FLOAT, R_FLOAT};
	uint vertex_format_size[2] = {3, 4}, i, count;
	count = 0;
	if(seduce_object_3d_pool != NULL)
		r_array_free(seduce_object_3d_pool);
	for(i = 0; i < SEDUCE_DRAW_3D_COUNT; i++)
		count += seduce_3d_object_size[i];
	seduce_object_3d_pool = r_array_allocate(count, vertex_format_types, vertex_format_size, 2, 0);
	for(i = 0; i < SEDUCE_DRAW_3D_COUNT; i++)
	{
		seduce_object_3d_sections[i] = r_array_section_allocate_vertex(seduce_object_3d_pool, seduce_3d_object_size[i]);
		r_array_load_vertex(seduce_object_3d_pool, seduce_object_3d_sections[i], seduce_3d_object_array[i], 0, seduce_3d_object_size[i]);
	}
}

void seduce_object_3d_init()
{
	uint i;
	char buf[5000];
	seduce_object_3d_geometry_init();
	seduce_object_3d_shader = r_shader_create_simple(buf, 5000, seduce_object_3d_shader_vertex, seduce_object_3d_shader_fragment, "3Dobjects");
	r_shader_set(seduce_object_3d_shader);
	seduce_object_3d_texture_shade = r_texture_allocate(R_IF_RGB_UINT8, 128, 128, 1, TRUE, TRUE, image_buffer);
	r_shader_texture_set(seduce_object_3d_shader, 0, seduce_object_3d_texture_shade);
	r_shader_texture_set(seduce_object_3d_shader, 1, seduce_object_3d_texture_shade);
	seduce_object_3d_location_fade_a = r_shader_uniform_location(seduce_object_3d_shader, "fade_a");
	seduce_object_3d_location_fade_b = r_shader_uniform_location(seduce_object_3d_shader, "fade_b");
	seduce_object_3d_location_fade = r_shader_uniform_location(seduce_object_3d_shader, "fade");
	seduce_object_3d_location_reflect_color = r_shader_uniform_location(seduce_object_3d_shader, "reflect_color");
	seduce_object_3d_location_light_color = r_shader_uniform_location(seduce_object_3d_shader, "light_color");
	seduce_object_3d_location_shade_color = r_shader_uniform_location(seduce_object_3d_shader, "shade_color");
	seduce_object_3d_color(0.5, 0.5, 0.5, 
							0.05, 0.05, 0.05,
							1.0, 1.0, 1.0);
//	sui_3d_export();
}

extern uint r_color_shader;
/*
void r_object_begin()
{
	r_shader_set(seduce_object_3d_shader);
}

void r_object_end()
{
	r_array_deactivate();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
} 

void r_object(uint id)
{
	r_array_section_draw(seduce_object_3d_pool, seduce_object_3d_sections[id], GL_TRIANGLES, 0, -1);
}*/


float seduce_object_3d_lightning(BInputState *input, float pos_x, float pos_y, float pos_z, uint *locations, uint location_count)
{
	static float *brightness_pos = NULL;
	static uint frame = 0;
	uint i, count;
	float vec[2], f, best, pos[3];

	RMatrix	*matrix;
	matrix = r_matrix_get();
	f_transform3f(pos, matrix->matrix[matrix->current], pos_x, pos_y, pos_z);
	pos[0] /= -pos[2];
	pos[1] /= -pos[2];

	count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(brightness_pos == NULL)
	{
		brightness_pos = malloc((sizeof *brightness_pos) * 3 * count);
		for(i = 0; i < count * 3; i++)
			brightness_pos[i] = 0;
	}
	if(frame != input->frame_number)
	{
		frame = input->frame_number;
		for(i = 0; i < input->pointer_count; i++)
		{
			brightness_pos[i * 3 + 0] = input->pointers[i].pointer_x;
			brightness_pos[i * 3 + 1] = input->pointers[i].pointer_y;
			if(input->pointers[i].button[0])
				brightness_pos[i * 3 + 2] = 1.0 * input->delta_time * 5.0 + brightness_pos[i * 3 + 2] * (1.0 - input->delta_time * 5.0);
			else if(input->pointers[i].delta_pointer_x > 0.001 || 
					input->pointers[i].delta_pointer_x < -0.001 || 
					input->pointers[i].delta_pointer_y > 0.001 || 
					input->pointers[i].delta_pointer_y < -0.001)
				brightness_pos[i * 3 + 2] = 0.5 * input->delta_time + brightness_pos[i * 3 + 2] * (1.0 - input->delta_time);
			else
				brightness_pos[i * 3 + 2] = brightness_pos[i * 3 + 2] * (1.0 - input->delta_time);
		}
		for(; i < count; i++)
			brightness_pos[i * 3 + 2] = brightness_pos[i * 3 + 2] * (1.0 - input->delta_time);
	}

	best = 1000000.0;
	for(i = 0; i < input->pointer_count; i++)
	{
		vec[0] = (brightness_pos[i * 3 + 0] - pos[0]) * 5.0;
		vec[1] = (brightness_pos[i * 3 + 1] - pos[1]) * 5.0;
		f = vec[0] * vec[0] + vec[1] * vec[1];
		if(f < best)
			best = f;
	}

	for(i = 0; i < location_count; i++)
	{
	/*	if(brightness_pos[i * 3 + 2] > 0.2)
			r_shader_vec4_set(NULL, locations[i], brightness_pos[i * 3], brightness_pos[i * 3 + 1], -0.75, 0.2);
		else */
			r_shader_vec4_set(NULL, locations[i], brightness_pos[i * 3], brightness_pos[i * 3 + 1], -0.75, brightness_pos[i * 3 + 2]);
	}

	return 1.0 - (1 / (1 + best));
}

void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float size, uint id, float fade, float *color)
{
	uint location[6];
	float f;
	RMatrix *matrix;
	if(input->mode != BAM_DRAW)
		return;
	r_shader_set(seduce_object_3d_shader);
	matrix = r_matrix_get();
	
	location[0] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos0");
	location[1] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos1");
	location[2] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos2");
	location[3] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos3");
	location[4] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos4");
	location[5] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos5");
	fade = (1.0 - fade) * seduce_object_3d_lightning(input, pos_x, pos_y, pos_z, location, 6);

	f = 0.4 - fade * 0.3;
	r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_fade_a, f, f, f);
	if(color == NULL)
	{
		f = 0.7 - fade * 0.4;
		r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_fade_b, f, f, f);
	}else
		r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_fade_b, color[0], color[1], color[2]);
	r_matrix_push(matrix);
	r_matrix_translate(matrix, pos_x, pos_y, pos_z);
	r_matrix_scale(matrix, size, size, size * (1.0 - fade));
	r_shader_float_set(seduce_object_3d_shader, seduce_object_3d_location_fade, fade);
	glBlendFunc(GL_ONE, GL_ZERO);
	glEnable(GL_DEPTH_TEST);
	r_shader_set(seduce_object_3d_shader);
	r_array_section_draw(seduce_object_3d_pool, seduce_object_3d_sections[id], GL_TRIANGLES, 0, -1);
	r_matrix_pop(matrix);
}



uint seduce_object_3d_count()
{
	return SEDUCE_DRAW_3D_COUNT;
}

char *seduce_object_3d_object_name(uint object)
{
	return seduce_3d_object_name[object];
}

uint seduce_object_3d_object_lookup(char *name)
{
	uint i, j;
	for(i = 0; i < SEDUCE_DRAW_3D_COUNT; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == seduce_3d_object_name[i][j]; j++);
		if(name[j] == seduce_3d_object_name[i][j])
			return i;
	}
	return -1;
}



void sui_3d_export()
{
	uint i, j;
	FILE *file;

	file = fopen("seduce_export.vml", "w"); 
	fprintf(file, "<vml version=\"1.0\">\n\n");

	for(i = 0; i < SEDUCE_DRAW_3D_COUNT; i++)
	{
		fprintf(file, "<node-geometry id=\"n%u\" name=\"%s\">\n", i, seduce_3d_object_name[i]);
		fprintf(file, "\t<layers>\n");
		fprintf(file, "\t\t<layer-vertex-xyz name=\"vertex\">\n");
		for(j = 0; j < seduce_3d_object_size[i]; j++)
		{
			fprintf(file, "\t\t\t<v>%u %f %f %f</v>\n", j, seduce_3d_object_array[i][j * 7], seduce_3d_object_array[i][j * 7 + 1], seduce_3d_object_array[i][j * 7 + 2]);
		}
		fprintf(file, "\t\t</layer-vertex-xyz>\n");
		fprintf(file, "\t\t<layer-polygon-corner-uint32 name=\"polygon\">\n");

		for(j = 0; j < seduce_3d_object_size[i]; j += 3)
		{
			fprintf(file, "\t\t\t<p>%u %u %u %u</p>\n", j, j + 1, j + 2, -1);
		}
		fprintf(file, "\t\t</layer-polygon-corner-uint32> \n");

		fprintf(file, "\t\t<layer-polygon-face-uint32 name=\"material\">\n");

		for(j = 0; j < seduce_3d_object_size[i]; j += 3)
		{
			if(seduce_3d_object_array[i][j * 7 + 6] > 0.5)
				fprintf(file, "\t\t\t<p>1</p>\n");
			else
				fprintf(file, "\t\t\t<p>0</p>\n");
		}
		fprintf(file, "\t\t</layer-polygon-face-uint32>\n");
		fprintf(file, "\t</layers>\n");
		fprintf(file, "\t<vertexcrease layer=\"\" default=\"0\"/>\n");
		fprintf(file, "\t<edgecrease layer=\"crease\" default=\"0\"/>\n");
		fprintf(file, "</node-geometry>\n\n");

	}

	fprintf(file, "</vml>\n\n");

	fclose(file);
	exit(0);
}

