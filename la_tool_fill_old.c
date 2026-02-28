#ifdef  DEPRECATED




#include "la_includes.h"
#include "la_geometry_undo.h"

uint la_t_edge_fill_find_edge_poly(uint edge_a, uint edge_b, boolean single)
{
	uint32 vertex_count, polygon_count, *ref, i, j, edge_count, output = -1;
	double *vertex;
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, NULL);
	for(i = 0; i < polygon_count * 4; i += 4)
	{
		if(ref[i + 0] < vertex_count && vertex[ref[i + 0] * 3] != E_REAL_MAX &&
			   ref[i + 1] < vertex_count && vertex[ref[i + 1] * 3] != E_REAL_MAX &&
			   ref[i + 2] < vertex_count && vertex[ref[i + 2] * 3] != E_REAL_MAX)	
		{
		     if(ref[i + 3] >= vertex_count || vertex[ref[i + 3] * 3] == E_REAL_MAX)
				 edge_count = 3;
			 else
				 edge_count = 4; 
			 for(j = 0; j < edge_count; j++)
			 {
				 if((ref[i + j] == edge_a && ref[i + (j + 1) % edge_count] == edge_b) ||
					 (ref[i + j] == edge_b && ref[i + (j + 1) % edge_count] == edge_a))					 
				 {
					 if(!single)
						 return i + j;
					 else
					 {
						 if(output == -1)
							 output =  i + j;
						 else
							 return -1;
					 }
				 }
			 }
		}
	}
	return output;
}

extern uint *la_compute_neighbor(uint *ref, uint ref_count, uint vertex_count, egreal *vertex);


uint la_t_edge_edge_next_open(uint edge, uint *n, uint *ref, uint vertex_count, boolean side)
{
	while(TRUE)
	{
		if(side)
		{
			if(ref[(edge / 4) * 4 + 3] >= vertex_count)
				edge = (edge / 4) * 4 + ((edge % 4) + 1) % 3;
			else
				edge = (edge / 4) * 4 + ((edge % 4) + 1) % 4;
		}else
		{
			if(ref[(edge / 4) * 4 + 3] >= vertex_count)
				edge = (edge / 4) * 4 + ((edge % 4) + 2) % 3;
			else
				edge = (edge / 4) * 4 + ((edge % 4) + 3) % 4;
		}
		if(n[edge] == -1)
			return edge;
		edge = n[edge];
	}
}


