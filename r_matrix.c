#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "r_include.h"


RMatrix r_default_matrix; 
void *r_matrix_state = NULL; 

void r_matrix_set(RMatrix *m)
{
	if(m == NULL)
		r_matrix_state = &r_default_matrix;
	else
		r_matrix_state = m;
}

void r_viewport(uint x_start, uint y_start, uint x_end, uint y_end)
{
	glViewport(x_start, y_start, x_end, y_end);
}



RMatrix *r_matrix_get()
{
	if(!((RMatrix *)r_matrix_state)->multiplied)
	{
		f_matrix_multiplyf(((RMatrix *)r_matrix_state)->modelviewprojection, ((RMatrix *)r_matrix_state)->projection, ((RMatrix *)r_matrix_state)->matrix[((RMatrix *)r_matrix_state)->current]);
		((RMatrix *)r_matrix_state)->multiplied = TRUE;
	}
	return (RMatrix *)r_matrix_state;
}


void r_matrix_frustum(RMatrix *matrix, float left, float right, float bottom, float top, float znear, float zfar)
{ 
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;

	matrix->projection[0] = 2.0 * znear / (right - left);
	matrix->projection[1] = 0.0;
	matrix->projection[2] = 0.0;
	matrix->projection[3] = 0.0;

	matrix->projection[4] = 0.0;
 	matrix->projection[5] = 2.0 * znear / (top - bottom);
	matrix->projection[6] = 0.0;
	matrix->projection[7] = 0.0;

	matrix->projection[8] = (right + left) / (right - left);
 	matrix->projection[9] = (top + bottom) / (top - bottom);
	matrix->projection[10] = -(zfar + znear) / (zfar - znear);
	matrix->projection[11] = -1.0;

	matrix->projection[12] = 0.0;
	matrix->projection[13] = 0.0;
	matrix->projection[14] = -2.0 * zfar * znear / (zfar - znear);
	matrix->projection[15] = 0.0;
	matrix->aspect = (top - bottom) / (right - left);
	matrix->multiplied = FALSE;
	matrix->frustum[0] = (left + right) / znear * 0.5;
	matrix->frustum[1] = (right - left) / znear * 0.5;
	matrix->frustum[2] = (bottom + top) / znear * 0.5;
	matrix->frustum[3] = (top - bottom) / znear * 0.5;
}

void r_matrix_ortho(RMatrix *matrix, float left, float right, float bottom, float top, float znear, float zfar)
{ 
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	matrix->projection[0] = 2.0f / (right - left);
	matrix->projection[1] = 0.0f;
	matrix->projection[2] = 0.0f;
	matrix->projection[3] = 0.0f;

	matrix->projection[4] = 0.0f;
 	matrix->projection[5] = 2.0f  / (top - bottom);
	matrix->projection[6] = 0.0f;
	matrix->projection[7] = 0.0f;

	matrix->projection[8] = 0.0f;
 	matrix->projection[9] = 0.0f;
	matrix->projection[10] = -2.0f / (zfar - znear);
	matrix->projection[11] = 0.0f;

	matrix->projection[12] = (right + left) / (right - left);
	matrix->projection[13] = (top + bottom) / (top - bottom);
	matrix->projection[14] = (zfar + znear) / (zfar - znear);
	matrix->projection[15] = 1.0f;
	matrix->aspect = (top - bottom) / (right - left);
	matrix->multiplied = FALSE;
}

void r_matrix_identity(RMatrix *matrix)
{
	static float t = 0.0;
	float *m, f = 1.0;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	matrix->aspect = 1.0;
	matrix->current = 0;
	m = matrix->matrix[0];
	matrix->multiplied = FALSE;
	m[0] = 1.0f;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = 1.0f;
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = 1.0f;
	m[11] = 0.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
	r_matrix_frustum(matrix, -0.05 * f, 0.05 * f, -0.05 * f, 0.05 * f, 0.05 * f, 100.0f);
}

