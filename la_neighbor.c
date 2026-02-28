#include "la_includes.h"
#include "la_geometry_undo.h"

#include <math.h>


uint *la_compute_neighbor(uint *ref, uint ref_count, uint vertex_count, egreal *vertex)
{
	uint i, j, cor, clear = 0, *n, *v, a, b;
	uint laps = 0, tq1, tq2;
	n = malloc((sizeof *n) * ref_count * 4);
	for(i = 0; i < ref_count * 4; i++)
		n[i] = -1;
	v = malloc((sizeof *v) * vertex_count);
	for(i = 0; i < vertex_count; i++)
		v[i] = -1;

	while(clear < ref_count * 4)
	{
		for(i = 0; i < ref_count * 4;)
		{
			if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
			{
				if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != V_REAL64_MAX)
					tq1 = 4;
				else
					tq1 = 3;
	
				for(j = 0; j < tq1; j++)
				{
					cor = v[ref[i]];
					if(cor == -1)
					{
						if(n[i] == -1 || n[(i / 4) * 4 + (i + tq1 - 1) % tq1] == -1)
							v[ref[i]] = i;
					}
					else if(cor == i)
					{
						v[ref[i]] = -1;
					}
					else
					{
						if(ref[((cor / 4) * 4) + 3] < vertex_count && vertex[ref[((cor / 4) * 4) + 3] * 3] != V_REAL64_MAX)
							tq2 = 4;
						else
							tq2 = 3;
						a = (i / 4) * 4;
						b = (cor / 4) * 4;
						if((n[cor] == -1 || n[a + (i + tq1 - a - 1) % tq1] == -1) && ref[a + (i + tq1 - a - 1) % tq1] == ref[b + (cor + 1 - b) % tq2])
						{
							n[a + (i + tq1 - a - 1) % tq1] = cor;
							n[cor] = a + (i + tq1 - a - 1) % tq1;
							clear = 0;
							if(n[b + (cor + tq2 - 1) % tq2] != -1)
							{
								if(n[i] == -1)
									v[ref[i]] = i;
								else
									v[ref[i]] = -1;
							}
						}
						if((n[i] == -1 || n[b + (cor + tq2 - b - 1) % tq2] == -1) && ref[a + (i - a + 1) % tq1] == ref[b + (cor + tq2 - b - 1) % tq2])
						{
							n[i] = b + (cor + tq2 - b - 1) % tq2;
							n[b + (cor + tq2 - b - 1) % tq2] = i;
							clear = 0;	
							if(n[cor] != -1)
							{
								if(n[a + (i + tq1 - a - 1) % tq1] == -1)
									v[ref[i]] = i;
								else
									v[ref[i]] = -1;
							}
						}
					}
					i++;
				}
				i = i + 4 - tq1;
			}else
				i += 4;
			clear++;
		}
	}
	free(v);
	return n; 
}
	
void la_t_poly_triangulate(void)
{
	uint i, id, side;
	uint32 vertex_count, ref_count, *ref, *crease;
	double *vertex, v[3], d, d2, r, r2;

	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);

	for(i = 0; i < ref_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
		{
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != V_REAL64_MAX)
			{
				if(udg_get_select(ref[i]) > 0.1 && udg_get_select(ref[i + 1]) > 0.1 && udg_get_select(ref[i + 2]) > 0.1 && udg_get_select(ref[i + 3]) > 0.1)
				{
					id = udg_find_empty_slot_polygon();
					v[0] = vertex[ref[i] * 3 + 0] - vertex[ref[i + 2] * 3 + 0];
					v[1] = vertex[ref[i] * 3 + 1] - vertex[ref[i + 2] * 3 + 1];
					v[2] = vertex[ref[i] * 3 + 2] - vertex[ref[i + 2] * 3 + 2];
					r = v[0] + v[1] + v[2];
					if(r < 0)
						r = -r;
					d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
					v[0] = vertex[ref[i + 1] * 3 + 0] - vertex[ref[i + 3] * 3 + 0];
					v[1] = vertex[ref[i + 1] * 3 + 1] - vertex[ref[i + 3] * 3 + 1];
					v[2] = vertex[ref[i + 1] * 3 + 2] - vertex[ref[i + 3] * 3 + 2];
					d2 = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
					r2 = v[0] + v[1] + v[2];
					if(r2 < 0)
						r2 = -r2;
					if(d > d2 * 1.1)
						side = 1;
					else if(d2 > d * 1.1)
						side = 0;
					else if(r > r2)
						side = 0;
					else
						side = 1;
					if(side == 0)
					{
						udg_polygon_set(i / 4, ref[i], ref[i + 1], ref[i + 2], -1);
						udg_crease_set(i / 4, crease[i], crease[i + 1], 0, 0);
						udg_polygon_set(id, ref[i + 2], ref[i + 3], ref[i], -1);
						udg_crease_set(id, crease[i + 2], crease[i + 3], 0, 0);
					}else
					{
						udg_polygon_set(i / 4, ref[i], ref[i + 1], ref[i + 3], -1);
						udg_crease_set(i / 4, crease[i], 0, crease[i + 3], 0);
						udg_polygon_set(id, ref[i + 1], ref[i + 2], ref[i + 3], -1);
						udg_crease_set(id, crease[i + 1], crease[i + 2], 0, 0);
					}
				}
			}
		}
	}
}


