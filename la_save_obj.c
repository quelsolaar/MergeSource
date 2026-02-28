#include "la_includes.h"

#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_draw_overlay.h"
#include "la_tool.h"
#include "la_pop_up.h"
#include "hxa.h"


uint geometry_read_uint(char *string, uint *output)
{
	uint i;
	*output = 0;
	for(i = 0; string[i] >= 48 && string[i] <= 58; i++)
		*output = (string[i] - 48) + (*output) * 10;
	return i;
}

void la_geometry_load_obj(char *file)
{
	FILE *obj;
	char line[512];
	ENode *node;
	EGeoLayer *layer;
	uint i, poly_id, ref_length = 0, vertex_length = 1, uv_length = 1, count, ref[5], uv_ref[5], *vertex_ids, poly_i, uv_layer_ids[2] = {2, 3};
	float x, y, z, *uv_array;

	obj = fopen(file, "r");
	if(obj != NULL)
	{
		for(i = 0; i < 512; i++)
			line[i] = 0;
		while((fgets(line, sizeof line, obj)) != NULL)
		{
			if(line[0] == 'v')
			{
				if(line[1] == 't')
				{
					if(sscanf(line, "vt %f %f", &x, &y) == 2)
						uv_length++;
				}else
					if(sscanf(line, "v %f %f %f", &x, &y, &z) == 3)
						vertex_length++;
			}
		}
		vertex_ids = malloc((sizeof *vertex_ids) * vertex_length);
		uv_array = malloc((sizeof *uv_array) * uv_length * 2);
		for(i = 0; i < vertex_length; i++)
			vertex_ids[i] = udg_find_empty_slot_vertex();
		rewind(obj); 

		if(uv_length > 0)
		{
			node = e_ns_get_node(0, udg_get_modeling_node());

			if(node != NULL)
			{

				if(e_nsg_get_layer_by_name(node, "map_u") == NULL || e_nsg_get_layer_by_name(node, "map_v") == NULL)
				{
					if(e_nsg_get_layer_by_name(node, "map_u") == NULL)
						verse_send_g_layer_create(udg_get_modeling_node(), -1, "map_u", VN_G_LAYER_POLYGON_CORNER_REAL, 0, 0);
					if(e_nsg_get_layer_by_name(node, "map_v") == NULL)
						verse_send_g_layer_create(udg_get_modeling_node(), -1, "map_v", VN_G_LAYER_POLYGON_CORNER_REAL, 0, 0);
					while(e_nsg_get_layer_by_name(node, "map_u") == NULL || 
							e_nsg_get_layer_by_name(node, "map_v") == NULL)
							verse_callback_update(1000);
					
				}
				uv_layer_ids[0] = e_nsg_get_layer_id(e_nsg_get_layer_by_name(node, "map_u"));
				uv_layer_ids[1] = e_nsg_get_layer_id(e_nsg_get_layer_by_name(node, "map_v"));
			}
		}
		uv_length = 0;
		i = 0;
		while((fgets(line, sizeof line, obj)) != NULL)
		{
			if(line[0] == 'v')
			{
				if(line[1] == 't')
				{
					count = sscanf(line, "vt %f %f ", &uv_array[uv_length * 2], &uv_array[uv_length * 2 + 1]);
					if(count == 2)
						uv_length++;
				}else
				{
					count = sscanf(line, "v %f %f %f", &x, &y, &z);
					if(count == 3)
						udg_vertex_set(vertex_ids[i++], NULL, x * 0.1, y * 0.1, z * 0.1);
				}
			}else if(line[0] == 'f') /* f */
			{
				uint pos = 2, add, value, corner = 0, on = 0, next;
				uv_ref[0] = uv_ref[1] = uv_ref[2] = uv_ref[3] = 0;
				while(line[pos] == ' ')
					pos++;
				while(line[pos] != 0 && corner < 5)
				{
					add = geometry_read_uint(&line[pos], &value);
					if(add != 0)
					{
						if(on == 0)
							ref[corner] = value - 1;
						else if(on == 1)
							uv_ref[corner] = value - 1;
					}
					pos += add;
					next = 0;
					while(line[pos] != 0 && (line[pos] < '0' || line[pos] > '9'))
					{
						if(line[pos] == '/')
							next++;
						pos++;
					}
					if(next == 0)
					{
						on = 0;
						corner++;
					}else
						on = next;
				}
				if(corner == 4 || corner == 3)
				{
					poly_id = udg_find_empty_slot_polygon();
					if(corner == 4)
					{
						printf("Corner 4 %u %u %u %u\n", ref[0], ref[1], ref[2], ref[3]);
						udg_polygon_set(poly_id, vertex_ids[ref[0]], vertex_ids[ref[1]], vertex_ids[ref[2]], vertex_ids[ref[3]]);
					}else
					{
						printf("Corner 3 %u %u %u %u\n", ref[0], ref[1], ref[2], ref[3]);
						udg_polygon_set(poly_id, vertex_ids[ref[0]], vertex_ids[ref[1]], vertex_ids[ref[2]], -1);
					}
			//		verse_send_g_polygon_set_corner_real64(udg_get_modeling_node(), uv_layer_ids[0], poly_id, uv_array[uv_ref[0] * 2 + 0], uv_array[uv_ref[1] * 2 + 0], uv_array[uv_ref[2] * 2 + 0], uv_array[uv_ref[3] * 2 + 0]);
			//		verse_send_g_polygon_set_corner_real64(udg_get_modeling_node(), uv_layer_ids[1], poly_id, uv_array[uv_ref[0] * 2 + 1], uv_array[uv_ref[1] * 2 + 1], uv_array[uv_ref[2] * 2 + 1], uv_array[uv_ref[3] * 2 + 1]);
				}
			}
		}
		fclose(obj);
	}
}

