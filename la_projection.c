#include "la_includes.h"

#include "la_projection.h"
#include "la_geometry_undo.h"
#include "la_particle_fx.h"
#include "la_tool.h"
 
#ifdef LO_NO_WAY_NO_DEF

extern void point_threw_matrix4(double *matrix, double *pos_x, double *pos_y, double *pos_z, double *pos_w);
extern void point_threw_matrix3(double *matrix, double *pos_x, double *pos_y, double *pos_z);

extern void matrix_multiply(const double *a,const  double *b, double *output);

extern void matrix_rotate_x(double *matrix, double degree);
extern void matrix_rotate_y(double *matrix, double degree);
extern void matrix_rotate_z(double *matrix, double degree);
extern void transform_matrix(double *matrix, double x, double y, double z);
extern void scale_matrix(double *matrix, double x, double y, double z);
extern void negate_matrix(double *matrix);
extern void reverse_matrix(double *matrix);
extern void matrix_print(const double *matrix);
extern void la_draw_force_update_persuade(void);

struct{
	double			matrix[16];
	double			model[16];
	double			pitch;
	double			yaw;
	double			pitch_target;
	double			yaw_target;
	double			target[3];
	double			position[3];
	double			speed;
	double			distance;
	double			distance_target;
    double			grid_size;
	float			grab[2];
	uint			view_axis;
	uint			axis;
	double			direction;
}ProjectionData;

void p_init(void)
{
	ProjectionData.pitch = 0;
	ProjectionData.yaw = 0;
	ProjectionData.pitch_target = 0;
	ProjectionData.yaw_target = 0;
	ProjectionData.position[0] = 0;
	ProjectionData.position[1] = 0;
	ProjectionData.position[2] = 0;
	ProjectionData.target[0] = 0;
	ProjectionData.target[1] = 0;
	ProjectionData.target[2] = 0;
	ProjectionData.distance = 1.0;
	ProjectionData.distance_target = 1.0;
	ProjectionData.view_axis = 3;
	ProjectionData.speed = 0.8;
    ProjectionData.grid_size = 1000;
	ProjectionData.axis = 3;
}

void p_view_change_start(BInputState *input)
{
	ProjectionData.grab[0] = input->pointers[0].pointer_x;
	ProjectionData.grab[1] = input->pointers[0].pointer_y;
	ProjectionData.speed = 0.8;
}

void p_view_change(BInputState *input)
{
	float a, b;
	if(input->mode == BAM_EVENT)
	{
		if(input->pointers[0].last_button[1] == TRUE)
		{
			if(input->pointers[0].button[2] == TRUE && input->pointers[0].button[0] == TRUE)
			{
				p_init();
			}else if(input->pointers[0].button[2] == TRUE)
			{
				ProjectionData.position[0] += ProjectionData.model[0] * ProjectionData.distance * 2 * (ProjectionData.grab[0] - input->pointers[0].pointer_x);
				ProjectionData.position[1] += ProjectionData.model[4] * ProjectionData.distance * 2 * (ProjectionData.grab[0] - input->pointers[0].pointer_x);
				ProjectionData.position[2] += ProjectionData.model[8] * ProjectionData.distance * 2 * (ProjectionData.grab[0] - input->pointers[0].pointer_x);

				ProjectionData.position[0] += ProjectionData.model[1] * ProjectionData.distance * 2 * (ProjectionData.grab[1] - input->pointers[0].pointer_y);
				ProjectionData.position[1] += ProjectionData.model[5] * ProjectionData.distance * 2 * (ProjectionData.grab[1] - input->pointers[0].pointer_y);
				ProjectionData.position[2] += ProjectionData.model[9] * ProjectionData.distance * 2 * (ProjectionData.grab[1] - input->pointers[0].pointer_y);
				ProjectionData.target[0] = ProjectionData.position[0];
				ProjectionData.target[1] = ProjectionData.position[1];
				ProjectionData.target[2] = ProjectionData.position[2];
			}else if(input->pointers[0].button[0] == TRUE)
			{
				a = ProjectionData.distance * (1 + (input->delta_pointer_x - input->delta_pointer_y) * 4);
				if(a > 90000)
					a = 90000;
				if(a < 0.01)
					a = 0.01;
				ProjectionData.distance_target = a;
			}else
			{
				a = ProjectionData.yaw_target + (input->pointers[0].pointer_x - ProjectionData.grab[0]) * 400; 
				b = ProjectionData.pitch_target - (input->pointers[0].pointer_y - ProjectionData.grab[1]) * 400;
				if(b > 90)
					b = 90;
				if(b < -90)
					b = -90;
		//		ProjectionData.pitch = b;
				ProjectionData.pitch_target = b;
		//		ProjectionData.yaw = a;
				ProjectionData.yaw_target = a;
				if(ProjectionData.yaw > 180 && ProjectionData.yaw_target > 180)
				{
					ProjectionData.yaw -= 360;
					ProjectionData.yaw_target -= 360;
				}else if(ProjectionData.yaw < -180 && ProjectionData.yaw_target < -180)
				{
					ProjectionData.yaw += 360;
					ProjectionData.yaw_target += 360;
				}
			}
		}
		ProjectionData.grab[0] = input->pointers[0].pointer_x;
		ProjectionData.grab[1] = input->pointers[0].pointer_y;

	/*	a = sqrt(ProjectionData.model[2] * ProjectionData.model[2] + ProjectionData.model[6] * ProjectionData.model[6] + ProjectionData.model[10] * ProjectionData.model[10]);
		if(ProjectionData.model[2] / a > 0.95 || ProjectionData.model[2] / a < -0.95)
			ProjectionData.view_axis = 0;
		else if(ProjectionData.model[6] / a > 0.95 || ProjectionData.model[6] / a < -0.95)
			ProjectionData.view_axis = 1;
		else if(ProjectionData.model[10] / a > 0.95 || ProjectionData.model[10] / a < -0.95)
			ProjectionData.view_axis = 2;
		else*/
			ProjectionData.view_axis = 3;

	/*	if(input->pointers[0].button[1] == TRUE && (input->delta_pointer_x > 0.0001 || input->delta_pointer_x < 0.0001 || input->delta_pointer_y > 0.0001 || input->delta_pointer_y < 0.0001))
			la_draw_force_update_persuade();*/
	//	if(input->pointers[0].button[1] == FALSE && input->pointers[0].last_button[1] == TRUE)
	//		la_draw_force_update_persuade();
	}
}

