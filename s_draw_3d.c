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
uint seduce_object_3d_location_light_pos[6];
uint seduce_object_3d_texture_shade;
uint seduce_object_3d_texture_reflection;

char *seduce_object_3d_shader_vertex = 
"attribute vec4 vertex;\n"
"attribute vec4 normal;\n"
"varying vec3 v;\n"
"varying vec3 n;\n"
"varying vec3 n2;\n"
"varying vec3 n3;\n"
"varying vec3 c;\n"
"varying vec3 surface_pos;"
"varying float fernel;\n"
"varying float limit;\n"
"varying vec4 temp_color;\n"
"uniform float fade;"
"uniform vec3 fade_a;\n"
"uniform vec3 fade_b;\n"
"uniform mat4 ModelViewMatrix;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"void main()\n"
"{\n"
"	c = mix(fade_a, fade_b, normal.aaa);\n"
"	n3 = (NormalMatrix * vec4(normalize(normal.xyz), 0.0)).xyz;\n"
"	fernel = (1.0 - n3.z) * 2.0;\n"
"	n = n3 * vec3(0.5) + vec3(0.5);\n"
"	n2 = normalize(normal.xyz) * vec3(0.5) + vec3(0.5);\n"
"	surface_pos = vec3((ModelViewMatrix * vec4(vertex.xyz, 1)).xy, 0.1);\n"
"	limit = normal.a * -0.3 + fade * 2.0;\n"
"	v = vertex.xyz;\n"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);\n"
"	surface_pos = gl_Position.xyz / gl_Position.zzz;\n"
"}\n";

char *seduce_object_3d_shader_fragment = 
"varying vec3 v;"
"varying vec3 n;"
"varying vec3 n2;"
"varying vec3 n3;"
"varying vec3 c;"
"varying float fernel;\n"
"varying float limit;\n"
"uniform sampler2D light;"
"uniform sampler2D reflection;"
"uniform vec3 reflect_color;"
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
"	float f, gray, alpha;"
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
"	alpha = (limit - length(v.xyz)) * 32.0;\n"
"	gl_FragColor = vec4(c * vec3(1.0 + f * 0.02) + vec3(fernel) * vec3(0.2, 0.2, 0.2), alpha);"
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
	uint vertex_format_size[2] = {3, 4}, i, j, count;
	count = 0;
	if(seduce_object_3d_pool != NULL)
		r_array_free(seduce_object_3d_pool);
	for(i = 0; i < SEDUCE_OBJECT_COUNT; i++)
		count += seduce_3d_object_size[i];
	seduce_object_3d_pool = r_array_allocate(count, vertex_format_types, vertex_format_size, 2, 0);
	for(i = 0; i < SEDUCE_OBJECT_COUNT; i++)
	{
		for(j = 0; j < seduce_3d_object_size[i]; j++)
			if(seduce_3d_object_array[i][j * 7 + 3] == 0 && seduce_3d_object_array[i][j * 7 + 4] == 0 && seduce_3d_object_array[i][j * 7 + 5] == 0 )
				seduce_3d_object_array[i][j * 7 + 5] = 1.0;
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
	seduce_object_3d_location_light_pos[0] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos0");
	seduce_object_3d_location_light_pos[1] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos1");
	seduce_object_3d_location_light_pos[2] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos2");
	seduce_object_3d_location_light_pos[3] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos3");
	seduce_object_3d_location_light_pos[4] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos4");
	seduce_object_3d_location_light_pos[5] = r_shader_uniform_location(seduce_object_3d_shader, "light_pos5");


	seduce_object_3d_color(0.5, 0.5, 0.5, 
							0.05, 0.05, 0.05,
							1.0, 1.0, 1.0);
//	sui_3d_export();
}


void seduce_object_3d_lightning(BInputState *input)
{
	static float *brightness_pos = NULL;
	uint i, max_count;
	float f;
	max_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(max_count < 6)
		max_count = 6;

	if(brightness_pos == NULL)
	{

		brightness_pos = malloc((sizeof *brightness_pos) * 3 * max_count);
		for(i = 0; i < max_count * 3; i++)
			brightness_pos[i] = 0;
	}

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
	for(; i < max_count; i++)
		brightness_pos[i * 3 + 2] = brightness_pos[i * 3 + 2] * (1.0 - input->delta_time);
	for(i = 0; i < 6; i++)
		r_shader_vec4_set(seduce_object_3d_shader, seduce_object_3d_location_light_pos[i], brightness_pos[i * 3], brightness_pos[i * 3 + 1], -0.75, brightness_pos[i * 3 + 2]);
}

