#include "la_includes.h"
#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_draw_overlay.h"
#include "la_tool.h"
#include "la_particle_fx.h"

#define PI  3.141592653

#define ROTATE_GRID_SPLITS  32
#define ROTATE_GRID_DEGREES  36

typedef enum{
	TMM_IDLE,
	TMM_PLACE,
	TMM_TRANSFORM,
	TMM_ROTATE,
	TMM_SCALE,
	TMM_NORMAL,
	TMM_TANGENT,
	TMM_SMOOTH
}TManipMode;

struct{
	float		*manipulator_array;
	float		*manipulator_normal;
	float		*manipulator_color;
	float		*manipulator_circle;
	float		*manipulator_grid;
	float		*manipulator_normal_array;
	uint		manipulator_normal_array_length;
	float		*manipulator_rings;
	float		*manipulator_rings_normals;
	float		*manipulator_rings_color;
	float		*manipulator_rings_shadow;
	double		manipulator_pos[3];
	double		manipulator_start[3];
	double		manipulator_size;
	float		manipulator_scale[3];
	double		manipulator_rotate_scale;
	SeduceLineObject *line_object;
	TManipMode	mode;
	double		grab_pos;
	double		grab_manip_pos;
	uint		grab_axis;
	double		*data;
	double		*normal;
	uint		data_length;
	double		*tags;
	uint		tag_length;
	boolean		hide;
	double		min_scale[3];
	double		max_scale[3];
	uint		update_rotate;
	float		matrix[16];
	boolean		accelerated_mode;
}GlobalTransformManipulator;

void matrix_rotate_x(double *a, double degree)
{
	double temp;

	temp = a[4];
	a[4] = (cos(degree) * a[4]) + (sin(degree) * a[8]);
	a[8] = (-sin(degree) * temp) + (cos(degree) * a[8]);

	temp = a[5];
	a[5] = (cos(degree) * a[5]) + (sin(degree) * a[9]);
	a[9] = (-sin(degree) * temp) + (cos(degree) * a[9]);

	temp = a[6];
	a[6] = (cos(degree) * a[6]) + (sin(degree) * a[10]);
	a[10] = (-sin(degree) * temp) + (cos(degree) * a[10]);

	temp = a[7];
	a[7] = (cos(degree) * a[7]) + (sin(degree) * a[11]);
	a[11] = (-sin(degree) * temp) + (cos(degree) * a[11]);
}

void matrix_rotate_y(double *a, double degree)
{
	double temp;

	temp = a[0];
	a[0] = (cos(degree) * a[0]) + (-sin(degree) * a[8]);
	a[8] = (sin(degree) * temp) + (cos(degree) * a[8]);

	temp = a[1];
	a[1] = (cos(degree) * a[1]) + (-sin(degree) * a[9]);
	a[9] = (sin(degree) * temp) + (cos(degree) * a[9]);

	temp = a[2];
	a[2] = (cos(degree) * a[2]) + (-sin(degree) * a[10]);
	a[10] = (sin(degree) * temp) + (cos(degree) * a[10]);

	temp = a[3];
	a[3] = (cos(degree) * a[3]) + (-sin(degree) * a[11]);
	a[11] = (sin(degree) * temp) + (cos(degree) * a[11]);

}

void matrix_rotate_z(double *a, double degree)
{
	double temp;

	temp = a[0];
	a[0] = (cos(degree) * a[0]) + (sin(degree) * a[4]);
	a[4] = (-sin(degree) * temp) + (cos(degree) * a[4]);

	temp = a[1];
	a[1] = (cos(degree) * a[1]) + (sin(degree) * a[5]);
	a[5] = (-sin(degree) * temp) + (cos(degree) * a[5]);

	temp = a[2];
	a[2] = (cos(degree) * a[2]) + (sin(degree) * a[6]);
	a[6] = (-sin(degree) * temp) + (cos(degree) * a[6]);

	temp = a[3];
	a[3] = (cos(degree) * a[3]) + (sin(degree) * a[7]);
	a[7] = (-sin(degree) * temp) + (cos(degree) * a[7]);
}

float *la_t_tm_matrix_get(void)
{
	return GlobalTransformManipulator.matrix;
}

void la_t_tm_init(void)
{
	uint i, j, k;
	float square[8];
	GlobalTransformManipulator.manipulator_normal_array = NULL;
	GlobalTransformManipulator.manipulator_normal_array_length = 0;
	GlobalTransformManipulator.manipulator_circle = malloc((sizeof *GlobalTransformManipulator.manipulator_circle) * (ROTATE_GRID_DEGREES * 6 + 24) * 3);
	GlobalTransformManipulator.manipulator_grid = malloc((sizeof *GlobalTransformManipulator.manipulator_grid) * (ROTATE_GRID_DEGREES * 4 + ROTATE_GRID_SPLITS * 4) * 3);
	GlobalTransformManipulator.accelerated_mode = FALSE;
	for(i = 0; i < 16; i++)
	{
		if(i % 5 == 0)
			GlobalTransformManipulator.matrix[i] = 1;
		else
			GlobalTransformManipulator.matrix[i] = 0;
	}
	k = 0;
	for(i = 0; i < ROTATE_GRID_DEGREES; i++)
	{
/*		r_set_vec3(GlobalTransformManipulator.manipulator_grid, k++, 0.96 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES + 0.01), 0.96 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES + 0.01), -0.002);
		r_set_vec3(GlobalTransformManipulator.manipulator_grid, k++, 0.99 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES + 0.006), 0.99 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES + 0.006), -0.018);
		r_set_vec3(GlobalTransformManipulator.manipulator_grid, k++, 0.99 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES - 0.006), 0.99 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES - 0.006), -0.018);
		r_set_vec3(GlobalTransformManipulator.manipulator_grid, k++, 0.96 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES - 0.01), 0.96 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES - 0.01), -0.002);


		r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 0, 1 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES), 1 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES), -0.01);
		r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 1, 1 * sin(2 * PI * (float)(i + 1) / ROTATE_GRID_DEGREES), 1 * cos(2 * PI * (float)(i + 1) / ROTATE_GRID_DEGREES), -0.01);
		r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 2, 0.95 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES), 0.95 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES), -0.0);
		r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 3, 0.95 * sin(2 * PI * (float)(i + 1) / ROTATE_GRID_DEGREES), 0.95 * cos(2 * PI * (float)(i + 1) / ROTATE_GRID_DEGREES), -0.0);
		r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 4, 0.9 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES), 0.9 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES), -0.015);
		r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 5, 0.9 * sin(2 * PI * (float)(i + 1) / ROTATE_GRID_DEGREES), 0.9 * cos(2 * PI * (float)(i + 1) / ROTATE_GRID_DEGREES), -0.015);
*/	}

/*	r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 0, 0, 0, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 1, 0, 0, 0.01);
	r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 2, 0, 0, -0.01);
	r_set_vec3(GlobalTransformManipulator.manipulator_circle, i * 6 + 3, 0, 0, -1);
	*
	for(i = 0; i < ROTATE_GRID_SPLITS; i++)
	{
		r_set_vec3(GlobalTransformManipulator.manipulator_grid, k++, 0.94 * sin(2 * PI * (float)i / ROTATE_GRID_SPLITS + 0.02), 0.94 * cos(2 * PI * (float)i / ROTATE_GRID_SPLITS + 0.02), -0.002);
		r_set_vec3(GlobalTransformManipulator.manipulator_grid, k++, 0.94 * sin(2 * PI * (float)i / ROTATE_GRID_SPLITS - 0.02), 0.94 * cos(2 * PI * (float)i / ROTATE_GRID_SPLITS - 0.02), -0.002);
		r_set_vec3(GlobalTransformManipulator.manipulator_grid, k++, 0.91 * sin(2 * PI * (float)i / ROTATE_GRID_SPLITS - 0.01), 0.91 * cos(2 * PI * (float)i / ROTATE_GRID_SPLITS - 0.01), -0.028);
		r_set_vec3(GlobalTransformManipulator.manipulator_grid, k++, 0.91 * sin(2 * PI * (float)i / ROTATE_GRID_SPLITS + 0.01), 0.91 * cos(2 * PI * (float)i / ROTATE_GRID_SPLITS + 0.01), -0.028);
	}*/
	k = 0;
	GlobalTransformManipulator.manipulator_array = malloc((sizeof *GlobalTransformManipulator.manipulator_array) * (12 + 24 * 6) * 3);
	GlobalTransformManipulator.manipulator_normal = malloc((sizeof *GlobalTransformManipulator.manipulator_normal) * (12 + 24 * 6) * 3);
	/*
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 1, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.3, 0, 0.01);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 1, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.1, 0, 0.014);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 1, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.1, 0.014, 0);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 1, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.3, 0.01, 0);

	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 0, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0, 0.1, 0.014);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 0, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0, 0.3, 0.01);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 0, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.01, 0.3, 0);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 0, 1);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.014, 0.1, 0);

	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 12, 0);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.014, 0, 0.1);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 1, 0);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.01, 0, 0.3);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 1, 0);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0, 0.01, 0.3);
	r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 1, 0);
	r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0, 0.014, 0.1);

	for(i = 1; i < 13; i++)
	{
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 0, 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0, 0.3 * sin(0.5 * PI * (float)i / 14), 0.3 * cos(0.5 * PI * (float)i / 14));
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 0, 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0, 0.3 * sin(0.5 * PI * (float)(i + 1) / 14), 0.3 * cos(0.5 * PI * (float)(i + 1) / 14));
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 0, 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0, 0.29 * sin(0.5 * PI * (float)(i + 1) / 14), 0.29 * cos(0.5 * PI * (float)(i + 1) / 14));
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 1, 0, 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0, 0.29 * sin(0.5 * PI * (float)i / 14), 0.29 * cos(0.5 * PI * (float)i / 14));

		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 1, 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.3 * sin(0.5 * PI * (float)i / 14), 0, 0.3 * cos(0.5 * PI * (float)i / 14));
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 1, 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.3 * sin(0.5 * PI * (float)(i + 1) / 14), 0, 0.3 * cos(0.5 * PI * (float)(i + 1) / 14));
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 1, 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.29 * sin(0.5 * PI * (float)(i + 1) / 14), 0, 0.29 * cos(0.5 * PI * (float)(i + 1) / 14));
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 1, 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.29 * sin(0.5 * PI * (float)i / 14), 0, 0.29 * cos(0.5 * PI * (float)i / 14));

		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 0, 1);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.3 * sin(0.5 * PI * (float)i / 14), 0.3 * cos(0.5 * PI * (float)i / 14), 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 0, 1);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.3 * sin(0.5 * PI * (float)(i + 1) / 14), 0.3 * cos(0.5 * PI * (float)(i + 1) / 14), 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 0, 1);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.29 * sin(0.5 * PI * (float)(i + 1) / 14), 0.29 * cos(0.5 * PI * (float)(i + 1) / 14), 0);
		r_set_vec3(GlobalTransformManipulator.manipulator_normal, k, 0, 0, 1);
		r_set_vec3(GlobalTransformManipulator.manipulator_array, k++, 0.29 * sin(0.5 * PI * (float)i / 14), 0.29 * cos(0.5 * PI * (float)i / 14), 0);	
	}
	*/
	GlobalTransformManipulator.manipulator_rings = malloc((sizeof *GlobalTransformManipulator.manipulator_array) * 4 * (ROTATE_GRID_DEGREES + 6) * 2);
	GlobalTransformManipulator.manipulator_rings_normals =  malloc((sizeof *GlobalTransformManipulator.manipulator_array) * 4 * (ROTATE_GRID_DEGREES + 6) * 3);
	GlobalTransformManipulator.manipulator_rings_shadow = malloc((sizeof *GlobalTransformManipulator.manipulator_array) *  16 * (ROTATE_GRID_DEGREES + 6) * 2);
	GlobalTransformManipulator.manipulator_rings_color = malloc((sizeof *GlobalTransformManipulator.manipulator_array) * 16 * (ROTATE_GRID_DEGREES + 6) * 4);
	for(i = 0; i < ROTATE_GRID_DEGREES; i++)
	{
		square[0] =  0.35 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES);
		square[1] = 0.35 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES);
		square[2] = 0.33 * sin(2 * PI * (float)i / ROTATE_GRID_DEGREES);
		square[3] = 0.33 * cos(2 * PI * (float)i / ROTATE_GRID_DEGREES);
		square[4] = 0.33 * sin(2 * PI * ((float)i + 0.5) / ROTATE_GRID_DEGREES);
		square[5] = 0.33 * cos(2 * PI * ((float)i + 0.5) / ROTATE_GRID_DEGREES);
		square[6] = 0.35 * sin(2 * PI * ((float)i + 0.5) / ROTATE_GRID_DEGREES);
		square[7] = 0.35 * cos(2 * PI * ((float)i + 0.5) / ROTATE_GRID_DEGREES);

