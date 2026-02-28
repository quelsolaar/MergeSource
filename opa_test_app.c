#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "betray.h"
#include "relinquish.h"

typedef struct{
	float start[2];
	float end[2];
	float color[3];
}Line;

typedef struct{
	Line *lines;
	uint line_count;
	float background_color[3];
}LineStorage;

void opa_demo_handler(BInputState *input, LineStorage *lines) /* This is our main loop */ 
{
	RMatrix matrix;
	uint i, x, y;
	float view[3] = {0.0, 0.0, 1}, aspect, origo[3] = {0, 0, 0};

	if(input->mode == BAM_DRAW) /* We should draw something */
	{
		r_matrix_identity(&matrix); /* clear a matric*/
		aspect = betray_screen_mode_get(&x, &y, NULL); /* get resolution and aspect from Betray */
		r_matrix_frustum(&matrix, -0.01, 0.01, -0.01 * aspect, 0.01 * aspect, 0.01, 100.0); /* set frustum */
		r_matrix_translate(&matrix, 0, 0, -1.0); /* move back to make xy plane visable*/
		r_matrix_set(&matrix); /* set this matrix as the active matrix */ 
		r_viewport(0, 0, x, y);

		glClearColor(lines->background_color[0], lines->background_color[1], lines->background_color[2], 0); /* clear the screen */
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		for(i = 0; i < lines->line_count; i++) /* draw all the colored lines */
			r_primitive_line_2d(lines->lines[i].start[0], lines->lines[i].start[1], 
								lines->lines[i].end[0], lines->lines[i].end[1], 
								lines->lines[i].color[0], lines->lines[i].color[1], lines->lines[i].color[2], 1.0);
		r_primitive_line_flush();
	}

	opa_watch(lines, "LineStorage", "./opa_test_app.c");
}

int main(int argc, char **argv)
{
	LineStorage lines;
	betray_init(B_CT_OPENGL, argc, argv, 1024, 768, 16, FALSE, "Test application for OPA"); /* initialize a window */ 
	r_init(betray_gl_proc_address_get()); /* Initialize my GL wrapper */

	lines.lines = NULL; /* initialize NULL as our array of lines, we can allocate this using OPA */
	lines.line_count = 0; /* set the length of lines */
	lines.background_color[0] = 0.5; /* Backgroud color */
	lines.background_color[1] = 0.5;
	lines.background_color[2] = 0.5;

	betray_action_func_set(opa_demo_handler, &lines); /* set out main loop function */
	betray_launch_main_loop(); /* launch main loop function */
	return TRUE;
}
