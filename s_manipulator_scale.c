#include <math.h>
#include <stdlib.h>
#include "seduce.h"
#include "s_draw_3d.h"

extern void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float scale, uint id, float fade,  float *color);
extern void seduce_widget_overlay_matrix(RMatrix *matrix);

typedef struct{
	void *id;
	boolean active;
	uint part;
	float grab_pos[3];
	float size[3];
	RMatrix matrix;
}SeduceManipulatorGrab;

STypeInState seduce_manipulator_radius(BInputState *input, RMatrix *m, float *pos, float *radius, void *id, float time)
{
	static boolean *a_button, *a_button_last;
	static SeduceManipulatorGrab *grab = NULL;
	static uint frame_id = -1;
	RMatrix *reset;
	float tmp[3], f, dist, matrix[16];
	uint i, count, active_pointer = -1, pointer_count, user_count;


	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);

	if(grab == NULL)
	{
		count = betray_support_functionality(B_SF_USER_COUNT_MAX);
		a_button = malloc((sizeof *a_button) * count);
		a_button_last = malloc((sizeof *a_button_last) * count);
		for(i = 0; i < count; i++)
			a_button[i] = a_button_last[i] = FALSE;
		count += betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
		{
			grab[i].id = NULL;
			grab[i].active = FALSE;
			r_matrix_identity(&grab[i].matrix);
		}
	}


	if(input->mode == BAM_DRAW)
	{
		reset = r_matrix_get();
		if(m == NULL)
			m = reset;
		r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
		dist = -tmp[2];
		if(tmp[2] < 0)
		{
			float color[4] = {0.2, 0.6, 1.0, 1.0}, expand = 0, a[3] = {0.483, 0.129, 0}, b[3] = {0.483, -0.129, 0};
			float size = 1;
			boolean active = FALSE;
			RMatrix draw_matrix;

			time = 1.0 - time;

			seduce_widget_overlay_matrix(&draw_matrix);
			size = *radius;
			for(i = 0; i < input->pointer_count; i++)
			{
				if(grab[i].id == id)
				{
					grab[i].matrix = *m;
					size = grab[i].size[0];
				}if((grab[i].id == id && grab[i].active) || id == seduce_element_pointer_id(input, i, NULL))
				{
					grab[i].matrix = *m;
					active = TRUE;
				}
			}
			for(i = 0; i < input->user_count; i++)
			{
				if(id == seduce_element_selected_id(i, NULL, NULL))
				{
					
					grab[i + pointer_count].matrix = *m;
					active = TRUE;
				}
			}
			r_matrix_translate(&draw_matrix, tmp[0], tmp[1], 0);
			expand = ((*radius) - size) / size + time * time * 10.0;
			if(expand < -1.0)
				expand = -1.0;
			r_matrix_scale(&draw_matrix, size * 2.0 / dist, size * 2.0 / dist, size * 2.0 / dist);
			for(i = 0; i < 12; i++)
			{
				r_matrix_push(&draw_matrix);
				if(expand < 0.0)
				{
					r_matrix_rotate(&draw_matrix, (float)i * 360.0 / 12.0 + expand * 45.0, 0, 0, 1);
					r_matrix_rotate(&draw_matrix, expand * -90.0, 1, 0, 0);
				}else
					r_matrix_rotate(&draw_matrix, (float)i * 360.0 / 12.0/* - expand * 20.0*/, 0, 0, 1);
				if(active)
					seduce_object_3d_draw(input, expand * 0.5, 0, 0, 1, SUI_3D_OBJECT_RADIUS_HANDLE, 1, color);
				else
					seduce_object_3d_draw(input, expand * 0.5, 0, 0, 1, SUI_3D_OBJECT_RADIUS_HANDLE, 1, NULL);
				seduce_element_add_line(input, id, 0, a, b, 0.02);
				r_matrix_pop(&draw_matrix);
			}
			r_matrix_set(reset);

		}
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(!input->pointers[i].button[0] && grab[i].active)
			{
				grab[i].active = FALSE;
				return S_TIS_DONE;
			}
			if(grab[i].id == id && grab[i].active)
			{	
				r_matrix_projection_vertexf(&grab[i].matrix, tmp, pos, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
				tmp[0] = tmp[0] - pos[0];
				tmp[1] = tmp[1] - pos[1];
				tmp[2] = tmp[2] - pos[2];
				*radius = f_length3f(tmp) / grab[i].grab_pos[0];
			}else if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, NULL))
					{
						grab[i].active = TRUE;
						grab[i].id = id;
						r_matrix_projection_vertexf(&grab[i].matrix, tmp, pos, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
						tmp[0] = tmp[0] - pos[0];
						tmp[1] = tmp[1] - pos[1];
						tmp[2] = tmp[2] - pos[2];
						grab[i].grab_pos[0] = f_length3f(tmp) / *radius;
						grab[i].size[0] = *radius;
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
				if(!a_button[i] && grab[i + pointer_count].active)
				{
					grab[i + pointer_count].active = FALSE;
					return S_TIS_DONE;
				}


				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
					uint axis;
					axis = seduce_element_primary_axis(input, i);
					if(axis != -1)
						*radius *= 1.0 + input->axis[axis].axis[0] * input->delta_time * 3.0;
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].size[0] = *radius;
					}		
				}
			}
		}
	}

	if(input->mode == BAM_MAIN)
	{
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].size[0] > *radius)
				{
					grab[i].size[0] -= input->delta_time * 0.8 * (*radius + grab[i].size[0]);
					if(grab[i].size[0] < *radius)
						grab[i].size[0] = *radius;
				}	
				if(grab[i].size[0] < *radius)
				{
					grab[i].size[0] += input->delta_time * 0.4 * (*radius + grab[i].size[0]);
					if(grab[i].size[0] > *radius)
						grab[i].size[0] = *radius;
				}
			}
		}
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}


