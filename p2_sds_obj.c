#define PERSUADE_INTERNAL
#include <stdlib.h>
#include "persuade2.h"
#include "p2_sds_geo.h"
#include "p2_sds_table.h"
#include "p2_sds_obj.h"

#include <math.h>

extern void p_get_tri_tess_index(uint *index, uint base_tess);
extern void p_get_quad_tess_index(uint *index, uint base_tess);
extern boolean p_lod_displacement_update_test(PMesh *mesh);

static uint p_sds_force_level = 4;
static float p_sds_mesh_factor = 1;

void p_geo_set_sds_force_level(uint level)
{
	p_sds_force_level = level;
}

uint p_geo_get_sds_force_level(void)
{
	return p_sds_force_level;
}

void p_geo_set_sds_mesh_factor(float factor)
{
	p_sds_mesh_factor = factor;
}

float p_geo_get_sds_mesh_factor(void)
{
	return p_sds_mesh_factor;
}

PMesh *p_rm_create(float compexity, uint force_tess_level)
{
	PMesh *mesh;
	mesh = malloc(sizeof *mesh);
	mesh->stage = 0;
	mesh->sub_stages[0] = 0;
	mesh->sub_stages[1] = 0;
	mesh->sub_stages[2] = 0;
	mesh->sub_stages[3] = 0;
	mesh->tess.force = force_tess_level;
	mesh->tess.factor = compexity;
	mesh->tess.edge_tess_func = p_sds_edge_tesselation_global_func;
	mesh->temp = NULL;
	mesh->next = NULL;
	mesh->tess.tess = NULL;
	mesh->tess.order_node = NULL;
	mesh->tess.order_temp_mesh = NULL;
	mesh->tess.order_temp_mesh_rev = NULL;
	mesh->tess.order_temp_poly_start = NULL;
	mesh->render.vertex_array = NULL;
	mesh->render.normal_array = NULL;
	mesh->render.reference = NULL;
	mesh->render.mat = NULL;
	mesh->render.shadows = TRUE;
	mesh->depend.reference = NULL;
	mesh->depend.weight = NULL;
	mesh->depend.ref_count = NULL;
	mesh->normal.normal_ref = NULL;	
	mesh->normal.normals = NULL;
	mesh->normal.draw_normals = NULL;
	mesh->param.array = NULL;
	mesh->param.version = NULL;
	return mesh;
}

void p_rm_set_eay(PMesh *mesh, pgreal *eye)
{
	mesh->tess.eye[0] = eye[0];
	mesh->tess.eye[1] = eye[1];
	mesh->tess.eye[2] = eye[2];
	mesh->tess.edge_tess_func = p_sds_edge_tesselation_global_func;
}


void persuade_lod_destroy(PMesh *mesh)
{
	uint i;
	if(mesh->temp != NULL)
		free(mesh->temp);
	if(mesh->tess.tess != NULL)
		free(mesh->tess.tess);
	if(mesh->tess.order_node != NULL)
		free(mesh->tess.order_node);
	if(mesh->tess.order_temp_mesh != NULL)
		free(mesh->tess.order_temp_mesh);
	if(mesh->tess.order_temp_mesh_rev != NULL)
		free(mesh->tess.order_temp_mesh_rev);
	if(mesh->tess.order_temp_poly_start != NULL)
		free(mesh->tess.order_temp_poly_start);

	if(mesh->render.vertex_array != NULL)
		free(mesh->render.vertex_array);
	if(mesh->render.normal_array != NULL)
		free(mesh->render.normal_array);
	if(mesh->render.reference != NULL)
		free(mesh->render.reference);
	if(mesh->render.mat != NULL)
		free(mesh->render.mat);

	if(mesh->depend.reference != NULL)
		free(mesh->depend.reference);
	if(mesh->depend.weight != NULL)
		free(mesh->depend.weight);
	if(mesh->depend.ref_count != NULL)
		free(mesh->depend.ref_count);

	if(mesh->normal.normal_ref != NULL)
		free(mesh->normal.normal_ref);	
	if(mesh->normal.normals != NULL)
		free(mesh->normal.normals);
	if(mesh->normal.draw_normals != NULL)
		free(mesh->normal.draw_normals);

	if(mesh->param.version != NULL)
		free(mesh->param.version);
	free(mesh);
}

