#include "la_includes.h"

#include "la_geometry_undo.h"

typedef struct{
	uint	vertex;
	uint	crease;
	uint	turn[2];
	uint	primitive;
	boolean polygon;
	boolean used;
}GSCList;

typedef struct{
	uint	create_ref[2];
	uint	destroy[2];
}GSCreateList;

struct{
	GSCList			*list[2];
	uint			list_length[2];
	uint			list_alloc;
	GSCreateList	*create;
	uint			create_length;
	uint			create_alloc;
}GlobalSurfaceCreate;

void la_t_init_edge_connector(void)
{
	GlobalSurfaceCreate.list_alloc = 16;
	GlobalSurfaceCreate.list[0] = malloc((sizeof *GlobalSurfaceCreate.list[0]) * GlobalSurfaceCreate.list_alloc);
	GlobalSurfaceCreate.list[1] = malloc((sizeof *GlobalSurfaceCreate.list[1]) * GlobalSurfaceCreate.list_alloc);
	GlobalSurfaceCreate.create_alloc = 16;
	GlobalSurfaceCreate.create = malloc((sizeof *GlobalSurfaceCreate.create) * GlobalSurfaceCreate.create_alloc);

}

void print_neighbor(void)
{
	uint i;
	printf("list 0 has %u entries\n", GlobalSurfaceCreate.list_length[0]);
	for(i = 0; i < GlobalSurfaceCreate.list_length[0]; i++)
		printf("list 0 entry %i is = %i balance = %i\n", i, GlobalSurfaceCreate.list[0][i].vertex, GlobalSurfaceCreate.list[0][i].turn);
	printf("list 1 has %u entries\n", GlobalSurfaceCreate.list_length[1]);
	for(i = 0; i < GlobalSurfaceCreate.list_length[1]; i++)
		printf("list 1 entry %i is = %i balance = %i\n", i, GlobalSurfaceCreate.list[1][i].vertex, GlobalSurfaceCreate.list[1][i].turn);
}

void print_edge_list(uint *edge, uint length)
{
	uint i;
	printf("edge_list length = %i\n", length);
	for(i = 0; i < length; i++)
		printf("edge = %i %i\n", edge[i * 2], edge[i * 2 + 1]);
}

/*
boolean set_surface_creace(uint *ref, uint length, uint vertex_length, uint *vertex)
{
	boolean output = TRUE;
	uint i, poly;
	length += 4; 
	for(i = 0; i < length; i++)
	{
		if(ref[i] == vertex[0])
		{
			poly = (i / 4) * 4;
			if(ref[poly + 3] < vertex_length)
			{
				if(ref[poly + ((i + 1) % 4)] == vertex[1]);
				if(ref[poly + ((i + 3) % 4)] == vertex[1]);
			}else
			{
				if(ref[poly + ((i + 1 - poly) % 3)] == vertex[1]);
				if(ref[poly + ((i + 2 - poly) % 3)] == vertex[1]);
			}
		}
	}
	return FALSE;
}
*/
void add_neighbor(uint list, uint vertex, uint turn, uint32 crease, uint32 primitive, boolean polygon)
{
	if(GlobalSurfaceCreate.list_length[list] == GlobalSurfaceCreate.list_alloc)
	{
		GlobalSurfaceCreate.list_alloc += 16;
		GlobalSurfaceCreate.list[0] = realloc(GlobalSurfaceCreate.list[0], (sizeof *GlobalSurfaceCreate.list[0]) * GlobalSurfaceCreate.list_alloc);
		GlobalSurfaceCreate.list[1] = realloc(GlobalSurfaceCreate.list[1], (sizeof *GlobalSurfaceCreate.list[1]) * GlobalSurfaceCreate.list_alloc);
	}
	GlobalSurfaceCreate.list[list][GlobalSurfaceCreate.list_length[list]].vertex = vertex;
	GlobalSurfaceCreate.list[list][GlobalSurfaceCreate.list_length[list]].turn[0] = 0;
	GlobalSurfaceCreate.list[list][GlobalSurfaceCreate.list_length[list]].turn[1] = 0;
	if(polygon)
		GlobalSurfaceCreate.list[list][GlobalSurfaceCreate.list_length[list]].turn[turn]++;
	GlobalSurfaceCreate.list[list][GlobalSurfaceCreate.list_length[list]].crease = crease;
	GlobalSurfaceCreate.list[list][GlobalSurfaceCreate.list_length[list]].primitive = primitive;
	GlobalSurfaceCreate.list[list][GlobalSurfaceCreate.list_length[list]].polygon = polygon;
	GlobalSurfaceCreate.list[list][GlobalSurfaceCreate.list_length[list]++].used = FALSE;
}

