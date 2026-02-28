#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "betray.h"
#include "relinquish.h" /*little opengl wrapper i use, in the case only to draw lines*/

uint b_test_sound = 0;

void b_test_draw_box(float x, float y, float z, float red, float green, float blue)
{
	float array[2 * 3 * 4 * 3] = {
		0.1, 0.1, 0.1, -0.1, 0.1, 0.1,
		0.1, -0.1, 0.1, -0.1, -0.1, 0.1,
		0.1, -0.1, -0.1, -0.1, -0.1, -0.1,
		0.1, 0.1, -0.1, -0.1, 0.1, -0.1,
		0.1, 0.1, 0.1, 0.1, -0.1, 0.1,
		0.1, 0.1, -0.1, 0.1, -0.1, -0.1,
		-0.1, 0.1, -0.1, -0.1, -0.1, -0.1,
		-0.1, 0.1, 0.1, -0.1, -0.1, 0.1,
		0.1, 0.1, 0.1, 0.1, 0.1, -0.1, 
		-0.1, 0.1, 0.1, -0.1, 0.1, -0.1,
		-0.1, -0.1, 0.1, -0.1, -0.1, -0.1,
		0.1, -0.1, 0.1, 0.1, -0.1, -0.1};
	uint i;
	for(i = 0; i < 2 * 3 * 4 * 3; i += 6)
		r_primitive_line_3d(array[i + 0] + x, array[i + 1] + y, array[i + 2] + z, array[i + 3] + x, array[i + 4] + y, array[i + 5] + z, red, green, blue, 1.0);
}

void b_test_draw_circle(float red, float green, float blue)
{
	uint i;
	for(i = 0; i < 64; i++)
		r_primitive_line_2d(sinf((double)i / 64.0 * 2.0 * PI),
							cosf((double)i / 64.0 * 2.0 * PI),
							sinf((double)(i + 1) / 64.0 * 2.0 * PI),
							cosf((double)(i + 1) / 64.0 * 2.0 * PI), red, green, blue, 1.0);
}

void b_test_context_update(void)
{
//	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	r_init((void *(*)(const char*proc))betray_gl_proc_address_get());
}

float my_test_value = 0;

