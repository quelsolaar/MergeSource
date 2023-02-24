#include <math.h>
#include <stdlib.h>

#include "seduce.h"
#include "s_draw_3d.h"

extern void seduce_background_circle_draw(BInputState *input, float pos_x, float pos_y, uint splits, float timer, uint selected);
extern void seduce_background_angle(BInputState *input, void *id, uint part, float pos_x, float pos_y, float angle_a, float angle_b, float timer);

extern void seduce_widget_overlay_matrix(RMatrix *matrix);
extern void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float scale, uint id, float fade,  float *color);



boolean seduce_angle_axis_detect(BInputState *input, uint user_id, float angle_a, float angle_b)
{
	float a[2], b[2];
	uint i;
	for(i = 0; i < input->axis_count; i++)
	{
		if(input->axis[i].axis[0] * input->axis[i].axis[0] + input->axis[i].axis[1] * input->axis[i].axis[1] > 0.01)
		{
			a[0] = sin(angle_a * PI * 2.0 / 360.0);
			a[1] = cos(angle_a * PI * 2.0 / 360.0);
			b[0] = sin(angle_b * PI * 2.0 / 360.0);
			b[1] = cos(angle_b * PI * 2.0 / 360.0);
			if(input->axis[i].axis[0] * a[1] - input->axis[i].axis[1] * a[0] > 0)
			{
				if(input->axis[i].axis[0] * b[1] - input->axis[i].axis[1] * b[0] < 0)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

uint seduce_popup_simple(BInputState *input, uint user_id, float pos_x, float pos_y, char **lables, uint element_count, float *time, boolean active, float red, float green, float blue, float red_active, float green_active, float blue_active)
{
	static float timer = 0;
	uint i, angle = -1, axis;
	float f, best = 100000, vec[2];

	if(input->mode == BAM_EVENT)
	{
		if(active)
		{
			if(*time < 0.2)
				*time = 0.2;
			*time += input->delta_time * 2.0;
		}else
			*time -= input->delta_time * 2.0;
		if(*time > 1)
			*time = 1;
		if(*time < 0)
			*time = 0;
	}
	if(*time < 0.0001)
		return -1;
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].user_id == user_id || user_id == -1)
		{
			vec[0] = input->pointers[i].pointer_x - pos_x;
			vec[1] = input->pointers[i].pointer_y - pos_y;
			f = vec[0] * vec[0] + vec[1] * vec[1];
			if(0.01 < f && f < best)
			{
				best = f;
				angle = (uint)((atan2(-vec[0], -vec[1]) + PI) * (float)element_count / (PI * 2.0) + 0.5) % element_count;
			}
		}
	}


	axis = seduce_element_primary_axis(input, user_id);
	if(axis != -1)
	{
		vec[0] = input->axis[axis].axis[0];
		vec[1] = input->axis[axis].axis[1];
		f = vec[0] * vec[0] + vec[1] * vec[1];
		if(0.01 < f && f < best)
		{
			best = f;
			angle = (uint)((atan2(-vec[0], -vec[1]) + PI) * (float)element_count / (PI * 2.0) + 0.5) % element_count;
		}
	}

	if(input->mode == BAM_DRAW)
	{
		RMatrix matrix, *save;
		timer += input->delta_time * 90;
		vec[0] = input->pointers[0].pointer_x - pos_x;
		vec[1] = input->pointers[0].pointer_y - pos_y;
		if(0.01 < vec[0] * vec[0] + vec[1] * vec[1])
			angle = (uint)((atan2(-vec[0], -vec[1]) + PI) * (float)element_count / (PI * 2.0) + 0.5) % element_count;
		save = r_matrix_get();
		seduce_widget_overlay_matrix(&matrix);
		r_matrix_translate(&matrix, pos_x, pos_y, 0.0);
		seduce_background_circle_draw(input, pos_x, pos_y, element_count, *time, angle);
		r_matrix_push(&matrix);
		glDisable(GL_DEPTH_TEST);
		if(element_count < 9)
		{

			r_matrix_push(&matrix);
			if(angle == 0)
			{
				f = *time;
				if(active)
					f = 1.0;
				seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE * -0.75 * (3.0 - f * 2.0), SEDUCE_T_SPACE, lables[0], -1), 0.15 - SEDUCE_T_SIZE, SEDUCE_T_SIZE * 1.5 * (3.0 - f * 2.0), SEDUCE_T_SPACE, lables[0], red_active, green_active, blue_active, *time, -1);
			}else
			{
				f = (*time * (float)(element_count + 10));
				f *= 0.1;
				if(f > 1.0)
					f = 1.0;
				if(f < 0.0)
					f = 0.0;
				f = f + (1.0 - f) * f * 2.0;
				r_matrix_scale(&matrix, f, f, 1.0);
				seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE * -0.5, SEDUCE_T_SPACE, lables[0], -1), 0.15 - SEDUCE_T_SIZE, SEDUCE_T_SIZE, SEDUCE_T_SPACE, lables[0], red, green, blue, *time, -1);
			}
			r_matrix_pop(&matrix);
			i = 1;
		}else
			i = 0;
		for(; i < element_count / 2; i++)
		{
			r_matrix_push(&matrix);

			if(angle == i)
			{
				r_matrix_rotate(&matrix, 90.0 - 1 * 360 * (float)i / (float)element_count, 0, 0, 1);
				f = *time;
				if(active)
					f = 1.0;
				seduce_text_line_draw(NULL, 0.1, 0.0, SEDUCE_T_SIZE * 1.5 * (3.0 - f * 2.0), SEDUCE_T_SPACE, lables[i], red_active, green_active, blue_active, f, -1);
			}else
			{
				if(active)
					r_matrix_rotate(&matrix, 90.0 - 360 * (float)i / (float)element_count, 0, 0, 1);
				
				seduce_text_line_draw(NULL, 0.1, 0.0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, lables[i], red, green, blue, *time, -1);
			}
			r_matrix_pop(&matrix);
		}
		if(element_count < 9 && element_count % 2 == 0)
		{
			r_matrix_push(&matrix);
			if(angle == i)
			{
				f = *time;
				if(active)
					f = 1.0;
				seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE * -0.75 * (3.0 - f * 2.0), SEDUCE_T_SPACE, lables[i], -1), -0.15, SEDUCE_T_SIZE * 1.5 * (3.0 - f * 2.0), SEDUCE_T_SPACE, lables[i], red_active, green_active, blue_active, f, -1);
			}else
			{
				r_matrix_rotate(&matrix, 180 + 360 * (float)i / (float)element_count, 0, 0, 1);
				seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE * -0.5, SEDUCE_T_SPACE, lables[i], -1), -0.15, SEDUCE_T_SIZE, SEDUCE_T_SPACE, lables[i], red, green, blue, *time, -1);
			}
			r_matrix_pop(&matrix);
			i++;
		}
		for(; i < element_count; i++)
		{
			r_matrix_push(&matrix);

			if(angle == i)
			{
				r_matrix_rotate(&matrix, (-90.0 - 360 * (float)i / (float)element_count), 0, 0, 1);
				f = *time;
				if(active)
					f = 1.0;
				seduce_text_line_draw(NULL, -0.1 - seduce_text_line_length(NULL, SEDUCE_T_SIZE * 1.5 * (3.0 - f * 2.0), SEDUCE_T_SPACE, lables[i], -1), 0.0, SEDUCE_T_SIZE * 1.5 * (3.0 - f * 2.0), SEDUCE_T_SPACE, lables[i], red_active, green_active, blue_active, f, -1);

			}else
			{
				r_matrix_rotate(&matrix, -90 - 360 * (float)i / (float)element_count, 0, 0, 1);
				seduce_text_line_draw(NULL, -0.1 - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, lables[i], -1), 0.0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, lables[i], red, green, blue, *time, -1);

			}
			r_matrix_pop(&matrix);
		}
		
		glEnable(GL_DEPTH_TEST);
		r_matrix_pop(&matrix);
		r_matrix_set(save);
	}
	if(input->mode == BAM_EVENT)
		return angle;
	return -1;
}

