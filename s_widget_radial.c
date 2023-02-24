/* Written by Eskil Steenberg Hald, Copyright Shimmer industries 2022 */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "seduce.h"
#include "s_draw_3d.h"


extern void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float scale, uint id, float fade,  float *color);

#define SUI_SLIDER_RADIAL_SPLITS 64
#define SUI_SLIDER_RADIAL_SNAP_ONE 100
#define SUI_SLIDER_RADIAL_SNAP_TWO 24
#define SUI_SLIDER_RADIAL_SNAP_THREE 16


void seduce_widget_overlay_matrix(RMatrix *matrix)
{
	float aspect, view[3] = {0.0, 0.0, 1.0};
	RMatrix *m;
	m = r_matrix_get();
	aspect = matrix->aspect = m->aspect;
	r_matrix_identity(matrix);
/*	r_matrix_frustum(matrix, -0.05, 0.05, -0.05 * aspect, 0.05 * aspect, 0.05, 10.0);
	r_matrix_translate(matrix, 0.0, 0.0, -1.0);
*/	betray_view_vantage(view);
/*	view[0] *= 1.2;
	view[1] *= 1.2;
	view[2] *= 1.2;*/
	r_matrix_frustum(matrix, -0.01 - view[0] * 0.01, 0.01 - view[0] * 0.01, -0.01 * aspect - view[1] * 0.01, 0.01 * aspect - view[1] * 0.01, 0.01 * view[2], 100.0); /* set frustum */
	r_matrix_translate(matrix, -view[0], -view[1], -view[2]);
	r_matrix_set(matrix);
}

float seduce_widget_slider_radial_value(BInputState *input, float pointer_x, float pointer_y, float scale, uint *snap, float *aim, float value, float init_value)
{
	float f, vec[2], rot[2], dist;
	uint i;

	if(pointer_x * pointer_x + pointer_y * pointer_y < 0.05 * 0.05 * scale * scale)
		return init_value;
	if(aim != NULL)
	{
		aim[0] = pointer_x / scale;
		aim[1] = pointer_y / scale;
	}

	f = atan2(-pointer_x, -pointer_y);
	f = 0.5 + f / (2.0 * PI);

	if(f > 0.75 && value < 0.25)
		f = 0.0;
	else if(f < 0.25 && value > 0.75)
		f = 1.0;
		
	vec[0] = pointer_x / scale;
	vec[1] = pointer_y / scale;

	dist = vec[0] * vec[0] + vec[1] * vec[1];
	if(dist > 0.096 * 0.096 && dist < 0.104 * 0.104)
	{
		f += 0.5 / SUI_SLIDER_RADIAL_SNAP_ONE;
		i = f * SUI_SLIDER_RADIAL_SNAP_ONE;
		if(snap != NULL)
			*snap = i;
		return  (float)i / SUI_SLIDER_RADIAL_SNAP_ONE;
	}

/*

	for(i = 0; i < SUI_SLIDER_RADIAL_SNAP_ONE; i++)
	{
		rot[0] = sin((float)i * PI * 2.0 / SUI_SLIDER_RADIAL_SNAP_ONE) * 0.1;
		rot[1] = cos((float)i * PI * 2.0 / SUI_SLIDER_RADIAL_SNAP_ONE) * 0.1;
		vec[0] = rot[0] - pointer_x / scale;
		vec[1] = rot[1] - pointer_y / scale;
		if(vec[0] * vec[0] + vec[1] * vec[1] < 0.08 * 0.08)
		{
			f = (float)i / SUI_SLIDER_RADIAL_SNAP_ONE;
			if(snap != NULL)
				*snap = i;
			if(aim != NULL)
			{
				aim[0] = rot[0];
				aim[1] = rot[1];
			}
			return f;
		}
	}*/



	for(i = 0; i < SUI_SLIDER_RADIAL_SNAP_TWO; i++)
	{
		if(i % 3 != 0)
		{
			rot[0] = sin((float)i * PI * 2.0 / SUI_SLIDER_RADIAL_SNAP_TWO) * 0.1325;
			rot[1] = cos((float)i * PI * 2.0 / SUI_SLIDER_RADIAL_SNAP_TWO) * 0.1325;
			vec[0] = rot[0] - pointer_x / scale;
			vec[1] = rot[1] - pointer_y / scale;
			if(vec[0] * vec[0] + vec[1] * vec[1] < 0.01 * 0.01)
			{
				f = (float)i / SUI_SLIDER_RADIAL_SNAP_TWO;
				if(snap != NULL)
					*snap = SUI_SLIDER_RADIAL_SNAP_ONE + i;
				if(aim != NULL)
				{
					aim[0] = rot[0];
					aim[1] = rot[1];
				}
				return f;
			}
		}
	}
	for(i = 0; i < SUI_SLIDER_RADIAL_SNAP_THREE; i++)
	{
		rot[0] = sin((float)i * PI * 2.0 / SUI_SLIDER_RADIAL_SNAP_THREE) * 0.1625;
		rot[1] = cos((float)i * PI * 2.0 / SUI_SLIDER_RADIAL_SNAP_THREE) * 0.1625;
		vec[0] = rot[0] - pointer_x / scale;
		vec[1] = rot[1] - pointer_y / scale;
		if(vec[0] * vec[0] + vec[1] * vec[1] < 0.01 * 0.01)
		{
			f = (float)i / SUI_SLIDER_RADIAL_SNAP_THREE;
			if(snap != NULL)
				*snap = SUI_SLIDER_RADIAL_SNAP_ONE + SUI_SLIDER_RADIAL_SNAP_TWO + i;
			if(aim != NULL)
			{
				aim[0] = rot[0];
				aim[1] = rot[1];
			}
			return f;
		}
	}
	return f;
}


float seduce_widget_slider_line_value(BInputState *input, float pointer_x, float pointer_y, float scale, float pos_a_x, float pos_a_y, float pos_b_x, float pos_b_y)
{
	float vec[2], f;
	vec[0] = pos_a_x - pos_b_x;
	vec[1] = pos_a_y - pos_b_y;
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	f = ((pointer_x - pos_a_x) * vec[0] / f + (pointer_y - pos_a_y) * vec[1] / f) / -f;
	if(f > 1.0)
		f = 1.0;
	if(f < 0.0)
		f = 0.0;
	return f;
}

extern void seduce_text_deactivate(uint user_id);

typedef struct{
	float init_value;
	float timer;
	float pos[6];
	void *id;
	boolean active;
	boolean text_active;
	RMatrix matrix;
}SRadialGrab;

