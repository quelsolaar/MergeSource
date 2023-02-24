#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float f_sqrtf(float number)
{
    int i;
    float x, y;
    x = number * 0.5;
    y  = number;
    i  = * (int * ) &y;
    i  = 0x5f3759df - (i >> 1);
    y  = * ( float * ) &i;
    y  = y * (1.5 - (x * y * y));
    y  = y * (1.5 - (x * y * y));
    return number * y;
}

float f_length2f(float *vec)
{
	return f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
}

float f_distance2f(float *a, float *b)
{
	float tmp[2];
	tmp[0] = a[0] - b[0];
	tmp[1] = a[1] - b[1];
	return f_sqrtf(tmp[0] * tmp[0] + tmp[1] * tmp[1]);
}

float f_dot2f(float *a, float *b)
{
	return a[0] * b[0] + a[1] * b[1];
}


void f_cross2f(float *output, float *a)
{
	output[0] = a[1];
	output[1] = -a[0];
}

void f_surface_cross2f(float *output, float *a, float *subtract)
{
	output[0] = a[1] - subtract[1];
	output[1] = subtract[0] - a[0];
}

float f_normalize2f(float *vec)
{
	float f;
	f = f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
	vec[0] /= f;
	vec[1] /= f;
	return f;
}

void f_reflect2f(float *output, float *pos, float *normal)
{
	float f;
	f = pos[0] * normal[0] + pos[1] * normal[1];
	output[0] = pos[0] - normal[0] * 2.0 * f;
	output[1] = pos[1] - normal[1] * 2.0 * f;
}

void f_flatten2f(float *output, float *pos, float *normal)
{
	float f;
	f = pos[0] * normal[0] + pos[1] * normal[1];
	output[0] = pos[0] - normal[0] * f;
	output[1] = pos[1] - normal[1] * f;
}

void f_project2f(float *output, float *plane_pos, float *normal, float *pos, float *vector)
{
	float f;
	f = normal[0] * (plane_pos[0] - pos[0]) + normal[1] * (plane_pos[1] - pos[1]);
	f /= normal[0] * vector[0] + normal[1] * vector[1];
	output[0] = pos[0] + vector[0] * f;
	output[1] = pos[1] + vector[1] * f;
}

/*---------------------------------------------------*/

float f_length3f(float *vec)
{
	return f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

float f_distance3f(float *a, float *b)
{
	float tmp[3];
	tmp[0] = a[0] - b[0];
	tmp[1] = a[1] - b[1];
	tmp[2] = a[2] - b[2];
	return f_sqrtf(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]);
}

float f_dot3f(float *a, float *b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void f_cross3f(float *output, float *a, float *b)
{
	output[0] = a[1] * b[2] - a[2] * b[1];
	output[1] = a[2] * b[0] - a[0] * b[2];
	output[2] = a[0] * b[1] - a[1] * b[0];
}



float f_normalize3f(float *vec)
{
	float f;
	f = f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
	return f;
}


float f_normalize4f(float *vec)
{
	float f;
	f = f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
	vec[3] /= f;
	return f;
}

void f_vector2f(float *vec, float *start, float *end)
{
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
}

void f_vector3f(float *vec, float *start, float *end)
{
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	vec[2] = end[2] - start[2];
}

void f_vector4f(float *vec, float *start, float *end)
{
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	vec[2] = end[2] - start[2];
	vec[3] = end[3] - start[3];
}

float f_vector_normalized2f(float *vec, float *start, float *end)
{
	float f;
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	f = f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1]);
	vec[0] /= f;
	vec[1] /= f;
	return f;
}

float f_vector_normalized3f(float *vec, float *start, float *end)
{
	float f;
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	vec[2] = end[2] - start[2];
	f = f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
	return f;
}

float f_vector_normalized4f(float *vec, float *start, float *end)
{
	float f;
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	vec[2] = end[2] - start[2];
	vec[3] = end[3] - start[3];
	f = f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
	vec[3] /= f;
	return f;
}


void f_normal2f(float *output, float *a, float *b)
{
	float f;
	output[0] = a[1] - b[1];
	output[1] = b[0] - a[0];
	f = f_sqrtf(output[0] * output[0] + output[1] * output[1]);
	output[0] /= f;
	output[1] /= f;
}

void f_normal3f(float *output, float *a, float *b, float *c)
{
	float a2[3], b2[3], f;
	a2[0] = a[0] - c[0];
	a2[1] = a[1] - c[1];
	a2[2] = a[2] - c[2];
	b2[0] = b[0] - c[0];
	b2[1] = b[1] - c[1];
	b2[2] = b[2] - c[2];
	output[0] = a2[1] * b2[2] - a2[2] * b2[1];
	output[1] = a2[2] * b2[0] - a2[0] * b2[2];
	output[2] = a2[0] * b2[1] - a2[1] * b2[0];
	f = f_sqrtf(output[0] * output[0] + output[1] * output[1] + output[2] * output[2]);
	output[0] /= f;
	output[1] /= f;
	output[2] /= f;
}

float f_area2f(float *a, float *b, float *c)
{
	float vec[2], area;
	vec[0] = a[0] - b[0];
	vec[1] = a[1] - b[1];
	area = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	vec[0] /= area;
	vec[1] /= area;
	area *= (c[0] - b[0]) * vec[1] - (c[1] - b[1]) * vec[0];
	if(area < 0.0)
		return area * -0.5;
	return area * 0.5;	
}

