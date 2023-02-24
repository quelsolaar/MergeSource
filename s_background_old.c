#include <math.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include "seduce.h"

#define SEDUCE_IMAGE_ANIMATION_SPLIT_SIZE 0.175

extern boolean seduce_angle_axis_detect(BInputState *input, uint user_id, float angle_a, float angle_b);
extern void seduce_object_3d_color(float col_a_r, float col_a_g, float col_a_b, float col_b_r, float col_b_g, float col_b_b, float reflect_r, float reflect_g, float reflect_b);

void *s_background_pool = NULL;
void *s_background_image = NULL;
void *s_background_negative = NULL;
uint s_background_used = 0;
float *s_background_buffer = NULL;

RShader *s_background_shader_negative = NULL;
RShader *s_background_shader_surface = NULL;
RShader *s_background_shader_transition = NULL;
RShader *s_background_shader_mask = NULL;
RShader *s_image_shader = NULL;
void *s_image_ring_shader = NULL;
void *s_background_color_wheel = NULL;

#define S_BACKGROUND_SPLITS 64
#define S_BACKGROUND_POLY_COUNT_MAX 1024
#define S_BACKGROUND_VERTEX_SIZE (3 + 3 + 4)
/*
typedef enum{
	S_PT_SPLAT_ONE,
	S_PT_SPLAT_TWO,
	S_PT_SPLAT_THREE,
	S_PT_SPLAT_FOUR,
	S_PT_LIGHT,
	S_PT_CLICK
}SPraticleType;

*/

char *s_background_shader_negative_vertex = 
"attribute vec3 vertex;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"void main()\n"
"{"
"	gl_Position = (ModelViewProjectionMatrix * vec4(vertex, 1.0));\n"
"}";

char *s_background_shader_negative_fragment = 
"void main()\n"
"{\n"
"	gl_FragColor = vec4(1.0);\n"
"}\n";



char *s_background_shader_surface_vertex = 
"attribute vec3 vertex;"
"attribute vec3 normal;"
"attribute vec4 surface_color;"
"uniform vec4 particle_a_color;"
"uniform vec4 particle_b_color;"
"uniform vec4 particle_c_color;"
"uniform vec4 particle_d_color;"
"varying vec4 sc;"
"varying vec4 pac;"
"varying vec4 pbc;"
"varying vec4 pcc;"
"varying vec4 pdc;"
"varying vec3 n;"
"varying vec3 surface_pos;"
"varying vec4 mapping;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 ModelViewMatrix;"
"uniform vec2 center;"
"uniform vec4 aspect_mult;"
"uniform vec4 aspect_add;"

"void main()"
"{"
"	sc = surface_color;"
"	pac = vec4(particle_a_color.rgb, sc.a);"
"	pbc = vec4(particle_b_color.rgb, sc.a);"
"	pcc = vec4(particle_c_color.rgb, sc.a);"
"	pdc = vec4(particle_d_color.rgb, sc.a);"
"	n = vec3(-normal.xy, 2.0);"
"	surface_pos = (ModelViewMatrix * vec4(vertex.xy - normal.xy, 0, 1)).xyz;"
"	gl_Position = (ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0));"
"	mapping = ModelViewMatrix * vec4(vertex.xyz, 1.0);"
"	mapping.xy /= mapping.zz;"
"	mapping = mapping * aspect_mult + aspect_add;"
"}";

#define VORONOI_SPLITS 16

char *s_background_shader_surface_fragment = 
"#define VORONOI_SPLITS 16.0\n"
"varying vec3 n;\n"
"varying vec3 surface_pos;\n"
"varying vec4 sc;\n"
"varying vec4 pac;\n"
"varying vec4 pbc;\n"
"varying vec4 pcc;\n"
"varying vec4 pdc;\n"
"varying vec4 mapping;\n"
"uniform sampler2D image;\n"
"uniform sampler2D pattern;\n"
"uniform sampler2D voronoi;\n"
"uniform sampler2D light;\n"
"uniform float fade;\n"
"float atan2(in vec2 xy)\n"
"{\n"
"   if(abs(xy.y) > abs(xy.y))\n"
"		return 3.141592653 / 2.0 - atan(xy.y, xy.x);\n"
"	else"
"		return atan(xy.x, xy.y);\n"
"}\n"
"void main()\n"
"{\n"
"	vec2 uv, map;\n"
"	vec4 test, color, tex;\n"
"	float f, best;\n"
"	color = texture2D(image, mapping.xy).rgba;\n"
"	tex = texture2D(pattern, mapping.rg * vec2(64.0, 64.0));\n"
"	if(color.r + color.g + color.b + color.a > tex.a)\n"
"	{\n"
"		if(color.r + color.g > color.b + color.a)\n"
"		{\n"
"			if(color.r > color.g)\n"
"				gl_FragColor = pac;\n"
"			else\n"
"				gl_FragColor = pbc;\n"
"		}else\n"
"		{\n"
"			if(color.b > color.a)\n"
"				gl_FragColor = pcc;\n"
"			else\n"
"				gl_FragColor = pdc;\n"
"		}\n"
"	}else\n"
"		gl_FragColor = sc;\n"
"	gl_FragColor += vec4(texture2D(light, mapping.xy + n.xy).rgb, 0.0);\n"
"}";


char *s_background_shader_transition_vertex = 
"attribute vec3 vertex;"
"attribute vec3 normal;"
"attribute vec4 surface_color;"
"uniform vec4 particle_a_color;"
"uniform vec4 particle_b_color;"
"uniform vec4 particle_c_color;"
"uniform vec4 particle_d_color;"
"varying vec4 sc;"
"varying vec4 pac;"
"varying vec4 pbc;"
"varying vec4 pcc;"
"varying vec4 pdc;"
"varying vec3 n;"
"varying vec3 surface_pos;"
"varying vec4 mapping_center;"
"varying vec4 mapping_particles;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 ModelViewMatrix;"
"uniform vec2 center;"
"uniform vec4 aspect_mult;"
"uniform vec4 aspect_add;"
"void main()"
"{"
"	sc = surface_color;"
"	pac = vec4(particle_a_color.rgb, sc.a);"
"	pbc = vec4(particle_b_color.rgb, sc.a);"
"	pcc = vec4(particle_c_color.rgb, sc.a);"
"	pdc = vec4(particle_d_color.rgb, sc.a);"
"	n = vec3(-normal.xy, 2.0);"
"	surface_pos = (ModelViewMatrix * vec4(vertex.xy - normal.xy, 0, 1)).xyz;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);"
"	mapping_particles = ModelViewMatrix * vec4(vertex.xyz, 1.0);"
"	mapping_center = (mapping_particles - vec4(center.xy, 0.0, 0.0)) * aspect_mult + aspect_add;"
"	mapping_particles.xy /= mapping_particles.zz;"
"	mapping_particles = mapping_particles * aspect_mult + aspect_add;"
"}";

char *s_background_shader_transition_fragment = 
"#define VORONOI_SPLITS 16.0\n"
"varying vec3 n;"
"varying vec3 surface_pos;"
"varying vec4 sc;"
"varying vec4 pac;"
"varying vec4 pbc;"
"varying vec4 pcc;"
"varying vec4 pdc;"
"varying vec4 mapping_center;"
"varying vec4 mapping_particles;"
"uniform sampler2D image;"
"uniform sampler2D pattern;"
"uniform sampler2D voronoi;"
"uniform sampler2D light;"
"uniform vec4 color_multiply;"
"uniform vec4 color_subtract;"
"uniform float fade;"

"float atan2(in vec2 xy)"
"{"
"   if(abs(xy.y) > abs(xy.y))"
"		return 3.141592653 / 2.0 - atan(xy.y, xy.x);"
"	else"
"		return atan(xy.x, xy.y);"
"}"



"void main()"
"{"
"	vec2 uv, map;"
"	vec4 test, color, tex;"
"	float f, best;"
"	map.x = 3.0 * (atan2(mapping_center.xy - vec2(0.5)) + 3.141592653) / (2.0 * 3.141592653);"
"	map.y = length(mapping_center.xy - vec2(0.5));"
"	map.y = sqrt(map.y);"
"	uv = mod(map.xy, 1.0 / VORONOI_SPLITS) * VORONOI_SPLITS;"
"	color = texture2D(voronoi, map.xy).rgba;"
"	best = length(uv - color.rg);"

"	test = texture2D(voronoi, map.xy + vec2(1.0 / VORONOI_SPLITS, 0.0)).rgba;"
"	f = length(uv - (test.rg + vec2(1.0, 0.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(-1.0 / VORONOI_SPLITS, 0.0)).rgba;"
"	f = length(uv - (test.rg + vec2(-1.0, 0.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(0.0, 1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(0.0, 1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(0.0, -1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(0.0, -1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(1.0 / VORONOI_SPLITS, 1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(1.0, 1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(1.0 / VORONOI_SPLITS, -1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(1.0, -1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"

"	test = texture2D(voronoi, map.xy + vec2(-1.0 / VORONOI_SPLITS, -1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(-1.0, -1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(-1.0 / VORONOI_SPLITS, 1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(-1.0, 1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	f = 5.0 * fade + 5.0 * map.y + best * 0.2 - color.r;"
"	if(f > 0)"
"		discard;"
"	map = (color.ba - vec2(0.5)) * vec2((1.0 + fade) * 0.25);"
"	color = texture2D(image, mapping_particles.xy).rgba;"
"	tex = texture2D(pattern, mapping_particles.rg * vec2(64.0, 64.0));"
"	if(color.r + color.g + color.b + color.a > tex.a)"
"	{"
"		if(color.r + color.g > color.b + color.a)"
"		{"
"			if(color.r > color.g)"
"				gl_FragColor = pac;"
"			else"
"				gl_FragColor = pbc;"
"		}else"
"		{"
"			if(color.b > color.a)"
"				gl_FragColor = pcc;"
"			else"
"				gl_FragColor = pdc;"
"		}"
"	}else"
"		gl_FragColor = sc;"
"	gl_FragColor += vec4(texture2D(light, mapping_particles.xy + map + n.xy).rgb, 0.0) * color_multiply - color_subtract;"
"}";


char *s_background_shader_mask_vertex = 
"attribute vec3 vertex;"
"attribute vec3 normal;"
"attribute vec4 surface_color;"
"varying vec3 n;"
"varying vec3 surface_pos;"
"varying vec4 mapping;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 ModelViewMatrix;"
"uniform vec2 center;"
"uniform vec4 aspect_mult;"
"uniform vec4 aspect_add;"
"void main()"
"{"
"	n = vec3(-normal.xy, 2.0);"
"	surface_pos = (ModelViewMatrix * vec4(vertex.xy - normal.xy, 0, 1)).xyz;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);"
//"	mapping = (ModelViewProjectionMatrix * vec4(vertex.xy, 0.0, 1.0)) * vec4(0.5) + vec4(0.5) - vec4(center.xy, 0.0, 0.0);"
"	mapping = (ModelViewMatrix * vec4(vertex.xyz, 1.0) - vec4(center.xy, 0.0, 0.0)) * aspect_mult + aspect_add;"
"}";

char *s_background_shader_mask_fragment = 
"#define VORONOI_SPLITS 16.0\n"
"varying vec3 n;"
"varying vec3 surface_pos;"
"varying vec4 mapping;"
"uniform sampler2D voronoi;"
"uniform float fade;"

"float atan2(in vec2 xy)"
"{"
"   if(abs(xy.y) > abs(xy.y))"
"		return 3.141592653 / 2.0 - atan(xy.y, xy.x);"
"	else"
"		return atan(xy.x, xy.y);"
"}"

"void main()"
"{"
"	vec2 uv, map;"
"	vec4 test, color, tex;"
"	float f, best;"
"	map.x = 3.0 * (atan2(mapping.xy - vec2(0.5)) + 3.141592653) / (2.0 * 3.141592653);"
"	map.y = length(mapping.xy - vec2(0.5));"
"	map.y = sqrt(map.y);"
"	uv = mod(map.xy, 1.0 / VORONOI_SPLITS) * VORONOI_SPLITS;"
"	color = texture2D(voronoi, map.xy).rgba;"
"	best = length(uv - color.rg);"

"	test = texture2D(voronoi, map.xy + vec2(1.0 / VORONOI_SPLITS, 0.0)).rgba;"
"	f = length(uv - (test.rg + vec2(1.0, 0.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(-1.0 / VORONOI_SPLITS, 0.0)).rgba;"
"	f = length(uv - (test.rg + vec2(-1.0, 0.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(0.0, 1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(0.0, 1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(0.0, -1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(0.0, -1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(1.0 / VORONOI_SPLITS, 1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(1.0, 1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(1.0 / VORONOI_SPLITS, -1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(1.0, -1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(-1.0 / VORONOI_SPLITS, -1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(-1.0, -1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	test = texture2D(voronoi, map.xy + vec2(-1.0 / VORONOI_SPLITS, 1.0 / VORONOI_SPLITS)).rgba;"
"	f = length(uv - (test.rg + vec2(-1.0, 1.0)));"
"	if(f < best)"
"	{"
"		best = f;"
"		color = test;"
"	}"
"	f = 5.0 * fade + 5.0 * map.y + best * 0.2;"
"	if(f > color.r)"
"		discard;"
"	gl_FragColor = vec4(1.0);"
"}";




char *s_background_shader_particle_vertex = 
"attribute vec4 vertex;"
"attribute vec4 uv;"
"attribute vec4 color;"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec2 uv1;"
"varying vec2 uv2;"
"varying vec2 uv3;"
"varying vec2 uv4;"
"varying vec4 c;"
"void main()"
"{"
"	uv1 = vertex.ba;"
"	uv2 = uv.rg;"
"	uv3 = uv.ba;"
"	uv4 = mix(uv.rg, uv.ba, 0.5);"
"	c = color;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xy, -1.0, 1.0);"
"}";

char *s_background_shader_particle_fragment = 
"varying vec2 uv1;"
"varying vec2 uv2;"
"varying vec2 uv3;"
"varying vec2 uv4;"
"varying vec4 c;"
"uniform sampler2D noise_texture;"
"uniform sampler2D mask_texture;"
"void main()"
"{"
"	vec4 color;"
"	color = texture2D(mask_texture, uv1);"
"	gl_FragColor = c * color/* * texture2D(noise_texture, uv2) * texture2D(noise_texture, uv3) * vec4(8.0)*/;"
"}";



