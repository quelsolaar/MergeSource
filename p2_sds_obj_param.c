#define PERSUADE_INTERNAL
#include "persuade2.h"
#include "p2_sds_geo.h"
#include "p2_sds_table.h"
#include "p2_sds_obj.h"


void create_param_polygon(ENode *g_node, pgreal *output, PTessTableElement *table, EGeoLayer *layer, uint32 *ref, uint channel, uint polygon, boolean quad)
{
	pgreal a, b, c, d, f;
	uint i, j = 0;

	output += channel;

	if(layer == NULL)
	{
		for(i = 0; i < table->vertex_count; i++)
			output[i * 3] = 0;
		return;
	}
	switch(e_nsg_get_layer_type(layer))
	{
		case VN_G_LAYER_VERTEX_XYZ :
		{
			pgreal *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			a = layer_data[ref[0] * 3 + channel];
			b = layer_data[ref[1] * 3 + channel];
			c = layer_data[ref[2] * 3 + channel];
			j = 0;
			if(quad)
			{
				d = layer_data[ref[3] * 3 + channel];
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					f += table->vertex_influence[j++] * d;
					*output = f;
					output += 3;
				}
			}else
			{
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					*output = f;
					output += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_VERTEX_UINT32 :
		{
			uint32 *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			a = layer_data[ref[0]];
			b = layer_data[ref[1]];
			c = layer_data[ref[2]];
			j = 0;
			if(quad)
			{
				d = layer_data[ref[3]];
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					f += table->vertex_influence[j++] * d;
					*output = f;
					output += 3;
				}
			}else
			{
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					*output = f;
					output += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_VERTEX_REAL :
		{
			pgreal *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			a = layer_data[ref[0]];
			b = layer_data[ref[1]];
			c = layer_data[ref[2]];
			j = 0;
			if(quad)
			{
				d = layer_data[ref[3]];
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					f += table->vertex_influence[j++] * d;
					*output = f;
					output += 3;
				}
			}else
			{
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					*output = f;
					output += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_POLYGON_CORNER_UINT32 :
		{
			uint32 *layer_data, *layer_pos;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			layer_pos = &layer_data[polygon * 4];
			a = (pgreal)layer_pos[0];
			b = (pgreal)layer_pos[1];
			c = (pgreal)layer_pos[2];
			j = 0;
			if(quad)
			{
				d = (pgreal)layer_pos[3];
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					f += table->vertex_influence[j++] * d;
					*output = f;
					output += 3;
				}
			}else
			{
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					*output = f;
					output += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_POLYGON_CORNER_REAL :
		{
			pgreal *layer_data, *layer_pos;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			layer_pos = &layer_data[polygon * 4];
			a = (pgreal)layer_pos[0];
			b = (pgreal)layer_pos[1];
			c = (pgreal)layer_pos[2];
			j = 0;
			if(quad)
			{
				d = (pgreal)layer_pos[3];
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					f += table->vertex_influence[j++] * d;
					*output = f;
					output += 3;
				}
			}else
			{
				for(i = 0; i < table->vertex_count; i++)
				{
					f = table->vertex_influence[j++] * a;
					f += table->vertex_influence[j++] * b;
					f += table->vertex_influence[j++] * c;
					*output = f;
					output += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_POLYGON_FACE_UINT8 :
		{
			uint8 *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			a = (pgreal)layer_data[polygon];
			for(i = 0; i < table->vertex_count; i++)
				output[i] = a;
		}
		break;
		case VN_G_LAYER_POLYGON_FACE_UINT32 :
		{
			uint32 *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			a = (pgreal)layer_data[polygon];
			for(i = 0; i < table->vertex_count; i++)
				output[i] = a;
		}
		break;
		case VN_G_LAYER_POLYGON_FACE_REAL :
		{
			pgreal *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			a = layer_data[polygon];
			for(i = 0; i < table->vertex_count; i++)
				output[i] = a;
		}
		break;
	}
}

uint create_param_segment(ENode *g_node, pgreal *output, uint output_start, PTessTableElement **table_array, EGeoLayer *layer, uint32 *order_node, uint channel, uint start, uint tri_start, uint end)
{
	PTessTableElement *table;
	pgreal a, b, c, d, f;
	uint32 *ref;
	uint i, j, k;
	output_start = output_start * 3 + channel;

	if(layer == NULL)
	{
		k = 0;
		for(i = start; i < end; i++)
			k += table_array[i]->vertex_count;
		k += 3;
		for(i = 0; i < k; i += 3)
			output[i + output_start] = 0;
		return (output_start + channel) / 3;
	}
	ref = e_nsg_get_layer_data(g_node, e_nsg_get_layer_by_id(g_node,  1));

	switch(e_nsg_get_layer_type(layer))
	{
		case VN_G_LAYER_VERTEX_XYZ :
		{
			pgreal *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			for(i = start; i < tri_start; i++)
			{
				table = table_array[i];
				a = layer_data[ref[order_node[i] * 4] * 3 + channel];
				b = layer_data[ref[order_node[i] * 4 + 1] * 3 + channel];
				c = layer_data[ref[order_node[i] * 4 + 2] * 3 + channel];
				d = layer_data[ref[order_node[i] * 4 + 3] * 3 + channel];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					f += table->vertex_influence[k++] * d;
					output[output_start] = f;
					output_start += 3;
				}
			}
			for(; i < end; i++)
			{
				table = table_array[i];
				a = layer_data[ref[order_node[i] * 4] * 3 + channel];
				b = layer_data[ref[order_node[i] * 4 + 1] * 3 + channel];
				c = layer_data[ref[order_node[i] * 4 + 2] * 3 + channel];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					output[output_start] = f;
					output_start += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_VERTEX_UINT32 :
		{
			uint32 *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			for(i = start; i < tri_start; i++)
			{
				table = table_array[i];
				a = (pgreal)layer_data[ref[order_node[i] * 4]];
				b = (pgreal)layer_data[ref[order_node[i] * 4 + 1]];
				c = (pgreal)layer_data[ref[order_node[i] * 4 + 2]];
				d = (pgreal)layer_data[ref[order_node[i] * 4 + 3]];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					f += table->vertex_influence[k++] * d;
					output[output_start] = f;
					output_start += 3;
				}
			}
			for(; i < end; i++)
			{
				table = table_array[i];
				a = (pgreal)layer_data[ref[order_node[i] * 4]];
				b = (pgreal)layer_data[ref[order_node[i] * 4 + 1]];
				c = (pgreal)layer_data[ref[order_node[i] * 4 + 2]];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					output[output_start] = f;
					output_start += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_VERTEX_REAL :
		{
			pgreal *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			for(i = start; i < tri_start; i++)
			{
				table = table_array[i];
				a = layer_data[ref[order_node[i] * 4]];
				b = layer_data[ref[order_node[i] * 4 + 1]];
				c = layer_data[ref[order_node[i] * 4 + 2]];
				d = layer_data[ref[order_node[i] * 4 + 3]];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					f += table->vertex_influence[k++] * d;
					output[output_start] = f;
					output_start += 3;
				}
			}
			for(; i < end; i++)
			{
				table = table_array[i];
				a = layer_data[ref[order_node[i] * 4]];
				b = layer_data[ref[order_node[i] * 4 + 1]];
				c = layer_data[ref[order_node[i] * 4 + 2]];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					output[output_start] = f;
					output_start += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_POLYGON_CORNER_UINT32 :
		{
			uint32 *layer_data, *layer_pos;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			for(i = start; i < tri_start; i++)
			{
				table = table_array[i];
				layer_pos = &layer_data[order_node[i] * 4];
				a = (pgreal)layer_pos[0];
				b = (pgreal)layer_pos[1];
				c = (pgreal)layer_pos[2];
				d = (pgreal)layer_pos[3];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					f += table->vertex_influence[k++] * d;
					output[output_start] = f;
					output_start += 3;
				}
			}
			for(; i < end; i++)
			{
				table = table_array[i];
				layer_pos = &layer_data[order_node[i] * 4];
				a = (pgreal)layer_pos[0];
				b = (pgreal)layer_pos[1];
				c = (pgreal)layer_pos[2];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					output[output_start] = f;
					output_start += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_POLYGON_CORNER_REAL :
		{
			pgreal *layer_data, *layer_pos;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			for(i = start; i < tri_start; i++)
			{
				table = table_array[i];
				layer_pos = &layer_data[order_node[i] * 4];
				a = layer_pos[0];
				b = layer_pos[1];
				c = layer_pos[2];
				d = layer_pos[3];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					f += table->vertex_influence[k++] * d;
					output[output_start] = f;
					output_start += 3;
				}
			}
			for(; i < end; i++)
			{
				table = table_array[i];
				layer_pos = &layer_data[order_node[i] * 4];
				a = layer_pos[0];
				b = layer_pos[1];
				c = layer_pos[2];
				k = 0;
				for(j = 0; j < table->vertex_count; j++)
				{
					f = table->vertex_influence[k++] * a;
					f += table->vertex_influence[k++] * b;
					f += table->vertex_influence[k++] * c;
					output[output_start] = f;
					output_start += 3;
				}
			}
		}			
		break;
		case VN_G_LAYER_POLYGON_FACE_UINT8 :
		{
			uint8 *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			for(i = start; i < tri_start; i++)
			{
				table = table_array[i];
				a = (pgreal)layer_data[order_node[i]];
				for(j = 0; j < table->vertex_count; j++)
				{
					output[output_start] = a;
					output_start += 3;
				}
			}
			for(; i < end; i++)
			{
				table = table_array[i];
				a = (pgreal)layer_data[order_node[i]];
				for(j = 0; j < table->vertex_count; j++)
				{
					output[output_start] = a;
					output_start += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_POLYGON_FACE_UINT32 :
		{
			uint32 *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			for(i = start; i < tri_start; i++)
			{
				table = table_array[i];
				a = (pgreal)layer_data[order_node[i]];
				for(j = 0; j < table->vertex_count; j++)
				{
					output[output_start] = a;
					output_start += 3;
				}
			}
			for(; i < end; i++)
			{
				table = table_array[i];
				a = (pgreal)layer_data[order_node[i]];
				for(j = 0; j < table->vertex_count; j++)
				{
					output[output_start] = a;
					output_start += 3;
				}
			}
		}
		break;
		case VN_G_LAYER_POLYGON_FACE_REAL :
		{
			pgreal *layer_data;
			layer_data = e_nsg_get_layer_data(g_node, layer);
			for(i = start; i < tri_start; i++)
			{
				table = table_array[i];
				a = layer_data[order_node[i]];
				for(j = 0; j < table->vertex_count; j++)
				{
					output[output_start] = a;
					output_start += 3;
				}
			}
			for(; i < end; i++)
			{
				table = table_array[i];
				a = layer_data[order_node[i]];
				for(j = 0; j < table->vertex_count; j++)
				{
					output[output_start] = a;
					output_start += 3;
				}
			}
		}
		break;
	}
	return (output_start + channel) / 3;
}
/*

void p_layer_param_create_old(ENode *g_node, PMesh *mesh)
{
	uint i, j, channel, param, material, start;
	ENode *m_node;
	if(mesh->sub_stages[0] == 0)
	{
		mesh->param.array_count = 0;
		for(i = 0; i < mesh->render.mat_count; i++)
			if((m_node = e_ns_get_node(0, mesh->render.mat[i].material)) != NULL)
				if(mesh->param.array_count < p_shader_get_param_count(m_node))
					mesh->param.array_count = p_shader_get_param_count(m_node);
		mesh->param.array = malloc((sizeof *mesh->param.array) * mesh->param.array_count);
		for(i = 0; i < mesh->param.array_count; i++)
			p_ra_clear_array(&mesh->param.array[i]);


		mesh->param.version = malloc((sizeof *mesh->param.version) * mesh->param.array_count * mesh->render.mat_count * 3);
		for(i = 0; i < mesh->param.array_count * mesh->render.mat_count * 3; i++)
			mesh->param.version[i] = 0;
		mesh->param.data_version = e_ns_get_node_version_data(g_node);
		mesh->sub_stages[0]++;
		return;
	}
	if(mesh->sub_stages[0] == 1)
	{
		VMatFrag *frag;
		EGeoLayer *layer = NULL;

		channel = (mesh->sub_stages[1] /  mesh->render.mat_count) % 3;
		material = mesh->sub_stages[1] % mesh->render.mat_count;

		param = mesh->sub_stages[1] / (3 * mesh->render.mat_count);

		if(material == 0)
			mesh->sub_stages[2] = 0;

		if(mesh->sub_stages[1] % 3 * mesh->render.mat_count == 0)
			mesh->temp = p_ra_get_array_real(&mesh->param.array[param], mesh->render.vertex_count);
		j = mesh->render.vertex_count;
		m_node = e_ns_get_node(0, mesh->render.mat[material].material);
		if(m_node != NULL && param < p_shader_get_param_count(m_node))
		{
			frag = p_shader_get_param(m_node, param);
			printf("\n\n%s\n%s\n%s\n\n", frag->geometry.layer_r, frag->geometry.layer_g, frag->geometry.layer_b);
			if(channel == 0)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_r);
			else if(channel == 1)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_g);
			else if(channel == 2)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_b);
			if(layer != 0)
				mesh->param.version[mesh->sub_stages[1]] = e_nsg_get_layer_version(layer);
		}

		if(material > 0)
			start = mesh->render.mat[material - 1].tri_end;
		else
			start = 0;

		mesh->sub_stages[2] = create_param_segment(g_node, mesh->temp, mesh->render.mat[material].vertex_start, (PTessTableElement **)mesh->tess.tess, layer, mesh->tess.order_node, channel, start, mesh->render.mat[material].quad_end, mesh->render.mat[material].tri_end);


		
		mesh->sub_stages[1]++;
		if(mesh->sub_stages[1] % 3 * mesh->render.mat_count == 0)
			p_ra_set_array_real(&mesh->param.array[param], (pgreal *)mesh->temp, mesh->render.vertex_count);

		if(mesh->sub_stages[1] == mesh->param.array_count * mesh->render.mat_count * 3)
		{
			mesh->stage++;
			mesh->sub_stages[0] = 0;
			mesh->sub_stages[1] = 0;
			mesh->sub_stages[2] = 0;
			mesh->sub_stages[3] = 0;
			mesh->temp = NULL;
		}
	}
}
*/

void p_lod_create_layer_param(ENode *g_node, PMesh *mesh)
{
	uint i, j, channel, param, material, start;
	ENode *m_node;
	if(mesh->sub_stages[0] == 0)
	{
		mesh->param.array_count = 0;
		for(i = 0; i < mesh->render.mat_count; i++)
		{
			if((m_node = e_ns_get_node(0, mesh->render.mat[i].material)) != NULL)
			{
				mesh->render.mat[i].material_version = e_ns_get_node_version_struct(m_node);
				if(mesh->param.array_count < p_shader_get_param_count(m_node))
					mesh->param.array_count = p_shader_get_param_count(m_node);
			}
		}
		if(mesh->param.array_count == 0)
		{
			mesh->stage++;
			mesh->param.array = NULL;
			return;
		}

		mesh->param.array = malloc((sizeof *mesh->param.array) * mesh->param.array_count);
		for(i = 0; i < mesh->param.array_count; i++)
			p_ra_clear_array(&mesh->param.array[i]);


		mesh->param.version = malloc((sizeof *mesh->param.version) * mesh->param.array_count * mesh->render.mat_count * 3);
		for(i = 0; i < mesh->param.array_count * mesh->render.mat_count * 3; i++)
			mesh->param.version[i] = 0;
		mesh->param.data_version = e_ns_get_node_version_data(g_node);
		mesh->sub_stages[0]++;
		return;
	}
	if(mesh->sub_stages[0] == 1)
	{
		VMatFrag *frag;
		EGeoLayer *layer = NULL;
		uint32 *ref;

		channel = (mesh->sub_stages[1] /  mesh->render.mat_count) % 3;
		material = mesh->sub_stages[1] % mesh->render.mat_count;
		param = mesh->sub_stages[1] / (3 * mesh->render.mat_count);

		if(mesh->sub_stages[1] % 3 * mesh->render.mat_count == 0)
			mesh->temp = p_ra_get_array_real(&mesh->param.array[param], mesh->render.vertex_count);
		j = mesh->render.vertex_count;
		m_node = e_ns_get_node(0, mesh->render.mat[material].material);
		if(m_node != NULL && param < p_shader_get_param_count(m_node))
		{
			frag = p_shader_get_param(m_node, param);
			if(channel == 0)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_r);
			else if(channel == 1)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_g);
			else if(channel == 2)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_b);
			if(layer != 0)
				mesh->param.version[mesh->sub_stages[1]] = e_nsg_get_layer_version(layer);
		}
		ref = e_nsg_get_layer_data(g_node, e_nsg_get_layer_by_id(g_node,  1));
		mesh->sub_stages[2] = 0;
		for(mesh->sub_stages[3] = 0; mesh->sub_stages[3] < mesh->render.mat[material].tri_end ; mesh->sub_stages[3]++)
		{
			create_param_polygon(g_node, &((pgreal *)mesh->temp)[mesh->sub_stages[2] * 3],
										((PTessTableElement **)mesh->tess.tess)[mesh->sub_stages[3]],
										layer,
										&ref[mesh->tess.order_node[mesh->sub_stages[3]] * 4],
										channel,
										mesh->tess.order_node[mesh->sub_stages[3]],
										mesh->sub_stages[3] < mesh->render.mat[material].quad_end);
			mesh->sub_stages[2] += ((PTessTableElement **)mesh->tess.tess)[mesh->sub_stages[3]]->vertex_count;
		}
		if(mesh->sub_stages[2] == mesh->render.vertex_count)
			mesh->sub_stages[2] = 0;
		if(mesh->sub_stages[3] == mesh->render.mat[material].tri_end)
		{
			mesh->sub_stages[1]++;
			if(mesh->sub_stages[1] % 3 * mesh->render.mat_count == 0)
				p_ra_set_array_real(&mesh->param.array[param], (pgreal *)mesh->temp, mesh->render.vertex_count);
		}
		if(mesh->sub_stages[1] == mesh->param.array_count * mesh->render.mat_count * 3)
		{
			mesh->stage++;
			mesh->sub_stages[0] = 0;
			mesh->sub_stages[1] = 0;
			mesh->sub_stages[2] = 0;
			mesh->sub_stages[3] = 0;
			mesh->temp = NULL;
		}
	}
}

void p_lod_update_material_param_count(ENode *g_node, PMesh *mesh)
{
	ENode *m_node;
	uint i, j, found = 0, *v;
	for(i = 0; i < mesh->render.mat_count; i++)
		if((m_node = e_ns_get_node(0, mesh->render.mat[i].material)) != NULL)
			if(mesh->render.mat[i].material_version != e_ns_get_node_version_struct(m_node))
				if(p_shader_get_param_count(m_node) > found)
					found = p_shader_get_param_count(m_node);


	if(mesh->param.array_count < found)
	{

		mesh->param.array = realloc(mesh->param.array, (sizeof *mesh->param.array) * found);
		v = malloc((sizeof *v) * found * mesh->render.mat_count * 3);

		for(i = 0; i < mesh->param.array_count; i++)
			v[i] = mesh->param.version[i];
		for(i = 0; i < found; i++)
			v[i] = -1;
		free(mesh->param.version);
		mesh->param.version = v;
		for(i = mesh->param.array_count; i < found; i++)
		{
			mesh->param.array[i].length = 0;
			p_ra_get_array_real(&mesh->param.array[i], mesh->render.vertex_count);
		}
		mesh->param.array_count = found;
		mesh->param.data_version = -1;
	}
}

boolean p_lod_update_shadow(ENode *g_node, PMesh *mesh)
{
	uint i;
	if(mesh->render.shadows)
	{
		for(i = 0; i < mesh->render.mat_count; i++)
			if(p_shader_transparancy(mesh->render.mat[i].material))
				break;
		if(i != mesh->render.mat_count)
			mesh->geometry_version--;
	}else if(mesh->render.open_edges == FALSE)
	{
		for(i = 0; i < mesh->render.mat_count; i++)
			if(p_shader_transparancy(mesh->render.mat[i].material))
				break;
		if(i == mesh->render.mat_count)
			mesh->geometry_version--;
	}
	return mesh->render.shadows;
}

boolean p_lod_update_layer_param(ENode *g_node, PMesh *mesh)
{
	uint i, channel, param, material, start;
	ENode *m_node;
	VMatFrag *frag;
	EGeoLayer *layer;
	p_lod_update_material_param_count(g_node, mesh);



	if(mesh->param.data_version == e_ns_get_node_version_data(g_node))
		return FALSE;

	for(i = 0; i < mesh->param.array_count * mesh->render.mat_count * 3; i++)
	{
		channel = (i /  mesh->render.mat_count) % 3;
		material = i % mesh->render.mat_count;
		param = i / (3 * mesh->render.mat_count);	
		
		m_node = e_ns_get_node(0, mesh->render.mat[material].material);
		if(m_node != NULL && param < p_shader_get_param_count(m_node))
		{
			layer = NULL;
			frag = p_shader_get_param(m_node, param);
			if(channel == 0)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_r);
			else if(channel == 1)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_g);
			else if(channel == 2)
				layer = e_nsg_get_layer_by_name(g_node, frag->geometry.layer_b);
			if((layer != NULL && mesh->param.version[i] != e_nsg_get_layer_version(layer)) || (layer == NULL && mesh->param.version[i] != -1))
			{
				if(material > 0)
					start = mesh->render.mat[material - 1].tri_end;
				else
					start = 0;
				create_param_segment(g_node, p_ra_get_array_real(&mesh->param.array[param], mesh->render.vertex_count), mesh->render.mat[material].vertex_start, (PTessTableElement **)mesh->tess.tess, layer, mesh->tess.order_node, channel, mesh->render.mat[material].vertex_start, mesh->render.mat[material].quad_end, mesh->render.mat[material].tri_end);
				if(layer != NULL)
					mesh->param.version[i] = e_nsg_get_layer_version(layer);
				else
					mesh->param.version[i] = -1;
				return TRUE;
			}
		}
	}
	mesh->param.data_version = e_ns_get_node_version_data(g_node);
	return FALSE;
}

