#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "seduce.h"
#include "s_draw_3d.h"

void s_widget_draw_lines(float x, float y, float length, float height)
{
/*	r_primitive_line_2d(x, y, x + length, y, 0, 0, 1, 1);
	r_primitive_line_2d(x, y - height, x + length, y - height, 0, 0, 1, 1);
	r_primitive_line_flush();*/
}

extern void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float scale, uint id, float fade,  float *color);

void s_widget_slider(BInputState *input, float pos_x, float pos_y, float length, float scale, char *text, float *value, void *id, float time, float red, float green, float blue)
{
	static void **grab = NULL;

	if(grab == NULL)
	{
		uint i, count;
		count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
			grab[i] = NULL;
	}

	if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				if(input->pointers[i].pointer_x > pos_x &&
					input->pointers[i].pointer_x < pos_x + length &&
					input->pointers[i].pointer_y > pos_y - scale * SEDUCE_T_SIZE * 3.0 &&
					input->pointers[i].pointer_y < pos_y)
					grab[i] = id;
			if(!input->pointers[i].button[0])
				grab[i] = NULL;
			
			if(grab[i] == id)
			{
				float f;
				*value = (input->pointers[i].pointer_x - pos_x - 16 * SEDUCE_T_SIZE * scale) / (length - 16 * SEDUCE_T_SIZE * scale);
				if(*value < 0.0)
					*value = 0.0;
				if(*value > 1.0)
					*value = 1.0;
			}
		}
	}

	
	if(input->mode == BAM_DRAW)
	{
	}
}


void s_widget_slider_transform(float *transform, float pos_a_x, float pos_a_y, float pos_b_x, float pos_b_y, float pointer_x, float pointer_y)
{
	float f, vec[2];
	vec[0] = pos_b_x - pos_a_x;
	vec[1] = pos_b_y - pos_a_y;
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	vec[0] /= f; 
	vec[1] /= f;
	transform[0] = ((pointer_x - pos_a_x) * vec[0] + 
					(pointer_y - pos_a_y) * vec[1]) / f;
	transform[1] = (pointer_x - pos_a_x) * vec[1] - 
						(pointer_y - pos_a_y) * vec[0];
}

