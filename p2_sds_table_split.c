#define PERSUADE_INTERNAL
#include "persuade2.h"
#include "p2_sds_table.h"

void p_sds_ts_init_weight_polygon(TessPolygonType type, weight_polygon *polygon)
{
	uint i, j;
	polygon->type = type;
	for(i = 0 ; i < 4 ; i++)
	{
		for(j = 0 ; j < 4 ; j++)
		{
			if(i == j)
				polygon->corner[i][j] = 1.0;
			else
				polygon->corner[i][j] = 0.0;
		}
		polygon->edge[i] = i;
		polygon->level_of_edge[i] = 0;
	}
}


void p_sds_ts_copy_vertex(weight_polygon *read, pgreal *target, uint vertex1)
{
	target[0] = read->corner[vertex1][0];
	target[1] = read->corner[vertex1][1];
	target[2] = read->corner[vertex1][2];
	target[3] = read->corner[vertex1][3];
}

void p_sds_ts_divide_edge(weight_polygon *read, pgreal *target, uint vertex1, uint vertex2)
{
	target[0] = (read->corner[vertex1][0] + read->corner[vertex2][0]) / 2;
	target[1] = (read->corner[vertex1][1] + read->corner[vertex2][1]) / 2;
	target[2] = (read->corner[vertex1][2] + read->corner[vertex2][2]) / 2;
	target[3] = (read->corner[vertex1][3] + read->corner[vertex2][3]) / 2;
}

void p_sds_ts_create_middel_vertex(weight_polygon *read, pgreal *target)
{
	target[0] = (pgreal)(read->corner[0][0] + read->corner[1][0] + read->corner[2][0] + read->corner[3][0]) / 4.0;
	target[1] = (pgreal)(read->corner[0][1] + read->corner[1][1] + read->corner[2][1] + read->corner[3][1]) / 4.0;
	target[2] = (pgreal)(read->corner[0][2] + read->corner[1][2] + read->corner[2][2] + read->corner[3][2]) / 4.0;
	target[3] = (pgreal)(read->corner[0][3] + read->corner[1][3] + read->corner[2][3] + read->corner[3][3]) / 4.0;
}

