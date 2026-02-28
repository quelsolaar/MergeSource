#include "la_includes.h"
#include "la_geometry_undo.h"

boolean la_poly_intersect(double *a, double *b, double *v0, double *v1, double *v2)
{
	double vec[3], t, u, v;
	vec[0] = b[0] - a[0];
	vec[1] = b[1] - a[1];
	vec[2] = b[2] - a[2];
	if(f_raycast_trid(a, vec, v0, v1, v2, &t, &u, &v))
	{
		if(t < 1 && t > 0)
			return TRUE;
	}
	return FALSE;
}
void la_poly_project(double *output, double *line_a, double *line_b, double *v0, double *v1, double *v2)
{
	double a[3], b[3], normal[3];
	a[0] = v0[0] - v2[0];
	a[1] = v0[1] - v2[1];
	a[2] = v0[2] - v2[2];
	b[0] = v1[0] - v2[0];
	b[1] = v1[1] - v2[1];
	b[2] = v1[2] - v2[2];
	f_cross3d(normal, a, b);
	f_normalize3d(normal);
	a[0] = line_b[0] - line_a[0];
	a[1] = line_b[1] - line_a[1];
	a[2] = line_b[2] - line_a[2];
	f_project3d(output, v2, normal, line_a, a);
}

void la_poly_csg()
{
	uint i, j, k, poly;
	uint32 vertex_count, ref_count, *n, *ref, *crease, out_crease[4];
	double *vertex, r, x, y, z, x2, y2, z2, x3, y3, z3, output[3];
	return;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);

	for(i = 0; i < ref_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
		{
			poly = 3;
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3]] != V_REAL64_MAX)
				poly = 4;

			for(j = 0; j < ref_count * 4; j += 4)
			{
				if(ref[j] < vertex_count && ref[j + 1] < vertex_count && ref[j + 2] < vertex_count && vertex[ref[j] * 3] != V_REAL64_MAX && vertex[ref[j + 1] * 3] != V_REAL64_MAX && vertex[ref[j + 2] * 3] != V_REAL64_MAX)
				{
					for(k = 0; k < poly; k++)
					{
						if(ref[i + k] != ref[j] &&
							ref[i + k] != ref[j + 1] &&
							ref[i + k] != ref[j + 2] &&
							ref[i + (k + 1) % poly] != ref[j] &&
							ref[i + (k + 1) % poly] != ref[j + 1] &&
							ref[i + (k + 1) % poly] != ref[j + 2])
						{
							if(la_poly_intersect(&vertex[ref[i + k] * 3], &vertex[ref[i + (k + 1) % poly] * 3], 
													&vertex[ref[j] * 3], &vertex[ref[j + 1] * 3], &vertex[ref[j + 2] * 3]))
							{
								la_poly_project(output, &vertex[ref[i + k] * 3], &vertex[ref[i + (k + 1) % poly] * 3],
													&vertex[ref[j] * 3], &vertex[ref[j + 1] * 3], &vertex[ref[j + 2] * 3]);
								r_primitive_line_3d(output[0] - 0.1, output[1], output[2], output[0] + 0.1, output[1], output[2], 1, 0, 0, 1);
								r_primitive_line_3d(output[0], output[1] - 0.1, output[2], output[0], output[1] + 0.1, output[2], 0, 1, 0, 1);
								r_primitive_line_3d(output[0], output[1], output[2] - 0.1, output[0], output[1], output[2] + 0.1, 0, 0, 1, 1);							
								
								r_primitive_line_3d(vertex[ref[j] * 3], vertex[ref[j] * 3 + 1], vertex[ref[j] * 3 + 2], 
																	output[0], output[1], output[2], 0, 0, 1, 0.3);	
								r_primitive_line_3d(vertex[ref[j + 1] * 3], vertex[ref[j + 1] * 3 + 1], vertex[ref[j + 1] * 3 + 2], 
																	output[0], output[1], output[2], 0, 0, 1, 0.3);	
								r_primitive_line_3d(vertex[ref[j + 2] * 3], vertex[ref[j + 2] * 3 + 1], vertex[ref[j + 2] * 3 + 2], 
																	output[0], output[1], output[2], 0, 0, 1, 0.3);	

								r_primitive_line_3d(vertex[ref[i + k] * 3], vertex[ref[i + k] * 3 + 1], vertex[ref[i + k] * 3 + 2], 
																	output[0], output[1], output[2], 1, 0, 1, 0.3);	
								r_primitive_line_3d(vertex[ref[i + (k + 1) % poly] * 3], vertex[ref[i + (k + 1) % poly] * 3 + 1], vertex[ref[i + (k + 1) % poly] * 3 + 2], 
																	output[0], output[1], output[2], 1, 0, 1, 0.3);	


							}
						}
					}
				/*	if(ref[j + 3] < vertex_count && vertex[ref[j + 3]] != V_REAL64_MAX)
					{
						for(k = 0; k < poly; k++)
						{
							if(ref[i + k] != ref[j] &&
								ref[i + k] != ref[j + 1] &&
								ref[i + k] != ref[j + 2] &&
								ref[i + (k + 1) % poly] != ref[j] &&
								ref[i + (k + 1) % poly] != ref[j + 1] &&
								ref[i + (k + 1) % poly] != ref[j + 2])
							{

								if(la_poly_intersect(&vertex[ref[i + k] * 3], &vertex[ref[i + (k + 1) % poly] * 3], 
														&vertex[ref[j] * 3], &vertex[ref[j + 2] * 3], &vertex[ref[j + 3] * 3]))
								{
									la_poly_project(output, &vertex[ref[i + k] * 3], &vertex[ref[i + (k + 1) % poly] * 3],
														&vertex[ref[j] * 3], &vertex[ref[j + 2] * 3], &vertex[ref[j + 3] * 3]);
									r_primitive_line_3d(output[0] - 0.1, output[1], output[2], output[0] + 0.1, output[1], output[2], 1, 0, 0, 1);
									r_primitive_line_3d(output[0], output[1] - 0.1, output[2], output[0], output[1] + 0.1, output[2], 0, 1, 0, 1);
									r_primitive_line_3d(output[0], output[1], output[2] - 0.1, output[0], output[1], output[2] + 0.1, 0, 0, 1, 1);
								}
							}
						}
					}*/
				}
			}
		}
	}
	r_primitive_line_flush();
}