STypeInState seduce_widget_slider_radial(BInputState *input, void *id, float *value, float pos_x, float pos_y, float size, float scale, float min, float max, float time, float *color)
{
	static boolean *a_button, *a_button_last;
	static SRadialGrab *grab = NULL;
	static uint frame_id = -1;
	int i, j, k, axis, pointer_count, user_count;
	char text_buf[128];
	float c[4] = {1, 1, 1, 1};
	RMatrix *matrix, overlay;
	if(color == NULL)
		color = c;
	


	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(grab == NULL)
	{
		uint i;
		grab = malloc((sizeof *grab) * (pointer_count + user_count));

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			grab[i].timer = 0;
			grab[i].id = NULL;
			grab[i].active = FALSE;
			grab[i].text_active = FALSE;
		}
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}

	if(input->mode == BAM_DRAW)
	{
		static SeduceLineObject *object = NULL, *snaps = NULL;
		float v, a[2], b[2], height, x, y, inner, outer;
		int count;
		float on[3] = {0.2, 0.6, 1.0}, *p = NULL, pos[3], f, timer = 0, aim[2];
		uint snap;

		if(object == NULL)
			object = seduce_primitive_line_object_allocate();

		p = color;
		if(p == NULL)
		{
			for(i = 0; i < input->pointer_count; i++)
				if(id == seduce_element_pointer_id(input, i, NULL))
					break;
			if(i < input->pointer_count)
				p = on;
			else
				for(i = 0; i < (pointer_count + user_count); i++)
					if(grab[i].id == id)
						p = on;
		}
		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, size);
		size *= (time + time * (1.0 - time) * 3.0) * 0.5;
		matrix = r_matrix_get();

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].timer > 0.01 && grab[i].id == id)
			{
				snap = -1;
				if(i < pointer_count)
					seduce_widget_slider_radial_value(input, input->pointers[i].pointer_x - grab[i].pos[0], input->pointers[i].pointer_y - grab[i].pos[1], scale, &snap, aim, *value, grab[i].init_value);			
				else
				{
					axis = seduce_element_primary_axis(input, i - pointer_count);
					if(axis != -1)
						seduce_widget_slider_radial_value(input, input->axis[axis].axis[0] * scale, input->axis[axis].axis[1] * scale, scale, &snap, aim, *value, grab[i].init_value);			
				}

				seduce_widget_overlay_matrix(&overlay);
				r_matrix_translate(&overlay, grab[i].pos[0], grab[i].pos[1], 0);
				scale *= 0.1;
				if(snaps == NULL)
					snaps = seduce_primitive_line_object_allocate();

			//	seduce_primitive_background_flare_draw(0, 0, -0.2 * scale, grab[i].timer * 0.5, scale * 2.0);
			//	r_matrix_rotate(&overlay, grab[i].timer * 90.0 - 90.0, 0, 1, 0);
			/*	seduce_primitive_circle_add_3d(snaps,
									pos_x, pos_y, 0,
									0, 1, 0,
									0, 0, 1,
									scale * 0.95,
									0, *value,
									0, 1,
									color[0], color[1], color[2], 1,
									color[0], color[1], color[2], 1);
				seduce_primitive_circle_add_3d(snaps,
									pos_x, pos_y, 0,
									0, 1, 0,
									0, 0, 1,
									scale * 0.95,
									*value, 1.0 - *value,
									0, 1,
									0.2, 0.2, 0.2, 1,
									0.2, 0.2, 0.2, 1);*/

				inner = grab[i].timer * scale;
				outer = grab[i].timer * grab[i].timer * scale;
				seduce_primitive_circle_add_3d(snaps,
									0, 0, 0,
									0, 1, 0,
									0, 0, 1,
									inner * 1.075,
									0, *value,
									0, *value * inner * 2.0 * PI,
									color[0], color[1], color[2], 1,
									color[0], color[1], color[2], 1);
				seduce_primitive_circle_add_3d(snaps,
									0, 0, 0,
									0, 1, 0,
									0, 0, 1,
									outer * 1.075,
									*value, (1.0 - *value),
									*value * inner * 2.0 * PI, (1.0 - *value) * inner * 2.0 * PI,
									0.2, 0.2, 0.2, 1,
									0.2, 0.2, 0.2, 1);
				k = 0;
				for(j = 0; j < SUI_SLIDER_RADIAL_SNAP_ONE; j++)
				{
					x = sin(j * 2.0 * PI / SUI_SLIDER_RADIAL_SNAP_ONE);
					y = cos(j * 2.0 * PI / SUI_SLIDER_RADIAL_SNAP_ONE);
					f = 0.96;
					if(j % 10 == 0)
						f = 0.92;
					else if(j % 5 == 0)
						f = 0.94;
					if(snap == k)
						seduce_primitive_line_add_3d(snaps,
								x * f * inner, y * f * inner, 0,
								x * 1.075 * inner, y * 1.075 * inner, 0,
								color[0], color[1], color[2], 1,
								color[0], color[1], color[2], 1);
					else
						seduce_primitive_line_add_3d(snaps,
								x * f * inner, y * f * inner, 0,
								x * 1.04 * inner, y * 1.04 * inner, 0,
								color[0], color[1], color[2], 1,
								color[0], color[1], color[2], 1);
					k++;
				}	

				for(j = 0; j < SUI_SLIDER_RADIAL_SNAP_TWO; j++)
				{
					if(j % 3 != 0)
					{
						x = sin(j * 2.0 * PI / SUI_SLIDER_RADIAL_SNAP_TWO);
						y = cos(j * 2.0 * PI / SUI_SLIDER_RADIAL_SNAP_TWO);
						if(snap == k)
							seduce_primitive_line_add_3d(snaps,
									x * 1.075 * inner, y * 1.075 * inner, 0,
									x * 1.3 * outer, y * 1.3 * outer, 0,
								color[0], color[1], color[2], 1,
								color[0], color[1], color[2], 1);
						else
							seduce_primitive_line_add_3d(snaps,
									x * 1.1 * inner, y * 1.1 * inner, 0,
									x * 1.3 * outer, y * 1.3 * outer, 0,
									0.1, 0.1, 0.1, 1,
									0.1, 0.1, 0.1, 1);
						seduce_primitive_line_add_3d(snaps,
								x * 1.30 * outer, y * 1.30 * outer, 0,
								x * 1.35 * outer, y * 1.35 * outer, 0,
								color[0], color[1], color[2], 1,
								color[0], color[1], color[2], 1);
					}
					k++;
				}		
				for(j = 0; j < SUI_SLIDER_RADIAL_SNAP_THREE; j++)
				{
					x = sin(j * 2.0 * PI / SUI_SLIDER_RADIAL_SNAP_THREE);
					y = cos(j * 2.0 * PI / SUI_SLIDER_RADIAL_SNAP_THREE);
					if(snap == k)
						seduce_primitive_line_add_3d(snaps,
								x * 1.075 * inner, y * 1.075 * inner, 0,
								x * 1.6 * outer, y * 1.6 * outer, 0,
								color[0], color[1], color[2], 1,
								color[0], color[1], color[2], 1);
					else
						seduce_primitive_line_add_3d(snaps,
								x * 1.1 * inner, y * 1.1 * inner, 0,
								x * 1.6 * outer, y * 1.6 * outer, 0,
								0.1, 0.1, 0.1, 1,
								0.1, 0.1, 0.1, 1);
					seduce_primitive_line_add_3d(snaps,
							x * 1.60 * outer, y * 1.60 * outer, 0,
							x * 1.65 * outer, y * 1.65 * outer, 0,
							color[0], color[1], color[2], 1,
							color[0], color[1], color[2], 1);
					k++;
				}			
				
				seduce_primitive_line_add_3d(snaps,
							0, 0, 0,
							sin(*value * 2.0 * PI), cos(*value * 2.0 * PI), 0,
							color[0], color[1], color[2], 1,
							color[0], color[1], color[2], 1);
				seduce_primitive_line_draw(snaps, 1.0, 1.0, 1.0, 1.0);
				r_matrix_set(matrix);

				/*
				timer = grab[i].timer;
				seduce_background_particle_spawn(input, pos[0], pos[1], sin(*value * 2.0 * PI) * 0.4, cos(*value * 2.0 * PI) * 0.4, 0.0, S_PT_SPLAT_ONE);
				seduce_widget_overlay_matrix(&overlay);
				r_matrix_translate(&overlay, grab[i].pos[0], grab[i].pos[1], 0);
				r_matrix_scale(&overlay, scale, scale, scale);
				snap = -1;
				if(i < pointer_count)
					seduce_widget_slider_radial_value(input, input->pointers[i].pointer_x - grab[i].pos[0], input->pointers[i].pointer_y - grab[i].pos[1], scale, &snap, aim, *value, grab[i].init_value);			
				else
				{
					axis = seduce_element_primary_axis(input, i - pointer_count);
					if(axis == -1)
						*value = grab[i].init_value;
					else
						seduce_widget_slider_radial_value(input, input->axis[axis].axis[0] * scale, input->axis[axis].axis[1] * scale, scale, &snap, aim, *value, grab[i].init_value);			
				}

				r_matrix_push(&overlay);
				r_matrix_translate(&overlay, aim[0], aim[1], 0);
				r_matrix_rotate(&overlay, *value * -360.0, 0, 0, 1);
				seduce_object_3d_draw(input, 0, 0, 0, 0.1, SUI_3D_OBJECT_SNAPLOCK, timer, color);
				r_matrix_pop(&overlay);
				f = (grab[i].timer - 0.2) / 0.3 * 0.3;
				if(f > 0.0)
				{
					if(f > 0.3)
						f = 0.3;
					r_matrix_push(&overlay);
					r_matrix_rotate(&overlay, (1.0 - grab[i].timer) * 72.0, 1, 0, 0);	
					for(j = 0; j < SUI_SLIDER_RADIAL_SNAP_ONE; j++)
					{
						r_matrix_push(&overlay);
						r_matrix_rotate(&overlay, j * -360.0 / SUI_SLIDER_RADIAL_SNAP_ONE, 0, 0, 1);
						if(j == snap)
							seduce_object_3d_draw(input, 0, f, 0, 0.15, SUI_3D_OBJECT_SNAP_SMALL, timer, on);
						else
							seduce_object_3d_draw(input, 0, f, 0, 0.1, SUI_3D_OBJECT_SNAP_SMALL, timer, NULL);
						r_matrix_pop(&overlay);
					}
					r_matrix_pop(&overlay);
				}

				r_matrix_push(&overlay);
				r_matrix_rotate(&overlay, (1.0 - grab[i].timer) * 72.0, 1, 1, 0);	
				for(j = 0; j < SUI_SLIDER_RADIAL_SNAP_TWO; j++)
				{
					r_matrix_push(&overlay);
					r_matrix_rotate(&overlay, j * -360.0 / SUI_SLIDER_RADIAL_SNAP_TWO + (1 - grab[i].timer) * 90.0, 0, 0, 1);
		
					if(j + SUI_SLIDER_RADIAL_SNAP_ONE == snap)
						seduce_object_3d_draw(input, 0, grab[i].timer * 0.4, 0, 0.15, SUI_3D_OBJECT_SNAP_LARGE, timer, on);
					else
						seduce_object_3d_draw(input, 0, grab[i].timer * 0.4, 0, 0.1, SUI_3D_OBJECT_SNAP_LARGE, timer, NULL);
					r_matrix_pop(&overlay);
				}
				r_matrix_pop(&overlay);
				f = (grab[i].timer - 0.6) / 0.4 * 0.5;
				if(f > 0.0)
				{
					r_matrix_push(&overlay);
					r_matrix_rotate(&overlay, (1.0 - grab[i].timer) * 72.0, 0, 1, 0);	
					if(f > 0.5)
						f = 0.5;
					for(j = 0; j < SUI_SLIDER_RADIAL_SNAP_THREE; j++)
					{
						r_matrix_push(&overlay);
						r_matrix_rotate(&overlay, j * -360.0 / SUI_SLIDER_RADIAL_SNAP_THREE, 0, 0, 1);
						if(j + SUI_SLIDER_RADIAL_SNAP_ONE + SUI_SLIDER_RADIAL_SNAP_TWO == snap)
							seduce_object_3d_draw(input, 0, f, 0, scale * 0.15, SUI_3D_OBJECT_SNAP_SMALL, timer, on);
						else
							seduce_object_3d_draw(input, 0, f, 0, scale * 0.1, SUI_3D_OBJECT_SNAP_SMALL, timer, NULL);
						r_matrix_pop(&overlay);
					}
					r_matrix_pop(&overlay);
				}
				r_matrix_set(matrix);
				if(size < scale * 0.2)
					size = size * (1.0 - grab[i].timer) + scale * 0.2 * grab[i].timer;
				*/
			}
		}

		seduce_primitive_circle_add_3d(object,
							pos_x, pos_y, 0,
							0, 1, 0,
							0, 0, 1,
							size,
							0, *value,
							0, size * 2 * PI,
							color[0], color[1], color[2], 1,
							color[0], color[1], color[2], 1);
		seduce_primitive_circle_add_3d(object,
							pos_x, pos_y, 0,
							0, 1, 0,
							0, 0, 1,
							size,
							*value, (1.0 - *value),
							size * *value * 2 * PI, size * (1.0 - *value) * 2 * PI,
							0.2, 0.2, 0.2, 1,
							0.2, 0.2, 0.2, 1);
		seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);