/*
typedef struct{
	SPopUpType	type;
	char		*text;
	union{
		float button_pos[2];
		float angle[2];
		struct{
			float pos[2];
			float size[2];
			uint texture_id;
		}image;
		struct{
			float angle[2];
			float *value;
		}slider_angle;
	}data;
}SUIPUElement;
*/

extern boolean seduce_button_angle(BInputState *input, void *id, float pos_x, float pos_y, float angle_a, float angle_b, const char *text, float timer);

boolean seduce_popup_top_bottom_background_element(BInputState *input, void *id, uint part, uint element_nr, char *text, float time, boolean fit, boolean top)
{
	float vec = 0.70710678118654752440084436210485, neg = 1, angle_a, angle_b, normal, shade, expand, expand_next;
	uint i, j, active;
	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(id == seduce_element_pointer_id(input, i, &j) && j == part)
				break;
		}
		active = i < input->pointer_count;

	//	for(i = 0; i < input->user_count; i++)
	//		seduce_element_colission_test(pos, part, user_id);

		for(i = 0; i < input->user_count; i++)
			if(id == seduce_element_selected_id(i, NULL, &j) && j == part)
				if(betray_button_get(i, BETRAY_BUTTON_FACE_A))
					active = TRUE;
		if(top)
			neg = -1;
		
		normal = 0;
		time *= time;
		expand = 0.01 + 0.04 * (1.0 - time);
		angle_a = (float)((element_nr + 1) % 2) * -expand;
		angle_b = (float)(element_nr % 2) * -expand;
		shade = 1.2 - (float)(element_nr % 2) * 0.4;
		if(element_nr == 0)
			expand = (SEDUCE_T_SIZE * -5.0) * time;
		else
			expand = (SEDUCE_T_SIZE * -5.0) * (float)element_nr * time;
		expand_next = (SEDUCE_T_SIZE * -5.0) * (float)(element_nr + 1.0) * time;
		if(active)
		{
			if(element_nr == 0)
				seduce_background_quad_draw(input, id, part,
										0.1 * vec - vec * 0.005, (-0.1 * vec - vec * 0.005) * neg, 0, 
										-0.1 * vec - vec * -0.005, (-0.1 * vec - vec * 0.005) * neg, 0, 
										-0.1 * vec - vec * -0.005 - (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand) * neg, 0, 
										0.1 * vec - vec * 0.005 + (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand) * neg, 0, 
										0, normal, 1,
										0.2, 0.6, 1.0, 0.9);
			else
				seduce_background_quad_draw(input, id, part,
										-0.1 * vec - vec * -0.005 - (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand) * neg, angle_a, 
										0.1 * vec - vec * 0.005 + (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand) * neg, angle_a, 
										0.1 * vec - vec * 0.005 + (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand_next) * neg, angle_b, 
										-0.1 * vec - vec * -0.005 - (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand_next) * neg, angle_b, 
										0, normal, 1,
										0.2 * shade, 0.6 * shade, 1.0 * shade, 0.9);
		}else
		{
			if(element_nr == 0)
				seduce_background_quad_draw(input, id, part,
										0.1 * vec - vec * 0.005, (-0.1 * vec - vec * 0.005) * neg, 0, 
										-0.1 * vec - vec * -0.005, (-0.1 * vec - vec * 0.005) * neg, 0, 
										-0.1 * vec - vec * -0.005 - (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand) * neg, 0, 
										0.1 * vec - vec * 0.005 + (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand) * neg, 0, 
										0, normal, 1,
										0.5, 0.5, 0.5, 0.7);
			else
				seduce_background_quad_draw(input, id, part,
										-0.1 * vec - vec * -0.005 - (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand) * neg, angle_a, 
										0.1 * vec - vec * 0.005 + (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand) * neg, angle_a, 
										0.1 * vec - vec * 0.005 + (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand_next) * neg, angle_b, 
										-0.1 * vec - vec * -0.005 - (SEDUCE_T_SIZE * 5.0), (-0.1 * vec - vec * 0.005 + expand_next) * neg, angle_b, 
										0, normal, 1,
										0.5 * shade, 0.5 * shade, 0.5 * shade, 0.7);
		}
