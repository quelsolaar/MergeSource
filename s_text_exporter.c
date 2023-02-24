
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "seduce.h"
/*
typedef unsigned int uint;
typedef unsigned short boolean;

#define TRUE 1
#define FALSE 0
*/
#define CHARACTER_SET_SIZE 256
#define CHARACTER_SPACE_SIZE 1.0
#define CHARACTER_KERNING_DEPTH 0.9
#define CHARACTER_KERNING_SIZE 0.15


void sui_geometry_messure_font_size(float *vertex, float *min, float *max)
{
	if(vertex[1] > 1.0 - CHARACTER_KERNING_SIZE)
	{
		if(vertex[1] < 1.001)
		{
			if(vertex[0] < min[1])
				min[1] = vertex[0];
			if(vertex[0] > max[1])
				max[1] = vertex[0];
		}else
		{
			if(vertex[0] < min[0])
				min[0] = vertex[0];
			if(vertex[0] > max[0])
				max[0] = vertex[0];
		}
	}else if(vertex[1] > CHARACTER_KERNING_SIZE)
	{
		if(vertex[0] < min[2])
			min[2] = vertex[0];
		if(vertex[0] > max[2])
			max[2] = vertex[0];
	}else
	{
		if(vertex[1] > -0.001)
		{
			if(vertex[0] < min[3])
				min[3] = vertex[0];
			if(vertex[0] > max[3])
				max[3] = vertex[0];
		}
	}
}

void sui_geometry_messure_font_poly_size(uint *ref, float *vertex, float *min, float *max)
{
	float pos[2], f, fi;
	uint i;
	sui_geometry_messure_font_size(&vertex[ref[0] * 2], min, max);
	sui_geometry_messure_font_size(&vertex[ref[1] * 2], min, max);
	sui_geometry_messure_font_size(&vertex[ref[2] * 2], min, max);

	for(i = 1; i < 30; i++)
	{
		f = (float)i / 30;
		fi = 1.0 - f;
		pos[0] = vertex[ref[0] * 2] * f + vertex[ref[1] * 2] * fi;
		pos[1] = vertex[ref[0] * 2 + 1] * f + vertex[ref[1] * 2 + 1] * fi;
		sui_geometry_messure_font_size(pos, min, max);
		pos[0] = vertex[ref[1] * 2] * f + vertex[ref[2] * 2] * fi;
		pos[1] = vertex[ref[1] * 2 + 1] * f + vertex[ref[2] * 2 + 1] * fi;
		sui_geometry_messure_font_size(pos, min, max);
		pos[0] = vertex[ref[2] * 2] * f + vertex[ref[0] * 2] * fi;
		pos[1] = vertex[ref[2] * 2 + 1] * f + vertex[ref[0] * 2 + 1] * fi;
		sui_geometry_messure_font_size(pos, min, max);
	}
}