float f_area3f(float *a, float *b, float *c)
{
	float vec[3], side[3], area, f;
	vec[0] = a[0] - b[0];
	vec[1] = a[1] - b[1];
	vec[2] = a[2] - b[2];
	area = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= area;
	vec[1] /= area;
	vec[2] /= area;
	side[0] = c[0] - b[0];
	side[1] = c[1] - b[1];
	side[2] = c[2] - b[2];
	f = vec[0] * side[0] + vec[1] * side[1] + vec[2] * side[2];
	side[0] -= vec[0] * f;
	side[1] -= vec[1] * f;
	side[2] -= vec[2] * f;
	area *= sqrtf(side[0] * side[0] + side[1] * side[1] + side[2] * side[2]);
	return area;	
}

void f_reflect3f(float *output, float *pos, float *normal)
{
	float f;
	f = pos[0] * normal[0] + pos[1] * normal[1] + pos[2] * normal[2];
	output[0] = pos[0] - normal[0] * 2.0 * f;
	output[1] = pos[1] - normal[1] * 2.0 * f;
	output[2] = pos[2] - normal[2] * 2.0 * f;
}

void f_flatten3f(float *output, float *pos, float *normal)
{
	float f;
	f = pos[0] * normal[0] + pos[1] * normal[1] + pos[2] * normal[2];
	output[0] = pos[0] - normal[0] * f;
	output[1] = pos[1] - normal[1] * f;
	output[2] = pos[2] - normal[2] * f;
}

void f_project3f(float *output, float *plane_pos, float *normal, float *pos, float *vector)
{
	float f;
	f = normal[0] * (plane_pos[0] - pos[0]) + normal[1] * (plane_pos[1] - pos[1]) + normal[2] * (plane_pos[2] - pos[2]);
	f /= normal[0] * vector[0] + normal[1] * vector[1] + normal[2] * vector[2];
	output[0] = pos[0] + vector[0] * f;
	output[1] = pos[1] + vector[1] * f;
    output[2] = pos[2] + vector[2] * f;
}


float f_distance_to_line3f(float *line_a, float *line_b, float *pos)
{
	float f, normal[3], vec[3];	
	normal[0] = line_a[0] - line_b[0]; 
	normal[1] = line_a[1] - line_b[1]; 
	normal[2] = line_a[2] - line_b[2]; 
	f = f_sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= f;
	normal[1] /= f;
	normal[2] /= f;
	vec[0] = pos[0] - line_b[0]; 
	vec[1] = pos[1] - line_b[1]; 
	vec[2] = pos[2] - line_b[2]; 
	f = normal[0] * vec[0] + normal[1] * vec[1] + normal[2] * vec[2];
	vec[0] -= normal[0] * f;
	vec[1] -= normal[1] * f;
	vec[2] -= normal[2] * f;
	return f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}


void f_intersect2f(float *output, float *line_a0, float *line_a1, float *line_b0, float *line_b1)
{
	output[0] = (line_a0[0] * line_a1[1] - line_a0[1] * line_a1[0])	* (line_b0[0] - line_b1[0]) - 
				(line_b0[0] * line_b1[1] - line_b0[1] * line_b1[0])	* (line_a0[0] - line_a1[0]);
	output[0] /= (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]) - (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
	output[1] = (line_a0[0] * line_a1[1] - line_a0[1] * line_a1[0])	* (line_b0[1] - line_b1[1]) - 
				(line_b0[0] * line_b1[1] - line_b0[1] * line_b1[0])	* (line_a0[1] - line_a1[1]);
	output[1] /= (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]) - (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
}

void f_intersect3f(float *output, float *line_a0, float *line_a1, float *line_b0, float *line_b1)
{
	float flatten[3], tmp[3], flat_line_a0[3], flat_line_a1[3], f, length;
	flatten[0] = line_b0[0] - line_b1[0];
	flatten[1] = line_b0[1] - line_b1[1];
	flatten[2] = line_b0[2] - line_b1[2];
	f_normalize3f(flatten);
	tmp[0] = line_a0[0] - line_b0[0];
	tmp[1] = line_a0[1] - line_b0[1];
	tmp[2] = line_a0[2] - line_b0[2];
	f = tmp[0] * flatten[0] + tmp[1] * flatten[1] + tmp[2] * flatten[2];
	flat_line_a0[0] = line_a0[0] - f * flatten[0];
	flat_line_a0[1] = line_a0[1] - f * flatten[1];
	flat_line_a0[2] = line_a0[2] - f * flatten[2];
	tmp[0] = line_a1[0] - line_b0[0];
	tmp[1] = line_a1[1] - line_b0[1];
	tmp[2] = line_a1[2] - line_b0[2];
	f = tmp[0] * flatten[0] + tmp[1] * flatten[1] + tmp[2] * flatten[2];
	flat_line_a1[0] = line_a1[0] - f * flatten[0];
	flat_line_a1[1] = line_a1[1] - f * flatten[1];
	flat_line_a1[2] = line_a1[2] - f * flatten[2];
		
	tmp[0] = flat_line_a1[0] - flat_line_a0[0];
	tmp[1] = flat_line_a1[1] - flat_line_a0[1];
	tmp[2] = flat_line_a1[2] - flat_line_a0[2];
	length = f_normalize3f(tmp);
	f = tmp[0] * (line_b0[0] - flat_line_a0[0]) + 
		tmp[1] * (line_b0[1] - flat_line_a0[1]) + 
		tmp[2] * (line_b0[2] - flat_line_a0[2]);

	output[0] = line_a0[0] + (line_a1[0] - line_a0[0]) * f / length;
	output[1] = line_a0[1] + (line_a1[1] - line_a0[1]) * f / length;
	output[2] = line_a0[2] + (line_a1[2] - line_a0[2]) * f / length;
}

