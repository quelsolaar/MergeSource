#include "la_includes.h"
#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_tool.h"


void me_create_tree()
{
	
	ENode *g_node;
	float *uv, space[2 * 4 * 4] = {0.5, 0.5, 1.0, 0.5, 1.0, 1.0, 0.5, 1.0, 0.5, 0.5, 0.0, 0.5, 0.0, 1.0, 0.5, 1.0, 0.5, 0.5, 1.0, 0.5, 1.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0};
	uint i, vertex[4], polygon, node_id;
	uint16 u, v;
	node_id = udg_get_modeling_node();
	printf("A");
	if((g_node = e_ns_get_node(0, udg_get_modeling_node())) == NULL)
		return;
	printf("B");
	if(NULL == e_nsg_get_layer_by_name(g_node, "map_u"))
		return;
	printf("C");
	u = e_nsg_get_layer_id(e_nsg_get_layer_by_name(g_node, "map_u"));
	if(NULL == e_nsg_get_layer_by_name(g_node, "map_v"))
		return;
	printf("D");
	v = e_nsg_get_layer_id(e_nsg_get_layer_by_name(g_node, "map_v"));
	for(i = 0; i < 5 * 5; i++)
	{
		uv = &space[2 * 4 * (i % 4)];
		vertex[0] = udg_find_empty_slot_vertex(); 
		vertex[1] = udg_find_empty_slot_vertex(); 
		vertex[2] = udg_find_empty_slot_vertex(); 
		vertex[3] = udg_find_empty_slot_vertex(); 
		udg_vertex_set(vertex[0], NULL, 0.0, (float)i * 0.1 + 0.03, 0.0);
		udg_vertex_set(vertex[1], NULL, 0.4, (float)i * 0.1, 0.0);
		udg_vertex_set(vertex[2], NULL, 0.4, (float)i * 0.1, 0.4);
		udg_vertex_set(vertex[3], NULL, 0.0, (float)i * 0.1, 0.4);
		polygon = udg_find_empty_slot_polygon();
		udg_polygon_set(polygon, vertex[0], vertex[1], vertex[2], -1);
		verse_send_g_polygon_set_corner_real64(node_id, u, polygon, uv[0 * 2 + 0], uv[1 * 2 + 0], uv[2 * 2 + 0], 0);
		verse_send_g_polygon_set_corner_real64(node_id, v, polygon, uv[0 * 2 + 1], uv[1 * 2 + 1], uv[2 * 2 + 1], 0);
		polygon = udg_find_empty_slot_polygon();
		udg_polygon_set(polygon, vertex[0], vertex[2], vertex[1], -1);
		verse_send_g_polygon_set_corner_real64(node_id, u, polygon, uv[0 * 2 + 0], uv[2 * 2 + 0], uv[1 * 2 + 0], 0);
		verse_send_g_polygon_set_corner_real64(node_id, v, polygon, uv[0 * 2 + 1], uv[2 * 2 + 1], uv[1 * 2 + 1], 0);
		polygon = udg_find_empty_slot_polygon();
		udg_polygon_set(polygon, vertex[0], vertex[2], vertex[3], -1);
		verse_send_g_polygon_set_corner_real64(node_id, u, polygon, uv[0 * 2 + 0], uv[2 * 2 + 0], uv[3 * 2 + 0], 0);
		verse_send_g_polygon_set_corner_real64(node_id, v, polygon, uv[0 * 2 + 1], uv[2 * 2 + 1], uv[3 * 2 + 1], 0);
		polygon = udg_find_empty_slot_polygon();
		udg_polygon_set(polygon, vertex[0], vertex[3], vertex[2], -1);
		verse_send_g_polygon_set_corner_real64(node_id, u, polygon, uv[0 * 2 + 0], uv[3 * 2 + 0], uv[2 * 2 + 0], 0);
		verse_send_g_polygon_set_corner_real64(node_id, v, polygon, uv[0 * 2 + 1], uv[3 * 2 + 1], uv[2 * 2 + 1], 0);
	}
}

