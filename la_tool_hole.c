#include "la_includes.h"
#include "la_geometry_undo.h"



void compute_normal(egreal *normal, egreal *vertex, uint *ref)
{
	egreal r, v0[3], v1[3];
	v0[0] = vertex[ref[1] * 3 + 0] - vertex[ref[0] * 3 + 0];
	v0[1] = vertex[ref[1] * 3 + 1] - vertex[ref[0] * 3 + 1];
	v0[2] = vertex[ref[1] * 3 + 2] - vertex[ref[0] * 3 + 2];
	r = sqrt(v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2]);
	v0[0] /= r;
	v0[1] /= r;
	v0[2] /= r;
	v1[0] = vertex[ref[2] * 3 + 0] - vertex[ref[0] * 3 + 0];
	v1[1] = vertex[ref[2] * 3 + 1] - vertex[ref[0] * 3 + 1];
	v1[2] = vertex[ref[2] * 3 + 2] - vertex[ref[0] * 3 + 2];
	r = sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
	v1[0] /= r;
	v1[1] /= r;
	v1[2] /= r;
	normal[0] = v0[1] * v1[2] - v0[2] * v1[1];
	normal[1] = v0[2] * v1[0] - v0[0] * v1[2];
	normal[2] = v0[0] * v1[1] - v0[1] * v1[0];
	r = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= r;
	normal[1] /= r;
	normal[2] /= r;
}

uint *c_triangelize(uint *output_length, uint *ref, uint poly_count, egreal *vertex, uint vertex_count)
{
	egreal normal[3];
	uint i, j = 0, *tri;
	for(i = 0; i < poly_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
		{
			j++;
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				j++;
		}
	}
	tri = malloc((sizeof *tri) * j * 6);
	j  = 0;
	for(i = 0; i < poly_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
		{
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
			{
				compute_normal(normal, vertex, &ref[i]);
				if(0 < normal[0] * (vertex[ref[i + 3] * 3 + 0] - vertex[ref[i] * 3 + 0]) +
					normal[1] * (vertex[ref[i + 3] * 3 + 1] - vertex[ref[i] * 3 + 1]) +
					normal[2] * (vertex[ref[i + 3] * 3 + 2] - vertex[ref[i] * 3 + 2]))
				{
					tri[j++] = ref[i];
					tri[j++] = ref[i + 1];
					tri[j++] = ref[i + 2];
					tri[j++] = ref[i];
					tri[j++] = ref[i + 2];
					tri[j++] = ref[i + 3];
				}else
				{
					tri[j++] = ref[i];
					tri[j++] = ref[i + 1];
					tri[j++] = ref[i + 3];
					tri[j++] = ref[i + 1];
					tri[j++] = ref[i + 2];
					tri[j++] = ref[i + 3];
				}
			}else
			{
				tri[j++] = ref[i];
				tri[j++] = ref[i + 1];
				tri[j++] = ref[i + 2];
			}
		}
	}
	*output_length = j / 3;
	return tri;
}


uint *tri_edge_ref(uint *ref, uint ref_length, uint vertex_length)
{
	uint i, j, c, *edge, *vertex;
	edge = malloc((sizeof *edge) * ref_length * 6);
	vertex = malloc((sizeof *vertex) * vertex_length);
	for(i = 0; i < vertex_length;i++)
		vertex[i] = -1;
	for(i = 0; i < ref_length * 6; i++)
		edge[i] = -1;
	j = 0;
	while(j < ref_length)
	{
		for(i = 0; i < ref_length * 3; i++)
		{
			if(vertex[ref[i]] == -1)
				vertex[ref[i]] = i;
			else if(vertex[ref[i]] == i)
				vertex[ref[i]] = -1;
			else
			{
				c = vertex[ref[i]];
				if(edge[i] == -1 && ref[i - i % 3 + (i + 1) % 3] == ref[c - c % 3 + (c + 2) % 3])
				{
					edge[i] = c - c % 3 + (c + 2) % 3;
					edge[c - c % 3 + (c + 2) % 3] = i;
					j = 0;
				}
				if(edge[c] == -1 && ref[i - i % 3 + (i + 2) % 3] == ref[c - c % 3 + (c + 1) % 3])
				{
					edge[i - i % 3 + (i + 2) % 3] = c;
					edge[c] = i - i % 3 + (i + 2) % 3;
					j = 0;
				}
			}
			j++;
		}
	}
	free(vertex);
	return edge;
}