void la_t_edge_fill_strip(uint edge_a, uint edge_b)
{
	uint i, j, a, b, start, v_list_a[2], v_list_b[2];
	uint32 vertex_count, ref_count, *n, *ref, *crease, edge;
	double *vertex, dist[3], sum_vec[3], bese_vec[3], d;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);
	n = la_compute_neighbor(ref, ref_count, vertex_count, vertex);

	a = b = start = la_t_edge_fill_find_edge_poly(edge_a, edge_b, TRUE);
	if(a == -1)
		return;
	v_list_a[0] = ref[a];
	if(ref[(a / 4) * 4 + 3] >= vertex_count)
		v_list_b[0] = ref[(a / 4) * 4 + (a + 1) % 3];
	else
		v_list_b[0] = ref[(a / 4) * 4 + (a + 1) % 4];

	if(n[a] == -1)
	{

		a = la_t_edge_edge_next_open(a, n, ref, vertex_count, FALSE);
		v_list_a[1] = ref[a];
		b = la_t_edge_edge_next_open(b, n, ref, vertex_count, TRUE);
		if(ref[(b / 4) * 4 + 3] >= vertex_count)
			v_list_b[1] = ref[(b / 4) * 4 + (b + 1) % 3];
		else
			v_list_b[1] = ref[(b / 4) * 4 + (b + 1) % 4];

		while(a != b)
		{

		r_primitive_line_3d(vertex[v_list_a[0] * 3 + 0], vertex[v_list_a[0] * 3 + 1], vertex[v_list_a[0] * 3 + 2],
							vertex[v_list_a[1] * 3 + 0], vertex[v_list_a[1] * 3 + 1], vertex[v_list_a[1] * 3 + 2], 0, 1, 0, 1);
		r_primitive_line_3d(vertex[v_list_b[0] * 3 + 0], vertex[v_list_b[0] * 3 + 1], vertex[v_list_b[0] * 3 + 2],
							vertex[v_list_b[1] * 3 + 0], vertex[v_list_b[1] * 3 + 1], vertex[v_list_b[1] * 3 + 2], 1, 0, 0, 1);

			bese_vec[0] = vertex[v_list_a[0] * 3] - vertex[v_list_b[0] * 3];
			bese_vec[1] = vertex[v_list_a[0] * 3 + 1] - vertex[v_list_b[0] * 3 + 1];
			bese_vec[2] = vertex[v_list_a[0] * 3 + 2] - vertex[v_list_b[0] * 3 + 2];
			f_normalize3d(bese_vec);
			
			sum_vec[0] = vertex[v_list_a[1] * 3] - vertex[v_list_a[0] * 3] + vertex[v_list_b[1] * 3] - vertex[v_list_b[0] * 3];
			sum_vec[1] = vertex[v_list_a[1] * 3 + 1] - vertex[v_list_a[0] * 3 + 1] + vertex[v_list_b[1] * 3 + 1] - vertex[v_list_b[0] * 3 + 1];
			sum_vec[2] = vertex[v_list_a[1] * 3 + 2] - vertex[v_list_a[0] * 3 + 2] + vertex[v_list_b[1] * 3 + 2] - vertex[v_list_b[0] * 3 + 2];
			f_normalize3d(sum_vec);
			d = bese_vec[0] * sum_vec[0] + bese_vec[1] * sum_vec[1] + bese_vec[2] * sum_vec[2];
			if(d > -0.2 && d < 0.2) /* QUAD */
			{
				udg_polygon_set(udg_find_empty_slot_polygon(), v_list_a[0], v_list_a[1], v_list_b[1], v_list_b[0]);
				r_primitive_line_3d(vertex[v_list_a[1] * 3 + 0], vertex[v_list_a[1] * 3 + 1], vertex[v_list_a[1] * 3 + 2],
									vertex[v_list_b[1] * 3 + 0], vertex[v_list_b[1] * 3 + 1], vertex[v_list_b[1] * 3 + 2], 1, 1, 1, 1);
				v_list_a[0] = v_list_a[1];
				a = la_t_edge_edge_next_open(a, n, ref, vertex_count, FALSE);
				v_list_a[1] = ref[a];
				
				v_list_b[0] = v_list_b[1];
				b = la_t_edge_edge_next_open(b, n, ref, vertex_count, TRUE);
				if(ref[(b / 4) * 4 + 3] >= vertex_count)
					v_list_b[1] = ref[(b / 4) * 4 + ((b % 4) + 1) % 3];
				else
					v_list_b[1] = ref[(b / 4) * 4 + (b + 1) % 4];
			}else
			{
				dist[1] = f_distance3d(&vertex[v_list_a[1] * 3], &vertex[v_list_b[0] * 3]);
				dist[2] = f_distance3d(&vertex[v_list_a[0] * 3], &vertex[v_list_b[1] * 3]);
				if(dist[1] < dist[2]) /* TRIS */
				{				
					udg_polygon_set(udg_find_empty_slot_polygon(), v_list_a[0], v_list_a[1], v_list_b[0], -1);
					r_primitive_line_3d(vertex[v_list_a[1] * 3 + 0], vertex[v_list_a[1] * 3 + 1], vertex[v_list_a[1] * 3 + 2],
										vertex[v_list_b[0] * 3 + 0], vertex[v_list_b[0] * 3 + 1], vertex[v_list_b[0] * 3 + 2], 1, 1, 1, 1);
					v_list_a[0] = v_list_a[1];
					a = la_t_edge_edge_next_open(a, n, ref, vertex_count, FALSE);
					v_list_a[1] = ref[a];
				
				}else
				{
					
					udg_polygon_set(udg_find_empty_slot_polygon(), v_list_a[0], v_list_b[1], v_list_b[0], -1);
					r_primitive_line_3d(vertex[v_list_a[0] * 3 + 0], vertex[v_list_a[0] * 3 + 1], vertex[v_list_a[0] * 3 + 2],
										vertex[v_list_b[1] * 3 + 0], vertex[v_list_b[1] * 3 + 1], vertex[v_list_b[1] * 3 + 2], 1, 1, 1, 1);
					v_list_b[0] = v_list_b[1];
					b = la_t_edge_edge_next_open(b, n, ref, vertex_count, TRUE);
					if(ref[(b / 4) * 4 + 3] >= vertex_count)
						v_list_b[1] = ref[(b / 4) * 4 + ((b % 4) + 1) % 3];
					else
						v_list_b[1] = ref[(b / 4) * 4 + (b + 1) % 4];
				}
			}
		}
	}