/*		v = (min - *value) / (max - min);
		if(v > 1.0)
			v = 1.0;
		if(v < 0.0)
			v = 0.0;

		i = (v * time) * 8.01;
		if(i < 0)
			i = 0;
		if(i > 8)
			i = 8;
	
		r_matrix_push(matrix);
		r_matrix_translate(matrix, pos_x, pos_y, 0);
		r_matrix_scale(matrix, time, time, time);
		seduce_object_3d_draw(input, 0, 0, 0, size, objects[i], timer, color);
		r_matrix_rotate(matrix, 360.0 - (v * time) * 360.0, 0, 0, 1);
		f = ((v * time) * 8.01 - (float)i);
		r_matrix_scale(matrix, (1 - ((v * time) * 8.01 - (float)i)) * (1 + f * 0.5), 1 + f * 0.5, 1);
		r_matrix_rotate(matrix, -45.0, 0, 0, 1);

		if(*value < 0.999)
			seduce_object_3d_draw(input, 0, 0, 0, size, SUI_3D_OBJECT_RADIAL_SHARD, timer, color);
		r_matrix_pop(matrix);*/

	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(!input->pointers[i].button[0] && !grab[i].text_active && grab[i].active)
			{
				grab[i].active = FALSE;
				return S_TIS_DONE;
			}
			if(grab[i].id == id && grab[i].active && input->pointers[i].button[0])
			{
				*value = seduce_widget_slider_radial_value(input, input->pointers[i].pointer_x - grab[i].pos[0], input->pointers[i].pointer_y - grab[i].pos[1], scale, NULL, NULL, *value, grab[i].init_value);
				if((input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) *
					(input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) +
					(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) *
					(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) >
					0.005 * 0.005)
				{
					grab[i].text_active = FALSE;
					seduce_text_deactivate(input->pointers[i].user_id);
				}
			}else if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, NULL))
					{
						grab[i].pos[0] = input->pointers[i].pointer_x;
						grab[i].pos[1] = input->pointers[i].pointer_y;
						grab[i].init_value = *value;
						grab[i].active = TRUE;
						grab[i].text_active = TRUE;
						grab[i].id = id;
						grab[i].timer = 0.1;
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			float pos[3];
			if(id == seduce_element_selected_id(i, pos, NULL))
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);
				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
					axis = seduce_element_primary_axis(input, i);
					if(axis == -1)
						*value = grab[i].init_value;
					else
						*value = seduce_widget_slider_radial_value(input, input->axis[axis].axis[0] * scale, input->axis[axis].axis[1] * scale, scale, NULL, NULL, *value, grab[i + pointer_count].init_value);
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						grab[i + pointer_count].pos[0] = pos[0];
						grab[i + pointer_count].pos[1] = pos[1];
						grab[i + pointer_count].init_value = *value;
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
				if(!a_button[i] && !grab[i + pointer_count].text_active)
				{
					grab[i + pointer_count].active = FALSE;
					return S_TIS_DONE;
				}
			}
		}
	}

	if(input->mode == BAM_MAIN && frame_id != input->frame_number)
	{
		frame_id = input->frame_number;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].active)
			{
				grab[i].timer += input->delta_time * 2.5;
				if(grab[i].timer > 1.0)		
					grab[i].timer = 1.0;
			}else
				grab[i].timer -= input->delta_time * 2.5;
		}
	}
	
	for(i = 0; i < (pointer_count + user_count) && (grab[i].id != id || (!grab[i].active && grab[i].timer < 0.0001)); i++);

	if(i < (pointer_count + user_count))
	{
		if(input->mode == BAM_DRAW)
		{
			r_matrix_push(matrix);
			r_matrix_translate(matrix, 0, 0, grab[i].timer * size * 0.35);

			/*		r_matrix_translate(matrix, pos_x, pos_y, grab[i].timer * size * 0.35);
					r_matrix_scale(matrix, grab[i].timer, grab[i].timer, grab[i].timer);
					sprintf(text_buf, "%.03f", *value);
					f = seduce_text_line_length(NULL, SEDUCE_T_SIZE * scale * 8.0 * -0.5, SEDUCE_T_SPACE, text_buf, -1);
					seduce_text_line_draw(NULL, f, 0, SEDUCE_T_SIZE * scale * 8.0, SEDUCE_T_SPACE, text_buf, 1, 1, 1, 1, -1);*/
		}
		if(!seduce_text_edit_float(input, id, NULL, value, pos_x, pos_y, 2, SEDUCE_T_SIZE * scale * 8.0, TRUE, NULL, NULL, 0, 0, 0, grab[i].timer, 0, 0, 0, grab[i].timer) && grab[i].text_active)
		{
			grab[i].active = FALSE;
			return S_TIS_DONE;
		}
		if(input->mode == BAM_DRAW)
			r_matrix_pop(matrix);
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}



STypeInState seduce_widget_slider_radius(BInputState *input, void *id, float *value, float pos_x, float pos_y, float size, float time, float *color)
{
	static boolean *a_button, *a_button_last;
	static SRadialGrab *grab = NULL;
	static uint frame_id = -1;
	int i, j, k, axis, pointer_count, user_count;
	char text_buf[128];
	float c[4] = {1, 1, 1, 1};
	RMatrix *matrix, overlay;
	if(color == NULL)
		color = c;
	size *= (time + time * (1.0 - time) * 3.0) * 0.5;


	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(grab == NULL)
	{
		uint i;
		grab = malloc((sizeof *grab) * (pointer_count + user_count));

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			grab[i].timer = 0;
			grab[i].id = NULL;
			grab[i].active = FALSE;
			grab[i].text_active = FALSE;
		}
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}

	if(input->mode == BAM_DRAW)
	{
		static SeduceLineObject *object = NULL, *snaps = NULL;
		float v, a[2], b[2], height, x, y, inner, outer;
		int count;
		float on[3] = {0.2, 0.6, 1.0}, *p = NULL, pos[3], f, timer = 0, aim[2];
		uint snap;

		if(object == NULL)
			object = seduce_primitive_line_object_allocate();

		p = color;
		if(p == NULL)
		{
			for(i = 0; i < input->pointer_count; i++)
				if(id == seduce_element_pointer_id(input, i, NULL))
					break;
			if(i < input->pointer_count)
				p = on;
			else
				for(i = 0; i < (pointer_count + user_count); i++)
					if(grab[i].id == id)
						p = on;
		}
		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, size);
		matrix = r_matrix_get();

		for(i = 0; i < (pointer_count + user_count) && (!grab[i].active || grab[i].id != id); i++);
		if(i < (pointer_count + user_count))
			size = *value;
		seduce_primitive_circle_add_3d(object,
							pos_x, pos_y, 0,
							0, 1, 0,
							0, 0, 1,
							size,
							0, 1,
							0, size * 2 * PI,
							p[0], p[1], p[2], 1,
							p[0], p[1], p[2], 1);
		seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(grab[i].id == id && grab[i].active && input->pointers[i].button[0])
			{
				*value = sqrt((input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) *
							(input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) +
							(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) *
							(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y));
				if(*value >
					0.005 * 0.005)
				{
					grab[i].text_active = FALSE;
					seduce_text_deactivate(input->pointers[i].user_id);
					return S_TIS_DONE;
				}
			}else if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, NULL))
					{
						grab[i].pos[0] = input->pointers[i].pointer_x;
						grab[i].pos[1] = input->pointers[i].pointer_y;
						grab[i].init_value = *value;
						grab[i].active = TRUE;
						grab[i].text_active = TRUE;
						grab[i].id = id;
						grab[i].timer = 0.1;
					}
				}
			}
			if(!input->pointers[i].button[0] && !grab[i].text_active)
			{
				grab[i].active = FALSE;
				return S_TIS_DONE;
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			float pos[3];
			if(id == seduce_element_selected_id(i, pos, NULL))
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);

				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
					axis = seduce_element_primary_axis(input, i);
					if(axis == -1)
						*value = grab[i].init_value;
					else
						*value = f_length2f(input->axis[axis].axis);
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						grab[i + pointer_count].pos[0] = pos[0];
						grab[i + pointer_count].pos[1] = pos[1];
						grab[i + pointer_count].init_value = *value;
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
				if(!a_button[i] && grab[i + pointer_count].id == id && !grab[i + pointer_count].text_active)
				{
					grab[i + pointer_count].active = FALSE;
					return S_TIS_DONE;
				}
			}
		}
	}

	if(input->mode == BAM_MAIN && frame_id != input->frame_number)
	{
		frame_id = input->frame_number;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].active)
			{
				grab[i].timer += input->delta_time * 2.5;
				if(grab[i].timer > 1.0)		
					grab[i].timer = 1.0;
			}else
				grab[i].timer -= input->delta_time * 2.5;
		}
	}
	
	for(i = 0; i < (pointer_count + user_count) && (grab[i].id != id || (!grab[i].active && grab[i].timer < 0.0001)); i++);

	if(i < (pointer_count + user_count))
	{
		if(input->mode == BAM_DRAW)
		{
			r_matrix_push(matrix);
			r_matrix_translate(matrix, 0, 0, grab[i].timer * size * 0.35);

			/*		r_matrix_translate(matrix, pos_x, pos_y, grab[i].timer * size * 0.35);
					r_matrix_scale(matrix, grab[i].timer, grab[i].timer, grab[i].timer);
					sprintf(text_buf, "%.03f", *value);
					f = seduce_text_line_length(NULL, SEDUCE_T_SIZE * scale * 8.0 * -0.5, SEDUCE_T_SPACE, text_buf, -1);
					seduce_text_line_draw(NULL, f, 0, SEDUCE_T_SIZE * scale * 8.0, SEDUCE_T_SPACE, text_buf, 1, 1, 1, 1, -1);*/
		}
		if(!seduce_text_edit_float(input, id, NULL, value, pos_x, pos_y, 2, SEDUCE_T_SIZE, TRUE, NULL, NULL, 0, 0, 0, grab[i].timer, 0, 0, 0, grab[i].timer) && grab[i].text_active)
		{
			grab[i].active = FALSE;
			return S_TIS_DONE;
		}

		if(input->mode == BAM_DRAW)
			r_matrix_pop(matrix);
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}

