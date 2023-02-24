
#include <math.h>
#include <stdlib.h>
#include "forge.h"
#include "betray.h"
#include "relinquish.h"

extern double	seduce_view_distance_camera_get(void *v);

typedef struct{
	float color[4];
	float pos[4];
	float start; 
	float end;
	float thickness;
	float distance;
	uint major_seed;
	uint minor_seed;
}SeduceLineImageGenLine;


void seduce_line_image_line_init(SeduceLineImageGenLine *line, uint major_seed, uint minor_seed, float start, float offset, float thicknes, float clump)
{
	float f;
	uint primary;
	line->color[0] = line->color[1] = line->color[2] = 0;
	primary = major_seed % 3;
	if(f_randf(minor_seed++) > 0.8)
		primary = (major_seed + minor_seed) % 3;
	line->color[primary] = 0.6 + 0.5 * f_randf(minor_seed++);
	if(f_randf(minor_seed++) > 0.3)
	{
		f = f_randf(minor_seed++);
		line->color[(primary + 1 + minor_seed % 2) % 3] = f * f * line->color[primary];
	}

	line->start = start + f_randf(minor_seed++) * clump * 0.1;
	line->end = line->start + 0.05 + f_randf(major_seed++) * 0.3;
	f = line->end - line->end;

	line->pos[0] = f_randf(major_seed++) + f_randnf(minor_seed++) * clump * 0.1 + offset;
	line->pos[1] = line->pos[0] + (f_randnf(major_seed++) * 0.12 + f_randnf(minor_seed++) * 0.04 * clump) * 1.4;
	line->pos[2] = line->pos[0] + (f_randnf(major_seed++) * 0.12 + f_randnf(minor_seed++) * 0.04 * clump) * 1.4;
	line->pos[3] = line->pos[0] + (f_randnf(major_seed++) * 0.12 + f_randnf(minor_seed++) * 0.04 * clump) * 1.4;

	f = f_randf(minor_seed++);
	line->thickness =  3.7 + f * 42.0;
	line->thickness =  (0.25 + f * 0.75) * thicknes;
	line->major_seed = major_seed;
	line->minor_seed = minor_seed;
	line->distance = 0; 
}

boolean seduce_line_image_line_lookup(SeduceLineImageGenLine *line, float pos, float *place, float *thickness, float *distance, float *color)
{
	static uint seed = 0;
	float f;
	if(pos < line->start)
		return FALSE;
	*distance = line->distance++;
	if(pos > line->end)
	{
		f = (line->pos[3] - line->pos[2]) / (line->end - line->start);
		line->start = line->end;
		line->end = line->start + 0.15 + 0.2 * f_randf(line->major_seed++);
		line->pos[0] = line->pos[3];
		line->pos[1] = line->pos[0] + f * (line->end - line->start);
		line->pos[2] = line->pos[1] + (line->end - line->start) * f_randnf(line->major_seed++) * 0.12 + f_randnf(line->minor_seed++) * 0.08;
		line->pos[3] = line->pos[1] + (line->end - line->start) * f_randnf(line->major_seed++) * 0.12 + f_randnf(line->minor_seed++) * 0.08;
	}
	if(line->thickness < line->distance)
		*thickness = line->thickness;
	else
		*thickness = line->distance;
	f = f_splinef((pos - line->start) / (line->end - line->start), line->pos[0], line->pos[1], line->pos[2], line->pos[3]);
	while(f < 0.0)
		f++;
	while(f > 1.0)
		f--;
	color[0] = line->color[0] * 2.0;
	color[1] = line->color[1] * 2.0;
	color[2] = line->color[2] * 2.0;
	color[3] = line->color[3] * 2.0;
/*	if(line->end > 1.0)
	{
		pos = (pos - line->start) / (1.0 - line->start);
		*thickness += pos * 50.0;
		pos *= pos * 0.3;
		color[0] += pos;
		color[1] += pos;
		color[2] += pos;
		color[3] += pos;
	}*/
	*place = f;
	return TRUE;
}



void seduce_line_image_line_render(float *buffer, uint resolution, float pos, float thickness, float *color)
{
	float f;
	uint pixel, x_pos;
	pos *= (float)resolution;
	pos -= thickness * 0.5;
	f = pos - (float)((int)pos);
 	f = 1.0 - f;
	if(f > thickness)
		f = thickness;
	pos += resolution;
	x_pos = (uint)pos * 4;
	pixel = x_pos  % (resolution * 4);
	buffer[pixel + 0] += color[0] * f;
	buffer[pixel + 1] += color[1] * f;
	buffer[pixel + 2] += color[2] * f;
//	buffer[pixel + 3] += color[3] * f;
	x_pos = (x_pos + 4)  % (resolution * 4);
	pixel = x_pos;
	thickness -= f;
	while(thickness > 1.0)
	{
		buffer[pixel + 0] += color[0];
		buffer[pixel + 1] += color[1];
		buffer[pixel + 2] += color[2];
//		buffer[pixel + 3] += color[3];
		x_pos = (x_pos + 4)  % (resolution * 4);
		pixel = x_pos;
		thickness--;
	}
	buffer[pixel + 0] += color[0] * thickness;
	buffer[pixel + 1] += color[1] * thickness;
	buffer[pixel + 2] += color[2] * thickness;
//	buffer[pixel + 3] += color[3] * thickness;
}



void seduce_line_image_start_render(float *buffer, uint resolution, SeduceLineImageGenLine *line)
{
	float pos, thickness, distance, f, color[4];
	uint i, pixel, x_pos;
	for(i = 0; i < resolution / 4; i++)
	{
		if(seduce_line_image_line_lookup(line, (float)i / resolution, &pos, &thickness, &distance, color))
		{
			f = (float)distance / (float)resolution * 4;
		/*	color[0] *= f;
			color[1] *= f;
			color[2] *= f;*/
			f *= f;
			f *= f;
			color[3] = f;
			thickness += f * f * 100.0;
			f = (float)i / (float)(resolution / 4);
			if(f > 0.5)
			{
				f = (f - 0.5) * 2.0;
				f *= f;
				f *= f;
				thickness += f * 200.0;
			}			
	/*		color[0] /= 1.0 + thickness * 0.04;
			color[1] /= 1.0 + thickness * 0.04;
			color[2] /= 1.0 + thickness * 0.04;*/
			seduce_line_image_line_render(&buffer[i * 4 * resolution], resolution, pos, thickness, color);
	/*		pos *= (float)resolution;
			pos -= thickness * 0.5;
			f = pos - (float)((int)pos);
 			f = 1.0 - f;
			if(f > thickness)
				f = thickness;
			x_pos = (uint)pos * 4;
			pixel = x_pos + resolution * 4 * i;
			buffer[pixel + 0] += buffer[pixel + 0] + (color[0] - buffer[pixel + 0]) * f;
			buffer[pixel + 1] += buffer[pixel + 1] + (color[1] - buffer[pixel + 1]) * f;
			buffer[pixel + 2] += buffer[pixel + 2] + (color[2] - buffer[pixel + 2]) * f;
			buffer[pixel + 3] += buffer[pixel + 3] + (color[3] - buffer[pixel + 3]) * f;
			x_pos = (x_pos + 4)  % (resolution * 4);
			pixel = x_pos + resolution * 4 * i;
			thickness -= f;
			while(thickness > 1.0)
			{
				buffer[pixel + 0] += color[0];
				buffer[pixel + 1] += color[1];
				buffer[pixel + 2] += color[2];
				buffer[pixel + 3] += color[3];
				x_pos = (x_pos + 4)  % (resolution * 4);
				pixel = x_pos + resolution * 4 * i;
				thickness--;
			}
			buffer[pixel + 0] += buffer[pixel + 0] + (color[0] - buffer[pixel + 0]) * thickness;
			buffer[pixel + 1] += buffer[pixel + 1] + (color[1] - buffer[pixel + 1]) * thickness;
			buffer[pixel + 2] += buffer[pixel + 2] + (color[2] - buffer[pixel + 2]) * thickness;
			buffer[pixel + 3] += buffer[pixel + 3] + (color[3] - buffer[pixel + 3]) * thickness;*/
		}
	}
}

void seduce_line_image_continius_render(float *buffer, uint resolution_x, uint resolution_y, SeduceLineImageGenLine *line, uint pixel)
{
	float pos, thickness, distance, f, color[4];
	uint i,  x_pos;
	for(i = 0; i < resolution_y; i++)
	{
		if(seduce_line_image_line_lookup(line, (float)i / resolution_x, &pos, &thickness, &distance, color))
		{
			seduce_line_image_line_render(&buffer[((pixel + i) % resolution_y) * 4 * resolution_x], resolution_x, pos, thickness, color);
		}
	}
}

uint seduce_line_image_gen(uint resolution)
{
	SeduceLineImageGenLine line;
	float *buffer, f, f2;
	uint i, j, seeds[10] = {3, 5, 17, 9, 16, 73, 43, 102, 324, 76}; // 12 is cool
	static float offset[10] = {0.610417, -0.256250, 0.401042, 0.601042, 0.833333, 0, 0.1, 0.5, 0.7, 0.9};
	buffer = malloc((sizeof *buffer) * resolution * resolution * 4);
	for(i = 0; i < resolution * resolution * 4; i++)
		buffer[i] = 0.0;
	for(i = 0; i < 40; i++)
	{
		seduce_line_image_line_init(&line, i / 2, i, (float)(i / 2) / 80.0, (float)i / 40.0, 40, 1.0);
		seduce_line_image_start_render(buffer, resolution, &line);
	}
	for(i = resolution / 4 - 100; i < resolution; i++)
	{
		f = (float)(i - (resolution / 4 - 100)) / 100.0;
		f *= f;
		f *= f;
		if(f > 1.0)
			f = 1.0;
		for(j = 0; j < resolution * 4; j += 4)
			if(i >= resolution / 4)
				buffer[i * resolution * 4 + j + 3] = 1;
		f = 1 - f;
		for(j = 0; j < resolution * 4; j += 4)
		{
			buffer[i * resolution * 4 + j] *= f;
			buffer[i * resolution * 4 + j + 1] *= f;
			buffer[i * resolution * 4 + j + 2] *= f;
		}
	}
	for(i = 0; i < 40; i++)
	{
		f = f_randf(i);
		f *= f;
		f *= f;
		seduce_line_image_line_init(&line, i / 3, i, (float)(i / 3) / 80.0, (float)(i / 3) / 7.0, 5 + f * 20.0, 0.1);
		seduce_line_image_continius_render(&buffer[resolution * 4 * resolution / 4], resolution, resolution / 4 * 3, &line, i * resolution / 40);
	}

	f = 1.0 / 33.0;
	for(i = 0; i < 12; i++)
	{
		f2 = f_splinef(f, 0, 2.25, 0, 0);
		for(j = 0; j < resolution * 4; j++)
			if(buffer[(resolution / 4 - 32 + i) * resolution * 4 + j] < f2)
				buffer[(resolution / 4 - 32 + i) * resolution * 4 + j] = f2;
		f += 1.0 / 33.0;
	}
	for(; i < 32; i++)
	{
		f2 = f_splinef(f, 0, 2.25, 0, 0);
		for(j = 0; j < resolution * 4; j++)
			buffer[(resolution / 4 - 32 + i) * resolution * 4 + j] = f2;
		f += 1.0 / 33.0;
	}

	for(i = 0; i < 24; i++)
	{
		for(j = 0; j < resolution * 4; j++)
		{
			f2 = buffer[(resolution / 4 + 1) * resolution * 4 + j];
			if(buffer[(resolution / 4 - 20 + i) * resolution * 4 + j] < f2)
				buffer[(resolution / 4 - 20 + i) * resolution * 4 + j] = f2;
		}
	}
	j = resolution * 4 * (resolution * 100 / 425 - 1);
	for(i = 0; i < resolution * 4 * 2; i++)
		buffer[j++] = 255;
	i = r_texture_allocate(R_IF_RGBA_FLOAT32, resolution, resolution, 1, TRUE, TRUE, buffer);
	free(buffer);
	return i;
}

#define FLARE_TEXTURE_SIZE 256

