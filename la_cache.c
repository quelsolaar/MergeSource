
#include "la_includes.h"


uint la_create_particle_material(uint size, float *data)
{
	uint texture_id, i;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGB, GL_FLOAT, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_2D);
	return texture_id;
}

boolean la_load_targa(char *file_name, uint *texture_id)
{
	FILE *image;
	float *draw = NULL;
	unsigned int i, j, identfeald, type, x_size, y_size, alpha;

	if((image = fopen(file_name, "rb")) == NULL)
	{
		printf("Warning: cannot open file: \"%s\"\n", file_name);
		return FALSE;
	}

	identfeald = fgetc(image);
	if(0 != fgetc(image))
	{
		printf("Error: File %s a non suported palet image\n", file_name);
		return FALSE;
	}
	type = fgetc(image);
	if(2 != type) /* type must be 2 uncompressed RGB */
	{
		printf("Error: File %s is not a uncompressed RGB image\n", file_name);
		return FALSE;
	}
	for(i = 3; i < 12; i++)
		fgetc(image); /* ignore some stuff */
	x_size = fgetc(image);
	x_size += 256 * fgetc(image);
	y_size = fgetc(image);
	y_size += 256 * fgetc(image);
	if(x_size != y_size) /* type must be 24 or 32 bits RGB */
	{
		printf("Error: File %s must be square\n", file_name);
		return FALSE;
	}

	alpha = fgetc(image);

	if(alpha != 24 && alpha != 32) /* type must be 24 or 32 bits RGB */
	{
		printf("Error: File %s is not a 24 or 32 bit RGB image\n", file_name);
		return FALSE;
	}

	for(i = 0; i < identfeald; i++)
		fgetc(image); /* ignore some stuff */

	draw = malloc((sizeof *draw) * x_size * y_size * 3);

	if(alpha == 32)
	{
		for(i = 0; i < x_size * y_size * 3; i += 3)
		{
			fgetc(image); /* ignore alpha */
			draw[i + 2] = (float)fgetc(image) / (float)255.0;
			draw[i + 1] = (float)fgetc(image) / (float)255.0;
			draw[i + 0] = (float)fgetc(image) / (float)255.0;
		}
	}
	else for(i = 0; i < x_size * y_size * 3; i += 3)
	{			
		draw[i] = (float)fgetc(image) / (float)255.0;
		draw[i + 2] = (float)fgetc(image) / (float)255.0;
		draw[i + 1] = (float)fgetc(image) / (float)255.0;
	}
	fclose(image);
	if(texture_id != NULL)
		*texture_id = la_create_particle_material(x_size, draw);
	free(draw);
	return TRUE;
}


uint la_save_targa(char *file_name, float *data, unsigned int size)
{
	FILE *image;
	char *foot = "TRUEVISION-XFILES.";
	unsigned int i, j;

	if((image = fopen(file_name, "wb")) == NULL)
	{
		printf("Could not create file: %s\n", file_name);
		return la_create_particle_material(size, data);
	}
	fputc(0, image);
	fputc(0, image);
	fputc(2, image); /* uncompressed */
	for(i = 3; i < 12; i++)
		fputc(0, image); /* ignore some stuff */
	fputc(size % 256, image);  /* size */
	fputc(size / 256, image);
	fputc(size % 256, image);
	fputc(size / 256, image);
	fputc(24, image); /* 24 bit image */
	for(i = 0; i < size * size * 3; i++)
	{
		if(data[i] < 0)
			data[i] = 0;
		if(data[i] > 1)
			data[i] = 1;		
	}
	for(i = 0; i < size * size * 3; i += 3)
	{
		fputc((int)(data[i] * 255.0), image);
		fputc((int)(data[i + 2] * 255.0), image);
		fputc((int)(data[i + 1] * 255.0), image);
	}
	for(i = 0; i < 9; i++)
		fputc(0, image); // ignore some stuff
	for(i = 0; foot[i] != 0; i++)
		fputc(foot[i], image); // ignore some stuff
	fputc(0, image);
	fclose(image);
	return la_create_particle_material(size, data);
}
