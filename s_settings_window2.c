#include <math.h>
#include <stdlib.h>
#include "seduce.h"
#include "s_draw_3d.h"


#define SEDUCE_WIDGET_LIST_ELEMENT_SIZE 0.02
#define SEDUCE_WIDGET_LIST_SPACING 0.01
#define SEDUCE_WIDGET_LIST_MARGIN 0.1

float seduce_widget_list_element_size(SeducePanelElement *element, float time, float *left)
{
	switch(element->type)
	{
		case SEDUCE_PET_BOOLEAN :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_TRIGGER :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_INTEGER :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.75 + 0.1;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
		break;
		case SEDUCE_PET_UNSIGNED_INTEGER :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.75 + 0.1;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
		break;
		case SEDUCE_PET_INTEGER_BOUND :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.75 + 0.1;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
		break;
		case SEDUCE_PET_REAL :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.75 + 0.1;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
		break;
		case SEDUCE_PET_REAL_BOUND :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_RADIUS :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_2D_POS :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_3D_POS :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3;
		break;
		case SEDUCE_PET_4D_POS :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3;
		break;
		case SEDUCE_PET_QUATERNION :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_2D_NORMAL :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2;
		break;
		case SEDUCE_PET_3D_NORMAL :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_2X2MATRIX :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2;
		break;
		case SEDUCE_PET_3X3MATRIX :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_4X4MATRIX :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_TEXT :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.75 + 0.2;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
		break;
		case SEDUCE_PET_PASSWORD :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.75 + 0.2;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
		break;
		case SEDUCE_PET_TEXT_BUFFER :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_COLOR_RGB :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 4.5 + 0.05;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 4.5;
		break;
		case SEDUCE_PET_COLOR_RGBA :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 6 + 0.05;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 6;
		break;
		case SEDUCE_PET_TIME :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_DATE :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_SELECT :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_POPUP :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_IMAGE :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_SECTION_START :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_SECTION_END :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_CUSTOM :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE;
		break;
		case SEDUCE_PET_OK_CANCEL :
			*left = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
			return SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
		break;
	}
	return 0;
}


void seduce_widget_list_text_line(SeduceLineObject *object, float x, float y, float size, float length, float *color, float time)
{
	size *= 0.5;
	y -= size;
	seduce_primitive_circle_add_3d(object,
									x + length, y, 0,
									0, 1, 0,
									0, 0, 1,
									size,
									0, 0.5,
									size, size,
									color[0], color[1], color[2], color[3],
									color[0], color[1], color[2], color[3]);
	seduce_primitive_circle_add_3d(object,
									x, y, 0,
									0, 1, 0,
									0, 0, 1,
									size,
									0.5, 0.5,
									size, size,
									color[0], color[1], color[2], color[3],
									color[0], color[1], color[2], color[3]);
	seduce_primitive_line_add_3d(object,
							x + length, y + size, 0,
							x, y + size, 0,
							color[0], color[1], color[2], color[3],
							color[0], color[1], color[2], color[3]);
	seduce_primitive_line_add_3d(object,
							x + length, y - size, 0,
							x, y - size, 0,
							color[0], color[1], color[2], color[3],
							color[0], color[1], color[2], color[3]);

}

void seduce_widget_list_done_func(void *user, char *text)
{
	uint i;
	char *t;
	t = user;
	for(i = 0; text[i] != 0 && i < 64 - 1; i++)
		t[i] = text[i];
	t[i] = 0;
}