uint seduce_line_bokeh_gen()
{
	float *data, f, a, b, c, d, x, y, scale, sum, base_sum, peek;
	uint i, j, k, texture_id;
	data = malloc((sizeof *data) * FLARE_TEXTURE_SIZE * FLARE_TEXTURE_SIZE * 3);
	for(i = 0; i < FLARE_TEXTURE_SIZE; i++)
	{
		for(j = 0; j < FLARE_TEXTURE_SIZE; j++)
		{
			for(k = 0; k < 3; k++)
			{
				y = (float)i / (float)(FLARE_TEXTURE_SIZE - 1);
				x = ((float)j - ((float)k * 3.6 - (3.6 * 2.0)) * (1 - y)) / (float)(FLARE_TEXTURE_SIZE - 2);
			//	x += 2.0 / FLARE_TEXTURE_SIZE;
				peek = f_splinef(y, 1, 1, 1.2, 2.0);
				if(x < 0)
					f = 0;
				else if(x * x < y)
				{
					f = x * x / y;
				//	f = f_splinef(f * f, 0, 0.2, 0.4, 1);
					f = f_splinef(f * f * f * f, 0, 0.04, 0.1, 1.0);
		//			if(k != 0)
		//				f = 0;
				}else
				{
					f = (x * x - y) / x * x;
					f = f_splinef(f, 1.0, 1.0, peek * 0.7, peek * 0.7);
		//			if(k != 1)
		//				f = 0;
				}
				data[((FLARE_TEXTURE_SIZE - i - 1) * FLARE_TEXTURE_SIZE + (FLARE_TEXTURE_SIZE - j - 1)) * 3 + k] = f;

			}
		}
	}
	for(i = 0; i < FLARE_TEXTURE_SIZE; i++)
	{
		f = 0;
		for(j = 0; j < FLARE_TEXTURE_SIZE * 3; j++)
			f += (float)(j + 1) * data[i * FLARE_TEXTURE_SIZE * 3 + j];
		f /= 1822.0;
		for(j = 0; j < FLARE_TEXTURE_SIZE * 3; j++)
			data[i * FLARE_TEXTURE_SIZE * 3 + j] /= f;
	}

	texture_id = r_texture_allocate(R_IF_RGB_FLOAT32, FLARE_TEXTURE_SIZE, FLARE_TEXTURE_SIZE, 1, TRUE, FALSE, data);
	for(i = 0; i < FLARE_TEXTURE_SIZE * FLARE_TEXTURE_SIZE * 3; i++)
		if(data[i] > 1.0 || data[i] < 0.0)
			data[i] = 1.0;

	free(data);
	return texture_id;
}

#define DIRT_TEXTURE_SIZE 512

void seduce_line_dirt_line_draw_channel(float *buffer, float x_start, float y_start, float x_end, float y_end, float brightness, float scale)
{
	float fi, fj, vec[2], f, x_normal, y_normal, length; 
	uint i, j, i_begin = 0, j_begin = 0, i_end = DIRT_TEXTURE_SIZE, j_end = DIRT_TEXTURE_SIZE;
	x_normal = x_end - x_start;
	y_normal = y_end - y_start;
	length = sqrt(x_normal * x_normal + y_normal * y_normal);
	x_normal /= length;
	y_normal /= length;
	f = x_start;
	if(x_end < f)
		f = x_end;
	if(f > 5.0 / scale)
		j_begin = (uint)(f - 5.0 / scale);
	f = x_start;
	if(x_end > f)
		f = x_end;
	if(f + 5.0 / scale < DIRT_TEXTURE_SIZE)
		j_end = (uint)(f + 5.0 / scale);
	f = y_start;
	if(y_end < f)
		f = y_end;
	if(f > 5.0 / scale)
		i_begin = (uint)(f - 5.0 / scale);
	f = y_start;
	if(y_end > f)
		f = y_end;
	if(f + 5.0 / scale < DIRT_TEXTURE_SIZE)
		i_end = (uint)(f + 5.0 / scale);
	for(i = i_begin; i < i_end; i++)
	{
		fi = (float)i;
		for(j = j_begin; j < j_end; j++)
		{
			fj = (float)j;
			vec[0] = fj - x_start;
			vec[1] = fi - y_start;
			f = vec[0] * x_normal + vec[1] * y_normal;
			if(f > 0)
			{
				if(f > length)
				{
					vec[0] = fj - x_end;
					vec[1] = fi - y_end;
				}else
				{
					vec[0] -= f * x_normal; 
					vec[1] -= f * y_normal; 
				}
			}
			vec[0] *= scale;
			vec[1] *= scale;
			f = 1.02 - brightness / (1 + vec[0] * vec[0] + vec[1] * vec[1]);
			if(f < buffer[(i * DIRT_TEXTURE_SIZE + j) * 3])
				buffer[(i * DIRT_TEXTURE_SIZE + j) * 3] = f;
		}
	}
}

void seduce_line_dirt_line_draw(float *buffer, float x_start, float y_start, float x_end, float y_end, float brightness, float scale)
{
	seduce_line_dirt_line_draw_channel(&buffer[0], (x_start - 0.5) * 0.998 + 0.5, 
									   (y_start - 0.5) * 0.998 + 0.5,
									   (x_end - 0.5) * 0.998 + 0.5,
									   (y_end - 0.5) * 0.998 + 0.5, brightness * 0.9, scale * 1.0);
	seduce_line_dirt_line_draw_channel(&buffer[1], x_start, y_start, x_end, y_end, brightness * 0.9, scale * 1.1);
	seduce_line_dirt_line_draw_channel(&buffer[2], (x_start - 0.5) * 1.004 + 0.5, 
									   (y_start - 0.5) * 1.004 + 0.5,
									   (x_end - 0.5) * 1.004 + 0.5,
									   (y_end - 0.5) * 1.004 + 0.5,  brightness * 0.9, scale * 0.9);
}

uint seduce_line_dirt_gen()
{
	float *data, x, y, x2, y2, spline[8], brightness, scale, f;
	uint i, j, k, texture_id;
	data = malloc((sizeof *data) * DIRT_TEXTURE_SIZE * DIRT_TEXTURE_SIZE * 3);
	for(i = 0; i < DIRT_TEXTURE_SIZE * DIRT_TEXTURE_SIZE * 3; i++)
		data[i] = 1.0;

	for(i = 0; i < 10; i++)
	{
		x2 = DIRT_TEXTURE_SIZE * f_randf(i * 8);
		y2 = DIRT_TEXTURE_SIZE * f_randf(i * 8 + 1);

		for(j = 0; j < 10; j++)
		{
			k = i * 80 + j * 8;
			x = x2 + f_randf(k) * 30.0;
			y = y2 + f_randf(k + 1) * 30.0;
			seduce_line_dirt_line_draw(data, x, y, x + f_randnf(k + 2) * 2.0, y + f_randnf(k + 3) * 2.0, 0.1 + 0.9 * f_randf(k + 4), 0.08 + 0.1 * f_randf(k + 5));
		}
	}
	for(i = 0; i < DIRT_TEXTURE_SIZE * DIRT_TEXTURE_SIZE * 3; i++)
	{
		if(data[i] < 0.5)
		{
			f = (0.5 - data[i]) * 4.0;
			f = 1.0 - (1.0 - f) * (1.0 - f);
			data[i] = f * 0.3 + 0.75 + f_randf(i) * 0.1;
		}else
		{
			f = (data[i] - 0.5) * 5.0;
			f = 0.75 + f * f;
			if(f > 1.0)
				data[i] = 1.0;
			else
				data[i] = f + f_randf(i) * 0.1;
		}
		/*	f = (0.5 - data[i]) * 2.0;
		f *= f;
		f *= f;
		data[i] = f;*/
	}

	for(i = 0; i < 100; i++)
	{
		x = DIRT_TEXTURE_SIZE * f_randf(i * 8);
		y = DIRT_TEXTURE_SIZE * f_randf(i * 8 + 1);
		seduce_line_dirt_line_draw(data, x, y, x + f_randnf(i * 8 + 2) * 2.0, y + f_randnf(i * 8 + 3) * 2.0, 0.1 + 0.9 * f_randf(i * 8 + 4), 0.8 + f_randf(i * 8 + 5));
		seduce_line_dirt_line_draw(data, x, y, x + f_randnf(i * 8 + 6) * 1.0, y + f_randnf(i * 8 + 7) * 1.0, 0.1 + 0.9 * f_randf(i * 8 + 4), 0.8 + f_randf(i * 8 + 5));
	}
	for(i = 0; i < 1000; i++)
	{
		x = DIRT_TEXTURE_SIZE * f_randf(i * 6);
		y = DIRT_TEXTURE_SIZE * f_randf(i * 6 + 1);
		seduce_line_dirt_line_draw(data, x, y, x  + f_randnf(i * 6 + 2) * 1.0, y  + f_randnf(i * 6 + 3) * 1.0, 0.0 + 0.3 * f_randf(i * 6 + 4), 1.0 + f_randf(i * 6 + 5));
	}

	for(i = 0; i < 30; i++)
	{
		scale = f_randf(i * 11 + 43);
		scale = 2.0 + 10.0 * scale * scale * scale * scale;
		spline[0] = DIRT_TEXTURE_SIZE * (0.2 + 0.6 * f_randf(i * 11 + 32));
		spline[1] = DIRT_TEXTURE_SIZE * (0.2 + 0.6 * f_randf(i * 10 + 54));
		spline[2] = spline[0] + scale * f_randnf(i * 10 + 2);
		spline[3] = spline[1] + scale * f_randnf(i * 10 + 3);
		spline[4] = spline[2] * 2 - spline[0] + scale * f_randnf(i * 10 + 4);
		spline[5] = spline[3] * 2 - spline[1] + scale * f_randnf(i * 10 + 5);
		spline[6] = spline[4] * 2 - spline[2] + scale * f_randnf(i * 10 + 6);
		spline[7] = spline[5] * 2 - spline[3] + scale * f_randnf(i * 10 + 7);
		brightness = 0.0 + 0.3 * f_randf(i * 10 + 8);
		scale = 1.0 + f_randf(i * 10 + 9);
		x = spline[0];
		y = spline[1];
		for(f = 0.1; f < 1.0; f += 0.04)
		{
			x2 = f_splinef(f, spline[0], spline[2], spline[4], spline[6]);
			y2 = f_splinef(f, spline[1], spline[3], spline[5], spline[7]);
			seduce_line_dirt_line_draw(data, x, y, x2, y2, brightness * (0.5 + 0.5 * f), scale);
			x = x2;
			y = y2;
		}
	}
	texture_id = r_texture_allocate(R_IF_RGB_FLOAT32, DIRT_TEXTURE_SIZE, DIRT_TEXTURE_SIZE, 1, TRUE, FALSE, data);
	free(data);
	return texture_id;
}


extern uint particle_debug_texture_id;

extern uint seduce_particle_texture_get();

void sediuce_line_image_test()
{
	BInputState *input;
	static uint texture_id = -1;
	return;
	input = betray_get_input_state();
	if(texture_id == -1)
		texture_id = seduce_line_image_gen(1024);
	r_primitive_image(0.2, -0.4, 0.1, 0.8, 0.8, 0.0, 0.0, 1.0, 1.0,/* seduce_particle_texture_get() */texture_id, 1, 1, 1, 1);
	r_primitive_image(-0.5, 0.45, 0.1, 1.0, 0.001, 1.0, input->pointers[0].pointer_x, 0.0, input->pointers[0].pointer_x, texture_id, 1, 1, 1, 1);
	r_primitive_image(-0.5, -0.45, 0.1, 1.0, 0.001, 1.0, input->minute_time * 10.0, 0.0, input->minute_time * 10.0, texture_id, 1, 1, 1, 1);
}

typedef struct{
	RShader *circle_shader;
	uint circle_shader_location_start;
	uint circle_shader_location_size;
	uint circle_shader_location_add;
	uint circle_shader_location_multiply;
	uint circle_shader_location_time;
	void *circle_pool;
	uint texture_id;
}SeduceLineDraw;

SeduceLineDraw seduce_line_draw;

char *seduce_circle_shader_vertex = 
"attribute vec2 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"uniform float start;"
"uniform float size;"
"uniform vec2 add;"
"uniform float time;"
"uniform vec2 multiply;"
"varying vec2 uv;"
"void main()"
"{"
"	vec4 center;"
"	center = ModelViewProjectionMatrix * vec4(0.0, 0.0, 0.0, 1.0);"
"	gl_Position = ModelViewProjectionMatrix * vec4(sin(start + vertex.x * size), cos(start + vertex.x * size), 0.0, 1.0);"
"	uv.x = add.x + vertex.y * multiply.x / center.z;"
"	uv.y = max(min(add.y + 0.02 * sin(vertex.y + time), 1.0), 0.0);"
"}";

 //min(1.0, max(add.y + sin(time + vertex * 7.0) * multiply.y, 0.0));

char *seduce_circle_shader_fragment = 
"uniform sampler2D tex;\n"
"varying vec2 uv;"
"void main()"
"{"
"	gl_FragColor = texture2D(tex, uv);"
"}";