void p_projection_update(void)
{
	double	proj[16];
	glGetDoublev(GL_PROJECTION_MATRIX, ProjectionData.matrix);
	glGetDoublev(GL_MODELVIEW_MATRIX, ProjectionData.model);
}
void p_set_grid_size(double grid_size)
{
    ProjectionData.grid_size = 1.0 / grid_size;
}

void p_get_view_center(double *center)
{
	center[0] = ProjectionData.position[0];
	center[1] = ProjectionData.position[1];
	center[2] = ProjectionData.position[2];
}

void p_get_view_camera(double *camera)
{
	camera[0] = ProjectionData.model[2] * ProjectionData.distance + ProjectionData.position[0];
	camera[1] = ProjectionData.model[6] * ProjectionData.distance + ProjectionData.position[1];
	camera[2] = ProjectionData.model[10] * ProjectionData.distance + ProjectionData.position[2];
}

double p_get_distance_camera(void)
{
	return ProjectionData.distance;
}

void p_view_set(void)
{
	static float rotate = 0;
	rotate++;

	ProjectionData.position[0] = ProjectionData.target[0] * (1 - ProjectionData.speed) + ProjectionData.position[0] * ProjectionData.speed;
	ProjectionData.position[1] = ProjectionData.target[1] * (1 - ProjectionData.speed) + ProjectionData.position[1] * ProjectionData.speed;
	ProjectionData.position[2] = ProjectionData.target[2] * (1 - ProjectionData.speed) + ProjectionData.position[2] * ProjectionData.speed;

	ProjectionData.pitch = ProjectionData.pitch_target * (1 - ProjectionData.speed) + ProjectionData.pitch * ProjectionData.speed;
	ProjectionData.yaw = ProjectionData.yaw_target * (1 - ProjectionData.speed) + ProjectionData.yaw * ProjectionData.speed;
	ProjectionData.distance = ProjectionData.distance_target * (1 - ProjectionData.speed) + ProjectionData.distance * ProjectionData.speed;
	glTranslated(0, 0, -ProjectionData.distance);
	glRotated(ProjectionData.pitch, 1, 0, 0); /* the rotate functions now handels radians too */
	glRotated(ProjectionData.yaw, 0, 1, 0); /* the rotate functions now handels radians too */
	glTranslated(-ProjectionData.position[0], -ProjectionData.position[1], -ProjectionData.position[2]);
	seduce_view_update(NULL);
}


void p_get_projection(double *output, float x, float y)
{
	output[0] = ProjectionData.model[0] * x * ProjectionData.distance;
	output[1] = ProjectionData.model[4] * x * ProjectionData.distance;
	output[2] = ProjectionData.model[8] * x * ProjectionData.distance;
	output[0] += ProjectionData.model[1] * y * ProjectionData.distance;
	output[1] += ProjectionData.model[5] * y * ProjectionData.distance;
	output[2] += ProjectionData.model[9] * y * ProjectionData.distance;
	output[0] += ProjectionData.position[0];
	output[1] += ProjectionData.position[1];
	output[2] += ProjectionData.position[2];
}

void p_get_projection_vertex(double *output, double *vertex, double x, double y)
{
	double dist, z;
	z = ProjectionData.model[2] * (vertex[0] - ProjectionData.position[0]) + ProjectionData.model[6] * (vertex[1] - ProjectionData.position[1]) + ProjectionData.model[10] * (vertex[2] - ProjectionData.position[2]);
	dist = (ProjectionData.distance - z);
	if(dist < 0)
		dist = ProjectionData.distance;
	output[0] = ProjectionData.model[0] * x * dist;
	output[1] = ProjectionData.model[4] * x * dist;
	output[2] = ProjectionData.model[8] * x * dist;
	output[0] += ProjectionData.model[1] * y * dist;
	output[1] += ProjectionData.model[5] * y * dist;
	output[2] += ProjectionData.model[9] * y * dist;
	output[0] += ProjectionData.model[2] * z;
	output[1] += ProjectionData.model[6] * z;
	output[2] += ProjectionData.model[10] * z;
	output[0] += ProjectionData.position[0];
	output[1] += ProjectionData.position[1];
	output[2] += ProjectionData.position[2];
}

void p_get_projection_screen(double *output, double x, double y, double z)
{
	double out[3];
	f_transform3d(out, ProjectionData.model, x, y, z);
	output[0] = out[0] / out[2];
	output[1] = out[1] / out[2];
	output[2] = out[2];
}
/*
			p_get_projection_screen(pos, vertex[i].x, vertex[i].y, vertex[i].z);
			pos[0] += x;
			pos[1] += y;
			r = pos[0] * pos[0] + pos[1] * pos[1];
			if(r < *selected_distance)
*/
double p_get_projection_screen_distance(double space_x, double space_y, double space_z, double screen_x, double screen_y)
{
	double out[3];
	f_transform3d(out, ProjectionData.model, space_x, space_y, space_z);
	out[0] = (out[0] / out[2]) + screen_x ;
	out[1] = (out[1] / out[2]) + screen_y;
	return out[0] * out[0] + out[2] * out[2];
}
/* r = (b[axis] - depth) / (b[axis] - a[axis]); */
void p_get_projection_plane(double *dist, uint axis, double pointer_x, double pointer_y , double depth)
{
	double a[3], b[3], r;
	a[0] = ProjectionData.model[0] * ProjectionData.distance * pointer_x;
	a[1] = ProjectionData.model[4] * ProjectionData.distance * pointer_x;
	a[2] = ProjectionData.model[8] * ProjectionData.distance * pointer_x;
	a[0] += ProjectionData.model[1] * ProjectionData.distance * pointer_y;
	a[1] += ProjectionData.model[5] * ProjectionData.distance * pointer_y;
	a[2] += ProjectionData.model[9] * ProjectionData.distance * pointer_y;
	a[0] += ProjectionData.position[0];
	a[1] += ProjectionData.position[1];
	a[2] += ProjectionData.position[2];
	b[0] = ProjectionData.model[2] * ProjectionData.distance;
	b[1] = ProjectionData.model[6] * ProjectionData.distance;
	b[2] = ProjectionData.model[10] * ProjectionData.distance;
	b[0] += ProjectionData.position[0];
	b[1] += ProjectionData.position[1];
	b[2] += ProjectionData.position[2];
	r = (b[axis] - depth) / (b[axis] - a[axis]);
	dist[0] = b[0] - (b[0] - a[0]) * r;
	dist[1] = b[1] - (b[1] - a[1]) * r;
	dist[2] = b[2] - (b[2] - a[2]) * r;
	dist[axis] = depth;
}

