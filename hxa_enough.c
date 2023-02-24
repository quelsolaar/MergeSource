#include <stdio.h>
#include <string.h>
#include "enough.h"
#include "hxa.h"
#include "hxa_utils.h"

/*
void la_geometry_save_hxa(char *file_name)
{
	FILE *obj;
	double *vertex, *select;
	uint *ref, ref_length, vertex_length, i, j, *count, face_count, out_ref_length;
	int32 *out_ref;
	uint8 *out_crease;
	HXALayer layers[4];
	HXANode node;
	HXAFile file;
	HXAMeta meta[2];
	uint32 *crease;
	udg_get_geometry(&vertex_length, &ref_length, &vertex, &ref, NULL);
	out_ref = malloc((sizeof *out_ref) * ref_length);
	out_crease = malloc((sizeof *out_crease) * ref_length);
	select = malloc((sizeof *select) * vertex_length);
	for(i = 0; i < vertex_length; i++)
		select[i] = udg_get_select(i);
	for(node.content.geometry.face_count = node.content.geometry.edge_corner_count = i = 0; i < ref_length; i += 4)
	{
		if(ref[i] < vertex_length && vertex[ref[i] * 3] != V_REAL64_MAX && 
			ref[i + 1] < vertex_length && vertex[ref[i + 1] * 3] != V_REAL64_MAX && 
			ref[i + 2] < vertex_length && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
		{
			if(ref[i + 3] < vertex_length && vertex[ref[i + 3] * 3 + 3] != V_REAL64_MAX)
			{
				out_ref[node.content.geometry.edge_corner_count] = ref[i];
				out_crease[node.content.geometry.edge_corner_count++] = crease[i] == 0;
				out_ref[node.content.geometry.edge_corner_count] = ref[i + 1];
				out_crease[node.content.geometry.edge_corner_count++] = crease[i + 1] == 0;
				out_ref[node.content.geometry.edge_corner_count] = ref[i + 2];
				out_crease[node.content.geometry.edge_corner_count++] = crease[i + 2] == 0;
				out_ref[node.content.geometry.edge_corner_count] = -ref[i + 3] - 1;
				out_crease[node.content.geometry.edge_corner_count++] = crease[i + 3] == 0;
			}else
			{
				out_ref[node.content.geometry.edge_corner_count] = ref[i];
				out_crease[node.content.geometry.edge_corner_count++] = crease[i] == 0;
				out_ref[node.content.geometry.edge_corner_count] = ref[i + 1];
				out_crease[node.content.geometry.edge_corner_count++] = crease[i + 1] == 0;
				out_ref[node.content.geometry.edge_corner_count] = -ref[i + 2] - 1;
				out_crease[node.content.geometry.edge_corner_count++] = crease[i + 2] == 0;
			}
			node.content.geometry.face_count++;
		}
	}
	file.node_array = &node;
	file.node_count = 1;
	node.type = HXA_NT_GEOMETRY; // what type of node is this? Stored in the file as a uint8.
	node.meta_data_count = 0; // how many meta data key/values are stored in the node
	node.meta_data = meta;
	sprintf(meta[0].name, "source");
	meta[0].type = HXA_MDT_TEXT;
	meta[0].value.text_value = "Loq Airou";
	for(meta[0].array_length = 0; meta[0].value.text_value[meta[0].array_length] != 0; meta[0].array_length++);
	sprintf(meta[1].name, "source");
	meta[1].type = HXA_MDT_TEXT;	
	meta[1].value.text_value = e_ns_get_node_name(e_ns_get_node(0, udg_get_modeling_node()));
	for(meta[1].array_length = 0; meta[1].value.text_value[meta[1].array_length] != 0; meta[1].array_length++);

	node.content.geometry.vertex_count = vertex_length;
	node.content.geometry.vertex_stack.layers = &layers[0];
	node.content.geometry.vertex_stack.layer_count = 2;
	node.content.geometry.edge_corner_stack.layers = &layers[0];
	node.content.geometry.edge_corner_stack.layer_count = 2;
	node.content.geometry.face_stack.layers = NULL;
	node.content.geometry.face_stack.layer_count = 0;

	sprintf(layers[0].name, HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME);
	layers[0].components = HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_COMPONENTS;
	layers[0].type = HXA_LDT_DOUBLE;
	layers[0].data.double_data = vertex;

	sprintf(layers[1].name, HXA_CONVENTION_SOFT_LAYER_SELECTION);
	layers[1].components = 1;
	layers[1].type = HXA_LDT_DOUBLE;
	layers[1].data.double_data = select;

	sprintf(layers[2].name, HXA_CONVENTION_HARD_BASE_EDGE_CORNER_LAYER_NAME);
	layers[2].components = HXA_CONVENTION_HARD_BASE_EDGE_CORNER_LAYER_COMPONENTS;
	layers[2].type = HXA_CONVENTION_HARD_BASE_EDGE_CORNER_LAYER_TYPE;
	layers[2].data.double_data = out_ref;

	sprintf(layers[3].name, HXA_CONVENTION_SOFT_LAYER_CREASES);
	layers[3].components = 1;
	layers[3].type = HXA_LDT_UINT8;
	layers[3].data.double_data = out_crease;

	hxa_save(file_name, &file);
	free(out_ref);
	free(out_crease);
	free(select);
}*/