STypeInState seduce_widget_slider_line(BInputState *input, void *id, float *value, float pos_a_x, float pos_a_y, float pos_b_x, float pos_b_y, float size, float scale, float min, float max, float time, float *color, boolean snaps)
{
	static SeduceLineObject *object = NULL, *snap = NULL;
	static boolean *a_button, *a_button_last;
	static SRadialGrab *grab = NULL;
	static uint frame_id = -1;
	int i, j, axis, pointer_count, user_count;
	char text_buf[128];
	RMatrix *matrix, overlay;

	size *= (time + time * (1.0 - time) * 3.0) * 0.5;

	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(grab == NULL)
	{
		uint i;
		grab = malloc((sizeof *grab) * (pointer_count + user_count));

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			grab[i].timer = 0;
			grab[i].id = NULL;
			grab[i].active = FALSE;
			grab[i].text_active = FALSE;
		}
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}

	if(input->mode == BAM_DRAW)
	{
		float a[2], b[2], height, vec[2];
		int count;
		float on[3] = {0.2, 0.6, 1.0}, *p = NULL, pos[6], f, timer = 0, aim[2], dist;
		uint snap;
		if(color == NULL)
			p = on;
		else
			p = color;
	//	if(object == NULL)
	//		object = seduce_primitive_line_object_allocate();
		if(p == NULL)
		{
			for(i = 0; i < input->pointer_count; i++)
				if(id == seduce_element_pointer_id(input, i, NULL))
					break;
			if(i < input->pointer_count)
				p = on;
			else
				for(i = 0; i < (pointer_count + user_count); i++)
					if(grab[i].id == id)
						p = on;
		}
		pos[0] = pos_a_x;
		pos[1] = pos_a_y;
		pos[2] = 0;
		pos[3] = pos_b_x;
		pos[4] = pos_b_y;
		pos[5] = 0;
		seduce_element_add_line(input, id, 0, pos, &pos[3], size * 4.0);

		matrix = r_matrix_get();

		f = (*value - min) / (max - min);
		if(f > 1.0)
			f = 1.0;
		if(f < 0.0)
			f = 0.0;
		dist = sqrt((pos_a_x - pos_b_x) * (pos_a_x - pos_b_x) + (pos_a_y - pos_b_y) * (pos_a_y - pos_b_y));
		
		vec[0] = (pos_b_x - pos_a_x) / dist * size;
		vec[1] = (pos_b_y - pos_a_y) / dist * size;
		pos_a_x += vec[0];
		pos_a_y += vec[1];
		pos_b_x -= vec[0];
		pos_b_y -= vec[1];

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].id == id)
			{
				r_matrix_projection_screenf(matrix, grab[i].pos, pos_a_x, pos_a_y, 0);
				r_matrix_projection_screenf(matrix, &grab[i].pos[3], pos_b_x, pos_b_y, 0);
				grab[i].pos[2] = 0;
				grab[i].pos[5] = 0;
				if(snaps && grab[i].timer > 0.01)
				{
			/*		float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, up[3] = {0, 0, 1}, f, lenght;
					
					timer = grab[i].timer;
				//	seduce_background_particle_spawn(input, pos[0], pos[1], sin(*value * 2.0 * PI) * 0.4, cos(*value * 2.0 * PI) * 0.4, 0.0, S_PT_PRIMARY);
					seduce_widget_overlay_matrix(&overlay);
					f_matrixxzf(m, grab[i].pos, &grab[i].pos[3], up);
					lenght = sqrt((grab[i].pos[0] - grab[i].pos[3]) * (grab[i].pos[0] - grab[i].pos[3]) + (grab[i].pos[1] - grab[i].pos[4]) * (grab[i].pos[1] - grab[i].pos[4]));
					seduce_object_3d_draw(input, m[12], m[13], m[14], scale * 0.1, SUI_3D_OBJECT_SNAP_LARGE, timer, color);
					r_matrix_matrix_mult(&overlay, m);
					j = 0;
					for(f = 0; f < lenght + 0.01; f += lenght / 10.0)
					{
						if(j % 5 == 0)
							seduce_object_3d_draw(input, f, scale * 0.1, 0, scale * 0.1, SUI_3D_OBJECT_SNAP_LARGE, timer, color);
						else
							seduce_object_3d_draw(input, f, scale * 0.1, 0, scale * 0.1, SUI_3D_OBJECT_SNAP_SMALL, timer, color);
						j++;
					}
					j = 0;
					for(f = 0; f < lenght + 0.01; f += lenght / 16.0)
					{
						if(j % 4 == 0)
							seduce_object_3d_draw(input, f, scale * -0.1, 0, scale * 0.1, SUI_3D_OBJECT_SNAP_LARGE, timer, color);
						else
							seduce_object_3d_draw(input, f, scale * -0.1, 0, scale * 0.1, SUI_3D_OBJECT_SNAP_SMALL, timer, color);
						j++;
					}
					r_matrix_set(matrix);	*/			
				}
			}
		}

	//	if(f * dist >  size)
	/*	r_primitive_line_3d(pos_a_x - vec[0], pos_a_y - vec[1], 0,
				pos_a_x + (pos_b_x - pos_a_x) * f - size * (pos_b_x - pos_a_x) / dist,
				pos_a_y + (pos_b_y - pos_a_y) * f - size * (pos_b_y - pos_a_y) / dist, 0, 
				p[0], p[1], p[2], 1);
		r_primitive_line_3d(pos_b_x + vec[0], pos_b_y + vec[1], 0,
				pos_a_x + (pos_b_x - pos_a_x) * f + size * (pos_b_x - pos_a_x) / dist,
				pos_a_y + (pos_b_y - pos_a_y) * f + size * (pos_b_y - pos_a_y) / dist, 0, 
				p[0], p[1], p[2], 1);
		r_primitive_line_3d(
				pos_a_x + (pos_b_x - pos_a_x) * f,
				pos_a_y + (pos_b_y - pos_a_y) * f  + 0.03, 0,
				pos_a_x + (pos_b_x - pos_a_x) * f,
				pos_a_y + (pos_b_y - pos_a_y) * f  - 0.03, 0, 
				p[0], p[1], p[2], 1);
		r_primitive_line_flush();*/

		if(f * dist > size * 0.75)
			seduce_primitive_line_add_3d(object, pos_a_x - vec[0], pos_a_y - vec[1], 0,
					pos_a_x + (pos_b_x - pos_a_x) * f - size * 1.75 * (pos_b_x - pos_a_x) / dist,
					pos_a_y + (pos_b_y - pos_a_y) * f - size * 1.75 * (pos_b_y - pos_a_y) / dist, 0, 
					p[0], p[1], p[2], 1,
					p[0], p[1], p[2], 1);
		if((1.0 - f) * dist > size * 0.75)
			seduce_primitive_line_add_3d(object, pos_b_x + vec[0], pos_b_y + vec[1], 0,
					pos_a_x + (pos_b_x - pos_a_x) * f + size * 1.75 * (pos_b_x - pos_a_x) / dist,
					pos_a_y + (pos_b_y - pos_a_y) * f + size * 1.75 * (pos_b_y - pos_a_y) / dist, 0, 
					p[0], p[1], p[2], 1,
					p[0], p[1], p[2], 1);
		seduce_primitive_circle_add_3d(object,
					pos_a_x + f * (pos_b_x - pos_a_x), pos_a_y + f * (pos_b_y - pos_a_y), 0,
					0, 1, 0,
					0, 0, 1,
					size,
					0, 1,
					0, size * PI * 2.0,
					p[0], p[1], p[2], 1,
					p[0], p[1], p[2], 1);
		seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(!input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				grab[i].active = FALSE;
			if(grab[i].id == id && grab[i].active)
			{
				*value = min + (max - min) * seduce_widget_slider_line_value(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, scale, grab[i].pos[0], grab[i].pos[1], grab[i].pos[3], grab[i].pos[4]);
				if(!input->pointers[i].button[0])
				{
					grab[i].active = FALSE;
					return S_TIS_DONE;
				}

			/*	if((input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) *
					(input->pointers[i].click_pointer_x[0] - input->pointers[i].pointer_x) +
					(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) *
					(input->pointers[i].click_pointer_y[0] - input->pointers[i].pointer_y) >
					0.005 * 0.005)
				{
					grab[i].text_active = FALSE;
					seduce_text_deactivate(input->pointers[i].user_id);
					return S_TIS_DONE;
				}*/
			}else if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, NULL))
					{

						grab[i].init_value = *value;
						grab[i].active = TRUE;
						grab[i].text_active = FALSE;
				//		grab[i].text_active = TRUE;
						grab[i].id = id;
						grab[i].timer = 0.1;
				//		FIX ME
						*value += 0.1;
				//		*value = min + (max - min) * seduce_widget_slider_line_value(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, scale, grab[i].pos[0], grab[i].pos[1], grab[i].pos[3], grab[i].pos[4]);
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			float pos[3];
			if(id == seduce_element_selected_id(i, pos, NULL))
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);


				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
					axis = seduce_element_primary_axis(input, i);
					if(axis == -1)
						*value = grab[i].init_value;
					else
						*value = min + (max - min) * seduce_widget_slider_line_value(input, input->axis[axis].axis[0] * scale, input->axis[axis].axis[1] * scale, scale, grab[i].pos[0], grab[i].pos[1], grab[i].pos[3], grab[i].pos[4]);
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
					/*	r_matrix_projection_screenf(&grab[i].matrix, grab[i].pos, pos_a_x, pos_a_y, 0);
						r_matrix_projection_screenf(&grab[i].matrix, &grab[i].pos[3], pos_b_x, pos_b_y, 0);
						grab[i].pos[2] = 0;
						grab[i].pos[5] = 0;*/
						grab[i + pointer_count].init_value = *value;
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
				if(!a_button[i] && !grab[i + pointer_count].text_active)
				{
					grab[i + pointer_count].active = FALSE;
					return S_TIS_DONE;
				}
			}
		}
	}

	if(input->mode == BAM_MAIN && frame_id != input->frame_number)
	{
		frame_id = input->frame_number;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].active)
			{
				grab[i].timer += input->delta_time * 2.5;
				if(grab[i].timer > 1.0)		
					grab[i].timer = 1.0;
			}else
				grab[i].timer -= input->delta_time * 2.5;
		}
	}
	
	for(i = 0; i < (pointer_count + user_count) && (grab[i].id != id || (!grab[i].active && grab[i].timer < 0.0001)); i++);

	if(i < (pointer_count + user_count))
	{
		float f;
		if(input->mode == BAM_DRAW)
		{
			r_matrix_push(matrix);
			r_matrix_translate(matrix, 0, 0, grab[i].timer * size * 0.35);
		}

		f = (*value - min) / (max - min);
	/*	if(color == NULL)
		{
			if(!seduce_text_edit_float(input, id, NULL, value, pos_a_x + f * (pos_b_x - pos_a_x), pos_a_y + f * (pos_b_y - pos_a_y), 0.2, SEDUCE_T_SIZE * scale * 8.0, TRUE, NULL, NULL, 1, 1, 1, grab[i].timer, 1, 1, 1, grab[i].timer) && grab[i].text_active)
				grab[i].active = FALSE;
		}else
			if(!seduce_text_edit_float(input, id, NULL, value, pos_a_x + f * (pos_b_x - pos_a_x), pos_a_y + f * (pos_b_y - pos_a_y), 0.2, SEDUCE_T_SIZE * scale * 8.0, TRUE, NULL, NULL, color[0], color[1], color[2], grab[i].timer, color[0], color[1], color[2], grab[i].timer) && grab[i].text_active)
				grab[i].active = FALSE;*/
		if(input->mode == BAM_DRAW)
			r_matrix_pop(matrix);
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}