/*		r_set_vec2(GlobalTransformManipulator.manipulator_rings, i * 4 + 0, square[0], square[1]);
		r_set_vec3(GlobalTransformManipulator.manipulator_rings_normals, i * 4 + 0, square[0], square[1], -5);
		r_set_vec2(GlobalTransformManipulator.manipulator_rings, i * 4 + 1, square[2], square[3]);
		r_set_vec3(GlobalTransformManipulator.manipulator_rings_normals, i * 4 + 1, square[2], square[3], -5);
		r_set_vec2(GlobalTransformManipulator.manipulator_rings, i * 4 + 2, square[4], square[5]);
		r_set_vec3(GlobalTransformManipulator.manipulator_rings_normals, i * 4 + 2, square[4], square[5], -5);
		r_set_vec2(GlobalTransformManipulator.manipulator_rings, i * 4 + 3, square[6], square[7]);
		r_set_vec3(GlobalTransformManipulator.manipulator_rings_normals, i * 4 + 3, square[6], square[7], -5);
		la_create_shadow_edge(-0.02, 4, &GlobalTransformManipulator.manipulator_rings_shadow[16 * i * 2], &GlobalTransformManipulator.manipulator_rings_color[16 * i * 4], square);
*/	}

	for(; i < ROTATE_GRID_DEGREES + 6; i++)
	{
		for(j = 0; j < 2; j++)
		{
			square[0] = -0.4 * cos(2 * PI * ((float)i - 2.4 + (float)j * 0.8) / 18);
			square[1] = 0.4 * sin(2 * PI * ((float)i - 2.4 + (float)j * 0.8) / 18);
			square[2] = -0.5 * cos(2 * PI * ((float)i - 2.2 + (float)j * 0.4) / 18);
			square[3] = 0.5 * sin(2 * PI * ((float)i - 2.2 + (float)j * 0.4) / 18);
			square[4] = -0.5 * cos(2 * PI * ((float)i - 2.1 + (float)j * 0.2) / 18);
			square[5] = 0.5 * sin(2 * PI * ((float)i - 2.1 + (float)j * 0.2) / 18);
			square[6] = -0.4 * cos(2 * PI * ((float)i - 2.2 + (float)j * 0.4) / 18);
			square[7] = 0.4 * sin(2 * PI * ((float)i - 2.2 + (float)j * 0.4) / 18);
			i += j; 
	/*		r_set_vec2(GlobalTransformManipulator.manipulator_rings, i * 4 + 0, square[0], square[1]);
			r_set_vec3(GlobalTransformManipulator.manipulator_rings_normals, i * 4 + 0, square[0], square[1], -5);
			r_set_vec2(GlobalTransformManipulator.manipulator_rings, i * 4 + 1, square[2], square[3]);
			r_set_vec3(GlobalTransformManipulator.manipulator_rings_normals, i * 4 + 1, square[2], square[3], -5);
			r_set_vec2(GlobalTransformManipulator.manipulator_rings, i * 4 + 2, square[4], square[5]);
			r_set_vec3(GlobalTransformManipulator.manipulator_rings_normals, i * 4 + 2, square[4], square[5], -5);
			r_set_vec2(GlobalTransformManipulator.manipulator_rings, i * 4 + 3, square[6], square[7]);
			r_set_vec3(GlobalTransformManipulator.manipulator_rings_normals, i * 4 + 3, square[6], square[7], -5);
			la_create_shadow_edge(0.02 - (float)j * 0.04, 4, &GlobalTransformManipulator.manipulator_rings_shadow[16 * i * 2], &GlobalTransformManipulator.manipulator_rings_color[16 * i * 4], square);
	*/	}
	}


	GlobalTransformManipulator.manipulator_pos[0] = 0.1;
	GlobalTransformManipulator.manipulator_pos[1] = 0.1;
	GlobalTransformManipulator.manipulator_pos[2] = 0.1;
	GlobalTransformManipulator.manipulator_size = 1;
	GlobalTransformManipulator.manipulator_rotate_scale = 1;
	GlobalTransformManipulator.mode = TMM_IDLE;
	GlobalTransformManipulator.data = NULL;
	GlobalTransformManipulator.hide = TRUE;
	GlobalTransformManipulator.tags = NULL;
	GlobalTransformManipulator.tag_length = 0;
}

void la_t_tm_place(double x, double y, double z)
{
	GlobalTransformManipulator.manipulator_pos[0] = x;
	GlobalTransformManipulator.manipulator_pos[1] = y;
	GlobalTransformManipulator.manipulator_pos[2] = z;
}
void la_t_tm_get_pos(double *pos)
{
	pos[0] = GlobalTransformManipulator.manipulator_pos[0];
	pos[1] = GlobalTransformManipulator.manipulator_pos[1];
	pos[2] = GlobalTransformManipulator.manipulator_pos[2];
}
void la_t_tm_get_vector(double *vector)
{
	double r;
	uint i;
	seduce_view_camera_getd(NULL, vector);
	vector[0] -= GlobalTransformManipulator.manipulator_pos[0];
	vector[1] -= GlobalTransformManipulator.manipulator_pos[1];
	vector[2] -= GlobalTransformManipulator.manipulator_pos[2];
	r = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
	vector[0] = (double)((uint)(vector[0] * 1.6 / r));
	vector[1] = (double)((uint)(vector[1] * 1.6 / r));
	vector[2] = (double)((uint)(vector[2] * 1.6 / r));
	f_normalize3d(vector);
}

void la_t_tm_hide(boolean hide)
{
	GlobalTransformManipulator.hide = hide;
//	GlobalTransformManipulator.hide = FALSE;
}

boolean la_t_tm_hiden()
{
	return GlobalTransformManipulator.hide;
}


