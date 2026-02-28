#define PERSUADE_INTERNAL
#include <math.h>
#include <stdlib.h>
#include "persuade2.h"
#include "p2_sds_geo.h"
#include "p2_sds_table.h"
#include "p2_sds_obj.h"

pgreal p_sds_edge_tesselation_global_func(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eay)
{
	pgreal length, edge, vector[3];
	length = ((v_0[0] - v_1[0]) * (v_0[0] - v_1[0])) + ((v_0[1] - v_1[1]) * (v_0[1] - v_1[1])) + ((v_0[2] - v_1[2]) * (v_0[2] - v_1[2]));

	vector[0] = (e_0[0] - ((v_0[0] + v_1[0]) * 0.5));
	vector[1] = (e_0[1] - ((v_0[1] + v_1[1]) * 0.5));
	vector[2] = (e_0[2] - ((v_0[2] + v_1[2]) * 0.5));
	edge = (vector[0] * vector[0]) + (vector[1] * vector[1]) + (vector[2] * vector[2]);
	return /*sqrt(*/edge / length/*)*/;
}

extern float	p_geo_get_sds_mesh_factor(void);

pgreal p_sds_edge_tesselation_global_func_new(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eye, pgreal factor)
{
	pgreal length, dist, edge[3], vector[3];

	vector[0] = eye[0] - e_0[0];
	vector[1] = eye[1] - e_0[1];
	vector[2] = eye[2] - e_0[2];

	dist = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

	vector[0] = v_0[0] - v_1[0];
	vector[1] = v_0[1] - v_1[1];
	vector[2] = v_0[2] - v_1[2];

	length = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
	vector[0] /= length;
	vector[1] /= length;
	vector[2] /= length;
	
	factor = factor * length / dist;

	edge[0] = e_0[0] - v_1[0];
	edge[1] = e_0[1] - v_1[1];
	edge[2] = e_0[2] - v_1[2];

	length = vector[0] * edge[0] + vector[1] * edge[1] + vector[2] * edge[2];
	
	edge[0] = (edge[0] - vector[0] * length) * factor;
	edge[1] = (edge[1] - vector[1] * length) * factor;
	edge[2] = (edge[2] - vector[2] * length) * factor;
	length = (edge[0] * edge[0]) + (edge[1] * edge[1]) + (edge[2] * edge[2]);
	return length;
}


pgreal p_sds_edge_tesselation_global_func_new_old(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eay, pgreal factor)
{
	pgreal length, edge[3], vector[3];
	vector[0] = v_0[0] - v_1[0];
	vector[1] = v_0[1] - v_1[1];
	vector[2] = v_0[2] - v_1[2];

	length = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

	vector[0] /= length;
	vector[1] /= length;
	vector[2] /= length;
	
	edge[0] = e_0[0] - v_1[0];
	edge[1] = e_0[1] - v_1[1];
	edge[2] = e_0[2] - v_1[2];

	length = vector[0] * edge[0] + vector[1] * edge[1] + vector[2] * edge[2];
	
	edge[0] = (edge[0] - vector[0] * length) * factor;
	edge[1] = (edge[1] - vector[1] * length) * factor;
	edge[2] = (edge[2] - vector[2] * length) * factor;
	length = (edge[0] * edge[0]) + (edge[1] * edge[1]) + (edge[2] * edge[2]);
	return length;
}

pgreal p_sds_edge_tesselation_test_func(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eay)
{
	pgreal v_a[3], v_b[3], f;

	if(v_0[0] + v_0[1] > v_1[0] + v_1[1])
	{
		v_a[0] = (v_1[0] - v_0[0]);
		v_a[1] = (v_1[1] - v_0[1]);
		v_a[2] = (v_1[2] - v_0[2]);

		f = sqrt(v_a[0] * v_a[0] + v_a[1] * v_a[1] + v_a[2] * v_a[2]);

		v_a[0] /= f; 
		v_a[1] /= f; 
		v_a[2] /= f; 

		v_b[0] = (e_0[0] - v_0[0]);
		v_b[1] = (e_0[1] - v_0[1]);
		v_b[2] = (e_0[2] - v_0[2]);

		f = v_a[0] * v_b[0] + v_a[1] * v_b[1] + v_a[2] * v_b[2];

		v_b[0] += v_a[0] * f;
		v_b[1] += v_a[1] * f;
		v_b[2] += v_a[2] * f;

		return v_b[0] * v_b[0] + v_b[1] * v_b[1] + v_b[2] * v_b[2];
	}else
	{
		v_a[0] = (v_0[0] - v_1[0]);
		v_a[1] = (v_0[1] - v_1[1]);
		v_a[2] = (v_0[2] - v_1[2]);

		f = sqrt(v_a[0] * v_a[0] + v_a[1] * v_a[1] + v_a[2] * v_a[2]);

		v_a[0] /= f; 
		v_a[1] /= f; 
		v_a[2] /= f; 

		v_b[0] = (e_0[0] - v_1[0]);
		v_b[1] = (e_0[1] - v_1[1]);
		v_b[2] = (e_0[2] - v_1[2]);

		f = v_a[0] * v_b[0] + v_a[1] * v_b[1] + v_a[2] * v_b[2];

		v_b[0] += v_a[0] * f;
		v_b[1] += v_a[1] * f;
		v_b[2] += v_a[2] * f;

		return v_b[0] * v_b[0] + v_b[1] * v_b[1] + v_b[2] * v_b[2];
	}
}