void la_geometry_save_obj(char *file)
{
	ENode *node;
	FILE *obj;
	double *vertex;
	uint *ref, ref_length, vertex_length, i, j, *count;
	udg_get_geometry(&vertex_length, &ref_length, &vertex, &ref, NULL);

	obj = fopen(file, "wt");
	if(obj != NULL)
	{
		count = malloc((sizeof *count) * vertex_length);
		for(i = 0; i < vertex_length; i++)
			count[i] = 0;
		for(i = 0; i < ref_length ; i++)
		{
			if(ref[i * 4] < vertex_length && ref[i * 4 + 1] < vertex_length && ref[i * 4 + 2] < vertex_length &&
				vertex[ref[i * 4] * 3] != V_REAL64_MAX && vertex[ref[i * 4 + 1] * 3] != V_REAL64_MAX && vertex[ref[i * 4 + 2] * 3] != V_REAL64_MAX)
			{
				count[ref[i * 4]]++;
				count[ref[i * 4 + 1]]++;
				count[ref[i * 4 + 2]]++;
				if(ref[i * 4 + 3] < vertex_length && vertex[ref[i * 4 + 3] * 3] != V_REAL64_MAX)
					count[ref[i * 4 + 3]]++;
			}
		}
		fprintf(obj, "# Object exported from Loq Airou by Eskil Steenberg (http://www.quelsolaar.com )\n\n");
		fprintf(obj, "g\n\n");
		for(i = 0; i < vertex_length ; i++)
			if(count[i] != 0)
				fprintf(obj, "v %f %f %f\n", vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);

		j = 0;
		for(i = 0; i < vertex_length; i++)
		{
			if(count[i] == 0)
				count[i] = j;
			else
				count[i] = j++;
		}
		node = e_ns_get_node(0, udg_get_modeling_node());
		if(node != NULL)
			fprintf(obj, "\ng %s\n\n", e_ns_get_node_name(node));
		else
			fprintf(obj, "\ng SDS_Cage\n\n");
		for(i = 0; i < ref_length ; i++)
		{
			if(ref[i * 4] < vertex_length && ref[i * 4 + 1] < vertex_length && ref[i * 4 + 2] < vertex_length)
			{
				if(ref[i * 4 + 3] < vertex_length)
					fprintf(obj, "f %i %i %i %i\n", count[ref[i * 4]] + 1, count[ref[i * 4 + 1]] + 1, count[ref[i * 4 + 2]] + 1, count[ref[i * 4 + 3]] + 1);
				else
					fprintf(obj, "f %i %i %i\n", count[ref[i * 4]] + 1, count[ref[i * 4 + 1]] + 1, count[ref[i * 4 + 2]] + 1);
			}
		}
		fprintf(obj, "\ng\n\n");
		fclose(obj);
	}
}
/*
#include "gather.h"

GatherData *la_save_data = NULL;

void geometry_save_obj(void *user, char *file)
{
	la_save_data = gather_load("halon.obj", G_FORMAT_WAVEFRONT_OBJ);
}

extern void udg_create_func(uint connection, uint id, VNodeType type, void *user);

void geometry_load_update()
{
	if(la_save_data != NULL)
	{
		if(gather_node_save_verse_geometry(la_save_data))
		{
			gather_free(la_save_data);
			la_save_data = NULL;
			e_ns_set_node_create_func(udg_create_func, NULL);
		}
	}
}*/