STypeInState seduce_widget_slider_square(BInputState *input, void *id, float *values, float pos_x, float pos_y, float size_x, float size_y, float size, float scale, float time, float *color)
{
	static SeduceLineObject *object = NULL, *snap = NULL;
	static boolean *a_button, *a_button_last;
	static SRadialGrab *grab = NULL;
	static uint frame_id = -1;
	int i, j, axis, pointer_count, user_count;
	char text_buf[128];
	RMatrix *matrix, overlay;

	size *= (time + time * (1.0 - time) * 3.0) * 0.5;

	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(grab == NULL)
	{
		uint i;
		grab = malloc((sizeof *grab) * (pointer_count + user_count));

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			grab[i].timer = 0;
			grab[i].id = NULL;
			grab[i].active = FALSE;
			grab[i].text_active = FALSE;
		}
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}

	if(input->mode == BAM_DRAW)
	{
		float a[2], b[2], height, vec[2];
		int count;
		float on[3] = {0.2, 0.6, 1.0}, *p = NULL, pos[6], f, timer = 0, aim[2], dist;
		uint snap;
		if(color == NULL)
			p = on;
		else
			p = color;
		//	if(object == NULL)
		//		object = seduce_primitive_line_object_allocate();
		if(p == NULL)
		{
			for(i = 0; i < input->pointer_count; i++)
				if(id == seduce_element_pointer_id(input, i, NULL))
					break;
			if(i < input->pointer_count)
				p = on;
			else
				for(i = 0; i < (pointer_count + user_count); i++)
					if(grab[i].id == id)
						p = on;
		}
		seduce_element_add_rectangle(input, id, 0, pos_x, pos_y, size_x, size_y);

		matrix = r_matrix_get();

		f = 0.0;
		vec[0] = 0;
		vec[1] = 0;

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].id == id)
			{
			//	r_matrix_projection_screenf(matrix, grab[i].pos, pos_a_x, pos_a_y, 0);
			//	r_matrix_projection_screenf(matrix, &grab[i].pos[3], pos_b_x, pos_b_y, 0);
				grab[i].pos[2] = 0;
				grab[i].pos[5] = 0;
	
			}
		}

		//	if(f * dist >  size)
		/*	r_primitive_line_3d(pos_a_x - vec[0], pos_a_y - vec[1], 0,
		pos_a_x + (pos_b_x - pos_a_x) * f - size * (pos_b_x - pos_a_x) / dist,
		pos_a_y + (pos_b_y - pos_a_y) * f - size * (pos_b_y - pos_a_y) / dist, 0, 
		p[0], p[1], p[2], 1);
		r_primitive_line_3d(pos_b_x + vec[0], pos_b_y + vec[1], 0,
		pos_a_x + (pos_b_x - pos_a_x) * f + size * (pos_b_x - pos_a_x) / dist,
		pos_a_y + (pos_b_y - pos_a_y) * f + size * (pos_b_y - pos_a_y) / dist, 0, 
		p[0], p[1], p[2], 1);
		r_primitive_line_3d(
		pos_a_x + (pos_b_x - pos_a_x) * f,
		pos_a_y + (pos_b_y - pos_a_y) * f  + 0.03, 0,
		pos_a_x + (pos_b_x - pos_a_x) * f,
		pos_a_y + (pos_b_y - pos_a_y) * f  - 0.03, 0, 
		p[0], p[1], p[2], 1);
		r_primitive_line_flush();*/

	/*	if(f * dist > size * 0.75)
			seduce_primitive_line_add_3d(object, pos_a_x - vec[0], pos_a_y - vec[1], 0,
										 pos_a_x + (pos_b_x - pos_a_x) * f - size * 1.75 * (pos_b_x - pos_a_x) / dist,
										 pos_a_y + (pos_b_y - pos_a_y) * f - size * 1.75 * (pos_b_y - pos_a_y) / dist, 0, 
										 p[0], p[1], p[2], 1,
										 p[0], p[1], p[2], 1);
		if((1.0 - f) * dist > size * 0.75)
			seduce_primitive_line_add_3d(object, pos_b_x + vec[0], pos_b_y + vec[1], 0,
										 pos_a_x + (pos_b_x - pos_a_x) * f + size * 1.75 * (pos_b_x - pos_a_x) / dist,
										 pos_a_y + (pos_b_y - pos_a_y) * f + size * 1.75 * (pos_b_y - pos_a_y) / dist, 0, 
										 p[0], p[1], p[2], 1,
										 p[0], p[1], p[2], 1);*/
		seduce_primitive_circle_add_3d(object,
									   pos_x + size * 1.5 + values[0] * (size_x - size * 3.0), 
									   pos_y + size * 1.5 + values[1] * (size_y - size * 3.0), 0,
									   0, 1, 0,
									   0, 0, 1,
									   size,
									   0, 1,
									   0, size * PI * 2.0,
									   p[0], p[1], p[2], 1,
									   p[0], p[1], p[2], 1);
		seduce_primitive_circle_add_3d(object,
									   pos_x + size * 1.5, 
									   pos_y + size * 1.5, 0,
									   0, 1, 0,
									   0, 0, 1,
									   size * 1.5,
									   0.5, 0.25,
									   0, size * PI * 2.0,
									   p[0], p[1], p[2], 1,
									   p[0], p[1], p[2], 1);
		seduce_primitive_circle_add_3d(object,
									   pos_x + size_x - size * 1.5, 
									   pos_y + size * 1.5, 0,
									   0, 1, 0,
									   0, 0, 1,
									   size * 1.5,
									   0.25, 0.25,
									   0, size * PI * 2.0,
									   p[0], p[1], p[2], 1,
									   p[0], p[1], p[2], 1);
		seduce_primitive_circle_add_3d(object,
									   pos_x + size * 1.5, 
									   pos_y + size_y - size * 1.5, 0,
									   0, 1, 0,
									   0, 0, 1,
									   size * 1.5,
									   0.75, 0.25,
									   0, size * PI * 2.0,
									   p[0], p[1], p[2], 1,
									   p[0], p[1], p[2], 1);
		seduce_primitive_circle_add_3d(object,
									   pos_x + size_x - size * 1.5, 
									   pos_y + size_y - size * 1.5, 0,
									   0, 1, 0,
									   0, 0, 1,
									   size * 1.5,
									   0.0, 0.25,
									   0, size * PI * 2.0,
									   p[0], p[1], p[2], 1,
									   p[0], p[1], p[2], 1);
		seduce_primitive_line_add_3d(object,
									 pos_x + size, 
									 pos_y + size_y, 0,
									 pos_x + size_x - size, 
									 pos_y + size_y, 0,
									 p[0], p[1], p[2], 1,
									 p[0], p[1], p[2], 1);
		seduce_primitive_line_add_3d(object,
									 pos_x + size, 
									 pos_y, 0,
									 pos_x + size_x - size, 
									 pos_y, 0,
									 p[0], p[1], p[2], 1,
									 p[0], p[1], p[2], 1);
		seduce_primitive_line_add_3d(object,
									 pos_x, 
									 pos_y + size_y - size, 0,
									 pos_x, 
									 pos_y + size, 0,
									 p[0], p[1], p[2], 1,
									 p[0], p[1], p[2], 1);
		seduce_primitive_line_add_3d(object,
									 pos_x + size_x, 
									 pos_y + size_y - size, 0,
									 pos_x + size_x, 
									 pos_y + size, 0,
									 p[0], p[1], p[2], 1,
									 p[0], p[1], p[2], 1);
		seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(!input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				grab[i].active = FALSE;
			if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, NULL))
					{

						grab[i].init_value = *values;
						grab[i].active = TRUE;
						grab[i].text_active = FALSE;
						//		grab[i].text_active = TRUE;
						grab[i].id = id;
						grab[i].timer = 0.1;
					//	*value = min + (max - min) * seduce_widget_slider_line_value(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, scale, grab[i].pos[0], grab[i].pos[1], grab[i].pos[3], grab[i].pos[4]);
					}
				}
			}
			if(grab[i].id == id && grab[i].active)
			{
				values[0] = (input->pointers[i].pointer_x - pos_x - size * 1.5) / (size_x - size * 3.0);
				values[1] = (input->pointers[i].pointer_y - pos_y - size * 1.5) / (size_y - size * 3.0);
				if(values[0] > 1.0)
					values[0] = 1.0;
				if(values[0] < 0.0)
					values[0] = 0.0;
				if(values[1] > 1.0)
					values[1] = 1.0;
				if(values[1] < 0.0)
					values[1] = 0.0;
				if(!input->pointers[i].button[0])
				{
					grab[i].active = FALSE;
					return S_TIS_DONE;
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			float pos[3];
			if(id == seduce_element_selected_id(i, pos, NULL))
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);


				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
				/*	axis = seduce_element_primary_axis(input, i);
					if(axis == -1)
						*value = grab[i].init_value;
					else
						*value = min + (max - min) * seduce_widget_slider_line_value(input, input->axis[axis].axis[0] * scale, input->axis[axis].axis[1] * scale, scale, grab[i].pos[0], grab[i].pos[1], grab[i].pos[3], grab[i].pos[4]);
				*/
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						/*	r_matrix_projection_screenf(&grab[i].matrix, grab[i].pos, pos_a_x, pos_a_y, 0);
						r_matrix_projection_screenf(&grab[i].matrix, &grab[i].pos[3], pos_b_x, pos_b_y, 0);
						grab[i].pos[2] = 0;
						grab[i].pos[5] = 0;*/
						grab[i + pointer_count].init_value = *values;
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
				if(!a_button[i] && !grab[i + pointer_count].text_active)
				{
					grab[i + pointer_count].active = FALSE;
					return S_TIS_DONE;
				}
			}
		}
	}

	if(input->mode == BAM_MAIN && frame_id != input->frame_number)
	{
		frame_id = input->frame_number;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].active)
			{
				grab[i].timer += input->delta_time * 2.5;
				if(grab[i].timer > 1.0)		
					grab[i].timer = 1.0;
			}else
				grab[i].timer -= input->delta_time * 2.5;
		}
	}

	for(i = 0; i < (pointer_count + user_count) && (grab[i].id != id || (!grab[i].active && grab[i].timer < 0.0001)); i++);

	if(i < (pointer_count + user_count))
	{
		float f;
		if(input->mode == BAM_DRAW)
		{
			r_matrix_push(matrix);
			r_matrix_translate(matrix, 0, 0, grab[i].timer * size * 0.35);
		}
		if(input->mode == BAM_DRAW)
			r_matrix_pop(matrix);
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}



