#include "la_includes.h"
#include "la_projection.h"
#include "la_geometry_undo.h"
#include "la_tool.h"


//#include "presuade.h"

char *lo_pfx_shader_spark_vertex = 
"attribute vec3 vertex;\n"
"attribute vec3 expand;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"uniform vec2 size;\n"
"varying vec2 mapping;\n"
"varying vec4 fade;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"	vec4 v;\n"
"	vec2 scale;\n"
"	float pos;\n"
"	scale = vec2(0.0 + expand.z * 3.0);\n"
"	v = vec4(vertex, 1.0) + vec4(expand.xy * size * scale, 0.0, 0.0) * NormalMatrix;\n"
"	mapping = expand.xy;\n"
"   pos = expand.z * expand.z;\n"
"   pos = pos * pos;\n"
"	color = mix(vec4(2.0, 1.5, 0.3, 1.0), vec4(1.2, 1.8, 1.9, 1.0), pos);\n"
"	fade = mix(vec4(0.7, 0.2, 0.6, 1.0), vec4(0.4, 0.5, 1, 1.0), pos);\n"
"	gl_Position = ModelViewProjectionMatrix * v;\n"
"}\n";


char *lo_pfx_shader_spark_fragment = 
"varying vec2 mapping;\n"
"varying vec4 fade;\n"
"varying vec4 color;\n"
"void main()\n"
"{\n"
"	float f;\n"
"	f =	length(mapping);\n"
"	f = max(1.0 - f, 0.0);\n"
"	f *= f;\n"
"	f *= f;\n"
"	f *= f;\n"
"	gl_FragColor = mix(fade, color, f * f) * vec4(f);\n"
"}\n";


char *lo_pfx_shader_wiggle_vertex = 
"attribute vec3 vertex0;\n"
"attribute vec3 vertex1;\n"
"attribute vec3 vertex2;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform float expand1;\n"
"uniform float expand2;\n"
"varying float distance;\n"
"void main()\n"
"{\n"
"	vec3 v;\n"
"	v = vertex0 + vertex1 * vec3(expand1) + vertex2 * vec3(expand2);\n"
"   distance = length(vertex2 * vec3(expand2));\n"
"	gl_Position = ModelViewProjectionMatrix * vec4(v, 1.0);\n"
"}\n";


char *lo_pfx_shader_wiggle_fragment = 
"varying float distance;\n"
"uniform float scale;\n"
"uniform float cutoff;\n"
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"	float f;\n"
"	if(distance > cutoff)\n"
"		discard;\n"
"	f = distance / scale;\n"
"	f *= f;\n"
"	f *= f;\n"
"	gl_FragColor = color + vec4(1.3, 0.5, 0.0, 0.0) * vec4(f * f);\n"
"}\n";




char *lo_pfx_shader_flare_vertex = 
"uniform mat4 ModelViewProjectionMatrix;"
"attribute vec3 vertex;"
"varying vec2 vector1;"
"varying vec2 vector2;"
"varying vec2 vector3;"
"varying vec2 vector4;"
"varying vec2 vector5;"
"varying vec2 red;"
"varying vec2 green;"
"varying vec2 blue;"
"varying vec2 center;"
"varying vec2 shade;"
"varying float distance;"
"uniform vec2 pointer;"

"void main()"
"{"
"	vec3 v;"
"	distance = length(pointer.xy);"
"	vector1 = (pointer.xy * -vec2(-1.5 + distance * 3.0) - vertex.xy) * vec2(10.0) * (2.0 + distance * -1.2);"
"	vector2 = (pointer.xy * vec2(1.3) - vertex.xy) * vec2(13.0) * (1.2 + distance * -0.7);"
"	vector3 = (pointer.xy * vec2(0.9) - vertex.xy) * vec2(5.0) * (1.0 + distance * 0.7);"
"	vector4 = (pointer.xy * -vec2(1.8) - vertex.xy) * vec2(7.0) * (0.6 + distance * -0.4);"
"	vector5 = (pointer.xy * -vec2(0.9) - vertex.xy) * vec2(25.0) * (0.5 + distance * 0.4);"
"	shade = (pointer.xy * -vec2(0.3) - vertex.xy) * vec2(2.2) * (0.7 + distance * 0.4);"
"	red = (pointer.xy * vec2(0.7) - vertex.xy) * vec2(6.0);"
"	green = (pointer.xy * vec2(0.8) - vertex.xy) * vec2(7.0);"
"	blue = (pointer.xy * vec2(0.9) - vertex.xy) * vec2(8.0);"
"	center = vertex.xy * vec2(0.4);"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);"
"}";


char *lo_pfx_shader_flare_fragment = 
"varying vec2 vector1;"
"varying vec2 vector2;"
"varying vec2 vector3;"
"varying vec2 vector4;"
"varying vec2 vector5;"
"varying vec2 red;"
"varying vec2 green;"
"varying vec2 blue;"
"varying vec2 center;"
"varying vec2 shade;"
"varying float distance;"
"void main()"
"{"
"	vec4 color;"
"	float f;"
"	f = 1.0 / (1.0 + dot(vector1, vector1));"
"	color += mix(vec4(0.3, 0.2, 0.3, 0.0), vec4(0.7, 0.3, 0.4, 0.0), length(vector1) * 0.1) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector2, vector2));"
"	color += mix(vec4(0.7, 0.3, 0.4, 0.0), vec4(0.3, 0.2, 0.3, 0.0), length(vector2) * 0.3) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector3, vector3));"
"	color += mix(vec4(0.2, 0.2, 0.3, 0.0), vec4(0.3, 0.1, 0.2, 0.0), length(vector3) * 0.2) * vec4(f);"
"	f = 2.0 / (1.0 + dot(vector4, vector4));"
"	color += mix(vec4(0.1, 0.17, 0.24, 0.0), vec4(0.2, 0.1, 0.3, 0.0), length(vector4) * 0.2) * vec4(f);"
"	f = 1.0 / (1.0 + dot(vector5, vector5));"
"	color += mix(vec4(0.1, 0.11, 0.12, 0.0), vec4(0.07, 0.14, 0.03, 0.0), length(vector5) * 0.2) * vec4(f);"


"	f = 1.0 / (1.0 + dot(red, red));"
"	color += vec4(0.3, 0.0, 0.0, 0.0) * vec4(f);"
"	f = 1.1 / (1.0 + dot(green, green));"
"	color += vec4(0.0, 0.3, 0.0, 0.0) * vec4(f);"
"	f = 1.2 / (1.0 + dot(blue, blue));"
"	color += vec4(0.0, 0.0, 0.3, 0.0) * vec4(f);"
"	gl_FragColor = color * vec4(distance);"
"	gl_FragColor += vec4(dot(center, center) / (1.0 + dot(shade, shade))) * vec4(1, 0.8, 0.6, 0) - vec4(0.05);"
"}";


extern boolean	la_load_targa(char *file_name, uint *texture_id);
extern uint	la_save_targa(char *file_name, float *data, unsigned int size);

#define TEX_SPLIT 8

#define BRIGHT_COUNT 12

typedef struct {
	double	age;
	double	size;
	double	pos[3];
	double	vector[3];
}DustParticle;

typedef struct {
	double	age;
	double	size;
	double	pos[3];
	double	vector[3];
}SparkParticle;

typedef struct {
	double	age;
	double	pos[3];
	double	size;
}BrightParticle;

typedef struct {
	float			wiggle_timer;
	boolean			wiggle_major;
	uint			wiggle_draw_lenght;
	uint			wiggle_used;
	void			*wiggle_section;
	float			*wiggle_array;
}WiggleBlock;

#define WIGGLE_BLOCK_COUNT 16

struct{
	DustParticle	*dust;
	uint			dust_length;
	uint			dust_count;
	uint			dust_start;
	float			*dust_pos;
	float			*dust_col;
	BrightParticle	*bright;
	uint			bright_count;
	uint			bright_start;
	SparkParticle	*spark;
	uint			spark_length;
	uint			spark_count;
	uint			spark_start;
	void			*spark_pool;
	float			*spark_array;
	RShader			*spark_shader;
	float			spark_delta;


	uint			wiggle_length;
	void			*wiggle_pool;
	RShader			*wiggle_shader;
	RShader			*flare_shader;
	void			*flare_pool;
	WiggleBlock		wiggle_blocks[WIGGLE_BLOCK_COUNT];

	uint			point_material;
	uint			star_material;
	uint			flare_material;
	uint			intro_material;
	uint			video_material;
	uint			soft_material;
	uint			soft_material2;
	uint			surface_material;
	float			*select_pos;
	float			*select_uv;
}GlobalParticleData;


void add_video(int size, float *data)
{
	int i, j, k, temp;
	float x, y, r, half, best;
	half = (float)size * 0.5;
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size; j++)
		{
			temp = size / 8;
			x = (temp / 2) - (i % temp);
			y = 0 - (j % (temp * 2));
			best = ((x * x) / (temp * temp)) + ((y * y) / (temp * temp));
			k = 0;
			x = temp - (i % temp);
			y = temp - (j % (temp * 2));
			r = ((x * x) / (temp * temp)) + ((y * y) / (temp * temp));
			if(r < best)
			{
				best = r;
				k = 2;
			}
			x = 0 - (i % temp);
			y = temp - (j % (temp * 2));
			r = ((x * x) / (temp * temp)) + ((y * y) / (temp * temp));
			if(r < best)
			{
				best = r;
				k = 1;
				data[(i * size + j) * 3 + ((1 + (i / temp) + 0) % 3)] += 1 - r;
			}
			x = (temp / 2) - (i % temp);
			y = (temp * 2) - (j % (temp * 2));
			r = ((x * x) / (temp * temp)) + ((y * y) / (temp * temp));
			if(r < best)
			{
				best = r;
				k = 0;
			}
			x = (i - half) / half;
			y = (j - half) / half;
			r = (x * x + y * y);
			best *= 2;
			data[(i * size + j) * 3 + ((k + (i / temp) + 0) % 3)] = 1.6 - best + r * 0.8 + 0.2;
			data[(i * size + j) * 3 + ((k + (i / temp) + 1) % 3)] = 1 - best + r * 0.8 + 0.2;
	//		data[(i * size + j) * 3 + ((k + (i / temp) + 2) % 3)] = r * 0.6 + 0.4;
			data[(i * size + j) * 3 + ((k + (i / temp) + 2) % 3)] = r * 0.4 + 0.6;
		}
	}
}

void add_star(int size, float *data)
{
	int i, j, k, temp;
	float x, y, r, half, best;
	half = (float)size * 0.5;
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size; j++)
		{
			best = 0;
			for(k = 0; k < 240; k++)
			{
				temp = k * 2;
				temp = (temp<<13) ^ temp;
				x = (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
				x = (((i - half) / half) + x) * 4;
				temp = k * 2 + 1;
				temp = (temp<<13) ^ temp;
				y = (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
				y = (((j - half) / half) + y) * 4;
				r = 1 - (x * x + y * y);
				if(r > best)
					best = r;
			}
			best = 0.5 + ((1 - best) * 0.8);
			x = (i - half) / half;
			y = (j - half) / half;
			r = (1 - (x * x + y * y)) * best;
			if(r < 0)
				r = 0;
			data[(i * size + j) * 3] = r * r;
			data[(i * size + j) * 3 + 1] = r * r * r;
			data[(i * size + j) * 3 + 2] = r;
		}
	}
}