int f_intersect_test2f(float *line_a0, float *line_a1, float *line_b0, float *line_b1)
{
	float vec[3];
	vec[0] = line_a0[0] - line_a1[0];
	vec[1] = line_a0[1] - line_a1[1];
	if((vec[1] * (line_b0[0] - line_a1[0]) - vec[0] * (line_b0[1] - line_a1[1]) > 0) == 
		(vec[1] * (line_b1[0] - line_a1[0]) - vec[0] * (line_b1[1] - line_a1[1]) > 0))
		return 0;
	vec[0] = line_b0[0] - line_b1[0];
	vec[1] = line_b0[1] - line_b1[1];
	if((vec[1] * (line_a0[0] - line_b1[0]) - vec[0] * (line_a0[1] - line_b1[1]) > 0) == 
		(vec[1] * (line_a1[0] - line_b1[0]) - vec[0] * (line_a1[1] - line_b1[1]) > 0))
		return 0;
	return 1;
}

double f_length2d(double *vec)
{
	return sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
}

double f_distance2d(double *a, double *b)
{
	double tmp[2];
	tmp[0] = a[0] - b[0];
	tmp[1] = a[1] - b[1];
	return sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1]);
}

double f_dot2d(double *a, double *b)
{
	return a[0] * b[0] + a[1] * b[1];
}

void f_cross2d(double *output, double *a)
{
	output[0] = a[1];
	output[1] = -a[0];
}

void f_surface_cross2d(double *output, double *a, double *subtract)
{
	output[0] = a[1] - subtract[1];
	output[1] = subtract[0] - a[0];
}

double f_normalize2d(double *vec)
{
	double f;
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	vec[0] /= f;
	vec[1] /= f;
	return f;
}

double f_normalize3d(double *vec)
{
	double f;
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
	return f;
}

double f_normalize4d(double *vec)
{
	double f;
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
	vec[3] /= f;
	return f;
}

void f_vector2d(double *vec, double *start, double *end)
{
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
}

void f_vector3d(double *vec, double *start, double *end)
{
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	vec[2] = end[2] - start[2];
}

void f_vector4d(double *vec, double *start, double *end)
{
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	vec[2] = end[2] - start[2];
	vec[3] = end[3] - start[3];
}

double f_vector_normalized2d(double *vec, double *start, double *end)
{
	double f;
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	vec[0] /= f;
	vec[1] /= f;
	return f;
}

double f_vector_normalized3d(double *vec, double *start, double *end)
{
	double f;
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	vec[2] = end[2] - start[2];
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
	return f;
}

double f_vector_normalized4d(double *vec, double *start, double *end)
{
	double f;
	vec[0] = end[0] - start[0];
	vec[1] = end[1] - start[1];
	vec[2] = end[2] - start[2];
	vec[3] = end[3] - start[3];
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
	vec[3] /= f;
	return f;
}

void f_normal2d(double *output, double *a, double *b)
{
	double f;
	output[0] = a[1] - b[1];
	output[1] = b[0] - a[0];
	f = sqrt(output[0] * output[0] + output[1] * output[1]);
	output[0] /= f;
	output[1] /= f;
}

void f_normal3d(double *output, double *a, double *b, double *c)
{
	double a2[3], b2[3], f;
	a2[0] = a[0] - c[0];
	a2[1] = a[1] - c[1];
	a2[2] = a[2] - c[2];
	b2[0] = b[0] - c[0];
	b2[1] = b[1] - c[1];
	b2[2] = b[2] - c[2];
	output[0] = a2[1] * b2[2] - a2[2] * b2[1];
	output[1] = a2[2] * b2[0] - a2[0] * b2[2];
	output[2] = a2[0] * b2[1] - a2[1] * b2[0];
	f = sqrt(output[0] * output[0] + output[1] * output[1] + output[2] * output[2]);
	output[0] /= f;
	output[1] /= f;
	output[2] /= f;
}


double f_area2d(double *a, double *b, double *c)
{
	double vec[2], area;
	vec[0] = a[0] - b[0];
	vec[1] = a[1] - b[1];
	area = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
	vec[0] /= area;
	vec[1] /= area;
	area *= (c[0] - b[0]) * vec[1] - (c[1] - b[1]) * vec[0];
	if(area < 0.0)
		return area * -0.5;
	return area * 0.5;	
}

