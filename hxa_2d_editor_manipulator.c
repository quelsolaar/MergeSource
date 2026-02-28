#include <math.h>
#include <stdlib.h>
#include <stdio.h> 
#include "betray.h"
#include "imagine.h"
#include "seduce.h"
#include "hxa.h"
#include "hxa_2d_editor_internal.h"

#define MO_MENU_EDIT_MANIPULATOR_SCALE_BASE 0.028248

char *hxa_2d_editor_manipulator_names[] = {"Stretch", // HXA_2DEMM_STRETCH
											"Scale", // HXA_2DEMM_SCALE
											"Rotate", // HXA_2DEMM_ROTATE
											"Rotate and stretch", // HXA_2DEMM_ROTATE_AND_STRETCH
											"Rotate and scale", // HXA_2DEMM_ROTATE_AND_SCALE
											"Mirror", // HXA_2DEMM_MIRROR
											"Ruler", // HXA_2DEMM_RULER
											"Grid", // HXA_2DEMM_GRID
											"Circle", // HXA_2DEMM_CIRCLE
											"Disabled"}; // HXA_2DEMM_OFF



void hxa_2d_editor_manipulator(BInputState *input, HxA2DEditorShape *edit, HxA2DEditorInstance *level, double scale)
{
	static uint grab_pointer = -1;
	double tmpd[3], tmpd2[3], d, vector[2], length, angle, *array, matrix[4];
	float f, ff, tmp[3], tmp2[3], *c, color[3], black[3] = {0, 0, 0}, manip_startf[2], manip_endf[2], vectorf[2], lengthf;
	uint i, j, int_grid, size;
	boolean used;
	manip_startf[0] = level->manip_start[0];
	manip_startf[1] = level->manip_start[1];
	manip_endf[0] = level->manip_end[0];
	manip_endf[1] = level->manip_end[1];
	vector[0] = level->manip_end[0] - level->manip_start[0];
	vector[1] = level->manip_end[1] - level->manip_start[1];
	length = sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
	vector[0] /= length;
	vector[1] /= length;
	vectorf[0] = (float)vector[0];
	vectorf[1] = (float)vector[1];
	lengthf = (float)length;
	if(input->mode == BAM_EVENT)
	{
		if(!seduce_text_edit_active_is_user_id(~0))
		{
			if(betray_button_get(-1, BETRAY_BUTTON_1))
				level->manip_mode = HXA_2DEMM_STRETCH;
			if(betray_button_get(-1, BETRAY_BUTTON_2))
				level->manip_mode = HXA_2DEMM_SCALE;
			if(betray_button_get(-1, BETRAY_BUTTON_3))
				level->manip_mode = HXA_2DEMM_ROTATE;
			if(betray_button_get(-1, BETRAY_BUTTON_4))
				level->manip_mode = HXA_2DEMM_ROTATE_AND_STRETCH;
			if(betray_button_get(-1, BETRAY_BUTTON_5))
				level->manip_mode = HXA_2DEMM_ROTATE_AND_SCALE;
			if(betray_button_get(-1, BETRAY_BUTTON_6))
				level->manip_mode = HXA_2DEMM_MIRROR;
			if(betray_button_get(-1, BETRAY_BUTTON_7))
				level->manip_mode = HXA_2DEMM_RULER;
			if(betray_button_get(-1, BETRAY_BUTTON_8))
				level->manip_mode = HXA_2DEMM_GRID;
		}
	}

	if(level->manip_mode >= HXA_2DEMM_OFF)
		return;

	if(input->mode == BAM_DRAW)
	{
		float quad[3 * 4];
		i = 0;
		f_hsv_to_rgb(color, (float)level->manip_mode / (float)HXA_2DEMM_COUNT, 1, 1);

		tmp[0] = manip_startf[0] - manip_endf[0];
		tmp[1] = manip_startf[1] - manip_endf[1];
		f = tmp[0] * tmp[0] + tmp[1] * tmp[1];
		if(f > scale * HXA_2D_EDITOR_MIN_EDGE_SIZE * scale * HXA_2D_EDITOR_MIN_EDGE_SIZE)
		{
			f = sqrtf(f) / (scale * 0.02 * 0.5);
			tmp[0] /= f;
			tmp[1] /= f;
			quad[0] = manip_startf[0] + tmp[0] - tmp[1];
			quad[1] = manip_startf[1] + tmp[1] + tmp[0];
			quad[3] = manip_startf[0] + tmp[0] + tmp[1];
			quad[4] = manip_startf[1] + tmp[1] - tmp[0];
			quad[6] = manip_endf[0] - tmp[0] + tmp[1];
			quad[7] = manip_endf[1] - tmp[1] - tmp[0];
			quad[9] = manip_endf[0] - tmp[0] - tmp[1];
			quad[10] = manip_endf[1] - tmp[1] + tmp[0];
			quad[2] = quad[5] = quad[8] = quad[11] = 0;
			seduce_element_add_quad(input, &level->manip_start[1], 0, quad, &quad[3], &quad[6], &quad[9]);
		}
		tmp[0] = manip_startf[0];
		tmp[1] = manip_startf[1];
		tmp[2] = 0;
		tmp2[0] = manip_endf[0];
		tmp2[1] = manip_endf[1];
		tmp2[2] = 0;
		seduce_element_add_point(input, level->manip_start, 0, tmp, scale * 0.02);
		seduce_element_add_point(input, level->manip_end, 0, tmp2, scale * 0.02);
	//	seduce_element_add_line(input, &level->manip_start[1], 0, tmp, tmp2, 0.04);


		for(i = 0; i < input->pointer_count; i++)
			if(&level->manip_start[1] == seduce_element_pointer_id(input, i, NULL))
				black[0] = black[1] = black[2] = 1;
		for(f = 0; f < length; f = ff)
		{
			ff = f + 0.04 * scale;
			if(i++ % 2 == 0)
				c = black;
			else
				c = color;
			if(ff < length)
				r_primitive_line_2d(manip_startf[0] + vectorf[0] * f, 
									manip_startf[1] + vectorf[1] * f, 
									manip_startf[0] + vectorf[0] * ff,
									manip_startf[1] + vectorf[1] * ff, c[0], c[1], c[2], 1);
			else
				r_primitive_line_2d(manip_startf[0] + vectorf[0] * f,
									manip_startf[1] + vectorf[1] * f, 
									manip_startf[0] + vectorf[0] * length,
									manip_startf[1] + vectorf[1] * length, c[0], c[1], c[2], 1);
		}


	if(input->mode == BAM_DRAW)
	{
		static float matrix[16] = {1, 0, 0, 0, 
								0, 1, 0, 0, 
								0, 0, 1, 0, 
								0, 0, 0, 1};
		tmp[0] = (float)(manip_endf[0] - manip_startf[0]);
		tmp[1] = (float)(manip_endf[1] - manip_startf[1]);
		f_normalize2f(tmp);
		if(tmp[0] > 0)
		{
			tmp[0] = -tmp[0];
			tmp[1] = -tmp[1];
		}

		matrix[0] = -tmp[0];
		matrix[1] = -tmp[1];
		matrix[4] = tmp[1];	
		matrix[5] = -tmp[0];
		matrix[12] = (manip_endf[0] + manip_startf[0]) * 0.5;
		matrix[13] = (manip_endf[1] + manip_startf[1]) * 0.5;
		r_matrix_push(NULL);
		r_matrix_matrix_mult(NULL, matrix);
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, scale * SEDUCE_T_SIZE * 0.5, SEDUCE_T_SPACE, hxa_2d_editor_manipulator_names[level->manip_mode], -1), scale * SEDUCE_T_SIZE, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, hxa_2d_editor_manipulator_names[level->manip_mode], color[0], color[1], color[2], 0.5, -1);
		r_matrix_pop(NULL);
	}



		for(i = 0; i < input->pointer_count; i++)
			if(manip_startf == seduce_element_pointer_id(input, i, NULL))
		if(i < input->pointer_count)
		{
			tmp2[0] = 0;
			tmp2[1] = 0.05;
			for(j = 1; j < 33; j++)
			{
				if(j % 2 == 0)
					c = black;
				else
					c = color;
				tmp[0] = sinf((float)j / 32.0 * 2 * PI) * 0.05;
				tmp[1] = cosf((float)j / 32.0 * 2 * PI) * 0.05;
				r_primitive_line_3d(manip_startf[0] + tmp[0],
									manip_startf[1] + tmp[1], 0.01, 
									manip_startf[0] + tmp2[0],
									manip_startf[1] + tmp2[1], 0.01, c[0], c[1], c[2], 1);
				tmp2[0] = tmp[0];
				tmp2[1] = tmp[1];
			}
		}
		for(i = 0; i < input->pointer_count; i++)
			if(manip_endf == seduce_element_pointer_id(input, i, NULL))
		if(i < input->pointer_count)
		{
			tmp2[0] = 0;
			tmp2[1] = 0.05;
			for(j = 1; j < 33; j++)
			{
				if(j % 2 == 0)
					c = black;
				else
					c = color;
				tmp[0] = sin((float)j / 32.0 * 2 * PI) * 0.05;
				tmp[1] = cos((float)j / 32.0 * 2 * PI) * 0.05;
				r_primitive_line_3d(manip_endf[0] + tmp[0],
									manip_endf[1] + tmp[1], 0.01, 
									manip_endf[0] + tmp2[0],
									manip_endf[1] + tmp2[1], 0.01, c[0], c[1], c[2], 1);
				tmp2[0] = tmp[0];
				tmp2[1] = tmp[1];
			}
		}
		
		c = color;
		switch(level->manip_mode)
		{
			case HXA_2DEMM_STRETCH :

				f = 1;
				for(i = 0; i < 11; i++)
				{
					ff = lengthf * (f - MO_MENU_EDIT_MANIPULATOR_SCALE_BASE) / (1.0 - MO_MENU_EDIT_MANIPULATOR_SCALE_BASE);
					
					r_primitive_line_2d(manip_startf[0] + vectorf[0] * ff + vectorf[1] * lengthf,
										manip_startf[1] + vectorf[1] * ff - vectorf[0] * lengthf, 
										manip_startf[0] + vectorf[0] * ff - vectorf[1] * lengthf,
										manip_startf[1] + vectorf[1] * ff + vectorf[0] * lengthf, c[0], c[1], c[2], 0.3);
					
					f *= 0.7;
				}
			break;
			case HXA_2DEMM_SCALE :
				f = 1;
				for(i = 0; i < 10; i++)
				{
					ff = lengthf * (f - MO_MENU_EDIT_MANIPULATOR_SCALE_BASE) / (1.0 - MO_MENU_EDIT_MANIPULATOR_SCALE_BASE);
					
					r_primitive_line_2d(manip_startf[0] + vectorf[0] * ff + vectorf[1] * ff,
										manip_startf[1] + vectorf[1] * ff - vectorf[0] * ff, 
										manip_startf[0] + vectorf[0] * ff - vectorf[1] * ff,
										manip_startf[1] + vectorf[1] * ff + vectorf[0] * ff, c[0], c[1], c[2], 0.3);
					r_primitive_line_2d(manip_startf[0] - vectorf[0] * ff + vectorf[1] * ff,
										manip_startf[1] - vectorf[1] * ff - vectorf[0] * ff, 
										manip_startf[0] - vectorf[0] * ff - vectorf[1] * ff,
										manip_startf[1] - vectorf[1] * ff + vectorf[0] * ff, c[0], c[1], c[2], 0.3);
					r_primitive_line_2d(manip_startf[0] - vectorf[0] * ff + vectorf[1] * ff,
										manip_startf[1] - vectorf[1] * ff - vectorf[0] * ff, 
										manip_startf[0] + vectorf[0] * ff + vectorf[1] * ff,
										manip_startf[1] + vectorf[1] * ff - vectorf[0] * ff, c[0], c[1], c[2], 0.3);
					r_primitive_line_2d(manip_startf[0] - vectorf[0] * ff - vectorf[1] * ff,
										manip_startf[1] - vectorf[1] * ff + vectorf[0] * ff, 
										manip_startf[0] + vectorf[0] * ff - vectorf[1] * ff,
										manip_startf[1] + vectorf[1] * ff + vectorf[0] * ff, c[0], c[1], c[2], 0.3);
					
					f *= 0.7;
				}
			break;
			case HXA_2DEMM_ROTATE :

				tmp2[0] = 0;
				tmp2[1] = lengthf;
				for(j = 1; j < 129; j++)
				{
					tmp[0] = sin((float)j / 128.0 * 2 * PI) * lengthf;
					tmp[1] = cos((float)j / 128.0 * 2 * PI) * lengthf;
					r_primitive_line_2d(manip_startf[0] + vectorf[0] * tmp[0] - vectorf[1] * tmp[1],
										manip_startf[1] + vectorf[1] * tmp[0] + vectorf[0] * tmp[1],
										manip_startf[0] + vectorf[0] * tmp2[0] - vectorf[1] * tmp2[1],
										manip_startf[1] + vectorf[1] * tmp2[0] + vectorf[0] * tmp2[1], c[0], c[1], c[2], 0.3);
					tmp2[0] = tmp[0];
					tmp2[1] = tmp[1];
				}

			break;

			break;
			case HXA_2DEMM_ROTATE_AND_STRETCH :
				f = 1.0;
				for(i = 0; i < 10; i++)
				{
					ff = lengthf * (f - MO_MENU_EDIT_MANIPULATOR_SCALE_BASE) / (1.0 - MO_MENU_EDIT_MANIPULATOR_SCALE_BASE);
					tmp2[0] = 0;
					tmp2[1] = lengthf;
					for(j = 1; j < 129; j++)
					{
						tmp[0] = sin((float)j / 128.0 * 2 * PI) * ff;
						tmp[1] = cos((float)j / 128.0 * 2 * PI) * lengthf;
						r_primitive_line_2d(manip_startf[0] + vectorf[0] * tmp[0] - vectorf[1] * tmp[1],
											manip_startf[1] + vectorf[1] * tmp[0] + vectorf[0] * tmp[1], 
											manip_startf[0] + vectorf[0] * tmp2[0] - vectorf[1] * tmp2[1],
											manip_startf[1] + vectorf[1] * tmp2[0] + vectorf[0] * tmp2[1], c[0], c[1], c[2], 0.3);
						tmp2[0] = tmp[0];
						tmp2[1] = tmp[1];
					}
					f *= 0.7;
				}
			break;
			case HXA_2DEMM_ROTATE_AND_SCALE :
				f = 1.0;
				for(i = 0; i < 10; i++)
				{
					ff = lengthf * (f - MO_MENU_EDIT_MANIPULATOR_SCALE_BASE) / (1.0 - MO_MENU_EDIT_MANIPULATOR_SCALE_BASE);
					tmp2[0] = 0;
					tmp2[1] = ff;
					for(j = 1; j < 129; j++)
					{
						tmp[0] = sin((float)j / 128.0 * 2 * PI) * ff;
						tmp[1] = cos((float)j / 128.0 * 2 * PI) * ff;
						r_primitive_line_2d(manip_startf[0] + vectorf[0] * tmp[0] - vectorf[1] * tmp[1],
											manip_startf[1] + vectorf[1] * tmp[0] + vectorf[0] * tmp[1], 
											manip_startf[0] + vectorf[0] * tmp2[0] - vectorf[1] * tmp2[1],
											manip_startf[1] + vectorf[1] * tmp2[0] + vectorf[0] * tmp2[1], c[0], c[1], c[2], 0.3);
						tmp2[0] = tmp[0];
						tmp2[1] = tmp[1];
					}
					f *= 0.7;
				}
			break;
			case HXA_2DEMM_MIRROR :
				if(level->loop_selected < level->loop_count)
				{
					tmp[0] = (level->loops[level->loop_selected].loop[0] - manip_startf[0]);
					tmp[1] = (level->loops[level->loop_selected].loop[1] - manip_startf[1]);
					f = tmp[0] * vectorf[1] - tmp[1] * vectorf[0];
					tmp[0] = level->loops[level->loop_selected].loop[0] + f * vectorf[1] * -2;
					tmp[1] = level->loops[level->loop_selected].loop[1] - f * vectorf[0] * -2;
					for(i = 0; i < level->loops[level->loop_selected].loop_size * 2;)
					{
						i += 2;
						j = i % (level->loops[level->loop_selected].loop_size * 2);
						tmp2[0] = (level->loops[level->loop_selected].loop[j] - manip_startf[0]);
						tmp2[1] = (level->loops[level->loop_selected].loop[j + 1] - manip_startf[1]);
						f = tmp2[0] * vectorf[1] - tmp2[1] * vectorf[0];
						tmp2[0] = level->loops[level->loop_selected].loop[j] + f * vectorf[1] * -2;
						tmp2[1] = level->loops[level->loop_selected].loop[j + 1] - f * vectorf[0] * -2;
						
						r_primitive_line_2d(tmp[0],
											tmp[1], 
											tmp2[0],
											tmp2[1], c[0], c[1], c[2], 0.3);

						tmp[0] = tmp2[0];
						tmp[1] = tmp2[1];
					}
				}
			break;
			case HXA_2DEMM_RULER :
				int_grid = level->manip_divisions;
				for(i = 0; i < int_grid * 3 + 1; i++)
				{
					ff = lengthf * (float)i / int_grid - lengthf;
					r_primitive_line_fade_2d(manip_startf[0] + vectorf[0] * ff + vectorf[1],
										manip_startf[1] + vectorf[1] * ff - vectorf[0], 
										manip_startf[0] + vectorf[0] * ff,
										manip_startf[1] + vectorf[1] * ff, 0, 0, 0, 0, c[0], c[1], c[2], 1);
					r_primitive_line_fade_2d(manip_startf[0] + vectorf[0] * ff - vectorf[1],
										manip_startf[1] + vectorf[1] * ff + vectorf[0], 
										manip_startf[0] + vectorf[0] * ff,
										manip_startf[1] + vectorf[1] * ff, 0, 0, 0, 0, c[0], c[1], c[2], 1);
			
				}
			break;
			case HXA_2DEMM_GRID :
				int_grid = level->manip_divisions;
				for(i = 0; i < int_grid * 3 + 1; i++)
				{
					ff = lengthf * (float)i / int_grid - lengthf;
					r_primitive_line_2d(manip_startf[0] + vectorf[0] * ff + vectorf[1] * lengthf,
										manip_startf[1] + vectorf[1] * ff - vectorf[0] * lengthf, 
										manip_startf[0] + vectorf[0] * ff,
										manip_startf[1] + vectorf[1] * ff, c[0], c[1], c[2], 0.3);
					r_primitive_line_2d(manip_startf[0] + vectorf[0] * ff - vectorf[1] * lengthf,
										manip_startf[1] + vectorf[1] * ff + vectorf[0] * lengthf, 
										manip_startf[0] + vectorf[0] * ff,
										manip_startf[1] + vectorf[1] * ff, c[0], c[1], c[2], 0.3);
			
				}
				for(i = 0; i < int_grid * 2 + 1; i++)
				{
					ff = lengthf * (float)i / int_grid - lengthf;
					r_primitive_line_2d(manip_startf[0] - vectorf[0] * lengthf - vectorf[1] * ff,
										manip_startf[1] - vectorf[1] * lengthf + vectorf[0] * ff, 
										manip_startf[0] + vectorf[0] * lengthf * 2 - vectorf[1] * ff,
										manip_startf[1] + vectorf[1] * lengthf * 2 + vectorf[0] * ff, c[0], c[1], c[2], 0.3);
				}
			break;
			case HXA_2DEMM_CIRCLE :
				int_grid = level->manip_divisions * 2;
				for(i = 0; i < int_grid; i++)
				{
					float xa, ya;
					ff = (float)i / (float)int_grid * 2.0 * PI;
					xa = sin(ff) * lengthf;
					ya = cos(ff) * lengthf;
					r_primitive_line_2d((manip_startf[0] + manip_endf[0]) * 0.5 - vectorf[1] * xa + vectorf[0] * ya,
										(manip_startf[1] + manip_endf[1]) * 0.5 + vectorf[0] * xa + vectorf[1] * ya, 
										(manip_startf[0] + manip_endf[0]) * 0.5,
										(manip_startf[1] + manip_endf[1]) * 0.5, c[0], c[1], c[2], 0.3);
				}

				if(int_grid < 128)
					j = 128 / int_grid;
				else
					j = 1;
				for(i = 0; i < int_grid * j; i++)
				{
					float xa, ya, xb, yb;
					ff = (float)i / (float)(int_grid * j) * 2.0 * PI;
					xa = sin(ff) * lengthf * 0.5;
					ya = cos(ff) * lengthf * 0.5;
					
					ff = (float)++i / (float)(int_grid * j) * 2.0 * PI;
					xb = sin(ff) * lengthf * 0.5;
					yb = cos(ff) * lengthf * 0.5;
					if((--i / j) % 2 == 0)
						r_primitive_line_2d((manip_startf[0] + manip_endf[0]) * 0.5 - vectorf[1] * xa + vectorf[0] * ya,
											(manip_startf[1] + manip_endf[1]) * 0.5 + vectorf[0] * xa + vectorf[1] * ya, 
											(manip_startf[0] + manip_endf[0]) * 0.5 - vectorf[1] * xb + vectorf[0] * yb,
											(manip_startf[1] + manip_endf[1]) * 0.5 + vectorf[0] * xb + vectorf[1] * yb, c[0], c[1], c[2], 0.3);
					else
						r_primitive_line_2d((manip_startf[0] + manip_endf[0]) * 0.5 - vectorf[1] * xa + vectorf[0] * ya,
											(manip_startf[1] + manip_endf[1]) * 0.5 + vectorf[0] * xa + vectorf[1] * ya, 
											(manip_startf[0] + manip_endf[0]) * 0.5 - vectorf[1] * xb + vectorf[0] * yb,
											(manip_startf[1] + manip_endf[1]) * 0.5 + vectorf[0] * xb + vectorf[1] * yb, c[0], c[1], c[2], 1.0);
				}
			break;
		}
	}
	if(input->mode == BAM_EVENT)
		if(grab_pointer == -1)
			for(i = 0; i < input->pointer_count; i++)
				if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
					if(&level->manip_start[1] == seduce_element_pointer_id(input, i, NULL))
						grab_pointer = i;



	if(grab_pointer != -1)
	{
		if(level->manip_mode == HXA_2DEMM_RULER || level->manip_mode == HXA_2DEMM_GRID || level->manip_mode == HXA_2DEMM_CIRCLE)
		{
			if(input->mode == BAM_EVENT)
			{
				level->manip_divisions += input->pointers[grab_pointer].delta_pointer_x / 0.05;
				if(level->manip_divisions < 1.5)
					level->manip_divisions = 1.5;
			}
		}
		if((input->mode == BAM_EVENT && (!input->pointers[grab_pointer].button[0] || level->manip_mode == HXA_2DEMM_MIRROR)) || input->mode == BAM_DRAW)
		{
			matrix[0] = vector[0];
			matrix[1] = vector[1];
			matrix[2] = -vector[1];
			matrix[3] = vector[0];
			switch(level->manip_mode)
			{
				case HXA_2DEMM_STRETCH :
					hxa_2d_editor_edit_guideline_snap(level, tmpd, input->pointers[grab_pointer].pointer_x, input->pointers[grab_pointer].pointer_y, scale, FALSE);
					tmpd[0] -= level->manip_start[0];
					tmpd[1] -= level->manip_start[1];
					d = (tmpd[0] * vector[0] + tmpd[1] * vector[1]) / length;
					matrix[0] = vector[0] * d;
					matrix[1] = vector[1];
					matrix[2] = vector[1] * d;
					matrix[3] = -vector[0];
				break;
				case HXA_2DEMM_SCALE :
					hxa_2d_editor_edit_guideline_snap(level, tmpd, input->pointers[grab_pointer].pointer_x, input->pointers[grab_pointer].pointer_y, scale, FALSE);
					tmpd[0] -= level->manip_start[0];
					tmpd[1] -= level->manip_start[1];
					d = (tmpd[0] * vector[0] + tmpd[1] * vector[1]) / length;
					matrix[0] = vector[0] * d;
					matrix[1] = vector[1] * d;
					matrix[2] = vector[1] * d;
					matrix[3] = -vector[0] * d;
				break;
				case HXA_2DEMM_ROTATE :
					hxa_2d_editor_edit_guideline_snap(level, tmpd, input->pointers[grab_pointer].pointer_x, input->pointers[grab_pointer].pointer_y, scale, FALSE);
					tmpd[0] -= level->manip_start[0];
					tmpd[1] -= level->manip_start[1];
					f_normalize2d(tmpd);
					matrix[0] = tmpd[0];
					matrix[1] = tmpd[1];
					matrix[2] = tmpd[1];
					matrix[3] = -tmpd[0];
				break;
				case HXA_2DEMM_ROTATE_AND_STRETCH :
					hxa_2d_editor_edit_guideline_snap(level, tmpd, input->pointers[grab_pointer].pointer_x, input->pointers[grab_pointer].pointer_y, scale, FALSE);
					tmpd[0] -= level->manip_start[0];
					tmpd[1] -= level->manip_start[1];
					matrix[0] = tmpd[0] / length;
					matrix[2] = tmpd[1] / length;
					f_normalize2d(tmpd);
					matrix[1] = tmpd[1];
					matrix[3] = -tmpd[0];
				break;
				case HXA_2DEMM_ROTATE_AND_SCALE :
					hxa_2d_editor_edit_guideline_snap(level, tmpd, input->pointers[grab_pointer].pointer_x, input->pointers[grab_pointer].pointer_y, scale, FALSE);
					tmpd[0] -= level->manip_start[0];
					tmpd[1] -= level->manip_start[1];
					matrix[0] = tmpd[0] / length;
					matrix[2] = tmpd[1] / length;
					matrix[1] = matrix[2];
					matrix[3] = -matrix[0];
				break;
				case HXA_2DEMM_MIRROR :
					if(level->loop_selected < level->loop_count)
					{
						for(j = 0; j < level->loops[level->loop_selected].loop_size * 2; j += 2)
						{
							tmp2[0] = (level->loops[level->loop_selected].loop[j] - level->manip_start[0]);
							tmp2[1] = (level->loops[level->loop_selected].loop[j + 1] - level->manip_start[1]);
							d = tmp2[0] * vector[1] - tmp2[1] * vector[0];
							level->loops[level->loop_selected].loop[j] += d * vector[1] * -2;
							level->loops[level->loop_selected].loop[j + 1] -= d * vector[0] * -2; 
						}

						for(j = 0; j < level->loops[level->loop_selected].loop_size / 2; j++)
						{
							d = level->loops[level->loop_selected].loop[j * 2];
							level->loops[level->loop_selected].loop[j * 2] = level->loops[level->loop_selected].loop[(level->loops[level->loop_selected].loop_size - j - 1) * 2];
							level->loops[level->loop_selected].loop[(level->loops[level->loop_selected].loop_size - j - 1) * 2] = d;
							d = level->loops[level->loop_selected].loop[j * 2 + 1];
							level->loops[level->loop_selected].loop[j * 2 + 1] = level->loops[level->loop_selected].loop[(level->loops[level->loop_selected].loop_size - j - 1) * 2 + 1];
							level->loops[level->loop_selected].loop[(level->loops[level->loop_selected].loop_size - j - 1) * 2 + 1] = d;
						}
						r_array_free(level->loops[level->loop_selected].pool);
						level->loops[level->loop_selected].pool = NULL;
						free(level->loops[level->loop_selected].triangle_array);
						level->loops[level->loop_selected].triangle_array = NULL;
					}
					grab_pointer = -1; 
				break;
				case HXA_2DEMM_RULER :
				case HXA_2DEMM_GRID :
				case HXA_2DEMM_CIRCLE :
					if(input->mode == BAM_EVENT)
					{
						level->manip_divisions += input->pointers[grab_pointer].delta_pointer_x / 0.05;
						if(level->manip_divisions < 1.5)
							level->manip_divisions = 1.5;
					}
				break;
			}

			array = NULL;
			if(level->manip_mode < HXA_2DEMM_MIRROR)
			{
		
				for(i = j = 0; i < level->loop_count; i++)
					if(j < level->loops[i].loop_size)
						j = level->loops[i].loop_size;
				array = malloc((sizeof *array) * j * 2);
				if(input->mode == BAM_EVENT)
				{
					level = hxa_2d_editor_structure_instance_add(edit);
					tmpd[0] = level->manip_end[0] - level->manip_start[0];
					tmpd[1] = level->manip_end[1] - level->manip_start[1];
					tmpd2[0] = tmpd[0] * vector[0] + tmpd[1] * vector[1];
					tmpd2[1] = tmpd[0] * vector[1] - tmpd[1] * vector[0];
					level->manip_end[0] = matrix[0] * tmpd2[0] + matrix[1] * tmpd2[1] + level->manip_start[0];
					level->manip_end[1] = matrix[2] * tmpd2[0] + matrix[3] * tmpd2[1] + level->manip_start[1];
					for(i = 0; i < level->entity_count; i++)
					{
						if(level->entity[i].selected)
						{
							tmpd[0] = level->entity[i].pos[0] - level->manip_start[0];
							tmpd[1] = level->entity[i].pos[1] - level->manip_start[1];
							tmpd2[0] = tmpd[0] * vector[0] + tmpd[1] * vector[1];
							tmpd2[1] = tmpd[0] * vector[1] - tmpd[1] * vector[0];
							level->entity[i].pos[0] = matrix[0] * tmpd2[0] + matrix[1] * tmpd2[1] + level->manip_start[0];
							level->entity[i].pos[1] = matrix[2] * tmpd2[0] + matrix[3] * tmpd2[1] + level->manip_start[1];
						}
					}

				}
				if(input->mode == BAM_DRAW)
				{
					for(i = 0; i < level->entity_count; i++)
					{
						if(level->entity[i].selected)
						{
							float pos[2];
							tmp[0] = (float)(level->entity[i].pos[0] - level->manip_start[0]);
							tmp[1] = (float)(level->entity[i].pos[1] - level->manip_start[1]);
							tmp2[0] = tmp[0] * vector[0] + tmp[1] * vector[1];
							tmp2[1] = tmp[0] * vector[1] - tmp[1] * vector[0];
							pos[0] = matrix[0] * tmp2[0] + matrix[1] * tmp2[1] + manip_startf[0];
							pos[1] = matrix[2] * tmp2[0] + matrix[3] * tmp2[1] + manip_startf[1];
							r_primitive_line_3d((float)level->entity[i].pos[0],
												(float)level->entity[i].pos[1], 0.02,
												pos[0],
												pos[1], 0.02, 0, 0, 0, 0.4);
							r_primitive_line_3d((float)level->entity[i].pos[0],
												(float)level->entity[i].pos[1], 0.02,
												(float)level->entity[i].pos[0],
												(float)level->entity[i].pos[1], 0.02, 0, 0, 0, 0.4);
							r_primitive_line_3d(pos[0],
												pos[1], 0.02,
												pos[0],
												pos[1], 0.0, 0, 0, 0, 0.4);
							tmp2[0] = 0;
							tmp2[1] = 0.01;
							for(j = 1; j < 17; j++)
							{
								tmp[0] = sinf((float)j / 16.0 * 2 * PI) * 0.01;
								tmp[1] = cosf((float)j / 16.0 * 2 * PI) * 0.01;
								r_primitive_line_2d(pos[0] + tmp[0],
													pos[1] + tmp[1], 
													pos[0] + tmp2[0],
													pos[1] + tmp2[1], 0, 0, 0, 1);
								tmp2[0] = tmp[0];
								tmp2[1] = tmp[1];
							}
						}
					}
				}

				for(i = 0; i < level->loop_count; i++)
				{
					used = FALSE;
					for(j = 0; j < level->loops[i].loop_size; j++)
					{
						if(level->loops[i].selection[j])
						{
							used = TRUE;
							tmpd[0] = level->loops[i].loop[j * 2] - level->manip_start[0];
							tmpd[1] = level->loops[i].loop[j * 2 + 1] - level->manip_start[1];
							tmpd2[0] = tmpd[0] * vector[0] + tmpd[1] * vector[1];
							tmpd2[1] = tmpd[0] * vector[1] - tmpd[1] * vector[0];
							array[j * 2 + 0] = matrix[0] * tmpd2[0] + matrix[1] * tmpd2[1] + level->manip_start[0];
							array[j * 2 + 1] = matrix[2] * tmpd2[0] + matrix[3] * tmpd2[1] + level->manip_start[1];
						}else
						{
							array[j * 2 + 0] = level->loops[i].loop[j * 2];
							array[j * 2 + 1] = level->loops[i].loop[j * 2 + 1];
						}
					}

					if(used)
					{
						size = level->loops[i].loop_size * 2 - 2;
						f = (array[0] - array[size]) * (array[1] + array[size + 1]);
						for(j = 0; j < size; j += 2)
							f += (array[j + 2] - array[j]) * (array[j + 2 + 1] + array[j + 1]);
						if(f < 0)
						{
							for(j = 0; j < level->loops[i].loop_size / 2; j++)
							{
								f = array[j * 2];
								array[j * 2] = array[(level->loops[i].loop_size - j - 1) * 2];
								array[(level->loops[i].loop_size - j - 1) * 2] = f;
								f = array[j * 2 + 1];
								array[j * 2 + 1] = array[(level->loops[i].loop_size - j - 1) * 2 + 1];
								array[(level->loops[i].loop_size - j - 1) * 2 + 1] = f;
							}
						}
						if(input->mode == BAM_DRAW)
						{
							if(!hxa_2d_editor_edit_loop_valid_test(array, level->loops[i].loop_size))
							{
								for(j = 0; j < level->loops[i].loop_size - 1; j++)
									r_primitive_line_2d((float)array[j * 2 + 0],
														(float)array[j * 2 + 1],
														(float)array[j * 2 + 2],
														(float)array[j * 2 + 3], 1, 0, 0, 1);
								r_primitive_line_2d((float)array[j * 2 + 0],
													(float)array[j * 2 + 1],
													(float)array[0],
													(float)array[1], 1, 0, 0, 1);
							}else
							{
								for(j = 0; j < level->loops[i].loop_size - 1; j++)
									r_primitive_line_2d((float)array[j * 2 + 0],
														(float)array[j * 2 + 1],
														(float)array[j * 2 + 2],
														(float)array[j * 2 + 3], 0, 0, 0, 1);
								r_primitive_line_2d((float)array[j * 2 + 0], 
													(float)array[j * 2 + 1],
													(float)array[0],
													(float)array[1], 0, 0, 0, 1);
							}
						}
						if(input->mode == BAM_EVENT && hxa_2d_editor_edit_loop_valid_test(array, level->loops[i].loop_size))
						{
							for(j = 0; j < level->loops[i].loop_size * 2; j++)
								level->loops[i].loop[j] = array[j];
							if(level->loops[i].pool != NULL)
								r_array_free(level->loops[i].pool);
							level->loops[i].pool = NULL;
							if(level->loops[i].triangle_array != NULL)
								free(level->loops[i].triangle_array);
							level->loops[i].triangle_array = NULL;
						}
					}
				}
			}
			if(array != NULL)
				free(array);
		}
		if(input->mode == BAM_EVENT && grab_pointer != ~0)
			if(input->pointers[grab_pointer].button[1] || !input->pointers[grab_pointer].button[0])
				grab_pointer = -1;
	}

}