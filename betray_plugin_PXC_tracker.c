#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include "betray.h"
#include "relinquish.h"
#include "betray_plugin_pxc_wrapper.h"

#define PXC_DRAW_DEBUG

uint facial_tracking_ray_cast_vertical(uint16 *depth, uint16 *accuracy, uint x_size, uint y_size, uint pixel_x, uint pixel_y)
{
	
	uint i, j, last, x_start, x_end;
	uint16 center;
	float weight;
	if(pixel_x >= x_size)
		pixel_x = x_size - 1;
	if(pixel_y >= y_size)
		pixel_y = y_size - 1;

	center = depth[x_size * pixel_y + pixel_x];
	last = pixel_y;
	for(i = pixel_y; i > 0 && last < 2 + i; i--)
	{
		if(depth[pixel_x + i * x_size] < center + 600)
			last = i;
	}
#ifdef PXC_DRAW_DEBUG
	r_primitive_line_fade_3d((float)pixel_x / (float)x_size - 0.5, 
							-((float)pixel_y - (float)y_size * 0.5) / (float)x_size, 0, 
							(float)pixel_x / (float)x_size - 0.5, 
							((float)y_size * 0.5 - (float)last) / (float)x_size, 0, 
							1, 0, 0, 1,
							1, 0, 0, 1);
#endif
	return last;
}


uint facial_tracking_ray_cast_horizontal(uint16 *depth, uint16 *accuracy, uint x_size, uint y_size, uint pixel_x, uint pixel_y)
{
	
	uint i, j, last, x_start, x_end;
	uint16 *line, *accuracy_line, center;
	float weight;
	line = &depth[x_size * pixel_y];
	accuracy_line = &accuracy[x_size * pixel_y];
	center = line[pixel_x];
	last = pixel_x;
	for(i = last; i < x_size && last + 6 > i; i++)
		if(line[i] < center + 100)
			last = i;
	if(i == x_size)
		return -1;
	x_end = last;
	last = pixel_x;
	for(j = last; j != 0 && last < 6 + j; j--)
	{
		if(line[j] < center + 100)
			last = j;
	}
	if(j == 0)
		return -1;
	x_start = last;
#ifdef PXC_DRAW_DEBUG
	r_primitive_line_fade_3d((float)x_start * 1.0 / (float)x_size - 0.5, 
							((float)y_size * 0.5 - (float)pixel_y) / (float)x_size, 0, 
							(float)x_end * 1.0 / (float)x_size - 0.5, 
							((float)y_size * 0.5 - (float)pixel_y) / (float)x_size, 0, 
							1, 0, 1, 1,
							1, 0, 1, 1);
#endif
	return (x_start + x_end) / 2;
}

boolean facial_tracking_center_head(uint16 *depth, uint16 *accuracy, uint *ouptput, uint x_size, uint y_size, uint pixel_x, uint pixel_y)
{
	int i, j, sum, add, d;

	d = depth[pixel_y * x_size + pixel_x];
	pixel_x = facial_tracking_ray_cast_horizontal(depth, accuracy, x_size, y_size, pixel_x, pixel_y);
	if(pixel_x == -1)
		return FALSE;
	pixel_y = facial_tracking_ray_cast_vertical(depth, accuracy, x_size, y_size, pixel_x, pixel_y);
	if(pixel_y == -1)
		return FALSE;
//	pixel_y += (30 * 500) / d;
	pixel_y += (60 * 500) / d;
	sum = j = 0;
	for(i = pixel_y - ((24 * 500) / d); i < pixel_y + ((16 * 500) / d); i++)
	{
		if(i >= 0 && i < 240)
		{
			add = facial_tracking_ray_cast_horizontal(depth, accuracy, x_size, y_size, pixel_x, i);
			if(add == -1)
				return FALSE;
			sum += add;
			j++;
		}
	}
	if(j == 0)
		return FALSE;
	pixel_x = sum / j;
	if(pixel_x > x_size)
		return FALSE;
	sum = j = 0;
	for(i = pixel_x - ((12 * 500) / d); i < pixel_x + ((12 * 500) / d); i++)
	{
		if(i >= 0 && i < 240)
		{
			add = facial_tracking_ray_cast_vertical(depth, accuracy, x_size, y_size, i, pixel_y);
			if(add == -1)
				return FALSE;
			sum += add;
			j++;
		}	
	}
	if(j == 0)
		return FALSE;
//	pixel_y = (uint)(sum / (float)j + (30 * 500) / d);
	pixel_y = (uint)(sum / (float)j + (60 * 500) / d);
	if(pixel_y > 218)
		pixel_y = 218;
	if(pixel_y < 22)
		pixel_y = 22;
	ouptput[0] = pixel_x;
	ouptput[1] = pixel_y;
	return TRUE;
}