pgreal p_sds_edge_tesselation_local_func(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eye)
{
	pgreal length, edge, dist, vector[3];
	length = ((v_0[0] - v_1[0]) * (v_0[0] - v_1[0])) + ((v_0[1] - v_1[1]) * (v_0[1] - v_1[1])) + ((v_0[2] - v_1[2]) * (v_0[2] - v_1[2]));

	vector[0] = (e_0[0] - ((v_0[0] + v_1[0]) * 0.5));
	vector[1] = (e_0[1] - ((v_0[1] + v_1[1]) * 0.5));
	vector[2] = (e_0[2] - ((v_0[2] + v_1[2]) * 0.5));
	edge = (vector[0] * vector[0]) + (vector[1] * vector[1]) + (vector[2] * vector[2]);
	vector[0] = (e_0[0] + eye[0]);
	vector[1] = (e_0[1] + eye[1]);
	vector[2] = (e_0[2] + eye[2]);
	dist = 1 / ((vector[0] * vector[0]) + (vector[1] * vector[1]) + (vector[2] * vector[2]));
	return sqrt((edge / length) * dist);
}
/*

PTessTableElement	*p_select_tri_tesselation(const PPolyStore *mesh, const pgreal *vertex, uint *index, uint polygon_times_size_times_three, pgreal tess_factor, uint force_tess, pgreal (*edge_tess_func)(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eay), pgreal *eay)
{
	PDepend *dep;
	uint i, j, pos, edge[4];
	pgreal value, edge_data[3 * 6];
	for(i = 0; i < 6; i++)
	{
		dep = &mesh->vertex_dependency[mesh->reference[index[i] + polygon_times_size_times_three]];
		pos = dep->element[0].vertex_ref * 3;
		edge_data[i * 3] = value * vertex[pos + 0];
		edge_data[i * 3 + 1] = value * vertex[pos + 1];
		edge_data[i * 3 + 2] = value * vertex[pos + 2];
		for(j = 1; j < dep->length; j++)
		{
			value = dep->element[j].value;
			pos = dep->element[j].vertex_ref * 3;
			edge_data[i * 3] += value * vertex[pos + 0];
			edge_data[i * 3 + 1] += value * vertex[pos + 1];
			edge_data[i * 3 + 2] += value * vertex[pos + 2];
		}
	}
		edge[0] = edge_tess_func(&edge_data[0], &edge_data[5], &edge_data[1], eay) * tess_factor;
		edge[1] = edge_tess_func(&edge_data[5], &edge_data[4], &edge_data[3], eay) * tess_factor;
		edge[2] = edge_tess_func(&edge_data[4], &edge_data[0], &edge_data[2], eay) * tess_factor;
	if(edge[0] < force_tess)
		edge[0] = force_tess;
	if(edge[1] < force_tess)
		edge[1] = force_tess;
	if(edge[2] < force_tess)
		edge[2] = force_tess;
	if(edge[0] > mesh->base_level)
		edge[0] = mesh->base_level;
	if(edge[1] > mesh->base_level)
		edge[1] = mesh->base_level;
	if(edge[2] > mesh->base_level)
		edge[2] = mesh->base_level;
	return get_dynamic_table_tri(mesh->base_level, edge);
}

PTessTableElement	*p_select_quad_tesselation(const PPolyStore *mesh, const pgreal *vertex, uint *index, uint polygon_times_size_times_four, pgreal tess_factor, uint force_tess, pgreal (*edge_tess_func)(pgreal *v_0, pgreal *v_1, pgreal *e_0, pgreal *eay), pgreal *eay)
{
	PDepend *dep;
	uint i, j, pos, edge[4];
	pgreal value, edge_data[3 * 9];
//	printf("looking for an edge\n");
	for(i = 0; i < 8; i++)
	{
		dep = &mesh->vertex_depend_temp[mesh->quad_reference[polygon_times_size_times_four + index[i]]];
		value = dep->element[0].value;
		pos = dep->element[0].vertex_ref * 3;
		edge_data[i * 3] = value * vertex[pos + 0];
		edge_data[i * 3 + 1] = value * vertex[pos + 1];
		edge_data[i * 3 + 2] = value * vertex[pos + 2];
//		printf("ref = %u %u %u ", polygon_times_size_times_four, index[i], mesh->quad_reference[polygon_times_size_times_four + index[i]]);
		for(j = 1; j < dep->length; j++)
		{
			value = dep->element[j].value;
			pos = dep->element[j].vertex_ref * 3;
			edge_data[i * 3] += value * vertex[pos + 0];
			edge_data[i * 3 + 1] += value * vertex[pos + 1];
			edge_data[i * 3 + 2] += value * vertex[pos + 2];

		}
//		printf("pos = %i, %f %f %f\n", i, edge_data[i * 3], edge_data[i * 3 + 1], edge_data[i * 3 + 2]);
	}
	edge[0] = edge_tess_func(&edge_data[0 * 3], &edge_data[3 * 3], &edge_data[2 * 3], eay) * tess_factor;
	edge[1] = edge_tess_func(&edge_data[3 * 3], &edge_data[7 * 3], &edge_data[4 * 3], eay) * tess_factor;
	edge[2] = edge_tess_func(&edge_data[7 * 3], &edge_data[5 * 3], &edge_data[6 * 3], eay) * tess_factor;
	edge[3] = edge_tess_func(&edge_data[5 * 3], &edge_data[0 * 3], &edge_data[1 * 3], eay) * tess_factor;

	if(edge[0] < force_tess)
		edge[0] = force_tess;
	if(edge[1] < force_tess)
		edge[1] = force_tess;
	if(edge[2] < force_tess)
		edge[2] = force_tess;
	if(edge[3] < force_tess)
		edge[3] = force_tess;
	if(edge[0] > mesh->base_level)
		edge[0] = mesh->base_level;
	if(edge[1] > mesh->base_level)
		edge[1] = mesh->base_level;
	if(edge[2] > mesh->base_level)
		edge[2] = mesh->base_level;
	if(edge[3] > mesh->base_level)
		edge[3] = mesh->base_level;
	return get_dynamic_table_quad(mesh->base_level, edge);
}
*/