void add_point2(int jump, int size, float *data, float *color)
{
	int i, j, k, l, temp;
	float x, y, r, half, best, noise;
	half = (float)size * 0.5;
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size; j++)
		{
			temp = j * 4546 + i;
			temp = (temp<<13) ^ temp;
			noise = (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * 0.005;
			for(l = 0; l < 3; l++)
			{
				best = 0;
				for(k = 0; k < 60; k++)
				{
					temp = k * 2;
					temp = (temp<<13) ^ temp;
					x = (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
					x = (((i - half) / half) + x) * (8 - l);
					temp = k * 2 + 1;
					temp = (temp<<13) ^ temp;
					y = (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
					y = (((j - half) / half) + y) * (8 - l);
					r = 1 - (x * x + y * y);
					if(r > best)
						best = r;
				}
				best = 1 - ((1 - best) * 0.2);
				x = (i - half) / (half - l);
				y = (j - half) / (half - l);
				r = (1 - (x * x + y * y));
				r *= 5;
				if(r > 0.3)
					r = 0.3 - (r - 0.3) * 0.04 * best;
				r *= 2;
				data[(i * jump + j) * 3 + l] = r * color[l] + noise;
			}
		}
	}
}

void add_point(int size, float *data, float *color)
{
	int i, j, k, temp;
	float x, y, r, r2, half;
	half = (float)size * 0.5;
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size; j++)
		{
			x = (i - half) / half;
			y = (j - half) / half;
			r = (1 - sqrt(sqrt(x * x + y * y)) * 0.5) - 0.5;
			r2 = sin(r * 3.14);
			data[(i * size + j) * 3 + 0] = r2 * color[0];
			data[(i * size + j) * 3 + 1] = r2 * color[1];
			data[(i * size + j) * 3 + 2] = r2 * color[2];
		}
	}
}

void add_flare(int size, float *data)
{
	int i, j, k, temp;
	float x, y, r, half, best;
	half = (float)size * 0.5;
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size; j++)
		{
			x = ((i - half) / half) * 8;
			y = ((j - half) / half) * 8;
			r = 1 / (x * x + y * y) - 0.01; 
			data[(i * size + j) * 3] = r * 0.8;
			data[(i * size + j) * 3 + 1] = r * 0.7;
			data[(i * size + j) * 3 + 2] = r * 1;
		}
	}
}



void add_intro_flare(int size, float *data)
{
	int i, j, k, temp;
	float x, y, r, half, best;
	half = (float)size * 0.5;
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size; j++)
		{
			x = ((i - half) / half) * 1.4;
			y = ((j - half) / half) * 1.4;
			r = 1 / (x * x + y * y) - 0.8; 
			if(r > 1)
				r = 1;
			data[(i * size + j) * 3] = 1 - r * 0.75;
			data[(i * size + j) * 3 + 1] = 1 - r * 0.45;
			data[(i * size + j) * 3 + 2] = 1 - r * 0.15;
		}
	}
}


