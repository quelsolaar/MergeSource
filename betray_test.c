#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "forge.h"
#include "betray.h"

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

	glColor4f(red, green, blue, 1.0);	
	glVertexPointer(3, GL_FLOAT , 0, array);
	glDrawArrays(GL_LINES, 0, 4 * 3 * 2);
}

void b_test_draw_circle(float red, float green, float blue)
{
	float array[2 * 2 * 64];
	uint i;

	for(i = 0; i < 64; i++)
	{
		array[i * 4 + 0] = sin((double)i / 64.0 * 2.0 * PI);
		array[i * 4 + 1] = cos((double)i / 64.0 * 2.0 * PI);
		array[i * 4 + 2] = sin((double)(i + 1) / 64.0 * 2.0 * PI);
		array[i * 4 + 3] = cos((double)(i + 1) / 64.0 * 2.0 * PI);
	}
	glColor4f(red, green, blue, 1.0);	
	glVertexPointer(2, GL_FLOAT , 0, array);
	glDrawArrays(GL_LINES, 0, 64 * 2);
}


void b_test_draw_line(float start_x, float start_y, float end_x, float end_y, float red, float green, float blue)
{
	float array[4];
	uint i;
	array[0] = start_x;
	array[1] = start_y;
	array[2] = end_x;
	array[3] = end_y;
	glColor4f(red, green, blue, 1.0);	
	glVertexPointer(2, GL_FLOAT , 0, array);
	glDrawArrays(GL_LINES, 0, 2);
}

void b_test_context_update(void)
{
	glMatrixMode(GL_MODELVIEW);	
	glClearColor(0.2, 0.2, 0.2, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
}

boolean b_test_button(BInputState *input, float pos_x, float pos_y, float red, float green, float blue)
{
	if(input->mode == BAM_DRAW)
	{
		float array[2 * 2 * 4] = {-0.2, 0.05, 0.2, 0.05, -0.2, -0.05, 0.2, -0.05, 0.2, -0.05, 0.2, 0.05, -0.2, -0.05, -0.2, 0.05};
		glPushMatrix();
		glTranslatef(pos_x, pos_y, -1.0);
		glColor4f(red, green, blue, 1.0);	
		glVertexPointer(2, GL_FLOAT , 0, array);
		glDrawArrays(GL_LINES, 0, 4 * 2);
		glPopMatrix();
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

void b_test_input_handler(BInputState *input, void *user_pointer)
{
	static float time = 0, pos[3] = {0, 0, -1}, vec[3] = {0, 0, 0};
	static uint sound_source = -1, tik = 0;
	uint i, j, k;
	float f, aspect,  matrix[16];

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
			pos[2] = -0.1; 
		}

		if(sound_source == -1)
			sound_source = betray_audio_sound_play(b_test_sound, pos, vec, 1, 0.8, TRUE, FALSE, FALSE); /* play a sound (ambient equals fixed position to head)*/
		betray_audio_sound_set(sound_source, pos, vec, 1, 0.8, TRUE, FALSE); /* change settings for an already playing sound*/
	}

	if(input->mode == BAM_DRAW)
	{
		uint x, y;
		float f;
		f = betray_screen_mode_get(&x, &y, NULL);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-0.01, 0.01, -0.01 * f, 0.01 * f, 0.01, 100.0);
		glViewport(0, 0, x, y);
		glMatrixMode(GL_MODELVIEW);
        glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glPushMatrix();

		/* Add direction matrix so that its possible to implement different view direction*/
	//	betray_view_direction(matrix);
	//	glMultMatrixf(matrix);
		glTranslatef(0, 0, -1.0);



		/* Lets draw some cubes so that we have something to look at*/
		glPushMatrix();
		glRotatef(180, f_noisef(time) - 0.5, f_noisef(time + 679.3) - 0.5, f_noisef(time + 23.7) - 0.5);
		for(i = 0; i < 21; i++)
		{
			for(j = 0; j < 21; j++)
			{
				for(k = 0; k < 21; k++)
				{
					glPushMatrix();
					glTranslatef(((float)i - 10.0), ((float)j - 10.0), ((float)k - 10.0));
					b_test_draw_box((float)i / 11, (float)j / 11, (float)k / 11);
					glPopMatrix();
				}
			}
		}
		glPopMatrix();
		/* lets draw pointers */
		for(i = 0; i < input->pointer_count; i++)
		{
			/* draw a circle around each pointer */
			glPushMatrix();
			glTranslatef(input->pointers[i].pointer_x, input->pointers[i].pointer_y, 0);
			glScalef(0.05, 0.05, 0.05);
			b_test_draw_circle(1, 1, 1);
			glPopMatrix();

			/* draw the state of the buttons (right and middle button SHOULD be switched)*/

			for(j = 0; j < 3; j++)
			{
				glPushMatrix();
				glTranslatef(input->pointers[i].pointer_x + ((float)j - 0.5 * (3.0 - 1.0)) * 0.05, input->pointers[i].pointer_y - 0.05 - 0.1 / 3.0, 0);
				if(input->pointers[i].button[j] && !input->pointers[i].button[j])
				{
					glPushMatrix();
					glScalef(0.1 / 3.0, 0.1 / 3.0, 0.05);
					b_test_draw_circle(1, 1, 1);
					glPopMatrix();
				}
				glScalef(0.05 / 3.0, 0.05 / 3.0, 0.05);
				if(input->pointers[i].button[j])
					b_test_draw_circle(0, 1, 0);
				else
					b_test_draw_circle(1, 0, 0);

				glPopMatrix();
			}
			/* draw a blue line from the pointer to the position of the down click */
			if(input->pointers[i].button[0])
				b_test_draw_line(input->pointers[i].pointer_x, input->pointers[i].pointer_y,
					input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0], 0, 0, 1);
		}
		aspect = betray_screen_mode_get(NULL, NULL, NULL); 
		b_test_draw_line(1.0 - 0.04, aspect - 0.04, 1.0 - 0.04, -aspect + 0.04, 0, 0, 0);
		b_test_draw_line(-1.0 + 0.04, aspect - 0.04, -1.0 + 0.04, -aspect + 0.04, 0, 0, 0);

		b_test_draw_line(1.0 - 0.04, -aspect + 0.04, -1.0 + 0.04, -aspect + 0.04, 0, 0, 0);
		b_test_draw_line(1.0 - 0.04, aspect - 0.04, -1.0 + 0.04, aspect - 0.04, 0, 0, 0);
 /* draw all axis */
		f = -0.85;
		for(i = 0; i < input->axis_count; i++)
		{
			switch(input->axis[i].axis_count)
			{
				case 1 :
					b_test_draw_line(f + 0.01, -0.3 + 0.11,
									f + 0.01, -0.3 - 0.11, 0.4, 0.4, 0.4);
					b_test_draw_line(f - 0.01, -0.3 + 0.11,
									f - 0.01, -0.3 - 0.11, 0.4, 0.4, 0.4);
					b_test_draw_line(f + 0.01, -0.3 + 0.11,
									f - 0.01, -0.3 + 0.11, 0.4, 0.4, 0.4);
					b_test_draw_line(f + 0.01, -0.3 - 0.11,
									f - 0.01, -0.3 - 0.11, 0.4, 0.4, 0.4);
					b_test_draw_line(f, -0.3, f, input->axis[i].axis[0] * 0.1 - 0.3, 1, 1, 1);
					printf("slider %s = %f", input->axis[i].name, input->axis[i].axis[0]);
				break;
				case 2 :
				case 3 :
					glPushMatrix();
					glTranslatef(f, -0.3, 0);
					glScalef(0.1, 0.1, 1.0);
					b_test_draw_circle(1, 1, 1);
					b_test_draw_line(0, 0, input->axis[i].axis[0] * 0.1, input->axis[i].axis[1] * 0.1, 1, 1, 1);
					glPopMatrix();
				break;
			}
			f += 0.25;
		}

		glPopMatrix();
	}
