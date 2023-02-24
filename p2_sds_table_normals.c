#define PERSUADE_INTERNAL
#include "persuade2.h"
#include "p2_sds_table.h"

void p_geo_table_test_angle(PTessTableElement *t, uint vertex, uint next, uint corners)
{
	pgreal v_h, v_v, n_h, n_v;
	if(t->edges[4] > vertex)
		return;

	v_h = t->vertex_influence[vertex * corners + 1] + t->vertex_influence[vertex * corners + 0];
	v_v = t->vertex_influence[vertex * corners + 1] + t->vertex_influence[vertex * corners + 2];

	n_h = t->vertex_influence[next * corners + 1] + t->vertex_influence[next * corners + 0];
	n_v = t->vertex_influence[next * corners + 1] + t->vertex_influence[next * corners + 2];


	if(v_h + 0.0001 > n_h && v_h - 0.0001 < n_h)
	{
		if(v_v > n_v)
			t->normals[vertex * 4 + 0] = next;
		else
			t->normals[vertex * 4 + 2] = next;

	}else if(v_v + 0.0001 > n_v && v_v - 0.0001 < n_v)
	{
		if(v_h > n_h)
			t->normals[vertex * 4 + 1] = next;	
		else
			t->normals[vertex * 4 + 3] = next;
	}
}

void p_geo_table_set_edge_normals(PTessTableElement *t)
{
	uint i;
	for(i = 0; i < t->element_count; i++)
	{
		if(t->index[i] < t->edges[4])
		{
			if(t->index[(i / 3) * 3 + (i + 2) % 3] == (t->index[i] + t->edges[4] - 1) % t->edges[4])
			{
				t->normals[t->index[i] * 4 + 2] = t->index[(i / 3) * 3 + (i + 2) % 3];
				t->normals[t->index[i] * 4 + 3] = t->index[(i / 3) * 3 + (i + 1) % 3];
			}
			if(t->index[(i / 3) * 3 + (i + 1) % 3] == (t->index[i] + t->edges[4] + 1) % t->edges[4])
			{
				t->normals[t->index[i] * 4 + 0] = t->index[(i / 3) * 3 + (i + 2) % 3];
				t->normals[t->index[i] * 4 + 1] = t->index[(i / 3) * 3 + (i + 1) % 3];
			}
		}
	}
}

void p_geo_table_gen_normals(PTessTableElement *t, uint corners)
{
	uint i, j;
	j = 0;
	t->normals = malloc((sizeof *t->normals) * t->vertex_count * 4);

	for(i = 0; i < t->vertex_count * 4; i++)
		t->normals[i] = i / 4;

	p_geo_table_set_edge_normals(t);
	for(i = 0; i < t->element_count; i += 3)
	{
		p_geo_table_test_angle(t, t->index[i + 0], t->index[i + 1], corners);
		p_geo_table_test_angle(t, t->index[i + 0], t->index[i + 2], corners);
		p_geo_table_test_angle(t, t->index[i + 1], t->index[i + 0], corners);
		p_geo_table_test_angle(t, t->index[i + 1], t->index[i + 2], corners);
		p_geo_table_test_angle(t, t->index[i + 2], t->index[i + 0], corners);
		p_geo_table_test_angle(t, t->index[i + 2], t->index[i + 1], corners);
	}
}
/*
void p_geo_table_tri_gen_normals(PTessTableElement *t)
{
	uint i, j;
	j = 0;
	t->normals = malloc((sizeof *t->normals) * t->vertex_count * 4);

	for(i = 0; i < t->vertex_count * 4; i++)
		t->normals[i] = i / 4;

	p_geo_table_set_edge_normals(t);
	for(i = 0; i < t->element_count; i += 3)
	{
		p_geo_table_test_quad_angle(t, t->index[i + 0], t->index[i + 1]);
		p_geo_table_test_quad_angle(t, t->index[i + 0], t->index[i + 2]);
		p_geo_table_test_quad_angle(t, t->index[i + 1], t->index[i + 0]);
		p_geo_table_test_quad_angle(t, t->index[i + 1], t->index[i + 2]);
		p_geo_table_test_quad_angle(t, t->index[i + 2], t->index[i + 0]);
		p_geo_table_test_quad_angle(t, t->index[i + 2], t->index[i + 1]);
	}
}*/