float facial_tracking_box_depth( uint16 *depth, uint16 *accuracy, uint x_size, uint y_size, uint pixel_x, uint pixel_y, uint size)
{
	uint i, j;
	uint16 center;
	float weight;
	
	center = depth[x_size * pixel_y + pixel_x] + 80;
	if(pixel_x < size)
		pixel_x = size;
	if(pixel_x > x_size - size)
		pixel_x = x_size - size;
	if(pixel_y < size)
		pixel_y = size;
	if(pixel_y > y_size - size)
		pixel_y = y_size - size;
	weight = 0;
	for(i = pixel_y - size; i < pixel_y + size; i++)
	{
		for(j = pixel_x - size; j < pixel_x + size; j++)
		{
			if(depth[j + i * x_size] < center)
			{
				weight += (float)depth[j + i * x_size];
			}
		}
	}
	return weight = weight / ((size * 2) * (size * 2));
}
void facial_tracking_box_average(float *output, uint16 *depth, uint16 *accuracy, uint x_size, uint y_size, uint pixel_x, uint pixel_y, uint size)
{
	
	uint i, j, last, x_start, x_end;
	uint16 center, dist = 30;
	float weight, x, y, f;
	boolean test;

	center = depth[x_size * pixel_y + pixel_x];
	if(center != 0)
		size = (size * 500) / center;

	if(size > y_size / 2)
		size = y_size / 2;

	if(pixel_x < size)
		pixel_x = size;
	if(pixel_x > x_size - size)
		pixel_x = x_size - size;
	if(pixel_y < size)
		pixel_y = size;
	if(pixel_y > y_size - size)
		pixel_y = y_size - size;

	output[2] = center = facial_tracking_box_depth(depth, accuracy, x_size, y_size, pixel_x,  pixel_y, size / 4);

	weight = x = y = 0;
	
	for(i = pixel_y - size; i < pixel_y + size; i++)
	{
		test = FALSE;
		for(j = pixel_x - size; j < pixel_x + size; j++)
		{
			if(depth[j + i * x_size] < center + dist)
			{
				f = (float)(depth[j + i * x_size] - (center + dist)) + (float)accuracy[j + i * x_size];
				y += f * (float)i;
				x += f * (float)j;
				weight += f;
				test = TRUE;
			}
		}
		if(!test && i > pixel_y)
			break;
	}
	output[0] = x / weight;
	output[1] = y / weight;
}

boolean facial_tracking_head_detect_old(uint16 *depth, uint16 *accuracy, uint x_size, uint y_size, uint pixel_x, uint pixel_y)
{
	uint16 center;
	uint i, end;
	center = depth[x_size * pixel_y + pixel_x];
	printf("center %u\n", (uint)center);
	if(pixel_x + (40 * center) / 500 < x_size)
		end = pixel_x + (40 * center) / 500;
	else
		end = x_size;
	for(i = pixel_x;  i < end; i++)
	{
		if(depth[x_size * pixel_y + i] < center - (80 * center) / 500)
			return FALSE;
		if(depth[x_size * pixel_y + i] > center + (80 * center) / 500)
			break;
	}
	if(i == end)
		return FALSE;
	if(pixel_x < (40 * center) / 500)
		end = 0;
	else
		end = pixel_x -  (40 * center) / 500;
	for(i = pixel_x; i > end; i--)
	{
		if(depth[x_size * pixel_y + i] < center -  (80 * center) / 500)
			return FALSE;
		if(depth[x_size * pixel_y + i] > center + (80 * center) / 500)
			break;
	}

	if(i == end)
		return FALSE;

	if(i == end)
		return FALSE;
	if(pixel_y <  (60 * center) / 500)
		end = 0;
	else
		end = pixel_y - (60 * center) / 500;
	for(i = pixel_y; i > end; i--)
	{
		if(depth[x_size * i + pixel_x] < center - (80 * center) / 500)
			return FALSE;
		if(depth[x_size * i + pixel_x] > center + (80 * center) / 500)
			break;
	}

	if(i == end)
		return FALSE;
	return TRUE;
}


