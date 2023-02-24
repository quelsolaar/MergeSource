#include <math.h>
#include <stdlib.h>
#include "seduce.h"
#include "s_draw_3d.h"

extern SViewData sui_default_view;

RMatrix *r_matrix_get();
extern void seduce_widget_overlay_matrix(RMatrix *matrix);
extern void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float scale, uint id, float fade,  float *color);

typedef struct{
	void *id;
	boolean active;
	uint axis;
	boolean invert;
	boolean rotate;
	float grab_matrix[16];
	RMatrix matrix;
}SeduceManipulatorGrab;


void seduce_manipulator_rotate_plain(BInputState *input, float pointer_x, float pointer_y, RMatrix *matrix, float *pos, float *vector, uint axis, float time)
{
	float color[5] = {0, 0.2, 1.0, 0.0, 0.2}, m[16];
	float origo[3] = {0, 0, 0}, x, y, t, scale = 1, tmp[3];
	uint i;
	r_matrix_push(matrix);
	r_matrix_projection_screenf(matrix, tmp, pos[0], pos[1], pos[2]);
	scale = -tmp[2];
	r_matrix_projection_surfacef(matrix, vector, pos, axis, pointer_x, pointer_y);
	vector[0] -= pos[0];
	vector[1] -= pos[1];
	vector[2] -= pos[2];

	for(i = 0; i < 8; i++)
	{
		x = sin((float)i / 8.0 * 2.0 * PI) * 0.17 * scale - vector[(axis + 1) % 3];
		y = cos((float)i / 8.0 * 2.0 * PI) * 0.17 * scale - vector[(axis + 2) % 3];
		if(x * x + y * y < 0.03 * 0.03 * scale * scale)
		{
			vector[(axis + 1) % 3] = sin((float)i / 8.0 * 2.0 * PI) * 0.15 * scale;
			vector[(axis + 2) % 3] = cos((float)i / 8.0 * 2.0 * PI) * 0.15 * scale;
			break;
		}
	}
	for(i = 0; i < 10; i++)
	{
		x = cos((float)i / 10.0 * 2.0 * PI) * 0.25 * scale - vector[(axis + 1) % 3];
		y = sin((float)i / 10.0 * 2.0 * PI) * 0.25 * scale - vector[(axis + 2) % 3];
		if(x * x + y * y < 0.03 * 0.03 * scale * scale)
		{
			vector[(axis + 1) % 3] = cos((float)i / 10.0 * 2.0 * PI) * 0.22 * scale;
			vector[(axis + 2) % 3] = sin((float)i / 10.0 * 2.0 * PI) * 0.22 * scale;
			break;
		}
	}
	for(i = 0; i < 12; i++)
	{
		x = sin((float)i / 12.0 * 2.0 * PI) * 0.32 * scale - vector[(axis + 1) % 3];
		y = cos((float)i / 12.0 * 2.0 * PI) * 0.32 * scale - vector[(axis + 2) % 3];
		if(x * x + y * y < 0.03 * 0.03 * scale * scale)
		{
			vector[(axis + 1) % 3] = sin((float)i / 12.0 * 2.0 * PI) * 0.29 * scale;
			vector[(axis + 2) % 3] = cos((float)i / 12.0 * 2.0 * PI) * 0.29 * scale;
			break;
		}
	}

	if(input->mode == BAM_DRAW)
	{
		t = 1.0 - time;
		r_matrix_push(matrix);
		origo[axis] = 1;
		f_matrixyzf(m, NULL, vector, origo);
		m[12] = vector[0];
		m[13] = vector[1];
		m[14] = vector[2];		
		r_matrix_translate(matrix, pos[0], pos[1], pos[2]);
		r_matrix_matrix_mult(matrix, m);
		seduce_object_3d_draw(input, 0, 0, 0, 0.1 * scale, SUI_3D_OBJECT_SNAPLOCK, time, &color[2 - axis]);
		r_matrix_pop(matrix);
		r_matrix_translate(matrix, pos[0], pos[1], pos[2]);
		r_matrix_scale(matrix, scale, scale, scale);
		if(axis == 0)
			r_matrix_rotate(matrix, -90, 0, 1, 0);
		if(axis == 1)
			r_matrix_rotate(matrix, -90, 1, 0, 0);
		for(i = 0; i < 8; i++)
		{
			r_matrix_push(matrix);
			r_matrix_rotate(matrix, (float)i * -360.0 / 8, 0, 0, 1);
			seduce_object_3d_draw(input, 0, 0.15 + t, 0, 0.1, SUI_3D_OBJECT_SNAP_SMALL, time, &color[2 - axis]);
			r_matrix_pop(matrix);
		}
		for(i = 0; i < 10; i++)
		{
			r_matrix_push(matrix);
			r_matrix_rotate(matrix, (float)i * -360.0 / 10, 0, 0, 1);
			seduce_object_3d_draw(input, 0, 0.22 + t * t, 0, 0.1, SUI_3D_OBJECT_SNAP_LARGE, time, &color[2 - axis]);
			r_matrix_pop(matrix);
		}
		for(i = 0; i < 12; i++)
		{
			r_matrix_push(matrix);
			r_matrix_rotate(matrix, (float)i * -360.0 / 12, 0, 0, 1);
			seduce_object_3d_draw(input, 0, 0.29 + t, 0, 0.1, SUI_3D_OBJECT_SNAP_SMALL, time, &color[2 - axis]);
			r_matrix_pop(matrix);
		}
	}
	r_matrix_pop(matrix);
}