void la_t_poly_compute_normal(double *normal, egreal *vertex, uint *ref)
{
	egreal r, v0[3], v1[3];
	v0[0] = vertex[ref[1] * 3 + 0] - vertex[ref[0] * 3 + 0];
	v0[1] = vertex[ref[1] * 3 + 1] - vertex[ref[0] * 3 + 1];
	v0[2] = vertex[ref[1] * 3 + 2] - vertex[ref[0] * 3 + 2];
	r = sqrt(v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2]);
	v0[0] /= r;
	v0[1] /= r;
	v0[2] /= r;
	v1[0] = vertex[ref[2] * 3 + 0] - vertex[ref[0] * 3 + 0];
	v1[1] = vertex[ref[2] * 3 + 1] - vertex[ref[0] * 3 + 1];
	v1[2] = vertex[ref[2] * 3 + 2] - vertex[ref[0] * 3 + 2];
	r = sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]);
	v1[0] /= r;
	v1[1] /= r;
	v1[2] /= r;
	normal[0] = v0[1] * v1[2] - v0[2] * v1[1];
	normal[1] = v0[2] * v1[0] - v0[0] * v1[2];
	normal[2] = v0[0] * v1[1] - v0[1] * v1[0];
	r = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= r;
	normal[1] /= r;
	normal[2] /= r;
}



boolean la_t_poly_surface(uint8 *list, uint polygon, uint *ref, uint *n, uint *crease, double *vertex, uint vertex_count, double *normal, LAPolySurfaceSeletcType type)
{
	uint i, poly, next;
	double vec[3], pos[3];
	boolean found = FALSE;
	if(ref[polygon * 4 + 3] < vertex_count && vertex[ref[polygon * 4 + 3] * 3] != V_REAL64_MAX)
		poly = 4;
	else
		poly = 3;
	if(type == LA_PSST_CONVEX)
	{
		pos[0] = vertex[ref[polygon * 4] * 3];
		pos[1] = vertex[ref[polygon * 4] * 3 + 1];
		pos[2] = vertex[ref[polygon * 4] * 3 + 2];
	}

	for(i = 0; i < poly; i++)
	{
		next = n[polygon * 4 + i];
		if(next != -1 && crease[polygon * 4 + i] < 4294967295 / 2)
		{
			next /= 4;;
			if(list[next] == 0)
			{
				switch(type)
				{
					case LA_PSST_PLANE :
					f_normal3d(vec, &vertex[ref[next * 4] * 3], &vertex[ref[next * 4 + 1] * 3], &vertex[ref[next * 4 + 2] * 3]);
					f_normalize3d(vec);
					if(vec[0] * normal[0] + vec[1] * normal[1] + vec[2] * normal[2] > 0.99)
					{
						list[next] = 1;
						found = TRUE;
					}
					break;
					case LA_PSST_SMOOTH :
						list[next] = 1;
						found = TRUE;
					break;
					case LA_PSST_CONVEX :
						if(ref[next * 4 + 3] < vertex_count && vertex[ref[next * 4 + 3] * 3] != V_REAL64_MAX)
						{
							next = n[polygon * 4 + i];
							next = (next / 4) * 4 + (next + 2) % 4;
							vec[0] = vertex[ref[next] * 3] - pos[0];
							vec[1] = vertex[ref[next] * 3 + 1] - pos[1];
							vec[2] = vertex[ref[next] * 3 + 2] - pos[2];
							if(vec[0] * normal[0] + vec[1] * normal[1] + vec[2] * normal[2] > -0.01)
							{
								next = (next / 4) * 4 + (next + 1) % 4;
								vec[0] = vertex[ref[next] * 3] - pos[0];
								vec[1] = vertex[ref[next] * 3 + 1] - pos[1];
								vec[2] = vertex[ref[next] * 3 + 2] - pos[2];
								if(vec[0] * normal[0] + vec[1] * normal[1] + vec[2] * normal[2] > -0.01)
								{
									list[next / 4] = 1;
									found = TRUE;
								}
							}
						}else
						{
							next = n[polygon * 4 + i];
							next = (next / 4) * 4 + ((next % 4) + 2) % 3;
							vec[0] = vertex[ref[next] * 3] - pos[0];
							vec[1] = vertex[ref[next] * 3 + 1] - pos[1];
							vec[2] = vertex[ref[next] * 3 + 2] - pos[2];
							if(vec[0] * normal[0] + vec[1] * normal[1] + vec[2] * normal[2] > -0.01)
							{
								list[next / 4] = 1;
								found = TRUE;
							}
						}
					break;
				}

			}
		}
	}
	return found;
}

extern uint *la_compute_neighbor(uint *ref, uint ref_count, uint vertex_count, egreal *vertex);
/*
typedef enum{
	LA_PSST_PLANE,
	LA_PSST_SMOOTH,
	LA_PSST_CONVEX,
	LA_PSST_COUNT
}LAPolySurfaceSeletcType;*/