boolean la_geometry_load_layers_create(ENode *node, char *name, char *posfix_a, char *posfix_b, char *posfix_c, char *posfix_d, uint *vertex_ids, uint *poly_ids, VNGLayerType type, uint count, HXALayer *hxa_layer, HXALayer *hxa_reference_layer)
{
	VNodeID node_id;
	EGeoLayer *layer = NULL;
	VLayerID layer_id;
	char names[4][64];
	uint i, j, k;
	node_id = e_ns_get_node_id(node);
	sprintf(names[0], "%s%s", name, posfix_a);
	if(posfix_b == NULL)
		names[1][0] = 0;
	else
		sprintf(names[1], "%s%s", name, posfix_b);
	if(posfix_c == NULL)
		names[2][0] = 0;
	else
		sprintf(names[2], "%s%s", name, posfix_c);
	if(posfix_b == NULL)
		names[3][0] = 0;
	else
		sprintf(names[3], "%s%s", name, posfix_d);
	for(i = 0; i < 4; i++)
		if(names[i][0] != 0 && e_nsg_get_layer_by_name(node, names[i]) == NULL)
			verse_send_g_layer_create(udg_get_modeling_node(), -1, names[i], type, 0, 0);

	for(i = 0; i < 4;)
	{
		verse_callback_update(1000);
		for(i = 0; i < 4; i++)
			if(names[i][0] != 0 && e_nsg_get_layer_by_name(node, names[i]) == NULL)
				break;
	}

	for(i = 0; i < 4; i++)
	{
		layer = e_nsg_get_layer_by_name(node, names[i]);
		if(layer != NULL)
		{
			layer_id = e_nsg_get_layer_id(layer);
			switch(type)
			{
				case VN_G_LAYER_VERTEX_XYZ :
					if(hxa_layer->type == HXA_LDT_FLOAT && hxa_layer->components >= 3)
					{
						for(j = 0; j < count; j++)
							verse_send_g_vertex_set_xyz_real32(node_id, layer_id, vertex_ids[j], hxa_layer->data.float_data[j * hxa_layer->components], hxa_layer->data.float_data[j * hxa_layer->components + 1], hxa_layer->data.float_data[j * hxa_layer->components + 2]);
					}
					if(hxa_layer->type == HXA_LDT_DOUBLE && hxa_layer->components >= 3)
					{
						for(j = 0; j < count; j++)
							verse_send_g_vertex_set_xyz_real64(node_id, layer_id, vertex_ids[j], hxa_layer->data.double_data[j * hxa_layer->components], hxa_layer->data.double_data[j * hxa_layer->components + 1], hxa_layer->data.double_data[j * hxa_layer->components + 2]);
					}
				break;
				case VN_G_LAYER_VERTEX_UINT32 :
					if(hxa_layer->type == HXA_LDT_UINT8)
					{
						for(j = 0; j < count; j++)
							verse_send_g_vertex_set_uint32(node_id, layer_id, vertex_ids[j], hxa_layer->data.uint8_data[j * hxa_layer->components + i]);
					}
					if(hxa_layer->type == HXA_LDT_INT32)
					{
						for(j = 0; j < count; j++)
							verse_send_g_vertex_set_uint32(node_id, layer_id, vertex_ids[j], hxa_layer->data.int32_data[j * hxa_layer->components + i]);
					}
				break;
				case VN_G_LAYER_VERTEX_REAL :
					if(hxa_layer->type == HXA_LDT_FLOAT)
					{
						for(j = 0; j < count; j++)
							verse_send_g_vertex_set_real32(node_id, layer_id, vertex_ids[j], hxa_layer->data.float_data[j * hxa_layer->components + i]);
					}
					if(hxa_layer->type == HXA_LDT_DOUBLE)
					{
						for(j = 0; j < count; j++)
							verse_send_g_vertex_set_real64(node_id, layer_id, vertex_ids[j], hxa_layer->data.double_data[j * hxa_layer->components + i]);
					}
				break;
				case VN_G_LAYER_POLYGON_CORNER_UINT32 :
					{
						if(hxa_reference_layer == hxa_layer)
						{
							for(j = k = 0; j < count; j++)
							{
								if(hxa_reference_layer->data.int32_data[k + 2] >= 0)
								{
									verse_send_g_polygon_set_corner_uint32(node_id, layer_id, poly_ids[j], vertex_ids[hxa_reference_layer->data.int32_data[k + 0]], vertex_ids[hxa_reference_layer->data.int32_data[k + 1]], vertex_ids[hxa_reference_layer->data.int32_data[k + 2]], vertex_ids[-1 - hxa_reference_layer->data.int32_data[k + 3]]);
									k += 4;
								}else
								{
									verse_send_g_polygon_set_corner_uint32(node_id, layer_id, poly_ids[j], vertex_ids[hxa_reference_layer->data.int32_data[k + 0]], vertex_ids[hxa_reference_layer->data.int32_data[k + 1]], vertex_ids[-1 - hxa_reference_layer->data.int32_data[k + 2]], ~0);
									k += 3;
								}
							}
						}else if(hxa_layer->type == HXA_LDT_UINT8)
						{
							for(j = k = 0; j < count; j++)
							{
								if(hxa_reference_layer->data.int32_data[k + 2] >= 0)
								{
									verse_send_g_polygon_set_corner_uint32(node_id, layer_id, poly_ids[j],
																						hxa_layer->data.uint8_data[(k + 0) * hxa_layer->components + i], 
																						hxa_layer->data.uint8_data[(k + 1) * hxa_layer->components + i], 
																						hxa_layer->data.uint8_data[(k + 2) * hxa_layer->components + i], 
																						hxa_layer->data.uint8_data[(k + 3) * hxa_layer->components + i]);
									k += 4;
								}else
								{
									verse_send_g_polygon_set_corner_uint32(node_id, layer_id, poly_ids[j], 
																						hxa_layer->data.uint8_data[(k + 0) * hxa_layer->components + i], 
																						hxa_layer->data.uint8_data[(k + 1) * hxa_layer->components + i], 
																						hxa_layer->data.uint8_data[(k + 2) * hxa_layer->components + i], ~0);
									k += 3;
								}
							}						
						}else if(hxa_layer->type == HXA_LDT_INT32)
						{
							for(j = k = 0; j < count; j++)
							{
								if(hxa_reference_layer->data.int32_data[k + 2] >= 0)
								{
									verse_send_g_polygon_set_corner_uint32(node_id, layer_id, poly_ids[j],
																						hxa_layer->data.int32_data[(k + 0) * hxa_layer->components + i], 
																						hxa_layer->data.int32_data[(k + 1) * hxa_layer->components + i], 
																						hxa_layer->data.int32_data[(k + 2) * hxa_layer->components + i], 
																						hxa_layer->data.int32_data[(k + 3) * hxa_layer->components + i]);
									k += 4;
								}else
								{
									verse_send_g_polygon_set_corner_uint32(node_id, layer_id, poly_ids[j], 
																						hxa_layer->data.int32_data[(k + 0) * hxa_layer->components + i], 
																						hxa_layer->data.int32_data[(k + 1) * hxa_layer->components + i], 
																						hxa_layer->data.int32_data[(k + 2) * hxa_layer->components + i], ~0);
									k += 3;
								}
							}						
						}
					}
				break;
				case VN_G_LAYER_POLYGON_CORNER_REAL :
					if(hxa_layer->type == HXA_LDT_DOUBLE)
					{
						for(j = k = 0; j < count; j++)
						{
							if(hxa_reference_layer->data.int32_data[k + 2] >= 0)
							{
								verse_send_g_polygon_set_corner_real64(node_id, layer_id, poly_ids[j],
																					hxa_layer->data.double_data[(k + 0) * hxa_layer->components + i], 
																					hxa_layer->data.double_data[(k + 1) * hxa_layer->components + i], 
																					hxa_layer->data.double_data[(k + 2) * hxa_layer->components + i], 
																					hxa_layer->data.double_data[(k + 3) * hxa_layer->components + i]);
								k += 4;
							}else
							{
								verse_send_g_polygon_set_corner_real64(node_id, layer_id, poly_ids[j], 
																					hxa_layer->data.double_data[(k + 0) * hxa_layer->components + i], 
																					hxa_layer->data.double_data[(k + 1) * hxa_layer->components + i], 
																					hxa_layer->data.double_data[(k + 2) * hxa_layer->components + i], ~0);
								k += 3;
							}
						}						
					}else if(hxa_layer->type == HXA_LDT_FLOAT)
					{
						for(j = k = 0; j < count; j++)
						{
							if(hxa_reference_layer->data.int32_data[k + 2] >= 0)
							{
								verse_send_g_polygon_set_corner_real32(node_id, layer_id, poly_ids[j],
																					hxa_layer->data.float_data[(k + 0) * hxa_layer->components + i], 
																					hxa_layer->data.float_data[(k + 1) * hxa_layer->components + i], 
																					hxa_layer->data.float_data[(k + 2) * hxa_layer->components + i], 
																					hxa_layer->data.float_data[(k + 3) * hxa_layer->components + i]);
								k += 4;
							}else
							{
								verse_send_g_polygon_set_corner_real32(node_id, layer_id, poly_ids[j], 
																					hxa_layer->data.float_data[(k + 0) * hxa_layer->components + i], 
																					hxa_layer->data.float_data[(k + 1) * hxa_layer->components + i], 
																					hxa_layer->data.float_data[(k + 2) * hxa_layer->components + i], ~0);
								k += 3;
							}
						}						
					}

				break;
				case VN_G_LAYER_POLYGON_FACE_UINT8 :
					if(hxa_layer->type == HXA_LDT_UINT8)
					{
						for(j = 0; j < count; j++)
							verse_send_g_polygon_set_face_uint8(node_id, layer_id, poly_ids[j], hxa_layer->data.uint8_data[j * hxa_layer->components + i]);
					}
				break;
				case VN_G_LAYER_POLYGON_FACE_UINT32 :
					if(hxa_layer->type == HXA_LDT_INT32)
					{
						for(j = 0; j < count; j++)
							verse_send_g_polygon_set_face_uint32(node_id, layer_id, poly_ids[j], hxa_layer->data.int32_data[j * hxa_layer->components + i]);
					}
				break;
				case VN_G_LAYER_POLYGON_FACE_REAL :
					if(hxa_layer->type == HXA_LDT_FLOAT)
					{
						for(j = 0; j < count; j++)
							verse_send_g_polygon_set_face_real32(node_id, layer_id, poly_ids[j], hxa_layer->data.float_data[j * hxa_layer->components + i]);
					}
					if(hxa_layer->type == HXA_LDT_DOUBLE)
					{
						for(j = 0; j < count; j++)
							verse_send_g_polygon_set_face_real64(node_id, layer_id, poly_ids[j], hxa_layer->data.double_data[j * hxa_layer->components + i]);
					}
				break;
			}
		}
	}
	return TRUE;
}


