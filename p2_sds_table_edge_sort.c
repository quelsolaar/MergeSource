#define PERSUADE_INTERNAL
#include "persuade2.h"
#include "p2_sds_table.h"

void p_geo_table_sort_edges(PTessTableElement *t, uint *edges, uint corners)
{
	uint i, j, k, l, temp, vertex = 0;
	pgreal space, pos, a;
	t->edges[0] = 0;	
	for(i = 0; i < corners; i++)
	{
		pos = 0;
		space = 1;
		for(j = 0; j < edges[i]; j++)
			space /= 2;
		for(pos = 0; pos + 0.0001 < 1.0; pos += space)
		{
			for(k = 0; k < t->vertex_count; k++)
			{
				a = t->vertex_influence[k * corners + (i + 1) % corners];
				if(a + 0.0001 > pos && a - 0.0001 < pos)
				{
					a = t->vertex_influence[k * corners + i];
					if(a + 0.0001 > 1 - pos && a - 0.0001 < 1 - pos)
					{
						temp = t->reference[k];
						t->reference[k] = t->reference[vertex];
						t->reference[vertex] = temp;
						for(l = 0; l < corners; l++)
						{
							a = t->vertex_influence[k * corners + l];
							t->vertex_influence[k * corners + l] = t->vertex_influence[vertex * corners + l];
							t->vertex_influence[vertex * corners + l] = a; 
						}
						for(l = 0; l < t->element_count; l++)
						{
							if(t->index[l] == k)
								t->index[l] = vertex;
							else if(t->index[l] == vertex)
								t->index[l] = k;
						}
						break;
					}
				}
			}
			vertex++;
		}
		t->edges[i + 1] = vertex;
	}
	if(corners == 3)
		t->edges[4] = t->edges[3];
}
