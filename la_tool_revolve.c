#include "la_includes.h"
#include "la_geometry_undo.h"
#include "la_tool.h"

void create_polygon_revolve(uint *a, uint *b, uint revolve)
{
	uint i, id, crease_a = 0, crease_b = 0;
	if(udg_get_select(a[0]) > 0.1)
		crease_a = -1;
	if(udg_get_select(b[0]) > 0.1)
		crease_b = -1;
	for(i = 0; i < revolve; i++)
	{
		id = udg_find_empty_slot_polygon();
		udg_polygon_set(udg_find_empty_slot_polygon(), b[i], a[i], a[(i + 1) % revolve], b[(i + 1) % revolve]);
		udg_crease_set(id, 0, crease_a, 0, crease_b);
	}
}

void fill_vertex_revolve(double *matrix, uint *array, uint vertex_id, uint revolve)
{
	uint i;
	double start[3], vertex[3], space[3], r, angle;
	udg_get_vertex_pos(vertex, vertex_id);
	vertex[0] -= matrix[12];
	vertex[1] -= matrix[13];
	vertex[2] -= matrix[14];

	space[0] = matrix[0] * vertex[0] + matrix[1] * vertex[1] + matrix[2] * vertex[2];
	space[1] = matrix[4] * vertex[0] + matrix[5] * vertex[1] + matrix[6] * vertex[2];
	space[2] = matrix[8] * vertex[0] + matrix[9] * vertex[1] + matrix[10] * vertex[2];

	r = sqrt(space[0] * space[0] + space[1] * space[1]);

	angle = atan2(space[0], space[1]);
	array[0] = vertex_id;
	for(i = 1; i < revolve; i++)
	{
		vertex[0] = sin(angle + (3.14 * 2 * (double)i / (double)revolve)) * r;
		vertex[1] = cos(angle + (3.14 * 2 * (double)i / (double)revolve)) * r;
		vertex[2] = space[2];
		array[i] = udg_find_empty_slot_vertex();
		f_transform3d(vertex, matrix, vertex[0], vertex[1], vertex[2]);
		udg_vertex_set(array[i], NULL, vertex[0], vertex[1], vertex[2]);
	}
}

void create_revolve_matrix(double *matrix, uint a, uint b)
{
	double start[3], end[3], r, value = -1;
	udg_get_vertex_pos(start, a);
	udg_get_vertex_pos(end, b);
	end[0] -= start[0];
	end[1] -= start[1];
	end[2] -= start[2];
	r = (end[0] * end[0] + end[1] * end[1] + end[2] * end[2]);
	if(r < 0)
		value = 1;
	r = sqrt(r);
	matrix[8] = end[0] / r;
	matrix[9] = end[1] / r;
	matrix[10] = end[2] / r;
	matrix[12] = start[0];
	matrix[13] = start[1];
	matrix[14] = start[2];
	start[0] = 0;
	start[1] = 0;
	start[2] = 0;
	
	if(matrix[8] * matrix[8] > matrix[9] * matrix[9])
		start[1] = value;
	else
		start[0] = value;
	matrix[4] = matrix[9] * start[2] - matrix[10] * start[1];
	matrix[5] = matrix[10] * start[0] - matrix[8] * start[2];
	matrix[6] = matrix[8] * start[1] - matrix[9] * start[0];
	r = sqrt(matrix[4] * matrix[4] + matrix[5] * matrix[5] + matrix[6] * matrix[6]);
	matrix[4] = matrix[4] / r;
	matrix[5] = matrix[5] / r;
	matrix[6] = matrix[6] / r;
	matrix[0] = matrix[9] * matrix[6] - matrix[10] * matrix[5];
	matrix[1] = matrix[10] * matrix[4] - matrix[8] * matrix[6];
	matrix[2] = matrix[8] * matrix[5] - matrix[9] * matrix[4];

	r = sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1] + matrix[2] * matrix[2]);
	matrix[0] = matrix[0] / r;
	matrix[1] = matrix[1] / r;
	matrix[2] = matrix[2] / r;
}

