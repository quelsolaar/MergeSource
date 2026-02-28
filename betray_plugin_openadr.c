
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "betray_plugin_api.h"


uint plugin_axis_id = -1;

void plugin_callback_main(BInputState *input)
{
	static float time = 0;
	float value = 0;
	time += 0.01;
	if(time > 2 * PI)
		time -= 2 * PI;

	value = sin(time);
	betray_plugin_axis_set(plugin_axis_id, value, 0, 0);	
}

void betray_plugin_init(void)
{
	uint device;
	device = betray_plugin_input_device_allocate(0, "OpenADR");
	plugin_axis_id = betray_plugin_axis_allocate(0, device, "OpenADR", B_AXIS_UNDEFINED, 1);
	betray_plugin_callback_set_main(plugin_callback_main);
}