char *s_image_shader_vertex = 
"attribute vec3 vertex;"
"attribute vec2 uv;"
"varying vec2 mapping;"
"varying vec4 screen_pos;"
"varying vec4 surface_pos;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform mat4 ModelViewMatrix;"
"void main()"
"{"
"	mapping = uv;"
"	surface_pos = ModelViewMatrix * vec4(vertex.xy, 0, 1);"
"	screen_pos = surface_pos * vec4(0.5) + vec4(0.5);"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xyz, 1.0);"
"}";

char *s_image_shader_fragment = 
"varying vec2 mapping;"
"varying vec4 screen_pos;"
"varying vec4 surface_pos;"
"uniform sampler2D image;"
"uniform sampler2D particles;"
"uniform sampler2D pattern;"
"uniform vec2 center;"
"uniform float time;"
"void main()"
"{"
"	vec4 color, tex, pixels;"
"	color = texture2D(particles, screen_pos.xy).rgba;"
"	tex = texture2D(pattern, screen_pos.rg * vec2(64.0, 64.0));"
"	pixels = texture2D(image, mapping.xy).rgba;"
"	if(time - tex.r * 0.1 < length(surface_pos.xy - center) - color.b * time)"
"		gl_FragColor = vec4(vec3((pixels.x + pixels.y + pixels.z) * 0.333333), pixels.a * time);"
"	else"
"		gl_FragColor = pixels + pixels * pixels * color.aaaa;"
"	gl_FragColor = pixels;"
"}";


#define S_BACKGROUND_PARTICLE_COUNT 512
#define S_BACKGROUND_SPLAT_COLOR_COUNT 4

typedef struct{
	SPraticleType type;
	float pos[2];
	float vector[2];
	float age;
}SBackgroundParticle;

struct{
	uint particle_texture;
	uint light_texture;
	void *particle_fbo;
	void *light_fbo;
	uint sprite_texture;
	uint click_texture;
	uint pattern_texture;
	uint voronoi_texture;
	uint perlin_texture;
	RShader *shader;
	RShader *particle_shader;
	SBackgroundParticle particles[S_BACKGROUND_PARTICLE_COUNT];
	uint next_particle;
	uint active_color;
	float color[S_BACKGROUND_SPLAT_COLOR_COUNT][4];
	uint color_count[S_BACKGROUND_SPLAT_COLOR_COUNT];
	void *color_id[S_BACKGROUND_SPLAT_COLOR_COUNT];
	uint rate;
	void *particle_pool;
	float *particle_array;
}SBackgroundRender;

uint particle_debug_texture_id = 0;


void seduce_background_particle_init()
{
	RFormats vertex_format_types[3] = {R_FLOAT, R_FLOAT, R_FLOAT};
	uint vertex_format_size[3] = {4, 4, 4};
	static boolean init = FALSE;
	uint8 *array;
	uint i, j;
	float x, y, f, f2;
	if(init)
		return;
	init = TRUE;

	SBackgroundRender.particle_texture = r_texture_allocate(R_IF_RGBA_FLOAT16, 256, 256, 1, TRUE, FALSE, NULL);
	particle_debug_texture_id = SBackgroundRender.particle_texture;
	SBackgroundRender.light_texture = r_texture_allocate(R_IF_RGB_FLOAT16, 128, 128, 1, TRUE, FALSE, NULL);
	particle_debug_texture_id = SBackgroundRender.light_texture;
	
	SBackgroundRender.particle_fbo = r_framebuffer_allocate(&SBackgroundRender.particle_texture, 1, -1, RELINQUISH_TARGET_2D_TEXTURE);
	SBackgroundRender.light_fbo = r_framebuffer_allocate(&SBackgroundRender.light_texture, 1, -1, RELINQUISH_TARGET_2D_TEXTURE);
	array = malloc((sizeof *array) * 256 * 256 * 4);
	for(i = 0; i < 256 * 256; i++)
	{
		if(i % 256 == 0 || i % 256 == 255 || i / 256 == 0 || i / 256 == 255)
			array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
		else
		{
			x = (float)(i % 256) / 64 - 2.0;
			y = (float)(i / 256) / 64 - 2.0;
			f = 1.0 / (1 + x * x + y * y);
			f = 255.0 * (f - 1.0 / 5.0) / (4.0 / 5.0);
			if(f > 0)
				array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = (uint8)f;
			else
				array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
		}
	}

	SBackgroundRender.sprite_texture = r_texture_allocate(R_IF_RGBA_UINT8, 256, 256, 1, TRUE, FALSE, array);
	
	for(i = 0; i < 256; i++)
	{
		for(j = 0; j < 256; j++)
		{
			f = f_noisert2f((float)i / 64, (float)j / 64, 4, 4) * 0.6;
			if(f < 0.0)
				f = 0;
			else
			{
				if(f > 0.9999)
					f = 255;
				else 
					f = f * 255;
			}
			array[(i * 256 + j) * 4 + 0] = 
			array[(i * 256 + j) * 4 + 1] = 
			array[(i * 256 + j) * 4 + 2] = 
			array[(i * 256 + j) * 4 + 3] = f;
		//	array[(i * 256 + j) * 4 + 3] = 128 * (f_noiset2f((float)i / 32, (float)j / 32, 8) * 0.5 + 0.5);
		}	
	}
	
	SBackgroundRender.perlin_texture = r_texture_allocate(R_IF_RGBA_UINT8, 256, 256, 1, TRUE, TRUE, array);


	for(i = 0; i < 256 * 256; i++)
	{
		if(i % 256 == 0 || i % 256 == 255 || i / 256 == 0 || i / 256 == 255)
			array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
		else
		{
			x = (float)(i % 256) / 64 - 2.0;
			y = (float)(i / 256) / 64 - 2.0;
			f = sqrt(x * x + y * y);

			if(f > 0.5 && f < 0.98)
			{
				f = (f - 0.5) / 0.48;
				f = (cos(f * PI * 2) - 1) * -128.0;
				array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = f;
			}else
				array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
		}
	}

	SBackgroundRender.click_texture = r_texture_allocate(R_IF_RGBA_UINT8, 256, 256, 1, TRUE, FALSE, array);


	for(i = 0; i < 2 * 2; i++)
	{
		if(((i % 2) + (i / 2)) % 2 == 0)
			array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 255;
		else
			array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
	}
	SBackgroundRender.pattern_texture = r_texture_allocate(R_IF_RGBA_UINT8, 2, 2, 1, TRUE, TRUE, array);

	for(i = 0; i < 256 * 256 * 4; i++)
		array[i] = f_randi(i) % 255;
	SBackgroundRender.voronoi_texture = r_texture_allocate(R_IF_RGBA_UINT8, VORONOI_SPLITS, VORONOI_SPLITS, 1, FALSE, TRUE, array);
	
	free(array);
	for(i = 0; i < S_BACKGROUND_PARTICLE_COUNT; i++)
	{
		SBackgroundRender.particles[i].type = 0;
		SBackgroundRender.particles[i].pos[0] = 0;
		SBackgroundRender.particles[i].pos[1] = 0;
		SBackgroundRender.particles[i].vector[0] = 0;
		SBackgroundRender.particles[i].vector[1] = 0;
		SBackgroundRender.particles[i].age = 1.1;
	}

	SBackgroundRender.particle_pool = r_array_allocate(S_BACKGROUND_PARTICLE_COUNT * 6, vertex_format_types, vertex_format_size, 3, 0);
	SBackgroundRender.particle_array = malloc((sizeof *SBackgroundRender.particle_array) * S_BACKGROUND_PARTICLE_COUNT * 6 * 12);
	{
		char buffer[2048];
		SBackgroundRender.particle_shader = r_shader_create_simple(buffer, 2048, s_background_shader_particle_vertex, s_background_shader_particle_fragment, "particle shader");
		r_shader_texture_set(SBackgroundRender.particle_shader, 0, SBackgroundRender.perlin_texture);
	}
	r_shader_texture_set(SBackgroundRender.particle_shader, 1, SBackgroundRender.sprite_texture);

}

SPraticleType seduce_background_particle_color_allocate(void *id, float red, float green, float blue)
{
	float lab[3], input_color[3], a, b, f;
	uint i, found;
	if(id != NULL)
	{
		for(i = 0; i < S_BACKGROUND_SPLAT_COLOR_COUNT; i++)
		{
			if(SBackgroundRender.color_id[i] == id)
			{
				SBackgroundRender.color[i][0] = red;
				SBackgroundRender.color[i][1] = green;
				SBackgroundRender.color[i][2] = blue;
				return i;
			}
		}	
	}
	f_rgb_to_lab(input_color, red, green, blue);
	for(i = 0; i < S_BACKGROUND_SPLAT_COLOR_COUNT; i++)
	{
		f_rgb_to_lab(lab, SBackgroundRender.color[i][0], SBackgroundRender.color[i][1], SBackgroundRender.color[i][2]);
		lab[0] -= input_color[0];
		lab[1] -= input_color[1];
		lab[2] -= input_color[2];
		f = lab[0] * lab[0] + lab[1] * lab[1] + lab[2] * lab[2];
		if(f <  0.01)
		{
			SBackgroundRender.color_id[i] = id;
			return i;
		}
	}

	found = 0;
	for(i = 1; i < S_BACKGROUND_SPLAT_COLOR_COUNT; i++)
		if(SBackgroundRender.color_count[i] < SBackgroundRender.color_count[found])
			found = i;

	SBackgroundRender.color[found][0] = red;
	SBackgroundRender.color[found][1] = green;
	SBackgroundRender.color[found][2] = blue;
	SBackgroundRender.color_id[found]= id;
	return found;
}

void seduce_background_particle_update(BInputState *input)
{
	static float particle_timer = 0;
	static uint seed = 0;
	float delta, f, vec[2], aspect;
	uint i, j;
	aspect = betray_screen_mode_get(NULL, NULL, NULL);
	particle_timer += input->delta_time;
	if(particle_timer > 0.0)
	{
		particle_timer = 0.0;
		for(i = 0; i < input->pointer_count; i++)
		{
			if(0.0001 < input->pointers[i].delta_pointer_x * input->pointers[i].delta_pointer_x + input->pointers[i].delta_pointer_y * input->pointers[i].delta_pointer_y)
			{
				seduce_background_particle_spawn(input, input->pointers[i].pointer_x + f_randnf(seed) * 0.1, input->pointers[i].pointer_y + f_randnf(seed + 1) * 0.1, 0, 0, 0, S_PT_LIGHT);
				seed++;
				particle_timer -= 0.015;
			}
		}
	}
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			seduce_background_particle_spawn(input, input->pointers[0].pointer_x, input->pointers[0].pointer_y, 0, 0, 0.0, S_PT_CLICK);
	}
	if(SBackgroundRender.rate == 0)
		delta = input->delta_time * 0.1;
	else
		delta = 2.0 * SBackgroundRender.rate / S_BACKGROUND_PARTICLE_COUNT;
	for(i = 0; i < S_BACKGROUND_PARTICLE_COUNT; i++)
	{
		if(SBackgroundRender.particles[i].age < 1.0)
		{
			if(SBackgroundRender.particles[i].type == S_PT_LIGHT)
				SBackgroundRender.particles[i].age += delta;
			else
				SBackgroundRender.particles[i].age += delta * 10.0;
			if(SBackgroundRender.particles[i].age >= 1.0)
				if(SBackgroundRender.particles[i].type < S_BACKGROUND_SPLAT_COLOR_COUNT)
					SBackgroundRender.color_count[SBackgroundRender.particles[i].type]--;
			SBackgroundRender.particles[i].pos[0] += SBackgroundRender.particles[i].vector[0] * input->delta_time;
			SBackgroundRender.particles[i].pos[1] += SBackgroundRender.particles[i].vector[1] * input->delta_time / aspect;
			SBackgroundRender.particles[i].vector[0] *= (1 -  input->delta_time * 1);
			SBackgroundRender.particles[i].vector[1] *= (1 -  input->delta_time * 1);
		}
	}
	for(i = 0; i < input->pointer_count; i++)
	{
		if(!input->pointers[i].button[0] && 0.002 * 0.002 < input->pointers[i].delta_pointer_x * input->pointers[i].delta_pointer_x + input->pointers[i].delta_pointer_y * input->pointers[i].delta_pointer_y)
		{
			for(j = 0; j < S_BACKGROUND_PARTICLE_COUNT; j++)
			{
		//	if(SBackgroundRender.particles[j].type < S_PT_LIGHT)
				{
					vec[0] = (SBackgroundRender.particles[j].pos[0] - input->pointers[i].pointer_x) * 10.0;
					vec[1] = (SBackgroundRender.particles[j].pos[1] - input->pointers[i].pointer_y) * 10.0;
					f = 1.0 / (1.0 + vec[0] * vec[0] + vec[1] * vec[1]);
					vec[0] = SBackgroundRender.particles[j].vector[0] * (1.0 - f) + f * input->pointers[i].delta_pointer_x / input->delta_time;
					vec[1] = SBackgroundRender.particles[j].vector[1] * (1.0 - f) + f * input->pointers[i].delta_pointer_y / input->delta_time;
					delta = input->delta_time * 4.0;
					SBackgroundRender.particles[j].vector[0] = SBackgroundRender.particles[j].vector[0] * (1.0 - delta) + delta * vec[0];
					SBackgroundRender.particles[j].vector[1] = SBackgroundRender.particles[j].vector[1] * (1.0 - delta) + delta * vec[1];
				}
			}
		}
	}
	SBackgroundRender.rate = 0;
}




