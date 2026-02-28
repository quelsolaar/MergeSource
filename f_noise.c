
#include "forge.h"

float f_smooth_stepf(float f)
{
	if(f < 0)
		return 0;
	if(f < 1.0)	
		return (3.0 * f * f - 2.0 * f * f * f);
	return 1.0;
}

double f_smooth_stepd(double f)
{
	if(f < 0)
		return 0;
	if(f < 1.0)	
		return (3.0 * f * f - 2.0 * f * f * f);
	return 1.0;
}

float f_noisef(float f)
{
	float a, b;
	f += 4096.0;
	a = f_randnf((int)f);
	b = f_randnf((int)f + 1);
	f -= (int)f;
	f = (3.0 * f * f - 2.0 * f * f * f);
	return a + (b - a) * f;
}

float f_noise2f(float x, float y)
{
	float a, b, c, d;
	int xi, yi;
	x += 256.0;
	xi = (int)x;
	x -= (float)xi;
	y += 4096.0;
	yi = (int)y;
	y -= (float)yi;
	yi *= 11;
	x = (3.0 * x * x - 2.0 * x * x * x);
	y = (3.0 * y * y - 2.0 * y * y * y);

	a = f_randnf((uint)xi + yi);
	b = f_randnf((uint)xi + yi + 1);
	c = a + (b - a) * x;

	a = f_randnf((uint)xi + yi + 11);
	b = f_randnf((uint)xi + yi + 12);
	d = a + (b - a) * x;

	return c + (d - c) * y;
}

float f_noise3f(float x, float y, float z)
{
	float a, b, c, d, e;
	int xi, yi, zi;
	x += 256.0;
	xi = (int)x;
	x -= (float)xi;
	y += 4096.0;
	yi = (int)y;
	y -= (float)yi;
	yi *= 11;
	z += 8192.0;
	zi = (int)z;
	z -= (float)zi;
	zi *= 17;
	x = (3.0 * x * x - 2.0 * x * x * x);
	y = (3.0 * y * y - 2.0 * y * y * y);
	z = (3.0 * z * z - 2.0 * z * z * z);

	a = f_randnf((uint)xi + yi + zi);
	b = f_randnf((uint)xi + yi + zi + 1);
	c = a + (b - a) * x;

	a = f_randnf((uint)xi + yi + zi + 11);
	b = f_randnf((uint)xi + yi + zi + 12);
	d = a + (b - a) * x;

	e = c + (d - c) * y;

	a = f_randnf((uint)xi + yi + zi + 17);
	b = f_randnf((uint)xi + yi + zi + 18);
	c = a + (b - a) * x;

	a = f_randnf((uint)xi + yi + zi + 28);
	b = f_randnf((uint)xi + yi + zi + 29);
	d = a + (b - a) * x;

	a = c + (d - c) * y;

	return e + (a - e) * z;
}



float f_noiserf(float f, uint recursions)
{
	float out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noisef(f / size) * size;
		size *= 0.5;
	}
	return out;
}

float f_noiser2f(float x, float y, uint recursions)
{
	float out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noise2f(x / size, y / size) * size;
		size *= 0.5;
	}
	return out;
}

float f_noiser3f(float x, float y, float z, uint recursions)
{
	float out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noise3f(x / size, y / size, z / size) * size;
		size *= 0.5;
	}
	return out;
}

double f_noised(double f)
{
	double a, b;
	f += 4096.0;
	a = f_randnd((int)f);
	b = f_randnd((int)f + 1);
	f -= (int)f;
	f = (3.0 * f * f - 2.0 * f * f * f);
	return a + (b - a) * f;
}

double f_noise2d(double x, double y)
{
	double a, b, c, d;
	int xi, yi;
	x += 256.0;
	xi = (int)x;
	x -= (double)xi;
	y += 4096.0;
	yi = (int)y;
	y -= (double)yi;
	yi *= 11;
	x = (3.0 * x * x - 2.0 * x * x * x);
	y = (3.0 * y * y - 2.0 * y * y * y);

	a = f_randnd((uint)xi + yi);
	b = f_randnd((uint)xi + yi + 1);
	c = a + (b - a) * x;

	a = f_randnd((uint)xi + yi + 11);
	b = f_randnd((uint)xi + yi + 12);
	d = a + (b - a) * x;

	return c + (d - c) * y;
}