double p_get_projection_line(double *dist, uint axis, double pointer_x, double pointer_y, double *pos)
{
	double a[3], b[3], r, r2, r3;
	a[0] = ProjectionData.model[0] * ProjectionData.distance * pointer_x;
	a[1] = ProjectionData.model[4] * ProjectionData.distance * pointer_x;
	a[2] = ProjectionData.model[8] * ProjectionData.distance * pointer_x;
	a[0] += ProjectionData.model[1] * ProjectionData.distance * pointer_y;
	a[1] += ProjectionData.model[5] * ProjectionData.distance * pointer_y;
	a[2] += ProjectionData.model[9] * ProjectionData.distance * pointer_y;
	b[0] = ProjectionData.model[2] * ProjectionData.distance;
	b[1] = ProjectionData.model[6] * ProjectionData.distance;
	b[2] = ProjectionData.model[10] * ProjectionData.distance;
	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];
	b[0] += ProjectionData.position[0] - pos[0];
	b[1] += ProjectionData.position[1] - pos[1];
	b[2] += ProjectionData.position[2] - pos[2];
	r = sqrt(b[(axis + 1) % 3] * b[(axis + 1) % 3] + b[(axis + 2) % 3] * b[(axis + 2) % 3]);
	r2 = sqrt(a[(axis + 1) % 3] * a[(axis + 1) % 3] + a[(axis + 2) % 3] * a[(axis + 2) % 3]);
	r3 = b[axis] + (a[axis] * r / r2);
	if(dist != NULL)
	{
		a[0] = pos[0];
		a[1] = pos[1];
		a[2] = pos[2];
		a[axis] += r3;
		f_transform3d(a, ProjectionData.model, a[0], a[1], a[2]);
		a[0] = (a[0] / a[2]) + pointer_x;
		a[1] = (a[1] / a[2]) + pointer_y;
		*dist = a[0] * a[0] + a[1] * a[1];
	}

	return r3;
}

void p_get_projection_line_snap(double *output, uint axis, double direction, double *start, double *snap, LASnapType snap_type)
{
	output[0] = start[0];
	output[1] = start[1];
	output[2] = start[2];
	if(snap_type == SUI_ST_VERTEX)
	{
		output[axis] = snap[axis];
	}else if(snap_type == SUI_ST_LENGTH)
	{
		if(direction > 0)
			output[axis] += snap[3];
		else
			output[axis] -= snap[3];
		printf("HH p_get_projection_line_snap %u %f \n", axis, snap[3]);
		printf("HH start %f %f %f\n", start[0], start[1], start[2]);
		printf("HH out %f %f %f\n", output[0], output[1], output[2]);
	}else if(snap_type == SUI_ST_TANGENT)
	{
		double r, vector[3] = {0, 0, 0};
		vector[axis] = 1;
		
		r = vector[0] * snap[3] + vector[1] * snap[4] + vector[2] * snap[5];
		if(r < 0.1 && r > -0.1)
			;//output[axis] += direction;
		else
		{
			r = (snap[3] * (start[0] - snap[0]) + snap[4] * (start[1] - snap[1]) + snap[5] * (start[2] - snap[2])) / r;
			output[0] = start[0] - r * vector[0];
			output[1] = start[1] - r * vector[1];
			output[2] = start[2] - r * vector[2];
		}
	}
}

void p_get_projection_vertex_with_axis(double *output, double *start, double pointer_x, double pointer_y, boolean snap, double *closest, LASnapType snap_type)
{
	double dist, best_dist, pos;
	//static uint axis; 
	uint i; 
//	printf("*closest = %f\n", *closest);
	if(snap != TRUE)
	{
		ProjectionData.direction = p_get_projection_line(&best_dist, 0, pointer_x, pointer_y, start);
		ProjectionData.axis = 0;
		for(i = 1; i < 3; i++)
		{
			pos = p_get_projection_line(&dist, i, pointer_x, pointer_y, start);
			if(dist < best_dist)
			{
				ProjectionData.direction = pos;
				best_dist = dist;
				ProjectionData.axis = i;
			}
		}
		if(ProjectionData.view_axis < 3)
		{	
			p_get_projection_plane(output, ProjectionData.view_axis, pointer_x, pointer_y, start[ProjectionData.view_axis]);
			return;
		}
		if(best_dist > 0.0001)
		{
			p_get_projection_vertex(output, start, pointer_x, pointer_y);
			return;
		}
	}
	output[0] = start[0];
	output[1] = start[1];
	output[2] = start[2];
	if(snap != TRUE)
		output[ProjectionData.axis] += ProjectionData.direction;
	else
	{
      /*  p_get_projection_line(&best_dist, 0, pointer_x, pointer_y, start);
		if(best_dist < 0.0001)
		{
			output[0] = p_get_projection_line(&best_dist, ProjectionData.axis, pointer_x, pointer_y, start);
			output[ProjectionData.axis] += (double)((int)(output[0] * ProjectionData.grid_size)) / ProjectionData.grid_size;
		}
		else
		{*/
			p_get_projection_line_snap(output, ProjectionData.axis, ProjectionData.direction, start, closest, snap_type);
	/*	}*/
	}
}

uint p_get_projection_axis(void)
{
	return ProjectionData.axis;
}


boolean p_find_closest_tag(double *pos, double distance, double x, double y)
{
	boolean output = FALSE;
	uint32 count, i;
	double r;
	UNDOTag	*tag;
	tag = udg_get_tags(&count);
	for(i = 0; i < count; i++)
	{
		p_get_projection_screen(pos, tag[i].vec[0], tag[i].vec[1], tag[i].vec[2]);
		if(pos[2] < 0)
		{
			pos[0] += x;
			pos[1] += y;
			r = pos[0] * pos[0] + pos[1] * pos[1];
			if(r < distance && tag[i].select < 0.001)
			{
				distance = r;
				pos[0] = tag[i].vec[0];
				pos[1] = tag[i].vec[1];
				pos[2] = tag[i].vec[2];
				output = TRUE;
			}
		}
	}
	return output;
}

#endif

struct{
	float *vertex_array;
	uint vertex_array_alloc;
	uint vertex_count;
	uint version;
	uint node_id;
	uint updateversion;
}LATransformCache;

void lo_projection_cache_init()
{
	LATransformCache.vertex_array = NULL;
	LATransformCache.vertex_array_alloc = 0;
	LATransformCache.vertex_count = 0; 
	LATransformCache.version = 0;
	LATransformCache.node_id = 0;
}