boolean facial_tracking_head_detect(uint16 *depth, uint16 *accuracy, uint x_size, uint y_size, uint pixel_x, uint pixel_y)
{
	uint16 center;
	uint i, end, size;
	center = depth[x_size * pixel_y + pixel_x];
	if(center == 0)
		return FALSE;
//	printf("center %u %u\n", (uint)center, (40 * center) / 500);
	if(pixel_x + (80 * 500) / center < x_size)
		end = pixel_x + (80 * 500) / center;
	else
		end = x_size;
	for(i = pixel_x;  i < end; i++)
	{
	/*	if(depth[x_size * pixel_y + i] < center - (80 * 500) / center)
			return FALSE;*/
		if(depth[x_size * pixel_y + i] > center + (1000 * 500) / center)
			break;
	}
	size = i;
	if(i == end)
		return FALSE;
	if(pixel_x < (80 * 500) / center)
		end = 0;
	else
		end = pixel_x - (80 * 500) / center;
	for(i = pixel_x; i > end; i--)
	{
	/*	if(depth[x_size * pixel_y + i] < center -  (80 * 500) / center)
			return FALSE;*/
		if(depth[x_size * pixel_y + i] > center + (1000 * 500) / center)
			break;
	}
	size = size - i;

	if(i == end)
		return FALSE;


	size = size * center;

/*	if(size > 160 || size < 100)
		return FALSE;*/
	if(size < 50000)
		return FALSE;
/*		r_primitive_line_2d(((float)pixel_x - 160 - (100 * 500) / center) / (float)x_size,
							(120 - (float)pixel_y) / (float)x_size,
							((float)pixel_x - 160 + (100 * 500) / center) / (float)x_size,
							(120 - (float)pixel_y) / (float)x_size, 0, 1, 0, 1);*/
	else
		r_primitive_line_2d(((float)pixel_x - 160 - (100 * 500) / center) / (float)x_size,
							(120 - (float)pixel_y) / (float)x_size,
							((float)pixel_x - 160 + (100 * 500) / center) / (float)x_size,
							(120 - (float)pixel_y) / (float)x_size, 1, 1, 0, 1);
//	return TRUE;


	if(pixel_y <  (80 * 500) / center)
		end = 0;
	else
	{
		end = pixel_y - (80 * 500) / center;
	}
	for(i = pixel_y; i > end; i--)
	{
/*		if(depth[x_size * i + pixel_x] < center - (80 * 500) / center)
			return FALSE;*/
		if(depth[x_size * i + pixel_x] > center + (2000 * 500) / center)
			break;
	}

	if(i == end)
		return FALSE;
	return TRUE;
}