void r_matrix_push(RMatrix *matrix)
{
	float *m, *m2;
	uint i;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(matrix->current > R_MATRIX_STACK_SIZE - 2)
	{
		uint *a = NULL;
		printf("RELINQUISH Error: r_matrix_pop one too many (%u) matrixes in the stack.\n", matrix->current);
		a[0] = 0;
		exit(0);
	}
	m = matrix->matrix[matrix->current];
	m2 = matrix->matrix[++matrix->current];
	for(i = 0; i < 16; i++)
		m2[i] = m[i];
	matrix->multiplied = FALSE;
}

void r_matrix_pop(RMatrix *matrix)
{
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(matrix->current > 0)
		--matrix->current;
	else
	{
		printf("RELINQUISH Error: r_matrix_pop one too many times.\n");
		exit(0);
	}
	matrix->multiplied = FALSE;
}

void r_matrix_matrix_mult(RMatrix *matrix, float *mult)
{
	float out[16], *m;
	uint i;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	m = matrix->matrix[matrix->current];
	f_matrix_multiplyf(out, matrix->matrix[matrix->current], mult);
	for(i = 0; i < 16; i++)
		m[i] = out[i];
	matrix->multiplied = FALSE;
}

void r_matrix_rotate(RMatrix *matrix, float angle, float x, float y, float z)
{
	float m[16], f, a, c, d;
	f = sqrt(x * x + y * y + z * z);
	x /= f;
	y /= f;
	z /= f;
	angle *= PI * 2 / 360; 
	c = cos(angle);
	a = 1.0f - c;
	d = sin(angle);

	m[0] = x * x * a + c;
	m[1] = x * y * a + z * d;
	m[2] = x * z * a - y * d;
	m[3] = 0.0f;
	m[4] = y * x * a - z * d;
	m[5] = y * y * a + c;
	m[6] = y * z * a + x * d;
	m[7] = 0.0f;		
	m[8] = z * x * a + y * d;
	m[9] = z * y * a - x * d;
	m[10] = z * z * a + c;
	m[11] = 0.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;

	r_matrix_matrix_mult(matrix, m);
}

void r_matrix_scale(RMatrix *matrix, float x, float y, float z)
{
	float m[16];
	m[0] = x;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;
	m[4] = 0.0f;
	m[5] = y;
	m[6] = 0.0f;
	m[7] = 0.0f;		
	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = z;
	m[11] = 0.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
	r_matrix_matrix_mult(matrix, m);
}

void r_matrix_translate(RMatrix *matrix, float x, float y, float z)
{
	float m[16];
	m[0] = 1.0f;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;
	m[4] = 0.0f;
	m[5] = 1.0f;
	m[6] = 0.0f;
	m[7] = 0.0f;		
	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = 1.0f;
	m[11] = 0.0f;
	m[12] = x;
	m[13] = y;
	m[14] = z;
	m[15] = 1.0f;
	r_matrix_matrix_mult(matrix, m);
}


void r_matrix_normalize_scale(RMatrix *matrix)
{
	float *m;
	m = matrix->matrix[matrix->current];
	f_normalize3f(&m[0]);
	f_normalize3f(&m[4]);
	f_normalize3f(&m[8]);
}

void r_matrix_cube_map(RMatrix *matrix, uint side)
{
	switch(side)
	{
		case 0 :
			r_matrix_rotate(matrix, 90, 0, 1, 0);
		break;
		case 1 :
			r_matrix_rotate(matrix, -90, 0, 1, 0);
		break;
		case 2 :
			r_matrix_rotate(matrix, 180, 0, 1, 0);
			r_matrix_rotate(matrix, 90, 1, 0, 0);
		break;
		case 3 :
			r_matrix_rotate(matrix, 180, 0, 1, 0);
			r_matrix_rotate(matrix, -90, 1, 0, 0);
		break;
		case 4 :
			r_matrix_rotate(matrix, 180, 0, 1, 0);
		break;
		case 5 :
		break;
	}
}


