#include "la_includes.h"
#include "la_geometry_undo.h"

#include "la_particle_fx.h"

typedef enum{
	SVP_INSIDE,
	SVP_OUTSIDE,
	SVP_UNSELECTED,
	SVP_MIDDLE
}SplitVertexPos;

SplitVertexPos compute_vertex_split_desition(double *pos, double *vector, uint id)
{
	double vertex[3];
	if(udg_get_select(id) < 0.01)
		return SVP_UNSELECTED;
	udg_get_vertex_pos(vertex, id);
	vertex[0] -= pos[0];
	vertex[1] -= pos[1];
	vertex[2] -= pos[2];
	if((vector[0] * vertex[0] + vector[1] * vertex[1] + vector[2] * vertex[2]) < 0)
		return SVP_OUTSIDE;
	else
		return SVP_INSIDE;
}

uint create_new_split_vertex(uint32 **edge, uint32 *edge_length, uint32 *edge_alloc, double *origo, double *vector, uint id_a, uint id_b)
{
	double a[3], b[3], r;
	uint i, id;
	for(i = 0; i < *edge_length && ((*edge)[i * 3] != id_a || (*edge)[i * 3 + 1] != id_b); i++);
	if(i == *edge_length)
	{
		if(*edge_length == *edge_alloc)
		{
			*edge_alloc += 16;
			*edge = realloc(*edge, (sizeof **edge) * *edge_alloc * 3);
		}
		(*edge_length)++;
		(*edge)[i * 3] = id_b;
		(*edge)[i * 3 + 1] = id_a;
		(*edge)[i * 3 + 2] = id = udg_find_empty_slot_vertex();
		udg_get_vertex_pos(a, id_a);
		udg_get_vertex_pos(b, id_b);
		b[0] -= a[0];
		b[1] -= a[1];
		b[2] -= a[2];
		r = b[0] * vector[0] + b[1] * vector[1] + b[2] * vector[2];
		r = (vector[0] * (a[0] - origo[0]) + vector[1] * (a[1] - origo[1]) + vector[2] * (a[2] - origo[2])) / r;
		a[0] -= r * b[0];
		a[1] -= r * b[1];
		a[2] -= r * b[2];
		udg_vertex_set(id, NULL, a[0], a[1], a[2]);
		la_pfx_create_spark(a);
	}else
	{
		(*edge_length)--;
		id = (*edge)[i * 3 + 2];
		(*edge)[i * 3] = (*edge)[*edge_length * 3];
		(*edge)[i * 3 + 1] = (*edge)[*edge_length * 3 + 1];
		(*edge)[i * 3 + 2] = (*edge)[*edge_length * 3 + 2];
	}
	return id;
}

void digonal_quad_split(SplitVertexPos *desition, uint32 *ref, uint32 *crease, uint id, boolean del)
{
	uint i, poly;
	if(desition[0] != SVP_MIDDLE || desition[1] != SVP_MIDDLE || desition[2] != SVP_MIDDLE || desition[3] != SVP_MIDDLE)
	{
		for(i = 0; i < 2; i++)
		{
			if(desition[i] == SVP_MIDDLE && desition[2 + 1] == SVP_MIDDLE)
			{
				if(desition[i + 1] == SVP_OUTSIDE && del)
				{
					udg_polygon_delete(id);
					udg_polygon_set(id, ref[i], ref[i + 2], ref[(i + 3) % 4], -1);
					udg_crease_set(id, 0, crease[i + 2], crease[(i + 3) % 4], 0);
				}
				else if(desition[(i + 3) % 4] == SVP_OUTSIDE && del)
				{
					udg_polygon_set(id, ref[i], ref[i + 1], ref[i + 2], -1);
					udg_crease_set(id, crease[i], crease[i + 1], 0, 0);
				}else
				{
					udg_polygon_set(id, ref[i], ref[i + 1], ref[i + 2], -1);
					udg_crease_set(id, crease[i], crease[i + 1], 0, 0);
					id = udg_find_empty_slot_polygon();
					udg_polygon_set(id, ref[i], ref[i + 2], ref[(i + 3) % 4], -1);
					udg_crease_set(id, 0, crease[i + 2], crease[(i + 3) % 4], 0);
				}
			}
		}
	}
}