uint p_rm_get_param_count(PMesh *mesh)
{
	return mesh->param.array_count;
}


#define P_TRI_TESS_SELECT 10000
#define P_QUAD_TESS_SELECT 7500
#define P_TRI_TESS_REF 10000
#define P_QUAD_TESS_REF 7500

double *p_lod_get_view_pos(void);

void p_lod_set(PPolyStore *geometry, PMesh *mesh);

PMesh *persuade2_lod_create(PPolyStore *smesh, uint *ref, pgreal *vertex, float compexity, uint force_tess_level, float *eye)
{
	PMesh *mesh;
	PTessTableElement *table;
	uint i, j, k, l, material, polygon, poly_size = 4;
	mesh = p_rm_create(compexity, force_tess_level);
	for(i = 1 ; i < smesh->level; i++)
		poly_size *= 4;

	mesh->tess.tri_count = smesh->base_tri_count;
	mesh->tess.quad_count = smesh->base_quad_count;
	mesh->tess.tess = malloc((sizeof *mesh->tess.tess) * (mesh->tess.tri_count + mesh->tess.quad_count));
	for(i = 0; i < (mesh->tess.tri_count + mesh->tess.quad_count); i++)
		mesh->tess.tess[i] = NULL;
	mesh->tess.eye[0] = eye[0];
	mesh->tess.eye[1] = eye[1];
	mesh->tess.eye[2] = eye[2];
	mesh->temp = NULL;
	mesh->render.element_count = 0;
	mesh->render.vertex_count = 0;
	mesh->render.open_edges = smesh->open_edges != 0;
	mesh->render.shadows = smesh->open_edges == 0;
	mesh->depend.length = 0;
	mesh->sub_stages[0] = 0;
	mesh->sub_stages[1] = 0;
	mesh->sub_stages[2] = 0;
	mesh->sub_stages[3] = 0;
	mesh->stage++;
	
	p_lod_set(smesh, mesh);

	p_lod_select_tesselation(mesh, smesh, vertex);

	mesh->render.vertex_array = malloc((sizeof *mesh->render.vertex_array) * mesh->render.vertex_count * 3);
	mesh->render.normal_array = malloc((sizeof *mesh->render.normal_array) * mesh->render.vertex_count * 3);
	mesh->normal.normal_ref = malloc((sizeof *mesh->normal.normal_ref) * mesh->render.vertex_count * 4);
	mesh->render.reference = malloc((sizeof *mesh->render.reference) * mesh->render.element_count * 3);
	mesh->depend.reference = malloc((sizeof *mesh->depend.reference) * mesh->depend.length);
	mesh->depend.weight = malloc((sizeof *mesh->depend.weight) * mesh->depend.length);
	mesh->depend.length_temp = mesh->depend.length;
	mesh->depend.length_temp2 = mesh->render.vertex_count;
	mesh->depend.length_temp3 = mesh->render.element_count;
	mesh->depend.ref_count = malloc((sizeof *mesh->depend.ref_count) * mesh->render.vertex_count);
	mesh->render.element_count = 0;
	mesh->render.vertex_count = 0;
	mesh->depend.length = 0;

				
				
	/* building reference */
	for(i = k = l = 0; i < mesh->tess.tri_count + mesh->tess.quad_count; i++)
	{
		if(i == mesh->render.mat[l].tri_end)
		{
			mesh->render.mat[l].render_end = mesh->render.element_count;
			l++;
		}
		table = mesh->tess.tess[i];
		for(j = 0; j < table->element_count;)
		{
			mesh->render.reference[mesh->render.element_count++] = table->index[j++] + k;
			mesh->render.reference[mesh->render.element_count++] = table->index[j++] + k;
			mesh->render.reference[mesh->render.element_count++] = table->index[j++] + k;
		}
		k += table->vertex_count;
	}
				
	mesh->render.mat[l].render_end = mesh->render.element_count;

	/* building depend */
	{
//		uint poly, temp = 0;
		for(i = material = l = 0; i < mesh->tess.tri_count + mesh->tess.quad_count; i++)
		{
			table = mesh->tess.tess[i];

			if(i == mesh->render.mat[material].tri_end)
				material++;
			if(i < mesh->render.mat[material].quad_end)
				polygon = mesh->tess.order_temp_mesh[i] * smesh->poly_per_base * 4;
			else
				polygon = smesh->base_quad_count * smesh->poly_per_base * 4 + (mesh->tess.order_temp_mesh[i] - smesh->base_quad_count) * smesh->poly_per_base * 3;

			for(j = 0; j < table->vertex_count; j++)
			{
				PDepend *dep;
			//	temp++;
				dep = &smesh->vertex_dependency[smesh->ref[table->reference[j] + polygon]];
				mesh->depend.ref_count[mesh->render.vertex_count++] = dep->length;
				for(k = 0; k < dep->length; k++)
				{
					mesh->depend.reference[mesh->depend.length] = dep->element[k].vertex * 3;
					mesh->depend.weight[mesh->depend.length] = dep->element[k].value;
					mesh->depend.length++;
				}
			}
			mesh->tess.order_temp_poly_start[i] = l;
			l += table->vertex_count;
		}
	}

	p_lod_compute_vertex_normals(smesh, mesh);
	p_lod_create_normal_ref_and_shadow_skirts(smesh, mesh);

	/*			p_lod_create_layer_param(g_node, mesh);
				if(o_node != NULL)	
				{
					if(p_lod_displacement_update_test(mesh))
					{
						uint ii;
						mesh->displacement.displacement = malloc((sizeof *mesh->displacement.displacement) * mesh->render.vertex_count);
						p_lod_create_displacement_array(g_node, o_node, mesh, smesh->level);
					//	for(ii = 0; ii < mesh->render.vertex_count; ii++)
					//		mesh->displacement.displacement[ii] = 0;
					}
				}
					p_lod_anim_bones_update_test(mesh, o_node, g_node);
					p_lod_anim_scale_update_test(mesh, o_node);
					p_lod_anim_layer_update_test(mesh, o_node, g_node);
			case POS_ANIMATE :
		
				if(o_node != NULL)	
					p_lod_anim_vertex_array(mesh->anim.cvs, mesh->anim.cv_count, mesh, g_node);
				mesh->stage++;
				break;
			case POS_CREATE_VERTICES :
				j = 0;
				for(i = 0; i < mesh->render.vertex_count; i++)
					j += mesh->depend.ref_count[i];
				if(o_node != NULL)	
					p_lod_compute_vertex_array(mesh->render.vertex_array, mesh->render.vertex_count, mesh->depend.ref_count, mesh->depend.reference, mesh->depend.weight, mesh->anim.cvs);
				else
					p_lod_compute_vertex_array(mesh->render.vertex_array, mesh->render.vertex_count, mesh->depend.ref_count, mesh->depend.reference, mesh->depend.weight, vertex);
				p_lod_compute_normal_array(mesh->render.normal_array, mesh->render.vertex_count, mesh->normal.normal_ref, mesh->render.vertex_array);
			//	if(o_node != NULL)	
			//		p_lod_create_displacement_array(g_node, o_node, mesh, smesh->level);
				if(mesh->displacement.displacement != NULL)
				{
					p_lod_compute_displacement_array(mesh->render.vertex_array, mesh->render.vertex_count, mesh->render.normal_array, mesh->displacement.displacement);
					p_lod_compute_normal_array(mesh->render.normal_array, mesh->render.vertex_count, mesh->normal.normal_ref, mesh->render.vertex_array);
				}
				mesh->stage++;
				if(store != NULL)
					p_rm_destroy(store);
				store = NULL;
				break;
			case POS_DONE :
				if(o_node != NULL)
				{
					boolean update = FALSE;
					static double timer = 0;
					timer += 0.1;
					p_lod_update_shadow(g_node, mesh);
					p_lod_update_layer_param(g_node, mesh);
					if(p_lod_anim_bones_update_test(mesh, o_node, g_node))
						update = TRUE;
					if(p_lod_anim_scale_update_test(mesh, o_node))
						update = TRUE;
					if(p_lod_anim_layer_update_test(mesh, o_node, g_node))
						update = TRUE;
					if(p_lod_displacement_update_test(mesh))
					{
						uint ii;
						p_lod_update_displacement_array(g_node, o_node, mesh, smesh->level);
						update = TRUE;
					}
					if(update)
					{
						p_lod_anim_vertex_array(mesh->anim.cvs, mesh->anim.cv_count, mesh, g_node);
						p_lod_compute_vertex_array(mesh->render.vertex_array, mesh->render.vertex_count, mesh->depend.ref_count, mesh->depend.reference, mesh->depend.weight, mesh->anim.cvs);
						p_lod_compute_normal_array(mesh->render.normal_array, mesh->render.vertex_count, mesh->normal.normal_ref, mesh->render.vertex_array);
						if(mesh->displacement.displacement != NULL)
						{
							p_lod_compute_displacement_array(mesh->render.vertex_array, mesh->render.vertex_count, mesh->render.normal_array, mesh->displacement.displacement);
							p_lod_compute_normal_array(mesh->render.normal_array, mesh->render.vertex_count, mesh->normal.normal_ref, mesh->render.vertex_array);
						}
					}
				}
				return mesh;	
		}
	}
	if(store != NULL)
		return store;*/
	return mesh;
}


