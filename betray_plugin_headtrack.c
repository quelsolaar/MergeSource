
#include "betray_plugin_api.h"
#include <stdio.h>
#include <math.h>

uint pxc_plugin_setting_ids[3];

void vantage(float *pos)
{
	float a, b, c;
	pos[0] = 0.0;
	pos[1] = 0.0;
	pos[2] = 2.5;

	a = betray_settings_number_float_get(pxc_plugin_setting_ids[0]);
	b = betray_settings_number_float_get(pxc_plugin_setting_ids[1]);
	c = betray_settings_number_float_get(pxc_plugin_setting_ids[2]);
}

void betray_plugin_init(void)
{
	betray_plugin_callback_set_view_vantage(vantage, FALSE);

	pxc_plugin_setting_ids[0] = betray_settings_create(BETRAY_ST_NUMBER_FLOAT, "Camera Offset X MM", 0, NULL);
	betray_settings_number_float_set(pxc_plugin_setting_ids[0], 0);
	pxc_plugin_setting_ids[1] = betray_settings_create(BETRAY_ST_NUMBER_FLOAT, "Camera Offset Y MM", 0, NULL);
	betray_settings_number_float_set(pxc_plugin_setting_ids[1], 200);
	pxc_plugin_setting_ids[2] = betray_settings_create(BETRAY_ST_NUMBER_FLOAT, "Screen Width MM", 0, NULL);
	betray_settings_number_float_set(pxc_plugin_setting_ids[1], 400);
}