float get_random(uint temp)
{
	temp = (temp<<13) ^ temp;
	return (1.0 - (float)((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

void add_shade(int size, float *data, float *pos, float *color, float color_rand, float brightness)
{
	int i, j, k, temp;
	float x, y, z, r, half, nex_color[3], nex_pos[3];
	half = (float)size * 0.5;
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size; j++)
		{
			x = ((i - half) / half);
			y = ((j - half) / half);
			z = (x * x + y * y);
			if(z < 1)
				z = sqrt(1 - z);
			else
				z = 0;
			for(k = 0; k < 50; k++)
			{
				nex_pos[0] = pos[0] + (get_random(k * 6 + 0) * 0.8);
				nex_pos[1] = pos[1] + (get_random(k * 6 + 1) * 0.8);
				nex_pos[2] = pos[2] + (get_random(k * 6 + 2) * 0.8);
				r = x * nex_pos[0] + y * nex_pos[1] + z * nex_pos[2];
				if(r > 0)
				{
					nex_color[0] = color[0] + get_random(k * 6 + 3) * color_rand;
					nex_color[1] = color[1] + get_random(k * 6 + 4) * color_rand;
					nex_color[2] = color[2] + get_random(k * 6 + 5) * color_rand;
			/*		data[(i * size + j) * 3] += (r * nex_color[0]) * (r * nex_color[0]) * brightness;
					data[(i * size + j) * 3 + 1] += (r * nex_color[1]) * (r * nex_color[1]) * brightness;
					data[(i * size + j) * 3 + 2] += (r * nex_color[2]) * (r * nex_color[2]) * brightness;*/
					data[(i * size + j) * 3] += (r * nex_color[0]) * brightness;
					data[(i * size + j) * 3 + 1] += (r * nex_color[1]) * brightness;
					data[(i * size + j) * 3 + 2] += (r * nex_color[2]) * brightness;
				}
			}
		}
	}
}


uint create_def_material(void)
{
	uint texture_id, size = 256, i;
	float *data, color[3] = {1, 1, 1};
	glEnable(GL_TEXTURE_2D);
	data = malloc((sizeof *data) * size * size * 3);
	color[0] = 2;
	color[1] = 1.6;
	color[2] = 1.2;
	add_point2(size, size, data, color);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGB, GL_FLOAT, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_2D);
	free(data);
	return texture_id;
}

#define SELECT_FLARE_SPITT 4

extern void get_randomized_color(float *color, uint32 index);

void la_pfx_image_init(uint texture_size)
{
	float *data, color[3];
	uint i, j, temp;
	data = malloc((sizeof *data) * texture_size * texture_size * 3);
	if(!la_load_targa("la_tmp_star.tga", &GlobalParticleData.star_material))
	{
		add_star(texture_size / 2, data);
		GlobalParticleData.star_material = la_save_targa("la_tmp_star.tga", data, texture_size / 2);
	}
	if(!la_load_targa("la_tmp_intro.tga", &GlobalParticleData.intro_material))
	{
		add_intro_flare(texture_size / 2, data);
		GlobalParticleData.intro_material = la_save_targa("la_tmp_intro.tga", data, texture_size / 2);
	}
	if(!la_load_targa("la_tmp_flare.tga", &GlobalParticleData.flare_material))
	{
		add_flare(texture_size / 2, data);
		GlobalParticleData.flare_material = la_save_targa("la_tmp_flare.tga", data, texture_size / 2);
	}




	if(!la_load_targa("la_tmp_points.tga", &GlobalParticleData.point_material))
	{
		for(i = 0; i < SELECT_FLARE_SPITT * SELECT_FLARE_SPITT; i++)
		{
			temp = i * 4 + 0;
			temp = (temp<<13) ^ temp;
			color[0] = (((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * 0.5;
			color[2] = color[0];
			color[1] = color[0] * color[0];
			color[0] = color[0] * color[0] * color[0];
			temp = i * 4 + 5;
			temp = (temp<<13) ^ temp;
			color[0] += (((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * 0.1;
			temp = i * 4 + 6;
			temp = (temp<<13) ^ temp;
			color[1] += (((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * 0.1;
			temp = i * 4 + 7;
			temp = (temp<<13) ^ temp;
			color[2] += (((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * 0.1;
		/*	color[0] *= 0.3;
			color[1] *= 0.3;
			color[2] *= 0.3;*/
			add_point2(texture_size, texture_size / SELECT_FLARE_SPITT, &data[(((texture_size / SELECT_FLARE_SPITT) * (i % SELECT_FLARE_SPITT)) + ((texture_size * texture_size / SELECT_FLARE_SPITT) * (i / SELECT_FLARE_SPITT))) * 3], color);
		}
		GlobalParticleData.point_material = la_save_targa("la_tmp_points.tga", data, texture_size);
	}

	if(!la_load_targa("la_tmp_surface.tga", &GlobalParticleData.surface_material))	
	{
		uint texture_id;
		float light_color[3] = {0.2, 0.6, 1};
		float light_pos[3] = {0.2, -0.2, 0} , r;
		for(i = 0; i < texture_size * texture_size * 3; i++)
			data[i] = 0;
		add_shade(texture_size / 2, data, light_pos, light_color, -0.0, 0.06);
		light_color[0] = 1;
		light_color[1] = 0.5;
		light_color[2] = 0.3;
		light_pos[0] = 0.3;
		light_pos[1] = -0.3;
		light_pos[2] = -1;
		add_shade(texture_size / 2, data, light_pos, light_color, -0.06, 0.04);
		light_color[0] = 0.4;
		light_color[1] = -0.2;
		light_color[2] = -0.2;
		light_pos[0] = -0.3;
		light_pos[1] = 0.3;
		light_pos[2] = -0.3;
		add_shade(texture_size / 2, data, light_pos, light_color, -0.06, 0.02);
		GlobalParticleData.surface_material = la_save_targa("la_tmp_surface.tga", data, texture_size / 2);
	}
	if(!la_load_targa("la_tmp_video.tga", &GlobalParticleData.video_material))
	{
		add_video(texture_size, data);
		GlobalParticleData.video_material = la_save_targa("la_tmp_video.tga", data, texture_size);
	}
	if(!la_load_targa("la_tmp_soft.tga", &GlobalParticleData.soft_material))
	{
		color[0] = 0.163;
		color[1] = 0.179;
		color[2] = 0.185;
		color[0] = 1;
		color[1] = 1;
		color[2] = 1;
		add_point(texture_size / 4, data, color);
		GlobalParticleData.soft_material = la_save_targa("la_tmp_soft.tga", data, texture_size / 4);
	}
	if(!la_load_targa("la_tmp_soft2.tga", &GlobalParticleData.soft_material2))	
	{
		color[0] = 0.085;
		color[1] = 0.063;
		color[2] = 0.179;
		add_point(texture_size / 4, data, color);
		GlobalParticleData.soft_material2 = la_save_targa("la_tmp_soft2.tga", data, texture_size / 4);
	}
	free(data);
}


void la_pfx_init(uint particle_count)
{
	RFormats types[3] = {R_FLOAT, R_FLOAT, R_FLOAT};
	uint  vertex_size[3] = {3, 3, 3}, size, ref_size, vertex_count;
	char buffer[2048];
	float *data, color[3], vertex[] = {-1, -100, 0, 
										-1, 100, 0, 
										1, 100, 0, 
										-1, -100, 0, 
										1, 100, 0, 
										1, -100, 0};
	uint i, j, temp;
	GlobalParticleData.dust = NULL;
	GlobalParticleData.dust_length = particle_count * 4;
	GlobalParticleData.dust_count = 0;
	GlobalParticleData.dust_start = 0;
	GlobalParticleData.dust_pos = 0;
	GlobalParticleData.dust_col = 0;
	GlobalParticleData.spark = NULL;
	GlobalParticleData.spark_length = particle_count; 
	GlobalParticleData.spark_count = 0;
	GlobalParticleData.spark_start = 0;
	GlobalParticleData.spark_delta = 0;
	GlobalParticleData.bright = malloc((sizeof *GlobalParticleData.bright) * BRIGHT_COUNT);
	for(i = 0; i < BRIGHT_COUNT; i++)
		GlobalParticleData.bright[i].age = -2.0;
	GlobalParticleData.bright_count = 0;
	GlobalParticleData.bright_start = 0;
	GlobalParticleData.select_pos = malloc((sizeof *GlobalParticleData.select_pos) * 4 * SELECT_FLARE_SPITT * SELECT_FLARE_SPITT * 3);
	GlobalParticleData.select_uv = malloc((sizeof *GlobalParticleData.select_pos) * 4 * SELECT_FLARE_SPITT * SELECT_FLARE_SPITT * 2);

	GlobalParticleData.spark_pool = r_array_allocate(GlobalParticleData.spark_length * 6, types, vertex_size, 2, 0);
	GlobalParticleData.spark_array = malloc((sizeof *GlobalParticleData.spark_array) * GlobalParticleData.spark_length * 6 * 6);
	for(i = 0; i < GlobalParticleData.spark_length; i++)
	{
		GlobalParticleData.spark_array[i * 6 * 6 + 3] = 1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 4] = 1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 9] = -1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 10] = 1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 15] = -1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 16] = -1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 21] = 1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 22] = 1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 27] = -1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 28] = -1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 33] = 1.0;
		GlobalParticleData.spark_array[i * 6 * 6 + 34] = -1.0;
	}
	GlobalParticleData.spark_shader = r_shader_create_simple(buffer, 2048, lo_pfx_shader_spark_vertex, lo_pfx_shader_spark_fragment, "spark shader");
	r_shader_state_set_depth_test(GlobalParticleData.spark_shader, R_DT_ALWAYS); 
	r_shader_state_set_blend_mode(GlobalParticleData.spark_shader, R_BM_ONE, R_BM_ONE); /* Controls the shaders source and destinating blend factor. */
	r_shader_state_set_mask(GlobalParticleData.spark_shader, TRUE, TRUE, TRUE, TRUE, FALSE);
	GlobalParticleData.wiggle_length = particle_count;

	GlobalParticleData.wiggle_pool = r_array_allocate(GlobalParticleData.wiggle_length * 2 * WIGGLE_BLOCK_COUNT, types, vertex_size, 3, 0);
	
	GlobalParticleData.wiggle_shader = r_shader_create_simple(buffer, 2048, lo_pfx_shader_wiggle_vertex, lo_pfx_shader_wiggle_fragment, "wiggle shader");
	
	for(i = 0; i < WIGGLE_BLOCK_COUNT; i++)
	{
		GlobalParticleData.wiggle_blocks[i].wiggle_array = NULL;
		GlobalParticleData.wiggle_blocks[i].wiggle_section = r_array_section_allocate_vertex(GlobalParticleData.wiggle_pool, GlobalParticleData.wiggle_length * 2);
		GlobalParticleData.wiggle_blocks[i].wiggle_timer = 2.0;
		GlobalParticleData.wiggle_blocks[i].wiggle_major = TRUE;
		GlobalParticleData.wiggle_blocks[i].wiggle_draw_lenght = 0;
		GlobalParticleData.wiggle_blocks[i].wiggle_used = 0;	
	}
	for(i = 0; i < SELECT_FLARE_SPITT * SELECT_FLARE_SPITT; i++)
	{
/*		r_set_vec2(GlobalParticleData.select_uv, i * 4 + 0, (float)(i % SELECT_FLARE_SPITT) / SELECT_FLARE_SPITT, (float)(i / SELECT_FLARE_SPITT) / SELECT_FLARE_SPITT);
		r_set_vec2(GlobalParticleData.select_uv, i * 4 + 1, (float)(i % SELECT_FLARE_SPITT) / SELECT_FLARE_SPITT, (float)(i / SELECT_FLARE_SPITT + 1) / SELECT_FLARE_SPITT);
		r_set_vec2(GlobalParticleData.select_uv, i * 4 + 2, (float)(i % SELECT_FLARE_SPITT + 1) / SELECT_FLARE_SPITT, (float)(i / SELECT_FLARE_SPITT + 1) / SELECT_FLARE_SPITT);
		r_set_vec2(GlobalParticleData.select_uv, i * 4 + 3, (float)(i % SELECT_FLARE_SPITT + 1) / SELECT_FLARE_SPITT, (float)(i / SELECT_FLARE_SPITT) / SELECT_FLARE_SPITT);
*/	}
	GlobalParticleData.flare_pool = r_array_allocate(6, types, vertex_size, 1, 0);
	r_array_load_vertex(GlobalParticleData.flare_pool, NULL, vertex, 0, 6);
	GlobalParticleData.flare_shader = r_shader_create_simple(buffer, 2048, lo_pfx_shader_flare_vertex, lo_pfx_shader_flare_fragment, "flare shader");
	r_shader_state_set_depth_test(GlobalParticleData.flare_shader, R_DT_ALWAYS);
	r_shader_state_set_cull_face(GlobalParticleData.flare_shader, FALSE);
	r_shader_state_set_blend_mode(GlobalParticleData.flare_shader, R_BM_ONE, R_BM_ONE);
}

extern void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float scale, uint id, float fade,  float *color);

void lo_pfx_draw_local_space(BInputState *input)
{
	float color[5] = {0, 0, 1, 0, 0}, m[16];
	if(input->mode == BAM_DRAW && la_axis_visible)
	{
		RMatrix *reset;
		uint i;
		reset = r_matrix_get();
		r_matrix_set(&la_world_matrix);
		r_matrix_push(&la_world_matrix);
		for(i = 0; i < 16; i++)
			m[i] = la_axis_matrix[i];
		r_matrix_matrix_mult(&la_world_matrix, m);
		r_matrix_projection_screenf(&la_world_matrix, m, la_axis_matrix[12], la_axis_matrix[13], la_axis_matrix[14]);
		seduce_object_3d_draw(input, 0, 0, 0, 0.6, SEDUCE_OBJECT_HANDLE_NORMAL, 1, &color[2]);	
		r_matrix_rotate(&la_world_matrix, -90, 0, 1, 0);
		seduce_object_3d_draw(input, 0, 0, 0, 0.6, SEDUCE_OBJECT_HANDLE_NORMAL, 1, &color[0]);	
		r_matrix_rotate(&la_world_matrix, 90, 0, 0, 1);
		seduce_object_3d_draw(input, 0, 0, 0, 0.6, SEDUCE_OBJECT_HANDLE_NORMAL, 1, &color[1]);	
		r_matrix_pop(&la_world_matrix);
		r_matrix_set(reset);
	}
}


void la_pfx_create_dust(double *pos, double size)
{
	DustParticle *p;
	if(GlobalParticleData.dust_count == 0)
	{
		uint	i;

		GlobalParticleData.dust = malloc((sizeof *GlobalParticleData.dust) * GlobalParticleData.dust_length);
		GlobalParticleData.dust_pos = malloc((sizeof *GlobalParticleData.dust_pos) * GlobalParticleData.dust_length * 2 * 3);
		GlobalParticleData.dust_col = malloc((sizeof *GlobalParticleData.dust_col) * GlobalParticleData.dust_length * 2 * 3);
		for(i = 0; i < GlobalParticleData.dust_length; i++)
			GlobalParticleData.dust[i].age = -2.0;
	}
	p = &GlobalParticleData.dust[GlobalParticleData.dust_start++ % GlobalParticleData.dust_length];
	if(p->age < 0.0)
		GlobalParticleData.dust_count++;
	p->pos[0] = pos[0];
	p->pos[1] = pos[1];
	p->pos[2] = pos[2];
	p->vector[0] = 0;
	p->vector[1] = 0;
	p->vector[2] = 0;
	p->size = size;
	p->age = 1;
}

void la_pfx_create_spark(double *pos)
{
	SparkParticle *p;
	double camera[3];
	static uint counter;
	uint temp;

	if(GlobalParticleData.spark_delta > 0)
		return;
	GlobalParticleData.spark_delta = 0.02;

	if(GlobalParticleData.spark_count == 0)
	{
		GlobalParticleData.spark = malloc((sizeof *GlobalParticleData.spark) * GlobalParticleData.spark_length);
		for(counter = 0; counter < GlobalParticleData.spark_length; counter++)
			GlobalParticleData.spark[counter].age = 2.0;
	}
	p = &GlobalParticleData.spark[GlobalParticleData.spark_start++ % GlobalParticleData.spark_length];
	if(p->age > 1.0)
		GlobalParticleData.spark_count++;
	p->pos[0] = pos[0];
	p->pos[1] = pos[1];
	p->pos[2] = pos[2];
	seduce_view_camera_getd(NULL, camera);
	p->size = 0.02 * sqrt((camera[0] - pos[0]) * (camera[0] - pos[0]) + (camera[1] - pos[1]) * (camera[1] - pos[1]) + (camera[2] - pos[2]) * (camera[2] - pos[2]));
	
	p->vector[0] = f_randnf(counter++) * p->size * 0.05;
	p->vector[1] = (f_randnf(counter++) + 0.5) * p->size * 0.05;
	p->vector[2] = f_randnf(counter++) * p->size * 0.05;
	p->age = 0;
}

void la_pfx_create_intro_spark(double *pos)
{
	SparkParticle *p;
	double camera[3];
	static uint counter;
	uint temp;
	if(GlobalParticleData.spark_count == 0)
	{
		GlobalParticleData.spark = malloc((sizeof *GlobalParticleData.spark) * GlobalParticleData.spark_length);
		for(counter = 0; counter < GlobalParticleData.spark_length; counter++)
			GlobalParticleData.spark[counter].age = -2.0;
	}
	p = &GlobalParticleData.spark[GlobalParticleData.spark_start++ % GlobalParticleData.spark_length];
	if(p->age < 0.0)
		GlobalParticleData.spark_count++;
	p->pos[0] = pos[0];
	p->pos[1] = pos[1];
	p->pos[2] = pos[2];
	seduce_view_camera_getd(NULL, camera);
//	p->size = 0.02 * sqrt((camera[0] - pos[0]) * (camera[0] - pos[0]) + (camera[1] - pos[1]) * (camera[1] - pos[1]) + (camera[2] - pos[2]) * (camera[2] - pos[2]));
	p->size = 0.006;
	counter++;
	temp = (counter<<13) ^ counter;
	p->vector[0] = (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * p->size;
	counter++;
	temp = (counter<<13) ^ counter;
	p->vector[1] = (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * p->size;
	counter++;
	temp = (counter<<13) ^ counter;
	p->vector[2] = (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * p->size;
	p->age = 1;
	p->size = 0.06;
}


void la_pfx_create_bright(double *pos)
{
	BrightParticle *p;
	double camera[3];
	if(BRIGHT_COUNT > GlobalParticleData.bright_count)
		GlobalParticleData.bright_count++;
	p = &GlobalParticleData.bright[GlobalParticleData.bright_start++ % BRIGHT_COUNT];
	seduce_view_camera_getd(NULL, camera);
	p->size = sqrt((camera[0] - pos[0]) * (camera[0] - pos[0]) + (camera[1] - pos[1]) * (camera[1] - pos[1]) + (camera[2] - pos[2]) * (camera[2] - pos[2]));
	p->pos[0] = pos[0];
	p->pos[1] = pos[1];
	p->pos[2] = pos[2];
	p->age = 1;
	
}

void la_pfx_create_dust_line(double *pos_a, double *pos_b)
{
	double pos[3], size, step, count, i;
	count = sqrt((pos_a[0] - pos_b[0]) * (pos_a[0] - pos_b[0]) + (pos_a[1] - pos_b[1]) * (pos_a[1] - pos_b[1]) + (pos_a[2] - pos_b[2]) * (pos_a[2] - pos_b[2]));
	seduce_view_camera_getd(NULL, pos);
	pos[0] -= (pos_a[0] + pos_b[0]) * 0.5;
	pos[1] -= (pos_a[1] + pos_b[1]) * 0.5;
	pos[2] -= (pos_a[2] + pos_b[2]) * 0.5;
	size = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);
	step = size / (count * GlobalParticleData.dust_length * 0.2);
	if(step  < 0.01)
			step  = 0.01;
	for(i = 0.1; i < 0.9; i += step)
	{
		pos[0] = pos_a[0] * i + pos_b[0] * (1.0 - i);
		pos[1] = pos_a[1] * i + pos_b[1] * (1.0 - i);
		pos[2] = pos_a[2] * i + pos_b[2] * (1.0 - i);
		la_pfx_create_dust(pos, size);
	}
}

void la_pfx_create_dust_selected_vertexes(double *mid)
{
	double pos[3], size;
	uint32 vertex_count, i, j;
	double *vertex;
	DustParticle *p;

	udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);
	seduce_view_camera_getd(NULL, pos);
	size = sqrt((mid[0] - pos[0]) * (mid[0] - pos[0]) + (mid[1] - pos[1]) * (mid[1] - pos[1]) + (mid[2] - pos[2]) * (mid[2] - pos[2]));
	for(i = 0; i < vertex_count; i++)
	{
		if(vertex[i * 3] != E_REAL_MAX && udg_get_select(i) > 0.1)
		{
			udg_get_vertex_pos(pos, i);
			for(j = 0; j < 10; j++)
			{
				la_pfx_create_dust(pos, size);
				p = &GlobalParticleData.dust[(GlobalParticleData.dust_start - 1) % GlobalParticleData.dust_length];
				p->vector[0] = (pos[0] - mid[0]) * 0.02;
				p->vector[1] = (pos[1] - mid[1]) * 0.02;
				p->vector[2] = (pos[2] - mid[2]) * 0.02;
			}
		}
	}
}

void la_pfx_draw(boolean intro)
{
	double matrix[16], size;
	float vertex[12], uv[8] = {0, 0, 1, 0, 1, 1, 0, 1};
	uint i = 0;
	seduce_view_model_matrixd(NULL, matrix);
	if(GlobalParticleData.dust_count > 0)
	{
		DustParticle *p;
		for(i = 0; i < GlobalParticleData.dust_length; i++)
		{
			p = &GlobalParticleData.dust[i];
			if(p->age > -1.0)
			{
				static uint32 counter = 0; 
				uint32 temp;
//				r_set_vec3(GlobalParticleData.dust_pos, i * 2 + 1, p->pos[0], p->pos[1], p->pos[2]);
				size = p->size * 0.03 * (1 - p->age) * (1 - p->age);
				counter++;
				temp = (counter<<13) ^ counter;
				p->vector[0] += (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * size;
				counter++;
				temp = (counter<<13) ^ counter;
				p->vector[1] += (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * size;
				p->vector[1] -= p->size * 0.0004;
				counter++;
				temp = (counter<<13) ^ counter;
				p->vector[2] += (1.0 - ((temp * (temp * temp * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * size;
				p->pos[0] += p->vector[0] * betray_time_delta_get() * 100;
				p->pos[1] += p->vector[1] * betray_time_delta_get() * 100;
				p->pos[2] += p->vector[2] * betray_time_delta_get() * 100;
				p->age -= betray_time_delta_get();
	//			r_set_vec3(GlobalParticleData.dust_pos, i * 2, p->pos[0], p->pos[1], p->pos[2]);
	//			r_set_vec3(GlobalParticleData.dust_col, i * 2, p->age * 1.2, p->age * 1.1, p->age * 1);
				if(p->age < 0)
				{
					p->age = -2.0;
					GlobalParticleData.dust_count--;
				}
			}
		}
		if(GlobalParticleData.dust_count == 0)
		{
			free(GlobalParticleData.dust);
			free(GlobalParticleData.dust_pos);
			free(GlobalParticleData.dust_col);
		}
	}
	if(GlobalParticleData.spark_count > 0)
	{
		SparkParticle *p;
		double matrix[16];
		seduce_view_model_matrixd(NULL, matrix);
		for(i = 0; i < GlobalParticleData.spark_length; i++)
		{
			p = &GlobalParticleData.spark[i];
			if(p->age > -1.0)
			{
				p->vector[0] *= 0.98;
				p->vector[1] *= 0.98;
				p->vector[2] *= 0.98;
				p->pos[0] += p->vector[0];
				p->pos[1] += p->vector[1]/* - 0.025 * p->size*/;
				p->pos[2] += p->vector[2];
				p->age -= 0.005;
				if(intro)
					size = p->size * p->age * 10;
				else
					size = p->size * p->age;
		//		r_set_vec3(vertex, 0, p->pos[0] - matrix[0] * size + matrix[1] * size, p->pos[1] - matrix[4] * size + matrix[5] * size, p->pos[2] - matrix[8] * size + matrix[9] * size);
		//		r_set_vec3(vertex, 1, p->pos[0] - matrix[0] * size - matrix[1] * size, p->pos[1] - matrix[4] * size - matrix[5] * size, p->pos[2] - matrix[8] * size - matrix[9] * size);
		//		r_set_vec3(vertex, 2, p->pos[0] + matrix[0] * size - matrix[1] * size, p->pos[1] + matrix[4] * size - matrix[5] * size, p->pos[2] + matrix[8] * size - matrix[9] * size);
		//		r_set_vec3(vertex, 3, p->pos[0] + matrix[0] * size + matrix[1] * size, p->pos[1] + matrix[4] * size + matrix[5] * size, p->pos[2] + matrix[8] * size + matrix[9] * size);
				glDepthMask(0);
				if(intro)
				{
			//		sui_set_blend_gl(GL_ZERO, GL_SRC_COLOR);
			//		sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.intro_material);
				}else
				{
			//		sui_set_blend_gl(GL_ONE, GL_ONE);
			//		sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.flare_material /**/);
				}
			//	r_gl(GL_QUADS, vertex, 4, 3, 1, 1, 1, 0.0);
				glDepthMask(1);
				if(p->age < 0)
				{
					p->age = -2.0;
					GlobalParticleData.spark_count--;
				}
			}
		}
		if(GlobalParticleData.spark_count == 0)
		{
			free(GlobalParticleData.spark);
		}
	}
	if(GlobalParticleData.bright_count > 0)
	{
		BrightParticle *p;
		double matrix[16], size, size2;
		seduce_view_model_matrixd(NULL, matrix);
		glDepthMask(0);
		glDisable(GL_DEPTH_TEST);
		for(i = 0; i < BRIGHT_COUNT; i++)
		{
			p = &GlobalParticleData.bright[i];
			if(p->age > -1.0)
			{
				p->age -= betray_time_delta_get() * 2;
				if(p->age < 0)
				{
				//	glDisable(GL_LIGHT2 + i);
					p->age = -2.0;
					GlobalParticleData.bright_count--;
					break;
				}
				size = p->size * 1.6 * p->age;
//				r_set_vec3(vertex, 0, p->pos[0] - matrix[0] * size + matrix[1] * size, p->pos[1] - matrix[4] * size + matrix[5] * size, p->pos[2] - matrix[8] * size + matrix[9] * size);
//				r_set_vec3(vertex, 1, p->pos[0] - matrix[0] * size - matrix[1] * size, p->pos[1] - matrix[4] * size - matrix[5] * size, p->pos[2] - matrix[8] * size - matrix[9] * size);
//				r_set_vec3(vertex, 2, p->pos[0] + matrix[0] * size - matrix[1] * size, p->pos[1] + matrix[4] * size - matrix[5] * size, p->pos[2] + matrix[8] * size - matrix[9] * size);
//				r_set_vec3(vertex, 3, p->pos[0] + matrix[0] * size + matrix[1] * size, p->pos[1] + matrix[4] * size + matrix[5] * size, p->pos[2] + matrix[8] * size + matrix[9] * size);
//				sui_set_blend_gl(GL_ONE, GL_ONE);
//				sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.flare_material);
			//	r_gl(GL_QUADS, vertex, 4, 3, 1, 1, 1, 0.0);

				size = p->size * 3 * p->age;
				size2 = p->size * 0.1 * p->age * p->age * p->age;
//				r_set_vec3(vertex, 0, p->pos[0] - matrix[0] * size + matrix[1] * size2, p->pos[1] - matrix[4] * size + matrix[5] * size2, p->pos[2] - matrix[8] * size + matrix[9] * size2);
//				r_set_vec3(vertex, 1, p->pos[0] - matrix[0] * size - matrix[1] * size2, p->pos[1] - matrix[4] * size - matrix[5] * size2, p->pos[2] - matrix[8] * size - matrix[9] * size2);
//				r_set_vec3(vertex, 2, p->pos[0] + matrix[0] * size - matrix[1] * size2, p->pos[1] + matrix[4] * size - matrix[5] * size2, p->pos[2] + matrix[8] * size - matrix[9] * size2);
//				r_set_vec3(vertex, 3, p->pos[0] + matrix[0] * size + matrix[1] * size2, p->pos[1] + matrix[4] * size + matrix[5] * size2, p->pos[2] + matrix[8] * size + matrix[9] * size2);
//				sui_set_blend_gl(GL_ONE, GL_ONE);
//				sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.star_material);
			//	r_gl(GL_QUADS, vertex, 4, 3, 1, 1, 1, 0.0);
				{
					float light[4];
					light[0] = p->pos[0];
					light[1] = p->pos[1];
					light[2] = p->pos[2];
					light[3] = 1;
					glLightfv(GL_LIGHT2 + i, GL_POSITION, light);
					light[0] = p->age * 5 * p->size;
					light[1] = p->age * 5 * p->size;
					light[2] = p->age * 5 * p->size;
					light[3] = 1;
					glLightfv(GL_LIGHT2 + i, GL_DIFFUSE, light);
				}
			}
		}
		glEnable(GL_DEPTH_TEST);
		glDepthMask(1);
	}
}

#define SELECT_FLARE_SIZE 0.015
#define MANIPULATOR_FLARE_SIZE 1
#define SELECT_OTHER_FLARE_SIZE 0.2

void la_pfx_select_vertex(void)
{
	static double t = 0;
	pgreal pos[3], output[3], size, dist, area, scale = 1;
	float v[12], uv[8] = {0, 0, 1, 0, 1, 1, 0, 1};
	uint32 vertex_count, i, j = 0;
	pgreal *vertex;
	udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);
	glDisable(GL_DEPTH_TEST);
	dist = seduce_view_distance_camera_get(NULL);
//	dist *= 2;

	area = 0;
	for(i = 0; i < vertex_count; i++)
	{
		if(vertex[i * 3] != E_REAL_MAX && udg_get_select(i) > 0.1)	
		{
			udg_get_vertex_pos(pos, i);
			seduce_view_projection_screend(NULL, output, pos[0], pos[1], pos[2]);
			size = ((-output[2] - dist) / dist) * SELECT_FLARE_SIZE + 0.01;
			area += size * size;
		}
	}
	area = sqrt(area);
	if(area > 1)
	{
		scale = 1 / area;
	}
	for(i = 0; i < vertex_count; i++)
	{
		if(vertex[i * 3] != E_REAL_MAX && udg_get_select(i) > 0.1)
		{
			udg_get_vertex_pos(pos, i);
			seduce_view_projection_screend(NULL, output, pos[0], pos[1], pos[2]);
			size = scale * (((-output[2] - dist) / dist) * SELECT_FLARE_SIZE + 0.01);
			if(output[2] < 0)
			{	
//				r_set_vec3(GlobalParticleData.select_pos, j * 4 + 0, -output[0] - size, -output[1] + size, -1);
//				r_set_vec3(GlobalParticleData.select_pos, j * 4 + 1, -output[0] + size, -output[1] + size, -1);
//				r_set_vec3(GlobalParticleData.select_pos, j * 4 + 2, -output[0] + size, -output[1] - size, -1);
//				r_set_vec3(GlobalParticleData.select_pos, j * 4 + 3, -output[0] - size, -output[1] - size, -1);
			}else
			{	
//				r_set_vec3(GlobalParticleData.select_pos, j * 4 + 0, 0, 0, -1);
//				r_set_vec3(GlobalParticleData.select_pos, j * 4 + 1, 0, 0, -1);
//				r_set_vec3(GlobalParticleData.select_pos, j * 4 + 2, 0, 0, -1);
//				r_set_vec3(GlobalParticleData.select_pos, j * 4 + 3, 0, 0, -1);
			}
			if(++j == SELECT_FLARE_SPITT * SELECT_FLARE_SPITT)
			{
//				sui_set_blend_gl(GL_ONE, GL_ONE);
//				sui_set_texture2D_array_gl(GlobalParticleData.select_uv, SELECT_FLARE_SPITT * SELECT_FLARE_SPITT * 4, 2, GlobalParticleData.point_material);
			//	r_gl(GL_QUADS, GlobalParticleData.select_pos, SELECT_FLARE_SPITT * SELECT_FLARE_SPITT * 4, 3, 0.3, 0.3, 0.3, 0.0);	
				j = 0;
			}
		}
	}
	if(j != 0)
	{
/*		for(j *= 4; j < SELECT_FLARE_SPITT * SELECT_FLARE_SPITT * 4; j++)
			r_set_vec3(GlobalParticleData.select_pos, j, 0, 0, -1);*/
//		sui_set_blend_gl(GL_ONE, GL_ONE);
//		sui_set_texture2D_array_gl(GlobalParticleData.select_uv, SELECT_FLARE_SPITT * SELECT_FLARE_SPITT * 4, 2, GlobalParticleData.point_material);
	//	r_gl(GL_QUADS, GlobalParticleData.select_pos, SELECT_FLARE_SPITT * SELECT_FLARE_SPITT * 4, 3, 0.3, 0.3, 0.3, 0.0);	
	}
	if(imagine_setting_integer_get("DISPLAY_SILLY_FLARES", TRUE, "Draw flares."))
	{
		j = 0; 
		/*for(i = 0; i < vertex_count; i++)
		{
			if(vertex[i].x < 1.1 && udg_get_select(i) > 0.1)
			{
				udg_get_vertex_pos(pos, i);
				seduce_view_projection_screend(NULL, output, pos[0], pos[1], pos[2]);
				if(output[2] < 0)
				{	
					nglSetVertex3d(GlobalParticleData.select_pos, j * 4 + 0, 8 * output[0] + SELECT_OTHER_FLARE_SIZE, 8 * output[1] + SELECT_OTHER_FLARE_SIZE, -1);
					nglSetVertex3d(GlobalParticleData.select_pos, j * 4 + 1, 8 * output[0] - SELECT_OTHER_FLARE_SIZE, 8 * output[1] + SELECT_OTHER_FLARE_SIZE, -1);
					nglSetVertex3d(GlobalParticleData.select_pos, j * 4 + 2, 8 * output[0] - SELECT_OTHER_FLARE_SIZE, 8 * output[1] - SELECT_OTHER_FLARE_SIZE, -1);
					nglSetVertex3d(GlobalParticleData.select_pos, j * 4 + 3, 8 * output[0] + SELECT_OTHER_FLARE_SIZE, 8 * output[1] - SELECT_OTHER_FLARE_SIZE, -1);
				}else
				{	
					nglSetVertex3d(GlobalParticleData.select_pos, j * 4 + 0, 0, 0, -1);
					nglSetVertex3d(GlobalParticleData.select_pos, j * 4 + 1, 0, 0, -1);
					nglSetVertex3d(GlobalParticleData.select_pos, j * 4 + 2, 0, 0, -1);
					nglSetVertex3d(GlobalParticleData.select_pos, j * 4 + 3, 0, 0, -1);
				}
				if(++j == SELECT_FLARE_SPITT * SELECT_FLARE_SPITT)
				{
					nglDrawArray(NGL_QUADS_FILLED, GlobalParticleData.select_pos, &GlobalParticleData.select_uv, 1, GlobalParticleData.point_material, 0);
					j = 0;
				}
			}
		}
		if(j != SELECT_FLARE_SPITT * SELECT_FLARE_SPITT)
		{
			for(j *= 4; j < SELECT_FLARE_SPITT * SELECT_FLARE_SPITT * 4; j++)
				nglSetVertex3d(GlobalParticleData.select_pos, j, 0, 0, 0);
			nglDrawArray(NGL_QUADS_FILLED, GlobalParticleData.select_pos, &GlobalParticleData.select_uv, 1, GlobalParticleData.point_material, 0);				
		}*/
		la_t_tm_get_pos(pos);
		seduce_view_projection_screend(NULL, output, pos[0], pos[1], pos[2]);
		t += betray_time_delta_get() * 0.2;
		output[0] += 0.1 * sin(t); 
		output[1] += 0.1 * cos(t);
/*		r_set_vec3(v, 0, 2 * output[0] + MANIPULATOR_FLARE_SIZE, 2 * output[1] + MANIPULATOR_FLARE_SIZE, -1);
		r_set_vec3(v, 1, 2 * output[0] - MANIPULATOR_FLARE_SIZE, 2 * output[1] + MANIPULATOR_FLARE_SIZE, -1);
		r_set_vec3(v, 2, 2 * output[0] - MANIPULATOR_FLARE_SIZE, 2 * output[1] - MANIPULATOR_FLARE_SIZE, -1);
		r_set_vec3(v, 3, 2 * output[0] + MANIPULATOR_FLARE_SIZE, 2 * output[1] - MANIPULATOR_FLARE_SIZE, -1);
	//	sui_set_blend_gl(GL_ONE, GL_ONE);
	//	sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.soft_material2);
	//	r_gl(GL_QUADS, v, 4, 3, 1, 1, 1, 0.0);

		r_set_vec3(v, 0, 4 * output[0] + MANIPULATOR_FLARE_SIZE * 0.5, 4 * output[1] + MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 1, 4 * output[0] - MANIPULATOR_FLARE_SIZE * 0.5, 4 * output[1] + MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 2, 4 * output[0] - MANIPULATOR_FLARE_SIZE * 0.5, 4 * output[1] - MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 3, 4 * output[0] + MANIPULATOR_FLARE_SIZE * 0.5, 4 * output[1] - MANIPULATOR_FLARE_SIZE * 0.5, -1);
	//	sui_set_blend_gl(GL_ONE, GL_ONE);
	//	sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.soft_material);
	//	r_gl(GL_QUADS, v, 4, 3, 1, 1, 1, 0.0);

		r_set_vec3(v, 0, -1.5 * output[0] + MANIPULATOR_FLARE_SIZE * 0.15, -1.5 * output[1] + MANIPULATOR_FLARE_SIZE * 0.15, -1);
		r_set_vec3(v, 1, -1.5 * output[0] - MANIPULATOR_FLARE_SIZE * 0.15, -1.5 * output[1] + MANIPULATOR_FLARE_SIZE * 0.15, -1);
		r_set_vec3(v, 2, -1.5 * output[0] - MANIPULATOR_FLARE_SIZE * 0.15, -1.5 * output[1] - MANIPULATOR_FLARE_SIZE * 0.15, -1);
		r_set_vec3(v, 3, -1.5 * output[0] + MANIPULATOR_FLARE_SIZE * 0.15, -1.5 * output[1] - MANIPULATOR_FLARE_SIZE * 0.15, -1);
	//	sui_set_blend_gl(GL_ONE, GL_ONE);
	//	sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.soft_material2);
	//	r_gl(GL_QUADS, v, 4, 3, 1, 1, 1, 0.0);

		r_set_vec3(v, 0, -1.8 * output[0] - 0.4 * output[1] + MANIPULATOR_FLARE_SIZE * 0.5, -1.8 * output[1] + 0.4 * output[0] + MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 1, -1.8 * output[0] - 0.4 * output[1] - MANIPULATOR_FLARE_SIZE * 0.5, -1.8 * output[1] + 0.4 * output[0] + MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 2, -1.8 * output[0] - 0.4 * output[1] - MANIPULATOR_FLARE_SIZE * 0.5, -1.8 * output[1] + 0.4 * output[0] - MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 3, -1.8 * output[0] - 0.4 * output[1] + MANIPULATOR_FLARE_SIZE * 0.5, -1.8 * output[1] + 0.4 * output[0] - MANIPULATOR_FLARE_SIZE * 0.5, -1);
//		sui_set_blend_gl(GL_ONE, GL_ONE);
//		sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.soft_material);
	//	r_gl(GL_QUADS, v, 4, 3, 1, 1, 1, 0.0);

		r_set_vec3(v, 0, -1.8 * output[0] + 0.4 * output[1] + MANIPULATOR_FLARE_SIZE * 0.5, -1.8 * output[1] - 0.4 * output[0] + MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 1, -1.8 * output[0] + 0.4 * output[1] - MANIPULATOR_FLARE_SIZE * 0.5, -1.8 * output[1] - 0.4 * output[0] + MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 2, -1.8 * output[0] + 0.4 * output[1] - MANIPULATOR_FLARE_SIZE * 0.5, -1.8 * output[1] - 0.4 * output[0] - MANIPULATOR_FLARE_SIZE * 0.5, -1);
		r_set_vec3(v, 3, -1.8 * output[0] + 0.4 * output[1] + MANIPULATOR_FLARE_SIZE * 0.5, -1.8 * output[1] - 0.4 * output[0] - MANIPULATOR_FLARE_SIZE * 0.5, -1);
	//	sui_set_blend_gl(GL_ONE, GL_ONE);
	//	sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.soft_material);
	//	r_gl(GL_QUADS, v, 4, 3, 1, 1, 1, 0.0);
		
		*/
	}

	glEnable(GL_DEPTH_TEST);
}

void la_pfx_draw_intro(void)
{
	uint i;
	float size, pos[12], uv[8] = {0, 0, 1, 0, 1, 1, 0, 1};
	if(GlobalParticleData.spark_count > 0)
	{
		SparkParticle *p;
		double matrix[16];
		seduce_view_update(NULL, 0.01);
		seduce_view_model_matrixd(NULL, matrix);
		for(i = 0; i < GlobalParticleData.spark_length; i++)
		{
			p = &GlobalParticleData.spark[i];
			if(p->age > -1.0)
			{
				p->vector[0] *= 0.98;
				p->vector[1] *= 0.98;
				p->vector[2] *= 0.98;
				p->pos[0] += 0.1 * p->vector[0];
				p->pos[1] += 0.1 * p->vector[1] + 0.025 * p->size;
				p->pos[2] += 0.1 * p->vector[2];
				p->age -= 0.005;
				size = p->size * p->age;
/*				r_set_vec3(pos, 0, p->pos[0] - matrix[0] * size + matrix[1] * size, p->pos[1] - matrix[4] * size + matrix[5] * size, p->pos[2] - matrix[8] * size + matrix[9] * size);
				r_set_vec3(pos, 1, p->pos[0] - matrix[0] * size - matrix[1] * size, p->pos[1] - matrix[4] * size - matrix[5] * size, p->pos[2] - matrix[8] * size - matrix[9] * size);
				r_set_vec3(pos, 2, p->pos[0] + matrix[0] * size - matrix[1] * size, p->pos[1] + matrix[4] * size - matrix[5] * size, p->pos[2] + matrix[8] * size - matrix[9] * size);
				r_set_vec3(pos, 3, p->pos[0] + matrix[0] * size + matrix[1] * size, p->pos[1] + matrix[4] * size + matrix[5] * size, p->pos[2] + matrix[8] * size + matrix[9] * size);
*/				glDepthMask(0);
	//			sui_set_blend_gl(GL_ZERO, GL_SRC_COLOR);
	//			sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.video_material);
			//	r_gl(GL_QUADS, pos, 4, 3, 1, 1, 1, 0.0);
				glDepthMask(1);
				if(p->age < 0)
				{
					p->age = -2.0;
					GlobalParticleData.spark_count--;
				}
			}
		}
		if(GlobalParticleData.spark_count == 0)
		{
			free(GlobalParticleData.spark);
		}
	}
	glEnable(GL_DEPTH_TEST);
}

void la_pfx_video_flare(void)
{
	float pos[8] = {0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, 0.5}, uv[8] = {0, 0, 1, 0, 1, 1, 0, 1};
	glDepthMask(0);
	glDisable(GL_DEPTH_TEST);
//	sui_set_blend_gl(GL_ZERO, GL_SRC_COLOR);
//	sui_set_texture2D_array_gl(uv, 4, 2, GlobalParticleData.video_material);
//	r_gl(GL_QUADS, pos, 4, 2, 1, 1, 1, 0.0);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(1);
}

uint la_pfx_surface_material(void)
{
	return GlobalParticleData.surface_material;
}




void la_pfx_manipulator_flare_render(void)
{
	RMatrix *reset;
	double manipulator[3];
	float pos[3], size, distance;
	RShader *shader;
	BInputState *input;
	input = betray_get_input_state();
	if(!imagine_setting_boolean_get("DISPLAY_SILLY_FLARES", TRUE, "Display flares"))
		return;

	la_t_tm_get_pos(manipulator);
	reset = r_matrix_get();
	r_matrix_set(&la_interface_matrix);
	r_matrix_projection_screenf(&la_world_matrix, pos, manipulator[0], manipulator[1], manipulator[2]);
	r_matrix_projection_screenf(&la_world_matrix, pos, manipulator[0] + pos[2] * 0.1, manipulator[1] + pos[2] * 0.1, manipulator[2] + pos[2] * 0.1);

	r_shader_set(GlobalParticleData.flare_shader);
//	r_shader_vec4_set(GlobalParticleData.flare_shader, r_shader_uniform_location(GlobalParticleData.flare_shader, "color"), 0.0, 0.6, 0.6, 1);
	r_shader_vec2_set(GlobalParticleData.flare_shader, r_shader_uniform_location(GlobalParticleData.flare_shader, "pointer"), pos[0], pos[1]);
		
	r_array_section_draw(GlobalParticleData.flare_pool, NULL, GL_TRIANGLES, 0, 6);
}

void la_pfx_manipulator_flare(void)
{
	RMatrix *reset;
	double manipulator[3];
	float pos[3], size, distance;
	if(!imagine_setting_boolean_get("DISPLAY_SILLY_FLARES", TRUE, "Display flares"))
		return;
	la_pfx_manipulator_flare_render();
	return;
	la_t_tm_get_pos(manipulator);
	reset = r_matrix_get();
	r_matrix_set(&la_interface_matrix);
	r_matrix_projection_screenf(&la_world_matrix, pos, manipulator[0], manipulator[1], manipulator[2]);
	glBlendFunc(GL_ONE, GL_ONE);
//	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	size = 0.1;
	distance = sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
	r_primitive_image(pos[0] - size, pos[1] - size, 0, size * 2, size * 2, 0, 0, 1, 1, GlobalParticleData.soft_material, 0.1, 0.01, 0.05, 1);
	size = 1.4;
	pos[0] *= -1.3;
	pos[1] *= -1.3;
	r_primitive_image(pos[0] - size, pos[1] - size, 0, size * 2, size * 2, 0, 0, 1, 1, GlobalParticleData.soft_material, 0.03, 0.05, 0.1, 1);
	size = 0.8 * (1 - distance);
	pos[0] *= 0.7;
	pos[1] *= 0.7;
	r_primitive_image(pos[0] - size, pos[1] - size, 0, size * 2, size * 2, 0, 0, 1, 1, GlobalParticleData.soft_material, 0.05, 0.03, 0.06, 1);
	size = 0.4;
	pos[0] *= -1.3;
	pos[1] *= -1.3;
	r_primitive_image(pos[0] - size, pos[1] - size, 0, size * 2, size * 2, 0, 0, 1, 1, GlobalParticleData.soft_material, 0.05, 0.04, 0.01, 1);
	r_matrix_set(reset);
	glEnable(GL_DEPTH_TEST);
}

void la_pfx_sparks(float delta)
{
	GlobalParticleData.spark_delta -= delta;
	if(GlobalParticleData.spark_count > 0)
	{
		RMatrix *reset;
		SparkParticle *p;
		float size, *f, slow;
		uint i, used = 0;
		slow = 1.0 - delta;
		reset = r_matrix_get();
		r_matrix_set(&la_world_matrix);
		for(i = 0; i < GlobalParticleData.spark_length; i++)
		{
			p = &GlobalParticleData.spark[i];
			if(p->age <= 1.0)
			{
				p->vector[0] *= slow;
				p->vector[1] *= slow;
				p->vector[2] *= slow;
				p->pos[0] += p->vector[0];
				p->pos[1] += p->vector[1] - p->size * 10.0 * delta;
				p->pos[2] += p->vector[2];
				p->age += delta / 5.0;
				f = &GlobalParticleData.spark_array[used++ * 6 * 6];
				f[0] = f[6] = f[12] = f[18] = f[24] = f[30] = p->pos[0];
				f[1] = f[7] = f[13] = f[19] = f[25] = f[31] = p->pos[1];
				f[2] = f[8] = f[14] = f[20] = f[26] = f[32] = p->pos[2]; 
				f[5] = f[11] = f[17] = f[23] = f[29] = f[35] = 1.0 - p->age; 
				if(p->age > 1.0)
				{
					p->age = 2.0;
					GlobalParticleData.spark_count--;
				}
			}
		}
		if(GlobalParticleData.spark_count == 0)
		{
			free(GlobalParticleData.spark);
			GlobalParticleData.spark = NULL;
		}
//		r_primitive_line_flush();
//		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		r_shader_set(GlobalParticleData.spark_shader);
		size = seduce_view_distance_camera_get(NULL) * 0.04;
		r_shader_vec2_set(GlobalParticleData.spark_shader, r_shader_uniform_location(GlobalParticleData.spark_shader, "size"), size, size);
		r_array_load_vertex(GlobalParticleData.spark_pool, NULL, GlobalParticleData.spark_array, 0, used * 6);
		r_array_section_draw(GlobalParticleData.spark_pool, NULL, GL_TRIANGLES, 0, used * 6);
		r_matrix_set(reset);
		glEnable(GL_DEPTH_TEST);
	}
}

void lo_pfx_wiggle_line_add(float ax, float ay, float az, float bx, float by, float bz, boolean major)
{
	static float seed = 0;
	float f, move, *array, size, add;
	uint section;
	size = seduce_view_distance_camera_get(NULL);
	add = 0.005 * size / sqrt((ax - bx) * (ax - bx) + (ay - by) * (ay - by) + (az - bz) * (az - bz));
	size *= 0.1;
	for(section = 0; section < WIGGLE_BLOCK_COUNT && 
		(GlobalParticleData.wiggle_blocks[section].wiggle_used == GlobalParticleData.wiggle_length ||
		GlobalParticleData.wiggle_blocks[section].wiggle_draw_lenght != 0); section++);
	if(section == WIGGLE_BLOCK_COUNT)
		return;

	GlobalParticleData.wiggle_blocks[section].wiggle_major = major;
	if(GlobalParticleData.wiggle_blocks[section].wiggle_array == NULL)
		GlobalParticleData.wiggle_blocks[section].wiggle_array = malloc((sizeof *GlobalParticleData.wiggle_blocks[section].wiggle_array) * GlobalParticleData.wiggle_length * 2 * 9);
	for(f = move = 0.0; f < 1; )
	{
		array = &GlobalParticleData.wiggle_blocks[section].wiggle_array[GlobalParticleData.wiggle_blocks[section].wiggle_used++ * 18];
		array[0] = ax + (bx - ax) * f;
		array[1] = ay + (by - ay) * f;
		array[2] = az + (bz - az) * f;
		f_wiggle3df(&array[3], seed + move * 1.0, 0.5 * size);
		f_wiggle3df(&array[6], seed + move * 7.0, 0.1 * size);
		f += add;
		move += 0.05;
		array[9] = ax + (bx - ax) * f;
		array[10] = ay + (by - ay) * f;
		array[11] = az + (bz - az) * f;
		f_wiggle3df(&array[12], seed + move * 1.0, 0.5 * size);
		f_wiggle3df(&array[15], seed + move * 7.0, 0.1 * size);
		if(GlobalParticleData.wiggle_blocks[section].wiggle_used == GlobalParticleData.wiggle_length)
		{
			GlobalParticleData.wiggle_blocks[section].wiggle_major = major;
			for(section = 0; section < WIGGLE_BLOCK_COUNT && 
				(GlobalParticleData.wiggle_blocks[section].wiggle_used == GlobalParticleData.wiggle_length ||
				GlobalParticleData.wiggle_blocks[section].wiggle_draw_lenght != 0); section++);
			if(section == WIGGLE_BLOCK_COUNT)
				return;
			if(GlobalParticleData.wiggle_blocks[section].wiggle_array == NULL)
				GlobalParticleData.wiggle_blocks[section].wiggle_array = malloc((sizeof *GlobalParticleData.wiggle_blocks[section].wiggle_array) * GlobalParticleData.wiggle_length * 2 * 9);
		}
	}
	seed += 3.24;
}

void lo_pfx_select_circle(float x, float y, float z, float normal_x, float normal_y, float normal_z, float radius)
{
	static float seed = 0;
	float f = 0, a[3], b[3], size, m[16], rx, ry, normal[3], other[3] = {23.3, 45.4, -76.0};
	uint i, splits;
	normal[0] = normal_x;
	normal[1] = normal_y;
	normal[2] = normal_z;
	f_matrixzyf(m, NULL, normal, other);
	size = seduce_view_distance_camera_get(NULL);
	splits = (uint)(360.0 * radius / size);
	if(splits < 40)
		splits = 40;
	if(splits > 96)
		splits = 96;
	for(i = 0; i < splits; )
	{
		rx = radius * sin(f * 2 * PI);
		ry = radius * cos(f * 2 * PI);
		a[0] = m[0] * rx + m[4] * ry + x;
		a[1] = m[1] * rx + m[5] * ry + y;
		a[2] = m[2] * rx + m[6] * ry + z;
		i++;
		f = (float)i / (float)splits;
		rx = radius * sin(f * 2 * PI);
		ry = radius * cos(f * 2 * PI);
		b[0] = m[0] * rx + m[4] * ry + x;
		b[1] = m[1] * rx + m[5] * ry + y;
		b[2] = m[2] * rx + m[6] * ry + z;
		r_primitive_line_3d(a[0], a[1], a[2], b[0], b[1], b[2], 0.5, 0.6, 0.7, 0.6);
	}
	r_primitive_line_flush();
}




void lo_pfx_wiggle_circle(float x, float y, float z, float normal_x, float normal_y, float normal_z, float radius, float expand, boolean major)
{
	static float seed = 0;
	float f, f2, move, move2, *array, size, add, tmp[3], m[16], rx, ry, normal[3], other[3] = {23.3, 45.4, -76.0}, speed = 0.3;
	uint i, section, splits;
	normal[0] = normal_x;
	normal[1] = normal_y;
	normal[2] = normal_z;
	f_matrixzyf(m, NULL, normal, other);
	if(major)
		f = 0;
	
	for(section = 0; section < WIGGLE_BLOCK_COUNT && 
		(GlobalParticleData.wiggle_blocks[section].wiggle_used != 0 ||
		GlobalParticleData.wiggle_blocks[section].wiggle_draw_lenght != 0); section++);
	if(section == WIGGLE_BLOCK_COUNT)
		return;
	if(GlobalParticleData.wiggle_blocks[section].wiggle_array == NULL)
		GlobalParticleData.wiggle_blocks[section].wiggle_array = malloc((sizeof *GlobalParticleData.wiggle_blocks[i].wiggle_array) * GlobalParticleData.wiggle_length * 2 * 9);

	GlobalParticleData.wiggle_blocks[section].wiggle_major = major;
	size = seduce_view_distance_camera_get(NULL) * 0.2;
	splits = (uint)(720.0 * radius / size);
	if(splits < 40)
		splits = 40;
	if(splits + 1 > GlobalParticleData.wiggle_length - GlobalParticleData.wiggle_blocks[section].wiggle_used)
		splits = GlobalParticleData.wiggle_length - GlobalParticleData.wiggle_blocks[section].wiggle_used - 1;
	add = 1 / (float)splits;
	size *= radius;
	move = f = 0.0;
	for(i = 0; i < splits; )
	{
		array = &GlobalParticleData.wiggle_blocks[section].wiggle_array[GlobalParticleData.wiggle_blocks[section].wiggle_used++ * 18];
		rx = radius * sin(f * 2 * PI);
		ry = radius * cos(f * 2 * PI);
		array[0] = m[0] * rx + m[4] * ry + x;
		array[1] = m[1] * rx + m[5] * ry + y;
		array[2] = m[2] * rx + m[6] * ry + z;
		f_wiggle3df(&array[3], seed + move * 0.1, 0.5 * 4.0 * size * f * (1.0 - f) / speed);
		f_wiggle3df(&array[6], seed + move * 1.0, 0.1 * 4.0 * size * f * (1.0 - f) / speed);

		f2 = f + 0.5;
		if(f2 > 1.0)
			f2 -= 1.0;

		if(f > 0.5)
			move2 = (1 - f); 
		else
			move2 = f;  
		f_wiggle3df(tmp, seed + move2 * 0.1, 0.5 * size * f2 * (1.0 - f2) / speed);
		array[3] += tmp[0] + (array[0] - x) * expand;
		array[4] += tmp[1] + (array[1] - y) * expand;
		array[5] += tmp[2] + (array[2] - z) * expand;
		f_wiggle3df(tmp, seed + move2 * 1.0, 0.1 * size * f2 * (1.0 - f2) / speed);
		array[6] += tmp[0];
		array[7] += tmp[1];
		array[8] += tmp[2];
		i++;
		f = (float)i / (float)splits;
		move += 0.05;
		rx = radius * sin(f * 2 * PI);
		ry = radius * cos(f * 2 * PI);
		array[9] = m[0] * rx + m[4] * ry + x;
		array[10] = m[1] * rx + m[5] * ry + y;
		array[11] = m[2] * rx + m[6] * ry + z;
		f_wiggle3df(&array[12], seed + move * 0.1, 0.5 * 4.0 * size * f * (1.0 - f) / speed);
		f_wiggle3df(&array[15], seed + move * 1.0, 0.1 * 4.0 * size * f * (1.0 - f) / speed);

		f2 = f + 0.5;
		if(f2 > 1.0)
			f2 -= 1.0;
		if(f > 0.5)
			move2 = (1 - f);
		else
			move2 = f;  
		f_wiggle3df(tmp, seed + move2 * 0.1, 0.5 * size * f2 * (1.0 - f2) / speed);
		array[12] += tmp[0] + (array[9] - x) * expand;
		array[13] += tmp[1] + (array[10] - y) * expand;
		array[14] += tmp[2] + (array[11] - z) * expand;
		f_wiggle3df(tmp, seed + move2 * 1.0, 0.1 * size * f2 * (1.0 - f2) / speed);
		array[15] += tmp[0];
		array[16] += tmp[1];
		array[17] += tmp[2];
	}
	seed += 3.24;
}

void lo_pfx_compute_tangent(BInputState *input, float *pos, float *normal, uint polygon_snap, boolean position)
{
	float v[3], x[3], y[3], aim[3], camera[3], distance;
	uint32 *ref, vertex_count, ref_count;
	double *vertex;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);
	if(polygon_snap < ref_count)
	{
		v[0] = vertex[ref[polygon_snap * 4] * 3 + 0];
		v[1] = vertex[ref[polygon_snap * 4] * 3 + 1];
		v[2] = vertex[ref[polygon_snap * 4] * 3 + 2];
		x[0] = vertex[ref[polygon_snap * 4 + 2] * 3 + 0] - v[0];
		x[1] = vertex[ref[polygon_snap * 4 + 2] * 3 + 1] - v[1];
		x[2] = vertex[ref[polygon_snap * 4 + 2] * 3 + 2] - v[2];
		y[0] = vertex[ref[polygon_snap * 4 + 1] * 3 + 0] - v[0];
		y[1] = vertex[ref[polygon_snap * 4 + 1] * 3 + 1] - v[1];
		y[2] = vertex[ref[polygon_snap * 4 + 1] * 3 + 2] - v[2];
		f_cross3f(normal, x, y);
		f_normalize3f(normal);
		if(position)
		{
			r_matrix_projection_vertexf(&la_world_matrix, aim, v, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
			seduce_view_camera_getf(NULL, camera);
			aim[0] -= camera[0];
			aim[1] -= camera[1];
			aim[2] -= camera[2];
			distance = seduce_view_distance_camera_get(NULL);
			f_project3f(pos, v, normal, camera, aim);
		}
	}
}

typedef struct{
	SUISnapType snap_type;
	union{
		uint selected_vertex;
		struct{
			uint polygon_snap;
			float pos[3];
		}tangent;
		uint edge_snap[2];
	}data;
	float time;
	boolean exploded;
}LOPFXSelectCircle;

void lo_pfx_draw_selected(BInputState *input, uint snap_type, uint selected_vertex, uint polygon_snap, uint *edge_snap, boolean action)
{
	static uint last_snap_type = SUI_ST_NONE, last_snap_data[2] = {-1, -1};
	static boolean regen;
	static float time, camera_normal[3], data[6];
	double dpos[3];
	LOPFXSelectCircle *c;
	boolean update = FALSE;
	float size;
	uint i, j;
	RMatrix *reset;
	size = seduce_view_distance_camera_get(NULL) * 0.04;
	seduce_view_camera_vector_getf(NULL, camera_normal, 0, 0);

	switch(last_snap_type)
	{
		case SUI_ST_NONE :
		case SUI_ST_VERTEX_FAR :
			if(snap_type != SUI_ST_NONE)
				update = TRUE;
		break;
		case SUI_ST_VERTEX_CLOSE :
			if(snap_type != last_snap_type || last_snap_data[0] != selected_vertex || action)
			{
				if(action)
					lo_pfx_wiggle_circle(data[0], data[1], data[2], camera_normal[0], camera_normal[1], camera_normal[2], time * size, 1, action);
				else
					lo_pfx_wiggle_circle(data[0], data[1], data[2], camera_normal[0], camera_normal[1], camera_normal[2], time * size, 0, action);
				update = TRUE;
			}
		break;
		case SUI_ST_LINE :
			if(snap_type != last_snap_type || last_snap_data[0] != edge_snap[0] || last_snap_data[1] != edge_snap[1])
			{
				lo_pfx_wiggle_line_add(data[0], data[1], data[2], data[3], data[4], data[5], action);
				update = TRUE;
			}
		break;
		case SUI_ST_TANGENT :
			if(snap_type != last_snap_type || last_snap_data[0] != polygon_snap)
			{
				if(action)
					lo_pfx_wiggle_circle(data[0], data[1], data[2], data[3], data[4], data[5], time * size, 1, action);
			//	else
			//		lo_pfx_wiggle_circle(data[0], data[1], data[2], data[3], data[4], data[5], time * size, 0, action);
				update = TRUE;
			}
		break;
	}
	if(update)
	{
		last_snap_type = snap_type;
		time = 0;
		switch(snap_type)
		{
			case SUI_ST_VERTEX_CLOSE :
				last_snap_data[0] = selected_vertex;
			break;
			case SUI_ST_LINE :
				last_snap_data[0] = edge_snap[0];
				last_snap_data[1] = edge_snap[1];
			break;
			case SUI_ST_TANGENT :
				last_snap_data[0] = polygon_snap;
			break;
		}
	}
	if(input->mode == BAM_MAIN)
	{
		time += input->delta_time / 0.25;
		if(time > 1.0)
			time = 1.0;
	}else if(input->mode == BAM_DRAW)
	{	
		reset = r_matrix_get();
		r_matrix_set(&la_world_matrix);
		switch(last_snap_type)
		{
			case SUI_ST_NONE :
			case SUI_ST_VERTEX_FAR :
			break;
			case SUI_ST_VERTEX_CLOSE :
				udg_get_vertex_pos(dpos, last_snap_data[0]);
				data[0] = dpos[0];
				data[1] = dpos[1];
				data[2] = dpos[2];
				lo_pfx_select_circle(data[0], data[1], data[2], camera_normal[0], camera_normal[1], camera_normal[2], time * size);
			break;
			case SUI_ST_LINE :
				udg_get_vertex_pos(dpos, last_snap_data[0]);
				data[0] = dpos[0];
				data[1] = dpos[1];
				data[2] = dpos[2];
				udg_get_vertex_pos(dpos, last_snap_data[1]);
				data[3] = dpos[0];
				data[4] = dpos[1];
				data[5] = dpos[2];		
	//			r_primitive_line_3d(data[0], data[1], data[2], data[3], data[4], data[5], 0.5, 0.6, 0.7, 0.6);
				r_primitive_line_3d(data[0], data[1], data[2], data[3], data[4], data[5], 1, 1, 1, 1);
				r_primitive_line_flush();
			break;
			case SUI_ST_TANGENT :
				
		//		lo_pfx_compute_tangent(input, data, &data[3], last_snap_data[0], TRUE);
		//		lo_pfx_select_circle(data[0], data[1], data[2], data[3], data[4], data[5], time * size);
			break;
		}

		r_matrix_set(reset);
	}
/*	if(input->mode == BAM_EVENT && action && list_count != 0 && regen)
	{
		switch(snap_type)
		{
			case SUI_ST_NONE :
			break;
			case SUI_ST_VERTEX_CLOSE :
				udg_get_vertex_pos(dpos, list[list_count - 1].data.selected_vertex);
				lo_pfx_wiggle_circle(dpos[0], dpos[1], dpos[2], camera_normal[0], camera_normal[1], camera_normal[2], list[list_count - 1].time * size, 1);
				regen = FALSE;
			break;
			case SUI_ST_LENGTH :
			break;
			case SUI_ST_TANGENT :
				
				lo_pfx_compute_tangent(input, center, normal, list[list_count - 1].data.tangent.polygon_snap, TRUE);
				lo_pfx_wiggle_circle(center[0], center[1], center[2], normal[0], normal[1], normal[2], list[list_count - 1].time * size, 1);

			break;
		}
	}
	if(input->mode != BAM_DRAW)
		return;
	for(i = 0; i < list_count && (regen || i + 1 < list_count); i++)
	{
		switch(list[i].snap_type)
		{
			case SUI_ST_NONE :
			break;
			case SUI_ST_VERTEX_CLOSE :
				udg_get_vertex_pos(dpos, list[i].data.selected_vertex);
				lo_pfx_select_circle(dpos[0], dpos[1], dpos[2], camera_normal[0], camera_normal[1], camera_normal[2], list[i].time * size);
			break;
			case SUI_ST_LENGTH :
			break;
			case SUI_ST_TANGENT :	
				lo_pfx_compute_tangent(input, list[i].data.tangent.pos, normal, list[i].data.tangent.polygon_snap, snap_type == SUI_ST_TANGENT && list[i].data.tangent.polygon_snap == polygon_snap);
				lo_pfx_select_circle(list[i].data.tangent.pos[0], 
									list[i].data.tangent.pos[1], 
									list[i].data.tangent.pos[2], normal[0], normal[1], normal[2], list[i].time * size, 1);
			break;
		}
	}*/
}


void lo_pfx_draw_snap(BInputState *input, uint snap_type, double  *snap, double *pos, boolean action)
{
	float f, aim[3], v[3], camera[3], normal[3], p[3],  p2[3], matrix[16], x, y;
	uint i;
	RMatrix *reset;
	if(!action && input->mode != BAM_DRAW)
		return;
	reset = r_matrix_get();
	r_matrix_set(&la_world_matrix);
	switch(snap_type)
	{
		case SUI_ST_NONE :
		case SUI_ST_VERTEX_FAR :
		case SUI_ST_VERTEX_CLOSE :
			if(input->mode == BAM_EVENT)
				lo_pfx_wiggle_line_add(snap[0], snap[1], snap[2], pos[0], pos[1], pos[2], TRUE);
			if(input->mode == BAM_DRAW)
			{
				r_primitive_line_3d(snap[0], snap[1], snap[2], pos[0], pos[1], pos[2], 0.5, 0.6, 0.7, 0.6);
				r_primitive_line_flush();
			}
		break;
		case SUI_ST_LINE :
			v[0] = snap[0];
			v[1] = snap[1];
			v[2] = snap[2];
			r_matrix_projection_vertexf(&la_world_matrix, aim, v, 0, 0);
			v[0] = snap[0] - snap[3];
			v[1] = snap[1] - snap[4];
			v[2] = snap[2] - snap[5];
			f_cross3f(normal, v, aim);
			f_normalize3f(normal);
			f = 0.1;
			for(i = 0; i < 33; i++)
			{
				p[0] = (float)snap[0] + (float)(snap[3] - snap[0]) * (float)i / 32.0;
				p[1] = (float)snap[1] + (float)(snap[4] - snap[1]) * (float)i / 32.0;
				p[2] = (float)snap[2] + (float)(snap[5] - snap[2]) * (float)i / 32.0;
				f = 0.1;
				if(i % 16 == 0)
					f = 0.3;
				else if(i % 8 == 0)
					f = 0.2;
				else if(i % 2 == 0)
					f = 0.14;
				if(action)
				{
					lo_pfx_wiggle_line_add(p[0] - normal[0] * 0.07, p[1] - normal[1] * 0.07, p[2] - normal[2] * 0.07, 
										p[0] - normal[0] * f, p[1] - normal[1] * f, p[2] - normal[2] * f, TRUE);
					lo_pfx_wiggle_line_add(p[0] + normal[0] * 0.07, p[1] + normal[1] * 0.07, p[2] + normal[2] * 0.07, 
										p[0] + normal[0] * f, p[1] + normal[1] * f, p[2] + normal[2] * f, TRUE);
				}else
				{
				/*	r_primitive_line_3d(p[0] - normal[0] * 0.07, p[1] - normal[1] * 0.07, p[2] - normal[2] * 0.07, 
										p[0] - normal[0] * f, p[1] - normal[1] * f, p[2] - normal[2] * f, 0.5, 0.6, 0.7, 0.6);
					r_primitive_line_3d(p[0] + normal[0] * 0.07, p[1] + normal[1] * 0.07, p[2] + normal[2] * 0.07, 
										p[0] + normal[0] * f, p[1] + normal[1] * f, p[2] + normal[2] * f, 0.5, 0.6, 0.7, 0.6);*/
					seduce_primitive_line_add_3d(NULL,
							p[0] - normal[0] * 0.07, p[1] - normal[1] * 0.07, p[2] - normal[2] * 0.07, 
							p[0] - normal[0] * f, p[1] - normal[1] * f, p[2] - normal[2] * f,
							0.5, 0.6, 0.7, 0.6,
							0.5, 0.6, 0.7, 0.6);
					seduce_primitive_line_add_3d(NULL,
							p[0] + normal[0] * 0.07, p[1] + normal[1] * 0.07, p[2] + normal[2] * 0.07, 
							p[0] + normal[0] * f, p[1] + normal[1] * f, p[2] + normal[2] * f,
							0.5, 0.6, 0.7, 0.6,
							0.5, 0.6, 0.7, 0.6);
				}
			}
			if(!action)
			{
			//	r_primitive_line_flush();
				seduce_primitive_line_draw(NULL, 1.0, 1.0, 1.0, 1.0);
			}
		break;
		case SUI_ST_TANGENT :
			v[0] = snap[0];
			v[1] = snap[1];
			v[2] = snap[2];
			r_matrix_projection_vertexf(&la_world_matrix, aim, v, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
			seduce_view_camera_getf(NULL, camera);
			aim[0] -= camera[0];
			aim[1] -= camera[1];
			aim[2] -= camera[2];
			normal[0] = (float)snap[3];
			normal[1] = (float)snap[4];
			normal[2] = (float)snap[5];
			f_project3f(p, v, normal, camera, aim);
			f = sqrt((pos[0] - p[0]) * (pos[0] - p[0]) + (pos[1] - p[1]) * (pos[1] - p[1]) + (pos[2] - p[2]) * (pos[2] - p[2]));
			if(action)
				lo_pfx_wiggle_circle(p[0], p[1], p[2], normal[0], normal[1], normal[2], f, 1, action);
			else if(input->mode == BAM_DRAW)
				lo_pfx_select_circle(p[0], p[1], p[2], normal[0], normal[1], normal[2], f);
			normal[0] += p[0];
			normal[1] += p[1];
			normal[2] += p[2];
			p2[0] = pos[0];
			p2[1] = pos[1];
			p2[2] = pos[2];
			f_matrixxyf(matrix, p, p2, normal);
			for(i = 0; i < 12; i++)
				matrix[i] *= f;
			if(action)
			{
				for(x = -1; x < 1.001; x += 0.1)
				{
					for(y = -1; y < 1.001; y += 0.1)
					{
						if(x * x + y * y < 0.99)
						{
							f_transform3f(p, matrix, x - 0.01, 0, y);
							f_transform3f(p2, matrix, x + 0.01, 0, y);
							lo_pfx_wiggle_line_add(p[0], p[1], p[2], p2[0], p2[1], p2[2], TRUE);
							f_transform3f(p, matrix, x, 0, y - 0.01);
							f_transform3f(p2, matrix, x, 0, y + 0.01);
							lo_pfx_wiggle_line_add(p[0], p[1], p[2], p2[0], p2[1], p2[2], TRUE);
						}
					}
				}
			}else
			{
				for(x = -1; x < 1.001; x += 0.1)
				{
					for(y = -1; y < 1.001; y += 0.1)
					{
						if(x * x + y * y < 0.99)
						{
							f_transform3f(p, matrix, x - 0.01, 0, y);
							f_transform3f(p2, matrix, x + 0.01, 0, y);
							r_primitive_line_3d(p[0], p[1], p[2], p2[0], p2[1], p2[2], 0.5, 0.6, 0.7, 0.6);
							f_transform3f(p, matrix, x, 0, y - 0.01);
							f_transform3f(p2, matrix, x, 0, y + 0.01);
							r_primitive_line_3d(p[0], p[1], p[2], p2[0], p2[1], p2[2], 0.5, 0.6, 0.7, 0.6);
						}
					}
				}
				r_primitive_line_flush();
			}
		break;
	}
}


void lo_pfx_wiggle_draw(float delta)
{
	float f, invf, *array, size;
	uint i;
	uint test = 0;
	RMatrix *reset;

	reset = r_matrix_get();
	glBlendFunc(GL_ONE, GL_ONE);
	r_matrix_set(&la_world_matrix);
	r_shader_set(GlobalParticleData.wiggle_shader);
	size = seduce_view_distance_camera_get(NULL) * 0.01;

	for(i = 0; i < WIGGLE_BLOCK_COUNT; i++)
	{
		if(GlobalParticleData.wiggle_blocks[i].wiggle_draw_lenght != 0)
		{
			if(GlobalParticleData.wiggle_blocks[i].wiggle_major)
				GlobalParticleData.wiggle_blocks[i].wiggle_timer += delta * 0.5;
			else
				GlobalParticleData.wiggle_blocks[i].wiggle_timer += delta * 0.5 * 5;
		}	
		if(GlobalParticleData.wiggle_blocks[i].wiggle_used != 0)
		{
			r_array_load_vertex(GlobalParticleData.wiggle_pool, GlobalParticleData.wiggle_blocks[i].wiggle_section, GlobalParticleData.wiggle_blocks[i].wiggle_array, 0, GlobalParticleData.wiggle_blocks[i].wiggle_used * 2);
			GlobalParticleData.wiggle_blocks[i].wiggle_draw_lenght = GlobalParticleData.wiggle_blocks[i].wiggle_used * 2;
			GlobalParticleData.wiggle_blocks[i].wiggle_used = 0; 
			GlobalParticleData.wiggle_blocks[i].wiggle_timer = 0;
			free(GlobalParticleData.wiggle_blocks[i].wiggle_array);
			GlobalParticleData.wiggle_blocks[i].wiggle_array = NULL;
		}
		if(GlobalParticleData.wiggle_blocks[i].wiggle_timer > 1.0)
		{
			GlobalParticleData.wiggle_blocks[i].wiggle_draw_lenght = 0;
			GlobalParticleData.wiggle_blocks[i].wiggle_used = 0; 
			GlobalParticleData.wiggle_blocks[i].wiggle_timer = 0;
		}else
		{
			r_shader_float_set(GlobalParticleData.wiggle_shader, r_shader_uniform_location(GlobalParticleData.wiggle_shader, "expand1"), GlobalParticleData.wiggle_blocks[i].wiggle_timer);
			r_shader_float_set(GlobalParticleData.wiggle_shader, r_shader_uniform_location(GlobalParticleData.wiggle_shader, "expand2"), GlobalParticleData.wiggle_blocks[i].wiggle_timer * GlobalParticleData.wiggle_blocks[i].wiggle_timer * GlobalParticleData.wiggle_blocks[i].wiggle_timer * GlobalParticleData.wiggle_blocks[i].wiggle_timer);
		 
			if(GlobalParticleData.wiggle_blocks[i].wiggle_timer > 0.5)
			{
				r_shader_float_set(GlobalParticleData.wiggle_shader, r_shader_uniform_location(GlobalParticleData.wiggle_shader, "scale"), size * (1.0 - (GlobalParticleData.wiggle_blocks[i].wiggle_timer - 0.5) * 2.0));
				r_shader_float_set(GlobalParticleData.wiggle_shader, r_shader_uniform_location(GlobalParticleData.wiggle_shader, "cutoff"), size * (1.0 - (GlobalParticleData.wiggle_blocks[i].wiggle_timer - 0.5) * 2.0));
			}else
			{
				r_shader_float_set(GlobalParticleData.wiggle_shader, r_shader_uniform_location(GlobalParticleData.wiggle_shader, "scale"), size);
				r_shader_float_set(GlobalParticleData.wiggle_shader, r_shader_uniform_location(GlobalParticleData.wiggle_shader, "cutoff"), size);
			}
			if(GlobalParticleData.wiggle_blocks[i].wiggle_major)
				r_shader_vec4_set(GlobalParticleData.wiggle_shader, r_shader_uniform_location(GlobalParticleData.wiggle_shader, "color"), 0.5, 0.6, 0.7, 1.0);
			else
				r_shader_vec4_set(GlobalParticleData.wiggle_shader, r_shader_uniform_location(GlobalParticleData.wiggle_shader, "color"), 0.5, 0.6, 0.7, 0.3);

			r_array_section_draw(GlobalParticleData.wiggle_pool, GlobalParticleData.wiggle_blocks[i].wiggle_section, GL_LINES, 0, GlobalParticleData.wiggle_blocks[i].wiggle_draw_lenght);
		}
	}
	r_matrix_set(reset);
}



void la_pfx_test(float delta)
{
	RMatrix *reset;
	static float time = 1, age = 0;
	float a[3], b[3], a2[3] = {0, 0, 0}, b2[3] = {0, 0, 0}, f, t;
	uint i;

	lo_pfx_wiggle_draw(delta);

//	return;
				
	time += delta;

//	lo_pfx_select_circle(0, 0, 0, 1, 1, 1, 2.0);
/*	if(time > 0.5)
	{
		lo_pfx_wiggle_circle(0, 0, 0, 1, 1, 1, 2.0, 0);
		time = 0;
	}*/
	return;
	t = time * time;
	t = t * t;
	for(f = 0.01; f < 2; )
	{
		f_wiggle3df(a, f * 1.0 + age, time * 0.5);
		f_wiggle3df(a2, f * 7.0 + age, t * 0.1);
		a[0] += f;
		f += 0.02;
		f_wiggle3df(b, f * 1.0 + age, time * 0.5);
		f_wiggle3df(b2, f * 7.0 + age, t * 0.1);
		b[0] += f;
		r_primitive_line_3d(a[0] + a2[0], a[1] + a2[1], a[2] + a2[2],
							b[0] + b2[0], b[1] + b2[1], b[2] + b2[2], 1, 1, 1, 1);
	}
	r_primitive_line_flush();
	r_matrix_set(reset);
}