boolean facial_tracking_head_find(uint16 *depth, uint16 *accuracy, uint *output_x, uint *output_y, uint x_size, uint y_size)
{
	float fpos[2];
	uint16 best, sample;
	uint i, j, found_x, found_y;
	boolean found = FALSE;
	best = 256 * 256 - 1;
	for(i = 10; i < x_size - 10; i += 2)
	{
		for(j = 10; j < y_size - 10; j += 2)
		{
			fpos[0] = ((float)i - 160) / (float)x_size;
			fpos[1] = (120 - (float)j) / (float)x_size;
			if(facial_tracking_head_detect(depth,  accuracy, x_size, y_size, i, j))
			{
				sample = depth[320 * j + i];
				if(sample < best)
				{
					found = TRUE;
					best = sample;
					*output_x = i;
					*output_y = j;
				}
#ifdef PXC_DRAW_DEBUG
				r_primitive_line_2d(fpos[0] + 0.01, fpos[1] + 0.01, fpos[0] - 0.01, fpos[1] - 0.01, 0, 1, 0, 1);
				r_primitive_line_2d(fpos[0] + 0.01, fpos[1] - 0.01, fpos[0] - 0.01, fpos[1] + 0.01, 0, 1, 0, 1);
#endif
			}else
			{
#ifdef PXC_DRAW_DEBUG
//				r_primitive_line_2d(fpos[0] + 0.01, fpos[1] + 0.01, fpos[0] - 0.01, fpos[1] - 0.01, 1, 0, 0, 1);
//				r_primitive_line_2d(fpos[0] + 0.01, fpos[1] - 0.01, fpos[0] - 0.01, fpos[1] + 0.01, 1, 0, 0, 1);
#endif
			}
		}
	}
	fpos[0] = ((float)*output_x - 160) / 320.0;
	fpos[1] = (120 - (float)*output_y) / 320.0;
	return found;
}