#define SEDUCE_LINE_CIRCLE_SECTIONS_MAX 128

void seduce_draw_line_init()
{
	RFormats vertex_format_types[1] = {R_FLOAT};
	char buffer[2048];
	float *array;
	uint i, size = 2;
	seduce_line_draw.texture_id = seduce_line_image_gen(512);
	seduce_line_draw.circle_shader = r_shader_create_simple(buffer, 2048, seduce_circle_shader_vertex, seduce_circle_shader_fragment, "fade primitive");
	r_shader_texture_set(seduce_line_draw.circle_shader, 0, seduce_line_draw.texture_id); 
	seduce_line_draw.circle_shader_location_start = r_shader_uniform_location(seduce_line_draw.circle_shader, "start");
	seduce_line_draw.circle_shader_location_size = r_shader_uniform_location(seduce_line_draw.circle_shader, "size");
	seduce_line_draw.circle_shader_location_add = r_shader_uniform_location(seduce_line_draw.circle_shader, "add");
	seduce_line_draw.circle_shader_location_multiply = r_shader_uniform_location(seduce_line_draw.circle_shader, "multiply");
	seduce_line_draw.circle_shader_location_time = r_shader_uniform_location(seduce_line_draw.circle_shader, "time");
	r_shader_state_set_blend_mode(seduce_line_draw.circle_shader, GL_ONE, GL_ONE);
	r_shader_state_set_depth_test(seduce_line_draw.circle_shader, GL_ALWAYS);
	

	array = malloc((sizeof *array) * 4 * SEDUCE_LINE_CIRCLE_SECTIONS_MAX);
	for(i = 0; i < SEDUCE_LINE_CIRCLE_SECTIONS_MAX; i++)
	{
		array[i * 4 + 0] = 2.0 * PI * (float)i / (float)SEDUCE_LINE_CIRCLE_SECTIONS_MAX;
		array[i * 4 + 2] = 2.0 * PI * (float)(i + 1) / (float)SEDUCE_LINE_CIRCLE_SECTIONS_MAX;
	}
	for(i = 0; i < SEDUCE_LINE_CIRCLE_SECTIONS_MAX / 2; i++)
	{
		array[i * 4 + 1] = 2.0 * PI * (float)i / (float)SEDUCE_LINE_CIRCLE_SECTIONS_MAX - PI;
		array[i * 4 + 3] = 2.0 * PI * (float)(i + 1) / (float)SEDUCE_LINE_CIRCLE_SECTIONS_MAX - PI;
	}
	for(; i < SEDUCE_LINE_CIRCLE_SECTIONS_MAX; i++)
	{
		array[i * 4 + 1] = 2.0 * PI * (float)(i - SEDUCE_LINE_CIRCLE_SECTIONS_MAX / 2) / (float)SEDUCE_LINE_CIRCLE_SECTIONS_MAX - PI;
		array[i * 4 + 3] = 2.0 * PI * (float)(i + 1 - SEDUCE_LINE_CIRCLE_SECTIONS_MAX / 2) / (float)SEDUCE_LINE_CIRCLE_SECTIONS_MAX - PI;
	}

	seduce_line_draw.circle_pool = r_array_allocate(SEDUCE_LINE_CIRCLE_SECTIONS_MAX * 2, vertex_format_types, &size, 1, 0);
	r_array_load_vertex(seduce_line_draw.circle_pool, NULL, array, 0, SEDUCE_LINE_CIRCLE_SECTIONS_MAX * 2);

}

void seduce_draw_circle_new(float x, float y, float z,
					float x_normal, float y_normal, float z_normal, float radius, float start, float end, 
					float color_r, float color_g, float color_b, float color_a, float timer)
{
	uint i, count;
	float axis[3], other[3], m[16], cur_x, cur_y, last_x, last_y;
	BInputState *input;
	input = betray_get_input_state();
	count = 64;
	axis[0] = x_normal;
	axis[1] = y_normal;
	axis[2] = z_normal;
	other[0] = y_normal;
	other[1] = -x_normal;
	other[2] = -z_normal;
	f_matrixzyf(m, NULL, axis, other);
	m[12] = x;
	m[13] = y;
	m[14] = z;
	r_matrix_push(NULL);
	r_matrix_matrix_mult(NULL, m);
	r_matrix_scale(NULL, radius, radius, radius);
	r_shader_set(seduce_line_draw.circle_shader);
	r_shader_float_set(seduce_line_draw.circle_shader, seduce_line_draw.circle_shader_location_start, start);
	r_shader_float_set(seduce_line_draw.circle_shader, seduce_line_draw.circle_shader_location_size, end - start);
	r_shader_vec2_set(seduce_line_draw.circle_shader, seduce_line_draw.circle_shader_location_add, radius * 1.37, 0.2 * sin(input->minute_time * PI * 2.0) + input->pointers[0].pointer_x);
	r_shader_float_set(seduce_line_draw.circle_shader, seduce_line_draw.circle_shader_location_time, input->minute_time * 30.0);
	r_shader_vec2_set(seduce_line_draw.circle_shader, seduce_line_draw.circle_shader_location_multiply, 0.4 * radius, input->pointers[0].pointer_x);
	r_array_section_draw(seduce_line_draw.circle_pool, NULL, R_PRIMITIVE_LINES, 0, 2 * SEDUCE_LINE_CIRCLE_SECTIONS_MAX);	
	r_primitive_line_flush();
	r_matrix_pop(NULL);
}

void seduce_draw_circle(float x, float y, float z,
					float x_normal, float y_normal, float z_normal, float radius, float start, float end, 
					float color_r, float color_g, float color_b, float color_a, float timer)
{
	uint i, count;
	float axis[3], other[3], m[16], cur_x, cur_y, last_x, last_y;
	count = 64;
	axis[0] = x_normal;
	axis[1] = y_normal;
	axis[2] = z_normal;
	other[0] = y_normal;
	other[1] = -x_normal;
	other[2] = -z_normal;
	f_matrixzyf(m, NULL, axis, other);
	m[12] = x;
	m[13] = y;
	m[14] = z;
	r_matrix_push(NULL);
	r_matrix_matrix_mult(NULL, m);
	last_x = sin(((end - start) + start) * PI * 2.0);
	last_y = cos(((end - start) + start) * PI * 2.0);
	for(i = 0; i < count;)
	{
		i++;
		cur_x = sin(((float)i / (float)count * (end - start) + start) * PI * 2.0);
		cur_y = cos(((float)i / (float)count * (end - start) + start) * PI * 2.0);
		r_primitive_line_2d(cur_x * radius, cur_y * radius, last_x * radius, last_y * radius, color_r, color_g, color_b, color_a);
		last_x = cur_x;
		last_y = cur_y;
	}
	r_primitive_line_flush();
	r_matrix_pop(NULL);
}


void seduce_draw_spline(float x_a, float y_a, float z_a,
						float x_b, float y_b, float z_b,
						float x_c, float y_c, float z_c,
						float x_d, float y_d, float z_d,
						float start, float end, 
						float color_r, float color_g, float color_b, float color_a, float timer)
{
	uint i, count;
	float f = 0, f2 = 0, add;
	f = f2 = start;
	count = 24; 
	add = (end - start) * timer * 1.0 / (float)count;
	for(i = 0; i < count; i++)
	{
		f += add;
		r_primitive_line_3d(f_splinef(f, x_a, x_b, x_c, x_d),
							f_splinef(f, x_a, x_b, x_c, x_d),
							f_splinef(f, x_a, x_b, x_c, x_d),
							f_splinef(f2, x_a, x_b, x_c, x_d),
							f_splinef(f2, x_a, x_b, x_c, x_d),
							f_splinef(f2, x_a, x_b, x_c, x_d),
							color_r, color_g, color_b, color_a);
		f2 = f;
	}
	r_primitive_line_flush();
	r_matrix_pop(NULL);
}

char *stellar_line_expand_vertex = 
"uniform mat4 ModelViewMatrix;\n"
"attribute vec4 vertex;\n"
"attribute vec4 color;\n"
"uniform vec4 base_color;\n"
"uniform float distance;\n"
"uniform float scroll;"
"varying vec4 col;\n"
"varying float coord;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = ModelViewMatrix * vec4(vertex.xyz, 1.0);\n"
"	coord = scroll + length(vertex.xyz) / distance;\n"
"	col = color * base_color;\n"
"}\n";

char *stellar_line_expand_geometry = 
"layout(lines) in;\n"
"layout(triangle_strip, max_vertices = 8) out;\n"
"uniform mat4 ProjectionMatrix;\n"
"uniform vec3 camera;\n"
"uniform float distance;\n"
"uniform float test;\n"
"uniform float resolution;\n"
"varying float coord[1];\n"
"varying vec4 col[1];\n"
"varying vec4 uv;\n"
"varying vec4 pixel;\n"
"varying float depth;\n"
"varying float u;\n"
"void main()\n"
"{\n"
"		vec3 expand;\n"
"		vec3 vector, camera_vec, normal;\n"
"		vec4 tmp;\n"
"		float x, depth_a, depth_b, scale, dist;\n"
"		scale = test;\n"
"		depth_a = 0.4 * (gl_in[0].gl_Position.z / distance + 1.0);\n"
"		depth_b = 0.4 * (gl_in[1].gl_Position.z / distance + 1.0);\n"
"		scale = max(abs(depth_a) + 0.05, abs(depth_b) + 0.05);\n"
"		vector = normalize(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz);\n"
"		expand = normalize(cross(normalize(gl_in[0].gl_Position.xyz), vector)) * vec3(resolution * distance) * vec3(scale);\n"
"		dist =  0.5 + 0.5 * length(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz) / (resolution * scale);\n"
"		vector = vector * vec3(0.02) * vec3(scale);\n"

"		pixel = col[0];\n"
"		u = coord[0];\n"
"		uv = vec4(scale, scale, 0, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position + vec4(expand + vector, 0.0));\n"
"		depth = depth_a;"
"		EmitVertex();\n"

"		pixel = col[0];\n"
"		u = coord[0];\n"
"		uv = vec4(scale, -scale, 0, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position - vec4(expand - vector, 0.0));\n"
"		depth = depth_a;"
"		EmitVertex();\n"

"		pixel = col[0];\n"
"		u = coord[0];\n"
"		uv = vec4(0, scale, 0.5, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position + vec4(expand, 0.0));\n"
"		depth = depth_a;"
"		EmitVertex();\n"

"		pixel = col[0];\n"
"		u = coord[0];\n"
"		uv = vec4(0, -scale, 0.5, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position - vec4(expand, 0.0));\n"
"		depth = depth_a;"
"		EmitVertex();\n"

"		pixel = col[1];\n"
"		u = coord[1];\n"
"		uv = vec4(0, scale, dist, 0.5);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position + vec4(expand, 0.0));\n"
"		depth = depth_b;"
"		EmitVertex();\n"

"		pixel = col[1];\n"
"		u = coord[1];\n"
"		uv = vec4(0, -scale, dist, 0.5);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position - vec4(expand, 0.0));\n"
"		depth = depth_b;"
"		EmitVertex();\n"

"		pixel = col[1];\n"
"		u = coord[1];\n"
"		uv = vec4(-scale, scale, dist, 0);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position + vec4(expand - vector, 0.0));\n"
"		depth = depth_b;"
"		EmitVertex();\n"

"		pixel = col[1];\n"
"		u = coord[1];\n"
"		uv = vec4(-scale, -scale, dist, 0);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position - vec4(expand + vector, 0.0));\n"
"		depth = depth_b;"
"		EmitVertex();\n"

"		EndPrimitive();\n"
"}\n";


char *stellar_line_expand_geometry_test = 
"layout(lines) in;\n"
"layout(triangle_strip, max_vertices = 8) out;\n"
"uniform mat4 ProjectionMatrix;\n"
"uniform vec3 camera;\n"
"uniform float distance;\n"
"uniform float test;\n"
"varying float coord[1];\n"
"varying vec4 col[1];\n"
"varying vec4 uv;\n"
//"varying vec4 pixel;\n"
//"varying float depth;\n"
//"varying float u;\n"
"void main()\n"
"{\n"
"		vec3 expand;\n"
"		vec3 vector, camera_vec, normal;\n"
"		vec4 tmp;\n"
"		float x, depth_a, depth_b, scale, dist;\n"
"		scale = test;\n"
"		depth_a = 0.4 * (gl_in[0].gl_Position.z / distance + 1.0);\n"
"		depth_b = 0.4 * (gl_in[1].gl_Position.z / distance + 1.0);\n"
"		scale = max(abs(depth_a) + 0.05, abs(depth_b) + 0.05);\n"
"		vector = normalize(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz);\n"
"		expand = normalize(cross(normalize(gl_in[0].gl_Position.xyz), vector)) * vec3(0.02 * distance) * vec3(scale);\n"
"		dist =  0.5 + 0.5 * length(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz) / (0.02 * scale);\n"
"		vector = vector * vec3(0.02) * vec3(scale);\n"