void find_neighbor_polygon(uint *ref, uint length, uint vertex_length, uint *vertex)
{
	boolean output = TRUE;
	int i, j, poly;
	length *= 4; 
	for(i = 0; i < length; i++)
	{
		for(j = 0; j < 2; j++)
		{
			if(ref[i] == vertex[j])
			{
				poly = (i / 4) * 4;
				if(ref[poly + 3] < vertex_length)
				{
					add_neighbor(j, ref[poly + ((i + 1) % 4)], j, 0, poly / 4, TRUE);
					add_neighbor(j, ref[poly + ((i + 3) % 4)], 1 - j, 0, poly / 4, TRUE);
				}else
				{
					add_neighbor(j, ref[poly + ((i + 1 - poly) % 3)], j, 0, poly / 4, TRUE);
					add_neighbor(j, ref[poly + ((i + 2 - poly) % 3)], 1 - j, 0, poly / 4, TRUE);
				}
			}
		}
	}
}

void find_neighbor_edges(uint *edge, uint length, uint *vertex)
{
	int i;
	length *= 2; 
	for(i = 0; i < length; i++)
	{
		if(edge[i] == vertex[0])
			add_neighbor(0, edge[(i / 2) * 2 + (i + 1) % 2], 0, 0, i / 2, FALSE);
		if(edge[i] == vertex[1])
			add_neighbor(1, edge[(i / 2) * 2 + (i + 1) % 2], 0, 0, i / 2, FALSE);
	}
}

void create_polygon(uint *vertex, uint list_a, uint list_b, uint destroy_a, uint destroy_b)
{
	uint i, j;
	for(j = 0; j < GlobalSurfaceCreate.create_length && (GlobalSurfaceCreate.create[j].create_ref[0] != GlobalSurfaceCreate.list[1][list_a].vertex || GlobalSurfaceCreate.create[j].create_ref[1] != GlobalSurfaceCreate.list[0][list_b].vertex); j++);
	if(j == GlobalSurfaceCreate.create_length)
	{
		if(GlobalSurfaceCreate.list[0][list_b].used == TRUE || GlobalSurfaceCreate.list[1][list_a].used == TRUE)
			return;
		for(i = 0; i < GlobalSurfaceCreate.list_length[0]; i++)
		{
			if(GlobalSurfaceCreate.list[0][list_b].vertex == GlobalSurfaceCreate.list[0][i].vertex)
			{
				GlobalSurfaceCreate.list[0][i].used = TRUE;
				destroy_a += GlobalSurfaceCreate.list[0][i].turn[0];
				destroy_b += GlobalSurfaceCreate.list[0][i].turn[1];
			}
		}
		for(i = 0; i < GlobalSurfaceCreate.list_length[1]; i++)
		{
			if(GlobalSurfaceCreate.list[1][list_a].vertex == GlobalSurfaceCreate.list[1][i].vertex)
			{
				GlobalSurfaceCreate.list[1][i].used = TRUE;
				destroy_a += GlobalSurfaceCreate.list[1][i].turn[0];
				destroy_b += GlobalSurfaceCreate.list[1][i].turn[1];
			}
		}

		GlobalSurfaceCreate.create[j].create_ref[0] = GlobalSurfaceCreate.list[1][list_a].vertex;
		if(GlobalSurfaceCreate.list[1][list_a].vertex == GlobalSurfaceCreate.list[0][list_b].vertex)
			GlobalSurfaceCreate.create[j].create_ref[1] = -1;
		else
			GlobalSurfaceCreate.create[j].create_ref[1] = GlobalSurfaceCreate.list[0][list_b].vertex;
		GlobalSurfaceCreate.create[j].destroy[0] = 0;
		GlobalSurfaceCreate.create[j].destroy[1] = 0;
		GlobalSurfaceCreate.create_length++;
	}
	GlobalSurfaceCreate.create[j].destroy[0] += destroy_a;
	GlobalSurfaceCreate.create[j].destroy[1] += destroy_b;
}