void cut_convex_edges(uint *n, uint *ref, egreal *vertex, uint ref_length)
{
	uint i, j, edge, v;
	egreal vec[3], f;
	for(i = 0; i < ref_length; i++)
	{
		compute_normal(vec, vertex, &ref[i * 3]);
		for(j = 0; j < 3; j++)
		{
			edge = n[i * 3 + j];
			v = ((edge / 3) * 3 + ((edge + 2) % 3)) * 3;

			f = vec[0] * (vertex[v] - vertex[ref[i * 3] * 3]) + 
					vec[1] * (vertex[v + 1] - vertex[ref[i * 3] * 3 + 1]) +
					vec[2] * (vertex[v + 2] - vertex[ref[i * 3] * 3 + 2]);
			if(f < -0.0001)
			{
				n[n[i * 3 + j]] = -1;
				n[i * 3 + j] = -1;
			}
		}
	}
}


boolean hull_test(uint *list, uint vertex_length, egreal *vertex, egreal *normal, egreal *origo)
{
	uint i;
	for(i = 0; i < vertex_length; i++)
	{
		if(list[i] != -1)
		{
			printf("hull test %f", (vertex[i * 3 + 0] - origo[0]) * normal[0] +
				(vertex[i * 3 + 1] - origo[1]) * normal[1] +
				(vertex[i * 3 + 2] - origo[2]) * normal[2]);
			if(-0.001 > (vertex[i * 3 + 0] - origo[0]) * normal[0] +
				(vertex[i * 3 + 1] - origo[1]) * normal[1] +
				(vertex[i * 3 + 2] - origo[2]) * normal[2])
				return FALSE;
		}
	}
	return TRUE;
}

typedef enum{
	PS_FULL,
	PS_PARSIAL,
	PS_OUTSIDE,
}PlaneStatus;
/*
	accepted[0] = ref[ref_length * 3 - 3];
	accepted[1] = ref[ref_length * 3 - 2];
	accepted[2] = ref[ref_length * 3 - 1];
*/

PlaneStatus hull_poly_test(uint *ref, PlaneStatus *status, uint ref_length, egreal *vertex, uint *polygon)
{
	egreal *origo, normal[3], *v1, *v2, *v3;
	uint test, i;
	PlaneStatus output = PS_FULL;

	for(i = 0; status[i] != PS_FULL; i++);

	compute_normal(normal, vertex, polygon);
	origo = &vertex[polygon[0] * 3];
	v1 = &vertex[ref[i * 3] * 3];
	v2 = &vertex[ref[i * 3 + 1] * 3];
	v3 = &vertex[ref[i * 3 + 2] * 3];
	if(-0.001 > (v1[0] - origo[0]) * normal[0] + (v1[1] - origo[1]) * normal[1] + (v1[2] - origo[2]) * normal[2])
		return PS_OUTSIDE;
	if(-0.001 > (v2[0] - origo[0]) * normal[0] + (v2[1] - origo[1]) * normal[1] + (v2[2] - origo[2]) * normal[2])
		return PS_OUTSIDE;
	if(-0.001 > (v3[0] - origo[0]) * normal[0] + (v3[1] - origo[1]) * normal[1] + (v3[2] - origo[2]) * normal[2])
		return PS_OUTSIDE;

	v1 = &vertex[polygon[0] * 3];
	v2 = &vertex[polygon[1] * 3];
	v3 = &vertex[polygon[2] * 3];
	
	for(i = 0; i < ref_length; i++)
	{
		if(status[i] != PS_OUTSIDE && polygon != &ref[i * 3])
		{
			compute_normal(normal, vertex, &ref[i * 3]);
			origo = &vertex[ref[i * 3] * 3];
			test = 0;
			if(-0.01 < (v1[0] - origo[0]) * normal[0] + (v1[1] - origo[1]) * normal[1] + (v1[2] - origo[2]) * normal[2])
				test++;
			if(-0.01 < (v2[0] - origo[0]) * normal[0] + (v2[1] - origo[1]) * normal[1] + (v2[2] - origo[2]) * normal[2])
				test++;
			if(-0.01 < (v3[0] - origo[0]) * normal[0] + (v3[1] - origo[1]) * normal[1] + (v3[2] - origo[2]) * normal[2])
				test++;
			if(test == 0)
				return PS_OUTSIDE;
			if(test < 3)
				output = PS_PARSIAL;
		}
	}
	return output;
}

