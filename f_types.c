#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "forge.h"

float f_randf(uint32 index)
{
	index = (index << 13) ^ index;
	return (((index * (index * index * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f) * 0.5f;
}

double f_randd(uint32 index)
{
	index = (index << 13) ^ index;
	return (((index * (index * index * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * 0.5;
}

float f_randnf(uint32 index)
{
//	static float array[7] = {0, -1, -0.4, 0.6, -0.1, 0.7, -0.2};
//	return array[index % 7];
	index = (index << 13) ^ index;
	return (((index * (index * index * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f) - 1.0f;
}

double f_randnd(uint32 index)
{
	index = (index << 13) ^ index;
	return (((index * (index * index * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) - 1.0;
}

uint f_randi(uint32 index)
{
	index = (index << 13) ^ index;
	return ((index * (index * index * 15731 + 789221) + 1376312589) & 0x7fffffff);
}

void f_rgb_to_hsv(float *output, float r, float g, float b)
{
	if(r < g)
	{
		if(r < b)
		{
			if(g < b)
			{
				output[0] = (4.0f - (g - r) / (b - r)) / 6.0f;
				output[1] = 1.0f - r / b;
				output[2] = b;
			}else
			{
				output[0] = (2.0f + (b - r) / (g - r)) / 6.0f;
				output[1] = 1.0f - r / g;
				output[2] = g;
			}
		}else
		{
			output[0] = (2.0f - (r - b) / (g - b)) / 6.0f;
			output[1] = 1.0f - b / g;
			output[2] = g;
		}
	}else
	{
		if(r < b)
		{
			output[0] = (4.0f + (r - g) / (b - g)) / 6.0f;
			output[1] = 1.0f - g / b;
			output[2] = b;
		}else
		{
			if(g < b)
			{
				output[0] = (6.0f - (b - g) / (r - g)) / 6.0f;
				output[1] = 1.0f - g / r;
				output[2] = r;
			}else if(r == g && r == b)
			{
				output[0] = 0.0f;
				output[1] = 0.0f;
				output[2] = r;
			}else
			{

				output[0] = (0.0f + (g - b) / (r - b)) / 6.0f;
				output[1] = 1.0f - b / r;
				output[2] = r;
			}
		}
	}
}

void f_hsv_to_rgb(float *output, float h, float s, float v)
{
	if(h > 1 || h < 0.0f)
		h = h - (float)((int)h);
	if(s < 0.0f)
		s = 0.0f;
	if(s > 1.0f)
		s = 1.0f;
	if(v < 0.0f)
		v = 0.0f;
	if(v > 1.0f)
		v = 1.0f;

	s = 1.0f - s;
	h *= 6.0f;
	s *= v;

	switch((uint)h)
	{
		case 0 :
			output[0] = v;
			output[1] = s + (0.0f + h) * (v - s);
			output[2] = s;
		break;
		case 1 :
			output[0] = s + (2.0f - h) * (v - s);
			output[1] = v;
			output[2] = s;
		break;
		case 2 :
			output[0] = s;
			output[1] = v;
			output[2] = s + (h - 2.0f) * (v - s);
		break;
		case 3 :
			output[0] = s;
			output[1] = s + (4.0f - h) * (v - s);
			output[2] = v;
		break;
		case 4 :
			output[0] = s + (h - 4.0f) * (v - s);
			output[1] = s;
			output[2] = v;
		break;
		case 5 :
		case 6 :
			output[0] = v;
			output[1] = s;
			output[2] = s + (6.0f - h) * (v - s);
		break;
	}

/*	switch((uint)h)
	{
		case 0 :
			output[0] = v;
			output[1] = s + (0.0f + h) * (v - s);
			output[2] = s;
		break;
		case 1 :
			output[0] = s + (2.0f - h) * (v - s);
			output[1] = v;
			output[2] = s;
		break;
		case 2 :
			output[0] = s;
			output[1] = v;
			output[2] = s + (h - 2.0f) * (v - s);
		break;
		case 3 :
			output[0] = s;
			output[1] = s + (4.0f - h) * (v - s);
			output[2] = v;
		break;
		case 4 :
			output[0] = s + (h - 4.0f) * (v - s);
			output[1] = s;
			output[2] = v;
		break;
		default :
			output[0] = v;
			output[1] = s;
			output[2] = s + (6.0f - h) * (v - s);
		break;
	}*/
}

void f_xyz_to_rgb3(float *output, float x, float y, float z)
{
	float tmp[3];
	tmp[0] = x / 100.0f;
	tmp[1] = y / 100.0f;
	tmp[2] = z / 100.0f;

	output[0] = tmp[0] *  3.2406f + tmp[1] * -1.5372f + tmp[2] * -0.4986f;
	output[1] = tmp[0] * -0.9689f + tmp[1] *  1.8758f + tmp[2] *  0.0415f;
	output[2] = tmp[0] *  0.0557f + tmp[1] * -0.2040f + tmp[2] *  1.0570f;
}

#define f_WHITEPOINT_X	0.950456
#define f_WHITEPOINT_Y	1.0
#define f_WHITEPOINT_Z	1.088754

void f_rgb_to_xyz(float *xyz, float r, float g, float b)
{
	if(r <= 0.0404482362771076f)
		r = r / 12.92f;
	else
		r = (float)pow((r + 0.055f) / 1.055f, 2.4f);
	if(g <= 0.0404482362771076f)
		g = g / 12.92f;
	else
		g = (float)pow((g + 0.055f) / 1.055f, 2.4f);
	if(b <= 0.0404482362771076f)
		b = b / 12.92f;
	else
		b = (float)pow((b + 0.055) / 1.055f, 2.4f);
	xyz[0] = 0.4123955889674142161f * r + 0.3575834307637148171f * g + 0.1804926473817015735f * b;
	xyz[1] = 0.2125862307855955516f * r + 0.7151703037034108499f * g + 0.07220049864333622685f * b;
	xyz[2] = 0.01929721549174694484f * r + 0.1191838645808485318f * g + 0.9504971251315797660f * b;
}


void f_kelvin_to_xyz(float *xyz, float kelvin)
{
	float xy[2];
	if(kelvin < 4000.0)
	{
		xy[0] = -266123900.0 / (kelvin * kelvin * kelvin) - 234258.9 / (kelvin * kelvin) + 877.6956 / kelvin + 0.179910;
		if(kelvin < 2222.0)
		{
			xy[1] = -1.1063814 * xy[0] * xy[0] * xy[0] - 1.34811020 * xy[0] * xy[0] + 2.18555832 * xy[0] - 0.20219683;
		}else
			xy[1] = -0.9529476 * xy[0] * xy[0] * xy[0] - 1.37418593 * xy[0] * xy[0] + 2.09137015 * xy[0] - 0.16748867;
	}else
	{
		xy[0] = -3025846900.0 / (kelvin * kelvin * kelvin) - 2107037.9 / (kelvin * kelvin) + 222.6347 / kelvin + 0.240390;
		xy[1] = 3.0817580 * xy[0] * xy[0] * xy[0] - 5.87338670 * xy[0] * xy[0] + 3.75112997 * xy[0] - 0.37001483;
	}
	xyz[0] = xy[0] * 1 / xy[1];
	xyz[1] = 1;
	xyz[2] = (1 - xy[0] - xy[1]) / xy[1];
}


/*
{\displaystyle x_{c}={\begin{cases}-0.2661239{\frac {10^{9}}{T^{3}}}-0.2343589{\frac {10^{6}}{T^{2}}}+0.8776956{\frac {10^{3}}{T}}+0.179910&1667{\text{K}}\leq T\leq 4000{\text{K}}\\-3.0258469{\frac {10^{9}}{T^{3}}}+2.1070379{\frac {10^{6}}{T^{2}}}+0.2226347{\frac {10^{3}}{T}}+0.240390&4000{\text{K}}\leq T\leq 25000{\text{K}}\end{cases}}}{\displaystyle x_{c}={\begin{cases}-0.2661239{\frac {10^{9}}{T^{3}}}-0.2343589{\frac {10^{6}}{T^{2}}}+0.8776956{\frac {10^{3}}{T}}+0.179910&1667{\text{K}}\leq T\leq 4000{\text{K}}\\-3.0258469{\frac {10^{9}}{T^{3}}}+2.1070379{\frac {10^{6}}{T^{2}}}+0.2226347{\frac {10^{3}}{T}}+0.240390&4000{\text{K}}\leq T\leq 25000{\text{K}}\end{cases}}}

{\displaystyle y_{c}={\begin{cases}-1.1063814x_{c}^{3}-1.34811020x_{c}^{2}+2.18555832x_{c}-0.20219683&1667{\text{K}}\leq T\leq 2222{\text{K}}\\-0.9549476x_{c}^{3}-1.37418593x_{c}^{2}+2.09137015x_{c}-0.16748867&2222{\text{K}}\leq T\leq 4000{\text{K}}\\+3.0817580x_{c}^{3}-5.87338670x_{c}^{2}+3.75112997x_{c}-0.37001483&4000{\text{K}}\leq T\leq 25000{\text{K}}\end{cases}}}y_{c}={\begin{cases}-1.1063814x_{c}^{3}-1.34811020x_{c}^{2}+2.18555832x_{c}-0.20219683&1667{\text{K}}\leq T\leq 2222{\text{K}}\\-0.9549476x_{c}^{3}-1.37418593x_{c}^{2}+2.09137015x_{c}-0.16748867&2222{\text{K}}\leq T\leq 4000{\text{K}}\\+3.0817580x_{c}^{3}-5.87338670x_{c}^{2}+3.75112997x_{c}-0.37001483&4000{\text{K}}\leq T\leq 25000{\text{K}}\end{cases}}
*/
void f_xyz_to_rgb(float *rgb, float x, float y, float z)
{	
	float min;	
	rgb[0] = 3.2406f * x - 1.5372f * y - 0.4986f * z;
	rgb[1] = -0.9689f * x + 1.8758f * y + 0.0415f * z;
	rgb[2] =  0.0557f * x - 0.2040f * y + 1.0570f * z;

	if(rgb[0] < rgb[1])
	{
		if(rgb[0] < rgb[2])
			min = rgb[0];
		else
			min = rgb[2];
	}else
	{
		if(rgb[1] < rgb[2])
			min = rgb[1];
		else
			min = rgb[2];
	}

	if(min < 0)
	{
		rgb[0] -= min;
		rgb[1] -= min;
		rgb[2] -= min;
	}
	
	if(rgb[0] <= 0.0031306684425005883f)
		rgb[0] = 12.92f * rgb[0];
	else
		rgb[0] = 1.055f * pow(rgb[0], 0.416666666666666667f) - 0.055f;
	
	if(rgb[1] <= 0.0031306684425005883f)
		rgb[1] = 12.92f * rgb[1];
	else
		rgb[1] = 1.055f * pow(rgb[1], 0.416666666666666667f) - 0.055f;

	if(rgb[2] <= 0.0031306684425005883f)
		rgb[2] = 12.92f * rgb[2];
	else
		rgb[2] = 1.055f * pow(rgb[2], 0.416666666666666667f) - 0.055f;
}

void f_xyz_to_lab(float *lab, float x, float y, float z)
{
	x /= f_WHITEPOINT_X;
	y /= f_WHITEPOINT_Y;
	z /= f_WHITEPOINT_Z;
	if(x >= 8.85645167903563082e-3f)
		x = pow(x, 0.333333333333333f);
	else
		x = (841.0f / 108.0f) * x + (4.0f / 29.0f);
	if(y >= 8.85645167903563082e-3f)
		y = pow(y, 0.333333333333333f);
	else
		y = (841.0f / 108.0f) * y + (4.0f / 29.0f);
	if(z >= 8.85645167903563082e-3f)
		z = pow(z, 0.333333333333333f);
	else
		z = (841.0f / 108.0f) * z + (4.0f / 29.0f);
	lab[0] = 116.0f * y - 16.0f;
	lab[1] = 500.0f * (x - y);
	lab[2] = 200.0f * (y - z);
}

void f_lab_to_xyz(float *xyz, float l, float a, float b)
{
	l = (l + 16.0f) / 116.0f;
	a = l + a / 500.0f;
	b = l - b / 200.0f;

	if(a >= 0.206896551724137931f)
		xyz[0] = f_WHITEPOINT_X * a * a * a;
	else
		xyz[0] = f_WHITEPOINT_X * (108.0f / 841.0f) * (a - (4.0f / 29.0f));

	if(l >= 0.206896551724137931f)
		xyz[1] = f_WHITEPOINT_Y * l * l * l;
	else
		xyz[1] = f_WHITEPOINT_Y * (108.0f / 841.0f) * (l - (4.0f / 29.0f));

	if(b >= 0.206896551724137931f)
		xyz[2] = f_WHITEPOINT_Z * b * b * b;
	else
		xyz[2] = f_WHITEPOINT_Z * (108.0f / 841.0f) * (b - (4.0f / 29.0f));
}


void f_aces_to_xyz(float *xyz, float r, float g, float b)
{
	float matrix[9] = {9.52552396e-01, 0.00000000e+00, 9.36786317e-05,
						3.43966450e-01, 7.28166097e-01, -7.21325464e-02,
							0.00000000e+00, 0.00000000e+00, 1.00882518e+00};
	xyz[0] = matrix[0] * r + matrix[1] * g + matrix[2] * b;
	xyz[1] = matrix[3] * r + matrix[4] * g + matrix[5] * b;
	xyz[2] = matrix[6] * r + matrix[7] * g + matrix[8] * b;
}

void f_xyz_to_aces(float *rgb, float x, float y, float z)
{
	float matrix[9] = {1.0498110175, 0.00000000, -0.00000974845,
						-0.4959030231, 1.3733130458, 0.0982400361,
						0.00000000, 0.00000000, 0.9912520182};
	rgb[0] = matrix[0] * x + matrix[1] * y + matrix[2] * z;
	rgb[1] = matrix[3] * x + matrix[4] * y + matrix[5] * z;
	rgb[2] = matrix[6] * x + matrix[7] * y + matrix[8] * z;
}

typedef enum{
	FORGE_CTP_INSIDE,
	FORGE_CTP_OUTSIDE_EDGE,
	FORGE_CTP_OUTSIDE_CORNER
}ForgeColorTrianglePriority;

typedef struct{
	ForgeColorTrianglePriority priority;
	float distance;
	float *prime_a;
	float *prime_b;
	float pos;
}ForgeColorNearest;

ForgeColorTrianglePriority f_xyz_to_primaries_evaluate_line(float x, float y, float *prime_a, float *prime_b, ForgeColorNearest *nearest)
{
	float vec_a[2], f, f2, dist;
	vec_a[0] = prime_b[0] - prime_a[0];
	vec_a[1] = prime_b[1] - prime_a[1];
	f = sqrt(vec_a[0] * vec_a[0] + vec_a[1] * vec_a[1]);
	vec_a[0] /= f; 
	vec_a[1] /= f; 
	f2 = vec_a[0] * (x - prime_a[0]) + vec_a[1] * (y - prime_a[1]);
	if(f2 < 0.0)
	{
		if(nearest->priority == FORGE_CTP_INSIDE)
			return;
		dist = (x - prime_a[0]) * (x - prime_a[0]) + (y - prime_a[1]) * (y - prime_a[1]);
		if(dist > nearest->distance)
			return;
		nearest->distance = dist;
		nearest->prime_a = prime_a;
		nearest->priority = FORGE_CTP_OUTSIDE_CORNER;
	}else if(f2 > f)
	{		
		if(nearest->priority == FORGE_CTP_INSIDE)
			return;
		dist = (x - prime_b[0]) * (x - prime_b[0]) + (y - prime_b[1]) * (y - prime_b[1]);
		if(dist > nearest->distance)
			return;
		nearest->distance = dist;
		nearest->prime_a = prime_b;
		nearest->priority = FORGE_CTP_OUTSIDE_CORNER;
	}else
	{	
		if(nearest->priority == FORGE_CTP_INSIDE)
			return;
		vec_a[0] = prime_a[0] + vec_a[0] * f2;
		vec_a[1] = prime_a[1] + vec_a[1] * f2;
		dist = (x - vec_a[0]) * (x - vec_a[0]) + (y - vec_a[1]) * (y - vec_a[1]);
		if(dist > nearest->distance)
			return;
		f2 /= f;
		nearest->distance = dist;
		nearest->prime_a = prime_a;
		nearest->prime_b = prime_b;
		nearest->pos = f2;
		nearest->priority = FORGE_CTP_OUTSIDE_EDGE;
	}
}


void f_xyz_to_primaries_evaluate_triangle(float *output_a, float *output_b, float *output_c, float x, float y, float *prime_a, float *prime_b, float *prime_c)
{
	float vec_a[2], pos[2], f, f2, dist, a, b, c;
	vec_a[0] = prime_b[0] - prime_a[0];
	vec_a[1] = prime_b[1] - prime_a[1];
	f = sqrt(vec_a[0] * vec_a[0] + vec_a[1] * vec_a[1]);
	vec_a[0] /= f; 
	vec_a[1] /= f; 
	c = vec_a[0] * (y - prime_a[1]) - vec_a[1] * (x - prime_a[0]);
	c /= (vec_a[0] * (prime_c[1] - prime_a[1]) - vec_a[1] * (prime_c[0] - prime_a[0]));
	b = (vec_a[0] * (x - (prime_a[0] + (prime_c[0] - prime_a[0]) * c)) + vec_a[1] * (y - (prime_a[1] + (prime_c[1] - prime_a[1]) * c))) / (f * (1.0 - c));
	a = 1.0 - b;
	*output_a += a * (1.0 - c);
	*output_b += b * (1.0 - c);
	*output_c += c;
}

ForgeColorTrianglePriority f_xyz_to_primaries_triangle_test_side(float *output_a, float *output_b, float *output_c, float x, float y, float *prime_a, float *prime_b, float *prime_c, ForgeColorNearest *nearest)
{
	int inside = 1;
	float vec_a[2];
	if((prime_b[0] - prime_a[0]) * (y - prime_a[1]) - (prime_b[1] - prime_a[1]) * (x - prime_a[0]) < 0)
	{
		if(nearest->priority == FORGE_CTP_INSIDE)
			return;
		inside = 0;
		f_xyz_to_primaries_evaluate_line(x, y, prime_a, prime_b, nearest);
	}
	if((prime_c[0] - prime_b[0]) * (y - prime_b[1]) - (prime_c[1] - prime_b[1]) * (x - prime_b[0]) < 0)
	{
		if(nearest->priority == FORGE_CTP_INSIDE)
			return;
		inside = 0;
		f_xyz_to_primaries_evaluate_line(x, y, prime_b, prime_c, nearest);
	}
	if((prime_a[0] - prime_c[0]) * (y - prime_c[1]) - (prime_a[1] - prime_c[1]) * (x - prime_c[0]) < 0)
	{
		if(nearest->priority == FORGE_CTP_INSIDE)
			return;
		inside = 0;
		f_xyz_to_primaries_evaluate_line(x, y, prime_c, prime_a, nearest);
	}
	if(!inside)
		return;
	f_xyz_to_primaries_evaluate_triangle(output_a, output_b, output_c, x, y, prime_a, prime_b, prime_c);
	nearest->priority = FORGE_CTP_INSIDE;
}

void f_xyz_to_primaries_triangle_test(float *output_a, float *output_b, float *output_c, float x, float y, float *prime_a, float *prime_b, float *prime_c, ForgeColorNearest *nearest)
{
	if((prime_b[0] - prime_a[0]) * (prime_c[1] - prime_a[1]) - (prime_b[1] - prime_a[1]) * (prime_c[0] - prime_a[0]) > 0)
		return f_xyz_to_primaries_triangle_test_side(output_a, output_b, output_c, x, y, prime_a, prime_b, prime_c, nearest);
	else
		return f_xyz_to_primaries_triangle_test_side(output_a, output_c, output_b, x, y, prime_a, prime_c, prime_b, nearest);
}

void f_xyz_to_primaries_brightness(float *output, float z, unsigned int prime_count, float *prime_coordinates)
{
	float max = 0, f, found = 1.0;
	unsigned int i;
	for(i = 0; i < prime_count; i++)
	{
		f = output[i] / prime_coordinates[i * 3 + 2];
		if(f > max)
		{
			max = f;
			found = output[i];
		}
	}
	if(z > 1.0)
		z = 1.0;
	f = z / max;
	for(i = 0; i < prime_count; i++)
		output[i] *= f / prime_coordinates[i * 3 + 2];
}

void f_xyz_to_primaries(float *output, float x, float y, float z, unsigned int prime_count, float *prime_coordinates)
{
	ForgeColorNearest nearest;
	unsigned int i, j, k;
//	z += x + y; 
	if(z < 0.00001)
	{
		for(i = 0; i < prime_count; i++)
			output[i] = 0;
		return FORGE_CTP_OUTSIDE_CORNER;
	}
//	x /= z;
//	y /= z;
//	z *= 0.3333333333;


	if(prime_count == 1)
	{
		output[0] = z;
		return FORGE_CTP_OUTSIDE_CORNER;
	}
	if(prime_count == 2)
	{
		float vec_a[2], f, f2;
		vec_a[0] = prime_coordinates[3] - prime_coordinates[0];
		vec_a[1] = prime_coordinates[4] - prime_coordinates[1];
		f = sqrt(vec_a[0] * vec_a[0] + vec_a[1] * vec_a[1]);
		vec_a[0] /= f; 
		vec_a[1] /= f; 
		f2 = vec_a[0] * (x - prime_coordinates[0]) + vec_a[1] * (y - prime_coordinates[1]);
		if(f2 < 0.0)
		{
			output[0] = z;
			output[1] = 0;
			return FORGE_CTP_OUTSIDE_CORNER;
		}else if(f2 > f)
		{
			output[0] = 0;
			output[1] = z;
			return FORGE_CTP_OUTSIDE_CORNER;
		}else
		{
			f2 /= f;
			output[0] = z * (1.0 - f2);
			output[1] = z * f2;
			return FORGE_CTP_OUTSIDE_EDGE;
		}
		
	}
	nearest.priority = FORGE_CTP_OUTSIDE_CORNER;
	nearest.distance = 1000000000.0;
	for(i = 0; i < prime_count; i++)
		output[i] = 0;

	if(prime_count == 3)
		f_xyz_to_primaries_triangle_test(&output[0], &output[1], &output[2], x, y, &prime_coordinates[0], &prime_coordinates[3], &prime_coordinates[6], &nearest);
	else
		for(i = 0; i < prime_count - 2; i++)
			for(j = i + 1; j < prime_count - 1; j++)
				for(k = j + 1; k < prime_count; k++)
					f_xyz_to_primaries_triangle_test(&output[i], &output[j], &output[k], x, y, &prime_coordinates[i * 3], &prime_coordinates[j * 3], &prime_coordinates[k * 3], &nearest);
	if(nearest.priority == FORGE_CTP_OUTSIDE_CORNER)
	{
		output[(nearest.prime_a - prime_coordinates) / 3] = 1.0;
	}else if(nearest.priority == FORGE_CTP_OUTSIDE_EDGE)
	{
		output[(nearest.prime_a - prime_coordinates) / 3] = 1.0 - nearest.pos;
		output[(nearest.prime_b - prime_coordinates) / 3] = nearest.pos;
	}
	f_xyz_to_primaries_brightness(output, z, prime_count, prime_coordinates);
}

void f_rgb_to_lab(float *lab, float r, float g, float b)
{
	float tmp[3];
	f_rgb_to_xyz(tmp, r, g, b);
	f_xyz_to_lab(lab, tmp[0], tmp[1], tmp[2]);
}

void f_lab_to_rgb(float *rgb, float l, float a, float b)
{
	float tmp[3];
	f_lab_to_xyz(tmp, l, a, b);
	f_xyz_to_rgb(rgb, tmp[0], tmp[1], tmp[2]);
}



float f_splinef(float f, float v0, float v1, float v2, float v3)
{
	float inv;
	inv = 1.0f - f;
	return ((v3 * f + v2 * inv) * f + (v2 * f + v1 * inv) * inv) * f + ((v2 * f + v1 * inv) * f + (v1 * f + v0 * inv) * inv) * inv;
}

double f_splined(double f, double v0, double v1, double v2, double v3)
{
	double inv;
	inv = 1.0f - f;
	return ((v3 * f + v2 * inv) * f + (v2 * f + v1 * inv) * inv) * f + ((v2 * f + v1 * inv) * f + (v1 * f + v0 * inv) * inv) * inv;
}

void f_spline2df(float *out, float f, float *v0, float *v1, float *v2, float *v3)
{
	float inv;
	inv = 1.0f - f;
	out[0] = ((v3[0] * f + v2[0] * inv) * f + (v2[0] * f + v1[0] * inv) * inv) * f + ((v2[0] * f + v1[0] * inv) * f + (v1[0] * f + v0[0] * inv) * inv) * inv;
	out[1] = ((v3[1] * f + v2[1] * inv) * f + (v2[1] * f + v1[1] * inv) * inv) * f + ((v2[1] * f + v1[1] * inv) * f + (v1[1] * f + v0[1] * inv) * inv) * inv;
}

void f_spline2dd(double *out, double f, double *v0, double *v1, double *v2, double *v3)
{
	double inv;
	inv = 1.0f - f;
	out[0] = ((v3[0] * f + v2[0] * inv) * f + (v2[0] * f + v1[0] * inv) * inv) * f + ((v2[0] * f + v1[0] * inv) * f + (v1[0] * f + v0[0] * inv) * inv) * inv;
	out[1] = ((v3[1] * f + v2[1] * inv) * f + (v2[1] * f + v1[1] * inv) * inv) * f + ((v2[1] * f + v1[1] * inv) * f + (v1[1] * f + v0[1] * inv) * inv) * inv;
}

void f_spline3df(float *out, float f, float *v0, float *v1, float *v2, float *v3)
{
	float inv;
	inv = 1.0f - f;
	out[0] = ((v3[0] * f + v2[0] * inv) * f + (v2[0] * f + v1[0] * inv) * inv) * f + ((v2[0] * f + v1[0] * inv) * f + (v1[0] * f + v0[0] * inv) * inv) * inv;
	out[1] = ((v3[1] * f + v2[1] * inv) * f + (v2[1] * f + v1[1] * inv) * inv) * f + ((v2[1] * f + v1[1] * inv) * f + (v1[1] * f + v0[1] * inv) * inv) * inv;
	out[2] = ((v3[2] * f + v2[2] * inv) * f + (v2[2] * f + v1[2] * inv) * inv) * f + ((v2[2] * f + v1[2] * inv) * f + (v1[2] * f + v0[2] * inv) * inv) * inv;
}

void f_spline3dd(double *out, double f, double *v0, double *v1, double *v2, double *v3)
{

	double inv;
	inv = 1.0 - f;
	out[0] = ((v3[0] * f + v2[0] * inv) * f + (v2[0] * f + v1[0] * inv) * inv) * f + ((v2[0] * f + v1[0] * inv) * f + (v1[0] * f + v0[0] * inv) * inv) * inv;
	out[1] = ((v3[1] * f + v2[1] * inv) * f + (v2[1] * f + v1[1] * inv) * inv) * f + ((v2[1] * f + v1[1] * inv) * f + (v1[1] * f + v0[1] * inv) * inv) * inv;
	out[2] = ((v3[2] * f + v2[2] * inv) * f + (v2[2] * f + v1[2] * inv) * inv) * f + ((v2[2] * f + v1[2] * inv) * f + (v1[2] * f + v0[2] * inv) * inv) * inv;
}

void f_spline4df(float *out, float f, float *v0, float *v1, float *v2, float *v3)
{
	float inv;
	inv = 1.0f - f;
	out[0] = ((v3[0] * f + v2[0] * inv) * f + (v2[0] * f + v1[0] * inv) * inv) * f + ((v2[0] * f + v1[0] * inv) * f + (v1[0] * f + v0[0] * inv) * inv) * inv;
	out[1] = ((v3[1] * f + v2[1] * inv) * f + (v2[1] * f + v1[1] * inv) * inv) * f + ((v2[1] * f + v1[1] * inv) * f + (v1[1] * f + v0[1] * inv) * inv) * inv;
	out[2] = ((v3[2] * f + v2[2] * inv) * f + (v2[2] * f + v1[2] * inv) * inv) * f + ((v2[2] * f + v1[2] * inv) * f + (v1[2] * f + v0[2] * inv) * inv) * inv;
	out[3] = ((v3[3] * f + v2[3] * inv) * f + (v2[3] * f + v1[3] * inv) * inv) * f + ((v2[3] * f + v1[3] * inv) * f + (v1[3] * f + v0[3] * inv) * inv) * inv;
}

void f_spline4dd(double *out, double f, double *v0, double *v1, double *v2, double *v3)
{
	double inv;
	inv = 1.0 - f;
	out[0] = ((v3[0] * f + v2[0] * inv) * f + (v2[0] * f + v1[0] * inv) * inv) * f + ((v2[0] * f + v1[0] * inv) * f + (v1[0] * f + v0[0] * inv) * inv) * inv;
	out[1] = ((v3[1] * f + v2[1] * inv) * f + (v2[1] * f + v1[1] * inv) * inv) * f + ((v2[1] * f + v1[1] * inv) * f + (v1[1] * f + v0[1] * inv) * inv) * inv;
	out[2] = ((v3[2] * f + v2[2] * inv) * f + (v2[2] * f + v1[2] * inv) * inv) * f + ((v2[2] * f + v1[2] * inv) * f + (v1[2] * f + v0[2] * inv) * inv) * inv;
	out[3] = ((v3[3] * f + v2[3] * inv) * f + (v2[3] * f + v1[3] * inv) * inv) * f + ((v2[3] * f + v1[3] * inv) * f + (v1[3] * f + v0[3] * inv) * inv) * inv;
}

float f_wigglef(float f, float size)
{
	float v0, v1, v2, v3;
	uint seed;
	seed = (float)f;
	f -= (float)seed;
	seed *= 2;
	size *= 2.0;
	v0 = (f_randf(seed++) - 0.5) * size;
	v1 = (f_randf(seed++) - 0.5) * size;
	v3 = (f_randf(seed++) - 0.5) * size;
	v2 = v3 * 2.0 - (f_randf(seed++) - 0.5) * size;
	return f_spline(f, v0, v1, v2, v3);
}

double f_wiggled(double f, double size)
{
	double v0, v1, v2, v3;
	uint seed;
	seed = (double)f;
	f -= (double)seed;
	seed *= 2;
	size *= 2.0;
	v0 = (f_randd(seed++) - 0.5) * size;
	v1 = (f_randd(seed++) - 0.5) * size;
	v3 = (f_randd(seed++) - 0.5) * size;
	v2 = v3 * 2.0 - (f_randd(seed++) - 0.5) * size;
	return f_spline(f, v0, v1, v2, v3);
}

void f_wiggle2df(float *out, float f, float size)
{
	float v0[2], v1[2], v2[2], v3[2];
	uint seed;
	seed = (float)f;
	f -= (float)seed;
	seed *= 4;
	size *= 2.0;
	v0[0] = (f_randf(seed++) - 0.5) * size;
	v0[1] = (f_randf(seed++) - 0.5) * size;
	v1[0] = (f_randf(seed++) - 0.5) * size;
	v1[1] = (f_randf(seed++) - 0.5) * size;
	v3[0] = (f_randf(seed++) - 0.5) * size;
	v3[1] = (f_randf(seed++) - 0.5) * size;
	v2[0] = v3[0] * 2.0 - (f_randf(seed++) - 0.5) * size;
	v2[1] = v3[1] * 2.0 - (f_randf(seed++) - 0.5) * size;
	f_spline2df(out, f, v0, v1, v2, v3);
}

void f_wiggle2dd(double *out, double f, double size)
{
	double v0[2], v1[2], v2[2], v3[2];
	uint seed;
	seed = (double)f;
	f -= (double)seed;
	seed *= 4;
	size *= 2.0;
	v0[0] = (f_randd(seed++) - 0.5) * size;
	v0[1] = (f_randd(seed++) - 0.5) * size;
	v1[0] = (f_randd(seed++) - 0.5) * size;
	v1[1] = (f_randd(seed++) - 0.5) * size;
	v3[0] = (f_randd(seed++) - 0.5) * size;
	v3[1] = (f_randd(seed++) - 0.5) * size;
	v2[0] = v3[0] * 2.0 - (f_randd(seed++) - 0.5) * size;
	v2[1] = v3[1] * 2.0 - (f_randd(seed++) - 0.5) * size;
	f_spline2dd(out, f, v0, v1, v2, v3);
}

void f_wiggle3df(float *out, float f, float size)
{
	float v0[3], v1[3], v2[3], v3[3];
	uint seed;
	seed = (float)f;
	f -= (float)seed;
	seed *= 6;
	size *= 2.0;
	v0[0] = (f_randf(seed++) - 0.5) * size;
	v0[1] = (f_randf(seed++) - 0.5) * size;
	v0[2] = (f_randf(seed++) - 0.5) * size;
	v1[0] = (f_randf(seed++) - 0.5) * size;
	v1[1] = (f_randf(seed++) - 0.5) * size;
	v1[2] = (f_randf(seed++) - 0.5) * size;
	v3[0] = (f_randf(seed++) - 0.5) * size;
	v3[1] = (f_randf(seed++) - 0.5) * size;
	v3[2] = (f_randf(seed++) - 0.5) * size;
	v2[0] = v3[0] * 2.0 - (f_randf(seed++) - 0.5) * size;
	v2[1] = v3[1] * 2.0 - (f_randf(seed++) - 0.5) * size;
	v2[2] = v3[2] * 2.0 - (f_randf(seed++) - 0.5) * size;
	f_spline3df(out, f, v0, v1, v2, v3);
}

void f_wiggle3dd(double *out, double f, double size)
{
	double v0[3], v1[3], v2[3], v3[3];
	uint seed;
	seed = (double)f;
	f -= (double)seed;
	seed *= 6;
	size *= 2.0;
	v0[0] = (f_randd(seed++) - 0.5) * size;
	v0[1] = (f_randd(seed++) - 0.5) * size;
	v0[2] = (f_randd(seed++) - 0.5) * size;
	v1[0] = (f_randd(seed++) - 0.5) * size;
	v1[1] = (f_randd(seed++) - 0.5) * size;
	v1[2] = (f_randd(seed++) - 0.5) * size;
	v3[0] = (f_randd(seed++) - 0.5) * size;
	v3[1] = (f_randd(seed++) - 0.5) * size;
	v3[2] = (f_randd(seed++) - 0.5) * size;
	v2[0] = v3[0] * 2.0 - (f_randd(seed++) - 0.5) * size;
	v2[1] = v3[1] * 2.0 - (f_randd(seed++) - 0.5) * size;
	v2[2] = v3[2] * 2.0 - (f_randd(seed++) - 0.5) * size;
	f_spline3dd(out, f, v0, v1, v2, v3);
}

#define F_EPSILON 0.000001

boolean f_raycast_trif(float orig[3], float dir[3], float vert0[3], float vert1[3], float vert2[3], float *t, float *u, float *v)
{
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det, inv_det;

	edge1[0] = vert1[0] - vert0[0];
	edge1[1] = vert1[1] - vert0[1];
	edge1[2] = vert1[2] - vert0[2]; 
	edge2[0] = vert2[0] - vert0[0];
	edge2[1] = vert2[1] - vert0[1];
	edge2[2] = vert2[2] - vert0[2];
	pvec[0] = dir[1] * edge2[2] - dir[2] * edge2[1];
	pvec[1] = dir[2] * edge2[0] - dir[0] * edge2[2];
	pvec[2] = dir[0] * edge2[1] - dir[1] * edge2[0];
	det = edge1[0] * pvec[0] + edge1[1] * pvec[1] + edge1[2] * pvec[2];

	if(det > -F_EPSILON && det < F_EPSILON)
		return FALSE;
	inv_det = 1.0 / det;

	tvec[0] = orig[0] - vert0[0];
	tvec[1] = orig[1] - vert0[1];
	tvec[2] = orig[2] - vert0[2];

	*u = (tvec[0] * pvec[0] + tvec[1] * pvec[1] + tvec[2] * pvec[2]) * inv_det;
	if(*u < 0.0 || *u > 1.0)
		return FALSE;

	qvec[0] = tvec[1] * edge1[2] - tvec[2] * edge1[1];
	qvec[1] = tvec[2] * edge1[0] - tvec[0] * edge1[2];
	qvec[2] = tvec[0] * edge1[1] - tvec[1] * edge1[0];

	*v = (dir[0] * qvec[0] + dir[1] * qvec[1] + dir[2] * qvec[2]) * inv_det;
	if(*v < 0.0 || *u + *v > 1.0)
		return FALSE;

	*t = (edge2[0] * qvec[0] + edge2[1] * qvec[1] + edge2[2] * qvec[2]) * inv_det;
	return TRUE;
}


boolean f_raycast_tri_cullf(float orig[3], float dir[3], float vert0[3], float vert1[3], float vert2[3], float *t, float *u, float *v)
{
	float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	float det, inv_det;

	edge1[0] = vert1[0] - vert0[0];
	edge1[1] = vert1[1] - vert0[1];
	edge1[2] = vert1[2] - vert0[2]; 
	edge2[0] = vert2[0] - vert0[0];
	edge2[1] = vert2[1] - vert0[1];
	edge2[2] = vert2[2] - vert0[2];
	pvec[0] = dir[1] * edge2[2] - dir[2] * edge2[1];
	pvec[1] = dir[2] * edge2[0] - dir[0] * edge2[2];
	pvec[2] = dir[0] * edge2[1] - dir[1] * edge2[0];
	det = edge1[0] * pvec[0] + edge1[1] * pvec[1] + edge1[2] * pvec[2];
 
	if(det < F_EPSILON)
		return FALSE;

	tvec[0] = orig[0] - vert0[0];
	tvec[1] = orig[1] - vert0[1];
	tvec[2] = orig[2] - vert0[2];

	*u = tvec[0] * pvec[0] + tvec[1] * pvec[1] + tvec[2] * pvec[2];
	if(*u < 0.0 || *u > det)
		return FALSE;

	qvec[0] = tvec[1] * edge1[2] - tvec[2] * edge1[1];
	qvec[1] = tvec[2] * edge1[0] - tvec[0] * edge1[2];
	qvec[2] = tvec[0] * edge1[1] - tvec[1] * edge1[0];
	*v = dir[0] * qvec[0] + dir[1] * qvec[1] + dir[2] * qvec[2];
	if(*v < 0.0 || *u + *v > det)
		return FALSE;

	*t = edge2[0] * qvec[0] + edge2[1] * qvec[1] + edge2[2] * qvec[2];
	inv_det = 1.0 / det;
	*t *= inv_det;
	*u *= inv_det;
	*v *= inv_det;

	return TRUE;
}


boolean f_raycast_trid(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v)
{
	double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det, inv_det;

	edge1[0] = vert1[0] - vert0[0];
	edge1[1] = vert1[1] - vert0[1];
	edge1[2] = vert1[2] - vert0[2]; 
	edge2[0] = vert2[0] - vert0[0];
	edge2[1] = vert2[1] - vert0[1];
	edge2[2] = vert2[2] - vert0[2];
	pvec[0] = dir[1] * edge2[2] - dir[2] * edge2[1];
	pvec[1] = dir[2] * edge2[0] - dir[0] * edge2[2];
	pvec[2] = dir[0] * edge2[1] - dir[1] * edge2[0];
	det = edge1[0] * pvec[0] + edge1[1] * pvec[1] + edge1[2] * pvec[2];

	if(det > -F_EPSILON && det < F_EPSILON)
		return FALSE;
	inv_det = 1.0 / det;

	tvec[0] = orig[0] - vert0[0];
	tvec[1] = orig[1] - vert0[1];
	tvec[2] = orig[2] - vert0[2];

	*u = (tvec[0] * pvec[0] + tvec[1] * pvec[1] + tvec[2] * pvec[2]) * inv_det;
	if(*u < 0.0 || *u > 1.0)
		return FALSE;

	qvec[0] = tvec[1] * edge1[2] - tvec[2] * edge1[1];
	qvec[1] = tvec[2] * edge1[0] - tvec[0] * edge1[2];
	qvec[2] = tvec[0] * edge1[1] - tvec[1] * edge1[0];

	*v = (dir[0] * qvec[0] + dir[1] * qvec[1] + dir[2] * qvec[2]) * inv_det;
	if(*v < 0.0 || *u + *v > 1.0)
		return FALSE;

	*t = (edge2[0] * qvec[0] + edge2[1] * qvec[1] + edge2[2] * qvec[2]) * inv_det;
	return TRUE;
}


boolean f_raycast_tri_culld(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v)
{
	double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	double det, inv_det;

	edge1[0] = vert1[0] - vert0[0];
	edge1[1] = vert1[1] - vert0[1];
	edge1[2] = vert1[2] - vert0[2]; 
	edge2[0] = vert2[0] - vert0[0];
	edge2[1] = vert2[1] - vert0[1];
	edge2[2] = vert2[2] - vert0[2];
	pvec[0] = dir[1] * edge2[2] - dir[2] * edge2[1];
	pvec[1] = dir[2] * edge2[0] - dir[0] * edge2[2];
	pvec[2] = dir[0] * edge2[1] - dir[1] * edge2[0];
	det = edge1[0] * pvec[0] + edge1[1] * pvec[1] + edge1[2] * pvec[2];
 
	if(det < F_EPSILON)
		return FALSE;

	tvec[0] = orig[0] - vert0[0];
	tvec[1] = orig[1] - vert0[1];
	tvec[2] = orig[2] - vert0[2];

	*u = tvec[0] * pvec[0] + tvec[1] * pvec[1] + tvec[2] * pvec[2];
	if(*u < 0.0 || *u > det)
		return FALSE;

	qvec[0] = tvec[1] * edge1[2] - tvec[2] * edge1[1];
	qvec[1] = tvec[2] * edge1[0] - tvec[0] * edge1[2];
	qvec[2] = tvec[0] * edge1[1] - tvec[1] * edge1[0];
	*v = dir[0] * qvec[0] + dir[1] * qvec[1] + dir[2] * qvec[2];
	if(*v < 0.0 || *u + *v > det)
		return FALSE;

	*t = edge2[0] * qvec[0] + edge2[1] * qvec[1] + edge2[2] * qvec[2];
	inv_det = 1.0 / det;
	*t *= inv_det;
	*u *= inv_det;
	*v *= inv_det;

	return TRUE;
}

int f_shape_inside2df(const float *array, uint point_count, float x, float y)
{
	uint count = 0;
	float *end;
	end = &array[point_count * 2];
	if(end[-2] < x)
	{		
		if(*array >= x)
		{		
			if(y < end[-1] + (array[1] - end[-1]) * (x - end[-2]) / (array[0] - end[-2]))
				count = 1;
			goto positive_start;
		}
	}else
	{	
		if(*array < x)
		{
			if(y < end[-1] + (array[1] - end[-1]) * (x - end[-2]) / (array[0] - end[-2]))
				count = 1;
		}else
			goto positive_start;
	}
	while(TRUE)
	{
		*end = x + 1;
		array += 2;
		while(*array < x)
			array += 2;
		if(array == end)
			return count & 1;
		if(y < array[-1] + (array[1] - array[-1]) * (x - array[-2]) / (array[0] - array[-2]))
			count++;
		positive_start : 
		*end = x - 1;
		array += 2;
		while(*array >= x)		
			array += 2;
		if(array == end)
			return count & 1;
		if(y < array[-1] + (array[1] - array[-1]) * (x - array[-2]) / (array[0] - array[-2]))
			count++;
	}
}

int f_shape_inside2df_read(float *array, uint point_count, float x, float y)
{
	uint count = 0;
	float *end;
	end = &array[point_count * 2];
	if(end[-2] < x)
	{		
		if(*array >= x)
		{		
			if(y < end[-1] + (array[1] - end[-1]) * (x - end[-2]) / (array[0] - end[-2]))
				count = 1;
			goto positive_start;
		}
	}else
	{	
		if(*array < x)
		{
			if(y < end[-1] + (array[1] - end[-1]) * (x - end[-2]) / (array[0] - end[-2]))
				count = 1;
		}else
			goto positive_start;
	}
	while(TRUE)
	{
		array += 2;
		if(array == end)
			return count & 1;
		while(*array < x)
		{		
			array += 2;
			if(array == end)
				return count & 1;
		}
		if(y < array[-1] + (array[1] - array[-1]) * (x - array[-2]) / (array[0] - array[-2]))
			count++;
		positive_start : 
		array += 2;
		if(array == end)
			return count & 1;
		while(*array >= x)
		{		
			array += 2;
			if(array == end)
				return count & 1;
		}
		if(y < array[-1] + (array[1] - array[-1]) * (x - array[-2]) / (array[0] - array[-2]))
			count++;
	}
}



int f_shape_inside2dd(double *array, uint point_count, double x, double y)
{
	uint count = 0;
	double *end;
	end = &array[point_count * 2];
	if(end[-2] < x)
	{		
		if(*array >= x)
		{		
			if(y < end[-1] + (array[1] - end[-1]) * (x - end[-2]) / (array[0] - end[-2]))
				count = 1;
			goto positive_start;
		}
	}else
	{	
		if(*array < x)
		{
			if(y < end[-1] + (array[1] - end[-1]) * (x - end[-2]) / (array[0] - end[-2]))
				count = 1;
		}else
			goto positive_start;
	}
	while(TRUE)
	{
		array += 2;
		if(array == end)
			return count & 1;
		while(*array < x)
		{		
			array += 2;
			if(array == end)
				return count & 1;
		}
		if(y < array[-1] + (array[1] - array[-1]) * (x - array[-2]) / (array[0] - array[-2]))
			count++;
		positive_start : 
		array += 2;
		if(array == end)
			return count & 1;
		while(*array >= x)
		{		
			array += 2;
			if(array == end)
				return count & 1;
		}
		if(y < array[-1] + (array[1] - array[-1]) * (x - array[-2]) / (array[0] - array[-2]))
			count++;
	}
}

void f_rasterize_polygon(uint *grid, uint x, uint y, float x0, float y0, float x1, float y1, float x2, float y2)
{
	float top[2], mid[2], bottom[2], start_pos, start_add, end_pos, end_add, f;
	uint i, j, end_y, end_x;

	if(y0 > y1)
	{
		if(y0 > y2)
		{
			top[0] = x0;
			top[1] = y0;
			if(y1 > y2)
			{
				mid[0] = x1;
				mid[1] = y1;
				bottom[0] = x2;
				bottom[1] = y2;
			}else
			{
				mid[0] = x2;
				mid[1] = y2;
				bottom[0] = x1;
				bottom[1] = y1;
			}
		}else
		{
			top[0] = x2;
			top[1] = y2;
			mid[0] = x0;
			mid[1] = y0;
			bottom[0] = x1;
			bottom[1] = y1;
		}
	}else
	{
		if(y0 > y2)
		{
			top[0] = x1;
			top[1] = y1;
			mid[0] = x0;
			mid[1] = y0;
			bottom[0] = x2;
			bottom[1] = y2;
		}else
		{
			bottom[0] = x0;
			bottom[1] = y0;
			if(y1 < y2)
			{
				top[0] = x2;
				top[1] = y2;
				mid[0] = x1;
				mid[1] = y1;
			}else
			{
				top[0] = x1;
				top[1] = y1;
				mid[0] = x2;
				mid[1] = y2;
			}
		}
	}
	
	end_y = (uint)(mid[1] + 1.0) * x;
	start_add = (top[0] - bottom[0]) / (top[1] - bottom[1]);
	end_add = (mid[0] - bottom[0]) / (mid[1] - bottom[1]);
	if(start_add > end_add)
	{
		f = start_add;
		start_add = end_add;
		end_add = f;
	}
	f = bottom[1] - (float)((uint)bottom[1]);
	start_pos = (uint)bottom[0] - start_add * f;
	end_pos = (uint)bottom[0] - end_add * f;
	f = bottom[0] - (float)((uint)bottom[0]);
	start_pos += f;
	end_pos += f;

	for(i = (uint)bottom[1] * x; i < end_y; i += x)
	{
		end_x = end_pos;
		if(end_x > x)
			end_x = x;
		for(j = start_pos + 1; j < end_pos; j++)
		{
			grid[i + j] = TRUE;
		}
		start_pos += start_add;
		end_pos += end_add;
	}

	end_y = (uint)(mid[1] - 0.0) * x;	
	start_add = -(bottom[0] - top[0]) / (bottom[1] - top[1]);
	end_add = -(mid[0] - top[0]) / (mid[1] - top[1]);
	if(start_add > end_add)
	{
		f = start_add;
		start_add = end_add;
		end_add = f;
	}
	f = top[1] - (float)((uint)top[1]);
	start_pos = (uint)top[0] + start_add * f;
	end_pos = (uint)top[0] + end_add * f;
	f = top[0] - (float)((uint)top[0]);
	start_pos += f + 1.0;
	end_pos += f;

	for(i = (uint)top[1] * x; i > end_y; i -= x)
	{
		end_x = end_pos;
		if(end_x > x)
			end_x = x;
		for(j = start_pos; j < end_pos; j++)
		{
			grid[i + j] = 2;
		}
		start_pos += start_add;
		end_pos += end_add;
	}
}


void f_tangent(float *output, float *v0, float *v1, float *v2, float *uv0, float *uv1, float *uv2)
{
	float x0, x1, y0, y1, z0, z1, s0, s1, t0, t1, r;
	x0 = v1[0] - v0[0];
	x1 = v2[0] - v0[0];
	y0 = v1[1] - v0[1];
	y1 = v2[1] - v0[1];
	z0 = v1[2] - v0[2];
	z1 = v2[2] - v0[2];
	s0 = uv1[0] - uv0[0];
	s1 = uv2[0] - uv0[0];
	t0 = uv1[1] - uv0[1];
	t1 = uv2[1] - uv0[1];
	r = 1.0f / (s0 * t1 - s1 * t0);
	output[0] = (t1 * x0 - t0 * x1) * r; 
	output[1] = (t1 * y0 - t0 * y1) * r;
	output[2] = (t1 * z0 - t0 * z1) * r;
   /*    (s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                (s1 * z2 - s2 * z1) * r);
   */
}