double f_noise3d(double x, double y, double z)
{
	double a, b, c, d, e;
	int xi, yi, zi;
	x += 256.0;
	xi = (int)x;
	x -= (double)xi;
	y += 4096.0;
	yi = (int)y;
	y -= (double)yi;
	yi *= 11;
	z += 8192.0;
	zi = (int)z;
	z -= (double)zi;
	zi *= 17;
	x = (3.0 * x * x - 2.0 * x * x * x);
	y = (3.0 * y * y - 2.0 * y * y * y);
	z = (3.0 * z * z - 2.0 * z * z * z);

	a = f_randnd((uint)xi + yi + zi);
	b = f_randnd((uint)xi + yi + zi + 1);
	c = a + (b - a) * x;

	a = f_randnd((uint)xi + yi + zi + 11);
	b = f_randnd((uint)xi + yi + zi + 12);
	d = a + (b - a) * x;

	e = c + (d - c) * y;

	a = f_randnd((uint)xi + yi + zi + 17);
	b = f_randnd((uint)xi + yi + zi + 18);
	c = a + (b - a) * x;

	a = f_randnd((uint)xi + yi + zi + 28);
	b = f_randnd((uint)xi + yi + zi + 29);
	d = a + (b - a) * x;

	a = c + (d - c) * y;

	return e + (a - e) * z;
}



double f_noiserd(double f, uint recursions)
{
	double out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noisef(f / size) * size;
		size *= 0.5;
	}
	return out;
}

double f_noiser2d(double x, double y, uint recursions)
{
	double out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noise2f(x / size, y / size) * size;
		size *= 0.5;
	}
	return out;
}

double f_noiser3d(double x, double y, double z, uint recursions)
{
	double out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noise3f(x / size, y / size, z / size) * size;
		size *= 0.5;
	}
	return out;
}

float f_snapf(float f, float step_size)
{
	if(f > 0)
		return (float)((int)(f / step_size + 0.5)) * step_size;
	else
		return (float)((int)(f / step_size - 0.5)) * step_size;
}

double f_snapd(double f, double step_size)
{
	if(f > 0)
		return (double)((int)(f / step_size + 0.5)) * step_size;
	else
		return (double)((int)(f / step_size - 0.5)) * step_size;
}


/* tiled versions */


float f_noisetf(float f, int period)
{
	float a, b;
	f += 4096.0;
	a = f_randnf(((int)f + 3242343) % period);
	b = f_randnf(((int)f + 3242344) % period);
	f -= (int)f;
	f = (3.0 * f * f - 2.0 * f * f * f);
	return a + (b - a) * f;
}

float f_noiset2f(float x, float y, int period)
{
	float a, b, c, d;
	int xi, yi, xin, yin;
	x += 256.0;
	xi = (int)x;
	x -= (float)xi;
	y += 4096.0;
	yi = (int)y;
	y -= (float)yi;
	x = (3.0 * x * x - 2.0 * x * x * x);
	y = (3.0 * y * y - 2.0 * y * y * y);
	xi = xi % period;
	xin = (xi + 1) % period;
	yin = ((yi + 1) % period) * 11;
	yi = (yi % period) * 11;
	a = f_randnf(xi + yi);
	b = f_randnf(xin + yi);
	c = a + (b - a) * x;
	a = f_randnf(xi + yin);
	b = f_randnf(xin + yin);
	d = a + (b - a) * x;

	return c + (d - c) * y;
}

float f_noiset3f(float x, float y, float z, int period)
{
	float a, b, c, d, e;
	int xi, yi, zi, xin, yin, zin;
	x += 256.0;
	xi = (int)x;
	x -= (float)xi;
	y += 4096.0;
	yi = (int)y;
	y -= (float)yi;
	yi *= 11;
	z += 8192.0;
	zi = (int)z;
	z -= (float)zi;
	zi *= 17;
	x = (3.0 * x * x - 2.0 * x * x * x);
	y = (3.0 * y * y - 2.0 * y * y * y);
	z = (3.0 * z * z - 2.0 * z * z * z);

	xi = xi % period;
	xin = (xi + 1) % period;
	yin = ((yi + 1) % period) * 11;
	yi = (yi % period) * 11;
	zin = ((zi + 1) % period) * 17;
	zi = (zi % period) * 17;

	a = f_randnf(xi + yi + zi);
	b = f_randnf(xin + yi + zi);
	c = a + (b - a) * x;

	a = f_randnf(xi + yin + zi);
	b = f_randnf(xin + yin + zi);
	d = a + (b - a) * x;

	e = c + (d - c) * y;

	a = f_randnf(xi + yi + zin);
	b = f_randnf(xin + yi + zin);
	c = a + (b - a) * x;

	a = f_randnf(xi + yin + zin);
	b = f_randnf(xin + yin + zin);
	d = a + (b - a) * x;

	a = c + (d - c) * y;

	return e + (a - e) * z;
}