double la_t_poly_quad_test(double *a, double *b, double *c, double *d)
{
	double vec0[3], vec1[3], tmp;	
	f_normal3d(vec0, a, b, c); 
	f_normal3d(vec1, c, d, a); 
	if(vec0[0] * vec1[0] + vec0[1] * vec1[1] + vec0[2] * vec1[2] < 0.85)
		return -1.0;

	vec0[0] = b[0] - a[0];
	vec0[1] = b[1] - a[1];
	vec0[2] = b[2] - a[2];
	vec1[0] = c[0] - d[0];
	vec1[1] = c[1] - d[1];
	vec1[2] = c[2] - d[2];
	f_normalize3d(vec0);
	f_normalize3d(vec1);
	if(vec0[0] * vec1[0] + vec0[1] * vec1[1] + vec0[2] * vec1[2] < 0.8)
	{
		vec0[0] = d[0] - a[0];
		vec0[1] = d[1] - a[1];
		vec0[2] = d[2] - a[2];
		vec1[0] = c[0] - b[0];
		vec1[1] = c[1] - b[1];
		vec1[2] = c[2] - b[2];
		f_normalize3d(vec0);
		f_normalize3d(vec1);
		if(vec0[0] * vec1[0] + vec0[1] * vec1[1] + vec0[2] * vec1[2] < 0.8)
			return -1.0;
	}
	vec0[0] = b[0] - a[0] + c[0] - d[0];
	vec0[1] = b[1] - a[1] + c[1] - d[1];
	vec0[2] = b[2] - a[2] + c[2] - d[2];
	vec1[0] = b[0] - c[0] + a[0] - d[0];
	vec1[1] = b[1] - c[1] + a[1] - d[1];
	vec1[2] = b[2] - c[2] + a[2] - d[2];
	f_normalize3d(vec0);
	f_normalize3d(vec1);
	tmp = vec0[0] * vec1[0] + vec0[1] * vec1[1] + vec0[2] * vec1[2];
	if(tmp > 0)
		return 1.0 - tmp;
	else
		return 1.0 + tmp;
}

boolean la_t_poly_find_quads_sort_func(uint bigger, uint smaller, void *user)
{
	return ((double *)user)[bigger] < ((double *)user)[smaller];
}

#define LA_T_QUAD_FIND_DISTORTION_LIMIT 0.8

