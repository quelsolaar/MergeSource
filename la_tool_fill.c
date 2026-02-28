#include "la_includes.h"
#include "la_geometry_undo.h"

extern uint *la_compute_neighbor(uint *ref, uint ref_count, uint vertex_count, egreal *vertex);

typedef struct{
	uint vertex_id;
	double normal[3];
	uint32 crease;
	double value;
	void *next;
}LAEdgeRing;

uint la_t_edge_fill_find_edge_poly(uint edge_a, uint edge_b, boolean single)
{
	uint i, j, poly, *neighbor, start = -1;
    uint32 *ref, vertex_count, polygon_count;
	double *vertex;
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, NULL);
	for(i = 0; i < polygon_count * 4 ; i += 4)
	{
		if(ref[i] < vertex_count && 
			vertex[ref[i] * 3] != V_REAL64_MAX &&
			ref[i + 1] < vertex_count && 
			vertex[ref[i + 1] * 3] != V_REAL64_MAX &&
			ref[i + 2] < vertex_count &&
			vertex[ref[i + 2] * 3] != V_REAL64_MAX)		
		{
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != V_REAL64_MAX)
				poly = 4;
			else
				poly = 3;
			for(j = 0; j < poly; j++)
			{
				if((ref[i + j] == edge_a && ref[i + (j + 1) % poly] == edge_b) ||
					(ref[i + j] == edge_b && ref[i + (j + 1) % poly] == edge_a))
				{
					if(single)
					{
						if(start != -1)
							return -1;
						start = i + j;
					}else
						return start;
				}
			}
		}
	}
	return start;
}

LAEdgeRing *la_t_edge_get_list(uint edge_a, uint edge_b, uint *ring_length)
{
	LAEdgeRing *ring;
	uint i, j, poly, *neighbor, start = -1, length, edge;
    uint32 *ref, *crease, vertex_count, polygon_count;
	double *vertex;
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, &crease);
	length = 1;
	start = la_t_edge_fill_find_edge_poly(edge_a, edge_b, TRUE);
	if(start == -1)
		return NULL;
	neighbor = la_compute_neighbor(ref, polygon_count, vertex_count, vertex);
	ring = malloc((sizeof * ring) * (polygon_count * 3 + 1));
	edge = start;
	*ring_length = 0;
	while(TRUE)
	{
		f_normal3d(ring[*ring_length].normal, &vertex[ref[(edge / 4) * 4 + 1] * 3], &vertex[ref[(edge / 4) * 4] * 3], &vertex[ref[(edge / 4) * 4 + 2] * 3]);	
		ring[*ring_length].vertex_id = ref[edge];
		ring[*ring_length].crease = crease[edge];
		(*ring_length)++;
		while(neighbor[edge] != -1)
		{
			edge = neighbor[edge];
			if(ref[(edge / 4) * 4 + 3] < vertex_count && vertex[ref[(edge / 4) * 4 + 3] * 3] != V_REAL64_MAX)
				edge = (edge / 4) * 4 + (edge + 1) % 4;
			else
				edge = (edge / 4) * 4 + ((edge % 4) + 1) % 3;
		}
		if(edge == start && (*ring_length) > 1)
			break;
		if(ref[(edge / 4) * 4 + 3] < vertex_count && vertex[ref[(edge / 4) * 4 + 3] * 3] != V_REAL64_MAX)
			edge = (edge / 4) * 4 + (edge + 1) % 4;
		else
			edge = (edge / 4) * 4 + ((edge % 4) + 1) % 3;
	}
	(*ring_length)--;
	free(neighbor);
	return ring;
}


LAEdgeRing *my_debug_ring = NULL;
uint my_debug_ring_length = 0;


void lo_t_hole_debug()
{
	double *vertex;
	uint32 vertex_count;
	uint i;
	udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);
	for(i = 0; i < my_debug_ring_length; i++)
	{
		printf("value %f\n", (float)my_debug_ring[i].value);
	if(my_debug_ring[i].vertex_id < vertex_count && 
		my_debug_ring[(i + 1) % my_debug_ring_length].vertex_id < vertex_count)
		r_primitive_line_3d(vertex[my_debug_ring[i].vertex_id * 3 + 0],
							vertex[my_debug_ring[i].vertex_id * 3 + 1],
							vertex[my_debug_ring[i].vertex_id * 3 + 2],
							vertex[my_debug_ring[(i + 1) % my_debug_ring_length].vertex_id * 3 + 0],
							vertex[my_debug_ring[(i + 1) % my_debug_ring_length].vertex_id * 3 + 1],
							vertex[my_debug_ring[(i + 1) % my_debug_ring_length].vertex_id * 3 + 2] + 0.01, my_debug_ring[i].value, 1, 0, 1);
	}
	r_primitive_line_flush();
}

