#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hxa.h"
#include "hxa_utils.h"

#define FALSE 0
#define TRUE !FALSE

int hxa_save_targa(char *file_name, unsigned char *data, unsigned int x, unsigned int y, unsigned int channels)
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
	fputc(2, image); 
	for(i = 3; i < 12; i++)
		fputc(0, image);
	fputc(x % 256, image);
	fputc(x / 256, image);
	fputc(y % 256, image);
	fputc(y / 256, image);
	if(channels == 3)
	{
		fputc(24, image);
		for(j = 0; j < y; j++)
		{
			for(i = 0; i < x; i++)
			{
				fputc(data[((y - j - 1) * x + i) * 3 + 0], image);
				fputc(data[((y - j - 1) * x + i) * 3 + 2], image);
				fputc(data[((y - j - 1) * x + i) * 3 + 1], image);
			}
		}
	}else if(channels == 4)
	{
		fputc(32, image);
		for(j = 0; j < y; j++)
		{
			for(i = 0; i < x; i++)
			{
				fputc(data[((y - j - 1) * x + i) * 3 + 0], image);
				fputc(data[((y - j - 1) * x + i) * 3 + 2], image);
				fputc(data[((y - j - 1) * x + i) * 3 + 1], image);
			}
		}
	}
	for(i = 0; i < 9; i++)
		fputc(0, image);
	for(i = 0; foot[i] != 0; i++)
		fputc(foot[i], image); 
	fputc(0, image);
	fclose(image);
}


int hxa_load_targa(HXAFile *file, char *file_name)
{
	unsigned char *draw = NULL;
	HXANode *node;
	unsigned int i, j, identfeald, type, x_size, y_size, bits, channel, length, descriptor;
	int add_x, add_y;
	size_t size, pos = 0;
	unsigned char buffer[4], *data;
	data = f_text_load(file_name, &size);
	if(data == NULL)
	{
		printf("HxA Error: Could not open file %s\n", file_name);
		return FALSE;
	}

	identfeald = data[0];
	if(0 != data[1])
	{
		printf("Error: File %s a non suported palet image\n", file_name);
		free(data);
		return FALSE;
	}
	type = data[2];
	if(2 != type && /* True color*/
		3 != type && /* monocromatic */
		10 != type && /* True color*/
		11 != type ) /* monocromatic */
	{
		printf("Error: File %s is not a supprted imageformat\n", file_name);
		free(data);
		return FALSE;
	}
	pos += 9; /* ignore some stuff */
	x_size = (unsigned int)data[12];
	x_size |= ((unsigned int)data[13] << 8);
	y_size = (unsigned int)data[14];
	y_size |= ((unsigned int)data[15] << 8);

	bits = data[16];
	if(2 == type || 10 == type)
	{
		if(bits != 24 && bits != 32)
		{
			printf("Error: File %s is not a 24 or 32 bit RGB image\n", file_name);
			return FALSE;
		}
		channel = bits / 8;
	}else
	{
		if(bits != 8)
		{
			printf("Error: File %s is not a 8 bit Gray-scale image\n", file_name);
			return FALSE;
		}
		channel = 1;
	}
	descriptor = data[17];
	file->node_array = realloc(file->node_array, sizeof(*file->node_array) * (file->node_count + 1));
	node = &file->node_array[file->node_count++];
	pos = 18 + identfeald; /* ignore some stuff */
	node->type = HXA_NT_IMAGE;
	node->meta_data = NULL;
	node->meta_data_count = 0;
	node->content.image.type = HXA_IT_2D_IMAGE;
	node->content.image.resolution[0] = x_size;
	node->content.image.resolution[1] = y_size;
	node->content.image.resolution[2] = 1;
	node->content.image.image_stack.layer_count = 1;
	node->content.image.image_stack.layers = malloc(sizeof *node->content.image.image_stack.layers);
	node->content.image.image_stack.layers->components = channel;
	node->content.image.image_stack.layers->name[0] = 0;
	node->content.image.image_stack.layers->type = HXA_LDT_UINT8;
	node->content.image.image_stack.layers->data.uint8_data = draw = malloc(channel * x_size * y_size);

	if(10 == type || 11 == type)
	{
		for(i = 0; i < x_size * y_size * channel;)
		{
			length = data[pos++];
			if(length & 128)
			{
				length = length & 127;
				length++;
				memcpy(buffer, &data[pos], channel);
				pos += channel;
				for(j = 0; j < length; j++)
				{
					memcpy(&draw[i], buffer, channel);
					i += channel;
				}
			}else
			{
				length = length & 127;
				length++;
				for(j = 0; j < length; j++)
				{
					memcpy(&draw[i], &data[pos], channel);
					i += channel;
					pos += channel;
				}
			}
		}
		free(data);
		data = draw;
		pos = 0;		
		node->content.image.image_stack.layers->data.uint8_data = draw = malloc(channel * x_size * y_size);
	}

	if((descriptor >> 5) & 1)
	{
		add_y = 0;
	}else
	{
		add_y = (int)(x_size * channel) * -2;
		draw = &draw[x_size * (y_size - 1) * channel];
	}			
	if((descriptor >> 4) & 1)
	{
		add_x = (int)channel * -1;
		draw = &draw[(x_size - 1) * channel];
		add_y += channel * x_size * 2;
	}else
		add_x = channel;

	switch(channel)
	{
		case 1 :
		for(i = 0; i < y_size; i++)
		{
			for(j = 0; j < x_size; j++)
			{
				draw[0] = data[pos++];
				draw += add_x;
			}
			draw += add_y;
		}
		break;
		case 3 :
		for(i = 0; i < y_size; i++)
		{
			for(j = 0; j < x_size; j++)
			{
				draw[2] = data[pos++];
				draw[1] = data[pos++];
				draw[0] = data[pos++];
				draw += add_x;
			}
			draw += add_y;
		}
		break;
		case 4 :
		for(i = 0; i < y_size; i++)
		{
			for(j = 0; j < x_size; j++)
			{
				draw[2] = data[pos++];
				draw[1] = data[pos++];
				draw[0] = data[pos++];
				draw[3] = data[pos++];
				draw += add_x;
			}
			draw += add_y;
		}
		break;
	}
	free(data);
	return TRUE;
}