void la_t_poly_surface_select(uint polygon, LAPolySurfaceSeletcType type)
{
	uint i, j, poly, ref_count, *ref, vertex_count, *n, *crease;
	double *vertex, normal[3];
	uint8 *list, *vertex_list;
	boolean found = TRUE;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, &crease);
	n = la_compute_neighbor(ref, ref_count, vertex_count, vertex);
	list = malloc((sizeof *list) * ref_count);
	vertex_list = malloc((sizeof *vertex_list) * vertex_count);
	for(i = 0; i < vertex_count; i++)
		vertex_list[i] = 0;
	for(i = 0; i < ref_count; i++)
		list[i] = 0;


	for(i = 0; i < ref_count * 4; i++)
		if(n[i] != -1)
			j = 0;
	list[polygon] = 2;
	if(type != LA_PSST_SMOOTH)
		crease = ref; 
	f_normal3d(normal, &vertex[ref[polygon * 4] * 3], &vertex[ref[polygon * 4 + 1] * 3], &vertex[ref[polygon * 4 + 2] * 3]);
	f_normalize3d(normal);
	if(la_t_poly_surface(list, polygon, ref, n, crease, vertex, vertex_count, normal, type))
	{
		while(found)
		{
			found = FALSE;
			for(i = 0; i < ref_count; i++)
			{
				if(list[i] == 1)
				{
					list[i] = 2;
					if(type == LA_PSST_PLANE)
					{
						if(la_t_poly_surface(list, i, ref, n, crease, vertex, vertex_count, normal, type))
							found = TRUE;
					}else
					{
						f_normal3d(normal, &vertex[ref[i * 4] * 3], &vertex[ref[i * 4 + 1] * 3], &vertex[ref[i * 4 + 2] * 3]);
						f_normalize3d(normal);
						if(la_t_poly_surface(list, i, ref, n, crease, vertex, vertex_count, normal, type))
							found = TRUE;	
					}
				}
			}			
		}
	}
	for(i = 0; i < ref_count; i += 4)
		if(list[i / 4] != 0)
			for(j = 0; j < 4; j++)
				if(ref[i + j] < vertex_count)
					vertex_list[ref[i + j]] = TRUE;


	for(i = 0; i < vertex_count; i++)
		if(vertex_list[i])
			udg_set_select(i, 1.0);
	free(vertex_list);
	free(list);
}



uint la_t_poly_egde_test(uint polygon, double x, double y)
{
	uint i, j, j2, poly, found, ref_count, *ref, vertex_count;
	double *vertex, temp[4][3], mouse[2], value, center[3];
	boolean test[4];
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);

	j = ref[polygon * 4] * 3;
	seduce_view_projection_screend(NULL, &temp[0][0], vertex[j], vertex[j + 1], vertex[j + 2]);
	j = ref[polygon * 4 + 1] * 3;
	seduce_view_projection_screend(NULL, &temp[1][0], vertex[j], vertex[j + 1], vertex[j + 2]);
	j = ref[polygon * 4 + 2] * 3;
	seduce_view_projection_screend(NULL, &temp[2][0], vertex[j], vertex[j + 1], vertex[j + 2]);
	center[0] = temp[0][0] + temp[1][0] + temp[2][0];
	center[1] = temp[0][1] + temp[1][1] + temp[2][1];
	center[2] = temp[0][2] + temp[1][2] + temp[2][2];
	if(ref[polygon * 4 + 3] < vertex_count && vertex[ref[polygon * 4 + 3] * 3] != V_REAL64_MAX)
	{
		j = ref[polygon * 4 + 3] * 3;
		seduce_view_projection_screend(NULL, &temp[3][0], vertex[j], vertex[j + 1], vertex[j + 2]);
		center[0] += temp[3][0];
		center[1] += temp[3][1];
		center[2] += temp[3][2];
		poly = 4;
	}else
		poly = 3;
	center[0] /= (double)poly;
	center[1] /= (double)poly;
	center[2] /= (double)poly;
	for(j = 0; j < poly; j++)
		test[j] = 0 < (x - center[0]) * (temp[j][1] - center[1]) + (y - center[1]) * (center[0] - temp[j][0]);
	poly--;
	for(j = 0; j < poly; j++)
		if(test[j] && !test[j + 1])
			return j;
	return poly;
}