void find_triangles(uint *vertex)
{
	int i, j, k, third;
	for(i = 0; i < GlobalSurfaceCreate.list_length[0]; i++)
		for(j = 0; j < GlobalSurfaceCreate.list_length[1]; j++)
			if(GlobalSurfaceCreate.list[0][i].vertex == GlobalSurfaceCreate.list[1][j].vertex)
				create_polygon(vertex, j, i, 0, 0);
}

boolean polygon_culling(double *vertex, uint vertex_0, uint vertex_1, uint vertex_2)
{
	double a[3], b[3], c[3];
	seduce_view_projection_screend(NULL, a, vertex[vertex_0 * 3], vertex[vertex_0 * 3 + 1], vertex[vertex_0 * 3 + 2]);
	seduce_view_projection_screend(NULL, b, vertex[vertex_1 * 3], vertex[vertex_1 * 3 + 1], vertex[vertex_1 * 3 + 2]);
	seduce_view_projection_screend(NULL, c, vertex[vertex_2 * 3], vertex[vertex_2 * 3 + 1], vertex[vertex_2 * 3 + 2]);
	return (a[0] - b[0]) * (c[1] - b[1]) + (a[1] - b[1]) * (b[0] - c[0]) > 0;
}

void find_quads(uint *vertex, uint *ref, uint ref_length, uint *edge, uint edge_length)
{
	int i, j, k, third, prev, next;

	ref_length *= 4;
	for(i = 0; i < ref_length; i++)
	{
		for(j = 0; j < GlobalSurfaceCreate.list_length[0]; j++)
		{
			if(ref[i] == GlobalSurfaceCreate.list[0][j].vertex && GlobalSurfaceCreate.list[0][j].vertex != vertex[0] && (GlobalSurfaceCreate.list[0][j].primitive != i / 4 || GlobalSurfaceCreate.list[0][j].polygon != TRUE))
			{
				if(ref[(i / 4) * 4 + 3] != -1)
				{
					prev = (i / 4) * 4 + ((i + 3) % 4);
					next = (i / 4) * 4 + ((i + 1) % 4);
					for(k = 0; k < GlobalSurfaceCreate.list_length[1]; k++)
					{
						if(ref[prev] == GlobalSurfaceCreate.list[1][k].vertex && GlobalSurfaceCreate.list[1][k].vertex != vertex[1] && (GlobalSurfaceCreate.list[1][k].primitive != i / 4 || GlobalSurfaceCreate.list[1][k].polygon != TRUE))
							create_polygon(vertex, k, j, 0, 1);
						if(ref[next] == GlobalSurfaceCreate.list[1][k].vertex && GlobalSurfaceCreate.list[1][k].vertex != vertex[1] && (GlobalSurfaceCreate.list[1][k].primitive != i / 4 || GlobalSurfaceCreate.list[1][k].polygon != TRUE))
							create_polygon(vertex, k, j, 1, 0);
					}
				}
				else
				{
					prev = (i / 4) * 4 + (((i % 4)+ 2) % 3);
					next = (i / 4) * 4 + (((i % 4)+ 1) % 3);
					for(k = 0; k < GlobalSurfaceCreate.list_length[1]; k++)
					{
						if(ref[prev] == GlobalSurfaceCreate.list[1][k].vertex && GlobalSurfaceCreate.list[1][k].vertex != vertex[1] && (GlobalSurfaceCreate.list[1][k].primitive != i / 4 || GlobalSurfaceCreate.list[1][k].polygon != TRUE))
							create_polygon(vertex, k, j, 0, 1);
						if(ref[next] == GlobalSurfaceCreate.list[1][k].vertex && GlobalSurfaceCreate.list[1][k].vertex != vertex[1] && (GlobalSurfaceCreate.list[1][k].primitive != i / 4 || GlobalSurfaceCreate.list[1][k].polygon != TRUE))
							create_polygon(vertex, k, j, 1, 0);
					}
				}
			}			
		}
	}
	edge_length *= 2;
	for(i = 0; i < edge_length; i++)
		for(j = 0; j < GlobalSurfaceCreate.list_length[0]; j++)
			if(edge[i] == GlobalSurfaceCreate.list[0][j].vertex)
				for(k = 0; k < GlobalSurfaceCreate.list_length[1]; k++)
					if(edge[i + (1 - (i % 2) * 2)] == GlobalSurfaceCreate.list[1][k].vertex)
						create_polygon(vertex, k, j, 0, 0);

}