void la_t_poly_find_quads(void)
{
	uint i, j, k, other, otherj, t, *list;
	uint32 vertex_count, ref_count, *n, *ref, *crease;
	double *vertex, *weight;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);
	n = la_compute_neighbor(ref, ref_count, vertex_count, vertex);
	weight = malloc((sizeof *weight) * ref_count * 4);

	for(i = 0; i < ref_count * 4; i++)
		weight[i] = -1;

	for(i = 0; i < ref_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
		{
			if(ref[i + 3] >= vertex_count || vertex[ref[i + 3] * 3] == V_REAL64_MAX)
			{
				if(udg_get_select(ref[i]) > 0.1 && udg_get_select(ref[i + 1]) > 0.1 && udg_get_select(ref[i + 2]) > 0.1)
				{
					for(j = 0; j < 3; j++)
					{
						if(n[i + j] < i + j && crease[i + j] == 0)
						{
							other = (n[i + j] / 4) * 4;
							otherj = n[i + j] - other;
							if(udg_get_select(ref[other + (otherj + 2) % 3]) > 0.1)
								weight[i + j] = weight[n[i + j]] = la_t_poly_quad_test(&vertex[ref[i + (j + 2) % 3] * 3], &vertex[ref[i + (j + 1) % 3] * 3], &vertex[ref[other + (otherj + 2) % 3] * 3], &vertex[ref[i + j] * 3]);
						}

					}
				}
			}
		}
	}
	list = f_sort_ids(ref_count * 4, NULL, la_t_poly_find_quads_sort_func, weight);
	for(k = 0; k < ref_count * 4; k++)
	{
		i = list[k];
		if(weight[i] > LA_T_QUAD_FIND_DISTORTION_LIMIT && weight[n[i]] > LA_T_QUAD_FIND_DISTORTION_LIMIT)
		{
			j = i % 4;
			i = (i / 4) * 4;
			other = (n[i + j] / 4) * 4;
			otherj = n[i + j] - other;
			udg_polygon_set(i / 4, ref[i + (j + 1) % 3], ref[i + (j + 2) % 3], ref[i + j], ref[other + (otherj + 2) % 3]);
			udg_crease_set(i / 4, crease[i + (j + 1) % 3], crease[i + (j + 2) % 3], crease[other + (otherj + 1) % 3], crease[other + (otherj + 2) % 3]);
			udg_polygon_delete(other / 4);
			weight[i + 0] = weight[i + 1] = weight[i + 2] = weight[i + 3] = -1.0;
			weight[other + 0] = weight[other + 1] = weight[other  + 2] = weight[other + 3] = -1.0;
		}
	}
	free(list);
	free(weight);
}



uint la_t_poly_quad_corner_test(double *a, double *b, double *c)
{
	double v[3], v2[3], v3[3], r0, r1, r2;
	v[0] = b[0] - a[0];
	v[1] = b[1] - a[1];
	v[2] = b[2] - a[2];
	r0 = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= r0;
	v[1] /= r0;
	v[2] /= r0;
	v2[0] = c[0] - b[0];
	v2[1] = c[1] - b[1];
	v2[2] = c[2] - b[2];
	r0 = sqrt(v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);
	v2[0] /= r0;
	v2[1] /= r0;
	v2[2] /= r0;
	v3[0] = a[0] - c[0];
	v3[1] = a[1] - c[1];
	v3[2] = a[2] - c[2];
	r0 = sqrt(v3[0] * v3[0] + v3[1] * v3[1] + v3[2] * v3[2]);
	v3[0] /= r0;
	v3[1] /= r0;
	v3[2] /= r0;
	r0 = v[0] * -v3[0] + v[1] * -v3[1] + v[2] * -v3[2];
	r1 = v2[0] * -v[0] + v2[1] * -v[1] + v2[2] * -v[2];
	r2 = v3[0] * -v2[0] + v3[1] * -v2[1] + v3[2] * -v2[2];

	if(r0 > r1)
	{
		if(r1 > r2)
			return 2;
		else
			return 1;
	}else
	{
		if(r0 > r2)
			return 2;
		else
			return 0;
	}
}

void la_t_poly_find_quads_old(void)
{
	uint i, j, other, otherj, t;
	uint32 vertex_count, ref_count, *n, *ref, *crease;
	double *vertex;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);
	n = la_compute_neighbor(ref, ref_count, vertex_count, vertex);
	for(i = 0; i < ref_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
		{
			if(ref[i + 3] >= vertex_count || vertex[ref[i + 3] * 3] == V_REAL64_MAX)
			{
				if(udg_get_select(ref[i]) > 0.1 && udg_get_select(ref[i + 1]) > 0.1 && udg_get_select(ref[i + 2]) > 0.1)
				{
					j = (la_t_poly_quad_corner_test(&vertex[ref[i] * 3], &vertex[ref[i + 1] * 3], &vertex[ref[i + 2] * 3]) + 1) % 3;
					if(n[i + j] != -1 && n[i + j] < i + j && crease[i + j] == 0)
					{
						other = (n[i + j] / 4) * 4;
						otherj = n[i + j] - other;
						if(udg_get_select(ref[other + (otherj + 2) % 3]) > 0.1)
						{
							if(ref[(other / 4) * 4 + 3] >= vertex_count || vertex[ref[(other / 4) * 4 + 3] * 3] == V_REAL64_MAX)
							{
								if(0 == (la_t_poly_quad_corner_test(&vertex[ref[other + otherj] * 3], &vertex[ref[other + (otherj + 1) % 3] * 3], &vertex[ref[other + (otherj + 2) % 3] * 3]) + 1) % 3)
								{
									udg_polygon_set(i / 4, ref[i + (j + 1) % 3], ref[i + (j + 2) % 3], ref[i + j], ref[other + (otherj + 2) % 3]);
									udg_crease_set(i / 4, crease[i + (j + 1) % 3], crease[i + (j + 2) % 3], crease[other + (otherj + 1) % 3], crease[other + (otherj + 2) % 3]);
									udg_polygon_delete(other / 4);
								}
							}
						}
					}
				}
			}
		}
	}
}

