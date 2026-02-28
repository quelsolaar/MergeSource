#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "betray.h"
#include "imagine.h"

uint sound_play_id;

void b_test_input_handler(BInputState *input, void *user_pointer)
{

	if(input->mode == BAM_MAIN)
		;
	if(input->mode == BAM_EVENT)
	{
		float pos[3] = {0, 0, 1}, vector[3] = {0, 0, 0};
		pos[0] = input->pointers[0].pointer_x;
		vector[0] = input->pointers[0].delta_pointer_x * input->delta_time;
		betray_audio_sound_set(sound_play_id, pos, vector, 1.0, 1, TRUE, FALSE);
		imagine_sleepd(0.1);
	}
}

#define BETRAY_SOUND_TEST_LOOP_SIZE 102

int main(int argc, char **argv)
{
	int16 buffer[BETRAY_SOUND_TEST_LOOP_SIZE];
	uint x, y, sound_id, i;
	float origo[3] = {0, 0, 0}, vector[5] = {0, 0, 1, 0, 0};
	for(i = 0; i < BETRAY_SOUND_TEST_LOOP_SIZE; i++)
		buffer[i] = (int16)(sin((float)i * PI * 2.0 / BETRAY_SOUND_TEST_LOOP_SIZE) * 32000);
	betray_init(B_CT_OPENGL_OR_ES, argc, argv, 500, 500, 1, FALSE, "Betray Relinquish Test Application");
	betray_action_func_set(b_test_input_handler, NULL);
	betray_screen_mode_get(&x, &y, NULL);
	betray_audio_listener(origo, origo, vector, &vector[2], 100, 10);
	sound_id = betray_audio_sound_create(BETRAY_TYPE_INT16, 1, BETRAY_SOUND_TEST_LOOP_SIZE, 44100, buffer, "sin");
	betray_audio_sound_play(sound_id, &vector[2], origo, 1, 1, TRUE, FALSE, FALSE); /* Play a sound at a particular speed, volume traeling along a vector. If looped the sound will keep replaying until stopped. The function returns a handle that can be used to stop, or modify the sound later. All sounds need to be deleted manulay once played, exept you the auto delete is set. Auto delete will automaticaly remove the sound once it has been played. If Auto delete is set to true you can not modify the sound after creating it. By setting the sound to ambient the position of the sound will be in the same space as the listener and will therfor not be afected by head movements of the listener.*/

	
	betray_launch_main_loop(); 
	return TRUE;
}