void select_found(double *array, uint *edge_vertex, uint *create_final)
{
	uint i, j, k, flip = 0, best[2] = {0, 1};
	if(GlobalSurfaceCreate.create_length > 2)
	{
		uint best_destroy = 6;
		for(i = 0; i < GlobalSurfaceCreate.create_length; i++)
		{
			for(j = 0; j < i; j++)
			{
				for(k = 0; k < 2; k++)
				{
					if(GlobalSurfaceCreate.create[i].destroy[k] + GlobalSurfaceCreate.create[j].destroy[(1 + k) % 2] == best_destroy)
					{
						if(polygon_culling(array, edge_vertex[(1 + k) % 2], edge_vertex[k], GlobalSurfaceCreate.create[i].create_ref[0]) && polygon_culling(array, edge_vertex[k], edge_vertex[(1 + k) % 2], GlobalSurfaceCreate.create[j].create_ref[0]))
						{
							best[0] = i;
							best[1] = j;
							flip = k;
						}
					}
					if(GlobalSurfaceCreate.create[i].destroy[k] + GlobalSurfaceCreate.create[j].destroy[(1 + k) % 2] < best_destroy)
					{
						best_destroy = GlobalSurfaceCreate.create[i].destroy[k] + GlobalSurfaceCreate.create[j].destroy[(1 + k) % 2];
						best[0] = i;
						best[1] = j;
						flip = k;
					}
				}
			}
		}
	}else if(GlobalSurfaceCreate.create_length == 2)
	{
		if(GlobalSurfaceCreate.create[0].destroy[1] + GlobalSurfaceCreate.create[1].destroy[0] < GlobalSurfaceCreate.create[1].destroy[1] + GlobalSurfaceCreate.create[0].destroy[0]
		|| (GlobalSurfaceCreate.create[0].destroy[1] + GlobalSurfaceCreate.create[1].destroy[0] == GlobalSurfaceCreate.create[1].destroy[1] + GlobalSurfaceCreate.create[0].destroy[0]
		&& polygon_culling(array, edge_vertex[0], edge_vertex[1], GlobalSurfaceCreate.create[0].create_ref[0])))
		flip = 1;
	}else if(GlobalSurfaceCreate.create[0].destroy[0] == GlobalSurfaceCreate.create[0].destroy[1])
	{
		if(polygon_culling(array, edge_vertex[0], edge_vertex[1], GlobalSurfaceCreate.create[0].create_ref[0]))
			flip = 1;

	}else if(GlobalSurfaceCreate.create[0].destroy[0] > GlobalSurfaceCreate.create[0].destroy[1])
		flip = 1;

	if((GlobalSurfaceCreate.create_length != 1 || flip == 1) && GlobalSurfaceCreate.create[best[1 - flip]].destroy[1] < 3)
	{
		create_final[0] = edge_vertex[0];	
		create_final[1] = edge_vertex[1];
		create_final[2] = GlobalSurfaceCreate.create[best[1 - flip]].create_ref[0];
		create_final[3] = GlobalSurfaceCreate.create[best[1 - flip]].create_ref[1];
	}
	if((GlobalSurfaceCreate.create_length != 1 || flip == 0) && GlobalSurfaceCreate.create[best[flip]].destroy[0] < 3)
	{
		create_final[4] = GlobalSurfaceCreate.create[best[flip]].create_ref[0];
		create_final[5] = edge_vertex[1];
		create_final[6] = edge_vertex[0];
		create_final[7] = GlobalSurfaceCreate.create[best[flip]].create_ref[1];
	}
}