void r_matrix_projection_screend(RMatrix *matrix, double *output, double x, double y, double z)
{
	float *model;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	model = matrix->matrix[matrix->current];
	output[0] = (model[0] * x) + (model[4] * y) + (model[8] * z) + model[12];
	output[1] = (model[1] * x) + (model[5] * y) + (model[9] * z) + model[13];
	output[2] = (model[2] * x) + (model[6] * y) + (model[10] * z) + model[14];
	output[0] = output[0] / -output[2] / matrix->frustum[1] - matrix->frustum[0];
	output[1] = output[1] / -output[2] / matrix->frustum[1] - matrix->frustum[2];
}

void r_matrix_projection_screenf(RMatrix *matrix, float *output, float x, float y, float z)
{
	float *model;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	model = matrix->matrix[matrix->current];
	output[0] = (model[0] * x) + (model[4] * y) + (model[8] * z) + model[12];
	output[1] = (model[1] * x) + (model[5] * y) + (model[9] * z) + model[13];
	output[2] = (model[2] * x) + (model[6] * y) + (model[10] * z) + model[14];
	output[0] = output[0] / -output[2] / matrix->frustum[1] - matrix->frustum[0];
	output[1] = output[1] / -output[2] / matrix->frustum[1] - matrix->frustum[2];
}

void r_matrix_projection_camerad(RMatrix *matrix, double *output, double x, double y, double z)
{
	float *model;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	model = matrix->matrix[matrix->current];
	output[0] = (model[0] * x) + (model[4] * y) + (model[8] * z) + model[12];
	output[1] = (model[1] * x) + (model[5] * y) + (model[9] * z) + model[13];
	output[2] = (model[2] * x) + (model[6] * y) + (model[10] * z) + model[14];
}

void r_matrix_projection_cameraf(RMatrix *matrix, float *output, float x, float y, float z)
{
	float *model;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	model = matrix->matrix[matrix->current];
	output[0] = (model[0] * x) + (model[4] * y) + (model[8] * z) + model[12];
	output[1] = (model[1] * x) + (model[5] * y) + (model[9] * z) + model[13];
	output[2] = (model[2] * x) + (model[6] * y) + (model[10] * z) + model[14];
}

void r_matrix_projection_worldf_save(RMatrix *matrix, float *output, float x, float y, float z)
{
	float f, m[16], out[4];
	float *model;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	out[0] = x;
	out[1] = y;
	out[2] = z;
	out[3] = 1.0;
	out[1] /= matrix->aspect;

	f_matrix_reverse4f(m, matrix->modelviewprojection);
	printf("in %f %f %f %f\n", out[0], out[1], out[2], out[3]);
	f_transform4f(out, m, out[0], out[1], out[2], out[3]);
	f_transform4f(out, matrix->modelviewprojection, out[0], out[1], out[2], out[3]);
	printf("out %f %f %f %f\n", out[0], out[1], out[2], out[3]);

	model = matrix->modelviewprojection;
	printf("M1 %f %f %f %f\n", model[0], model[1], model[2], model[3]);
	printf("M1 %f %f %f %f\n", model[4], model[5], model[6], model[7]);
	printf("M1 %f %f %f %f\n", model[8], model[9], model[10], model[11]);
	printf("M1 %f %f %f %f\n", model[12], model[13], model[14], model[15]);

	f_matrix_reverse4f(m, matrix->matrix[matrix->current]);
	f_transform3f(output, m, output[0], output[1], output[2]);
}