//	r_primitive_line_flush();
}

void la_t_edge_compute_normal(double *vertex, double *normal, uint vertex_a, uint vertex_b, uint vertex_c)
{
	f_surface_cross3d(normal, &vertex[vertex_a * 3], &vertex[vertex_c * 3], &vertex[vertex_b * 3]);
	f_normalize3d(normal);
}


boolean la_t_edge_select(uint edge_a, uint edge_b)
{
	uint i, j, start, *list = NULL, list_count = 0;
	uint32 vertex_count, ref_count, *n, *ref, *crease, edge;
	double *vertex, dist[3];
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);
	n = la_compute_neighbor(ref, ref_count, vertex_count, vertex);

	start = i = la_t_edge_fill_find_edge_poly(edge_a, edge_b, TRUE);
	if(i == -1)
		return FALSE;
	while(TRUE)
	{
		if(list_count % 64 == 0)
		{
			list = realloc(list, (sizeof *list) * (list_count + 64)); 
		}
		i = la_t_edge_edge_next_open(i, n, ref, vertex_count, TRUE);
		list[list_count] = ref[i];
		for(j = 0; j < list_count && list[j] != list[list_count]; j++);
		if(j < list_count)
			list_count = j + 1;
		else
			list_count++;
		if(i == start)
			break;
	}
	if(list == NULL)
		return FALSE;
	for(i = 0; i < list_count; i++)
		udg_set_select(list[i], 1.0);
	free(list);
	return TRUE;
}



uint *debug_list = NULL;
uint debug_list_count = 0;
double *debug_edge_normals = NULL; 
extern RMatrix la_world_matrix;

boolean lo_t_hole_ray_test(double *orig, double *dir, double *vert0, double *vert1, double *vert2)
{
	double vec[3], t, u, v, normal[3];
	boolean test;
	vec[0] = dir[0] - orig[0];
	vec[1] = dir[1] - orig[1];
	vec[2] = dir[2] - orig[2];
	if(f_raycast_trid(orig, vec, vert0, vert1, vert2, &t, &u, &v))
	{
		f_normal3d(normal, vert0, vert1, vert2);
		if((0 > normal[0] * (orig[0] - vert0[0]) + normal[1] * (orig[1] - vert0[1]) + normal[2] * (orig[2] - vert0[2])) != 
			(0 > normal[0] * (dir[0] - vert0[0]) + normal[1] * (dir[1] - vert0[1]) + normal[2] * (dir[2] - vert0[2])))
			return TRUE;
	}
	return FALSE;
}

boolean lo_t_hole_colission_test(uint vertex_a, uint vertex_b)
{
	double *vertex, vec[3], t, u, v;
	uint32 vertex_count, polygon_count, *ref, *crease, i;
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, &crease);
	
	vec[0] = vertex[ref[vertex_b] * 3] - vertex[ref[vertex_a] * 3];
	vec[1] = vertex[ref[vertex_b] * 3 + 1] - vertex[ref[vertex_a] * 3 + 1];
	vec[2] = vertex[ref[vertex_b] * 3 + 2] - vertex[ref[vertex_a] * 3 + 2];
	for(i = 0; i < polygon_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
		{
			if(vertex_a != ref[i] && vertex_a != ref[i + 1] && vertex_a != ref[i + 2] && vertex_b != ref[i] && vertex_b != ref[i + 1] && vertex_b != ref[i + 2] && 
				lo_t_hole_ray_test(&vertex[vertex_a * 3], &vertex[vertex_b * 3], &vertex[ref[i] * 3], &vertex[ref[i + 1] * 3], &vertex[ref[i + 2] * 3]))
				return TRUE;
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				if(vertex_a != ref[i] && vertex_a != ref[i + 2] && vertex_a != ref[i + 3] && vertex_b != ref[i] && vertex_b != ref[i + 2] && vertex_b != ref[i + 3] && 
					lo_t_hole_ray_test(&vertex[vertex_a * 3], &vertex[vertex_b * 3], &vertex[ref[i] * 3], &vertex[ref[i + 2] * 3], &vertex[ref[i + 3] * 3]))
					return TRUE;
		}
	}
	return FALSE;
}