void compute_tangent(double *vertex, uint edge_a, uint edge_b, uint other)
{
	float r;
	double normal[3], temp[3];
	edge_a *= 3;
	edge_b *= 3;
	other *= 3;
	normal[0] = vertex[edge_a + 0] - vertex[other + 0];
	normal[1] = vertex[edge_a + 1] - vertex[other + 1];
	normal[2] = vertex[edge_a + 2] - vertex[other + 2];
	temp[0] = vertex[edge_a + 0] - vertex[edge_b + 0];
	temp[1] = vertex[edge_a + 1] - vertex[edge_b + 1];
	temp[2] = vertex[edge_a + 2] - vertex[edge_b + 2];
	f_normalize3d(temp);
	r = normal[0] * temp[0] + normal[1] * temp[1] + normal[2] * temp[2];
	normal[0] -= temp[0] * r;
	normal[1] -= temp[1] * r;
	normal[2] -= temp[2] * r;
	f_normalize3d(normal);
	GlobalTransformManipulator.normal[edge_a] += normal[0];
	GlobalTransformManipulator.normal[edge_a + 1] += normal[1];
	GlobalTransformManipulator.normal[edge_a + 2] += normal[2];
	r = GlobalTransformManipulator.normal[edge_a] * normal[0] +
		GlobalTransformManipulator.normal[edge_a + 1] * normal[1] +
		GlobalTransformManipulator.normal[edge_a + 2] * normal[2];
	GlobalTransformManipulator.normal[edge_a] /= r;
	GlobalTransformManipulator.normal[edge_a + 1] /= r;
	GlobalTransformManipulator.normal[edge_a + 2] /= r;
	GlobalTransformManipulator.normal[edge_b] += normal[0];
	GlobalTransformManipulator.normal[edge_b + 1] += normal[1];
	GlobalTransformManipulator.normal[edge_b + 2] += normal[2];
	r = GlobalTransformManipulator.normal[edge_b] * normal[0] +
		GlobalTransformManipulator.normal[edge_b + 1] * normal[1] +
		GlobalTransformManipulator.normal[edge_b + 2] * normal[2];
	GlobalTransformManipulator.normal[edge_b] /= r;
	GlobalTransformManipulator.normal[edge_b + 1] /= r;
	GlobalTransformManipulator.normal[edge_b + 2] /= r;
}


void lock_transform_vertexes(BInputState *input, boolean normal, boolean tangent, boolean smooth)
{
	ENode *node;
	UNDOTag *tag;
	uint i, j, *ref, ref_count, vertex_count, count;
	double select, *vertex, pos[3];
	boolean computed;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);
	GlobalTransformManipulator.data_length = vertex_count;
	GlobalTransformManipulator.data = malloc((sizeof *GlobalTransformManipulator.data) * GlobalTransformManipulator.data_length * 3);

	pos[0] = la_axis_matrix[0] * GlobalTransformManipulator.manipulator_pos[0] + la_axis_matrix[1] * GlobalTransformManipulator.manipulator_pos[1] + la_axis_matrix[2] * GlobalTransformManipulator.manipulator_pos[2];
	pos[1] = la_axis_matrix[4] * GlobalTransformManipulator.manipulator_pos[0] + la_axis_matrix[5] * GlobalTransformManipulator.manipulator_pos[1] + la_axis_matrix[6] * GlobalTransformManipulator.manipulator_pos[2];
	pos[2] = la_axis_matrix[8] * GlobalTransformManipulator.manipulator_pos[0] + la_axis_matrix[9] * GlobalTransformManipulator.manipulator_pos[1] + la_axis_matrix[10] * GlobalTransformManipulator.manipulator_pos[2];

	for(i = 0; i < 3; i++)
	{
		GlobalTransformManipulator.min_scale[i] = pos[i];
		GlobalTransformManipulator.max_scale[i] = pos[i];
	}
	for(i = 0; i < GlobalTransformManipulator.data_length; i++)
	{
		GlobalTransformManipulator.data[i * 3] = vertex[i * 3];
		GlobalTransformManipulator.data[i * 3 + 1] = vertex[i * 3 + 1];
		GlobalTransformManipulator.data[i * 3 + 2] = vertex[i * 3 + 2];
		select = udg_get_select(i);
		if(select > 0.0001)
		{
			pos[0] = la_axis_matrix[0] * vertex[i * 3] + la_axis_matrix[1] * vertex[i * 3 + 1] + la_axis_matrix[2] * vertex[i * 3 + 2];
			pos[1] = la_axis_matrix[4] * vertex[i * 3] + la_axis_matrix[5] * vertex[i * 3 + 1] + la_axis_matrix[6] * vertex[i * 3 + 2];
			pos[2] = la_axis_matrix[8] * vertex[i * 3] + la_axis_matrix[9] * vertex[i * 3 + 1] + la_axis_matrix[10] * vertex[i * 3 + 2];

			if(pos[0] > GlobalTransformManipulator.max_scale[0])
				GlobalTransformManipulator.max_scale[0] = pos[0];
			if(pos[0] < GlobalTransformManipulator.min_scale[0])
				GlobalTransformManipulator.min_scale[0] = pos[0];

			if(pos[1] > GlobalTransformManipulator.max_scale[1])
				GlobalTransformManipulator.max_scale[1] = pos[1];
			if(pos[1] < GlobalTransformManipulator.min_scale[1])
				GlobalTransformManipulator.min_scale[1] = pos[1];

			if(pos[2] > GlobalTransformManipulator.max_scale[2])
				GlobalTransformManipulator.max_scale[2] = pos[2];
			if(pos[2] < GlobalTransformManipulator.min_scale[2])
				GlobalTransformManipulator.min_scale[2] = pos[2];					
		}
	}
	GlobalTransformManipulator.normal = NULL;
	if(normal)
	{
		double x, y, z, x2, y2, z2, r;
		uint *pos, poly, j2;
		GlobalTransformManipulator.normal = malloc((sizeof *GlobalTransformManipulator.normal) * vertex_count * 3);
		for(i = 0; i < vertex_count; i++)
		{
			GlobalTransformManipulator.normal[i * 3] = 0;
			GlobalTransformManipulator.normal[i * 3 + 1] = 0;
			GlobalTransformManipulator.normal[i * 3 + 2] = 0;
		}
		pos = ref;
		for(i = 0; i < ref_count; pos += 4)
		{
			i++;
			if(pos[0] < vertex_count &&
			   pos[1] < vertex_count &&
			   pos[2] < vertex_count &&
			   vertex[pos[0] * 3] != V_REAL64_MAX &&
			   vertex[pos[1] * 3] != V_REAL64_MAX &&
			   vertex[pos[2] * 3] != V_REAL64_MAX)
			{
				if(pos[3] < vertex_count && vertex[pos[3] * 3] != V_REAL64_MAX)
					poly = 4;
				else
					poly = 3;
				if(udg_get_select(pos[0]) > 0.01 && udg_get_select(pos[1]) > 0.01 && udg_get_select(pos[2]) > 0.01 && (poly == 3 || udg_get_select(pos[3]) > 0.01))
				{
					x = ((vertex[pos[0] * 3 + 1] - vertex[pos[1] * 3 + 1]) * (vertex[pos[2] * 3 + 2] - vertex[pos[1] * 3 + 2]) - (vertex[pos[0] * 3 + 2] - vertex[pos[1] * 3 + 2]) * (vertex[pos[2] * 3 + 1] - vertex[pos[1] * 3 + 1]));
					y = ((vertex[pos[0] * 3 + 2] - vertex[pos[1] * 3 + 2]) * (vertex[pos[2] * 3 + 0] - vertex[pos[1] * 3 + 0]) - (vertex[pos[0] * 3 + 0] - vertex[pos[1] * 3 + 0]) * (vertex[pos[2] * 3 + 2] - vertex[pos[1] * 3 + 2]));
					z = ((vertex[pos[0] * 3 + 0] - vertex[pos[1] * 3 + 0]) * (vertex[pos[2] * 3 + 1] - vertex[pos[1] * 3 + 1]) - (vertex[pos[0] * 3 + 1] - vertex[pos[1] * 3 + 1]) * (vertex[pos[2] * 3 + 0] - vertex[pos[1] * 3 + 0]));
					r = sqrt(x * x + y * y + z * z);
					x = x / r;
					y = y / r;
					z = z / r;
					for(j = 0; j < poly; j++)
					{
						GlobalTransformManipulator.normal[pos[j] * 3] += x;
						GlobalTransformManipulator.normal[pos[j] * 3 + 1] += y;
						GlobalTransformManipulator.normal[pos[j] * 3 + 2] += z;
					}
				}
			}
		}
		pos = ref;
		for(i = 0; i < vertex_count; i++)
		{
			x = GlobalTransformManipulator.normal[i * 3];
			y = GlobalTransformManipulator.normal[i * 3 + 1];
			z = GlobalTransformManipulator.normal[i * 3 + 2];
			r = x * x + y * y + z * z;
			if(r > 0.001)
			{
				r = sqrt(r);
				GlobalTransformManipulator.normal[i * 3] = x / r;
				GlobalTransformManipulator.normal[i * 3 + 1] = y / r;
				GlobalTransformManipulator.normal[i * 3 + 2] = z / r;
			}
		}
	}
	if(tangent)
	{
		double x, y, z, x2, y2, z2, r;
		uint *pos, poly, j2;
		boolean *edge;
		GlobalTransformManipulator.normal = malloc((sizeof *GlobalTransformManipulator.normal) * vertex_count * 3);
		edge = malloc((sizeof *edge) * vertex_count);
		for(i = 0; i < vertex_count; i++)
		{
			GlobalTransformManipulator.normal[i * 3] = 0;
			GlobalTransformManipulator.normal[i * 3 + 1] = 0;
			GlobalTransformManipulator.normal[i * 3 + 2] = 0;
			edge[i] = FALSE;
		}
		pos = ref;
		for(i = 0; i < ref_count; i++)
		{
			if(pos[3] < vertex_count)
				if(udg_get_select(pos[0]) < 0.01 || udg_get_select(pos[1]) < 0.01 || udg_get_select(pos[2]) < 0.01 || (pos[3] < vertex_count && udg_get_select(pos[3]) < 0.01))
					for(j = 0; j < 4; j++)
						if(pos[j] < vertex_count && udg_get_select(pos[j]) > 0.01)
							edge[pos[j]] = TRUE;	
			pos += 4;
		}
		pos = ref;
		for(i = 0; i < ref_count; i++)
		{
			if(pos[3] < vertex_count)
			{
				if(udg_get_select(pos[0]) > 0.01 && udg_get_select(pos[1]) > 0.01 && udg_get_select(pos[2]) > 0.01 && (pos[3] > vertex_count || udg_get_select(pos[3]) > 0.01))
				{
					if(edge[pos[0]] && edge[pos[1]])
						compute_tangent(vertex, pos[0], pos[1], pos[2]);
					if(edge[pos[1]] && edge[pos[2]])
						compute_tangent(vertex, pos[1], pos[2], pos[0]);
					if(pos[3] >= vertex_count && edge[pos[2]] && edge[pos[0]])
						compute_tangent(vertex, pos[2], pos[0], pos[1]);
					if(pos[3] < vertex_count && edge[pos[2]] && edge[pos[3]])
						compute_tangent(vertex, pos[2], pos[3], pos[0]);
					if(pos[3] < vertex_count && edge[pos[3]] && edge[pos[0]])
						compute_tangent(vertex, pos[3], pos[0], pos[1]);
				}
			}
			pos += 4;
		}
		j = 0;
		for(i = 0; i < vertex_count; i++)
			if(GlobalTransformManipulator.normal[i * 3] != 0 || GlobalTransformManipulator.normal[i * 3 + 1] != 0 || GlobalTransformManipulator.normal[i * 3 + 2] != 0)
				j++;
/*		for(i = 0; i < vertex_count; i++)
		{
			printf("tangent output %f %f %f edge %u\n",GlobalTransformManipulator.normal[i * 3],
			GlobalTransformManipulator.normal[i * 3 + 1],
			GlobalTransformManipulator.normal[i * 3 + 2],
			edge[i]);
		}
*/		free(edge);
	}
	if(smooth)
	{
		uint *count, poly;
		double vertex[3], sum[3];
		GlobalTransformManipulator.normal = malloc((sizeof *GlobalTransformManipulator.normal) * vertex_count * 3);
		count = malloc((sizeof *count) * vertex_count);
		for(i = 0; i < vertex_count; i++)
		{
			GlobalTransformManipulator.normal[i * 3] = 0;
			GlobalTransformManipulator.normal[i * 3 + 1] = 0;
			GlobalTransformManipulator.normal[i * 3 + 2] = 0;
			count[i] = 0;
		}
		for(i = 0; i < ref_count * 4; i += 4)
		{
			if(ref[i] < vertex_count && ref[i + 1] < vertex_count && ref[i + 2] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && vertex[ref[i + 1] * 3] != V_REAL64_MAX && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
			{
				if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != V_REAL64_MAX)
					poly = 4;
				else
					poly = 3;
				sum[0] = 0;
				sum[1] = 0;
				sum[2] = 0;
				for(j = 0; j < poly; j++)
				{
					udg_get_vertex_pos(vertex, ref[i + j]);
					sum[0] += vertex[0];
					sum[1] += vertex[1];
					sum[2] += vertex[2];
				}
				for(j = 0; j < poly; j++)
				{
					udg_get_vertex_pos(vertex, ref[i + j]);
					GlobalTransformManipulator.normal[ref[i + j] * 3] += sum[0] - vertex[0];
					GlobalTransformManipulator.normal[ref[i + j] * 3 + 1] += sum[1] - vertex[1];
					GlobalTransformManipulator.normal[ref[i + j] * 3 + 2] += sum[2] - vertex[2];
					count[ref[i + j]] += poly - 1;
				}
			}
		}
		for(i = 0; i < vertex_count; i++)
		{
			if(count[i] != 0)
			{
				udg_get_vertex_pos(vertex, i);
				GlobalTransformManipulator.normal[i * 3] = (vertex[0] - (GlobalTransformManipulator.normal[i * 3] / count[i])) / (GlobalTransformManipulator.manipulator_size * 0.4);
				GlobalTransformManipulator.normal[i * 3 + 1] = (vertex[1] - (GlobalTransformManipulator.normal[i * 3 + 1] / count[i])) / (GlobalTransformManipulator.manipulator_size * 0.4);
				GlobalTransformManipulator.normal[i * 3 + 2] = (vertex[2] - (GlobalTransformManipulator.normal[i * 3 + 2] / count[i])) / (GlobalTransformManipulator.manipulator_size * 0.4);
			}
		}
		free(count);
	}
	if(tangent || normal || smooth)
	{
		GlobalTransformManipulator.line_object = seduce_primitive_line_object_allocate();
		for(i = 0; i < vertex_count; i++)
		{
			if(GlobalTransformManipulator.normal[i * 3] != 0 || GlobalTransformManipulator.normal[i * 3 + 1] != 0 || GlobalTransformManipulator.normal[i * 3 + 2] != 0)
			{
				if(udg_get_select(i) > 0.01)
				{
					seduce_primitive_line_add_3d(GlobalTransformManipulator.line_object,
							vertex[i * 3] - GlobalTransformManipulator.normal[i * 3] * GlobalTransformManipulator.manipulator_size * 0.4, vertex[i * 3 + 1] - GlobalTransformManipulator.normal[i * 3 + 1] * GlobalTransformManipulator.manipulator_size * 0.4, vertex[i * 3 + 2] - GlobalTransformManipulator.normal[i * 3 + 2] * GlobalTransformManipulator.manipulator_size * 0.4,
							vertex[i * 3] + GlobalTransformManipulator.normal[i * 3] * GlobalTransformManipulator.manipulator_size * 0.4, vertex[i * 3 + 1] + GlobalTransformManipulator.normal[i * 3 + 1] * GlobalTransformManipulator.manipulator_size * 0.4, vertex[i * 3 + 2] + GlobalTransformManipulator.normal[i * 3 + 2] * GlobalTransformManipulator.manipulator_size * 0.4,
							1, 1, 1, 1, 1, 1, 1, 1);
				}
			}
		}
	}else
	{
		tag = udg_get_tags(&GlobalTransformManipulator.tag_length);
		for(i = 0; i < GlobalTransformManipulator.tag_length && tag[i].select < 0.01; i++);
		if(i == GlobalTransformManipulator.tag_length)
			return;
		GlobalTransformManipulator.tags = malloc((sizeof *GlobalTransformManipulator.tags) * GlobalTransformManipulator.tag_length * 3);
		for(i = 0; i < GlobalTransformManipulator.tag_length; i++)
		{
			GlobalTransformManipulator.tags[i * 3] = tag[i].vec[0];
			GlobalTransformManipulator.tags[i * 3 + 1] = tag[i].vec[1];
			GlobalTransformManipulator.tags[i * 3 + 2] = tag[i].vec[2];
		}
	}
}