float f_noisertf(float f, uint recursions, int period)
{
	float out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noisetf(f / size, period) * size;
		size *= 0.5;
		period *= 2;
	}
	return out;
}

float f_noisert2f(float x, float y, uint recursions, int period)
{
	float out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noiset2f(x / size, y / size, period) * size;
		size *= 0.5;
		period *= 2;
	}
	return out;
}

float f_noisert3f(float x, float y, float z, uint recursions, int period)
{
	float out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noiset3f(x / size, y / size, z / size, period) * size;
		size *= 0.5;
		period *= 2;
	}
	return out;
}

/*tiled for doubles */



double f_noisetd(double f, int period)
{
	double a, b;
	f += 4096.0;
	a = f_randnd(((int)f + 3242343) % period);
	b = f_randnd(((int)f + 3242344) % period);
	f -= (int)f;
	f = (3.0 * f * f - 2.0 * f * f * f);
	return a + (b - a) * f;
}

double f_noiset2d(double x, double y, int period)
{
	double a, b, c, d;
	int xi, yi, xin, yin;
	x += 256.0;
	xi = (int)x;
	x -= (double)xi;
	y += 4096.0;
	yi = (int)y;
	y -= (double)yi;
	x = (3.0 * x * x - 2.0 * x * x * x);
	y = (3.0 * y * y - 2.0 * y * y * y);
	xi = xi % period;
	xin = (xi + 1) % period;
	yin = ((yi + 1) % period) * 11;
	yi = (yi % period) * 11;
	a = f_randnd(xi + yi);
	b = f_randnd(xin + yi);
	c = a + (b - a) * x;
	a = f_randnd(xi + yin);
	b = f_randnd(xin + yin);
	d = a + (b - a) * x;

	return c + (d - c) * y;
}

double f_noiset3d(double x, double y, double z, int period)
{
	double a, b, c, d, e;
	int xi, yi, zi, xin, yin, zin;
	x += 256.0;
	xi = (int)x;
	x -= (double)xi;
	y += 4096.0;
	yi = (int)y;
	y -= (double)yi;
	yi *= 11;
	z += 8192.0;
	zi = (int)z;
	z -= (double)zi;
	zi *= 17;
	x = (3.0 * x * x - 2.0 * x * x * x);
	y = (3.0 * y * y - 2.0 * y * y * y);
	z = (3.0 * z * z - 2.0 * z * z * z);

	xi = xi % period;
	xin = (xi + 1) % period;
	yin = ((yi + 1) % period) * 11;
	yi = (yi % period) * 11;
	zin = ((zi + 1) % period) * 17;
	zi = (zi % period) * 17;

	a = f_randnd(xi + yi + zi);
	b = f_randnd(xin + yi + zi);
	c = a + (b - a) * x;

	a = f_randnd(xi + yin + zi);
	b = f_randnd(xin + yin + zi);
	d = a + (b - a) * x;

	e = c + (d - c) * y;

	a = f_randnd(xi + yi + zin);
	b = f_randnd(xin + yi + zin);
	c = a + (b - a) * x;

	a = f_randnd(xi + yin + zin);
	b = f_randnd(xin + yin + zin);
	d = a + (b - a) * x;

	a = c + (d - c) * y;

	return e + (a - e) * z;
}



double f_noisertd(double f, uint recursions, int period)
{
	double out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noisetd(f / size, period) * size;
		size *= 0.5;
		period *= 2;
	}
	return out;
}

double f_noisert2d(double x, double y, uint recursions, int period)
{
	double out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noiset2d(x / size, y / size, period) * size;
		size *= 0.5;
		period *= 2;
	}
	return out;
}

double f_noisert3d(double x, double y, double z, uint recursions, int period)
{
	double out = 0.0, size = 1.0;
	uint i;
	for(i = 0; i < recursions; i++)
	{
		out += f_noiset3d(x / size, y / size, z / size, period) * size;
		size *= 0.5;
		period *= 2;
	}
	return out;
}