void hxa_to_enough_node_geometry(HXANode *haxa_node, ENode *e_node)
{
//	verse_send_g_layer_create(VNodeID node_id, VLayerID layer_id, const char *name, VNGLayerType type, uint32 def_uint, real64 def_real);

}

void enough_to_hxa_node_geometry(ENode *e_node, HXANode *hxa_node)
{
	EGeoLayer *layer, *ref_layer;
	uint i, j, vertex_length, polygon_length, *ref, edge_count, sides, edge_layers, face_layers;
	double *vertex;
	void *data;
	hxa_node->type = HXA_NT_GEOMETRY; // what type of node is this? Stored in the file as a uint8.
	hxa_node->meta_data = NULL;
	hxa_node->meta_data_count = 0;
	hxa_node->content.geometry.vertex_count = vertex_length = e_nsg_get_vertex_length(e_node);
	polygon_length = e_nsg_get_polygon_length(e_node);
	hxa_node->content.geometry.vertex_stack.layer_count = 0;
	hxa_node->content.geometry.corner_stack.layer_count = 0;
	hxa_node->content.geometry.edge_stack.layer_count = 0;
	hxa_node->content.geometry.edge_stack.layers = NULL;
	hxa_node->content.geometry.face_stack.layer_count = 0;
	f_debug_memory();
	for(layer = e_nsg_get_layer_next(e_node, 0); layer != NULL; layer = e_nsg_get_layer_next(e_node, e_nsg_get_layer_id(layer) + 1))
	{
		switch(e_nsg_get_layer_type(layer))
		{
			case VN_G_LAYER_VERTEX_XYZ :
			case VN_G_LAYER_VERTEX_UINT32 :
			case VN_G_LAYER_VERTEX_REAL :
				hxa_node->content.geometry.vertex_stack.layer_count++;
			break;
			case VN_G_LAYER_POLYGON_CORNER_UINT32 :
			case VN_G_LAYER_POLYGON_CORNER_REAL :
				hxa_node->content.geometry.corner_stack.layer_count++;
			break;
			case VN_G_LAYER_POLYGON_FACE_UINT8 :
			case VN_G_LAYER_POLYGON_FACE_UINT32 :
			case VN_G_LAYER_POLYGON_FACE_REAL :
				hxa_node->content.geometry.face_stack.layer_count++;
			break;
		}
	}	
	if(hxa_node->content.geometry.vertex_stack.layer_count != 0)
		hxa_node->content.geometry.vertex_stack.layers = malloc((sizeof *hxa_node->content.geometry.vertex_stack.layers) * hxa_node->content.geometry.vertex_stack.layer_count);
	else		
		hxa_node->content.geometry.vertex_stack.layers = NULL;
	if(hxa_node->content.geometry.corner_stack.layer_count != 0)
		hxa_node->content.geometry.corner_stack.layers = malloc((sizeof *hxa_node->content.geometry.corner_stack.layers) * hxa_node->content.geometry.corner_stack.layer_count);
	else		
		hxa_node->content.geometry.corner_stack.layers = NULL;

	if(hxa_node->content.geometry.edge_stack.layer_count != 0)
		hxa_node->content.geometry.edge_stack.layers = malloc((sizeof *hxa_node->content.geometry.edge_stack.layers) * hxa_node->content.geometry.edge_stack.layer_count);
	else		
		hxa_node->content.geometry.edge_stack.layers = NULL;

	if(hxa_node->content.geometry.face_stack.layer_count != 0)
		hxa_node->content.geometry.face_stack.layers = malloc((sizeof *hxa_node->content.geometry.face_stack.layers) * hxa_node->content.geometry.face_stack.layer_count);
	else		
		hxa_node->content.geometry.face_stack.layers = NULL;
	f_debug_memory();
	hxa_node->content.geometry.vertex_stack.layer_count = 0;
	hxa_node->content.geometry.corner_stack.layer_count = 0;
	hxa_node->content.geometry.face_stack.layer_count = 0;
	for(layer = e_nsg_get_layer_next(e_node, 0); layer != NULL; layer = e_nsg_get_layer_next(e_node, e_nsg_get_layer_id(layer) + 1))
	{
		switch(e_nsg_get_layer_type(layer))
		{
			case VN_G_LAYER_VERTEX_XYZ :
				sprintf(hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].name, e_nsg_get_layer_name(layer));
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].components = 3;
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].type = HXA_LDT_DOUBLE;
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].data.double_data = malloc(sizeof(double) * 3 * vertex_length);
				memcpy(hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].data.double_data, e_nsg_get_layer_data(e_node, layer), sizeof(double) * 3 * vertex_length);
				hxa_node->content.geometry.vertex_stack.layer_count++;
			break;
			case VN_G_LAYER_VERTEX_UINT32 :			
				sprintf(hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].name, e_nsg_get_layer_name(layer));
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].components = 1;
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].type = HXA_LDT_INT32;
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].data.double_data = malloc(sizeof(int32) * vertex_length);
				memcpy(hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].data.double_data, e_nsg_get_layer_data(e_node, layer), sizeof(int32) * vertex_length);
				hxa_node->content.geometry.vertex_stack.layer_count++;
			break;
			case VN_G_LAYER_VERTEX_REAL :
				sprintf(hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].name, e_nsg_get_layer_name(layer));
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].name[0];
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].components = 1;
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].type = HXA_LDT_DOUBLE;
				hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].data.double_data = malloc(sizeof(double) * vertex_length);
				memcpy(hxa_node->content.geometry.vertex_stack.layers[hxa_node->content.geometry.vertex_stack.layer_count].data.double_data, e_nsg_get_layer_data(e_node, layer), sizeof(double) * vertex_length);
				hxa_node->content.geometry.vertex_stack.layer_count++;
			break;
			case VN_G_LAYER_POLYGON_CORNER_UINT32 :
				sprintf(hxa_node->content.geometry.corner_stack.layers[hxa_node->content.geometry.corner_stack.layer_count].name, e_nsg_get_layer_name(layer));
				hxa_node->content.geometry.corner_stack.layers[hxa_node->content.geometry.corner_stack.layer_count].components = 1;
				hxa_node->content.geometry.corner_stack.layers[hxa_node->content.geometry.corner_stack.layer_count].type = HXA_LDT_INT32;
				hxa_node->content.geometry.corner_stack.layers[hxa_node->content.geometry.corner_stack.layer_count].data.double_data = malloc(sizeof(int32) * polygon_length * 4);
				hxa_node->content.geometry.corner_stack.layer_count++;
			break;
			case VN_G_LAYER_POLYGON_CORNER_REAL :
				sprintf(hxa_node->content.geometry.corner_stack.layers[hxa_node->content.geometry.corner_stack.layer_count].name, e_nsg_get_layer_name(layer));
				hxa_node->content.geometry.corner_stack.layers[hxa_node->content.geometry.corner_stack.layer_count].components = 1;
				hxa_node->content.geometry.corner_stack.layers[hxa_node->content.geometry.corner_stack.layer_count].type = HXA_LDT_DOUBLE;
				hxa_node->content.geometry.corner_stack.layers[hxa_node->content.geometry.corner_stack.layer_count].data.double_data = malloc(sizeof(double) * polygon_length * 4);
				hxa_node->content.geometry.corner_stack.layer_count++;
			break;
			case VN_G_LAYER_POLYGON_FACE_UINT8 :
				sprintf(hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].name, e_nsg_get_layer_name(layer));
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].components = 1;
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].type = HXA_LDT_UINT8;
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].data.double_data = malloc(sizeof(double) * polygon_length);
				hxa_node->content.geometry.face_stack.layer_count++;
			break;
			case VN_G_LAYER_POLYGON_FACE_UINT32 :
				sprintf(hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].name, e_nsg_get_layer_name(layer));
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].components = 1;
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].type = HXA_LDT_INT32;
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].data.double_data = malloc(sizeof(double) * polygon_length);
				hxa_node->content.geometry.face_stack.layer_count++;
			break;
			case VN_G_LAYER_POLYGON_FACE_REAL :
				sprintf(hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].name, e_nsg_get_layer_name(layer));
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].components = 1;
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].type = HXA_LDT_DOUBLE;
				hxa_node->content.geometry.face_stack.layers[hxa_node->content.geometry.face_stack.layer_count].data.double_data = malloc(sizeof(double) * polygon_length);
				hxa_node->content.geometry.face_stack.layer_count++;
			break;
		}
	}	
	layer = e_nsg_get_layer_by_id(e_node, 0);
	vertex = e_nsg_get_layer_data(e_node, layer);	
	ref_layer = e_nsg_get_layer_by_id(e_node, 1);
	ref = e_nsg_get_layer_data(e_node, ref_layer);
	hxa_node->content.geometry.face_count = 0;
	f_debug_memory();
	for(i = edge_count = 0; i < polygon_length; i++)
	{
		if(ref[i * 4 + 0] < vertex_length && vertex[ref[i * 4 + 0] * 3] != V_REAL64_MAX &&
			ref[i * 4 + 1] < vertex_length && vertex[ref[i * 4 + 1] * 3] != V_REAL64_MAX &&
			ref[i * 4 + 2] < vertex_length && vertex[ref[i * 4 + 2] * 3] != V_REAL64_MAX)
		{
			if(ref[i * 4 + 3] < vertex_length && vertex[ref[i * 4 + 3] * 3] != V_REAL64_MAX)
			{
				hxa_node->content.geometry.corner_stack.layers[0].data.int32_data[edge_count + 0] = ref[i * 4 + 0];
				hxa_node->content.geometry.corner_stack.layers[0].data.int32_data[edge_count + 1] = ref[i * 4 + 1];
				hxa_node->content.geometry.corner_stack.layers[0].data.int32_data[edge_count + 2] = ref[i * 4 + 2];
				hxa_node->content.geometry.corner_stack.layers[0].data.int32_data[edge_count + 3] = -1 - ((int)ref[i * 4 + 3]);
				sides = 4;
			}else 
			{
				hxa_node->content.geometry.corner_stack.layers[0].data.int32_data[edge_count + 0] = ref[i * 4 + 0];
				hxa_node->content.geometry.corner_stack.layers[0].data.int32_data[edge_count + 1] = ref[i * 4 + 1];
				hxa_node->content.geometry.corner_stack.layers[0].data.int32_data[edge_count + 2] = -1 - ((int)ref[i * 4 + 2]);
				sides = 3;
			}
		
			edge_layers = 1;
			face_layers = 0;
			for(layer = e_nsg_get_layer_next(e_node, 2); layer != NULL; layer = e_nsg_get_layer_next(e_node, e_nsg_get_layer_id(layer) + 1))
			{
				data = e_nsg_get_layer_data(e_node, layer);
				switch(e_nsg_get_layer_type(layer))
				{
					case VN_G_LAYER_VERTEX_XYZ :
					case VN_G_LAYER_VERTEX_UINT32 :			
					case VN_G_LAYER_VERTEX_REAL :
					break;
					case VN_G_LAYER_POLYGON_CORNER_UINT32 :
						for(j = 0; j < sides; j++)
							hxa_node->content.geometry.corner_stack.layers[edge_layers].data.int32_data[edge_count + j] = ((uint *)data)[i * 4 + j];
						edge_layers++;
					break;
					case VN_G_LAYER_POLYGON_CORNER_REAL :
						for(j = 0; j < sides; j++)
							hxa_node->content.geometry.corner_stack.layers[edge_layers].data.double_data[edge_count + j] = ((uint *)data)[i * 4 + j];
						edge_layers++;
					break;
					case VN_G_LAYER_POLYGON_FACE_UINT8 :
						hxa_node->content.geometry.face_stack.layers[face_layers].data.uint8_data[edge_count] = ((uint8 *)data)[i];
						face_layers++;
					break;
					case VN_G_LAYER_POLYGON_FACE_UINT32 :
						hxa_node->content.geometry.face_stack.layers[face_layers].data.int32_data[edge_count] = ((uint32 *)data)[i];
						face_layers++;
					break;
					case VN_G_LAYER_POLYGON_FACE_REAL :
						hxa_node->content.geometry.face_stack.layers[face_layers].data.double_data[edge_count] = ((double *)data)[i];
						face_layers++;
					break;
				}
			}
			hxa_node->content.geometry.face_count++;
			edge_count += sides;
		}
	}
	f_debug_memory();
	hxa_node->content.geometry.edge_corner_count = edge_count;

	for(i = 0; i < edge_count; i++)
		printf("ref[%u] = %i\n", i, hxa_node->content.geometry.corner_stack.layers[0].data.int32_data[i]);