uint persuade2_lod_vertex_length_get(PMesh *mesh)
{
	return mesh->render.vertex_count;
}


void persuade2_lod_vertex_shape(PMesh *mesh, pgreal *render_vertex, uint output_stride, pgreal *cvs)
{
	p_lod_compute_vertex_array(render_vertex, output_stride, mesh->render.vertex_count, mesh->depend.ref_count, mesh->depend.reference, mesh->depend.weight, cvs);

}


void persuade2_lod_normal_shape(PMesh *mesh, pgreal *render_normal, uint normal_stride, pgreal *render_vertex, uint vertex_stride)
{
	p_lod_compute_normal_array(render_normal, normal_stride, mesh->render.vertex_count, mesh->normal.normal_ref, render_vertex, vertex_stride);
}


/*
void p_rm_compute(PMesh *mesh, pgreal *vertex, uint start, uint length)
{
	uint i, j, k = 0, l, count, *ref;
	pgreal f;
	ref = mesh->depend.reference;

	for(i = 0; i < mesh->render.vertex_count; i++)
	{
		mesh->render.vertex_array[i * 3] = 0;
		mesh->render.vertex_array[i * 3 + 1] = 0;
		mesh->render.vertex_array[i * 3 + 2] = 0;
		count = mesh->depend.ref_count[i];
		for(j = 0; j < count; j++)
		{
			l = mesh->depend.reference[k];
			f = mesh->depend.weight[k];
			mesh->render.vertex_array[i * 3] += vertex[l++] * f;
			mesh->render.vertex_array[i * 3 + 1] += vertex[l++] * f;
			mesh->render.vertex_array[i * 3 + 2] += vertex[l] * f;
			k++;
		}
	}
}
*/