void seduce_background_particle_spawn(BInputState *input, float pos_x, float pos_y, float vec_x, float vec_y, float start_age, uint type)
{
	static uint seed = 0;
	float f, pos[3];
	r_matrix_projection_screenf(r_matrix_get(), pos, pos_x, pos_y, 0);
	pos[0] = pos_x;
	pos[1] = pos_y;
	SBackgroundRender.next_particle = (SBackgroundRender.next_particle + 1) % S_BACKGROUND_PARTICLE_COUNT;
	if(SBackgroundRender.particles[SBackgroundRender.next_particle].age < 1.0)
		if(SBackgroundRender.particles[SBackgroundRender.next_particle].type < S_BACKGROUND_SPLAT_COLOR_COUNT)
			SBackgroundRender.color_count[SBackgroundRender.particles[SBackgroundRender.next_particle].type]--;
	SBackgroundRender.particles[SBackgroundRender.next_particle].type = type;
	if(type < S_BACKGROUND_SPLAT_COLOR_COUNT)
		SBackgroundRender.color_count[type]++;

	SBackgroundRender.particles[SBackgroundRender.next_particle].age = start_age;
	SBackgroundRender.particles[SBackgroundRender.next_particle].pos[0] = pos[0];
	SBackgroundRender.particles[SBackgroundRender.next_particle].pos[1] = pos[1]; 
/*	for(f = 2; f > 1.0; f = vec[0] * vec[0] + vec[1] * vec[1])
	{
		vec[0] = f_randnf(seed++);
		vec[1] = f_randnf(seed++);
	}*/

	SBackgroundRender.particles[SBackgroundRender.next_particle].vector[0] = vec_x; 
	SBackgroundRender.particles[SBackgroundRender.next_particle].vector[1] = vec_y; 
//	SBackgroundRender.rate++;
}


void seduce_background_particle_burst(BInputState *input, float pos_x, float pos_y, uint count, float speed, uint  type)
{
	static uint seed = 0;
	float f, vec[2], pos[3];
	uint i;
	r_matrix_projection_screenf(r_matrix_get(), pos, pos_x, pos_y, 0);

	for(i = 0; i < count; i++)
	{
		f = (0.5 + f_randf(seed++)) * speed * 1.2;
		vec[0] = sin(f_randnf(seed) * PI) * f;
		vec[1] = cos(f_randnf(seed++) * PI) * f;
		seduce_background_particle_spawn(input, pos[0] + f_randnf(seed) * 0.01, pos[1] + f_randnf(seed + 1) * 0.01, vec[0], vec[1], f_randf(seed + 2) * 0.2, type);
		seed++;
	}
	SBackgroundRender.rate = 0;
}

void seduce_background_particle_square(BInputState *input, float pos_x, float pos_y, float size_x, float size_y, uint count, uint type)
{
	static uint seed = 0;
	float vec[2], f, pos[3];
	uint i;
	for(i = 0; i < count; i++)
	{
		f = f_randf(seed++) * 0.01;
		vec[0] = sin(f_randnf(seed) * PI) * f;
		vec[1] = cos(f_randnf(seed++) * PI) * f;
		pos[0] = pos_x + f_randf(seed) * size_x; 
		pos[1] = pos_y + f_randf(seed + 1) * size_y;
		seduce_background_particle_spawn(input, pos[0], pos[1], vec[0], vec[1], 0.2, type);
		seed++;
	}
	SBackgroundRender.rate = 0;
}

void seduce_background_particle_set(float *array, float size, float pos_x, float pos_y, float scroll_a_u, float scroll_a_v, float scroll_b_u, float scroll_b_v, float aspect, float red, float green, float blue, float alpha)
{
	static float x[6] = {-1, 1, 1, -1, 1, -1};
	static float y[6] = {-1, -1, 1, -1, 1, 1};
	static float u[6] = {0, 1, 1, 0, 1, 0};
	static float v[6] = {0, 0, 1, 0, 1, 1};
	uint i;
	for(i = 0; i < 6; i++)
	{
		*array++ = pos_x + x[i] * size;
		*array++ = pos_y + y[i] * size / aspect;
		*array++ = u[i];
		*array++ = v[i];
		*array++ = u[i] * 0.5 + scroll_a_u * 0.5;
		*array++ = v[i] * 0.5 + scroll_a_v * 0.5;
		*array++ = u[i] * 0.25 + scroll_b_u * 0.25;
		*array++ = v[i] * 0.25 + scroll_b_v * 0.25;
		*array++ = red;
		*array++ = green;
		*array++ = blue;
		*array++ = alpha;
	}
}


void seduce_background_particle(BInputState *input)
{
	float splat_color[] = {0, 0, 0, 1, 0, 0, 0};
	seduce_background_particle_init();
	if(input->mode == BAM_MAIN)
		seduce_background_particle_update(input);

	if(input->mode == BAM_DRAW && input->draw_id == 0)
	{
		RMatrix matrix;
		float f, f2, aspect, rgb[3];
		uint i, j, x, y;
		aspect = betray_screen_mode_get(&x, &y, NULL);
		r_framebuffer_bind(SBackgroundRender.particle_fbo);
		r_matrix_set(&matrix);
		r_matrix_identity(&matrix);
		r_matrix_frustum(&matrix, -0.05, 0.05, -0.05, 0.05, 0.05, 10.0f);
		r_viewport(0, 0, 128, 128);
		glClearColor(1.0, 0.0, 0.0, 0.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		j = 0;
/*		for(i = 0; i < S_BACKGROUND_PARTICLE_COUNT; i++)
		{
			if(SBackgroundRender.particles[i].age < 1.0 && FALSE)
			{
				f = 0.1 * (1.0 - SBackgroundRender.particles[i].age);
			//	if(SBackgroundRender.particles[i].type < S_BACKGROUND_SPLAT_COLOR_COUNT)
				{
					seduce_background_particle_set(&SBackgroundRender.particle_array[j++ * 12 * 6], f, 
													SBackgroundRender.particles[i].pos[0], 
													SBackgroundRender.particles[i].pos[1], 
													SBackgroundRender.particles[i].pos[0] * 1.7, 
													SBackgroundRender.particles[i].pos[1] * 1.7, 
													SBackgroundRender.particles[i].pos[0] * -0.7, 
													SBackgroundRender.particles[i].pos[1] * -0.7,  aspect,
													splat_color[3 - SBackgroundRender.particles[i].type], 
													splat_color[4 - SBackgroundRender.particles[i].type], 
													splat_color[5 - SBackgroundRender.particles[i].type], 
													splat_color[6 - SBackgroundRender.particles[i].type]);
				}
			}
		}
*/
		if(j != 0)
		{
			r_shader_set(SBackgroundRender.particle_shader);
			r_array_load_vertex(SBackgroundRender.particle_pool, NULL, SBackgroundRender.particle_array, 0, j * 6);
		//	r_array_section_draw(SBackgroundRender.particle_pool, NULL, GL_POINTS, 0, j * 6);
			r_array_section_draw(SBackgroundRender.particle_pool, NULL, GL_TRIANGLES, 0, j * 6);
		}
		
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		r_framebuffer_bind(SBackgroundRender.light_fbo);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		f_hsv_to_rgb(rgb, input->minute_time, 0.4, 0.1);

		for(i = 0; i < S_BACKGROUND_PARTICLE_COUNT; i++)
		{
			if(SBackgroundRender.particles[i].age < 1.0)
			{
				switch(SBackgroundRender.particles[i].type)	
				{
					case S_PT_LIGHT :
					if(SBackgroundRender.particles[i].age < 0.025)
						f = SBackgroundRender.particles[i].age / 0.025;
					else
						f = 1 - (SBackgroundRender.particles[i].age - 0.025) / (1.0 - 0.025);
					f *= 0.25;
					r_primitive_image(SBackgroundRender.particles[i].pos[0] - 0.25, 
									(SBackgroundRender.particles[i].pos[1] - 0.25) / aspect, -1.0, 0.5, 0.5 / aspect, 0, 0, 1, 1, SBackgroundRender.sprite_texture, f_randf(i) * f * 0.5, f_randf(i + 1) * f, f_randf(i + 2) * f, f);
					break;
					case S_PT_CLICK :
					f = (1.0 - SBackgroundRender.particles[i].age);
					f *= f;
					f2 = 1.0 - f;
					f *= 0.1; 
					r_primitive_image(SBackgroundRender.particles[i].pos[0] - f2, 
									(SBackgroundRender.particles[i].pos[1] - f2) / aspect, -1.0, f2 * 2.0, f2 * 2.0 / aspect, 0, 0, 1, 1, SBackgroundRender.click_texture, f_randf(i) * f, f_randf(i + 1) * f, f_randf(i + 2) * f, f);
					break;
				}	
			}
		}
		r_framebuffer_bind(NULL);
		r_matrix_set(NULL);
		r_viewport(0, 0, x, y);
	}
}


//float seduce_background_shader_surface_color[4] = {0.3, 0.3, 0.3, 0.2};
/*
float seduce_background_shader_surface_color[4] = {0.1, 0.1, 0.1, 0.1};
float seduce_background_shader_light_color[4] = {0.4, 0.4, 0.3, 0};
float seduce_background_shader_shade_color[4] = {0.25, 0.5, 0.4, 0};
*/

float seduce_background_shader_surface_color[4] = {0.35, 0.35, 0.35, 0.15};
float seduce_background_shader_light_color[4] = {0.3, 0.4, 0.5, 0.45};
float seduce_background_shader_shade_color[4] = {0.1, 0.1, 0.1, 0.35};

void seduce_background_color(float surface_r, float surface_g, float surface_b, float surface_a, 
							 float light_r, float light_g, float light_b, float light_a, 
							 float shade_r, float shade_g, float shade_b, float shade_a)
{
	seduce_background_shader_surface_color[0] = surface_r;
	seduce_background_shader_surface_color[1] = surface_g;
	seduce_background_shader_surface_color[2] = surface_b;
	seduce_background_shader_surface_color[3] = surface_a;

	seduce_background_shader_light_color[0] = light_r;
	seduce_background_shader_light_color[1] = light_g;
	seduce_background_shader_light_color[2] = light_b;
	seduce_background_shader_light_color[3] = light_a;

	seduce_background_shader_shade_color[0] = shade_r;
	seduce_background_shader_shade_color[1] = shade_g;
	seduce_background_shader_shade_color[2] = shade_b;
	seduce_background_shader_shade_color[3] = shade_a;
	seduce_object_3d_color(0.5, 0.5, 0.5, 
							0.05, 0.05, 0.05,
							1.0, 1.0, 1.0);
}

void s_background_shader_set(BInputState *input, float time, float *center)
{
	char *light_pos_name[6] = {"light_pos0", "light_pos1", "light_pos2", "light_pos3", "light_pos4", "light_pos5"};
	static float brightness[6] = {0, 0, 0, 0, 0, 0};
	float aspect;
	RShader *shader;
	uint i, j;
	aspect = betray_screen_mode_get(NULL, NULL, NULL);
	if(time	< 0.999)
	{
		float f;
		shader = s_background_shader_transition;
		r_shader_set(shader);
		if(center == NULL)
			r_shader_vec2_set(shader, r_shader_uniform_location(shader, "center"), 0, 0);
		else
			r_shader_vec2_set(shader, r_shader_uniform_location(shader, "center"), center[0], center[1]/* * betray_screen_mode_get(NULL, NULL, NULL)*/);
		r_shader_float_set(shader, r_shader_uniform_location(shader, "time"), time * 0.7 + time * time);
		f = 1.0 - time;
		r_shader_float_set(shader, r_shader_uniform_location(shader, "linger"), 1.0 - f * f * f * f);
		f = 7.0 - 6.0 * time;
		r_shader_vec4_set(shader, r_shader_uniform_location(shader, "color_multiply"), f, f, f, 1);
		f = 0.6 * (1.0 - time);
		r_shader_vec4_set(shader, r_shader_uniform_location(shader, "color_subtract"), f, f, f, 0);
	}else
	{
		shader = s_background_shader_surface;
		r_shader_set(shader);

		r_shader_state_set_offset(shader, 0.0, 1000.0);
	}
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "light_color"), seduce_background_shader_light_color[0], seduce_background_shader_light_color[1], seduce_background_shader_light_color[2], seduce_background_shader_light_color[3]);
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "light_shade"), seduce_background_shader_shade_color[0], seduce_background_shader_shade_color[1], seduce_background_shader_shade_color[2], seduce_background_shader_shade_color[3]);
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "particle_a_color"), SBackgroundRender.color[0][0], SBackgroundRender.color[0][1], SBackgroundRender.color[0][2], SBackgroundRender.color[0][3]);
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "particle_b_color"), SBackgroundRender.color[1][0], SBackgroundRender.color[1][1], SBackgroundRender.color[1][2], SBackgroundRender.color[1][3]);
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "particle_c_color"), SBackgroundRender.color[2][0], SBackgroundRender.color[2][1], SBackgroundRender.color[2][2], SBackgroundRender.color[2][3]);
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "particle_d_color"), SBackgroundRender.color[3][0], SBackgroundRender.color[3][1], SBackgroundRender.color[3][2], SBackgroundRender.color[3][3]);
	r_shader_float_set(shader, r_shader_uniform_location(shader, "fade"), 0.0 - time);
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "aspect_mult"), -0.5, -0.5 / aspect, 0.5, 0.5);
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "aspect_add"), 0.5, 0.5, 0.5, 0.5);


	r_shader_texture_set(shader, 0, SBackgroundRender.particle_texture);
	r_shader_texture_set(shader, 1, SBackgroundRender.pattern_texture);
	r_shader_texture_set(shader, 2, SBackgroundRender.voronoi_texture);
	r_shader_texture_set(shader, 3, SBackgroundRender.light_texture);
	if(input->draw_id == 0) /* MOVE THIS */
	{
		for(i = 0; i < 6 && i < input->pointer_count; i++)
		{
			for(j = 0; j < B_POINTER_BUTTONS_COUNT && !input->pointers[i].button[j]; j++);
			if(j < B_POINTER_BUTTONS_COUNT)
				brightness[i] = 0.6 * input->delta_time * 5.0 + brightness[i] * (1.0 - input->delta_time * 5.0);
			else if(input->pointers[i].delta_pointer_x > 0.001 || 
					input->pointers[i].delta_pointer_x < -0.001 || 
					input->pointers[i].delta_pointer_y > 0.001 || 
					input->pointers[i].delta_pointer_y < -0.001)
				brightness[i] = 0.4 * input->delta_time + brightness[i] * (1.0 - input->delta_time);
			else
				brightness[i] = brightness[i] * (1.0 - input->delta_time * 0.1);
			r_shader_vec4_set(NULL, r_shader_uniform_location(shader, light_pos_name[i]), input->pointers[i].pointer_x, input->pointers[i].pointer_y, -0.75, brightness[i]);
		}
		for(; i < 6; i++)
			r_shader_vec4_set(NULL, r_shader_uniform_location(shader, light_pos_name[i]), 0, 0, -0.75, 0);
	}
}

