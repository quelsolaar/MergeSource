#include "la_includes.h"

#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_draw_overlay.h"

struct{
	uint edge[6];
}GlobalReshapeData;

uint reshape_find_next(uint id, double *pos, double *vector)
{
	uint i, vertex_count, *ref, ref_count, next, poly, temp, found = -1;
	double test[3], r, best = -0.0;
	udg_get_geometry(&vertex_count, &ref_count, NULL, &ref, NULL);
	r = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
	vector[0] = vector[0] / r;
	vector[1] = vector[1] / r;
	vector[2] = vector[2] / r;
	ref_count *= 4;
	for(i = 0; i < ref_count; i++)
	{
		if(ref[i] == id)
		{
			temp = (i / 4) * 4;
			if(ref[temp + 3] > vertex_count)
				poly = 3;
			else
				poly = 4;
		
			next = ref[temp + (i + 1 - temp) % poly];
			if(next < vertex_count && next != found)
			{
				udg_get_vertex_pos(test, next);
				test[0] -= pos[0];
				test[1] -= pos[1];
				test[2] -= pos[2];
				r = sqrt(test[0] * test[0] + test[1] * test[1] + test[2] * test[2]);
				r = (test[0] / r) * vector[0] + (test[1] / r) * vector[1] + (test[2] / r) * vector[2];
				if(r < best)
				{
					best = r;
					found = next;
				}
			}
			next = ref[temp + (i + poly - 1 - temp) % poly];
			if(next < vertex_count && next != found)
			{
				udg_get_vertex_pos(test, next);
				test[0] -= pos[0];
				test[1] -= pos[1];
				test[2] -= pos[2];
				r = sqrt(test[0] * test[0] + test[1] * test[1] + test[2] * test[2]);
				r = (test[0] / r) * vector[0] + (test[1] / r) * vector[1] + (test[2] / r) * vector[2];
				if(r < best)
				{
					best = r;
					found = next;
				}
			}
		}
	}
	ref = udg_get_edge_data(&ref_count);
	ref_count *= 2;
	for(i = 0; i < ref_count; i++)
	{
		if(ref[i] == id)
		{
			next = ref[((i / 2) * 2) + (i + 1) % 2];
			if(next < vertex_count && next != found)
			{
				udg_get_vertex_pos(test, next);
				test[0] -= pos[0];
				test[1] -= pos[1];
				test[2] -= pos[2];
				r = sqrt(test[0] * test[0] + test[1] * test[1] + test[2] * test[2]);
				r = (test[0] / r) * vector[0] + (test[1] / r) * vector[1] + (test[2] / r) * vector[2];
				if(r < best)
				{
					best = r;
					found = next;
				}
			}
		}
	}
	return found;
}

void reshape_hull_place_vertex(uint id, double *pos, double *vector, double x, double y)
{
	double vertex[3], state[3], r;
	r = sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
	vector[0] = vector[0] / r;
	vector[1] = vector[1] / r;
	r = vector[1] * (-x + pos[0]) + -vector[0] * (-y + pos[1]);
	udg_get_vertex_pos(state, id);
	seduce_view_projection_vertexd(NULL, vertex, state, -pos[0] + vector[1] * r,  -pos[1] - vector[0] * r);
	udg_vertex_set(id, state, vertex[0], vertex[1], vertex[2]);
}

void la_t_reshape_hull_start(BInputState *input, uint *edge)
{
	double first[3], second[3], third[3], fourth[3], temp[3];

	GlobalReshapeData.edge[2] = edge[0];
	GlobalReshapeData.edge[3] = edge[1];
	udg_get_vertex_pos(second, GlobalReshapeData.edge[2]);
	udg_get_vertex_pos(third, GlobalReshapeData.edge[3]);
	temp[0] = second[0] - third[0];
	temp[1] = second[1] - third[1];
	temp[2] = second[2] - third[2];
	GlobalReshapeData.edge[4] = reshape_find_next(GlobalReshapeData.edge[3], third, temp);
	temp[0] = third[0] - second[0];
	temp[1] = third[1] - second[1];
	temp[2] = third[2] - second[2];
	GlobalReshapeData.edge[1] = reshape_find_next(GlobalReshapeData.edge[2], second, temp);
	if(GlobalReshapeData.edge[1] != -1)
	{
//		seduce_view_projection_screend(NULL, first, vertex[GlobalReshapeData.edge[1]].x, vertex[GlobalReshapeData.edge[1]].y, vertex[GlobalReshapeData.edge[1]].z);
		udg_get_vertex_pos(first, GlobalReshapeData.edge[1]);
		temp[0] = second[0] - first[0];
		temp[1] = second[1] - first[1];
		temp[2] = second[2] - first[2];
		GlobalReshapeData.edge[0] = reshape_find_next(GlobalReshapeData.edge[1], first, temp);
	}else
		GlobalReshapeData.edge[0] = -1;

	if(GlobalReshapeData.edge[4] != -1)
	{
//		seduce_view_projection_screend(NULL, fourth, vertex[GlobalReshapeData.edge[4]].x, vertex[GlobalReshapeData.edge[4]].y, vertex[GlobalReshapeData.edge[4]].z);
		udg_get_vertex_pos(fourth, GlobalReshapeData.edge[4]);
		temp[0] = third[0] - fourth[0];
		temp[1] = third[1] - fourth[1];
		temp[2] = third[2] - fourth[2];
		GlobalReshapeData.edge[5] = reshape_find_next(GlobalReshapeData.edge[4], fourth, temp);
	}else
		GlobalReshapeData.edge[5] = -1;
}