boolean p_rm_draw_ready(PMesh *mesh)
{
	return mesh->stage > 7;
}

#define NORMAL_ADD 0.001

void p_lod_compute_vertex_array(pgreal *vertex, uint output_stride, uint vertex_count, const uint *ref_count, const uint *reference,  const pgreal *weight, const pgreal *cvs)
{
	uint i, j, k = 0, count, r;
	pgreal f;
//	printf("vertex_count %u\n", vertex_count);
	for(i = 0; i < vertex_count; i++)
	{
		vertex[0] = 0;
		vertex[1] = 0;
		vertex[2] = 0;
		count = ref_count[i];
		for(j = 0; j < count; j++)
		{
			r = reference[k];
			f = weight[k++];
	//		printf("cvs %u %f %f %f\n", r / 3, cvs[r], cvs[r + 1], cvs[r + 2]);
			vertex[0] += cvs[r++] * f;
			vertex[1] += cvs[r++] * f;
			vertex[2] += cvs[r] * f;
		}
	//	printf("float %f %f %f\n", vertex[v + 0], vertex[v + 1], vertex[v + 2]);
		vertex = (pgreal *)(&((uint8 *)vertex)[output_stride]);
	}

}

void p_lod_compute_displacement_array(pgreal *vertex, uint vertex_count, const pgreal *normals, const pgreal *displacement)
{
	uint i;
	for(i = 0; i < vertex_count; i++)
	{
		*vertex++ += *normals++ * *displacement * 100;
		*vertex++ += *normals++ * *displacement * 100;
		*vertex++ += *normals++ * *displacement++ * 100;
	}
}

