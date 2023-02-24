#include <math.h>
#include <stdlib.h>
#include "seduce.h"
#include "s_draw_3d.h"

extern SViewData sui_default_view;

RMatrix *r_matrix_get();
extern void seduce_widget_overlay_matrix(RMatrix *matrix);
extern void seduce_object_3d_draw(BInputState *input, float pos_x, float pos_y, float pos_z, float scale, uint id, float fade,  float *color);
/*
void seduce_manipulator_matrix_clear_and_clone(RMatrix *matrix, RMatrix *clone)
{
	uint m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	uint i;
	if(clone == NULL)
		clone = r_matrix_get();
	for(i = 0; i < 16; i++)
	{
		matrix->matrix[0][i] = m[i];
		matrix->projection[i] = clone->projection[i];
	}
	matrix->current = 0;
}*/

typedef struct{
	void *id;
	boolean active;
	float timer;
	uint part;
	float grab_pos[3];
	RMatrix matrix;
}SeduceManipulatorSpaceGrab;

boolean	seduce_manipulator_point_axis_internal(BInputState *input, SeduceManipulatorSpaceGrab *grab, RMatrix *m, float *pos, void *id, uint part, float *snap, boolean snap_active, uint axis, float scale, float *matrix, uint icon)
{
	static boolean *a_button = NULL, *a_button_last;
	float tmp[3], f;
	uint i, count, active_pointer = -1, pointer_count, user_count, p;

	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);

	if(a_button == NULL)
	{
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}
	if(input->mode == BAM_DRAW)
	{
		r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
		if(tmp[2] < 0)
		{

			float color[4] = {0.2, 0.6, 1.0, 1.0};
			static float time = 0;
			boolean active = FALSE;

			for(i = 0; i < input->pointer_count; i++)
			{
				if((grab[i].id == id && grab[i].active && grab[i].part == part) || (id == seduce_element_pointer_id(input, i, &p) && p == part))
				{
					active = TRUE;
					grab[i].matrix = *m;
				}
			}
			for(i = 0; i < input->user_count; i++)
			{
				if(id == seduce_element_selected_id(i, NULL, &p) && p == part)
				{
					active = TRUE;
					grab[i + pointer_count].matrix = *m;
				}
			}
			r_matrix_push(m);
			r_matrix_translate(m, pos[0], pos[1], pos[2]);
			r_matrix_normalize_scale(m);
			r_matrix_matrix_mult(m, matrix);
			if(active)
				seduce_object_3d_draw(input, 0, 0, 0, scale, icon, 1, color);
			else
				seduce_object_3d_draw(input, 0, 0, 0, scale, icon, 1, NULL);
			r_matrix_pop(m);
			seduce_element_add_point(input, id, part, pos, scale);
		}
	}

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(!input->pointers[i].button[0])
				grab[i].active = FALSE;
			if(grab[i].id == id && grab[i].active && grab[i].part == part)
			{	
				if(snap != NULL && snap_active)
				{
					pos[axis] = snap[axis];
				}else
				{
					r_matrix_projection_axisf(&grab[i].matrix, tmp, pos, axis, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
					pos[0] = tmp[0] + grab[i].grab_pos[0];
					pos[1] = tmp[1] + grab[i].grab_pos[1];
					pos[2] = tmp[2] + grab[i].grab_pos[2];
				}
			}else if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, &p) && p == part)
					{
						grab[i].active = TRUE;
						grab[i].id = id;
						grab[i].part = part;
						grab[i].timer = 0.1;
						r_matrix_projection_axisf(&grab[i].matrix, tmp, pos, axis, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
						grab[i].grab_pos[0] = pos[0] - tmp[0];
						grab[i].grab_pos[1] = pos[1] - tmp[1];
						grab[i].grab_pos[2] = pos[2] - tmp[2];
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(id == seduce_element_selected_id(i, NULL, &p) && p == part)
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);
				if(!a_button[i])
					grab[i + pointer_count].active = FALSE;

				if(grab[i + pointer_count].id == id && grab[i + pointer_count].active && grab[i + pointer_count].part == part)
				{
					uint a;
					a = seduce_element_primary_axis(input, i);
					if(a != -1)
					{
						r_matrix_projection_screenf(&grab[i + pointer_count].matrix, tmp, pos[0], pos[1], pos[2]);
						tmp[0] += input->axis[a].axis[0] * input->delta_time;
						tmp[1] += input->axis[a].axis[1] * input->delta_time;
						r_matrix_projection_axisf(&grab[i + pointer_count].matrix, tmp, pos, axis, tmp[0], tmp[1]);
						pos[0] = tmp[0];
						pos[1] = tmp[1];
						pos[2] = tmp[2];
					}
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						grab[i + pointer_count].part = part;
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
			}
		}
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active && grab[i].part == part)
			return TRUE;
	return FALSE;
}