//"		pixel = col[0];\n"
//"		u = coord[0];\n"
"		uv = vec4(scale, scale, 0, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position + vec4(expand + vector, 0.0));\n"
//"		depth = depth_a;"
"		EmitVertex();\n"

//"		pixel = col[0];\n"
//"		u = coord[0];\n"
"		uv = vec4(scale, -scale, 0, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position - vec4(expand - vector, 0.0));\n"
//"		depth = depth_a;"
"		EmitVertex();\n"

//"		pixel = col[0];\n"
//"		u = coord[0];\n"
"		uv = vec4(0, scale, 0.5, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position + vec4(expand, 0.0));\n"
//"		depth = depth_a;"
"		EmitVertex();\n"

//"		pixel = col[0];\n"
//"		u = coord[0];\n"
"		uv = vec4(0, -scale, 0.5, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position - vec4(expand, 0.0));\n"
//"		depth = depth_a;"
"		EmitVertex();\n"

//"		pixel = col[1];\n"
//"		u = coord[1];\n"
"		uv = vec4(0, scale, dist, 0.5);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position + vec4(expand, 0.0));\n"
//"		depth = depth_b;"
"		EmitVertex();\n"

//"		pixel = col[1];\n"
//"		u = coord[1];\n"
"		uv = vec4(0, -scale, dist, 0.5);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position - vec4(expand, 0.0));\n"
//"		depth = depth_b;"
"		EmitVertex();\n"

//"		pixel = col[1];\n"
//"		u = coord[1];\n"
"		uv = vec4(-scale, scale, dist, 0);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position + vec4(expand - vector, 0.0));\n"
//"		depth = depth_b;"
"		EmitVertex();\n"

//"		pixel = col[1];\n"
//"		u = coord[1];\n"
"		uv = vec4(-scale, -scale, dist, 0);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position - vec4(expand + vector, 0.0));\n"
//"		depth = depth_b;"
"		EmitVertex();\n"

"		EndPrimitive();\n"
"}\n";

char *stellar_line_expand_fragment_test = 
"uniform sampler2D image;\n"
"uniform float time;"
"uniform mat3 color_space;"
"varying vec4 uv;\n"
//"varying vec4 pixel;\n"
//"varying float depth;\n"
//"varying float u;\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = vec4(1.0, 0, 0, 1);\n"
"}\n";

char *stellar_line_expand_fragment = 
"uniform sampler2D image;\n"
"uniform float time;"
"uniform mat3 color_space;"
"varying vec4 pixel;\n"
"varying vec4 uv;\n"
"varying float depth;\n"
"varying float u;\n"
"\n"
"void main()\n"
"{\n"
"	float f, dist;"
"	vec4 tex;"
"	f = abs(depth) + 0.05;"
"	dist = abs(uv.y);"
"	dist = min(dist, f);\n"
"	dist /= f;"
"	dist = 1.0 - dist * dist * (3.0 - 2.0 * dist);"
"	f = 1.0 / (1.0 + f * 4.0);"
"	f *= f;"
"	tex = texture2D(image, vec2(u, time));"
"	tex = vec4(0.5);"
"	gl_FragColor = vec4(min(min(uv.b, uv.a), 1.0)) * vec4(dist * dist * f * f) * max(tex.aaaa * pixel, vec4(tex.rgb * color_space, pixel.a));\n"
"}\n";

char *stellar_circle_expand_vertex = 
"uniform mat4 ModelViewMatrix;\n"
"attribute vec4 center;\n" // alpha is radius
"attribute vec4 vector;\n" // alpha is start
"attribute vec4 normal;\n" // alpha is end
"attribute vec4 color_start;\n"
"attribute vec4 color_end;\n"
"attribute vec2 mapping;\n"
"varying vec3 vec_sin;\n"
"varying vec3 vec_cos;\n"
"varying vec4 col_base;\n"
"varying vec4 col_add;\n"
"varying float u_base;\n"
"varying float u_add;\n"
"varying float start;\n"
"varying float end;\n"
"varying float add;\n"
"uniform vec4 base_color;\n"
"uniform float scroll;"
"\n"
"void main()\n"
"{\n"
"	vec4 tmp;"
"	vec3 vec, n;"
"	float divides;"
"	n = normalize(normal.xyz);\n"
"	vec = normalize(vector.xyz - n.xyz * dot(vector.xyz, n.xyz));\n"
"	tmp = ModelViewMatrix * vec4(center.xyz, 1.0);\n"
"	divides = %f;\n"
"	gl_Position = tmp;\n"
"	tmp = ModelViewMatrix * vec4(vec * center.aaa, 0.0);\n"
"	vec_cos = tmp.xyz;\n"
"	tmp = ModelViewMatrix * vec4(cross(vec, n) * center.aaa, 0.0);\n"
"	vec_sin = tmp.xyz;\n"
"	col_base = color_start * base_color;\n"
"	col_add = (color_end - color_start) / vec4(divides) * base_color;\n"
"	start = vector.a * 3.141592653 * 2.0;\n"
"	add = (3.141592653 * 2.0) * normal.a / divides;\n"
"	end = vector.a + normal.a * 3.141592653 * 2.0 + add * 0.5 + add;\n"
"	u_base = mapping.x + scroll;"
"	u_add = mapping.y / divides;\n"
"}\n";

char *stellar_circle_expand_geometry = 
"layout(points) in;\n"
"layout(triangle_strip, max_vertices = %u) out;\n"
"uniform mat4 ProjectionMatrix;\n"
"uniform float distance;\n"
"uniform float resolution;\n"
"varying vec3 vec_sin;\n"
"varying vec3 vec_cos;\n"
"varying vec4 col_base;\n"
"varying vec4 col_add;\n"
"varying float u_base;\n"
"varying float u_add;\n"
"varying float start;\n"
"varying float end;\n"
"varying float add;\n"
"varying vec4 pixel;\n"
"varying float depth;\n"
"varying vec2 uv;\n"
"varying float u;\n"
"void main()\n"
"{\n"
"	vec4 prev, pos_a, pos_b, next, color;\n"
"	vec3 vector, camera_vec, expand, last_expand;\n"
"	int i;\n"
"	float s, depth_a, depth_b, u_accumilator;\n"
"	last_expand = vec3(1, 1, 1);"
"	color = col_base[0];"
"	s = start[0] - add[0];\n"
"	prev = vec4(gl_in[0].gl_Position.xyz + vec_sin[0] * vec3(sin(s)) + vec_cos[0] * vec3(cos(s)), 1);\n"
"	s = start[0];\n"
"	pos_a = vec4(gl_in[0].gl_Position.xyz + vec_sin[0] * vec3(sin(s)) + vec_cos[0] * vec3(cos(s)), 1);\n"
"	s += add[0];\n"
"	pos_b = vec4(gl_in[0].gl_Position.xyz + vec_sin[0] * vec3(sin(s)) + vec_cos[0] * vec3(cos(s)), 1);\n"
"	depth_a = 0.4 * (pos_a.z / distance + 1.0);\n"
"	vector = normalize(pos_b.xyz - prev.xyz);\n"
"	last_expand = normalize(cross(normalize(pos_a.xyz), vector)) * vec3(resolution * distance);\n"
"	next = vec4(0, 0, 0, 1);\n"
"	u_accumilator = u_base[0];\n"
"	for(i = 0; i < %u; i++)\n"
"	{\n"
"		float scale;"
"		s += add[0];\n"
"		next.xyz = gl_in[0].gl_Position.xyz + vec_sin[0] * vec3(sin(s)) + vec_cos[0] * vec3(cos(s));\n"
"		depth_b = 0.4 * (pos_b.z / distance + 1.0);\n"
"		scale = max(abs(depth_a) + 0.5, abs(depth_b) + 0.5);\n"
"		vector = normalize(next.xyz - pos_a.xyz);\n"
"		expand = normalize(cross(normalize(pos_b.xyz), vector)) * vec3(resolution * distance);\n"
"		u = u_accumilator;\n"
"		pixel = color;\n"
"		depth = depth_a;\n"
"		uv = vec2(scale, 0);\n"
"		gl_Position = ProjectionMatrix * (pos_a - vec4(last_expand * vec3(scale), 0.0));\n"
"		EmitVertex();\n"
"		u = u_accumilator;\n"
"		pixel = color;\n"
"		depth = depth_a;\n"
"		uv = vec2(-scale, 0);\n"
"		gl_Position = ProjectionMatrix * (pos_a + vec4(last_expand * vec3(scale), 0.0));\n"
"		EmitVertex();\n"
"		color += col_add[0];"
"		u_accumilator += u_add[0];\n"
"		last_expand = expand;\n"
"		depth_a = depth_b;\n"
"		prev = pos_a;\n"
"		pos_a = pos_b;\n"
"		pos_b = next;\n"
"	}\n"
"	EndPrimitive();\n"
"}\n";


char *stellar_circle_expand_fragment = 
"uniform sampler2D image;\n"
"uniform mat3 color_space;"
"varying vec4 pixel;\n"
"varying float depth;\n"
"varying vec2 uv;\n"
"uniform float time;"
"varying float u;\n"
"void main()\n"
"{\n"
"	float f, dist;\n"
"	vec4 tex;\n"
"	f = abs(depth) + 0.05;\n"
"	dist = length(uv)\n;"
"	if(dist > f)\n"
"		discard;\n"
"	dist /= f;\n"
"	dist = 1.0 - dist * dist * (3.0 - 2.0 * dist);\n"
"	f = 1.0 / (1.0 + f * 4.0);\n"
"	f *= f;\n"
"	tex = texture2D(image, vec2(u, time));"
"	f = dist * dist * f * f;\n"
"	gl_FragColor = vec4(f) * max(tex.aaaa * pixel, vec4(tex.rgb * color_space, pixel.a));\n"
"}\n";


char *stellar_spline_expand_vertex = 
"uniform mat4 ModelViewMatrix;\n"
"attribute vec4 pos_a;\n" // alpha is pos_d x
"attribute vec4 pos_b;\n" // alpha is pos_d y
"attribute vec4 pos_c;\n" // alpha is pos_d z
"attribute vec4 color_start;\n"
"attribute vec4 color_end;\n"
"attribute vec2 mapping;\n"
"uniform float scroll;"
"uniform vec4 base_color;"
"varying vec3 cv_b;\n"
"varying vec3 cv_c;\n"
"varying vec3 cv_d;\n"
"varying vec4 col_base;\n"
"varying vec4 col_add;\n"
"varying float u_base;\n"
"varying float u_add;\n"
"\n"
"void main()\n"
"{\n"
"	vec4 tmp;\n"

"	tmp = ModelViewMatrix * vec4(pos_a.xyz, 1.0);\n"
"	gl_Position = tmp;\n"
"	tmp = ModelViewMatrix * vec4(pos_b.xyz, 1.0);\n"
"	cv_b = tmp.xyz;\n"
"	tmp = ModelViewMatrix * vec4(pos_c.xyz, 1.0);\n"
"	cv_c = tmp.xyz;\n"
"	tmp = ModelViewMatrix * vec4(pos_a.a, pos_b.a, pos_c.a, 1.0);\n"
"	cv_d = tmp.xyz;\n"
"	col_base = color_start * base_color;\n"
"	col_add = (color_end - color_start) / 127.0 * base_color;\n"
"	u_base = scroll + mapping.x;\n"
"	u_add = mapping.y / %f;\n"
"}\n";