void lo_projection_cache_update(RMatrix *matrix)
{
	double *vertex, tmp[3];
	uint32 vertex_count, i;
	udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);
	LATransformCache.vertex_count = vertex_count;
	if(LATransformCache.vertex_array_alloc < vertex_count ||
		LATransformCache.vertex_array_alloc > vertex_count * 2)
	{
		LATransformCache.vertex_array_alloc = vertex_count;
		if(LATransformCache.vertex_array != NULL)
			free(LATransformCache.vertex_array);
		LATransformCache.vertex_array = malloc((sizeof *LATransformCache.vertex_array) * LATransformCache.vertex_array_alloc * 2);
	}

	for(i = 0; i < vertex_count; i++)
	{
		if(vertex[i * 3] != V_REAL64_MAX)
		{
			seduce_view_projection_screend(NULL, tmp, vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);
			if(tmp[2] < 0)
			{
				LATransformCache.vertex_array[i * 2] = tmp[0];
				LATransformCache.vertex_array[i * 2 + 1] = tmp[1];
			}else
				LATransformCache.vertex_array[i * 2] = 1000000;
		}else
			LATransformCache.vertex_array[i * 2] = 1000000;
	}
	LATransformCache.version = udg_get_version(FALSE, TRUE, FALSE, FALSE, FALSE);
	LATransformCache.node_id = udg_get_modeling_node();
	LATransformCache.updateversion++;
}

void lo_projection_cache_update_test(RMatrix *matrix)
{
//	if(LATransformCache.version != udg_get_version(FALSE, TRUE, FALSE, FALSE, FALSE) ||
//		LATransformCache.node_id != udg_get_modeling_node())
		lo_projection_cache_update(matrix);
}


void lo_projection_cache_draw()
{
	float color[3];
	uint32 i, vertex_count;
	return;
	color[0] = f_randf(LATransformCache.updateversion);
	color[1] = f_randf(LATransformCache.updateversion + 1);
	color[2] = f_randf(LATransformCache.updateversion + 2);
	for(i = 0; i < LATransformCache.vertex_count; i++)
	{
		r_primitive_line_2d(LATransformCache.vertex_array[i * 2] + 0.01, 
							LATransformCache.vertex_array[i * 2 + 1], 
							LATransformCache.vertex_array[i * 2] - 0.01, 
							LATransformCache.vertex_array[i * 2 + 1], color[0], color[1], color[2], 1);
		r_primitive_line_2d(LATransformCache.vertex_array[i * 2], 
							LATransformCache.vertex_array[i * 2 + 1] + 0.01, 
							LATransformCache.vertex_array[i * 2], 
							LATransformCache.vertex_array[i * 2 + 1] - 0.01, color[0], color[1], color[2], 1);
	}
	r_primitive_line_flush();
}



boolean p_find_closest_edge_test(float *a, float *b, float x, float y)
{
	float temp, r;
	if((a[0] - b[0]) * (x - b[0]) + (a[1] - b[1]) * (y - b[1]) < 0)
		return FALSE;
	if((b[0] - a[0]) * (x - a[0]) + (b[1] - a[1]) * (y - a[1]) < 0)
		return FALSE;
	if(a[0] > 100000 || b[0] > 100000)
		return FALSE;
	r = sqrt((b[1] - a[1]) * (b[1] - a[1]) + -(b[0] - a[0]) * -(b[0] - a[0]));
	temp = (x - a[0]) * ((b[1] - a[1]) / r) + (y - a[1]) * (-(b[0] - a[0]) / r);
	if(temp > 0.008 || temp < -0.008 || r < 0.0001)
		return FALSE;
	return TRUE;
}


boolean p_find_closest_edge(uint *edge, double *snap, double x, double y)
{
	float *a, *b, *c, *d, pointer_x, pointer_y;
	uint32 vertex_count, ref_count, *ref, i;
	double *vertex;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);
	ref_count *= 4;
	edge[0] = -1;
	edge[1] = -1;
	pointer_x = x;
	pointer_y = y;
	if(LATransformCache.vertex_count < vertex_count)
		vertex_count = LATransformCache.vertex_count;
	for(i = 0; i < ref_count; i += 4)
	{
		if(ref[i] < vertex_count && 
			ref[i + 1] < vertex_count &&  
			ref[i + 2] < vertex_count)
		{			
			a = &LATransformCache.vertex_array[ref[i] * 2];
			b = &LATransformCache.vertex_array[ref[i + 1] * 2];
			c = &LATransformCache.vertex_array[ref[i + 2] * 2];
		//	if((a[0] - b[0]) * (c[1] - b[1]) + (a[1] - b[1]) * (b[0] - c[0]) > 0)
			{
				if(p_find_closest_edge_test(a, b, pointer_x, pointer_y))
				{
					edge[0] = ref[i];
					edge[1] = ref[i + 1];
					break;
				}
				if(p_find_closest_edge_test(b, c, pointer_x, pointer_y))
				{
					edge[0] = ref[i + 1];
					edge[1] = ref[i + 2];
					break;
				}
				if(ref[i + 3] < vertex_count)
				{					
					d = &LATransformCache.vertex_array[ref[i + 3] * 2];
					if(p_find_closest_edge_test(c, d, pointer_x, pointer_y))
					{
						edge[0] = ref[i + 2];
						edge[1] = ref[i + 3];
						break;
					}
					if(p_find_closest_edge_test(a, d, pointer_x, pointer_y))
					{
						edge[0] = ref[i + 3];
						edge[1] = ref[i + 0];
						break;
					}
				}else if(p_find_closest_edge_test(c, a, pointer_x, pointer_y))
				{
					edge[0] = ref[i + 2];
					edge[1] = ref[i + 0];
					break;
				}
			}
		}
	}
	if(edge[0] == -1)
	{
		ref = udg_get_edge_data(&ref_count);
		for(i = 0; i < ref_count; i++)
		{
			a = &LATransformCache.vertex_array[ref[i * 2] * 2];
			b = &LATransformCache.vertex_array[ref[i * 2 + 1] * 2];
			if(p_find_closest_edge_test(a, b, pointer_x, pointer_y))
			{
				edge[0] = ref[i * 2];
				edge[1] = ref[i * 2 + 1];
				break;
			}
		}
	}
	if(edge[0] != -1)
	{
		snap[0] = vertex[edge[0] * 3 + 0];
		snap[1] = vertex[edge[0] * 3 + 1];
		snap[2] = vertex[edge[0] * 3 + 2];
		snap[3] = vertex[edge[1] * 3 + 0];
		snap[4] = vertex[edge[1] * 3 + 1];
		snap[5] = vertex[edge[1] * 3 + 2];
		return TRUE;
	}
	return FALSE;
}

