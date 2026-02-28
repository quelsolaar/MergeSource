#include "betray_plugin_api.h"
#include "relinquish.h"
#include <stdio.h>
#include <math.h>

uint screenshot_settings[5];
uint screenshot_sides = 0;
uint screen_shot_number = 0;

void save_targa(char *file_name, uint8 *data, unsigned int x, unsigned int y)
{
	FILE *image;
	char *foot = "TRUEVISION-XFILES.";
	unsigned int i, j;

	if((image = fopen(file_name, "wb")) == NULL)
	{
		printf("Could not create file: %s\n", file_name);
		return;
	}
	fputc(0, image);
	fputc(0, image);
	fputc(2, image); /* uncompressed */
	for(i = 3; i < 12; i++)
		fputc(0, image); /* ignore some stuff */
	fputc(x % 256, image);  /* size */
	fputc(x / 256, image);
	fputc(y % 256, image);
	fputc(y / 256, image);
	fputc(32, image); /* 24 bit image */
	for(j = 0; j < y; j++)
	{
		for(i = 0; i < x; i++)
		{
			fputc(data[((y - j - 1) * x + (x - i - 1)) * 4 + 3], image);
			fputc(data[((y - j - 1) * x + (x - i - 1)) * 4 + 2], image);
			fputc(data[((y - j - 1) * x + (x - i - 1)) * 4 + 1], image);
			fputc(data[((y - j - 1) * x + (x - i - 1)) * 4 + 0], image);
		}
	}
	for(i = 0; i < 9; i++)
		fputc(0, image);
	for(i = 0; foot[i] != 0; i++)
		fputc(foot[i], image); 
	fputc(0, image);
	fclose(image);
}

void betray_plugin_main_loop(BInputState *input)
{
	char *name, *record = "Record"; 
	uint i, j;
	if(betray_settings_trigger(screenshot_settings[2]))
		screenshot_sides = 1;
	if(betray_settings_trigger(screenshot_settings[4]))
		screenshot_sides = 6;
	for(i = 0; i < betray_settings_select_count_get(0); i++)
	{
		name = betray_settings_select_name_get(0, i);
		for(j = 0; name[j] == record[j] && name[j] != 0; j++);
		if(name[j] == record[j])
			betray_settings_select_set(0, i);
	}
}

boolean betray_plugin_image_warp(BInputState *input)
{
	FILE *f;
	char file_name[128], *file_type;
	RMatrix matrix;
	uint x, y, i, sides = 1;
	uint8 *data;
	uint texture_id = 0;
	uint depth_id = 0;
	uint fbo_id = 0;
	if(screenshot_sides == 0)
		return FALSE;
	betray_plugin_screen_mode_get(&x, &y, NULL);
	x = 4096;
	y = 4096;
	texture_id = r_texture_allocate(R_IF_RGB_UINT8, x, y, 1, TRUE, TRUE, NULL);
	depth_id = r_texture_allocate(R_IF_DEPTH32, x, y, 1, TRUE, TRUE, NULL);
	fbo_id = r_framebuffer_allocate(&texture_id, 1, depth_id, RELINQUISH_TARGET_2D_TEXTURE);
	data = malloc((sizeof *data) * x * y * 4 * sides);
	for(i = 0; i < sides; i++)
	{
		betray_plugin_application_draw(fbo_id, x, y, NULL, FALSE, NULL);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[x * y * 4 * i]);
		glDisable(GL_TEXTURE_2D);
	}
	if(sides == 1)
		file_type = "screen";
	else
		file_type = "environment";


	sprintf(file_name, "Betray_%s_capture_%.5d.tga", file_type, screen_shot_number++);
	f = fopen(file_name, "rb");
	while(f != NULL)
	{
		fclose(f);
		sprintf(file_name, "Betray_%s_capture_%.5d.tga", file_type, screen_shot_number++);
		f = fopen(file_name, "rb");
	}
	save_targa(file_name, data, x, y * sides);
	free(data);

	r_texture_free(texture_id);
	r_texture_free(depth_id);
	r_framebuffer_free(fbo_id);
	betray_settings_select_set(0, betray_settings_select_count_get(0) - 1);
	screenshot_sides = 0;
	return FALSE;
}


void betray_plugin_init(void)
{
	uint x, y;
	betray_plugin_screen_mode_get(&x, &y, NULL);
	betray_plugin_callback_set_main(betray_plugin_main_loop);
	betray_plugin_callback_set_image_warp(betray_plugin_image_warp, "Record");
	r_init(betray_plugin_gl_proc_address_get());
	screenshot_settings[0] = betray_settings_create(BETRAY_ST_NUMBER_INT, "Width ", 4096, NULL);
	screenshot_settings[1] = betray_settings_create(BETRAY_ST_NUMBER_INT, "Height", 4096, NULL);
	screenshot_settings[2] = betray_settings_create(BETRAY_ST_TRIGGER, "Screen Capture", 1024, NULL);
	screenshot_settings[3] = betray_settings_create(BETRAY_ST_NUMBER_INT, "Cube Size", 1024, NULL);
	screenshot_settings[4] = betray_settings_create(BETRAY_ST_TRIGGER, "Cube Capture", 1024, NULL);
}
