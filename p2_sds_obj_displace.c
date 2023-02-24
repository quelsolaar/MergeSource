 
#define PERSUADE_INTERNAL
#include "persuade2.h"
#include "p2_sds_geo.h"
#include "p2_sds_table.h"
#include "p2_sds_obj.h"
#include "p2_shader.h"

#include <math.h>

typedef struct{
	ENode	*o_node; 
	ENode	*g_node;
	ENode	*m_node;
	PTessTableElement *table;
	uint32	*ref;
	egreal	*normals;
	uint	polygon;
	boolean quad;
}PDispalacemetParam;

extern void create_param_polygon(ENode *g_node, egreal *output, PTessTableElement *table, EGeoLayer *layer, uint32 *ref, uint channel, uint polygon, boolean quad);
extern egreal p_noise_multi_function(egreal *vec);
extern egreal p_noise_point_function(egreal *vec);

void displacement_func(PDispalacemetParam *param, egreal *output, uint16 id)
{
	VMatFrag *frag;
	uint i, length;
	egreal f;
	length = param->table->vertex_count * 3;
	if((frag = e_nsm_get_fragment(param->m_node, id)) == NULL || !e_nsm_enter_fragment(param->m_node, id))
	{
		for(i = 0; i < length; i++)
			output[i] = 0;
		return;
	}
	switch(e_nsm_get_fragment_type(param->m_node, id))
	{
		case VN_M_FT_COLOR :
			for(i = 0; i < length;)
			{
				output[i++] = frag->color.red;
				output[i++] = frag->color.green;
				output[i++] = frag->color.blue;	
			}
		break;
		case VN_M_FT_LIGHT :
			for(i = 0; i < length;)
			{
				output[i++] = 0;
				output[i++] = 0;
				output[i++] = 0;	
			}
		break;
		case VN_M_FT_REFLECTION :
			for(i = 0; i < length;)
			{
				output[i++] = 0;
				output[i++] = 0;
				output[i++] = 0;	
			}
		break;
		case VN_M_FT_TRANSPARENCY :
			for(i = 0; i < length;)
			{
				output[i++] = 0;
				output[i++] = 0;
				output[i++] = 0;	
			}
		break;
		case VN_M_FT_VIEW :
			for(i = 0; i < length;)
			{
				output[i++] = 0;
				output[i++] = 0;
				output[i++] = 0;	
			}
		break;
		case VN_M_FT_VOLUME :
			for(i = 0; i < length;)
			{
				output[i++] = 0;
				output[i++] = 0;
				output[i++] = 0;	
			}
		break;
		case VN_M_FT_GEOMETRY :
			create_param_polygon(param->g_node, output, param->table, e_nsg_get_layer_by_name(param->g_node, frag->geometry.layer_r), param->ref, 0, param->polygon, param->quad);
			create_param_polygon(param->g_node, output, param->table, e_nsg_get_layer_by_name(param->g_node, frag->geometry.layer_g), param->ref, 1, param->polygon, param->quad);
			create_param_polygon(param->g_node, output, param->table, e_nsg_get_layer_by_name(param->g_node, frag->geometry.layer_b), param->ref, 2, param->polygon, param->quad);
		break;
		case VN_M_FT_TEXTURE :
		{	
			ebreal pixel[3];
			EBMHandle *h;
			h = e_nsb_get_image_handle(frag->texture.bitmap, frag->texture.layer_r, frag->texture.layer_g, frag->texture.layer_b);
			displacement_func(param, output, frag->texture.mapping);
			for(i = 0; i < length;)
			{
				e_nsb_evaluate_image_handle_tile(h, pixel, (ebreal)output[i], (ebreal)output[i + 1], (ebreal)output[i + 2]);
				output[i++] = (egreal)pixel[0];
				output[i++] = (egreal)pixel[1];
				output[i++] = (egreal)pixel[2];	
			}
			e_nsb_destroy_image_handle(h);
		}
		break;
		case VN_M_FT_NOISE :
			displacement_func(param, output, frag->noise.mapping);
			switch(frag->noise.type)
			{
			case VN_M_NOISE_PERLIN_ZERO_TO_ONE :
				for(i = 0; i < length;)
				{
					f = p_noise_multi_function(&output[i]);
					output[i++] = f;
					output[i++] = f;
					output[i++] = f;	
				}
			break;
			case VN_M_NOISE_PERLIN_MINUS_ONE_TO_ONE :
				for(i = 0; i < length;)
				{
					f = p_noise_multi_function(&output[i]) * 2.0 - 1.0;
					output[i++] = f;
					output[i++] = f;
					output[i++] = f;	
				}
			break;
			case VN_M_NOISE_POINT_ZERO_TO_ONE :
				for(i = 0; i < length;)
				{
					f = p_noise_point_function(&output[i]);
					output[i++] = f;
					output[i++] = f;
					output[i++] = f;	
				}
			break;
			case VN_M_NOISE_POINT_MINUS_ONE_TO_ONE : 
				for(i = 0; i < length;)
				{
					f = p_noise_point_function(&output[i]) * 2.0 - 1.0;
					output[i++] = f;
					output[i++] = f;
					output[i++] = f;	
				}
			break;
			}
		break;
		case VN_M_FT_BLENDER :
		{
			egreal *a, *b, *c = NULL;
			a = malloc((sizeof *a) * length);
			displacement_func(param, a, frag->blender.data_a);
			b = malloc((sizeof *b) * length);
			displacement_func(param, b, frag->blender.data_b);
			switch(frag->blender.type)
			{
				case VN_M_BLEND_FADE :
					c = malloc((sizeof *a) * length);
					displacement_func(param, c, frag->blender.control);
					for(i = 0; i < length;)
					{
						output[i] = a[i] * (i - c[i]) + b[i] * c[i];
						i++;
						output[i] = a[i] * (i - c[i]) + b[i] * c[i];
						i++;
						output[i] = a[i] * (i - c[i]) + b[i] * c[i];	
						i++;
					}	
					free(c);
				break;
				case VN_M_BLEND_ADD :
					for(i = 0; i < length;)
					{
						output[i] = a[i] + b[i];
						i++;
						output[i] = a[i] + b[i];
						i++;
						output[i] = a[i] + b[i];	
						i++;
					}
				break;
				case VN_M_BLEND_SUBTRACT :
					for(i = 0; i < length;)
					{
						output[i] = a[i] - b[i];
						i++;
						output[i] = a[i] - b[i];
						i++;
						output[i] = a[i] - b[i];	
						i++;
					}
				break;
				case VN_M_BLEND_MULTIPLY :
					for(i = 0; i < length;)
					{
						output[i] = a[i] * b[i];
						i++;
						output[i] = a[i] * b[i];
						i++;
						output[i] = a[i] * b[i];	
						i++;
					}
				break;
				case VN_M_BLEND_DIVIDE :
					for(i = 0; i < length;)
					{
						output[i] = a[i] / b[i];
						i++;
						output[i] = a[i] / b[i];
						i++;
						output[i] = a[i] / b[i];	
						i++;
					}
				break;
			}

			free(a);
			free(b);
		}
		break;
		case VN_M_FT_CLAMP :
		{
			displacement_func(param, output, frag->clamp.data);
			if(frag->clamp.min)
			{
				for(i = 0; i < length;)
				{
					if(output[i] > frag->clamp.red)
						output[i] = frag->clamp.red;
					i++;
					if(output[i] > frag->clamp.green)
						output[i] = frag->clamp.green;
					i++;
					if(output[i] > frag->clamp.blue)
						output[i] = frag->clamp.blue;	
					i++;
				}
			}else
			{
				for(i = 0; i < length;)
				{
					if(output[i] < frag->clamp.red)
						output[i] = frag->clamp.red;
					i++;
					if(output[i] < frag->clamp.green)
						output[i] = frag->clamp.green;
					i++;
					if(output[i] < frag->clamp.blue)
						output[i] = frag->clamp.blue;	
					i++;
				}
			}
		}
		case VN_M_FT_MATRIX :
		{
			egreal tmp[3];
			displacement_func(param, output, frag->matrix.data);
			for(i = 0; i < length;)
			{
				tmp[0] = frag->matrix.matrix[0] * output[i] + frag->matrix.matrix[1] * output[i + 1] + frag->matrix.matrix[2] * output[i + 2] + frag->matrix.matrix[3] * 1;
				tmp[1] = frag->matrix.matrix[4] * output[i] + frag->matrix.matrix[5] * output[i + 1] + frag->matrix.matrix[6] * output[i + 2] + frag->matrix.matrix[7] * 1;
				tmp[2] = frag->matrix.matrix[8] * output[i] + frag->matrix.matrix[9] * output[i + 1] + frag->matrix.matrix[10] * output[i + 2] + frag->matrix.matrix[11] * 1;
				output[i++] = tmp[0];
				output[i++] = tmp[1];
				output[i++] = tmp[2];	
			}
		}
		break;
		case VN_M_FT_RAMP :
			
		{
			uint j;
			float f;
			displacement_func(param, output, frag->noise.mapping);
			for(i = 0; i < length; i += 3)
			{
				f = output[i + frag->ramp.channel];
				if(f < frag->ramp.ramp[0].pos)
				{
					output[i] = frag->ramp.ramp[0].red;
					output[i + 1] = frag->ramp.ramp[0].green;
					output[i + 2] = frag->ramp.ramp[0].blue;
				}else
				{
					for(j = 1; j < frag->ramp.point_count; j++)
					{
						if(f < frag->ramp.ramp[j].pos)
						{
							f = (f - frag->ramp.ramp[j - 1].pos) / (frag->ramp.ramp[j].pos - frag->ramp.ramp[j - 1].pos);
							output[i] = frag->ramp.ramp[j].red * f + frag->ramp.ramp[j - 1].red * (1 - f);
							output[i + 1] = frag->ramp.ramp[j].green * f + frag->ramp.ramp[j - 1].green * (1 - f);
							output[i + 2] = frag->ramp.ramp[j].blue * f + frag->ramp.ramp[j - 1].blue * (1 - f);
							break;
						}
					}
					if(j == frag->ramp.point_count)
					{
						--j;
						output[i] = frag->ramp.ramp[j].red;
						output[i + 1] = frag->ramp.ramp[j].green;
						output[i + 2] = frag->ramp.ramp[j].blue;
					}
				}
			}	
		}
		break;
		case VN_M_FT_ANIMATION :
			for(i = 0; i < length;)
			{
				output[i++] = 0;
				output[i++] = 0;
				output[i++] = 0;	
			}	
		break;
		case VN_M_FT_ALTERNATIVE :
			if((frag = e_nsm_get_fragment(param->m_node, frag->alternative.alt_a)) != NULL)
				displacement_func(param, output, frag->alternative.alt_a);
			else
				displacement_func(param, output, frag->alternative.alt_b);
		break;
		case VN_M_FT_OUTPUT :
			displacement_func(param, output, frag->output.front);
		break;
	}
	e_nsm_leave_fragment(param->m_node, id);
}