boolean	seduce_manipulator_point_plane_internal(BInputState *input, SeduceManipulatorSpaceGrab *grab, RMatrix *m, float *pos, void *id, uint part, float *snap, boolean snap_active, uint axis, float scale, float *matrix, uint icon)
{
	static boolean *a_button = NULL, *a_button_last;
	float tmp[3], f;
	uint i, count, active_pointer = -1, pointer_count, user_count, p;

	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	if(a_button == NULL)
	{
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}
	if(input->mode == BAM_DRAW)
	{
		r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
		if(tmp[2] < 0)
		{
			float color[4] = {0.2, 0.6, 1.0, 1.0};
			static float time = 0;
			boolean active = FALSE;

			for(i = 0; i < input->pointer_count; i++)
			{
				if((grab[i].id == id && grab[i].active && grab[i].part == part) || (id == seduce_element_pointer_id(input, i, &p) && p == part))
				{
					active = TRUE;
					grab[i].matrix = *m;
				}
			}
			for(i = 0; i < input->user_count; i++)
			{
				if(id == seduce_element_selected_id(i, NULL, &p) && p == part)
				{
					active = TRUE;
					grab[i + pointer_count].matrix = *m;
				}
			}
			r_matrix_push(m);
			r_matrix_translate(m, pos[0], pos[1], pos[2]);
			r_matrix_normalize_scale(m);
			r_matrix_matrix_mult(m, matrix);
			if(active)
				seduce_object_3d_draw(input, 0, 0, 0, scale, icon, 1, color);
			else
				seduce_object_3d_draw(input, 0, 0, 0, scale, icon, 1, NULL);
			r_matrix_pop(m);
			seduce_element_add_point(input, id, part, pos, scale);
		}
	}

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(!input->pointers[i].button[0])
				grab[i].active = FALSE;
			if(grab[i].id == id && grab[i].active && grab[i].part == part)
			{	
				if(snap != NULL && snap_active)
				{
					pos[(axis + 1) % 3] = snap[(axis + 1) % 3];
					pos[(axis + 2) % 3] = snap[(axis + 2) % 3];
				}else
				{
					if(r_matrix_projection_surfacef(&grab[i].matrix, tmp, pos, axis, input->pointers[i].pointer_x, input->pointers[i].pointer_y))
					{
						pos[0] = tmp[0] + grab[i].grab_pos[0];
						pos[1] = tmp[1] + grab[i].grab_pos[1];
						pos[2] = tmp[2] + grab[i].grab_pos[2];
					}
				}
			}else if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, &p) && part == p)
					{
						if(r_matrix_projection_surfacef(&grab[i].matrix, tmp, pos, axis, input->pointers[i].pointer_x, input->pointers[i].pointer_y))
						{
							grab[i].active = TRUE;
							grab[i].id = id;
							grab[i].part = part;
							grab[i].timer = 0.1;
							grab[i].grab_pos[0] = pos[0] - tmp[0];
							grab[i].grab_pos[1] = pos[1] - tmp[1];
							grab[i].grab_pos[2] = pos[2] - tmp[2];
						}
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(id == seduce_element_selected_id(i, NULL, &p) && p == part)
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);
				if(!a_button[i])
					grab[i + pointer_count].active = FALSE;

				if(grab[i + pointer_count].id == id && grab[i + pointer_count].part == part && grab[i + pointer_count].active)
				{
					uint a;
					a = seduce_element_primary_axis(input, i);
					if(a != -1)
					{
						r_matrix_projection_screenf(&grab[i + pointer_count].matrix, tmp, pos[0], pos[1], pos[2]);
						tmp[0] += input->axis[a].axis[0] * input->delta_time;
						tmp[1] += input->axis[a].axis[1] * input->delta_time;
						if(r_matrix_projection_surfacef(&grab[i + pointer_count].matrix, tmp, pos, axis, tmp[0], tmp[1]))
						{
							pos[0] = tmp[0];
							pos[1] = tmp[1];
							pos[2] = tmp[2];
						}
					}
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].part = part;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
			}
		}
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active && grab[i].part == part)
			return TRUE;
	return FALSE;
}