void s_widget_slider_internal(BInputState *input, float pos_a_x, float pos_a_y, float pos_b_x, float pos_b_y, float scale, float time, float *value, void *id, float *color, float *color_text, float *color_low, float *color_high)
{
	static void **grab = NULL;

	if(grab == NULL)
	{
		uint i, count;
		count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
			grab[i] = NULL;
	}

	if(input->mode == BAM_EVENT)
	{
		uint i;
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			{
				float transformed[2];
				s_widget_slider_transform(transformed, pos_a_x, pos_a_y, pos_b_x, pos_b_y, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
				if(transformed[0] > scale * -0.02 && transformed[0] < 1.0 + scale * 0.02 && transformed[1] > scale * -0.02 && transformed[1] < scale * 0.02)
					grab[i] = id;
			}
			if(input->pointers[i].button[0])
			{
				if(grab[i] == id)
				{
					float transformed[2], vec[2], f, steps;

					vec[0] = pos_a_x - pos_b_x;
					vec[1] = pos_a_y - pos_b_y;
					f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);

					steps = 1.0;
					while((float)steps * 0.03 * scale < f)
					{
						steps *= 2.0;
						if((float)steps * 0.03 * scale >= f)
							break;
						steps *= 5.0;
					}
					s_widget_slider_transform(transformed, pos_a_x, pos_a_y, pos_b_x, pos_b_y, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
					if(transformed[1] > scale * 0.04 && transformed[1] < scale * 0.1)
						transformed[0] = (float)((int)((transformed[0] + 1.0 / (steps * 2.0)) * steps)) / steps;


					steps = 1.0;
					while((float)steps * 0.03 * scale < f)
						steps *= 2.0;
					if(transformed[1] < scale * -0.04 && transformed[1] > scale * -0.1)
						transformed[0] = (float)((int)((transformed[0] + 1.0 / (steps * 2.0)) * steps)) / steps;

					*value = transformed[0];
					if(*value > 1.0)
						*value = 1.0;
					if(*value < 0.0)
						*value = 0.0;
				}
			}
			else
				grab[i] = NULL;
		}
	}

	
	if(input->mode == BAM_DRAW)
	{
		float pos[2], vec[2], f, f2, length, transformed[2], tint, fade, matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		uint i, snap = -1, steps;
		char text[32];
		text[0] = 0;
		vec[0] = pos_a_x - pos_b_x;
		vec[1] = pos_a_y - pos_b_y;
		length = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		vec[0] /= length;
		vec[1] /= length;

		for(i = 0; i < input->pointer_count; i++)
			if(grab[i] == id)
				break;
		if(i < input->pointer_count)
		{
			steps = 1;
			while((float)steps * 0.03 * scale < length)
				steps *= 2;

			for(; i < input->pointer_count; i++)
			{
				if(grab[i] == id)
				{
					s_widget_slider_transform(transformed, pos_a_x, pos_a_y, pos_b_x, pos_b_y, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
				/*	if(transformed[1] > scale * 0.04 && transformed[1] < scale * 0.1)
						snap = TRUE;*/
					if(transformed[1] < scale * -0.04 && transformed[1] > scale * -0.1)
						snap = (int)((transformed[0] + 1.0 / (steps * 2.0)) * steps);
				}
			}


			if(snap != -1)
				tint = time * 0.5;
			else
				tint = time * 0.2;
			for(i = 0; i < steps + 1; i++)
			{
				f = (float)i / (float)steps;
				pos[0] = pos_a_x + (pos_b_x - pos_a_x) * f;
				pos[1] = pos_a_y + (pos_b_y - pos_a_y) * f;
				if(i % 2 == 0)
				{
					if(i % 8 == 0)
						f = scale * 0.08;
					else
						f = scale * 0.06;

					r_primitive_line_fade_3d(pos[0] + vec[1] * scale * 0.04, 
												pos[1] - vec[0] * scale * 0.04,
												0.0,
												pos[0] + vec[1] * f, 
												pos[1] - vec[0] * f,
												0.0,
												color[0], color[1], color[2], tint, 
												color[0], color[1], color[2], tint);
				}
				r_primitive_line_fade_3d(pos[0] + vec[1] * scale * 0.04, 
											pos[1] - vec[0] * scale * 0.04,
											0.0,
											pos[0] + vec[1] * scale * 0.01, 
											pos[1] - vec[0] * scale * 0.01,
											scale * -0.01,
											color[0], color[1], color[2], tint, 
											color[0], color[1], color[2], 0.0);

			}
			pos[0] = pos_a_x + (pos_b_x - pos_a_x) * *value;
			pos[1] = pos_a_y + (pos_b_y - pos_a_y) * *value;
			r_primitive_line_fade_3d(pos[0] + vec[1] * scale * 0.04, 
											pos[1] - vec[0] * scale * 0.04,
											0.0,
											pos[0] + vec[1] * scale * 0.08, 
											pos[1] - vec[0] * scale * 0.08,
											0.0,
											color_low[0] + (color_high[0] - color_low[0]) * *value,
											color_low[1] + (color_high[1] - color_low[1]) * *value,
											color_low[2] + (color_high[2] - color_low[2]) * *value, 1, 
											color_low[0] + (color_high[0] - color_low[0]) * *value,
											color_low[1] + (color_high[1] - color_low[1]) * *value,
											color_low[2] + (color_high[2] - color_low[2]) * *value, 1);
			pos[0] = pos_a_x + (pos_b_x - pos_a_x) * *value;
			pos[1] = pos_a_y + (pos_b_y - pos_a_y) * *value;
			r_primitive_line_fade_3d(pos[0] + vec[1] * scale * -0.04, 
											pos[1] - vec[0] * scale * -0.04,
											0.0,
											pos[0] + vec[1] * scale * -0.08, 
											pos[1] - vec[0] * scale * -0.08,
											0.0,
											color_low[0] + (color_high[0] - color_low[0]) * *value,
											color_low[1] + (color_high[1] - color_low[1]) * *value,
											color_low[2] + (color_high[2] - color_low[2]) * *value, 1, 
											color_low[0] + (color_high[0] - color_low[0]) * *value,
											color_low[1] + (color_high[1] - color_low[1]) * *value,
											color_low[2] + (color_high[2] - color_low[2]) * *value, 1);

			tint = time;
			if(snap == -1)
				tint = time * 0.1;
			for(i = 0; i < steps * 4; i += 4)
			{

				f = (float)(i + 1) / (float)(steps * 4);
				f2 = (float)(i + 3) / (float)(steps * 4);
				r_primitive_line_fade_3d(pos_a_x + (pos_b_x - pos_a_x) * f + vec[1] * scale * 0.04, 
												pos_a_y + (pos_b_y - pos_a_y) * f - vec[0] * scale * 0.04,
												0.0,
												pos_a_x + (pos_b_x - pos_a_x) * f2 + vec[1] * scale * 0.04, 
												pos_a_y + (pos_b_y - pos_a_y) * f2 - vec[0] * scale * 0.04,
												0.0,
												color[0], color[1], color[2], tint, 
												color[0], color[1], color[2], tint);

			}

			if(snap != -1)
			{
				while(snap % 2 == 0 && steps % 2 == 0)
				{
					snap /= 2;
					steps /= 2;
				}
				sprintf(text, "%u / %u", snap, steps);
			}
/*decimal */

			steps = 1;
			while((float)steps * 0.03 * scale < length)
			{
				steps *= 2;
				if((float)steps * 0.03 * scale >= length)
					break;
				steps *= 5;
			}
			snap = -1;
			for(i = 0; i < input->pointer_count; i++)
			{
				if(grab[i] == id)
				{
					s_widget_slider_transform(transformed, pos_a_x, pos_a_y, pos_b_x, pos_b_y, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
					if(transformed[1] > scale * 0.04 && transformed[1] < scale * 0.1)
						snap = (int)((transformed[0] + 1.0 / (steps * 2.0)) * steps);
				/*	if(transformed[1] < scale * -0.04 && transformed[1] > scale * -0.1)
						snap = TRUE;*/
				}
			}

			if(snap != -1)
				tint = time * 0.5;
			else
				tint = time * 0.2;
			for(i = 0; i < steps + 1; i++)
			{
				f = (float)i / (float)steps;
				pos[0] = pos_a_x + (pos_b_x - pos_a_x) * f;
				pos[1] = pos_a_y + (pos_b_y - pos_a_y) * f;
				if(i % 5 == 0)
				{
					if(i % 2 == 0)
						f = scale * -0.08;
					else
						f = scale * -0.06;

					r_primitive_line_fade_3d(pos[0] + vec[1] * scale * -0.04, 
												pos[1] - vec[0] * scale * -0.04,
												0.0,
												pos[0] + vec[1] * f, 
												pos[1] - vec[0] * f,
												0.0,
												color[0], color[1], color[2], tint, 
												color[0], color[1], color[2], tint);
				}
				r_primitive_line_fade_3d(pos[0] + vec[1] * scale * -0.04, 
											pos[1] - vec[0] * scale * -0.04,
											0.0,
											pos[0] + vec[1] * scale * -0.01, 
											pos[1] - vec[0] * scale * -0.01,
											scale * -0.01,
											color[0], color[1], color[2], tint, 
											color[0], color[1], color[2], 0.0);

			}

			tint = time;
			if(snap == -1)
				tint = time * 0.1;
			for(i = 0; i < steps * 4; i += 4)
			{

				f = (float)(i + 1) / (float)(steps * 4);
				f2 = (float)(i + 3) / (float)(steps * 4);
				r_primitive_line_fade_3d(pos_a_x + (pos_b_x - pos_a_x) * f + vec[1] * scale * -0.04, 
												pos_a_y + (pos_b_y - pos_a_y) * f - vec[0] * scale * -0.04,
												0.0,
												pos_a_x + (pos_b_x - pos_a_x) * f2 + vec[1] * scale * -0.04, 
												pos_a_y + (pos_b_y - pos_a_y) * f2 - vec[0] * scale * -0.04,
												0.0,
												color[0], color[1], color[2], tint, 
												color[0], color[1], color[2], tint);


			}
			if(snap != -1)
			{
				if(snap < 101)
					sprintf(text, "%u%%", (uint)(*value * 100.001));
				else if(snap < 1001)
					sprintf(text, "%u.%u%%", (uint)(*value * 1000.001) / 10, (uint)(*value * 1000.001) % 10);
				else
					sprintf(text, "%u.%u%%", (uint)(*value * 10000.001) / 100, (uint)(*value * 10000.001) % 100);
			}

		}

		if(text[0] == 0)
			sprintf(text, "%.4f", *value);

		matrix[0] = -vec[0];
		matrix[1] = -vec[1];
		matrix[4] = vec[1];
		matrix[5] = -vec[0];
		if(*value < 0.5)
		{
			matrix[12] = pos_a_x - vec[0] * *value * length - vec[0] * 0.02 * scale;
			matrix[13] = pos_a_y - vec[1] * *value * length - vec[1] * 0.02 * scale;

			if(0 < *value * length - 0.02 * scale)
			{
				fade = *value - (0.02 * scale) / length;
				r_primitive_line_fade_2d(pos_a_x, 
												pos_a_y,
												pos_a_x - vec[0] * (*value * length - 0.02 * scale),
												pos_a_y - vec[1] * (*value * length - 0.02 * scale),
												color_low[0], color_low[1], color_low[2], time, 
												color_low[0] + (color_high[0] - color_low[0]) * fade,
												color_low[1] + (color_high[1] - color_low[1]) * fade,
												color_low[2] + (color_high[2] - color_low[2]) * fade, time);
				r_primitive_line_fade_3d(pos_a_x + vec[1] * scale * 0.005, 
												pos_a_y - vec[0] * scale * 0.005,
												0.0,
												pos_a_x - vec[1] * scale * 0.005, 
												pos_a_y + vec[0] * scale * 0.005,
												0.0,
												color_low[0], color_low[1], color_low[2], 1, 
												color_low[0], color_low[1], color_low[2], 1);
			}
//float *color_low, float *color_high
			f = seduce_text_line_length(NULL, SEDUCE_T_SIZE * scale, SEDUCE_T_SPACE, text, -1) + 0.03 * scale + *value * length;
			if(f < length)
			{
				fade = f / length;
				r_primitive_line_fade_2d(pos_b_x, 
												pos_b_y,
												pos_a_x - vec[0] * f,
												pos_a_y - vec[1] * f,
												color_high[0], color_high[1], color_high[2],  time,
												color_low[0] + (color_high[0] - color_low[0]) * fade,
												color_low[1] + (color_high[1] - color_low[1]) * fade,
												color_low[2] + (color_high[2] - color_low[2]) * fade,  time);
				r_primitive_line_fade_3d(pos_b_x + vec[1] * scale * 0.005, 
												pos_b_y - vec[0] * scale * 0.005,
												0.0,
												pos_b_x - vec[1] * scale * 0.005, 
												pos_b_y + vec[0] * scale * 0.005,
												0.0,
												color_high[0], color_high[1], color_high[2], 1, 
												color_high[0], color_high[1], color_high[2], 1);
			}
		}else
		{

			f = seduce_text_line_length(NULL, SEDUCE_T_SIZE * scale, SEDUCE_T_SPACE, text, -1) + 0.02 * scale;
			matrix[12] = pos_a_x - vec[0] * *value * length + vec[0] * f;
			matrix[13] = pos_a_y - vec[1] * *value * length + vec[1] * f;

			if(*value * length + 0.02 * scale < length)
			{
				fade = *value + 0.02 * scale / length;
				r_primitive_line_fade_2d(pos_b_x, 
												pos_b_y,
												pos_a_x - vec[0] * *value * length - vec[0] * 0.02 * scale,
												pos_a_y - vec[1] * *value * length - vec[1] * 0.02 * scale,
												color_high[0], color_high[1], color_high[2], time,
												color_low[0] + (color_high[0] - color_low[0]) * fade,
												color_low[1] + (color_high[1] - color_low[1]) * fade,
												color_low[2] + (color_high[2] - color_low[2]) * fade, time);
				r_primitive_line_fade_3d(pos_b_x + vec[1] * scale * 0.005, 
												pos_b_y - vec[0] * scale * 0.005,
												0.0,
												pos_b_x - vec[1] * scale * 0.005, 
												pos_b_y + vec[0] * scale * 0.005,
												0.0,
												color_high[0], color_high[1], color_high[2], 1, 
												color_high[0], color_high[1], color_high[2], 1);
			}
			f += 0.01 * scale - *value * length;
			if(f < 0)
			{
				fade = -f / length;
				r_primitive_line_fade_2d(pos_a_x, 
												pos_a_y,
												pos_a_x + vec[0] * f,
												pos_a_y + vec[1] * f,
												color_low[0], color_low[1], color_low[2], time, 
												color_low[0] + (color_high[0] - color_low[0]) * fade,
												color_low[1] + (color_high[1] - color_low[1]) * fade,
												color_low[2] + (color_high[2] - color_low[2]) * fade, time);
				r_primitive_line_fade_3d(pos_a_x + vec[1] * scale * 0.005, 
												pos_a_y - vec[0] * scale * 0.005,
												0.0,
												pos_a_x - vec[1] * scale * 0.005, 
												pos_a_y + vec[0] * scale * 0.005,
												0.0,
												color_low[0], color_low[1], color_low[2], 1, 
												color_low[0], color_low[1], color_low[2], 1);
			}
	/*
			f += 0.01 * scale;
			r_primitive_line_fade_2d(pos_a_x, 
											pos_a_y,
											pos_a_x - vec[0] * *value * length + vec[0] * f,
											pos_a_y - vec[1] * *value * length + vec[1] * f,
											color[0], color[1], color[2], time, 
											color[0], color[1], color[2], time);			
			*/

		}
		r_primitive_line_flush();
		{
			RMatrix	*m;
			m = r_matrix_get();
			r_matrix_push(m);
			//seduce_text_line_length(NULL, size, spacing, &text[scroll_start], select_end - scroll_start);
			seduce_text_line_draw(NULL, 0, -0.5 * SEDUCE_T_SIZE * scale, SEDUCE_T_SIZE * scale, SEDUCE_T_SPACE, text, color_text[0], color_text[1], color_text[2], time, -1);
			r_matrix_pop(m);
			r_matrix_push(m);
			r_matrix_translate(m, pos_a_x + (pos_b_x - pos_a_x) * *value, 
									pos_a_y + (pos_b_y - pos_a_y) * *value, 0);
			r_matrix_scale(m, scale * 0.02, scale * 0.02, scale * 0.02);
			r_matrix_pop(m);
		}



/*
	r_matrix_push(matrix);
		r_matrix_translate(matrix, pos_x + length - scale * SEDUCE_T_SIZE, pos_y + scale * SEDUCE_T_SIZE * -1.5, 0);
		r_matrix_scale(matrix, 0.08 * scale * time, 0.08 * scale * time, 0.08 * scale * time);

*/
	}
}

void s_widget_slider_turn(BInputState *input, float pos_a_x, float pos_a_y, float pos_b_x, float pos_b_y, float scale, float time, float *value, void *id, float *color, float *color_text, float *color_low, float *color_high)
{
	if(pos_a_x < pos_b_x)
		s_widget_slider_internal(input, pos_a_x, pos_a_y, pos_b_x, pos_b_y, scale, time, value, id, color, color_text, color_low, color_high);
	else
		s_widget_slider_internal(input, pos_b_x, pos_b_y, pos_a_x, pos_a_y, scale, time, value, id, color, color_text, color_low, color_high);
}

void s_widget_slider_new(BInputState *input, float pos_a_x, float pos_a_y, float pos_b_x, float pos_b_y, float scale, float time, float *value, void *id, float *color, float *color_text, float *color_low, float *color_high)
{
	s_widget_slider_turn(input, pos_a_x, pos_a_y, pos_b_x, pos_b_y, scale, time, value, id, color, color_text, color_low, color_high);
}

typedef struct{
	void *id;
	float time;
}SSelectGrab;


void s_widget_select(BInputState *input, float pos_x, float pos_y, float length, float scale, char *text, uint *select, uint count, char **options, void *id, float time, float red, float green, float blue, float select_red, float select_green, float select_blue)
{
	static SSelectGrab *grab = NULL;
	uint i, out;

	if(grab == NULL)
	{
		uint count;
		count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
			grab[i].id = NULL;
	}

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			{	if(input->pointers[i].pointer_x > pos_x &&
					input->pointers[i].pointer_x < pos_x + length &&
					input->pointers[i].pointer_y > pos_y - scale * SEDUCE_T_SIZE * 3.0 &&
					input->pointers[i].pointer_y < pos_y)
				{
					grab[i].id = id;
					grab[i].time = 0.0;
				}
			}
			if(!input->pointers[i].button[0] && grab[i].time < 0.01)
				grab[i].id = NULL;
		}
	}
	for(i = 0; i < input->pointer_count; i++)
	{
		if(grab[i].id == id)
		{
			out = seduce_popup_simple(input, input->pointers[i].user_id, input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0], options, count, &grab[i].time, input->pointers[i].button[0], red, green, blue, red, green, blue);
			if(out != -1 && input->mode == BAM_EVENT)
				*select = out;
		}
	}	
	if(input->mode == BAM_DRAW)
	{
		seduce_text_line_draw(NULL, pos_x, pos_y + scale * SEDUCE_T_SIZE * -2.0, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE + (1 - time) * 3.0, text, red, green, blue, time, -1);
		seduce_text_line_draw(NULL, pos_x + length - scale * seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE + (1 - time) * 3.0, options[*select], -1), pos_y + scale * SEDUCE_T_SIZE * -2.0, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE + (1 - time) * 3.0, options[*select], select_red, select_green, select_blue, time, -1);
		s_widget_draw_lines(pos_x, pos_y, length, scale * SEDUCE_T_SIZE * 3.0);
	}
}