boolean p_lod_displacement_version(ENode *m_node, ENode *g_node, uint16 id, uint32 *version)
{
	ENode *b_node;
	EGeoLayer *layer;
	VMatFrag *frag;
	boolean output = FALSE;
	if((frag = e_nsm_get_fragment(m_node, id)) == NULL)
		return FALSE;
	if(!e_nsm_enter_fragment(m_node, id))
		return FALSE;

	*version += e_nsm_get_fragment_version(m_node, id);
	e_nsm_leave_fragment(m_node, id);
	switch(e_nsm_get_fragment_type(m_node, id))
	{

		case VN_M_FT_REFLECTION :
		output = TRUE;
		case VN_M_FT_TRANSPARENCY :
		output = TRUE;
		case VN_M_FT_VIEW :
		output = TRUE;
		case VN_M_FT_VOLUME :
		output = TRUE;
		case VN_M_FT_GEOMETRY :
			if((layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_r)) != NULL)
				*version += e_nsg_get_layer_version(layer);
			if((layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_g)) != NULL)
				*version += e_nsg_get_layer_version(layer);
			if((layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_b)) != NULL)
				*version += e_nsg_get_layer_version(layer);
		break;
		case VN_M_FT_TEXTURE :
			if((b_node = e_ns_get_node(0, frag->texture.bitmap)) != NULL && V_NT_BITMAP == e_ns_get_node_type(b_node))
				*version += e_ns_get_node_version_data(b_node);
			output = p_lod_displacement_version(m_node, g_node, frag->texture.mapping, version);
		break;
		case VN_M_FT_NOISE :
			output = p_lod_displacement_version(m_node, g_node, frag->noise.mapping, version);
		break;
		case VN_M_FT_BLENDER :
		{
			if(p_lod_displacement_version(m_node, g_node, frag->blender.data_a, version))
				output = TRUE;
			if(p_lod_displacement_version(m_node, g_node, frag->blender.data_b, version))
				output = TRUE;
			if(frag->blender.type == VN_M_BLEND_FADE)
				output = p_lod_displacement_version(m_node, g_node, frag->blender.control, version);
		}
		break;
		case VN_M_FT_CLAMP :
			output = p_lod_displacement_version(m_node, g_node, frag->clamp.data, version);
		break;
		case VN_M_FT_MATRIX :
			output = p_lod_displacement_version(m_node, g_node, frag->matrix.data, version);
		break;
		case VN_M_FT_RAMP :
			output = p_lod_displacement_version(m_node, g_node, frag->ramp.mapping, version);	
		break;
		case VN_M_FT_ANIMATION :
			output = TRUE;	
		break;
		case VN_M_FT_ALTERNATIVE :
			if((frag = e_nsm_get_fragment(m_node, frag->alternative.alt_a)) != NULL)
				output = p_lod_displacement_version(m_node, g_node, frag->alternative.alt_a, version);
			else
				output = p_lod_displacement_version(m_node, g_node, frag->alternative.alt_b, version);
		break;
		case VN_M_FT_OUTPUT :
			output = p_lod_displacement_version(m_node, g_node, frag->output.front, version);
		break;
	}
	e_nsm_leave_fragment(m_node, id);
	return output;
}