void create_hulls(uint *ref, uint ref_length, egreal *vertex)
{
	uint i, key, hulls = 0;
	PlaneStatus *status, *accepted;
	accepted = malloc((sizeof *accepted) * ref_length);
	status = malloc((sizeof *status) * ref_length);

	for(i = 0; i < ref_length; i++)
		accepted[i] = PS_OUTSIDE;
	status[0] = PS_FULL;
	i = 0;
	while(i != ref_length)
	{

		key = i;
		status[key] = PS_FULL;
		for(i = 0; i < ref_length; i++)
			if(i != key)
				status[i] = hull_poly_test(&ref[key * 3], &status[key], 1, vertex, &ref[i * 3]);

		for(i = 0; i < ref_length; i++)
			if(i != key)
				status[i] = hull_poly_test(ref, status, ref_length, vertex, &ref[i * 3]);

		for(i = 0; i < ref_length; i++)
			if(status[i] == PS_FULL)
				accepted[i] = PS_FULL;
		for(i = 0; i < ref_length; i++)
			printf("status %u\n", status[i]);

		for(i = 0; i < ref_length; i++)
			if(status[i] == PS_FULL)
				udg_polygon_set(udg_find_empty_slot_polygon(), ref[i * 3], ref[i * 3 + 1], ref[i * 3 + 2], -1);
	//	if(hulls == 1)
	//			udg_polygon_set(udg_find_empty_slot_polygon(), ref[key * 3], ref[key * 3 + 1], ref[key * 3 + 2], -1);


		for(i = 0; i < ref_length && accepted[i] == PS_FULL; i++);
		hulls++;
	}
}


void move_polygon(uint *ref, uint *n, uint from, uint to)
{
}
/*
uint *find_corners(uint *ref, uint *n, egreal *vertex, uint ref_length, uint vertex_length)
{
	uint i, j, a, b, c;
	for(i = 0; i < ref_length; i++)
	{
		for(j = 0; j < 3; j++)
		{
			a = i * 3 + j;
			b = n[a];
			b = (b / 3) * 3 + (b + 1) % 3; 
			c = n[b];
			c = (c / 3) * 3 + (c + 1) % 3; 
			if(i == n[k] / 3)
			{
				if(n[(a / 3) * 3 + (a + 2) % 3] / 3 == n[(b / 3) * 3 + (b + 2) % 3] / 3)
				{
					Last one!
				}else
				{
					(b / 3) * 3 + (b + 2) % 3; 
					(c / 3) * 3 + (c + 2) % 3; 
					}
			}
		}
	}

}*/


uint *create_polygon_map(uint *ref, egreal *vertex, uint ref_length, uint vertex_length)
{
	uint found = 0, *poly_own, *vertex_own, i = 0, j, group = 0;
	egreal normal[3];

	poly_own = malloc((sizeof *poly_own) * ref_length * 2);
	for(i = 0; i < ref_length; i++)
		poly_own[i] = -1;
	vertex_own = malloc((sizeof *vertex_own) * vertex_length);

	i  = 0;
	while(i < ref_length)
	{
		for(j = 0; j < vertex_length; j++)
			vertex_own[j] = -1;
		poly_own[i] = group;
		vertex_own[ref[i * 3 + 0]] = group;
		vertex_own[ref[i * 3 + 1]] = group;
		vertex_own[ref[i * 3 + 2]] = group;
		found = 1;
		printf("group %i\n", group);
		while(found != 0)
		{
			found = 0;
			for(i = 0; i < ref_length; i++)
			{
				if(poly_own[i] == -1)
				{
					if(vertex_own[ref[i * 3 + 0]] != -1 && vertex_own[ref[i * 3 + 1]] != -1 && vertex_own[ref[i * 3 + 2]] != -1)
						poly_own[i] = group;
					else
					{
						printf("a\n");
						for(j = 0; j < 3; j++)
						{
							if(vertex_own[ref[i * 3 + j]] == -1 && vertex_own[ref[i * 3 + ((j + 1) % 3)]] != -1 && vertex_own[ref[i * 3 + ((j + 2) % 3)]] != -1)
							{
								printf("b\n");
								compute_normal(normal, vertex, &ref[i * 3]);
								if(hull_test(vertex_own, vertex_length, vertex, normal, &vertex[ref[i * 3 + j] * 3]))
								{
									vertex_own[ref[i * 3 + j]] = group;
									poly_own[i] = group;
									found = 1;
								}
							}
						}
					}
				}
			}
		}
		group++;
		for(i = 0; i < ref_length && poly_own[i] != -1; i++);
	}
	free(vertex_own);
	return poly_own;
}

typedef struct{
	egreal plane[3];
	egreal dist;
}CPlane;


typedef struct{
	CPlane *planes;
	uint 	count;
}CHull;

typedef struct{
	CHull	*huls;
	uint 	count;
}CBody;