boolean seduce_widget_button_invisible(BInputState *input, void *id, float pos_x, float pos_y, float scale, boolean down_click)
{
	uint i;
	if(input->mode == BAM_DRAW && scale > 0.00001)
	{
		float pos[3];
		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, scale);
	}
	if(input->mode == BAM_EVENT)
	{
		if(down_click)
		{
			for(i = 0; i < input->pointer_count; i++)
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
					if(id == seduce_element_pointer_id(input, i, NULL))
						return TRUE;
		}
		else
		{
			for(i = 0; i < input->pointer_count; i++)
				if(!input->pointers[i].button[0] && input->pointers[i].last_button[0])
					if(id == seduce_element_pointer_id(input, i, NULL) && id == seduce_element_pointer_down_click_id(input, i, NULL) &&
						(input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) * 
						(input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) + 
						(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) * 
						(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) < 0.01)
						return TRUE;
		}
		for(i = 0; i < input->user_count; i++)
			if(id == seduce_element_selected_id(i, NULL, NULL))
				if(betray_button_get(i, BETRAY_BUTTON_FACE_A))
					return TRUE;
	}
	return FALSE;
}

boolean seduce_widget_button_icon(BInputState *input, void *id, uint icon, float pos_x, float pos_y,  float scale, float time, float *color)
{
	uint i;
	if(input->mode == BAM_DRAW)
	{
		float on[3] = {0.2, 0.6, 1.0}, *p = NULL, pos[3];
		for(i = 0; i < input->pointer_count; i++)
			if(id == seduce_element_pointer_id(input, i, NULL))
				break;
		if(i < input->pointer_count)
		{
			p = on;
			if(color != NULL)
				p = color;
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(id == seduce_element_selected_id(i, NULL, NULL))
			{
				if(betray_button_get(i, BETRAY_BUTTON_FACE_A))
				{
					p = on;
					if(color != NULL)
						p = color;
				}
			}
		}
		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, scale);
		if(time < 0.999)
		{	
			RMatrix *matrix;
			matrix = r_matrix_get();
			r_matrix_push(matrix);
			r_matrix_translate(matrix, pos_x, pos_y, 0);
			r_matrix_rotate(matrix, 360.0 - time * 360.0, 0, 1, 0);
			r_matrix_scale(matrix, time, 2 - time, time);
			seduce_object_3d_draw(input, 0, 0, 0, scale, icon, FALSE, p);
			r_matrix_pop(matrix);
		}else
			seduce_object_3d_draw(input, pos_x, pos_y, 0, scale, icon, FALSE, p);

	}
	if(input->mode == BAM_EVENT)
	{
		return seduce_widget_button_invisible(input, id, pos_x, pos_y, 0, FALSE);
	}
	return FALSE;
}