boolean p_lod_displacement_update_test(PMesh *mesh)
{
	uint32 i, tmp, version = 0;
	boolean output = FALSE;
	ENode *m_node, *g_node;

	if((g_node = e_ns_get_node(0, mesh->geometry_id)) == NULL)
		return FALSE;

	if(mesh->displacement.live || mesh->displacement.displacement == NULL)
	{
		for(i = 0; i < mesh->render.mat_count; i++)
			if((m_node = e_ns_get_node(0, mesh->render.mat[i].material)) != NULL)
				version += e_ns_get_node_version_data(m_node);
		if(version == mesh->displacement.node_version)
		{
			if(mesh->displacement.live)
				return TRUE;
			else
				return FALSE;
		}
	}

	mesh->displacement.live = FALSE;
	mesh->displacement.node_version = 0;
	mesh->displacement.tree_version = 0;

	for(i = 0; i < mesh->render.mat_count; i++)
	{
		if((m_node = e_ns_get_node(0, mesh->render.mat[i].material)) != NULL)
		{
			mesh->displacement.node_version += e_ns_get_node_version_data(m_node);
			if(p_lod_displacement_version(m_node, g_node, e_nsm_get_fragment_color_displacement(m_node), &tmp))
				mesh->displacement.live = TRUE;
			version += tmp;
		}
	}
	if(mesh->displacement.tree_version != version)
	{
		mesh->displacement.tree_version = version;
		return TRUE;
	}
	return mesh->displacement.live;
}
	

