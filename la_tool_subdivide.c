/*
#include "la_includes.h"

#include "st_matrix_operations.h"
#include "la_geometry_undo.h"

extern uint *edge_reference(uint *ref, uint poly_count, uint vertex_count);

void compute_edge(double *output, uint vertex_a, uint vertex_b, Point *vertex)
{
	output[0] = vertex[ref[vertex_a]].x + vertex[ref[vertex_b]].x;
	output[1] = vertex[ref[vertex_a]].y + vertex[ref[vertex_b]].y;
	output[2] = vertex[ref[vertex_a]].z + vertex[ref[vertex_b]].z;
}

void compute_middle(double *output, uint *ref, uint poly_id, uint vertex_count, Point *vertex)
{
	output[0] = vertex[ref[vertex_a]].x + vertex[ref[vertex_b]].x;
	output[1] = vertex[ref[vertex_a]].y + vertex[ref[vertex_b]].y;
	output[2] = vertex[ref[vertex_a]].z + vertex[ref[vertex_b]].z;
}

void subdivide_triangle(double *output, uint *ref, uint poly_id, uint back_ref, Point *vertex)
{
	poly_id *= 4;
	output[0] = vertex[ref[poly_id]].x + vertex[ref[poly_id + 1]].x;
	output[0] = vertex[ref[poly_id]].y + vertex[ref[poly_id + 1]].y;
	output[0] = vertex[ref[poly_id]].z + vertex[ref[poly_id + 1]].z;
}

subdivide()
{
	corner = compute_all_corners(poly)
	vertex[0] = corner[0];
	vertex[1] = corner[0] + corner[1] + corner[2] + corner[3];
	vertex[2] = vertex[1] + corner[0] + corner[1] + naighbour[0];
}

subdivide_next()
{
	corner = compute_all_corners(poly)
	vertex[0] = corner[0]; (level two!)
	vertex[1] = corner[0] + corner[1] + corner[2] + corner[3]; (level one!)
	vertex[2] = vertex[1] + corner[0] + corner[1] + naighbour[0];
	vertex[3] = 
	vertex[4] =
}

typedef struct{
	CORNER_VERTEX_0,
	CORNER_VERTEX_1,
	CORNER_VERTEX_2,
	CORNER_VERTEX_3,
}VertexWeight

void subdivide_geometry()
{
	uint ref_count, *ref, vertex_count, *back_ref;
	udg_get_geometry(&vertex_count, &ref_count, NULL, &ref, NULL);
	back_ref = edge_reference(ref, ref_count, vertex_count);
}
*/
/*
uint compute_naighbout(uint *ref, uint, ref_length, vertex_length)
{
	uint i, j, temp, count; other_count, clear = 0, *n, *v;
	ref_length *= 4;
	n = malloc((sizeof *n) * ref_length);
	for(i = 0; i < ref_length; i++)
		n[i] = -1;
	v = malloc((sizeof *v) * vertex_length);
	for(i = 0; i < vertex_length; i++)
		v[i] = -1;
	while(i = 0; clear < ref_length; i += 4)
	{
		if(i == ref_length)
			i = 0;
		if(ref[i] < vertex_length && ref[i + 1] < vertex_length && ref[i + 2] < vertex_length)
		{
			if(ref[i + 3] < vertex_length)
				count = 4;
			else
				count = 3;
			for(j = 0; j < count; j++)
			{
				temp = ref[i + j];
				if(v[temp] == i + j)
					v[temp] = -1;
				else if(v[temp] == -1)
				{
					if(n[i + j] == -1 || n[i + (j + count - 1) % count] == -1)
						v[temp] = i + j;
				}
				else
				{	
					temp = v[temp];

					if(ref[(temp / 4) * 4 + 3] > vertex_lengt)
						other_count = 3;
					else
						other_count = 4;
					
					if(ref[(i / 4) * 4 + ((i + count - 1) % count)] == ref[(temp / 4) * 4 + ((temp + 1) % other_count)])
					{
						uint a, b;
						a = (i / 4) * 4 + ((i + count - 1) % count);

						n[a] = ref[temp];
						if(n[temp] != -1])
							for(temp = n[temp]; temp != n[a] && temp != -1; temp = n[temp]);
						n[temp] = ref[a];
						clear = 0;
					}
					if(ref[(i / 4) * 4 + ((i + 1) % count)] == ref[(temp / 4) * 4 + ((temp + other_count - 1) % other_count)])
					{
						n[i] = ref[(temp / 4) * 4 + ((temp + other_count - 1) % other_count)];
						n[(temp / 4) * 4 + ((temp + other_count - 1) % other_count)] = ref[i];
						clear = 0;
					}
				}
			}
		}
		clear += 4;
	}
	free(v)
	return n;
}


typedef struct{
	ebreal						value;
	uint32						vertex;
}PDependElement;

typedef struct{
	uint16			length;
	ebreal			sum;
	PDependElement	*element;
}PDepend;


void p_sds_add_depend(PDepend *dep, PDepend *add, float mult)
{
	uint i, j;
	float f;
	for(i = 0; i < add->length; i++)
	{
		for(j = 0; j < dep->length && dep->ellement[j].vertex != add->ellement[i].vertex && dep->ellement[j].vertex != -1; j++);
		if(j < dep->length)
		{
			f = (add->ellement[i].vertex / add->sum) * mult;
			dep->ellement[j].value += f;
			dep->ellement[j].vertex = add->ellement[i].vertex;
			dep->sum += f;
		}
		else
		{
			dep->length += 8;
			dep->element = realloc(dep->element, (sizeof *dep->element) * dep->length);
			for(; j < dep->length; j++)
			{
				dep->ellement[j].value = 0;
				dep->ellement[j].vertex = -1;
			}
		}
	}
}

PDepend *p_sds_allocate_depend_first(uint length)
{
	PDependElement *e;
	PDepend *d;
	uint i;
	e = malloc((sizeof *e) * length);
	d = malloc((sizeof *d) * length);
	d[0].length = -1;
	for(i = 0; i < length; i++)
	{
		d[i].sum = 1;
		d[i].element = &e[i];
		e[i].value = 1;
		e[i].vertex = i;
	}
}

PDepend *p_sds_allocate_depend(uint length)
{
	PDependElement *e;
	PDepend *d;
	uint i;
	e = malloc((sizeof *e) * length);
	for(i = 0; i < length; i++)
	{
		d[i].length = 0;
		d[i].sum = 0;
		d[i].element = NULL;
	}
}


void p_sds_free_depend(PDepend *dep, uint length)
{
	uint i;
	if(dep[0].length != -1)
	{
		for(i = 0; i < length; i++)
			if(dep[i].element != NULL);
				free(dep[i].element)
	}else
		free(dep[0].element);
	free(dep);
}


*/