boolean seduce_widget_toggle_icon(BInputState *input, void *id, boolean *value, uint icon, float pos_x, float pos_y, float scale, float time)
{
	uint i;
	if(input->mode == BAM_DRAW)
	{
		float on[3] = {0.2, 0.6, 1.0}, off[3] = {1.0, 0.1, 0.3}, *p, pos[3];
		for(i = 0; i < input->pointer_count; i++)
			if(id == seduce_element_pointer_id(input, i, NULL))
				break;

		if(value != NULL && *value)
			p = on;
		else
			p = off;
		for(i = 0; i < input->pointer_count; i++)
		{
			if(id == seduce_element_pointer_id(input, i, NULL))
			{
				p[0] *= 1.5;
				p[1] *= 1.5;
				p[2] *= 1.5;
				break;
			}
		}
		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, scale);

		if(time < 0.999)
		{	
			RMatrix *matrix;
			matrix = r_matrix_get();
			r_matrix_push(matrix);
			r_matrix_translate(matrix, pos_x, pos_y, 0);
			r_matrix_rotate(matrix, 360.0 - time * 360.0, 0, 1, 0);
			r_matrix_scale(matrix, time, 2 - time, time);
			seduce_object_3d_draw(input, 0, 0, 0, scale, icon, FALSE, p);
			r_matrix_pop(matrix);
		}else
			seduce_object_3d_draw(input, pos_x, pos_y, 0, scale, icon, FALSE, p);

	}
	if(input->mode == BAM_EVENT)
	{
		if(seduce_widget_button_invisible(input, id, pos_x, pos_y, 0, FALSE))
		{
			*value = !*value;
			return TRUE;
		}
	}
	return FALSE;
}