/*

boolean p_find_closest_edge_old(uint *edge, double *snap, double x, double y)
{
	double *vertex;
	double a[3], b[3], c[3], d[3];
	uint32 vertex_count, ref_count, *ref, i;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);
	ref_count *= 4;

	edge[0] = -1;
	edge[1] = -1;

	for(i = 0; i < ref_count; i += 4)
	{
		if(ref[i] < vertex_count && vertex[ref[i] * 3] != V_REAL64_MAX && 
			ref[i + 1] < vertex_count && vertex[ref[i + 1] * 3] != V_REAL64_MAX && 
			ref[i + 2] < vertex_count && vertex[ref[i + 2] * 3] != V_REAL64_MAX)
		{			
			seduce_view_projection_screend(NULL, a, vertex[ref[i] * 3], vertex[ref[i] * 3 + 1], vertex[ref[i] * 3 + 2]);
			seduce_view_projection_screend(NULL, b, vertex[ref[i + 1] * 3], vertex[ref[i + 1] * 3 + 1], vertex[ref[i + 1] * 3 + 2]);
			seduce_view_projection_screend(NULL, c, vertex[ref[i + 2] * 3], vertex[ref[i + 2] * 3 + 1], vertex[ref[i + 2] * 3 + 2]);
			if((a[0] - b[0]) * (c[1] - b[1]) + (a[1] - b[1]) * (b[0] - c[0]) > 0)
			{
				if(p_find_closest_edge_test(a, b, x, y))
				{
					edge[0] = ref[i];
					edge[1] = ref[i + 1];
					break;
				}
				if(p_find_closest_edge_test(b, c, x, y))
				{
					edge[0] = ref[i + 1];
					edge[1] = ref[i + 2];
					break;
				}
				if(ref[i + 3] < vertex_count && vertex[ref[i + 3] * 3] != V_REAL64_MAX)
				{
					seduce_view_projection_screend(NULL, d, vertex[ref[i + 3] * 3], vertex[ref[i + 3] * 3 + 1], vertex[ref[i + 3] * 3 + 2]);
					if(p_find_closest_edge_test(c, d, x, y))
					{
						edge[0] = ref[i + 2];
						edge[1] = ref[i + 3];
						break;
					}
					if(p_find_closest_edge_test(a, d, x, y))
					{
						edge[0] = ref[i + 3];
						edge[1] = ref[i + 0];
						break;
					}
				}else if(p_find_closest_edge_test(c, a, x, y))
				{
					edge[0] = ref[i + 2];
					edge[1] = ref[i + 0];
					break;
				}
			}
		}
	}
	if(edge[0] == -1)
	{
		ref = udg_get_edge_data(&ref_count);
		for(i = 0; i < ref_count; i++)
		{
			seduce_view_projection_screend(NULL, a, vertex[ref[i * 2] * 3], vertex[ref[i * 2] * 3 + 1], vertex[ref[i * 2] * 3 + 2]);
			seduce_view_projection_screend(NULL, b, vertex[ref[i * 2 + 1] * 3], vertex[ref[i * 2 + 1] * 3 + 1], vertex[ref[i * 2 + 1] * 3 + 2]);
			if(p_find_closest_edge_test(a, b, x, y))
			{
				edge[0] = ref[i * 2];
				edge[1] = ref[i * 2 + 1];
				break;
			}
		}
	}
	if(edge[0] != -1)
	{
		snap[0] = vertex[edge[0] * 3 + 0];
		snap[1] = vertex[edge[0] * 3 + 1];
		snap[2] = vertex[edge[0] * 3 + 2];
		snap[3] = vertex[edge[1] * 3 + 0];
		snap[4] = vertex[edge[1] * 3 + 1];
		snap[5] = vertex[edge[1] * 3 + 2];
		return TRUE;
	}
	return FALSE;
}

*/
extern void la_t_poly_compute_normal(double *normal, egreal *vertex, uint *ref);

uint p_find_closest_polygon(BInputState *input, double *mid, double *normal, boolean *selected)
{
	uint i, j, j2, poly, found, ref_count, *ref, vertex_count;
	double *vertex;
	float *temp[4], mouse[2], value;
	boolean output = FALSE;
	udg_get_geometry(&vertex_count, &ref_count, &vertex, &ref, NULL);
	ref_count *= 4;
	if(vertex_count > LATransformCache.vertex_count)
		vertex_count = LATransformCache.vertex_count;
	mouse[0] = input->pointers[0].pointer_x;
	mouse[1] = input->pointers[0].pointer_y;
	for(i = 0; i < ref_count; i += 4)
	{
		if(ref[i] < vertex_count && 
			ref[i + 1] < vertex_count && 
			ref[i + 2] < vertex_count)
		{

			temp[0] = &LATransformCache.vertex_array[ref[i] * 2];
			temp[1] = &LATransformCache.vertex_array[ref[i + 1] * 2];
			temp[2] = &LATransformCache.vertex_array[ref[i + 2] * 2];
			if(ref[i + 3] < vertex_count)
			{
				temp[3] = &LATransformCache.vertex_array[ref[i + 3] * 2];
				poly = 4;
			}else
				poly = 3;
			found = 0;
			for(j = 0; j < poly; j++)
			{
				j2 = (j + 1) % poly;
				value = (mouse[0] - temp[j][0]) * (temp[j][1] - temp[j2][1]) + (mouse[1] - temp[j][1]) * (temp[j2][0] - temp[j][0]);
				if(value >= 0 || temp[j][0] > 100000)
					break;
			/*	value = (temp[j][0] - temp[j2][0]) * (temp[j][0] - temp[j2][0]) + (temp[j][1] - temp[j2][1]) * (temp[j][1] - temp[j2][1]);
				if(value < 0.0001)
					break;*/
			}
			if(j == poly)
			{
				if(mid != NULL)
				{
					mid[0] = 0;
					mid[1] = 0;
					mid[2] = 0;
					for(j = 0; j < poly; j++)
					{
						mid[0] += vertex[ref[i + j] * 3 + 0] / (double)poly;
						mid[1] += vertex[ref[i + j] * 3 + 1] / (double)poly;
						mid[2] += vertex[ref[i + j] * 3 + 2] / (double)poly;
					}
				}
				if(normal != NULL)
					la_t_poly_compute_normal(normal, vertex, &ref[i]);
				if(selected)
				{
					for(j = 0; j < poly; j++)
						if(udg_get_select(ref[i + j]) > 0.01)
							break;
					*selected = j < poly;
				}
				return i / 4;
			}
		}
	}
	return -1;
}