void seduce_background_color_wheel(BInputState *input);

STypeInState seduce_widget_color_wheel_radial(BInputState *input, void *id, float *color, float pos_x, float pos_y, float size, float scale, float time)
{
	static SeduceLineObject *object = NULL;
	static boolean *a_button, *a_button_last;
	static SRadialGrab *grab = NULL;
	static uint frame_id = -1;
	int i, j, axis, pointer_count, user_count;
	size *= (time + time * (1.0 - time) * 3.0);

	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(grab == NULL)
	{
		uint i;
		grab = malloc((sizeof *grab) * (pointer_count + user_count));

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			grab[i].timer = 0;
			grab[i].id = NULL;
			grab[i].active = FALSE;
		}
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}

	if(input->mode == BAM_DRAW)
	{
		float on[3], *p = NULL, pos[3], f, timer = 0, aim[2], hsv[3];
		RMatrix *matrix, overlay;
		boolean drawn = FALSE; 
		if(object == NULL)
			object = seduce_primitive_line_object_allocate();
		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, size * 0.5);
		matrix = r_matrix_get();
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].id == id && grab[i].timer > 0.01)
			{
				f_transform3f(pos, matrix->matrix[matrix->current], pos_x, pos_y, 0);
				pos[0] /= -pos[2];
				pos[1] /= -pos[2];
				seduce_widget_overlay_matrix(&overlay);
				seduce_primitive_color_wheel_add_3d(object, grab[i].pos[0] * grab[i].timer + pos[0] * (1.0 - grab[i].timer), grab[i].pos[1] * grab[i].timer + pos[1] * (1.0 - grab[i].timer), 0, scale * grab[i].timer * 0.5 + size * 0.75);
				f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
				seduce_primitive_circle_add_3d(object,
							(grab[i].pos[0] + sin(hsv[0] * PI * 2.0) * 0.5 * scale * hsv[1]) * grab[i].timer + pos[0] * (1.0 - grab[i].timer), 
							(grab[i].pos[1] + cos(hsv[0] * PI * 2.0) * 0.5 * scale * hsv[1]) * grab[i].timer + pos[1] * (1.0 - grab[i].timer), 0,
							1, 0, 0,
							0, 0, 1,
							size * 0.3 + size * 0.2 * grab[i].timer,
							0, 1,
							0, 1,
							color[0], color[1], color[2], 1,
							color[0], color[1], color[2], 1);							
				seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);
				r_matrix_set(matrix);
				drawn = TRUE; 
			}
		}
		if(!drawn)
		{
			seduce_primitive_color_wheel_add_3d(object, pos_x, pos_y, 0, size * 0.5);
			seduce_primitive_circle_add_3d(object,
							pos_x, pos_y, 0,
							1, 0, 0,
							0, 0, 1,
							size * 0.3,
							0, 1,
							0, 1,
							color[0], color[1], color[2], 1,
							color[0], color[1], color[2], 1);
			seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);
		}
	}

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(grab[i].id == id && grab[i].active)
			{
				if(!input->pointers[i].button[0])
					grab[i].active = FALSE;
				else
				{
					float f;
					f = 2.0 * sqrt((input->pointers[i].pointer_x - grab[i].pos[0]) * 
								(input->pointers[i].pointer_x - grab[i].pos[0]) + 
								(input->pointers[i].pointer_y - grab[i].pos[1]) * 
								(input->pointers[i].pointer_y - grab[i].pos[1])) / scale;
					if(f > 1.0)
						f = 1.0;
					f_hsv_to_rgb(color, (atan2(-input->pointers[i].pointer_x + grab[i].pos[0], -input->pointers[i].pointer_y + grab[i].pos[1]) + PI) / (2.0 * PI), f, grab[i].init_value);
					{

						float hsv[3], error;
						f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
						
						if(grab[i].init_value - 0.01 > hsv[2] || grab[i].init_value + 0.01 < hsv[2])
						{
							error = 0;
						}
					}
				}
			}
			if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, NULL))
					{
						float hsv[3];
						f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
						grab[i].pos[0] = input->pointers[i].pointer_x - sin(hsv[0] * PI * 2.0) * 0.5 * scale * hsv[1];
						grab[i].pos[1] = input->pointers[i].pointer_y - cos(hsv[0] * PI * 2.0) * 0.5 * scale * hsv[1];
						if(hsv[2] < 0.01)
							grab[i].init_value = 1;
						else
							grab[i].init_value = hsv[2];
						grab[i].active = TRUE;
						grab[i].id = id;
						grab[i].timer = 0.1;
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			float pos[3];
			if(id == seduce_element_selected_id(i, pos, NULL))
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);
				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
					axis = seduce_element_primary_axis(input, i);
					if(axis != -1)
					{
						f_hsv_to_rgb(color, (atan2(-input->axis[axis].axis[0], -input->axis[axis].axis[1]) + PI) / (2.0 * PI), sqrt(input->axis[axis].axis[0] * input->axis[axis].axis[0] + input->axis[axis].axis[1] * input->axis[axis].axis[1]) * 2.0, 1/*grab[i + pointer_count].init_value*/);
					}
					if(!a_button[i])
					{					
						grab[i + pointer_count].active = FALSE;
						return S_TIS_DONE;
					}
				}
				if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						float hsv[3];
						f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
						grab[i + pointer_count].pos[0] = pos[0];
						grab[i + pointer_count].pos[1] = pos[1];
						if(hsv[2] < 0.01)
							grab[i + pointer_count].init_value = 1;
						else
							grab[i + pointer_count].init_value = hsv[2];
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
			}
		}
	}

	if(input->mode == BAM_MAIN && frame_id != input->frame_number)
	{
		frame_id = input->frame_number;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
		//	if(grab[i].id == id)
			{
				if(grab[i].active)
				{
					grab[i].timer += input->delta_time * 2.5;
					if(grab[i].timer > 1.0)		
						grab[i].timer = 1.0;
				}else
					grab[i].timer -= input->delta_time * 2.5;
			}
		}
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}


#define SEDUCE_TRIANGLE_SIN 0.86602540378443864676372317075294
#define SEDUCE_TRIANGLE_COS 0.5