uint get_next_edge(uint a, uint b, boolean del)
{
	boolean found = FALSE;
	uint *edge, count, id, i, output = -1;
	edge = udg_get_edge_data(&count);
	count *= 2;
	for(i = 0; i < count; i++)
	{
		if(edge[i] == a && edge[((i / 2) * 2) + ((i + 1) % 2)] != b)
		{
			if(found == TRUE)
				return -1;
			found = TRUE;
			output = edge[((i / 2) * 2) + ((i + 1) % 2)];
			id = i / 2;
		}
	}
	if(del && found)
		udg_destroy_edge(id);
	return output;
}

uint find_end(uint *edge, uint count, uint id, uint skip)
{
	boolean found;
	uint next, next_skip, i;
	count *= 2;
	while(TRUE)
	{
		found = FALSE;
		for(i = 0; i < count; i++)
		{
			if(edge[i] == id && i / 2 != skip)
			{
				if(found == TRUE)
					return id;
				found = TRUE;
				next_skip = i / 2;
				next = edge[((i / 2) * 2) + ((i + 1) % 2)];
			}
		}
		if(found != TRUE)
			return id;
		else
		{
			id = next;
			skip = next_skip;
		}
	}
	return 0;
}

void fill_to_end(double *matrix, uint *a, uint *b, uint start, uint skip, uint end, uint revolve, boolean side)
{
	uint i, *temp, *edge, edge_count, id, crease, crease2;
	edge = udg_get_edge_data(&edge_count);
	for(i = 0; i < edge_count * 2; i++)
		if(edge[i] == start && edge[((i / 2) * 2) + ((i + 1) % 2)] != skip)
			break;
	skip = start;
	start = edge[((i / 2) * 2) + ((i + 1) % 2)];
	while(start != end)
	{
		udg_destroy_edge(i / 2);
		edge = udg_get_edge_data(&edge_count);
		crease = 0;
		crease2 = 0;
		fill_vertex_revolve(matrix, b, start, revolve);
		if(udg_get_select(a[0]) > 0.1)
			crease = -1;
		if(udg_get_select(b[0]) > 0.1)
			crease2 = -1;
		if(side)
		{

			for(i = 0; i < revolve; i++)
			{
				id = udg_find_empty_slot_polygon();
				udg_polygon_set(id, a[i], b[i], b[(i + 1) % revolve], a[(i + 1) % revolve]);
				udg_crease_set(id, 0, crease2, 0, crease);

			}
		}
		else
		{
			for(i = 0; i < revolve; i++)
			{
				id = udg_find_empty_slot_polygon();
				udg_polygon_set(id, a[i], a[(i + 1) % revolve], b[(i + 1) % revolve], b[i]);
				udg_crease_set(id, crease, 0, crease2, 0);
			}
		}
		temp = a;
		a = b;
		b = temp;

		for(i = 0; i < edge_count * 2; i++)
			if(edge[i] == start && edge[((i / 2) * 2) + ((i + 1) % 2)] != skip)
				break;
		skip = start;
		start = edge[((i / 2) * 2) + ((i + 1) % 2)];
	}
	udg_destroy_edge(i / 2);
	crease = 0;
	if(udg_get_select(a[0]) > 0.1)
		crease = -1;
	if(side)
	{
		for(i = 0; i < revolve; i++)
		{
			id = udg_find_empty_slot_polygon();
			udg_polygon_set(id, end, a[(i + 1) % revolve], a[i], -1);
			udg_crease_set(id, 0, crease, 0, 0);
		}
	}
	else
	{
		for(i = 0; i < revolve; i++)
		{
			id = udg_find_empty_slot_polygon();
			udg_polygon_set(id, end, a[i], a[(i + 1) % revolve],  -1);
			udg_crease_set(id, 0, crease, 0, 0);
		}
	}
}