void pxc_facial_tracking_algorithem(float *view_pos, float *image_pos, uint16 *buf_depth, uint16 *buf_accuracy, uint buffer_size_x, uint buffer_size_y, float delta_time, float screen_size, float x_displace, float y_displace)
{
	static uint x_pos = 160, y_pos = 120, i_pos[3] = {160, 120, 1000};
	static float x_posf = 0, y_posf = 0, z_posf = 0, output[3], img_output[3];
	boolean found;
	uint x, y, i, j, k, texture_id, depth_id;
	float aspect, sum, f;

	x_pos = buffer_size_x / 2;
	y_pos = buffer_size_y / 2;
	x_posf = 0;
	y_posf = 0;
	z_posf = 0;
	found = facial_tracking_head_find(buf_depth,  buf_accuracy, &i_pos[0], &i_pos[1], buffer_size_x, buffer_size_y);
/*	return;
	if(i_pos[0] > buffer_size_x || i_pos[1] > buffer_size_y || !facial_tracking_head_detect(buf_depth,  buf_accuracy, buffer_size_x, buffer_size_y, i_pos[0], i_pos[1]))
	{
		found = facial_tracking_head_find(buf_depth,  buf_accuracy, &i_pos[0], &i_pos[1], buffer_size_x, buffer_size_y);
	}else
		found = TRUE;
*/	if(found)
	{
		found = FALSE;
#ifdef PXC_DRAW_DEBUG
		r_primitive_line_2d(x_posf * -0.5 + 0.01, y_posf * 0.5 + 0.01, x_posf * -0.5 - 0.01, y_posf * 0.5 - 0.01, 1, 0, 0, 1);
		r_primitive_line_2d(x_posf * -0.5 + 0.01, y_posf * 0.5 - 0.01, x_posf * -0.5 - 0.01, y_posf * 0.5 + 0.01, 1, 0, 0, 1);
#endif
		if(facial_tracking_center_head(buf_depth,  buf_accuracy, i_pos, buffer_size_x, buffer_size_y, i_pos[0], i_pos[1]))
		{
			output[0] = i_pos[0];
			output[1] = i_pos[1];
			f = (float)buffer_size_x / 2.0;
			output[0] = (f  - output[0]) / f;
			output[1] = (((float)buffer_size_y / 2.0)  - output[1]) / f;
	#ifdef PXC_DRAW_DEBUG
			r_primitive_line_2d(output[0] * -0.5 + 0.1, output[1] * 0.5 + 0.1, output[0] * -0.5 - 0.1, output[1] * 0.5 - 0.1, 1, 1, 0, 1);
			r_primitive_line_2d(output[0] * -0.5 + 0.1, output[1] * 0.5 - 0.1, output[0] * -0.5 - 0.1, output[1] * 0.5 + 0.1, 1, 1, 0, 1);
	#endif
			facial_tracking_box_average(output, buf_depth, buf_accuracy,  buffer_size_x, buffer_size_y, i_pos[0], i_pos[1], 60);

			if((output[0] - i_pos[0]) * (output[0] - i_pos[0]) + (output[1] - i_pos[1]) + (output[1] - i_pos[1]) > 16 * 16)
			{
				output[0] = i_pos[0];
				output[1] = i_pos[1];
			}
			output[2] = facial_tracking_box_depth(buf_depth, buf_accuracy, buffer_size_x, buffer_size_y, i_pos[0],  i_pos[1], 4);
			f = (float)buffer_size_x / 2.0;

			printf("pre output %f %f %f - screen %f\n", output[0], output[1], output[2], screen_size);
			output[0] = (f  - output[0]) / f;
			output[1] = (((float)buffer_size_y / 2.0)  - output[1]) / f;
			

			printf("post output %f %f %f - screen %f\n", output[0], output[1], output[2], screen_size);
#ifdef PXC_DRAW_DEBUG
			r_primitive_line_2d(output[0] * -0.5 + 0.1, output[1] * 0.5 + 0.1, output[0] * -0.5 - 0.1, output[1] * 0.5 - 0.1, 0, 1, 1, 1);
			r_primitive_line_2d(output[0] * -0.5 + 0.1, output[1] * 0.5 - 0.1, output[0] * -0.5 - 0.1, output[1] * 0.5 + 0.1, 0, 1, 1, 1);
#endif
			img_output[0] = output[0];
			img_output[1] = output[1];
			img_output[2] = output[2];

			output[2] = output[2] / screen_size;

			output[2] = output[2] * 1.28;
			output[0] = (output[0] * 1.28 * output[2] + x_displace);
			output[1] = (output[1] * 1.28 * output[2] + y_displace);
			for(i = 0; i < 3; i++)
			{
				if(output[i] > 8.0)
					output[i] = 8.0;
				if(output[i] < -8.0)
					output[i] = -8.0;
			}
			found = TRUE;
		}
	}
	delta_time = 0.01;
	if(found)
	{
		f = (output[0] - view_pos[0]) * (output[0] - view_pos[0]) + (output[1] - view_pos[1]) * (output[1] - view_pos[1]) + (output[2] - view_pos[2]) / 2.0 * (output[2] - view_pos[2]) / 2.0;
	/*	if(0.5 * 0.5 < f)
		{
			delta_time *= 5.0;
			view[0] = view[0] * (1 - delta_time) + output[0] * delta_time;
			view[1] = view[1] * (1 - delta_time) + output[1] * delta_time;
			view[2] = view[2] * (1 - delta_time) + output[2] * delta_time;
		}else*/ if(0.002 * 0.002 < f)
		{
			view_pos[0] = output[0];
			view_pos[1] = output[1];
			view_pos[2] = output[2];	
			image_pos[0] = img_output[0];
			image_pos[1] = img_output[1];
			image_pos[2] = img_output[2];
		}
	}else
	{
	/*	view[0] = view[0] * (1 - delta_time);
		view[1] = view[1] * (1 - delta_time);
		view[2] = view[2] * (1 - delta_time) + 2.0 * delta_time;*/
	}
}

// 380.0 size
// 0.0
// 0.6  float screen_size, float x_displace, float y_displace)

/*
void pxc_facial_tracking(float *view, float delta_time, float screen_size, float x_displace, float y_displace)
{
	static boolean init = FALSE, active = FALSE;
	static float pos[3] = {0, 0, 2};
	unsigned char *buf_color;
	unsigned short *buf_depth;
	unsigned short *buf_accuracy;
	float *uvmap = 0;
	PXCFaceData data;
//	exit(0);
	if(!init)
	{
		if(0 == pxc_init(640, 480))
			active = TRUE;
		init = TRUE;
	}
	printf("screen_size %f\n", screen_size);
	if(active)
	{
		pxc_get_frame(&buf_color, &buf_depth, &buf_accuracy, &uvmap, &data);
		pxc_facial_tracking_algorithem(pos, NULL, buf_depth, buf_accuracy, 320, 240, delta_time, screen_size, x_displace, y_displace);
		pxc_release_frame(); 
	}
	view[0] = pos[0];
	view[1] = pos[1];
	view[2] = pos[2];
}
*/