boolean p_find_line_intersect_test(float *a, float *b, float *c, float *d, boolean *del)
{
	float start, end;
	if(a[2] > 100000 || b[1] > 100000)
		return FALSE;
	start = (a[0] - b[0]) * (c[1] - b[1]) + (a[1] - b[1]) * (b[0] - c[0]);
	end = (a[0] - b[0]) * (d[1] - b[1]) + (a[1] - b[1]) * (b[0] - d[0]);
	if((start > 0 && end < 0) || (start < 0 && end > 0))
	{
		start = (c[0] - d[0]) * (a[1] - d[1]) + (c[1] - d[1]) * (d[0] - a[0]);
		end = (c[0] - d[0]) * (b[1] - d[1]) + (c[1] - d[1]) * (d[0] - b[0]);
		if((start > 0 && end < 0) || (start < 0 && end > 0))
		{
			*del = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

boolean p_find_line_intersect(float *start, float *end, void (*func)(uint id))
{
	ENode *node;
	float *vertex, *a, *b, *c, *d;
	boolean del = FALSE;
	uint vertex_length, *ref, ref_length, i;
	udg_get_geometry(&vertex_length, &ref_length, NULL, &ref, NULL);
	if(vertex_length > LATransformCache.vertex_count)
		vertex_length = LATransformCache.vertex_count;
	ref_length *= 4;
	for(i = 0; i < ref_length; i+= 4)
	{
		if(ref[i] < vertex_length && ref[i + 1] < vertex_length && ref[i + 2] < vertex_length)
		{
			a = &LATransformCache.vertex_array[ref[i] * 2];
			b = &LATransformCache.vertex_array[ref[i + 1] * 2];
			c = &LATransformCache.vertex_array[ref[i + 2] * 2];
			if((a[0] - b[0]) * (c[1] - b[1]) + (a[1] - b[1]) * (b[0] - c[0]) > 0)
			{
				if(ref[i + 3] > vertex_length)
				{			
					if(p_find_line_intersect_test(a, b, start, end, &del) ||
						p_find_line_intersect_test(b, c, start, end, &del) || 
						p_find_line_intersect_test(c, a, start, end, &del))
						func(i / 4);
				}
				else
				{
					d = &LATransformCache.vertex_array[ref[i + 2] * 2];
					if(p_find_line_intersect_test(a, b, start, end, &del) ||
						p_find_line_intersect_test(b, c, start, end, &del) || 
						p_find_line_intersect_test(b, d, start, end, &del) || 
						p_find_line_intersect_test(d, a, start, end, &del))
						func(i / 4);
				}
			}
		}
	}
	ref = udg_get_edge_data(&i);
	while(i != 0)
	{
		i--;
		a = &LATransformCache.vertex_array[ref[i * 2] * 2];
		b = &LATransformCache.vertex_array[ref[i * 2 + 1] * 2];
	/*	if(draw_line_delete_test(vertex, a, b, &del))
			la_do_edge_delete_air(a, b);*/
	}
	return del;
}



uint p_find_click_tag(double x, double y)
{
	uint32 count, i, output = -1;
	double r, pos[3];
	UNDOTag	*tag;
	tag = udg_get_tags(&count);
	for(i = 0; i < count; i++)
	{
		seduce_view_projection_screend(NULL, pos, tag[i].vec[0], tag[i].vec[1], tag[i].vec[2]);
		if(pos[2] < 0)
		{
			pos[0] += x;
			pos[1] += y;
			r = pos[0] * pos[0] + pos[1] * pos[1];
			if(r < 0.001)
			{
				output = i;
			}
		}
	}
	return output;
}

boolean p_find_click_tag_lable(double x, double y)
{
	uint32 count, i;
	double f, f2, pos[3];
	UNDOTag	*tag;
	tag = udg_get_tags(&count);
	for(i = 0; i < count; i++)
	{
		seduce_view_projection_screend(NULL, pos, tag[i].vec[0], tag[i].vec[1], tag[i].vec[2]);
		if(pos[2] < 0 && pos[1] - 0.02 < -y && pos[1] + 0.02 > -y)
		{
			f = seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, tag[i].group, -1);
			f2 = seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, tag[i].tag, -1);
			if(pos[0] > -x && pos[0] - (f + f2 + 0.06) < -x)

			
//			if(sui_box_click_test(pos[0] - f + 0.04, pos[1] + 0.03, -f - 0.04 , -0.06))
		//	if(sui_box_click_test(pos[0] - f + 0.04, pos[1] + 0.03, -f - 0.04 , -0.06))
			{
				udg_rename_tag(i);
				return TRUE;
			}
/*			if(sui_box_click_test(pos[0] + f + 0.04, pos[1] + 0.03, sui_compute_text_length(SEDUCE_T_SIZE, SEDUCE_T_SPACE, tag[i].tag) + 0.02 , -0.06))
			{
				exit(0);
				udg_rename_tag_group(i);
				return TRUE;
			}*/
		}
	}
	return FALSE;
}

boolean p_find_closest_vertex(uint *closest, uint *selected, double *distance, double *selected_distance, double x, double y, boolean include_selected)
{
	uint32 vertex_count, i;
	float pointer_x, pointer_y, pos_x, pos_y, r;
	boolean closest_found = FALSE;
	pointer_x = (float)x;
	pointer_y = (float)y;
	vertex_count = LATransformCache.vertex_count * 2;
	for(i = 0; i < vertex_count; i += 2)
	{
		pos_x = LATransformCache.vertex_array[i] - x;
		pos_y  = LATransformCache.vertex_array[i + 1] - y;
		r = pos_x * pos_x + pos_y * pos_y;
		if(r < *selected_distance && r >= 0.0)
		{
			*selected_distance = r;
			*selected = i / 2;
			if(r <= *distance && (include_selected || udg_get_select(i / 2) < 0.001))
			{
				*distance = r;
				*closest = i / 2;
				closest_found = TRUE;
			}
		}
	}
	return closest_found;
}
/*
boolean p_find_closest_vertex_old(uint *closest, uint *selected, double *distance, double *selected_distance, double x, double y, boolean include_selected)
{
	uint32 vertex_count, i;
	double	*vertex;
	double	pos[3], r;
	boolean closest_found = FALSE;
	udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);

	for(i = 0; i < vertex_count; i++)
	{
		if(vertex[i * 3] != V_REAL64_MAX)
		{
			seduce_view_projection_screend(NULL, pos, vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);
			if(pos[2] < 0)
			{
				pos[0] -= x;
				pos[1] -= y;
				r = pos[0] * pos[0] + pos[1] * pos[1];
				if(r < *selected_distance && r >= 0.0)
				{
					*selected_distance = r;
					*selected = i;
					if(r <= *distance && (include_selected || udg_get_select(i) < 0.001))
					{
						*distance = r;
						*closest = i;
						closest_found = TRUE;
					}
				}
			}
		}
	}
	return closest_found;
}*/

void p_set_view_center(double *center)
{
	uint32 vertex_count, polygon_count, *ref, i;
	double *vertex;
	boolean rotate = FALSE;
	double normal[3] = {0, 0, 0}, temp, size = 0.001;
	seduce_view_center_set(NULL, center[0], center[1], center[2]);
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, NULL);
	for(i = 0; i < polygon_count; i++)
	{
		if(ref[i * 4] < vertex_count && vertex[ref[i * 4] * 3] != V_REAL64_MAX && 0.01 < udg_get_select(ref[i * 4]) && ref[i * 4 + 1] < vertex_count && vertex[ref[i * 4 + 1] * 3] != V_REAL64_MAX && 0.01 < udg_get_select(ref[i * 4 + 1]) && ref[i * 4 + 2] < vertex_count  && vertex[ref[i * 4 + 2] * 3] != V_REAL64_MAX && 0.01 < udg_get_select(ref[i * 4 + 2]) && (ref[i * 4 + 3] >= vertex_count || vertex[ref[i * 4 + 3] * 3] == V_REAL64_MAX || 0.01 < udg_get_select(ref[i * 4 + 3])))
		{	
			normal[0] += ((vertex[ref[i * 4] * 3 + 1] - vertex[ref[i * 4 + 1] * 3 + 1]) * (vertex[ref[i * 4 + 2] * 3 + 2] - vertex[ref[i * 4 + 1] * 3 + 2]) - (vertex[ref[i * 4] * 3 + 2] - vertex[ref[i * 4 + 1] * 3 + 2]) * (vertex[ref[i * 4 + 2] * 3 + 1] - vertex[ref[i * 4 + 1] * 3 + 1]));
			normal[1] += ((vertex[ref[i * 4] * 3 + 2] - vertex[ref[i * 4 + 1] * 3 + 2]) * (vertex[ref[i * 4 + 2] * 3 + 0] - vertex[ref[i * 4 + 1] * 3 + 0]) - (vertex[ref[i * 4] * 3 + 0] - vertex[ref[i * 4 + 1] * 3 + 0]) * (vertex[ref[i * 4 + 2] * 3 + 2] - vertex[ref[i * 4 + 1] * 3 + 2]));
			normal[2] += ((vertex[ref[i * 4] * 3 + 0] - vertex[ref[i * 4 + 1] * 3 + 0]) * (vertex[ref[i * 4 + 2] * 3 + 1] - vertex[ref[i * 4 + 1] * 3 + 1]) - (vertex[ref[i * 4] * 3 + 1] - vertex[ref[i * 4 + 1] * 3 + 1]) * (vertex[ref[i * 4 + 2] * 3 + 0] - vertex[ref[i * 4 + 1] * 3 + 0]));
			rotate = TRUE;
		}
	}
	if(rotate != TRUE)
	{
		for(i = 0; i < polygon_count; i++)
		{
			if(ref[i * 4] < vertex_count && vertex[ref[i * 4] * 3] != V_REAL64_MAX && ref[i * 4 + 1] < vertex_count && vertex[ref[i * 4 + 1] * 3] != V_REAL64_MAX && ref[i * 4 + 2] < vertex_count && vertex[ref[i * 4 + 2] * 3] != V_REAL64_MAX)
			{	
				if(0.01 < udg_get_select(ref[i * 4]) || 0.01 < udg_get_select(ref[i * 4 + 1]) || 0.01 < udg_get_select(ref[i * 4 + 2]) || (ref[i * 4 + 3] < vertex_count && vertex[ref[i * 4 + 3] * 3] != V_REAL64_MAX && 0.01 < udg_get_select(ref[i * 4 + 3])))
				{
					normal[0] += ((vertex[ref[i * 4] * 3 + 1] - vertex[ref[i * 4 + 1] * 3 + 1]) * (vertex[ref[i * 4 + 2] * 3 + 2] - vertex[ref[i * 4 + 1] * 3 + 2]) - (vertex[ref[i * 4] * 3 + 2] - vertex[ref[i * 4 + 1] * 3 + 2]) * (vertex[ref[i * 4 + 2] * 3 + 1] - vertex[ref[i * 4 + 1] * 3 + 1]));
					normal[1] += ((vertex[ref[i * 4] * 3 + 2] - vertex[ref[i * 4 + 1] * 3 + 2]) * (vertex[ref[i * 4 + 2] * 3 + 0] - vertex[ref[i * 4 + 1] * 3 + 0]) - (vertex[ref[i * 4] * 3 + 0] - vertex[ref[i * 4 + 1] * 3 + 0]) * (vertex[ref[i * 4 + 2] * 3 + 2] - vertex[ref[i * 4 + 1] * 3 + 2]));
					normal[2] += ((vertex[ref[i * 4] * 3 + 0] - vertex[ref[i * 4 + 1] * 3 + 0]) * (vertex[ref[i * 4 + 2] * 3 + 1] - vertex[ref[i * 4 + 1] * 3 + 1]) - (vertex[ref[i * 4] * 3 + 1] - vertex[ref[i * 4 + 1] * 3 + 1]) * (vertex[ref[i * 4 + 2] * 3 + 0] - vertex[ref[i * 4 + 1] * 3 + 0]));
					rotate = TRUE;
				}
			}
		}
	}
	if(rotate == TRUE)
		seduce_view_direction_set(NULL, normal[0], normal[1], normal[2]);

	for(i = 0; i < vertex_count; i++)
	{
		if(vertex[i * 3] != V_REAL64_MAX  && 0.01 < udg_get_select(i))
		{	
			temp = (vertex[i * 3] - center[0]) * (vertex[i * 3] - center[0]) + (vertex[i * 3 + 1] - center[1]) * (vertex[i * 3 + 1] - center[1]) + (vertex[i * 3 + 2] - center[2]) * (vertex[i * 3 + 2] - center[2]);
			if(temp > size)
				size = temp;
		}
	}
	size = sqrt(size) * 2;
	seduce_view_distance_camera_set(NULL, size);
}