void sui_geometry_messure_min_max(float *min, float *max)
{
	if(min[0] > min[1] + CHARACTER_KERNING_DEPTH)
		min[0] = min[1] + CHARACTER_KERNING_DEPTH;
	if(min[2] > min[1] + CHARACTER_KERNING_DEPTH)
		min[2] = min[1] + CHARACTER_KERNING_DEPTH;
	if(min[1] > min[0] + CHARACTER_KERNING_DEPTH)
		min[1] = min[0] + CHARACTER_KERNING_DEPTH;
	if(min[2] > min[0] + CHARACTER_KERNING_DEPTH)
		min[2] = min[0] + CHARACTER_KERNING_DEPTH;
	if(min[1] > min[2] + CHARACTER_KERNING_DEPTH)
		min[1] = min[2] + CHARACTER_KERNING_DEPTH;
	if(min[0] > min[2] + CHARACTER_KERNING_DEPTH)
		min[0] = min[2] + CHARACTER_KERNING_DEPTH;
	if(min[0] > min[3] + CHARACTER_KERNING_DEPTH)
		min[0] = min[3] + CHARACTER_KERNING_DEPTH;
	if(min[1] > min[3] + CHARACTER_KERNING_DEPTH)
		min[1] = min[3] + CHARACTER_KERNING_DEPTH;
	if(min[2] > min[3] + CHARACTER_KERNING_DEPTH)
		min[2] = min[3] + CHARACTER_KERNING_DEPTH;
	if(min[3] > min[0] + CHARACTER_KERNING_DEPTH)
		min[3] = min[0] + CHARACTER_KERNING_DEPTH;
	if(min[3] > min[1] + CHARACTER_KERNING_DEPTH)
		min[3] = min[1] + CHARACTER_KERNING_DEPTH;
	if(min[3] > min[2] + CHARACTER_KERNING_DEPTH)
		min[3] = min[2] + CHARACTER_KERNING_DEPTH;

	if(max[0] < max[1] - CHARACTER_KERNING_DEPTH)
		max[0] = max[1] - CHARACTER_KERNING_DEPTH;
	if(max[2] < max[1] - CHARACTER_KERNING_DEPTH)
		max[2] = max[1] - CHARACTER_KERNING_DEPTH;
	if(max[1] < max[0] - CHARACTER_KERNING_DEPTH)
		max[1] = max[0] - CHARACTER_KERNING_DEPTH;
	if(max[2] < max[0] - CHARACTER_KERNING_DEPTH)
		max[2] = max[0] - CHARACTER_KERNING_DEPTH;
	if(max[1] < max[2] - CHARACTER_KERNING_DEPTH)
		max[1] = max[2] - CHARACTER_KERNING_DEPTH;
	if(max[0] < max[2] - CHARACTER_KERNING_DEPTH)
		max[0] = max[2] - CHARACTER_KERNING_DEPTH;
	if(max[0] > max[3] + CHARACTER_KERNING_DEPTH)
		max[0] = max[3] + CHARACTER_KERNING_DEPTH;
	if(max[1] > max[3] + CHARACTER_KERNING_DEPTH)
		max[1] = max[3] + CHARACTER_KERNING_DEPTH;
	if(max[2] > max[3] + CHARACTER_KERNING_DEPTH)
		max[2] = max[3] + CHARACTER_KERNING_DEPTH;
	if(max[3] > max[0] + CHARACTER_KERNING_DEPTH)
		max[3] = max[0] + CHARACTER_KERNING_DEPTH;
	if(max[3] > max[1] + CHARACTER_KERNING_DEPTH)
		max[3] = max[1] + CHARACTER_KERNING_DEPTH;
	if(max[3] > max[2] + CHARACTER_KERNING_DEPTH)
		max[3] = max[2] + CHARACTER_KERNING_DEPTH;
}


