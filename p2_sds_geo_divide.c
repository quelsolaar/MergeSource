#define PERSUADE_INTERNAL
#include <stdio.h>
#include <stdlib.h>
#include "persuade2.h"
#include "p2_sds_geo.h"

uint p_sds_get_corner_next(PPolyStore *mesh, uint corner, int move)
{
	if(corner < mesh->quad_length)
		return ((corner / 4) * 4) + (corner + (uint)(4 + move)) % 4;
	else
	{
		uint a;
		a = corner - mesh->quad_length;
		return mesh->quad_length + ((a / 3) * 3) + (a + (uint)(3 + move)) % 3;
	}
}

void p_sds_add_edge_polygon(PPolyStore *old_mesh, PDepend *dep, uint poly, pgreal weight)
{
	if(poly < old_mesh->quad_length)
	{
		weight /= 4.0;
		poly = (poly / 4) * 4;
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly]], weight);			
	}else
	{
		weight /= 3.0;	
		weight *= 2;	
		poly = old_mesh->quad_length + (((poly - old_mesh->quad_length) / 3) * 3);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly]], weight);		
	}
}

void p_sds_add_vertex_polygon(PPolyStore *old_mesh, PDepend *dep, uint poly, pgreal weight)
{
	if(poly < old_mesh->quad_length)
	{
		weight /= 4.0;
		poly = (poly / 4) * 4;
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly]], weight);			
	}else
	{
		weight /= 4.0;	
		poly = old_mesh->quad_length + (((poly - old_mesh->quad_length) / 3) * 3);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly++]], weight);
		p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[poly]], weight);		
	}
}


typedef struct{
	uint	edge;
	uint	polygon;
	pgreal	crease;
}PSDSVertexSet;

