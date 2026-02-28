#include "la_includes.h"
#include "la_geometry_undo.h"

void collapse_polygon(uint id, uint *ref, uint collapse, uint vertex_count, boolean *del)
{
	uint i, count = 0, shift, create_ref[4];
	for(i = 0; i < 4; i++)
		if(ref[i] == collapse)
			count++;
	if(count == 1)
	{
		udg_polygon_set(id, ref[0], ref[1], ref[2], ref[3]);
		*del = FALSE;
	}else if(count > 2 || (count == 2 && ref[3] > vertex_count))
		udg_polygon_delete(id);
	else if((ref[1] == collapse && ref[3] == collapse) || (ref[0] == collapse && ref[2] == collapse))
		udg_polygon_delete(id);
	else
	{
		for(i = 0; i < 4 && (ref[i] != collapse || ref[(i + 1) % 4] != collapse); i++);
		shift = (3 - i);
		for(i = 0; i < 4; i++)
			create_ref[(i + shift) % 4] = ref[i];
		udg_polygon_set(id, create_ref[0], create_ref[1], create_ref[2], -1);
		*del = FALSE;
	}
}

void la_t_collapse_two_vertexes(uint vertex_a, uint vertex_b)
{
	uint32 new_ref[4], vertex_count, polygon_count, *ref, i, j;
	double a[3], b[3];
	boolean del = TRUE;
	udg_get_geometry(&vertex_count, &polygon_count, NULL, &ref, NULL);

	for(i = 0; i < polygon_count; i++)
	{
		if(ref[i * 4] <= vertex_count)
		{
			for(j = 0; j < 4; j++)
			{
				if(ref[i * 4 + j] == vertex_a)
				{
					new_ref[0] = ref[i * 4];
					new_ref[1] = ref[i * 4 + 1];
					new_ref[2] = ref[i * 4 + 2];
					new_ref[3] = ref[i * 4 + 3];
					new_ref[j] = vertex_b;
					collapse_polygon(i, new_ref, vertex_b, vertex_count, &del);
				}
			}
		}
	}
    ref = udg_get_edge_data(&polygon_count);
    for(i = 0; i < polygon_count; i++)
    {
        for(j = 0; j < 2; j++)
        {
            if(ref[i * 2 + j] == vertex_a)
			{
				if(ref[i * 2 + (j + 1) % 2] != vertex_b)
				{
					udg_create_edge(ref[i * 2 + (j + 1) % 2], vertex_b);
					del = FALSE;
				}
				udg_destroy_edge(i);
				i--;
				polygon_count--;
			}
		}
	}
	if(del)
		udg_vertex_delete(vertex_b);
	else
	{
		udg_get_vertex_pos(a, vertex_a);
		udg_get_vertex_pos(b, vertex_b);
		udg_vertex_set(vertex_b, b, (a[0] + b[0]) * 0.5, (a[1] + b[1]) * 0.5, (a[2] + b[2]) * 0.5);
	}
	udg_vertex_delete(vertex_a);	
}

void la_t_collapse_selected_vertexes(void)
{
	uint32 new_ref[4], vertex_count, polygon_count, *ref, i, j, collapse;
	boolean recreate, del = TRUE;
	double vertex[3], middle[3] = {0, 0, 0}, sum = 0, select;
	
	udg_get_geometry(&vertex_count, &polygon_count, NULL, &ref, NULL);

	for(i = 0; i < vertex_count && 0.01 > udg_get_select(i); i++);
	if(i == vertex_count)
		return;
	collapse = i;
	for(; i < vertex_count; i++)
	{
		select = udg_get_select(i);
		if(0.01 < select)
		{
			udg_get_vertex_pos(vertex, i);
			if(vertex[0] != V_REAL64_MAX)
			{
				middle[0] += vertex[0] * select;
				middle[1] += vertex[1] * select;
				middle[2] += vertex[2] * select;
				sum += select;
			}
		}
	}
	for(i = 0; i < polygon_count; i++)
	{
		if(ref[i * 4] <= vertex_count)
		{
			recreate = FALSE;
			for(j = 0; j < 4; j++)
			{
				new_ref[j] = ref[i * 4 + j];
				if(new_ref[j] <= vertex_count && 0.01 < udg_get_select(new_ref[j]) && new_ref[j] != collapse)
				{
					new_ref[j] = collapse;
					recreate = TRUE;
				}					
			}
			if(recreate == TRUE)
				collapse_polygon(i, new_ref, collapse, vertex_count, &del);
		}
	}
	if(del)
		udg_vertex_delete(collapse);
	for(i = collapse + 1; i < vertex_count; i++);
		if(0.01 < udg_get_select(i))
			udg_vertex_delete(i);
	udg_get_vertex_pos(vertex, collapse);
	udg_vertex_set(collapse, vertex, middle[0] / sum, middle[1] / sum, middle[2] / sum);
}


double la_t_compute_weld_distance(uint a, uint b)
{
	double vertex_a[3], vertex_b[3];
	udg_get_vertex_pos(vertex_a, a);
	udg_get_vertex_pos(vertex_b, b);
	if(vertex_a[0] != V_REAL64_MAX && vertex_b[0] != V_REAL64_MAX)
		if(0.01 < udg_get_select(a) && 0.01 < udg_get_select(b))
			return (vertex_a[0] - vertex_b[0]) * (vertex_a[0] - vertex_b[0]) + (vertex_a[1] - vertex_b[1]) * (vertex_a[1] - vertex_b[1]) + (vertex_a[2] - vertex_b[2]) * (vertex_a[2] - vertex_b[2]);
	return 10000000;
}

void la_t_weld_selected_vertexes(void)
{
	uint32 new_ref[4], vertex_count, polygon_count, *ref, poly, i, j, k;
	double f, best = 10000000;
	udg_get_geometry(&vertex_count, &polygon_count, NULL, &ref, NULL);

	for(i = 0; i < polygon_count; i++)
	{
		if(ref[i * 4] < vertex_count && ref[i * 4 + 1] < vertex_count && ref[i * 4 + 2] < vertex_count)
		{
			poly = 3;
			if(ref[i * 4 + 3] < vertex_count)
				poly = 4;
			for(j = 0; j < poly; j++)
			{
				f = la_t_compute_weld_distance(ref[i * 4 + j], ref[i * 4 + (j + 1) % poly]);
				if(f > 0 && f < best)
					best = f;
			}	
		}
	}
	best = sqrt(best) * 0.9;
	best *= best;
	for(i = 0; i < polygon_count; i++)
	{
		if(ref[i * 4] < vertex_count && ref[i * 4 + 1] < vertex_count && ref[i * 4 + 2] < vertex_count)
		{
			poly = 3;
			new_ref[3] = -1;
			if(ref[i * 4 + 3] < vertex_count)
				poly = 4;
			for(j = 0; j < poly; j++)
			{
				new_ref[j] = ref[i * 4 + j];
				for(k = 0; k < ref[i * 4 + j]; k++)
				{
					if(la_t_compute_weld_distance(ref[i * 4 + j], k) < best)
					{
						udg_vertex_delete(ref[i * 4 + j]);
						new_ref[j] = k;
						break;
					}
				}
			}
			for(j = 0; j < poly && ref[i * 4 + j] == new_ref[j]; j++);
			if(j != poly)
				udg_polygon_set(i, new_ref[0], new_ref[1], new_ref[2], new_ref[3]);
		}
	}
}