char *stellar_spline_expand_geometry = 
"layout(points) in;\n"
"layout(triangle_strip, max_vertices = %u) out;\n"
"uniform mat4 ProjectionMatrix;\n"
"uniform float distance;\n"
"varying vec3 cv_b;\n"
"varying vec3 cv_c;\n"
"varying vec3 cv_d;\n"
"varying vec4 col_base;\n"
"varying vec4 col_add;\n"
"varying vec4 pixel;\n"
"varying float depth;\n"
"varying float u;\n"
"varying vec2 uv;\n"
"varying float u_base;\n"
"varying float u_add;\n"
"void main()\n"
"{\n"
"	vec4 prev, pos_a, pos_b, next, color;\n"
"	vec3 vector, camera_vec, expand, last_expand, mid;\n"
"	int i;\n"
"	float f, depth_a, depth_b, u_accumilator;\n"
"	last_expand = vec3(1, 1, 1);\n"
"	color = col_base[0];\n"
"	mid = mix(cv_b[0], cv_c[0], -1.0 / 127.0);\n"
"	prev.xyz = mix(mix(mix(gl_in[0].gl_Position.xyz, cv_b[0], -1.0 / 127.0), mid, -1.0 / 127.0), mix(mid, mix(cv_c[0], cv_d[0], -1.0 / 127.0), -1.0 / 127.0), -1.0 / 127.0);\n"
"	pos_a = vec4(gl_in[0].gl_Position.xyz, 1.0);\n"
"	mid = mix(cv_b[0], cv_c[0], 1.0 / 127.0);\n"
"	pos_b.xyz = mix(mix(mix(gl_in[0].gl_Position.xyz, cv_b[0], 1.0 / 127.0), mid, 1.0 / 127.0), mix(mid, mix(cv_c[0], cv_d[0], 1.0 / 127.0), 1.0 / 127.0), 1.0 / 127.0);\n"
"	depth_a = 0.4 * (pos_a.z / distance + 1.0);\n"
"	f = 1.0 / 127.0;\n"
"	vector = normalize(pos_b.xyz - prev.xyz);\n"
"	last_expand = normalize(cross(normalize(pos_a.xyz), vector)) * vec3(2.0 * distance);\n"
"	next = vec4(0, 0, 0, 1);\n"
"	u_accumilator = u_base[0];\n"
"	while(f < 1.001)\n"
"	{\n"
"		float scale;"
"		f += 1.0 / 127.0;\n"
"		mid = mix(cv_b[0], cv_c[0], f);\n"
"		next.xyz = mix(mix(mix(gl_in[0].gl_Position.xyz, cv_b[0], f), mid, f), mix(mid, mix(cv_c[0], cv_d[0], f), f), f);\n"
"		depth_b = 0.4 * (pos_b.z / distance + 1.0);\n"
"		scale = max(abs(depth_a) + 0.05, abs(depth_b) + 0.05);\n"
"		vector = normalize(next.xyz - pos_a.xyz);\n"
"		expand = normalize(cross(normalize(pos_b.xyz), vector)) * vec3(2.0 * distance);\n"
"		u = u_accumilator;\n"
"		pixel = color;\n"
"		depth = depth_a;\n"
"		uv = vec2(1, 0);\n"
//"		uv = vec2(scale, 0);\n"
"		gl_Position = ProjectionMatrix * (pos_a - vec4(last_expand * vec3(scale), 0.0));\n"
"		EmitVertex();\n"
"		u = u_accumilator;\n"
"		pixel = color;\n"
"		depth = depth_a;\n"
"		uv = vec2(-1, 0);\n"
//"		uv = vec2(-scale, 0);\n"
"		gl_Position = ProjectionMatrix * (pos_a + vec4(last_expand * vec3(scale), 0.0));\n"
"		EmitVertex();\n"
"		color += col_add[0];"
"		u_accumilator += u_add[0];\n"
"		last_expand = expand;\n"
"		depth_a = depth_b;\n"
"		prev = pos_a;\n"
"		pos_a = pos_b;\n"
"		pos_b = next;\n"
"	}\n"
"	EndPrimitive();\n"
"}\n";

char *stellar_bokhe_vertex = 
"attribute vec3 vertex;\n"
"attribute vec4 color;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"uniform float distance;\n"
"uniform float offset;\n"
"uniform vec2 size;\n"
"varying vec2 anim;\n"
"varying float dust;\n"
"varying vec4 c;\n"
"varying vec2 map;\n"
"varying vec2 map_dirt;\n"
"varying vec2 map_offset;\n"
"\n"
"void main()\n"
"{\n"
"	vec4 v, center;\n"
"	float f, focus, bright, expand;\n"
"	v = ModelViewProjectionMatrix * vec4(vertex, 1.0);\n"
"	focus =	(v.z - distance) / distance;\n"
//"   focus *= focus;\n"
//"	anim = min(vec2(sqrt(focus), focus) * vec2(2.0, 1.5), vec2(0.99));\n"
"   if(focus > 0.0)\n"
"		focus =	1.0 - (1.0 / (1.0 + focus * focus));\n"
"	else\n"
"	    focus = focus * focus * (3.0 + 2.0 * focus);\n"
"	anim.xy = vec2(focus);\n"
//"   focus = abs(focus) + 0.015 * v.z;\n"
"	bright = max(max(color.r, color.g), color.b);"
"   if(bright > 0.00001)\n"
"	{\n"
"		c = vec4(color.rgb, 1.0);\n"
"		if(color.a > 128.0)\n"
"			map.x = 1.0;\n"
"		else\n"
"			map.x = -1.0;\n"
"		if(mod(color.a, 128.0) > 64.0)\n"
"			map.y = 1.0;\n"
"		else\n"
"			map.y = -1.0;\n"
"		dust = focus * 2.0;\n"
"		f = 1.0 - anim.y;\n"
"		expand = mix(2.0, 4.0 * bright, f * f);\n"
"		c = mix(color, color / vec4(bright), f * f);\n"
"		gl_Position = v + vec4(map * vec2(v.z * 0.1 * expand) * size, 0.0, 0.0);\n"
"		map_offset = vec2(0.8 * map) + v.xy / v.zz  * vec2(offset * focus);\n"
"		map_dirt = vec2(0.5 * map + vec2(0.5))/* + v.xy * vec2(offset * -0.2)*/;\n"
"	}else\n"
"		gl_Position = v;\n"
"}\n";

/*char *stellar_bokhe_fragment = 
"uniform sampler2D image;\n"
"uniform sampler2D dirt;\n"
"uniform float distance;\n"
"varying vec2 anim;\n"
"varying float dust;\n"
"varying vec4 c;\n"
"varying vec2 map;\n"
"varying vec2 map_dirt;\n"
"varying vec2 map_offset;\n"
"\n"
"oid main()\n"
"{\n"
"	gl_FragColor = vec4(distance);\n"
"}\n";*/

char *stellar_bokhe_fragment = 
"uniform sampler2D image;\n"
"uniform sampler2D dirt;\n"
"varying vec2 anim;\n"
"varying float dust;\n"
"varying vec4 c;\n"
"varying vec2 map;\n"
"varying vec2 map_dirt;\n"
"varying vec2 map_offset;\n"
"\n"
"void main()\n"
"{\n"
"	float dist1, dist2;\n"
"	dist1 = length(map) + 1.0 / 128;\n"
"	dist2 = length(map_offset) + 1.0 / 128;\n"
"	gl_FragColor = texture2D(image, vec2(dist1, anim.y)) * mix(vec4(1.0), texture2D(dirt, map_dirt), dust) * c + vec4(0.0, 0.0, 0.0, 0.0);\n"
"}\n";

extern uint seduce_line_image_gen(uint resolution);

#define GL_VERTEX_SHADER_ARB				0x8B31
#define GL_FRAGMENT_SHADER_ARB				0x8B30
#define GL_GEOMETRY_SHADER_ARB				0x8DD9

void stellar_light_draw_lines(void *pool)
{
	static RShader *pixel_shader = NULL;
	uint i, count;
	static float brightness = 0.1, camera[3];
	static uint texture_id = 0;
	static void *test_pool = NULL;
	return;
	if(pool == NULL)
	{
		if(test_pool == NULL)	
		{
			RFormats vertex_format_types[2] = {R_FLOAT, R_FLOAT};
			uint vertex_format_size[2] = {4, 4};
			float *array, f = 0;
			array = malloc((sizeof * array) * 200 * 16);
			for(i = 0; i < 200; i++)
			{
				array[i * 16 + 0] = sin(f * 0.3);
				array[i * 16 + 1] = f * 0.01;
				array[i * 16 + 2] = cos(f * 0.3);
				array[i * 16 + 3] = 1.0;

				array[i * 16 + 4] = 2;
				array[i * 16 + 5] = 2;
				array[i * 16 + 6] = 2;
				array[i * 16 + 7] = f * 0.02121;
				f++;
				array[i * 16 + 8] = sin(f * 0.3);
				array[i * 16 + 9] = f * 0.01;
				array[i * 16 + 10] = cos(f * 0.3);
				array[i * 16 + 11] = 1.0;

				array[i * 16 + 12] = array[i * 16 + 4];
				array[i * 16 + 13] = array[i * 16 + 5];
				array[i * 16 + 14] = array[i * 16 + 6];
				array[i * 16 + 15] = f * 0.02121;

			}
			
			i = 0;
			array[i * 16 + 0] = 0.3;
			array[i * 16 + 1] = 0;
			array[i * 16 + 2] = 0;
			array[i * 16 + 3] = 1.0;

			array[i * 16 + 4] = 2;
			array[i * 16 + 5] = 2;
			array[i * 16 + 6] = 2;
			array[i * 16 + 7] = 0.02121;

			array[i * 16 + 8] = 0;
			array[i * 16 + 9] = 0;
			array[i * 16 + 10] = 0;
			array[i * 16 + 11] = 1.0;

			array[i * 16 + 12] = 2;
			array[i * 16 + 13] = 2;
			array[i * 16 + 14] = 2;
			array[i * 16 + 15] = 0.02121;
			i = 1;
			array[i * 16 + 0] = -0.3;
			array[i * 16 + 1] = 0;
			array[i * 16 + 2] = 0;
			array[i * 16 + 3] = 1.0;

			array[i * 16 + 4] = 2;
			array[i * 16 + 5] = 2;
			array[i * 16 + 6] = 2;
			array[i * 16 + 7] = 0.02121;

			array[i * 16 + 8] = 0;
			array[i * 16 + 9] = 0;
			array[i * 16 + 10] = 0;
			array[i * 16 + 11] = 1.0;

			array[i * 16 + 12] = 2;
			array[i * 16 + 13] = 2;
			array[i * 16 + 14] = 2;
			array[i * 16 + 15] = 0.02121;

			test_pool = r_array_allocate(400, vertex_format_types, vertex_format_size, 2, 0);
			r_array_load_vertex(test_pool, NULL, array, 0, 400); 
			
			texture_id = seduce_line_image_gen(1024);
		}
		pool = test_pool;
	}

	if(pixel_shader == NULL)
/*	{
		pixel_shader = r_shader_create_simple(NULL, 0, stellar_pixel_vertex, stellar_pixel_fragment, "pixel");
		r_shader_texture_set(pixel_shader, 0, stellar_gen_bokeh_image());
		r_shader_state_set_blend_mode(pixel_shader, 1, 1);
	}*/
	{
		char *source[3];
		uint stages[3];
		source[0] = stellar_line_expand_vertex;
		source[1] = stellar_line_expand_geometry; 
		source[2] = stellar_line_expand_fragment;
		stages[0] = GL_VERTEX_SHADER_ARB;
		stages[1] = GL_GEOMETRY_SHADER_ARB;
		stages[2] = GL_FRAGMENT_SHADER_ARB;
		pixel_shader = r_shader_create(NULL, 0, source, stages, 3, "Expand shader", NULL);
		r_shader_texture_set(pixel_shader, 0, texture_id);
		r_shader_state_set_depth_test(pixel_shader, GL_ALWAYS);
	}

/*	shader = r_shader_presets_get(P_SP_COLOR_UNIFORM);
	r_shader_vec4_set(shader, r_shader_uniform_location(shader, "color"), 2.0, 2.0, 2.0, 1);
	r_shader_set(shader);*/
	r_shader_set(pixel_shader);
	r_shader_vec3_set(pixel_shader, r_shader_uniform_location(pixel_shader, "camera"), camera[0], camera[1], camera[2]);
	r_shader_float_set(pixel_shader, r_shader_uniform_location(pixel_shader, "distance"), seduce_view_distance_camera_get(NULL));
	/*{
		BInputState *input;
		static float f = 0;
		input = betray_get_input_state();
		f += input->delta_time * 0.1;
		if(f > 1.0 - 1.0 / 2048.0)
			r_shader_float_set(pixel_shader, r_shader_uniform_location(pixel_shader, "time"), 1.0 - 1.0 / 2048.0);
		else
			r_shader_float_set(pixel_shader, r_shader_uniform_location(pixel_shader, "time"), f);
		if(f > 1.2)
			f = 0;

		r_shader_float_set(pixel_shader, r_shader_uniform_location(pixel_shader, "test"), input->pointers[0].pointer_x + 1.0);

	}*/
	r_shader_state_set_blend_mode(pixel_shader, 1, 1);
	r_shader_state_set_mask(pixel_shader, TRUE, TRUE, TRUE, TRUE, FALSE); 
//	r_array_draw(pool, NULL, R_PRIMITIVE_TRIANGLES, 0, -1, NULL, NULL, 1);
//	r_array_draw(pool, NULL, GL_TRIANGLE_STRIP, 0, -1, NULL, NULL, 1);
	r_array_draw(pool, NULL, GL_LINES, 0, 200, NULL, NULL, 1);
}