boolean	seduce_manipulator_point_pos_internal(BInputState *input, SeduceManipulatorSpaceGrab *grab, RMatrix *m, float *pos, void *id, uint part, float *snap, boolean snap_active, float scale, float *matrix, uint icon)
{
	static boolean *a_button = NULL, *a_button_last;
	float tmp[3], f;
	uint i, count, active_pointer = -1, pointer_count, user_count, p;

	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	if(a_button == NULL)
	{
		a_button = malloc((sizeof *a_button) * user_count);
		a_button_last = malloc((sizeof *a_button_last) * user_count);
		for(i = 0; i < user_count; i++)
			a_button[i] = a_button_last[i] = FALSE;
	}
	if(input->mode == BAM_DRAW)
	{
		r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
		if(tmp[2] < 0)
		{
			float color[4] = {0.2, 0.6, 1.0, 1.0};
			static float time = 0;
			boolean active = FALSE;

			for(i = 0; i < input->pointer_count; i++)
			{
				if((grab[i].id == id && grab[i].active && grab[i].part == part) || (id == seduce_element_pointer_id(input, i, &p) && p == part))
				{
					active = TRUE;
					grab[i].matrix = *m;
				}
			}
			for(i = 0; i < input->user_count; i++)
			{
				if(id == seduce_element_selected_id(i, NULL, &p) && p == part)
				{
					active = TRUE;
					grab[i + pointer_count].matrix = *m;
				}
			}
			r_matrix_push(m);
			r_matrix_translate(m, pos[0], pos[1], pos[2]);
			r_matrix_normalize_scale(m);
			r_matrix_matrix_mult(m, matrix);
			if(active)
				seduce_object_3d_draw(input, 0, 0, 0, scale, icon, 1, color);
			else
				seduce_object_3d_draw(input, 0, 0, 0, scale, icon, 1, NULL);
			r_matrix_pop(m);
			seduce_element_add_point(input, id, part, pos, scale);
		}

	}

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(!input->pointers[i].button[0])
				grab[i].active = FALSE;
			if(grab[i].id == id && grab[i].active && grab[i].part == part)
			{	
				if(snap != NULL && snap_active)
				{
					pos[0] = snap[0];
					pos[1] = snap[1];
					pos[2] = snap[2];
				}else
				{
					r_matrix_projection_vertexf(&grab[i].matrix, tmp, pos, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
					pos[0] = tmp[0] + grab[i].grab_pos[0];
					pos[1] = tmp[1] + grab[i].grab_pos[1];
					pos[2] = tmp[2] + grab[i].grab_pos[2];
				}
			}else if(!grab[i].active)
			{
				if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
				{
					if(id == seduce_element_pointer_id(input, i, &p) && part == p)
					{
						grab[i].active = TRUE;
						grab[i].id = id;
						grab[i].part = part;
						grab[i].timer = 0.1;
						r_matrix_projection_vertexf(&grab[i].matrix, tmp, pos, input->pointers[i].pointer_x, input->pointers[i].pointer_y);
						grab[i].grab_pos[0] = pos[0] - tmp[0];
						grab[i].grab_pos[1] = pos[1] - tmp[1];
						grab[i].grab_pos[2] = pos[2] - tmp[2];
					}
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(id == seduce_element_selected_id(i, NULL, &p) && p == part)
			{
				betray_button_get_up_down(i, &a_button[i], &a_button_last[i], BETRAY_BUTTON_FACE_A);
				if(!a_button[i])
					grab[i + pointer_count].active = FALSE;

				if(grab[i + pointer_count].id == id && grab[i + pointer_count].part == part && grab[i + pointer_count].active)
				{
					uint a;
					a = seduce_element_primary_axis(input, i);
					if(a != -1)
					{
						r_matrix_projection_screenf(&grab[i + pointer_count].matrix, tmp, pos[0], pos[1], pos[2]);
						tmp[0] += input->axis[a].axis[0] * input->delta_time;
						tmp[1] += input->axis[a].axis[1] * input->delta_time;
						r_matrix_projection_worldf(&grab[i + pointer_count].matrix, tmp, tmp[0], tmp[1], tmp[2]);
						pos[0] = tmp[0];
						pos[1] = tmp[1];
						pos[2] = tmp[2];
					}
				}else if(!grab[i + pointer_count].active)
				{
					if(a_button[i] && !a_button_last[i])
					{
						grab[i + pointer_count].active = TRUE;
						grab[i + pointer_count].id = id;
						grab[i + pointer_count].part = part;
						grab[i + pointer_count].timer = 0.1;
					}		
				}
			}
		}
	}

	for(i = 0; i < (pointer_count + user_count); i++)
		if(grab[i].id == id && grab[i].active && grab[i].part == part)
			return TRUE;
	return FALSE;
}

boolean	seduce_manipulator_square_centered(BInputState *input, RMatrix *m, float *pos, float *size, void *id, float *snap, boolean snap_active, boolean aspect_lock, float scale, float time)
{
	static SeduceManipulatorSpaceGrab *grab = NULL;
	static uint frame_id = -1;
	uint i, count, pointer_count, user_count;
	float tmp[3], matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	boolean output = FALSE;
	RMatrix *reset;
	if(m == NULL)
		m = r_matrix_get();


	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);

	if(grab == NULL)
	{
		count = betray_support_functionality(B_SF_USER_COUNT_MAX);
		count += betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
		{
			grab[i].id = NULL;
			r_matrix_identity(&grab[i].matrix);
		}
	}

	if(input->mode == BAM_DRAW)
	{
		reset = r_matrix_get();
		r_matrix_set(m);
		r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
		scale *= -tmp[2] * 0.025;
	}else
	{
		for(i = 0; i < pointer_count + user_count && grab[i].id != id; i++);
		if(i < pointer_count + user_count)
			r_matrix_projection_screenf(&grab[i].matrix, tmp, pos[0], pos[1], pos[2]);
		else		
			r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
	}

	time = (1.0 - time);
	time = time * time * 0.3 * -tmp[2];
	tmp[0] = pos[0] + size[0] + time;
	tmp[1] = pos[1];
	tmp[2] = pos[2];
	matrix[0] = 0;
	matrix[1] = 1;
	matrix[4] = -1;
	matrix[5] = 0;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 0, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_2D))
	{
		if(aspect_lock)
		{
			size[1] *= (tmp[0] - pos[0]) / size[0];
			size[0] = tmp[0] - pos[0];
		}else
			size[0] = tmp[0] - pos[0];
		output = TRUE;
	}
	tmp[0] = pos[0] - size[0] - time;
	tmp[1] = pos[1];
	tmp[2] = pos[2];
	matrix[0] = 0;
	matrix[1] = -1;
	matrix[4] = 1;
	matrix[5] = 0;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 1, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_2D))
	{
		if(aspect_lock)
		{
			size[1] *= (pos[0] - tmp[0]) / size[0];
			size[0] = pos[0] - tmp[0];
		}else
			size[0] = pos[0] - tmp[0];
		output = TRUE;
	}
	tmp[0] = pos[0];
	tmp[1] = pos[1] + size[1] + time;
	tmp[2] = pos[2];
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[4] = 0;
	matrix[5] = -1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 2, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_2D))
	{
		if(aspect_lock)
		{
			size[0] *= (tmp[1] - pos[1]) / size[1];
			size[1] = tmp[1] - pos[1];
		}else
			size[1] = tmp[1] - pos[1];
		output = TRUE;
	}
	tmp[0] = pos[0];
	tmp[1] = pos[1] - size[1] - time;
	tmp[2] = pos[2];
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[4] = 0;
	matrix[5] = 1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 3, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_2D))
	{
		if(aspect_lock)
		{
			size[0] *= (pos[1] - tmp[1]) / size[1];
			size[1] = pos[1] - tmp[1];
		}else
			size[1] = pos[1] - tmp[1];
		output = TRUE;
	}

	if(!aspect_lock)
	{
		tmp[0] = pos[0] + size[0] + time;
		tmp[1] = pos[1] + size[1] + time;
		tmp[2] = pos[2];
		matrix[0] = 0;
		matrix[1] = -1;
		matrix[4] = -1;
		matrix[5] = 0;
		if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 4, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_2D))
		{
			size[0] = tmp[0] - pos[0];
			size[1] = tmp[1] - pos[1];
			output = TRUE;
		}
		tmp[0] = pos[0] - size[0] - time;
		tmp[1] = pos[1] + size[1] + time;
		tmp[2] = pos[2];
		matrix[0] = 0;
		matrix[1] = -1;
		matrix[4] = 1;
		matrix[5] = 0;
		if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 5, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_2D))
		{
			size[0] = pos[0] - tmp[0];
			size[1] = tmp[1] - pos[1];
			output = TRUE;
		}		
		tmp[0] = pos[0] - size[0] - time;
		tmp[1] = pos[1] - size[1] - time;
		tmp[2] = pos[2];
		matrix[0] = 1;
		matrix[1] = 0;
		matrix[4] = 0;
		matrix[5] = 1;
		if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 6, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_2D))
		{
			size[0] = pos[0] - tmp[0];
			size[1] = pos[1] - tmp[1];
			output = TRUE;
		}
		tmp[0] = pos[0] + size[0] + time;
		tmp[1] = pos[1] - size[1] - time;
		tmp[2] = pos[2];
		matrix[0] = -1;
		matrix[1] = 0;
		matrix[4] = 0;
		matrix[5] = 1;
		if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 7, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_2D))
		{
			size[0] = tmp[0] - pos[0];
			size[1] = pos[1] - tmp[1];
			output = TRUE;
		}
	}
	if(size[0] < 0)
	{
		size[0] = -size[0];
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 0)
					grab[i].part = 1; 
				else if(grab[i].part == 1)
					grab[i].part = 0; 
				else if(grab[i].part == 4)
					grab[i].part = 5; 
				else if(grab[i].part == 5)
					grab[i].part = 4; 
				else if(grab[i].part == 6)
					grab[i].part = 7; 
				else if(grab[i].part == 7)
					grab[i].part = 6; 
			}
		}
	}
	if(size[1] < 0)
	{
		size[1] = -size[1];
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 2)
					grab[i].part = 3; 
				else if(grab[i].part == 3)
					grab[i].part = 2; 
				else if(grab[i].part == 4)
					grab[i].part = 6; 
				else if(grab[i].part == 6)
					grab[i].part = 4; 
				else if(grab[i].part == 5)
					grab[i].part = 7; 
				else if(grab[i].part == 7)
					grab[i].part = 5; 
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
	if(input->mode == BAM_DRAW)
		r_matrix_set(reset);
	return output;
}