void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float size, uint id, float time, float *color)
{
	uint location[6];
	float f;
	RMatrix *matrix;
	if(input->mode != BAM_DRAW)
		return;
	r_shader_set(seduce_object_3d_shader);
	matrix = r_matrix_get();
	f = 0.4 - time * 0.3;
	r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_fade_a, f, f, f);
	if(color == NULL)
	{
		f = 0.7 - time * 0.4;
		r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_fade_b, f, f, f);
	}else
		r_shader_vec3_set(seduce_object_3d_shader, seduce_object_3d_location_fade_b, color[0], color[1], color[2]);
	r_matrix_push(matrix);
	r_matrix_translate(matrix, pos_x, pos_y, pos_z);
	r_matrix_scale(matrix, size, size, size);
//	r_shader_state_set_cull_face(seduce_object_3d_shader, GL_BACK);
//	r_shader_state_set_offset(seduce_object_3d_shader, input->pointers[0].pointer_x * 10,  input->pointers[0].pointer_y * 1.6); 
	r_shader_float_set(seduce_object_3d_shader, seduce_object_3d_location_fade, time * time);
	r_array_section_draw(seduce_object_3d_pool, seduce_object_3d_sections[id], GL_TRIANGLES, 0, -1);
//	r_array_section_draw(seduce_object_3d_pool, seduce_object_3d_sections[id], GL_LINES, 0, -1);
	r_matrix_pop(matrix);
}

uint seduce_object_3d_count()
{
	return SEDUCE_OBJECT_COUNT;
}

char *seduce_object_3d_object_name(uint object)
{
	return seduce_3d_object_names[object];
}

void seduce_object_3d_test(BInputState *input)
{
	float pos_x, pos_y, color[3] = {1, 1, 1.0};
	uint i;
	return;
	for(i = 21; i < SEDUCE_OBJECT_COUNT; i++)
	{
		pos_x = (i % 9) / 5.0 - 0.9;
		pos_y = (i / 9) / 5.0 - 0.7;
		seduce_object_3d_draw(input, pos_x, pos_y, 0, 0.06, i, 1, color);
	}
}