void seduce_background_polygon_flush_internal(BInputState *input, void *pool, uint vertex_count, float time, float *center)
{
	if(input->mode == BAM_DRAW && time > 0.0)
	{
//		glDepthFunc(GL_ALWAYS);
//		glEnable(GL_BLEND);
		if(time > 0.999)
		{
			r_shader_set(s_background_shader_negative);
		}else
		{
			float f, aspect;
			r_shader_set(s_background_shader_mask);
			aspect = betray_screen_mode_get(NULL, NULL, NULL);
			if(center == NULL)
				r_shader_vec2_set(s_background_shader_mask, r_shader_uniform_location(s_background_shader_mask, "center"), 0, 0);
			else
				r_shader_vec2_set(s_background_shader_mask, r_shader_uniform_location(s_background_shader_mask, "center"), center[0], center[1]);

			r_shader_vec4_set(s_background_shader_mask, r_shader_uniform_location(s_background_shader_mask, "aspect_mult"), -0.5, -0.5 / aspect, 0.5, 0.5);
			r_shader_vec4_set(s_background_shader_mask, r_shader_uniform_location(s_background_shader_mask, "aspect_add"), 0.5, 0.5, 0.5, 0.5);
			f = 1.0 - time;
		//	r_shader_float_set(s_background_shader_mask, r_shader_uniform_location(s_background_shader_mask, "linger"), 1.0 - f * f * f * f);
		//	r_shader_float_set(s_background_shader_mask, r_shader_uniform_location(s_background_shader_mask, "time"), time * 0.7 + time * time);
			r_shader_float_set(s_background_shader_mask, r_shader_uniform_location(s_background_shader_mask, "fade"), 0.0 - time);

			r_shader_texture_set(s_background_shader_mask, 0, SBackgroundRender.voronoi_texture);
		}
		r_array_section_draw(pool, NULL, GL_TRIANGLES, 0, vertex_count);

	//	glBlendFunc();
		s_background_shader_set(input, time, center);

		r_array_section_draw(pool, NULL, GL_TRIANGLES, 0, vertex_count); 
///		glDepthFunc(GL_LEQUAL);
	}
}


void seduce_background_polygon_flush(BInputState *input, float *center, float time)
{
	float f, f2;
	if(input->mode == BAM_DRAW)
	{
		
		r_array_load_vertex(s_background_pool, NULL, s_background_buffer, 0, s_background_used * 3);
		seduce_background_polygon_flush_internal(input, s_background_pool, s_background_used * 3, time, center);

		
/*		f = 0.4 + time;
		f2 = 1.0 - time;
		f2 *= f2;
		if(center != NULL)
		{
			r_primitive_image(center[0] * 0.5 - 0.5 * f, center[1] * 0.5 - 0.5 * f, 0.0, 1 * f, 1 * f, 0, 0, 1, 1, SBackgroundRender.sprite_texture, 0.1 * f2, 0.3 * f2, 0.5 * f2, 0);
			r_primitive_image(center[0] * 0.5 - 0.2 * f, center[1] * 0.5 - 0.2 * f, 0.0, 0.4 * f, 0.4 * f, 0, 0, 1, 1, SBackgroundRender.sprite_texture, 1 * f2, 0.5 * f2, 0.4 * f2, 0);
		}*/
		s_background_used = 0;
	}
}


void seduce_background_init()
{
	char buffer[2048];
	float *array;
	uint size[3] = {3, 3, 4}, i;
	RFormats types[3] = {R_FLOAT, R_FLOAT, R_FLOAT};
	s_background_negative = r_array_allocate(6, types, size, 1, 0);
	s_background_pool = r_array_allocate(18 + 6 * S_BACKGROUND_SPLITS + 3 * S_BACKGROUND_POLY_COUNT_MAX, types, size, 3, 0);
	s_background_buffer = malloc((sizeof *s_background_buffer) * S_BACKGROUND_POLY_COUNT_MAX * 3 * S_BACKGROUND_VERTEX_SIZE);
	s_background_color_wheel = r_array_allocate(3 * S_BACKGROUND_SPLITS, types, size, 3, 0);
	size[1] = 2;
	s_background_image = r_array_allocate(6, types, size, 2, 0);
	size[1] = 3;
	array = malloc((sizeof *array) * S_BACKGROUND_SPLITS * 3 * S_BACKGROUND_VERTEX_SIZE);

	array[0] = -0.5;
	array[1] = -0.5;
	array[2] = 0;
	array[3] = 0;
	array[4] = 0;
	array[5] = 0.5;
	array[6] = -0.5;
	array[7] = 0;
	array[8] = 1;
	array[9] = 0;
	array[10] = 0.5;
	array[11] = 0.5;
	array[12] = 0;
	array[13] = 1;
	array[14] = 1;
	array[15] = -0.5;
	array[16] = -0.5;
	array[17] = 0;
	array[18] = 0;
	array[19] = 0;
	array[20] = 0.5;
	array[21] = 0.5;
	array[22] = 0;
	array[23] = 1;
	array[24] = 1;
	array[25] = -0.5;
	array[26] = 0.5;
	array[27] = 0;
	array[28] = 0;
	array[29] = 1;
	r_array_load_vertex(s_background_image, NULL, array, 0, 6);
	s_background_shader_surface = r_shader_create_simple(buffer, 2048, s_background_shader_surface_vertex, s_background_shader_surface_fragment, "background surface");
	r_shader_state_set_blend_mode(s_background_shader_surface, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(s_background_shader_surface == NULL)
		array[29] = 1;
	r_shader_state_set_depth_test(s_background_shader_surface, GL_ALWAYS);
	
	s_background_shader_transition = r_shader_create_simple(buffer, 2048, s_background_shader_transition_vertex, s_background_shader_transition_fragment, "background transition");
	r_shader_state_set_blend_mode(s_background_shader_transition, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if(s_background_shader_transition == NULL)
		array[29] = 1;
	r_shader_state_set_depth_test(s_background_shader_transition, GL_ALWAYS);
	s_background_shader_negative = r_shader_create_simple(buffer, 2048, s_background_shader_negative_vertex, s_background_shader_negative_fragment, "negative surface");
	r_shader_state_set_depth_test(s_background_shader_negative, GL_ALWAYS);
//	r_shader_state_set_blend_mode(s_background_shader_negative, GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	r_shader_state_set_blend_mode(s_background_shader_negative, GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
//	r_shader_state_set_blend_mode(s_background_shader_negative, GL_ONE, GL_ONE);
	s_background_shader_mask = r_shader_create_simple(buffer, 2048, s_background_shader_mask_vertex, s_background_shader_mask_fragment , "background mask");
	r_shader_state_set_blend_mode(s_background_shader_mask, GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
	if(s_background_shader_mask == NULL)
		array[29] = 1;
	r_shader_state_set_depth_test(s_background_shader_mask, GL_ALWAYS);
	s_image_shader = r_shader_create_simple(buffer, 2048, s_image_shader_vertex, s_image_shader_fragment, "background image");
	r_shader_state_set_depth_test(s_image_shader, GL_ALWAYS);
	for(i = 0; i < S_BACKGROUND_SPLITS; i++)
	{

		array[(i * 3 + 0) * S_BACKGROUND_VERTEX_SIZE + 0] = sin(((float)i / S_BACKGROUND_SPLITS) * PI * 2.0);
		array[(i * 3 + 0) * S_BACKGROUND_VERTEX_SIZE + 1] = cos(((float)i / S_BACKGROUND_SPLITS) * PI * 2.0);
		array[(i * 3 + 0) * S_BACKGROUND_VERTEX_SIZE + 2] = 0; 
		array[(i * 3 + 1) * S_BACKGROUND_VERTEX_SIZE + 0] = sin(((float)(i + 1) / S_BACKGROUND_SPLITS) * PI * 2.0);
		array[(i * 3 + 1) * S_BACKGROUND_VERTEX_SIZE + 1] = cos(((float)(i + 1) / S_BACKGROUND_SPLITS) * PI * 2.0);
		array[(i * 3 + 1) * S_BACKGROUND_VERTEX_SIZE + 2] = 0; 
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 0] = 0;
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 1] = 0;
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 2] = 0; 

		array[(i * 3 + 0) * S_BACKGROUND_VERTEX_SIZE + 3] = 0;
		array[(i * 3 + 0) * S_BACKGROUND_VERTEX_SIZE + 4] = 0;
		array[(i * 3 + 0) * S_BACKGROUND_VERTEX_SIZE + 5] = 1; 
		array[(i * 3 + 1) * S_BACKGROUND_VERTEX_SIZE + 3] = 0;
		array[(i * 3 + 1) * S_BACKGROUND_VERTEX_SIZE + 4] = 0;
		array[(i * 3 + 1) * S_BACKGROUND_VERTEX_SIZE + 5] = 1; 
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 3] = 0;
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 4] = 0;
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 5] = 1; 
		
		f_hsv_to_rgb(&array[(i * 3 + 0) * S_BACKGROUND_VERTEX_SIZE + 6], (float)i / S_BACKGROUND_SPLITS, 1.0, 1.0);
		f_hsv_to_rgb(&array[(i * 3 + 1) * S_BACKGROUND_VERTEX_SIZE + 6], (float)(i + 1) / S_BACKGROUND_SPLITS, 1.0, 1.0);
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 6] = 1.0;
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 7] = 1.0;
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 8] = 1.0;

		array[(i * 3 + 0) * S_BACKGROUND_VERTEX_SIZE + 9] = 1.0;
		array[(i * 3 + 1) * S_BACKGROUND_VERTEX_SIZE + 9] = 1.0;
		array[(i * 3 + 2) * S_BACKGROUND_VERTEX_SIZE + 9] = 1.0;
	}
	r_array_load_vertex(s_background_color_wheel, NULL, array, 0, S_BACKGROUND_SPLITS * 3); 
	free(array);
	seduce_background_particle_init();
}

void seduce_background_color_wheel(BInputState *input)
{
	s_background_shader_set(input, 1.0, NULL);
	r_array_section_draw(s_background_color_wheel, NULL, GL_TRIANGLES, 0, S_BACKGROUND_SPLITS * 3);
}

void seduce_background_square_fill(float *array, float pos_x, float pos_y, float size_x, float size_y, float split, float tilt, float timer, float transparancy)
{
	float space = 0, a, b, c, d;

	if(timer < 0)
		timer = 0;
	if(timer > 1.0)
		timer = 1.0;

	a = timer * 5.0;

	if(a < 1.0)
	{
		pos_x += (1 - a * a) * split * size_x;
		size_x *= a * a;
	}

	a = tilt;
	b = 0;
	c = tilt;
	d = 0;
	

	if(timer < 0.999)
	{
		timer *= 6.0;
		switch((uint)timer)
		{
			case 0 :
				space = 0.05;
				a *= -1.0;
				b = 2.0 * tilt;
				c *= -1.0;
				d = 2.0 * tilt;
			break;
			case 1 :
				space = 0.05;
				a *= -1.0 + 2.0 * (timer - 1.0) * (timer - 1.0);
				b = 2.0 * tilt;
				c *= -1.0;
				d = 2.0 * tilt;
			break;
			case 2 :
				space = 0.05;
				b = 2.0 * tilt * (3.0 - timer) * (3.0 - timer);
				c *= -1.0;
				d = 2.0 * tilt;
			break;
			case 3 :
				space = 0.05;
				c *= -1.0 + 2.0 * (timer - 3.0) * (timer - 3.0);
				d = 2.0 * tilt;
			break;
			case 4 :
				space = 0.05;
				d = 2.0 * tilt * (5.0 - timer) * (5.0 - timer);
			break;
			case 5 :
				space = (1.0 - 1.0 * (timer - 5.0) * (timer - 5.0)) * 0.05;
			break;
		}
	}

	pos_x -= space;

	array[0] = pos_x;
	array[1] = pos_y;
	array[2] = 0;
	array[3] = 0.02;
	array[4] = 0;
	array[5] = transparancy;

	array[6] = pos_x + size_x * split;
	array[7] = pos_y - size_x * split * a;
	array[8] = 0.025;
	array[9] = 0.02;
	array[10] = 0;
	array[11] = transparancy;

	array[12] = pos_x + size_x * split;
	array[13] = pos_y + size_y + size_x * split * c;
	array[14] = 0.025;
	array[15] = -0.1;
	array[16] = 0;
	array[17] = transparancy;

	array[18] = pos_x;
	array[19] = pos_y + size_y;
	array[20] = 0;
	array[21] = 0.02;
	array[22] = 0;
	array[23] = transparancy;

	array[24] = pos_x + size_x * split;
	array[25] = pos_y + size_y + size_x * split * c;
	array[26] = 0.025;
	array[27] = 0.02;
	array[28] = 0;
	array[29] = transparancy;

	array[30] = pos_x;
	array[31] = pos_y;
	array[32] = 0;
	array[33] = 0.02;
	array[34] = 0;
	array[35] = transparancy;

	pos_x += space * 2.0;

	array[36] = pos_x + size_x * split;
	array[37] = pos_y + size_x * (1.0 - split) * b;
	array[38] = 0.025;
	array[39] = -0.02;
	array[40] = 0;
	array[41] = transparancy;

	array[42] = pos_x + size_x;
	array[43] = pos_y + size_y - size_x * (1.0 - split) * tilt;
	array[44] = 0;
	array[45] = -0.02;
	array[46] = 0;
	array[47] = transparancy;

	array[48] = pos_x + size_x * split;
	array[49] = pos_y + size_y - size_x * (1.0 - split) * d;
	array[50] = 0.025;
	array[51] = -0.02;
	array[52] = 0;
	array[53] = transparancy;

	array[54] = pos_x + size_x * split;
	array[55] = pos_y + size_x * (1.0 - split) * b;
	array[56] = 0.025;
	array[57] = -0.02;
	array[58] = 0;
	array[59] = transparancy;

	array[60] = pos_x + size_x;
	array[61] = pos_y + size_y - size_x * (1.0 - split) * tilt;
	array[62] = 0;
	array[63] = -0.02;
	array[64] = 0;
	array[65] = transparancy;

	array[66] = pos_x + size_x;
	array[67] = pos_y + size_x * (1.0 - split) * tilt;
	array[68] = 0;
	array[69] = -0.02;
	array[70] = 0;
	array[71] = transparancy;
}