void r_matrix_projection_worldf_old(RMatrix *matrix, float *output, float x, float y, float z)
{
	float f, center[4], x_axis[4], y_axis[4], z_axis[4], out[4];
	float *model;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	out[0] = x * z;
	out[1] = y * z;
	out[2] = z;
	out[3] = 1.0;
	out[1] /= matrix->aspect;
	model = matrix->projection;
	f_transform4f(center, model, 0, 0, 0, 1);
	f_transform4f(x_axis, model, 1, 0, 0, 1);
	f_transform4f(y_axis, model, 0, 1, 0, 1);
	f_transform4f(z_axis, model, 0, 0, 1, 1);
	out[0] -= center[0];
	out[1] -= center[1];
	out[2] -= center[2];
	out[3] -= center[3];
	x_axis[0] -= center[0];
	x_axis[1] -= center[1];
	x_axis[2] -= center[2];
	x_axis[3] -= center[3];
	y_axis[0] -= center[0];
	y_axis[1] -= center[1];
	y_axis[2] -= center[2];
	y_axis[3] -= center[3];
	z_axis[0] -= center[0];
	z_axis[1] -= center[1];
	z_axis[2] -= center[2];
	z_axis[3] -= center[3];
	center[0] = x_axis[0] * x_axis[0] + x_axis[1] * x_axis[1] + x_axis[2] * x_axis[2] + x_axis[3] * x_axis[3];
	center[1] = y_axis[0] * y_axis[0] + y_axis[1] * y_axis[1] + y_axis[2] * y_axis[2] + y_axis[3] * y_axis[3];
	center[2] = z_axis[0] * z_axis[0] + z_axis[1] * z_axis[1] + z_axis[2] * z_axis[2] + z_axis[3] * z_axis[3];
	x_axis[0] /= center[0];
	x_axis[1] /= center[0];
	x_axis[2] /= center[0];
	x_axis[3] /= center[0];
	y_axis[0] /= center[1];
	y_axis[1] /= center[1];
	y_axis[2] /= center[1];
	y_axis[3] /= center[1];
	z_axis[0] /= center[2];
	z_axis[1] /= center[2];
	z_axis[2] /= center[2];
	z_axis[3] /= center[2];
	out[0] = out[0] * x_axis[0] + out[1] * x_axis[1] + out[2] * x_axis[2] + out[3] * x_axis[3];
	out[1] = out[0] * y_axis[0] + out[1] * y_axis[1] + out[2] * y_axis[2] + out[3] * x_axis[3];
	out[2] = out[0] * z_axis[0] + out[1] * z_axis[1] + out[2] * z_axis[2] + out[3] * x_axis[3];
	model = matrix->matrix[matrix->current];
	f_transform3f(center, model, 0, 0, 0);
	f_transform3f(x_axis, model, 1, 0, 0);
	f_transform3f(y_axis, model, 0, 1, 0);
	f_transform3f(z_axis, model, 0, 0, 1);
	out[0] -= center[0];
	out[1] -= center[1];
	out[2] -= center[2];
	x_axis[0] -= center[0];
	x_axis[1] -= center[1];
	x_axis[2] -= center[2];
	y_axis[0] -= center[0];
	y_axis[1] -= center[1];
	y_axis[2] -= center[2];
	z_axis[0] -= center[0];
	z_axis[1] -= center[1];
	z_axis[2] -= center[2];
	center[0] = x_axis[0] * x_axis[0] + x_axis[1] * x_axis[1] + x_axis[2] * x_axis[2];
	center[1] = y_axis[0] * y_axis[0] + y_axis[1] * y_axis[1] + y_axis[2] * y_axis[2];
	center[2] = z_axis[0] * z_axis[0] + z_axis[1] * z_axis[1] + z_axis[2] * z_axis[2];
	x_axis[0] /= center[0];
	x_axis[1] /= center[0];
	x_axis[2] /= center[0];
	y_axis[0] /= center[1];
	y_axis[1] /= center[1];
	y_axis[2] /= center[1];
	z_axis[0] /= center[2];
	z_axis[1] /= center[2];
	z_axis[2] /= center[2];
	output[0] = out[0] * x_axis[0] + out[1] * x_axis[1] + out[2] * x_axis[2];
	output[1] = out[0] * y_axis[0] + out[1] * y_axis[1] + out[2] * y_axis[2];
	output[2] = out[0] * z_axis[0] + out[1] * z_axis[1] + out[2] * z_axis[2];
}

