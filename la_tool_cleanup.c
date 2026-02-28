#include "la_includes.h"
#include "la_geometry_undo.h"

void la_t_model_vertex_delete()
{
	double *vertex;
	uint32 vertex_count, polygon_count, *ref, *crease, *tmp_buffer, i, j, count;
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, &crease);

	tmp_buffer = malloc((sizeof *tmp_buffer) * vertex_count);
	for(i = 0; i < vertex_count; i++)
		tmp_buffer[i] = 0;
	count = 0;
	for(i = 0; i < polygon_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
		{
			tmp_buffer[ref[i]]++;
			tmp_buffer[ref[i + 1]]++;
			tmp_buffer[ref[i + 2]]++;
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				tmp_buffer[ref[i + 3]]++;
		}	
	}
	for(i = 0; i < vertex_count; i++)
		if(tmp_buffer[i] == 0 && udg_get_select(i) > 0.01)
			udg_vertex_delete(i);
}

void la_t_model_cleanup()
{
	double *vertex;
	uint32 vertex_count, polygon_count, *ref, *crease, *tmp_buffer, i, j, count;
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, &crease);

	tmp_buffer = malloc((sizeof *tmp_buffer) * vertex_count);
	for(i = 0; i < vertex_count; i++)
		tmp_buffer[i] = 0;
	j = 0;
	for(i = 0; i < vertex_count; i++)
	{
		if(vertex[i * 3] != E_REAL_MAX)
		{
			tmp_buffer[i] = j++;	
			udg_vertex_set(tmp_buffer[i], NULL, vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);
			udg_set_select(tmp_buffer[i], udg_get_select(i));
		}else
			tmp_buffer[i] = -1;
	}
	for(i = j; i < vertex_count; i++)
		if(vertex[i * 3] != E_REAL_MAX)
			udg_vertex_delete(i);

	j = 0;
	for(i = 0; i < polygon_count * 4; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
		{
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				udg_polygon_set(j++, tmp_buffer[ref[i + 0]], tmp_buffer[ref[i + 1]], tmp_buffer[ref[i + 2]], tmp_buffer[ref[i + 3]]);
			else
				udg_polygon_set(j++, tmp_buffer[ref[i + 0]], tmp_buffer[ref[i + 1]], tmp_buffer[ref[i + 2]], -1);
		}	
	}
	for(i = j * 4; i < polygon_count * 4; i += 4)
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
			udg_polygon_delete(i / 4);
/*extern egreal	*udg_get_base_layer(void);

extern void		udg_vertex_set(uint32 id, double *state, double x, double y, double z);
extern void		udg_vertex_move(uint32 id, double x, double y, double z);
extern void		udg_vertex_delete(uint32 id);
extern void		udg_get_vertex_pos(double *pos, uint vertex_id);
extern void		udg_polygon_set(uint32 id, uint32 a, uint32 b, uint32 c, uint32 d);
extern void		udg_polygon_delete(uint32 id);
extern void		udg_crease_set(uint32 id, uint32 a, uint32 b, uint32 c, uint32 d);*/
}