void la_t_revolve(uint *start_edge, uint revolve)
{
	uint *edge, count, i, ends[2], current, last, id, *a, *b, *c, crease, crease2;
	double matrix[16];
	edge = udg_get_edge_data(&count);
	for(i = 0; i < count && (edge[i * 2] != start_edge[0] || edge[i * 2 + 1] != start_edge[1]); i++);
	if(i == count)
        return;
	ends[0] = find_end(edge, count, start_edge[0], i);
	ends[1] = find_end(edge, count, start_edge[1], i);
	if(start_edge[0] == ends[0] && start_edge[0] == ends[1])
		return;
	create_revolve_matrix(matrix, ends[0], ends[1]);
	a = malloc((sizeof *a) * revolve);
	b = malloc((sizeof *b) * revolve);
	c = malloc((sizeof *c) * revolve);
	udg_destroy_edge(i);
	if(start_edge[0] == ends[0])
	{
		fill_vertex_revolve(matrix, a, start_edge[1], revolve);
		if(udg_get_select(a[0]) > 0.1)
			crease = -1;
		else
			crease = 0;
		for(i = 0; i < revolve; i++)
		{
			id = udg_find_empty_slot_polygon();
			udg_polygon_set(id, ends[0], a[i], a[(i + 1) % revolve], -1);
			udg_crease_set(id, 0, crease, 0, 0);
		}
		fill_to_end(matrix, a, b, start_edge[1], start_edge[0], ends[1], revolve, FALSE);
	}
	else if(start_edge[1] == ends[1])
	{
		fill_vertex_revolve(matrix, a, start_edge[0], revolve);
		if(udg_get_select(a[0]) > 0.1)
			crease = -1;
		else
			crease = 0;
		for(i = 0; i < revolve; i++)
		{
			id = udg_find_empty_slot_polygon();
			udg_polygon_set(id, ends[1], a[(i + 1) % revolve], a[i], -1);
			udg_crease_set(id, 0, crease, 0, 0);
		}
		fill_to_end(matrix, a, b, start_edge[0], start_edge[1], ends[0], revolve, FALSE);
	}	
	else
	{
		fill_vertex_revolve(matrix, a, start_edge[0], revolve);
		for(i = 0; i < revolve; i++)
			c[i] = a[i];
		fill_to_end(matrix, a, b, start_edge[0], start_edge[1], ends[0], revolve, FALSE);
		
		fill_vertex_revolve(matrix, a, start_edge[1], revolve);
		if(udg_get_select(a[0]) > 0.1)
			crease = -1;
		else
			crease = 0;
		if(udg_get_select(c[0]) > 0.1)
			crease2 = -1;
		else
			crease2 = 0;
		for(i = 0; i < revolve; i++)
		{
			id = udg_find_empty_slot_polygon();
			udg_polygon_set(id, a[i], a[(i + 1) % revolve], c[(i + 1) % revolve], c[i]);
			udg_crease_set(id, crease, 0, crease2, 0);
		}
		fill_to_end(matrix, a, b, start_edge[1], start_edge[0], ends[1], revolve, TRUE);
	}
	free(a);
	free(b);
	free(c);
}

void fill_vertex_tube(double *matrix, uint *array, uint vertex, uint revolve, double scale, double dir)
{
	double a, b, state[3];
	uint i;
	udg_get_vertex_pos(state, vertex);
	udg_vertex_set(vertex, state, matrix[12] + matrix[4] * scale, matrix[13] + matrix[5] * scale, matrix[14] + matrix[6] * scale);
	array[0] = vertex;
	for(i = 1; i < revolve; i++)
	{	
		a = sin(((double)i / (double)revolve) * 2.0 * 3.14 * dir) * scale;
		b = cos(((double)i / (double)revolve) * 2.0 * 3.14 * dir) * scale;
		array[i] = udg_find_empty_slot_vertex();
		udg_vertex_set(array[i], NULL, 
		matrix[0] * a + matrix[4] * b + matrix[12],
		matrix[1] * a + matrix[5] * b + matrix[13],
		matrix[2] * a + matrix[6] * b + matrix[14]);
	}
}

void create_tube_matrix(double *matrix, double *vector, uint a, uint b, uint c)
{
	double origo[3], point[3];
	udg_get_vertex_pos(origo, a);
	if(c == -1)
	{
		udg_get_vertex_pos(point, b);
	}
	else
	{
		udg_get_vertex_pos(point, c);
	}
	point[0] -= origo[0];
	point[1] -= origo[1];
	point[2] -= origo[2];
	udg_get_vertex_pos(origo, b);
	point[0] += origo[0];
	point[1] += origo[1];
	point[2] += origo[2];
	f_matrixzxd(matrix, origo, point, vector);
}