double lo_t_hole_value(double *vertex, uint *list, uint list_count, double *edge_normals, uint entry)
{
	double surface_normal[3], output, d, vec_a[3], vec_b[3], vec_c[3];
	uint nextnext, next;
	next = (entry + 1) % list_count;
	nextnext = (entry + 2) % list_count;
	la_t_edge_compute_normal(vertex, surface_normal, list[entry], list[(entry + 2) % list_count], list[(entry + 1) % list_count]);

	/* good make surfaces continius with naighbours.*/

	output = (surface_normal[0] * edge_normals[entry * 3] + surface_normal[1] * edge_normals[entry * 3 + 1] + surface_normal[2] * edge_normals[entry * 3 + 2]) * 0.5 + 0.5;
	output *= (surface_normal[0] * edge_normals[next * 3] + surface_normal[1] * edge_normals[next * 3 + 1] + surface_normal[2] * edge_normals[next * 3 + 2]) * 0.5 + 0.5;
	
	/* good aspect ratio.*/

	vec_a[0] = vertex[list[next] * 3 + 0] - vertex[list[entry] * 3 + 0];
	vec_a[1] = vertex[list[next] * 3 + 1] - vertex[list[entry] * 3 + 1];
	vec_a[2] = vertex[list[next] * 3 + 2] - vertex[list[entry] * 3 + 2];
	f_normalize3d(vec_a);
	vec_b[0] = vertex[list[nextnext] * 3 + 0] - vertex[list[next] * 3 + 0];
	vec_b[1] = vertex[list[nextnext] * 3 + 1] - vertex[list[next] * 3 + 1];
	vec_b[2] = vertex[list[nextnext] * 3 + 2] - vertex[list[next] * 3 + 2];
	f_normalize3d(vec_b);
	d = vec_a[0] * vec_b[0] + vec_a[1] * vec_b[1] + vec_a[2] * vec_b[2];
	if(d < 0)
		d = -d;
	output *= 1.0 - d * d * d;

	d = (vec_b[0] * edge_normals[entry * 3] + vec_b[1] * edge_normals[entry * 3 + 1] + vec_b[2] * edge_normals[entry * 3 + 2]);
	if(d > 0.1)
		output *= 0.1;
		
	d = (vec_a[0] * edge_normals[next * 3] + vec_a[1] * edge_normals[next * 3 + 1] + vec_a[2] * edge_normals[next * 3 + 2]);
	if(d < -0.1)
		output *= 0.1;
	
	/* shorter is better.*/
//	output = 1.0;

	vec_c[0] = vertex[list[nextnext] * 3 + 0] - vertex[list[entry] * 3 + 0];
	vec_c[1] = vertex[list[nextnext] * 3 + 1] - vertex[list[entry] * 3 + 1];
	vec_c[2] = vertex[list[nextnext] * 3 + 2] - vertex[list[entry] * 3 + 2];
	d = f_length3d(vec_c);
	output /= d;

	if(lo_t_hole_colission_test(list[entry], list[nextnext]))
		output *= 0.1; 
	return output;
}