/*
		if(active)
			seduce_background_quad_draw(input, id, 0,
									0.08, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 2), 0, 
									0.08, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 3) + 0.01, 0, 
									-0.08, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 3) + 0.01, 0, 
									-0.08, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 2), 0,
									0, 0, 
									0.2, 0.6, 1.0, 0.9, 
									0.5, 0.5, 0.5, 0.7, 
									1.0, 0.2, 0.6, 0.9);
		else
			seduce_background_quad_draw(input, id, 0,
									0.08, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 2), 0, 
									0.08, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 3) + 0.01, 0, 
									-0.08, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 3) + 0.01, 0, 
									-0.08, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 2), 0,
									0, 0, 
									0.5, 0.5, 0.5, 0.7, 
									0.2, 0.6, 1.0, 0.9, 
									1.0, 0.2, 0.6, 0.9);
*/
	}
    return FALSE;
}

/*
	text_spacing = SEDUCE_T_SPACE + SEDUCE_T_SPACE * (1.0 - time) * 50.0;
	text_spacing = SEDUCE_T_SPACE;
	if(top)
	{
		if(seduce_text_button(input, id, 0, 0.06 + (0.012 + element_nr * 0.032) * time, 0.5, SEDUCE_T_SIZE, text_spacing, text, 0.0, 0.0, 0.0, time, 1, 1, 1, time))
			return TRUE;
	}else
		if(seduce_text_button(input, id, 0, (SEDUCE_T_SIZE * -5.0 - 0.01) * (element_nr + 2), 0.5, SEDUCE_T_SIZE, text_spacing, text, 0.0, 0.0, 0.0, time, 1, 1, 1, time))
			return TRUE;
	return FALSE;
*/