void la_t_tm_compute_accelerated_mode()
{
	uint vetex_count, ref_count, i, found;
	udg_get_geometry(&vetex_count, &ref_count, NULL, NULL, NULL);
	GlobalTransformManipulator.accelerated_mode = ref_count > imagine_setting_integer_get("SDS_LIMIT", 10000, "Maximum number of prolygons drawn befor SDS is turned off");
	if(!GlobalTransformManipulator.accelerated_mode)
	{
//		if(found > imagine_setting_integer_get("MANIPULATOR_LIMIT", 100, "Maximum number of selected vertices before anipulations are optimized"))
			GlobalTransformManipulator.accelerated_mode = TRUE;
	}
}

boolean la_t_tm_grab(BInputState *input)
{
	uint i;
/*	if(GlobalTransformManipulator.hide)
		return;*/
	static float size[3] = {1, 1, 1}, rotation_matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, m[16], manipulator_pos[3];
	float pos[3];
	double dpos[3];

	if(GlobalTransformManipulator.hide)
		return FALSE;
	
	r_matrix_projection_screenf(&la_world_matrix, pos, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);
	if(input->mode == BAM_DRAW)
	{
		float /*a[3] = {0, 0, 0},*/ b[3] = {0, 0, 0}, c[3] = {0, 0, 0};
		pos[2] = 0;
		for(i = 0; i < 16; i++)
		{
			b[0] = sin((double)i / 16.0 * 2.0 * PI) * 0.1 + pos[0];
			b[1] = cos((double)i / 16.0 * 2.0 * PI) * 0.1 + pos[1];
			c[0] = sin((double)(i + 1) / 16.0 * 2.0 * PI) * 0.1 + pos[0];
			c[1] = cos((double)(i + 1) / 16.0 * 2.0 * PI) * 0.1 + pos[1];
			seduce_element_add_triangle(input, &GlobalTransformManipulator.manipulator_pos[4], 0, pos, b, c);
		}
	}
	if(seduce_text_button(input, &GlobalTransformManipulator.manipulator_scale[0], pos[0] + 0.16, pos[1] + 0.1, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Normal", 0.6, 0.6, 0.6, 1.0, 0.2, 0.6, 1.0, 1.0))
	{
		GlobalTransformManipulator.mode = TMM_NORMAL;
		lock_transform_vertexes(input, TRUE, FALSE, FALSE);
		la_t_tm_compute_accelerated_mode();
		return TRUE;
	}
	if(seduce_text_button(input, &GlobalTransformManipulator.manipulator_scale[1], pos[0] + 0.17, pos[1], 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Tangent", 0.6, 0.6, 0.6, 1.0, 0.2, 0.6, 1.0, 1.0))
//	if(seduce_widget_button_invisible(input, &GlobalTransformManipulator.manipulator_scale[1], pos[0] + 0.18, pos[1], 0.01, TRUE))
	{
		GlobalTransformManipulator.mode = TMM_TANGENT;
		lock_transform_vertexes(input, FALSE, TRUE, FALSE);
		la_t_tm_compute_accelerated_mode();
		return TRUE;
	}
	if(seduce_text_button(input, &GlobalTransformManipulator.manipulator_scale[2], pos[0] + 0.16, pos[1] - 0.1, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Smooth", 0.6, 0.6, 0.6, 1.0, 0.2, 0.6, 1.0, 1.0))
//	if(seduce_widget_button_invisible(input, &GlobalTransformManipulator.manipulator_scale[2], pos[0] + 0.18, pos[1] - 0.1, 0.01, TRUE))
	{
		GlobalTransformManipulator.mode = TMM_SMOOTH;
		lock_transform_vertexes(input, FALSE, FALSE, TRUE);
		la_t_tm_compute_accelerated_mode();
		return TRUE;
	}
	
	for(i = 0; i < 16; i++)
		m[i] = la_axis_matrix[i];
	r_matrix_push(&la_world_matrix);
	r_matrix_matrix_mult(&la_world_matrix, m);
//	r_matrix_pop(&la_world_matrix);
//	return FALSE;

	f_transforminv3d(dpos, la_axis_matrix, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);  /* Transforms a 3D point with a 4x4 64 bit double matrix.*/
	manipulator_pos[0] = dpos[0];
	manipulator_pos[1] = dpos[1];
	manipulator_pos[2] = dpos[2];



	if(seduce_manipulator_pos_xyz(input, &la_world_matrix,
										manipulator_pos, 
										&GlobalTransformManipulator.manipulator_pos[0], 
										NULL, input->pointers[0].button[1],
										TRUE, TRUE, TRUE, 1.2, 1))
	{
		if(input->mode == BAM_EVENT)
		{
			GlobalTransformManipulator.mode = TMM_TRANSFORM;
			GlobalTransformManipulator.manipulator_start[0] = GlobalTransformManipulator.manipulator_pos[0];
			GlobalTransformManipulator.manipulator_start[1] = GlobalTransformManipulator.manipulator_pos[1];
			GlobalTransformManipulator.manipulator_start[2] = GlobalTransformManipulator.manipulator_pos[2];
			lock_transform_vertexes(input, FALSE, FALSE, FALSE);
			r_matrix_pop(&la_world_matrix);
			la_t_tm_compute_accelerated_mode();
			return TRUE;
		}
	}
	if(seduce_manipulator_point(input, &la_world_matrix,
										manipulator_pos, 
										&GlobalTransformManipulator.manipulator_pos[3], 1))
	{
		if(input->mode == BAM_EVENT)
		{
			GlobalTransformManipulator.mode = TMM_PLACE;
			GlobalTransformManipulator.manipulator_start[0] = GlobalTransformManipulator.manipulator_pos[0];
			GlobalTransformManipulator.manipulator_start[1] = GlobalTransformManipulator.manipulator_pos[1];
			GlobalTransformManipulator.manipulator_start[2] = GlobalTransformManipulator.manipulator_pos[2];
			r_matrix_pop(&la_world_matrix);
			la_t_tm_compute_accelerated_mode();
			return TRUE;
		}
	}

	if(seduce_manipulator_scale(input, &la_world_matrix, manipulator_pos, size, &GlobalTransformManipulator.manipulator_pos[1],
					NULL, input->pointers[0].button[1], TRUE, TRUE, TRUE, 1, 1))
	{
		if(input->mode == BAM_EVENT)
		{
			GlobalTransformManipulator.mode = TMM_SCALE;
			GlobalTransformManipulator.manipulator_scale[0] = 1;
			GlobalTransformManipulator.manipulator_scale[1] = 1;
			GlobalTransformManipulator.manipulator_scale[2] = 1;
			lock_transform_vertexes(input, FALSE, FALSE, FALSE);
			r_matrix_pop(&la_world_matrix);
			la_t_tm_compute_accelerated_mode();
			return TRUE;
		}
	}
	if(seduce_manipulator_rotate(input, &la_world_matrix, manipulator_pos, rotation_matrix, &GlobalTransformManipulator.manipulator_pos[2],
					NULL, input->pointers[0].button[1], 1, 1))
	{
		if(input->mode == BAM_EVENT)
		{
			GlobalTransformManipulator.mode = TMM_ROTATE;
			lock_transform_vertexes(input, FALSE, FALSE, FALSE);
			r_matrix_pop(&la_world_matrix);
			la_t_tm_compute_accelerated_mode();
			return TRUE;
		}
	}
	r_matrix_pop(&la_world_matrix);
	return FALSE;
}

void la_t_tm_view_center(void)
{
	seduce_view_center_set(NULL, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);
}

boolean la_t_tm_test_center(BInputState *input)
{
	double manip[3], pos;
	void *element;
	if(GlobalTransformManipulator.hide == TRUE)
		return FALSE;
	element = seduce_element_pointer_id(input, 0, NULL);
	return element == &GlobalTransformManipulator.manipulator_pos[0] ||
			element == &GlobalTransformManipulator.manipulator_pos[1] ||
			element == &GlobalTransformManipulator.manipulator_pos[2] ||
			element == &GlobalTransformManipulator.manipulator_pos[3] ||
			element == &GlobalTransformManipulator.manipulator_pos[4];


	seduce_view_projection_screend(NULL, manip, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);
	if((manip[0] + input->pointers[0].pointer_x) * (manip[0] + input->pointers[0].pointer_x) + (manip[1] + input->pointers[0].pointer_y) * (manip[1] + input->pointers[0].pointer_y) < 0.165 * 0.165)
		return TRUE;
	pos = (-input->pointers[0].pointer_x - manip[0]) - (-input->pointers[0].pointer_y - manip[1]);
	if(pos > 0 && pos < 0.4)
	{
		pos = (-input->pointers[0].pointer_x - manip[0]) + (-input->pointers[0].pointer_y - manip[1]);
		if(pos > -0.05 && pos < 0.05)
			return TRUE;
	}
	return FALSE;
}

void  matrix_func(double *output, uint vertex_id, void *data)
{
	double vertex[3], select;
	select = udg_get_select(vertex_id);
	vertex[0] = GlobalTransformManipulator.data[vertex_id * 3];
	vertex[1] = GlobalTransformManipulator.data[vertex_id * 3 + 1];
	vertex[2] = GlobalTransformManipulator.data[vertex_id * 3 + 2];
	f_transform3d(vertex, (double *)data, vertex[0], vertex[1], vertex[2]);
	output[0] = vertex[0] * select + GlobalTransformManipulator.data[vertex_id * 3] * (1 - select);
	output[1] = vertex[1] * select + GlobalTransformManipulator.data[vertex_id * 3 + 1] * (1 - select);
	output[2] = vertex[2] * select + GlobalTransformManipulator.data[vertex_id * 3 + 2] * (1 - select);
}
 
void  normal_func(double *output, uint vertex_id, void *data)
{
	double select;
	select = udg_get_select(vertex_id);
	output[0] = GlobalTransformManipulator.data[vertex_id * 3] + (select * *(double *)data * GlobalTransformManipulator.normal[vertex_id * 3]);
	output[1] = GlobalTransformManipulator.data[vertex_id * 3 + 1] + (select * *(double *)data * GlobalTransformManipulator.normal[vertex_id * 3 + 1]);
	output[2] = GlobalTransformManipulator.data[vertex_id * 3 + 2] + (select * *(double *)data * GlobalTransformManipulator.normal[vertex_id * 3 + 2]);
}

void La_t_tm_maipulate_box_draw()
{
	float x = 1, y = 1, z = 1;
	x /= GlobalTransformManipulator.manipulator_scale[0];
	y /= GlobalTransformManipulator.manipulator_scale[1];
	z /= GlobalTransformManipulator.manipulator_scale[2];
	r_primitive_line_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
						GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
						GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2],
						GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
						GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2], 1, 1, 1, 1);

	r_primitive_line_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
						GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
						GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
						GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
						GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2], 1, 1, 1, 1);

	r_primitive_line_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
						GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
						GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
						GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2], 1, 1, 1, 1);
	r_primitive_line_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
						GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2], 1, 1, 1, 1);


	
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.max_scale[0] + x, GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1] + y, GlobalTransformManipulator.max_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2] + z, 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);

	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.min_scale[0] - x, GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1] + y, GlobalTransformManipulator.max_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.max_scale[2] + z, 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);

	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.max_scale[0] + x, GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1] - y, GlobalTransformManipulator.max_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2] + z, 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);

	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.min_scale[0] - x, GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1] - y, GlobalTransformManipulator.max_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2],
							GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.max_scale[2] + z, 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);


	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.max_scale[0] + x, GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1] + y, GlobalTransformManipulator.min_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2] - z, 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);

	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.min_scale[0] - x, GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1] + y, GlobalTransformManipulator.min_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.max_scale[1], GlobalTransformManipulator.min_scale[2] + z, 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);

	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.max_scale[0] + x, GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1] - y, GlobalTransformManipulator.min_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.max_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2] + z, 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);

	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.min_scale[0] - x, GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1] - y, GlobalTransformManipulator.min_scale[2], 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);
	r_primitive_line_fade_3d(GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2],
							GlobalTransformManipulator.min_scale[0], GlobalTransformManipulator.min_scale[1], GlobalTransformManipulator.min_scale[2] + z, 0.2, 0.2, 0.2, 1, 0.0, 0.0, 0.0, 0);

	r_primitive_line_flush();
}

