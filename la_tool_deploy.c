#include "la_includes.h"
#include "la_geometry_undo.h"

boolean *selection_fill(uint vertex)
{
    uint32 vertex_count, polygon_count, *ref, i;
	boolean *select, found = TRUE;
    udg_get_geometry(&vertex_count, &polygon_count, NULL, &ref, NULL);
    select = malloc((sizeof *select) * vertex_count);
    for(i = 0; i < vertex_count; i++)
        select[i] = FALSE;
	select[vertex] = TRUE;
	polygon_count *= 4;
    while(found == TRUE)
    {
        found = FALSE;
        for(i = 0; i < polygon_count; i += 4)
        {
            if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count)
            {
                if(select[ref[i]] || select[ref[i + 1]] || select[ref[i + 2]] || (ref[i + 3] < vertex_count && select[ref[i +3]]))
                {
                    if(select[ref[i]] == FALSE || select[ref[i + 1]] == FALSE|| select[ref[i + 2]] == FALSE || (ref[i + 3] < vertex_count && select[ref[i +3]] == FALSE))
                    {
                        select[ref[i]] = TRUE;
                        select[ref[i + 1]] = TRUE;
                        select[ref[i + 2]] = TRUE;
                        if(ref[i + 3] < vertex_count)
							select[ref[i +3]] = TRUE;
                        found = TRUE;
                    }
                }
            }
        }
    }
	return select;
}

void la_t_polygon_select_fill(uint poly)
{
    uint i, vertex_count, *ref;
    boolean *select;
    udg_get_geometry(&vertex_count, NULL, NULL, &ref, NULL);    
    select = selection_fill(ref[poly * 4]);
    for(i = 0; i < vertex_count; i++)
        if(select[i] == TRUE)
            udg_set_select(i, 1);
    free(select);
}

void la_t_vertex_select_fill(uint vertex)
{
    uint i, vertex_count;
    boolean *select;
    udg_get_geometry(&vertex_count, NULL, NULL, NULL, NULL);    
    select = selection_fill(vertex);
    for(i = 0; i < vertex_count; i++)
        if(select[i] == TRUE)
            udg_set_select(i, 1);
    free(select);
}

void create_matrix_from_polygon(double *matrix, uint *ref, boolean quad)
{
	double origo[3], point_a[3], point_b[3], a[3], b[3], c[3], d[3];
	udg_get_vertex_pos(a, ref[0]);
	udg_get_vertex_pos(b, ref[1]);
	udg_get_vertex_pos(c, ref[2]);
	if(quad)
	{
		udg_get_vertex_pos(d, ref[3]);
		origo[0] = (a[0] + b[0] + c[0] + d[0]) * 0.25;
		origo[1] = (a[1] + b[1] + c[1] + d[1]) * 0.25;
		origo[2] = (a[2] + b[2] + c[2] + d[2]) * 0.25;
	}else
	{
		origo[0] = (a[0] + b[0] + c[0]) / 3.0;
		origo[1] = (a[1] + b[1] + c[1]) / 3.0;
		origo[2] = (a[2] + b[2] + c[2]) / 3.0;
	}
	point_a[0] = (a[0] + b[0]) * 0.5;
	point_a[1] = (a[1] + b[1]) * 0.5;
	point_a[2] = (a[2] + b[2]) * 0.5;
	point_b[0] = (b[0] + c[0]) * 0.5;
	point_b[1] = (b[1] + c[1]) * 0.5;
	point_b[2] = (b[2] + c[2]) * 0.5;
	f_matrixzxd(matrix, origo, point_a, point_b);
}


typedef struct{
	uint	poly;
	uint32	v_count;
	uint32	p_count;
	uint32	*ref;
	uint32	*crease;
	uint32	*vertex_id;
	double	*vertex;
	double	origo[3];
	double	vectors[12];
}LATDeployParam;

