#include "la_includes.h"
#include "la_geometry_undo.h"

uint *edge_reference(uint *ref, uint poly_count, uint vertex_count)
{
	uint *back, *cur, *end, *vertex, i, j, poly, poly2, update, temp, temp2;
	poly_count *= 4;
	back = malloc((sizeof *back) * poly_count);
	cur = back;
	for(end = cur + poly_count; cur != end; cur++)
		*cur = -1;
	vertex = malloc((sizeof *back) * vertex_count);
	cur = vertex;
	for(i = 0; i < poly_count; i++)
		if(ref[i] < vertex_count)
			vertex[ref[i]] = i;
	update = poly_count;
	for(i = 0; 0 != update; i += 4)
	{
		if(ref[i] < poly_count)
		{
			poly = 3;
			if(ref[i + 3] < vertex_count)
				poly = 4;
			for(j = 0; j < poly; j++)
			{
				cur = &vertex[ref[i + j]];
				if(*cur == -1)
					*cur = i + j;
				else
				{
					if(*cur == i + j)
						*cur = -1;
					else
					{
						if(back[i + j] == -1)
						{
							uint temp;
							temp = (*cur / 4 * 4);
							if(ref[temp + 3] < vertex_count)
								poly2 = 4;
							else
								poly2 = 3;
							temp = temp + (*cur + poly2 - temp - 1) % poly2;
							temp2 = i + (j + 1) % poly;
							if(ref[temp] == ref[temp2])
							{
								back[i + j] = temp;
								back[temp] = i + j;
								if(back[temp2] != -1)
									*cur = -1;
								update = poly_count;
							}
						}
						temp2 = i + ((j + poly - 1) % poly);
						if(back[temp2] == -1)
						{	
							temp = (*cur / 4 * 4);
							if(ref[temp + 3] < vertex_count)
								poly2 = 4;
							else
								poly2 = 3;
							temp = temp + (*cur - temp + 1) % poly2;
							if(ref[temp] == ref[temp2])
							{
								back[temp2] = *cur;
								back[*cur] = temp2;
								if(back[i + j] == -1)
									*cur = -1;
								update = poly_count;
							}
						}
					}
				}
			}
		}
		update -= 4;
		if(i == poly_count)
			i = 0;
	}
	free(vertex);
	return back;
}


void test_ref_creator(void)
{
	uint *data;
	uint *ref, i, poly_count, vertex_count;
	udg_get_geometry(&vertex_count, &poly_count, NULL, &ref, NULL);
	data = edge_reference(ref, poly_count, vertex_count);
	free(data);
}