//	hxa_util_node_vertex_purge(hxa_node);
}


void enough_to_hxa_one(char *file_name, ENode *e_node)
{
	HXAFile *file;
	HXANode *hxa_node;
	file = malloc(sizeof *file);
	hxa_node = malloc(sizeof *hxa_node);
	file->node_array = hxa_node;
	enough_to_hxa_node_geometry(e_node, hxa_node);
	file->node_count = 1;
	hxa_print(file, TRUE);
	hxa_save(file_name, file);
	hxa_util_free_file(file);
}

void enough_to_hxa_all(char *file_name)
{
	ENode *e_node;
	HXAFile *file;
	HXANode *hxa_node;
	file = malloc(sizeof *file);
	file->node_count = 0;
	for(e_node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY); e_node != NULL; e_node = e_ns_get_node_next(e_ns_get_node_id(e_node) + 1, 0, V_NT_GEOMETRY))
		file->node_count++;
	hxa_node = malloc((sizeof *hxa_node) * file->node_count);
	file->node_array = hxa_node;
	file->node_count = 0;
	for(e_node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY); e_node != NULL; e_node = e_ns_get_node_next(e_ns_get_node_id(e_node) + 1, 0, V_NT_GEOMETRY))
		enough_to_hxa_node_geometry(e_node, &hxa_node[file->node_count++]);
	hxa_save(file_name, file);
	hxa_util_free_file(file);
}