void create_poly_vectors(double	*origo, double	*vectors, uint *ref, uint poly)
{
	uint i;
	double r, vertex[12];
	udg_get_vertex_pos(vertex, ref[0]);
	udg_get_vertex_pos(&vertex[3], ref[1]);
	udg_get_vertex_pos(&vertex[6], ref[2]);
	if(poly == 4)
	{
		udg_get_vertex_pos(&vertex[9], ref[3]);
		origo[0] = (vertex[0] + vertex[3] + vertex[6] + vertex[9]) * 0.25;
		origo[1] = (vertex[1] + vertex[4] + vertex[7] + vertex[10]) * 0.25;
		origo[2] = (vertex[2] + vertex[5] + vertex[8] + vertex[11]) * 0.25;
	}else
	{
		origo[0] = (vertex[0] + vertex[3] + vertex[6]) / 3.0;
		origo[1] = (vertex[1] + vertex[4] + vertex[7]) / 3.0;
		origo[2] = (vertex[2] + vertex[5] + vertex[8]) / 3.0;
	}
	for(i = 0; i < poly; i++)
	{
		vectors[i * 3 + 0] = (vertex[i * 3 + 0] + vertex[((i + 1) % poly) * 3 + 0]) / 2.0 - origo[0];
		vectors[i * 3 + 1] = (vertex[i * 3 + 1] + vertex[((i + 1) % poly) * 3 + 1]) / 2.0 - origo[1];
		vectors[i * 3 + 2] = (vertex[i * 3 + 2] + vertex[((i + 1) % poly) * 3 + 2]) / 2.0 - origo[2];
		r = sqrt(vectors[i * 3 + 0] * vectors[i * 3 + 0] + vectors[i * 3 + 1] * vectors[i * 3 + 1] + vectors[i * 3 + 2] * vectors[i * 3 + 2]);
		vectors[i * 3 + 0] /= r;		
		vectors[i * 3 + 1] /= r;
		vectors[i * 3 + 2] /= r;
	}
}


void negate_matrix(double *matrix)
{
	double swap;

	swap = matrix[1];
	matrix[1] = matrix[4];
	matrix[4] = swap;

	swap = matrix[2];
	matrix[2] = matrix[8];
	matrix[8] = swap;

	swap = matrix[3];
	matrix[3] = matrix[12];
	matrix[12] = swap;

	swap = matrix[6];
	matrix[6] = matrix[9];
	matrix[9] = swap;

	swap = matrix[7];
	matrix[7] = matrix[13];
	matrix[13] = swap;

	swap = matrix[11];
	matrix[11] = matrix[14];
	matrix[14] = swap;
}

uint select_poly_rotate(double *matrix, double *v_a, double *pos_a, double *v_b, double *pos_b, uint poly)
{
	double r, matrix2[16], matrix3[16], pos[] = {0, 0, 0}, best = -1;
	uint dir_a, dir_b, i, j;
	for(i = 0; i < poly; i++)
	{
		for(j = 0; j < poly; j++)
		{
			r = v_a[i * 3 + 0] * v_b[j * 3 + 0] + v_a[i * 3 + 1] * v_b[j * 3 + 1] + v_a[i * 3 + 2] * v_b[j * 3 + 2];
			if(r > best)
			{
				best = r;
				dir_a = i;
				dir_b = j;
			}
		}
	}
	f_matrixzxd(matrix2, pos, &v_a[dir_a * 3], &v_a[((poly + dir_a - 1) % poly) * 3]);
	f_matrixzxd(matrix3, pos, &v_b[dir_b * 3], &v_b[((poly + dir_b + 1) % poly) * 3]);
	negate_matrix(matrix2);
	f_matrix_multiplyd(matrix, matrix3, matrix2);
	return (poly + dir_a + dir_b + 2) % poly;
}

void grabb_marked_mesh(LATDeployParam *param, uint poly)
{
    uint32 vertex_count, polygon_count, *ref, i, j, k, *crease;
	boolean *select, found = TRUE;
	egreal *vertex;
	param->v_count = 0;
	param->p_count = 0;
    udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, &crease);
	param->poly = 3;
	if(ref[poly * 4 + 3] < vertex_count)
		param->poly = 4;
    select = selection_fill(ref[poly * 4]);
	for(i = 0; i < vertex_count; i++)
		if(select[i])
			param->v_count++;
	for(i = 0; i < polygon_count; i++)
		if(ref[i * 4] < vertex_count && select[ref[i * 4]])
			param->p_count++;
	param->ref = malloc((sizeof *param->ref) * param->p_count * 4);
	param->crease = malloc((sizeof *param->crease) * param->p_count * 4);
	param->vertex_id = malloc((sizeof *param->vertex_id) * param->v_count);
	param->vertex = malloc((sizeof *param->vertex) * param->v_count * 3);
	j = 0;
	polygon_count *= 4;
	poly *= 4;
	param->ref[3] = -1;
	for(i = 0; i < 3 || (i == 3 && ref[poly + 3] < vertex_count); i++)
	{
		param->vertex_id[i] = ref[poly + i];
		param->vertex[i * 3] = vertex[ref[poly + i] * 3];
		param->vertex[i * 3 + 1] = vertex[ref[poly + i] * 3 + 1];
		param->vertex[i * 3 + 2] = vertex[ref[poly + i] * 3 + 2];
		printf("A param->vertex %f %f %f\n", param->vertex[i * 3 + 0], param->vertex[i * 3 + 1], param->vertex[i * 3 + 2]);
		param->ref[i]  = ref[poly + i];
		param->crease[i] = crease[i];
	}
	param->v_count = i;
	param->p_count = 4;
	for(i = 0; i < polygon_count; i += 4)
	{
		if(ref[i] < vertex_count && select[ref[i]] && i != poly)
		{
			for(j = 0 ; j < 4; j++)
			{
				if(j == 3 && ref[i + j] >= vertex_count)
					k = -1;
				else
				{
					for(k = 0; param->vertex_id[k] != ref[i + j] && k < param->v_count; k++);
					if(k == param->v_count)
					{
						param->vertex_id[param->v_count] = ref[i + j];
						param->vertex[param->v_count * 3 + 0] = vertex[ref[i + j] * 3 + 0];
						param->vertex[param->v_count * 3 + 1] = vertex[ref[i + j] * 3 + 1];
						param->vertex[param->v_count * 3 + 2] = vertex[ref[i + j] * 3 + 2];
						param->v_count++;
					}
				}
				param->ref[param->p_count] = k;
				param->crease[param->p_count++] = crease[i + j];
			}
		}
	}
	create_poly_vectors(param->origo, param->vectors, &ref[poly], param->poly);
	param->p_count /= 4;
}

