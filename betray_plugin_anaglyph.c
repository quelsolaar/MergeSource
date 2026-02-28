#include "betray_plugin_api.h"
#include "relinquish.h"
#include <stdio.h>
#include <math.h>

uint settings_id;
uint texture_id = 0;
uint depth_id = 0;
uint fbo_id = 0;
uint screen_size[2] = {0, 0};

boolean betray_plugin_image_warp(BInputState *input)
{
	static uint seed = 0;
	RMatrix matrix;
	float f, view[3] = {0, 0, 0}, color[3], eye_distance;
	uint x, y;

	seed++;

	glDisable(GL_DEPTH_TEST);
	f = betray_plugin_screen_mode_get(&x, &y, NULL);
	if(screen_size[0] != x || screen_size[1] != y)
	{
		screen_size[0] = x;
		screen_size[1] = y;
		if(texture_id != 0)
			r_texture_free(texture_id);
		if(depth_id != 0)
			r_texture_free(depth_id);
		if(fbo_id != 0)	
			r_framebuffer_free(fbo_id);
		texture_id = r_texture_allocate(R_IF_RGB_UINT8, x, y, 1, TRUE, TRUE, NULL);
		depth_id = r_texture_allocate(R_IF_DEPTH32, x, y, 1, TRUE, TRUE, NULL);
		fbo_id = r_framebuffer_allocate(&texture_id, 1, depth_id, RELINQUISH_TARGET_2D_TEXTURE);
	}
	eye_distance = betray_settings_number_float_get(settings_id);
	eye_distance *= eye_distance;
	view[0] = -eye_distance;
	betray_plugin_application_draw(fbo_id, x, y, view, TRUE, NULL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glViewport(0, 0, x, y);
	r_matrix_set(&matrix);
	r_matrix_identity(&matrix);
	r_matrix_frustum(&matrix, -0.01, 0.01, -0.01 * f, 0.01 * f, 0.01, 100.0);
	r_matrix_translate(&matrix, 0, 0, -1.0);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	r_primitive_image(-1, -f, 0.0, 2, 2 * f, 0, 0, 1, 1, texture_id, 1, 0, 0, 1);
	view[0] = eye_distance;
	betray_plugin_application_draw(fbo_id, x, y, view, TRUE, NULL);
	r_primitive_image(-1, -f, 0.0, 2, 2 * f, 0, 0, 1, 1, texture_id, 0, 1, 1, 1);
	return TRUE;
}


void betray_plugin_init(void)
{
	uint x, y;
	betray_plugin_callback_set_image_warp(betray_plugin_image_warp, "Anaglyph 3D");
	r_init(betray_plugin_gl_proc_address_get());
	settings_id = betray_settings_create(BETRAY_ST_SLIDER, "Eye distance", 0, NULL);
	betray_settings_number_float_set(settings_id, 0.15);
}