void la_t_reshape_hull(BInputState *input)
{
//	Point *vertex;
	double a[3], b[3], first[3], second[3], vector[3];

	udg_get_vertex_pos(first, GlobalReshapeData.edge[2]);
	seduce_view_projection_screend(NULL, first, first[0], first[1], first[2]);
	udg_get_vertex_pos(second, GlobalReshapeData.edge[3]);
	seduce_view_projection_screend(NULL, second, second[0], second[1], second[2]);
	if(GlobalReshapeData.edge[1] != -1)
	{
		udg_get_vertex_pos(a, GlobalReshapeData.edge[1]);
		seduce_view_projection_screend(NULL, a, a[0], a[1], a[2]);
		vector[0] = first[0] - a[0];
		vector[1] = first[1] - a[1];
		vector[2] = first[2] - a[2];
	}else
	{
		vector[0] = second[0] - first[0];
		vector[1] = second[1] - first[1];
		vector[2] = second[2] - first[2];
	}
	if(-vector[0] * (input->pointers[0].pointer_x - first[0]) + -vector[1] * (input->pointers[0].pointer_y - first[1]) > 0)
	{
		GlobalReshapeData.edge[5] = GlobalReshapeData.edge[4];
		GlobalReshapeData.edge[4] = GlobalReshapeData.edge[3];
		GlobalReshapeData.edge[3] = GlobalReshapeData.edge[2];
		GlobalReshapeData.edge[2] = GlobalReshapeData.edge[1];
		GlobalReshapeData.edge[1] = GlobalReshapeData.edge[0];
		reshape_hull_place_vertex(GlobalReshapeData.edge[3], first, vector, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
		if(GlobalReshapeData.edge[0] != -1)
		{
			udg_get_vertex_pos(a, GlobalReshapeData.edge[2]);
			udg_get_vertex_pos(b, GlobalReshapeData.edge[1]);
	//		p_get_projection_screen(b, b[0], b[1], b[2]);
			vector[0] = a[0] - b[0];
			vector[1] = a[1] - b[1];
			vector[2] = a[2] - b[2];
			GlobalReshapeData.edge[0] = reshape_find_next(GlobalReshapeData.edge[0], b, vector);
		}
	}else
	{
		if(GlobalReshapeData.edge[4] != -1)
		{
			udg_get_vertex_pos(a, GlobalReshapeData.edge[4]);
			seduce_view_projection_screend(NULL, a, a[0], a[1], a[2]);
			vector[0] = second[0] - a[0];
			vector[1] = second[1] - a[1];
			vector[2] = second[2] - a[2];
		}else
		{
			vector[0] = first[0] - second[0];
			vector[1] = first[1] - second[1];
			vector[2] = first[2] - second[2];
		}
		if(-vector[0] * (input->pointers[0].pointer_x - second[0]) + -vector[1] * (input->pointers[0].pointer_y - second[1]) > 0)
		{
			GlobalReshapeData.edge[0] = GlobalReshapeData.edge[1];
			GlobalReshapeData.edge[1] = GlobalReshapeData.edge[2];
			GlobalReshapeData.edge[2] = GlobalReshapeData.edge[3];
			GlobalReshapeData.edge[3] = GlobalReshapeData.edge[4];
			GlobalReshapeData.edge[4] = GlobalReshapeData.edge[5];
			reshape_hull_place_vertex(GlobalReshapeData.edge[2], second, vector, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
			if(GlobalReshapeData.edge[5] != -1)
			{
				udg_get_vertex_pos(a, GlobalReshapeData.edge[3]);
				udg_get_vertex_pos(b, GlobalReshapeData.edge[4]);
			//	p_get_projection_screen(b, b[0], b[1], b[2]);
				vector[0] = a[0] - b[0];
				vector[1] = a[1] - b[1];
				vector[2] = a[2] - b[2];
				GlobalReshapeData.edge[5] = reshape_find_next(GlobalReshapeData.edge[4], b, vector);
			}
		}
	}
}

void la_t_reshape_hull_draw(void)
{
	double *vertex, a[3], b[3], c[3], d[3], e[3], f[3];
	udg_get_geometry(NULL, NULL, &vertex, NULL, NULL);

	if(GlobalReshapeData.edge[0] != -1)
		seduce_view_projection_screend(NULL, a, vertex[GlobalReshapeData.edge[0] * 3], vertex[GlobalReshapeData.edge[0] * 3 + 1], vertex[GlobalReshapeData.edge[0] * 3 + 2]);
	if(GlobalReshapeData.edge[1] != -1)
		seduce_view_projection_screend(NULL, b, vertex[GlobalReshapeData.edge[1] * 3], vertex[GlobalReshapeData.edge[1] * 3 + 1], vertex[GlobalReshapeData.edge[1] * 3 + 2]);
	if(GlobalReshapeData.edge[2] != -1)
		seduce_view_projection_screend(NULL, c, vertex[GlobalReshapeData.edge[2] * 3], vertex[GlobalReshapeData.edge[2] * 3 + 1], vertex[GlobalReshapeData.edge[2] * 3 + 2]);
	if(GlobalReshapeData.edge[3] != -1)
		seduce_view_projection_screend(NULL, d, vertex[GlobalReshapeData.edge[3] * 3], vertex[GlobalReshapeData.edge[3] * 3 + 1], vertex[GlobalReshapeData.edge[3] * 3 + 2]);
	if(GlobalReshapeData.edge[4] != -1)
		seduce_view_projection_screend(NULL, e, vertex[GlobalReshapeData.edge[4] * 3], vertex[GlobalReshapeData.edge[4] * 3 + 1], vertex[GlobalReshapeData.edge[4] * 3 + 2]);
	if(GlobalReshapeData.edge[5] != -1)
		seduce_view_projection_screend(NULL, f, vertex[GlobalReshapeData.edge[5] * 3], vertex[GlobalReshapeData.edge[5] * 3 + 1], vertex[GlobalReshapeData.edge[5] * 3 + 2]);

/*	if(GlobalReshapeData.edge[0] != -1 && GlobalReshapeData.edge[1] != -1)
		la_do_edge_select(a, b);	
	if(GlobalReshapeData.edge[1] != -1 && GlobalReshapeData.edge[2] != -1)
		la_do_edge_select(b, c);	
	if(GlobalReshapeData.edge[2] != -1 && GlobalReshapeData.edge[3] != -1)
		la_do_edge_select(c, d);	
	if(GlobalReshapeData.edge[3] != -1 && GlobalReshapeData.edge[4] != -1)
		la_do_edge_select(d, e);	
	if(GlobalReshapeData.edge[4] != -1 && GlobalReshapeData.edge[5] != -1)
		la_do_edge_select(e, f);	*/

}


void la_t_select_hull(uint *edge)
{
	uint count, i, a, b;
	boolean *select;
	double a_cur[3], b_cur[3], a_last[3], b_last[3], vector[3];
	udg_get_geometry(&count, NULL, NULL, NULL, NULL);
	select = malloc((sizeof *select) * count);
	for(i = 0; i < count; i++)
		select[i] = FALSE;
	a = edge[0];
	b = edge[1];
	select[a] = TRUE;
	select[b] = TRUE;
	udg_get_vertex_pos(a_cur, edge[0]);
	udg_get_vertex_pos(b_last, edge[0]);
	udg_get_vertex_pos(b_cur, edge[1]);
	udg_get_vertex_pos(a_last, edge[1]);
	while(a < count || b < count)
	{
		if(a < count)
		{
			vector[0] = -(a_cur[0] - a_last[0]);
			vector[1] = -(a_cur[1] - a_last[1]);
			vector[2] = -(a_cur[2] - a_last[2]);
			a = reshape_find_next(a, a_cur, vector);
			if(a < count)
			{
				if(select[a] == TRUE)
					a = -1;
				else
				{
					a_last[0] = a_cur[0];
					a_last[1] = a_cur[1];
					a_last[2] = a_cur[2];
					udg_get_vertex_pos(a_cur, a);
					select[a] = TRUE;
				}
			}
		}
		if(b < count)
		{
			vector[0] = -(b_cur[0] - b_last[0]);
			vector[1] = -(b_cur[1] - b_last[1]);
			vector[2] = -(b_cur[2] - b_last[2]);
			b = reshape_find_next(b, b_cur, vector);
			if(b < count)
			{
				if(select[b] == TRUE)
					b = -1;
				else
				{
					b_last[0] = b_cur[0];
					b_last[1] = b_cur[1];
					b_last[2] = b_cur[2];
					udg_get_vertex_pos(b_cur, b);
					select[b] = TRUE;
				}
			}
		}
	}

	for(i = 0; i < count; i++)
		if(select[i] == TRUE)
			udg_set_select(i, 1);
	free(select);
}