boolean seduce_popup_top_bottom_text_element(BInputState *input, void *id, uint element_nr, char *text, float time,  boolean top)
{
	float vec = 0.70710678118654752440084436210485;
	if(top)
	{
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, SEDUCE_T_SIZE * 0.5, SEDUCE_T_SPACE, text, -1), 0.1 * vec + vec * 0.005 + (SEDUCE_T_SIZE * 2.0) + (SEDUCE_T_SIZE * 5.0) * (float)element_nr * time * time, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, 0.5, 0.5, 0.5, time * time, -1);
	}else
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, SEDUCE_T_SIZE * 0.5, SEDUCE_T_SPACE, text, -1), -0.1 * vec - vec * 0.005 + (SEDUCE_T_SIZE * -3.0) + (SEDUCE_T_SIZE * -5.0) * (float)element_nr * time * time, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, 0.5, 0.5, 0.5, time * time, -1);
    return FALSE;
}

boolean seduce_text_angle(BInputState *input, void *id, float pos_x, float pos_y, float angle_a, float angle_b, char *text, float time)
{
	RMatrix *m = NULL;
	uint i;
	if(input->mode == BAM_DRAW)
	{
		m = r_matrix_get();
		r_matrix_push(m);
		if(angle_a > 359)
			angle_a -= 360;
		if(angle_a + angle_b * 0.5 < 180.0)
			r_matrix_rotate(m, angle_a + angle_b * 0.5 - 90.0, 0, 0, -1.0);
		else
			r_matrix_rotate(m, angle_a + angle_b * 0.5 + 90.0, 0, 0, -1.0);
	}
	if(angle_a + angle_b * 0.5 < 180.0)
	{
		seduce_text_line_draw(NULL, 0.12, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, 0.0, 0.0, 0.0, time * time, -1);
	}else
	{
		
		seduce_text_line_draw(NULL, -0.12 - seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, -1), 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, text, 0.5, 0.5, 0.5, time * time, -1);
	}
		
	if(input->mode == BAM_DRAW)
		r_matrix_pop(m);
	return FALSE;
}


