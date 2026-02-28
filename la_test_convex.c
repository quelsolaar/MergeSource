#include "forge.h"
//#include "seduce.h"

boolean la_inside_tri_check(double *vertex, uint ref_0, uint ref_1, uint ref_2, double *pos)
{
	double v[9], n[3], f;
	v[0] = vertex[ref_0 * 3];
	v[1] = vertex[ref_0 * 3 + 1];

	v[3] = vertex[ref_1 * 3]; // 2
	v[4] = vertex[ref_1 * 3 + 1]; // 3

	v[6] = vertex[ref_2 * 3]; // 4
	v[7] = vertex[ref_2 * 3 + 1]; // 5

	if(0 > (pos[0] - v[0]) * (v[4] - v[1]) - (pos[1] - v[1]) * (v[3] - v[0]))
	{
		if(0 < (pos[0] - v[3]) * (v[7] - v[4]) - (pos[1] - v[4]) * (v[6] - v[3]))
			return FALSE;

		if(0 < (pos[0] - v[6]) * (v[1] - v[7]) - (pos[1] - v[7]) * (v[0] - v[6]))
			return FALSE;
	}else
	{
		if(0 > (pos[0] - v[3]) * (v[7] - v[4]) - (pos[1] - v[4]) * (v[6] - v[3]))
			return FALSE;

		if(0 > (pos[0] - v[6]) * (v[1] - v[7]) - (pos[1] - v[7]) * (v[0] - v[6]))
			return FALSE;
	}
	v[2] = vertex[ref_0 * 3 + 2];
	v[5] = vertex[ref_1 * 3 + 2];
	v[8] = vertex[ref_2 * 3 + 2];

	n[0] = (v[4] - v[1]) * (v[8] - v[2]) - (v[5] - v[2]) * (v[7] - v[1]);
	n[1] = (v[5] - v[2]) * (v[6] - v[0]) - (v[3] - v[0]) * (v[8] - v[2]);
	n[2] = (v[3] - v[0]) * (v[7] - v[1]) - (v[4] - v[1]) * (v[6] - v[0]);

	f = (pos[0] - v[0]) * n[0] + 
			(pos[1] - v[1]) * n[1] +
			(pos[2] - v[2]) * n[2];

	n[2] *= f;
	if(n[2] > 0.0)
		return FALSE;



	return TRUE;
}

boolean la_inside_check(double *vertex, uint *ref, uint ref_length, uint vertex_count, double *pos)
{
	uint i, count = 0, test = 0;
	for(i = 0; i < ref_length; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count)
		{
			if(la_inside_tri_check(vertex, ref[i], ref[i + 1], ref[i + 2], pos))
				count++;
			if(ref[i + 3] < vertex_count &&
				la_inside_tri_check(vertex, ref[i], ref[i + 2], ref[i + 3], pos))
				count++;

		}
	}
	return count % 2 == 1;
}

#define LA_GAP_SIZE 0.1
/*
void la_inside_test(double *vertex, uint *ref, uint ref_length, uint vertex_count)
{
	double x, y, z, pos[3];
	uint i, count;

	for(x = -1; x < 1.1; x += LA_GAP_SIZE)
	{
		for(y = -1; y < 1.1; y += LA_GAP_SIZE)
		{
			for(z = -1; z < 1.1; z += LA_GAP_SIZE)
			{
				pos[0] = x;
				pos[1] = y;
				pos[2] = z;

				if(la_inside_check(vertex, ref, ref_length, vertex_count, pos))
				{
					r_3d_line_gl(x + LA_GAP_SIZE * 0.3, y, z, x - LA_GAP_SIZE * 0.3, y, z, count % 2, (count / 2) % 2, (count / 4) % 2, 0);
					r_3d_line_gl(x, y + LA_GAP_SIZE * 0.3, z, x, y - LA_GAP_SIZE * 0.3, z, count % 2, (count / 2) % 2, (count / 4) % 2, 0);
					r_3d_line_gl(x, y, z + LA_GAP_SIZE * 0.3, x, y, z - LA_GAP_SIZE * 0.3, count % 2, (count / 2) % 2, (count / 4) % 2, 0);
				}else
				{
					r_3d_line_gl(x + LA_GAP_SIZE * 0.1, y, z, x - LA_GAP_SIZE * 0.1, y, z, 0.1, 0.1, 0.1, 0);
					r_3d_line_gl(x, y + LA_GAP_SIZE * 0.1, z, x, y - LA_GAP_SIZE * 0.1, z, 0.1, 0.1, 0.1, 0);
					r_3d_line_gl(x, y, z + LA_GAP_SIZE * 0.1, x, y, z - LA_GAP_SIZE * 0.1, 0.1, 0.1, 0.1, 0);
				}
			}
		}
	}
	prinrtf("uint ref[%u] = {%u", ref_length, ref[0]);
	for(i = 1; i < ref_length; i++)
		prinrtf(", %u", ref[i]);
	prinrtf("};\n");

	prinrtf("float vertex[%u] = {%f", vertex_count * 3, ref[0]);
	for(i = 1; i < vertex_count * 3; i++)
		prinrtf(", %f", vertex[i]);
	prinrtf("};\nvertex count = %u", vertex_count);
}*/