STypeInState seduce_widget_color_triangle_radial(BInputState *input, void *id, float *color, float pos_x, float pos_y, float size, float scale, float time)
{
	static SeduceLineObject *object = NULL;
	static boolean *a_button, *a_button_last;
	static SRadialGrab *grab = NULL;
	static uint frame_id = -1;
	int i, j, axis, pointer_count, user_count;
	size *= (time + time * (1.0 - time) * 3.0);

	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(grab == NULL)
	{
		uint i;
		grab = malloc((sizeof *grab) * (pointer_count + user_count));

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			grab[i].timer = 0;
			grab[i].id = NULL;
			grab[i].active = FALSE;
		}
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}

	if(input->mode == BAM_DRAW)
	{
		float c[3], *p = NULL, pos[3], f, timer = 0, aim[2], hsv[3];
		RMatrix *matrix, overlay;
		boolean drawn = FALSE; 
		if(object == NULL)
			object = seduce_primitive_line_object_allocate();
		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, size * 0.5);
		matrix = r_matrix_get();
		f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
		f_hsv_to_rgb(c, hsv[0], 1, 1);
		f = size * 0.5;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].id == id && grab[i].timer > 0.01)
			{
				f_transform3f(pos, matrix->matrix[matrix->current], pos_x, pos_y, 0);
				pos[0] /= -pos[2];
				pos[1] /= -pos[2];
				seduce_widget_overlay_matrix(&overlay);
				f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
				pos[0] = grab[i].pos[0] * grab[i].timer + pos[0] * (1.0 - grab[i].timer);
				pos[1] = grab[i].pos[1] * grab[i].timer + pos[1] * (1.0 - grab[i].timer);
				scale = scale * grab[i].timer + size * (1.0 - grab[i].timer);
				f = scale * 0.8;						
				drawn = TRUE; 
				break;
			}
		}
		seduce_primitive_line_add_3d(object,
						pos[0] - f * SEDUCE_TRIANGLE_SIN, 
						pos[1] + f * SEDUCE_TRIANGLE_COS + f * 0.1, 0,
						pos[0] + f * SEDUCE_TRIANGLE_SIN, 
						pos[1] + f * SEDUCE_TRIANGLE_COS + f * 0.1, 0,
						1, 1, 1, 1,
						c[0], c[1], c[2], 1);
		seduce_primitive_line_add_3d(object,
						pos[0] + f * 0.1 * SEDUCE_TRIANGLE_SIN, 
						pos[1] - f - f * 0.1 * SEDUCE_TRIANGLE_COS, 0,
						pos[0] + f * SEDUCE_TRIANGLE_SIN + f * 0.1 * SEDUCE_TRIANGLE_SIN, 
						pos[1] + f * SEDUCE_TRIANGLE_COS - f * 0.1 * SEDUCE_TRIANGLE_COS, 0,
						0, 0, 0, 1,
						c[0], c[1], c[2], 1);	
		seduce_primitive_line_add_3d(object,
						pos[0] - f * 0.1 * SEDUCE_TRIANGLE_SIN, 
						pos[1] - f - f * 0.1 * SEDUCE_TRIANGLE_COS, 0,
						pos[0] - f * SEDUCE_TRIANGLE_SIN - f * 0.1 * SEDUCE_TRIANGLE_SIN, 
						pos[1] + f * SEDUCE_TRIANGLE_COS - f * 0.1 * SEDUCE_TRIANGLE_COS, 0,
						0, 0, 0, 1,
						1, 1, 1, 1);			
		seduce_primitive_circle_add_3d(object,
						pos[0], pos[1] - f, 0,
						0, 1, 0,
						0, 0, 1,
						f * 0.1,
						2.0 / 6.0, 2.0 / 6.0,
						1.0 / 6.0, 2.0 / 6.0,
						0, 0, 0, 1.0,
						0, 0, 0, 1.0);	
		seduce_primitive_circle_add_3d(object,
						pos[0] + f * SEDUCE_TRIANGLE_SIN, pos[1] + f * SEDUCE_TRIANGLE_COS, 0,
						0, 1, 0,
						0, 0, 1,
						f * 0.1,
						0.0 / 6.0, 2.0 / 6.0,
						1.0 / 6.0, 2.0 / 6.0,
						c[0], c[1], c[2], 1.0,
						c[0], c[1], c[2], 1.0);	
		seduce_primitive_circle_add_3d(object,
						pos[0] - f * SEDUCE_TRIANGLE_SIN, pos[1] + f * SEDUCE_TRIANGLE_COS, 0,
						0, 1, 0,
						0, 0, 1,
						f * 0.1,
						4.0 / 6.0, 2.0 / 6.0,
						1.0 / 6.0, 2.0 / 6.0,
						1.0, 1.0, 1.0, 1.0,
						1.0, 1.0, 1.0, 1.0);	
		seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);

		if(drawn)
			r_matrix_set(matrix);
	}

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(grab[i].id == id && grab[i].active)
			{
				float hsv[3];
				float x, y;

				f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
				hsv[2] = (input->pointers[i].pointer_y - grab[i].pos[1] + scale * 0.8) / (scale * 1.2);

				if(hsv[2] > 1.0)
					hsv[2] = 1.0;
				if(hsv[2] <= 0.0)
					hsv[2] = 0.0;
				else
					hsv[1] = (input->pointers[i].pointer_x - grab[i].pos[0]) / (2.0 * scale * 0.8 * SEDUCE_TRIANGLE_SIN * hsv[2]) + 0.5;
				if(hsv[1] > 1.0)
					hsv[1] = 1.0;
				if(hsv[1] < 0.0)
					hsv[1] = 0.0;		
//					hsv[1] = (input->pointers[i].pointer_y - grab[i].pos[1]) + scale * 0.4) / (scale * 0.8) / hsv[2];

				f_hsv_to_rgb(color, grab[i].init_value, hsv[1], hsv[2]);

				if(!input->pointers[i].button[0])
				{
					grab[i].active = FALSE;
					return S_TIS_DONE;
				}
			}
			if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, NULL))
					{
						float hsv[3];
						f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
						grab[i].pos[0] = input->pointers[i].pointer_x + scale * SEDUCE_TRIANGLE_SIN * (0.5 - hsv[1]) * hsv[2] / 0.4;

						grab[i].pos[1] = input->pointers[i].pointer_y + 0.8 * scale - 1.2 * scale * hsv[2];
						if(hsv[2] < 0.01)
							grab[i].init_value = 1;
						else
							grab[i].init_value = hsv[0];
						grab[i].active = TRUE;
						grab[i].id = id;
						grab[i].timer = 0.1;
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			float pos[3];
			if(id == seduce_element_selected_id(i, pos, NULL))
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);
				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
					axis = seduce_element_primary_axis(input, i);
					if(axis != -1)
					{
						f_hsv_to_rgb(color, (atan2(-input->axis[axis].axis[0], -input->axis[axis].axis[1]) + PI) / (2.0 * PI), sqrt(input->axis[axis].axis[0] * input->axis[axis].axis[0] + input->axis[axis].axis[1] * input->axis[axis].axis[1]) * 2.0, 1/*grab[i + pointer_count].init_value*/);
					}
					if(!a_button[i])
					{
						grab[i + pointer_count].active = FALSE;
						return S_TIS_DONE;
					}
				}
				if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						float hsv[3];
						f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
						grab[i + pointer_count].pos[0] = pos[0];
						grab[i + pointer_count].pos[1] = pos[1];
						if(hsv[2] < 0.01)
							grab[i + pointer_count].init_value = 1;
						else
							grab[i + pointer_count].init_value = hsv[2];
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
			}
		}
	}

	if(input->mode == BAM_MAIN && frame_id != input->frame_number)
	{
		frame_id = input->frame_number;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
		//	if(grab[i].id == id)
			{
				if(grab[i].active)
				{
					grab[i].timer += input->delta_time * 2.5;
					if(grab[i].timer > 1.0)		
						grab[i].timer = 1.0;
				}else
					grab[i].timer -= input->delta_time * 2.5;
			}
		}
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}