/*
				if(mesh->sub_stages[0] == mesh->render.mat[mesh->sub_stages[1]].tri_end)
						mesh->sub_stages[1]++;
					if(mesh->sub_stages[0] < mesh->render.mat[mesh->sub_stages[1]].quad_end)
						poly = mesh->sub_stages[0] * smesh->poly_per_base * 4;
					else
						poly = smesh->quad_length / 4 + mesh->sub_stages[0] * smesh->poly_per_base * 3;

					for(j = 0; j < table->vertex_count; j++)
					{
						
						dep = &smesh->vertex_dependency[smesh->ref[table->reference[j] + poly]];
						mesh->depend.ref_count[mesh->render.vertex_count++] = dep->length;
						for(k = 0; k < dep->length; k++)
						{
							mesh->depend.reference[mesh->depend.length] = dep->element[k].vertex * 3;
							mesh->depend.weight[mesh->depend.length] = dep->element[k].value;
							mesh->depend.length++;
						}
					}
*/

static double view_cam_lod_eye_pos[3];
static double view_cam_lod_eye_matrix[16];
static double view_cam_lod_factor = 0.01;
static double view_cam_lod_limit = 1.5;
static double view_cam_lod_geometry_only = 1000;

void p_lod_set_view_pos(double *view_cam)
{
	view_cam_lod_eye_pos[0] = view_cam[0];
	view_cam_lod_eye_pos[1] = view_cam[1];
	view_cam_lod_eye_pos[2] = view_cam[2];
}
void p_lod_set_view_matrix(double *view_matrix)
{
	uint i;
	for(i = 0; i < 16; i++)
		view_cam_lod_eye_matrix[i] = view_matrix[i];
}

double *p_lod_get_view_pos(void)
{
	return view_cam_lod_eye_pos;
}