void b_test_input_handler(BInputState *input, void *user_pointer)
{
	static float time = 0, pos[3] = {0, 0, -1}, vec[3] = {0, 0, 0};
	static uint sound_source = -1;
	uint i, j, k;
	float f, aspect;

	if(input->mode == BAM_DRAW)
	{
		static uint random = 0;
		uint x, y, forward_axis, up_axis;
		float aspect, view[3] = { 0, 0, 1 }, matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
//		glClearColor(0.2, 0.2, 0.2, 0);
	//	betray_view_vantage(view);
		aspect = betray_screen_mode_get(&x, &y, NULL);
		r_viewport(0, 0, x, y);
		r_matrix_set(NULL);
		r_matrix_identity(NULL);
		r_matrix_frustum(NULL, -0.01 - view[0] * 0.01, 0.01 - view[0] * 0.01, -0.01 * aspect - view[1] * 0.01, 0.01 * aspect - view[1] * 0.01, 0.01 * view[2], 100.0); /* set frustum */
//		glClearColor(0.2 * f_randf(random), 0.2 * f_randf(random + 1), 0.2 * f_randf(random + 2), 1);
		random++;




		/* Add direction matrix so that its possible to implement different view direction*/
	//	betray_view_direction(matrix); 
		r_matrix_matrix_mult(NULL, matrix);
		r_matrix_translate(NULL, -view[0], -view[1], -view[2]); /* move the camera if betray wants us to. */
	//	r_matrix_rotate(NULL, input->minute_time * 360 * 10.0, 1, 1, 1);
	// 	r_matrix_rotate(NULL, random, 1, 1, 1);
		for(forward_axis = 0; forward_axis < input->axis_count; forward_axis++)
			if(input->axis[forward_axis].axis_count == 3 && input->axis[forward_axis].axis_type == B_AXIS_SCREEN_FORWARD)
				break;
		for(up_axis = 0; up_axis < input->axis_count; up_axis++)
			if(input->axis[up_axis].axis_count == 3 && input->axis[up_axis].axis_type == B_AXIS_SCREEN_UP)
				break;
		if(forward_axis < input->axis_count && up_axis < input->axis_count)
		{
			float m[16], output[16];
			f_matrixzyf(m, NULL, input->axis[forward_axis].axis, input->axis[up_axis].axis); 
			f_matrix_reverse4f(output, m); 
			r_matrix_matrix_mult(NULL, output);
		}
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		/* Lets draw some cubes so that we have something to look at*/
		r_matrix_push(NULL);
	//	r_matrix_translate(NULL, input->pointers[0].pointer_x, input->pointers[0].pointer_y, 0);
		for(i = 0; i < 11; i++)
		{
			for(j = 0; j < 11; j++)
			{
				for(k = 0; k < 6; k++)
				{
			//		r_matrix_push(NULL);
			//		r_matrix_translate(NULL, ((float)i - 5.0), ((float)j - 5.0), ((float)k - 5.0));
					b_test_draw_box(((float)i - 5.0), ((float)j - 5.0), ((float)k - 5.0), (float)i / 11, (float)j / 11, (float)k / 11);
			//		r_matrix_pop(NULL);
				}
			}
		}
		r_primitive_line_flush();
		r_matrix_pop(NULL);

		for(i = 0; i < input->pointer_count; i++)
		{
			/* draw a circle around each pointer */

			r_matrix_push(NULL);
			r_matrix_translate(NULL, input->pointers[i].pointer_x, input->pointers[i].pointer_y, 0);
			r_matrix_scale(NULL, 0.05, 0.05, 0.05);
			b_test_draw_circle(1, 1, 1);
			r_primitive_line_flush();
			r_matrix_pop(NULL);

			/* draw the state of the buttons (right and middle button SHOULD be switched)*/

			for(j = 0; j < input->pointers[i].button_count; j++)
			{
				r_matrix_push(NULL);
				r_matrix_translate(NULL, input->pointers[i].pointer_x + ((float)j - 0.5 * (3.0 - 1.0)) * 0.05, input->pointers[i].pointer_y - 0.05 - 0.1 / 3.0, 0);
				if(input->pointers[i].button[j] && !input->pointers[i].button[j])
				{
					r_matrix_push(NULL);
					r_matrix_scale(NULL, 0.1 / 3.0, 0.1 / 3.0, 0.05);
					b_test_draw_circle(1, 1, 1);
					r_primitive_line_flush();
					r_matrix_pop(NULL);
				}
				r_matrix_scale(NULL, 0.05 / 3.0, 0.05 / 3.0, 0.05);
				if(input->pointers[i].button[j])
					b_test_draw_circle(0, 1, 0);
				else
					b_test_draw_circle(1, 0, 0);
				r_primitive_line_flush();
				r_matrix_pop(NULL);
			}
				r_primitive_line_2d(input->pointers[i].pointer_x, input->pointers[i].pointer_y + 0.1,
									input->pointers[i].pointer_x - input->pointers[i].delta_pointer_x, 
									input->pointers[i].pointer_y - input->pointers[i].delta_pointer_y + 0.1, 0, 1, 1, 1);
			/* draw a blue line from the pointer to the position of the down click */
			if(input->pointers[i].button[0])
			{
				r_primitive_line_2d(input->pointers[i].pointer_x, input->pointers[i].pointer_y,
					input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0], 0, 0, 1, 1);
				r_primitive_line_flush();
			}
		}
		aspect = betray_screen_mode_get(NULL, NULL, NULL); 
		r_primitive_line_2d(1.0 - 0.04, aspect - 0.04, 1.0 - 0.04, -aspect + 0.04, 0, 0, 0, 1);
		r_primitive_line_2d(-1.0 + 0.04, aspect - 0.04, -1.0 + 0.04, -aspect + 0.04, 0, 0, 0, 1);

		r_primitive_line_2d(1.0 - 0.04, -aspect + 0.04, -1.0 + 0.04, -aspect + 0.04, 0, 0, 0, 1);
		r_primitive_line_2d(1.0 - 0.04, aspect - 0.04, -1.0 + 0.04, aspect - 0.04, 0, 0, 0, 1);
		r_primitive_line_flush();
 /* draw all axis */
		f = -0.85;
		for(i = 0; i < input->axis_count; i++) /* how many axis do we have */
		{
			switch(input->axis[i].axis_count) /* how many dimentions does the axis have */
			{
				case 1 :
					r_primitive_line_2d(f + 0.01, -0.3 + 0.11,
									f + 0.01, -0.3 - 0.11, 0.4, 0.4, 0.4, 1);
					r_primitive_line_2d(f - 0.01, -0.3 + 0.11,
									f - 0.01, -0.3 - 0.11, 0.4, 0.4, 0.4, 1);
					r_primitive_line_2d(f + 0.01, -0.3 + 0.11,
									f - 0.01, -0.3 + 0.11, 0.4, 0.4, 0.4, 1);
					r_primitive_line_2d(f + 0.01, -0.3 - 0.11,
									f - 0.01, -0.3 - 0.11, 0.4, 0.4, 0.4, 1);
					r_primitive_line_2d(f, -0.3, f, input->axis[i].axis[0] * 0.1 - 0.3, 1, 1, 1, 1);
					r_primitive_line_flush();
				break;
				case 2 :
				case 3 :
					r_matrix_push(NULL);
					r_matrix_translate(NULL, f, -0.3, 0);
					r_matrix_scale(NULL, 0.1, 0.1, 1.0);
					b_test_draw_circle(1, 1, 1);
					r_primitive_line_2d(0, 0, input->axis[i].axis[0], input->axis[i].axis[1], 1, 1, 1, 1);
					r_primitive_line_flush();
					r_matrix_pop(NULL);
				break;
			}
			f += 0.25;
		}
	}
	if(input->mode == BAM_DRAW)
		r_primitive_line_flush();
}

int main(int argc, char **argv)
{
	betray_init(B_CT_OPENGL_OR_ES, argc, argv, 100, 100, 1, FALSE, "Betray Relinquish Test Application");
	r_init((void *(*)(const char *))betray_gl_proc_address_get());
	betray_action_func_set(b_test_input_handler, NULL);
	betray_launch_main_loop();
	return TRUE;
}