void s_widget_text(BInputState *input, float pos_x, float pos_y, float length, float scale, char *text, char *text_buffer, uint buffer_size, float time, float red, float green, float blue)
{
	static SSelectGrab *grab = NULL;
	uint i, out;
//	sui_text_line_edit(BInputState *input, float pos_x, float pos_y, float length, float size, char *text, uint buffer_size, void (*done_func)(void *user, char *text), void *user, float red, float green, float blue, float alpha)

//	sui_text_line_edit(input, pos_x + scale * SEDUCE_T_SIZE, pos_y + scale * SEDUCE_T_SIZE * -2.0, 1.0, scale * SEDUCE_T_SIZE, text_buffer, buffer_size, NULL, NULL, red, green, blue, 1.0);
	if(input->mode == BAM_DRAW)
	{
		seduce_text_line_draw(NULL, pos_x, pos_y + scale * SEDUCE_T_SIZE * -2.0, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE + (1 - time) * 3.0, text, red, green, blue, time, -1);
		s_widget_draw_lines(pos_x, pos_y, length, scale * SEDUCE_T_SIZE * 3.0);
	}
}

#define SUI_COLOR_SECLECT_SPLITS 32

void *sui_color_selector_pool;
void *sui_color_selector_section;
float *sui_color_selector_array;
void *sui_color_selector_program;