void la_geometry_load_hxa_node(ENode *node, HXANode *hxa_node)
{
	uint i, vertex_length = 1, poly_length = 1, *vertex_ids, *poly_ids;

	vertex_length = hxa_node->content.geometry.vertex_count;
	vertex_ids = malloc((sizeof *vertex_ids) * vertex_length); 
	for(i = 0; i < vertex_length; i++)
	{
		vertex_ids[i] = udg_find_empty_slot_vertex();
	}

	poly_length = hxa_node->content.geometry.face_count;
	poly_ids = malloc((sizeof *vertex_ids) * poly_length); 
	for(i = 0; i < poly_length; i++)
		poly_ids[i] = udg_find_empty_slot_polygon();


	la_geometry_load_layers_create(node, "vertex", "", NULL, NULL, NULL, vertex_ids, poly_ids, VN_G_LAYER_VERTEX_XYZ, vertex_length, &hxa_node->content.geometry.vertex_stack.layers[HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_ID], &hxa_node->content.geometry.corner_stack.layers[HXA_CONVENTION_HARD_BASE_CORNER_LAYER_ID]);
	la_geometry_load_layers_create(node, "polygon", "", NULL, NULL, NULL, vertex_ids, poly_ids, VN_G_LAYER_POLYGON_CORNER_UINT32, poly_length, &hxa_node->content.geometry.corner_stack.layers[HXA_CONVENTION_HARD_BASE_CORNER_LAYER_ID], &hxa_node->content.geometry.corner_stack.layers[HXA_CONVENTION_HARD_BASE_CORNER_LAYER_ID]);

}

extern HXAFile *hxa_load(char *file_name, int silent);
extern void hxa_util_free_file(HXAFile *file);

void la_geometry_load_hxa(char *file_name)
{
	HXAFile	*file;
	ENode *node;
	node = e_ns_get_node(0, udg_get_modeling_node());
	if(node != NULL)
	{ 
		file = hxa_load(file_name, TRUE); /* Load a Hxa file in to memory.*/
		if(file != NULL)
		{
			if(file->node_count > 0)	
				la_geometry_load_hxa_node(node, file->node_array);
			hxa_util_free_file(file);
		}
	}
} 