void destroy_surface(uint *ref, uint *crease, uint vertex_length, uint ref_length, uint *create_ref, uint *create_crease)
{
	uint vertex, i, next;
	ref_length *= 4;
	for(i = 0; i < ref_length; i++)
	{
		if(ref[(i / 4) * 4 + (i + 1) % 4] > vertex_length)
			next = ref[(i / 4) * 4];
		else
			next = ref[(i / 4) * 4 + (i + 1) % 4];
		if(ref[i]  < vertex_length && next < vertex_length)
		{
			if(ref[i] == create_ref[0])
			{
				if(next == create_ref[1])
					udg_polygon_delete(i / 4);
				if(crease != NULL)
				{
					if(next == create_ref[3])
						create_crease[3] = crease[i];
					if(next == create_ref[2])
						create_crease[2] = crease[i];
				}
			}
			if(ref[i] == create_ref[1])
			{
				if(next == create_ref[2])
					udg_polygon_delete(i / 4);
				if(crease != NULL)
					if(next == create_ref[0])
						create_crease[0] = crease[i];
			}
			if(ref[i] == create_ref[2])
			{
				if(next == create_ref[3] || next == create_ref[0])
					udg_polygon_delete(i / 4);
				if(crease != NULL)
					if(next == create_ref[1])
						create_crease[1] = crease[i];
			}
			if(ref[i] == create_ref[3])
			{
				if(next == create_ref[0])
					udg_polygon_delete(i / 4);
				if(crease != NULL)
					if(next == create_ref[2])
						create_crease[2] = crease[i];
			}
			if(ref[i] == create_ref[4])
			{
				if(next == create_ref[5])
					udg_polygon_delete(i / 4);
				if(crease != NULL)
				{
					if(next == create_ref[7])
						create_crease[7] = crease[i];
					if(next == create_ref[6])
						create_crease[6] = crease[i];
				}
			}
			if(ref[i] == create_ref[5])
			{
				if(next == create_ref[6])
					udg_polygon_delete(i / 4);
				if(crease != NULL)
					if(next == create_ref[4])
						create_crease[4] = crease[i];
			}
			if(ref[i] == create_ref[6])
			{
				if(next == create_ref[7] || next == create_ref[4])
					udg_polygon_delete(i / 4);
				if(crease != NULL)
					if(next == create_ref[5])
						create_crease[5] = crease[i];
			}
			if(ref[i] == create_ref[7])
			{
				if(next == create_ref[4])
					udg_polygon_delete(i / 4);
				if(crease != NULL)
					if(next == create_ref[6])
						create_crease[6] = crease[i];
			}
		}else
			if(i % 4 == 0)
				i += 3;
	}
}

void destroy_edges(uint *edge, uint edge_length, uint *create_ref)
{
	int i;
	uint j, next, prev;
	for(i = edge_length - 1; i >= 0; i--)
	{
		prev = edge[i * 2];
		next = edge[i * 2 + 1];
		for(j = 0; j < 8; j++)
			if(create_ref[j] == prev)
				if(create_ref[(j / 4) * 4] == next || create_ref[(j / 4) * 4 + 1] == next || create_ref[(j / 4) * 4 + 2] == next || create_ref[(j / 4) * 4 + 3] == next)
					udg_destroy_edge(i);
	}
}