/*
typedef struct{
	ENode	*o_node; 
	ENode	*g_node;
	ENode	*m_node;
	PTessTableElement *table;
	uint32	*ref;
	egreal	*normals;
	uint	polygon;
	boolean quad;
}PDispalacemetParam;
*/
void p_lod_create_displacement_array(ENode *g_node, ENode *o_node, PMesh *mesh, uint base_level)
{
	PDispalacemetParam param;
	egreal *output, tmp[3];
	uint i, level[4], *ref;
	level[0] = base_level;
	level[1] = base_level;
	level[2] = base_level;
	level[3] = base_level;

  	param.o_node = o_node; 
	param.g_node = g_node;
	param.m_node = e_ns_get_node(0, mesh->render.mat[mesh->sub_stages[1]].material);
	ref = e_nsg_get_layer_data(g_node, e_nsg_get_layer_by_id(g_node,  1));

	if(param.m_node != NULL && e_nsm_get_fragment(param.m_node, e_nsm_get_fragment_color_displacement(param.m_node)) != NULL)
	{
		output = malloc((sizeof *output) * get_dynamic_table_quad(base_level, level)->vertex_count * 3);
		for(; mesh->sub_stages[0] < mesh->tess.tri_count + mesh->tess.quad_count; mesh->sub_stages[0]++)
		{
			if(mesh->sub_stages[0] == mesh->render.mat[mesh->sub_stages[1]].tri_end)
				param.m_node = e_ns_get_node(0, mesh->render.mat[++mesh->sub_stages[1]].material);
			param.quad = mesh->sub_stages[0] < mesh->render.mat[mesh->sub_stages[1]].quad_end;
			
			param.polygon = mesh->tess.order_node[mesh->sub_stages[0]];
			param.normals = &mesh->normal.normals[mesh->sub_stages[2] * 3];
			param.table = mesh->tess.tess[mesh->sub_stages[0]];
			param.ref = &ref[mesh->tess.order_node[mesh->sub_stages[0]] * 4];

			displacement_func(&param, output, e_nsm_get_fragment_color_displacement(param.m_node));

			for(i = 0; i < param.table->vertex_count; i++)
				mesh->displacement.displacement[mesh->sub_stages[2] + i] = (output[i * 3] + output[i * 3 + 1] + output[i * 3 + 2]) / 3.0 * 0.01;
			mesh->sub_stages[2] += param.table->vertex_count;
		}
		free(output);
	}else
	{
		for(; mesh->sub_stages[0] < mesh->tess.tri_count + mesh->tess.quad_count; mesh->sub_stages[0]++)
		{
			param.table = mesh->tess.tess[mesh->sub_stages[0]];
			for(i = 0; i < param.table->vertex_count; i++)
				mesh->displacement.displacement[mesh->sub_stages[2] + i] = 0;
			mesh->sub_stages[2] += param.table->vertex_count;
		}
	}
	if(mesh->sub_stages[0] == mesh->tess.tri_count + mesh->tess.quad_count)
	{
		mesh->stage++;
		mesh->sub_stages[0] = 0;
		mesh->sub_stages[1] = 0;
		mesh->sub_stages[2] = 0;
		mesh->sub_stages[3] = 0;
	}

}

void p_lod_update_displacement_array(ENode *g_node, ENode *o_node, PMesh *mesh, uint base_level)
{
	uint stage;
	stage = mesh->stage;
	while(stage == mesh->stage)
		p_lod_create_displacement_array(g_node, o_node, mesh, base_level);
	mesh->stage = stage;
}
void p_compute_displacement(PMesh *meshl)
{

//	memesh->normal.displacement[mesh->sub_stages[2] + i]
}