void p_sds_ts_tesselate_weight_polygon(weight_polygon *read, weight_polygon *write, uint level)
{
	uint i, j, k, read_count;
	read_count = 1;
	for(i = 0 ; i < level; i++)
		read_count *= 4;

	for(k = 0 ; k < read_count; k++)
	{
		if(read->type == TESS_POLYGON_TRI)
		{
			write[(k * 4) + 0].edge[0] = read[k].edge[0];	     
			write[(k * 4) + 1].edge[0] = read[k].edge[0];	     
	        
			write[(k * 4) + 1].edge[1] = read[k].edge[1];	     
			write[(k * 4) + 2].edge[1] = read[k].edge[1];

			write[(k * 4) + 2].edge[2] = read[k].edge[2];	     
			write[(k * 4) + 0].edge[2] = read[k].edge[2];

			write[(k * 4) + 0].edge[1] = 5;
			write[(k * 4) + 1].edge[2] = 5;
			write[(k * 4) + 2].edge[0] = 5;
			write[(k * 4) + 3].edge[0] = 5;
			write[(k * 4) + 3].edge[1] = 5;
			write[(k * 4) + 3].edge[2] = 5;

			p_sds_ts_copy_vertex(&read[k], write[(k * 4) + 0].corner[0], 0); /* first polygon */
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 0].corner[1], 0, 1);
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 0].corner[2], 2, 0);	  
			write[k * 4].type = TESS_POLYGON_TRI;
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 1].corner[0], 0, 1);	/* second polygon */
			p_sds_ts_copy_vertex(&read[k], write[(k * 4) + 1].corner[1], 1);
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 1].corner[2], 1, 2);
			write[(k * 4) + 1].type = TESS_POLYGON_TRI;
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 2].corner[0], 2, 0);/* third polygon */
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 2].corner[1], 1, 2);
			p_sds_ts_copy_vertex(&read[k], write[(k * 4) + 2].corner[2], 2); 
			write[(k * 4) + 2].type = TESS_POLYGON_TRI;
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 3].corner[0], 0, 1);	 /* fourth polygon */
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 3].corner[1], 1, 2);
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 3].corner[2], 2, 0);
			write[(k * 4) + 3].type = TESS_POLYGON_TRI;
		}
		else
		{
			p_sds_ts_copy_vertex(&read[k], write[k * 4].corner[0], 0); /* first polygon */
			p_sds_ts_divide_edge(&read[k], write[k * 4].corner[1], 0, 1);
			p_sds_ts_create_middel_vertex(&read[k], write[k * 4].corner[2]);	       
			p_sds_ts_divide_edge(&read[k], write[k * 4].corner[3], 3, 0);	    
			write[k * 4].type = TESS_POLYGON_QUAD;
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 1].corner[0], 0, 1); /* second polygon */
			p_sds_ts_copy_vertex(&read[k], write[(k * 4) + 1].corner[1], 1);
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 1].corner[2], 1, 2);
			p_sds_ts_create_middel_vertex(&read[k], write[(k * 4) + 1].corner[3]);
			write[(k * 4) + 1].type = TESS_POLYGON_QUAD;
			p_sds_ts_create_middel_vertex(&read[k], write[(k * 4) + 2].corner[0]); /* third polygon */
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 2].corner[1], 1, 2);
			p_sds_ts_copy_vertex(&read[k], write[(k * 4) + 2].corner[2], 2);	       
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 2].corner[3], 2, 3);
			write[(k * 4) + 2].type = TESS_POLYGON_QUAD;
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 3].corner[0], 0, 3);	  /* fourth polygon */
			p_sds_ts_create_middel_vertex(&read[k], write[(k * 4) + 3].corner[1]);
			p_sds_ts_divide_edge(&read[k], write[(k * 4) + 3].corner[2], 2, 3);	  
			p_sds_ts_copy_vertex(&read[k], write[(k * 4) + 3].corner[3], 3);
			write[(k * 4) + 3].type = TESS_POLYGON_QUAD;
		}
		for(i = 0 ; i < 4 ; i++)
		{
			for(j = 0 ; j < 4 ; j++)
			{
				write[(k * 4) + i].level_of_edge[j] = level + 1;
			}
		}
	}
}

uint p_sds_ts_corner_split(weight_polygon *polygon, weight_square *square, uint tess0, uint tess1)
{
	uint i, poly_count = 0;
	pgreal h, v;
	for(i = 0; i < tess0; i++)
	{
		polygon[poly_count].corner[0][0] = (1 - square->vertical3) * (1 - square->horizontal3);
		polygon[poly_count].corner[0][1] = (1 - square->vertical3) * (square->horizontal3);
		polygon[poly_count].corner[0][2] = (square->vertical3) * (square->horizontal3);
		polygon[poly_count].corner[0][3] = (square->vertical3) * (1 - square->horizontal3);

		h = ((tess0 - i) * square->horizontal0 + i * square->horizontal1) / tess0;
		v = ((tess0 - i) * square->vertical0 + i * square->vertical1) / tess0;
	        
		polygon[poly_count].corner[1][0] = (1 - v) * (1 - h);
		polygon[poly_count].corner[1][1] = (1 - v) * h;
		polygon[poly_count].corner[1][2] = v * h;
		polygon[poly_count].corner[1][3] = v * (1 - h);

		i++;
		h = ((tess0 - i) * square->horizontal0 + i * square->horizontal1) / tess0;
		v = ((tess0 - i) * square->vertical0 + i * square->vertical1) / tess0;
		i--;

		polygon[poly_count].corner[2][0] = (1 - v) * (1 - h);
		polygon[poly_count].corner[2][1] = (1 - v) * h;
		polygon[poly_count].corner[2][2] = v * h;
		polygon[poly_count].corner[2][3] = v * (1 - h);		       
		poly_count++;
	}
	for(i = 0; i < tess1; i++)
	{
		polygon[poly_count].corner[0][0] = (1 - square->vertical3) * (1 - square->horizontal3);
		polygon[poly_count].corner[0][1] = (1 - square->vertical3) * (square->horizontal3);
		polygon[poly_count].corner[0][2] = (square->vertical3) * (square->horizontal3);
		polygon[poly_count].corner[0][3] = (square->vertical3) * (1 - square->horizontal3);

		h = ((tess1 - i) * square->horizontal1 + i * square->horizontal2) / tess1;
		v = ((tess1 - i) * square->vertical1 + i * square->vertical2) / tess1;
	        
		polygon[poly_count].corner[1][0] = (1 - v) * (1 - h);
		polygon[poly_count].corner[1][1] = (1 - v) * h;
		polygon[poly_count].corner[1][2] = v * h;
		polygon[poly_count].corner[1][3] = v * (1 - h);

		i++;
		h = ((tess1 - i) * square->horizontal1 + i * square->horizontal2) / tess1;
		v = ((tess1 - i) * square->vertical1 + i * square->vertical2) / tess1;
		i--;

		polygon[poly_count].corner[2][0] = (1 - v) * (1 - h);
		polygon[poly_count].corner[2][1] = (1 - v) * h;
		polygon[poly_count].corner[2][2] = v * h;
		polygon[poly_count].corner[2][3] = v * (1 - h);		       
		poly_count++;
	}
	return poly_count;
}