boolean add_plane(CHull *hull, egreal *normal, egreal dist)
{
	uint i;
	for(i = 0; i < hull->count; i++)
		if(0.97 < normal[0] * hull->planes[i].plane[0] + normal[1] * hull->planes[i].plane[1] + normal[2] * hull->planes[i].plane[2])
			return FALSE;
	hull->planes[i].plane[0] = normal[0];
	hull->planes[i].plane[1] = normal[1];
	hull->planes[i].plane[2] = normal[2];
	hull->planes[i].dist = dist;
	hull->count++;
	return TRUE;
}
/*
CPlane *create_planes(uint *ref, egreal *vertex, uint ref_length, uint vertex_length)
{
	CPlane *planes;
	egreal normal[3];
	uint i, length = 0;
	planes = malloc((sizeof *planes) * ref_length);
	for(i = 0; i < ref_length; i++)
	{
		compute_normal(normal, vertex, &ref[i * 3]);
		add_plane(planes, &length, normal, normal[0] * vertex[ref[i * 3] * 3] +
											normal[1] * vertex[ref[i * 3] * 3 + 1] +
											normal[2] * vertex[ref[i * 3] * 3 + 2]);
	}
	return planes;
}*/

void sort_planes(CPlane *plane, uint *length)
{
	egreal f, bestf, normal[3] = {0, 0, 0};
	uint i, j, besti;
	CPlane p;
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 0;
	for(i = 0; i < *length; i++)
	{
		normal[0] += plane[i].plane[0];
		normal[1] += plane[i].plane[1];
		normal[2] += plane[i].plane[2];
		bestf = 100000000;
		for(j = i + 1; j < *length; j++)
		{
			f = normal[0] * plane[j].plane[0] + normal[1] * plane[j].plane[1] + normal[2] * plane[j].plane[2];
			if(f < bestf)
			{
				bestf = f;
				besti = j;
			}
		}
		p = plane[i + 1];
		plane[i + 1] = plane[besti]; 
		plane[besti] = p; 
	}
}

uint close_a_hole(uint *triangles, uint *n, /*uint *group,*/ uint *length, uint start)
{
	uint i;
	i = start;
	while(TRUE)
	{
		printf("i %i %i, %i\n", i, (i / 3 * 3) + (i + 1) % 3, *length);
		i = (i / 3 * 3) + (i + 1) % 3;
	/*	if(i > *length * 3)
			exit(0);*/
		if(n[i] == -1)
		{
			if(triangles[start] == triangles[(i / 3 * 3) + (i + 1) % 3])
			{
				n[start] = i;
				n[i] = start;
				return -1;
			}else
			{
			/*	group[*length] = triangles[start / 3];*/
				triangles[*length * 3 + 0] = triangles[start];
				triangles[*length * 3 + 1] = triangles[(i / 3 * 3) + (i + 1) % 3];
				triangles[*length * 3 + 2] = triangles[(start / 3 * 3) + (start + 1) % 3];
				n[*length * 3 + 0] = -1;
				n[*length * 3 + 1] = i;
				n[*length * 3 + 2] = start;
				n[-1] = *length * 3 + 0;
				n[i] = *length * 3 + 1;
				n[start] = *length * 3 + 2;
				(*length)++;
				return (*length - 1) * 3;
			}
		}

		i = n[i];
	}	
}



void close_all_holes(uint *triangles, uint *n, /*uint *group,*/ uint *length)
{
	uint i, start;
	for(i = 0; i < *length; i++)
	{
		if(n[i] == -1)
		{
			start = i;
			printf("start %i\n", start);
			while((start = close_a_hole(triangles, n, /*group,*/ length, start)) != -1); 
		}
	}
}


void la_close_all_holes()
{
	uint i, *map, *count, ref_count, *ref, vertex_count, *tri, tri_count, org_tri_count, *n;
	egreal *vertex;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);
	tri = c_triangelize(&org_tri_count, ref, ref_count, vertex, vertex_count);
/*	n = tri_edge_ref(tri, org_tri_count, vertex_count);
	map = create_polygon_map(tri, vertex, org_tri_count, vertex_count);
	for(i = 0; i < org_tri_count * 3; i++)
		if(n[i] != -1 && map[i / 3] != map[n[i] / 3])
			n[i] = -1;
	for(i = 0; i < org_tri_count; i++)
		printf("map %i\n", map[i]);
	printf("\n");

	for(i = 0; i < org_tri_count * 3; i++)
		if(map[i / 3] = 0)
			udg_set_select(tri[i], 1);
	tri_count = org_tri_count;
	close_all_holes(tri, n,  &tri_count);
	for(i = org_tri_count; i < tri_count; i++)
		udg_polygon_set(udg_find_empty_slot_polygon(), tri[i * 3], tri[i * 3 + 1], tri[i * 3 + 2], -1);
*/
	create_hulls(tri, org_tri_count, vertex);
	
	undo_event_done();

//	printf("la_close_all_holes()");
//	exit(0);
}
/*
typedef struct{
	egreal plane[3];
	egreal dist;
}CPlane;


typedef struct{
	CPlane *planes;
	uint 	count;
}CHull;

typedef struct{
	CHull	*huls;
	uint 	count;
}CBody;*/