typedef struct{
	uint line_length;
	uint line_used;
	float *line_buffer;
	void *line_pool;
	uint circle_length;
	uint circle_used;
	float *circle_buffer;
	void *circle_pool;
	uint spline_length;
	uint spline_used;
	float *spline_buffer;
	void *spline_pool;
	uint point_length;
	uint point_used;
	float *point_buffer;
	void *point_pool;
}SeduceLineObject;

SeduceLineObject *seduce_global_line_buffer; 
RShader *seduce_global_line_shader = NULL;
RShader *seduce_global_circle_shader = NULL; 
RShader *seduce_global_spline_shader = NULL; 
RShader *seduce_global_point_shader = NULL;

SeduceLineObject *seduce_primitive_line_object_allocate()
{
	SeduceLineObject *object;
	object = malloc(sizeof *object);
	object->line_length = 0;
	object->line_used = 0;
	object->line_buffer = NULL;
	object->line_pool = NULL;
	
	object->circle_length = 0;
	object->circle_used = 0;
	object->circle_buffer = NULL;
	object->circle_pool = NULL;

	object->spline_length = 0;
	object->spline_used = 0;
	object->spline_buffer = NULL;
	object->spline_pool = NULL;

	object->point_length = 0;
	object->point_used = 0;
	object->point_buffer = NULL;
	object->point_pool = NULL;

	return object;
}

void seduce_primitive_line_object_free(SeduceLineObject *object)
{
	if(object->line_buffer != NULL)
		free(object->line_buffer);
	if(object->line_pool != NULL)
		r_array_free(object->line_pool);

	if(object->circle_buffer != NULL)
		free(object->circle_buffer);
	if(object->circle_pool != NULL)
		r_array_free(object->circle_pool);

	if(object->spline_buffer != NULL)
		free(object->spline_buffer);
	if(object->spline_pool != NULL)
		r_array_free(object->spline_pool);

	if(object->point_buffer != NULL)
		free(object->point_buffer);
	if(object->point_pool != NULL)
		r_array_free(object->point_pool);
	free(object);
}

void seduce_primitive_line_focal_depth_set(float distance)
{
	r_shader_float_set(seduce_global_line_shader, r_shader_uniform_location(seduce_global_line_shader, "distance"), distance);
	r_shader_float_set(seduce_global_circle_shader, r_shader_uniform_location(seduce_global_circle_shader, "distance"), distance);	
	r_shader_float_set(seduce_global_spline_shader, r_shader_uniform_location(seduce_global_spline_shader, "distance"), distance);
}

float seduce_primitive_line_color_matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
float seduce_primitive_line_color_brightness = 1;
float seduce_primitive_line_scroll = 0;
float seduce_primitive_line_speed = 0;
float seduce_primitive_line_add = 0;
float seduce_primitive_line_timer = 1;

void seduce_primitive_line_color_set(float red_a, float green_a, float blue_a, 
									float red_b, float green_b, float blue_b, 
									float red_c, float green_c, float blue_c)
{
	seduce_primitive_line_color_matrix[0] = red_a;
	seduce_primitive_line_color_matrix[3] = green_a;
	seduce_primitive_line_color_matrix[6] = blue_a;
	seduce_primitive_line_color_matrix[1] = red_b;
	seduce_primitive_line_color_matrix[4] = green_b;
	seduce_primitive_line_color_matrix[7] = blue_b;
	seduce_primitive_line_color_matrix[2] = red_c;
	seduce_primitive_line_color_matrix[5] = green_c;
	seduce_primitive_line_color_matrix[8] = blue_c;
}

void seduce_primitive_line_animation_set(float scroll, float add, float speed, float timer, float color_brightness)
{
	seduce_primitive_line_scroll = scroll;
	seduce_primitive_line_speed = speed;
	seduce_primitive_line_timer = timer;
	seduce_primitive_line_color_brightness = color_brightness;
	seduce_primitive_line_add = add;
}

uint seduce_line_texture_id;

char *over_ride_shader =
"#version 450\n"
"layout(lines) in;\n"
"layout(triangle_strip, max_vertices = 8) out;\n"
"uniform float distance;\n"
"uniform mat4 ProjectionMatrix;\n"
"uniform vec3 camera;\n"
"uniform float test;\n"
"in vec4 col[];\n"
"in float coord[];\n"
"out vec4 uv;\n"
"void main()\n"
"{\n"
"		vec3 expand;\n"
"		vec3 vector, camera_vec, normal;\n"
"		vec4 tmp;\n"
"		float x, depth_a, depth_b, scale, dist;\n"
"		scale = test;\n"
"		depth_a = 0.4 * (gl_in[0].gl_Position.z / distance + 1.0);\n"
"		depth_b = 0.4 * (gl_in[1].gl_Position.z / distance + 1.0);\n"
"		scale = max(abs(depth_a) + 0.05, abs(depth_b) + 0.05);\n"
"		vector = normalize(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz);\n"
"		expand = normalize(cross(normalize(gl_in[0].gl_Position.xyz), vector)) * vec3(0.02 * distance) * vec3(scale);\n"
"		dist =  0.5 + 0.5 * length(gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz) / (0.02 * scale);\n"
"		vector = vector * vec3(0.02) * vec3(scale);\n"
"		uv = vec4(scale, scale, 0, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position + vec4(expand + vector, 0.0));\n"
"		EmitVertex();\n"
"		uv = vec4(scale, -scale, 0, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position - vec4(expand - vector, 0.0));\n"
"		EmitVertex();\n"
"		uv = vec4(0, scale, 0.5, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position + vec4(expand, 0.0));\n"
"		EmitVertex();\n"
"		uv = vec4(0, -scale, 0.5, dist);\n"
"		gl_Position = ProjectionMatrix * (gl_in[0].gl_Position - vec4(expand, 0.0));\n"
"		EmitVertex();\n"
"		uv = vec4(0, scale, dist, 0.5);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position + vec4(expand, 0.0));\n"
"		EmitVertex();\n"
"		uv = vec4(0, -scale, dist, 0.5);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position - vec4(expand, 0.0));\n"
"		EmitVertex();\n"
"		uv = vec4(-scale, scale, dist, 0);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position + vec4(expand - vector, 0.0));\n"
"		EmitVertex();\n"
"		uv = vec4(-scale, -scale, dist, 0);\n"
"		gl_Position = ProjectionMatrix * (gl_in[1].gl_Position - vec4(expand + vector, 0.0));\n"
"		EmitVertex();\n"
"		EndPrimitive();\n"
"}";

extern void r_shader_debug_override(char *shaders, uint stages);

void seduce_primitive_line_init()
{
	uint i, count;
	static float brightness = 0.1, camera[3];
	static uint texture_id = 0;
	static void *test_pool = NULL;
	char *source[3];
	uint stages[3], segments = 128;
	RFormats vertex_format_types[6] = {R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT};
	uint vertex_format_size[6] = {4, 4, 4, 4, 4, 2};


	i = r_extension_query(R_Q_GEOMETRY_OUTPUT_COMPONENTS_MAX) / 24;
	if(i < segments)
		segments = i;
	segments /= 2;
	seduce_line_texture_id = seduce_line_image_gen(1024);

	source[0] = stellar_line_expand_vertex;
	source[1] = stellar_line_expand_geometry;
	source[2] = stellar_line_expand_fragment;
	stages[0] = GL_VERTEX_SHADER_ARB;
	stages[1] = GL_GEOMETRY_SHADER_ARB;
	stages[2] = GL_FRAGMENT_SHADER_ARB;
//	r_shader_debug_override(over_ride_shader, 1);
	seduce_global_line_shader = r_shader_create(NULL, 0, source, stages, 3, "Expand shader", NULL);

	r_shader_texture_set(seduce_global_line_shader, 0, seduce_line_texture_id);
	r_shader_state_set_mask(seduce_global_line_shader, TRUE, TRUE, TRUE, TRUE, FALSE); 
	r_shader_state_set_blend_mode(seduce_global_line_shader, 1, GL_ONE_MINUS_SRC_ALPHA);
	r_shader_state_set_depth_test(seduce_global_line_shader, GL_ALWAYS);

	source[0] = malloc(4096);
	source[1] = malloc(4096);
	sprintf(source[0], stellar_circle_expand_vertex, (float)segments - 1.0);
	sprintf(source[1], stellar_circle_expand_geometry, segments * 2, segments);
	source[2] = stellar_circle_expand_fragment;
	stages[0] = GL_VERTEX_SHADER_ARB;
	stages[1] = GL_GEOMETRY_SHADER_ARB;
	stages[2] = GL_FRAGMENT_SHADER_ARB;

	seduce_global_circle_shader = r_shader_create(NULL, 0, source, stages, 3, "Circle shader", NULL);
	r_shader_texture_set(seduce_global_circle_shader, 0, seduce_line_texture_id);
	r_shader_state_set_mask(seduce_global_circle_shader, TRUE, TRUE, TRUE, TRUE, FALSE); 
	r_shader_state_set_blend_mode(seduce_global_circle_shader, 1, GL_ONE_MINUS_SRC_ALPHA);
	r_shader_state_set_depth_test(seduce_global_circle_shader, GL_ALWAYS);

	sprintf(source[0], stellar_spline_expand_vertex, (float)segments - 1.0);
	sprintf(source[1], stellar_spline_expand_geometry, segments * 2);
	source[2] = stellar_circle_expand_fragment;
	stages[0] = GL_VERTEX_SHADER_ARB;
	stages[1] = GL_GEOMETRY_SHADER_ARB;
	stages[2] = GL_FRAGMENT_SHADER_ARB;
	seduce_global_spline_shader = r_shader_create(NULL, 0, source, stages, 3, "Spline shader", NULL);
	r_shader_texture_set(seduce_global_spline_shader, 0, seduce_line_texture_id);
	r_shader_state_set_mask(seduce_global_spline_shader, TRUE, TRUE, TRUE, TRUE, FALSE); 
	r_shader_state_set_blend_mode(seduce_global_line_shader, 1, GL_ONE_MINUS_SRC_ALPHA);
	seduce_primitive_line_focal_depth_set(1);
	r_shader_state_set_blend_mode(seduce_global_line_shader, 1, GL_ONE_MINUS_SRC_ALPHA);
	r_shader_state_set_mask(seduce_global_line_shader, TRUE, TRUE, TRUE, TRUE, FALSE); 
	r_shader_state_set_depth_test(seduce_global_line_shader, GL_ALWAYS);
	free(source[0]);
	free(source[1]);

	source[0] = stellar_bokhe_vertex;
	source[1] = stellar_bokhe_fragment;
	stages[0] = GL_VERTEX_SHADER_ARB;
	stages[1] = GL_FRAGMENT_SHADER_ARB;
	//	r_shader_debug_override(over_ride_shader, 1);
	seduce_global_point_shader = r_shader_create(NULL, 0, source, stages, 2, "Boke shader", NULL);

	r_shader_texture_set(seduce_global_point_shader, 0, seduce_line_bokeh_gen());
	r_shader_texture_set(seduce_global_point_shader, 1, seduce_line_dirt_gen());
	r_shader_state_set_mask(seduce_global_point_shader, TRUE, TRUE, TRUE, TRUE, FALSE); 
	r_shader_state_set_blend_mode(seduce_global_point_shader, 1, 1);
	r_shader_state_set_depth_test(seduce_global_point_shader, GL_ALWAYS);

	seduce_global_line_buffer = seduce_primitive_line_object_allocate();

	seduce_global_line_buffer->line_length = 1024 * 10; 
	seduce_global_line_buffer->line_buffer = malloc((sizeof *seduce_global_line_buffer->line_buffer) * 8 * seduce_global_line_buffer->line_length);
	seduce_global_line_buffer->line_pool = r_array_allocate(seduce_global_line_buffer->line_length, vertex_format_types, vertex_format_size, 2, 0);	

	seduce_global_line_buffer->circle_length = 64;
	seduce_global_line_buffer->circle_buffer = malloc((sizeof *seduce_global_line_buffer->circle_buffer) * 22 * seduce_global_line_buffer->circle_length);
	seduce_global_line_buffer->circle_pool = r_array_allocate(seduce_global_line_buffer->circle_length, vertex_format_types, vertex_format_size, 6, 0);	

	seduce_global_line_buffer->spline_length = 64;
	seduce_global_line_buffer->spline_buffer = malloc((sizeof *seduce_global_line_buffer->spline_buffer) * 22 * seduce_global_line_buffer->spline_length);
	seduce_global_line_buffer->spline_pool = r_array_allocate(seduce_global_line_buffer->spline_length, vertex_format_types, vertex_format_size, 6, 0);

	vertex_format_size[0] = 3;
	seduce_global_line_buffer->point_length = 1024;
	seduce_global_line_buffer->point_buffer = malloc((sizeof *seduce_global_line_buffer->point_buffer) * 7 * 6 * seduce_global_line_buffer->point_length);
	seduce_global_line_buffer->point_pool = r_array_allocate(seduce_global_line_buffer->point_length * 6, vertex_format_types, vertex_format_size, 2, 0);

	r_primitive_image(0, 0, -1.0, 0.0, 0.0, 0, 0, 1, 1, seduce_line_texture_id, 1, 1, 1, 0); /* Fix me!! */
}