uint p_sds_ts_middle_split(weight_polygon *polygon, weight_square *square, uint h_tess, uint v_tess)
{
	uint i, j, poly_count = 0;
	pgreal v, h, v1, h1;

	h = (square->horizontal1 - square->horizontal0) / (pgreal)h_tess;
	v = (square->vertical3 - square->vertical0) / (pgreal)v_tess;

	for(i = 0; i < h_tess; i++) /* filling the midle of the quad whit polygons*/
	{
		for(j = 0; j < v_tess; j++)
		{
			h1 = square->horizontal0 + i * h;
			v1 = square->vertical0 + j * v;
        
			polygon[poly_count].corner[0][0] = (1 - v1) * (1 - h1);
			polygon[poly_count].corner[0][1] = (1 - v1) * (h1);
			polygon[poly_count].corner[0][2] = (v1) * (h1);
			polygon[poly_count].corner[0][3] = (v1) * (1 - h1);

			h1 = square->horizontal0 + i * h + h;
			v1 = square->vertical0 + j * v;

			polygon[poly_count].corner[1][0] = (1 - v1) * (1 - h1);
			polygon[poly_count].corner[1][1] = (1 - v1) * (h1);
			polygon[poly_count].corner[1][2] = (v1) * (h1);
			polygon[poly_count].corner[1][3] = (v1) * (1 - h1);

			h1 = square->horizontal0 + i * h;
			v1 = square->vertical0 + j * v + v;

			polygon[poly_count].corner[2][0] = (1 - v1) * (1 - h1);
			polygon[poly_count].corner[2][1] = (1 - v1) * (h1);
			polygon[poly_count].corner[2][2] = (v1) * (h1);
			polygon[poly_count].corner[2][3] = (v1) * (1 - h1);

			h1 = square->horizontal0 + i * h;
			v1 = square->vertical0 + j * v + v;

			poly_count++;

			polygon[poly_count].corner[0][0] = (1 - v1) * (1 - h1);
			polygon[poly_count].corner[0][1] = (1 - v1) * (h1);
			polygon[poly_count].corner[0][2] = (v1) * (h1);
			polygon[poly_count].corner[0][3] = (v1) * (1 - h1);

			h1 = square->horizontal0 + i * h + h;
			v1 = square->vertical0 + j * v;

			polygon[poly_count].corner[1][0] = (1 - v1) * (1 - h1);
			polygon[poly_count].corner[1][1] = (1 - v1) * (h1);
			polygon[poly_count].corner[1][2] = (v1) * (h1);
			polygon[poly_count].corner[1][3] = (v1) * (1 - h1);

			h1 = square->horizontal0 + i * h + h;
			v1 = square->vertical0 + j * v + v;

			polygon[poly_count].corner[2][0] = (1 - v1) * (1 - h1);
			polygon[poly_count].corner[2][1] = (1 - v1) * (h1);
			polygon[poly_count].corner[2][2] = (v1) * (h1);
			polygon[poly_count].corner[2][3] = (v1) * (1 - h1);

			poly_count++;
		}
	}
	return poly_count;
}
uint p_sds_ts_edge_split(weight_polygon *polygon, weight_square *square, uint tess_0, uint tess_1)
{
	uint i, j, poly_count = 0;
	pgreal v0, v1, h0, h1;

	v0 = square->vertical0;
	v1 = square->vertical3;
	h0 = square->horizontal0;
	h1 = square->horizontal3;

	j = tess_0 / (tess_1 * 2);

	for(i = 0; ; i++)
	{
		for(; j < tess_0 / tess_1; j++)
		{
			polygon[poly_count].corner[0][0] = (1 - v1) * (1 - h1);
			polygon[poly_count].corner[0][1] = (1 - v1) * (h1);
			polygon[poly_count].corner[0][2] = (v1) * (h1);
			polygon[poly_count].corner[0][3] = (v1) * (1 - h1);

			polygon[poly_count].corner[1][0] = (1 - v0) * (1 - h0);
			polygon[poly_count].corner[1][1] = (1 - v0) * (h0);
			polygon[poly_count].corner[1][2] = (v0) * (h0);
			polygon[poly_count].corner[1][3] = (v0) * (1 - h0);

			v0 += (square->vertical1 - square->vertical0) / tess_0;
			h0 += (square->horizontal1 - square->horizontal0) / tess_0;

			polygon[poly_count].corner[2][0] = (1 - v0) * (1 - h0);
			polygon[poly_count].corner[2][1] = (1 - v0) * (h0);
			polygon[poly_count].corner[2][2] = (v0) * (h0);
			polygon[poly_count].corner[2][3] = (v0) * (1 - h0);

			poly_count++;

			if(poly_count == tess_1 + tess_0)
				return poly_count;
		}

		polygon[poly_count].corner[0][0] = (1 - v1) * (1 - h1);
		polygon[poly_count].corner[0][1] = (1 - v1) * (h1);
		polygon[poly_count].corner[0][2] = (v1) * (h1);
		polygon[poly_count].corner[0][3] = (v1) * (1 - h1);

		polygon[poly_count].corner[1][0] = (1 - v0) * (1 - h0);
		polygon[poly_count].corner[1][1] = (1 - v0) * (h0);
		polygon[poly_count].corner[1][2] = (v0) * (h0);
		polygon[poly_count].corner[1][3] = (v0) * (1 - h0);

		v1 += (square->vertical2 - square->vertical3) / tess_1;
		h1 += (square->horizontal2 - square->horizontal3) / tess_1;

		polygon[poly_count].corner[2][0] = (1 - v1) * (1 - h1);
		polygon[poly_count].corner[2][1] = (1 - v1) * (h1);
		polygon[poly_count].corner[2][2] = (v1) * (h1);
		polygon[poly_count].corner[2][3] = (v1) * (1 - h1);

		poly_count++;
			if(poly_count == tess_1 + tess_0)
			{
				return poly_count;
			}
		j = 0;
	}
	return poly_count;
}