double *p_lod_get_view_matrix(void)
{
	return view_cam_lod_eye_matrix;
}
/*
pgreal p_lod_compute_lod_level(ENode *o_node, ENode *g_node, uint32 time_s, uint32 time_f)
{
	double tmp[3], f;
	pgreal high_x, low_x, high_y, low_y, high_z, low_z;
	if(o_node == NULL)
		return view_cam_lod_geometry_only;
	e_nso_get_scale(o_node, tmp);
	high_x = 1;
	low_x = -1;
	high_y = 1;
	low_y = -1;
	high_z = 1;
	low_z = -1;
	e_nsg_get_bounding_box(g_node, &high_x, &low_x, &high_y, &low_y, &high_z, &low_z);
	f = high_x * tmp[0];
	if(high_y * tmp[1] > f)
		f = high_y * tmp[1];
	if(high_z * tmp[2] > f)
		f = high_z * tmp[2];
	if(-low_x * tmp[0] > f)
		f = -low_x * tmp[0];
	if(-low_y * tmp[1] > f)
		f = -low_y * tmp[1];
	if(-low_z * tmp[2] > f)
		f = -low_z * tmp[2];
	e_nso_get_pos_time(o_node, tmp, time_s, time_f);
	tmp[0] = (tmp[0] - view_cam_lod_eye_pos[0]) / f;
	tmp[1] = (tmp[1] - view_cam_lod_eye_pos[1]) / f;
	tmp[2] = (tmp[2] - view_cam_lod_eye_pos[2]) / f;	
	f = sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]);
	if(f < 3)
		f = 3;
	f = 1.0 / f;
	return f;
}*/
/*
boolean p_lod_compute_lod_update(ENode *o_node, ENode *g_node, uint32 time_s, uint32 time_f, pgreal factor)
{
	double f;
	if(o_node == NULL)
		return FALSE;
	f = p_lod_compute_lod_level(o_node, g_node, time_s, time_f);
	if(f * (view_cam_lod_limit + view_cam_lod_limit - 1) < factor || f / view_cam_lod_limit > factor)
		return TRUE;
	else 
		return FALSE;
}*/

extern uint p_lod_handle_edge_count(PPolyStore *smesh, PMesh *mesh, uint current_poly, uint current_edge);