boolean seduce_background_square_draw(BInputState *input, void *id, float pos_x, float pos_y, float size_x, float size_y, float split, float tilt, float timer)
{
	if(input->mode == BAM_DRAW && timer > 0.01)
	{
		float array[72];
		seduce_background_square_fill(array, pos_x, pos_y, size_x, size_y, split, tilt, timer, 0.5);
		if(id != NULL)
		{
			seduce_element_add_quad(input, id, 0, array, &array[6], &array[12], &array[18]);
			seduce_element_add_quad(input, id, 0, &array[36], &array[48], &array[42], &array[66]);
		}
	//	seduce_background_polygon_flush_internal(input, array, 3 * 4);
		return FALSE;
	}else if(input->mode == BAM_EVENT && id != NULL)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					return TRUE;
	}
	return FALSE;
}


void seduce_background_popup_fill(float *array, float pos_x, float pos_y, float size_x, float size_y, float timer, float pointer_x, float pointer_y, float transparancy)
{
	float space = 0, a, b, f, split = 0.5, tilt = 0.05, rot[6], gap, height;
	RShader	*shader;
	RMatrix	*matrix;

	pos_x *= PI * 2.0 / 360.0; 
	size_x *= PI * 2.0 / 360.0; 
	tilt *= size_x;
	

	if(timer < 0)
		timer = 0;
	if(timer > 1.0)
		timer = 1.0;

	a = timer * 3.0;

	if(a < 1.0)
	{
		pos_x += (1 - a * a) * split * size_x;
		size_x *= a * a;
	}

	a = 1;
	b = 0;

	gap = -0.005 * timer;

//	gap = 0.0;

	if(timer < 0.999)
	{
		timer *= 3.0;
		switch((uint)timer)
		{
			case 0 :
				a = 0;
				b = 0.1;
			break;
			case 1 :
				a = (timer - 1.0);
				b = 0.1;
			break;
			case 2 :
				a = 1;
				b = (3.0 - timer) * (3.0 - timer) * 0.1;
			break;
		}
	}
	height = 0.0;

	rot[0] = sin(pos_x + size_x * 0.5);
	rot[1] = cos(pos_x + size_x * 0.5);

	split += 0.2 * (rot[1] * pointer_x - rot[0] * pointer_y);
	rot[0] = sin(pos_x);
	rot[1] = cos(pos_x);
	rot[2] = sin(pos_x + size_x * split);
	rot[3] = cos(pos_x + size_x * split);
	rot[4] = sin(pos_x + size_x);
	rot[5] = cos(pos_x + size_x);
	pos_y *= 2.0 - b;

	f = pos_y;
	array[0] = rot[2] * b + rot[0] * f - rot[1] * gap;
	array[1] = rot[3] * b + rot[1] * f + rot[0] * gap;
	array[2] = 0;
	array[3] = -rot[3] * 0.1;
	array[4] = rot[2] * 0.1;
	array[5] = transparancy;

	f = pos_y - tilt * a * split;
	array[6] = rot[2] * b + rot[2] * f;
	array[7] = rot[3] * b + rot[3] * f;
	array[8] = height;
	array[9] = -rot[3] * 0.1;
	array[10] = rot[2] * 0.1;
	array[11] = transparancy;

	f = pos_y + size_y;
	array[12] = rot[2] * b + rot[0] * f - rot[1] * gap;
	array[13] = rot[3] * b + rot[1] * f + rot[0] * gap;
	array[14] = 0;
	array[15] = -rot[3] * 0.1;
	array[16] = rot[2] * 0.1;
	array[17] = transparancy;

	f = pos_y + size_y;
	array[18] = rot[2] * b + rot[0] * f - rot[1] * gap;
	array[19] = rot[3] * b + rot[1] * f + rot[0] * gap;
	array[20] = 0;
	array[21] = -rot[3] * 0.1;
	array[22] = rot[2] * 0.1;
	array[23] = transparancy;

	f = pos_y + size_y + tilt * a * split;
	array[24] = rot[2] * b + rot[2] * f;
	array[25] = rot[3] * b + rot[3] * f;
	array[26] = height;
	array[27] = -rot[3] * 0.1;
	array[28] = rot[2] * 0.1;
	array[29] = transparancy;

	f = pos_y - tilt * a * split;
	array[30] = rot[2] * b + rot[2] * f;
	array[31] = rot[3] * b + rot[3] * f;
	array[32] = height;
	array[33] = -rot[3] * 0.1;
	array[34] = rot[2] * 0.1;
	array[35] = transparancy;
/* ------------------------------------ */
	f = pos_y - tilt * a * (1.0 - split);
	array[36] = rot[2] * b + rot[2] * f;
	array[37] = rot[3] * b + rot[3] * f;
	array[38] = height;
	array[39] = rot[3] * 0.1;
	array[40] = -rot[2] * 0.1;
	array[41] = transparancy;

	f = pos_y + size_y;
	array[42] = rot[2] * b + rot[4] * f + rot[5] * gap;
	array[43] = rot[3] * b + rot[5] * f - rot[4] * gap;
	array[44] = 0;
	array[45] = rot[3] * 0.1;
	array[46] = -rot[2] * 0.1;
	array[47] = transparancy;

	f = pos_y + size_y + tilt * a * (1.0 - split);
	array[48] = rot[2] * b + rot[2] * f;
	array[49] = rot[3] * b + rot[3] * f;
	array[50] = height;
	array[51] = rot[3] * 0.1;
	array[52] = -rot[2] * 0.1;
	array[53] = transparancy;

	f = pos_y - tilt * a * (1.0 - split);
	array[54] = rot[2] * b + rot[2] * f;
	array[55] = rot[3] * b + rot[3] * f;
	array[56] = height;
	array[57] = rot[3] * 0.1;
	array[58] = -rot[2] * 0.1;
	array[59] = transparancy;

	f = pos_y + size_y;
	array[60] = rot[2] * b + rot[4] * f + rot[5] * gap;
	array[61] = rot[3] * b + rot[5] * f - rot[4] * gap;
	array[62] = 0;
	array[63] = rot[3] * 0.1;
	array[64] = -rot[2] * 0.1;
	array[65] = transparancy;

	f = pos_y/* + tilt * a * (1.0 - split)*/;
	array[66] = rot[2] * b + rot[4] * f + rot[5] * gap;
	array[67] = rot[3] * b + rot[5] * f - rot[4] * gap;
	array[68] = 0;
	array[69] = rot[3] * 0.1;
	array[70] = -rot[2] * 0.1;
	array[71] = transparancy;
}


void seduce_background_angle_set(BInputState *input, void *id, float angle_a, float angle_b, boolean active)
{
	static uint seed = 0;
	float *color, inactive_color[4] = {0.2, 0.2, 0.2, 0.7}, active_color[4] = {0.2, 0.6, 1.0, 0.8};
	float x[3], y[3], output[3], f;
	x[0] = sin(angle_a * PI * 2.0 / 360.0);
	y[0] = cos(angle_a * PI * 2.0 / 360.0);
	x[1] = sin((angle_a + angle_b) * PI / 360.0);
	y[1] = cos((angle_a + angle_b) * PI / 360.0);
	x[2] = sin(angle_b * PI * 2.0 / 360.0);
	y[2] = cos(angle_b * PI * 2.0 / 360.0);
	if(y[1] < 0.0)
		f = 0.3 + (1 + y[1]) * 0.2;
	else
		f = 0.3 + (1 - y[1]) * 0.2;
	if(active)
		color = active_color;
	else
		color = inactive_color;
	
	
	seduce_background_quad_draw(input, id, 0, x[0] * 0.08 + y[0] * 0.005, y[0] * 0.08 - x[0] * 0.005, 0, 
											x[0] * 0.2 + y[0] * 0.005, y[0] * 0.2 - x[0] * 0.005, 0, 
											x[1] * f, y[1] * f, 0, 
											x[1] * 0.08, y[1] * 0.08, 0, 
											y[0] * 0.1, -x[0] * 0.1, 1,
											color[0] - 0.2, color[1] - 0.2, color[2] - 0.2, color[3]);
	seduce_background_quad_draw(input, id, 0, x[1] * 0.08, y[1] * 0.08, 0, 
											x[1] * f, y[1] * f, 0, 
											x[2] * 0.2 - y[2] * 0.005, y[2] * 0.2 + x[2] * 0.005, 0, 
											x[2] * 0.08 - y[2] * 0.005, y[2] * 0.08 + x[2] * 0.005, 0, 
											-y[2] * 0.1, x[2] * 0.1, 1,
											color[0] - 0.2, color[1] - 0.2, color[2] - 0.2, color[3]);
/*	if(active)
	{
		angle_a = (angle_a - 20.0) + ((angle_b + 20.0) - (angle_a - 20.0)) * f_randf(seed++);
		f *=  0.2 + f_randf(seed++);
		x[0] = sin(angle_a * 2.0 * PI / 360.0) * f;
		y[0] = cos(angle_a * 2.0 * PI / 360.0) * f;
		r_matrix_projection_screenf(r_matrix_get(), output, 0, 0, 0);
		f = f_randf(seed++);
		if(f < 0.3)
			seduce_background_particle_spawn(input, output[0] + x[0], output[1] + y[0], (f_randnf(seed) + x[1]) * 0.2, (f_randnf(seed + 1) + y[1]) * 0.2, 0.0, S_PT_PRIMARY);
		else
			seduce_background_particle_spawn(input, output[0] + x[1] * 0.05, output[1] + y[1] * 0.05, x[1] * 0.2, y[1] * 0.2, 0.05, S_PT_PRIMARY);
	}*/
}

void seduce_background_betweeners_set(BInputState *input, float angle)
{
	float x, y, f;
	x = sin(angle * PI * 2.0 / 360.0);
	y = cos(angle * PI * 2.0 / 360.0);
	seduce_background_quad_draw(input, NULL, 0, x * 0.2 - y * 0.005, y * 0.2 + x * 0.005, 0, 
											x * 0.08 - y * 0.005, y * 0.08 + x * 0.005, 0,  
											x * 0.08 + y * 0.005, y * 0.08 - x * 0.005, 0, 
											x * 0.2 + y * 0.005, y * 0.2 - x * 0.005, 0, 
											-y * 0.1, x * 0.1, 1, 
											0.8, 0.8, 0.8, 0.7);
}


void seduce_background_circle_draw(BInputState *input, float pos_x, float pos_y, uint splits, float timer, uint selected)
{
	if(input->mode == BAM_DRAW)
	{
		float output[3];
		uint i, divide;

		if(splits > 2)
		{
			for(divide = splits - 1; divide != 1 && splits % divide != 0; divide--);
			if(divide == 1)
				divide = splits;
		}else	
			divide = 1;

		for(i = 0; i < splits; i++)
		{
			seduce_background_angle_set(input, NULL, 360.0 * ((float)i - 0.5) / (float)splits, 360.0 * ((float)i + 0.5) / (float)splits, i == selected);
			seduce_background_betweeners_set(input, 360.0 * ((float)i - 0.5) / (float)splits);
		}
		r_matrix_projection_screenf(r_matrix_get(), output, 0, 0, 0);
		seduce_background_polygon_flush(input, output, 0.1 + timer * 0.9);
	}
}