void la_t_tube(uint *start_edge, uint revolve)
{
	uint i, j, cur[2], next, *a, *b, *c, *temp;
	double matrix[16], scale, vector[] = {1, 3, 2};
	udg_get_vertex_pos(matrix, start_edge[0]);
	udg_get_vertex_pos(&matrix[3], start_edge[1]);
	scale = sqrt((matrix[0] - matrix[3]) * (matrix[0] - matrix[3]) + (matrix[1] - matrix[4]) * (matrix[1] - matrix[4]) + (matrix[2] - matrix[5]) * (matrix[2] - matrix[5]));
	a = malloc((sizeof *a) * revolve);
	b = malloc((sizeof *b) * revolve);
	c = malloc((sizeof *c) * revolve);

	temp = udg_get_edge_data(&j);
	for(i = 0; i < j && (temp[i * 2] != start_edge[0] || temp[i * 2 + 1] != start_edge[1]); i++);
	if(i != j)
		udg_destroy_edge(i);
	
	cur[0] = start_edge[0];
	cur[1] = start_edge[1];
//	next = get_next_edge(cur[1], cur[0], FALSE);
//	create_tube_matrix(matrix, vector, cur[1], cur[0], next);
	next = get_next_edge(cur[0], cur[1], FALSE);
	create_tube_matrix(matrix, vector, next, cur[0], cur[1]);

	fill_vertex_tube(matrix, a, cur[0], revolve, scale * 0.5, 1);
	for(i = 0; i < revolve; i++)
		c[i] = a[i];
	while(cur[1] != -1 && cur[1] != start_edge[0])
	{
		next = get_next_edge(cur[1], cur[0], TRUE);
		create_tube_matrix(matrix, vector, cur[0], cur[1], next);
		fill_vertex_tube(matrix, b, cur[1], revolve, scale * 0.5, 1);
		create_polygon_revolve(b, a, revolve);
		temp = b;
		b = a;
		a = temp;
		cur[0] = cur[1];
		cur[1] = next;
	/*	vector[0] = matrix[8];
		vector[1] = matrix[9];
		vector[2] = matrix[10];*/
	}
	if(cur[1] == start_edge[0])
	{
		create_polygon_revolve(c, a, revolve);
		free(a);
		free(b);
		free(c);
		return;
	}
	free(a);
	for(i = 0; i < 3; i++)
		vector[i] = -vector[i];
	next = get_next_edge(start_edge[0], start_edge[1], TRUE);
	cur[0] = start_edge[0];
	cur[1] = next;
	a = c;
    
	while(cur[1] != -1)
	{
		next = get_next_edge(cur[1], cur[0], TRUE);
		create_tube_matrix(matrix, vector, cur[0], cur[1], next);
		fill_vertex_tube(matrix, b, cur[1], revolve, scale * 0.5, -1);
		create_polygon_revolve(a, b, revolve);
		temp = b;
		b = a;
		a = temp;
		cur[0] = cur[1];
		cur[1] = next;
	/*	vector[0] = matrix[8];
		vector[1] = matrix[9];
		vector[2] = matrix[10];*/
	}
	free(a);
	free(b);
}

void la_t_select_open_edge(void)
{
	uint32 vertex_count, polygon_count, i, *ref, *array;
	udg_get_geometry(&vertex_count, &polygon_count, NULL, &ref, NULL);
	array = malloc((sizeof *array) * vertex_count);
	for(i = 0; i < vertex_count; i++)
		array[i] = 0;
	for(i = 0; i < polygon_count; i++)
	{
		if(ref[i * 4] < vertex_count)
		{
			if(ref[i * 4 + 3] < vertex_count)
			{
				array[ref[i * 4 + 0]] += ref[i * 4 + 3] - ref[i * 4 + 1];
				array[ref[i * 4 + 1]] += ref[i * 4 + 0] - ref[i * 4 + 2];
				array[ref[i * 4 + 2]] += ref[i * 4 + 1] - ref[i * 4 + 3];
				array[ref[i * 4 + 3]] += ref[i * 4 + 2] - ref[i * 4 + 0];
			}else
			{
				array[ref[i * 4 + 0]] += ref[i * 4 + 2] - ref[i * 4 + 1];
				array[ref[i * 4 + 1]] += ref[i * 4 + 0] - ref[i * 4 + 2];
				array[ref[i * 4 + 2]] += ref[i * 4 + 1] - ref[i * 4 + 0];
			}
		}
	}
	if(la_t_tm_hiden())
	{
	 	for(i = 0; i < vertex_count; i++)
			if(array[i] != 0)
				udg_set_select(i, 1);
	}else
	{
	 	for(i = 0; i < vertex_count; i++)
		{
			if(array[i] != 0 && udg_get_select(i) > 0.01)
				udg_set_select(i, 1);
			else
				udg_set_select(i, 0);
		}
	}
	free(array);
}

