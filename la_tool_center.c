#include "la_includes.h"
#include "la_tool.h"
#include "la_geometry_undo.h"

double high_x, low_x, high_y, low_y, high_z, low_z;

void scale_test_vertex(double *vertex)
{
	if(high_x == E_REAL_MAX)
	{
		high_x = vertex[0];
		low_x = vertex[0];
		high_y = vertex[1];
		low_y = vertex[1];
		high_z = vertex[2];
		low_z = vertex[2];
	}else
	{
		if(vertex[0] > high_x)
			high_x = vertex[0];
		if(vertex[0] < low_x)
			low_x = vertex[0];
		if(vertex[1] > high_y)
			high_y = vertex[1];
		if(vertex[1] < low_y)
			low_y = vertex[1];
		if(vertex[2] > high_z)
			high_z = vertex[2];
		if(vertex[2] < low_z)
			low_z = vertex[2];
	}
}

void la_t_center_geometry(void)
{
	uint32 i, vertex_count, polygon_count, *ref;
	double *vertex, size, center[3];
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, NULL);
	high_x = E_REAL_MAX;
	for(i = 0; i < polygon_count * 4 ; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
		{
			if(udg_get_select(ref[i]) > 0.01)
				scale_test_vertex(&vertex[ref[i] * 3]);
			if(udg_get_select(ref[i + 1]) > 0.01)
				scale_test_vertex(&vertex[ref[i + 1] * 3]);
			if(udg_get_select(ref[i + 2]) > 0.01)
				scale_test_vertex(&vertex[ref[i + 2] * 3]);
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				if(udg_get_select(ref[i + 3]) > 0.01)
					scale_test_vertex(&vertex[ref[i + 2] * 3]);
		}
	}
	if(high_x == E_REAL_MAX)
		return;
	size = high_x - low_x;
	if(high_y - low_y > size)
		size = high_y - low_y;
	if(high_z - low_z > size)
		size = high_z - low_z;

	if(size < 0.000001)
		return;
	center[0] = low_x + (high_x - low_x) * 0.5;
	center[1] = low_y + (high_y - low_y) * 0.5;
	center[2] = low_z + (high_z - low_z) * 0.5;
	for(i = 0; i < vertex_count; i++)
		if(vertex[i * 3] != E_REAL_MAX)
			if(udg_get_select(i) > 0.01)
				udg_vertex_set(i, &vertex[i * 3], (vertex[i * 3] - center[0]) / size, (vertex[i * 3 + 1] - center[1]) / size, (vertex[i * 3 + 2] - center[2]) / size);
	undo_event_done();
}

void la_t_center_manipulator(void)
{	
	uint32 i, vertex_count, polygon_count, *ref;
	double *vertex, size, center[3];
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, NULL);
	high_x = E_REAL_MAX;
	for(i = 0; i < polygon_count * 4 ; i += 4)
	{
		if(ref[i] < vertex_count && ref[i + 1] < vertex_count &&  ref[i + 2] < vertex_count && vertex[ref[i] * 3] != E_REAL_MAX && vertex[ref[i + 1] * 3] != E_REAL_MAX && vertex[ref[i + 2] * 3] != E_REAL_MAX)
		{
			if(udg_get_select(ref[i]) > 0.01)
				scale_test_vertex(&vertex[ref[i] * 3]);
			if(udg_get_select(ref[i + 1]) > 0.01)
				scale_test_vertex(&vertex[ref[i + 1] * 3]);
			if(udg_get_select(ref[i + 2]) > 0.01)
				scale_test_vertex(&vertex[ref[i + 2] * 3]);
			if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != E_REAL_MAX)
				if(udg_get_select(ref[i + 3]) > 0.01)
					scale_test_vertex(&vertex[ref[i + 3] * 3]);
		}
	}
	if(high_x == E_REAL_MAX)
		return;
	size = high_x - low_x;
	if(high_y - low_y > size)
		size = high_y - low_y;
	if(high_z - low_z > size)
		size = high_z - low_z;

	center[0] = low_x + (high_x - low_x) * 0.5;
	center[1] = low_y + (high_y - low_y) * 0.5;
	center[2] = low_z + (high_z - low_z) * 0.5;

	la_t_tm_place(center[0], center[1], center[2]);
}