void seduce_background_angle(BInputState *input, void *id, uint part, float pos_x, float pos_y, float angle_a, float angle_b, float timer)
{
	uint i, j;
	if(input->mode == BAM_DRAW)
	{
		boolean active = FALSE;
		float x, y, length, box[12];
	//	float *array;
		RShader	*shader;
		uint i, divide;
	//	array = &s_background_buffer[s_background_used * 3 * 6];
	//	s_background_used += 4;
		if(angle_a > 359)
			angle_a -= 360;
		x = y = 0;
		for(i = 0; i < input->pointer_count; i++)
		{
			x += input->pointers[i].pointer_x - pos_x;
			y += input->pointers[i].pointer_y - pos_y;
			if(id == seduce_element_pointer_id(input, i, &j) && j == part)
				break;
		}
		active = i < input->pointer_count;

		for(i = 0; i < input->user_count; i++)
			if(id == seduce_element_selected_id(i, NULL, &j) && j == part)
				if(betray_button_get(i, BETRAY_BUTTON_FACE_A))
				active = TRUE;

		if(seduce_angle_axis_detect(input, 0, angle_a, angle_a + angle_b))
			active = TRUE;

/*		if(active)
			seduce_background_popup_fill(array, angle_a, 0.05, angle_b, 0.12, timer * 0.9, 0, y, 0.9);
		else
			seduce_background_popup_fill(array, angle_a, 0.04, angle_b, 0.1, timer * 0.9, x, y, 0.7);*/
		angle_a = angle_a / 180.0 * PI;
		angle_b = angle_b / 180.0 * PI;
		box[0] = sin(angle_a);
		box[1] = cos(angle_a);
		box[2] = 0.0;

		box[3] = box[0] * 0.1;
		box[4] = box[1] * 0.1;
		box[5] = 0.0;

		box[9] = sin(angle_a + angle_b);
		box[10] = cos(angle_a + angle_b);
		box[11] = 0.0;

		box[6] = box[9] * 0.1;
		box[7] = box[10] * 0.1;
		box[8] = 0.0;
		
		seduce_element_add_quad(input, id, part, &box[0], &box[3], &box[6], &box[9]);
		if(angle_b < PI / 3)
		{
			box[0] = sin(angle_a);
			box[1] = cos(angle_a);
			box[2] = sin(angle_a + angle_b * 0.5);
			box[3] = cos(angle_a + angle_b * 0.5);
			box[4] = sin(angle_a + angle_b);
			box[5] = cos(angle_a + angle_b);
			if(active)
			{
				seduce_background_quad_draw(input, id, part,
										box[0] * 0.25 + box[1] * 0.005, box[1] * 0.25 - box[0] * 0.005, 0, 
										box[0] * 0.1 + box[1] * 0.005, box[1] * 0.1 - box[0] * 0.005, 0, 
										box[2] * 0.09 - box[3] * 0.0, box[3] * 0.09 + box[2] * 0.0, 0, 
										box[2] * 0.42 - box[3] * 0.0, box[3] * 0.42 + box[2] * 0.0, 0,
										box[1] * 0.1, -box[0] * 0.1,  1,
										0.2, 0.6, 1.0, 0.9);
				seduce_background_tri_draw(input, id, part, 
										box[0] * 0.25 + box[1] * 0.005, box[1] * 0.25 - box[0] * 0.005, 0, 
										box[0] * 0.25 + box[1] * 0.005 + box[2] * 0.15, box[1] * 0.25 - box[0] * 0.005 + box[3] * 0.15, 0, 
										box[2] * 0.42 - box[3] * 0.0, box[3] * 0.42 + box[2] * 0.0, 0,
										box[1] * 0.1, -box[0] * 0.1, 1,
										0.2, 0.6, 1.0, 0.9);
				seduce_background_quad_draw(input, id, part,
										box[2] * 0.42 + box[3] * 0.0, box[3] * 0.42 - box[2] * 0.0, 0, 
										box[2] * 0.09 + box[3] * 0.0, box[3] * 0.09 - box[2] * 0.0, 0, 
										box[4] * 0.1 - box[5] * 0.005, box[5] * 0.1 + box[4] * 0.005, 0, 
										box[4] * 0.25 - box[5] * 0.005, box[5] * 0.25 + box[4] * 0.005, 0,
										-box[5] * 0.1, box[4] * 0.1, 1,
										0.2, 0.6, 1.0, 0.9);
				seduce_background_tri_draw(input, id, part,
										box[4] * 0.25 - box[5] * 0.005, box[5] * 0.25 + box[4] * 0.005, 0, 
										box[4] * 0.25 - box[5] * 0.005 + box[2] * 0.15, box[5] * 0.25 + box[4] * 0.005 + box[3] * 0.15, 0, 
										box[2] * 0.42   - box[3] * 0.0, box[3] * 0.42 + box[2] * 0.0, 0,
										-box[5] * 0.1, box[4] * 0.1, 1,
										0.2, 0.6, 1.0, 0.9);
			}
			else
			{
				seduce_background_quad_draw(input, id, part,
										box[0] * 0.2 + box[1] * 0.005, box[1] * 0.2 - box[0] * 0.005, 0, 
										box[0] * 0.1 + box[1] * 0.005, box[1] * 0.1 - box[0] * 0.005, 0, 
										box[2] * 0.09 - box[3] * 0.0, box[3] * 0.09 + box[2] * 0.0, 0, 
										box[2] * 0.36 - box[3] * 0.0, box[3] * 0.36 + box[2] * 0.0, 0,
										box[1] * 0.1, -box[0] * 0.1, 1,
										0.5, 0.5, 0.5, 0.7);
				seduce_background_tri_draw(input, id, part,
										box[0] * 0.2 + box[1] * 0.005, box[1] * 0.2 - box[0] * 0.005, 0, 
										box[0] * 0.15 + box[1] * 0.005 + box[2] * 0.2, box[1] * 0.15 - box[0] * 0.005 + box[3] * 0.2, 0, 
										box[2] * 0.36 - box[3] * 0.0, box[3] * 0.36 + box[2] * 0.0, 0,
										box[1] * 0.1, -box[0] * 0.1,  1,
										0.5, 0.5, 0.5, 0.7);
				seduce_background_quad_draw(input, id, part,
										box[2] * 0.36 + box[3] * 0.0, box[3] * 0.36 - box[2] * 0.0, 0, 
										box[2] * 0.09 + box[3] * 0.0, box[3] * 0.09 - box[2] * 0.0, 0, 
										box[4] * 0.1 - box[5] * 0.005, box[5] * 0.1 + box[4] * 0.005, 0, 
										box[4] * 0.2 - box[5] * 0.005, box[5] * 0.2 + box[4] * 0.005, 0,
										-box[5] * 0.1, box[4] * 0.1, 1,
										0.5, 0.5, 0.5, 0.7);
				seduce_background_tri_draw(input, id, part,
										box[4] * 0.2 - box[5] * 0.005, box[5] * 0.2 + box[4] * 0.005, 0, 
										box[4] * 0.15 - box[5] * 0.005 + box[2] * 0.2, box[5] * 0.15 + box[4] * 0.005 + box[3] * 0.2, 0, 
										box[2] * 0.36 - box[3] * 0.0, box[3] * 0.36 + box[2] * 0.0, 0,
										-box[5] * 0.1, box[4] * 0.1, 1,
										0.5, 0.5, 0.5, 0.7);

			}
		}else
		{
			i = 0;
			box[0] = sin(angle_a + angle_b * ((float)i / 10.0));
			box[1] = cos(angle_a + angle_b * ((float)i / 10.0));
			box[2] = sin(angle_a + angle_b * ((float)(i + 1) / 10.0));
			box[3] = cos(angle_a + angle_b * ((float)(i + 1) / 10.0));
			if(active)
				seduce_background_quad_draw(input, id, part,
										box[0] * 0.2 + box[1] * 0.005, box[1] * 0.2 - box[0] * 0.005, 0, 
										box[0] * 0.1 + box[1] * 0.005, box[1] * 0.1 - box[0] * 0.005, 0, 
										box[2] * 0.1 - box[3] * 0.0, box[3] * 0.1 + box[2] * 0.0, 0, 
										box[2] * 0.25 - box[3] * 0.0, box[3] * 0.25 + box[2] * 0.0, 0, 
										0, 0, 1,
										0.2, 0.6, 1.0, 0.9);
			else
				seduce_background_quad_draw(input, id, part,
										box[0] * 0.2 + box[1] * 0.005, box[1] * 0.2 - box[0] * 0.005, 0, 
										box[0] * 0.1 + box[1] * 0.005, box[1] * 0.1 - box[0] * 0.005, 0, 
										box[2] * 0.1 - box[3] * 0.0, box[3] * 0.1 + box[2] * 0.0, 0, 
										box[2] * 0.25 - box[3] * 0.0, box[3] * 0.25 + box[2] * 0.0, 0,
										0, 0, 1,
										0.5, 0.5, 0.5, 0.7);

		for(i = 1; i < 9; i++)
		{
			box[0] = sin(angle_a + angle_b * ((float)i / 10.0));
			box[1] = cos(angle_a + angle_b * ((float)i / 10.0));
			box[2] = sin(angle_a + angle_b * ((float)(i + 1) / 10.0));
			box[3] = cos(angle_a + angle_b * ((float)(i + 1) / 10.0));
			if(active)
				seduce_background_quad_draw(input, id, part,
										box[0] * 0.25 + box[1] * 0.0, box[1] * 0.25 - box[0] * 0.0, 0, 
										box[0] * 0.1 + box[1] * 0.0, box[1] * 0.1 - box[0] * 0.0, 0, 
										box[2] * 0.1 - box[3] * 0.0, box[3] * 0.1 + box[2] * 0.0, 0, 
										box[2] * 0.25 - box[3] * 0.0, box[3] * 0.25 + box[2] * 0.0, 0, 
										0, 0, 1,
										0.2, 0.6, 1.0, 0.9);
			else
				seduce_background_quad_draw(input, id, part,
										box[0] * 0.25 + box[1] * 0.0, box[1] * 0.25 - box[0] * 0.0, 0, 
										box[0] * 0.1 + box[1] * 0.0, box[1] * 0.1 - box[0] * 0.0, 0, 
										box[2] * 0.1 - box[3] * 0.0, box[3] * 0.1 + box[2] * 0.0, 0, 
										box[2] * 0.25 - box[3] * 0.0, box[3] * 0.25 + box[2] * 0.0, 0,
										0, 0, 1,
										0.5, 0.5, 0.5, 0.7);
		}
			box[0] = sin(angle_a + angle_b * ((float)i / 10.0));
			box[1] = cos(angle_a + angle_b * ((float)i / 10.0));
			box[2] = sin(angle_a + angle_b * ((float)(i + 1) / 10.0));
			box[3] = cos(angle_a + angle_b * ((float)(i + 1) / 10.0));
			if(active)
				seduce_background_quad_draw(input, id, part,
										box[0] * 0.25 + box[1] * 0.0, box[1] * 0.25 - box[0] * 0.0, 0, 
										box[0] * 0.1 + box[1] * 0.0, box[1] * 0.1 - box[0] * 0.0, 0, 
										box[2] * 0.1 - box[3] * 0.005, box[3] * 0.1 + box[2] * 0.005, 0, 
										box[2] * 0.2 - box[3] * 0.005, box[3] * 0.2 + box[2] * 0.005, 0, 
										0, 0, 1,
										0.2, 0.6, 1.0, 0.9);
			else
				seduce_background_quad_draw(input, id, part,
										box[0] * 0.25 + box[1] * 0.0, box[1] * 0.25 - box[0] * 0.0, 0, 
										box[0] * 0.1 + box[1] * 0.0, box[1] * 0.1 - box[0] * 0.0, 0, 
										box[2] * 0.1 - box[3] * 0.005, box[3] * 0.1 + box[2] * 0.005, 0, 
										box[2] * 0.2 - box[3] * 0.005, box[3] * 0.2 + box[2] * 0.005, 0,
										0, 0, 1,
										0.5, 0.5, 0.5, 0.7);
		}
	}
}

boolean seduce_background_shape_draw(BInputState *input, void *id, float a_x, float a_y, float b_x, float b_y, float c_x, float c_y, float d_x, float d_y, float timer, float normal_x, float normal_y, float *center, float transparancy)
{
	if(input->mode == BAM_DRAW)
	{
		float *array, vec[4], t;
		RShader	*shader;
		if(s_background_used + 2 > S_BACKGROUND_POLY_COUNT_MAX)
			return FALSE;
		array = &s_background_buffer[s_background_used * 3 * 6];
		s_background_used += 2;
		if(timer > 1.0)
			timer = 1.0;
		if(timer < 0.5)
		{
			t = timer * 2.0;
			timer = 0.5;
		}else
		{
			t = 1.0;
			timer = 1.0 - timer * 1.0;
		}
		
		vec[0] = (b_x - a_x);
		vec[1] = (b_y - a_y);
		vec[2] = (c_x - d_x);
		vec[3] = (c_y - d_y);

		array[0] = a_x + vec[0] * timer;
		array[1] = a_y + vec[1] * timer;
		array[2] = 0;
		array[3] = normal_x;
		array[4] = normal_y;
		array[5] = transparancy;

		array[6] = a_x + vec[0] * t + vec[0] * timer;
		array[7] = a_y + vec[1] * t + vec[1] * timer;
		array[8] = 0;
		array[9] = normal_x;
		array[10] = normal_y;
		array[11] = transparancy;
		
		array[12] = d_x + vec[2] * t + vec[2] * timer;
		array[13] = d_y + vec[3] * t + vec[3] * timer;
		array[14] = 0;
		array[15] = normal_x;
		array[16] = normal_y;
		array[17] = transparancy;

		array[18] = array[0];
		array[19] = array[1];
		array[20] = 0;
		array[21] = normal_x;
		array[22] = normal_y;
		array[23] = transparancy;

		array[24] = array[12];
		array[25] = array[13];
		array[26] = 0;
		array[27] = normal_x;
		array[28] = normal_y;
		array[29] = transparancy;

		array[30] = d_x + vec[2] * timer;
		array[31] = d_y + vec[3] * timer;
		array[32] = 0;
		array[33] = normal_x;
		array[34] = normal_y;
		array[35] = transparancy;
		if(id != NULL)
			seduce_element_add_quad(input, id, 0, array, &array[6], &array[12], &array[30]);
	}else if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					return TRUE;
	}
	return FALSE;
}


boolean seduce_background_tri_draw(BInputState *input, void *id, uint part,
														float a_x, float a_y, float a_z, 
														float b_x, float b_y, float b_z, 
														float c_x, float c_y, float c_z, 
														float normal_x, float normal_y, float particle_influence,
														float surface_r, float surface_g, float surface_b, float surface_a)
{
	uint i;
	if(input->mode == BAM_DRAW)
	{
		float *array;
		if(s_background_used + 1 > S_BACKGROUND_POLY_COUNT_MAX)
			return FALSE;
		array = &s_background_buffer[s_background_used++ * 3 * (3 + 3 + 4)];
		array[S_BACKGROUND_VERTEX_SIZE * 0 + 0] = a_x;
		array[S_BACKGROUND_VERTEX_SIZE * 0 + 1] = a_y;
		array[S_BACKGROUND_VERTEX_SIZE * 0 + 2] = a_z;
		array[S_BACKGROUND_VERTEX_SIZE * 1 + 0] = b_x;
		array[S_BACKGROUND_VERTEX_SIZE * 1 + 1] = b_y;
		array[S_BACKGROUND_VERTEX_SIZE * 1 + 2] = b_z;
		array[S_BACKGROUND_VERTEX_SIZE * 2 + 0] = c_x;
		array[S_BACKGROUND_VERTEX_SIZE * 2 + 1] = c_y;
		array[S_BACKGROUND_VERTEX_SIZE * 2 + 2] = c_z;
		for(i = 0; i < 3; i++)
		{
			array[S_BACKGROUND_VERTEX_SIZE * i + 3] = normal_x;
			array[S_BACKGROUND_VERTEX_SIZE * i + 4] = normal_y;
			array[S_BACKGROUND_VERTEX_SIZE * i + 5] = particle_influence;

			array[S_BACKGROUND_VERTEX_SIZE * i + 6] = surface_r;
			array[S_BACKGROUND_VERTEX_SIZE * i + 7] = surface_g;
			array[S_BACKGROUND_VERTEX_SIZE * i + 8] = surface_b;
			array[S_BACKGROUND_VERTEX_SIZE * i + 9] = surface_a;
		}

		if(id != NULL)
		{
			seduce_element_add_triangle(input, id, part, array, &array[S_BACKGROUND_VERTEX_SIZE * 1], &array[S_BACKGROUND_VERTEX_SIZE * 2]);

			for(i = 0; i < input->pointer_count; i++)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0] && id == seduce_element_pointer_id(input, i, NULL))	
				{
					float x, y, y2, pos[3], vec_b[3], vec_c[3];
					static uint seed = 0;
					SPraticleType type;
					uint count;
					type = seduce_background_particle_color_allocate(id, surface_r, surface_g, surface_b);
					vec_b[0] = b_x - a_x;
					vec_b[1] = b_y - a_y;
					vec_b[2] = b_z - a_z;
					vec_c[0] = c_x - a_x;
					vec_c[1] = c_y - a_y;
					vec_c[2] = c_z - a_z;

					x = sqrt(vec_b[0] * vec_b[0] + vec_b[1] * vec_b[1]);
					y = vec_c[0] * vec_b[1] / x -  vec_c[1] * vec_b[0] / x;
					if(y > 0)
						count = x * y * S_BACKGROUND_PARTICLE_COUNT * 0.4;
					else
						count = x * -y * S_BACKGROUND_PARTICLE_COUNT * 0.4;
					if(count < 1)
						count = 1;
					if(count > S_BACKGROUND_PARTICLE_COUNT)
						count = S_BACKGROUND_PARTICLE_COUNT;

					for(i = 0; i < count; i++)
					{
						x = f_randf(seed++);
						y = f_randf(seed++);
						y2 = y * y;
						y = 1.0 - y2;
		
						pos[0] = a_x + y2 * vec_c[0] + x * y * vec_b[0];
						pos[1] = a_y + y2 * vec_c[1] + x * y * vec_b[1];
						pos[2] = a_z + y2 * vec_c[2] + x * y * vec_b[2];

						seduce_background_particle_spawn(input, pos[0], pos[1], f_randnf(seed) * 1, f_randnf(seed + 1) * 1, f_randnf(seed + 2) * 0.5, type);
						seed++;
					}
				}
			}
		}

	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					return TRUE;
	}
	return FALSE;
}