uint p_sds_get_vertex(PPolyStore *old_mesh, uint corner)
{
	PDepend *dep;
	dep = &((PPolyStore *)old_mesh->next)->vertex_dependency[old_mesh->ref[corner]];

	if(dep->element[0].vertex == -1)
	{
//		PSDSVertexSet list[2000];
		uint	vertex[2000];
		uint	crease[2000];
		uint	polys[2000];
		uint	vertex_count = 2;
		uint	poly_count = 1;
		uint	i, a;
		pgreal best, second_best, third_best;

		crease[0] = 0;
		crease[1] = 0;
		crease[2] = 0;
		crease[3] = 0;
		crease[4] = 0;
		crease[5] = 0;
		crease[6] = 0;

		vertex[0] = old_mesh->ref[p_sds_get_corner_next(old_mesh, corner, 1)];
		crease[0] = 1 - p_sds_get_crease(old_mesh, corner);
		a = p_sds_get_corner_next(old_mesh, corner, -1);
		vertex[1] = old_mesh->ref[a];
		crease[1] = 1 - p_sds_get_crease(old_mesh, a);
		polys[0] = corner;

		a = old_mesh->neighbor[corner];
		if(a != -1)
			a = p_sds_get_corner_next(old_mesh, a, 1);



		while(a != -1)
		{
			polys[poly_count++] = a;
			vertex[vertex_count] = old_mesh->ref[p_sds_get_corner_next(old_mesh, a, 1)];
			for(i = 0; i < vertex_count && i < 2000 && vertex[i] != vertex[vertex_count]; i++);
			if(i < vertex_count)
				break;
			crease[vertex_count++] = 1 - p_sds_get_crease(old_mesh, a);
			a = old_mesh->neighbor[a];
			if(a != -1)
				a = p_sds_get_corner_next(old_mesh, a, 1);
		}
		if(a == -1)

		{
			a = old_mesh->neighbor[p_sds_get_corner_next(old_mesh, corner, -1)];
			if(a != -1)
				a = p_sds_get_corner_next(old_mesh, a, -1);

			while(a != -1)
			{
				vertex[vertex_count] = old_mesh->ref[a];
				for(i = 0; i < vertex_count && i < 2000 && vertex[i] != vertex[vertex_count]; i++);
				if(i < vertex_count)
					break;
				polys[poly_count++] = a;
				crease[vertex_count++] = (1 - p_sds_get_crease(old_mesh, a));
				a = old_mesh->neighbor[a];
				if(a != -1)
					a = p_sds_get_corner_next(old_mesh, a, -1);
			}
		}

		if(poly_count == 1)
			p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[corner]], 1);
		else
		{
			best = 0;
			second_best = 0;
			third_best = 0;	
			for(i = 0; i < vertex_count; i++)
			{
				if(crease[i] >= best)
				{
					third_best = second_best;
					second_best = best;
					best = crease[i];
				}else if(crease[i] >= second_best)
				{
					third_best = second_best;
					second_best = crease[i];
				}else if(crease[i] >= third_best)
					third_best = crease[i];
			}

			for(i = 0; i < vertex_count; i++)
				p_sds_add_depend(dep, &old_mesh->vertex_dependency[vertex[i]], ((1 - second_best) + crease[i] * (1 - third_best)) / vertex_count);
			for(i = 0; i < poly_count; i++)
				p_sds_add_vertex_polygon(old_mesh, dep, polys[i], (1 - second_best) * (1 - third_best) / vertex_count);
			p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[corner]], (vertex_count - 2) * (1 - second_best) + 2 * second_best);
		}
		/*
		uint i, j, a, b, list_count = 0, corner_start, vertex;
		pgreal best, third_best;
		corner_start = corner;

		while(corner != -1)
		{
			vertex = old_mesh->ref[p_sds_get_corner_next(old_mesh, corner, 1)];
			for(i = 0; i < list_count && i < 2000 && list[i].edge != vertex; i++);
			if(i < list_count)
				break;
			list[list_count].edge = vertex;
			list[list_count].crease = 1 - p_sds_get_crease(old_mesh, corner);
			list[list_count].polygon = corner;
			a = corner;
			corner = old_mesh->neighbor[corner];
			if(corner != -1)
				corner = p_sds_get_corner_next(old_mesh, corner, 1);
			list_count++;
		}
		if(corner == -1)
		{
			corner = p_sds_get_corner_next(old_mesh, corner_start, -1);
			while(corner != -1)
			{
				vertex = old_mesh->ref[p_sds_get_corner_next(old_mesh, corner, -1)];
				for(i = 0; i < list_count; i++)
					if(list[i].edge == vertex || i == 1999)
						break;
				if(list[i].edge == vertex)
					break;
				list[list_count].edge = vertex;
				list[list_count].crease = 1 - p_sds_get_crease(old_mesh, p_sds_get_corner_next(old_mesh, corner, -1));
				list[list_count].polygon = corner;
				corner = old_mesh->neighbor[corner];
				if(corner != -1)
					corner = p_sds_get_corner_next(old_mesh, corner, -1);
				else
				{
					p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[corner_start]], 20);
					if(list_count != 1)
					{
						p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[p_sds_get_corner_next(old_mesh, a, 1)]], 10);
						p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[list[list_count].polygon]], 10);
					}
					return old_mesh->ref[corner_start];
				}
				list_count++;
			}
		}´*/
/*		printf("list start %u\n", corner_start);
		for(i = 0; i < list_count; i++)
		{
			printf("list[%u].edge %u\n", i, list[i].edge);
			printf("list[%u].crease %f", i, list[i].crease);
			if(list[i].crease > 0.1)
				printf("----------------------------------------");
			printf("\nlist[%u].polygon %u\n", i, list[i].polygon);
			printf("ref %u %u %u %u\n",
				old_mesh->ref[(list[i].polygon / 4) * 4 + 0],
				old_mesh->ref[(list[i].polygon / 4) * 4 + 1],
				old_mesh->ref[(list[i].polygon / 4) * 4 + 2],
				old_mesh->ref[(list[i].polygon / 4) * 4 + 3]);
		}
		printf("list end\n");
*/ /*		best = 0;
		a = -1;
		b = -1;
		for(i = 1; i < list_count; i++)
		{
			if(list[i].crease > 0.01)
			{
				for(j = 0; j < i; j++)
				{
					if(list[j].crease > 0.01)
					{	
						if((list[i].crease + list[j].crease) > best)
						{
							best = (list[i].crease + list[j].crease);
							a = i;
							b = j;
						}
					}
				}
			}
		}
		best *= 0.5;
		third_best = 0;
		if(list_count == 1)
			third_best = 1;
		else for(i = 0; i < list_count; i++)
			if(i != a && i != b && list[i].crease > third_best)
				third_best = list[i].crease;
		//	printf("best %f third %f\n", best, third_best);
	//	best = 1;
	//	third_best = 1;
		if(third_best < 0.99)
		{
		for(i = 0; i < list_count; i++)
		{
			if(list[i].crease > 0.01)
			{
				for(j = 0; j < i; j++)
				{
					if(list[j].crease > 0.01)
					{	
						p_sds_add_depend(dep, &old_mesh->vertex_dependency[list[a].edge], best / 8 * (list[i].crease + list[j].crease) * 0.5 * (1 - third_best));
						p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[corner_start]], (best * 6) / 8 * (list[i].crease + list[j].crease) * 0.5 * (1 - third_best));
						p_sds_add_depend(dep, &old_mesh->vertex_dependency[list[b].edge], best / 8 * (list[i].crease + list[j].crease) * 0.5 * (1 - third_best));
					}
				}
			}
		}
		if(best < 1)
		{
			for(i = 0; i < list_count; i++)
			{
				p_sds_add_depend(dep, &old_mesh->vertex_dependency[list[i].edge], 1.0 / (list_count) * (1 - best));
				p_sds_add_polygon(old_mesh, dep, list[i].polygon, 1.0 / (list_count) * (1 - best));
			}
			p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[corner_start]], (list_count - 2) * (1 - best));
		}
		}
		if(third_best > 0.001)
			p_sds_add_depend(dep, &old_mesh->vertex_dependency[old_mesh->ref[corner_start]], third_best);

*/
		return old_mesh->ref[corner];
	}
	return old_mesh->ref[corner];
}