void lo_t_hole_fill(double *vertex, uint *list, uint list_count, double *edge_normals)
{
	RMatrix *reset;
	double best, move = 0, normal[3], *weight;
	uint i, j, found = 0, test = 0;
	BInputState *input;
	uint k = 0;
	input = betray_get_input_state();
	weight = malloc((sizeof *weight) * list_count);
	for(i = 0; i < list_count; i++)
		weight[i] = lo_t_hole_value(vertex, list, list_count, edge_normals, i);

	while(list_count > 2/* && k++ < (uint)((input->pointers[0].pointer_x + 1.0) * 100.0)*/)
	{
		found = 0;
		for(i = 1; i < list_count; i++)
			if( weight[i] > weight[found])
				found = i;

		udg_polygon_set(udg_find_empty_slot_polygon(), list[found], list[(found + 2) % list_count], list[(found + 1) % list_count], -1);

		move += 0.0;

		la_t_edge_compute_normal(vertex, normal, list[(found + 0) % list_count], list[(found + 2) % list_count], list[(found + 1) % list_count]);
		i = (found + 1) % list_count--;
		for(i = (found + 1) % (list_count + 1); i < list_count; i++)
		{
			list[i] = list[i + 1];
			weight[i] = weight[i + 1];
			edge_normals[i * 3 + 0] = edge_normals[i * 3 + 3];
			edge_normals[i * 3 + 1] = edge_normals[i * 3 + 4];
			edge_normals[i * 3 + 2] = edge_normals[i * 3 + 5];
		}
		found = found % list_count;
		edge_normals[found * 3 + 0] = normal[0];
		edge_normals[found * 3 + 1] = normal[1];
		edge_normals[found * 3 + 2] = normal[2];
		weight[found] = lo_t_hole_value(vertex, list, list_count, edge_normals, found);
		i = (found + list_count - 1) % list_count;
		weight[i] = lo_t_hole_value(vertex, list, list_count, edge_normals, i);
		i = (found + list_count - 2) % list_count;
		weight[i] = lo_t_hole_value(vertex, list, list_count, edge_normals, i);
		for(i = 0; i < list_count; i++)
		{
			best = lo_t_hole_value(vertex, list, list_count, edge_normals, i);
			if(weight[i] > best * 1.01 || weight[i] < best / 1.01)
				j = 0;
		}
	}
/*	for(i = 0; i < list_count; i++)
	{
		r_primitive_line_3d(vertex[list[i] * 3 + 0],
							vertex[list[i] * 3 + 1],
							vertex[list[i] * 3 + 2] + move,
							vertex[list[(i + 1) % list_count] * 3 + 0],
							vertex[list[(i + 1) % list_count] * 3 + 1],
							vertex[list[(i + 1) % list_count] * 3 + 2] + move, 1, 1, 1, 1);
		r_primitive_line_3d((vertex[list[i] * 3 + 0] + vertex[list[(i + 1) % list_count] * 3 + 0]) * 0.5,
							(vertex[list[i] * 3 + 1] + vertex[list[(i + 1) % list_count] * 3 + 1]) * 0.5,
							(vertex[list[i] * 3 + 2] + vertex[list[(i + 1) % list_count] * 3 + 2]) * 0.5,
							(vertex[list[i] * 3 + 0] + vertex[list[(i + 1) % list_count] * 3 + 0]) * 0.5 + edge_normals[i * 3 + 0] * 0.1,
							(vertex[list[i] * 3 + 1] + vertex[list[(i + 1) % list_count] * 3 + 1]) * 0.5 + edge_normals[i * 3 + 1] * 0.1,
							(vertex[list[i] * 3 + 2] + vertex[list[(i + 1) % list_count] * 3 + 2]) * 0.5 + edge_normals[i * 3 + 2] * 0.1, 1, 1, 1, 1);
	}*/
	free(weight);
}

void lo_t_hole_debug()
{
	RMatrix *reset;
	double *vertex;
	uint *list;
	double *edge_normals;
	uint i;
	if(debug_list == NULL)
		return;
	reset = r_matrix_get();
	r_matrix_set(&la_world_matrix);
	udg_get_geometry(NULL, NULL, &vertex, NULL, NULL);
/*	for(i = 0; i < debug_list_count; i++)
	{
		r_primitive_line_3d(vertex[debug_list[i] * 3 + 0],
							vertex[debug_list[i] * 3 + 1],
							vertex[debug_list[i] * 3 + 2],
							vertex[debug_list[(i + 1) % debug_list_count] * 3 + 0],
							vertex[debug_list[(i + 1) % debug_list_count] * 3 + 1],
							vertex[debug_list[(i + 1) % debug_list_count] * 3 + 2], 1, 1, 1, 1);
		r_primitive_line_3d((vertex[debug_list[i] * 3 + 0] + vertex[debug_list[(i + 1) % debug_list_count] * 3 + 0]) * 0.5,
							(vertex[debug_list[i] * 3 + 1] + vertex[debug_list[(i + 1) % debug_list_count] * 3 + 1]) * 0.5,
							(vertex[debug_list[i] * 3 + 2] + vertex[debug_list[(i + 1) % debug_list_count] * 3 + 2]) * 0.5,

							(vertex[debug_list[i] * 3 + 0] + vertex[debug_list[(i + 1) % debug_list_count] * 3 + 0]) * 0.5 + 0.1 * debug_edge_normals[i * 3 + 0],
							(vertex[debug_list[i] * 3 + 1] + vertex[debug_list[(i + 1) % debug_list_count] * 3 + 1]) * 0.5 + 0.1 * debug_edge_normals[i * 3 + 1],
							(vertex[debug_list[i] * 3 + 2] + vertex[debug_list[(i + 1) % debug_list_count] * 3 + 2]) * 0.5 + 0.1 * debug_edge_normals[i * 3 + 2], 1, 1, 1, 1);
	}*/
	
	list = malloc((sizeof *list) * debug_list_count);
	for(i = 0; i < debug_list_count; i++)
		list[i] = debug_list[i];
	edge_normals = malloc((sizeof *edge_normals) * debug_list_count * 3);
	for(i = 0; i < debug_list_count * 3; i++)
		edge_normals[i] = debug_edge_normals[i];
	lo_t_hole_fill(vertex, list, debug_list_count, edge_normals);
	free(list);
	free(edge_normals);

	r_primitive_line_flush();
	r_matrix_set(reset);
}

