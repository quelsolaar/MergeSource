#define PERSUADE_INTERNAL
#include <stdlib.h>
#include "persuade2.h"
#include "p2_sds_table.h"

PTessTableElement	*global_tess_table_tri[TESSELATION_TABLE_MAX_LEVEL];
PTessTableElement	*global_tess_table_quad[TESSELATION_TABLE_MAX_LEVEL];
uint				global_tess_level_dynamic;

uint p_get_max_tess_level(void)
{
	return global_tess_level_dynamic; 
}

void p_sds_t_make_table_element(PTessTableElement *table, weight_polygon *read, uint polycount, PTessTableElement *base, TessPolygonType type)
{
	uint i, j, k, corner,	found, corner_compare;
	table->index = malloc((sizeof *table->index) * polycount * type);
	table->vertex_influence = malloc((sizeof *table->vertex_influence) * polycount * type * type);
	table->reference = malloc((sizeof *table->reference) * polycount * type);

	table->element_count = 0;
	table->vertex_count = 0;

	corner_compare = type;
	if(base != NULL)
		corner_compare = 3;

	for(j = 0 ; j < polycount; j++)
	{
		for(corner = 0 ; corner < corner_compare ; corner++)
		{
			found = -1;
			if(type == 3)
			{
				for(i = 0 ; i < table->vertex_count && found == -1 ; i++) /* finding vertexes allready used by the polygon*/
					if(table->vertex_influence[i * 3] == read[j].corner[corner][0] &&
					table->vertex_influence[(i * 3) + 1] == read[j].corner[corner][1] &&
					table->vertex_influence[(i * 3) + 2] == read[j].corner[corner][2])
					found = i;
			}
			else
			{
				for(i = 0 ; i < table->vertex_count && found == -1 ; i++) /* finding vertexes allready used by the polygon*/
					if(table->vertex_influence[i * 4] == read[j].corner[corner][0] &&
					table->vertex_influence[(i * 4) + 1] == read[j].corner[corner][1] &&
					table->vertex_influence[(i * 4) + 2] == read[j].corner[corner][2] &&
					table->vertex_influence[(i * 4) + 3] == read[j].corner[corner][3])
					found = i;
			}
			if(found != -1) 
			{
				table->index[table->element_count] = found;
			}
			else
			{
				if(base == NULL)
				{
					table->reference[table->vertex_count] = table->element_count;
				}
				else
				{
					if(type == 3)
						for(k = 0 ;read[j].corner[corner][0] 
									!= base->vertex_influence[k * 3] ||
								   read[j].corner[corner][1] 
								   != base->vertex_influence[(k * 3) + 1] ||
								   read[j].corner[corner][2]
								   != base->vertex_influence[(k * 3) + 2] ;k++)
							;
					else
						for(k = 0 ;read[j].corner[corner][0] 
									!= base->vertex_influence[k * 4] ||
								   read[j].corner[corner][1] 
								   != base->vertex_influence[(k * 4) + 1] ||
								   read[j].corner[corner][2] 
								   != base->vertex_influence[(k * 4) + 2] ||
								   read[j].corner[corner][3]
								   != base->vertex_influence[(k * 4) + 3] ;k++)
							;
					table->reference[table->vertex_count] = base->reference[k];
				}
				table->index[table->element_count] = table->vertex_count;
				table->vertex_influence[table->vertex_count * type] = read[j].corner[corner][0];
				table->vertex_influence[(table->vertex_count * type) + 1] = read[j].corner[corner][1];
				table->vertex_influence[(table->vertex_count * type) + 2] = read[j].corner[corner][2];
				if(type == 4)
					table->vertex_influence[(table->vertex_count * type) + 3] = read[j].corner[corner][3];
				table->vertex_count++;
			}
			table->element_count++;
		}
	}
	table->reference = realloc(table->reference, sizeof(uint) * table->element_count);
}