uint p_sds_get_middle(PPolyStore *old_mesh, uint poly)
{
	PPolyStore *mesh;
	PDepend *dep;
	mesh = old_mesh->next;
	dep = &mesh->vertex_dependency[mesh->vertex_count];
	p_sds_add_vertex_polygon(old_mesh, dep, poly, 1);	
	return mesh->vertex_count++;
}

uint p_sds_get_edge(PPolyStore *old_mesh, uint edge)
{
	static tri_edge[3] = {1, 5, 2}, quad_edge[4] = {1, 6, 11, 3};
	PPolyStore *mesh;
	pgreal crease;
	uint a;
	mesh = old_mesh->next;
	if(old_mesh->neighbor[edge] < edge/* && old_mesh->neighbor[edge] < old_mesh->quad_length*/)
	{
		a = old_mesh->neighbor[edge];
		if(a > old_mesh->quad_length)
		{
			a -= old_mesh->quad_length;
			return mesh->ref[(old_mesh->quad_length + (a - a % 3)) * 4 + tri_edge[a % 3]];
		}else
			return mesh->ref[(a - (a % 4)) * 4 + quad_edge[a % 4]];
	}
	crease = p_sds_get_crease(old_mesh, edge);

	p_sds_add_depend(&mesh->vertex_dependency[mesh->vertex_count], &old_mesh->vertex_dependency[old_mesh->ref[edge]], 1);
	p_sds_add_depend(&mesh->vertex_dependency[mesh->vertex_count], &old_mesh->vertex_dependency[old_mesh->ref[p_sds_get_corner_next(old_mesh, edge, 1)]], 1);
	if(crease > 0.01 && old_mesh->neighbor[edge] != -1)
	{
		p_sds_add_edge_polygon(old_mesh, &mesh->vertex_dependency[mesh->vertex_count], old_mesh->neighbor[edge], crease);
		p_sds_add_edge_polygon(old_mesh, &mesh->vertex_dependency[mesh->vertex_count], edge, crease);
	}
	return mesh->vertex_count++;	
}

