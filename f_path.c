#include <stdlib.h>
#include "forge.h"

typedef struct{
	uint parent;
	uint node;
	float cost;
}FPathCell;

typedef struct{
	uint id;
	float dist;
	float cost;
}FPathNode;

uint *f_path_find(uint *output_count, uint cell_count, uint naighbour_max_count, uint start, uint goal, uint (* naighbout_func)(uint start, uint goal, uint *list, float *cost, float *dist, void *user), void *user, uint max_cells)
{
	FPathCell *cells;
	FPathNode *open_list, current, c;
	uint i, j, k, length = 1, list_count, list_alloc, *list, block, *output = NULL;
	float *cost, *dist, f;
	
	list = malloc((sizeof *list) * naighbour_max_count);
	cost = malloc((sizeof *cost) * naighbour_max_count);
	dist = malloc((sizeof *dist) * naighbour_max_count);

	cells = malloc((sizeof *cells) * cell_count);
	for(i = 0; i < cell_count; i++)
		cells[i].parent = -1;

	list_count = 1;
	list_alloc = 4096;
	open_list = malloc((sizeof * open_list) * list_alloc);
	
	open_list[0].id = start;
	open_list[0].dist = 0;
	open_list[0].cost = 0;

	while(list_count != 0 && open_list[list_count - 1].id != goal)
	{
		current = open_list[--list_count];
		cells[current.id].node = -2;
		length = naighbout_func(current.id, goal, list, cost, dist, user);
		if(list_count + length > list_alloc)
		{
			list_alloc *= 2;
			open_list = realloc(open_list, (sizeof * open_list) * list_alloc);
		} 
		for(i = 0; i < length; i++)
		{
			block = list[i];
			if(cells[block].parent == -1)
			{
				if(--max_cells == 0)
				{
					free(list);
					free(cost);
					free(dist);
					free(cells);
					free(open_list);
					return output;
				}
				cells[block].parent = current.id;
				f = cells[block].cost = current.cost + cost[i];
				f += dist[i];
				for(j = list_count; j != 0 && open_list[j - 1].cost + open_list[j - 1].dist < f; j--);
				for(k = list_count++; k != j; k--)
				{
					open_list[k] = open_list[k - 1];
					cells[open_list[k].id].node = k;
				}
				open_list[j].id = block;
				open_list[j].dist = dist[i];
				open_list[j].cost = cost[i] + current.cost;
				
				cells[block].node = j;
				cells[block].parent = current.id;
				cells[block].cost = open_list[j].cost;
			}else
			{
				f = cost[i] + current.cost;
				if(cells[block].cost > f)
				{
					cells[block].cost = f;
					cells[block].parent = current.id;
					if(cells[block].node < list_count)
					{
						c = open_list[cells[block].node];
						c.cost = f;
						f = c.cost + c.dist;						
						for(j = cells[block].node + 1; j < list_count && open_list[j].cost + open_list[j].dist > f; j++)
						{
							k = j - 1;
							open_list[k] = open_list[j];
							cells[open_list[k].id].node = k;
						}
						open_list[j - 1] = c;
						cells[c.id].node = j - 1;
					}
				}
			}
			if(block == goal)
				break;
			if(list_count > 1)
			{			
				for(j = 1; j < list_count; j++)
				{
					if(open_list[j].cost + open_list[j].dist > open_list[j - 1].cost + open_list[j - 1].dist)
						k = 0;
				}
			}
		}
	}
/*	{
		float a[3], b[3];

		me_geo_block_pos_no_obj(start, TRUE, a, 0);
		me_geo_block_pos_no_obj(goal, TRUE, b, 0);
		r_primitive_line_3d(a[0], a[1], a[2], b[0], b[1], b[2], 1, 1, 0.1, 0.1);
		for(i = 0; i < cell_count; i++)
		{
			if(cells[i].parent != -1)
			{
				me_geo_block_pos_no_obj(i % (ME_BLOCK_COUNT * ME_BLOCK_COUNT * 6), i < (ME_BLOCK_COUNT * ME_BLOCK_COUNT * 6), a, 0);
				me_geo_block_pos_no_obj(cells[i].parent % (ME_BLOCK_COUNT * ME_BLOCK_COUNT * 6), cells[i].parent < (ME_BLOCK_COUNT * ME_BLOCK_COUNT * 6), b, 0);
				r_primitive_line_3d(a[0], a[1], a[2], b[0], b[1], b[2], 0.3, 0.1, 0.1, 0.1);
			}
		}
		if(list_count != 0)
		{
			for(i = goal; i != start; i = cells[i].parent)
			{
				me_geo_block_pos_no_obj(i % (ME_BLOCK_COUNT * ME_BLOCK_COUNT * 6), i < (ME_BLOCK_COUNT * ME_BLOCK_COUNT * 6), a, 0);
				me_geo_block_pos_no_obj(cells[i].parent % (ME_BLOCK_COUNT * ME_BLOCK_COUNT * 6), cells[i].parent < (ME_BLOCK_COUNT * ME_BLOCK_COUNT * 6), b, 0);
				r_primitive_line_3d(a[0], a[1], a[2], b[0], b[1], b[2], 1, 0.1, 1, 0.1);
			}
		}
		r_primitive_line_flush();
	}*/
	if(list_count != 0)
	{
		j = 1;
		for(i = goal; i != start; i = cells[i].parent)
			j++;
		output = malloc((sizeof *output) * j);
		j = 0;
		for(i = goal; i != start; i = cells[i].parent)
			output[j++] = i;
		output[j++] = i;
		*output_count = j;
	}else
	{
		output = NULL;
		output_count = 0;
	}
	free(list);
	free(cost);
	free(dist);
	free(cells);
	free(open_list);
	return output;
}