boolean	seduce_manipulator_square_cornered(BInputState *input, RMatrix *m, float *down_left, float *up_right, void *id, float *snap, boolean snap_active, boolean aspect_lock, float scale, float time)
{
	static SeduceManipulatorSpaceGrab *grab = NULL;
	static uint frame_id = -1;
	uint i, count, pointer_count, user_count;
	float f, size, tmp[3], matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	boolean output = FALSE;
	RMatrix *reset;
	if(m == NULL)
		m = r_matrix_get();
	if(input->mode == BAM_DRAW)
	{
		reset = r_matrix_get();
		r_matrix_set(m);
	}

	r_matrix_projection_screenf(m, tmp, (down_left[0] + up_right[0]) * 0.5, (down_left[1] + up_right[1]) * 0.5, (down_left[2] + up_right[2]) * 0.5);
	scale *= -tmp[2] * 0.025;

	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);

	if(grab == NULL)
	{
		count = betray_support_functionality(B_SF_USER_COUNT_MAX);
		count += betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
		{
			grab[i].id = NULL;
			r_matrix_identity(&grab[i].matrix);
		}
	}

	time = (1.0 - time);
	time = time * time * 0.3 * -tmp[2];
	tmp[0] = up_right[0] + time;
	tmp[1] = (down_left[1] + up_right[1]) * 0.5;
	tmp[2] = down_left[2];
	matrix[0] = 0;
	matrix[1] = 1;
	matrix[4] = -1;
	matrix[5] = 0;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 0, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_2D))
	{
		if(aspect_lock)
		{
	/*		size = (up_right[1] - down_left[1]) / (up_right[0] - down_left[0]);
			size = (tmp[0] - down_left[0]) * size * 0.5;
			if(size < 0)
				size = -size;
			f = (down_left[1] + up_right[1]) * 0.5;
			down_left[1] = f - size;
			up_right[1] = f + size;*/
		}
		up_right[0] = tmp[0];
		output = TRUE;
	}
	tmp[0] = down_left[0] - time;
	tmp[1] = (down_left[1] + up_right[1]) * 0.5;
	tmp[2] = down_left[2];
	matrix[0] = 0;
	matrix[1] = -1;
	matrix[4] = 1;
	matrix[5] = 0;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 1, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_2D))
	{
		if(aspect_lock)
		{
	/*		size = (up_right[1] - down_left[1]) / (up_right[0] - down_left[0]);
			size = (up_right[0] - tmp[0]) * size * 0.5;
			if(size < 0)
				size = -size;
			f = (down_left[1] + up_right[1]) * 0.5;
			down_left[1] = f - size;
			up_right[1] = f + size;*/
		}
		down_left[0] = tmp[0];
		output = TRUE;
	}
	tmp[0] = (down_left[0] + up_right[0]) * 0.5;
	tmp[1] = up_right[1] + time;
	tmp[2] = down_left[2];
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[4] = 0;
	matrix[5] = -1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 2, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_2D))
	{
	/*	if(aspect_lock)
		{
			size = up_right[1] - down_left[1];
			f = (down_left[1] + up_right[1]) * 0.5;
			down_left[1] = f - size * tmp[0] / up_right[0] * 0.5;
			up_right[1] = f + size * tmp[0] / up_right[0] * 0.5;
		}*/
		up_right[1] = tmp[1];
		output = TRUE;
	}
	tmp[0] = (down_left[0] + up_right[0]) * 0.5;
	tmp[1] = down_left[1] - time;
	tmp[2] = down_left[2];
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[4] = 0;
	matrix[5] = 1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 3, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_2D))
	{
	/*	if(aspect_lock)
		{
			size = up_right[1] - down_left[1];
			f = (down_left[1] + up_right[1]) * 0.5;
			down_left[1] = f - size * tmp[0] / up_right[0] * 0.5;
			up_right[1] = f + size * tmp[0] / up_right[0] * 0.5;
		}*/
		down_left[1] = tmp[1];
		output = TRUE;
	}

	if(!aspect_lock)
	{
		tmp[0] = up_right[0] + time;
		tmp[1] = up_right[1] + time;
		tmp[2] = up_right[2];
		matrix[0] = 0;
		matrix[1] = -1;
		matrix[4] = -1;
		matrix[5] = 0;
		if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 4, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_2D))
		{
			up_right[0] = tmp[0];
			up_right[1] = tmp[1];
			output = TRUE;
		}
		tmp[0] = down_left[0] - time;
		tmp[1] = up_right[1] + time;
		tmp[2] = up_right[2];
		matrix[0] = 0;
		matrix[1] = -1;
		matrix[4] = 1;
		matrix[5] = 0;
		if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 5, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_2D))
		{
			down_left[0] = tmp[0];
			up_right[1] = tmp[1];
			output = TRUE;
		}		
		tmp[0] = down_left[0] - time;
		tmp[1] = down_left[1] - time;
		tmp[2] = up_right[2];
		matrix[0] = 1;
		matrix[1] = 0;
		matrix[4] = 0;
		matrix[5] = 1;
		if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 6, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_2D))
		{
			down_left[0] = tmp[0];
			down_left[1] = tmp[1];
			output = TRUE;
		}
		tmp[0] = up_right[0] + time;
		tmp[1] = down_left[1] - time;
		tmp[2] = up_right[2];
		matrix[0] = -1;
		matrix[1] = 0;
		matrix[4] = 0;
		matrix[5] = 1;
		if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 7, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_2D))
		{
			up_right[0] = tmp[0];
			down_left[1] = tmp[1];
			output = TRUE;
		}
	}
	if(up_right[0] < down_left[0])
	{
		f = up_right[0];
		up_right[0] = down_left[0];
		down_left[0] = up_right[0];
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 0)
					grab[i].part = 1; 
				else if(grab[i].part == 1)
					grab[i].part = 0; 
				else if(grab[i].part == 4)
					grab[i].part = 5; 
				else if(grab[i].part == 5)
					grab[i].part = 4; 
				else if(grab[i].part == 6)
					grab[i].part = 7; 
				else if(grab[i].part == 7)
					grab[i].part = 6; 
			}
		}
	}
	if(up_right[1] < down_left[1])
	{
		f = up_right[1];
		up_right[1] = down_left[1];
		down_left[1] = up_right[1];
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 2)
					grab[i].part = 3; 
				else if(grab[i].part == 3)
					grab[i].part = 2; 
				else if(grab[i].part == 4)
					grab[i].part = 7; 
				else if(grab[i].part == 7)
					grab[i].part = 4; 
				else if(grab[i].part == 5)
					grab[i].part = 6; 
				else if(grab[i].part == 6)
					grab[i].part = 5; 
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
	if(input->mode == BAM_DRAW)
		r_matrix_set(reset);
	return output;
}