double la_t_edge_fill_polygon_evaluate(LAEdgeRing *ring, double *vertex)
{
	double *v0, *v1, *v2, vec0[3], vec1[3], vec2[3], f, best, *n1, *n2, normal[3], normal_factor;
	LAEdgeRing *r1, *r2;
	r1 = ring;
	v0 = &vertex[ring->vertex_id * 3];
	n1 = ring->normal;
	ring = r2 = ring->next;
	v1 = &vertex[ring->vertex_id * 3];
	n2 = ring->normal;
	ring = ring->next;
	v2 = &vertex[ring->vertex_id * 3];
	f_normal3d(normal, v0, v1, v2);	
	normal_factor = normal[0] * ring->normal[0] + normal[1] * ring->normal[1] + normal[2] * ring->normal[2];
	normal_factor += normal[0] * r2->normal[0] + normal[1] * r2->normal[1] + normal[2] * r2->normal[2];
	vec0[0] = v0[0] - v1[0];
	vec0[1] = v0[1] - v1[1];
	vec0[2] = v0[2] - v1[2];
	f_normalize3d(vec0);
	vec1[0] = v1[0] - v2[0];
	vec1[1] = v1[1] - v2[1];
	vec1[2] = v1[2] - v2[2];
	f_normalize3d(vec1);
	vec2[0] = v2[0] - v0[0];
	vec2[1] = v2[1] - v0[1];
	vec2[2] = v2[2] - v0[2];
	f_normalize3d(vec2);

	best = vec0[0] * vec1[0] + vec0[1] * vec1[1] + vec0[2] * vec1[2];
	f = vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
	if(f > best)
		best = f;
	f = vec2[0] * vec0[0] + vec2[1] * vec0[1] + vec2[2] * vec0[2];
	if(f > best)
		best = f;
	return -best * 5.0 + normal_factor;
//extern void 
}

void la_t_edge_fill_hole(uint edge_a, uint edge_b)
{
	LAEdgeRing *ring = NULL, *r, *prev, *found, *found_prev;
	uint i, j, center_id, poly_id, ring_length;
	double center[3] = {0, 0, 0}, *v, *vertex;
	float best;
    ring = la_t_edge_get_list(edge_a, edge_b, &ring_length);
	if(ring == NULL)
		return;
	udg_get_geometry(NULL, NULL, &vertex, NULL, NULL);
	for(i = 0; i < ring_length; i++)
		ring[i].next = &ring[(i + 1) % ring_length];

	for(i = 0; i < ring_length; i++)
		ring[i].value = la_t_edge_fill_polygon_evaluate(&ring[i], vertex);
	/*
	my_debug_ring = ring;
	my_debug_ring_length = ring_length;
	return;*/

	r = prev = ring;
	j = 0;
	while(ring_length > 2)
	{
		best = -10000;
		r = prev->next;
		for(i = 0; i < ring_length; i++)
		{
			if(r->value > best)
			{
				found = r;
				found_prev = prev;
				best = r->value;
			}
			r = r->next;
			prev = prev->next;
		}
		r = found->next;

		poly_id = udg_find_empty_slot_polygon();
		udg_polygon_set(poly_id, r->vertex_id, found->vertex_id, ((LAEdgeRing *)r->next)->vertex_id, -1);
		udg_crease_set(poly_id, r->crease, found->crease, 0, 0);
		f_normal3d(found->normal, &vertex[r->vertex_id * 3], &vertex[found->vertex_id * 3], &vertex[((LAEdgeRing *)r->next)->vertex_id * 3]);	
		found->next = r->next;
		found_prev->value = la_t_edge_fill_polygon_evaluate(found_prev, vertex);
		found->value = la_t_edge_fill_polygon_evaluate(found, vertex);
		ring_length--;
		prev = found->next;
//		if(j++ > 1)
//			break;
	}
	free(ring);
}



void la_t_edge_fill_hole_fan(uint edge_a, uint edge_b)
{
	LAEdgeRing *ring = NULL;
	uint i, center_id, poly_id, ring_length;
	double center[3] = {0, 0, 0}, *v, *vertex;
    ring = la_t_edge_get_list(edge_a, edge_b, &ring_length);
	if(ring == NULL)
		return;
	udg_get_geometry(NULL, NULL, &vertex, NULL, NULL);
	for(i = 0; i < ring_length; i++)
	{
		v = &vertex[ring[i].vertex_id * 3];
		center[0] += v[0];  
		center[1] += v[1];  
		center[2] += v[2];  
	}
	center[0] /= (float)ring_length;
	center[1] /= (float)ring_length;
	center[2] /= (float)ring_length;
	center_id = udg_find_empty_slot_vertex();
	udg_vertex_set(center_id, NULL, center[0], center[1], center[2]);

	for(i = 0; i < ring_length; i++)
	{
		poly_id = udg_find_empty_slot_polygon();
		udg_polygon_set(poly_id, ring[(i + 1) % ring_length].vertex_id, ring[i].vertex_id, center_id, -1);
		udg_crease_set(poly_id, 0, ring[i].crease, 0, 0);
	}
	free(ring);
}
