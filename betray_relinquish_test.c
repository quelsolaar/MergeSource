#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "betray.h"
#include "relinquish.h" /*little opengl wrapper i use, in the case only to draw lines*/

uint b_test_sound = 0;

void b_test_draw_box(float red, float green, float blue)
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
		r_primitive_line_3d(array[i + 0], array[i + 1], array[i + 2], array[i + 3], array[i + 4], array[i + 5], red, green, blue, 1.0);
	r_primitive_line_flush();
}

void b_test_draw_circle(float red, float green, float blue)
{
	uint i;
	for(i = 0; i < 64; i++)
		r_primitive_line_2d(sin((double)i / 64.0 * 2.0 * PI),
							cos((double)i / 64.0 * 2.0 * PI),
							sin((double)(i + 1) / 64.0 * 2.0 * PI),
							cos((double)(i + 1) / 64.0 * 2.0 * PI), red, green, blue, 1.0);
}

/*
Set up a function that Betray can call if the context is lost
*/

void b_test_context_update(void)
{
//	glMatrixMode(GL_MODELVIEW);	
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//	glEnableClientState(GL_VERTEX_ARRAY);
	r_init((void*(*)(const char*))betray_gl_proc_address_get());
}

/*
Make a button!
*/

boolean b_test_button(BInputState *input, float pos_x, float pos_y, float red, float green, float blue)
{
	if(input->mode == BAM_DRAW)
	{
		r_primitive_line_2d(pos_x - 0.2, pos_y + 0.05, pos_x + 0.2, pos_y + 0.05, red, green, blue, 1.0); 
		r_primitive_line_2d(pos_x - 0.2, pos_y - 0.05, pos_x + 0.2, pos_y - 0.05, red, green, blue, 1.0);	 
		r_primitive_line_2d(pos_x + 0.2, pos_y - 0.05, pos_x + 0.2, pos_y + 0.05, red, green, blue, 1.0);	 
		r_primitive_line_2d(pos_x - 0.2, pos_y - 0.05, pos_x - 0.2, pos_y + 0.05, red, green, blue, 1.0);	
	}
	if(input->mode == BAM_EVENT && input->pointer_count > 0)
	{
		if(!input->pointers[0].button[0] && input->pointers[0].last_button[0])
		{
			if(input->pointers[0].pointer_x - 0.2 < pos_x &&
				input->pointers[0].pointer_x + 0.2 > pos_x &&
				input->pointers[0].pointer_y - 0.05 < pos_y &&
				input->pointers[0].pointer_y + 0.05 > pos_y)
			{
				if(input->pointers[0].click_pointer_x[0] - 0.2 < pos_x &&
					input->pointers[0].click_pointer_x[0] + 0.2 > pos_x &&
					input->pointers[0].click_pointer_y[0] - 0.05 < pos_y &&
					input->pointers[0].click_pointer_y[0] + 0.05 > pos_y)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/* This is the main loop of the application. It can be triggerd for 3 reasons:
 BAM_MAIN Time has progressed
 BAM_DRAW TThe screen needs to be drawn
 BAM_EVENT Input needs to be parsed
*/

float my_test_value = 0;

void b_test_input_handler(BInputState *input, void *user_pointer)
{
	static float time = 0, pos[3] = {0, 0, -1}, vec[3] = {0, 0, 0};
	static uint sound_source = -1;
	uint i, j, k;
	float f, aspect,  matrix[16];

	/* Move arround a 3D sound source. */
	if(input->mode == BAM_MAIN)
	{
		time += input->delta_time * 0.2;

		f_wiggle3df(pos, time, 2.0);
		if(input->pointer_count > 0)
		{
			vec[0] = vec[0] * 0.99 + 0.01 * input->pointers[0].delta_pointer_x / input->delta_time;
			vec[1] = vec[1] * 0.99 + 0.01 * input->pointers[0].delta_pointer_y / input->delta_time;
			vec[2] = 0;
			pos[0] = input->pointers[0].pointer_x; 
			pos[1] = input->pointers[0].pointer_y;
			pos[2] = -0.2; 
		}

		if(input->pointers[0].button[1])
		{
			if(sound_source == -1)
				sound_source = betray_audio_sound_play(b_test_sound, pos, vec, 1, 0.8, TRUE, FALSE, FALSE); /* play a sound (ambient equals fixed position to head)*/
			betray_audio_sound_set(sound_source, pos, vec, 1, 0.8, TRUE, FALSE); /* change settings for an already playing sound*/
		}
		if(!input->pointers[0].button[1] && sound_source != -1)
		{
			betray_audio_sound_stop(sound_source);
			sound_source = -1;
		}
	}

	if(input->mode == BAM_EVENT)
	{
	//	betray_settings_number_float_set(3, input->pointers[0].pointer_x);
		/* Lets read all the incomming button events*/
		for(i = 0; i < input->button_event_count; i++)
		{
			char button_name[64];
			/* look up the name of a button press (can be slow) */
			if(betray_button_get_name(-1, input->button_event[i].button, button_name, 64))
			{
				if(input->button_event[i].state)
					printf("Button press %s\n", button_name);
				else
					printf("Button release %s\n", button_name);
			}
		}
	}

	/* Use the F keys to toggle settings */
	if(input->mode == BAM_EVENT)
		for(i = 0; i < 12; i++)	
			if(betray_button_get(-1, BETRAY_BUTTON_F1 + i))
				for(k = j = 0; j < betray_settings_count(); j++)
					if(BETRAY_ST_TOGGLE == betray_settings_type(j))
						if(k++ == i)
							betray_settings_toggle_set(j, !betray_settings_toggle_get(j));

	if(input->mode == BAM_DRAW)
	{
		uint x, y;
		float aspect, view[3] = {0, 0, 1}, matrix[16];
//		glClearColor(0.2, 0.2, 0.2, 0);
	//	betray_view_vantage(view);
		aspect = betray_screen_mode_get(&x, &y, NULL);
		r_viewport(0, 0, x, y);
		r_matrix_set(NULL);
		r_matrix_identity(NULL);
		betray_view_vantage(view);
		r_matrix_frustum(NULL, -0.01 - view[0] * 0.01, 0.01 - view[0] * 0.01, -0.01 * aspect - view[1] * 0.01, 0.01 * aspect - view[1] * 0.01, 0.01 * view[2], 100.0); /* set frustum */
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		/* Add direction matrix so that its possible to implement different view direction*/
		betray_view_direction(matrix); 
		r_matrix_matrix_mult(NULL, matrix);

//		r_matrix_rotate(NULL, input->minute_time * 360 * 10.0, 1, 1, 1);

		/* Lets draw some cubes so that we have something to look at*/
		r_matrix_push(NULL);
		r_matrix_translate(NULL, input->pointers[0].pointer_x, input->pointers[0].pointer_y, 0);

		for(i = 0; i < 11; i++)
		{
			for(j = 0; j < 11; j++)
			{
				for(k = 0; k < 6; k++)
				{
					r_matrix_push(NULL);
					r_matrix_translate(NULL, sin(my_test_value) + ((float)i - 5.0), ((float)j - 5.0), ((float)k - 5.0));
					b_test_draw_box((float)i / 11, (float)j / 11, (float)k / 11);
					r_matrix_pop(NULL);
				}
			}
		}
		r_matrix_pop(NULL);
		
		/* draw out any data comming from the Microphone*/
		
	/*	if(betray_audio_read_units() > 0)
		{
			static int16 *buffer = NULL;
			if(buffer == NULL)
				buffer = (int16 *)malloc((sizeof *buffer) * 1000 * 2);
			j = betray_audio_read(0, buffer, BETRAY_TYPE_INT16, 1000);

			for(i = 3; i < j; i++)
				r_primitive_line_2d((float)i * 1.0 / (float)j - 0.5,
									(float)buffer[i - 1] / 16000.0,
									(float)(i + 1) * 1.0 / (float)j - 0.5,
									(float)buffer[i] / 16000.0, 0, 1, 1, 1);
			r_primitive_line_flush();
		}*/

		for(i = 0; i < input->pointer_count; i++)
		{
			/* draw a circle around each pointer */

			r_matrix_push(NULL);
			r_matrix_translate(NULL, input->pointers[i].pointer_x, input->pointers[i].pointer_y, input->pointers[i].pointer_z);
			r_matrix_scale(NULL, 0.05, 0.05, 0.05);
			b_test_draw_circle(1, 1, 1);
			r_primitive_line_flush();
			r_matrix_pop(NULL);




			
			r_primitive_line_3d(input->pointers[i].pointer_x, input->pointers[i].pointer_y, input->pointers[i].pointer_z,
								input->pointers[i].origin[0], input->pointers[i].origin[1], input->pointers[i].origin[2], 0, 1, 1, 1);
			r_primitive_line_flush();

			/* draw the state of the buttons (right and middle button SHOULD be switched)*/

			for(j = 0; j < input->pointers[i].button_count; j++)
			{
				r_matrix_push(NULL);
				r_matrix_translate(NULL, input->pointers[i].pointer_x + ((float)j - 0.5 * (3.0 - 1.0)) * 0.05, input->pointers[i].pointer_y - 0.05 - 0.1 / 3.0, input->pointers[i].pointer_z);
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
				r_primitive_line_3d(input->pointers[i].pointer_x, input->pointers[i].pointer_y + 0.1, input->pointers[i].pointer_z,
									input->pointers[i].pointer_x - input->pointers[i].delta_pointer_x, 
									input->pointers[i].pointer_y - input->pointers[i].delta_pointer_y + 0.1, input->pointers[i].pointer_z, 0, 1, 1, 1);
			/* draw a blue line from the pointer to the position of the down click */
			if(input->pointers[i].button[0])
			{
				r_primitive_line_2d(input->pointers[i].pointer_x, input->pointers[i].pointer_y,
					input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0], 0, 0, 1, 1);
				r_primitive_line_flush();
			}
		}


		r_matrix_translate(NULL, 0, 0, -1); /* move the camera if betray wants us to. */

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
/* Lets draw a few buttons */

/* red button shows a save requester */
	if(b_test_button(input, 0.7, 0.4, 1, 0, 0))
		betray_requester_save(NULL, 0, NULL);
/* yellow button shows a load requester */
	if(b_test_button(input, 0.7, 0.2, 1, 1, 0))
		betray_requester_load(NULL, 0, NULL);
/* cyan button shanges screen mode to a windowed mode */
	if(b_test_button(input, 0.7, -0.2, 0, 1, 1))
		betray_screen_mode_set(920, 580, 1, FALSE);
/* blue button shows a fullscreen mode with the same resolution as the desktop */	
	if(b_test_button(input, 0.7, -0.4, 0, 0, 1))
		betray_screen_mode_set(0, 0, 1, TRUE); 

	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < betray_settings_count(); i++)
		{
			if(BETRAY_ST_SELECT == betray_settings_type(i))
			{
				//betray_settings_name(uint id);
				for(j = 0; j < betray_settings_select_count_get(i); j++)
					if(betray_button_get(-1, BETRAY_BUTTON_F1 + j))
						betray_settings_select_set(i, j);
			}
		}
		if(betray_button_get(-1, BETRAY_BUTTON_Q))
			exit(0);
	}


/* flush all line draws! */
	if(input->mode == BAM_DRAW)
		r_primitive_line_flush();
}

int main(int argc, char **argv)
{
	float f;
	int16 *buffer;
	uint i;
	char *clip_buffer;
	/* Check if betray implementation supports OpenGL or OpenGLES, Relinqish runs on both, so any is fine*/
	if(!betray_support_context(B_CT_OPENGL_OR_ES))
	{
		printf("betray_relinquish_test.c Requires B_CT_OPENGL or B_CT_OPENGLES2 to be available, Program exit.\n");
		exit(0);
	}
	/* initialize betray by opening a screen */
//	betray_init(B_CT_OPENGL_OR_ES, argc, argv, 0, 0, 1, TRUE, "Betray Relinquish Test Application");
	betray_init(B_CT_OPENGL_OR_ES, argc, argv, 800, 600, 0, FALSE, "Betray Relinquish Test Application");
	
	/* Accessing clip board */

	if(betray_support_functionality(B_SF_CLIPBOARD))
	{
		clip_buffer = betray_clipboard_get();
		printf("reading out %s form the clipboard\n", clip_buffer);
		free(clip_buffer); // dont forget th free the buffer!

		/* insert something in to clipboard*/
		betray_clipboard_set("Hey Boy, hey girl");
	}

	/* initialize Relinqish */

	b_test_context_update();

	/* Use queryies to find out what the implementation of the api can do */

	printf("Betray implementation supports %u pointers\n", betray_support_functionality(B_SF_POINTER_COUNT_MAX));
	

	if(betray_support_functionality(B_SF_FULLSCREEN))
		printf("Betray implementation supports fullscreen\n");
	else
		printf("Betray implementation does not support fullscreen\n");

	if(betray_support_functionality(B_SF_VIEW_POSITION))
		printf("Betray implementation supports view position\n");
	else
		printf("Betray implementation does not support position\n");

	if(betray_support_functionality(B_SF_VIEW_ROTATION))
		printf("Betray implementation supports view rotation\n");
	else
		printf("Betray implementation does not support view rotation\n");

	if(betray_support_functionality(B_SF_MOUSE_WARP))
		printf("Betray implementation supports mouse warp\n");
	else
		printf("Betray implementation does not support mouse warp\n");

	if(betray_support_functionality(B_SF_EXECUTE))
		printf("Betray implementation supports Execute\n");
	else
		printf("Betray implementation does not support Execute\n");

	if(betray_support_functionality(B_SF_REQUESTER))
		printf("Betray implementation supports Requester\n");
	else
		printf("Betray implementation does not support Requester\n");

	if(betray_support_functionality(B_SF_CLIPBOARD))
		printf("Betray implementation supports Clipboard\n");
	else
		printf("Betray implementation does not support Clipboard\n");


	/* lets make some noise and store it in a betray buffer */

	buffer = (int16 *)malloc((sizeof *buffer) * 32000);
	for(i = 0; i < 32000; i++)
	{
		f = i / 32000.0;
		f = f * (1.0 - f) * 1.0;
		f = sin((float)i * PI * 2.0 * 200.0 / 32000.0);
	//	f *= f_noise((float)i * 0.03);
		buffer[i] = (int16)(f * 32000);
	}
	b_test_sound = betray_audio_sound_create(BETRAY_TYPE_INT16, 1, 32000, 44100, buffer, "tone"); /* load sound data in to betray*/
 
	/*Set the context update fun (that will be called if the openGL context is lost)*/

	betray_gl_context_update_func_set(b_test_context_update);

	/* List all the settings in the Betray API */
	
	for(i = 0; i < betray_settings_count(); i++)
		printf("%u:%s - %u\n", i, betray_settings_name(i), betray_settings_type(i));

	/* Set the action funtio that will act as the main loop for the aplication */

	betray_action_func_set(b_test_input_handler, NULL);
	
	/* run the main loop (b_test_input_handler)*/

	betray_launch_main_loop();
	return TRUE;
}
/*
int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
	char *argument = "file.exe";
//	my_nCmdShow = nCmdShow;
	main(1, &argument);
    return TRUE;
}*/