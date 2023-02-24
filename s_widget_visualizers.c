#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "seduce.h"
#include "s_draw_3d.h"

void s_widget_pie(BInputState *input, float pos_x, float pos_y, float size, float scale, float time, uint slize_count, float *values, float *colors)
{
	uint i, j, count, prev, next;
	float a[2], b[2], sum = 0, f = 0;

	for(i = 0; i < slize_count; i++)
		sum += values[i];

	if(input->mode == BAM_DRAW)
	{
		b[0] = pos_x;
		b[1] = pos_y + size;
		scale *= 0.4 * time;
		for(i = 0; i < slize_count; i++)
		{
			count = (uint)(64.0 * values[i] / sum);
			if(count < 1)
				count = 1;
			j = (i + slize_count - 1) % slize_count;
			r_primitive_line_3d(b[0], b[1], values[j] / sum * scale, pos_x, pos_y, values[j] / sum * scale, colors[j * 4 + 0], colors[j * 4 + 1], colors[j * 4 + 2], colors[j * 4 + 3]);
			r_primitive_line_3d(b[0], b[1], values[i] / sum * scale, pos_x, pos_y, values[i] / sum * scale, colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);

			if(values[i] < values[j])
			{
				r_primitive_line_3d(b[0], b[1], values[i] / sum * scale, b[0], b[1], 0, colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);
				r_primitive_line_3d(b[0], b[1], values[i] / sum * scale, b[0], b[1], values[j] / sum * scale, colors[j * 4 + 0], colors[j * 4 + 1], colors[j * 4 + 2], colors[j * 4 + 3]);
			}else
			{
				r_primitive_line_3d(b[0], b[1], values[j] / sum * scale, b[0], b[1], values[i] / sum * scale, colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);
				r_primitive_line_3d(b[0], b[1], values[j] / sum * scale, b[0], b[1], 0, colors[j * 4 + 0], colors[j * 4 + 1], colors[j * 4 + 2], colors[j * 4 + 3]);
			}

			for(j = 0; j < count;)
			{
				j++;
				f += (PI * 2.0 * values[i] / sum) / (float)count;
				a[0] = pos_x + sin(f) * size;
				a[1] = pos_y + cos(f) * size;
				r_primitive_line_3d(a[0], a[1], 0.0, b[0], b[1], 0, colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);
				r_primitive_line_3d(a[0], a[1], values[i] / sum * scale, b[0], b[1], values[i] / sum  * scale, colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);
				b[0] = a[0];
				b[1] = a[1];
			}
		}

		prev = 0;
		for(i = 1; i < slize_count; i++)
			if(values[i] < values[prev])
				prev = i;
		while(TRUE)
		{
			next = -1;
			for(i = 0; i < slize_count; i++)
				if(values[i] > values[prev] && (next == -1 || values[i] < values[next]))
					next = i;
			r_primitive_line_3d(pos_x, pos_y, values[next] / sum * scale, pos_x, pos_y, values[prev] / sum  * scale, colors[next * 4 + 0], colors[next * 4 + 1], colors[next * 4 + 2], colors[next * 4 + 3]);
			if(next == -1)
				break;
			prev = next;
		}
		r_primitive_line_flush();
	}
}

void s_widget_chart(BInputState *input, float pos_x, float pos_y, float size_x, float size_y, float scale, float time, uint chart_count, uint chart_length, float **values, float min, float max, boolean auto_scale, float *colors, char **lables, boolean indicators, float select_red, float select_green, float select_blue)
{
	uint i, j, count, prev, next;
	float a[2], b[2], f = 0;

	if(auto_scale)
	{
		min = max = values[0][0];
		for(i = 0; i < chart_count; i++)
			for(j = 0; j < chart_length; j++)
				if(min > values[i][j])
					min = values[i][j];
		for(i = 0; i < chart_count; i++)
			for(j = 0; j < chart_length; j++)
				if(max < values[i][j])
					max = values[i][j];
	}
	if(input->mode == BAM_DRAW)
	{
	//	if(indicators)
		{
			float step, positive;
			step = 1;

			if(step * 20.0 > max - min)
			{
				while(step * 10.0 < max - min)
					step *= 10.0;
			}else
				while(step * 10.0 > max - min)
					step *= 0.1;
			for(f = (float)((int)(min / step) * step); f < max; f += step)
			{
				if(f < step * 0.5 && f > step * -0.5)
					r_primitive_line_3d(pos_x, pos_y + (f - min) / (max - min) * size_y, 0,
												pos_x + size_x, pos_y + (f - min) / (max - min) * size_y, 0, 
												select_red, select_green, select_blue, 0.5);
				else
				{	
					positive = f;
					if(positive < 0)
						positive = -positive;
					if((uint)(positive / step) % 5 == 0)
						r_primitive_line_3d(pos_x, pos_y + (f - min) / (max - min) * size_y, 0,
													pos_x - time * scale * 0.02, pos_y + (f - min) / (max - min) * size_y, 0, 
													select_red, select_green, select_blue, 0.5);
					else
						r_primitive_line_3d(pos_x, pos_y + (f - min) / (max - min) * size_y, 0,
													pos_x - time * scale * 0.01, pos_y + (f - min) / (max - min) * size_y, 0, 
													select_red, select_green, select_blue, 0.5);
				}
			}

		}
		max = size_y * time / (max - min);
		for(i = 0; i < chart_count; i++)
		{
			f = (float)i * 0.1 * scale / (float)chart_count;
			r_primitive_line_3d(pos_x, pos_y + (values[i][0] - min) * max, f,
				pos_x, pos_y, f,
				colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);

			for(j = 0; j < chart_length - 1; j++)
			{
				(values[i][j] - min) * max,
				r_primitive_line_3d(pos_x + size_x * (float)j / (float)(chart_length - 1), pos_y + (values[i][j] - min) * max, f,
											pos_x + size_x * (float)(j + 1) / (float)(chart_length - 1), pos_y +(values[i][j + 1] - min) * max, f,
											colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);
	
			}
			r_primitive_line_3d(pos_x + size_x, pos_y + (values[i][j] - min) * max, f,
				pos_x + size_x, pos_y, f,
				colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);
			r_primitive_line_3d(pos_x, pos_y, f,
				pos_x + size_x, pos_y, f,
				colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3]);
		}


		r_primitive_line_flush();
		if(lables != NULL)
			for(i = 0; i < chart_count; i++)
				seduce_text_line_draw(NULL, pos_x + size_x, pos_y + (values[i][chart_length - 1] - min) * max, SEDUCE_T_SIZE, SEDUCE_T_SPACE, lables[i], colors[i * 4 + 0], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3], -1);
			

	}
}
