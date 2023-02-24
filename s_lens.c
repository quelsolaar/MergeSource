#include <math.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include "forge.h"
#include "seduce.h"


char *s_lens_shader_vertex = 
"attribute vec3 vertex;"
"uniform mat4 ModelViewMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform vec4 color_a;\n"
"uniform vec4 color_b;\n"
"uniform vec4 color_c;\n"
"varying vec3 pos;\n"
"varying vec2 uv;\n"
"varying float tilt;\n"
"varying vec4 c_a;\n"
"varying vec4 c_b;\n"
"varying vec4 c_c;\n"
"void main()"
"{"
"	float f;"
"	uv = vertex.xy * vec2(0.6);"
"	pos = vertex.xyz * vec3(0.6);"
"	gl_Position = (ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0));"
"	f = NormalMatrix[2][2] * NormalMatrix[2][2];"
"	tilt = f * f;"
"	c_a = color_a * vec4(0.1 * tilt);\n"
"	c_b = color_b * vec4(3.4);\n"
"	c_c = color_c * vec4(0.6);\n"
"}";


char *s_lens_shader_fragment = 
"uniform sampler2D image;"
"uniform mat4 ModelViewMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec3 pos;\n"
"varying vec2 uv;\n"
"varying float tilt;\n"
"varying vec4 c_a;\n"
"varying vec4 c_b;\n"
"varying vec4 c_c;\n"
"void main()\n"
"{\n"
"	vec4 c;"
"	vec4 add, n, tmp;"
"	vec2 space;"
"	float f, flare, layer1, layer2, layer3, layer4, layer5, layer6, warp;"
"	add = (ModelViewMatrix * vec4(pos, 1.0)) * NormalMatrix;"
"	f = 1.0 - length(uv) * 2.0;\n"
"	add.xy /= add.zz;"
"	tmp =  uv.xyxy - vec4(0.4, 0.4, 0.5, 0.5) * add.xyxy;"
"	layer1 = length(tmp.xy);"
"	layer2 = length(tmp.ba);"
"	tmp =  uv.xyxy - vec4(0.25, 0.25, 0.1, 0.1) * add.xyxy;"
"	layer3 = length(tmp.xy);"
"	layer4 = length(tmp.ba);"
"	tmp =  uv.xyxy + vec4(0.15, 0.15, 0.25, 0.25) * add.xyxy;"
"	layer5 = length(tmp.xy);"
"	layer6 = length(tmp.ba);"
"	warp = length(uv + uv * vec2(layer2) + vec2(2.7) * add.xy);"
"	gl_FragColor += texture2D(image, vec2(layer3 * 0.1 + add.z * 1.0, 0.09 / add.z)) * vec4(layer1 * layer2 * 0.05);\n"
"	gl_FragColor += texture2D(image, vec2(layer1 * 0.6 + add.z * 3.0, 0.09 / add.z)) * vec4(layer1 * layer2 * 0.05);\n"
"	gl_FragColor += texture2D(image, vec2(layer2 * 0.3 - add.z, 0.08 / add.z)) * vec4(layer2 * warp * 0.05);\n"
"	add = normalize(add);"
"	gl_FragColor += texture2D(image, vec2(warp * 0.1 - add.z * 3.0, 0.015 / add.z)) * vec4(layer1 * warp * 0.003);\n"
"	flare = warp / (1.0 - 0.8 * length(uv + vec2(f) * add.xy));"
//"	gl_FragColor += vec4(flare * flare) * vec4(0.08, 0.04, 0.16, 0.0);" 
"	c = vec4(flare * flare) * c_a;"
"	gl_FragColor += c * c;"
"	flare = layer2 / (2.0 + 5.0 * length(uv + vec2(f * 4.3) * add.xy));"
//"	gl_FragColor += vec4(flare * flare) * vec4(1.8, 1.5, 3.4, 0.0);"
"	gl_FragColor += vec4(flare * flare) * c_b;"
"	flare = warp / (2.5 + 7.0 * length(uv + vec2(f * 0.5) * normalize(add.xy)));"
"	gl_FragColor += vec4(flare) * vec4(0.4, 0.5, 0.4, 0.0);"
//"	gl_FragColor += vec4(flare) * c_c;"
"	space = uv - vec2(1.5) * add.xy;"
"	gl_FragColor += vec4(1.0 / (1.0 + dot(space, space) * 6.0)) * c_c * vec4(tilt);"
"	add = vec4(0.0);"
"	if(layer1 < 0.3 * tilt)"
"		add += vec4(0.04, 0.06, 0.02, 0.0);"
"	if(layer2 < 0.35 - 0.2 * tilt)"
"		add += vec4(0.04, 0.02, 0.04, 0.0);"
"	if(layer3 < 0.4 + 0.2 * tilt)"
"		add += vec4(0.01, 0.03, 0.04, 0.0);"
"	if(layer4 < 0.15 - 0.2 * tilt)"
"		add += vec4(0.03, 0.01, 0.02, 0.0);"
"	if(layer5 < 0.2 + 0.2 * tilt)"
"		add += vec4(0.01, 0.03, 0.04, 0.0);"
"	if(layer6 < 0.4 - 0.2 * tilt)"
"		add += vec4(0.04, 0.02, 0.03, 0.0);"
"	add = add * vec4(4.0) + vec4(1.0);"
"	gl_FragColor *= add;"
"}";