float p_sds_divide(PPolyStore *mesh)
{
	static quad_first[4] = {4, 9, 14, 3}, quad_second[4] = {0, 5, 10, 15};
	static tri_first[3] = {3, 7, 2}, tri_second[3] = {0, 4, 8};
	static quad_edge[8] = {0, 4, 5, 9, 10, 14, 15, 3}, tri_edge[6] = {0, 3, 4, 7, 8, 2};
	PPolyStore *new_mesh;
	uint i,  j, *ref, *n;
	new_mesh = mesh->next;
	for(i = 0; i < mesh->quad_length; i += 4)
	{
		ref = &new_mesh->ref[i * 4];
		ref[0] = p_sds_get_vertex(mesh, i); // first corner
		ref[1] = p_sds_get_edge(mesh, i); // first edge
		ref[2] = p_sds_get_middle(mesh, i); // mid point
		ref[3] = p_sds_get_edge(mesh, i + 3); // fourth edge
		ref[4] = ref[1];
		ref[5] = p_sds_get_vertex(mesh, i + 1); // second corner
		ref[6] = p_sds_get_edge(mesh, i + 1); // second edge
		ref[7] = ref[2];
		ref[8] = ref[2];
		ref[9] = ref[6];
		ref[10] = p_sds_get_vertex(mesh, i + 2); // third corner
		ref[11] = p_sds_get_edge(mesh, i + 2); // third edge
		ref[12] = ref[3];
		ref[13] = ref[2];
		ref[14] = ref[11];
		ref[15] = p_sds_get_vertex(mesh, i + 3); // fourth corner
		n = &new_mesh->neighbor[i * 4];

		for(j = 0; j < 4; j++)
		{
			if(mesh->neighbor[i + j] == -1)
			{
				n[quad_edge[j * 2 + 0]] = -1;
				n[quad_edge[j * 2 + 1]] = -1;		
			}else if(mesh->neighbor[i + j] < mesh->quad_length)
			{
				n[quad_edge[j * 2 + 0]] = (mesh->neighbor[i + j] / 4) * 16 + quad_first[mesh->neighbor[i + j] % 4];
				n[quad_edge[j * 2 + 1]] = (mesh->neighbor[i + j] / 4) * 16 + quad_second[mesh->neighbor[i + j] % 4];
			}else
			{
				n[quad_edge[j * 2 + 0]] = mesh->quad_length * 4 + ((mesh->neighbor[i + j] - mesh->quad_length) / 3) * 12 + tri_first[(mesh->neighbor[i + j] - mesh->quad_length) % 3];
				n[quad_edge[j * 2 + 1]] = mesh->quad_length * 4 + ((mesh->neighbor[i + j] - mesh->quad_length) / 3) * 12 + tri_second[(mesh->neighbor[i + j] - mesh->quad_length) % 3];
			}	
		}
		n[1] = i * 4 + 7;
		n[2] = i * 4 + 12;
		n[6] = i * 4 + 8;
		n[7] = i * 4 + 1;
		n[8] = i * 4 + 6;
		n[11] = i * 4 + 13;
		n[12] = i * 4 + 2;
		n[13] = i * 4 + 11;

	}
	for(; i < mesh->quad_length + mesh->tri_length; i += 3)
	{
		ref = &new_mesh->ref[i * 4];
		ref[0] = p_sds_get_vertex(mesh, i); // first corner
		ref[1] = p_sds_get_edge(mesh, i); // first edge
		ref[2] = p_sds_get_edge(mesh, i + 2); // third edge
		ref[3] = ref[1];
		ref[4] = p_sds_get_vertex(mesh, i + 1); // second corner
		ref[5] = p_sds_get_edge(mesh, i + 1); // second edge
		ref[6] = ref[2];
		ref[7] = ref[5];
		ref[8] = p_sds_get_vertex(mesh, i + 2); // third corner;
		ref[9] = ref[1]; // third corner
		ref[10] = ref[5]; // third edge
		ref[11] = ref[2];

		n = &new_mesh->neighbor[i * 4];
		for(j = 0; j < 3; j++)
		{
			if(mesh->neighbor[i + j] == -1)
			{
				n[tri_edge[j * 2 + 0]] = -1;
				n[tri_edge[j * 2 + 1]] = -1;		
			}else if(mesh->neighbor[i + j] < mesh->quad_length)
			{
				n[tri_edge[j * 2 + 0]] = (mesh->neighbor[i + j] / 4) * 16 + quad_first[mesh->neighbor[i + j] % 4];
				n[tri_edge[j * 2 + 1]] = (mesh->neighbor[i + j] / 4) * 16 + quad_second[mesh->neighbor[i + j] % 4];
			}else
			{
				n[tri_edge[j * 2 + 0]] = mesh->quad_length * 4 + ((mesh->neighbor[i + j] - mesh->quad_length) / 3) * 12 + tri_first[(mesh->neighbor[i + j] - mesh->quad_length) % 3];
				n[tri_edge[j * 2 + 1]] = mesh->quad_length * 4 + ((mesh->neighbor[i + j] - mesh->quad_length) / 3) * 12 + tri_second[(mesh->neighbor[i + j] - mesh->quad_length) % 3];
			}	
		}
		j = i * 4;
		n[1] = j + 11;
		n[5] = j + 9;
		n[6] = j + 10;
		n[9] = j + 5;
		n[10] = j + 6;
		n[11] = j + 1;
	}
	return (float)i;
//	mesh_new->quad_length = mesh_new->quad_length - length;

}
