
#include "la_includes.h"
#include "forge.h"

struct{
	boolean rec;
	boolean play;
	FILE	*file;
}LAInputHandlerGlobal;

void start_macro_record(char *file)
{
	LAInputHandlerGlobal.file = fopen(file, "w");
	LAInputHandlerGlobal.rec = TRUE;
}

void start_macro_play_back(char *file)
{
	LAInputHandlerGlobal.file = fopen(file, "r");
	LAInputHandlerGlobal.play = TRUE;
}

void stop_macro(void)
{
	if(LAInputHandlerGlobal.file != NULL)
		fclose(LAInputHandlerGlobal.file);
	LAInputHandlerGlobal.rec = FALSE;
	LAInputHandlerGlobal.play = FALSE;
}

void init_key_input(void)
{
	LAInputHandlerGlobal.file = NULL;
	LAInputHandlerGlobal.rec = FALSE;
	LAInputHandlerGlobal.play = FALSE;
}

void save_macro_input(BInputState *output)
{
	fprintf(LAInputHandlerGlobal.file, "%f %f %u %u %u\n", output->pointers[0].pointer_x, output->pointers[0].pointer_y, output->pointers[0].button[0], output->pointers[0].button[1], output->pointers[0].button[2]);
}

void load_macro_input(BInputState *output, boolean cansle)
{
	char	line[512];
	uint	i, a, b, c;
	for(i = 0; i < 512; i++)
		line[i] = 0;
	if(cansle != TRUE && (fgets(line, sizeof line, LAInputHandlerGlobal.file)) != NULL)
	{
		if(sscanf(line, "%f %f %u %u %u", &output->pointers[0].pointer_x, &output->pointers[0].pointer_y, &a, &b, &c) != 5)
		{
			LAInputHandlerGlobal.play = FALSE;
			fclose(LAInputHandlerGlobal.file);
		}
	}else
	{
		LAInputHandlerGlobal.play = FALSE;
		fclose(LAInputHandlerGlobal.file);
	}
	output->pointers[0].last_button[0] = output->pointers[0].button[0];
	output->pointers[0].last_button[1] = output->pointers[0].button[1];
	output->pointers[0].last_button[2] = output->pointers[0].button[2];
	output->pointers[0].button[0] = a;
	output->pointers[0].button[1] = b;
	output->pointers[0].button[2] = c;
}
/*
typedef struct{
	uint	button;
	double state;
}ButtonEvent;

typedef struct{
	boolean	clear;
	ButtonEvent	event[16];
	uint		event_count;
	boolean		device_buttons[DEVICE_BUTTONS_COUNT];
	double		axis_state[AXIS_COUNT];
	boolean	axis_update_flag[AXIS_COUNT];
	char		*axis_name[AXIS_COUNT];
}InputData;

void la_input_handeler(BInputState *output, InputData *data, uint screen_x, uint screen_y)
{
	uint	i, temp[DEVICE_BUTTONS_COUNT];
    static uint modefyer = 0, button[DEVICE_BUTTONS_COUNT] = {FALSE, FALSE, FALSE};
	double mouse_x, mouse_y;
	if(LAInputHandlerGlobal.play)
		load_macro_input(output, data->device_buttons[0] && data->device_buttons[1] && data->device_buttons[2]);
	else
	{
		if(LAInputHandlerGlobal.rec)
			save_macro_input(output);
        for(i = 0; i < data->event_count; i++)
        {
            if(data->event[i].button == SDLK_LSHIFT)
                button[0] = data->event[i].state;
            if(data->event[i].button == SDLK_LALT)
                button[1] = data->event[i].state;
            if(data->event[i].button == SDLK_LCTRL)
                button[2] = data->event[i].state;
        }       
		if(data->device_buttons[0] == FALSE && data->device_buttons[modefyer] == FALSE && data->device_buttons[modefyer] == FALSE)
        {
            modefyer = 0;
            for(i = 1; i < 3; i++)
                if(button[i])
                    modefyer = i; 
        }
        for(i = 0; i < DEVICE_BUTTONS_COUNT; i++)
		{
			output->last_mouse_button[i] = output->mouse_button[i];
            output->mouse_button[i] =((data->device_buttons[i] && i != 0) || (i == modefyer && data->device_buttons[0]) || (button[i] && data->device_buttons[0]));
		}
		output->pointer_x = (data->axis_state[AXIS_MOUSE_X] / screen_x) - 0.5; 
		output->pointer_y = -(data->axis_state[AXIS_MOUSE_Y] / screen_x) + 0.5 * ((float)screen_y / (float)screen_x);
		output->pointer_x *= 2; 
		output->pointer_y *= 2;
	}

	if(output->mouse_button[0] == TRUE && output->last_mouse_button[0] != TRUE)
	{
		output->click_pointer_x[0] = output->pointer_x;
		output->click_pointer_y[0] = output->pointer_y;
	}
}*/