void p_lod_compute_normal_array(pgreal *normals, uint normal_stride, uint vertex_count, const uint *normal_ref, const pgreal *vertex, uint vertex_stride)
{
	uint i = 0, j = 0, k = 0;
	pgreal x, y, z, f, vec0[3], vec1[3], vec2[3], vec3[3];

	
	normal_stride /= sizeof(pgreal);
	vertex_stride /= sizeof(pgreal);
	vertex_count *= normal_stride;
	if(TRUE)
	{
		while(i < vertex_count)
		{
			x = vertex[k];
			y = vertex[k + 1];
			z = vertex[k + 2];
			k += vertex_stride;
			vec0[0] = vertex[normal_ref[j] * vertex_stride] - x;
			vec0[1] = vertex[normal_ref[j] * vertex_stride + 1] - y;
			vec0[2] = vertex[normal_ref[j] * vertex_stride + 2] - z;
			j++;
			vec1[0] = vertex[normal_ref[j] * vertex_stride] - x;
			vec1[1] = vertex[normal_ref[j] * vertex_stride + 1] - y;
			vec1[2] = vertex[normal_ref[j] * vertex_stride + 2] - z;
			j++;
			vec2[0] = vertex[normal_ref[j] * vertex_stride] - x;
			vec2[1] = vertex[normal_ref[j] * vertex_stride + 1] - y;
			vec2[2] = vertex[normal_ref[j] * vertex_stride + 2] - z;
			j++;
			vec3[0] = vertex[normal_ref[j] * vertex_stride] - x;
			vec3[1] = vertex[normal_ref[j] * vertex_stride + 1] - y;
			vec3[2] = vertex[normal_ref[j] * vertex_stride + 2] - z;
			j++;

			x = (vec0[1] * vec1[2] - vec0[2] * vec1[1]) + (vec2[1] * vec3[2] - vec2[2] * vec3[1]);
			y = (vec0[2] * vec1[0] - vec0[0] * vec1[2]) + (vec2[2] * vec3[0] - vec2[0] * vec3[2]);
			z = (vec0[0] * vec1[1] - vec0[1] * vec1[0]) + (vec2[0] * vec3[1] - vec2[1] * vec3[0]);

			f = sqrt(x * x + y * y + z * z);
			normals[i] = x / f;
			normals[i + 1] = y / f;
			normals[i + 2] = z / f;
			i += normal_stride;
		}
	}else
	{
		while(i < vertex_count)
		{
			x = vertex[k];
			y = vertex[k + 1];
			z = vertex[k + 2];
			k += vertex_stride;
			vec0[0] = vertex[normal_ref[j] * vertex_stride] - x;
			vec0[1] = vertex[normal_ref[j] * vertex_stride + 1] - y;
			vec0[2] = vertex[normal_ref[j] * vertex_stride + 2] - z;
			f = sqrt(vec0[0] * vec0[0] + vec0[1] * vec0[1] + vec0[2] * vec0[2]);
			vec0[0] /= f;
			vec0[1] /= f;
			vec0[2] /= f;
			j++;
			vec1[0] = vertex[normal_ref[j] * vertex_stride] - x;
			vec1[1] = vertex[normal_ref[j] * vertex_stride + 1] - y;
			vec1[2] = vertex[normal_ref[j] * vertex_stride + 2] - z;
			f = sqrt(vec1[0] * vec1[0] + vec1[1] * vec1[1] + vec1[2] * vec1[2]);
			vec1[0] /= f;
			vec1[1] /= f;
			vec1[2] /= f;
			j++;
			vec2[0] = vertex[normal_ref[j] * vertex_stride] - x;
			vec2[1] = vertex[normal_ref[j] * vertex_stride + 1] - y;
			vec2[2] = vertex[normal_ref[j] * vertex_stride + 2] - z;
			f = sqrt(vec2[0] * vec2[0] + vec2[1] * vec2[1] + vec2[2] * vec2[2]);
			vec2[0] /= f;
			vec2[1] /= f;
			vec2[2] /= f;
			j++;
			vec3[0] = vertex[normal_ref[j] * vertex_stride] - x;
			vec3[1] = vertex[normal_ref[j] * vertex_stride + 1] - y;
			vec3[2] = vertex[normal_ref[j] * vertex_stride + 2] - z;
			f = sqrt(vec3[0] * vec3[0] + vec3[1] * vec3[1] + vec3[2] * vec3[2]);
			vec3[0] /= f;
			vec3[1] /= f;
			vec3[2] /= f;
			j++;

			x = (vec0[1] * vec1[2] - vec0[2] * vec1[1]) + (vec2[1] * vec3[2] - vec2[2] * vec3[1]);
			y = (vec0[2] * vec1[0] - vec0[0] * vec1[2]) + (vec2[2] * vec3[0] - vec2[0] * vec3[2]);
			z = (vec0[0] * vec1[1] - vec0[1] * vec1[0]) + (vec2[0] * vec3[1] - vec2[1] * vec3[0]);

			f = sqrt(x * x + y * y + z * z);
			normals[i] = x / f;
			normals[i + 1] = y / f;
			normals[i + 2] = z / f;
			i += normal_stride;
		}
	}
}


pgreal *p_rm_get_vertex(PMesh *mesh)
{
	return mesh->render.vertex_array;
}

pgreal *p_rm_get_normal(PMesh *mesh)
{
	return mesh->render.normal_array;
}

uint p_rm_get_vertex_length(PMesh *mesh)
{
	return mesh->render.vertex_count;
}

uint *p_rm_get_reference(PMesh *mesh)
{
	return mesh->render.reference;
}

uint p_rm_get_ref_length(PMesh *mesh)
{
	return mesh->render.element_count;
}

uint p_rm_get_mat_count(PMesh *mesh)
{
	return mesh->render.mat_count;
}

uint p_rm_get_material_range(PMesh *mesh, uint mat)
{
	return mesh->render.mat[mat].render_end;
}

uint p_rm_get_material(PMesh *mesh, uint mat)
{
	return mesh->render.mat[mat].material;
}

boolean p_rm_get_shadow(PMesh *mesh)
{
	return mesh->render.shadows;
}