boolean seduce_background_quad_draw(BInputState *input, void *id, uint part,
														float a_x, float a_y, float a_z, 
														float b_x, float b_y, float b_z, 
														float c_x, float c_y, float c_z, 
														float d_x, float d_y, float d_z, 
														float normal_x, float normal_y, float particle_influence,
														float surface_r, float surface_g, float surface_b, float surface_a)
{
	if(seduce_background_tri_draw(input, id, part, a_x, a_y, a_z, 
											b_x, b_y, b_z, 
											c_x, c_y, c_z, 
											normal_x, normal_y, particle_influence,
											surface_r, surface_g, surface_b, surface_a) ||
		seduce_background_tri_draw(input, id, part, a_x, a_y, a_z, 
											c_x, c_y, c_z, 
											d_x, d_y, d_z, 
											normal_x, normal_y, particle_influence,
											surface_r, surface_g, surface_b, surface_a))
		return TRUE;
	return FALSE;
}

/*
boolean seduce_background_shape_draw(BInputState *input, void *id, float a_x, float a_y, float b_x, float b_y, float c_x, float c_y, float d_x, float d_y, float timer, float normal_x, float normal_y, float *center, float transparancy)
{
	if(input->mode == BAM_DRAW)
	{
		float *array, vec[4], t;
		RShader	*shader;
		if(s_background_used + 2 > S_BACKGROUND_POLY_COUNT_MAX)
			return FALSE;
		array = &s_background_buffer[s_background_used * 3 * 6];
		if(timer > 1.0)
			timer = 1.0;
		if(timer < 0.5)
		{
			t = timer * 2.0;
			timer = 0.5;
		}else
		{
			t = 1.0;
			timer = 1.0 - timer * 1.0;
		}
		
		vec[0] = (b_x - a_x);
		vec[1] = (b_y - a_y);
		vec[2] = (c_x - d_x);
		vec[3] = (c_y - d_y);

		array[0] = a_x + vec[0] * timer;
		array[1] = a_y + vec[1] * timer;
		array[2] = 0;
		array[3] = normal_x;
		array[4] = normal_y;
		array[5] = transparancy;

		array[6] = a_x + vec[0] * t + vec[0] * timer;
		array[7] = a_y + vec[1] * t + vec[1] * timer;
		array[8] = 0;
		array[9] = normal_x;
		array[10] = normal_y;
		array[11] = transparancy;
		
		array[12] = d_x + vec[2] * t + vec[2] * timer;
		array[13] = d_y + vec[3] * t + vec[3] * timer;
		array[14] = 0;
		array[15] = normal_x;
		array[16] = normal_y;
		array[17] = transparancy;

		array[18] = array[0];
		array[19] = array[1];
		array[20] = 0;
		array[21] = normal_x;
		array[22] = normal_y;
		array[23] = transparancy;

		array[24] = array[12];
		array[25] = array[13];
		array[26] = 0;
		array[27] = normal_x;
		array[28] = normal_y;
		array[29] = transparancy;

		array[30] = d_x + vec[2] * timer;
		array[31] = d_y + vec[3] * timer;
		array[32] = 0;
		array[33] = normal_x;
		array[34] = normal_y;
		array[35] = transparancy;
		if(id != NULL)
			seduce_element_add_quad(input, id, 0, array, &array[6], &array[12], &array[30]);
	}else if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					return TRUE;
	}
	return FALSE;
}
*/


boolean seduce_background_shape_draw2(BInputState *input, void *id, float a_x, float a_y, float b_x, float b_y, float c_x, float c_y, float d_x, float d_y, float timer, float normal_x, float normal_y, float *center)
{
	return FALSE;
	if(input->mode == BAM_DRAW)
	{
		float array[30], vec[4], t;
		if(timer > 1.0)
			timer = 1.0;
		if(timer < 0.5)
		{
			t = timer * 2.0;
			timer = 0.5;
		}else
		{
			t = 1.0;
			timer = 1.0 - timer * 1.0;
		}

		vec[0] = (b_x - a_x);
		vec[1] = (b_y - a_y);
		vec[2] = (c_x - d_x);
		vec[3] = (c_y - d_y);

		array[0] = a_x + vec[0] * timer;
		array[1] = a_y + vec[1] * timer;
		array[2] = 0;
		array[3] = normal_x;
		array[4] = normal_y;

		array[5] = a_x + vec[0] * t + vec[0] * timer;
		array[6] = a_y + vec[1] * t + vec[1] * timer;
		array[7] = 0;
		array[8] = normal_x;
		array[9] = normal_y;

		array[10] = d_x + vec[2] * t + vec[2] * timer;
		array[11] = d_y + vec[3] * t + vec[3] * timer;
		array[12] = 0;
		array[13] = normal_x;
		array[14] = normal_y;

		array[15] = array[0];
		array[16] = array[1];
		array[17] = 0;
		array[18] = normal_x;
		array[19] = normal_y;

		array[20] = array[10];
		array[21] = array[11];
		array[22] = 0;
		array[23] = normal_x;
		array[24] = normal_y;

		array[25] = d_x + vec[2] * timer;
		array[26] = d_y + vec[3] * timer;
		array[27] = 0;
		array[28] = normal_x;
		array[29] = normal_y;

		r_array_load_vertex(s_background_pool, NULL, array, 0, 6);
	//	s_background_shader_set(input);
	//	r_shader_vec4_set(NULL, r_shader_uniform_location(s_background_shader, "surface_color"), 0.3, 0.2, 0.1, 1);
		r_array_section_draw(s_background_pool, NULL, GL_TRIANGLES, 0, 6);
	//	r_shader_vec4_set(NULL, r_shader_uniform_location(s_background_shader, "surface_color"), seduce_background_shader_surface_color[0], seduce_background_shader_surface_color[1], seduce_background_shader_surface_color[2], seduce_background_shader_surface_color[3]);
		if(id != NULL)
		{
			seduce_element_add_quad(input, id, 0, array, &array[5], &array[10], &array[25]);
		}
	}else if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					return TRUE;
	}
	return FALSE;
}

boolean seduce_background_image_draw(BInputState *input, void *id, float pos_x, float pos_y, float pos_z, float size_x, float size_y, float u_start, float v_start, float u_size, float v_size, float timer, float *center, uint texture_id)
{
	float array[30], f;

	if(input->mode == BAM_DRAW)
	{
		array[25] = array[15] = array[0] = pos_x;
		array[16] = array[6] = array[1] = pos_y;
		array[27] = array[22] = array[17] = array[12] = array[7] = array[2] = pos_z;
		array[28] = array[18] = array[3] = u_start;
		array[19] = array[9] = array[4] = v_start;
		array[20] = array[10] = array[5] = pos_x + size_x;
		array[23] = array[13] = array[8] = u_start + u_size;
		array[26] = array[21] = array[11] = pos_y + size_y;
		array[29] = array[24] = array[14] = v_start + v_size;
		r_shader_set(s_image_shader);
		if(center != NULL)
			r_shader_vec2_set(s_image_shader, r_shader_uniform_location(s_image_shader, "center"), center[0], center[1]);
		else
			r_shader_vec2_set(s_image_shader, r_shader_uniform_location(s_image_shader, "center"), pos_x + size_x * 0.5, pos_y + size_y * 0.5);
		r_shader_float_set(s_image_shader, r_shader_uniform_location(s_image_shader, "time"), timer * 0.7 + timer * timer);
		r_shader_texture_set(s_image_shader, 0, texture_id);
		r_shader_texture_set(s_image_shader, 1, SBackgroundRender.particle_texture);
		r_shader_texture_set(s_image_shader, 2, SBackgroundRender.pattern_texture);
		r_array_load_vertex(s_background_image, NULL, array, 0, 6);
		r_array_section_draw(s_background_image, NULL, GL_TRIANGLES, 0, 6);
		if(id != NULL)
			seduce_element_add_quad(input, id, 0, array, &array[5], &array[10], &array[25]);
	}else if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					return TRUE;
	}
	return FALSE;
}


uint seduce_background_gallery_vertical_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float size_x, float *size_y, float image_height, uint *texture_id, float *aspects, uint image_count, float timer, float *center)
{
	float length = 0, row_length, x, y;
	uint i, j, start, end, rows;
	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < image_count; i++)
			length += aspects[i];
		rows = 1 + length * image_height;
		y = pos_y;
		for(i = start = 0; i < rows; i++)
		{
			row_length = 0;
			if(i + 1 < rows)
			{
				for(end = start; row_length + aspects[end] < length / (float)rows; end++)
					row_length += aspects[end];
				if(row_length - length / (float)rows < length / (float)rows - (row_length + aspects[end]))
					row_length += aspects[end++];
			}else
				for(end = start; end < image_count; end++)
					row_length += aspects[end];
			x = pos_x;
			y -= size_x / row_length;
			for(j = start; j < end; j++)
			{
				seduce_background_image_draw(input, &texture_id[j], x, y, 0, size_x * aspects[j] / row_length, size_x  / row_length, 0, 1, 1, -1, timer, center, texture_id[j]);
				x += aspects[j] / row_length;
			}
			start = end;
		}
		if(size_y != NULL)
			*size_y = y;
	}else if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
			if(!input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(&texture_id[i] == seduce_element_pointer_down_click_id(input, i, NULL) &&
					&texture_id[i] == seduce_element_pointer_id(input, i, NULL))
					return i;
	}
    return -1;
}


uint seduce_background_gallery_horizontal_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float *size_x, float size_y, float image_height, uint *texture_id, float *aspects, uint image_count, float timer, float *center)
{
	float length = 0, columns_height, x, y;
	uint i, j, start, end, columns;
	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < image_count; i++)
			length += 1.0 / aspects[i];
		columns = 1 + length * image_height;
		x = pos_x;
		for(i = start = 0; i < columns; i++)
		{
			columns_height = 0;
			if(i + 1 < columns)
			{
				for(end = start; end < image_count && columns_height + 1.0 / aspects[end] < length / (float)columns ; end++)
					columns_height += 1.0 / aspects[end];
				if(end < image_count && (end == start || columns_height - length / (float)columns < length / (float)columns - (columns_height + 1.0 / aspects[end])))
					columns_height += 1.0 / aspects[end++];
			}else
				for(end = start; end < image_count; end++)
					columns_height += 1.0 / aspects[end];
			y = pos_y;
			for(j = start; j < end; j++)
			{
				y -= (size_y / aspects[j]) / columns_height;
				seduce_background_image_draw(input, &texture_id[j], x, y, 0, size_y / columns_height, (size_y / aspects[j]) / columns_height, 0, 1, 1, -1, timer, center, texture_id[j]);

			}
			if(input->pointers[0].button[2])
			{
				if(size_x != NULL)
					*size_x = x;
			}
			x += size_y / columns_height;
			start = end;
		}
		if(size_x != NULL)
			*size_x = x;
	}else if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
			if(!input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(&texture_id[i] == seduce_element_pointer_down_click_id(input, i, NULL) &&
					&texture_id[i] == seduce_element_pointer_id(input, i, NULL))
					return i;
	}
    return -1;
}




boolean seduce_background_image_draw_old(BInputState *input, void *id, float pos_x, float pos_y, float size_x, float size_y, float u_start, float v_start, float u_size, float v_size, float timer, float *center, uint texture_id)
{
	static float array[12] = {-0.5, -0.5, 0.0, -0.5, 0.5, 0.0, 0.5, 0.5, 0.0, 0.5, -0.5, 0.0}, f;

	if(input->mode == BAM_DRAW)
	{	
		RMatrix	*matrix;
		matrix = r_matrix_get();

		if(timer < 0.999)
		{
			if(size_x > SEDUCE_IMAGE_ANIMATION_SPLIT_SIZE * 1.98 || size_y > SEDUCE_IMAGE_ANIMATION_SPLIT_SIZE * 1.98)
			{
				uint splits_x, splits_y, i, j;
				float fsplits_x, fsplits_y, fi, fj;
				splits_x = (uint)(size_x / SEDUCE_IMAGE_ANIMATION_SPLIT_SIZE);
				splits_y = (uint)(size_y / SEDUCE_IMAGE_ANIMATION_SPLIT_SIZE);
				if(splits_x < 1)
					splits_x = 1;
				if(splits_y < 1)
					splits_y = 1;
				fsplits_x = (float)splits_x;
				fsplits_y = (float)splits_y;
				for(i = 0; i < splits_x; i++)
				{
					fi = (float)i;
					if(i % 2 == 0)
					{
						for(j = 0; j < splits_y; j++)
						{
							fj = (float)j;
							seduce_background_image_draw_old(input, NULL, 
								pos_x + size_x * fi / fsplits_x, pos_y + size_y * fj / fsplits_y, 
								size_x / fsplits_x, size_y / fsplits_y, 
								u_start + u_size * fi / fsplits_x, v_start + v_size * fj / fsplits_y, 
								u_size / fsplits_x, v_size / fsplits_y, 
								timer, center, texture_id);
						}
					}else
					{
						seduce_background_image_draw_old(input, NULL, 
								pos_x + size_x * fi / fsplits_x, pos_y, 
								size_x / fsplits_x, size_y * 0.5 / fsplits_y, 
								u_start + u_size * fi / fsplits_x, v_start, 
								u_size / fsplits_x, v_size * 0.5 / fsplits_y, 
								timer, center, texture_id);
						for(j = 0; j < splits_y - 1; j++)
						{
							fj = (float)j + 0.5;
							seduce_background_image_draw_old(input, NULL, 
								pos_x + size_x * fi / fsplits_x, pos_y + size_y * fj / fsplits_y, 
								size_x / fsplits_x, size_y / fsplits_y, 
								u_start + u_size * fi / fsplits_x, v_start + v_size * fj / fsplits_y, 
								u_size / fsplits_x, v_size / fsplits_y, 
								timer, center, texture_id);
						}
						seduce_background_image_draw_old(input, NULL, 
								pos_x + size_x * fi / fsplits_x, pos_y + size_y - size_y * 0.5 / fsplits_y, 
								size_x / fsplits_x, size_y / fsplits_y * 0.5, 
								u_start + u_size * fi / fsplits_x, v_start + v_size - v_size * 0.5 / fsplits_y, 
								u_size / fsplits_x, v_size / fsplits_y * 0.5, 
								timer, center, texture_id);
					}
				}
				return FALSE;
			}
		}

		r_matrix_push(matrix);

		r_matrix_translate(matrix, pos_x + size_x * 0.5, pos_y + size_y * 0.5, 0);
		if(timer < 0.9)
		{
			f = ((pos_x + pos_y + 2.0)) * 0.8;
			if(f > 2.0)
				f = 2.0;
			if(f < 0.0)
				f = 0.0;
			timer = timer * 3.0 - f;
			if(timer > 1.0)
				timer = 1.0;
			if(timer < 0.0)
				timer = 0.0;
			f = 1.0 - (1.0 - timer) * (1.0 - timer);
			if(timer < 0.999)
				r_matrix_rotate(matrix, (1.0 - timer) * 90.0, 1, 1, 0.1);
			f *= 0.9;
			r_matrix_scale(matrix, size_x * f, size_y * f, f);
		//	r_matrix_rotate(matrix, 1.0, 1.0, 1.0, 0.0);
		}else
		{
			r_matrix_scale(matrix, size_x * timer, size_y * timer, 0);
		//	r_matrix_rotate(matrix, 10.0 * (1.0 - timer), 1.0, 1.0, 0.0);
		}
		r_shader_set(s_image_shader);
		r_shader_texture_set(s_image_shader, 0, texture_id);
		r_shader_vec4_set(NULL, r_shader_uniform_location(s_image_shader, "uv_transform"), u_start, v_start, u_size, v_size);
		r_array_section_draw(s_background_image, NULL, GL_TRIANGLES, 0, 6);
		if(id != NULL)
			seduce_element_add_quad(input, id, 0, array, &array[3], &array[6], &array[9]);
		r_matrix_pop(matrix);
	}else if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					return TRUE;
	}
	return FALSE;
}

