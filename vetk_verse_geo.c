#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "enough.h"
#include "vetk.h"

void vetk_compute_neighbor(ENode *node, VETKQuadTri *poly)
{
	uint i, cor, clear = 0, *n, *v, a, b, *ref;
	uint counter = 0, laps = 0, vertex_count;
	ref = poly->ref;
	vertex_count = e_nsg_get_vertex_length(node);
	n = malloc((sizeof *n) * (poly->quad_length + poly->tri_length));
	for(i = 0; i < (poly->quad_length + poly->tri_length); i++)
		n[i] = -1;
	v = malloc((sizeof *v) * vertex_count);
	for(i = 0; i < vertex_count; i++)
		v[i] = -1;
	while(clear < poly->quad_length + poly->tri_length)
	{
		for(i = 0; i < poly->quad_length && clear < poly->quad_length + poly->tri_length; i++)
		{
			counter++;
			cor = v[ref[i]];
			if(cor == -1)
			{
				if(n[i] == -1 || n[(i / 4) * 4 + (i + 3) % 4] == -1)
					v[ref[i]] = i;
		//		else
		//			printf("jump!");
			}
			else if(cor == i)
				v[ref[i]] = -1;
			else
			{
				if(cor >= poly->quad_length)
				{	/* other poly is a tri */
					a = (i / 4) * 4;
					b = poly->quad_length + ((cor - poly->quad_length) / 3) * 3;
					if((n[cor] == -1 && n[a + (i + 3) % 4] == -1) && ref[a + (i + 3) % 4] == ref[b + (cor - b + 1) % 3])
					{
						n[a + (i + 3) % 4] = cor;
						n[cor] = a + (i + 3) % 4;
//						printf("i = %u clear = %u\n", i, clear); 
						clear = 0;
						if(n[b + (cor - b + 2) % 3] != -1)
						{
							if(n[i] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
					}
					if((n[i] == -1 && n[b + (cor - b + 2) % 3] == -1) && ref[a + (i + 1) % 4] == ref[b + (cor - b + 2) % 3])
					{
						n[i] = b + (cor - b + 2) % 3;						
						n[b + (cor - b + 2) % 3] = i;
//						printf("i = %u clear = %u\n", i, clear); 
						clear = 0;
						if(n[cor] != -1)
						{
							if(n[a + (i + 3) % 4] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
				}else
				{	
					/* other poly is a quad */
					a = (i / 4) * 4;
					b = (cor / 4) * 4;
					if((n[cor] == -1 && n[a + (i + 3) % 4] == -1) && ref[a + (i + 3) % 4] == ref[b + (cor + 1) % 4])
					{
						n[a + (i + 3) % 4] = cor;
						n[cor] = a + (i + 3) % 4;
//						printf("i = %u clear = %u\n", i, clear); 
						clear = 0;	
//						v[ref[i]] = -1;
						if(n[b + (cor + 3) % 4] != -1)
						{
							if(n[i] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
					}
					if((n[i] == -1 && n[b + (cor + 3) % 4] == -1) && ref[a + (i + 1) % 4] == ref[b + (cor + 3) % 4])
					{
						n[i] = b + (cor + 3) % 4;
						n[b + (cor + 3) % 4] = i;
//						printf("i = %u clear = %u\n", i, clear); 
						clear = 0;	
						if(n[cor] != -1)
						{
							if(n[a + (i + 3) % 4] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
					}
				}						
			}
			clear++;
		}
		for(; i < poly->quad_length + poly->tri_length && clear < poly->quad_length + poly->tri_length; i++)
		{
			cor = v[ref[i]];
			if(cor == -1)
			{
			//	if(ncor == -1)
				v[ref[i]] = i;
			}
			else if(cor == i)
				v[ref[i]] = -1;
			else 
			{
				if(cor >= poly->quad_length)
				{	/* other poly is a tri */
					a = poly->quad_length + ((i - poly->quad_length) / 3) * 3;
					b = poly->quad_length + ((cor - poly->quad_length) / 3) * 3;
					if((n[cor] == -1 && n[a + (i - a + 2) % 3] == -1) && ref[a + (i - a + 2) % 3] == ref[b + (cor - b + 1) % 3])
					{
						n[a + (i - a + 2) % 3] = cor;
						n[cor] = a + (i - a + 2) % 3;
		//				printf("i = %u clear = %u\n", i, clear); 
						clear = 0;
						if(n[b + (cor - b + 2) % 3] != -1)
						{
							if(n[i] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
					if((n[i] == -1 && n[b + (cor - b + 2) % 3] == -1) && ref[a + (i - a + 1) % 3] == ref[b + (cor - b + 2) % 3])
					{
						n[i] = b + (cor - b + 2) % 3;						
						n[b + (cor - b + 2) % 3] = i;
//						printf("i = %u clear = %u\n", i, clear); 
						clear = 0;
						if(n[cor] != -1)
						{
							if(n[a + (i - a + 2) % 3] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
				}else
				{
					/* other poly is a quad */
					a = poly->quad_length + ((i - poly->quad_length) / 3) * 3;
					b = (cor / 4) * 4;
					if((n[cor] == -1 && n[a + (i - a + 2) % 3] == -1) && ref[a + (i - a + 2) % 3] == ref[b + (cor + 1) % 4])
					{
						n[a + (i - a + 2) % 3] = cor;
						n[cor] = a + (i - a + 2) % 3;
//						printf("i = %u clear = %u\n", i, clear); 
						clear = 0;
						if(n[b + (cor + 3) % 4] != -1)
						{
							if(n[i] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
					if((n[i] == -1 && n[(cor - b + 3) % 4] == -1) && ref[a + (i - a + 1) % 3] == ref[b + (cor + 3) % 4])
					{
						n[i] = b + (cor + 3) % 4;
						n[b + (cor + 3) % 4] = i;				
//						printf("i = %u clear = %u\n", i, clear); 						
						clear = 0;
						if(n[cor] != -1)
						{
							if(n[a + (i - a + 2) % 3] == -1)
								v[ref[i]] = i;
							else
								v[ref[i]] = -1;
						}
//						v[ref[i]] = -1;
					}
				}						
			}
			counter++;
			clear++;
		}
		laps++;
		
	}
	counter = 0;
	free(v);
	poly->neighbor = n;
}

void *vetk_get_vertex_param(ENode *node, VETKQuadTri *mesh, char **vertex_params, uint param_count, float *normal)
{
	uint i, j, tri, quad, ref_count, vertex_count, *ref, axis;
	egreal *vertex;
	char *name, *n = "normal";
	EGeoLayer *layer;
	void *data;
	void *output;

	vertex_count = e_nsg_get_vertex_length(node);
	ref_count = e_nsg_get_polygon_length(node) * 4;
	vertex = e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node, 0));
	ref = e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node, 1));
	output = malloc(4 * (mesh->quad_length + mesh->tri_length) * param_count);
	for(i = 0; i < param_count; i++)
	{
		axis = 0;
		quad = 0;
		tri = mesh->quad_length;
		name = vertex_params[i];
		if(name[1] == 46 && name[0] >= 120 && name[0] <= 122)
		{
			axis = name[0] - 120;
			name = &name[2];
		}
		layer = e_nsg_get_layer_by_name(node, name);
		data = e_nsg_get_layer_data(node, layer);

		if(layer == NULL || data == NULL || (VN_G_LAYER_VERTEX_XYZ != e_nsg_get_layer_type(layer) && axis != 0))
		{
			for(j = 0; name[j] == n[j] && name[j] != 0; j++);
			if(name[j] != 0)
			{
				for(j = 0; j < mesh->quad_length + mesh->tri_length; j++)
					((float *)output)[j * param_count + i] = 0.0;
			}else
			{
				for(j = 0; j < mesh->quad_length + mesh->tri_length; j++)
				{
					((float *)output)[j * param_count + i] = normal[j * 3 + axis];
				}
			}
		}else
		{
			switch(e_nsg_get_layer_type(layer))
			{
				case VN_G_LAYER_VERTEX_XYZ :
					for(j = 0; j < ref_count; j += 4)
					{
						if(ref[j] < vertex_count && ref[j + 1] < vertex_count &&  ref[j + 2] < vertex_count && vertex[ref[j] * 3] != E_REAL_MAX && vertex[ref[j + 1] * 3] != E_REAL_MAX && vertex[ref[j + 2] * 3] != E_REAL_MAX)
						{
							if(ref[j + 3] < vertex_count && vertex[ref[j + 3] * 3] != E_REAL_MAX)
							{
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[ref[j + 0] * 3 + axis];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[ref[j + 1] * 3 + axis];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[ref[j + 2] * 3 + axis];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[ref[j + 3] * 3 + axis];
							}else
							{
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[ref[j + 0] * 3 + axis];
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[ref[j + 1] * 3 + axis];
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[ref[j + 2] * 3 + axis];
							}
						}
					}
				break;
				case VN_G_LAYER_VERTEX_UINT32 :
					for(j = 0; j < ref_count; j += 4)
					{
						if(ref[j] < vertex_count && ref[j + 1] < vertex_count &&  ref[j + 2] < vertex_count && vertex[ref[j] * 3] != E_REAL_MAX && vertex[ref[j + 1] * 3] != E_REAL_MAX && vertex[ref[j + 2] * 3] != E_REAL_MAX)
						{
							if(ref[j + 3] < vertex_count && vertex[ref[j + 3] * 3] != E_REAL_MAX)
							{
								((uint32 *)output)[quad++ * param_count + i] = ((uint32 *)data)[ref[j + 0]];
								((uint32 *)output)[quad++ * param_count + i] = ((uint32 *)data)[ref[j + 1]];
								((uint32 *)output)[quad++ * param_count + i] = ((uint32 *)data)[ref[j + 2]];
								((uint32 *)output)[quad++ * param_count + i] = ((uint32 *)data)[ref[j + 3]];
							}else
							{
								((uint32 *)output)[tri++ * param_count + i] = ((uint32 *)data)[ref[j + 0]];
								((uint32 *)output)[tri++ * param_count + i] = ((uint32 *)data)[ref[j + 1]];
								((uint32 *)output)[tri++ * param_count + i] = ((uint32 *)data)[ref[j + 2]];
							}
						}
					}
				break;
				case VN_G_LAYER_VERTEX_REAL :
					for(j = 0; j < ref_count; j += 4)
					{
						if(ref[j] < vertex_count && ref[j + 1] < vertex_count &&  ref[j + 2] < vertex_count && vertex[ref[j] * 3] != E_REAL_MAX && vertex[ref[j + 1] * 3] != E_REAL_MAX && vertex[ref[j + 2] * 3] != E_REAL_MAX)
						{
							if(ref[j + 3] < vertex_count && vertex[ref[j + 3] * 3] != E_REAL_MAX)
							{
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[ref[j + 0]];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[ref[j + 1]];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[ref[j + 2]];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[ref[j + 3]];
							}else
							{
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[ref[j + 0]];
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[ref[j + 1]];
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[ref[j + 2]];
							}
						}
					}
				break;
				case VN_G_LAYER_POLYGON_CORNER_UINT32 :
					for(j = 0; j < ref_count; j += 4)
					{
						if(ref[j] < vertex_count && ref[j + 1] < vertex_count &&  ref[j + 2] < vertex_count && vertex[ref[j] * 3] != E_REAL_MAX && vertex[ref[j + 1] * 3] != E_REAL_MAX && vertex[ref[j + 2] * 3] != E_REAL_MAX)
						{
							if(ref[j + 3] < vertex_count && vertex[ref[j + 3] * 3] != E_REAL_MAX)
							{
								((float *)output)[quad++ * param_count + i] = ((uint32 *)data)[j + 0];
								((float *)output)[quad++ * param_count + i] = ((uint32 *)data)[j + 1];
								((float *)output)[quad++ * param_count + i] = ((uint32 *)data)[j + 2];
								((float *)output)[quad++ * param_count + i] = ((uint32 *)data)[j + 3];
							}else
							{
								((float *)output)[tri++ * param_count + i] = ((uint32 *)data)[j + 0];
								((float *)output)[tri++ * param_count + i] = ((uint32 *)data)[j + 1];
								((float *)output)[tri++ * param_count + i] = ((uint32 *)data)[j + 2];
							}
						}
					}
				break;
				case VN_G_LAYER_POLYGON_CORNER_REAL :
					for(j = 0; j < ref_count; j += 4)
					{
						if(ref[j] < vertex_count && ref[j + 1] < vertex_count &&  ref[j + 2] < vertex_count && vertex[ref[j] * 3] != E_REAL_MAX && vertex[ref[j + 1] * 3] != E_REAL_MAX && vertex[ref[j + 2] * 3] != E_REAL_MAX)
						{
							if(ref[j + 3] < vertex_count && vertex[ref[j + 3] * 3] != E_REAL_MAX)
							{
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[j + 0];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[j + 1];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[j + 2];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[j + 3];
							}else
							{
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[j + 0];
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[j + 1];
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[j + 2];
							}
						}
					}
				break;
				case VN_G_LAYER_POLYGON_FACE_UINT8 :
					for(j = 0; j < ref_count; j += 4)
					{
						if(ref[j] < vertex_count && ref[j + 1] < vertex_count &&  ref[j + 2] < vertex_count && vertex[ref[j] * 3] != E_REAL_MAX && vertex[ref[j + 1] * 3] != E_REAL_MAX && vertex[ref[j + 2] * 3] != E_REAL_MAX)
						{
							if(ref[j + 3] < vertex_count && vertex[ref[j + 3] * 3] != E_REAL_MAX)
							{
								((float *)output)[quad++ * param_count + i] = ((uint8 *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((uint8 *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((uint8 *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((uint8 *)data)[j / 4];
							}else
							{
								((float *)output)[tri++ * param_count + i] = ((uint8 *)data)[j / 4];
								((float *)output)[tri++ * param_count + i] = ((uint8 *)data)[j / 4];
								((float *)output)[tri++ * param_count + i] = ((uint8 *)data)[j / 4];
							}
						}
					}
				break;
				case VN_G_LAYER_POLYGON_FACE_UINT32 :
					for(j = 0; j < ref_count; j += 4)
					{
						if(ref[j] < vertex_count && ref[j + 1] < vertex_count &&  ref[j + 2] < vertex_count && vertex[ref[j] * 3] != E_REAL_MAX && vertex[ref[j + 1] * 3] != E_REAL_MAX && vertex[ref[j + 2] * 3] != E_REAL_MAX)
						{
							if(ref[j + 3] < vertex_count && vertex[ref[j + 3] * 3] != E_REAL_MAX)
							{
								((float *)output)[quad++ * param_count + i] = ((uint32 *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((uint32 *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((uint32 *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((uint32 *)data)[j / 4];
							}else
							{
								((float *)output)[tri++ * param_count + i] = ((uint32 *)data)[j / 4];
								((float *)output)[tri++ * param_count + i] = ((uint32 *)data)[j / 4];
								((float *)output)[tri++ * param_count + i] = ((uint32 *)data)[j / 4];
							}
						}
					}
				break;
				case VN_G_LAYER_POLYGON_FACE_REAL :
					for(j = 0; j < ref_count; j += 4)
					{
						if(ref[j] < vertex_count && ref[j + 1] < vertex_count &&  ref[j + 2] < vertex_count && vertex[ref[j] * 3] != E_REAL_MAX && vertex[ref[j + 1] * 3] != E_REAL_MAX && vertex[ref[j + 2] * 3] != E_REAL_MAX)
						{
							if(ref[j + 3] < vertex_count && vertex[ref[j + 3] * 3] != E_REAL_MAX)
							{
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[j / 4];
								((float *)output)[quad++ * param_count + i] = ((egreal *)data)[j / 4];
							}else
							{
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[j / 4];
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[j / 4];
								((float *)output)[tri++ * param_count + i] = ((egreal *)data)[j / 4];
							}
						}
					}
				break;
			}
		}
	}
	return output;
}


uint *vetk_poly_clean(ENode *node, VETKQuadTri *mesh, char **vertex_params, uint param_count)
{
	uint *ref, *crease, vertex_count, ref_count, i, def,tri, quad;
	egreal *vertex;
	uint *c;
	vertex = e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node, 0));
	ref = e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node, 1));
	crease = e_nsg_get_layer_data(node, e_nsg_get_layer_crease_edge_layer(node));
	vertex_count = e_nsg_get_vertex_length(node);
	ref_count = e_nsg_get_polygon_length(node) * 4;
	mesh->quad_length = 0;
	mesh->tri_length = 0;	
	for(i = 0; i < ref_count; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
		{
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				mesh->quad_length += 4;
			else
				mesh->tri_length += 3;				
		}
	}
	if(mesh->quad_length + mesh->tri_length == 0)
		return NULL;
	mesh->ref = malloc((sizeof *mesh->ref) * (mesh->quad_length + mesh->tri_length));
	c = malloc((sizeof *c) * (mesh->quad_length + mesh->tri_length));
	tri = mesh->quad_length;
	quad = 0;
	if(crease == NULL)
	{
		def = e_nsg_get_layer_crease_edge_value(node);
		for(i = 0; i < mesh->quad_length + mesh->tri_length; i++)
			c[i] = def;

		for(i = 0; i < ref_count; i += 4)
		{
			if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
			{
				if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				{
					mesh->ref[quad++] = ref[i];
					mesh->ref[quad++] = ref[i + 1];
					mesh->ref[quad++] = ref[i + 2];
					mesh->ref[quad++] = ref[i + 3];
				}else
				{
					mesh->ref[tri++] = ref[i];
					mesh->ref[tri++] = ref[i + 1];
					mesh->ref[tri++] = ref[i + 2];
				}
			}
		}
	}else
	{
		for(i = 0; i < ref_count; i += 4)
		{
			if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
			{
				if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				{
					mesh->ref[quad] = ref[i];
					c[quad++] = crease[i];
					mesh->ref[quad] = ref[i + 1];
					c[quad++] = crease[i + 1];
					mesh->ref[quad] = ref[i + 2];
					c[quad++] = crease[i + 2];
					mesh->ref[quad] = ref[i + 3];
					c[quad++] = crease[i + 3];
				}else
				{
					mesh->ref[tri] = ref[i];
					c[tri++] = crease[i];
					mesh->ref[tri] = ref[i + 1];
					c[tri++] = crease[i + 1];
					mesh->ref[tri] = ref[i + 2];
					c[tri++] = crease[i + 2];
				}
			}
		}
	}
	return c;
}

void vetk_vec_normalize(egreal *output, egreal *vec_a, egreal *vec_b)
{
	egreal f;
	output[0] = vec_a[0] - vec_b[0];
	output[1] = vec_a[1] - vec_b[1];
	output[2] = vec_a[2] - vec_b[2];
	f = sqrt(output[0] * output[0] + output[1] * output[1] + output[2] * output[2]);
	output[0] /= f;
	output[1] /= f;
	output[2] /= f;
}

void vetk_corner_normal_old(VETKQuadTri *mesh, float *corner_normal, float *poly_normal, uint first, uint *crease)
{
	uint cur, p, next = -1;
	egreal n[3], f;
	next = first; 
	n[0] = 0;
	n[1] = 0;
	n[2] = 0;
	while(TRUE)
	{
		cur = next;
		if(cur > mesh->quad_length)
			p = mesh->quad_length / 4 + (cur - mesh->quad_length) / 3;
		else
			p = cur / 4;
	//	printf("add %u - %f %f %f\n", p, poly_normal[p * 3 + 0], poly_normal[p * 3 + 1], poly_normal[p * 3 + 2]);
		n[0] += poly_normal[p * 3 + 0];
		n[1] += poly_normal[p * 3 + 1];
		n[2] += poly_normal[p * 3 + 2];
		if(crease[cur] > 2000000000 || mesh->neighbor[cur] == -1)
			break;
		cur = mesh->neighbor[cur];
		if(cur > mesh->quad_length)
			next = mesh->quad_length + (((cur - mesh->quad_length) / 3) * 3) + (((cur - mesh->quad_length) + 1) % 3);
		else
			next = ((cur / 4) * 4) + ((cur + 1) % 4);
		if(next  == first)
			break;
	}
	if(next != first)
	{
		if(crease[cur] < 2000000000 && mesh->neighbor[next] != -1)
		{
			cur = mesh->neighbor[next];
			if(cur != -1)
			{
				if(cur > mesh->quad_length)
					next = mesh->quad_length + (((cur - mesh->quad_length) / 3) * 3) + (((cur - mesh->quad_length) + 3) % 3);
				else
					next = ((cur / 4) * 4) + ((cur + 2) % 4);
				first = next;
				while(TRUE)
				{
					cur = next;
					if(cur > mesh->quad_length)
						p = mesh->quad_length / 4 + (cur - mesh->quad_length) / 3;
					else
						p = cur / 4;
				//	printf("add %u - %f %f %f\n", p, poly_normal[p * 3 + 0], poly_normal[p * 3 + 1], poly_normal[p * 3 + 2]);
					n[0] += poly_normal[p * 3 + 0];
					n[1] += poly_normal[p * 3 + 1];
					n[2] += poly_normal[p * 3 + 2];
					if(crease[cur] > 2000000000 || mesh->neighbor[cur] == -1)
						break;
					cur = mesh->neighbor[cur];
					if(cur > mesh->quad_length)
						next = mesh->quad_length + (((cur - mesh->quad_length) / 3) * 3) + (((cur - mesh->quad_length) + 3) % 3);
					else
						next = ((cur / 4) * 4) + ((cur + 2) % 4);
					if(next == first)
						break;
				}
			}
		}
	}

	f = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
	corner_normal[first * 3 + 0] = n[0] / f;
	corner_normal[first * 3 + 1] = n[1] / f;
	corner_normal[first * 3 + 2] = n[2] / f;
}

void vetk_corner_normal(VETKQuadTri *mesh, float *corner_normal, float *poly_normal, uint first, uint *crease)
{
	uint cur, p, next = -1, counter = 0, break_out;
	egreal n[3], f;
	next = first; 
	n[0] = 0;
	n[1] = 0;
	n[2] = 0;
	for(break_out = 0;  break_out < 256; break_out++)
	{
		cur = next;
		next = -1;
		if(cur > mesh->quad_length)
			p = mesh->quad_length / 4 + (cur - mesh->quad_length) / 3;
		else
			p = cur / 4;
		n[0] += poly_normal[p * 3 + 0];
		n[1] += poly_normal[p * 3 + 1];
		n[2] += poly_normal[p * 3 + 2];
		counter++;
		if(crease[cur] > 2000000000 || mesh->neighbor[cur] == -1)
			break;
		cur = mesh->neighbor[cur];
		if(cur > mesh->quad_length)
			next = mesh->quad_length + (((cur - mesh->quad_length) / 3) * 3) + (((cur - mesh->quad_length) + 1) % 3);
		else
			next = ((cur / 4) * 4) + ((cur + 1) % 4);
		if(next == first)
			break;
	}
	if(next != first)
	{
		cur = first;
		if(cur > mesh->quad_length)
			next = mesh->quad_length + (((cur - mesh->quad_length) / 3) * 3) + (((cur - mesh->quad_length) + 2) % 3);
		else
			next = ((cur / 4) * 4) + ((cur + 3) % 4);
		counter = 0;
		if(crease[next] < 2000000000 && mesh->neighbor[next] != -1)
		{
			cur = mesh->neighbor[next];
			if(cur > mesh->quad_length)
				next = mesh->quad_length + (((cur - mesh->quad_length) / 3) * 3) + (((cur - mesh->quad_length) + 2) % 3);
			else
				next = ((cur / 4) * 4) + ((cur + 3) % 4);
			while(TRUE)
			{
				cur = next;
				if(cur > mesh->quad_length)
					p = mesh->quad_length / 4 + (cur - mesh->quad_length) / 3;
				else
					p = cur / 4;
				
				n[0] += poly_normal[p * 3 + 0];
				n[1] += poly_normal[p * 3 + 1];
				n[2] += poly_normal[p * 3 + 2];
				counter++;
				if(crease[cur] > 2000000000 || mesh->neighbor[cur] == -1 || counter > 500)
					break;
				cur = mesh->neighbor[cur];
				if(cur > mesh->quad_length)
					next = mesh->quad_length + (((cur - mesh->quad_length) / 3) * 3) + (((cur - mesh->quad_length) + 2) % 3);
				else
					next = ((cur / 4) * 4) + ((cur + 3) % 4);
				if(next == first)
					break;
			}
		}
	}

	f = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
	corner_normal[first * 3 + 0] = n[0] / f;
	corner_normal[first * 3 + 1] = n[1] / f;
	corner_normal[first * 3 + 2] = n[2] / f;
}

float *vetk_poly_normal(ENode *node, VETKQuadTri *mesh)
{
	egreal *vertex, *v0, *v1, *v2, *v3, vec_a[3], vec_b[3], n[3], f;
	float *normal;
	uint i, j = 0;
	vertex = e_nsg_get_layer_data(node, e_nsg_get_layer_by_id(node, 0));
	normal = malloc((sizeof *normal) * (mesh->quad_length / 4 + mesh->tri_length / 3) * 3);
	for(i = 0; i < mesh->quad_length; i += 4)
	{
		v0 = &vertex[mesh->ref[i] * 3];
		v1 = &vertex[mesh->ref[i + 1] * 3];
		v2 = &vertex[mesh->ref[i + 2] * 3];
		v3 = &vertex[mesh->ref[i + 3] * 3];
		vetk_vec_normalize(vec_a, v0, v3);
		vetk_vec_normalize(vec_b, v0, v1);
		n[0] = vec_a[1] * vec_b[2] - vec_a[2] * vec_b[1];
		n[1] = vec_a[2] * vec_b[0] - vec_a[0] * vec_b[2];
		n[2] = vec_a[0] * vec_b[1] - vec_a[1] * vec_b[0];
		vetk_vec_normalize(vec_a, v2, v1);
		vetk_vec_normalize(vec_b, v2, v3);
		n[0] += vec_a[1] * vec_b[2] - vec_a[2] * vec_b[1];
		n[1] += vec_a[2] * vec_b[0] - vec_a[0] * vec_b[2];
		n[2] += vec_a[0] * vec_b[1] - vec_a[1] * vec_b[0];
		f = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		normal[j++] = n[0] / f;
		normal[j++] = n[1] / f;
		normal[j++] = n[2] / f;
	}
	for(; i < mesh->quad_length + mesh->tri_length; i += 3)
	{
		v0 = &vertex[mesh->ref[i] * 3];
		v1 = &vertex[mesh->ref[i + 1] * 3];
		v2 = &vertex[mesh->ref[i + 2] * 3];
		vetk_vec_normalize(vec_a, v0, v2);
		vetk_vec_normalize(vec_b, v0, v1);
		n[0] = vec_a[1] * vec_b[2] - vec_a[2] * vec_b[1];
		n[1] = vec_a[2] * vec_b[0] - vec_a[0] * vec_b[2];
		n[2] = vec_a[0] * vec_b[1] - vec_a[1] * vec_b[0];
		f = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		normal[j++] = n[0] / f;
		normal[j++] = n[1] / f;
		normal[j++] = n[2] / f;
	}
	return normal;
}

void vetk_generate_ref(VETKQuadTri *mesh, uint param_count)
{
	float *vertex;
	uint *ref, i, j, k, used = 0;
	vertex = mesh->vertex;
	ref = malloc((sizeof *ref) * (mesh->quad_length + mesh->tri_length));
	for(i = 0; i < mesh->quad_length + mesh->tri_length; i++)
	{
		for(j = 0; j < used; j++)
		{
	/*		for(k = 0; k < 6 && vertex[j * param_count + k] > vertex[i * param_count + k] - 0.00001 && vertex[j * param_count + k] < vertex[i * param_count + k] + 0.00001; k++);
			if(k == 6)
			{
				ref[i] = j;
				break;
			}*/
			for(k = 0; k < param_count && vertex[j * param_count + k] > vertex[i * param_count + k] - 0.00001 && vertex[j * param_count + k] < vertex[i * param_count + k] + 0.00001; k++);
			if(k == param_count)
			{
				ref[i] = j;
				break;
			}	
		}
		if(j == used)
		{
			for(k = 0; k < param_count; k++)
				vertex[used * param_count + k] = vertex[i * param_count + k];
			ref[i] = used;
			used++;
		}
	}
	mesh->ref = ref;
	mesh->vertex_length = used;	
}


void vetk_triangulate_vertex(VETKQuadTri *mesh, uint param_count)
{
	float *vertex;
	uint i, j, k, used = 0;
	vertex = mesh->vertex;
	mesh->vertex = malloc((sizeof *mesh->vertex) * ((mesh->quad_length / 4 * 6) + mesh->tri_length) * param_count);
	for(i = 0; i < mesh->quad_length; i += 4)
	{
		for(k = 0; k < param_count; k++)
			mesh->vertex[used++] = vertex[(i + 0) * param_count + k];
		for(k = 0; k < param_count; k++)
			mesh->vertex[used++] = vertex[(i + 1) * param_count + k];
		for(k = 0; k < param_count; k++)
			mesh->vertex[used++] = vertex[(i + 2) * param_count + k];

		for(k = 0; k < param_count; k++)
			mesh->vertex[used++] = vertex[(i + 0) * param_count + k];
		for(k = 0; k < param_count; k++)
			mesh->vertex[used++] = vertex[(i + 2) * param_count + k];
		for(k = 0; k < param_count; k++)
			mesh->vertex[used++] = vertex[(i + 3) * param_count + k];
	}
	for(i = mesh->quad_length * param_count; i < (mesh->quad_length + mesh->tri_length) * param_count; i++)
		mesh->vertex[used++] = vertex[i];
	mesh->tri_length = ((mesh->quad_length / 4 * 6) + mesh->tri_length);
	mesh->quad_length = 0;
	free(vertex);
}

void vetk_triangulate_ref(VETKQuadTri *mesh)
{
	uint *ref, i, j, k, used = 0;
	ref = mesh->ref;
	mesh->ref = malloc((sizeof *ref) * ((mesh->quad_length / 4 * 6) + mesh->tri_length));
	for(i = 0; i < mesh->quad_length; i += 4)
	{
		mesh->ref[used++] = ref[i + 0];
		mesh->ref[used++] = ref[i + 1];
		mesh->ref[used++] = ref[i + 2];

		mesh->ref[used++] = ref[i + 0];
		mesh->ref[used++] = ref[i + 2];
		mesh->ref[used++] = ref[i + 3];
	}
	for(i = mesh->quad_length; i < (mesh->quad_length + mesh->tri_length); i++)
		mesh->ref[used++] = ref[i];
	mesh->tri_length = ((mesh->quad_length / 4 * 6) + mesh->tri_length);
	mesh->quad_length = 0;
	free(ref);
}

uint vetk_triangulate_neighbor_find(VETKQuadTri *mesh, uint edge)
{
	if(edge == -1)
		return -1;
	if(edge > mesh->quad_length)
		return (mesh->quad_length * 6 / 4) + (edge - mesh->quad_length);
	else
		return ((edge / 4) * 6) + ((edge % 4) / 2) * 2+ edge % 4;
}

void vetk_triangulate_neighbor(VETKQuadTri *mesh)
{
	uint *n, i, j, k, used = 0;
	n = mesh->neighbor;
	mesh->neighbor = malloc((sizeof *mesh->neighbor) * ((mesh->quad_length / 4 * 6) + mesh->tri_length));
	for(i = 0; i < mesh->quad_length; i += 4)
	{
		mesh->neighbor[used++] = vetk_triangulate_neighbor_find(mesh, n[i + 0]);
		mesh->neighbor[used++] = vetk_triangulate_neighbor_find(mesh, n[i + 1]);
		mesh->neighbor[used] = used + 1;
		used++;

		mesh->neighbor[used] = used - 1;
		used++;
		mesh->neighbor[used++] = vetk_triangulate_neighbor_find(mesh, n[i + 2]);
		mesh->neighbor[used++] = vetk_triangulate_neighbor_find(mesh, n[i + 3]);
	}
	for(i = mesh->quad_length; i < (mesh->quad_length + mesh->tri_length); i++)
		mesh->neighbor[used++] = vetk_triangulate_neighbor_find(mesh, n[i]);
	free(n);
}

VETKQuadTri *vetk_create_mesh(ENode *node, char **vertex_params, uint param_count, boolean reference, boolean quads)
{
	VETKQuadTri *mesh;
	uint i, *crease;
	float *data, *poly_normal, *corner_normal;
	mesh = malloc(sizeof *mesh);
	crease = vetk_poly_clean(node, mesh, vertex_params, param_count);
	if(crease == NULL)
	{
		free(mesh);
		return NULL;
	}
	vetk_compute_neighbor(node, mesh);
	
	poly_normal = vetk_poly_normal(node, mesh);
	//(mesh->quad_length / 4 + mesh->tri_length / 3) * 3);
	corner_normal = malloc((sizeof *corner_normal) * (mesh->quad_length + mesh->tri_length) * 3);
	for(i = 0; i < mesh->quad_length + mesh->tri_length; i++)
		vetk_corner_normal(mesh, corner_normal, poly_normal, i, crease);
	free(poly_normal);
	mesh->vertex = vetk_get_vertex_param(node, mesh, vertex_params, param_count, corner_normal);
	free(corner_normal);
	if(!quads)
		vetk_triangulate_neighbor(mesh);
	if(reference)
	{
		vetk_generate_ref(mesh, param_count);
		if(!quads)
			vetk_triangulate_ref(mesh);
	}
	else
	{
		if(!quads)
			vetk_triangulate_vertex(mesh, param_count);
		if(mesh->ref != NULL)
		{
			free(mesh->ref);
			mesh->ref = NULL;
		}
		mesh->vertex_length = mesh->quad_length + mesh->tri_length;
	}
	free(crease);
	return mesh;
}

void vetk_destroy_mesh(VETKQuadTri *mesh)
{
	if(mesh->neighbor != NULL)
		free(mesh->neighbor);
	if(mesh->vertex != NULL)
		free(mesh->vertex);
	if(mesh->ref != NULL)
		free(mesh->ref);
	free(mesh);
}