/* Lets draw a few buttons */
    if(betray_button_get(-1, BETRAY_BUTTON_LEFT))
        exit(0);
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

}

int main(int argc, char **argv)
{
	float f;
	int16 *buffer;
	uint i;
	char *clip_buffer;
	/* Check if betray implementation supports OpenGL */
	if(!betray_support_context(B_CT_OPENGL))
	{
		printf("betray_test.c Requires B_CT_OPENGL to be available, Program exit.\n");
		exit(0);
	}
	/* initialize betray by opening a screen */
	betray_init(B_CT_OPENGL, argc, argv, 800, 600, 4, FALSE, "Betray Test Application");
	
	/* Accessing clip board */

	if(betray_support_functionality(B_SF_CLIPBOARD))
	{
		clip_buffer = betray_clipboard_get();
		printf("reading out %s form the clipboard", clip_buffer);
		free(clip_buffer); // dont forget th free the buffer!

		/* insert something in to clopboard*/
		betray_clipboard_set("Hey Boy hey girl");
	}

	/*Set my opengl state*/

	b_test_context_update();

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


	/* lets make some noise... */

	buffer = malloc((sizeof *buffer) * 32000);
	for(i = 0; i < 32000; i++)
	{
		f = i / 32000.0;
		f = f * (1.0 - f) * 1.0;
	//	f *= sin((float)i * 0.01);
		f *= f_noise((float)i * 0.03);
		buffer[i] = (int16)(f * 32000);
	}
	b_test_sound = betray_audio_sound_create(BETRAY_TYPE_INT16, sizeof *buffer, 32000, 44100, buffer, "tone"); /* load sound data in to betray*/
 
	/*Set the context update fun (that will be called if the openGL context is lost)*/

	betray_gl_context_update_func_set(b_test_context_update);

	/*Set the action funtio that will act as the main loop for the aplication*/
	
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