void la_t_edge_fill_hole(uint edge_a, uint edge_b)
{
	uint i, j, start, *list = NULL, list_count = 0;
	uint32 vertex_count, ref_count, *n, *ref, *crease, edge;
	double *vertex, dist[3], *edge_normals = NULL;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);
	n = la_compute_neighbor(ref, ref_count, vertex_count, vertex);

	start = i = la_t_edge_fill_find_edge_poly(edge_a, edge_b, TRUE);
	if(i == -1)
		return;
	while(TRUE)
	{
		if(list_count % 64 == 0)
		{
			list = realloc(list, (sizeof *list) * (list_count + 64)); 
			edge_normals = realloc(edge_normals, (sizeof *edge_normals) * (list_count + 64) *  3); 
		}
		i = la_t_edge_edge_next_open(i, n, ref, vertex_count, TRUE);
		list[list_count] = ref[i];
		la_t_edge_compute_normal(vertex, &edge_normals[list_count * 3], ref[(i / 4) * 4 + 0], 
																		ref[(i / 4) * 4 + 1],
																		ref[(i / 4) * 4 + 2]);
		for(j = 0; j < list_count && list[j] != list[list_count]; j++);
		if(j < list_count)
			list_count = j + 1;
		else
			list_count++;
		if(i == start)
			break;
	}
	if(list == NULL)
		return;
	lo_t_hole_fill(vertex, list, list_count, edge_normals);
	free(list);
	free(edge_normals); 
//	r_primitive_line_flush();
}