uint seduce_popup_old(BInputState *input, void *id, SUIPUElement *elements, uint element_count, float time)
{
	uint i, j, top = 0, bottom = 0, output = SEDUCE_POP_UP_NO_ACTION;
	boolean angle = FALSE;
	RMatrix *matrix;
	float text_spacing, f, center[3], square[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	for(i = 0; i < element_count && elements[i].type != S_PUT_ANGLE; i++);
		angle = i < element_count;
	if(input->mode == BAM_DRAW)
	{
		r_matrix_projection_screenf(NULL, center, 0, 0, 0);
		for(i = 0; i < input->pointer_count; i++)
		{
			for(j = 0; j < input->pointers[i].button_count && !input->pointers[i].button[j]; j++);
			if(j < input->pointers[i].button_count)
				seduce_background_particle_spawn(input, 0, 0, input->pointers[i].pointer_x - center[0], input->pointers[i].pointer_y - center[1], 0.1, S_PT_SPLAT_ONE);
		}
		for(i = 0; i < input->user_count; i++)
		{
			j = seduce_element_primary_axis(input, i);
			if(j < input->axis_count)
				seduce_background_particle_spawn(input, 0, 0, input->axis[j].axis[0] * 0.2, input->axis[j].axis[1] * 0.2, 0.1, S_PT_SPLAT_ONE);
		}

		

		for(i = 0; i < element_count; i++)
		{
			if(elements[i].type == S_PUT_TOP)
				seduce_popup_top_bottom_background_element(input, id, i, top++, elements[i].text, time, angle, TRUE);
			if(elements[i].type == S_PUT_BOTTOM)
				seduce_popup_top_bottom_background_element(input, id, i, bottom++, elements[i].text, time, angle, FALSE);
			if(elements[i].type == S_PUT_ANGLE)
				seduce_background_angle(input, id, i, 0, 0, elements[i].data.angle[0], elements[i].data.angle[1] - elements[i].data.angle[0], time);
			
		}
		seduce_background_polygon_flush(input, center, 0.1 + time * 0.9);
	//	seduce_background_polygon_flush(input, center, time);
	}
	/* draw up down background */
	text_spacing = SEDUCE_T_SPACE + SEDUCE_T_SPACE * (1.0 - time) * 50.0;
	top = 0;
	bottom = 0;
	if(input->mode == BAM_DRAW)
	{
		for(i = 0; i < element_count; i++)
		{
			switch(elements[i].type)
			{
				case S_PUT_TOP :
					seduce_popup_top_bottom_text_element(input, &elements[i], top++, elements[i].text, time,  TRUE);
				break;
				case S_PUT_BOTTOM :
					seduce_popup_top_bottom_text_element(input, &elements[i], bottom++, elements[i].text, time,  FALSE);
				break;
				case S_PUT_ANGLE :
					seduce_text_angle(input, &elements[i], 0, 0, elements[i].data.angle[0], elements[i].data.angle[1] - elements[i].data.angle[0], elements[i].text, time);
				case S_PUT_BUTTON :
				break;
				case S_PUT_IMAGE :
					square[0] = elements[i].data.image.pos[0];
					square[1] = elements[i].data.image.pos[1];
					square[3] = elements[i].data.image.pos[0];
					square[4] = elements[i].data.image.pos[1] + elements[i].data.image.size[1];
					square[6] = elements[i].data.image.pos[0] + elements[i].data.image.size[0];
					square[7] = elements[i].data.image.pos[1] + elements[i].data.image.size[1];
					square[9] = elements[i].data.image.pos[0] + elements[i].data.image.size[0];
					square[10] = elements[i].data.image.pos[1];
					seduce_element_add_quad(input, &elements[i], 0, square, &square[3], &square[6], &square[9]);
					r_primitive_image(elements[i].data.image.pos[0], elements[i].data.image.pos[1], 0, elements[i].data.image.size[0], elements[i].data.image.size[1], 0, 0, 1, 1, elements[i].data.image.texture_id, 1, 1, 1, 1);
				break;
			}
		}
	}	
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			{
				output = SEDUCE_POP_UP_DEACTIVATE;
				if(id == seduce_element_pointer_id(input, i, &j))
					return j;
			}				
		}
		for(i = 0; i < input->button_event_count; i++)
		{
			if(input->button_event[i].state && (input->button_event[i].button == BETRAY_BUTTON_FACE_A || input->button_event[i].button == BETRAY_BUTTON_YES))
			{
				uint axis;
				axis = seduce_element_primary_axis(input, input->button_event[i].user_id);
				if(axis != -1)
				{
					r_matrix_projection_screenf(NULL, center, 0, 0, 0);
					center[0] += input->axis[axis].axis[0];
					center[1] += input->axis[axis].axis[1];
					center[2] = 0;
					if(id == seduce_element_colission_test(input->axis[axis].axis, &j, input->axis[axis].user_id))
						return j;
				}
			}				
		}
	}
	return output;
}



STypeInState seduce_popup_text(BInputState *input, void *id, uint *selected, char **lables, uint element_count, SPopUpType type, float pos_x, float pos_y, float center, float size, float spacing, const char *text, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select, boolean release_only)
{
	seduce_text_button(input, id, pos_x, pos_y, center, size, spacing, text, red, green, blue, alpha, red_select, green_select, blue_select, alpha_select);
	return seduce_widget_select_radial(input, id, selected, lables, element_count, type,  pos_x, pos_y, 0, 0.0, 1, release_only);
}