STypeInState seduce_manipulator_rotate(BInputState *input, RMatrix *m, float *pos, float *rotation_matrix, void *id, float *snap, boolean snap_active, float scale, float time)
{
	static SeduceManipulatorGrab *grab = NULL;
	static boolean *a_button, *a_button_last;
	RMatrix *reset;
	uint i, part, pointer_count, user_count;
	float vector[3];
	scale *= 0.6;
	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);

	if(grab == NULL)
	{
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
		grab = malloc((sizeof *grab) * (pointer_count + user_count));
		for(i = 0; i < pointer_count + user_count; i++)
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
		r_matrix_projection_screenf(m, vector, pos[0], pos[1], pos[2]);
		scale *= -vector[2];
		if(vector[2] < 0)
		{
			boolean active[3] = {FALSE, FALSE, FALSE};
			r_matrix_set(m);


			for(i = 0; i < pointer_count; i++)
			{
				if(i < pointer_count && id == seduce_element_pointer_id(input, i, &part) && part < 3)
				{
					active[part] = TRUE;
					grab[i].matrix = *m;
				}
			}
			for(i = 0; i < input->user_count; i++)
			{
				if(id == seduce_element_selected_id(i, NULL, &part) && part < 3)
				{
					active[part] = TRUE;
					grab[i + pointer_count].matrix = *m;
				}
			}

			for(i = 0; i < (pointer_count + user_count); i++)
				if(grab[i].id == id && grab[i].active)
					break;

			if(i < (pointer_count + user_count))
			{
				seduce_manipulator_rotate_plain(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, m, pos, vector, grab[i].axis, time);
			}else
			{
				boolean x, y, z;
				float point[12] = {0.037, 0.022, 0, 0.045, 0.005, 0, 0.022, 0.037, 0, 0.005, 0.045, 0};
				float *c, color[5] = {0, 0.2, 1.0, 0.0, 0.2};
				float flip[3], vec[3];
				uint i;
				r_matrix_projection_worldf(m, vec, 0, 0, 0);
				for(i = 0; i < 3; i++)
				{
					vec[i] -= pos[i];
					if(vec[i] > 0)
						flip[i] = scale * 3.0;
					else
						flip[i] = -scale * 3.0;
				}

				f_normalize3f(vec);

				x = vec[0] > 0.2 || vec[0] < -0.2;
				y = vec[1] > 0.2 || vec[1] < -0.2;
				z = vec[2] > 0.2 || vec[2] < -0.2;
				r_matrix_push(m);			
				r_matrix_translate(m, pos[0], pos[1], pos[2]);
				r_matrix_scale(m, flip[0], flip[1], flip[2]);
				if(z)
				{
					if(active[2])
						c = &color[0];
					else
						c = NULL;
					r_matrix_push(m);
					seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
					seduce_element_add_line(input, id, 2, &point[0], &point[3], scale * 0.06);
					seduce_element_add_line(input, id, 2, &point[6], &point[9], scale * 0.06);
					if(!x && !y)
					{
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 2, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 2, &point[6], &point[9], scale * 0.06);
						r_matrix_scale(m, 1, -1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 2, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 2, &point[6], &point[9], scale * 0.06);
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 2, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 2, &point[6], &point[9], scale * 0.06);
					}else if(!x)
					{
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 2, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 2, &point[6], &point[9], scale * 0.06);
					}else if(!y)
					{
						r_matrix_scale(m, 1, -1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 2, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 2, &point[6], &point[9], scale * 0.06);
					}
					r_matrix_pop(m);
				}
				r_matrix_rotate(m, 90.0, 1, 0, 0);
				r_matrix_scale(m, 1, 1, -1);

				if(y)
				{
					if(active[1])
						c = &color[1];
					else
						c = NULL;
					r_matrix_push(m);
					seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
					seduce_element_add_line(input, id, 1, &point[0], &point[3], scale * 0.06);
					seduce_element_add_line(input, id, 1, &point[6], &point[9], scale * 0.06);
					if(!x && !z)
					{
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 1, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 1, &point[6], &point[9], scale * 0.06);
						r_matrix_scale(m, 1, -1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 1, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 1, &point[6], &point[9], scale * 0.06);
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 1, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 1, &point[6], &point[9], scale * 0.06);
					}else if(!z)
					{
						r_matrix_scale(m, 1, -1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 1, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 1, &point[6], &point[9], scale * 0.06);
					}else if(!x)
					{
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 1, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 1, &point[6], &point[9], scale * 0.06);
					}
					r_matrix_pop(m);
				}
				r_matrix_rotate(m, -90.0, 0, 1, 0);
				r_matrix_scale(m, 1, 1, -1);
				if(x)
				{
					if(active[0])
						c = &color[2];
					else
						c = NULL;
					r_matrix_push(m);
					seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
					seduce_element_add_line(input, id, 0, &point[0], &point[3], scale * 0.06);
					seduce_element_add_line(input, id, 0, &point[6], &point[9], scale * 0.06);
					if(!y && !z)
					{
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 0, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 0, &point[6], &point[9], scale * 0.06);
						r_matrix_scale(m, 1, -1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 0, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 0, &point[6], &point[9], scale * 0.06);
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 0, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 0, &point[6], &point[9], scale * 0.06);
					}else if(!z)
					{
						r_matrix_scale(m, 1, -1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 0, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 0, &point[6], &point[9], scale * 0.06);
					}else if(!y)
					{
						r_matrix_scale(m, -1, 1, 1);
						seduce_object_3d_draw(input, 0, 0.0, 0, 0.1, SUI_3D_OBJECT_HANDLE_ROTATE2, time, c);
						seduce_element_add_line(input, id, 0, &point[0], &point[3], scale * 0.06);
						seduce_element_add_line(input, id, 0, &point[6], &point[9], scale * 0.06);
					}
					r_matrix_pop(m);
				}
				r_matrix_pop(m);

			}
		
			r_matrix_set(reset);
		}
	}
	for(i = 0; i < input->pointer_count; i++)
	{
		if(!input->pointers[i].button[0] && grab[i].active && grab[i].id == id && input->mode == BAM_EVENT)
		{
			float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
			uint j;
			grab[i].active = FALSE;
			return S_TIS_DONE;
		}
 		if(grab[i].id == id && grab[i].active)
		{	
			float vector[3], f, axis[3] = {0, 0, 0}, matrix[16];
			if(input->mode != BAM_EVENT)
				return S_TIS_ACTIVE;
			if(snap_active)
			{
				vector[0] = snap[0] - pos[0];
				vector[1] = snap[1] - pos[1];
				vector[2] = snap[2] - pos[2];
				vector[grab[i].axis] = 0;
			}else
				seduce_manipulator_rotate_plain(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, &grab[i].matrix, pos, vector, grab[i].axis, time);

			if(grab[i].rotate)
			{
				f = vector[(grab[i].axis + 2) % 3];
				vector[(grab[i].axis + 2) % 3] = -vector[(grab[i].axis + 1) % 3];
				vector[(grab[i].axis + 1) % 3] = f;
			}
			if(grab[i].invert)
			{
				vector[(grab[i].axis + 1) % 3] = -vector[(grab[i].axis + 1) % 3];
				vector[(grab[i].axis + 2) % 3] = -vector[(grab[i].axis + 2) % 3];
			}
			vector[grab[i].axis] = 0.0;
			axis[grab[i].axis] = 1.0;
			if(grab[i].axis == 0)
				f_matrixxyf(matrix, NULL, axis, vector);
			if(grab[i].axis == 1)
				f_matrixyzf(matrix, NULL, axis, vector);
			if(grab[i].axis == 2)
				f_matrixzxf(matrix, NULL, axis, vector);					
			f_normalize3f(&matrix[0]);
			f_normalize3f(&matrix[4]);
			f_normalize3f(&matrix[8]);
			f_matrix_multiplyf(rotation_matrix, matrix, grab[i].grab_matrix);
			return S_TIS_ACTIVE;
		}else if(!grab[i].active)
		{
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			{
				uint part, j;
				if(id == seduce_element_pointer_id(input, i, &part) && part < 3)
				{
					float vector[3];
					grab[i].active = TRUE;
					grab[i].id = id;
					grab[i].axis = part;
					seduce_manipulator_rotate_plain(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, &grab[i].matrix, pos, vector, part, time);
					grab[i].rotate = vector[(part + 1) % 3] * vector[(part + 1) % 3] < vector[(part + 2) % 3] * vector[(part + 2) % 3];
					if(grab[i].rotate)
						grab[i].invert = vector[(part + 2) % 3] < 0.0;
					else
						grab[i].invert = vector[(part + 1) % 3] < 0.0;
					for(j = 0; j < 16; j++)
						grab[i].grab_matrix[j] = rotation_matrix[j];
					return S_TIS_ACTIVE;
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
			if(grab[i + pointer_count].id == id && grab[i + pointer_count].active && !a_button[i])
			{
				grab[i + pointer_count].active = FALSE;
				return S_TIS_DONE;
			}
			if(grab[i + pointer_count].id == id && grab[i + pointer_count].active)
			{
				uint axis;
				axis = seduce_element_primary_axis(input, i);
				if(axis != -1)
				{
				/*	r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
					tmp[0] += input->axis[axis].axis[0] * input->delta_time;
					tmp[1] += input->axis[axis].axis[1] * input->delta_time;
					r_matrix_projection_worldf(m, tmp, tmp[0], tmp[1], tmp[2]);
					pos[0] = tmp[0];
					pos[1] = tmp[1];
					pos[2] = tmp[2];*/
				}
			}else if(!grab[i + pointer_count].active)
			{
				if(a_button[i] && !a_button_last[i])
				{
					grab[i + pointer_count].active = TRUE;
					grab[i + pointer_count].id = id;
				/*	grab[i + pointer_count].timer = 0.1;

					seduce_manipulator_rotate_plain(input, input->pointers[i].pointer_x, input->pointers[i].pointer_y, m, pos, vector, part, time);
					printf("rotate axis %u - %f %f %f\n", part, vector[0], vector[1], vector[2]);
					grab[i].rotate = vector[(part + 1) % 3] * vector[(part + 1) % 3] < vector[(part + 2) % 3] * vector[(part + 2) % 3];
					if(grab[i].rotate)
						grab[i].invert = vector[(part + 2) % 3] < 0.0;
					else
						grab[i].invert = vector[(part + 1) % 3] < 0.0;
					for(j = 0; j < 16; j++)
						grab[i].grab_matrix[j] = rotation_matrix[j];*/
				}		
			}
		}
	}
	return S_TIS_IDLE;
}