boolean la_t_tm_manipulate(BInputState *input, double *snap, uint snap_type)
{
	RMatrix manipulator_matrix;
	ENode *node;
	double select, delta, vertex[3], state[3], matrix[16], normal_tangent;
	float m[16];
	uint i, j, id, update_length = -1; 
	void  (*func)(double *output, uint vertex_id, void *data) = NULL;
	void *data;
	node = e_ns_get_node_selected(0, V_NT_GEOMETRY);

	manipulator_matrix = la_world_matrix;
	for(i = 0; i < 16; i++)
		m[i] = la_axis_matrix[i];
	r_matrix_set(&manipulator_matrix);
	r_matrix_matrix_mult(&manipulator_matrix, m);
	if(input->pointers[0].button[0] == TRUE)
		update_length = 800.0 * input->delta_time;
	else
		GlobalTransformManipulator.update_rotate = 0;

	if(GlobalTransformManipulator.mode == TMM_IDLE)
	{
		if(GlobalTransformManipulator.manipulator_pos[0] > 100 || GlobalTransformManipulator.manipulator_pos[0] < -100)
			i = 0;
		GlobalTransformManipulator.manipulator_start[0] = GlobalTransformManipulator.manipulator_pos[0];
		GlobalTransformManipulator.manipulator_start[1] = GlobalTransformManipulator.manipulator_pos[1];
		GlobalTransformManipulator.manipulator_start[2] = GlobalTransformManipulator.manipulator_pos[2];
		r_matrix_set(&la_world_matrix);
		return FALSE;
	}
	if(GlobalTransformManipulator.mode == TMM_PLACE)
	{
		double posd[3];
		static float pos[3] = {0, 0, 0}, size[3] = {1, 1, 1}, rotation_matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		GlobalTransformManipulator.manipulator_pos[0] = snap[0];
		GlobalTransformManipulator.manipulator_pos[1] = snap[1];
		GlobalTransformManipulator.manipulator_pos[2] = snap[2];
		f_transforminv3d(posd, la_axis_matrix, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);
		pos[0] = posd[0];
		pos[1] = posd[1];
		pos[2] = posd[2];
		seduce_manipulator_pos_xyz(input, &manipulator_matrix,
										pos, 
										&GlobalTransformManipulator.manipulator_pos[0], 
										NULL, input->pointers[0].button[1],
										TRUE, TRUE, TRUE, 1.2, 1);
		seduce_manipulator_scale(input, &manipulator_matrix, pos, size, &GlobalTransformManipulator.manipulator_pos[1], NULL, input->pointers[0].button[1], TRUE, TRUE, TRUE, 1, 1);
		seduce_manipulator_rotate(input, &manipulator_matrix, pos, rotation_matrix, &GlobalTransformManipulator.manipulator_pos[2], NULL, input->pointers[0].button[1], 1.0, 1);
		if(input->pointers[0].button[0] != TRUE)
			GlobalTransformManipulator.mode = TMM_IDLE;
		r_matrix_set(&la_world_matrix);
		return FALSE;
	}
	

	if(GlobalTransformManipulator.mode == TMM_NORMAL || GlobalTransformManipulator.mode == TMM_SMOOTH || GlobalTransformManipulator.mode == TMM_TANGENT)
	{
		double output[3];
		seduce_view_projection_screend(NULL, output, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);
		normal_tangent = ((input->pointers[0].pointer_x - input->pointers[0].click_pointer_x[0]) + (input->pointers[0].pointer_y - input->pointers[0].click_pointer_y[0])) * GlobalTransformManipulator.manipulator_size - GlobalTransformManipulator.grab_manip_pos;
		if(normal_tangent < 0.0)
		{
			if(normal_tangent < -0.01)
				normal_tangent += 0.01;
			else
				normal_tangent = 0.0;
		}else
		{
			if(normal_tangent > 0.01)
				normal_tangent -= 0.01;
			else
				normal_tangent = 0.0;
		}		
		func = normal_func;
		data = &normal_tangent;
	}

	f_matrix_cleard(matrix);
	if(GlobalTransformManipulator.mode == TMM_TRANSFORM)
	{
		double pos[3], d;
		float fpos[3], fsnap[3];
		f_transforminv3d(pos, la_axis_matrix, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);  /* Transforms a 3D point with a 4x4 64 bit double matrix.*/
		fpos[0] = (float)pos[0];
		fpos[1] = (float)pos[1];
		fpos[2] = (float)pos[2];
		if(input->pointers[0].button[1] && snap_type != SUI_ST_NONE)
		{
			lo_pfx_draw_snap(input, snap_type, snap, GlobalTransformManipulator.manipulator_pos, !input->pointers[0].button[0]);
			switch(snap_type)
			{
				case SUI_ST_VERTEX_FAR :
				case SUI_ST_VERTEX_CLOSE :
					f_transforminv3d(pos, la_axis_matrix, snap[0], snap[1], snap[2]);
					fsnap[0] = (float)pos[0];
					fsnap[1] = (float)pos[1];
					fsnap[2] = (float)pos[2];
				break;
				case SUI_ST_LINE :
					f_transforminv3d(pos, la_axis_matrix, GlobalTransformManipulator.manipulator_start[0], GlobalTransformManipulator.manipulator_start[1], GlobalTransformManipulator.manipulator_start[2]);
					d =	sqrt((snap[0] - snap[3]) * (snap[0] - snap[3]) +
							(snap[1] - snap[4]) * (snap[1] - snap[4]) +
							(snap[2] - snap[5]) * (snap[2] - snap[5]));

					if(pos[0] > fpos[0] + 0.01)
						fsnap[0] = pos[0] - d;
					else
						fsnap[0] = pos[0] + d;
					if(pos[1] > fpos[1])
						fsnap[1] = pos[1] - d;
					else
						fsnap[1] = pos[1] + d;
					if(pos[2] > fpos[2])
						fsnap[2] = pos[2] - d;
					else
						fsnap[2] = pos[2] + d;
				break;
				case SUI_ST_TANGENT :
				{
					double  normal[3], vector[5] = {0, 0, 1, 0, 0}, output[3], start[3];
					f_transforminv3d(pos, la_axis_matrix, snap[0], snap[1], snap[2]);
					f_transforminv3d(normal, la_axis_matrix, snap[3], snap[4], snap[5]);
					f_transforminv3d(start, la_axis_matrix, GlobalTransformManipulator.manipulator_start[0], GlobalTransformManipulator.manipulator_start[1], GlobalTransformManipulator.manipulator_start[2]);
					if(normal[0] > 0.1 || normal[0] < -0.1)
					{
						f_project3d(output, pos, normal, start, &vector[2]);
						fsnap[0] = output[0];
					}else
						fsnap[0] = snap[0];
					if(normal[1] > 0.1 || normal[1] < -0.1)
					{
						f_project3d(output, pos, normal, start, &vector[1]);
						fsnap[1] = output[1];
					}else
						fsnap[1] = snap[1];
					if(normal[2] > 0.1 || normal[2] < -0.1)
					{
						f_project3d(output, pos, normal, start, &vector[0]);
						fsnap[2] = output[2];
					}else
						fsnap[2] = snap[2];
				}
				break;
			}
		}
		seduce_manipulator_pos_xyz(input, &manipulator_matrix,
										fpos, 
										&GlobalTransformManipulator.manipulator_pos[0], 
										fsnap, input->pointers[0].button[1] && snap_type != SUI_ST_NONE,
										TRUE, TRUE, TRUE, 1.2, 1);
		f_transform3d(GlobalTransformManipulator.manipulator_pos, la_axis_matrix, fpos[0], fpos[1], fpos[2]);
		matrix[12] = GlobalTransformManipulator.manipulator_pos[0] - GlobalTransformManipulator.manipulator_start[0];
		matrix[13] = GlobalTransformManipulator.manipulator_pos[1] - GlobalTransformManipulator.manipulator_start[1];
		matrix[14] = GlobalTransformManipulator.manipulator_pos[2] - GlobalTransformManipulator.manipulator_start[2];
		func = matrix_func;
		data = matrix;
	}
	
	if(GlobalTransformManipulator.mode == TMM_ROTATE)
	{
		double pos[3], t_snap[3], dm[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, dm2[16];
		float fpos[3], fsnap[3];
		static float rotation_matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		f_transforminv3d(pos, la_axis_matrix, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);  /* Transforms a 3D point with a 4x4 64 bit double matrix.*/
		fpos[0] = (float)pos[0];
		fpos[1] = (float)pos[1];
		fpos[2] = (float)pos[2];
	
		f_transforminv3d(t_snap, la_axis_matrix, snap[0], snap[1], snap[2]);
		fsnap[0] = t_snap[0];
		fsnap[1] = t_snap[1];
		fsnap[2] = t_snap[2];
		seduce_manipulator_rotate(input, &manipulator_matrix,
										fpos, 
										rotation_matrix,
										&GlobalTransformManipulator.manipulator_pos[2], 
										fsnap, input->pointers[0].button[1] && snap_type != SUI_ST_NONE,
										1, 1);
		f_matrix_multiplyd(dm, matrix, la_axis_matrix);
		dm2[0] = (rotation_matrix[0] * dm[0]) + (rotation_matrix[1] * dm[4]) + (rotation_matrix[2] * dm[8]) + (rotation_matrix[3] * dm[12]);
		dm2[4] = (rotation_matrix[4] * dm[0]) + (rotation_matrix[5] * dm[4]) + (rotation_matrix[6] * dm[8]) + (rotation_matrix[7] * dm[12]);
		dm2[8] = (rotation_matrix[8] * dm[0]) + (rotation_matrix[9] * dm[4]) + (rotation_matrix[10] * dm[8]) + (rotation_matrix[11] * dm[12]);
		dm2[12] = (rotation_matrix[12] * dm[0]) + (rotation_matrix[13] * dm[4]) + (rotation_matrix[14] * dm[8]) + (rotation_matrix[15] * dm[12]);

		dm2[1] = (rotation_matrix[0] * dm[1]) + (rotation_matrix[1] * dm[5]) + (rotation_matrix[2] * dm[9]) + (rotation_matrix[3] * dm[13]);
		dm2[5] = (rotation_matrix[4] * dm[1]) + (rotation_matrix[5] * dm[5]) + (rotation_matrix[6] * dm[9]) + (rotation_matrix[7] * dm[13]);
		dm2[9] = (rotation_matrix[8] * dm[1]) + (rotation_matrix[9] * dm[5]) + (rotation_matrix[10] * dm[9]) + (rotation_matrix[11] * dm[13]);
		dm2[13] = (rotation_matrix[12] * dm[1]) + (rotation_matrix[13] * dm[5]) + (rotation_matrix[14] * dm[9]) + (rotation_matrix[15] * dm[13]);

		dm2[2] = (rotation_matrix[0] * dm[2]) + (rotation_matrix[1] * dm[6]) + (rotation_matrix[2] * dm[10]) + (rotation_matrix[3] * dm[14]);
		dm2[6] = (rotation_matrix[4] * dm[2]) + (rotation_matrix[5] * dm[6]) + (rotation_matrix[6] * dm[10]) + (rotation_matrix[7] * dm[14]);
		dm2[10] = (rotation_matrix[8] * dm[2]) + (rotation_matrix[9] * dm[6]) + (rotation_matrix[10] * dm[10]) + (rotation_matrix[11] * dm[14]);
		dm2[14] = (rotation_matrix[12] * dm[2]) + (rotation_matrix[13] * dm[6]) + (rotation_matrix[14] * dm[10]) + (rotation_matrix[15] * dm[14]);

		dm2[3] = (rotation_matrix[0] * dm[3]) + (rotation_matrix[1] * dm[7]) + (rotation_matrix[2] * dm[11]) + (rotation_matrix[3] * dm[15]);
		dm2[7] = (rotation_matrix[4] * dm[3]) + (rotation_matrix[5] * dm[7]) + (rotation_matrix[6] * dm[11]) + (rotation_matrix[7] * dm[15]);
		dm2[11] = (rotation_matrix[8] * dm[3]) + (rotation_matrix[9] * dm[7]) + (rotation_matrix[10] * dm[11]) + (rotation_matrix[11] * dm[15]);
		dm2[15] = (rotation_matrix[12] * dm[3]) + (rotation_matrix[13] * dm[7]) + (rotation_matrix[14] * dm[11]) + (rotation_matrix[15] * dm[15]);
		f_matrix_reverse4d(dm, la_axis_matrix);
		f_matrix_multiplyd(matrix, dm2, dm);
		f_transform3d(pos, matrix, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);
		matrix[12] -=  pos[0] - GlobalTransformManipulator.manipulator_pos[0];
		matrix[13] -=  pos[1] - GlobalTransformManipulator.manipulator_pos[1];
		matrix[14] -=  pos[2] - GlobalTransformManipulator.manipulator_pos[2];
		func = matrix_func;
		data = matrix;
	}

	if(GlobalTransformManipulator.mode == TMM_SCALE)
	{
		double pos[3], scale[3], dm[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, inv[16], d;
		float fpos[3], fsnap[3];
		f_transforminv3d(pos, la_axis_matrix, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);  /* Transforms a 3D point with a 4x4 64 bit double matrix.*/
		fpos[0] = (float)pos[0];
		fpos[1] = (float)pos[1];
		fpos[2] = (float)pos[2];
		if(input->pointers[0].button[1] && snap_type != SUI_ST_NONE)
		{
			switch(snap_type)
			{
				case SUI_ST_VERTEX_FAR :
				case SUI_ST_VERTEX_CLOSE :
					f_transforminv3d(scale, la_axis_matrix, snap[0], snap[1], snap[2]);
					if(scale[0] > pos[0])
						fsnap[0] = (scale[0] - pos[0]) / (GlobalTransformManipulator.max_scale[0] - pos[0]);
					else
						fsnap[0] = (scale[0] - pos[0]) / (GlobalTransformManipulator.min_scale[0] - pos[0]);
					if(scale[1] > pos[1])
						fsnap[1] = (scale[1] - pos[1]) / (GlobalTransformManipulator.max_scale[1] - pos[1]);
					else
						fsnap[1] = (scale[1] - pos[1]) / (GlobalTransformManipulator.min_scale[1] - pos[1]);
					if(scale[2] > pos[2])
						fsnap[2] = (scale[2] - pos[2]) / (GlobalTransformManipulator.max_scale[2] - pos[2]);
					else
						fsnap[2] = (scale[2] - pos[2]) / (GlobalTransformManipulator.min_scale[2] - pos[2]);
				break;
				case SUI_ST_LINE :
					f_transforminv3d(scale, la_axis_matrix, snap[0], snap[1], snap[2]);
					f_transforminv3d(pos, la_axis_matrix, GlobalTransformManipulator.manipulator_start[0], GlobalTransformManipulator.manipulator_start[1], GlobalTransformManipulator.manipulator_start[2]);
					d =	sqrt((snap[0] - snap[3]) * (snap[0] - snap[3]) +
							(snap[1] - snap[4]) * (snap[1] - snap[4]) +
							(snap[2] - snap[5]) * (snap[2] - snap[5]));
					if(scale[0] > pos[0])
						fsnap[0] = d / (GlobalTransformManipulator.max_scale[0] - pos[0]);
					else
						fsnap[0] = -d / (GlobalTransformManipulator.min_scale[0] - pos[0]);
					if(scale[1] > pos[1])
						fsnap[1] = d / (GlobalTransformManipulator.max_scale[1] - pos[1]);
					else
						fsnap[1] = -d / (GlobalTransformManipulator.min_scale[1] - pos[1]);
					if(scale[2] > pos[2])
						fsnap[2] = d / (GlobalTransformManipulator.max_scale[2] - pos[2]);
					else
						fsnap[2] = -d / (GlobalTransformManipulator.min_scale[2] - pos[2]);

				break;
				case SUI_ST_TANGENT :
				{
					double  normal[3], vector[5] = {0, 0, 1, 0, 0}, output[3], start[3], origo[3];
					f_transforminv3d(origo, la_axis_matrix, snap[0], snap[1], snap[2]);
					f_transforminv3d(normal, la_axis_matrix, snap[3], snap[4], snap[5]);
					f_transforminv3d(start, la_axis_matrix, GlobalTransformManipulator.manipulator_start[0], GlobalTransformManipulator.manipulator_start[1], GlobalTransformManipulator.manipulator_start[2]);
					f_project3d(output, origo, normal, start, &vector[2]);
					fsnap[0] = output[0];
					f_project3d(output, origo, normal, start, &vector[1]);
					fsnap[1] = output[1];
					f_project3d(output, origo, normal, start, &vector[0]);
					fsnap[2] = output[2];

					if(fsnap[0] > pos[0])
						fsnap[0] = (fsnap[0] - pos[0]) / (GlobalTransformManipulator.max_scale[0] - pos[0]);
					else
						fsnap[0] = (fsnap[0] - pos[0]) / (GlobalTransformManipulator.min_scale[0] - pos[0]);
					if(scale[1] > pos[1])
						fsnap[1] = (fsnap[1] - pos[1]) / (GlobalTransformManipulator.max_scale[1] - pos[1]);
					else
						fsnap[1] = (fsnap[1] - pos[1]) / (GlobalTransformManipulator.min_scale[1] - pos[1]);
					if(scale[2] > pos[2])
						fsnap[2] = (fsnap[2] - pos[2]) / (GlobalTransformManipulator.max_scale[2] - pos[2]);
					else
						fsnap[2] = (fsnap[2] - pos[2]) / (GlobalTransformManipulator.min_scale[2] - pos[2]);
				}
				break;
			}
		}
		seduce_manipulator_scale(input, &manipulator_matrix,
										fpos, 
										GlobalTransformManipulator.manipulator_scale,
										&GlobalTransformManipulator.manipulator_pos[1], 
										fsnap, input->pointers[0].button[1] && snap_type != SUI_ST_NONE,
										TRUE, TRUE, TRUE, 1, 1);
		if(input->mode == BAM_DRAW)
		{
			r_matrix_set(&manipulator_matrix);
			r_matrix_push(&manipulator_matrix);
			r_matrix_translate(&manipulator_matrix, fpos[0], fpos[1], fpos[2]);
			r_matrix_scale(&manipulator_matrix, GlobalTransformManipulator.manipulator_scale[0], GlobalTransformManipulator.manipulator_scale[1], GlobalTransformManipulator.manipulator_scale[2]);
			r_matrix_translate(&manipulator_matrix, -fpos[0], -fpos[1], -fpos[2]);
			La_t_tm_maipulate_box_draw();
			r_primitive_line_flush();
			r_matrix_pop(&manipulator_matrix);
		}

		f_matrix_multiplyd(dm, matrix, la_axis_matrix);
		dm[0] *= GlobalTransformManipulator.manipulator_scale[0];
		dm[1] *= GlobalTransformManipulator.manipulator_scale[0];
		dm[2] *= GlobalTransformManipulator.manipulator_scale[0];
		dm[4] *= GlobalTransformManipulator.manipulator_scale[1];
		dm[5] *= GlobalTransformManipulator.manipulator_scale[1];
		dm[6] *= GlobalTransformManipulator.manipulator_scale[1];
		dm[8] *= GlobalTransformManipulator.manipulator_scale[2];	
		dm[9] *= GlobalTransformManipulator.manipulator_scale[2];	
		dm[10] *= GlobalTransformManipulator.manipulator_scale[2];
		f_matrix_reverse4d(inv, la_axis_matrix);
		f_matrix_multiplyd(matrix, dm, inv);
		f_transform3d(pos, matrix, GlobalTransformManipulator.manipulator_pos[0], GlobalTransformManipulator.manipulator_pos[1], GlobalTransformManipulator.manipulator_pos[2]);
		matrix[12] -=  pos[0] - GlobalTransformManipulator.manipulator_pos[0];
		matrix[13] -=  pos[1] - GlobalTransformManipulator.manipulator_pos[1];
		matrix[14] -=  pos[2] - GlobalTransformManipulator.manipulator_pos[2];
		func = matrix_func;
		data = matrix;
	}
	if(input->mode == BAM_DRAW && GlobalTransformManipulator.line_object != NULL)
		seduce_primitive_line_draw(GlobalTransformManipulator.line_object, 1, 1, 1, 1);
	if(input->mode != BAM_EVENT)
	{
		r_matrix_set(&la_world_matrix);
		return input->pointers[0].button[0];
	}
	if(func == matrix_func && GlobalTransformManipulator.tags != NULL)
	{	
		UNDOTag *tag;
		tag = udg_get_tags(&i);
		if(i == GlobalTransformManipulator.tag_length)
		{
			for(i = 0; i < GlobalTransformManipulator.tag_length; i++)
			{
				vertex[0] = GlobalTransformManipulator.tags[i * 3];
				vertex[1] = GlobalTransformManipulator.tags[i * 3 + 1];
				vertex[2] = GlobalTransformManipulator.tags[i * 3 + 2];
	

				f_transform3d(vertex, matrix, vertex[0], vertex[1], vertex[2]);
				vertex[0] = vertex[0] * tag[i].select + GlobalTransformManipulator.tags[i * 3] * (1 - tag[i].select);
				vertex[1] = vertex[1] * tag[i].select + GlobalTransformManipulator.tags[i * 3 + 1] * (1 - tag[i].select);
				vertex[2] = vertex[2] * tag[i].select + GlobalTransformManipulator.tags[i * 3 + 2] * (1 - tag[i].select);
				udg_move_tag(i, vertex);
			}
		}
	}

	if(input->mode == BAM_EVENT && (input->pointers[0].delta_pointer_x != 0.0 || input->pointers[0].delta_pointer_y != 0.0 || (input->pointers[0].button[2] != TRUE && input->pointers[0].last_button[2] == TRUE || input->pointers[0].button[0] != TRUE))) 
	{
		if(func == matrix_func && GlobalTransformManipulator.accelerated_mode)
		{
			for(i = 0; i < 16; i++)
				GlobalTransformManipulator.matrix[i] = (float)((double *)data)[i];
		}else
		{
			for(i = 0; i < 16; i++)
			{
				if(i % 5 == 0)
					GlobalTransformManipulator.matrix[i] = 1;
				else
					GlobalTransformManipulator.matrix[i] = 0;
			}
		}
		if(data != NULL)
		{
			if((input->pointers[0].button[2] != TRUE && input->pointers[0].last_button[2] == TRUE) || func != matrix_func || input->pointers[0].button[0] != TRUE || !GlobalTransformManipulator.accelerated_mode)
			{
				j = 0;
				for(i = 0; i < GlobalTransformManipulator.data_length; i++)
				{
					select = udg_get_select(i);
					if(select > 0.0001)
					{
						state[0] = GlobalTransformManipulator.data[i * 3];
						state[1] = GlobalTransformManipulator.data[i * 3 + 1];
						state[2] = GlobalTransformManipulator.data[i * 3 + 2];
						func(vertex, i, data);
						udg_vertex_set(i, state, vertex[0], vertex[1], vertex[2]);
					/*	if(++j > update_length)
						{
							GlobalTransformManipulator.update_rotate = i;
							break;
						}*/
					}
				}
				if(i == GlobalTransformManipulator.data_length)
					GlobalTransformManipulator.update_rotate = 0;
				if(input->pointers[0].button[2] != TRUE && input->pointers[0].last_button[2] == TRUE)
				{
					undo_event_done();
					la_t_extrude(GlobalTransformManipulator.data_length, func, data);
				}
			}
		}else
		{
			for(i = 0; i < GlobalTransformManipulator.data_length; i++)
			{
				select = udg_get_select(i);
				if(select > 0.0001)
				{
					func(vertex, i, data);
					udg_vertex_move(i, vertex[0], vertex[1], vertex[2]);
				}
			}
		}	}
	if(input->mode == BAM_EVENT && !input->pointers[0].button[0])
	{	
		for(i = 0; i < 16; i++)
		{
			if(i % 5 == 0)
				GlobalTransformManipulator.matrix[i] = 1;
			else
				GlobalTransformManipulator.matrix[i] = 0;
		}
		if(GlobalTransformManipulator.data != NULL)
			free(GlobalTransformManipulator.data);

		if(GlobalTransformManipulator.line_object)
		{
			seduce_primitive_line_object_free(GlobalTransformManipulator.line_object);
			GlobalTransformManipulator.line_object = NULL;
		}
		if(GlobalTransformManipulator.normal != NULL)
		{
			free(GlobalTransformManipulator.manipulator_normal_array);
			free(GlobalTransformManipulator.normal);
		}
		if(GlobalTransformManipulator.tags != NULL)
		{
			free(GlobalTransformManipulator.tags);
			GlobalTransformManipulator.tags = NULL;
		}
		GlobalTransformManipulator.mode = TMM_IDLE;
	}

	
	r_matrix_set(&la_world_matrix);
	if(input->mode == BAM_EVENT)
		return input->pointers[0].button[0];
	return TRUE;
}

void grab_one_vertex(BInputState *input, uint id, double *pos)
{
    static int direction, recursion;
	static double *select;
    uint i, j;
    double delta[3], base[3];
    if(input->pointers[0].button[0] == TRUE && input->pointers[0].last_button[0] == FALSE)
    {
        direction = 1;
        recursion = 0;
        lock_transform_vertexes(input, FALSE, FALSE, FALSE);
        select = malloc((sizeof *select) * GlobalTransformManipulator.data_length);
        for(i = 0; i < GlobalTransformManipulator.data_length; i++)
            select[i] = 0;
        select[id] = 1;
    }
    delta[0] = pos[0] - GlobalTransformManipulator.data[id * 3];
    delta[1] = pos[1] - GlobalTransformManipulator.data[id * 3 + 1];
    delta[2] = pos[2] - GlobalTransformManipulator.data[id * 3 + 2];     
    if(input->pointers[0].button[0] == FALSE && input->pointers[0].last_button[0] == TRUE)
    {
        for(i = 0; i < GlobalTransformManipulator.data_length; i++)
        {
            if(select[i] > 0.0001)
            {
                base[0] = GlobalTransformManipulator.data[i * 3];
                base[1] = GlobalTransformManipulator.data[i * 3 + 1];
                base[2] = GlobalTransformManipulator.data[i * 3 + 2];
                udg_vertex_set(i, base, base[0] + delta[0] * select[i], base[1] + delta[1] * select[i], base[2] + delta[2] * select[i]);
            }
        }

        free(select);
        free(GlobalTransformManipulator.data);
        return;
    }
    if(input->pointers[0].button[2] == FALSE && input->pointers[0].last_button[2] == TRUE)
    {
        recursion += direction;
        if(direction < 0)
            for(i = 0; i < GlobalTransformManipulator.data_length; i++)
                if(select[i] > 0.0001)
                    udg_vertex_move(i, GlobalTransformManipulator.data[i * 3], GlobalTransformManipulator.data[i * 3 + 1], GlobalTransformManipulator.data[i * 3 + 2]);
        if(recursion == 0 || recursion == 5)
            direction = 0 - direction;
        for(i = 0; i < GlobalTransformManipulator.data_length; i++)
            select[i] = 0;
        select[id] = 1;            
        if(recursion != 0)
        {
            uint  *count, ref_count, *ref;
            double *value, temp;
            udg_get_geometry(NULL, &ref_count, NULL, &ref, NULL);
            ref_count *= 4;  
            count = malloc((sizeof *count) * GlobalTransformManipulator.data_length);
            value = malloc((sizeof *value) * GlobalTransformManipulator.data_length);

            for(j = 0; j < recursion; j++)
            {
            	for(i = 0; i < GlobalTransformManipulator.data_length; i++)
                {
                    count[i] = 0;
                    value[i] = 0;
                }
                for(i = 0; i < ref_count; i += 4)
                {
                    if(ref[i] < GlobalTransformManipulator.data_length)
                    {
                        temp = select[ref[i]] + select[ref[i + 1]] + select[ref[i + 2]];
                        if(ref[i + 3] < GlobalTransformManipulator.data_length)
                            temp = (temp + select[ref[i + 3]]) / 4;
                        else
                            temp = temp / 3;
                        count[ref[i]]++;
                        value[ref[i]] += temp;
                        count[ref[i + 1]]++;
                        value[ref[i + 1]] += temp;
                        count[ref[i + 2]]++;
                        value[ref[i + 2]] += temp;
                        if(ref[i + 3] < GlobalTransformManipulator.data_length)
                        {
                            count[ref[i + 3]]++;
                            value[ref[i + 3]] += temp;
                        }
                    }
                }
                for(i = 0; i < GlobalTransformManipulator.data_length; i++)
					if(count[i] > 0)
	                    select[i] = value[i] / (double)count[i];
            }
            temp = 0;
            for(i = 0; i < GlobalTransformManipulator.data_length; i++)
                if(temp < select[i])
                    temp = select[i];
            for(i = 0; i < GlobalTransformManipulator.data_length; i++)
                select[i] = select[i] / temp;
/*		for(i = 0; i < GlobalTransformManipulator.data_length; i++)
			printf("select[%u] = %f\n", i, select[i]);
*/
            free(count);
            free(value);
        }
    }
    for(i = 0; i < GlobalTransformManipulator.data_length; i++)
        if(select[i] > 0.0001)
            udg_vertex_move(i, GlobalTransformManipulator.data[i * 3] + delta[0] * select[i],
			    GlobalTransformManipulator.data[i * 3 + 1] + delta[1] * select[i],
			    GlobalTransformManipulator.data[i * 3 + 2] + delta[2] * select[i]);
}