double f_area3d(double *a, double *b, double *c)
{
	double vec[3], side[3], area, f;
	vec[0] = a[0] - b[0];
	vec[1] = a[1] - b[1];
	vec[2] = a[2] - b[2];
	area = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= area;
	vec[1] /= area;
	vec[2] /= area;
	side[0] = c[0] - b[0];
	side[1] = c[1] - b[1];
	side[2] = c[2] - b[2];
	f = vec[0] * side[0] + vec[1] * side[1] + vec[2] * side[2];
	side[0] -= vec[0] * f;
	side[1] -= vec[1] * f;
	side[2] -= vec[2] * f;
	area *= sqrt(side[0] * side[0] + side[1] * side[1] + side[2] * side[2]);
	return area;	
}

void f_reflect2d(double *output, double *pos, double *normal)
{
	double f;
	f = pos[0] * normal[0] + pos[1] * normal[1];
	output[0] = pos[0] - normal[0] * 2.0 * f;
	output[1] = pos[1] - normal[1] * 2.0 * f;
}

void f_flatten2d(double *output, double *pos, double *normal)
{
	double f;
	f = pos[0] * normal[0] + pos[1] * normal[1];
	output[0] = pos[0] - normal[0] * f;
	output[1] = pos[1] - normal[1] * f;
}

void f_project2d(double *output, double *plane_pos, double *normal, double *pos, double *vector)
{
	double f;
	f = normal[0] * (plane_pos[0] - pos[0]) + normal[1] * (plane_pos[1] - pos[1]);
	f /= normal[0] * vector[0] + normal[1] * vector[1];
	output[0] = pos[0] + vector[0] * f;
	output[1] = pos[1] + vector[1] * f;
}

void f_intersect2d(double *output, double *line_a0, double *line_a1, double *line_b0, double *line_b1)
{
	output[0] = (line_a0[0] * line_a1[1] - line_a0[1] * line_a1[0])	* (line_b0[0] - line_b1[0]) - 
				(line_b0[0] * line_b1[1] - line_b0[1] * line_b1[0])	* (line_a0[0] - line_a1[0]);
	output[0] /= (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]) - (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
	output[1] = (line_a0[0] * line_a1[1] - line_a0[1] * line_a1[0])	* (line_b0[1] - line_b1[1]) - 
				(line_b0[0] * line_b1[1] - line_b0[1] * line_b1[0])	* (line_a0[1] - line_a1[1]);
	output[1] /= (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]) - (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
}


void f_intersect3d(double *output, double *line_a0, double *line_a1, double *line_b0, double *line_b1)
{
	double flatten[3], tmp[3], flat_line_a0[3], flat_line_a1[3], f, length;
	flatten[0] = line_b0[0] - line_b1[0];
	flatten[1] = line_b0[1] - line_b1[1];
	flatten[2] = line_b0[2] - line_b1[2];
	f_normalize3d(flatten);
	tmp[0] = line_a0[0] - line_b0[0];
	tmp[1] = line_a0[1] - line_b0[1];
	tmp[2] = line_a0[2] - line_b0[2];
	f = tmp[0] * flatten[0] + tmp[1] * flatten[1] + tmp[2] * flatten[2];
	flat_line_a0[0] = line_a0[0] - f * flatten[0];
	flat_line_a0[1] = line_a0[1] - f * flatten[1];
	flat_line_a0[2] = line_a0[2] - f * flatten[2];
	tmp[0] = line_a1[0] - line_b0[0];
	tmp[1] = line_a1[1] - line_b0[1];
	tmp[2] = line_a1[2] - line_b0[2];
	f = tmp[0] * flatten[0] + tmp[1] * flatten[1] + tmp[2] * flatten[2];
	flat_line_a1[0] = line_a1[0] - f * flatten[0];
	flat_line_a1[1] = line_a1[1] - f * flatten[1];
	flat_line_a1[2] = line_a1[2] - f * flatten[2];
		
	tmp[0] = flat_line_a1[0] - flat_line_a0[0];
	tmp[1] = flat_line_a1[1] - flat_line_a0[1];
	tmp[2] = flat_line_a1[2] - flat_line_a0[2];
	length = f_normalize3d(tmp);
	f = tmp[0] * (line_b0[0] - flat_line_a0[0]) + 
		tmp[1] * (line_b0[1] - flat_line_a0[1]) + 
		tmp[2] * (line_b0[2] - flat_line_a0[2]);

	output[0] = line_a0[0] + (line_a1[0] - line_a0[0]) * f / length;
	output[1] = line_a0[1] + (line_a1[1] - line_a0[1]) * f / length;
	output[2] = line_a0[2] + (line_a1[2] - line_a0[2]) * f / length;
}

int f_intersect_test2d(double *line_a0, double *line_a1, double *line_b0, double *line_b1)
{
	double vec[3];
	vec[0] = line_a0[0] - line_a1[0];
	vec[1] = line_a0[1] - line_a1[1];
	if((vec[1] * (line_b0[0] - line_a1[0]) - vec[0] * (line_b0[1] - line_a1[1]) > 0) == 
		(vec[1] * (line_b1[0] - line_a1[0]) - vec[0] * (line_b1[1] - line_a1[1]) > 0))
		return 0;
	vec[0] = line_b0[0] - line_b1[0];
	vec[1] = line_b0[1] - line_b1[1];
	if((vec[1] * (line_a0[0] - line_b1[0]) - vec[0] * (line_a0[1] - line_b1[1]) > 0) == 
		(vec[1] * (line_a1[0] - line_b1[0]) - vec[0] * (line_a1[1] - line_b1[1]) > 0))
		return 0;
	return 1;
}