uint seduce_object_3d_object_lookup(char *name)
{
	uint i, j;
	for(i = 0; i < SEDUCE_OBJECT_COUNT; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == seduce_3d_object_names[i][j]; j++);
		if(name[j] == seduce_3d_object_names[i][j])
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

	for(i = 0; i < SEDUCE_OBJECT_COUNT; i++)
	{
		fprintf(file, "<node-geometry id=\"n%u\" name=\"%s\">\n", i, seduce_3d_object_names[i]);
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

//#define SEDUCE_HXA_IMPORT
#ifdef SEDUCE_HXA_IMPORT
#include "hxa_utils.h"

extern float *hxa_vertex_array_build(HXANode *node, char **names, unsigned char *component_ids, float *defaults, unsigned intcomponent_count);
extern float *hxa_unreferenced_vertex_array_build(HXANode *node, char **names, unsigned char *component_ids, float *defaults, unsigned intcomponent_count);
extern HXAFile *hxa_load(char *file_name, int silent);
extern void hxa_util_triangulate_node(HXANode *node, unsigned int max_sides);
extern int hxa_util_validate(HXAFile *file, int silent);
unsigned int *hxa_neighbour_node(HXANode *node);

void *hxa_util_array_extract(HXANode *node, size_t vertex_stride, size_t *vertex_param_offsets, unsigned int *vertex_param_types, char **vertex_param_names, hxa_uint8 *vertex_component, void ** defaults, unsigned int param_count);


void seduce_print_hxa_node(FILE *file, HXANode *node, char *name)
{
	char *names[7] = {HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_SOFT_LAYER_NORMALS, HXA_CONVENTION_SOFT_LAYER_NORMALS, HXA_CONVENTION_SOFT_LAYER_NORMALS, "material"};
	unsigned char component_ids[7] = {0, 1, 2, 0, 1, 2, 0};
	float *array, defaults[7] = {0, 0, 0, 0, 0, 1, 0};
	size_t vertex_param_offsets[7] = {0, sizeof(float), sizeof(float) * 2, sizeof(float) * 3, sizeof(float) * 4, sizeof(float) * 5, sizeof(float) * 6};
	unsigned int vertex_param_types[7] = {HXA_TC_FLOAT32, HXA_TC_FLOAT32, HXA_TC_FLOAT32, HXA_TC_FLOAT32, HXA_TC_FLOAT32, HXA_TC_FLOAT32, HXA_TC_FLOAT32};
	uint i, count;
f_debug_mem_check_bounds();
//	hxa_util_normal_corner(node);
f_debug_mem_check_bounds();
//	hxa_neighbour_node(node);
f_debug_mem_check_bounds();
	hxa_util_triangulate_node(node, 3);
f_debug_mem_check_bounds();
f_debug_mem_check_bounds();
	count = node->content.geometry.edge_corner_count * 7;
	array = hxa_util_array_extract(node, sizeof(float) * 7, vertex_param_offsets, vertex_param_types, names, component_ids, defaults, 7);


//	array = hxa_unreferenced_vertex_array_build(node, names, component_ids, NULL, 7);
	fprintf(file, "float seduce_array_%s[%u * 7] = {%ff", name, node->content.geometry.edge_corner_count, array[0]);
/*	for(i = 0; i < count; i += 7)
		if(array[i + 6] > 1.0)
			array[i + 6] = 1.0;*/
	for(i = 1; i < count; i++)
	{
		if(i % (7 * 8) == 0)
			fprintf(file, "\n\t\t\t\t");
		fprintf(file, ", %ff", array[i]);
	}
	fprintf(file, "};\n");
}

/*
void seduce_print_hxa_node(FILE *file, HXANode *node, char *name)
{
	char *names[7] = {HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_SOFT_LAYER_NORMALS, HXA_CONVENTION_SOFT_LAYER_NORMALS, HXA_CONVENTION_SOFT_LAYER_NORMALS, "material"};
	unsigned char component_ids[7] = {0, 1, 2, 0, 1, 2, 0};
	float *array, defaults[7] = {0, 0, 0, 0, 0, 1, 0};
	uint i, count;
	hxa_neighbour_node(node);
	hxa_util_triangulate_node(node, 3);
	count = node->content.geometry.edge_corner_count * 7;
	array = hxa_unreferenced_vertex_array_build(node, names, component_ids, NULL, 7);
	fprintf(file, "float seduce_array_%s[%u * 7] = {%ff", name, node->content.geometry.edge_corner_count, array[0]);
	for(i = 0; i < count; i += 7)
		if(array[i + 6] > 1.0)
			array[i + 6] = 1.0;
	for(i = 1; i < count; i++)
	{
		if(i % (7 * 8) == 0)
			fprintf(file, "\n\t\t\t\t");
		fprintf(file, ", %ff", array[i]);
	}
	fprintf(file, "};\n");
}*/

void seduce_print_hxa(char *file_name)
{
	FILE *save;
	HXAFile *file;
	uint8 *data;
	size_t size;
	char *name = HXA_CONVENTION_SOFT_NAME;
	uint i, j, k;
	data = f_text_load(file_name, &size);
	if(data == NULL)
		return;
	file = hxa_unserialize(data, size, FALSE);
//	file->node_array[0].content.geometry.corner_stack.layers = realloc(file->node_array[0].content.geometry.corner_stack.layers, (sizeof *file->node_array[0].content.geometry.corner_stack.layers) * file->node_array[0].content.geometry.corner_stack.layer_count);
//	hxa_util_normal_corner(file->node_array);
	free(data);
//	file = hxa_load(file_name, TRUE);
	if(file == NULL)
		return;
	hxa_print(file, TRUE);
	save = fopen("icon_output.c", "w");
	if(save == NULL)
		return;
	hxa_util_validate(file, FALSE);
	for(i = 0; i < file->node_count; i++)
	{
		if(file->node_array[i].type == HXA_NT_GEOMETRY)
		{
			
			for(j = 0; j < file->node_array[i].meta_data_count; j++)
			{
				for(k = 0; file->node_array[i].meta_data[j].name[k] == name[k] && name[k] != 0; k++);
				if(file->node_array[i].meta_data[j].name[k] == name[k])
				{
					seduce_print_hxa_node(save, &file->node_array[i], file->node_array[i].meta_data[j].value.text_value);
					break;
				}
			}
			if(j == file->node_array[i].meta_data_count)
				seduce_print_hxa_node(save, &file->node_array[i], "unnamed");
		}
	}
	fclose(save);
}
#endif