void seduce_primitive_line_draw(SeduceLineObject *object, float red, float green, float blue, float alpha)
{
	BInputState *input;
	float camera[3], *m, color_matrix[9], f, scroll, resolution;
	uint i, line_length = 0, circle_length = 0, spline_length = 0, point_length = 0;

	if(object == NULL)
	{
		object = seduce_global_line_buffer;
		if(object->line_used != 0)
		{
			line_length = object->line_used;
			r_array_load_vertex(object->line_pool, NULL, object->line_buffer, 0, line_length);
			object->line_used = 0;
		}else
			line_length = 0;
		if(object->circle_used != 0)
		{
			circle_length = object->circle_used;
			r_array_load_vertex(object->circle_pool, NULL, object->circle_buffer, 0, circle_length);
			object->circle_used = 0;
		}else
			circle_length = 0;
		if(object->spline_used != 0)
		{
			spline_length = object->spline_used;
			r_array_load_vertex(object->spline_pool, NULL, object->spline_buffer, 0, spline_length);
			object->circle_used = 0;
		}else
			spline_length = 0;
		if(object->point_used != 0)
		{
			point_length = object->point_used * 6;
			r_array_load_vertex(object->point_pool, NULL, object->point_buffer, 0, point_length);
			object->point_used = 0;
		}else
			point_length = 0;

	}else
	{
		line_length = -1;
		if(object->line_used != 0)
		{
			RFormats vertex_format_types[2] = {R_FLOAT, R_FLOAT};
			uint vertex_format_size[2] = {4, 4};
			if(object->line_pool == NULL)
			{
				object->line_pool = r_array_allocate(object->line_used, vertex_format_types, vertex_format_size, 2, 0);
				r_array_load_vertex(object->line_pool, NULL, object->line_buffer, 0, object->line_used);
				free(object->line_buffer);
				object->line_buffer = NULL;
				object->line_length = 0;
			}else
				r_array_load_vertex(object->line_pool, NULL, object->line_buffer, 0, object->line_used);
			object->line_used = 0;
		}else if(object->line_pool == NULL)
			line_length = 0;

		circle_length = -1;
		if(object->circle_used != 0)
		{
			RFormats vertex_format_types[6] = {R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT};
			uint vertex_format_size[6] = {4, 4, 4, 4, 4, 2};

			if(object->circle_pool == NULL)
			{
				object->circle_pool = r_array_allocate(object->circle_used, vertex_format_types, vertex_format_size, 6, 0);
				r_array_load_vertex(object->circle_pool, NULL, object->circle_buffer, 0, object->circle_used);
				free(object->circle_buffer);
				object->circle_buffer = NULL;
				object->circle_length = 0;
			}else
				r_array_load_vertex(object->circle_pool, NULL, object->circle_buffer, 0, object->circle_used);
			object->circle_used = 0;
		}else if(object->circle_pool == NULL)
			circle_length = 0;

		spline_length = -1;
		if(object->spline_used != 0)
		{
			RFormats vertex_format_types[6] = {R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT, R_FLOAT};
			uint vertex_format_size[6] = {4, 4, 4, 4, 4, 2};
			
			if(object->spline_pool == NULL)
			{
				object->spline_pool = r_array_allocate(object->spline_used, vertex_format_types, vertex_format_size, 6, 0);
				r_array_load_vertex(object->spline_pool, NULL, object->spline_buffer, 0, object->spline_used);
				free(object->spline_buffer);
				object->spline_buffer = NULL;
				object->spline_length = 0;
			}else
				r_array_load_vertex(object->spline_pool, NULL, object->spline_buffer, 0, object->spline_used);
			object->spline_used = 0;
		}else if(object->spline_pool == NULL)
			spline_length = 0;
	}
	input = betray_get_input_state();
	m = seduce_primitive_line_color_matrix;
	if(seduce_primitive_line_timer >= 0.999)
	{
		for(i = 0; i < 9; i++)
			color_matrix[i] = seduce_primitive_line_color_matrix[i] * seduce_primitive_line_color_brightness;
		m = color_matrix;
	
		f = input->minute_time * 5.0 * seduce_primitive_line_speed + seduce_primitive_line_add;
		f -= (float)((int)f); 
		f = 0.25 + f * 0.75;		
	}else
	{
		f = seduce_primitive_line_timer * 0.24 + 0.01;
	}
	scroll = input->minute_time * 60.0 * seduce_primitive_line_scroll;
	betray_screen_mode_get(&i, NULL, NULL); 
	resolution = 50.0 / (float)i;
	if(line_length != 0)
	{
		r_shader_set(seduce_global_line_shader);
		r_shader_vec4_set(seduce_global_line_shader, r_shader_uniform_location(seduce_global_line_shader, "base_color"), red * 2, green * 2, blue * 2, alpha);
		r_shader_float_set(seduce_global_line_shader, r_shader_uniform_location(seduce_global_line_shader, "time"), f);
		r_shader_float_set(seduce_global_line_shader, r_shader_uniform_location(seduce_global_line_shader, "scroll"), scroll);
		r_shader_mat3v_set(seduce_global_line_shader, r_shader_uniform_location(seduce_global_line_shader, "color_space"), m);
		r_shader_float_set(seduce_global_line_shader, r_shader_uniform_location(seduce_global_line_shader, "resolution"), resolution);
		r_array_draw(object->line_pool, NULL, GL_LINES, 0, line_length, NULL, NULL, 1);
	}
	if(circle_length != 0)
	{
		r_shader_set(seduce_global_circle_shader);
		r_shader_vec4_set(seduce_global_circle_shader, r_shader_uniform_location(seduce_global_circle_shader, "base_color"), red, green, blue, alpha);
		r_shader_float_set(seduce_global_circle_shader, r_shader_uniform_location(seduce_global_circle_shader, "time"), f);
		r_shader_float_set(seduce_global_circle_shader, r_shader_uniform_location(seduce_global_circle_shader, "scroll"), scroll);
		r_shader_mat3v_set(seduce_global_circle_shader, r_shader_uniform_location(seduce_global_circle_shader, "color_space"), m);
		r_shader_float_set(seduce_global_circle_shader, r_shader_uniform_location(seduce_global_circle_shader, "resolution"), resolution);
		r_array_draw(object->circle_pool, NULL, GL_POINTS, 0, circle_length, NULL, NULL, 1);
	}
	if(spline_length != 0)
	{
		r_shader_set(seduce_global_spline_shader);
		r_shader_vec4_set(seduce_global_spline_shader, r_shader_uniform_location(seduce_global_spline_shader, "base_color"), red, green, blue, alpha);
		r_shader_float_set(seduce_global_spline_shader, r_shader_uniform_location(seduce_global_spline_shader, "time"), f);
		r_shader_float_set(seduce_global_spline_shader, r_shader_uniform_location(seduce_global_spline_shader, "scroll"), scroll);
		r_shader_mat3v_set(seduce_global_spline_shader, r_shader_uniform_location(seduce_global_spline_shader, "color_space"), m);
		r_array_draw(object->spline_pool, NULL, GL_POINTS, 0, spline_length, NULL, NULL, 1);
	}
	if(point_length != 0)
	{
		r_shader_set(seduce_global_point_shader);
		r_shader_vec4_set(seduce_global_point_shader, r_shader_uniform_location(seduce_global_point_shader, "base_color"), red, green, blue, alpha);
		r_shader_float_set(seduce_global_point_shader, r_shader_uniform_location(seduce_global_point_shader, "offset"), 0.9);
		r_shader_float_set(seduce_global_point_shader, r_shader_uniform_location(seduce_global_point_shader, "distance"), seduce_view_distance_camera_get(NULL));
		r_shader_vec2_set(seduce_global_point_shader, r_shader_uniform_location(seduce_global_point_shader, "size"), 0.1, 0.1 / 0.6);
		r_array_draw(object->point_pool, NULL, GL_TRIANGLES, 0, point_length, NULL, NULL, 1);
	}
}

void seduce_primitive_line_add_3d(SeduceLineObject *object,
						float pos_a_x, float pos_a_y, float pos_a_z,
						float pos_b_x, float pos_b_y, float pos_b_z,
						float red_a, float green_a, float blue_a, float alpha_a,
						float red_b, float green_b, float blue_b, float alpha_b)
{
	float *buffer;
	if(object == NULL)
	{
		object = seduce_global_line_buffer;
		if(object->line_length == object->line_used)
		{
			seduce_primitive_line_draw(NULL, 1.0, 1.0, 1.0, 1.0);
			object->line_used = 2;
			buffer = object->line_buffer;
		}else
		{
			buffer = &object->line_buffer[object->line_used * 8];
			object->line_used += 2;
		}
	}else
	{
		if(object->line_length == object->line_used)
		{
			if(object->line_length == 0)
				object->line_length = 256;
			object->line_length *= 2;
			object->line_buffer = realloc(object->line_buffer, (sizeof *object->line_buffer) * 8 * object->line_length);
			if(object->line_buffer == NULL)
			{
				object->line_buffer[0] = 0;
			}
		}
		buffer = &object->line_buffer[object->line_used * 8];
		object->line_used += 2;
		*buffer = pos_a_x;
//		f_debug_memory();
	}
	*buffer++ = pos_a_x;
	*buffer++ = pos_a_y;
	*buffer++ = pos_a_z;
	*buffer++ = 0;

	*buffer++ = red_a;
	*buffer++ = green_a;      
	*buffer++ = blue_a;
	*buffer++ = alpha_a;

	*buffer++ = pos_b_x;
	*buffer++ = pos_b_y;
	*buffer++ = pos_b_z;
	*buffer++ = 0;

	*buffer++ = red_b;
	*buffer++ = green_b;
	*buffer++ = blue_b;
	*buffer++ = alpha_b;
}

/*
"attribute vec4 center;\n" // alpha is radius
"attribute vec4 vector;\n" // alpha is start
"attribute vec4 normal;\n" // alpha is end
"attribute vec4 color;\n"
*/ 
void seduce_primitive_circle_add_3d(SeduceLineObject *object,
						float center_x, float center_y, float center_z,
						float vec_x, float vec_y, float vec_z,
						float normal_x, float normal_y, float normal_z,
						float radius,
						float start, float end,
						float start_u, float length_u,
						float red_a, float green_a, float blue_a, float alpha_a,
						float red_b, float green_b, float blue_b, float alpha_b)
{
	float *buffer;
	if(radius < 0.00001)
		return;
	if(object == NULL)
	{
		object = seduce_global_line_buffer;
		if(object->circle_length == object->circle_used)
		{
			seduce_primitive_line_draw(NULL, 1.0, 1.0, 1.0, 1.0);
			object->circle_used = 1;
			buffer = object->circle_buffer;
		}else
			buffer = &object->circle_buffer[object->circle_used++ * 22];
	}else
	{
		if(object->circle_length == object->circle_used)
		{
			if(object->circle_length == 0)
				object->circle_length = 64;
			object->circle_length *= 2;
			object->circle_buffer = realloc(object->circle_buffer, (sizeof *object->circle_buffer) * 22 * object->circle_length);
		}
		buffer = &object->circle_buffer[object->circle_used++ * 22];
	}
	*buffer++ = center_x;
	*buffer++ = center_y;
	*buffer++ = center_z;
	*buffer++ = radius;

	*buffer++ = vec_x;
	*buffer++ = vec_y;
	*buffer++ = vec_z;
	*buffer++ = start;

	*buffer++ = normal_x;
	*buffer++ = normal_y;
	*buffer++ = normal_z;
	*buffer++ = end;
	
	*buffer++ = red_a;
	*buffer++ = green_a;
	*buffer++ = blue_a;
	*buffer++ = alpha_a;

	*buffer++ = red_b;
	*buffer++ = green_b;
	*buffer++ = blue_b;
	*buffer++ = alpha_b;
	
	*buffer++ = start_u;
	*buffer++ = length_u;
}