boolean	seduce_manipulator_cube_centered(BInputState *input, RMatrix *m, float *pos, float *size, void *id, float *snap, boolean snap_active, float scale, float time)
{
	static SeduceManipulatorSpaceGrab *grab = NULL;
	static uint frame_id = -1;
	uint i, count, pointer_count, user_count;
	float tmp[3], matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, origo[3] = {0, 0, 0};
	boolean output = FALSE;
	RMatrix *reset;
	if(m == NULL)
		m = r_matrix_get();
	if(pos == NULL)
		pos = origo;

	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);

	if(grab == NULL)
	{
		count = betray_support_functionality(B_SF_USER_COUNT_MAX);
		count += betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
		{
			grab[i].id = NULL;
			r_matrix_identity(&grab[i].matrix);
		}
	}
	if(input->mode == BAM_DRAW)
	{
		reset = r_matrix_get();
		r_matrix_set(m);
		r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
		scale *= -tmp[2] * 0.01;
	}else
	{
		for(i = 0; i < pointer_count + user_count && grab[i].id != id; i++);
		if(i < pointer_count + user_count)
			r_matrix_projection_screenf(&grab[i].matrix, tmp, pos[0], pos[1], pos[2]);
		else		
			r_matrix_projection_screenf(m, tmp, pos[0], pos[1], pos[2]);
	}


	time = (1.0 - time);
	time = time * time * 0.3 * -tmp[2];
	tmp[0] = pos[0] + size[0] + time;
	tmp[1] = pos[1];
	tmp[2] = pos[2];
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 0, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		size[0] = tmp[0] - pos[0];
		output = TRUE;
	}
	tmp[0] = pos[0] - size[0] - time;
	tmp[1] = pos[1];
	tmp[2] = pos[2];

	matrix[0] = -1;
	matrix[5] = -1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 1, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		size[0] = pos[0] - tmp[0];
		output = TRUE;
	}

	tmp[0] = pos[0];
	tmp[1] = pos[1] + size[1] + time;
	tmp[2] = pos[2];
	matrix[0] = 0;
	matrix[1] = 1;
	matrix[4] = 1;
	matrix[5] = 0;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 2, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		size[1] = tmp[1] - pos[1];
		output = TRUE;
	}
	tmp[0] = pos[0];
	tmp[1] = pos[1] - size[1] - time;
	tmp[2] = pos[2];
	matrix[1] = -1;
	matrix[4] = -1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 3, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		size[1] = pos[1] - tmp[1];
		output = TRUE;
	}


	tmp[0] = pos[0];
	tmp[1] = pos[1];
	tmp[2] = pos[2] + size[2] + time;
	matrix[1] = 0;
	matrix[4] = 0;
	matrix[2] = 1;
	matrix[5] = 1;
	matrix[8] = 1;
	matrix[10] = 0;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 4, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		size[2] = tmp[2] - pos[2];
		output = TRUE;
	}
	tmp[0] = pos[0];
	tmp[1] = pos[1];
	tmp[2] = pos[2] - size[2] - time;
	matrix[2] = -1;
	matrix[5] = 1;
	matrix[8] = -1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 5, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		size[2] = pos[2] - tmp[2];
		output = TRUE;
	}
	tmp[0] = pos[0] + size[0] + time;
	tmp[1] = pos[1] + size[1] + time;
	tmp[2] = pos[2];
	matrix[0] = 1;
	matrix[5] = 0;
	matrix[6] = 1;
	matrix[2] = 0;
	matrix[8] = 0;
	matrix[9] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 6, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[0] = tmp[0] - pos[0];
		size[1] = tmp[1] - pos[1];
		output = TRUE;
	}

	tmp[0] = pos[0] - size[0] - time;
	tmp[1] = pos[1] + size[1] + time;
	tmp[2] = pos[2];
	matrix[0] = -1;
	matrix[6] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 7, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[0] = pos[0] - tmp[0];
		size[1] = tmp[1] - pos[1];
		output = TRUE;
	}
	tmp[0] = pos[0] - size[0] - time;
	tmp[1] = pos[1] - size[1] - time;
	tmp[2] = pos[2];
	matrix[0] = -1;
	matrix[6] = 1;
	matrix[9] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 8, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[0] = pos[0] - tmp[0];
		size[1] = pos[1] - tmp[1];
		output = TRUE;
	}	
	tmp[0] = pos[0] + size[0] + time;
	tmp[1] = pos[1] - size[1] - time;
	tmp[2] = pos[2];
	matrix[0] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 9, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[0] = tmp[0] - pos[0];
		size[1] = pos[1] - tmp[1];
		output = TRUE;
	}

	tmp[0] = pos[0] + size[0] + time;
	tmp[1] = pos[1];
	tmp[2] = pos[2] + size[2] + time;
	matrix[0] = 0;
	matrix[5] = 1;
	matrix[6] = 0;
	matrix[2] = 1;
	matrix[8] = 1;
	matrix[9] = 0;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 10, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[0] = tmp[0] - pos[0];
		size[2] = tmp[2] - pos[2];
		output = TRUE;
	}

	tmp[0] = pos[0] - size[0] + time;
	tmp[1] = pos[1];
	tmp[2] = pos[2] + size[2] + time;
	matrix[5] = 1;
	matrix[2] = 1;
	matrix[8] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 11, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[0] = pos[0] - tmp[0];
		size[2] = tmp[2] - pos[2];
		output = TRUE;
	}

	tmp[0] = pos[0] + size[0] + time;
	tmp[1] = pos[1];
	tmp[2] = pos[2] - size[2] + time;

	matrix[5] = 1;
	matrix[2] = -1;
	matrix[8] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 12, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[0] = tmp[0] - pos[0];
		size[2] = pos[2] - tmp[2];
		output = TRUE;
	}

	tmp[0] = pos[0] - size[0] + time;
	tmp[1] = pos[1];
	tmp[2] = pos[2] - size[2] + time;
	matrix[5] = 1;
	matrix[2] = -1;
	matrix[8] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 13, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[0] = pos[0] - tmp[0];
		size[2] = pos[2] - tmp[2];
		output = TRUE;
	}


	tmp[0] = pos[0];
	tmp[1] = pos[1] + size[1] + time;
	tmp[2] = pos[2] + size[2] + time;

	matrix[1] = 1;
	matrix[2] = 0;
	matrix[4] = 1;
	matrix[5] = 0;
	matrix[8] = 0;
	matrix[10] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 14, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[1] = tmp[1] - pos[1];
		size[2] = tmp[2] - pos[2];
		output = TRUE;
	}

	tmp[0] = pos[0];
	tmp[1] = pos[1] - size[1] - time;
	tmp[2] = pos[2] + size[2] + time;
	matrix[1] = -1;
	matrix[4] = 1;
	matrix[10] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 15, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[1] = pos[1] - tmp[1];
		size[2] = tmp[2] - pos[2];
		output = TRUE;
	}
	tmp[0] = pos[0];
	tmp[1] = pos[1] + size[1] + time;
	tmp[2] = pos[2] - size[2] - time;
	matrix[1] = 1;
	matrix[4] = 1;
	matrix[10] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 16, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[1] = tmp[1] - pos[1];
		size[2] = pos[2] - tmp[2];
		output = TRUE;
	}
	tmp[0] = pos[0];
	tmp[1] = pos[1] - size[1] - time;
	tmp[2] = pos[2] - size[2] - time;
	matrix[1] = -1;
	matrix[4] = 1;
	matrix[10] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 17, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		size[1] = pos[1] - tmp[1];
		size[2] = pos[2] - tmp[2];
		output = TRUE;
	}
	matrix[1] = 0;
	matrix[4] = 0;
	matrix[8] = 0;
	for(i = 0; i < 8; i++)
	{
		matrix[0] = tmp[0] = (float)(i % 2) * 2.0 - 1.0;
		matrix[5] = tmp[1] = (float)((i / 2) % 2) * -2.0 + 1.0;
		matrix[10] = tmp[2] = (float)((i / 4) % 2) * 2.0 - 1.0;
		tmp[0] = pos[0] + (size[0] + time) * tmp[0];
		tmp[1] = pos[1] + (size[1] + time) * -tmp[1];
		tmp[2] = pos[2] + (size[2] + time) * tmp[2];
		if(seduce_manipulator_point_pos_internal(input, grab, m, tmp, id, 18 + i, snap, snap_active, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_3D))
		{
			size[0] = (tmp[0] - pos[0]) * matrix[0];
			size[1] = (tmp[1] - pos[1]) * -matrix[5];
			size[2] = (tmp[2] - pos[2]) * matrix[10];
			output = TRUE;
		}
	}



	if(size[0] < 0)
	{
		size[0] = -size[0];
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 0)
					grab[i].part = 1; 
				else if(grab[i].part == 1)
					grab[i].part = 0; 
				else if(grab[i].part == 6)
					grab[i].part = 7; 
				else if(grab[i].part == 7)
					grab[i].part = 6; 
				else if(grab[i].part == 8)
					grab[i].part = 9; 
				else if(grab[i].part == 9)
					grab[i].part = 8; 
				else if(grab[i].part == 10)
					grab[i].part = 11; 
				else if(grab[i].part == 11)
					grab[i].part = 10; 
				else if(grab[i].part == 12)
					grab[i].part = 13; 
				else if(grab[i].part == 13)
					grab[i].part = 12; 

				else if(grab[i].part == 18)
					grab[i].part = 19;
				else if(grab[i].part == 19)
					grab[i].part = 18; 	
				else if(grab[i].part == 20)
					grab[i].part = 21;
				else if(grab[i].part == 21)
					grab[i].part = 20; 
				else if(grab[i].part == 22)
					grab[i].part = 23;
				else if(grab[i].part == 23)
					grab[i].part = 22; 
				else if(grab[i].part == 24)
					grab[i].part = 25;
				else if(grab[i].part == 25)
					grab[i].part = 24; 
			}
		}
	}
	if(size[1] < 0)
	{
		size[1] = -size[1];
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 2)
					grab[i].part = 3; 
				else if(grab[i].part == 3)
					grab[i].part = 2; 
				else if(grab[i].part == 6)
					grab[i].part = 9; 
				else if(grab[i].part == 9)
					grab[i].part = 6; 
				else if(grab[i].part == 7)
					grab[i].part = 8; 
				else if(grab[i].part == 8)
					grab[i].part = 7; 

				else if(grab[i].part == 14)
					grab[i].part = 15; 
				else if(grab[i].part == 15)
					grab[i].part = 14; 
				else if(grab[i].part == 16)
					grab[i].part = 17; 
				else if(grab[i].part == 17)
					grab[i].part = 16; 


				else if(grab[i].part == 18)
					grab[i].part = 20;
				else if(grab[i].part == 20)
					grab[i].part = 18; 	
				else if(grab[i].part == 19)
					grab[i].part = 21;
				else if(grab[i].part == 21)
					grab[i].part = 19; 
				else if(grab[i].part == 22)
					grab[i].part = 24;
				else if(grab[i].part == 24)
					grab[i].part = 22; 
				else if(grab[i].part == 23)
					grab[i].part = 25;
				else if(grab[i].part == 25)
					grab[i].part = 23; 
			}
		}
	}

	if(size[2] < 0)
	{
		size[2] = -size[2];
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 4)
					grab[i].part = 5; 
				else if(grab[i].part == 5)
					grab[i].part = 4; 

				else if(grab[i].part == 10)
					grab[i].part = 12; 
				else if(grab[i].part == 12)
					grab[i].part = 10; 
				else if(grab[i].part == 11)
					grab[i].part = 13; 
				else if(grab[i].part == 13)
					grab[i].part = 11; 
				else if(grab[i].part == 14)
					grab[i].part = 16; 
				else if(grab[i].part == 16)
					grab[i].part = 14; 
				else if(grab[i].part == 15)
					grab[i].part = 17; 
				else if(grab[i].part == 17)
					grab[i].part = 15; 
				else if(grab[i].part == 18)
					grab[i].part = 22;
				else if(grab[i].part == 22)
					grab[i].part = 18; 	
				else if(grab[i].part == 19)
					grab[i].part = 23;
				else if(grab[i].part == 23)
					grab[i].part = 19; 
				else if(grab[i].part == 20)
					grab[i].part = 24;
				else if(grab[i].part == 24)
					grab[i].part = 20; 
				else if(grab[i].part == 21)
					grab[i].part = 25;
				else if(grab[i].part == 25)
					grab[i].part = 21; 
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
	if(input->mode == BAM_DRAW)
		r_matrix_set(reset);
	return output;
}


