 
#include "forge.h"
#include "r_include.h"
#include "relinquish.h"
/*
void sui_set_blend_gl(uint source, uint destination)
{
	glBlendFunc(source, destination);
	glEnable(GL_BLEND);
}

void r_gl(uint draw_type, const float *array, uint length, uint dimensions, float red, float green, float blue, float alpha)
{
	return;
	glColor4f(red, green, blue, alpha);	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(dimensions, GL_FLOAT , 0, array);
	glDrawArrays(draw_type, 0, length);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
}


void r_2d_line_gl(float start_x, float start_y, float end_x, float end_y, float red, float green, float blue, float alpha)
{
	float array[4];
	return;
	array[0] = start_x;
	array[1] = start_y;
	array[2] = end_x;
	array[3] = end_y;
	r_gl(GL_LINES, array, 2, 2, red, green, blue, alpha);
}

void r_3d_line_gl(float start_x, float start_y,  float start_z, float end_x, float end_y, float end_z, float red, float green, float blue, float alpha)
{
	float array[6];
	return;
	array[0] = start_x;
	array[1] = start_y;
	array[2] = start_z;
	array[3] = end_x;
	array[4] = end_y;
	array[5] = end_z;
	r_gl(GL_LINES, array, 2, 3, red, green, blue, alpha);
}

void r_elements_gl(uint draw_type, float *array, uint *reference, uint length, uint dimensions, float red, float green, float blue, float alpha)
{
	return;
	glColor4f(red, green, blue, alpha);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(dimensions, GL_FLOAT, 0, array);
	glDrawElements(draw_type, length, GL_UNSIGNED_INT, reference);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
}

void sui_set_color_array_gl(float *array, uint length, uint channels)
{
	return;
	{
		uint i;
		float pos;
		for(i = 0; i < length * channels; i++)
			pos = array[i];
	}
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(channels, GL_FLOAT, 0, array);	
}

void sui_set_normal_array_gl(float *array, uint length)
{
	return;
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0 , array);
	glEnable(GL_LIGHTING);
}

void sui_set_texture2D_array_gl(float *array, uint length, uint dimensions, uint texture)
{
	return;
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(dimensions, GL_FLOAT , 0 , array);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void r_set_vec2(float *array, uint pos, float a, float b)
{
	array += pos * 2;
	*(array++) = a;
	*(array) = b;
}

void r_set_vec3(float *array, uint pos, float a, float b, float c)
{
	array += pos * 3;
	*(array++) = a;
	*(array++) = b;
	*(array) = c;
}

void r_set_vec4(float *array, uint pos, float a, float b, float c, float d)
{
	array += pos * 4;
	*(array++) = a;
	*(array++) = b;
	*(array++) = c;
	*(array) = d;
}

void r_set_ivec2(uint *array, uint pos, uint a, uint b)
{
	array += pos * 2;
	*(array++) = a;
	*(array) = b;
}

void r_set_ivec3(uint *array, uint pos, uint a, uint b, uint c)
{
	array += pos * 3;
	*(array++) = a;
	*(array++) = b;
	*(array) = c;
}

void r_set_ivec4(uint *array, uint pos, uint a, uint b, uint c, uint d)
{
	array += pos * 4;
	*(array++) = a;
	*(array++) = b;
	*(array++) = c;
	*(array) = d;
}
*/