void r_matrix_projection_worldf(RMatrix *matrix, float *output, float x, float y, float z)
{
	float f, center[3], x_axis[3], y_axis[3], z_axis[3], out[3];
	float *model;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	out[0] = (x + matrix->frustum[0]) * -z * matrix->frustum[1];
	out[1] = (y + matrix->frustum[2]) * -z * matrix->frustum[1];
	out[2] = z; 
	model = matrix->matrix[matrix->current];
	f_transform3f(center, model, 0, 0, 0);
	f_transform3f(x_axis, model, 1, 0, 0);
	f_transform3f(y_axis, model, 0, 1, 0);
	f_transform3f(z_axis, model, 0, 0, 1);
	out[0] -= center[0];
	out[1] -= center[1];
	out[2] -= center[2];
	x_axis[0] -= center[0];
	x_axis[1] -= center[1];
	x_axis[2] -= center[2];
	y_axis[0] -= center[0];
	y_axis[1] -= center[1];
	y_axis[2] -= center[2];
	z_axis[0] -= center[0];
	z_axis[1] -= center[1];
	z_axis[2] -= center[2];
	center[0] = x_axis[0] * x_axis[0] + x_axis[1] * x_axis[1] + x_axis[2] * x_axis[2];
	center[1] = y_axis[0] * y_axis[0] + y_axis[1] * y_axis[1] + y_axis[2] * y_axis[2];
	center[2] = z_axis[0] * z_axis[0] + z_axis[1] * z_axis[1] + z_axis[2] * z_axis[2];
	x_axis[0] /= center[0];
	x_axis[1] /= center[0];
	x_axis[2] /= center[0];
	y_axis[0] /= center[1];
	y_axis[1] /= center[1];
	y_axis[2] /= center[1];
	z_axis[0] /= center[2];
	z_axis[1] /= center[2];
	z_axis[2] /= center[2];
	output[0] = out[0] * x_axis[0] + out[1] * x_axis[1] + out[2] * x_axis[2];
	output[1] = out[0] * y_axis[0] + out[1] * y_axis[1] + out[2] * y_axis[2];
	output[2] = out[0] * z_axis[0] + out[1] * z_axis[1] + out[2] * z_axis[2];
}