boolean persuade_init(uint tess_level)
{
	static uint i, j, k, polycount, base_level;
	static weight_polygon *write0, *write1, *write2, *temp;
	static PTessTableElement *table;
	static uint stage[3] = {0, 0, 0}, edge[4];

	global_tess_level_dynamic = tess_level;

	tess_level++;
	write0 = malloc(sizeof(weight_polygon) * p_sds_ts_pow_level(tess_level) * p_sds_ts_pow_level(tess_level) * 2);
	write1 = malloc(sizeof(weight_polygon) * p_sds_ts_pow_level(tess_level) * p_sds_ts_pow_level(tess_level) * 2);
	write2 = malloc(sizeof(weight_polygon) * p_sds_ts_pow_level(tess_level) * p_sds_ts_pow_level(tess_level) * 2);
	table = malloc((sizeof *table) * (tess_level + 1));

	polycount = 1;
	p_sds_ts_init_weight_polygon(TESS_POLYGON_TRI, write1);
	p_sds_t_make_table_element(table, write1, polycount, NULL, TESS_POLYGON_TRI);
	p_sds_ts_tesselate_weight_polygon(write1, write0, 0);

	for(i = 1 ; i < tess_level; i++)
	{
		polycount *= 4;
		p_sds_t_make_table_element(&table[i], write0 , polycount, NULL, TESS_POLYGON_TRI);

		p_sds_ts_tesselate_weight_polygon(write0, write1, i);
		temp = write1;
		write1 = write0;
		write0 = temp;
	}
	tess_level--;

	for(base_level = 1 ; base_level <= tess_level ; base_level++)
		global_tess_table_tri[base_level] = malloc(sizeof(PTessTableElement) * (base_level + 1) * (base_level + 1) * (base_level + 1));
	stage[0] = 1;
	stage[1] = 1;
	stage[2] = 0;

	for(; stage[1] <= tess_level ; stage[1]++)
	{
		for(; stage[2] < (stage[1] + 1) * (stage[1] + 1) * (stage[1] + 1); stage[2]++)
		{
			stage[1]++; 
			edge[0] = stage[2] % stage[1];
			j = stage[2] / stage[1];
			edge[1] = j % stage[1];
			j = j / stage[1];
			edge[2] = j % stage[1];
			stage[1]--;
			j = edge[0];
			if(j > edge[1])
				j = edge[1];
			if(j > edge[2])
				j = edge[2];

			polycount = 1;
			if(j > 0)
			{
				p_sds_ts_init_weight_polygon(TESS_POLYGON_TRI, write1);
				p_sds_ts_tesselate_weight_polygon(write1, write0, 0);
				for(k = 1 ; k < j; k++)
				{
					polycount *= 4;
					p_sds_ts_tesselate_weight_polygon(write0, write1, k);
					temp = write1;
					write1 = write0;
					write0 = temp;
				}
				polycount = p_sds_ts_calculate_tri_polyon_count(edge[0], edge[1], edge[2]);
				temp = p_sds_ts_split_tri_edges(write0, write1,	 p_sds_ts_calculate_tri_polyon_count(j, j, j) , polycount, edge);
			}
			else
			{
				p_sds_ts_init_weight_polygon(TESS_POLYGON_TRI, write0);
				polycount = p_sds_ts_calculate_tri_polyon_count(edge[0], edge[1], edge[2]);
				temp = p_sds_ts_split_tri_edges(write0, write1, 1, polycount, edge);
			}

			p_sds_t_make_table_element(&global_tess_table_tri[stage[1]][ edge[0] + (edge[1] * (stage[1] + 1))  + (edge[2] * (stage[1] + 1) * (stage[1] + 1) )], temp , polycount,  &table[stage[1]], TESS_POLYGON_TRI);
			p_geo_table_sort_edges(&global_tess_table_tri[stage[1]][ edge[0] + (edge[1] * (stage[1] + 1))  + (edge[2] * (stage[1] + 1) * (stage[1] + 1) )], edge, 3);
			p_geo_table_gen_normals(&global_tess_table_tri[stage[1]][ edge[0] + (edge[1] * (stage[1] + 1))  + (edge[2] * (stage[1] + 1) * (stage[1] + 1) )], 3);

		}
		stage[2] = 0;
	}
	stage[0] = 2;


	tess_level++;
	polycount = 1;
	p_sds_ts_init_weight_polygon(TESS_POLYGON_QUAD, write1);
	p_sds_t_make_table_element(table, write1, polycount, NULL, TESS_POLYGON_QUAD);
	p_sds_ts_tesselate_weight_polygon(write1, write0, 0);

	for(i = 1 ; i < tess_level ; i++ )
	{
		polycount *= 4;

		p_sds_t_make_table_element(&table[i], write0 , polycount, NULL, TESS_POLYGON_QUAD);
		p_sds_ts_tesselate_weight_polygon(write0, write1, i);
		temp = write1;
		write1 = write0;
		write0 = temp;
	}
	tess_level--;
	for(base_level = 1 ; base_level <= tess_level ; base_level++)
		global_tess_table_quad[base_level] = malloc(sizeof(PTessTableElement) * (base_level + 1) * (base_level + 1) * (base_level + 1) * (base_level + 1));
	stage[0] = 3;
	stage[1] = 1;
	stage[2] = 0;

	for(; stage[1] <= tess_level ; stage[1]++)
	{
		for(; stage[2] < (stage[1] + 1) * (stage[1] + 1) * (stage[1] + 1) * (stage[1] + 1); stage[2]++)
		{
			stage[1]++;
			edge[0] = stage[2] % stage[1];
			j = stage[2] / stage[1];
			edge[1] = j % stage[1];
			j = j / stage[1];
			edge[2] = j % stage[1];
			j = j / stage[1];
			edge[3] = j % stage[1];
			stage[1]--;
			polycount = p_sds_ts_init_weight_quad(temp, edge[0], edge[1], edge[2], edge[3]);
			p_sds_t_make_table_element(&global_tess_table_quad[stage[1]][edge[0] + (edge[1] * (stage[1] + 1))  + (edge[2] * (stage[1] + 1) * (stage[1] + 1) ) + (edge[3] * (stage[1] + 1) * (stage[1] + 1) * (stage[1] + 1))], temp , polycount,  &table[stage[1]], TESS_POLYGON_QUAD);
			p_geo_table_sort_edges(&global_tess_table_quad[stage[1]][edge[0] + (edge[1] * (stage[1] + 1))  + (edge[2] * (stage[1] + 1) * (stage[1] + 1) ) + (edge[3] * (stage[1] + 1) * (stage[1] + 1) * (stage[1] + 1))], edge, 4);
			p_geo_table_gen_normals(&global_tess_table_quad[stage[1]][edge[0] + (edge[1] * (stage[1] + 1))  + (edge[2] * (stage[1] + 1) * (stage[1] + 1) ) + (edge[3] * (stage[1] + 1) * (stage[1] + 1) * (stage[1] + 1))], 4);
		}
		stage[2] = 0;
	}
	stage[0] = 4;

	free(table);
	free(write0);
	free(write1);
	free(write2);
	return TRUE;
}


