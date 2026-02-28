#include "la_includes.h"
#include "la_geometry_undo.h"

uint *la_polygon_buffer = NULL;
uint la_polygon_buffer_length = 0;
uint *la_crease_buffer = NULL;
egreal *la_vertex_buffer = NULL;
uint la_vertex_buffer_length = 0;
uint *la_edge_buffer = NULL;
uint la_edge_buffer_length = 0;
uint la_new_node_paste = -1;

void la_t_copy(egreal *pos)
{
	uint32 vertex_count, ref_length, i, *ref, *crease, *translate;
	egreal *vertex;
	udg_get_geometry(&vertex_count, &ref_length, &vertex, &ref, &crease);
	ref_length *= 4;

	if(la_polygon_buffer != NULL)
	{
		free(la_polygon_buffer);
		la_polygon_buffer = NULL; 
		free(la_crease_buffer);
		la_crease_buffer = NULL; 
	}
	if(la_vertex_buffer != NULL)
	{
		free(la_vertex_buffer);
		la_vertex_buffer = NULL; 
	}
	if(la_edge_buffer != NULL)
	{
		free(la_edge_buffer);
		la_edge_buffer = NULL; 
	}

	la_vertex_buffer_length = 0;

	translate = malloc((sizeof *translate) * vertex_count);
	for(i = 0; i < vertex_count; i++)
	{
		if(udg_get_select(i) > 0.1)
			translate[i] = la_vertex_buffer_length++;
		else 
			translate[i] = 0;
	}

	if(la_vertex_buffer_length == 0)
		return;

	la_vertex_buffer = malloc((sizeof *la_vertex_buffer) * la_vertex_buffer_length * 3);
	la_vertex_buffer_length = 0;
	for(i = 0; i < vertex_count; i++)
	{
		if(udg_get_select(i) > 0.1)
		{
			la_vertex_buffer[la_vertex_buffer_length * 3 + 0] = vertex[i * 3 + 0] - pos[0];
			la_vertex_buffer[la_vertex_buffer_length * 3 + 1] = vertex[i * 3 + 1] - pos[1];
			la_vertex_buffer[la_vertex_buffer_length * 3 + 2] = vertex[i * 3 + 2] - pos[2];
			la_vertex_buffer_length++;
		}
	}


	la_polygon_buffer_length = 0;
	for(i = 0; i < ref_length; i += 4)
		if((ref[i + 0] < vertex_count && vertex[ref[i + 0] * 3] != E_REAL_MAX && udg_get_select(ref[i + 0]) > 0.01) &&
		   (ref[i + 1] < vertex_count && vertex[ref[i + 1] * 3] != E_REAL_MAX && udg_get_select(ref[i + 1]) > 0.01) &&
		   (ref[i + 2] < vertex_count && vertex[ref[i + 2] * 3] != E_REAL_MAX && udg_get_select(ref[i + 2]) > 0.01) &&
		   (ref[i + 3] >= vertex_count || vertex[ref[i + 3] * 3] == E_REAL_MAX || udg_get_select(ref[i + 3]) > 0.01))
			la_polygon_buffer_length++;

	if(la_polygon_buffer_length != 0)
	{
		la_polygon_buffer = malloc((sizeof *la_polygon_buffer) * la_polygon_buffer_length * 4);
		la_crease_buffer = malloc((sizeof *la_crease_buffer) * la_polygon_buffer_length * 4);
		la_polygon_buffer_length = 0;
		for(i = 0; i < ref_length; i += 4)
		{
			if((ref[i + 0] < vertex_count && vertex[ref[i + 0] * 3] != E_REAL_MAX && udg_get_select(ref[i + 0]) > 0.01) &&
			   (ref[i + 1] < vertex_count && vertex[ref[i + 1] * 3] != E_REAL_MAX && udg_get_select(ref[i + 1]) > 0.01) &&
			   (ref[i + 2] < vertex_count && vertex[ref[i + 2] * 3] != E_REAL_MAX && udg_get_select(ref[i + 2]) > 0.01) &&
		       (ref[i + 3] >= vertex_count || vertex[ref[i + 3] * 3] == E_REAL_MAX || udg_get_select(ref[i + 3]) > 0.01))	
			{
				la_polygon_buffer[la_polygon_buffer_length] = translate[ref[i]];
				la_crease_buffer[la_polygon_buffer_length++] = crease[i];
				la_polygon_buffer[la_polygon_buffer_length] = translate[ref[i + 1]];
				la_crease_buffer[la_polygon_buffer_length++] = crease[i + 1];
				la_polygon_buffer[la_polygon_buffer_length] = translate[ref[i + 2]];
				la_crease_buffer[la_polygon_buffer_length++] = crease[i + 2];
				if(ref[i + 3] < vertex_count)
					la_polygon_buffer[la_polygon_buffer_length] = translate[ref[i + 3]];
				la_crease_buffer[la_polygon_buffer_length++] = crease[i + 3];
			}
		}
		la_polygon_buffer_length /= 4;
	}

	la_edge_buffer_length = 0;
	ref = udg_get_edge_data(&ref_length);
	for(i = 0; i < ref_length; i++)
		if(udg_get_select(ref[i * 2]) > 0.1 && udg_get_select(ref[i * 2 + 1]) > 0.1)
			la_edge_buffer_length++;
	
	if(la_edge_buffer_length != 0)
	{
		la_edge_buffer = malloc((sizeof *la_edge_buffer) * la_edge_buffer_length * 2);
		la_edge_buffer_length = 0;
		for(i = 0; i < ref_length; i++)
		{
			if(udg_get_select(ref[i * 2]) > 0.1 && udg_get_select(ref[i * 2 + 1]) > 0.1)
			{
				la_edge_buffer[la_edge_buffer_length * 2 + 0] = translate[ref[i * 2 + 0]];
				la_edge_buffer[la_edge_buffer_length * 2 + 1] = translate[ref[i * 2 + 1]];
				la_edge_buffer_length++;
			}
		}
	}
	free(translate);


}