double f_distance_to_line3d(double *line_a, double *line_b, double *pos)
{
	double f, normal[3], vec[3];	
	normal[0] = line_a[0] - line_b[0]; 
	normal[1] = line_a[1] - line_b[1]; 
	normal[2] = line_a[2] - line_b[2]; 
	f = f_sqrtf(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= f;
	normal[1] /= f;
	normal[2] /= f;
	vec[0] = pos[0] - line_b[0]; 
	vec[1] = pos[1] - line_b[1]; 
	vec[2] = pos[2] - line_b[2]; 
	f = normal[0] * pos[0] + normal[1] * pos[1] + normal[2] * pos[2];
	vec[0] -= normal[0] * f;
	vec[1] -= normal[1] * f;
	vec[2] -= normal[2] * f;
	return f_sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

/*---------------------------------------------------*/

double f_length3d(double *vec)
{
	return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

double f_distance3d(double *a, double *b)
{
	double tmp[3];
	tmp[0] = a[0] - b[0];
	tmp[1] = a[1] - b[1];
	tmp[2] = a[2] - b[2];
	return sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]);
}

double f_dot3d(double *a, double *b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void f_cross3d(double *output, double *a, double *b)
{
	output[0] = a[1] * b[2] - a[2] * b[1];
	output[1] = a[2] * b[0] - a[0] * b[2];
	output[2] = a[0] * b[1] - a[1] * b[0];
}


void f_surface_cross3d(double *output, double *a, double *b, double *subtract)
{
	double a2[3], b2[3];
	a2[0] = a[0] - subtract[0];
	a2[1] = a[1] - subtract[1];
	a2[2] = a[2] - subtract[2];
	b2[0] = b[0] - subtract[0];
	b2[1] = b[1] - subtract[1];
	b2[2] = b[2] - subtract[2];
	output[0] = a2[1] * b2[2] - a2[2] * b2[1];
	output[1] = a2[2] * b2[0] - a2[0] * b2[2];
	output[2] = a2[0] * b2[1] - a2[1] * b2[0];
}


void f_reflect3d(double *output, double *pos, double *normal)
{
	double f;
	f = pos[0] * normal[0] + pos[1] * normal[1] + pos[2] * normal[2];
	output[0] = pos[0] - normal[0] * 2.0 * f;
	output[1] = pos[1] - normal[1] * 2.0 * f;
	output[2] = pos[2] - normal[2] * 2.0 * f;
}

void f_flatten3d(double *output, double *pos, double *normal)
{
	double f;
	f = pos[0] * normal[0] + pos[1] * normal[1] + pos[2] * normal[2];
	output[0] = pos[0] - normal[0] * f;
	output[1] = pos[1] - normal[1] * f;
	output[2] = pos[2] - normal[2] * f;
}

void f_project3d(double *output, double *plane_pos, double *normal, double *pos, double *vector)
{
	double f;
	f = normal[0] * (plane_pos[0] - pos[0]) + normal[1] * (plane_pos[1] - pos[1]) + normal[2] * (plane_pos[2] - pos[2]);
	f /= normal[0] * vector[0] + normal[1] * vector[1] + normal[2] * vector[2];
	output[0] = pos[0] + vector[0] * f;
	output[1] = pos[1] + vector[1] * f;
    output[2] = pos[2] + vector[2] * f;
}

/*---------------------------------------------------*/


#define f_sqrt_step(shift) \
    if((0x40000000l >> shift) + root <= value)          \
    {                                                   \
        value -= (0x40000000l >> shift) + root;         \
        root = (root >> 1) | (0x40000000l >> shift);    \
    }                                                   \
    else                                                \
    {                                                   \
        root = root >> 1;                               \
    }

long f_sqrti(long value)
{
    long root = 0;
    f_sqrt_step(0);
    f_sqrt_step(2);
    f_sqrt_step(4);
    f_sqrt_step(6);
    f_sqrt_step(8);
    f_sqrt_step(10);
    f_sqrt_step(12);
    f_sqrt_step(14);
    f_sqrt_step(16);
    f_sqrt_step(18);
    f_sqrt_step(20);
    f_sqrt_step(22);
    f_sqrt_step(24);
    f_sqrt_step(26);
    f_sqrt_step(28);
    f_sqrt_step(30);
    if(root < value)
        ++root;
    return root;
}

unsigned long long f_sqrti64(unsigned long long value)
{	
	unsigned long long i, root = 0, add;
	for(i = 32; i > 0;)
	{
		i--;
		add = (((unsigned long long)1) << i) + root;
//		printf("add[%llu] = %llu (%llu %llu)\n", i, add, add * add, value);
		if(add * add <= value)
		{
//			printf("copy = %llu\n", add);
			root = add;
		}
	}
/*	add = root * root;
	exit(0);*/
	return root;
}

unsigned char f_normalize_2di(int *point, int fixed_point_multiplyer)
{
	int length;
	length = f_sqrti(point[0] * point[0] + point[1] * point[1]);
	if(length != 0)
	{
		point[0] = (point[0] * fixed_point_multiplyer) / length;
		point[1] = (point[1] * fixed_point_multiplyer) / length;
		return 1;
	}
	return 0;
}

unsigned char f_normalize_3di(int *point, int fixed_point_multiplyer)
{
	int length;
	length = f_sqrti(point[0] * point[0] + point[1] * point[1] + point[2] * point[2]);
	if(length != 0)
	{
		point[0] = (point[0] * fixed_point_multiplyer) / length;
		point[1] = (point[1] * fixed_point_multiplyer) / length;
		point[2] = (point[2] * fixed_point_multiplyer) / length;
		return 1;
	}
	return 0;
}

typedef long long int64;



unsigned char f_normalize_2di64(int64 *point, int64 fixed_point_multiplyer)
{
	int64 length;
	length = f_sqrti64(point[0] * point[0] + point[1] * point[1]);
	if(length != 0)
	{
		point[0] = (point[0] * fixed_point_multiplyer) / length;
		point[1] = (point[1] * fixed_point_multiplyer) / length;
		return 1;
	}
	return 0;
}

unsigned char f_normalize_3di64(int64 *point, int64 fixed_point_multiplyer)
{
	int64 length;
	length = f_sqrti64(point[0] * point[0] + point[1] * point[1] + point[2] * point[2]);
	if(length != 0)
	{
		point[0] = (point[0] * fixed_point_multiplyer) / length;
		point[1] = (point[1] * fixed_point_multiplyer) / length;
		point[2] = (point[2] * fixed_point_multiplyer) / length;
		return 1;
	}
	return 0;
}


void f_intersect2di(int *output, int *line_a0, int *line_a1, int *line_b0, int *line_b1)
{
	int64 i, tmp, line64_a0[2], line64_a1[2], line64_b0[2], line64_b1[2];
	line64_a0[0] = (int64)line_a0[0];
	line64_a0[1] = (int64)line_a0[1];
	line64_a1[0] = (int64)line_a1[0];
	line64_a1[1] = (int64)line_a1[1];
	line64_b0[0] = (int64)line_b0[0];
	line64_b0[1] = (int64)line_b0[1];
	line64_b1[0] = (int64)line_b1[0];
	line64_b1[1] = (int64)line_b1[1];
	i = (line64_a0[0] - line64_a1[0]) * (line64_b0[1] - line64_b1[1]) - (line64_a0[1] - line64_a1[1]) * (line64_b0[0] - line64_b1[0]);
	if(i == 0)
	{
		output[0] = (line_a0[0] + line_a1[0]) / 2;
		output[1] = (line_a0[1] + line_a1[1]) / 2;
		return;
	}
	tmp = (line64_a0[0] * line64_a1[1] - line64_a0[1] * line64_a1[0])	* (line64_b0[0] - line64_b1[0]) - 
				(line64_b0[0] * line64_b1[1] - line64_b0[1] * line64_b1[0])	* (line64_a0[0] - line64_a1[0]);
	tmp /= i;
	output[0] = (int)tmp;
	i = (line64_a0[0] - line64_a1[0]) * (line64_b0[1] - line64_b1[1]) - (line64_a0[1] - line64_a1[1]) * (line64_b0[0] - line64_b1[0]);
	if(i == 0)
	{
		output[0] = (line_a0[0] + line_a1[0]) / 2;
		output[1] = (line_a0[1] + line_a1[1]) / 2;
		return;
	}
	tmp = (line64_a0[0] * line64_a1[1] - line64_a0[1] * line64_a1[0])	* (line64_b0[1] - line64_b1[1]) - 
				(line64_b0[0] * line64_b1[1] - line64_b0[1] * line64_b1[0])	* (line64_a0[1] - line64_a1[1]);
	tmp /= i;
	output[1] = (int)tmp;
}

void f_intersect2di64(int64 *output, int64 *line_a0, int64 *line_a1, int64 *line_b0, int64 *line_b1)
{
	int64 i, tmp, line64_a0[2], line64_a1[2], line64_b0[2], line64_b1[2];
	line64_a0[0] = (int64)line_a0[0];
	line64_a0[1] = (int64)line_a0[1];
	line64_a1[0] = (int64)line_a1[0];
	line64_a1[1] = (int64)line_a1[1];
	line64_b0[0] = (int64)line_b0[0];
	line64_b0[1] = (int64)line_b0[1];
	line64_b1[0] = (int64)line_b1[0];
	line64_b1[1] = (int64)line_b1[1];
	i = (line64_a0[0] - line64_a1[0]) * (line64_b0[1] - line64_b1[1]) - (line64_a0[1] - line64_a1[1]) * (line64_b0[0] - line64_b1[0]);
	if(i == 0)
	{
		output[0] = (line_a0[0] + line_a1[0]) / 2;
		output[1] = (line_a0[1] + line_a1[1]) / 2;
		return;
	}
	tmp = (line64_a0[0] * line64_a1[1] - line64_a0[1] * line64_a1[0]) * (line64_b0[0] - line64_b1[0]) - 
				(line64_b0[0] * line64_b1[1] - line64_b0[1] * line64_b1[0])	* (line64_a0[0] - line64_a1[0]);
	tmp /= i;
	output[0] = tmp;
	i = (line64_a0[0] - line64_a1[0]) * (line64_b0[1] - line64_b1[1]) - (line64_a0[1] - line64_a1[1]) * (line64_b0[0] - line64_b1[0]);
	if(i == 0)
	{
		output[0] = (line_a0[0] + line_a1[0]) / 2;
		output[1] = (line_a0[1] + line_a1[1]) / 2;
		return;
	}
	tmp = (line64_a0[0] * line64_a1[1] - line64_a0[1] * line64_a1[0])	* (line64_b0[1] - line64_b1[1]) - 
				(line64_b0[0] * line64_b1[1] - line64_b0[1] * line64_b1[0])	* (line64_a0[1] - line64_a1[1]);
	tmp /= i;
	output[1] = tmp;
}


void f_intersect2di_working_questionmark(int *output, int *line_a0, int *line_a1, int *line_b0, int *line_b1)
{
	int i;
	i = (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]) - (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
	if(i == 0)
	{
		output[0] = (line_a0[0] + line_a1[0]) / 2;
		output[1] = (line_a0[1] + line_a1[1]) / 2;
		return;
	}
	output[0] = (line_a0[0] * line_a1[1] - line_a0[1] * line_a1[0])	* (line_b0[0] - line_b1[0]) - 
				(line_b0[0] * line_b1[1] - line_b0[1] * line_b1[0])	* (line_a0[0] - line_a1[0]);
	output[0] /= i;
	i = (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]) - (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
	if(i == 0)
	{
		output[0] = (line_a0[0] + line_a1[0]) / 2;
		output[1] = (line_a0[1] + line_a1[1]) / 2;
		return;
	}
	output[1] = (line_a0[0] * line_a1[1] - line_a0[1] * line_a1[0])	* (line_b0[1] - line_b1[1]) - 
				(line_b0[0] * line_b1[1] - line_b0[1] * line_b1[0])	* (line_a0[1] - line_a1[1]);
	output[1] /= i;
}
/*
void f_intersect2di(int *output, int *line_a0, int *line_a1, int *line_b0, int *line_b1)
{
	int i, a, b;
	i = (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]) - (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
	a = (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]);
	b = (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
	if(i == 0)
	{
		output[0] = (line_a0[0] + line_a1[0]) / 2;
		output[1] = (line_a0[1] + line_a1[1]) / 2;
		return;
	}
	output[0] = ((line_a0[0] * line_a1[1] - line_a0[1] * line_a1[0]) / i) * (line_b0[0] - line_b1[0]);
	output[0] -= ((line_b0[0] * line_b1[1] - line_b0[1] * line_b1[0]) / i) * (line_a0[0] - line_a1[0]);
	i = (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]) - (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
	a = (line_a0[0] - line_a1[0]) * (line_b0[1] - line_b1[1]);
	b = (line_a0[1] - line_a1[1]) * (line_b0[0] - line_b1[0]);
	if(i == 0)
	{
		output[0] = (line_a0[0] + line_a1[0]) / 2;
		output[1] = (line_a0[1] + line_a1[1]) / 2;
		return;
	}
	output[1] = ((line_a0[0] * line_a1[1] - line_a0[1] * line_a1[0]) / i) * (line_b0[1] - line_b1[1]); 
	output[1] -= ((line_b0[0] * line_b1[1] - line_b0[1] * line_b1[0]) / i) * (line_a0[1] - line_a1[1]);

}*/



short f_float32_to_float16(float value)
{
	union{int integer; float real;}convert;
	int sign, exponent, sig, t, a, b;
	convert.real = value;

	sign =  (convert.integer >> 16) & 0x00008000;
	exponent = ((convert.integer >> 23) & 0x000000ff) - (127 - 15);
	sig = convert.integer & 0x007fffff;

    if(exponent <= 0)
    {
		if(exponent < -10)
			return sign;
		sig = sig | 0x00800000;
		t = 14 - exponent;
		a = (1 << (t - 1)) - 1;
		b = (sig >> t) & 1;

		sig = (sig + a + b) >> t;
		return sign | sig;
	}else if(exponent == 0xff - (127 - 15))
	{
		if(sig == 0)
			return sign | 0x7c00;
		else
		{
			sig >>= 13;
			return sign | 0x7c00 | sig | (sig == 0);
		}
    }else
    {
		sig = sig + 0x00000fff + ((sig >> 13) & 1);

		if(sig & 0x00800000)
		{
			sig =  0;
			exponent += 1;
		}
		if (exponent > 30)
			return sign | 0x7c00;	/*inf*/
		return sign | (exponent << 10) | (sig >> 13);
    }
}

float f_float16_to_float32(short value)
{
	union{int integer; float real;}convert;
	int sign, exponent, sig;
	sign = (value >> 15) & 0x00000001;
	exponent = (value >> 10) & 0x0000001f;
	sig =  value & 0x000003ff;

	if (exponent == 0)
	{
		if (sig == 0)
		{
			convert.integer = sign << 31;
			return convert.real;
		}else
		{
			while(!(sig & 0x00000400))
			{
				sig <<= 1;
				exponent -=  1;
			}
			exponent += 1;
			sig &= ~0x00000400;
		}
	}else if (exponent == 31)
	{
		if (sig == 0)
		{
			convert.integer = (sign << 31) | 0x7f800000;
			return convert.real;
		}else
		{
			convert.integer = (sign << 31) | 0x7f800000 | (sig << 13);
			return convert.real;
		}
	}

	exponent = exponent + (127 - 15);
	sig = sig << 13;

	convert.integer = (sign << 31) | (exponent << 23) | sig;
	return convert.real;
}
/*
void f_image_scale_float_line_x(float *line, float *output, unsigned int in_x, unsigned int out_x, unsigned int channels)
{
	float f, i_f = 0, value, in_xf, out_xf, *temp;
	unsigned int i, j, pixel;
	in_x -= 1;
	out_x -= 1;
	in_xf = (float)in_x;	
	out_xf = (float)out_x;
	out_x *= channels;
	i_f = 1;
	for(i = 0; i < out_x + 1; i++)
	{
		i_f = (float)i;
		f = in_xf * i_f / out_xf;
		f -= i_f;
		pixel = (out_x * i) / in_x;
		pixel *= channels;
		for(j = 0; j < channels; j++)
		{
			printf("pixel %f offset %f\n", pixel, f);			
			output[i * channels + j] = line[pixel] + (line[pixel + channels] - line[pixel]) * f;
			pixel++;
		}
	}
}*/


void f_image_scale_float_line_x(float *input, float *output, unsigned int in_size, unsigned int out_size, unsigned int in_jump, unsigned int out_jump, unsigned int channels)
{
	float pixel_size_in, pixel_size_out, in_pos, out_pos, next, f, weight;
	unsigned int j, in_pixel, out_pixel;
	pixel_size_in = 1.0 / (float)in_size;
	pixel_size_out = 1.0 / (float)out_size;
	f = 1;
	in_pos = 0;
	out_pos = -0.0001;
	out_size *= out_jump;
	for(out_pixel = in_pixel = 0; out_pixel < out_size; out_pixel += out_jump)
	{
		out_pos += pixel_size_out;
		weight = 0.0;
		for(j = 0; j < channels; j++)
 			output[out_pixel + j] = 0; 
		while(1)
		{
			next = in_pos + pixel_size_in;
			if(next >= out_pos)
			{
				f = (out_pos - in_pos) / pixel_size_in; 
			//	f = 1.0;
				weight += f;
				for(j = 0; j < channels; j++)
					output[out_pixel + j] = (output[out_pixel + j] + input[in_pixel + j] * f) / weight;
				f = 1.0 - f;
				break;
			}
			weight += f;
			for(j = 0; j < channels; j++)
				output[out_pixel + j] += input[in_pixel + j] * f;
			f = 1.0;
			in_pos = next;
			in_pixel += in_jump;
		}

	}
}


void f_image_scale_test()
{
	float line[3] = {1, 0.0, 1};
	float output[9];
	unsigned int i;
	f_image_scale_float_line_x(line, output, 3, 9, 1, 1, 1);
	for(i = 0; i < 9; i++)
		printf("pixel[%i] %f\n", i, output[i]);
	exit(0);
}
/*
1 / 3.0
0.33


1, 0, 1
0.66 0.66*/

float *f_image_scale_float(float *data, unsigned int in_x, unsigned int in_y, unsigned int out_x, unsigned int out_y, unsigned int channels, unsigned int line_padding)
{
	float *tmp, *output;
	unsigned int i;
	output = malloc((sizeof *output) * out_x * out_y * channels);
	if(out_y * in_x < out_x * in_y || 1)
	{
		tmp = malloc((sizeof *tmp) * in_y * out_x * channels);
		for(i = 0; i < in_y; i++)
			f_image_scale_float_line_x(&data[i * in_x * channels], &tmp[i * out_x * channels], in_x, out_x, channels, channels, channels);

		for(i = 0; i < out_x; i++)
			f_image_scale_float_line_x(&tmp[i * channels], &output[i * channels], in_y, out_y, out_x * channels + line_padding, out_x * channels + line_padding, channels);
		free(tmp);
	}else
	{
	}

	return output;
}


unsigned char *f_image_scale_uint8(unsigned char *data, unsigned int in_x, unsigned int in_y, unsigned int out_x, unsigned int out_y, unsigned int channels, unsigned int line_padding)
{
	float *fin, *fout;
	unsigned int i, j, size, line_size;
	unsigned char *output;
	size = in_x * in_y * channels;
	fin = malloc((sizeof *fin) * size);

	line_size = in_x * channels + line_padding;
	for(i = 0; i < in_y; i++)
		for(j = 0; j < in_x * channels; j++)
			fin[i * in_x * channels + j] = (float)data[i * line_size + j] + (1.0 / 512);
	size = out_x * out_y * channels;
	fout = f_image_scale_float(fin, in_x, in_y, out_x, out_y, channels, 0);
	free(fin);
	size = out_x * out_y * channels;
	output = malloc((sizeof *output) * size);
	if(output == NULL)
	{
		free(fout);
		return NULL;
	}
	for(i = 0; i < size; i++)
		output[i] = (unsigned char)(fout[i]);
	free(fout);
	return output;
}
/*
uint8 *f_image_scale_uint8(uint8 *data, uint in_x, uint in_y, uint out_x, uint out_y, uint channels, uint line_padding)
{
	powf(float x, float y);

}*/