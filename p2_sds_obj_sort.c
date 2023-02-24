#define PERSUADE_INTERNAL
#include "persuade2.h"
#include "p2_sds_geo.h"
#include "p2_sds_table.h"
#include "p2_sds_obj.h"

void p_lod_gap_count(PPolyStore *geometry, PMesh *mesh, void *material, uint bits)
{
	PMeshMaterial *mat;
	pgreal *v;
	uint32 i, j, k, *ref, stage, ref_count, vertex_count, mat_count = 0, param_count = 0;
	uint32 *buf;
	
	if(material == NULL)
	{
		mesh->render.mat = mat = malloc(sizeof *mat);
		mesh->render.mat_count = 0;
		mat[mesh->render.mat_count].tri_end = 0;
		mat[mesh->render.mat_count].quad_end = 0;
		mat[mesh->render.mat_count].id = 0;
		mat[mesh->render.mat_count].layer = -1;
		mat[mesh->render.mat_count].material = -1;
		mat[mesh->render.mat_count].material_version = 0;
		mesh->render.mat_count++;
	}else
	{
/*		ref_length = geometry->ref_length;
		buf = malloc((sizeof *buf) * ref_count);
		if(bits == 8)
		{
			uint8 *m;
			m  = material; 
			for(i = 0; i < ref_length; i++)
			{
				for(j = 0; j < mesh->render.mat_count && m[i] != mat[j].id; j++);
				if(material == NULL)
				{
					mesh->render.mat = mat = malloc(sizeof *mat);
					mesh->render.mat_count = 0;
					mat[mesh->render.mat_count].tri_end = 0;
					mat[mesh->render.mat_count].quad_end = 0;
					mat[mesh->render.mat_count].id = 0;
					mat[mesh->render.mat_count].layer = -1;
					mat[mesh->render.mat_count].material = -1;
					mat[mesh->render.mat_count].material_version = 0;
					mesh->render.mat_count++;
				}
			}
		}
			*/
	}

	mesh->tess.order_node = malloc((sizeof *mesh->tess.order_node) * (geometry->base_quad_count + geometry->base_tri_count));
	mesh->tess.order_temp_mesh = malloc((sizeof *mesh->tess.order_temp_mesh) * (geometry->base_quad_count + geometry->base_tri_count));
	mesh->tess.order_temp_mesh_rev = malloc((sizeof *mesh->tess.order_temp_mesh_rev) * (geometry->base_quad_count + geometry->base_tri_count));
	mesh->tess.order_temp_poly_start = malloc((sizeof *mesh->tess.order_temp_poly_start) * (geometry->base_quad_count + geometry->base_tri_count));
	mesh->sub_stages[0] = 2;
	mesh->sub_stages[1] = 0;
	mesh->sub_stages[2] = 0;
	mesh->sub_stages[3] = geometry->base_quad_count;

	for(i = 0; i < geometry->base_quad_count + geometry->base_tri_count; i++)
	{
		mesh->tess.order_temp_mesh[stage] = stage;
		mesh->tess.order_temp_mesh_rev[stage] = stage;
	}
}



void p_lod_set(PPolyStore *geometry, PMesh *mesh)
{
	PMeshMaterial *mat;
	pgreal *v;
	uint32 i, j, k, *ref, stage, ref_count, vertex_count, mat_count = 0, param_count = 0;
	uint32 *buf;

	mesh->render.mat = mat = malloc(sizeof *mat);
	mesh->render.mat_count = 0;
	mat[mesh->render.mat_count].tri_end = mesh->tess.tri_count + mesh->tess.quad_count;
	mat[mesh->render.mat_count].quad_end = mesh->tess.quad_count;
	mat[mesh->render.mat_count].id = 0;
	mat[mesh->render.mat_count].layer = -1;
	mat[mesh->render.mat_count].material = -1;
	mat[mesh->render.mat_count].material_version = 0;
	mesh->render.mat_count++;

	mesh->tess.order_node = malloc((sizeof *mesh->tess.order_node) * (geometry->base_quad_count + geometry->base_tri_count));
	mesh->tess.order_temp_mesh = malloc((sizeof *mesh->tess.order_temp_mesh) * (geometry->base_quad_count + geometry->base_tri_count));
	mesh->tess.order_temp_mesh_rev = malloc((sizeof *mesh->tess.order_temp_mesh_rev) * (geometry->base_quad_count + geometry->base_tri_count));
	mesh->tess.order_temp_poly_start = malloc((sizeof *mesh->tess.order_temp_poly_start) * (geometry->base_quad_count + geometry->base_tri_count));
	for(i = 0; i < geometry->base_quad_count + geometry->base_tri_count; i++)
	{
		mesh->tess.order_temp_mesh[i] = i;
		mesh->tess.order_temp_mesh_rev[i] = i;
	}
}