void seduce_primitive_point_add_3d(SeduceLineObject *object,
									float cv_x, float cv_y, float cv_z,
									float red, float green, float blue, float alpha)
{
	float *buffer;
	if(object == NULL)
	{
		object = seduce_global_line_buffer;
		if(object->point_length == object->point_used)
		{
			seduce_primitive_line_draw(NULL, 1.0, 1.0, 1.0, 1.0);
			object->point_used = 1;
			buffer = object->point_buffer;
		}else
			buffer = &object->point_buffer[object->point_used++ * (7 * 6)];
	}else
	{
		if(object->point_length == object->spline_used)
		{
			if(object->point_length == 0)
				object->point_length = 64;
			object->point_length *= 2;
			object->point_buffer = realloc(object->point_buffer, (sizeof *object->point_buffer) * (7 * 6) * object->point_length);
		}
		buffer = &object->point_buffer[object->point_used++ * (7 * 6)];
	}

	*buffer++ = cv_x;
	*buffer++ = cv_y;
	*buffer++ = cv_z;
	*buffer++ = red;
	*buffer++ = green;
	*buffer++ = blue;
	*buffer++ = 64.0 * 0.5;

	*buffer++ = cv_x;
	*buffer++ = cv_y;
	*buffer++ = cv_z;
	*buffer++ = red;
	*buffer++ = green;
	*buffer++ = blue;
	*buffer++ = 64.0 * 1.5;

	*buffer++ = cv_x;
	*buffer++ = cv_y;
	*buffer++ = cv_z;
	*buffer++ = red;
	*buffer++ = green;
	*buffer++ = blue;
	*buffer++ = 64.0 * 3.5;

	*buffer++ = cv_x;
	*buffer++ = cv_y;
	*buffer++ = cv_z;
	*buffer++ = red;
	*buffer++ = green;
	*buffer++ = blue;
	*buffer++ = 64.0 * 0.5;

	*buffer++ = cv_x;
	*buffer++ = cv_y;
	*buffer++ = cv_z;
	*buffer++ = red;
	*buffer++ = green;
	*buffer++ = blue;
	*buffer++ = 64.0 * 3.5;

	*buffer++ = cv_x;
	*buffer++ = cv_y;
	*buffer++ = cv_z;
	*buffer++ = red;
	*buffer++ = green;
	*buffer++ = blue;
	*buffer++ = 64.0 * 2.5;
}




void seduce_primitive_spline_add_3d(SeduceLineObject *object,
						float cv_a_x, float cv_a_y, float cv_a_z,
						float cv_b_x, float cv_b_y, float cv_b_z,
						float cv_c_x, float cv_c_y, float cv_c_z,
						float cv_d_x, float cv_d_y, float cv_d_z,
						float start_u, float length_u,
						float red_a, float green_a, float blue_a, float alpha_a,
						float red_b, float green_b, float blue_b, float alpha_b)
{
	float *buffer;
	if(object == NULL)
	{
		object = seduce_global_line_buffer;
		if(object->spline_length == object->spline_used)
		{
			seduce_primitive_line_draw(NULL, 1.0, 1.0, 1.0, 1.0);
			object->spline_used = 1;
			buffer = object->spline_buffer;
		}else
			buffer = &object->spline_buffer[object->spline_used++ * 22];
	}else
	{
		if(object->spline_length == object->spline_used)
		{
			if(object->spline_length == 0)
				object->spline_length = 64;
			object->spline_length *= 2;
			object->spline_buffer = realloc(object->spline_buffer, (sizeof *object->spline_buffer) * 22 * object->spline_length);
		}
		buffer = &object->spline_buffer[object->spline_used++ * 22];
	}
	*buffer++ = cv_a_x;
	*buffer++ = cv_a_y;
	*buffer++ = cv_a_z;
	*buffer++ = cv_d_x;

	*buffer++ = cv_b_x;
	*buffer++ = cv_b_y;
	*buffer++ = cv_b_z;
	*buffer++ = cv_d_y;

	*buffer++ = cv_c_x;
	*buffer++ = cv_c_y;
	*buffer++ = cv_c_z;
	*buffer++ = cv_d_z;
	
	*buffer++ = red_a;
	*buffer++ = green_a;
	*buffer++ = blue_a;
	*buffer++ = alpha_a;

	*buffer++ = red_b;
	*buffer++ = green_b;
	*buffer++ = blue_b;
	*buffer++ = alpha_b;

	*buffer++ = start_u;
	*buffer++ = length_u;
}


void seduce_primitive_color_wheel_add_3d(SeduceLineObject *object,
						float center_x, float center_y, float center_z,
						float radius)
{
/*	uint i;
	double cur, next, spacing = 0.6, spline[8], output[2], best, found, f, scale;

	cur = (double)0 * PI * 2.0 / 6.0;
	next = (double)(0 + 1) * PI * 2.0  / 6.0;

	found = 0.5;
	scale = 0.5;
	for(i = 0; i < 10; i++)
	{
		best = 1000000;
		for(spacing = found - scale; spacing < found + scale; spacing += scale / 100.0)
		{
			spline[0] = sin(cur);
			spline[1] = cos(cur);
			spline[2] = sin(cur) + cos(cur) * spacing;
			spline[3] = cos(cur) - sin(cur) * spacing;
			spline[4] = sin(next) - cos(next) * spacing;
			spline[5] = cos(next) + sin(next) * spacing;
			spline[6] = sin(next);
			spline[7] = cos(next);
		
			f_spline2dd(output, 0.5, &spline[0], &spline[2], &spline[4], &spline[6]);
			f = f_length2d(output);
			f_spline2dd(output, 0.4, &spline[0], &spline[2], &spline[4], &spline[6]);
			f += f_length2d(output);
			f_spline2dd(output, 0.3, &spline[0], &spline[2], &spline[4], &spline[6]);
			f += f_length2d(output);
			f_spline2dd(output, 0.2, &spline[0], &spline[2], &spline[4], &spline[6]);
			f += f_length2d(output);
			f_spline2dd(output, 0.1, &spline[0], &spline[2], &spline[4], &spline[6]);
			f += f_length2d(output);
			f -= 5;
			if(f < 0.0)
				f = -f;
			if(f < best)
			{
				found = spacing;
				best = f;
			}
		}
		scale *= 0.1;

	}
	spacing = found;

	
	for(i = 0; i < 6; i++)
	{
		cur = (double)i * PI * 2.0 / 6.0;
		next = (double)(i + 1) * PI * 2.0  / 6.0;
		spline[0] = sin(cur);
		spline[1] = cos(cur);
		spline[2] = sin(cur) + cos(cur) * spacing;
		spline[3] = cos(cur) - sin(cur) * spacing;
		spline[4] = sin(next) - cos(next) * spacing;
		spline[5] = cos(next) + sin(next) * spacing;
		spline[6] = sin(next);
		spline[7] = cos(next);	
		printf("%f, %f, %f, %f, %f, %f, %f, %f,\n",
			(float)spline[0],
			(float)spline[1],
			(float)spline[2],
			(float)spline[3],
			(float)spline[4],
			(float)spline[5],
			(float)spline[6],
			(float)spline[7]);
	}
	exit(0);*/
	
	seduce_primitive_circle_add_3d(object,
						center_x, center_y, center_z,
						0, 1, 0,
						0, 0, 1,
						radius,
						0.0 / 6.0, 1.0 / 6.0,
						0.0 / 6.0 * radius * PI * 2.0, 1.0 / 6.0 * radius * PI * 2.0,
						1, 0, 0, 1,
						1, 1, 0, 1);
	seduce_primitive_circle_add_3d(object,
						center_x, center_y, center_z,
						0, 1, 0,
						0, 0, 1,
						radius,
						1.0 / 6.0, 1.0 / 6.0,
						1.0 / 6.0 * radius * PI * 2.0, 1.0 / 6.0 * radius * PI * 2.0,
						1, 1, 0, 1,
						0, 1, 0, 1);
	seduce_primitive_circle_add_3d(object,
						center_x, center_y, center_z,
						0, 1, 0,
						0, 0, 1,
						radius,
						2.0 / 6.0, 1.0 / 6.0,
						2.0 / 6.0 * radius * PI * 2.0, 1.0 / 6.0 * radius * PI * 2.0,
						0, 1, 0, 1,
						0, 1, 1, 1);
	seduce_primitive_circle_add_3d(object,
						center_x, center_y, center_z,
						0, 1, 0,
						0, 0, 1,
						radius,
						3.0 / 6.0, 1.0 / 6.0,
						3.0 / 6.0 * radius * PI * 2.0, 1.0 / 6.0 * radius * PI * 2.0,
						0, 1, 1, 1,
						0, 0, 1, 1);
	seduce_primitive_circle_add_3d(object,
						center_x, center_y, center_z,
						0, 1, 0,
						0, 0, 1,
						radius,
						4.0 / 6.0, 1.0 / 6.0,
						4.0 / 6.0 * radius * PI * 2.0, 1.0 / 6.0 * radius * PI * 2.0,
						0, 0, 1, 1,
						1, 0, 1, 1);
	seduce_primitive_circle_add_3d(object,
						center_x, center_y, center_z,
						0, 1, 0,
						0, 0, 1,
						radius,
						5.0 / 6.0, 1.0 / 6.0,
						5.0 / 6.0 * radius * PI * 2.0, 1.0 / 6.0 * radius * PI * 2.0,
						1, 0, 1, 1,
						1, 0, 0, 1);

/*
seduce_primitive_spline_add_3d(object, center_x + 0.000000 * radius, center_y + 1.000000 * radius, center_z, center_x + 0.357214 * radius, center_y + 1.000000 * radius, center_z, center_x + 0.687419 * radius, center_y + 0.809356 * radius, center_z, center_x + 0.866025 * radius, center_y + 0.500000 * radius, center_z, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1);
seduce_primitive_spline_add_3d(object, center_x + 0.866025 * radius, center_y + 0.500000 * radius, center_z, center_x + 1.044632 * radius, center_y + 0.190644 * radius, center_z, center_x + 1.044632 * radius, center_y + -0.190644 * radius, center_z, center_x + 0.866025 * radius, center_y + -0.500000 * radius, center_z, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1);
seduce_primitive_spline_add_3d(object, center_x + 0.866025 * radius, center_y + -0.500000 * radius, center_z, center_x + 0.687419 * radius, center_y + -0.809356 * radius, center_z, center_x + 0.357214 * radius, center_y + -1.000000 * radius, center_z, center_x + 0.000000 * radius, center_y + -1.000000 * radius, center_z, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1);
seduce_primitive_spline_add_3d(object, center_x + 0.000000 * radius, center_y + -1.000000 * radius, center_z, center_x + -0.357214 * radius, center_y + -1.000000 * radius, center_z, center_x + -0.687419 * radius, center_y + -0.809356 * radius, center_z, center_x + -0.866025 * radius, center_y + -0.500000 * radius, center_z, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1);
seduce_primitive_spline_add_3d(object, center_x + -0.866025 * radius, center_y + -0.500000 * radius, center_z, center_x + -1.044632 * radius, center_y + -0.190644 * radius, center_z, center_x + -1.044632 * radius, center_y + 0.190644 * radius, center_z, center_x + -0.866025 * radius, center_y + 0.500000 * radius, center_z, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1);
seduce_primitive_spline_add_3d(object, center_x + -0.866025 * radius, center_y + 0.500000 * radius, center_z, center_x + -0.687419 * radius, center_y + 0.809356 * radius, center_z, center_x + -0.357214 * radius, center_y + 1.000000 * radius, center_z, center_x + -0.000000 * radius, center_y + 1.000000 * radius, center_z, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1);
*/
/*
	for(i = 0; i < 6; i++)
	{
		cur = (float)i * PI * 2.0 / 6.0;
		next = (float)(i + 1) * PI * 2.0  / 6.0;
		seduce_primitive_spline_add_3d(object,
								sin(cur), cos(cur), 0,
								sin(cur) + cos(cur) * spacing, cos(cur) - sin(cur) * spacing, 0,
								sin(next) - cos(next) * spacing, cos(next) + sin(next) * spacing, 0,
								sin(next), cos(next), 0,
								(float)i, (float)i + 1.0 / 6.0,
								1, 1, 1, 1,
								1, 1, 1, 1);
	}*/
}		
