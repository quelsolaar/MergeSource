#include <math.h>

#ifndef NULL
#define NULL (void*)0
#endif

void f_matrixxyf(float *matrix, const float *origo, const float *point_a, const float *point_b)
{
	float r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[0] = a[0] / r;
	matrix[1] = a[1] / r;
	matrix[2] = a[2] / r;
	matrix[8] = matrix[1] * b[2] - matrix[2] * b[1];
	matrix[9] = matrix[2] * b[0] - matrix[0] * b[2];
	matrix[10] = matrix[0] * b[1] - matrix[1] * b[0];
	r = sqrt(matrix[8] * matrix[8] + matrix[9] * matrix[9] + matrix[10] * matrix[10]);
	matrix[8] = matrix[8] / r;
	matrix[9] = matrix[9] / r;
	matrix[10] = matrix[10] / r;
	matrix[4] = matrix[2] * matrix[9] - matrix[1] * matrix[10];
	matrix[5] = matrix[0] * matrix[10] - matrix[2] * matrix[8];
	matrix[6] = matrix[1] * matrix[8] - matrix[0] * matrix[9];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixxzf(float *matrix, const float *origo, const float *point_a, const float *point_b)
{
	float r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[0] = a[0] / r;
	matrix[1] = a[1] / r;
	matrix[2] = a[2] / r;
	matrix[4] = matrix[2] * b[1] - matrix[1] * b[2];
	matrix[5] = matrix[0] * b[2] - matrix[2] * b[0];
	matrix[6] = matrix[1] * b[0] - matrix[0] * b[1];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[8] = matrix[1] * matrix[6] - matrix[2] * matrix[5];
	matrix[9] = matrix[2] * matrix[4] - matrix[0] * matrix[6];
	matrix[10] = matrix[0] * matrix[5] - matrix[1] * matrix[4];
	r = sqrt(matrix[8] * matrix[8] + matrix[9] * matrix[9] + matrix[10] * matrix[10]);
	matrix[8] = matrix[8] / r;
	matrix[9] = matrix[9] / r;
	matrix[10] = matrix[10] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixyxf(float *matrix, const float *origo, const float *point_a, const float *point_b)
{
	float r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[4] = a[0] / r;
	matrix[5] = a[1] / r;
	matrix[6] = a[2] / r;
	matrix[8] = matrix[6] * b[1] - matrix[5] * b[2];
	matrix[9] = matrix[4] * b[2] - matrix[6] * b[0];
	matrix[10] = matrix[5] * b[0] - matrix[4] * b[1];
	r = sqrt(matrix[8] * matrix[8] + matrix[9] * matrix[9] + matrix[10] * matrix[10]);
	matrix[8] = matrix[8] / r;
	matrix[9] = matrix[9] / r;
	matrix[10] = matrix[10] / r;
	matrix[0] = matrix[5] * matrix[10] - matrix[6] * matrix[9];
	matrix[1] = matrix[6] * matrix[8] - matrix[4] * matrix[10];
	matrix[2] = matrix[4] * matrix[9] - matrix[5] * matrix[8];
	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixyzf(float *matrix, const float *origo, const float *point_a, const float *point_b)
{
	float r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[4] = a[0] / r;
	matrix[5] = a[1] / r;
	matrix[6] = a[2] / r;
	matrix[0] = matrix[5] * b[2] - matrix[6] * b[1];
	matrix[1] = matrix[6] * b[0] - matrix[4] * b[2];
	matrix[2] = matrix[4] * b[1] - matrix[5] * b[0];
	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
	matrix[8] = matrix[6] * matrix[1] - matrix[5] * matrix[2];
	matrix[9] = matrix[4] * matrix[2] - matrix[6] * matrix[0];
	matrix[10] = matrix[5] * matrix[0] - matrix[4] * matrix[1];
	r = sqrt(matrix[8] * matrix[8] + matrix[9] * matrix[9] + matrix[10] * matrix[10]);
	matrix[8] = matrix[8] / r;
	matrix[9] = matrix[9] / r;
	matrix[10] = matrix[10] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixzxf(float *matrix, const float *origo, const float *point_a, const float *point_b)
{
	float r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[8] = a[0] / r;
	matrix[9] = a[1] / r;
	matrix[10] = a[2] / r;
	matrix[4] = matrix[9] * b[2] - matrix[10] * b[1];
	matrix[5] = matrix[10] * b[0] - matrix[8] * b[2];
	matrix[6] = matrix[8] * b[1] - matrix[9] * b[0];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[0] = matrix[10] * matrix[5] - matrix[9] * matrix[6];
	matrix[1] = matrix[8] * matrix[6] - matrix[10] * matrix[4];
	matrix[2] = matrix[9] * matrix[4] - matrix[8] * matrix[5];
	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixzyf(float *matrix, const float *origo, const float *point_a, const float *point_b)
{
	float r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[8] = a[0] / r;
	matrix[9] = a[1] / r;
	matrix[10] = a[2] / r;
	matrix[0] = matrix[10] * b[1] - matrix[9] * b[2];
	matrix[1] = matrix[8] * b[2] - matrix[10] * b[0];
	matrix[2] = matrix[9] * b[0] - matrix[8] * b[1];
	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
	matrix[4] = matrix[9] * matrix[2] - matrix[10] * matrix[1];
	matrix[5] = matrix[10] * matrix[0] - matrix[8] * matrix[2];
	matrix[6] = matrix[8] * matrix[1] - matrix[9] * matrix[0];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixxyd(double *matrix, const double *origo, const double *point_a, const double *point_b)
{
	double r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[0] = a[0] / r;
	matrix[1] = a[1] / r;
	matrix[2] = a[2] / r;
	matrix[8] = matrix[1] * b[2] - matrix[2] * b[1];
	matrix[9] = matrix[2] * b[0] - matrix[0] * b[2];
	matrix[10] = matrix[0] * b[1] - matrix[1] * b[0];
	r = sqrt(matrix[8] * matrix[8] + matrix[9] * matrix[9] + matrix[10] * matrix[10]);
	matrix[8] = matrix[8] / r;
	matrix[9] = matrix[9] / r;
	matrix[10] = matrix[10] / r;
	matrix[4] = matrix[2] * matrix[9] - matrix[1] * matrix[10];
	matrix[5] = matrix[0] * matrix[10] - matrix[2] * matrix[8];
	matrix[6] = matrix[1] * matrix[8] - matrix[0] * matrix[9];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixxzd(double *matrix, const double *origo, const double *point_a, const double *point_b)
{
	double r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[0] = a[0] / r;
	matrix[1] = a[1] / r;
	matrix[2] = a[2] / r;
	matrix[4] = matrix[2] * b[1] - matrix[1] * b[2];
	matrix[5] = matrix[0] * b[2] - matrix[2] * b[0];
	matrix[6] = matrix[1] * b[0] - matrix[0] * b[1];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[8] = matrix[1] * matrix[6] - matrix[2] * matrix[5];
	matrix[9] = matrix[2] * matrix[4] - matrix[0] * matrix[6];
	matrix[10] = matrix[0] * matrix[5] - matrix[1] * matrix[4];
	r = sqrt(matrix[8] * matrix[8] + matrix[9] * matrix[9] + matrix[10] * matrix[10]);
	matrix[8] = matrix[8] / r;
	matrix[9] = matrix[9] / r;
	matrix[10] = matrix[10] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixyxd(double *matrix, const double *origo, const double *point_a, const double *point_b)
{
	double r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[4] = a[0] / r;
	matrix[5] = a[1] / r;
	matrix[6] = a[2] / r;
	matrix[8] = matrix[6] * b[1] - matrix[5] * b[2];
	matrix[9] = matrix[4] * b[2] - matrix[6] * b[0];
	matrix[10] = matrix[5] * b[0] - matrix[4] * b[1];
	r = sqrt(matrix[8] * matrix[8] + matrix[9] * matrix[9] + matrix[10] * matrix[10]);
	matrix[8] = matrix[8] / r;
	matrix[9] = matrix[9] / r;
	matrix[10] = matrix[10] / r;
	matrix[0] = matrix[5] * matrix[10] - matrix[6] * matrix[9];
	matrix[1] = matrix[6] * matrix[8] - matrix[4] * matrix[10];
	matrix[2] = matrix[4] * matrix[9] - matrix[5] * matrix[8];
	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixyzd(double *matrix, const double *origo, const double *point_a, const double *point_b)
{
	double r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[4] = a[0] / r;
	matrix[5] = a[1] / r;
	matrix[6] = a[2] / r;
	matrix[0] = matrix[5] * b[2] - matrix[6] * b[1];
	matrix[1] = matrix[6] * b[0] - matrix[4] * b[2];
	matrix[2] = matrix[4] * b[1] - matrix[5] * b[0];
	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
	matrix[8] = matrix[6] * matrix[1] - matrix[5] * matrix[2];
	matrix[9] = matrix[4] * matrix[2] - matrix[6] * matrix[0];
	matrix[10] = matrix[5] * matrix[0] - matrix[4] * matrix[1];
	r = sqrt(matrix[8] * matrix[8] + matrix[9] * matrix[9] + matrix[10] * matrix[10]);
	matrix[8] = matrix[8] / r;
	matrix[9] = matrix[9] / r;
	matrix[10] = matrix[10] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixzxd(double *matrix, const double *origo, const double *point_a, const double *point_b)
{
	double r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[8] = a[0] / r;
	matrix[9] = a[1] / r;
	matrix[10] = a[2] / r;
	matrix[4] = matrix[9] * b[2] - matrix[10] * b[1];
	matrix[5] = matrix[10] * b[0] - matrix[8] * b[2];
	matrix[6] = matrix[8] * b[1] - matrix[9] * b[0];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[0] = matrix[10] * matrix[5] - matrix[9] * matrix[6];
	matrix[1] = matrix[8] * matrix[6] - matrix[10] * matrix[4];
	matrix[2] = matrix[9] * matrix[4] - matrix[8] * matrix[5];
	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

void f_matrixzyd(double *matrix, const double *origo, const double *point_a, const double *point_b)
{
	double r, a[3], b[3];
	if(origo != NULL)
	{
		a[0] = point_a[0] - origo[0];
		a[1] = point_a[1] - origo[1];
		a[2] = point_a[2] - origo[2];
		matrix[12] = origo[0];
		matrix[13] = origo[1];
		matrix[14] = origo[2];
		b[0] = point_b[0] - origo[0];
		b[1] = point_b[1] - origo[1];
		b[2] = point_b[2] - origo[2];
	}else
	{
		a[0] = point_a[0];
		a[1] = point_a[1];
		a[2] = point_a[2];
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		b[0] = point_b[0];
		b[1] = point_b[1];
		b[2] = point_b[2];
	}
	r = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	matrix[8] = a[0] / r;
	matrix[9] = a[1] / r;
	matrix[10] = a[2] / r;
	matrix[0] = matrix[10] * b[1] - matrix[9] * b[2];
	matrix[1] = matrix[8] * b[2] - matrix[10] * b[0];
	matrix[2] = matrix[9] * b[0] - matrix[8] * b[1];
	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
	matrix[4] = matrix[9] * matrix[2] - matrix[10] * matrix[1];
	matrix[5] = matrix[10] * matrix[0] - matrix[8] * matrix[2];
	matrix[6] = matrix[8] * matrix[1] - matrix[9] * matrix[0];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[3] = 0;
	matrix[7] = 0;
	matrix[11] = 0;
	matrix[15] = 1;
}