uint p_sds_ts_pow_level(uint level)
{
	uint pow, i;
	pow = 1;
	for(i = 0; i < level; i++)
		pow *= 2;
	return pow;
}

uint p_sds_ts_lowest_level(uint level0, uint level1)
{
	if(level0 > level1)
		return level1;
	else
		return level0;
}

uint p_sds_ts_init_weight_quad(weight_polygon *polygon, uint level3, uint level2, uint level1, uint level0)
{
	int poly_count, h_level = 0, v_level = 0;
	weight_square square, square2;

	poly_count = 0;

	level0 = p_sds_ts_pow_level(level0);
	level1 = p_sds_ts_pow_level(level1);
	level2 = p_sds_ts_pow_level(level2);
	level3 = p_sds_ts_pow_level(level3);
        

	square.horizontal0 = 0;
	square.horizontal1 = 1;
	square.horizontal2 = 1;
	square.horizontal3 = 0;
	square.vertical0 = 0;
	square.vertical1 = 0;
	square.vertical2 = 1;
	square.vertical3 = 1;

/*	poly_count += middle_split(polygon, &square, lowest_level(level0, level2), lowest_level(level1, level3));
		return poly_count;
*/
	if(level3 > level1)
	{
		square.vertical0 = 1.0 / (pgreal)p_sds_ts_lowest_level(level2, level0);
		square.vertical1 = square.vertical0;
		h_level = level1;
		v_level -= 1;
	}
	else
	{
		h_level = level3;
		if(level3 != level1)
		{
			square.vertical2 = ((pgreal)p_sds_ts_lowest_level(level2, level0) - 1.0) / (pgreal)p_sds_ts_lowest_level(level2, level0);
			square.vertical3 = square.vertical2;
			v_level -= 1;
		}
	}

	if(level0 > level2)
	{
		square.horizontal0 = 1.0 / (pgreal)p_sds_ts_lowest_level(level3, level1);
		square.horizontal3 = square.horizontal0;
		v_level += level2;
		h_level -= 1;
	}
	else
	{
		v_level += level0;
		if(level0 != level2)
		{	 
			square.horizontal1 = ((pgreal)p_sds_ts_lowest_level(level3, level1) - 1.0) / (pgreal)p_sds_ts_lowest_level(level3, level1);
			square.horizontal2 = square.horizontal1;
			h_level -= 1;
		}
	}	 
	if(h_level > 0 && v_level > 0)
		poly_count += p_sds_ts_middle_split(polygon, &square, h_level, v_level);

	if(h_level != 0)
	{
		if(level3 > level1)
		{

			square2 = square;
			square2.vertical2 = square.vertical0;
			square2.vertical3 = square.vertical1;
			square2.vertical0 = 0;
			square2.vertical1 = 0;
	
			poly_count += p_sds_ts_edge_split(&polygon[poly_count], &square2, h_level * (level3 / level1), h_level);
		}
		else
		{
			if(level3 != level1)
			{
				square2 = square;
				square2.horizontal0 = square2.horizontal1;
				square2.horizontal1 = square2.horizontal3;
				square2.horizontal2 = square2.horizontal3;
				square2.horizontal3 = square2.horizontal0;
				square2.vertical0 = 1;
				square2.vertical1 = 1;

				poly_count += p_sds_ts_edge_split(&polygon[poly_count], &square2, h_level * (level1 / level3), h_level);
			}
		}
	}
	if(v_level != 0)
	{
		if(level2 > level0)
		{
			square2 = square;
			square2.horizontal0 = 1;
			square2.horizontal1 = 1;
			square2.horizontal3 = square.horizontal1;
			square2.vertical1 = square.vertical2;
			square2.vertical3 = square.vertical0;	

			poly_count += p_sds_ts_edge_split(&polygon[poly_count], &square2, v_level * (level2 / level0), v_level);
		}
		else
		{
			if(level2 != level0)
			{
				square2 = square;
				square2.horizontal0 = 0;
				square2.horizontal1 = 0;
				square2.horizontal2 = square.horizontal0;
				square2.vertical0 = square.vertical3;
				square2.vertical1 = square.vertical0;
				square2.vertical2 = square.vertical0;	     

				poly_count += p_sds_ts_edge_split(&polygon[poly_count], &square2, v_level * (level0 / level2), v_level);
			}
		}
	}

	if(level3 == level1 && level0 == level2)
		return poly_count;

//	if((v_level % 2 == 1 && h_level % 2 == 1) || (v_level == 0 && h_level == 0))
	{
		if(square.horizontal0 == 0)
		{
			if(square.vertical0 == 0)
			{
				square2.horizontal0 = 1;
				square2.horizontal1 = 1;
				square2.horizontal2 = square.horizontal1;
				square2.horizontal3 = square.horizontal1;
				square2.vertical0 = square.vertical2;
				square2.vertical1 = 1;
				square2.vertical2 = 1;
				square2.vertical3 = square.vertical2;
				poly_count += p_sds_ts_corner_split(&polygon[poly_count], &square2, level2 / level0, level1 / level3);
			}
			else
			{
				square2.horizontal0 = square.horizontal1;
				square2.horizontal1 = 1;
				square2.horizontal2 = 1;
				square2.horizontal3 = square.horizontal1;
				square2.vertical0 = 0;
				square2.vertical1 = 0;
				square2.vertical2 = square.vertical1;
				square2.vertical3 = square.vertical1;
				poly_count += p_sds_ts_corner_split(&polygon[poly_count], &square2, level3 / level1, level2 / level0);
			}
		}
		else
		{
			if(square.vertical0 == 0)
			{
				square2.horizontal0 = square.horizontal0;
				square2.horizontal1 = 0;
				square2.horizontal2 = 0;
				square2.horizontal3 = square.horizontal0;
				square2.vertical0 = 1;
				square2.vertical1 = 1;
				square2.vertical2 = square.vertical2;
				square2.vertical3 = square.vertical2;
				poly_count += p_sds_ts_corner_split(&polygon[poly_count], &square2, level1 / level3, level0 / level2);
			}
			else
			{
				square2.horizontal0 = 0;
				square2.horizontal1 = 0;
				square2.horizontal2 = square.horizontal0;
				square2.horizontal3 = square.horizontal0;
				square2.vertical0 = square.vertical0;
				square2.vertical1 = 0;
				square2.vertical2 = 0;
				square2.vertical3 = square.vertical0;
				poly_count += p_sds_ts_corner_split(&polygon[poly_count], &square2, level0 / level2, level3 / level1);
			}
		}
	}
	return poly_count;
}