boolean	seduce_manipulator_cube_cornered(BInputState *input, RMatrix *m, float *min, float *max, void *id, float *snap, boolean snap_active, float scale, float time)
{
	static SeduceManipulatorSpaceGrab *grab = NULL;
	static uint frame_id = -1;
	uint i, count, pointer_count, user_count;
	float f, tmp[3], matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, center[3];
	boolean output = FALSE;
	RMatrix *reset;
	if(m == NULL)
		m = r_matrix_get();
	if(input->mode == BAM_DRAW)
	{
		reset = r_matrix_get();
		r_matrix_set(m);
	}

	center[0] = (min[0] + max[0]) * 0.5;
	center[1] = (min[1] + max[1]) * 0.5;
	center[2] = (min[2] + max[2]) * 0.5;
	r_matrix_projection_screenf(m, tmp, center[0], center[1], center[2]);
	scale *= -tmp[2] * 0.025;

	pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
	user_count = betray_support_functionality(B_SF_USER_COUNT_MAX);

	if(grab == NULL)
	{
		count = betray_support_functionality(B_SF_USER_COUNT_MAX);
		count += betray_support_functionality(B_SF_POINTER_COUNT_MAX);
		grab = malloc((sizeof *grab) * count);
		for(i = 0; i < count; i++)
		{
			grab[i].id = NULL;
			r_matrix_identity(&grab[i].matrix);
		}
	}

	time = (1.0 - time);
	time = time * time * 0.3 * -tmp[2];
	tmp[0] = max[0] + time;
	tmp[1] = center[1];
	tmp[2] = center[2];
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 0, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		max[0] = tmp[0];
		output = TRUE;
	}
	tmp[0] = min[0] - time;
	tmp[1] = center[1];
	tmp[2] = center[2];

	matrix[0] = -1;
	matrix[5] = -1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 1, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		min[0] = tmp[0];
		output = TRUE;
	}

	tmp[0] = center[0];
	tmp[1] = max[1] + time;
	tmp[2] = center[2];
	matrix[0] = 0;
	matrix[1] = 1;
	matrix[4] = 1;
	matrix[5] = 0;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 2, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		max[1] = tmp[1];
		output = TRUE;
	}
	tmp[0] = center[0];
	tmp[1] = min[1] - time;
	tmp[2] = center[2];
	matrix[1] = -1;
	matrix[4] = -1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 3, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		min[1] = tmp[1];
		output = TRUE;
	}


	tmp[0] = center[0];
	tmp[1] = center[1];
	tmp[2] = max[2] + time;
	matrix[1] = 0;
	matrix[4] = 0;
	matrix[2] = 1;
	matrix[5] = 1;
	matrix[8] = 1;
	matrix[10] = 0;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 4, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		max[2] = tmp[2];
		output = TRUE;
	}
	tmp[0] = center[0];
	tmp[1] = center[1];
	tmp[2] = min[2] - time;
	matrix[2] = -1;
	matrix[5] = 1;
	matrix[8] = -1;
	if(seduce_manipulator_point_axis_internal(input, grab, m, tmp, id, 5, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SURF_3D))
	{
		min[2] = tmp[2];
		output = TRUE;
	}
	tmp[0] = max[0] + time;
	tmp[1] = max[1] + time;
	tmp[2] = center[2];
	matrix[0] = 1;
	matrix[5] = 0;
	matrix[6] = 1;
	matrix[2] = 0;
	matrix[8] = 0;
	matrix[9] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 6, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		max[0] = tmp[0];
		max[1] = tmp[1];
		output = TRUE;
	}

	tmp[0] = min[0] - time;
	tmp[1] = max[1] + time;
	tmp[2] = center[2];
	matrix[0] = -1;
	matrix[6] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 7, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		min[0] = tmp[0];
		max[1] = tmp[1];
		output = TRUE;
	}
	tmp[0] = min[0] - time;
	tmp[1] = min[1] - time;
	tmp[2] = center[2];
	matrix[0] = -1;
	matrix[6] = 1;
	matrix[9] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 8, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		min[0] = tmp[0];
		min[1] = tmp[1];
		output = TRUE;
	}	
	tmp[0] = max[0] + time;
	tmp[1] = min[1] - time;
	tmp[2] = center[2];
	matrix[0] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 9, snap, snap_active, 2, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		max[0] = tmp[0];
		min[1] = tmp[1];
		output = TRUE;
	}

	tmp[0] = max[0] + time;
	tmp[1] = center[1];
	tmp[2] = max[2] + time;
	matrix[0] = 0;
	matrix[5] = 1;
	matrix[6] = 0;
	matrix[2] = 1;
	matrix[8] = 1;
	matrix[9] = 0;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 10, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		max[0] = tmp[0];
		max[2] = tmp[2];
		output = TRUE;
	}

	tmp[0] = min[0] + time;
	tmp[1] = center[1];
	tmp[2] = max[2] + time;
	matrix[5] = 1;
	matrix[2] = 1;
	matrix[8] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 11, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		min[0] = tmp[0];
		max[2] = tmp[2];
		output = TRUE;
	}

	tmp[0] = max[0] + time;
	tmp[1] = center[1];
	tmp[2] = min[2] - time;

	matrix[5] = 1;
	matrix[2] = -1;
	matrix[8] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 12, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		max[0] = tmp[0];
		min[2] = tmp[2];
		output = TRUE;
	}

	tmp[0] = min[0] - time;
	tmp[1] = center[1];
	tmp[2] = min[2] - time;
	matrix[5] = 1;
	matrix[2] = -1;
	matrix[8] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 13, snap, snap_active, 1, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		min[0] = tmp[0];
		min[2] = tmp[2];
		output = TRUE;
	}


	tmp[0] = center[0];
	tmp[1] = max[1] + time;
	tmp[2] = max[2] + time;

	matrix[1] = 1;
	matrix[2] = 0;
	matrix[4] = 1;
	matrix[5] = 0;
	matrix[8] = 0;
	matrix[10] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 14, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		max[1] = tmp[1];
		max[2] = tmp[2];
		output = TRUE;
	}

	tmp[0] = center[0];
	tmp[1] = min[1] - time;
	tmp[2] = max[2] + time;
	matrix[1] = -1;
	matrix[4] = 1;
	matrix[10] = 1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 15, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		min[1] = tmp[1];
		max[2] = tmp[2];
		output = TRUE;
	}
	tmp[0] = center[0];
	tmp[1] = max[1] + time;
	tmp[2] = min[2] - time;
	matrix[1] = 1;
	matrix[4] = 1;
	matrix[10] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 16, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		max[1] = tmp[1];
		min[2] = tmp[2];
		output = TRUE;
	}
	tmp[0] = center[0];
	tmp[1] = min[1] - time;
	tmp[2] = min[2] - time;
	matrix[1] = -1;
	matrix[4] = 1;
	matrix[10] = -1;
	if(seduce_manipulator_point_plane_internal(input, grab, m, tmp, id, 17, snap, snap_active, 0, scale, matrix, SUI_3D_OBJECT_HANDLE_SIDE_3D))
	{
		min[1] = tmp[1];
		min[2] = tmp[2];
		output = TRUE;
	}
	matrix[1] = 0;
	matrix[4] = 0;
	matrix[8] = 0;
	for(i = 0; i < 8; i++)
	{
		matrix[0] = tmp[0] = (float)(i % 2) * 2.0 - 1.0;
		matrix[5] = tmp[1] = (float)((i / 2) % 2) * -2.0 + 1.0;
		matrix[10] = tmp[2] = (float)((i / 4) % 2) * 2.0 - 1.0;
		if(matrix[0] < 0)
			tmp[0] = min[0] - time;
		else
			tmp[0] = max[0] + time;
		if(-matrix[5] < 0)
			tmp[1] = min[1] - time;
		else
			tmp[1] = max[1] + time;
		if(matrix[10] < 0)
			tmp[2] = min[2] - time;
		else
			tmp[2] = max[2] + time;
		if(seduce_manipulator_point_pos_internal(input, grab, m, tmp, id, 18 + i, snap, snap_active, scale, matrix, SUI_3D_OBJECT_HANDLE_CORNER_3D))
		{
			if(matrix[0] < 0)
				min[0] = tmp[0];
			else
				max[0] = tmp[0];
			if(-matrix[5] < 0)
				min[1] = tmp[1];
			else
				max[1] = tmp[1];
			if(matrix[10] < 0)
				min[2] = tmp[2];
			else
				max[2] = tmp[2];
			output = TRUE;
		}
	}



	if(max[0] < min[0])
	{
		f = max[0];
		max[0] = min[0];
		min[0] = f;
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 0)
					grab[i].part = 1; 
				else if(grab[i].part == 1)
					grab[i].part = 0; 
				else if(grab[i].part == 6)
					grab[i].part = 7; 
				else if(grab[i].part == 7)
					grab[i].part = 6; 

				else if(grab[i].part == 8)
					grab[i].part = 9; 
				else if(grab[i].part == 9)
					grab[i].part = 8; 

				else if(grab[i].part == 10)
					grab[i].part = 11; 
				else if(grab[i].part == 11)
					grab[i].part = 10; 

				else if(grab[i].part == 12)
					grab[i].part = 13; 
				else if(grab[i].part == 13)
					grab[i].part = 12; 

				else if(grab[i].part == 18)
					grab[i].part = 19;
				else if(grab[i].part == 19)
					grab[i].part = 18; 	
				else if(grab[i].part == 20)
					grab[i].part = 21;
				else if(grab[i].part == 21)
					grab[i].part = 20; 
				else if(grab[i].part == 22)
					grab[i].part = 23;
				else if(grab[i].part == 23)
					grab[i].part = 22; 
				else if(grab[i].part == 24)
					grab[i].part = 25;
				else if(grab[i].part == 25)
					grab[i].part = 24; 
			}
		}
	}
	if(max[1] < min[1])
	{
		f = max[1];
		max[1] = min[1];
		min[1] = f;
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 2)
					grab[i].part = 3; 
				else if(grab[i].part == 3)
					grab[i].part = 2; 
				else if(grab[i].part == 6)
					grab[i].part = 9; 
				else if(grab[i].part == 9)
					grab[i].part = 6; 
				else if(grab[i].part == 7)
					grab[i].part = 8; 
				else if(grab[i].part == 8)
					grab[i].part = 7; 

				else if(grab[i].part == 14)
					grab[i].part = 15; 
				else if(grab[i].part == 15)
					grab[i].part = 14; 
				else if(grab[i].part == 16)
					grab[i].part = 17; 
				else if(grab[i].part == 17)
					grab[i].part = 16; 


				else if(grab[i].part == 18)
					grab[i].part = 20;
				else if(grab[i].part == 20)
					grab[i].part = 18; 	
				else if(grab[i].part == 19)
					grab[i].part = 21;
				else if(grab[i].part == 21)
					grab[i].part = 19; 
				else if(grab[i].part == 22)
					grab[i].part = 24;
				else if(grab[i].part == 24)
					grab[i].part = 22; 
				else if(grab[i].part == 23)
					grab[i].part = 25;
				else if(grab[i].part == 25)
					grab[i].part = 23; 
			}
		}
	}

	if(max[2] < min[2])
	{
		f = max[2];
		max[2] = min[2];
		min[2] = f;
		for(i = 0; i < pointer_count + user_count; i++)
		{
			if(grab[i].id == id)
			{
				if(grab[i].part == 4)
					grab[i].part = 5; 
				else if(grab[i].part == 5)
					grab[i].part = 4; 

				else if(grab[i].part == 10)
					grab[i].part = 12; 
				else if(grab[i].part == 12)
					grab[i].part = 10; 
				else if(grab[i].part == 11)
					grab[i].part = 13; 
				else if(grab[i].part == 13)
					grab[i].part = 11; 
				else if(grab[i].part == 14)
					grab[i].part = 16; 
				else if(grab[i].part == 16)
					grab[i].part = 14; 
				else if(grab[i].part == 15)
					grab[i].part = 17; 
				else if(grab[i].part == 17)
					grab[i].part = 15; 
				else if(grab[i].part == 18)
					grab[i].part = 22;
				else if(grab[i].part == 22)
					grab[i].part = 18; 	
				else if(grab[i].part == 19)
					grab[i].part = 23;
				else if(grab[i].part == 23)
					grab[i].part = 19; 
				else if(grab[i].part == 20)
					grab[i].part = 24;
				else if(grab[i].part == 24)
					grab[i].part = 20; 
				else if(grab[i].part == 21)
					grab[i].part = 25;
				else if(grab[i].part == 25)
					grab[i].part = 21; 
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
	if(input->mode == BAM_DRAW)
		r_matrix_set(reset);
	return output;
}