STypeInState seduce_widget_color_square_radial(BInputState *input, void *id, float *color, uint component, float pos_x, float pos_y, float size, float scale, float time)
{
	static SeduceLineObject *object = NULL;
	static boolean *a_button, *a_button_last;
	static SRadialGrab *grab = NULL;
	static uint frame_id = -1;
	int i, j, axis, pointer_count, user_count;
	size *= (time + time * (1.0 - time) * 3.0);

	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	if(grab == NULL)
	{
		uint i;
		grab = malloc((sizeof *grab) * (pointer_count + user_count));

		for(i = 0; i < (pointer_count + user_count); i++)
		{
			grab[i].timer = 0;
			grab[i].id = NULL;
			grab[i].active = FALSE;
		}
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}

	if(input->mode == BAM_DRAW)
	{
		float c[3 * 4], *p = NULL, pos[3], f, timer = 0, aim[2];
		RMatrix *matrix, overlay;
		boolean drawn = FALSE; 
		if(object == NULL)
			object = seduce_primitive_line_object_allocate();
		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, size * 0.5);
		matrix = r_matrix_get();
		c[0] = c[3] = c[6] = c[9] = color[0];
		c[1] = c[4] = c[7] = c[10] = color[1];
		c[2] = c[5] = c[8] = c[11] = color[2];
		i = (component + 2) % 3;
		c[i] = c[3 + i] = 0;
		c[6 + i] = c[9 + i] = 1;
		i = (component + 1) % 3;
		c[i] = c[6 + i] = 0;
		c[3 + i] = c[9 + i] = 1;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].id == id && grab[i].timer > 0.01)
			{
				f_transform3f(pos, matrix->matrix[matrix->current], pos_x, pos_y, 0);
				pos[0] /= -pos[2];
				pos[1] /= -pos[2];
				seduce_widget_overlay_matrix(&overlay);
				pos[0] = grab[i].pos[0] * grab[i].timer + pos[0] * (1.0 - grab[i].timer);
				pos[1] = grab[i].pos[1] * grab[i].timer + pos[1] * (1.0 - grab[i].timer);
				scale = scale * grab[i].timer + size * (1.0 - grab[i].timer);
				f = scale * 0.8;
				seduce_primitive_line_add_3d(object,
								pos[0] - scale * 0.4, pos[1] - scale * 0.5, 0,
								pos[0] + scale * 0.4, pos[1] - scale * 0.5, 0,
								c[0], c[1], c[2], 1,
								c[3], c[4], c[5], 1);
				seduce_primitive_line_add_3d(object,
								pos[0] - scale * 0.4, pos[1] + scale * 0.5, 0,
								pos[0] + scale * 0.4, pos[1] + scale * 0.5, 0,
								c[6], c[7], c[8], 1,
								c[9], c[10], c[11], 1);
				seduce_primitive_line_add_3d(object,
								pos[0] - scale * 0.5, pos[1] - scale * 0.4, 0,
								pos[0] - scale * 0.5, pos[1] + scale * 0.4, 0,
								c[0], c[1], c[2], 1,
								c[6], c[7], c[8], 1);
				seduce_primitive_line_add_3d(object,
								pos[0] + scale * 0.5, pos[1] - scale * 0.4, 0,
								pos[0] + scale * 0.5, pos[1] + scale * 0.4, 0,
								c[3], c[4], c[5], 1,
								c[9], c[10], c[11], 1);		
				seduce_primitive_circle_add_3d(object,
							pos[0] - scale * 0.4, pos[1] - scale * 0.4, 0,
							0, 1, 0,
							0, 0, 1,
							scale * 0.1,
							0.5, 0.25,
							0.5, 0.25,
							c[0], c[1], c[2], 1,
							c[0], c[1], c[2], 1);	
							
				seduce_primitive_circle_add_3d(object,
							pos[0] + scale * 0.4, pos[1] - scale * 0.4, 0,
							0, 1, 0,
							0, 0, 1,
							scale * 0.1,
							0.25, 0.25,
							0.25, 0.25,
							c[3], c[4], c[5], 1,
							c[3], c[4], c[5], 1);	
							
				seduce_primitive_circle_add_3d(object,
							pos[0] - scale * 0.4, pos[1] + scale * 0.4, 0,
							0, 1, 0,
							0, 0, 1,
							scale * 0.1,
							0.75, 0.25,
							0.75, 0.25,
							c[6], c[7], c[8], 1,
							c[6], c[7], c[8], 1);
				seduce_primitive_circle_add_3d(object,
							pos[0] + scale * 0.4, pos[1] + scale * 0.4, 0,
							0, 1, 0,
							0, 0, 1,
							scale * 0.1,
							0, 0.25,
							0, 0.25,
							c[9], c[10], c[11], 1,
							c[9], c[10], c[11], 1);								
				seduce_primitive_circle_add_3d(object,
							pos[0] + scale * 0.8 * (color[(component + 1) % 3] - 0.5), pos[1] + scale * 0.8 * (color[(component + 2) % 3] - 0.5), 0,
							0, 1, 0,
							0, 0, 1,
							size * 0.4,
							0, 1,
							0, 1,
							color[0], color[1], color[2], 1,
							color[0], color[1], color[2], 1);													


				seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);
				r_matrix_set(matrix);
				drawn = TRUE; 
			}
		}
		if(!drawn)
		{
			f = size * 0.8;
			seduce_primitive_line_add_3d(object,
							pos[0] - size * 0.4, pos[1] - size * 0.5, 0,
							pos[0] + size * 0.4, pos[1] - size * 0.5, 0,
							c[0], c[1], c[2], 1,
							c[3], c[4], c[5], 1);
			seduce_primitive_line_add_3d(object,
							pos[0] - size * 0.4, pos[1] + size * 0.5, 0,
							pos[0] + size * 0.4, pos[1] + size * 0.5, 0,
							c[6], c[7], c[8], 1,
							c[9], c[10], c[11], 1);
			seduce_primitive_line_add_3d(object,
							pos[0] - size * 0.5, pos[1] - size * 0.4, 0,
							pos[0] - size * 0.5, pos[1] + size * 0.4, 0,
							c[0], c[1], c[2], 1,
							c[6], c[7], c[8], 1);
			seduce_primitive_line_add_3d(object,
							pos[0] + size * 0.5, pos[1] - size * 0.4, 0,
							pos[0] + size * 0.5, pos[1] + size * 0.4, 0,
							c[3], c[4], c[5], 1,
							c[9], c[10], c[11], 1);	
			seduce_primitive_circle_add_3d(object,
							pos[0] - size * 0.4, pos[1] - size * 0.4, 0,
							0, 1, 0,
							0, 0, 1,
							size * 0.1,
							0.5, 0.25,
							0.5, 0.25,
							c[0], c[1], c[2], 1,
							c[0], c[1], c[2], 1);	
							
			seduce_primitive_circle_add_3d(object,
							pos[0] + size * 0.4, pos[1] - size * 0.4, 0,
							0, 1, 0,
							0, 0, 1,
							size * 0.1,
							0.25, 0.25,
							0.25, 0.25,
							c[3], c[4], c[5], 1,
							c[3], c[4], c[5], 1);	
							
			seduce_primitive_circle_add_3d(object,
							pos[0] - size * 0.4, pos[1] + size * 0.4, 0,
							0, 1, 0,
							0, 0, 1,
							size * 0.1,
							0.75, 0.25,
							0.75, 0.25,
							c[6], c[7], c[8], 1,
							c[6], c[7], c[8], 1);
			seduce_primitive_circle_add_3d(object,
							pos[0] + size * 0.4, pos[1] + size * 0.4, 0,
							0, 1, 0,
							0, 0, 1,
							size * 0.1,
							0, 0.25,
							0, 0.25,
							c[9], c[10], c[11], 1,
							c[9], c[10], c[11], 1);
			seduce_primitive_circle_add_3d(object,
							pos[0], pos[1], 0,
							0, 1, 0,
							0, 0, 1,
							size * 0.4,
							0, 1,
							0, 1,
							color[0], color[1], color[2], 1,
							color[0], color[1], color[2], 1);	

			seduce_primitive_line_draw(object, 1.0, 1.0, 1.0, 1.0);
		}
	}

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(grab[i].id == id && grab[i].active)
			{
				color[(component + 1) % 3] = (input->pointers[i].pointer_x - grab[i].pos[0] + scale * 0.4) / (scale * 0.8);
				if(color[(component + 1) % 3] > 1.0)
					color[(component + 1) % 3] = 1.0;
				if(color[(component + 1) % 3] < 0.0)
					color[(component + 1) % 3] = 0.0;
				color[(component + 2) % 3] = (input->pointers[i].pointer_y - grab[i].pos[1] + scale * 0.4) / (scale * 0.8);
				if(color[(component + 2) % 3] > 1.0)
					color[(component + 2) % 3] = 1.0;
				if(color[(component + 2) % 3] < 0.0)
					color[(component + 2) % 3] = 0.0;
				if(!input->pointers[i].button[0])
				{
					grab[i].active = FALSE;
					return S_TIS_DONE;
				}
			}
			if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, NULL))
					{
						grab[i].pos[0] = input->pointers[i].pointer_x + 0.8 * scale * (0.5 - color[(component + 1) % 3]);
						grab[i].pos[1] = input->pointers[i].pointer_y + 0.8 * scale * (0.5 - color[(component + 2) % 3]);
						grab[i].active = TRUE;
						grab[i].id = id;
						grab[i].timer = 0.1;
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			float pos[3];
			if(id == seduce_element_selected_id(i, pos, NULL))
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);
				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
					axis = seduce_element_primary_axis(input, i);
					if(axis != -1)
						f_hsv_to_rgb(color, (atan2(-input->axis[axis].axis[0], -input->axis[axis].axis[1]) + PI) / (2.0 * PI), sqrt(input->axis[axis].axis[0] * input->axis[axis].axis[0] + input->axis[axis].axis[1] * input->axis[axis].axis[1]) * 2.0, 1/*grab[i + pointer_count].init_value*/);
					if(!a_button[i])
					{
						grab[i + pointer_count].active = FALSE;
						return S_TIS_DONE;
					}
				}
				if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						float hsv[3];
						f_rgb_to_hsv(hsv, color[0], color[1], color[2]);
						grab[i + pointer_count].pos[0] = pos[0];
						grab[i + pointer_count].pos[1] = pos[1];
						if(hsv[2] < 0.01)
							grab[i + pointer_count].init_value = 1;
						else
							grab[i + pointer_count].init_value = hsv[2];
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
			}
		}
	}

	if(input->mode == BAM_MAIN && frame_id != input->frame_number)
	{
		frame_id = input->frame_number;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
		//	if(grab[i].id == id)
			{
				if(grab[i].active)
				{
					grab[i].timer += input->delta_time * 2.5;
					if(grab[i].timer > 1.0)		
						grab[i].timer = 1.0;
				}else
					grab[i].timer -= input->delta_time * 2.5;
			}
		}
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}


extern uint seduce_popup_simple(BInputState *input, uint user_id, float pos_x, float pos_y, char **lables, uint element_count, float *time, boolean active, float red, float green, float blue, float red_active, float green_active, float blue_active);

boolean seduce_widget_select_radial_old(BInputState *input, void *id, uint *selected, char **lables, uint element_count, SPopUpType type, float pos_x, float pos_y, float size, float scale, float time, boolean release_only)
{
	static boolean *a_button, *a_button_last;
	static SRadialGrab *grab = NULL;
	static uint frame_id = -1;
	int i, j, axis, output, pointer_count, user_count;
	size *= (time + time * (1.0 - time) * 3.0);

	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	pointer_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	if(grab == NULL)
	{
		uint i;
		grab = malloc((sizeof *grab) * (user_count + pointer_count));

		for(i = 0; i < user_count; i++)
		{
			grab[i].timer = 0;
			grab[i].id = NULL;
			grab[i].active = FALSE;
		}
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}

	if(input->mode == BAM_DRAW)
	{
		float on[3], *p = NULL, pos[3], f, timer = 0, aim[2], hsv[3];
		RMatrix *matrix;
		boolean drawn = FALSE; 

		pos[0] = pos_x;
		pos[1] = pos_y;
		pos[2] = 0;
		seduce_element_add_point(input, id, 0, pos, size * 0.5);
	
		matrix = r_matrix_get();
		r_matrix_push(matrix);
		r_matrix_translate(matrix, pos_x, pos_y, 0);
		r_matrix_rotate(matrix, 360.0 - time * 360.0, 0, 0, 1);
		r_matrix_scale(matrix, time, time, time);
		seduce_object_3d_draw(input, 0, 0, 0, size, SUI_3D_OBJECT_POPUPBUTTON, timer, NULL);
		r_matrix_pop(matrix);
	}

	for(i = 0; i < (user_count + pointer_count); i++)
		if(grab[i].id == id)
			output = seduce_popup_simple(input, 0, grab[i].pos[0], grab[i].pos[1], lables, element_count, &grab[i].timer, grab[i].active, 0, 0, 0, 0, 0, 0);

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < user_count; i++)
		{
			if(grab[i].id == id && grab[i].active)
			{
				if(output != -1 && !release_only)
					*selected = output;
				if(!input->pointers[i].button[0])
				{
					if(output != -1)
						*selected = output;
					grab[i].active = FALSE;
					return TRUE;
				}
			}
		}
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			{
				if(id == seduce_element_pointer_id(input, i, NULL))
				{
					grab[i].pos[0] = input->pointers[i].pointer_x;
					grab[i].pos[1] = input->pointers[i].pointer_y;
					grab[i].active = TRUE;
					grab[i].id = id;
					grab[i].timer = 0.1;
				}
			}
		}
	}		
	return FALSE;
}


typedef struct{
	void *id;
	SUIPUElement *elements;
	uint element_count;
	uint output;
}SSelectParams;

void seduce_widget_select_radial_func(BInputState *input, float time, void *user)
{
	SSelectParams *params;
	params = user;
	params->output = seduce_popup(input, params->id, params->elements, params->element_count, time);
}

STypeInState seduce_widget_select_radial(BInputState *input, void *id, uint *selected, char **lables, uint element_count, SPopUpType type, float pos_x, float pos_y, float size, float scale, float time, boolean release_only)
{
	SSelectParams params;
	SUIPUElement elements[16];
	STypeInState output;
	uint i;
	if(element_count <= 16)
		params.elements = elements;
	else
		params.elements = malloc((sizeof *params.elements) * element_count * 20);
	for(i = 0; i < element_count; i++)
	{
		params.elements[i].text = lables[i];
		params.elements[i].type = type;
		params.elements[i].data.angle[0] = ((float)i + 0.5) * 360.0 / (float)element_count;
		params.elements[i].data.angle[1] = ((float)i + 1.5) * 360.0 / (float)element_count;
	}
	params.id = id;
	params.output = *selected;
	params.element_count = element_count;
	output = seduce_popup_detect_icon(input, id, 22, pos_x, pos_y, size, time, seduce_widget_select_radial_func, &params, TRUE, NULL);
	if(output != S_TIS_IDLE && params.output < element_count && (!release_only || output == S_TIS_DONE))
		*selected = params.output;
	if(element_count > 16)
		free(params.elements);
	params.elements = NULL;
	if(output == S_TIS_DONE && *selected >= element_count)
		return S_TIS_IDLE;
	return output;
}