/* backup before dangerous switch

uint p_sds_ts_init_weight_quad(weight_polygon *polygon, uint level3, uint level2, uint level1, uint level0)
{
	int poly_count, h_level = 0, v_level = 0;
	weight_square square, square2;

	poly_count = 0;

	level0 = p_sds_ts_pow_level(level0);
	level1 = p_sds_ts_pow_level(level1);
	level2 = p_sds_ts_pow_level(level2);
	level3 = p_sds_ts_pow_level(level3);
        

	square.horizontal0 = 0;
	square.horizontal1 = 1;
	square.horizontal2 = 1;
	square.horizontal3 = 0;
	square.vertical0 = 0;
	square.vertical1 = 0;
	square.vertical2 = 1;
	square.vertical3 = 1;

//	poly_count += middle_split(polygon, &square, lowest_level(level0, level2), lowest_level(level1, level3));
//		return poly_count;

	if(level0 > level2)
	{
		square.vertical0 = 1.0 / (pgreal)p_sds_ts_lowest_level(level1, level3);
		square.vertical1 = 1.0 / (pgreal)p_sds_ts_lowest_level(level1, level3);
		h_level = level2;
		v_level -= 1;
	}
	else
	{
		h_level = level0;
		if(level0 != level2)
		{
			square.vertical2 = ((pgreal)p_sds_ts_lowest_level(level1, level3) - 1.0) / (pgreal)p_sds_ts_lowest_level(level1, level3);
			square.vertical3 = ((pgreal)p_sds_ts_lowest_level(level1, level3) - 1.0) / (pgreal)p_sds_ts_lowest_level(level1, level3);
			v_level -= 1;
		}
	}

	if(level3 > level1)
	{
		square.horizontal0 = 1.0 / (pgreal)p_sds_ts_lowest_level(level0, level2);
		square.horizontal3 = 1.0 / (pgreal)p_sds_ts_lowest_level(level0, level2);
		v_level += level1;
		h_level -= 1;
	}
	else
	{
		v_level += level3;
		if(level3 != level1)
		{	 
			square.horizontal1 = ((pgreal)p_sds_ts_lowest_level(level0, level2) - 1.0) / (pgreal)p_sds_ts_lowest_level(level0, level2);
			square.horizontal2 = ((pgreal)p_sds_ts_lowest_level(level0, level2) - 1.0) / (pgreal)p_sds_ts_lowest_level(level0, level2);
			h_level -= 1;
		}
	}	 
	if(h_level > 0 && v_level > 0)
		poly_count += p_sds_ts_middle_split(polygon, &square, h_level, v_level);

	if(h_level != 0)
	{
		if(level0 > level2)
		{

			square2 = square;
			square2.vertical2 = square.vertical0;
			square2.vertical3 = square.vertical1;
			square2.vertical0 = 0;
			square2.vertical1 = 0;
	
			poly_count += p_sds_ts_edge_split(&polygon[poly_count], &square2, h_level * (level0 / level2), h_level);
		}
		else
		{
			if(level0 != level2)
			{
				square2 = square;
				square2.horizontal0 = square2.horizontal1;
				square2.horizontal1 = square2.horizontal3;
				square2.horizontal2 = square2.horizontal3;
				square2.horizontal3 = square2.horizontal0;
				square2.vertical0 = 1;
				square2.vertical1 = 1;

				poly_count += p_sds_ts_edge_split(&polygon[poly_count], &square2, h_level * (level2 / level0), h_level);
			}
		}
	}
	if(v_level != 0)
	{
		if(level1 > level3)
		{
			square2 = square;
			square2.horizontal0 = 1;
			square2.horizontal1 = 1;
			square2.horizontal3 = square.horizontal1;
			square2.vertical1 = square.vertical2;
			square2.vertical3 = square.vertical0;	

			poly_count += p_sds_ts_edge_split(&polygon[poly_count], &square2, v_level * (level1 / level3), v_level);
		}
		else
		{
			if(level1 != level3)
			{
				square2 = square;
				square2.horizontal0 = 0;
				square2.horizontal1 = 0;
				square2.horizontal2 = square.horizontal0;
				square2.vertical0 = square.vertical3;
				square2.vertical1 = square.vertical0;
				square2.vertical2 = square.vertical0;	     

				poly_count += p_sds_ts_edge_split(&polygon[poly_count], &square2, v_level * (level3 / level1), v_level);
			}
		}
	}

	if(level1 == level3 && level2 == level0)
		return poly_count;

//	if((v_level % 2 == 1 && h_level % 2 == 1) || (v_level == 0 && h_level == 0))
	{
		if(square.horizontal0 == 0)
		{
			if(square.vertical0 == 0)
			{
				square2.horizontal0 = 1;
				square2.horizontal1 = 1;
				square2.horizontal2 = square.horizontal1;
				square2.horizontal3 = square.horizontal1;
				square2.vertical0 = square.vertical2;
				square2.vertical1 = 1;
				square2.vertical2 = 1;
				square2.vertical3 = square.vertical2;
				poly_count += p_sds_ts_corner_split(&polygon[poly_count], &square2, level1 / level3, level2 / level0);
			}
			else
			{
				square2.horizontal0 = square.horizontal1;
				square2.horizontal1 = 1;
				square2.horizontal2 = 1;
				square2.horizontal3 = square.horizontal1;
				square2.vertical0 = 0;
				square2.vertical1 = 0;
				square2.vertical2 = square.vertical1;
				square2.vertical3 = square.vertical1;
				poly_count += p_sds_ts_corner_split(&polygon[poly_count], &square2, level0 / level2, level1 / level3);
			}
		}
		else
		{
			if(square.vertical0 == 0)
			{
				square2.horizontal0 = square.horizontal0;
				square2.horizontal1 = 0;
				square2.horizontal2 = 0;
				square2.horizontal3 = square.horizontal0;
				square2.vertical0 = 1;
				square2.vertical1 = 1;
				square2.vertical2 = square.vertical2;
				square2.vertical3 = square.vertical2;
				poly_count += p_sds_ts_corner_split(&polygon[poly_count], &square2, level2 / level0, level3 / level1);
			}
			else
			{
				square2.horizontal0 = 0;
				square2.horizontal1 = 0;
				square2.horizontal2 = square.horizontal0;
				square2.horizontal3 = square.horizontal0;
				square2.vertical0 = square.vertical0;
				square2.vertical1 = 0;
				square2.vertical2 = 0;
				square2.vertical3 = square.vertical0;
				poly_count += p_sds_ts_corner_split(&polygon[poly_count], &square2, level3 / level1, level0 / level2);
			}
		}
	}
	return poly_count;
}

  */
