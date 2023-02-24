#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "betray.h"

#include "Cube.h"

void b_test_input_handler(BInputState *input, void *user_pointer)
{
	if(input->mode == BAM_MAIN)
		Cube_update();
	if(input->mode == BAM_DRAW)
	{
		Cube_prepare();
		Cube_draw();
	}
}

int main(int argc, char **argv)
{
	uint x, y;
	betray_init(B_CT_OPENGL_OR_ES, argc, argv, 0, 0, 1, TRUE, "Betray Relinquish Test Application");
	betray_action_func_set(b_test_input_handler, NULL);
	betray_screen_mode_get(&x, &y, NULL);
	Cube_setupGL((double)x, (double)y);
	betray_launch_main_loop(); 
	Cube_tearDownGL();
	return TRUE;
}
