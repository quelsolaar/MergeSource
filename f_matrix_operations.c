#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void f_matrix_clearf(float *matrix)
{
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;
	matrix[4] = 0;
	matrix[5] = 1;
	matrix[6] = 0;
	matrix[7] = 0;
	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = 1;
	matrix[11] = 0;
	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 1;
}

void f_transform3f(float *output, const float *matrix, const float x, const float y, const float z)
{
	output[0] = (matrix[0] * x) + (matrix[4] * y) + (matrix[8] * z) + matrix[12];
	output[1] = (matrix[1] * x) + (matrix[5] * y) + (matrix[9] * z) + matrix[13];
	output[2] = (matrix[2] * x) + (matrix[6] * y) + (matrix[10] * z) + matrix[14];
}

void f_transforminv3f(float *out, const float *matrix, float x, float y, float z)
{
	x -= matrix[12];
	y -= matrix[13];
	z -= matrix[14];
	out[0] = (matrix[0] * x) + (matrix[1] * y) + (matrix[2] * z);
	out[1] = (matrix[4] * x) + (matrix[5] * y) + (matrix[6] * z);
	out[2] = (matrix[8] * x) + (matrix[9] * y) + (matrix[10] * z);
}

void f_transforminv_scaled3f(float *output, const float *matrix, float x, float y, float z)
{
	float center[3], x_axis[3], y_axis[3], z_axis[3], out[3];
	f_transform3f(center, matrix, 0, 0, 0);
	f_transform3f(x_axis, matrix, 1, 0, 0);
	f_transform3f(y_axis, matrix, 0, 1, 0);
	f_transform3f(z_axis, matrix, 0, 0, 1);
	out[0] = x - center[0];
	out[1] = y - center[1];
	out[2] = z - center[2];
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

void f_transform4f(float *output, const float *matrix, const float x, const float y, const float z, const double w)
{
	output[0] = (matrix[0] * x) + (matrix[4] * y) + (matrix[8] * z) + (matrix[12] * w);
	output[1] = (matrix[1] * x) + (matrix[5] * y) + (matrix[9] * z) + (matrix[13] * w);
	output[2] = (matrix[2] * x) + (matrix[6] * y) + (matrix[10] * z) + (matrix[14] * w);
	output[3] = (matrix[3] * x) + (matrix[7] * y) + (matrix[11] * z) + (matrix[15] * w);
}

void f_matrix_multiplyf(float *output, const float *a, const float *b)
{
	output[0] = (b[0] * a[0]) + (b[1] * a[4]) + (b[2] * a[8]) + (b[3] * a[12]);
	output[4] = (b[4] * a[0]) + (b[5] * a[4]) + (b[6] * a[8]) + (b[7] * a[12]);
	output[8] = (b[8] * a[0]) + (b[9] * a[4]) + (b[10] * a[8]) + (b[11] * a[12]);
	output[12] = (b[12] * a[0]) + (b[13] * a[4]) + (b[14] * a[8]) + (b[15] * a[12]);

	output[1] = (b[0] * a[1]) + (b[1] * a[5]) + (b[2] * a[9]) + (b[3] * a[13]);
	output[5] = (b[4] * a[1]) + (b[5] * a[5]) + (b[6] * a[9]) + (b[7] * a[13]);
	output[9] = (b[8] * a[1]) + (b[9] * a[5]) + (b[10] * a[9]) + (b[11] * a[13]);
	output[13] = (b[12] * a[1]) + (b[13] * a[5]) + (b[14] * a[9]) + (b[15] * a[13]);

	output[2] = (b[0] * a[2]) + (b[1] * a[6]) + (b[2] * a[10]) + (b[3] * a[14]);
	output[6] = (b[4] * a[2]) + (b[5] * a[6]) + (b[6] * a[10]) + (b[7] * a[14]);
	output[10] = (b[8] * a[2]) + (b[9] * a[6]) + (b[10] * a[10]) + (b[11] * a[14]);
	output[14] = (b[12] * a[2]) + (b[13] * a[6]) + (b[14] * a[10]) + (b[15] * a[14]);

	output[3] = (b[0] * a[3]) + (b[1] * a[7]) + (b[2] * a[11]) + (b[3] * a[15]);
	output[7] = (b[4] * a[3]) + (b[5] * a[7]) + (b[6] * a[11]) + (b[7] * a[15]);
	output[11] = (b[8] * a[3]) + (b[9] * a[7]) + (b[10] * a[11]) + (b[11] * a[15]);
	output[15] = (b[12] * a[3]) + (b[13] * a[7]) + (b[14] * a[11]) + (b[15] * a[15]);
} 

void f_matrix_reverse4d(double *output, const double *matrix)
{
	output[0] = matrix[5] * matrix[10] * matrix[15] + matrix[6] * matrix[11] * matrix[13] + matrix[7] * matrix[9] * matrix[14] - matrix[5] * matrix[11] * matrix[14] - matrix[6] * matrix[9] * matrix[15] - matrix[7] * matrix[10] * matrix[13];
	output[1] = matrix[1] * matrix[11] * matrix[14] + matrix[2] * matrix[9] * matrix[15] + matrix[3] * matrix[10] * matrix[13] - matrix[1] * matrix[10] * matrix[15] - matrix[2] * matrix[11] * matrix[13] - matrix[3] * matrix[9] * matrix[14];
	output[2] = matrix[1] * matrix[6] * matrix[15] + matrix[2] * matrix[7] * matrix[13] + matrix[3] * matrix[5] * matrix[14] - matrix[1] * matrix[7] * matrix[14] - matrix[2] * matrix[5] * matrix[15] - matrix[3] * matrix[6] * matrix[13];
	output[3] = matrix[1] * matrix[7] * matrix[10] + matrix[2] * matrix[5] * matrix[11] + matrix[3] * matrix[6] * matrix[9] - matrix[1] * matrix[6] * matrix[11] - matrix[2] * matrix[7] * matrix[9] - matrix[3] * matrix[5] * matrix[10];
	output[4] = matrix[4] * matrix[11] * matrix[14] + matrix[6] * matrix[8] * matrix[15] + matrix[7] * matrix[10] * matrix[12] - matrix[4] * matrix[10] * matrix[15] - matrix[6] * matrix[11] * matrix[12] - matrix[7] * matrix[8] * matrix[14];
	output[5] = matrix[0] * matrix[10] * matrix[15] + matrix[2] * matrix[11] * matrix[12] + matrix[3] * matrix[8] * matrix[14] - matrix[0] * matrix[11] * matrix[14] - matrix[2] * matrix[8] * matrix[15] - matrix[3] * matrix[10] * matrix[12];
	output[6] = matrix[0] * matrix[7] * matrix[14] + matrix[2] * matrix[4] * matrix[15] + matrix[3] * matrix[6] * matrix[12] - matrix[0] * matrix[6] * matrix[15] - matrix[2] * matrix[7] * matrix[12] - matrix[3] * matrix[4] * matrix[14];
	output[7] = matrix[0] * matrix[6] * matrix[11] + matrix[2] * matrix[7] * matrix[8] + matrix[3] * matrix[4] * matrix[10] - matrix[0] * matrix[7] * matrix[10] - matrix[2] * matrix[4] * matrix[11] - matrix[3] * matrix[6] * matrix[8];
	output[8] = matrix[4] * matrix[9] * matrix[15] + matrix[5] * matrix[11] * matrix[12] + matrix[7] * matrix[8] * matrix[13] - matrix[4] * matrix[11] * matrix[13] - matrix[5] * matrix[8] * matrix[15] - matrix[7] * matrix[9] * matrix[12];
	output[9] = matrix[0] * matrix[11] * matrix[13] + matrix[1] * matrix[8] * matrix[15] + matrix[3] * matrix[9] * matrix[12] - matrix[0] * matrix[9] * matrix[15] - matrix[1] * matrix[11] * matrix[12] - matrix[3] * matrix[8] * matrix[13];
	output[10] = matrix[0] * matrix[5] * matrix[15] + matrix[1] * matrix[7] * matrix[12] + matrix[3] * matrix[4] * matrix[13] - matrix[0] * matrix[7] * matrix[13] - matrix[1] * matrix[4] * matrix[15] - matrix[3] * matrix[5] * matrix[12];
	output[11] = matrix[0] * matrix[7] * matrix[9] + matrix[1] * matrix[4] * matrix[11] + matrix[3] * matrix[5] * matrix[8] - matrix[0] * matrix[5] * matrix[11] - matrix[1] * matrix[7] * matrix[8] - matrix[3] * matrix[4] * matrix[9];
	output[12] = matrix[4] * matrix[10] * matrix[13] + matrix[5] * matrix[8] * matrix[14] + matrix[6] * matrix[9] * matrix[12] - matrix[4] * matrix[9] * matrix[14] - matrix[5] * matrix[10] * matrix[12] - matrix[6] * matrix[8] * matrix[13];
	output[13] = matrix[0] * matrix[9] * matrix[14] + matrix[1] * matrix[10] * matrix[12] + matrix[2] * matrix[8] * matrix[13] - matrix[0] * matrix[10] * matrix[13] - matrix[1] * matrix[8] * matrix[14] - matrix[2] * matrix[9] * matrix[12];
	output[14] = matrix[0] * matrix[6] * matrix[13] + matrix[1] * matrix[4] * matrix[14] + matrix[2] * matrix[5] * matrix[12] - matrix[0] * matrix[5] * matrix[14] - matrix[1] * matrix[6] * matrix[12] - matrix[2] * matrix[4] * matrix[13];
	output[15] = matrix[0] * matrix[5] * matrix[10] + matrix[1] * matrix[6] * matrix[8] + matrix[2] * matrix[4] * matrix[9] - matrix[0] * matrix[6] * matrix[9] - matrix[1] * matrix[4] * matrix[10] - matrix[2] * matrix[5] * matrix[8];
}

void f_matrix_reverse4f(float *output, const float *matrix)
{
	output[0] = matrix[5] * matrix[10] * matrix[15] + matrix[6] * matrix[11] * matrix[13] + matrix[7] * matrix[9] * matrix[14] - matrix[5] * matrix[11] * matrix[14] - matrix[6] * matrix[9] * matrix[15] - matrix[7] * matrix[10] * matrix[13];
	output[1] = matrix[1] * matrix[11] * matrix[14] + matrix[2] * matrix[9] * matrix[15] + matrix[3] * matrix[10] * matrix[13] - matrix[1] * matrix[10] * matrix[15] - matrix[2] * matrix[11] * matrix[13] - matrix[3] * matrix[9] * matrix[14];
	output[2] = matrix[1] * matrix[6] * matrix[15] + matrix[2] * matrix[7] * matrix[13] + matrix[3] * matrix[5] * matrix[14] - matrix[1] * matrix[7] * matrix[14] - matrix[2] * matrix[5] * matrix[15] - matrix[3] * matrix[6] * matrix[13];
	output[3] = matrix[1] * matrix[7] * matrix[10] + matrix[2] * matrix[5] * matrix[11] + matrix[3] * matrix[6] * matrix[9] - matrix[1] * matrix[6] * matrix[11] - matrix[2] * matrix[7] * matrix[9] - matrix[3] * matrix[5] * matrix[10];
	output[4] = matrix[4] * matrix[11] * matrix[14] + matrix[6] * matrix[8] * matrix[15] + matrix[7] * matrix[10] * matrix[12] - matrix[4] * matrix[10] * matrix[15] - matrix[6] * matrix[11] * matrix[12] - matrix[7] * matrix[8] * matrix[14];
	output[5] = matrix[0] * matrix[10] * matrix[15] + matrix[2] * matrix[11] * matrix[12] + matrix[3] * matrix[8] * matrix[14] - matrix[0] * matrix[11] * matrix[14] - matrix[2] * matrix[8] * matrix[15] - matrix[3] * matrix[10] * matrix[12];
	output[6] = matrix[0] * matrix[7] * matrix[14] + matrix[2] * matrix[4] * matrix[15] + matrix[3] * matrix[6] * matrix[12] - matrix[0] * matrix[6] * matrix[15] - matrix[2] * matrix[7] * matrix[12] - matrix[3] * matrix[4] * matrix[14];
	output[7] = matrix[0] * matrix[6] * matrix[11] + matrix[2] * matrix[7] * matrix[8] + matrix[3] * matrix[4] * matrix[10] - matrix[0] * matrix[7] * matrix[10] - matrix[2] * matrix[4] * matrix[11] - matrix[3] * matrix[6] * matrix[8];
	output[8] = matrix[4] * matrix[9] * matrix[15] + matrix[5] * matrix[11] * matrix[12] + matrix[7] * matrix[8] * matrix[13] - matrix[4] * matrix[11] * matrix[13] - matrix[5] * matrix[8] * matrix[15] - matrix[7] * matrix[9] * matrix[12];
	output[9] = matrix[0] * matrix[11] * matrix[13] + matrix[1] * matrix[8] * matrix[15] + matrix[3] * matrix[9] * matrix[12] - matrix[0] * matrix[9] * matrix[15] - matrix[1] * matrix[11] * matrix[12] - matrix[3] * matrix[8] * matrix[13];
	output[10] = matrix[0] * matrix[5] * matrix[15] + matrix[1] * matrix[7] * matrix[12] + matrix[3] * matrix[4] * matrix[13] - matrix[0] * matrix[7] * matrix[13] - matrix[1] * matrix[4] * matrix[15] - matrix[3] * matrix[5] * matrix[12];
	output[11] = matrix[0] * matrix[7] * matrix[9] + matrix[1] * matrix[4] * matrix[11] + matrix[3] * matrix[5] * matrix[8] - matrix[0] * matrix[5] * matrix[11] - matrix[1] * matrix[7] * matrix[8] - matrix[3] * matrix[4] * matrix[9];
	output[12] = matrix[4] * matrix[10] * matrix[13] + matrix[5] * matrix[8] * matrix[14] + matrix[6] * matrix[9] * matrix[12] - matrix[4] * matrix[9] * matrix[14] - matrix[5] * matrix[10] * matrix[12] - matrix[6] * matrix[8] * matrix[13];
	output[13] = matrix[0] * matrix[9] * matrix[14] + matrix[1] * matrix[10] * matrix[12] + matrix[2] * matrix[8] * matrix[13] - matrix[0] * matrix[10] * matrix[13] - matrix[1] * matrix[8] * matrix[14] - matrix[2] * matrix[9] * matrix[12];
	output[14] = matrix[0] * matrix[6] * matrix[13] + matrix[1] * matrix[4] * matrix[14] + matrix[2] * matrix[5] * matrix[12] - matrix[0] * matrix[5] * matrix[14] - matrix[1] * matrix[6] * matrix[12] - matrix[2] * matrix[4] * matrix[13];
	output[15] = matrix[0] * matrix[5] * matrix[10] + matrix[1] * matrix[6] * matrix[8] + matrix[2] * matrix[4] * matrix[9] - matrix[0] * matrix[6] * matrix[9] - matrix[1] * matrix[4] * matrix[10] - matrix[2] * matrix[5] * matrix[8];
}
void f_matrix_cleard(double *matrix)
{
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;
	matrix[4] = 0;
	matrix[5] = 1;
	matrix[6] = 0;
	matrix[7] = 0;
	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = 1;
	matrix[11] = 0;
	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 1;
}

void f_transform3d(double *out, const double *matrix, const double x, const double y, const double z)
{
	out[0] = (matrix[0] * x) + (matrix[4] * y) + (matrix[8] * z) + matrix[12];
	out[1] = (matrix[1] * x) + (matrix[5] * y) + (matrix[9] * z) + matrix[13];
	out[2] = (matrix[2] * x) + (matrix[6] * y) + (matrix[10] * z) + matrix[14];
}

void f_transforminv3d(double *out, const double *matrix, double x, double y, double z)
{
	x = x - matrix[12];
	y = y - matrix[13];
	z = z - matrix[14];
	out[0] = (matrix[0] * x) + (matrix[1] * y) + (matrix[2] * z);
	out[1] = (matrix[4] * x) + (matrix[5] * y) + (matrix[6] * z);
	out[2] = (matrix[8] * x) + (matrix[9] * y) + (matrix[10] * z);
}

void f_transform4d(double *out, const double *matrix, const double x, const double y, const double z, const double w)
{
	out[0] = (matrix[0] * x) + (matrix[4] * y) + (matrix[8] * z) + (matrix[12] * w);
	out[1] = (matrix[1] * x) + (matrix[5] * y) + (matrix[9] * z) + (matrix[13] * w);
	out[2] = (matrix[2] * x) + (matrix[6] * y) + (matrix[10] * z) + (matrix[14] * w);
	out[3] = (matrix[3] * x) + (matrix[7] * y) + (matrix[11] * z) + (matrix[15] * w);
}

void f_matrix_multiplyd(double *output, const double *a, const double *b)
{
	output[0] = (b[0] * a[0]) + (b[1] * a[4]) + (b[2] * a[8]) + (b[3] * a[12]);
	output[4] = (b[4] * a[0]) + (b[5] * a[4]) + (b[6] * a[8]) + (b[7] * a[12]);
	output[8] = (b[8] * a[0]) + (b[9] * a[4]) + (b[10] * a[8]) + (b[11] * a[12]);
	output[12] = (b[12] * a[0]) + (b[13] * a[4]) + (b[14] * a[8]) + (b[15] * a[12]);

	output[1] = (b[0] * a[1]) + (b[1] * a[5]) + (b[2] * a[9]) + (b[3] * a[13]);
	output[5] = (b[4] * a[1]) + (b[5] * a[5]) + (b[6] * a[9]) + (b[7] * a[13]);
	output[9] = (b[8] * a[1]) + (b[9] * a[5]) + (b[10] * a[9]) + (b[11] * a[13]);
	output[13] = (b[12] * a[1]) + (b[13] * a[5]) + (b[14] * a[9]) + (b[15] * a[13]);

	output[2] = (b[0] * a[2]) + (b[1] * a[6]) + (b[2] * a[10]) + (b[3] * a[14]);
	output[6] = (b[4] * a[2]) + (b[5] * a[6]) + (b[6] * a[10]) + (b[7] * a[14]);
	output[10] = (b[8] * a[2]) + (b[9] * a[6]) + (b[10] * a[10]) + (b[11] * a[14]);
	output[14] = (b[12] * a[2]) + (b[13] * a[6]) + (b[14] * a[10]) + (b[15] * a[14]);

	output[3] = (b[0] * a[3]) + (b[1] * a[7]) + (b[2] * a[11]) + (b[3] * a[15]);
	output[7] = (b[4] * a[3]) + (b[5] * a[7]) + (b[6] * a[11]) + (b[7] * a[15]);
	output[11] = (b[8] * a[3]) + (b[9] * a[7]) + (b[10] * a[11]) + (b[11] * a[15]);
	output[15] = (b[12] * a[3]) + (b[13] * a[7]) + (b[14] * a[11]) + (b[15] * a[15]);
}

void f_quaternion_to_rotationd(double *rotation, double x, double y, double z, double w)
{
	double a, b;         
	a = 2.0 * (y * z + w * x);
	b = w * w - x * x - y * y + z * z;
	rotation[0] = 0.0;
	if (a != 0.0 && b != 0.0) 
		rotation[0] = atan2(a, b);
	a = -2.0 * (x * z - w * y);
	rotation[1] = 0.0;
	if(a >= 1.0 )
		rotation[1] = 3.141592653 / 2.0;
	else if(a <= -1.0 )
		rotation[1] = -3.141592653 /2.0;
	else 
		rotation[1] = asin(a);
	a = 2.0 * (x * y + w * z);
	b = w * w + x * x - y * y - z * z;
	rotation[2] = 0.0;
	if(a != 0.0 && b != 0.0)
		rotation[2] = atan2(a, b);
}


void f_quaternion_to_rotationf(float *rotation, float x, float y, float z, float w)
{
	float a, b;         
	a = 2.0 * (y * z + w * x);
	b = w * w - x * x - y * y + z * z;
	rotation[0] = 0.0;
	if (a != 0.0 && b != 0.0) 
		rotation[0] = atan2f(a, b);
	a = -2.0 * (x * z - w * y);
	rotation[1] = 0.0;
	if(a >= 1.0 )
		rotation[1] = 3.141592653 / 2.0;
	else if(a <= -1.0 )
		rotation[1] = -3.141592653 /2.0;
	else 
		rotation[1] = asinf(a);
	a = 2.0 * (x * y + w * z);
	b = w * w + x * x - y * y - z * z;
	rotation[2] = 0.0;
	if(a != 0.0 && b != 0.0)
		rotation[2] = atan2f(a, b);
}

void f_quaternion_to_matrixf(float *matrix, float x, float y, float z, float w)
{
	float xx, xy, xz, xw, yy, yz, yw, zz, zw; 
	xx = sqrt(x * x + y * y + z * z + w * w);
	x /= xx;
	y /= xx;
	z /= xx;
	w /= xx;
	xx = x * x;
    xy = x * y;
    xz = x * z;
    xw = x * w;
    yy = y * y;
    yz = y * z;
    yw = y * w;
    zz = z * z;
    zw = z * w;
    matrix[0]  = 1 - 2 * (yy + zz);
    matrix[1]  = 2 * (xy - zw);
    matrix[2]  = 2 * (xz + yw);
    matrix[4]  = 2 * (xy + zw);
    matrix[5]  = 1 - 2 * (xx + zz);
    matrix[6]  = 2 * (yz - xw);
    matrix[8]  = 2 * (xz - yw);
    matrix[9]  = 2 * (yz + xw);
    matrix[10] = 1 - 2 * (xx + yy);
    matrix[3]  = matrix[7] = matrix[11] = matrix[12] = matrix[13] = matrix[14] = 0;
    matrix[15] = 1;
}

void f_matrix_to_quaternionf(float *quaternion, float *matrix)
{
	float trace, s;
	trace = matrix[0 * 4 + 0] + matrix[1 * 4 + 1] + matrix[2 * 4 + 2];
	if(trace > 0.0f) 
	{
		s = 0.5f / sqrt(trace + 1.0f);
		quaternion[3] = 0.25f / s;
		quaternion[0] = (matrix[2 * 4 + 1] - matrix[1 * 4 + 2]) * s;
		quaternion[1] = (matrix[0 * 4 + 2] - matrix[2 * 4 + 0]) * s;
		quaternion[2] = (matrix[1 * 4 + 0] - matrix[0 * 4 + 1]) * s;
	}else 
	{
		if(matrix[0 * 4 + 0] > matrix[1 * 4 + 1] && matrix[0 * 4 + 0] > matrix[2 * 4 + 2]) 
		{
			s = 2.0f * sqrt(1.0f + matrix[0 * 4 + 0] - matrix[1 * 4 + 1] - matrix[2 * 4 + 2]);
			quaternion[3] = (matrix[2 * 4 + 1] - matrix[1 * 4 + 2]) / s;
			quaternion[0] = 0.25f * s;
			quaternion[1] = (matrix[0 * 4 + 1] + matrix[1 * 4 + 0]) / s;
			quaternion[2] = (matrix[0 * 4 + 2] + matrix[2 * 4 + 0]) / s;
		}else if (matrix[1 * 4 + 1] > matrix[2 * 4 + 2]) 
		{
			s = 2.0f * sqrt(1.0f + matrix[1 * 4 + 1] - matrix[0 * 4 + 0] - matrix[2 * 4 + 2]);
			quaternion[3] = (matrix[0 * 4 + 2] - matrix[2 * 4 + 0]) / s;
			quaternion[0] = (matrix[0 * 4 + 1] + matrix[1 * 4 + 0]) / s;
			quaternion[1] = 0.25f * s;
			quaternion[2] = (matrix[1 * 4 + 2] + matrix[2 * 4 + 1]) / s;
		}else 
		{
			s = 2.0f * sqrt(1.0f + matrix[2 * 4 + 2] - matrix[0 * 4 + 0] - matrix[1 * 4 + 1]);
			quaternion[3] = (matrix[1 * 4 + 0] - matrix[0 * 4 + 1]) / s;
			quaternion[0] = (matrix[0 * 4 + 2] + matrix[2 * 4 + 0]) / s;
			quaternion[1] = (matrix[1 * 4 + 2] + matrix[2 * 4 + 1]) / s;
			quaternion[2] = 0.25f * s;
		}
	}
	s = sqrt(quaternion[0] * quaternion[0] + quaternion[1] * quaternion[1] + quaternion[2] * quaternion[2] + quaternion[3] * quaternion[3]);
	quaternion[0] /= s;
	quaternion[1] /= s;
	quaternion[2] /= s;
	quaternion[3] /= s;
}

extern float f_length3f(float *vec);

void f_matrix_to_pos_quaternion_scalef(float *matrix, float *pos, float *quaternion, float *scale)
{
	f_matrix_to_quaternionf(quaternion, matrix);
	pos[0] = matrix[12];
	pos[1] = matrix[13];
	pos[2] = matrix[14];
	scale[0] = f_length3f(matrix);
	scale[1] = f_length3f(&matrix[4]);
	scale[2] = f_length3f(&matrix[8]);
}


void f_pos_quaternion_scale_to_matrixf(float *pos, float *quaternion, float *scale, float *matrix)
{
	f_quaternion_to_matrixf(matrix, quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
	matrix[12] = pos[0];
	matrix[13] = pos[1];
	matrix[14] = pos[2];
	matrix[0] *= scale[0];
	matrix[1] *= scale[0];
	matrix[2] *= scale[0];
	matrix[4] *= scale[1];
	matrix[5] *= scale[1];
	matrix[6] *= scale[1];
	matrix[8] *= scale[2];
	matrix[9] *= scale[2];
	matrix[10] *= scale[2];
}

void f_pos_quaternion_scale_to_matrix_invf(float *pos, float *quaternion, float *scale, float *matrix)
{
	float x, y, z;
	f_quaternion_to_matrixf(matrix, -quaternion[0], -quaternion[1], -quaternion[2], -quaternion[3]);
	x = -pos[0] / scale[0];
	y = -pos[1] / scale[1];
	z = -pos[2] / scale[2];
	matrix[12] = (matrix[0] * x) + (matrix[4] * y) + (matrix[8] * z);
	matrix[13] = (matrix[1] * x) + (matrix[5] * y) + (matrix[9] * z);
	matrix[14] = (matrix[2] * x) + (matrix[6] * y) + (matrix[10] * z);
	matrix[0] /= scale[0];
	matrix[1] /= scale[0];
	matrix[2] /= scale[0];
	matrix[4] /= scale[1];
	matrix[5] /= scale[1];
	matrix[6] /= scale[1];
	matrix[8] /= scale[2];
	matrix[9] /= scale[2];
	matrix[10] /= scale[2];
}

void f_quaternion_to_matrixd(double *matrix, double x, double y, double z, double w)
{
	double xx, xy, xz, xw, yy, yz, yw, zz, zw; 
	xx = x * x;
    xy = x * y;
    xz = x * z;
    xw = x * w;
    yy = y * y;
    yz = y * z;
    yw = y * w;
    zz = z * z;
    zw = z * w;
    matrix[0]  = 1 - 2 * (yy + zz);
    matrix[1]  = 2 * (xy - zw);
    matrix[2]  = 2 * (xz + yw);
    matrix[4]  = 2 * (xy + zw);
    matrix[5]  = 1 - 2 * (xx + zz);
    matrix[6]  = 2 * (yz - xw);
    matrix[8]  = 2 * (xz - yw);
    matrix[9]  = 2 * (yz + xw);
    matrix[10] = 1 - 2 * (xx + yy);
    matrix[3]  = matrix[7] = matrix[11] = matrix[12] = matrix[13] = matrix[14] = 0;
    matrix[15] = 1;
}

void f_matrix_to_quaterniond(double *quaternion, double *matrix)
{
	double trace, s;
	trace = matrix[0 * 4 + 0] + matrix[1 * 4 + 1] + matrix[2 * 4 + 2];
	if(trace > 0.0) 
	{
		s = 0.f / sqrt(trace + 1.0);
		quaternion[3] = 0.25 / s;
		quaternion[0] = (matrix[2 * 4 + 1] - matrix[1 * 4 + 2]) * s;
		quaternion[1] = (matrix[0 * 4 + 2] - matrix[2 * 4 + 0]) * s;
		quaternion[2] = (matrix[1 * 4 + 0] - matrix[0 * 4 + 1]) * s;
	}else 
	{
		if(matrix[0 * 4 + 0] > matrix[1 * 4 + 1] && matrix[0 * 4 + 0] > matrix[2 * 4 + 2]) 
		{
			s = 2.0 * sqrt(1.0f + matrix[0 * 4 + 0] - matrix[1 * 4 + 1] - matrix[2 * 4 + 2]);
			quaternion[3] = (matrix[2 * 4 + 1] - matrix[1 * 4 + 2]) / s;
			quaternion[0] = 0.25 * s;
			quaternion[1] = (matrix[0 * 4 + 1] + matrix[1 * 4 + 0]) / s;
			quaternion[2] = (matrix[0 * 4 + 2] + matrix[2 * 4 + 0]) / s;
		}else if (matrix[1 * 4 + 1] > matrix[2 * 4 + 2]) 
		{
			s = 2.0 * sqrt(1.0f + matrix[1 * 4 + 1] - matrix[0 * 4 + 0] - matrix[2 * 4 + 2]);
			quaternion[3] = (matrix[0 * 4 + 2] - matrix[2 * 4 + 0]) / s;
			quaternion[0] = (matrix[0 * 4 + 1] + matrix[1 * 4 + 0]) / s;
			quaternion[1] = 0.25 * s;
			quaternion[2] = (matrix[1 * 4 + 2] + matrix[2 * 4 + 1]) / s;
		}else 
		{
			s = 2.0 * sqrt(1.0 + matrix[2 * 4 + 2] - matrix[0 * 4 + 0] - matrix[1 * 4 + 1]);
			quaternion[3] = (matrix[1 * 4 + 0] - matrix[0 * 4 + 1]) / s;
			quaternion[0] = (matrix[0 * 4 + 2] + matrix[2 * 4 + 0]) / s;
			quaternion[1] = (matrix[1 * 4 + 2] + matrix[2 * 4 + 1]) / s;
			quaternion[2] = 0.25 * s;
		}
	}
}

extern double f_length3d(double *vec);

void f_matrix_to_pos_quaternion_scaled(double *matrix, double *pos, double *quaternion, double *scale)
{
	f_matrix_to_quaterniond(quaternion, matrix);
	pos[0] = matrix[12];
	pos[1] = matrix[13];
	pos[2] = matrix[14];
	scale[0] = f_length3d(matrix);
	scale[1] = f_length3d(&matrix[4]);
	scale[2] = f_length3d(&matrix[8]);
}


void f_pos_quaternion_scale_to_matrixd(double *pos, double *quaternion, double *scale, double *matrix)
{
	f_quaternion_to_matrixd(matrix, quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
	matrix[12] = pos[0];
	matrix[13] = pos[1];
	matrix[14] = pos[2];
	matrix[0] *= scale[0];
	matrix[1] *= scale[0];
	matrix[2] *= scale[0];
	matrix[4] *= scale[1];
	matrix[5] *= scale[1];
	matrix[6] *= scale[1];
	matrix[8] *= scale[2];
	matrix[9] *= scale[2];
	matrix[10] *= scale[2];
}

void f_pos_quaternion_scale_to_matrix_invd(double *pos, double *quaternion, double *scale, double *matrix)
{
	double x, y, z;
	f_quaternion_to_matrixd(matrix, -quaternion[0], -quaternion[1], -quaternion[2], -quaternion[3]);
	x = -pos[0] / scale[0];
	y = -pos[1] / scale[1];
	z = -pos[2] / scale[2];
	matrix[12] = (matrix[0] * x) + (matrix[4] * y) + (matrix[8] * z);
	matrix[13] = (matrix[1] * x) + (matrix[5] * y) + (matrix[9] * z);
	matrix[14] = (matrix[2] * x) + (matrix[6] * y) + (matrix[10] * z);
	matrix[0] /= scale[0];
	matrix[1] /= scale[0];
	matrix[2] /= scale[0];
	matrix[4] /= scale[1];
	matrix[5] /= scale[1];
	matrix[6] /= scale[1];
	matrix[8] /= scale[2];
	matrix[9] /= scale[2];
	matrix[10] /= scale[2];
}


void f_matrix_rotatef(float *matrix, float angle, float x, float y, float z)
{
	float f, a, c, d;
	f = sqrt(x * x + y * y + z * z);
	x /= f;
	y /= f;
	z /= f;
	angle *= 3.141592653 * 2.0 / 360.0; 
	c = cos(angle);
	a = 1.0f - c;
	d = sin(angle);
	matrix[0] = x * x * a + c;
	matrix[1] = x * y * a + z * d;
	matrix[2] = x * z * a - y * d;
	matrix[3] = 0.0f;
	matrix[4] = y * x * a - z * d;
	matrix[5] = y * y * a + c;
	matrix[6] = y * z * a + x * d;
	matrix[7] = 0.0f;		
	matrix[8] = z * x * a + y * d;
	matrix[9] = z * y * a - x * d;
	matrix[10] = z * z * a + c;
	matrix[11] = 0.0f;
	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;
}

void f_matrix_rotated(double *matrix, double angle, double x, double y, double z)
{
	double f, a, c, d;
	f = sqrt(x * x + y * y + z * z);
	x /= f;
	y /= f;
	z /= f;
	angle *= 3.141592653 * 2.0 / 360.0; 
	c = cos(angle);
	a = 1.0f - c;
	d = sin(angle);
	matrix[0] = x * x * a + c;
	matrix[1] = x * y * a + z * d;
	matrix[2] = x * z * a - y * d;
	matrix[3] = 0.0;
	matrix[4] = y * x * a - z * d;
	matrix[5] = y * y * a + c;
	matrix[6] = y * z * a + x * d;
	matrix[7] = 0.0;		
	matrix[8] = z * x * a + y * d;
	matrix[9] = z * y * a - x * d;
	matrix[10] = z * z * a + c;
	matrix[11] = 0.0;
	matrix[12] = 0.0;
	matrix[13] = 0.0;
	matrix[14] = 0.0;
	matrix[15] = 1.0;
}

/* #define f_MATRIX_CODE_GEN */

#ifdef f_MATRIX_CODE_GEN

#include <stdio.h>




void f_gen_matrix_code(FILE *f, int a, int b, char *type, char *letter, int neg)
{
	char *axis[3] = {"x", "y", "z"};
	int c;
	c = 3 - (a + b);
/*	fprintf(f, "extern void f_matrix%s%s%s(%s *matrix, const %s *origo, const %s *point_a, const %s *point_b);\n", axis[a], axis[b], letter, type, type, type, type);*/

	fprintf(f, "void f_matrix%s%s%s(%s *matrix, const %s *origo, const %s *point_a, const %s *point_b)\n", axis[a], axis[b], letter, type, type, type, type);
	fprintf(f, "{\n");
	fprintf(f, "\t%s r, a[3], b[3];\n", type);
	fprintf(f, "\tif(origo != NULL)\n");
	fprintf(f, "\t{\n");
	fprintf(f, "\t\ta[0] = point_a[0] - origo[0];\n");
	fprintf(f, "\t\ta[1] = point_a[1] - origo[1];\n");
	fprintf(f, "\t\ta[2] = point_a[2] - origo[2];\n");
	fprintf(f, "\t\tmatrix[12] = origo[0];\n");
	fprintf(f, "\t\tmatrix[13] = origo[1];\n");
	fprintf(f, "\t\tmatrix[14] = origo[2];\n");
	fprintf(f, "\t\tb[0] = point_b[0] - origo[0];\n");
	fprintf(f, "\t\tb[1] = point_b[1] - origo[1];\n");
	fprintf(f, "\t\tb[2] = point_b[2] - origo[2];\n");
	fprintf(f, "\t}else\n");
	fprintf(f, "\t{\n");
	fprintf(f, "\t\ta[0] = point_a[0];\n");
	fprintf(f, "\t\ta[1] = point_a[1];\n");
	fprintf(f, "\t\ta[2] = point_a[2];\n");
	fprintf(f, "\t\tmatrix[12] = 0;\n");
	fprintf(f, "\t\tmatrix[13] = 0;\n");
	fprintf(f, "\t\tmatrix[14] = 0;\n");
	fprintf(f, "\t\tb[0] = point_b[0];\n");
	fprintf(f, "\t\tb[1] = point_b[1];\n");
	fprintf(f, "\t\tb[2] = point_b[2];\n");
	fprintf(f, "\t}\n");
	fprintf(f, "\tr = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);\n");
	fprintf(f, "\tmatrix[%u] = a[0] / r;\n", a * 4);
	fprintf(f, "\tmatrix[%u] = a[1] / r;\n", a * 4 + 1);
	fprintf(f, "\tmatrix[%u] = a[2] / r;\n", a * 4 + 2);
	if(neg)
	{
		fprintf(f, "\tmatrix[%u] = matrix[%u] * b[1] - matrix[%u] * b[2];\n", c * 4 + 0, a * 4 + 2, a * 4 + 1);
		fprintf(f, "\tmatrix[%u] = matrix[%u] * b[2] - matrix[%u] * b[0];\n", c * 4 + 1, a * 4 + 0, a * 4 + 2);
		fprintf(f, "\tmatrix[%u] = matrix[%u] * b[0] - matrix[%u] * b[1];\n", c * 4 + 2, a * 4 + 1, a * 4 + 0);
	}else
	{
		fprintf(f, "\tmatrix[%u] = matrix[%u] * b[2] - matrix[%u] * b[1];\n", c * 4 + 0, a * 4 + 1, a * 4 + 2);
		fprintf(f, "\tmatrix[%u] = matrix[%u] * b[0] - matrix[%u] * b[2];\n", c * 4 + 1, a * 4 + 2, a * 4 + 0);
		fprintf(f, "\tmatrix[%u] = matrix[%u] * b[1] - matrix[%u] * b[0];\n", c * 4 + 2, a * 4 + 0, a * 4 + 1);
	}
	fprintf(f, "\tr = sqrt(matrix[%u] * matrix[%u] + matrix[%u] * matrix[%u] + matrix[%u] * matrix[%u]);\n", c * 4 + 0, c * 4 + 0, c * 4 + 1, c * 4 + 1, c * 4 + 2, c * 4 + 2);
	fprintf(f, "\tmatrix[%u] = matrix[%u] / r;\n", c * 4 + 0, c * 4 + 0);
	fprintf(f, "\tmatrix[%u] = matrix[%u] / r;\n", c * 4 + 1, c * 4 + 1);
	fprintf(f, "\tmatrix[%u] = matrix[%u] / r;\n", c * 4 + 2, c * 4 + 2);
	if(neg)
	{
		fprintf(f, "\tmatrix[%u] = matrix[%u] * matrix[%u] - matrix[%u] * matrix[%u];\n", b * 4 + 0, a * 4 + 1, c * 4 + 2, a * 4 + 2, c * 4 + 1);
		fprintf(f, "\tmatrix[%u] = matrix[%u] * matrix[%u] - matrix[%u] * matrix[%u];\n", b * 4 + 1, a * 4 + 2, c * 4 + 0, a * 4 + 0, c * 4 + 2);
		fprintf(f, "\tmatrix[%u] = matrix[%u] * matrix[%u] - matrix[%u] * matrix[%u];\n", b * 4 + 2, a * 4 + 0, c * 4 + 1, a * 4 + 1, c * 4 + 0);
	}else
	{
		fprintf(f, "\tmatrix[%u] = matrix[%u] * matrix[%u] - matrix[%u] * matrix[%u];\n", b * 4 + 0, a * 4 + 2, c * 4 + 1, a * 4 + 1, c * 4 + 2);
		fprintf(f, "\tmatrix[%u] = matrix[%u] * matrix[%u] - matrix[%u] * matrix[%u];\n", b * 4 + 1, a * 4 + 0, c * 4 + 2, a * 4 + 2, c * 4 + 0);
		fprintf(f, "\tmatrix[%u] = matrix[%u] * matrix[%u] - matrix[%u] * matrix[%u];\n", b * 4 + 2, a * 4 + 1, c * 4 + 0, a * 4 + 0, c * 4 + 1);
	}
	fprintf(f, "\tr = sqrt(matrix[%u] * matrix[%u] + matrix[%u] * matrix[%u] + matrix[%u] * matrix[%u]);\n", b * 4 + 0, b * 4 + 0, b * 4 + 1, b * 4 + 1, b * 4 + 2, b * 4 + 2);
	fprintf(f, "\tmatrix[%u] = matrix[%u] / r;\n", b * 4 + 0, b * 4 + 0);
	fprintf(f, "\tmatrix[%u] = matrix[%u] / r;\n", b * 4 + 1, b * 4 + 1);
	fprintf(f, "\tmatrix[%u] = matrix[%u] / r;\n", b * 4 + 2, b * 4 + 2);
	fprintf(f, "\tmatrix[3] = 0;\n");
	fprintf(f, "\tmatrix[7] = 0;\n");
	fprintf(f, "\tmatrix[11] = 0;\n");
	fprintf(f, "\tmatrix[15] = 1;\n");
	fprintf(f, "}\n\n");
}



int main()
{
	FILE *f;
	int i, j, neg[9] = {-1, 0, 1, 1, -1, 0, 0, 1, -1};
	f = fopen("f_matrix_make.c", "w");
	fprintf(f, "#include <math.h>\n");
	fprintf(f, "#define NULL (void*)0\n\n");
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			if(i != j)
				f_gen_matrix_code(f, i, j, "float", "f", neg[i * 3 + j]);
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			if(i != j)
				f_gen_matrix_code(f, i, j, "double", "d", neg[i * 3 + j]);
	fclose(f);
}

#endif