uint p_sds_ts_calculate_tri_polyon_count(uint edge_level0, uint edge_level1, uint edge_level2)
{
	uint polycount, edgecount, i, j;
	uint tess_level = edge_level0;

	if(edge_level1 < tess_level)
		tess_level = edge_level1;
	if(edge_level2 < tess_level)
		tess_level = edge_level2;

	edgecount = 1;
	polycount = 1;
	for(i = 0 ;i < tess_level; i++)
	{
		edgecount *= 2;
		polycount *= 4;
	}

	j = 1;
	for(i = 0 ;i < edge_level0 - tess_level; i++)
		j *= 2;
	polycount += (j * edgecount) - edgecount;

	j = 1;
	for(i = 0 ;i < edge_level1 - tess_level; i++)
		j *= 2;
	polycount += (j * edgecount) - edgecount;

	j = 1;
	for(i = 0 ;i < edge_level2 - tess_level; i++)
		j *= 2;
	polycount += (j * edgecount) - edgecount;

	return polycount;
}

void p_sds_ts_split_polygon(weight_polygon *read, weight_polygon *write, uint edge)
{
	uint vertex0 , vertex1 , vertex2;

	if(edge == 0)
	{
		vertex0 = 0;
		vertex1 = 1;
		vertex2 = 2;
	}
	else if(edge == 1)
	{
		vertex0 = 1;
		vertex1 = 2;
		vertex2 = 0;
	}
	else
	{
	        
		vertex0 = 2;
		vertex1 = 0;
		vertex2 = 1;
	}

	write[0].edge[vertex0] = read->edge[vertex0];		     
	write[0].edge[vertex1] = 5;
	write[0].edge[vertex2] = read->edge[vertex2];
	write[0].edge[3] = 5;

	write[1].edge[vertex0] = read->edge[vertex0];
	write[1].edge[vertex1] = read->edge[vertex1];	     
	write[1].edge[vertex2] = 5;
	write[1].edge[3] = 5;

	write[0].level_of_edge[vertex0] = read->level_of_edge[vertex0] + 1;
	write[0].level_of_edge[vertex1] = 0;
	write[0].level_of_edge[vertex2] = read->level_of_edge[vertex2];

	write[1].level_of_edge[vertex0] = read->level_of_edge[vertex0] + 1;
	write[1].level_of_edge[vertex1] = read->level_of_edge[vertex1];
	write[1].level_of_edge[vertex2] = 0;

	p_sds_ts_copy_vertex(read, write[0].corner[vertex0], vertex0); /* first polygon */
	p_sds_ts_divide_edge(read, write[0].corner[vertex1], vertex0, vertex1);
	p_sds_ts_copy_vertex(read, write[0].corner[vertex2], vertex2);	      

	p_sds_ts_divide_edge(read, write[1].corner[vertex0], vertex1, vertex0);/* second polygon */
	p_sds_ts_copy_vertex(read, write[1].corner[vertex1], vertex1); 
	p_sds_ts_copy_vertex(read, write[1].corner[vertex2], vertex2); 
}

weight_polygon *p_sds_ts_split_tri_edges(weight_polygon *read, weight_polygon *write1, uint in_count, uint out_count, uint *level)
{
	weight_polygon *temp; 
	uint current_polygon, added_polygons = 0, current_edge = 1;
	uint i;

	while(in_count < out_count)
	{
		current_polygon = 0;
		while(current_polygon < in_count )
		{
			for(i = 0 ; i < 3 ; i++)
			{
				if(read[current_polygon].edge[i] == current_edge)
				{
					if(read[current_polygon].level_of_edge[i] < level[current_edge])
					{
						p_sds_ts_split_polygon(&read[current_polygon], &write1[current_polygon + added_polygons], i);
						added_polygons++;
						i = 5;
					}
				}
			}
			if(i == 3)
				 write1[current_polygon + added_polygons] = read[current_polygon];

			current_polygon++;
		}


		in_count += added_polygons;
		added_polygons = 0;
		++current_edge;
		if(current_edge == 3)
			current_edge = 0;

		temp = read;
		read = write1;
		write1 = temp;
	}
	return read;
}