void sui_font_generate(void *user, char *file)
{
	FILE	*obj, *font;
	boolean first = FALSE;
	char	line[512];
	uint	i, j,  *ref, ref_length = 0, vertex_length = 0, count, line_break, junk, length[CHARACTER_SET_SIZE], start[CHARACTER_SET_SIZE];
	float	x, y, z, f, *vertex, max[CHARACTER_SET_SIZE][4], min[CHARACTER_SET_SIZE][4], top = -1000.0, bottom = 1000.0;
	obj = fopen(file, "r");
	font = fopen("sui_text_output.c", "w");
	if(obj != NULL)
	{
	/* count all vertices and polygons */
		while((fgets(line, sizeof line, obj)) != NULL)
		{
			if(line[0] == 118) /* v */
			{
				count = sscanf(line, "v %f %f %f", &x, &y, &z);
				if(count == 3)
					vertex_length++;
			}
			if(line[0] == 102) /* f */
			{
				count = sscanf(line, "f %u/%u %u/%u %u/%u %u/%u\n", &junk, &junk, &junk, &junk, &junk, &junk, &junk, &junk);
				if(count == 6)
					ref_length++;
			}
		}
		vertex = malloc((sizeof *vertex) * vertex_length * 2);
		ref = malloc((sizeof *ref) * ref_length * 3);

	/* clear all the init letter sizes*/

		for(i = 0; i < CHARACTER_SET_SIZE; i++)
			min[i][0] = min[i][1] = min[i][2] = min[i][3] = 1000;
		for(i = 0; i < CHARACTER_SET_SIZE; i++)
			max[i][0] = max[i][1] = max[i][2] = max[i][3] = -1000;
		for(i = 0; i < CHARACTER_SET_SIZE; i++)
			length[i] = 0;
		for(i = 0; i < CHARACTER_SET_SIZE; i++)
			start[i] = 0;
		for(i = 0; i < ref_length * 3; i++)
			ref[i] = 0;
		for(i = 0; i < vertex_length * 2; i++)
			vertex[i] = 0;

	/* read out all the vertext info */

		vertex_length = 0;
		ref_length = 0;
		rewind(obj); 
		line_break = 1;
		start[0] = 0; 
		i = 0;
		while((fgets(line, sizeof line, obj)) != NULL)
		{
			if(line[0] == 103) /* g */
			{
				char string[512];
				uint length;
//text_931_1
				count = sscanf(line, "g Trim_%u",  &i);
				i = i % 256;
				start[i] = ref_length * 3;
			}

			if(line[0] == 118) /* v */
			{
				count = sscanf(line, "v %f %f %f", &x, &y, &z);
				if(count == 3)
				{
					vertex[vertex_length * 2 + 0] = x;
					vertex[vertex_length * 2 + 1] = y;
					vertex_length++;
				}
			}
			else if(line[0] == 102) /* f */
			{
				count = sscanf(line, "f %u/%u %u/%u %u/%u %u/%u\n", &ref[ref_length * 3 + 0], &junk, &ref[ref_length * 3 + 1], &junk, &ref[ref_length * 3 + 2], &junk, &junk, &junk);
				if(count == 6)
				{
					length[i] += 3;
					ref[ref_length * 3 + 0]--;
					ref[ref_length * 3 + 1]--;
					ref[ref_length * 3 + 2]--;
					ref_length++; 
				}
			}
		}
/*		for(i = 0; i < 256; i++)
			fprintf(font, "%u\n", length[i]);
		fprintf(font, "------------------------------------------------------------------------------------------------\n");
		for(i = 0; i < ref_length * 3; i++)
			fprintf(font, "%u\n", ref[i]);
		fprintf(font, "------------------------------------------------------------------------------------------------\n");
		for(i = 0; i < vertex_length * 2; i++)
			fprintf(font, "%f\n", vertex[i]);
		fprintf(font, "------------------------------------------------------------------------------------------------\n");
*/
		top = bottom = vertex[ref[start[118]] * 2 + 1];
		for(j = 1; j < length[118]; j++)
		{
			if(vertex[ref[start[118] + j] * 2 + 1] > top)
				top = vertex[ref[start[118] + j] * 2 + 1];
			if(vertex[ref[start[118] + j] * 2 + 1] < bottom)
				bottom = vertex[ref[start[118] + j] * 2 + 1];
		}
		
		for(i = 0; i < vertex_length; i++)
		{
			vertex[i * 2 + 0] /= top - bottom;
			vertex[i * 2 + 1] = (vertex[i * 2 + 1] - bottom) / (top - bottom);
		}

	/* get size of font */

		for(i = 0; i < CHARACTER_SET_SIZE; i++)
			for(j = 0; j < length[i]; j += 3)
				sui_geometry_messure_font_poly_size(&ref[start[i] + j], vertex, min[i], max[i]);

		for(i = 0; i < CHARACTER_SET_SIZE; i++)
			sui_geometry_messure_min_max(min[i], max[i]);

		for(i = 0; i < CHARACTER_SET_SIZE; i++)
		{
			if(length[i] > 0)
			{
				fprintf(font, "unsigned int sui_font_ref_array_%u[%u] = {%u", i, length[i], ref[start[i]]);
				for(j = 1; j < length[i]; j++)
				{
					fprintf(font, ", %u", ref[start[i] + j]);
					if(j % 30 == 21)
						fprintf(font, "\n");
				}
				fprintf(font, "};\n\n");
			}
		}
	/* move / scale all letters */

		for(i = 0; i < CHARACTER_SET_SIZE; i++)
		{
			uint i_max, i_min;
			if(length[i] > 0)
			{
				i_max = i_min = ref[start[i]];
				for(j = 1; j < length[i]; j++)
				{
					if(ref[start[i] + j] > i_max)
						i_max = ref[start[i] + j];
					if(ref[start[i] + j] < i_min)
						i_min = ref[start[i] + j];	
				}
				f = min[i][0];
				if(min[i][1] < f)
					f = min[i][1];
				if(min[i][2] < f)
					f = min[i][2];
				if(min[i][3] < f)
					f = min[i][3];
				for(j = i_min; j < i_max + 1; j++)
					vertex[j * 2 + 0] -= f;
			}
		}

	/*print out vertex array */

		fprintf(font, "\n\nfloat sui_font_vertex_array[%u] = {(float)%f, (float)%f", vertex_length * 2, vertex[0], vertex[1]);
		for(i = 1; i < vertex_length; i++)
		{
			if(i % 20 == 0)
				fprintf(font, "\n");
			fprintf(font, ", (float)%f, (float)%f", vertex[i * 2 + 0], vertex[i * 2 + 1]);
		}
		fprintf(font, "};\n\n");

	/* print all array_lengthes */

		fprintf(font, "\n\nunsigned int sui_font_ref_size[%u] = {%u", CHARACTER_SET_SIZE, length[0]);
		for(i = 1; i < CHARACTER_SET_SIZE; i++)
		{
			if(i % 20 == 0)
				fprintf(font, "\n");
			fprintf(font, ", %u", length[i]);
		}
		fprintf(font, "};\n\n");

	/* print all leter sizes */
		fprintf(font, "\n\nfloat sui_font_letter_size[%u] = {(float)0.0, (float)0.0, (float)0.0, (float)0.0, (float)%f, (float)%f, (float)%f, (float)%f",  CHARACTER_SET_SIZE * 8, CHARACTER_SPACE_SIZE, CHARACTER_SPACE_SIZE, CHARACTER_SPACE_SIZE, CHARACTER_SPACE_SIZE);
		for(i = 1; i < CHARACTER_SET_SIZE; i++)
		{
			if(i % 8 == 0)
				fprintf(font, "\n");
			if(max[i][1] < min[i][1])
				fprintf(font, ", (float)0.0, (float)0.0, (float)0.0, (float)0.0, (float)%f, (float)%f, (float)%f, (float)%f", CHARACTER_SPACE_SIZE, CHARACTER_SPACE_SIZE, CHARACTER_SPACE_SIZE, CHARACTER_SPACE_SIZE);
			else
			{
				f = min[i][0];
				if(min[i][1] < f)
					f = min[i][1];
				if(min[i][2] < f)
					f = min[i][2];
				if(min[i][3] < f)
					f = min[i][3];
				fprintf(font, ", (float)%f, (float)%f, (float)%f, (float)%f, (float)%f, (float)%f, (float)%f, (float)%f", min[i][0] - f, min[i][1] - f, min[i][2] - f, min[i][3] - f, max[i][0] - f, max[i][1] - f, max[i][2] - f, max[i][3] - f);
			}
		}
		fprintf(font, "};\n\n");

		fclose(obj);
	
	/* print init and draw func */

		fprintf(font, "unsigned int *sui_font_ref_array[%u];\n\n", CHARACTER_SET_SIZE);
		fprintf(font, "void seduce_font_init()\n");
		fprintf(font, "{\n");
		fprintf(font, "\tstatic int init = 0;\n");
		fprintf(font, "\tunsigned int i;\n");
		fprintf(font, "\tif(!init)\n");
		fprintf(font, "\t{\n");
		fprintf(font, "\t\tinit = 1;\n");
		fprintf(font, "\t\tfor(i = 0; i < %i; i++)\n", CHARACTER_SET_SIZE);
		fprintf(font, "\t\t\tsui_font_ref_array[i] = 0;\n");
		for(i = 0; i < CHARACTER_SET_SIZE; i++)
			if(length[i] > 0)
				fprintf(font, "\t\tsui_font_ref_array[%u] = sui_font_ref_array_%u;\n", i, i);
		fprintf(font, "\t}\n");
		fprintf(font, "}\n");

		font = fopen("text_output.h", "w");
		fprintf(font, "float \t\t\tsui_font_vertex_array[%u];\n", vertex_length * 2);
		fprintf(font, "unsigned int\t*sui_font_ref_array[%u];\n", CHARACTER_SET_SIZE);
		fprintf(font, "unsigned int\t*sui_font_ref_array[%u];\n", CHARACTER_SET_SIZE);
		fprintf(font, "float \t\t\tsui_font_letter_size[%u];\n",  CHARACTER_SET_SIZE * 8);
	}
}