STypeInState seduce_manipulator_scale(BInputState *input, RMatrix *m, float *pos, float *size, void *id, float *snap, boolean snap_active, boolean x, boolean y, boolean z, float scale, float time)
{
	static boolean *a_button, *a_button_last;
	static SeduceManipulatorGrab *grab = NULL;
	static uint frame_id = -1;
	float tmp[3], tmp2[3], f, matrix[16];
	uint i, j,count, active_pointer = -1, pointer_count, user_count;


	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);

	if(grab == NULL)
	{
		count = betray_support_functionality(B_SF_USER_COUNT_MAX);
		a_button = malloc((sizeof *a_button) * count);
		a_button_last = malloc((sizeof *a_button_last) * count);
		for(i = 0; i < count; i++)
			a_button[i] = a_button_last[i] = FALSE;
		count += betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
		{
			grab[i].id = NULL;
			grab[i].active = FALSE;
			r_matrix_identity(&grab[i].matrix);
		}
	}

	if(input->mode == BAM_DRAW)
	{
		if(m == NULL)
			m = r_matrix_get();
		
		r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
		if(tmp[2] < 0)
		{
			float flip[3], vector[3], a[3] = {0.0, 0.0, 0.0}, color[11] = {0, 0.3, 1, 0, 0, 0.8, 1.0, 0, 0.8, 0.9, 1.0}, t, move[3] = {1, 1, 1};
			uint part = -1;
			boolean parts[7] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
			RMatrix *reset;
			reset = r_matrix_get();
			t = 1.0 - time;
			t = t * t * -2.0 + t; 
			r_matrix_projection_worldf(m, flip, 0, 0, 0);
			for(i = 0; i < 3; i++)
			{
				vector[i] = flip[i] - pos[i];
				if(vector[i] > 0.0)
					flip[i] = -1.0;
				else
					flip[i] = 1.0;
			}
			f_normalize3f(vector);
			if(0.15 > vector[1] * vector[1] + vector[2] * vector[2])
				x = FALSE;
			if(0.15 > vector[2] * vector[2] + vector[0] * vector[0])
				y = FALSE;
			if(0.15 > vector[0] * vector[0] + vector[1] * vector[1])
				z = FALSE;
			move[0] = scale * 0.12 * tmp[2] + t;
			move[1] = scale * 0.12 * tmp[2] + t;
			move[2] = scale * 0.12 * tmp[2] + t;
			for(i = 0; i < user_count + pointer_count; i++)
			{
				if(grab[i].id == id && grab[i].active)
				{
					move[0] *= size[0] / grab[i].size[0];
					move[1] *= size[1] / grab[i].size[1];
					move[2] *= size[2] / grab[i].size[2];
					parts[grab[i].part] = TRUE;
					grab[i].matrix = *m;
				}
				else if(i < pointer_count && id == seduce_element_pointer_id(input, i, &part))
				{
					parts[part] = TRUE;
					grab[i].matrix = *m;
				}
			}
			for(i = 0; i < input->user_count; i++)
			{
				if(id == seduce_element_selected_id(i, NULL, &part))
				{
					parts[part] = TRUE;		
					grab[i + pointer_count].matrix = *m;
				}
			}
			r_matrix_set(m);
			r_matrix_push(m);
			r_matrix_translate(m, pos[0], pos[1], pos[2]);
			r_matrix_normalize_scale(m);
			r_matrix_scale(m, flip[0], flip[1], flip[2]);
			if(x && y && z)
			{
				r_matrix_push(m);
				a[0] = move[0];
				a[1] = move[1];
				a[2] = move[2];
				if(parts[6])
					seduce_object_3d_draw(input, move[0], move[1], move[2], scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_3D, 1, &color[8]);
				else
					seduce_object_3d_draw(input, move[0], move[1], move[2], scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_3D, 1, NULL);
				seduce_element_add_point(input, id, 6, a, scale * 0.015 * -tmp[2]);
				r_matrix_pop(m);
			}

			if(x)
			{
				r_matrix_push(m);
				a[0] = move[0];
				a[1] = 0;
				a[2] = 0;
				if(parts[0])
					seduce_object_3d_draw(input, a[0], 0, 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_AXIS, 1, &color[2]);
				else
					seduce_object_3d_draw(input, a[0], 0, 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_AXIS, 1, NULL);
				seduce_element_add_point(input, id, 0, a, scale * 0.015 * -tmp[2]);
				r_matrix_pop(m);
			}
			if(x && y)
			{
				r_matrix_push(m);
				a[0] = move[0];
				a[1] = move[1];
				a[2] = 0;
				if(parts[5])
					seduce_object_3d_draw(input, move[0], move[1], 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_PLAIN, 1, &color[5]);
				else
					seduce_object_3d_draw(input, move[0], move[1], 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_PLAIN, 1, NULL);
				seduce_element_add_point(input, id, 5, a, scale * 0.015 * -tmp[2]);
				r_matrix_pop(m);
			}

			r_matrix_rotate(m, 90, 0, 0, 1);
			if(y)
			{
				r_matrix_push(m);
				a[0] = move[1];
				a[1] = 0;
				a[2] = 0;
				if(parts[1])
					seduce_object_3d_draw(input, a[0], 0, 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_AXIS, 1, &color[1]);
				else
					seduce_object_3d_draw(input, a[0], 0, 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_AXIS, 1, NULL);
				seduce_element_add_point(input, id, 1, a, scale * 0.015 * -tmp[2]);
				r_matrix_pop(m);
			}
			if(y && z)
			{
				r_matrix_push(m);
			
				r_matrix_rotate(m, 90, 1, 0, 0);
				a[0] = move[1];
				a[1] = move[2];
				a[2] = 0;
				if(parts[3])
					seduce_object_3d_draw(input, move[1], move[2], 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_PLAIN, 1, &color[4]);
				else
					seduce_object_3d_draw(input, move[1], move[2], 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_PLAIN, 1, NULL);
				seduce_element_add_point(input, id, 3, a, scale * 0.015 * -tmp[2]);
				r_matrix_pop(m);
			}

			if(z)
			{
				r_matrix_rotate(m, -90, 0, 1, 0);
				r_matrix_push(m);
				a[0] = move[2];
				a[1] = 0;
				a[2] = 0;
				if(parts[2])
					seduce_object_3d_draw(input, a[0], 0, 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_AXIS, 1, &color[0]);
				else
					seduce_object_3d_draw(input, a[0], 0, 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_AXIS, 1, NULL);
				seduce_element_add_point(input, id, 2, a, scale * 0.015 * -tmp[2]);
				r_matrix_pop(m);
			}
			if(x && z)
			{
				r_matrix_push(m);
				r_matrix_rotate(m, -90, 0, 0, 1);
				a[0] = move[0];
				a[1] = move[2];
				a[2] = 0;
				if(parts[4])
					seduce_object_3d_draw(input, move[0], move[2], 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_PLAIN, 1, &color[6]);
				else
					seduce_object_3d_draw(input, move[0], move[2], 0, scale * 0.015 * -tmp[2], SUI_3D_OBJECT_SCALE_PLAIN, 1, NULL);
				seduce_element_add_point(input, id, 4, a, scale * 0.015 * -tmp[2]);
				r_matrix_pop(m);
			}

			r_matrix_pop(m);
			r_matrix_set(reset);
		}
	}

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(!input->pointers[i].button[0] && grab[i].active)
			{
				grab[i].active = FALSE;
				return S_TIS_DONE;
			}
			if(grab[i].id == id && grab[i].active)
			{
				if(grab[i].part < 3)
				{
					if(snap != NULL && snap_active)
						size[grab[i].part] = snap[grab[i].part];
					if(r_matrix_projection_axisf(&grab[i].matrix, tmp, pos, grab[i].part, input->pointers[i].pointer_x, input->pointers[i].pointer_y))
					{
						size[grab[i].part] = grab[i].size[grab[i].part] * tmp[grab[i].part] / grab[i].grab_pos[grab[i].part];
					}
				}else
				{
					float vector[3] = {-1, -1, -1};

					if(snap != NULL && snap_active)
					{
						size[(grab[i].part + 1) % 3] = snap[(grab[i].part + 1) % 3];
						size[(grab[i].part + 2) % 3] = snap[(grab[i].part + 2) % 3];
					}
					r_matrix_projection_worldf(&grab[i].matrix, vector, 0, 0, 0);
					for(j = 0; j < 3; j++)
					{
						vector[j] -= pos[j];
						if(vector[j] > 0)
							vector[j] = -1.0;
						else
							vector[j] = 1.0;
					}

					if(grab[i].part < 6)
						vector[grab[i].part % 3] = 0.0;
					r_matrix_projection_vectorf(&grab[i].matrix, tmp, pos, vector, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
					tmp[0] = tmp[0] - pos[0];
					tmp[1] = tmp[1] - pos[1];
					tmp[2] = tmp[2] - pos[2];
					f = sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]);
					if(vector[0] > 0.1 || vector[0] < -0.1)
						size[0] = vector[0] * grab[i].size[0] * f / grab[i].grab_pos[0];
					if(vector[1] > 0.1 || vector[1] < -0.1)
						size[1] = vector[1] * grab[i].size[1] * f / grab[i].grab_pos[1];
					if(vector[2] > 0.1 || vector[2] < -0.1)
						size[2] = vector[2] * grab[i].size[2] * f / grab[i].grab_pos[2];
					for(i = 0; i < 3; i++)
						if(size[i] < 0.0)
							size[i] = -size[i];
				}
			}else if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, &grab[i].part))
					{
						grab[i].active = TRUE;
						grab[i].id = id;
						grab[i].size[0] = size[0];
						grab[i].size[1] = size[1];
						grab[i].size[2] = size[2];
						grab[i].grab_pos[0] = grab[i].grab_pos[1] = grab[i].grab_pos[2] = 0,0;
						if(grab[i].part < 3)
						{
							r_matrix_projection_axisf(&grab[i].matrix, tmp, pos, grab[i].part, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
							grab[i].grab_pos[0] = tmp[0];
							grab[i].grab_pos[1] = tmp[1];
							grab[i].grab_pos[2] = tmp[2];
						}else
						{
							float vector[3] = {-1, -1, -1};
							r_matrix_projection_worldf(&grab[i].matrix, vector, 0, 0, 0);
							for(j = 0; j < 3; j++)
							{
								vector[j] -= pos[j];
								if(vector[j] > 0)
									vector[j] = -1.0;
								else
									vector[j] = 1.0;
							}
							if(grab[i].part < 6)
								vector[grab[i].part % 3] = 0.0;
							if(r_matrix_projection_vectorf(&grab[i].matrix, tmp, pos, vector, input->pointers[i].pointer_x, input->pointers[i].pointer_y))
							{
								tmp[0] = tmp[0] - pos[0];
								tmp[1] = tmp[1] - pos[1];
								tmp[2] = tmp[2] - pos[2];
								f = sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]);
								grab[i].grab_pos[0] = f * vector[0];
								grab[i].grab_pos[1] = f * vector[1];
								grab[i].grab_pos[2] = f * vector[2];
							}else
								grab[i].id = NULL;
						}
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			uint part;
			if(id == seduce_element_selected_id(i, NULL, &part))
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);
				if(!a_button[i])
					grab[i + pointer_count].active = FALSE;

				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
				{
					if(snap != NULL && snap_active)
						pos[grab[i + pointer_count].part] = snap[grab[i + pointer_count].part];
					else 
					{
						uint axis;
						axis = seduce_element_primary_axis(input, i);
						if(axis != -1)
						{
							float vec[3] = {0, 0, 0}, dist;
							r_matrix_projection_screenf(&grab[i].matrix, tmp, pos[0], pos[1], pos[2]);
							dist = tmp[2];

							r_matrix_projection_worldf(&grab[i].matrix, vec, 0, 0, 0);
							for(j = 0; j < 3; j++)
							{
								vec[j] -= pos[j];
								if(vec[j] > 0)
									vec[j] = -dist;
								else
									vec[j] = dist;
							}
							if(grab[i + pointer_count].part < 3)
							{
								vec[(grab[i + pointer_count].part + 1) % 3] = 0;
								vec[(grab[i + pointer_count].part + 2) % 3] = 0;
							}else if(grab[i + pointer_count].part < 6)
								vec[grab[i + pointer_count].part % 3] = 0;
							r_matrix_projection_screenf(&grab[i].matrix, tmp, pos[0] - vec[0], pos[1] - vec[1], pos[2] - vec[2]);
							r_matrix_projection_screenf(&grab[i].matrix, tmp2, pos[0] + vec[0], pos[1] + vec[1], pos[2] + vec[2]);
							tmp[0] -= tmp2[0];
							tmp[1] -= tmp2[1];
							f_normalize2f(tmp);
							f = (tmp[0] * input->axis[axis].axis[0] + tmp[1] * input->axis[axis].axis[1]);
							if(f > 0)
								f *= f;
							else
								f *= -f;
							if(vec[0] != 0)
								size[0] *= 1 + f * input->delta_time * dist;
							if(vec[1] != 0)
								size[1] *= 1 + f * input->delta_time * dist;
							if(vec[2] != 0)
								size[2] *= 1 + f * input->delta_time * dist;
				
						}
					}
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{

						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].part = part;
						grab[i + pointer_count].size[0] = size[0];
						grab[i + pointer_count].size[1] = size[1];
						grab[i + pointer_count].size[2] = size[2];
		//				grab[i + pointer_count].timer = 0.1;
					}		
				}
			}
		}
	}

	if(input->mode == BAM_MAIN && frame_id != input->frame_number)
	{
	/*	frame_id = input->frame_number;
		for(i = 0; i < (pointer_count + user_count); i++)
		{
			if(grab[i].active)
			{
				grab[i].timer += input->delta_time * 2.5;
				if(grab[i].timer > 1.0)		
					grab[i].timer = 1.0;
			}else
				grab[i].timer -= input->delta_time * 2.5;
		}*/
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}