void la_t_crease_selected(uint32 crease_value)
{
	uint32 vertex_count, polygon_count, i, *ref, *crease, new_crease[4];
	boolean set;
	udg_get_geometry(&vertex_count, &polygon_count, NULL, &ref, &crease);
	for(i = 0; i < polygon_count; i++)
	{
		if(ref[i * 4] < vertex_count)
		{
			set = FALSE;
			new_crease[0] = crease[i * 4];
			new_crease[1] = crease[i * 4 + 1];
			new_crease[2] = crease[i * 4 + 2];
			new_crease[3] = crease[i * 4 + 3];
			if(udg_get_select(ref[i * 4]) > 0.01 && udg_get_select(ref[i * 4 + 1]) > 0.01)
			{
				new_crease[0] = crease_value;
				set = TRUE;
			}
			if(udg_get_select(ref[i * 4 + 1]) > 0.01 && udg_get_select(ref[i * 4 + 2]) > 0.01)
			{
				new_crease[1] = crease_value;
				set = TRUE;
			}
			if(ref[i * 4 + 3] < vertex_count)
			{
				if(udg_get_select(ref[i * 4 + 2]) > 0.01 && udg_get_select(ref[i * 4 + 3]) > 0.01)
				{
					new_crease[2] = crease_value;
					set = TRUE;
				}
				if(udg_get_select(ref[i * 4 + 3]) > 0.01 && udg_get_select(ref[i * 4]) > 0.01)
				{
					new_crease[3] = crease_value;
					set = TRUE;
				}
			}else if(udg_get_select(ref[i * 4 + 2]) > 0.01 && udg_get_select(ref[i * 4]) > 0.01)
			{
				new_crease[2] = crease_value;
				set = TRUE;
			}
			if(set == TRUE)
				udg_crease_set(i, new_crease[0], new_crease[1], new_crease[2], new_crease[3]);
		}
	}
}
/*
void la_t_create_tube(uint *start_edge, uint revolve)
{
	fill_vertex_revolve(matrix, a, start_edge[1], revolve);
	a = malloc((sizeof *a) * revolve);
	b = malloc((sizeof *b) * revolve);
}*/