void r_matrix_projection_worldd(RMatrix *matrix, double *output, double x, double y, double z)
{
	double f, center[3], x_axis[3], y_axis[3], z_axis[3], out[3], m[16];
	float *model;
	uint i;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	out[0] = x * z;
	out[1] = y * z;
	out[2] = z;
	out[1] /= matrix->aspect;
	model = matrix->projection;
	for(i = 0; i < 16; i++)
		m[i] = model[i];
	f_transform3d(center, m, 0, 0, 0);
	f_transform3d(x_axis, m, 1, 0, 0);
	f_transform3d(y_axis, m, 0, 1, 0);
	f_transform3d(z_axis, m, 0, 0, 1);
	out[0] -= center[0];
	out[1] -= center[1];
	out[2] -= center[2];
	x_axis[0] -= center[0];
	x_axis[1] -= center[1];
	x_axis[2] -= center[2];
	y_axis[0] -= center[0];
	y_axis[1] -= center[1];
	y_axis[2] -= center[2];
	z_axis[0] -= center[0];
	z_axis[1] -= center[1];
	z_axis[2] -= center[2];
	center[0] = x_axis[0] * x_axis[0] + x_axis[1] * x_axis[1] + x_axis[2] * x_axis[2];
	center[1] = y_axis[0] * y_axis[0] + y_axis[1] * y_axis[1] + y_axis[2] * y_axis[2];
	center[2] = z_axis[0] * z_axis[0] + z_axis[1] * z_axis[1] + z_axis[2] * z_axis[2];
	x_axis[0] /= center[0];
	x_axis[1] /= center[0];
	x_axis[2] /= center[0];
	y_axis[0] /= center[1];
	y_axis[1] /= center[1];
	y_axis[2] /= center[1];
	z_axis[0] /= center[2];
	z_axis[1] /= center[2];
	z_axis[2] /= center[2];
	out[0] = out[0] * x_axis[0] + out[1] * x_axis[1] + out[2] * x_axis[2];
	out[1] = out[0] * y_axis[0] + out[1] * y_axis[1] + out[2] * y_axis[2];
	out[2] = out[0] * z_axis[0] + out[1] * z_axis[1] + out[2] * z_axis[2];
	model = matrix->matrix[matrix->current];
	f_transform3d(center, m, 0, 0, 0);
	f_transform3d(x_axis, m, 1, 0, 0);
	f_transform3d(y_axis, m, 0, 1, 0);
	f_transform3d(z_axis, m, 0, 0, 1);
	out[0] -= center[0];
	out[1] -= center[1];
	out[2] -= center[2];
	x_axis[0] -= center[0];
	x_axis[1] -= center[1];
	x_axis[2] -= center[2];
	y_axis[0] -= center[0];
	y_axis[1] -= center[1];
	y_axis[2] -= center[2];
	z_axis[0] -= center[0];
	z_axis[1] -= center[1];
	z_axis[2] -= center[2];
	center[0] = x_axis[0] * x_axis[0] + x_axis[1] * x_axis[1] + x_axis[2] * x_axis[2];
	center[1] = y_axis[0] * y_axis[0] + y_axis[1] * y_axis[1] + y_axis[2] * y_axis[2];
	center[2] = z_axis[0] * z_axis[0] + z_axis[1] * z_axis[1] + z_axis[2] * z_axis[2];
	x_axis[0] /= center[0];
	x_axis[1] /= center[0];
	x_axis[2] /= center[0];
	y_axis[0] /= center[1];
	y_axis[1] /= center[1];
	y_axis[2] /= center[1];
	z_axis[0] /= center[2];
	z_axis[1] /= center[2];
	z_axis[2] /= center[2];
	output[0] = out[0] * x_axis[0] + out[1] * x_axis[1] + out[2] * x_axis[2];
	output[1] = out[0] * y_axis[0] + out[1] * y_axis[1] + out[2] * y_axis[2];
	output[2] = out[0] * z_axis[0] + out[1] * z_axis[1] + out[2] * z_axis[2];
}

void r_matrix_projection_vertexf(RMatrix *matrix, float *output, float *vertex, float x, float y)
{
	float *model, z;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	model = matrix->modelviewprojection;
	z = (model[2] * vertex[0]) + (model[6] * vertex[1]) + (model[10] * vertex[2]) + model[14];
	r_matrix_projection_worldf(matrix, output, x, y, -z);
}

void r_matrix_projection_vertexd(RMatrix *matrix, double *output, double *vertex, double x, double y)
{
	float *model;
	double z;
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	if(!matrix->multiplied)
	{
		f_matrix_multiplyf(matrix->modelviewprojection, matrix->projection, matrix->matrix[matrix->current]);
		matrix->multiplied = TRUE;
	}
	model = matrix->modelviewprojection;
	z = (model[2] * vertex[0]) + (model[6] * vertex[1]) + (model[10] * vertex[2]) + model[14];
	r_matrix_projection_worldd(matrix, output, x, y, z);
}

boolean r_matrix_projection_surfacef(RMatrix *matrix, float *output, float *pos, uint axis, float x, float y)
{
	float camera[3], pointer[3], axis_vec[3] = {0, 0, 0}, zero[3] = {0, 0, 0};
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	r_matrix_projection_worldf(matrix, pointer, x, y, -1);
	r_matrix_projection_worldf(matrix, camera, 0, 0, 0);
	pointer[0] -= camera[0];
	pointer[1] -= camera[1];
	pointer[2] -= camera[2];
/*	if(0 < pointer[0] * camera[0] + pointer[1] * camera[1] + pointer[2] * camera[2])
		return FALSE;*/
	f_normalize3f(pointer);
	axis_vec[axis] = 1.0;
	f_project3f(output, pos, axis_vec, camera, pointer);
	output[axis] = pos[axis];
	return TRUE;
}