void la_t_slice(double *pos, double *vector, boolean del)
{
	uint32 vertex_count, ref_count, *ref, *crease, i, j, id, poly, new_vertex[4];
	uint32 *edge, edge_length, edge_alloc;
	SplitVertexPos desition[4];
	udg_get_geometry(&vertex_count, &ref_count, NULL, &ref, &crease);
	edge_alloc = 32;
	edge_length = 0;
	edge = malloc((sizeof *edge) * edge_alloc * 3);
	ref_count *= 4;
	for(i = 0; i < ref_count; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count)
		{
			if(ref[i + 3] < vertex_count)
				poly = 4;
			else
				poly = 3;

			for(j = 0; j < poly && udg_get_select(ref[i + j]) > 0.001; j++);
			if(j == poly)
			{
				for(j = 0; j < poly; j++)
					desition[j] = compute_vertex_split_desition(pos, vector, ref[i + j]);
				if(del)
				{
					for(j = 0; j < poly && desition[j] != SVP_INSIDE; j++);
					if(j == poly)
						udg_polygon_delete(i);
				}
			
				for(j = 0; j < poly && desition[j] != SVP_OUTSIDE; j++);
				if(j != poly)
				{
					for(j = 0; j < poly && desition[j] != SVP_INSIDE; j++);
					if(j != poly)
					{
						if(poly == 4)
						{
							if((desition[0] == SVP_INSIDE && desition[1] == SVP_OUTSIDE && desition[2] == SVP_INSIDE && desition[3] == SVP_OUTSIDE) || (desition[0] == SVP_OUTSIDE && desition[1] == SVP_INSIDE && desition[2] == SVP_OUTSIDE && desition[3] == SVP_INSIDE))
							{
								new_vertex[0] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + 0], ref[i + 1]);
								new_vertex[1] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + 1], ref[i + 2]);
								new_vertex[2] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + 2], ref[i + 3]);
								new_vertex[3] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + 3], ref[i + 0]);
								udg_polygon_set(i / 4, new_vertex[0], new_vertex[1], new_vertex[2], new_vertex[3]);
								udg_crease_set(i / 4, 0, 0, 0, 0);
								for(j = 0; j < 4; j++)
								{
									if(desition[j] != SVP_OUTSIDE || del != TRUE)
									{
										id = udg_find_empty_slot_polygon();
										udg_polygon_set(id, new_vertex[(j + 3) % 4], ref[i + j], new_vertex[j], -1);
										if(crease != NULL)
											udg_crease_set(id, crease[i + (j + 3) % 4], crease[i + (j + 0) % 4], 0, 0);
									}
								}

							}else for(j = 0; j < 4; j++)
							{
								if(desition[j] == SVP_INSIDE && desition[(j + 1) % 4] == SVP_OUTSIDE && desition[(j + 2) % 4] == SVP_INSIDE)
								{
									new_vertex[0] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + j], ref[i + (j + 1) % 4]);
									new_vertex[1] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + (j + 1) % 4], ref[i + (j + 2) % 4]);
									udg_polygon_set(i / 4, new_vertex[0], new_vertex[1], ref[i + (j + 2) % 4], ref[i + j]);
									if(crease != NULL)
										udg_crease_set(i / 4, 0, crease[i + (j + 1) % 4], 0, crease[i + j]);					
									id = udg_find_empty_slot_polygon();
									udg_polygon_set(id, ref[i + (j + 2) % 4], ref[i + (j + 3) % 4], ref[i + j], -1);
									if(crease != NULL)
										udg_crease_set(id, crease[i + (j + 2) % 4], crease[i + (j + 3) % 4], 0, 0);
									if(del != TRUE)
									{
										id = udg_find_empty_slot_polygon();
										udg_polygon_set(id, new_vertex[0], ref[i + (j + 1) % 4], new_vertex[1], -1);
										if(crease != NULL)
											udg_crease_set(id, crease[i + j], crease[i + (j + 1) % 4], 0, 0);
									}
									break;
								}
								if(desition[j] == SVP_OUTSIDE && desition[(j + 1) % 4] == SVP_INSIDE && desition[(j + 2) % 4] == SVP_OUTSIDE)
								{
									new_vertex[0] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + j], ref[i + (j + 1) % 4]);
									new_vertex[1] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + (j + 1) % 4], ref[i + (j + 2) % 4]);
									udg_polygon_set(i / 4, new_vertex[0], ref[i + (j + 1) % 4], new_vertex[1], -1);
									if(crease != NULL)
										udg_crease_set(i / 4, crease[i + j], crease[i + (j + 1) % 4], 0, 0);
									if(del != TRUE)
									{
										id = udg_find_empty_slot_polygon();
										udg_polygon_set(id, new_vertex[0], new_vertex[1], ref[i + (j + 2) % 4], ref[i + j]);
										if(crease != NULL)
											udg_crease_set(id, 0, crease[i + (j + 1) % 3], 0, crease[i + j]);					
										id = udg_find_empty_slot_polygon();
										udg_polygon_set(id, ref[i + (j + 2) % 4], ref[i + (j + 3) % 4], ref[i + j], -1);
										if(crease != NULL)
											udg_crease_set(id, crease[i + (j + 2) % 4], crease[i + (j + 3) % 4], 0, 0);
									}
									break;
								}
								if(desition[j] == SVP_INSIDE && desition[(j + 1) % 4] == SVP_INSIDE && desition[(j + 2) % 4] == SVP_OUTSIDE && desition[(j + 3) % 4] == SVP_OUTSIDE)
								{
									new_vertex[0] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + (j + 1) % 4], ref[i + (j + 2) % 4]);
									new_vertex[1] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + (j + 3) % 4], ref[i + (j + 0) % 4]);
									udg_polygon_set(i / 4, ref[i + (j + 0) % 4], ref[i + (j + 1) % 4], new_vertex[0], new_vertex[1]);
									if(crease != NULL)
										udg_crease_set(i / 4, crease[i + j], crease[i + (j + 1) % 4], 0, crease[i + (j + 3) % 4]);
									if(del != TRUE)
									{
										id = udg_find_empty_slot_polygon();
										udg_polygon_set(id, new_vertex[1], new_vertex[0], ref[i + (j + 2) % 4], ref[i + (j + 3) % 4]);
										if(crease != NULL)
											udg_crease_set(id, 0, crease[i + (j + 1) % 4], crease[i + (j + 2) % 4], crease[i + (j + 3) % 4]);
									}
								}
							}
							digonal_quad_split(desition, &ref[i], &crease[i], i / 4, del);
						}else
						{
							for(j = 0; j < 3; j++)
							{
								if(desition[j] == SVP_UNSELECTED || desition[j] == SVP_MIDDLE)
								{
									new_vertex[0] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + (j + 1) % 3], ref[i + (j + 2) % 3]);
									udg_polygon_set(i / 4, ref[i + j], ref[i + (j + 1) % 3], new_vertex[0], -1);
									if(crease != NULL)
										udg_crease_set(i / 4, crease[i + j], crease[i + (j + 1) % 3], 0, 0);
									id = udg_find_empty_slot_polygon();
									udg_polygon_set(id, ref[i + j], new_vertex[0], ref[i + (j + 2) % 3], -1);
									if(crease != NULL)
										udg_crease_set(id, 0, crease[i + (j + 1) % 3], crease[i + (j + 2) % 3], 0);
									break;
								}
								if(desition[j] == SVP_INSIDE && desition[(j + 1) % 3] == SVP_OUTSIDE && desition[(j + 2) % 3] == SVP_INSIDE)
								{
									new_vertex[0] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + j], ref[i + ((j + 1) % 3)]);
									new_vertex[1] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + ((j + 1) % 3)], ref[i + ((j + 2) % 3)]);
									udg_polygon_set(i / 4, new_vertex[0], new_vertex[1], ref[i + (j + 2) % 3], ref[i + j]);
									if(crease != NULL)
										udg_crease_set(i / 4, 0, crease[i + (j + 1) % 3], crease[i + (j + 2) % 3], crease[i + j]);
									if(del != TRUE)
									{
										id = udg_find_empty_slot_polygon();
										udg_polygon_set(id, new_vertex[0], ref[i + (j + 1) % 3], new_vertex[1], -1);
										if(crease != NULL)
											udg_crease_set(id, crease[i + j], crease[i + (j + 1) % 3], 0, 0);
									}
									break;
								}
								if(desition[j] == SVP_OUTSIDE && desition[(j + 1) % 3] == SVP_INSIDE && desition[(j + 2) % 3] == SVP_OUTSIDE)
								{
									new_vertex[0] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + j], ref[i + ((j + 1) % 3)]);
									new_vertex[1] = create_new_split_vertex(&edge, &edge_length, &edge_alloc, pos, vector, ref[i + ((j + 1) % 3)], ref[i + ((j + 2) % 3)]);
									udg_polygon_set(i / 4, new_vertex[0], ref[i + (j + 1) % 3], new_vertex[1], -1);
									if(crease != NULL)
										udg_crease_set(i / 4, crease[i + j], crease[i + (j + 1) % 3], 0, 0);
									if(del != TRUE)
									{
										id = udg_find_empty_slot_polygon();
										udg_polygon_set(id, new_vertex[0], new_vertex[1], ref[i + (j + 2) % 3], ref[i + j]);
										if(crease != NULL)
											udg_crease_set(id, 0, crease[i + (j + 1) % 3], crease[i + (j + 2) % 3], crease[i + j]);
									}
									break;
								}
							}
						}
					}else if(del)
						udg_polygon_delete(i / 4);
				}
			}
		}
	}
	free(edge);
}
