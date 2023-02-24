#define PERSUADE_INTERNAL
#include "persuade2.h"
#include "p2_sds_geo.h"
#include "p2_sds_obj.h"
//extern ESObjectNode *e_ns_get_node(uint connection, uint node_id);

/*typedef void EObjLink;*/

double p_anim_evaluate_anim(ENode *o_node, double *output, uint seconds, uint fractions, char *name, double default_value)
{
	double write[4], tmp[4], scale[4], pos[4];
	ENode *c_node;
	EObjLink *link;
	ECurve *curve;
	boolean found = FALSE;

	if(output == NULL)
		output = tmp;
	output[0] = default_value;
	output[1] = 0;
	output[2] = 0;
	output[3] = 0;
	for(link = e_nso_get_next_link(o_node, 0); link != NULL; link = e_nso_get_next_link(o_node, e_nso_get_link_id(link) + 1))
	{
		c_node = e_ns_get_node(0, e_nso_get_link_node(link));
		if(c_node != NULL && e_ns_get_node_type(c_node) == V_NT_CURVE)
		{
			if(e_nso_get_anim_active(link))
			{
				e_nso_get_anim_evaluate_pos(link, pos, seconds, fractions);
				e_nso_get_anim_evaluate_scale(link, scale, seconds, fractions);
				if((curve = e_nsc_get_curve_by_name(c_node, name)) != NULL)
				{
					if(!found)
						output[0] = 0;
					found = TRUE;
					e_nsc_evaluate_curve(curve, write, pos[0]);
					output[0] += write[0] * scale[0];
					output[1] += write[1] * scale[0];
					output[2] += write[2] * scale[0];
					output[3] += write[3] * scale[0];
				}
			}
		}
	}
	return output[0];
}

boolean p_lod_anim_bones_update_test(PMesh *mesh, ENode *o_node, ENode *g_node)
{
	return FALSE;
}


boolean p_lod_anim_scale_update_test(PMesh *mesh, ENode *o_node)
{
	double scale[3];
	e_nso_get_scale(o_node, scale);
	if(mesh->anim.scale[0] != (pgreal)scale[0] || mesh->anim.scale[1] != (pgreal)scale[1] || mesh->anim.scale[2] != (pgreal)scale[2])
	{
		mesh->anim.scale[0] = (pgreal)scale[0];
		mesh->anim.scale[1] = (pgreal)scale[1];
		mesh->anim.scale[2] = (pgreal)scale[2];
		return TRUE;
	}
	return FALSE;
}


boolean p_lod_anim_layer_update_test(PMesh *mesh, ENode *o_node, ENode *g_node)
{
	double weight;
	boolean update = FALSE;
	EGeoLayer *layer;
	uint i = 0;
	
	if(mesh->anim.layers.layers == NULL)
	{
		if(mesh->anim.layers.layers != NULL)
			free(mesh->anim.layers.layers);
		mesh->anim.layers.layer_count = 0;
		for(layer = e_nsg_get_layer_next(g_node, 0); layer != NULL; layer = e_nsg_get_layer_next(g_node, e_nsg_get_layer_id(layer) + 1))
			if(VN_G_LAYER_VERTEX_XYZ == e_nsg_get_layer_type(layer))
				mesh->anim.layers.layer_count++;
		mesh->anim.layers.layers = malloc((sizeof *mesh->anim.layers.layers) * mesh->anim.layers.layer_count);
		for(i = 0; i < mesh->anim.layers.layer_count; i++)
		{
			mesh->anim.layers.layers[i].version = 0;
			mesh->anim.layers.layers[i].data = NULL;
			mesh->anim.layers.layers[i].weight[0] = 0;
		}
	}

	if(mesh->anim.layers.data_version != e_ns_get_node_version_data(g_node) || mesh->anim.play.layers)
	{
		mesh->anim.play.layers = FALSE;
		i = 0;
		for(layer = e_nsg_get_layer_next(g_node, 0); layer != NULL; layer = e_nsg_get_layer_next(g_node, e_nsg_get_layer_id(layer) + 1))
		{
			if(VN_G_LAYER_VERTEX_XYZ == e_nsg_get_layer_type(layer))
			{
				if(i == 0)
					weight = p_anim_evaluate_anim(o_node, NULL, mesh->anim.seconds, mesh->anim.fractions, e_nsg_get_layer_name(layer), 1);
				else
					weight = p_anim_evaluate_anim(o_node, NULL, mesh->anim.seconds, mesh->anim.fractions, e_nsg_get_layer_name(layer), 0);
				if((mesh->anim.layers.layers[i].weight[0] / mesh->anim.scale[0] + 0.0001 > weight &&
					mesh->anim.layers.layers[i].weight[0] / mesh->anim.scale[0] - 0.0001 < weight) ||
					mesh->anim.layers.layers[i].version != e_nsg_get_layer_version(layer))
					update = TRUE;
				mesh->anim.layers.layers[i].weight[0] = weight * mesh->anim.scale[0];
				mesh->anim.layers.layers[i].weight[1] = weight * mesh->anim.scale[1];
				mesh->anim.layers.layers[i].weight[2] = weight * mesh->anim.scale[2];
				mesh->anim.layers.layers[i++].version = e_nsg_get_layer_version(layer);
			}
		}
		mesh->anim.layers.data_version = e_ns_get_node_version_data(g_node);
	}
	return update || mesh->anim.play.layers;
}

	
	
	
	
	
	
	/*
	
	if(mesh->anim.geometry_data_version != e_ns_get_node_version_data(g_node))
			update_bones = TRUE;


		if(update_bones != TRUE)
		{
			i = 0;
			for(layer = e_nsg_get_layer_next(g_node, 0); layer != NULL; layer = e_nsg_get_layer_next(g_node, 0))
				if(mesh->anim.layer_versions[i++] != e_nsg_get_layer_version(layer))
					break;
			if(layer != NULL)
			{
				uint16 bone;
				for(bone = e_nsg_get_bone_next(g_node, 0); update_bones != TRUE && bone != (uint16)-1; bone = e_nsg_get_bone_next(g_node, bone + 1))
				{
					bone_layer = e_nsg_get_layer_by_name(g_node, e_nsg_get_bone_weight(g_node, bone));
					if(bone_layer != NULL)
					{
						i = 0;
						for(layer = e_nsg_get_layer_next(g_node, 0); layer != NULL && layer != bone_layer; layer = e_nsg_get_layer_next(g_node, 0))
							i++;
						if(layer == bone_layer && mesh->anim.layer_versions[i] != e_nsg_get_layer_version(layer))
							update_bones = TRUE;
					}
					bone_layer = e_nsg_get_layer_by_name(g_node, e_nsg_get_bone_reference(g_node, bone));

					if(bone_layer != NULL)
					{
						i = 0;
						for(layer = e_nsg_get_layer_next(g_node, 0); layer != NULL && layer != bone_layer; layer = e_nsg_get_layer_next(g_node, 0))
							i++;
						if(layer == bone_layer && mesh->anim.layer_versions[i] != e_nsg_get_layer_version(layer))
							update_bones = TRUE;
					}
				}
			}
		}
	}
	if(mesh->anim.global_curve_mode != e_ns_get_global_version(0, V_NT_CURVE))
	{
		update = TRUE;
	}

	e_nso_get_scale(o_node, scale);
	if(mesh->anim.scale[0] != scale[0] || mesh->anim.scale[1] != scale[1] || mesh->anim.scale[2] != scale[2])
	{
		mesh->anim.scale[0] = scale[0];
		mesh->anim.scale[1] = scale[1];
		mesh->anim.scale[2] = scale[2];
		update = TRUE;
	}

	mesh->anim.object_version = e_ns_get_node_version_struct(o_node);
	mesh->anim.geometry_data_version = e_ns_get_node_version_data(g_node);
	mesh->anim.global_curve_mode = e_ns_get_global_version(0, V_NT_CURVE);
	return update;
}
*/