uint seduce_widget_list(BInputState *input, void *id, SeducePanelElement *elements, uint element_count, float time, char *title, float *color, float *background_color, SeduceWidgetListStyle style)
{
	SeduceLineObject *object;
	float y_text, y_widget, f, length, margin, text_size = 0, element_width, element_right, element_left, element_size, c[4] = {0.0, 0.0, 0.0, 1}, *text_color, space_time, vec[2];
	uint8 *id_fragmented;
	uint i;

	float size = SEDUCE_T_SIZE;

	if(color == NULL)
		color = c;
	text_color = color;
	length = 0;
	if(time < 0.0001)
		return -1;
	element_left = element_right = 0.0;
	for(i = 0; i < element_count; i++)
	{
		element_size = seduce_widget_list_element_size(&elements[i], time, &element_width);
		if(element_left < element_size)
			element_left = element_size;
		if(element_right < element_width)	
			element_right = element_width;
		length += element_size + SEDUCE_WIDGET_LIST_SPACING;
		f = seduce_text_line_length(NULL, SEDUCE_T_SIZE,  SEDUCE_T_SPACE, elements[i].text, -1) + element_size * 0.5 + SEDUCE_T_SIZE * 2;
		if(f > text_size)
			text_size = f;
	}
	element_left *= 0.5;
	length -= SEDUCE_WIDGET_LIST_SPACING;
	margin = SEDUCE_WIDGET_LIST_MARGIN;
	if(title != NULL)
	{
		if(background_color != NULL)
			length += margin * 0.5 + SEDUCE_T_SIZE * 1.5;
		else
			length += SEDUCE_WIDGET_LIST_SPACING + SEDUCE_T_SIZE * 1.5;
	}
	time = f_smooth_stepf(time);
	y_text = length / 2.0;
	y_widget = length / 2.0 * time;
	color[0] *= time;
	color[1] *= time;
	color[2] *= time;
	color[3] *= time;

	if(input->mode == BAM_DRAW)
	{	
		seduce_primitive_line_color_set(0, 0, 0, 0, 0, 0, 0, 0, 0);
		seduce_primitive_line_focal_depth_set(1.0);
		seduce_primitive_line_animation_set(0, 0, 0.01, time, 1.0);
		if(background_color != NULL)
		{
			SeduceBackgroundObject *object;
			object = seduce_background_object_allocate();
			r_matrix_push(NULL);
			if(style == SEDUCE_WLS_PANEL)
			{
				margin = SEDUCE_WIDGET_LIST_MARGIN;
				seduce_background_shadow_square_add(object,
													-text_size - margin, length * -0.5 - margin,
													text_size + element_right + 2.0 * margin, length + 2.0 * margin, 0.03);
				seduce_background_square_add(object, id, 0,
													-text_size - margin, length * -0.5 - margin, 0, 
													text_size + element_right + 2.0 * margin, length + 2.0 * margin,
													background_color[0], background_color[1], background_color[2], background_color[3]);
			}else
			{
				margin = SEDUCE_WIDGET_LIST_SPACING;
				seduce_background_shadow_square_add(object,
													-element_left - margin, length * -0.5 - margin,
													element_left + element_right + 2.0 * margin, length + 2.0 * margin, 0.03);
				seduce_background_square_add(object, id, 0,
													-element_left - margin, length * -0.5 - margin, 0, 
													element_left + element_right + 2.0 * margin, length + 2.0 * margin,
													background_color[0], background_color[1], background_color[2], background_color[3]);
			}
			seduce_primitive_surface_draw(input, object, time);
			seduce_primitive_background_object_free(object);
			r_matrix_pop(NULL);
		 }

/*		f = 0;
		for(i = 0; i < element_count; i++)
		{
			f_wiggle2df(vec, (float)i * 3.0 + input->minute_time * 6.0, 0.2);		
			seduce_primitive_circle_add_3d(object,
											0, vec[0] * length * 2.0, 0,
											0, 0, 1,
											0, 1, 0,
											vec[1] * 0.5 + 0.1,
											0, 1,
											vec[0], vec[1] * 10.0,
											color[0] * 0.2, color[1] * 0.2, color[2] * 0.2, color[3] * 0.2,
											color[0] * 0.2, color[1] * 0.2, color[2] * 0.2, color[3] * 0.2);
		}*/
	}

	if(input->mode == BAM_DRAW)
	{
		seduce_element_add_rectangle(input, id, 0, -SEDUCE_WIDGET_LIST_SPACING, length / -2.0, SEDUCE_WIDGET_LIST_SPACING * 2.0, length);
		object = seduce_primitive_line_object_allocate();
		if(title != NULL)
		{
			if(background_color != NULL)
				seduce_text_line_draw(NULL, ((element_right - text_size) - seduce_text_line_length(NULL, SEDUCE_T_SIZE * 1.5,  SEDUCE_T_SPACE * 6.0, title, -1)) * 0.5, y_text, SEDUCE_T_SIZE * 1.5, SEDUCE_T_SPACE * 6.0, title, color[0], color[1], color[2], color[3], -1);
			else
				seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE * 1.5,  SEDUCE_T_SPACE * 6.0, title, -1) * -0.5, y_text, SEDUCE_T_SIZE * 1.5, SEDUCE_T_SPACE * 6.0, title, color[0], color[1], color[2], color[3], -1);
		}
	/*
		f = seduce_text_line_length(NULL, 1.0,  SEDUCE_T_SPACE * 3.0, title, -1);
		r_matrix_push(NULL);
		r_matrix_rotate(NULL, 90, 0, 0, 1);
		r_matrix_translate(NULL, 0, 0, -0.05);
		seduce_text_line_draw(NULL, length * -0.5, 0.05, length / f,  SEDUCE_T_SPACE * 3.0, title, color[2] * 0.2, color[0] * 0.2, color[1] * 0.2, 0.0, -1);
	
		r_matrix_pop(NULL);*/
	}
	if(title != NULL)
	{
		if(background_color != NULL)
		{
			y_text -= margin * 0.5 + SEDUCE_T_SIZE * 1.5;
			y_widget -= margin * 0.5 + SEDUCE_T_SIZE * 1.5;
		}else
		{
			y_text -= SEDUCE_WIDGET_LIST_SPACING + SEDUCE_T_SIZE * 1.5;
			y_widget -= SEDUCE_WIDGET_LIST_SPACING + SEDUCE_T_SIZE * 1.5;
		}
	}	

	for(i = 0; i < element_count; i++)
	{
		if(input->mode == BAM_DRAW)	
			seduce_tool_tip(input, &elements[i], elements[i].text, elements[i].description);
		element_size = seduce_widget_list_element_size(&elements[i], time, &element_width);
		switch(elements[i].type)
		{
			case SEDUCE_PET_BOOLEAN :
				if(input->mode == BAM_DRAW)
				{
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
					if(elements[i].param.active)
						seduce_primitive_circle_add_3d(object,
												0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
												0, 1, 0,
												0, 0, 1,
												element_size * 0.3 + (1.0 - time) * f_randf(i + 1) * 0.4,
												0, 1,
												y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
												color[0], color[1], color[2], color[3],
												color[0], color[1], color[2], color[3]);
				}
				if(seduce_widget_button_invisible(input, &elements[i], 0, y_widget - element_size * 0.5, element_size, TRUE))
				{
					elements[i].param.active = !elements[i].param.active;
					return i;
				}
			break;
			case SEDUCE_PET_TRIGGER :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
				elements[i].param.trigger =	seduce_widget_button_invisible(input, &elements[i], 0, y_widget - element_size * 0.5, element_size, TRUE);
				if(elements[i].param.trigger)
					return i;
			break;
			case SEDUCE_PET_INTEGER :
				if(input->mode == BAM_DRAW)
					seduce_widget_list_text_line(object, 0, y_widget, element_size + (1.0 - time) * f_randf(i + 1) * 0.4, 0.1, color, time);
				if(S_TIS_DONE == seduce_text_edit_int(input, &elements[i], NULL, &elements[i].param.integer, 0, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
					return i;
			break;
			case SEDUCE_PET_UNSIGNED_INTEGER :
				if(input->mode == BAM_DRAW)
					seduce_widget_list_text_line(object, 0, y_widget, element_size + (1.0 - time) * f_randf(i + 1) * 0.4, 0.1, color, time);
				if(S_TIS_DONE == seduce_text_edit_uint(input, &elements[i], NULL, &elements[i].param.uinteger, 0, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
					return i;
			break;
			case SEDUCE_PET_INTEGER_BOUND :
				if(input->mode == BAM_DRAW)
					seduce_widget_list_text_line(object, 0, y_widget, element_size + (1.0 - time) * f_randf(i + 1) * 0.4, 0.1, color, time);
				if(S_TIS_DONE == seduce_text_edit_int(input, &elements[i], NULL, &elements[i].param.integer, 0, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
					return i;
			break;
			case SEDUCE_PET_REAL :
				if(input->mode == BAM_DRAW)
					seduce_widget_list_text_line(object, 0, y_widget, element_size + (1.0 - time) * f_randf(i + 1) * 0.4, 0.1, color, time);
				if(S_TIS_DONE == seduce_text_edit_double(input, &elements[i], NULL, &elements[i].param.real.value, 0, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
					return i;
			break;
			case SEDUCE_PET_REAL_BOUND :
				f =	elements[i].param.real.value;
				if(seduce_widget_slider_radial(input, &elements[i], &f, 0, y_text - element_size * 0.5, element_size, 1, elements[i].param.real.min, elements[i].param.real.max, time, color) && input->mode == BAM_EVENT)
				{
					elements[i].param.real.value = f;
					return i;
				}
			break;
			case SEDUCE_PET_RADIUS :
				f =	elements[i].param.real.value;
				if(seduce_widget_slider_radius(input, &elements[i], &f, 0, y_text - element_size * 0.5, element_size, time, color) && input->mode == BAM_EVENT)
				{
					elements[i].param.real.value = f;
					return i;
				}
			break;
			case SEDUCE_PET_2D_POS :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_3D_POS :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_4D_POS :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_QUATERNION :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_2D_NORMAL :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_3D_NORMAL :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_2X2MATRIX :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_3X3MATRIX :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_4X4MATRIX :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_TEXT :
				if(input->mode == BAM_DRAW)
					seduce_widget_list_text_line(object, 0, y_widget, element_size + (1.0 - time) * f_randf(i + 1) * 0.4, 0.2, color, time);
				if(S_TIS_DONE == seduce_text_edit_line(input, &elements[i], NULL, elements[i].param.text, 64, 0, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, 0.2, SEDUCE_T_SIZE, "", TRUE, seduce_widget_list_done_func, elements[i].param.text, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
					return i;
			break;
			case SEDUCE_PET_PASSWORD :
				if(input->mode == BAM_DRAW)
					seduce_widget_list_text_line(object, 0, y_widget, element_size + (1.0 - time) * f_randf(i + 1) * 0.4, 0.2, color, time);
				if(S_TIS_DONE == seduce_text_edit_obfuscated(input, &elements[i], elements[i].param.text, 64, 0, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, 0.2, SEDUCE_T_SIZE, "", TRUE, seduce_widget_list_done_func, elements[i].param.text, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
					return i;
			break;
			case SEDUCE_PET_TEXT_BUFFER :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_COLOR_RGB :
			case SEDUCE_PET_COLOR_RGBA :				
				{
					float x_rot, y_rot, f;
					double d, radius, widget_count;
					if(elements[i].type == SEDUCE_PET_COLOR_RGB)
					{
						widget_count = 5;
						radius = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.25;
					}else
					{
						widget_count = 6;
						radius = SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.75;
					}
					if(input->mode == BAM_DRAW)
						seduce_primitive_circle_add_3d(object,
												0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
												0, 1, 0,
												0, 0, 1,
												element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
												0, 1,
												y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
												color[0], color[1], color[2], color[3],
												color[0], color[1], color[2], color[3]);

					id_fragmented  = (uint8 *)&elements[i];
					x_rot = sin((float)0 / widget_count * PI * 2.0) * radius;
					y_rot = cos((float)0 / widget_count * PI * 2.0) * radius + y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7;
					if(seduce_widget_color_triangle_radial(input, &id_fragmented[0], elements[i].param.color, x_rot, y_rot, SEDUCE_WIDGET_LIST_ELEMENT_SIZE, 0.4, time))
						return i;
					x_rot = sin((float)1 / widget_count * PI * 2.0) * radius;
					y_rot = cos((float)1 / widget_count * PI * 2.0) * radius + y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7;
					if(seduce_widget_color_wheel_radial(input, &id_fragmented[1], elements[i].param.color, x_rot, y_rot, SEDUCE_WIDGET_LIST_ELEMENT_SIZE, 0.4, time))
						return i;
					x_rot = sin((float)2 / widget_count * PI * 2.0) * radius;
					y_rot = cos((float)2 / widget_count * PI * 2.0) * radius + y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7;
					if(seduce_widget_color_square_radial(input, &id_fragmented[2], elements[i].param.color, 0, x_rot, y_rot, SEDUCE_WIDGET_LIST_ELEMENT_SIZE, 0.4, time))
						return i;
					x_rot = sin((float)3 / widget_count * PI * 2.0) * radius;
					y_rot = cos((float)3 / widget_count * PI * 2.0) * radius + y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7;
					if(seduce_widget_color_square_radial(input, &id_fragmented[3], elements[i].param.color, 1, x_rot, y_rot, SEDUCE_WIDGET_LIST_ELEMENT_SIZE, 0.4, time))
						return i;
					x_rot = sin((float)4 / widget_count * PI * 2.0) * radius;
					y_rot = cos((float)4 / widget_count * PI * 2.0) * radius + y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7;
					if(seduce_widget_color_square_radial(input, &id_fragmented[4], elements[i].param.color, 2, x_rot, y_rot, SEDUCE_WIDGET_LIST_ELEMENT_SIZE, 0.4, time))
						return i;
					
					
					if(elements[i].type == SEDUCE_PET_COLOR_RGB)
					{

						d = elements[i].param.color[0];
						if(S_TIS_DONE == seduce_text_edit_double(input, &id_fragmented[5], NULL, &d, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2.75, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.75 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
						{
							elements[i].param.color[0] = d;
							return i;
						}
						d = elements[i].param.color[1];
						if(S_TIS_DONE == seduce_text_edit_double(input, &id_fragmented[6], NULL, &d, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.25, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
						{
							elements[i].param.color[1] = d;
							return i;
						}
						d = elements[i].param.color[2];
						if(S_TIS_DONE == seduce_text_edit_double(input, &id_fragmented[7], NULL, &d, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2.75, y_widget - element_size * 0.5 - SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.75 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
						{
							elements[i].param.color[2] = d;
							return i;
						}
						if(input->mode == BAM_DRAW)
						{
							seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.0, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2.5, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
							seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.5, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.75, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
							seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.0, y_widget - element_size * 0.5 - SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.0, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
						}
					}else
					{
						x_rot = sin((float)5 / widget_count * PI * 2.0) * radius;										
						y_rot = cos((float)5 / widget_count * PI * 2.0) * radius + y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7;

						f = elements[i].param.color[3];
						if(seduce_widget_slider_radial(input, &id_fragmented[5], &f, x_rot, y_rot, SEDUCE_WIDGET_LIST_ELEMENT_SIZE, 1, 0, 1, time, color))
						{
							elements[i].param.color[3] = f;
							return i;
						}
						d = elements[i].param.color[0];
						if(S_TIS_DONE == seduce_text_edit_double(input, &id_fragmented[6], NULL, &d, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.0, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2.625 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
						{
							elements[i].param.color[0] = d;
							return i;
						}
						d = elements[i].param.color[1];
						if(S_TIS_DONE == seduce_text_edit_double(input, &id_fragmented[7], NULL, &d, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.75, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.875 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
						{
							elements[i].param.color[1] = d;
							return i;
						}
						d = elements[i].param.color[2];
						if(S_TIS_DONE == seduce_text_edit_double(input, &id_fragmented[8], NULL, &d, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.75, y_widget - element_size * 0.5 - SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.875 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
						{
							elements[i].param.color[2] = d;
							return i;
						}
						d = elements[i].param.color[3];
						if(S_TIS_DONE == seduce_text_edit_double(input, &id_fragmented[9], NULL, &d, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.0, y_widget - element_size * 0.5 - SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2.625 - SEDUCE_T_SIZE * 0.75, 0.1, SEDUCE_T_SIZE, TRUE, NULL, NULL, color[0], color[1], color[2], color[3], text_color[0], text_color[1], text_color[2], text_color[3]))
						{
							elements[i].param.color[3] = d;
							return i;
						}
						if(input->mode == BAM_DRAW)
						{							
							seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.25, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * (2.5 + 0.875), SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
							seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 4.0, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * (0.75 + 0.875), SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
							seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 4.0, y_widget - element_size * 0.5 - SEDUCE_WIDGET_LIST_ELEMENT_SIZE * (1.0 - 0.875), SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
							seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.25, y_widget - element_size * 0.5 - SEDUCE_WIDGET_LIST_ELEMENT_SIZE * (2.75 - 0.875), SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
						}
					}
				}
			break;
				if(input->mode == BAM_DRAW)
				{
					float x_rot, y_rot;
					uint j;
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
					for(j = 0; j < 6; j++)
					{
						x_rot = sin((float)j / 6.0 * PI * 2.0) * SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
						y_rot = cos((float)j / 6.0 * PI * 2.0) * SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5;
						seduce_primitive_circle_add_3d(object,
											x_rot, y_rot + y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
					}		
					seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.0, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 2.5, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
					seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.5, y_widget - element_size * 0.5 + SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 0.75, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);							
					seduce_widget_list_text_line(object, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 3.0, y_widget - element_size * 0.5 - SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.0, SEDUCE_WIDGET_LIST_ELEMENT_SIZE * 1.5 + (1.0 - time) * f_randf(i + 1) * 0.4, 0.05, color, time);										
				}				
			break;
			case SEDUCE_PET_TIME :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_DATE :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_SELECT :
			/*	if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);*/

				if(S_TIS_DONE == seduce_widget_select_radial(input, &elements[i], &elements[i].param.select.active, elements[i].param.select.text, elements[i].param.select.count, elements[i].param.select.style, 0, y_widget - element_size * 0.5, element_size, 1, time, FALSE))
					return i;
	
				if(input->mode == BAM_DRAW)
					seduce_text_line_draw(NULL, element_size * 0.5 + SEDUCE_T_SIZE, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, SEDUCE_T_SIZE,  SEDUCE_T_SPACE, elements[i].param.select.text[elements[i].param.select.active], text_color[0], text_color[1], text_color[2], text_color[3], -1);

			break;
			case SEDUCE_PET_POPUP :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_IMAGE :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_SECTION_START :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_SECTION_END :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_CUSTOM :
				if(input->mode == BAM_DRAW)
					seduce_primitive_circle_add_3d(object,
											0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
											0, 1, 0,
											0, 0, 1,
											element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
											0, 1,
											y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			break;
			case SEDUCE_PET_OK_CANCEL :
				f = ((text_size + element_right) - 0.05) / 2.0 - element_size;
				if(input->mode == BAM_DRAW)
				{
					seduce_widget_list_text_line(object, -text_size + element_size * 0.5, y_widget, element_size + (1.0 - time) * f_randf(i + 1) * 0.4, f, color, time);
					seduce_widget_list_text_line(object, element_right - element_size * 0.5 - f, y_widget, element_size + (1.0 - time) * f_randf(i + 1) * 0.4, f, color, time);
				}
				elements[i].param.ok_cancel = SEDUCE_PEOCS_UNDECIDED;
				if(seduce_text_button(input, &elements[i].param.ok_cancel, -text_size + element_size * 0.5 + f * 0.5, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, 0.5, SEDUCE_T_SIZE,  SEDUCE_T_SPACE, "CANCEL", color[0], color[1], color[2], color[3], 1.0 - color[0], 1.0 - color[1], 1.0 - color[2], color[3]))			
				{
					elements[i].param.ok_cancel = SEDUCE_PEOCS_CANCEL;
					return i;
				}
				if(seduce_text_button(input, &elements[i].type, element_right - element_size * 0.5 - f * 0.5, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, 0.5, SEDUCE_T_SIZE,  SEDUCE_T_SPACE, "OK", color[0], color[1], color[2], color[3], 1.0 - color[0], 1.0 - color[1], 1.0 - color[2], color[3]))
				{
					elements[i].param.ok_cancel = SEDUCE_PEOCS_OK;
					return i;
				}
			break;
		}

	/*	if(input->mode == BAM_DRAW)
		{
			seduce_primitive_circle_add_3d(object,
									0, y_widget - element_size * 0.5, (1.0 - time) * f_randnf(i) * 0.7,
									0, 1, 0,
									0, 0, 1,
									element_size * 0.5 + (1.0 - time) * f_randf(i + 1) * 0.4,
									0, 1,
									y_widget + f_randnf(i + 2) * time * 0.1, element_size * 5 + (1.0 - time) * f_randf(i + 1),
									color[0], color[1], color[2], color[3],
									color[0], color[1], color[2], color[3]);

		}*/
		
		if(input->mode == BAM_DRAW)
		{
			f = (float)i / (float)element_count - 0.5;
			if(f < 0.0)
				f = -f;
			if(time > f && elements[i].type != SEDUCE_PET_OK_CANCEL)
			{
				f = -0.2 + (time - f) * 0.5;
				if(f > 0.0)
					f = 0.0;
				if(style == SEDUCE_WLS_COMPACT && background_color != NULL)
					seduce_text_line_draw(NULL, f - seduce_text_line_length(NULL, SEDUCE_T_SIZE,  SEDUCE_T_SPACE, elements[i].text, -1) - element_left - SEDUCE_WIDGET_LIST_SPACING - SEDUCE_T_SIZE * 2, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, SEDUCE_T_SIZE,  SEDUCE_T_SPACE, elements[i].text, text_color[0], text_color[1], text_color[2], text_color[3], -1);			
				else
					seduce_text_line_draw(NULL, f - seduce_text_line_length(NULL, SEDUCE_T_SIZE,  SEDUCE_T_SPACE, elements[i].text, -1) - element_size * 0.5 - SEDUCE_T_SIZE * 2, y_text - element_size * 0.5 - SEDUCE_T_SIZE * 0.75, SEDUCE_T_SIZE,  SEDUCE_T_SPACE, elements[i].text, text_color[0], text_color[1], text_color[2], text_color[3], -1);
			}
			y_text -= element_size;
			y_widget -= element_size * time;
			if(i + 1 < element_count && elements[i + 1].type != SEDUCE_PET_OK_CANCEL)
				seduce_primitive_line_add_3d(object,
											0, y_text, 0,
											0, y_text - SEDUCE_WIDGET_LIST_SPACING, 0,
											color[0], color[1], color[2], color[3],
											color[0], color[1], color[2], color[3]);
			y_text -= SEDUCE_WIDGET_LIST_SPACING;
			y_widget -= SEDUCE_WIDGET_LIST_SPACING * time;
		}
	}

	if(input->mode == BAM_DRAW)
	{
		seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);
		seduce_primitive_line_object_free(object);
	}
	return -1;
}


void seduce_widget_list_element_test2(BInputState *input)
{
	static SeducePanelElement element[SEDUCE_PET_COUNT];
	static uint init = FALSE, selected = -1;
	static float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, timer = 0;
	char *select[3] = {"Option one", "Another option", "The final option"};
	return;
	if(!init)
	{
		init = TRUE;
		element[SEDUCE_PET_BOOLEAN].type = SEDUCE_PET_BOOLEAN;
		element[SEDUCE_PET_BOOLEAN].text = "BOOLEAN"; 
		element[SEDUCE_PET_BOOLEAN].param.active = TRUE;
		element[SEDUCE_PET_TRIGGER].type = SEDUCE_PET_TRIGGER;
		element[SEDUCE_PET_TRIGGER].text = "TRIGGER"; 
		element[SEDUCE_PET_TRIGGER].param.trigger = TRUE;
		element[SEDUCE_PET_INTEGER].type = SEDUCE_PET_INTEGER;
		element[SEDUCE_PET_INTEGER].text = "INTEGER"; 
		element[SEDUCE_PET_INTEGER].param.integer = 1;
		element[SEDUCE_PET_UNSIGNED_INTEGER].type = SEDUCE_PET_UNSIGNED_INTEGER;
		element[SEDUCE_PET_UNSIGNED_INTEGER].text = "UNSIGNED INTEGER";
		element[SEDUCE_PET_UNSIGNED_INTEGER].param.uinteger = 1;
		element[SEDUCE_PET_INTEGER_BOUND].type = SEDUCE_PET_INTEGER_BOUND;
		element[SEDUCE_PET_INTEGER_BOUND].text = "INTEGER BOUND";
	//	element[SEDUCE_PET_INTEGER_BOUND].param.value = 01;
		element[SEDUCE_PET_REAL].type = SEDUCE_PET_REAL;
		element[SEDUCE_PET_REAL].text = "REAL";
		element[SEDUCE_PET_REAL].param.real.value = 0.5;
		element[SEDUCE_PET_REAL_BOUND].type = SEDUCE_PET_REAL_BOUND;
		element[SEDUCE_PET_REAL_BOUND].text = "REAL BOUND"; 
		element[SEDUCE_PET_REAL].param.real.value = 0.5;
		element[SEDUCE_PET_REAL].param.real.min = 0.0;
		element[SEDUCE_PET_REAL].param.real.max = 1.0;
		element[SEDUCE_PET_2D_POS].type = SEDUCE_PET_2D_POS;
		element[SEDUCE_PET_2D_POS].text = "2D POS"; 
		element[SEDUCE_PET_3D_POS].type = SEDUCE_PET_3D_POS;
		element[SEDUCE_PET_3D_POS].text = "3D POS"; 
		element[SEDUCE_PET_4D_POS].type = SEDUCE_PET_4D_POS;
		element[SEDUCE_PET_4D_POS].text = "4D POS"; 
		element[SEDUCE_PET_QUATERNION].type = SEDUCE_PET_QUATERNION;
		element[SEDUCE_PET_QUATERNION].text = "QUATERNION"; 
		element[SEDUCE_PET_2D_NORMAL].type = SEDUCE_PET_2D_NORMAL;
		element[SEDUCE_PET_2D_NORMAL].text = "2D NORMAL"; 
		element[SEDUCE_PET_3D_NORMAL].type = SEDUCE_PET_3D_NORMAL;
		element[SEDUCE_PET_3D_NORMAL].text = "3D NORMAL"; 
		element[SEDUCE_PET_2X2MATRIX].type = SEDUCE_PET_2X2MATRIX;
		element[SEDUCE_PET_2X2MATRIX].text = "2X2MATRIX"; 
		element[SEDUCE_PET_3X3MATRIX].type = SEDUCE_PET_3X3MATRIX;
		element[SEDUCE_PET_3X3MATRIX].text = "3X3MATRIX"; 
		element[SEDUCE_PET_4X4MATRIX].type = SEDUCE_PET_4X4MATRIX;
		element[SEDUCE_PET_4X4MATRIX].text = "4X4MATRIX"; 
		element[SEDUCE_PET_TEXT].type = SEDUCE_PET_TEXT;
		element[SEDUCE_PET_TEXT].text = "TEXT"; 
		element[SEDUCE_PET_PASSWORD].type = SEDUCE_PET_PASSWORD;
		element[SEDUCE_PET_PASSWORD].text = "PASSWORD"; 
		element[SEDUCE_PET_TEXT_BUFFER].type = SEDUCE_PET_TEXT_BUFFER;
		element[SEDUCE_PET_TEXT_BUFFER].text = "TEXT BUFFER"; 
		element[SEDUCE_PET_COLOR_RGB].type = SEDUCE_PET_COLOR_RGB;
		element[SEDUCE_PET_COLOR_RGB].text = "COLOR RGB"; 
		element[SEDUCE_PET_COLOR_RGBA].type = SEDUCE_PET_COLOR_RGBA;
		element[SEDUCE_PET_COLOR_RGBA].text = "COLOR RGBA"; 
		element[SEDUCE_PET_TIME].type = SEDUCE_PET_TIME;
		element[SEDUCE_PET_TIME].text = "TIME"; 
		element[SEDUCE_PET_DATE].type = SEDUCE_PET_DATE;
		element[SEDUCE_PET_DATE].text = "DATE"; 
		element[SEDUCE_PET_SELECT].type = SEDUCE_PET_SELECT;
		element[SEDUCE_PET_SELECT].text = "SELECT"; 
		element[SEDUCE_PET_SELECT].param.select.active = 0;
		element[SEDUCE_PET_SELECT].param.select.text = select;
		element[SEDUCE_PET_SELECT].param.select.count = 3;
		element[SEDUCE_PET_SELECT].param.select.style = S_PUT_BOTTOM;
		element[SEDUCE_PET_SECTION_START].type = SEDUCE_PET_SECTION_START;
		element[SEDUCE_PET_SECTION_START].text = "SECTION START"; 
		element[SEDUCE_PET_SECTION_END].type = SEDUCE_PET_SECTION_END;
		element[SEDUCE_PET_SECTION_END].text = "SECTION END";
	}

	seduce_animate(input, &timer, !input->pointers[0].button[1], 1.0); 
	seduce_background_shape_matrix_interact(input, &init, matrix, TRUE, TRUE);
	r_matrix_push(NULL);
	r_matrix_matrix_mult(NULL, matrix);
	
//	r_matrix_rotate(NULL,input->minute_time * 720, 1, 0, 0);
	seduce_widget_list(input, &init, element, SEDUCE_PET_SECTION_END + 1, timer, "SETTINGS", NULL, NULL, 0);
	r_matrix_pop(NULL);

}