/*


void seduce_view_direction_set(SViewData *v, double normal_x, double normal_y, double normal_z)
{
	double f;
	if(v == NULL)
		v = &sui_default_view;
	f = 360 * atan2(-normal_x, normal_z) / PI * 2.0;
	if(f > 0 &&  v->yaw < f - 180.0)
		v->yaw += 360;
	if(f < 0 &&  v->yaw > f + 180.0)
		v->yaw -= 360;
	v->yaw_target = temp;
	temp = (normal_x * normal_x) + (normal_y * normal_y) + (normal_z * normal_z);
	if(0.0 != temp)
		v->pitch_target = 360.0 * atan(normal_y / sqrt(f)) / (PI * 2.0);
}

void 
*/


double la_axis_matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
boolean la_axis_visible = FALSE;

void la_view_axis_matrix_poly_set(uint polygon, uint edge) 
{
	uint32 vertex_count, polygon_count, *ref;
	double *vertex;
	la_axis_visible = TRUE;
	udg_get_geometry(&vertex_count, &polygon_count, &vertex, &ref, NULL);
	if(polygon >= polygon_count)
		return;
	if(ref[polygon * 4 + 3] > vertex_count || vertex[ref[polygon * 4 + 3] * 3] == V_REAL64_MAX)
	{
		f_matrixxyd(la_axis_matrix, &vertex[ref[polygon * 4 + edge] * 3],
									&vertex[ref[polygon * 4 + (edge + 1) % 3] * 3],
									&vertex[ref[polygon * 4 + (edge + 2) % 3] * 3]);
	}else
	{
		f_matrixxyd(la_axis_matrix, &vertex[ref[polygon * 4 + edge] * 3],
									&vertex[ref[polygon * 4 + (edge + 1) % 4] * 3],
									&vertex[ref[polygon * 4 + (edge + 3) % 4] * 3]);
	}
}