/*
extern void			e_nsg_get_bone_pos32(ENode *g_node, uint16 bone_id, float *pos);
extern void			e_nsg_get_bone_pos64(ENode *g_node, uint16 bone_id, double *pos);
extern void			e_nsg_get_bone_rot32(ENode *g_node, uint16 bone_id, float *rot);
extern void			e_nsg_get_bone_rot64(ENode *g_node, uint16 bone_id, double *rot);
*/
void p_lod_anim_vertex_array(pgreal *vertex, uint cv_count, PMesh *mesh, ENode *g_node)
{
	pgreal *v, *data, x, y, z;
	uint i = 1, j;
	EGeoLayer *layer;
	
	layer = e_nsg_get_layer_by_id(g_node, 0);
	x = mesh->anim.layers.layers[0].weight[0];
	y = mesh->anim.layers.layers[0].weight[1];
	z = mesh->anim.layers.layers[0].weight[2];
	v = vertex;
	data = e_nsg_get_layer_data(g_node, layer);
	for(j = 0; j < cv_count; j++)
	{
		*v++ = *data++ * x;
		*v++ = *data++ * y;
		*v++ = *data++ * z;
	}

	for(layer = e_nsg_get_layer_next(g_node, 0); layer != NULL; layer = e_nsg_get_layer_next(g_node, e_nsg_get_layer_id(layer) + 1))
	{		
		if(VN_G_LAYER_VERTEX_XYZ == e_nsg_get_layer_type(layer) && i < mesh->anim.layers.layer_count)
		{
			if(mesh->anim.layers.layers[i].weight[0] > 0.00001 || mesh->anim.layers.layers[i].weight[0] < -0.00001 ||
				mesh->anim.layers.layers[i].weight[1] > 0.00001 || mesh->anim.layers.layers[i].weight[1] < -0.00001 ||
				mesh->anim.layers.layers[i].weight[2] > 0.00001 || mesh->anim.layers.layers[i].weight[2] < -0.00001)
			{
				x = mesh->anim.layers.layers[i].weight[0];
				y = mesh->anim.layers.layers[i].weight[1];
				z = mesh->anim.layers.layers[i].weight[2];
				v = vertex;
				data = e_nsg_get_layer_data(g_node, layer);
				for(j = 0; j < cv_count; j++)
				{
					*v++ += *data++ * x;
					*v++ += *data++ * y;
					*v++ += *data++ * z;
				}
			}
		}
	}
}
/*
	if(mesh->anim.bonereference)
	{
		k = 0;
		for(i = 0; i < cv_count; i++)
		{
			x = *v;
			*v = 0;
			y = v[1];
			v[1] = 0;
			z = v[2];
			v[2] = 0;
			for(j = 0; j < mesh->anim.ref_count[i]; j++)
			{
				m = &matrix[mesh->anim.bonereference[k] * 16];
				f = mesh->anim.boneweight[k++];
				v[0] += (m[0] * x + m[1] * y + m[2] * z + m[3]) * f;
				v[1] += (m[4] * x + m[5] * y + m[6] * z + m[7]) * f;
				v[2] += (m[8] * x + m[9] * y + m[10] * z + m[11]) * f;
			}
			v += 3;
		}
	}
}*/
