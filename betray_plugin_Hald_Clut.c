#include "betray_plugin_api.h"
#include "relinquish.h"
#include <stdio.h>
#include <math.h>

uint texture_id = 0;
uint hald_clut_id = 0;
uint depth_id = 0;
uint fbo_id = 0;
uint screen_size[2] = {0, 0};
RShader *clut_lookup_shader = NULL;
void *clut_surface_pool = NULL;

char *clut_lookup_vertex = 
"attribute vec4 vertex;"
"uniform mat4 ModelViewProjectionMatrix;"
"varying vec2 mapping;"
"void main()"
"{"
"	mapping = vertex.ba;"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex.xy, 0.0, 1.0);"
"}";

char *clut_lookup_fragment = 
"uniform sampler2D image;"
"uniform sampler3D clut;"
"varying vec2 mapping;"
"void main()"
"{"
"	vec4 sample;"
"	sample = texture2D(image, mapping);"
"	gl_FragColor = texture3D(clut, sample.rgb);"
"}";


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
	fputc(24, image); /* 24 bit image */
	for(j = 0; j < y; j++)
	{
		for(i = 0; i < x; i++)
		{
			fputc(data[((y - j - 1) * x + i) * 3 + 0], image);
			fputc(data[((y - j - 1) * x + i) * 3 + 2], image);
			fputc(data[((y - j - 1) * x + i) * 3 + 1], image);
		}
	}
	for(i = 0; i < 9; i++)
		fputc(0, image);
	for(i = 0; foot[i] != 0; i++)
		fputc(foot[i], image); 
	fputc(0, image);
	fclose(image);
}


uint8 *load_targa(char *file_name, unsigned int *x, unsigned int *y)
{
	FILE *image;
	uint8 *draw = NULL;
	unsigned int i, j, identfeald, type, x_size, y_size, alpha;

	if((image = fopen(file_name, "rb")) == NULL)
	{
		printf("can not open file: %s\n", file_name);
		return NULL;
	}

	identfeald = fgetc(image);
	if(0 != fgetc(image))
	{
		printf("Error: File %s a non suported palet image\n", file_name);
		return NULL;
	}
	type = fgetc(image);
	if(2 != type) /* type must be 2 uncompressed RGB */
	{
		printf("Error: File %s is not a uncompressed RGB image\n", file_name);
		return NULL;
	}
	for(i = 3; i < 12; i++)
		fgetc(image); /* ignore some stuff */
	x_size = fgetc(image);
	x_size += 256 * fgetc(image);
	y_size = fgetc(image);
	y_size += 256 * fgetc(image);

	alpha = fgetc(image);

	if(alpha != 24 && alpha != 32) /* type must be 24 or 32 bits RGB */
	{
		printf("Error: File %s is not a 24 or 32 bit RGB image\n", file_name);
		return NULL;
	}

	for(i = 0; i < identfeald; i++)
		fgetc(image); /* ignore some stuff */
	*x = x_size;
	*y = y_size;
	
	draw = malloc((sizeof *draw) * x_size * y_size * 3);

	for(j = 0; j < y_size * x_size * 3; j++)
		draw[j] = 0;

	if(alpha == 32)
	{
		for(j = 0; j < y_size; j++)
		{
			for(i = 0; i < x_size; i++)
			{
				fgetc(image); /* ignore alpha */
				draw[((y_size - j - 1) * x_size + i) * 3 + 2] = fgetc(image);
				draw[((y_size - j - 1) * x_size + i) * 3 + 1] = fgetc(image);
				draw[((y_size - j - 1) * x_size + i) * 3 + 0] = fgetc(image);
			}
		}
	}else for(j = 0; j < y_size; j++)
	{
		for(i = 0; i < x_size; i++)
		{			
			draw[((y_size - j - 1) * x_size + i) * 3 + 0] = fgetc(image);
			draw[((y_size - j - 1) * x_size + i) * 3 + 2] = fgetc(image);
			draw[((y_size - j - 1) * x_size + i) * 3 + 1] = fgetc(image);
		}
	}


	fclose(image);
	return draw;
}


boolean betray_plugin_image_warp(BInputState *input)
{
	static uint seed = 0;
	RMatrix matrix;
	float f;
	uint x, y, i;
	seed++;
	f = betray_plugin_screen_mode_get(&x, &y, NULL);
	if(screen_size[0] != x || screen_size[1] != y)
	{
	
		float surface[6 * 4] = {-1, -1, 0, 0, 
								1, -1, 1, 0, 
								1, 1, 1, 1,
								-1, -1, 0, 0,
								1, 1, 1, 1, 
								-1, 1, 0, 1};
		for(i = 0; i < 6; i++)
			surface[i * 4 + 1] *= f;
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
		r_array_load_vertex(clut_surface_pool, NULL, surface, 0, 6);
	}
	betray_plugin_application_draw(fbo_id, x, y, NULL, FALSE, NULL);

	r_viewport(0, 0, x, y);
	r_matrix_set(&matrix);
	r_matrix_identity(&matrix);
	r_matrix_frustum(&matrix, -0.01, 0.01, -0.01 * f, 0.01 * f, 0.01, 100.0);
	glClearColor(0, 0, 1, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	r_matrix_translate(&matrix, 0, 0, -1.0);
	
	r_shader_set(clut_lookup_shader);
	r_shader_texture_set(clut_lookup_shader, 0, texture_id);
	r_shader_texture_set(clut_lookup_shader, 1, hald_clut_id);
	r_array_section_draw(clut_surface_pool, NULL, GL_TRIANGLES, 0, 6);

	return TRUE;
}


void betray_plugin_init(void)
{
	char *file_name = "Betray_hald_clut.tga";
	unsigned int i, j, k, size, level = 16, x, y;
	uint8 *data, *p;
	RFormats vertex_format_types = R_FLOAT;
	uint vertex_format_size = 4;
	betray_plugin_callback_set_image_warp(betray_plugin_image_warp, "Hald Clut CC");
	data = load_targa(file_name, &x, &y);
	if(data == NULL)
	{
		level = 8;
		size = level * level;
		data = p = malloc((sizeof *data) * size * size * size * 3);
		for(i = 0; i < size; i++)
		{
			for(j = 0; j < size; j++)
			{
				for(k = 0; k < size; k++)
				{
					*p++ = (uint8)(k * 256 / size);
					*p++ = (uint8)(j * 256 / size);
					*p++ = (uint8)(i * 256 / size);
				}
			}
		}
		save_targa(file_name, data, level * level * level, level * level * level);
		printf("Hald CLUT with level %u created, %u by %u pixels size, file name: %s\n", level, size * level, size * level, file_name);
		return;
	}
	if(x != y)
	{
		printf("Betray Plugin Error: Hald CLUT not sqare\n");
		return;
	}

	for(i = 0; i * i * i < x * y; i++);
	if(i * i * i != x * y)
	{
		printf("Betray Plugin Error: Hald CLUT size error\n");
		return;
	}
	printf("Hald CLUT Loaded\n");
	r_init(betray_plugin_gl_proc_address_get());
	hald_clut_id = r_texture_allocate(R_IF_RGB_UINT8, i, i, i, TRUE, FALSE, data);
	free(data);
	clut_lookup_shader = r_shader_create_simple(NULL, 0, clut_lookup_vertex, clut_lookup_fragment, "Clut Lookup shader");
	clut_surface_pool = r_array_allocate(6, &vertex_format_types, &vertex_format_size, 1, 0);
}