void la_view_axis_matrix_edge_set(uint edge_a, uint edge_b) 
{
	uint result;
	la_axis_visible = TRUE;
	result = la_t_edge_fill_find_edge_poly(edge_a, edge_b, FALSE);
	if(result != -1)
		la_view_axis_matrix_poly_set(result / 4, result % 4); 
}


void la_view_axis_matrix_reset() 
{
	la_axis_visible = FALSE;
	la_axis_matrix[0] = 1;
	la_axis_matrix[1] = 0;
	la_axis_matrix[2] = 0;
	la_axis_matrix[3] = 0;
	la_axis_matrix[4] = 0;
	la_axis_matrix[5] = 1;
	la_axis_matrix[6] = 0;
	la_axis_matrix[7] = 0;
	la_axis_matrix[8] = 0;
	la_axis_matrix[9] = 0;
	la_axis_matrix[10] = 1;
	la_axis_matrix[11] = 0;
	la_axis_matrix[12] = 0;
	la_axis_matrix[13] = 0;
	la_axis_matrix[14] = 0;
	la_axis_matrix[15] = 1;
}

extern void lo_pfx_wiggle_line_add(float ax, float ay, float az, float bx, float by, float bz, boolean major);


void la_view_projection_vertex_with_axis(BInputState *input, RMatrix *matrix, double *output, double *start, double pointer_x, double pointer_y, boolean snap, double *closest, SUISnapType snap_type)
{
	static uint best_axis = 0;
	static double direction = 1;
	double pos[3], dout[3];
	float out[3], v[3], screen[3], f, best = 10000000;
	uint i;
	v[0] = start[0];
	v[1] = start[1];
	v[2] = start[2];
	r_matrix_projection_vertexf(&la_world_matrix, out, v, pointer_x, pointer_y);
	output[0] = out[0];
	output[1] = out[1];
	output[2] = out[2];

	if(!snap || snap_type == SUI_ST_NONE)
	{
		for(i = 0; i < 3; i++)
		{
			float axis[3];
			axis[0] = la_axis_matrix[i * 4];
			axis[1] = la_axis_matrix[i * 4 + 1];
			axis[2] = la_axis_matrix[i * 4 + 2];
			r_matrix_projection_vectorf(&la_world_matrix, out, v, axis, pointer_x, pointer_y);
			r_matrix_projection_screenf(&la_world_matrix, screen, out[0], out[1], out[2]);
			screen[0] -= pointer_x;
			screen[1] -= pointer_y;
			f = screen[0] * screen[0] + screen[1] * screen[1];
			if(0.01 * 0.01 > f)
			{
				output[0] = out[0];
				output[1] = out[1];
				output[2] = out[2];
			}
			if(f < best)
			{
				best = f;
				best_axis = i;
				

				f_transforminv3d(dout, la_axis_matrix, output[0], output[1], output[2]);
				f_transforminv3d(pos, la_axis_matrix, start[0], start[1], start[2]);
				if(pos[i] < dout[i])
					direction = 1;
				else
					direction = -1;
			}
		}
	}else
	{
		float fpos[3];
		if(snap_type == SUI_ST_VERTEX_CLOSE)
			fpos[0] = 0;
		switch(snap_type)
		{
			case SUI_ST_VERTEX_FAR :
			case SUI_ST_VERTEX_CLOSE :
				f_transforminv3d(dout, la_axis_matrix, closest[0], closest[1], closest[2]);
				f_transforminv3d(pos, la_axis_matrix, start[0], start[1], start[2]);
				pos[best_axis] = dout[best_axis];
				f_transform3d(output, la_axis_matrix, pos[0], pos[1], pos[2]);
			break;
			case SUI_ST_LINE :
				f_transforminv3d(dout, la_axis_matrix, out[0], out[1], out[2]);
				f_transforminv3d(pos, la_axis_matrix, start[0], start[1], start[2]);
				pos[best_axis] += direction * sqrt((closest[0] - closest[3]) * (closest[0] - closest[3]) +
													(closest[1] - closest[4]) * (closest[1] - closest[4]) +
													(closest[2] - closest[5]) * (closest[2] - closest[5]));
				f_transform3d(output, la_axis_matrix, pos[0], pos[1], pos[2]);
			break;
			case SUI_ST_TANGENT :
			{
				double  normal[3], vector[5] = {0, 0, 1, 0, 0}, t_start[3];
				f_transforminv3d(pos, la_axis_matrix, closest[0], closest[1], closest[2]);
				f_transforminv3d(normal, la_axis_matrix, closest[3], closest[4], closest[5]);
				f_transforminv3d(t_start, la_axis_matrix, start[0], start[1], start[2]);
				dout[0] = t_start[0];
				dout[1] = t_start[1];
				dout[2] = t_start[2];
				f_project3d(output, pos, normal, t_start, &vector[2 - best_axis]);
				dout[best_axis] = output[best_axis];
				
				f_transform3d(output, la_axis_matrix, dout[0], dout[1], dout[2]);
			}
			break;
		}
		lo_pfx_draw_snap(input, snap_type, closest, output, !input->pointers[0].button[0]);
	}
}