void p_lod_select_tesselation(PMesh *mesh, PPolyStore *smesh, pgreal *cvs)
{
	pgreal *vertex, *v, *v2, *v3, f, temp[3], limit[7] = {0.0001, 0.05, 0.17, 1.0, 5.0, 25, 125};
	PDepend *dep;
	uint poly, i, j, stage;

	vertex = malloc((sizeof *vertex) * smesh->control_vertex_count * 3);
	if(mesh->tess.force > smesh->level)
		mesh->tess.force = smesh->level;
	v = &vertex[0];
	for(i = 0; i < smesh->control_vertex_count; i++)
	{
		v[0] = 0;
		v[1] = 0;
		v[2] = 0;
		dep = &smesh->vertex_dependency[i];
		for(j = 0; j < dep->length; j++)
		{
			f = dep->element[j].value;
			v3 = &cvs[dep->element[j].vertex * 3];
			v[0] += v3[0] * f;
			v[1] += v3[1] * f;
			v[2] += v3[2] * f;
		}
		v += 3;
	}
	{
		PTessTableElement *table, *tri_table, *quad_table;
		uint level[4] = {1, 1, 1, 1};
		tri_table = get_dynamic_table_tri(smesh->level, level);
		quad_table = get_dynamic_table_quad(smesh->level, level);
	//	mesh->render.mat[0].vertex_start = 0;
		for(stage = mesh->sub_stages[1]; stage < smesh->base_tri_count + smesh->base_quad_count; stage++)
		{
			if(stage < mesh->tess.quad_count)
			{
				
				poly = stage * smesh->poly_per_base * 4;
				v2 = &vertex[smesh->ref[poly + quad_table->reference[0]] * 3];
				for(i = 0; i < 4; i++)
				{
					j = stage * 4 + i;
					if(smesh->base_neighbor[j] < j)
					{
						j = smesh->base_neighbor[j];
						if(j / 4 < mesh->tess.quad_count)
							level[i] = get_dynamic_table_quad_level(smesh->level, mesh->tess.tess[mesh->tess.order_temp_mesh_rev[j / 4]], j % 4);
						else
							level[i] = get_dynamic_table_tri_level(smesh->level, mesh->tess.tess[mesh->tess.order_temp_mesh_rev[mesh->tess.quad_count + (j - mesh->tess.quad_count * 4) / 3]], (j - mesh->tess.quad_count * 4) % 3);
						v = v2;
						v2 = &vertex[smesh->ref[poly + quad_table->reference[((i + 1) % 4) * 2]] * 3];
					}else
					{
						temp[0] = 0;
						temp[1] = 0;
						temp[2] = 0;
						dep = &smesh->vertex_dependency[smesh->ref[poly + quad_table->reference[i * 2 + 1]]];
						for(j = 0; j < dep->length; j++)
						{
							f = dep->element[j].value;
							v3 = &cvs[dep->element[j].vertex * 3];
							temp[0] += v3[0] * f;
							temp[1] += v3[1] * f;
							temp[2] += v3[2] * f;
						}
						v = v2;
						v2 = &vertex[smesh->ref[poly + quad_table->reference[((i + 1) % 4) * 2]] * 3];
						f = p_sds_edge_tesselation_global_func_new(v, v2, temp, mesh->tess.eye, mesh->tess.factor);
						for(j = mesh->tess.force; j < smesh->level && limit[j] < f; j++);
						level[i] = j;
					}
				}
				table = get_dynamic_table_quad(smesh->level, level);

		//		for(i = 0; i < 4; i++)
		//			mesh->render.element_count += p_lod_handle_edge_count(smesh, mesh, stage, i);

			}else
			{
				poly = smesh->quad_length + (stage - smesh->base_quad_count) * smesh->poly_per_base * 3;
				poly = smesh->base_quad_count * smesh->poly_per_base * 4 + (stage - smesh->base_quad_count) * smesh->poly_per_base * 3;
				v2 = &vertex[smesh->ref[poly + tri_table->reference[0]] * 3];
				for(i = 0; i < 3; i++)
				{
					j = smesh->base_quad_count * 4 + (stage - smesh->base_quad_count) * 3 + i;
					if(smesh->base_neighbor[j] < j)
					{
						j = smesh->base_neighbor[j];
						if(j / 4 < mesh->tess.quad_count)
							level[i] = get_dynamic_table_quad_level(smesh->level, mesh->tess.tess[mesh->tess.order_temp_mesh_rev[j / 4]], j % 4);
						else
							level[i] = get_dynamic_table_tri_level(smesh->level, mesh->tess.tess[mesh->tess.order_temp_mesh_rev[mesh->tess.quad_count + (j - mesh->tess.quad_count * 4) / 3]], (j - mesh->tess.quad_count * 4) % 3);
						v = v2;
						v2 = &vertex[smesh->ref[poly + tri_table->reference[(i + 1) % 3 * 2]] * 3];
					}else
					{
						temp[0] = 0;
						temp[1] = 0;
						temp[2] = 0;
						dep = &smesh->vertex_dependency[smesh->ref[poly + tri_table->reference[i * 2 + 1]]];
						for(j = 0; j < dep->length; j++)
						{
							f = dep->element[j].value;
							v3 = &cvs[dep->element[j].vertex * 3];
							temp[0] += v3[0] * f;
							temp[1] += v3[1] * f;
							temp[2] += v3[2] * f;
						}
						v = v2;
						v2 = &vertex[smesh->ref[poly + tri_table->reference[(i + 1) % 3 * 2]] * 3];
						f = p_sds_edge_tesselation_global_func_new(v, v2, temp, mesh->tess.eye, mesh->tess.factor);
						for(j = mesh->tess.force; j < smesh->level && limit[j] < f; j++);
						level[i] = j;
					}
				}
				table = get_dynamic_table_tri(smesh->level, level);
	//			for(i = 0; i < 3; i++)
	//				mesh->render.element_count += p_lod_handle_edge_count(smesh, mesh, stage, i);
			}
			for(j = 0; j < table->vertex_count; j++)
				mesh->depend.length += smesh->vertex_dependency[smesh->ref[poly + table->reference[j]]].length;
			mesh->tess.tess[mesh->tess.order_temp_mesh_rev[stage]] = table;
			mesh->render.element_count += table->element_count;
			mesh->render.vertex_count += table->vertex_count;

		}
		mesh->sub_stages[1] = stage;
		if(mesh->sub_stages[1] == mesh->tess.tri_count + mesh->tess.quad_count)
		{

			for(j = 0; j < mesh->tess.tri_count + mesh->tess.quad_count; j++) /* FIX ME */
			{
				if(stage < mesh->tess.quad_count)
				{
					for(i = 0; i < 4; i++)
						mesh->render.element_count += p_lod_handle_edge_count(smesh, mesh, j, i);
				}else
				{
					for(i = 0; i < 3; i++)
						mesh->render.element_count += p_lod_handle_edge_count(smesh, mesh, j, i);
				}
			}

			mesh->stage++;
			mesh->sub_stages[0] = 0;
			mesh->sub_stages[1] = 0;
			mesh->sub_stages[2] = 0;
			mesh->sub_stages[3] = 0;
			if(mesh->temp != NULL)
			{
				free(mesh->temp);
				mesh->temp = NULL;
			}
		/*	exit(0);*/
		}
	}
}