extern uint seduce_line_texture_id;

RShader *seduce_lens_shader_get()
{
	static RShader *shader = NULL;
	char buffer[2048];
	if(shader != NULL)
		return shader;
	shader = r_shader_create_simple(buffer, 2048, s_lens_shader_vertex, s_lens_shader_fragment, "lens");
	r_shader_texture_set(shader, 0, seduce_line_texture_id);
	r_shader_state_set_blend_mode(shader, GL_ONE, GL_ZERO);
	return shader;
}

#define SEDUCE_LENS_CIRCLE_SPLITS 96

void seduce_lens_draw(float color_edge_red, float color_edge_green, float color_edge_blue, float color_depth_red, float color_depth_green, float color_depth_blue, float color_center_red, float color_center_green, float color_center_blue)
{
	RShader *shader;
	static void *pool = NULL;
	static uint location_a, location_b, location_c;
	shader = seduce_lens_shader_get();
	r_shader_set(shader);
	if(pool == NULL)
	{
		RFormats vertex_format_types = R_FLOAT;
		uint size = 2, i, j;
		float *array;
		array = malloc((sizeof *array) * 6 * SEDUCE_LENS_CIRCLE_SPLITS);
		for(i = j = 0; i < SEDUCE_LENS_CIRCLE_SPLITS;)
		{
			array[j++] = sin((double)i * PI * 2.0 / SEDUCE_LENS_CIRCLE_SPLITS) * 1.0;
			array[j++] = cos((double)i * PI * 2.0 / SEDUCE_LENS_CIRCLE_SPLITS) * 1.0;
			i++;
			array[j++] = 0;
			array[j++] = 0;
			array[j++] = sin((double)i * PI * 2.0 / SEDUCE_LENS_CIRCLE_SPLITS) * 1.0;
			array[j++] = cos((double)i * PI * 2.0 / SEDUCE_LENS_CIRCLE_SPLITS) * 1.0;
		}		
		pool = r_array_allocate(SEDUCE_LENS_CIRCLE_SPLITS * 3, &vertex_format_types, &size, 1, 0);
		r_array_load_vertex(pool, NULL, array, 0, SEDUCE_LENS_CIRCLE_SPLITS * 3);
		free(array);
		location_a = r_shader_uniform_location(shader, "color_a");
		location_b = r_shader_uniform_location(shader, "color_b");
		location_c = r_shader_uniform_location(shader, "color_c");
	}
	r_shader_vec4_set(shader, location_a, color_edge_red, color_edge_green, color_edge_blue, 0.0);
	r_shader_vec4_set(shader, location_b, color_depth_red, color_depth_green, color_depth_blue, 0.0);
	r_shader_vec4_set(shader, location_c, color_center_red, color_center_green, color_center_blue, 0.0);
	r_array_draw(pool, NULL, R_PRIMITIVE_TRIANGLES, 0, -1, NULL, NULL, 1);
}