/*
udg_vertex_set(uint32 id, double *state, double x, double y, double z);
extern void udg_vertex_move(uint32 id, double x, double y, double z);
extern void udg_vertex_delete(uint32 id);
extern void udg_get_vertex_pos(double *pos, uint vertex_id);

*/

void copy_marked_mech(LATDeployParam *param, uint poly)
{
    uint32 *ref, i, vertex_count, rot;
	double vertex[3], origo[3], matrix[16], vectors[12];
	udg_get_geometry(&vertex_count, NULL, NULL, &ref, NULL);
	create_poly_vectors(origo, vectors, &ref[poly * 4], param->poly);
	rot = select_poly_rotate(matrix, param->vectors, param->origo, vectors, origo, param->poly);
	for(i = 0; i < param->poly; i++)
		param->vertex_id[param->poly - i - 1] = ref[poly * 4 + ((i + rot) % param->poly)];
	udg_get_vertex_pos(vertex, ref[poly * 4]);
	vertex[0] -= param->vertex[0];
	vertex[1] -= param->vertex[1];
	vertex[2] -= param->vertex[2];
	for(; i < param->v_count; i++)
	{
		param->vertex_id[i] = udg_find_empty_slot_vertex();
		vertex[0] = param->vertex[i * 3] - param->origo[0];
		vertex[1] = param->vertex[i * 3 + 1] - param->origo[1];
		vertex[2] = param->vertex[i * 3 + 2] - param->origo[2];
		f_transform3d(vertex, matrix, vertex[0], vertex[1], vertex[2]);
		udg_vertex_set(param->vertex_id[i], NULL, vertex[0] + origo[0], vertex[1] + origo[1], vertex[2] + origo[2]);
	}
	for(i = 1; i < param->p_count; i++)
	{
		if(param->ref[i * 4 + 3] < param->v_count)
			udg_polygon_set(poly, param->vertex_id[param->ref[i * 4]], param->vertex_id[param->ref[i * 4 + 1]], param->vertex_id[param->ref[i * 4 + 2]], param->vertex_id[param->ref[i * 4 + 3]]);
		else
			udg_polygon_set(poly, param->vertex_id[param->ref[i * 4]], param->vertex_id[param->ref[i * 4 + 1]], param->vertex_id[param->ref[i * 4 + 2]], -1);
		udg_crease_set(poly, param->crease[i * 4], param->crease[i * 4 + 1], param->crease[i * 4 + 2], param->crease[i * 4 + 3]);
		poly = udg_find_empty_slot_polygon();
	}
}

void la_t_deploy(uint poly)
{
	LATDeployParam param;
    uint32 vertex_count, polygon_count, *ref, i, j;
	grabb_marked_mesh(&param, poly);
    udg_get_geometry(&vertex_count, &polygon_count, NULL, &ref, NULL);
	for(i = 0; i < polygon_count; i++)
	{
		if(ref[i * 4] < vertex_count)
		{
			for(j = 0; j < param.poly && ref[i * 4 + j] < vertex_count && udg_get_select(ref[i * 4 + j]) > 0.01; j++);
			if(j == param.poly)
				copy_marked_mech(&param, i);
		}
	}
}