/*

void la_t_edge_fill_vertex_remove(uint vertex)
{
	uint i, j, a, b, start, v_list_a[2], v_list_b[2];
	uint32 vertex_count, ref_count, *n, *ref, *crease, edge;
	double *vertex, dist[3], sum_vec[3], bese_vec[3], d;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);
	n = la_compute_neighbor(ref, ref_count, vertex_count, vertex);

	a = b = start = la_t_edge_fill_find_edge_poly(edge_a, edge_b);

	v_list_a[0] = ref[a];
	if(ref[(a / 4) * 4 + 3] >= vertex_count)
		v_list_b[0] = ref[(a / 4) * 4 + (a + 1) % 3];
	else
		v_list_b[0] = ref[(a / 4) * 4 + (a + 1) % 4];

	if(n[a] == -1)
	{

		a = la_t_edge_edge_next_open(a, n, ref, vertex_count, FALSE);
		v_list_a[1] = ref[a];
		b = la_t_edge_edge_next_open(b, n, ref, vertex_count, TRUE);
		if(ref[(b / 4) * 4 + 3] >= vertex_count)
			v_list_b[1] = ref[(b / 4) * 4 + (b + 1) % 3];
		else
			v_list_b[1] = ref[(b / 4) * 4 + (b + 1) % 4];

		while(a != b)
		{

		r_primitive_line_3d(vertex[v_list_a[0] * 3 + 0], vertex[v_list_a[0] * 3 + 1], vertex[v_list_a[0] * 3 + 2],
							vertex[v_list_a[1] * 3 + 0], vertex[v_list_a[1] * 3 + 1], vertex[v_list_a[1] * 3 + 2], 0, 1, 0, 1);
		r_primitive_line_3d(vertex[v_list_b[0] * 3 + 0], vertex[v_list_b[0] * 3 + 1], vertex[v_list_b[0] * 3 + 2],
							vertex[v_list_b[1] * 3 + 0], vertex[v_list_b[1] * 3 + 1], vertex[v_list_b[1] * 3 + 2], 1, 0, 0, 1);

			bese_vec[0] = vertex[v_list_a[0] * 3] - vertex[v_list_b[0] * 3];
			bese_vec[1] = vertex[v_list_a[0] * 3 + 1] - vertex[v_list_b[0] * 3 + 1];
			bese_vec[2] = vertex[v_list_a[0] * 3 + 2] - vertex[v_list_b[0] * 3 + 2];
			f_normalize3d(bese_vec);
			
			sum_vec[0] = vertex[v_list_a[1] * 3] - vertex[v_list_a[0] * 3] + vertex[v_list_b[1] * 3] - vertex[v_list_b[0] * 3];
			sum_vec[1] = vertex[v_list_a[1] * 3 + 1] - vertex[v_list_a[0] * 3 + 1] + vertex[v_list_b[1] * 3 + 1] - vertex[v_list_b[0] * 3 + 1];
			sum_vec[2] = vertex[v_list_a[1] * 3 + 2] - vertex[v_list_a[0] * 3 + 2] + vertex[v_list_b[1] * 3 + 2] - vertex[v_list_b[0] * 3 + 2];
			f_normalize3d(sum_vec);
			d = bese_vec[0] * sum_vec[0] + bese_vec[1] * sum_vec[1] + bese_vec[2] * sum_vec[2];
			if(d > -0.2 && d < 0.2)
			{
				udg_polygon_set(udg_find_empty_slot_polygon(), v_list_a[0], v_list_a[1], v_list_b[1], v_list_b[0]);
				r_primitive_line_3d(vertex[v_list_a[1] * 3 + 0], vertex[v_list_a[1] * 3 + 1], vertex[v_list_a[1] * 3 + 2],
									vertex[v_list_b[1] * 3 + 0], vertex[v_list_b[1] * 3 + 1], vertex[v_list_b[1] * 3 + 2], 1, 1, 1, 1);
				v_list_a[0] = v_list_a[1];
				a = la_t_edge_edge_next_open(a, n, ref, vertex_count, FALSE);
				v_list_a[1] = ref[a];
				
				v_list_b[0] = v_list_b[1];
				b = la_t_edge_edge_next_open(b, n, ref, vertex_count, TRUE);
				if(ref[(b / 4) * 4 + 3] >= vertex_count)
					v_list_b[1] = ref[(b / 4) * 4 + ((b % 4) + 1) % 3];
				else
					v_list_b[1] = ref[(b / 4) * 4 + (b + 1) % 4];
			}else
			{
				dist[1] = f_distance3d(&vertex[v_list_a[1] * 3], &vertex[v_list_b[0] * 3]);
				dist[2] = f_distance3d(&vertex[v_list_a[0] * 3], &vertex[v_list_b[1] * 3]);
				if(dist[1] < dist[2]) 
				{				
					udg_polygon_set(udg_find_empty_slot_polygon(), v_list_a[0], v_list_a[1], v_list_b[0], -1);
					r_primitive_line_3d(vertex[v_list_a[1] * 3 + 0], vertex[v_list_a[1] * 3 + 1], vertex[v_list_a[1] * 3 + 2],
										vertex[v_list_b[0] * 3 + 0], vertex[v_list_b[0] * 3 + 1], vertex[v_list_b[0] * 3 + 2], 1, 1, 1, 1);
					v_list_a[0] = v_list_a[1];
					a = la_t_edge_edge_next_open(a, n, ref, vertex_count, FALSE);
					v_list_a[1] = ref[a];
				
				}else
				{
					
					udg_polygon_set(udg_find_empty_slot_polygon(), v_list_a[0], v_list_b[1], v_list_b[0], -1);
					r_primitive_line_3d(vertex[v_list_a[0] * 3 + 0], vertex[v_list_a[0] * 3 + 1], vertex[v_list_a[0] * 3 + 2],
										vertex[v_list_b[1] * 3 + 0], vertex[v_list_b[1] * 3 + 1], vertex[v_list_b[1] * 3 + 2], 1, 1, 1, 1);
					v_list_b[0] = v_list_b[1];
					b = la_t_edge_edge_next_open(b, n, ref, vertex_count, TRUE);
					if(ref[(b / 4) * 4 + 3] >= vertex_count)
						v_list_b[1] = ref[(b / 4) * 4 + ((b % 4) + 1) % 3];
					else
						v_list_b[1] = ref[(b / 4) * 4 + (b + 1) % 4];
				}
			}
		}
	}
//	r_primitive_line_flush();
}*/

#endif //  DEPRECATED