void la_t_poly_select(uint polygon)
{
	uint *ref, vertex_count;
	double value = 1, *vertex, center[3];
	udg_get_geometry(&vertex_count, NULL, &vertex, &ref, NULL);
	ref += polygon * 4;		
	if(udg_get_select(ref[0]) > 0.5 && 	udg_get_select(ref[1]) > 0.5 &&	udg_get_select(ref[2]) > 0.5 &&	(ref[3] >= vertex_count || udg_get_select(ref[3]) > 0.5))
		value = 0;
	udg_set_select(ref[0], value);
	udg_set_select(ref[1], value);
	udg_set_select(ref[2], value);
	if(ref[3] < vertex_count)
	{
		udg_set_select(ref[3], value);
		if(la_t_tm_hiden())
		{
			center[0] = (vertex[ref[0] * 3 + 0] + vertex[ref[1] * 3 + 0] + vertex[ref[2] * 3 + 0] + vertex[ref[3] * 3 + 0]) / 4.0;
			center[1] = (vertex[ref[0] * 3 + 1] + vertex[ref[1] * 3 + 1] + vertex[ref[2] * 3 + 1] + vertex[ref[3] * 3 + 1]) / 4.0;
			center[2] = (vertex[ref[0] * 3 + 2] + vertex[ref[1] * 3 + 2] + vertex[ref[2] * 3 + 2] + vertex[ref[3] * 3 + 2]) / 4.0;
			la_t_tm_place(center[0], center[1], center[2]);
		}
	}else if(la_t_tm_hiden())
	{
		center[0] = (vertex[ref[0] * 3 + 0] + vertex[ref[1] * 3 + 0] + vertex[ref[2] * 3 + 0]) / 3.0;
		center[1] = (vertex[ref[0] * 3 + 1] + vertex[ref[1] * 3 + 1] + vertex[ref[2] * 3 + 1]) / 3.0;
		center[2] = (vertex[ref[0] * 3 + 2] + vertex[ref[1] * 3 + 2] + vertex[ref[2] * 3 + 2]) / 3.0;
		la_t_tm_place(center[0], center[1], center[2]);
	}

}

void la_t_smooth_select(void)
{
	uint i, *count, ref_count, *ref, vertex_count;
	double *vertex, *value, temp;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);
	ref_count *= 4;
	count = malloc((sizeof *count) * vertex_count);
	value = malloc((sizeof *value) * vertex_count);
	for(i = 0; i < vertex_count; i++)
	{
		count[i] = 1;
		value[i] = 0;
	}
	for(i = 0; i < ref_count; i += 4)
	{
		if(ref[i] < vertex_count)
		{
			temp = udg_get_select(ref[i]) + udg_get_select(ref[i + 1]) + udg_get_select(ref[i + 2]);
			if(ref[i + 3] < vertex_count)
				temp = (temp + udg_get_select(ref[i + 3])) / 4;
			else
				temp = temp / 3;
			count[ref[i]]++;
			value[ref[i]] += temp;
			count[ref[i + 1]]++;
			value[ref[i + 1]] += temp;
			count[ref[i + 2]]++;
			value[ref[i + 2]] += temp;
			if(ref[i + 3] < vertex_count)
			{
				count[ref[i + 3]]++;
				value[ref[i + 3]] += temp;
			}
		}
	}
	temp = 0;
	for(i = 0; i < vertex_count; i++)
	{
		value[i] /= (double)count[i];
		if(temp < value[i])
			temp = value[i];
	}

	for(i = 0; i < vertex_count; i++)
		if(vertex[i * 3] != V_REAL64_MAX)
			if(value[i] / temp < udg_get_select(i) - 0.0001 || value[i] / temp > udg_get_select(i) + 0.0001)
				udg_set_select(i, value[i] / temp);
	free(count);
	free(value);
}


void la_t_revert_to_base(void)
{
	uint i, vertex_count;
	double *vertex, *base, f;
	udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);
	base = udg_get_base_layer();
	if(base != NULL)
	{
		for(i = 0; i < vertex_count; i++)
		{
			f = udg_get_select(i);
			if(f > 0.01)
				udg_vertex_set(i, NULL, base[i * 3 + 0] * f + vertex[i * 3 + 0] * (1.0 - f),
										base[i * 3 + 1] * f + vertex[i * 3 + 1] * (1.0 - f),
										base[i * 3 + 2] * f + vertex[i * 3 + 2] * (1.0 - f));
		}
	}
}