void la_t_paste(egreal *pos)
{
	uint *temp, i, id;

	if(la_vertex_buffer_length == 0)
		return;

	temp = malloc((sizeof *temp) * la_vertex_buffer_length);

	for(i = 0; i < la_vertex_buffer_length; i++)
	{
		temp[i] = udg_find_empty_slot_vertex();
		udg_vertex_set(temp[i], NULL, la_vertex_buffer[i * 3 + 0] + pos[0],
										la_vertex_buffer[i * 3 + 1] + pos[1],
										la_vertex_buffer[i * 3 + 2] + pos[2]);
	}

	for(i = 0; i < la_polygon_buffer_length; i++)
	{
		id = udg_find_empty_slot_polygon();
		if(la_polygon_buffer[i * 4 + 3] < la_vertex_buffer_length)
			udg_polygon_set(id, temp[la_polygon_buffer[i * 4 + 0]],
								temp[la_polygon_buffer[i * 4 + 1]], 
								temp[la_polygon_buffer[i * 4 + 2]], 
								temp[la_polygon_buffer[i * 4 + 3]]);
		else
			udg_polygon_set(id, temp[la_polygon_buffer[i * 4 + 0]],
								temp[la_polygon_buffer[i * 4 + 1]], 
								temp[la_polygon_buffer[i * 4 + 2]], 
								-1);
		udg_crease_set(id, la_crease_buffer[i * 4 + 0],
							la_crease_buffer[i * 4 + 1],
							la_crease_buffer[i * 4 + 2],
							la_crease_buffer[i * 4 + 3]);
	}
	for(i = 0; i < la_edge_buffer_length; i++)
		udg_create_edge(temp[la_edge_buffer[i * 2 + 0]], temp[la_edge_buffer[i * 2 + 1]]);
}

void la_t_copy_to_new_geometry()
{
	egreal pos[3] = {0.0, 0.0, 0.0};
	la_t_copy(pos);
	udg_create_new_modeling_node();
	la_new_node_paste = udg_get_modeling_node();
}

void la_t_paste_to_new_geometry()
{
	egreal pos[3] = {0.0, 0.0, 0.0};
	if(la_new_node_paste == -1)
		return;
	if(la_new_node_paste == udg_get_modeling_node())
		return;
	la_t_paste(pos);
	la_new_node_paste = -1;
}