boolean seduce_background_shape_matrix_interact(BInputState *input, void *id, float *matrix, boolean scale, boolean rotate)
{
	static void **grab = NULL;
	if(input->mode == BAM_EVENT)
	{
		static boolean a_button = FALSE, a_button_last = FALSE, x_button = FALSE, x_button_last = FALSE;
		uint i, j, count, a = -1, b = -1;
		float size, f, vec_a[2], vec_b[2], center[3], m[16];

		count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		if(grab == NULL)
		{		
			grab = malloc((sizeof *grab) * count);
			for(i = 0; i < count; i++)
				grab[i] = NULL;
		}



		for(j = 0; j < input->user_count; j++)
		{
			if(id == seduce_element_selected_id(j, NULL, NULL))
			{
				float size = 1.0;
				betray_button_get_up_down(j, &a_button, &a_button_last, BETRAY_BUTTON_FACE_A);

				for(i = 0; i < input->axis_count && (input->axis[i].axis_type != B_AXIS_STICK || input->axis[i].axis_count == 1 || input->axis[i].user_id != j); i++);
				if(i < input->axis_count)
				{
					if(a_button)
					{
						matrix[12] += input->delta_time * input->axis[i].axis[0] * 1.0;
						matrix[13] += input->delta_time * input->axis[i].axis[1] * 1.0;
					}
					for(i++; i < input->axis_count && (input->axis[i].axis_type != B_AXIS_STICK || input->axis[i].user_id != j); i++);
					if(i < input->axis_count)
					{
						matrix[12] += input->delta_time * input->axis[i].axis[0] * 1.0;
						matrix[13] += input->delta_time * input->axis[i].axis[1] * 1.0;
					}
				}

				if(scale)
				{
					for(i = 0; i < input->axis_count && (input->axis[i].axis_type != B_AXIS_ZOOM || input->axis[i].user_id != j); i++);
					if(i < input->axis_count)
						size *= 1.0 + input->delta_time * input->axis[i].axis[0] * 1.0;
					
					for(i = 0; i < input->axis_count && (input->axis[i].axis_type != B_AXIS_BUMPER_LEFT || input->axis[i].user_id != j); i++);
					if(i < input->axis_count)
						size *= 1.0 + input->delta_time * input->axis[i].axis[0] * 1.0;

					for(i = 0; i < input->axis_count && (input->axis[i].axis_type != B_AXIS_BUMPER_RIGHT || input->axis[i].user_id != j); i++);
					if(i < input->axis_count)
						size *= 1.0 - input->delta_time * input->axis[i].axis[0] * 1.0;
				}
				for(i = 0; i < input->axis_count && (input->axis[i].axis_type != B_AXIS_SCORLL || input->axis[i].user_id != j); i++);
				if(i < input->axis_count)
				{
					matrix[12] += input->delta_time * input->axis[i].axis[0] * 1.0;
					matrix[13] += input->delta_time * input->axis[i].axis[1] * 1.0;
				}
				betray_button_get_up_down(-1, &x_button, &x_button_last, BETRAY_BUTTON_FACE_X);
				if(scale && x_button)
				{
					for(i = 0; i < input->axis_count && (input->axis[i].axis_type != B_AXIS_STICK || input->axis[i].axis_count == 1 || input->axis[i].user_id != j); i++)
					if(i < input->axis_count)
						size *= 1.0 + input->delta_time * input->axis[0].axis[0] * 1.0;
				}
				if(size > 1.001 || size < 0.999)
				{
					seduce_element_selected_id(j, center, NULL);
					for(i = 0; i < 12; i++)
						matrix[i] *= size;
					matrix[12] = center[0] + (matrix[12] - center[0]) * size;
					matrix[13] = center[1] + (matrix[13] - center[1]) * size;
				}
			}
		}

		for(i = 0; i < input->pointer_count; i++)
			if(grab[i] == id)
				break;

		if(i < input->pointer_count)
		{
			a = i;
			for(i++; i < input->pointer_count; i++)
			{
				if(grab[i] == id && input->pointers[a].user_id == input->pointers[i].user_id)
				{
					b = i;
					break;
				}
			}

			size = f_length3f(matrix);

			if(betray_button_get(input->pointers[a].user_id, BETRAY_BUTTON_SCROLL_UP) && scale)
			{
				matrix[12] -= (input->pointers[a].pointer_x - matrix[12]) * 0.1;
				matrix[13] -= (input->pointers[a].pointer_y - matrix[13]) * 0.1;
				for(i = 0; i < 12; i++)
					matrix[i] *= 1.1;
			}
			if(betray_button_get(input->pointers[a].user_id, BETRAY_BUTTON_SCROLL_DOWN) && scale)
			{
				matrix[12] += (input->pointers[a].pointer_x - matrix[12]) * 0.1;
				matrix[13] += (input->pointers[a].pointer_y - matrix[13]) * 0.1;
				for(i = 0; i < 12; i++)
					matrix[i] *= 0.9;
			}

			if(b == -1)
			{
				if(input->pointers[a].button[1] && rotate)
				{
					center[0] = input->pointers[a].click_pointer_x[1];
					center[1] = input->pointers[a].click_pointer_y[1];
					vec_a[0] = sin((input->pointers[a].delta_pointer_x + input->pointers[a].delta_pointer_y) * 10.0);
					vec_a[1] = cos((input->pointers[a].delta_pointer_x + input->pointers[a].delta_pointer_y) * 10.0);
		
					vec_b[0] = 0;
					vec_b[1] = 1;
					m[0] = vec_a[0] * matrix[0] + -vec_a[1] * matrix[1];
					m[1] = vec_a[1] * matrix[0] + vec_a[0] * matrix[1];

					m[4] = vec_a[0] * matrix[4] + -vec_a[1] * matrix[5];
					m[5] = vec_a[1] * matrix[4] + vec_a[0] * matrix[5];

					matrix[12] -= center[0];
					matrix[13] -= center[1]; 
					m[12] = vec_a[0] * matrix[12] + -vec_a[1] * matrix[13];
					m[13] = vec_a[1] * matrix[12] + vec_a[0] * matrix[13];

					matrix[0] = (m[0] * vec_b[0] + m[4] * -vec_b[1]);
					matrix[1] = (m[1] * vec_b[0] + m[5] * -vec_b[1]);

					matrix[4] = (m[0] * vec_b[1] + m[4] * vec_b[0]);
					matrix[5] = (m[1] * vec_b[1] + m[5] * vec_b[0]);

					matrix[12] = (vec_b[0] * m[12] + vec_b[1] * m[13]);
					matrix[13] = (-vec_b[1] * m[12] + vec_b[0] * m[13]);

					matrix[12] += center[0];
					matrix[13] += center[1];
				}else if(input->pointers[a].button[2] && scale) 
				{
					size = 1.0 + (input->pointers[a].delta_pointer_x + input->pointers[a].delta_pointer_y);
					for(i = 0; i < 12; i++)
						matrix[i] *= size;
					matrix[12] = input->pointers[a].click_pointer_x[2] + (matrix[12] - input->pointers[a].click_pointer_x[2]) * size;
					matrix[13] = input->pointers[a].click_pointer_y[2] + (matrix[13] - input->pointers[a].click_pointer_y[2]) * size;
				}else
				{

					matrix[12] += input->pointers[a].delta_pointer_x;
					matrix[13] += input->pointers[a].delta_pointer_y;
				}

			}else
			{
				vec_a[0] = input->pointers[a].pointer_x - input->pointers[b].pointer_x;
				vec_a[1] = input->pointers[a].pointer_y - input->pointers[b].pointer_y;
				size = sqrt(vec_a[0] * vec_a[0] + vec_a[1] * vec_a[1]);
				vec_a[0] /= size;
				vec_a[1] /= size;
				center[0] = (input->pointers[a].pointer_x + input->pointers[b].pointer_x) / 2.0;
				center[1] = (input->pointers[a].pointer_y + input->pointers[b].pointer_y) / 2.0;
				vec_b[0] = (input->pointers[a].pointer_x - input->pointers[a].delta_pointer_x) - (input->pointers[b].pointer_x - input->pointers[b].delta_pointer_x);
				vec_b[1] = (input->pointers[a].pointer_y - input->pointers[a].delta_pointer_y) - (input->pointers[b].pointer_y - input->pointers[b].delta_pointer_y);
				f = sqrt(vec_b[0] * vec_b[0] + vec_b[1] * vec_b[1]);
				vec_b[0] /= f;
				vec_b[1] /= f;
				size /= f;
				if(!scale)
					size = 1.0;
				if(rotate)
				{
					m[0] = vec_a[0] * matrix[0] + -vec_a[1] * matrix[1];
					m[1] = vec_a[1] * matrix[0] + vec_a[0] * matrix[1];

					m[4] = vec_a[0] * matrix[4] + -vec_a[1] * matrix[5];
					m[5] = vec_a[1] * matrix[4] + vec_a[0] * matrix[5];

					matrix[12] -= center[0];
					matrix[13] -= center[1]; 
					m[12] = vec_a[0] * matrix[12] + -vec_a[1] * matrix[13];
					m[13] = vec_a[1] * matrix[12] + vec_a[0] * matrix[13];

					matrix[0] = (m[0] * vec_b[0] + m[4] * -vec_b[1]) * size;
					matrix[1] = (m[1] * vec_b[0] + m[5] * -vec_b[1]) * size;

					matrix[4] = (m[0] * vec_b[1] + m[4] * vec_b[0]) * size;
					matrix[5] = (m[1] * vec_b[1] + m[5] * vec_b[0]) * size;

					matrix[12] = (vec_b[0] * m[12] + vec_b[1] * m[13]) * size;
					matrix[13] = (-vec_b[1] * m[12] + vec_b[0] * m[13]) * size;

					matrix[12] += center[0];
					matrix[13] += center[1]; 
				}else if(scale)
				{
					for(i = 0; i < 12; i++)
						matrix[i] *= size;
					matrix[12] = center[0] + (matrix[12] - center[0]) * size;
					matrix[13] = center[1] + (matrix[13] - center[1]) * size;
				}
				matrix[12] += (input->pointers[a].delta_pointer_x + input->pointers[b].delta_pointer_x) * 0.5;
				matrix[13] += (input->pointers[a].delta_pointer_y + input->pointers[b].delta_pointer_y) * 0.5;
			}
		}
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					grab[i] = id;
			
			if(!input->pointers[i].button[0])
				grab[i] = NULL;
		}

	}
	return FALSE;
}
void seduce_background_negative_draw(float a_x, float a_y, float b_x, float b_y, float c_x, float c_y, float d_x, float d_y, float timer)
{
	float array[36], vec[4], t;
	RShader	*shader;
	if(timer > 1.0)
		timer = 1.0;
	if(timer < 0.5)
	{
		t = timer * 2.0;
		timer = 0.5;
	}else
	{
		t = 1.0;
		timer = 1.0 - timer * 1.0;
	}

	vec[0] = (b_x - a_x);
	vec[1] = (b_y - a_y);
	vec[2] = (c_x - d_x);
	vec[3] = (c_y - d_y);
	
	array[0] = a_x;
	array[1] = a_y;
	array[2] = 0;

	array[3] = b_x;
	array[4] = b_y;
	array[5] = 0;
	
	array[6] = c_x;
	array[7] = c_y;
	array[8] = 0;

	array[9] = a_x;
	array[10] = a_y;
	array[11] = 0;
	
	array[12] = c_x;
	array[13] = c_y;
	array[14] = 0;

	array[15] = d_x;
	array[16] = d_y;
	array[17] = 0;

	r_array_load_vertex(s_background_negative, NULL, array, 0, 6);
	shader = r_shader_presets_get(P_SP_COLOR_UNIFORM);
	r_shader_set(shader);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	r_shader_vec4_set(NULL, r_shader_uniform_location(shader, "color"), 1, 1, 1, 1);
	r_array_section_draw(s_background_negative, NULL, GL_TRIANGLES, 0, 6);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}