char *sui_color_selector_shader_vertex = 
"attribute vec3 vertex;"
"attribute vec3 color;"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec3 c;"
"void main()"
"{"
"	c = color;"

"	gl_Position = ModelViewProjectionMatrix * vec4(vertex, 1.0);"
"}";

char *sui_color_selector_shader_fragment = 
"varying vec3 c;"
"void main()"
"{"
"	gl_FragColor = vec4(c, 1);"
"}";

void s_widget_color_set(float *array, float x, float y, float length, float width, float red_aa, float green_aa, float blue_aa, float red_ba, float green_ba, float blue_ba
						, float red_ab, float green_ab, float blue_ab, float red_bb, float green_bb, float blue_bb)
{
	array[0] = x;
	array[1] = y;
	array[2] = 0.0;
	array[3] = red_aa;
	array[4] = green_aa;
	array[5] = blue_aa;

	array[6] = x + length;
	array[7] = y + width;
	array[8] = 0.0;
	array[9] = red_bb;
	array[10] = green_bb;
	array[11] = blue_bb;

	array[12] = x + length;
	array[13] = y;
	array[14] = 0.0;
	array[15] = red_ba;
	array[16] = green_ba;
	array[17] = blue_ba;

	array[18] = x;
	array[19] = y;
	array[20] = 0.0;
	array[21] = red_aa;
	array[22] = green_aa;
	array[23] = blue_aa;

	array[24] = x + length;
	array[25] = y + width;
	array[26] = 0.0;
	array[27] = red_bb;
	array[28] = green_bb;
	array[29] = blue_bb;

	array[30] = x;
	array[31] = y + width;
	array[32] = 0.0;
	array[33] = red_ab;
	array[34] = green_ab;
	array[35] = blue_ab;
}

void s_widget_color_init()
{
	RFormats vertex_format_types[3] = {R_FLOAT, R_FLOAT};
	uint i, vertex_format_size[2] = {3, 3};
	float x = 0.35, y = 0.0, rgb[3] = {0.2, 0.6, 1.0};
	sui_color_selector_program = r_shader_create_simple(NULL, 0, sui_color_selector_shader_vertex, sui_color_selector_shader_fragment, "color selector");
//	r_shader_attrib_bind(sui_color_selector_program, 0, "vertex");
//	r_shader_attrib_bind(sui_color_selector_program, 1, "color");
	
	sui_color_selector_pool = r_array_allocate(SUI_COLOR_SECLECT_SPLITS * 3 + 6 * 7, vertex_format_types, vertex_format_size, 2, 0);
	sui_color_selector_section = r_array_section_allocate_vertex(sui_color_selector_pool, SUI_COLOR_SECLECT_SPLITS * 3 + 6 * 7);

	sui_color_selector_array = malloc((sizeof *sui_color_selector_array) * (SUI_COLOR_SECLECT_SPLITS * 3 * 6 + 42 * 6));
	for(i = 0; i < SUI_COLOR_SECLECT_SPLITS; i++)
	{
		sui_color_selector_array[i * 6 * 3 + 0] = x;
		sui_color_selector_array[i * 6 * 3 + 1] = y;
		sui_color_selector_array[i * 6 * 3 + 2] = 0;
		f_hsv_to_rgb(&sui_color_selector_array[i * 6 * 3 + 3], (float)i / (float)SUI_COLOR_SECLECT_SPLITS, 0, 1);

		sui_color_selector_array[i * 6 * 3 + 6] = 0.3;
		sui_color_selector_array[i * 6 * 3 + 7] = -0.3;
		sui_color_selector_array[i * 6 * 3 + 8] = -1.0;

		sui_color_selector_array[i * 6 * 3 + 9] = 1.0;
		sui_color_selector_array[i * 6 * 3 + 10] = 1.0;
		sui_color_selector_array[i * 6 * 3 + 11] = 1.0;

		sui_color_selector_array[i * 6 * 3 + 12] = x = sin((i + 1) * PI * 2.0 / (float)SUI_COLOR_SECLECT_SPLITS) * 0.7 * 0.5 + 0.7 * 0.5;
		sui_color_selector_array[i * 6 * 3 + 13] = y = cos((i + 1) * PI * 2.0 / (float)SUI_COLOR_SECLECT_SPLITS) * 0.7 * 0.5 - 0.7 * 0.5;
		sui_color_selector_array[i * 6 * 3 + 14] = 0.0;
		f_hsv_to_rgb(&sui_color_selector_array[i * 6 * 3 + 15], (float)(i + 1) / (float)SUI_COLOR_SECLECT_SPLITS, 0, 1);
	}
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 0], 0.0, -0.25, 0.25, 0.25, 
		rgb[0], 0, 0, rgb[0], 0, 1, rgb[0], 1, 0, rgb[0], 1, 1);
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 1], 0.375, -0.25, 0.25, 0.25, 
		0, rgb[1], 0, 0, rgb[1], 1, 1, rgb[1], 0, 1, rgb[1], 1);
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 2], 0.75, -0.25, 0.25, 0.25, 
		0, 0, rgb[2], 0, 1, rgb[2], 1, 0, rgb[2], 1, 1, rgb[2]);

	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 3], 0.0, -0.0, 1.0, 0.05, 
		0, rgb[1], rgb[2], 1, rgb[1], rgb[2], 0, rgb[1], rgb[2], 1, rgb[1], rgb[2]);

	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 4], 0.0, -0.1, 1.0, 0.05, 
		rgb[0], 0, rgb[2], rgb[0], 1, rgb[2], rgb[0], 0, rgb[2], rgb[0], 1, rgb[2]);

	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 5], 0.0, -0.2, 1.0, 0.05, 
		rgb[0], rgb[1], 0, rgb[0], rgb[1], 1, rgb[0], rgb[1], 0, rgb[0], rgb[1], 1);