void la_t_wrap_around(uint *start_edge, uint revolve)
{
	uint32 vertex_count, polygon_count, i, j, k, *ref, *crease, new_crease[4], *vertex_ids, sides;
	double matrix[16], scale, a[3], b[3], vector[3], *vertex_array, *transformed, min, max, x, z;
	udg_get_vertex_pos(matrix, start_edge[0]);
	udg_get_vertex_pos(&matrix[3], start_edge[1]);	
	vector[0] = (b[1] - a[1]) + a[0];
	vector[1] = (b[2] - a[2]) + a[1];
	vector[2] = (b[0] - a[0]) + a[2];
	f_matrixzxd(matrix, a, b, vector);


	udg_get_geometry(&vertex_count, &polygon_count, &vertex_array, &ref, &crease);
	transformed = malloc((sizeof *transformed) * vertex_count * 3);
	memcpy(transformed, vertex_array, (sizeof *vertex_array) * 3 * vertex_count);

	vertex_ids = malloc((sizeof *vertex_ids) * vertex_count);
	min = 10000000000;
	max = -10000000000;

	la_t_tm_get_pos(vector);
	for(i = 0; i < polygon_count; i++)
	{
		sides = 0;
		if(ref[i * 4] < vertex_count &&
			ref[i * 4 + 1] < vertex_count &&
			ref[i * 4 + 2] < vertex_count &&
			udg_get_select(ref[i * 4 + 0]) > 0.01 &&
			udg_get_select(ref[i * 4 + 1]) > 0.01 &&
			udg_get_select(ref[i * 4 + 2]) > 0.01)
		{
			if(ref[i * 4 + 3] < vertex_count)
			{		
				if(udg_get_select(ref[i * 4 + 3]) > 0.01)		
					sides = 4;	
			}else
				sides = 3;
		}
		for(j = 0; j < sides; j++)
		{
			if(vertex_array[ref[i * 4 + j] * 3 + 0] > max)
				max = vertex_array[ref[i * 4 + j] * 3 + 0];
			if(vertex_array[ref[i * 4 + j] * 3 + 0] < min)
				min = vertex_array[ref[i * 4 + j] * 3 + 0];
		}
	}
	if(max - min < 0.000001)
		return;
	for(i = 0; i < vertex_count; i++)
	{
		if(udg_get_select(i) > 0.001)
		{
			x = vector[0] + sin(vertex_array[i * 3 + 0] * 2.0 * PI / (max - min) / (double)revolve) * (vertex_array[i * 3 + 2] - vector[2]);
			z = vector[2] + cos(vertex_array[i * 3 + 0] * 2.0 * PI / (max - min) / (double)revolve) * (vertex_array[i * 3 + 2] - vector[2]);
			udg_vertex_set(i, NULL, x, vertex_array[i * 3 + 1], z);
		}
	}
	for(j = 1; j < revolve; j++)
	{	
		for(i = 0; i < vertex_count; i++)
			vertex_ids[i] = -1;
		for(i = 0; i < polygon_count; i++)
		{
			sides = 0;
			if(ref[i * 4] < vertex_count &&
				ref[i * 4 + 1] < vertex_count &&
				ref[i * 4 + 2] < vertex_count &&
				udg_get_select(ref[i * 4 + 0]) > 0.01 &&
				udg_get_select(ref[i * 4 + 1]) > 0.01 &&
				udg_get_select(ref[i * 4 + 2]) > 0.01)
			{
				if(ref[i * 4 + 3] < vertex_count)
				{		
					if(udg_get_select(ref[i * 4 + 3]) > 0.01)		
						sides = 4;	
				}else
					sides = 3;
			}
			if(sides != 0)
			{
				for(k = 0; k < sides; k++)
				{
					if(vertex_ids[ref[i * 4 + k]] == -1)
					{
						vertex_ids[ref[i * 4 + k]] = udg_find_empty_slot_vertex();
						x = vector[0] + sin((vertex_array[ref[i * 4 + k] * 3 + 0] / (max - min) + j) / (double)revolve * 2.0 * PI) * (vertex_array[ref[i * 4 + k] * 3 + 2] - vector[2]);
						z = vector[2] + cos((vertex_array[ref[i * 4 + k] * 3 + 0] / (max - min) + j) / (double)revolve * 2.0 * PI) * (vertex_array[ref[i * 4 + k] * 3 + 2] - vector[2]);
						udg_vertex_set(vertex_ids[ref[i * 4 + k]], NULL, x, vertex_array[ref[i * 4 + k] * 3 + 1], z);
					}
				}
				if(sides == 4)
				{
					k = udg_find_empty_slot_polygon();
					udg_polygon_set(k, vertex_ids[ref[i * 4 + 0]], vertex_ids[ref[i * 4 + 1]], vertex_ids[ref[i * 4 + 2]], vertex_ids[ref[i * 4 + 3]]);
					udg_crease_set(k, crease[i * 4 + 0], crease[i * 4 + 1], crease[i * 4 + 2], crease[i * 4 + 3]);
				}else
				{
					k = udg_find_empty_slot_polygon();
					udg_polygon_set(k, vertex_ids[ref[i * 4 + 0]], vertex_ids[ref[i * 4 + 1]], vertex_ids[ref[i * 4 + 2]], -1);
					udg_crease_set(k, crease[i * 4 + 0], crease[i * 4 + 1], crease[i * 4 + 2], -1);
				}
			}
		}
	}

	
	free(transformed);
	free(vertex_ids);
/*	for(i = 0; i < vertex_count; i++)
	{
		vector[0] = vertex_array[i * 3 + 0] - matrix[12];
		vector[1] = vertex_array[i * 3 + 1] - matrix[13];
		vector[2] = vertex_array[i * 3 + 2] - matrix[14];
		transformed[i * 3 + 0] = vector[0] * matrix[0] + vector[1] * matrix[1] + vector[2] * matrix[2];
		transformed[i * 3 + 1] = vector[0] * matrix[4] + vector[1] * matrix[5] + vector[2] * matrix[6];
		transformed[i * 3 + 2] = vector[0] * matrix[8] + vector[1] * matrix[9] + vector[2] * matrix[10];
		if()
	}*/
}