void create_surface(uint *create, uint *crease)
{
	uint poly;
	if(create[0] != -1)
	{
		poly = udg_find_empty_slot_polygon();
		udg_polygon_set(poly, create[0], create[1], create[2], create[3]);
		udg_crease_set(poly, crease[0], crease[1], crease[2], crease[3]);
	}
	if(create[4] != -1)
	{
		poly = udg_find_empty_slot_polygon();
		udg_polygon_set(poly, create[4], create[5], create[6], create[7]);
		udg_crease_set(poly, crease[4], crease[5], crease[6], crease[7]);
	}
}

boolean crease_edge(uint *crease, uint *vertex, uint *ref, uint ref_length)
{
	boolean output = FALSE; 
	uint32 replace; 
	uint next, i, j, new_crease[4];
	if(crease == NULL)
		return FALSE;
	ref_length *= 4;
	for(i = 0; i < ref_length; i += 4)
	{
		if(ref[i] != -1)
		{
			for(j = 0; j < 4; j++)
			{
				if(vertex[0] == ref[i + j] || vertex[1] == ref[i + j])
				{
					if(ref[i + 3] == -1)
						next = i + ((j + 1) % 3);
					else
					{
						next = i + ((j + 1) % 4);
						if(ref[i + ((j + 2) % 4)] == vertex[1])
						{
							udg_polygon_set(i / 4, vertex[0], vertex[1], ref[i + ((j + 3) % 4)], -1);
							udg_crease_set(i / 4, 0, crease[i + ((j + 2) % 4)], crease[i + ((j + 3) % 4)], -1);
							replace = udg_find_empty_slot_polygon();
							udg_polygon_set(replace, vertex[1], vertex[0], ref[i + ((j + 1) % 4)], -1);
							udg_crease_set(replace, 0, crease[i], crease[i + ((j + 1) % 4)], -1);
							return TRUE;
						}
					}
					if(ref[next] == vertex[1] || ref[next] == vertex[0])
					{
						if(output == FALSE)
						{
							output = TRUE;
							if(2147483647 < crease[i + j])
								replace = 0;
							else
								replace = -1;
						}
						new_crease[0] = crease[i];
						new_crease[1] = crease[i + 1];
						new_crease[2] = crease[i + 2];
						new_crease[3] = crease[i + 3];
						new_crease[j] = replace;
						udg_crease_set(i / 4, new_crease[0], new_crease[1], new_crease[2], new_crease[3]);
					}
				}
			}
		}
	}
	return output;
}

boolean la_t_edge_connector(uint *vertex)
{
	double *vertex_array;
	uint *edge, edge_length, *crease, create_final[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }, crease_final[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint *ref, ref_length, vertex_length;

	udg_get_geometry(&vertex_length, &ref_length, &vertex_array, &ref, &crease);
	if(crease_edge(crease, vertex, ref, ref_length))
		return FALSE;
	edge = udg_get_edge_data(&edge_length);
//	print_edge_list(edge, edge_length);
	GlobalSurfaceCreate.list_length[0] = 0;
	GlobalSurfaceCreate.list_length[1] = 0;
	GlobalSurfaceCreate.create_length = 0;
	find_neighbor_polygon(ref, ref_length, vertex_length, vertex);
	find_neighbor_edges(edge, edge_length, vertex);
//	print_neighbor();
	find_triangles(vertex);
	find_quads(vertex, ref, ref_length, edge, edge_length);
	if(GlobalSurfaceCreate.create_length == 0)
		return TRUE;
	select_found(vertex_array, vertex, create_final);
	destroy_edges(edge, edge_length, create_final);
	destroy_surface(ref, crease, vertex_length, ref_length, create_final, crease_final);
	create_surface(create_final, crease_final);
	return FALSE;
}