PTessTableElement *get_dynamic_table_tri(uint base_level, uint *edge)
{
	base_level++;
	return &global_tess_table_tri[base_level - 1][edge[0] + (edge[1] * base_level) + (edge[2] * base_level * base_level)];
}


PTessTableElement *get_dynamic_table_quad(uint base_level, uint *edge)
{
	base_level++;
	return &global_tess_table_quad[base_level - 1][edge[0] + (edge[1] * base_level) + (edge[2] * base_level * base_level) + (edge[3] * base_level * base_level * base_level)];
}

uint get_dynamic_table_tri_level(uint base_level, PTessTableElement *table, uint edge)
{
	uint i;
	i = table - global_tess_table_tri[base_level];
	base_level++;
	if(edge == 0)
		return i % base_level;
	if(edge == 1)
		return (i / base_level) % base_level;
	if(edge == 2)
		return (i / (base_level * base_level)) % base_level;
	return 0;
}


uint get_dynamic_table_quad_level(uint base_level, PTessTableElement *table, uint edge)
{
	uint i;
	i = table - global_tess_table_quad[base_level];
	base_level++;
	if(edge == 0)
		return i % base_level;
	if(edge == 1)
		return (i / base_level) % base_level;
	if(edge == 2)
		return (i / (base_level * base_level)) % base_level;
	if(edge == 3)
		return (i / (base_level * base_level * base_level)) % base_level;
	return 0;
}