boolean r_matrix_projection_axisf(RMatrix *matrix, float *output, float *pos, uint axis, float x, float y)
{
	float camera[3], pointer[3], axis_vec[3] = {0, 0, 0}, zero[3] = {0, 0, 0};
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	r_matrix_projection_worldf(matrix, pointer, x, y, -1);
	r_matrix_projection_worldf(matrix, camera, 0, 0, 0);
	pointer[0] -= camera[0];
	pointer[1] -= camera[1];
	pointer[2] -= camera[2];
/*	if(0 < pointer[0] * camera[0] + pointer[1] * camera[1] + pointer[2] * camera[2])
		return FALSE;*/
	f_normalize3f(pointer);
	axis_vec[0] = camera[0] - pos[0];
	axis_vec[1] = camera[1] - pos[1];
	axis_vec[2] = camera[2] - pos[2];
	axis_vec[axis % 3] = 0.0;
	f_normalize3f(axis_vec);
	f_project3f(output, pos, axis_vec, camera, pointer);
	output[(axis + 1) % 3] = pos[(axis + 1) % 3];
	output[(axis + 2) % 3] = pos[(axis + 2) % 3];
	return TRUE;
}



boolean r_matrix_projection_vectorf(RMatrix *matrix, float *output, float *pos, float *vec, float x, float y)
{
	float f, camera[3], pointer[3], axis_vec[3], normalized[3], zero[3] = {0, 0, 0};
	if(matrix == NULL)
		matrix = (RMatrix *)r_matrix_state;
	r_matrix_projection_worldf(matrix, pointer, x, y, -1);
	r_matrix_projection_worldf(matrix, camera, 0, 0, 0);
	pointer[0] -= camera[0];
	pointer[1] -= camera[1];
	pointer[2] -= camera[2];
	if(0 < pointer[0] * camera[0] + pointer[1] * camera[1] + pointer[2] * camera[2])
		return FALSE;
	f_normalize3f(pointer);
	axis_vec[0] = camera[0] - pos[0];
	axis_vec[1] = camera[1] - pos[1];
	axis_vec[2] = camera[2] - pos[2];
//	printf("axis_vec %f %f %f\n", axis_vec[0], axis_vec[1], axis_vec[2]);
	normalized[0] = vec[0];
	normalized[1] = vec[1];
	normalized[2] = vec[2];
	f_normalize3f(normalized);
//	printf("normalized %f %f %f\n", normalized[0], normalized[1], normalized[2]);
	f = normalized[0] * axis_vec[0] + normalized[1] * axis_vec[1] + normalized[2] * axis_vec[2];
	axis_vec[0] -= normalized[0] * f;
	axis_vec[1] -= normalized[1] * f;
	axis_vec[2] -= normalized[2] * f;
	f_normalize3f(axis_vec);
//	printf("axis_vec %f %f %f\n", axis_vec[0], axis_vec[1], axis_vec[2]);
//	printf("pos %f %f %f\n", pos[0], pos[1], pos[2]);
//	printf("camera %f %f %f\n", camera[0], camera[1], camera[2]);
//	printf("pointer %f %f %f\n", pointer[0], pointer[1], pointer[2]);
	f_project3f(output, pos, axis_vec, camera, pointer);
//	printf("output %f %f %f\n", output[0], output[1], output[2]);
	output[0] -= pos[0];
	output[1] -= pos[1];
	output[2] -= pos[2];
	f = normalized[0] * output[0] + normalized[1] * output[1] + normalized[2] * output[2];
//	printf("normalized = %f %f %f\n", normalized[0], normalized[1], normalized[2]);
//	printf("output %f %f %f\n", output[0], output[1], output[2]);
//	printf("f = %f\n", f);
	output[0] = pos[0] + normalized[0] * f;
	output[1] = pos[1] + normalized[1] * f;
	output[2] = pos[2] + normalized[2] * f;
	return TRUE;
}