/*	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 5 * 3 + 30], 1, 0.2, 2, 0.1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0);
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 5 * 3 + 60], 1, 0.4, 2, 0.1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0);
/*
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 5 * 3 + 0], 1, 0.0, 2, 0.1, 0.2, 0.6, 1, 1, 0.6, 1);
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 5 * 3 + 30], 1, 0.2, 2, 0.1, 0.2, 0, 1, 0.2, 1, 1);
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 5 * 3 + 60], 1, 0.4, 2, 0.1, 0.2, 0.6, 0, 0.2, 0.6, 1);
*/	r_array_load_vertex(sui_color_selector_pool, sui_color_selector_section, sui_color_selector_array, 0, SUI_COLOR_SECLECT_SPLITS * 3 + 42);
}


void s_widget_outline(BInputState *input, float pos_x, float pos_y, float scale, float time, float *color)
{
	if(input->mode == BAM_DRAW)
	{
		scale *= 0.5;
		r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + -scale, pos_y + scale, 0, 1, 1, 1, 1.0, 1, 1, 1, 1.0);
		r_primitive_line_fade_3d(pos_x + scale, pos_y + -scale, 0, pos_x + -scale, pos_y + -scale, 0, 1, 1, 1, 1.0, 1, 1, 1, 1.0);

		r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + scale, pos_y + -scale, 0, 1, 1, 1, 1.0, 1, 1, 1, 1.0);
		r_primitive_line_fade_3d(pos_x + -scale, pos_y + scale, 0, pos_x + -scale, pos_y + -scale, 0, 1, 1, 1, 1.0, 1, 1, 1, 1.0);
		r_primitive_line_flush();
	}
}

void s_widget_color_area(BInputState *input, float pos_x, float pos_y, float scale, float time, uint axis, float *color)
{
	uint i;
	if(input->mode == BAM_DRAW)
	{
		scale *= 0.5;
		if(axis == 0)
		{
			r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + -scale, pos_y + scale, 0, 
				color[0], 1, 1, 1.0, color[0], 0, 1, 1.0);
			r_primitive_line_fade_3d(pos_x + scale, pos_y + -scale, 0, pos_x + -scale, pos_y + -scale, 0,  
				color[0], 1, 0, 1.0, color[0], 0, 0, 1.0);
			r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + scale, pos_y + -scale, 0,  
				color[0], 1, 1, 1.0, color[0], 1, 0, 1.0);
			r_primitive_line_fade_3d(pos_x + -scale, pos_y + scale, 0, pos_x + -scale, pos_y + -scale, 0,  
				color[0], 0, 1, 1.0, color[0], 0, 0, 1.0);
		}
		if(axis == 1)
		{
			r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + -scale, pos_y + scale, 0, 
				1, color[1], 1, 1.0, 0, color[1], 1, 1.0);
			r_primitive_line_fade_3d(pos_x + scale, pos_y + -scale, 0, pos_x + -scale, pos_y + -scale, 0,  
				1, color[1], 0, 1.0, 0, color[1], 0, 1.0);
			r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + scale, pos_y + -scale, 0,  
				1, color[1], 1, 1.0, 1, color[1], 0, 1.0);
			r_primitive_line_fade_3d(pos_x + -scale, pos_y + scale, 0, pos_x + -scale, pos_y + -scale, 0,  
				0, color[1], 1, 1.0, 0, color[1], 0, 1.0);
		}
		if(axis == 2)
		{
			r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + -scale, pos_y + scale, 0, 
				1, 1, color[2], 1.0, 0, 1, color[2], 1.0);
			r_primitive_line_fade_3d(pos_x + scale, pos_y + -scale, 0, pos_x + -scale, pos_y + -scale, 0,  
				1, 0, color[2], 1.0, 0, 0, color[2], 1.0);
			r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + scale, pos_y + -scale, 0,  
				1, 1, color[2], 1.0, 1, 0, color[2], 1.0);
			r_primitive_line_fade_3d(pos_x + -scale, pos_y + scale, 0, pos_x + -scale, pos_y + -scale, 0,  
				0, 1, color[2], 1.0, 0, 0, color[2], 1.0);
		}
		r_primitive_line_flush();
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] &&
				input->pointers[i].click_pointer_x[0] > pos_x + -scale &&
				input->pointers[i].click_pointer_x[0] < pos_x + scale &&
				input->pointers[i].click_pointer_y[0] > pos_y + -scale &&
				input->pointers[i].click_pointer_y[0] < pos_y + scale)
			{
				if(!input->pointers[i].last_button[0])
				{
					color[(axis + 1) % 3] = (input->pointers[i].pointer_x - (pos_x - scale * 0.5)) / scale;
					color[(axis + 2) % 3] = (input->pointers[i].pointer_y - (pos_y - scale * 0.5)) / scale;
				}else
				{
					color[(axis + 1) % 3] += input->pointers[i].delta_pointer_x * 2.0;
					color[(axis + 2) % 3] += input->pointers[i].delta_pointer_y * 2.0;
				}		
				if(color[(axis + 1) % 3] > 1.0)
					color[(axis + 1) % 3] = 1.0;
				if(color[(axis + 1) % 3] < 0.0)
					color[(axis + 1) % 3] = 0.0;
				if(color[(axis + 2) % 3] > 1.0)
					color[(axis + 2) % 3] = 1.0;
				if(color[(axis + 2) % 3] < 0.0)
					color[(axis + 2) % 3] = 0.0;
			}
		}
	}
}