void la_t_poly_auto_crease(void)
{
	uint i, j;
	uint32 vertex_count, ref_count, *n, *ref, *crease, poly, out_crease[4];
	double *vertex, r, x, y, z, x2, y2, z2, x3, y3, z3, *normals;

	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);
	n = la_compute_neighbor(ref, ref_count, vertex_count, vertex);
	normals = malloc((sizeof *normals) * ref_count * 30);

	for(i = 0; i < ref_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
		{
			x = vertex[ref[i] * 3 + 0] - vertex[ref[i + 1] * 3 + 0];
			y = vertex[ref[i] * 3 + 1] - vertex[ref[i + 1] * 3 + 1];
			z = vertex[ref[i] * 3 + 2] - vertex[ref[i + 1] * 3 + 2];
			r = sqrt(x * x + y * y + z * z);
			x = x / r;
			y = y / r;
			z = z / r;
			x2 = vertex[ref[i + 2] * 3 + 0] - vertex[ref[i + 1] * 3 + 0];
			y2 = vertex[ref[i + 2] * 3 + 1] - vertex[ref[i + 1] * 3 + 1];
			z2 = vertex[ref[i + 2] * 3 + 2] - vertex[ref[i + 1] * 3 + 2];
			r = sqrt(x2 * x2 + y2 * y2 + z2 * z2);
			x2 = x2 / r;
			y2 = y2 / r;
			z2 = z2 / r;
			x3 = y * z2 - z * y2;
			y3 = z * x2 - x * z2;
			z3 = x * y2 - y * x2;
			r = sqrt(x3 * x3 + y3 * y3 + z3 * z3);
			normals[(i / 4) * 3 + 0] = y3 / r;
			normals[(i / 4) * 3 + 1] = z3 / r;
			normals[(i / 4) * 3 + 2] = x3 / r;
		}
	}

	for(i = 0; i < ref_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
		{
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != V_REAL64_MAX)
				poly = 4;
			else
				poly = 3;

			for(j = 0; j < poly; j++)
			{
				if(udg_get_select(ref[i + j]) > 0.1 && udg_get_select(ref[i + (j + 1) % poly]) && n[i + j] != -1)
				{
				/*	printf("normals %i %f\n", j , normals[(n[i + j] / 4) * 3 + 0]
						* normals[(i / 4) * 3 + 0]
						+ normals[(n[i + j] / 4) * 3 + 1]
						* normals[(i / 4) * 3 + 1]
						+ normals[(n[i + j] / 4) * 3 + 2]
						* normals[(i / 4) * 3 + 2]);*/
					if(normals[(n[i + j] / 4) * 3 + 0]
						* normals[(i / 4) * 3 + 0]
						+ normals[(n[i + j] / 4) * 3 + 1]
						* normals[(i / 4) * 3 + 1]
						+ normals[(n[i + j] / 4) * 3 + 2]
						* normals[(i / 4) * 3 + 2] > 0.5)
						out_crease[j] = 0;
					else
						out_crease[j] = -1;
				}else
					out_crease[j] = crease[i + j];
			}
		//	printf("set = %u %u %u %u\n", out_crease[0], out_crease[1], out_crease[2], out_crease[3]);
			if(out_crease[0] != crease[i] || out_crease[1] != crease[i + 1] || out_crease[2] != crease[i + 2] || out_crease[3] != crease[i + 3])
				udg_crease_set(i / 4, out_crease[0], out_crease[1], out_crease[2], out_crease[3]);
		}
	}
	free(normals);
}
