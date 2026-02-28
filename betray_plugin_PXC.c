
#include "betray_plugin_api.h"
#include "betray_plugin_pxc_wrapper.h"
#include "imagine.h"
#include <stdio.h>
#include <math.h>

//#define DRAW_IMAGE
FILE *output_file = NULL;

uint pxc_plugin_setting_ids[3];
float pxc_plugin_setting_values[3];
float pxc_plugin_vantage[3] = {0, 0, 2};
void *pxc_plugin_mutex = NULL;
void betray_view_vantage2(float *view);

extern void pxc_facial_tracking(float *view, float delta_time, float screen_size, float x_displace, float y_displace);

void pxc_plugin_vantage_func(float *pos)
{
	imagine_mutex_lock(pxc_plugin_mutex);
	pos[0] = pxc_plugin_vantage[0];
	pos[1] = pxc_plugin_vantage[1];
	pos[2] = pxc_plugin_vantage[2];
	pxc_plugin_setting_values[0] = betray_settings_number_float_get(pxc_plugin_setting_ids[0]);
	pxc_plugin_setting_values[1] = betray_settings_number_float_get(pxc_plugin_setting_ids[1]);
	pxc_plugin_setting_values[2] = betray_settings_number_float_get(pxc_plugin_setting_ids[2]); 
	imagine_mutex_unlock(pxc_plugin_mutex);
}

void pxc_plugin_thread_func(void *data)
{
	float vantage[3] = {0, 0, 2}, settings[3];
	settings[0] = pxc_plugin_setting_values[0];
	settings[1] = pxc_plugin_setting_values[1];
	settings[2] = pxc_plugin_setting_values[2]; 
	while(TRUE)
	{
		pxc_facial_tracking(vantage, 0.01, settings[0], settings[1], settings[2]);
		imagine_mutex_lock(pxc_plugin_mutex);
		pxc_plugin_vantage[0] = vantage[0];
		pxc_plugin_vantage[1] = vantage[1];
		pxc_plugin_vantage[2] = vantage[2];
		settings[0] = pxc_plugin_setting_values[0];
		settings[1] = pxc_plugin_setting_values[1];
		settings[2] = pxc_plugin_setting_values[2]; 
		imagine_mutex_unlock(pxc_plugin_mutex);
	}
}
void betray_plugin_init(void)
{
	uint status;
	pxc_plugin_mutex = imagine_mutex_create();
	betray_plugin_callback_set_view_vantage(pxc_plugin_vantage_func, FALSE);
	pxc_plugin_setting_ids[0] = betray_settings_create(BETRAY_ST_NUMBER_FLOAT, "Screen Width MM", 0, NULL);
	betray_settings_number_float_set(pxc_plugin_setting_ids[0], 240);
	pxc_plugin_setting_ids[1] = betray_settings_create(BETRAY_ST_NUMBER_FLOAT, "Camera Offset X", 0, NULL);
	betray_settings_number_float_set(pxc_plugin_setting_ids[1], 0);
	pxc_plugin_setting_ids[2] = betray_settings_create(BETRAY_ST_NUMBER_FLOAT, "Camera Offset Y", 0, NULL);
	betray_settings_number_float_set(pxc_plugin_setting_ids[2], 1);
	imagine_thread(pxc_plugin_thread_func, NULL, "Betray PXC plugin");
}