void s_widget_color_saturation_value(BInputState *input, float pos_x, float pos_y, float scale, float time, uint axis, float *color)
{
	float rgb[3], hsv[3];
	uint i;
	if(input->mode == BAM_DRAW)
	{
		scale *= 0.5;
		f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
		f_hsv_to_rgb(rgb, hsv[0], 0, 1);
		r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + -scale, pos_y + scale, 0, 
			rgb[0], rgb[1], rgb[2], 1.0, 0.0, 0.0, 0.0, 1.0);
		r_primitive_line_fade_3d(pos_x + scale, pos_y + -scale, 0, pos_x + -scale, pos_y + -scale, 0,  
			1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0);
		r_primitive_line_fade_3d(pos_x + scale, pos_y + scale, 0, pos_x + scale, pos_y + -scale, 0,  
			rgb[0], rgb[1], rgb[2], 1.0, 1.0, 1.0, 1.0, 1.0);
		r_primitive_line_fade_3d(pos_x - scale, pos_y + scale, 0, pos_x - scale, pos_y + -scale, 0,  
			0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
		r_primitive_line_flush();
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] &&
				input->pointers[i].click_pointer_x[0] > pos_x + -scale &&
				input->pointers[i].click_pointer_x[0] < pos_x + scale &&
				input->pointers[i].click_pointer_y[0] > pos_y + -scale &&
				input->pointers[i].click_pointer_y[0] < pos_y + scale)
			{
				f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
				if(!input->pointers[i].last_button[0])
				{
					hsv[2] = 1.0 - (input->pointers[i].pointer_x - (pos_x - scale * 0.5)) / scale;
					hsv[1] = 1.0 - (input->pointers[i].pointer_y - (pos_y - scale * 0.5)) / scale;
				}else
				{
					hsv[2] -= input->pointers[i].delta_pointer_x * 2.0;
					hsv[1 ] -= input->pointers[i].delta_pointer_y * 2.0;
				}		
				if(hsv[1] > 1.0)
					hsv[1] = 1.0;
				if(hsv[1] < 0.0)
					hsv[1] = 0.0;
				if(hsv[2] > 1.0)
					hsv[2] = 1.0;
				if(hsv[2] < 0.0)
					hsv[2] = 0.0;
				f_hsv_to_rgb(color, hsv[0], hsv[1], hsv[2]);
			}
		}
	}
}


void s_widget_areas(BInputState *input, float pos_x, float pos_y, float length, float scale, float time, float *color)
{
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 0], pos_x + 0.0 * length, pos_x - 0.25 * length, 0.25 * length, 0.25 * length, 
		color[0], 0, 0, color[0], 0, 1, color[0], 1, 0, color[0], 1, 1);
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 1], pos_x + 0.375 * length, pos_x - 0.25 * length, 0.25 * length, 0.25 * length, 
		0, color[1], 0, 0, color[1], 1, 1, color[1], 0, 1, color[1], 1);
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 2], pos_x + 0.75 * length, pos_x - 0.25 * length, 0.25 * length, 0.25 * length, 
		0, 0, color[2], 0, 1, color[2], 1, 0, color[2], 1, 1, color[2]);
}

void s_widget_rgb(BInputState *input, float pos_x, float pos_y, float length, float scale, float time, float *color)
{
	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 3], pos_x, pos_y - 0.0, length, 0.025, 
		0, color[1], color[2], 1, color[1], color[2], 0, color[1], color[2], 1, color[1], color[2]);

	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 4], pos_x, pos_y - 0.05, length, 0.025, 
		color[0], 0, color[2], color[0], 1, color[2], color[0], 0, color[2], color[0], 1, color[2]);

	s_widget_color_set(&sui_color_selector_array[SUI_COLOR_SECLECT_SPLITS * 6 * 3 + 36 * 5], pos_x, pos_y - 0.1, length, 0.025, 
		color[0], color[1], 0, color[0], color[1], 1, color[0], color[1], 0, color[0], color[1], 1);
}
