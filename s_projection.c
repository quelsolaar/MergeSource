#include <math.h>
#include <stdlib.h>
#include "seduce.h"

/*
void point_threw_matrix(float *matrix, float *vertex)
*/
extern void point_threw_matrix4(double *matrix, double *pos_x, double *pos_y, double *pos_z, double *pos_w);
extern void point_threw_matrix3(double *matrix, double *pos_x, double *pos_y, double *pos_z);


extern void matrix_rotate_x(double *matrix, double degree);
extern void matrix_rotate_y(double *matrix, double degree);
extern void matrix_rotate_z(double *matrix, double degree);
extern void transform_matrix(double *matrix, double x, double y, double z);
extern void scale_matrix(double *matrix, double x, double y, double z);
extern void negate_matrix(double *matrix);
extern void reverse_matrix(double *matrix);
extern void matrix_print(const double *matrix);

SViewData sui_default_view;

typedef enum{
	S_VM_CAMERA,
	S_VM_ORBIT,
	S_VM_ROTATE,
	S_VM_PAN,
	S_VM_DISTANCE,
	S_VM_SCROLL,
	S_VM_COUNT
}SViewMode;


void seduce_view_interpolation_style_set(SViewData *v, SViewInterpolationStyle style, float speed)
{
	if(v == NULL)
		v = &sui_default_view;
	v->interpolation_style = style;
	v->speed = speed;
}


void seduce_view_look_at_matrix(SViewData *v, float *target, float *camera, float *up)
{
	float axis[3] = {0, 1, 0}, tmp[3], m[16], f;
	if(up == NULL)
		up = axis;
	tmp[0] = target[0] - camera[0];
	tmp[1] = target[1] - camera[1];
	tmp[2] = target[2] - camera[2];
	f_matrixzyf(v->model, NULL, tmp, up);
	f = v->model[1];
	v->model[1] = v->model[4];
	v->model[4] = f;
	f = v->model[2];
	v->model[2] = v->model[8];
	v->model[8] = f;
	f = v->model[6];
	v->model[6] = v->model[9];
	v->model[9] = f;
	v->model[12] = v->model[0] * camera[0] + v->model[4] * camera[1] + v->model[8] * camera[2];
	v->model[13] = v->model[1] * camera[0] + v->model[5] * camera[1] + v->model[9] * camera[2];
	v->model[14] = v->model[2] * camera[0] + v->model[6] * camera[1] + v->model[10] * camera[2];
	v->model[15] = 1.0;
	v->distance = sqrt(tmp[0] * tmp[0] + tmp[1] * tmp[1] + tmp[2] * tmp[2]);
	v->camera[0] = camera[0];
	v->camera[1] = camera[1];
	v->camera[2] = camera[2];
	v->target[0] = target[0];
	v->target[1] = target[1];
	v->target[2] = target[2];
}

void seduce_view_orbit_matrix(SViewData *v, float x, float y)
{
	float vec[3], up[3], f;
	vec[0] = v->model[0] * x + v->model[4] * y;
	vec[1] = v->model[1] * x + v->model[5] * y;
	vec[2] = v->model[2] * x + v->model[6] * y;
	up[0] = v->model[1];
	up[1] = v->model[5];
	up[2] = v->model[9];
	vec[0] = vec[0] * -5.0 - v->target[0] + v->camera[0];
	vec[1] = vec[1] * -5.0 - v->target[1] + v->camera[1];
	vec[2] = vec[2] * -5.0 - v->target[2] + v->camera[2];	
	f = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]) / v->distance;
	vec[0] = v->target[0] + vec[0] / f; 
	vec[1] = v->target[1] + vec[1] / f; 
	vec[2] = v->target[2] + vec[2] / f;
	seduce_view_look_at_matrix(v, v->target, vec, up);
}

void seduce_view_pan_matrix(SViewData *v, float x, float y)
{
	float vec[3], up[3], f;
	vec[0] = v->model[0] * x + v->model[1] * y;
	vec[1] = v->model[4] * x + v->model[5] * y;
	vec[2] = v->model[8] * x + v->model[9] * y;
	up[0] = v->model[1];
	up[1] = v->model[5];
	up[2] = v->model[9];
	v->camera[0] -= vec[0];
	v->camera[1] -= vec[1];
	v->camera[2] -= vec[2];
	v->target[0] -= vec[0];
	v->target[1] -= vec[1];
	v->target[2] -= vec[2];
	seduce_view_look_at_matrix(v, v->target, v->camera, up);
}


void seduce_view_distance_matrix(SViewData *v, float distance)
{
	float camera[3], up[3];
	camera[0] = v->target[0] + (v->camera[0] - v->target[0]) / v->distance * distance;
	camera[1] = v->target[1] + (v->camera[1] - v->target[1]) / v->distance * distance;
	camera[2] = v->target[2] + (v->camera[2] - v->target[2]) / v->distance * distance;
	up[0] = v->model[1];
	up[1] = v->model[5];
	up[2] = v->model[9];
	seduce_view_look_at_matrix(v, v->target, camera, up);
}


void seduce_view_rotate_matrix(SViewData *v, float x, float y)
{
	float camera[3], up[3], f;
	x *= 1.0;
	y *= 1.0;
/*	if(y > PI * 0.499)
		y = PI * 0.499;
	if(y < -PI * 0.499)
		y = -PI * 0.499;*/
	f = cos(y);
	camera[0] = v->target[0] + cos(x) * f * v->distance;
	camera[1] = v->target[1] + sin(y) * v->distance;
	camera[2] = v->target[2] + sin(x) * f * v->distance;
	seduce_view_look_at_matrix(v, v->target, camera, NULL);
}

void seduce_view_init(SViewData *v)
{
	float target[3] = {0, 0, 0}, camera[3] = {0, 0, -1};
	if(v == NULL)
		v = &sui_default_view;
	seduce_view_look_at_matrix(v, target, camera, NULL);
	v->interpolation_style = S_VIS_SMOOTH;
	v->speed = 1.0;
	v->distance = 1;
	v->type = -1;
}

void seduce_view_slide(SViewData *v, boolean slide)
{	
	if(v == NULL)
		v = &sui_default_view;
	v->data.rotate.slide = slide;
}

void seduce_view_update(SViewData *v, float delta)
{
	float f, tmp[3], progress, d;
	if(v == NULL)
		v = &sui_default_view;
	switch(v->type)
	{
		case S_VM_CAMERA :
			if(v->data.camera.progress < 1.0)
			{
				delta *= v->speed;
				switch(v->interpolation_style)
				{
					case S_VIS_LINEAR :
					progress =  v->data.camera.progress;
					f = delta;
					break;
					case S_VIS_EASE_IN :
					progress = v->data.camera.progress * v->data.camera.progress;
					f = (v->data.camera.progress + delta) * (v->data.camera.progress + delta);
					f -= progress;
					break;
					case S_VIS_EASE_OUT :
					progress = 1.0 - (1.0 - v->data.camera.progress) * (1.0 - v->data.camera.progress);
					f = 1.0 - (1.0 - (v->data.camera.progress + delta)) * (1.0 - (v->data.camera.progress + delta));
					f -= progress;
					break;
					case S_VIS_SMOOTH :
					progress = f_smooth_stepf(v->data.camera.progress);
					f = f_smooth_stepf(v->data.camera.progress + delta);
					f -= progress;
					break;
					case S_VIS_CUT :
					progress = 0;
					f = 1;
					break;
					default :
					progress = 0;
					f = 1;
					break;
				}
				if(f + progress > 1.0)
					f = 1.0 - progress;
				f = f / (1.0 - progress);
				if(f > 1.0 || progress > 0.99)
					f = 1.0;

				v->camera[0] += (v->data.camera.target_camera[0] - v->camera[0]) * f;
				v->camera[1] += (v->data.camera.target_camera[1] - v->camera[1]) * f;
				v->camera[2] += (v->data.camera.target_camera[2] - v->camera[2]) * f;

				v->target[0] += (v->data.camera.target_target[0] - v->target[0]) * f;
				v->target[1] += (v->data.camera.target_target[1] - v->target[1]) * f;
				v->target[2] += (v->data.camera.target_target[2] - v->target[2]) * f;
/*
				v->target[0] = v->data.camera.target_target[0];
				v->target[1] = v->data.camera.target_target[1];
				v->target[2] = v->data.camera.target_target[2];
*/
		/*		tmp[0] = v->matrix[4] + (v->data.camera.target_up[0] - v->matrix[4]) * f;
				tmp[1] = v->matrix[5] + (v->data.camera.target_up[1] - v->matrix[5]) * f;
				tmp[2] = v->matrix[6] + (v->data.camera.target_up[2] - v->matrix[6]) * f;*/

				tmp[0] = 0;
				tmp[1] = 1;
				tmp[2] = 0;
				v->data.camera.progress += delta;
				seduce_view_look_at_matrix(v, v->target, v->camera, tmp);
			}
		break;
		case S_VM_ORBIT :
			seduce_view_orbit_matrix(v, v->data.orbit.delta_rot[0] * delta, v->data.orbit.delta_rot[1] * delta);
		break;
		case S_VM_ROTATE :
			v->data.rotate.current_rot[0] += v->data.rotate.delta_rot[0] * delta; 
			v->data.rotate.current_rot[1] += v->data.rotate.delta_rot[1] * delta;
			if(!v->data.rotate.slide)
				v->data.rotate.delta_rot[0] = v->data.rotate.delta_rot[1] = 0;
			if(v->data.rotate.current_rot[1] > PI * 0.4999)
			{
				v->data.rotate.current_rot[1] = PI * 0.4999;
				if(v->data.rotate.slide)
					v->data.rotate.delta_rot[1] = -v->data.rotate.delta_rot[1];
			}
			if(v->data.rotate.current_rot[1] < PI * -0.4999)
			{
				v->data.rotate.current_rot[1] = PI * -0.4999;
				if(v->data.rotate.slide)
					v->data.rotate.delta_rot[1] = -v->data.rotate.delta_rot[1];
			}
			seduce_view_rotate_matrix(v, v->data.rotate.current_rot[0], v->data.rotate.current_rot[1]);
		break;
		case S_VM_PAN :
			seduce_view_pan_matrix(v, v->data.pan.delta_pan[0] * delta, v->data.pan.delta_pan[1] * delta);
		break;
		case S_VM_DISTANCE :
			seduce_view_distance_matrix(v, (1.0 + 2.0 * (v->data.distance.delta_distance * delta)) * v->distance);
		break;
		case S_VM_SCROLL :
			if(v->data.camera.progress < 1.0)
			{
				delta *= 8.0;
				progress =  v->data.scroll.progress;
				f = delta;
				if(f + progress > 1.0)
					f = 1.0 - progress;
				f = f / (1.0 - progress);
				if(f > 1.0 || progress > 0.99)
					f = 1.0;
				v->camera[0] += (v->data.scroll.target_camera[0] - v->camera[0]) * f;
				v->camera[1] += (v->data.scroll.target_camera[1] - v->camera[1]) * f;
				v->camera[2] += (v->data.scroll.target_camera[2] - v->camera[2]) * f;
				v->target[0] += (v->data.scroll.target_target[0] - v->target[0]) * f;
				v->target[1] += (v->data.scroll.target_target[1] - v->target[1]) * f;
				v->target[2] += (v->data.scroll.target_target[2] - v->target[2]) * f;
				tmp[0] = 0;
				tmp[1] = 1;
				tmp[2] = 0;
				v->data.scroll.progress += delta;
				seduce_view_look_at_matrix(v, v->target, v->camera, tmp);
			}
		break;

	}
}

void seduce_view_change_look_at(SViewData *v, float *target, float *camera, float *up)
{
	if(v == NULL)
		v = &sui_default_view;
	v->type = S_VM_CAMERA;
	v->data.camera.progress = 1.0;
/*	v->camera[0] = v->data.camera.target_camera[0] = camera[0];
	v->camera[1] = v->data.camera.target_camera[1] = camera[1];
	v->camera[2] = v->data.camera.target_camera[2] = camera[2];
	v->target[0] = v->data.camera.target_target[0] = target[0];
	v->target[1] = v->data.camera.target_target[1] = target[1];
	v->target[2] = v->data.camera.target_target[2] = target[2];*/
	v->data.camera.target_camera[0] = camera[0];
	v->data.camera.target_camera[1] = camera[1];
	v->data.camera.target_camera[2] = camera[2];
	v->data.camera.target_target[0] = target[0];
	v->data.camera.target_target[1] = target[1];
	v->data.camera.target_target[2] = target[2];
	if(up == NULL)
	{
		v->data.camera.target_up[0] = 0;
		v->data.camera.target_up[1] = 1;
		v->data.camera.target_up[2] = 0;
	}else
	{
		v->data.camera.target_up[0] = up[0];
		v->data.camera.target_up[1] = up[1];
		v->data.camera.target_up[2] = up[2];
	}
	seduce_view_look_at_matrix(v, target, camera, up);
}


void seduce_view_change_orbit(SViewData *v, BInputState *input, uint button, boolean slide)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f;
	if(v == NULL)
		v = &sui_default_view;	
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[button] || input->pointers[i].last_button[button])
		{
			pointer = i;
			count++;
		}
	}
	if(count == 1)
	{
		if(v->type == S_VM_CAMERA)
			seduce_view_update(v, 10000.0);
		v->type = S_VM_ORBIT;
		seduce_view_orbit_matrix(v, input->pointers[pointer].delta_pointer_x, input->pointers[pointer].delta_pointer_y);
		v->data.orbit.delta_rot[0] = v->data.orbit.delta_rot[1] = 0.0;
		if(!input->pointers[pointer].button[button] && slide)
		{
			v->data.orbit.delta_rot[0] = input->pointers[pointer].delta_pointer_x / input->delta_time;
			v->data.orbit.delta_rot[1] = input->pointers[pointer].delta_pointer_y / input->delta_time;
		}
	}
}

void seduce_view_change_orbit_delta(SViewData *v, BInputState *input, uint button, boolean slide)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f;
	if(v == NULL)
		v = &sui_default_view;	
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[button] || input->pointers[i].last_button[button])
		{
			pointer = i;
			count++;
		}
	}
	if(count == 1)
	{

		if(v->type == S_VM_CAMERA)
			seduce_view_update(v, 10000.0);
		v->type = S_VM_ORBIT;
		v->data.orbit.delta_rot[0] += input->pointers[pointer].delta_pointer_x;
		v->data.orbit.delta_rot[1] += input->pointers[pointer].delta_pointer_y;
		if(!input->pointers[pointer].button[button] && !slide)
			v->data.orbit.delta_rot[0] = v->data.orbit.delta_rot[1] = 0.0;

	}
}

void seduce_view_orbit(SViewData *v, float x, float y)
{
	if(v == NULL)
		v = &sui_default_view;	
	seduce_view_orbit_matrix(v, x, y);
}

void seduce_view_rotate(SViewData *v, BInputState *input, float x, float y)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f, f2;
	if(v == NULL)
		v = &sui_default_view;	
	if(v->type == S_VM_CAMERA)
		seduce_view_update(v, 10000.0);
	if(v->type != S_VM_ROTATE)
	{
		vec[0] = v->camera[0] - v->target[0];
		vec[1] = v->camera[1] - v->target[1];
		vec[2] = v->camera[2] - v->target[2];
		v->data.rotate.current_rot[0] = atan2(vec[2], vec[0]);
		v->data.rotate.current_rot[1] = tan(vec[1] / f_length3f(vec));
		v->data.rotate.delta_rot[0] = 0.0;
		v->data.rotate.delta_rot[1] = 0.0;
		v->type = S_VM_ROTATE;
	}

	v->data.rotate.delta_rot[0] = /*v->data.rotate.delta_rot[0] * 0.5 +*/ x / input->delta_time;
	v->data.rotate.delta_rot[1] = /*v->data.rotate.delta_rot[1] * 0.5 +*/ y / input->delta_time;
}

//seduce_view_rotate_matrix(SViewData *v, float x, float y)

void seduce_view_change_rotate(SViewData *v, BInputState *input, uint button, boolean slide)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f, f2;
	if(v == NULL)
		v = &sui_default_view;	
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[button] || input->pointers[i].last_button[button])
		{
			pointer = i;
			count++;
		}
	}
	if(count == 1)
	{
		seduce_view_rotate(v, input, input->pointers[pointer].delta_pointer_x, input->pointers[pointer].delta_pointer_y);
	/*	if(v->type == S_VM_CAMERA)
			seduce_view_update(v, 10000.0);
		if(v->type != S_VM_ROTATE)
		{
			vec[0] = v->camera[0] - v->target[0];
			vec[1] = v->camera[1] - v->target[1];
			vec[2] = v->camera[2] - v->target[2];
			v->data.rotate.current_rot[0] = atan2(vec[2], vec[0]);
			v->data.rotate.current_rot[1] = tan(vec[1] / f_length3f(vec));
			v->data.rotate.delta_rot[0] = 0.0;
			v->data.rotate.delta_rot[1] = 0.0;
			v->data.rotate.slide = slide;
			v->type = S_VM_ROTATE;
		}
		if(!input->pointers[pointer].last_button[button])
			v->data.rotate.delta_rot[0] = v->data.rotate.delta_rot[1] = 0.0;

		v->data.rotate.delta_rot[0] = v->data.rotate.delta_rot[0] * 0.5 + input->pointers[pointer].delta_pointer_x * 1.0 / input->delta_time;
		v->data.rotate.delta_rot[1] = v->data.rotate.delta_rot[1] * 0.5 + input->pointers[pointer].delta_pointer_y * 1.0 / input->delta_time;
		if(!input->pointers[pointer].button[button])
			v->data.rotate.delta_rot[0] = v->data.rotate.delta_rot[1] = 0.0;*/
	}
}

void seduce_view_change_mouse_look(SViewData *v, BInputState *input)
{
	uint i, pointer, count = 0;
	float vec[3], target[3], camera[3], x, y, f;
	if(v == NULL)
		v = &sui_default_view;	

	v->type = -1;
	if(input->pointers[0].delta_pointer_x > 0.001 ||
			input->pointers[0].delta_pointer_y > 0.001 ||
			input->pointers[0].delta_pointer_x < -0.001 ||
			input->pointers[0].delta_pointer_y < -0.001)
	{
		vec[0] = v->target[0] - v->camera[0];
		vec[1] = v->target[1] - v->camera[1];
		vec[2] = v->target[2] - v->camera[2];
		x = atan2(vec[2] / v->distance, vec[0] / v->distance) + input->pointers[0].delta_pointer_x;
		y = tan(vec[1] / f_length3f(vec)) - input->pointers[0].delta_pointer_y;
		if(y > 0.4999 * PI)
			y = 0.4999 * PI;
		if(y < -0.4999 * PI)
			y = -0.4999 * PI;
		f = cos(y);
		target[0] = v->camera[0] + cos(x) * f * v->distance;
		target[1] = v->camera[1] + sin(y) * v->distance;
		target[2] = v->camera[2] + sin(x) * f * v->distance;
		seduce_view_look_at_matrix(v, target, v->camera, NULL);
	}
}


void seduce_view_change_rotate_delta(SViewData *v, BInputState *input, uint button, boolean slide)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f, f2;
	if(v == NULL)
		v = &sui_default_view;	
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[button] || input->pointers[i].last_button[button])
		{
			pointer = i;
			count++;
		}
	}
	if(count == 1)
	{
		if(v->type == S_VM_CAMERA)
			seduce_view_update(v, 10000.0);
		v->type = S_VM_ROTATE;
		if(!input->pointers[pointer].last_button[button])
		{
			vec[0] = v->camera[0] - v->target[0];
			vec[1] = v->camera[1] - v->target[1];
			vec[2] = v->camera[2] - v->target[2];
			v->data.rotate.current_rot[0] = atan2(vec[2], vec[0]);
			v->data.rotate.current_rot[1] = tan(vec[1] / f_length3f(vec));
			v->data.rotate.delta_rot[0] = 0.0;
			v->data.rotate.delta_rot[1] = 0.0;
		}
		v->data.rotate.delta_rot[0] += input->pointers[pointer].delta_pointer_x * 0.1 / input->delta_time;
		v->data.rotate.delta_rot[1] -= input->pointers[pointer].delta_pointer_y * 0.1 / input->delta_time;
		if(!input->pointers[pointer].last_button[button] && !slide)
			v->data.rotate.delta_rot[0] = v->data.rotate.delta_rot[1] = 0.0;
	}
}

void seduce_view_change_pan(SViewData *v, BInputState *input, uint button, boolean slide)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f;
	if(v == NULL)
		v = &sui_default_view;	
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[button] || input->pointers[i].last_button[button])
		{
			pointer = i;
			count++;
		}
	}
	if(count == 1)
	{
		if(v->type == S_VM_CAMERA)
			seduce_view_update(v, 10000.0);
		v->type = S_VM_PAN;
		seduce_view_pan_matrix(v, input->pointers[pointer].delta_pointer_x * -v->distance, input->pointers[pointer].delta_pointer_y * -v->distance);
		v->data.pan.delta_pan[0] = v->data.pan.delta_pan[1] = 0.0;
		if(!input->pointers[pointer].button[button] && slide)
		{
			v->data.pan.delta_pan[0] = input->pointers[pointer].delta_pointer_x / input->delta_time * -v->distance;
			v->data.pan.delta_pan[1] = input->pointers[pointer].delta_pointer_y / input->delta_time * -v->distance;
		}
	}
}

void seduce_view_change_pan_delta(SViewData *v, BInputState *input, uint button, boolean slide)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f;
	if(v == NULL)
		v = &sui_default_view;	
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[button] || input->pointers[i].last_button[button])
		{
			pointer = i;
			count++;
		}
	}
	if(count == 1)
	{
		if(v->type == S_VM_CAMERA)
			seduce_view_update(v, 10000.0);
		v->type = S_VM_PAN;
		if(!input->pointers[pointer].last_button[button])
			v->data.pan.delta_pan[0] = v->data.pan.delta_pan[1] = 0.0;
		v->data.pan.delta_pan[0] += input->pointers[pointer].delta_pointer_x;
		v->data.pan.delta_pan[1] += input->pointers[pointer].delta_pointer_y;
		if(!input->pointers[pointer].button[button] && !slide)
			v->data.pan.delta_pan[0] = v->data.pan.delta_pan[1] = 0.0;
	}
}

void seduce_view_pan(SViewData *v, float x, float y)
{
	if(v == NULL)
		v = &sui_default_view;	
	seduce_view_pan_matrix(v, x * v->distance, y * v->distance);
}

void seduce_view_pan_to_set(SViewData *v, float position_x, float position_y, float position_z)
{
	if(v == NULL)
		v = &sui_default_view;
	if(v->type != S_VM_CAMERA);
	{	
		v->type = S_VM_CAMERA;
		v->data.camera.progress = 0.0;
		v->data.camera.target_camera[0] = v->camera[0] + position_x - v->target[0];
		v->data.camera.target_camera[1] = v->camera[1] + position_y - v->target[1];
		v->data.camera.target_camera[2] = v->camera[2] + position_z - v->target[2];
		v->data.camera.target_up[0] = v->matrix[4];
		v->data.camera.target_up[1] = v->matrix[5];
		v->data.camera.target_up[2] = v->matrix[6];
	}
	v->data.camera.progress = 0.0;
	v->data.camera.target_target[0] = position_x;
	v->data.camera.target_target[1] = position_y;
	v->data.camera.target_target[2] = position_z;
}
/*
void seduce_view_distance(SViewData *v, BInputState *input, float delta_pointer_x, float delta_pointer_y)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f;
	if(v == NULL)
		v = &sui_default_view;

	if(v->type == S_VM_CAMERA)
		seduce_view_update(v, 10000.0);
	v->type = S_VM_DISTANCE;
	seduce_view_distance_matrix(v, (1.0 + 2.0 * (input->pointers[pointer].delta_pointer_x - input->pointers[pointer].delta_pointer_y)) * v->distance);
	v->data.distance.delta_distance = (input->pointers[pointer].delta_pointer_x - input->pointers[pointer].delta_pointer_y) / input->delta_time;
}*/

void seduce_view_distance(SViewData *v, float x, float y)
{
	if(v == NULL)
		v = &sui_default_view;
	seduce_view_distance_matrix(v, (1.0 + 2.0 * (x + y)) * v->distance);
}

void seduce_view_change_distance(SViewData *v, BInputState *input, uint button, boolean slide)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f;
	if(v == NULL)
		v = &sui_default_view;

	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[button] || input->pointers[i].last_button[button])
		{
			pointer = i;
			count++;
		}
	}
	if(count == 1)
	{
		seduce_view_distance(v, /*input,*/ input->pointers[pointer].delta_pointer_x, input->pointers[pointer].delta_pointer_y);
	/*	if(v->type == S_VM_CAMERA)
			seduce_view_update(v, 10000.0);
		v->type = S_VM_DISTANCE;
		seduce_view_distance_matrix(v, (1.0 + 2.0 * (input->pointers[pointer].delta_pointer_x - input->pointers[pointer].delta_pointer_y)) * v->distance);
		v->data.distance.delta_distance =  0.0;
		if(!input->pointers[pointer].button[button] && slide)
			v->data.distance.delta_distance = (input->pointers[pointer].delta_pointer_x - input->pointers[pointer].delta_pointer_y) / input->delta_time;*/
	}
}

void seduce_view_change_distance_delta(SViewData *v, BInputState *input, uint button, boolean slide)
{
	uint i, pointer, count = 0;
	float vec[3], up[3], f;
	if(v == NULL)
		v = &sui_default_view;

	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[button] || input->pointers[i].last_button[button])
		{
			pointer = i;
			count++;
		}
	}
	if(count == 1)
	{
		if(v->type == S_VM_CAMERA)
			seduce_view_update(v, 10000.0);
		v->type = S_VM_DISTANCE;
		if(!input->pointers[pointer].last_button[button])
			v->data.distance.delta_distance = 1.0;
		if(!input->pointers[pointer].button[button] && slide)
			v->data.distance.delta_distance += input->pointers[pointer].delta_pointer_x + input->pointers[pointer].delta_pointer_y;
	}
}




void seduce_view_change_distance_scroll(SViewData *v, BInputState *input)
{
	static boolean scroll_up = FALSE, last_scroll_up = FALSE, scroll_down = FALSE, last_scroll_down = FALSE;
	if(v == NULL)
		v = &sui_default_view;
	betray_button_get_up_down(0, &scroll_up, &last_scroll_up, BETRAY_BUTTON_SCROLL_UP);
	betray_button_get_up_down(0, &scroll_down, &last_scroll_down, BETRAY_BUTTON_SCROLL_DOWN);

	if((scroll_down && !last_scroll_down) || (scroll_up && !last_scroll_up))
	{
		if(v->type != S_VM_SCROLL)
		{
			v->data.scroll.target_camera[0] = v->camera[0];
			v->data.scroll.target_camera[1] = v->camera[1];
			v->data.scroll.target_camera[2] = v->camera[2];
			v->data.scroll.target_target[0] = v->target[0];
			v->data.scroll.target_target[1] = v->target[1];
			v->data.scroll.target_target[2] = v->target[2];
			v->type = S_VM_SCROLL;
		}
		v->data.camera.progress = 0;
	}
	if(scroll_up && !last_scroll_up)
	{		
		v->data.scroll.target_camera[0] = v->data.scroll.target_camera[0] + (v->camera[0] - v->target[0]) * 0.2;
		v->data.scroll.target_camera[1] = v->data.scroll.target_camera[1] + (v->camera[1] - v->target[1]) * 0.2;
		v->data.scroll.target_camera[2] = v->data.scroll.target_camera[2] + (v->camera[2] - v->target[2]) * 0.2;
	}

	if(scroll_down && !last_scroll_down)
	{
		v->data.scroll.target_camera[0] = v->data.scroll.target_camera[0] - (v->camera[0] - v->target[0]) * 0.2;
		v->data.scroll.target_camera[1] = v->data.scroll.target_camera[1] - (v->camera[1] - v->target[1]) * 0.2;
		v->data.scroll.target_camera[2] = v->data.scroll.target_camera[2] - (v->camera[2] - v->target[2]) * 0.2;
	}
}

void seduce_view_center_set(SViewData *v, float position_x, float position_y, float position_z)
{
	if(v == NULL)
		v = &sui_default_view;
	if(v->type != S_VM_CAMERA)
	{	
		v->type = S_VM_CAMERA;
		v->data.camera.target_camera[0] = v->camera[0];
		v->data.camera.target_camera[1] = v->camera[1];
		v->data.camera.target_camera[2] = v->camera[2];
		v->data.camera.target_up[0] = v->matrix[4];
		v->data.camera.target_up[1] = v->matrix[5];
		v->data.camera.target_up[2] = v->matrix[6];
	}
	v->data.camera.progress = 0.0;
	v->data.camera.target_target[0] = -position_x;
	v->data.camera.target_target[1] = -position_y;
	v->data.camera.target_target[2] = -position_z;
}

void seduce_view_camera_set(SViewData *v, float position_x, float position_y, float position_z)
{
	if(v == NULL)
		v = &sui_default_view;
	if(v->type != S_VM_CAMERA)
	{	
		v->type = S_VM_CAMERA;
		v->data.camera.target_target[0] = v->camera[0] + v->matrix[2] * v->distance;
		v->data.camera.target_target[1] = v->camera[1] + v->matrix[6] * v->distance;
		v->data.camera.target_target[2] = v->camera[2] + v->matrix[10] * v->distance;
		v->data.camera.target_up[0] = v->matrix[4];
		v->data.camera.target_up[1] = v->matrix[5];
		v->data.camera.target_up[2] = v->matrix[6];
	}
	v->data.camera.progress = 0.0;
	v->data.camera.target_camera[0] = -position_x;
	v->data.camera.target_camera[1] = -position_y;
	v->data.camera.target_camera[2] = -position_z;
}


void seduce_view_up_set(SViewData *v, float up_x, float up_y, float up_z)
{
	if(v == NULL)
		v = &sui_default_view;
	if(v->type != S_VM_CAMERA)
	{	
		v->type = S_VM_CAMERA;
		v->data.camera.target_target[0] = v->camera[0] + v->matrix[2] * v->distance;
		v->data.camera.target_target[1] = v->camera[1] + v->matrix[6] * v->distance;
		v->data.camera.target_target[2] = v->camera[2] + v->matrix[10] * v->distance;
		v->data.camera.target_camera[0] = v->camera[0];
		v->data.camera.target_camera[1] = v->camera[1];
		v->data.camera.target_camera[2] = v->camera[2];
	}
	v->data.camera.progress = 0.0;
	v->data.camera.target_up[0] = up_x;
	v->data.camera.target_up[1] = up_y;
	v->data.camera.target_up[2] = up_z;
}


void seduce_view_direction_set(SViewData *v, float normal_x, float normal_y, float normal_z)
{
	if(v == NULL)
		v = &sui_default_view;
	if(v->type != S_VM_CAMERA)
	{	
		v->type = S_VM_CAMERA;	
		v->data.camera.target_camera[0] = v->camera[0];
		v->data.camera.target_camera[1] = v->camera[1];
		v->data.camera.target_camera[2] = v->camera[2];
		v->data.camera.target_target[0] = v->target[0];
		v->data.camera.target_target[1] = v->target[1];
		v->data.camera.target_target[2] = v->target[2];
		v->data.camera.target_up[0] = v->matrix[4];
		v->data.camera.target_up[1] = v->matrix[5];
		v->data.camera.target_up[2] = v->matrix[6];
	}
	v->data.camera.progress = 0.0;
	v->data.camera.target_camera[0] = v->target[0] + normal_x;
	v->data.camera.target_camera[1] = v->target[1] + normal_y;
	v->data.camera.target_camera[2] = v->target[2] + normal_z;
}

void seduce_view_distance_camera_set(SViewData *v, double distance)
{
	v->data.scroll.target_camera[0] = v->data.scroll.target_camera[0] + (v->camera[0] - v->target[0]) * 0.2;
	v->data.scroll.target_camera[1] = v->data.scroll.target_camera[1] + (v->camera[1] - v->target[1]) * 0.2;
	v->data.scroll.target_camera[2] = v->data.scroll.target_camera[2] + (v->camera[2] - v->target[2]) * 0.2;
}

boolean seduce_view_change_right_button(SViewData *v, BInputState *input)
{
	static boolean active = FALSE;
	float a, b, center[2], delta[2], pan_dist[2], zoom_dist, rotate_dist[2];
	boolean pan = FALSE, zoom = FALSE, rotate = FALSE;
	uint i, pointer, count = 0;
	if(input->mode == BAM_EVENT)
	{
		if(v == NULL)
			v = &sui_default_view;

		center[0] = center[1] = delta[0] = delta[1] = 0;
		for(i = 0; i < input->pointer_count; i++)
		{	
			if(input->pointers[i].button[1] || input->pointers[i].last_button[1])
			{
				pointer = i;
				count++;
			}
		}
		if(count == 1)
		{
			if(!input->pointers[pointer].button[1])
				active = FALSE;
			if(input->pointers[pointer].button[1] && !input->pointers[pointer].last_button[1])
			{
				for(i = 0; i < input->pointers[pointer].button_count && input->pointers[pointer].button[i] == (i == 1); i++);
				active = i == input->pointers[pointer].button_count;
			}

			if(active)
			{
				if(!input->pointers[pointer].button[0] && 
					!input->pointers[pointer].last_button[0] && 
					!input->pointers[pointer].button[2] && 
					!input->pointers[pointer].last_button[2])
					seduce_view_change_rotate(v, input, 1, FALSE);
				else
				{	
					seduce_view_change_distance(v, input, 0, FALSE);
					seduce_view_change_pan(v, input, 2, FALSE);
				}
			}
		}else
			active = FALSE;
		return active;
	}
	return FALSE;
}

typedef struct{
	uint pointers[3];
	uint count;
	float distance;
}SeduceWiewMultiTouchUser;

boolean seduce_view_change_multi_touch(SViewData *v, BInputState *input, void *id)
{
	if(input->mode == BAM_EVENT)
	{
		static SeduceWiewMultiTouchUser *users = NULL;
		uint i, j, count;
		float x, y, dist;
		count = betray_support_functionality(B_SF_USER_COUNT_MAX);
		if(users == NULL)
		{
			users = malloc((sizeof *users) * count);
			for(i = 0; i < count; i++)
			{
				users[i].pointers[0] = -1;
				users[i].pointers[1] = -1;
				users[i].pointers[2] = -1;
				users[i].count = 0;
			}
		}
		if(v == NULL)
			v = &sui_default_view;

		for(i = 0; i < input->pointer_count; i++)
		{	
			if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			{
				if(id == seduce_element_pointer_id(input, i, 0))
				{
					if(users[input->pointers[i].user_id].count >= 3)
						users[input->pointers[i].user_id].count++;
					else
						users[input->pointers[i].user_id].pointers[users[input->pointers[i].user_id].count++] = i;
					users[input->pointers[i].user_id].distance = 1000000.0;
				}
			}
		/*	if(!input->pointers[i].button[0] && users[input->pointers[i].user_id].count <= 3)
			{
				for(j = 0; j < users[input->pointers[i].user_id].count; j++)
				{
					if(users[input->pointers[i].user_id].pointers[j] == i)
					{
						users[input->pointers[i].user_id].pointers[j] = users[input->pointers[i].user_id].pointers[--users[input->pointers[i].user_id].count];
						users[input->pointers[i].user_id].distance = 1000000.0;
					}
				}
			}*/
		}
		for(i = 0; i < count; i++)
		{
			if(users[i].count > 3)
			{
				for(j = 0; j < input->pointer_count && (!input->pointers[i].button[0] || input->pointers[i].user_id != i); j++);
				if(j == input->pointer_count)
					users[j].count = 0;
			}else
			{
				for(j = 0; j < users[i].count; j++)
				{
					if(!input->pointers[users[i].pointers[j]].button[0])
					{
						users[i].pointers[j] = users[i].pointers[--users[i].count];
						users[i].distance = 1000000.0;
					}
				}
			}


			if(users[i].count == 2)
			{
				x = (input->pointers[users[i].pointers[0]].delta_pointer_x + input->pointers[users[i].pointers[1]].delta_pointer_x) * 0.5;
				y = (input->pointers[users[i].pointers[0]].delta_pointer_y + input->pointers[users[i].pointers[1]].delta_pointer_y) * 0.5;
				seduce_view_rotate(v, input, x, y);
			}else if(users[i].count == 3)
			{
				x = (input->pointers[users[i].pointers[0]].delta_pointer_x + input->pointers[users[i].pointers[1]].delta_pointer_x + input->pointers[users[i].pointers[2]].delta_pointer_x) * 0.333;
				y = (input->pointers[users[i].pointers[0]].delta_pointer_y + input->pointers[users[i].pointers[1]].delta_pointer_y + input->pointers[users[i].pointers[2]].delta_pointer_y) * 0.333;
				seduce_view_pan(v, -x, -y);
			}
			if(users[i].count == 2 || users[i].count == 3)
			{
				x = input->pointers[users[i].pointers[0]].pointer_x - input->pointers[users[i].pointers[1]].pointer_x;
				y = input->pointers[users[i].pointers[0]].pointer_y - input->pointers[users[i].pointers[1]].pointer_y;
				dist = sqrt(x * x + y * y);
				if(users[input->pointers[i].user_id].distance < 100.0)
					seduce_view_distance_matrix(v, v->distance * users[i].distance / dist);
				users[i].distance = dist;
				return TRUE;
			}
		}
	}
	return FALSE;
}


//	betray_button_get_up_down(uint user_id, boolean *press, boolean *last_press, uint button);

typedef struct{
	boolean alt_press;
	boolean ctrl_press;
	boolean shift_press;
	uint pointer_id;
}SViewKeyGrab;

boolean seduce_view_change_keys(SViewData *v, BInputState *input, void *id)
{
	if(input->mode == BAM_EVENT)
	{
		static SViewKeyGrab *grab = NULL;
		static uint grab_count = 0;
		boolean active = FALSE;
		uint i;
		if(grab_count < input->user_count)
		{
			grab = malloc((sizeof *grab) * input->user_count);
			for(i = grab_count; i < input->user_count; i++)
			{
				grab[i].alt_press = FALSE;
				grab[i].ctrl_press = FALSE;
				grab[i].shift_press = FALSE;
				grab[i].pointer_id = -1;
			}
			grab_count = input->user_count;
		}

		for(i = 0; i < input->button_event_count; i++)
		{
			if(input->button_event[i].button == BETRAY_BUTTON_SHIFT)
				grab[input->button_event[i].user_id].shift_press = input->button_event[i].state;
			if(input->button_event[i].button == BETRAY_BUTTON_ALT)
				grab[input->button_event[i].user_id].alt_press = input->button_event[i].state;
			if(input->button_event[i].button == BETRAY_BUTTON_CONTROL)
				grab[input->button_event[i].user_id].ctrl_press = input->button_event[i].state;
		}
		for(i = 0; i < input->pointer_count; i++)
		{
			if(grab[input->pointers[i].user_id].alt_press)
			{
				if(id == seduce_element_pointer_id(input, i, NULL) && (
					(input->pointers[i].button[0] && !input->pointers[i].last_button[0]) ||
					(input->pointers[i].button[1] && !input->pointers[i].last_button[1]) ||
					(input->pointers[i].button[2] && !input->pointers[i].last_button[2])))
					grab[input->pointers[i].user_id].pointer_id = i;

				if(grab[input->pointers[i].user_id].pointer_id == i)
				{
					if(input->pointers[i].button[0])
					{
						if(grab[input->button_event[i].user_id].ctrl_press)
							seduce_view_pan(v, -input->pointers[i].delta_pointer_x, -input->pointers[i].delta_pointer_y);
						else if(grab[input->button_event[i].user_id].shift_press)
							seduce_view_distance(v,/* input,*/ input->pointers[i].delta_pointer_x, input->pointers[i].delta_pointer_y);
						else
							seduce_view_rotate(v, input, input->pointers[i].delta_pointer_x, input->pointers[i].delta_pointer_y);
					}else
					{
						if(input->pointers[i].button[2])
							seduce_view_pan(v, -input->pointers[i].delta_pointer_x, -input->pointers[i].delta_pointer_y);
						else if(input->pointers[i].button[1])
							seduce_view_distance(v,/* input,*/ input->pointers[i].delta_pointer_x, input->pointers[i].delta_pointer_y);
					}
				}
				active = TRUE;
			}else
				grab[input->pointers[i].user_id].pointer_id = -1;

		}
		return active;
	}
	return FALSE;
}

boolean seduce_view_change_scroll_wheel(SViewData *v, BInputState *input)
{
	if(input->mode == BAM_EVENT)
	{
		if(betray_button_get(-1, BETRAY_BUTTON_SCROLL_UP))
		{
			if(v == NULL)
				v = &sui_default_view;
			seduce_view_distance_matrix(v, v->distance * 0.9);
			return TRUE;
		}
		if(betray_button_get(-1, BETRAY_BUTTON_SCROLL_DOWN))
		{
			if(v == NULL)
				v = &sui_default_view;
			seduce_view_distance_matrix(v, v->distance / 0.9);
			return TRUE;
		}
	}
	return FALSE;
}

void seduce_view_grid_size_set(SViewData *v, double grid_size)
{
/*	if(v == NULL)
		v = &sui_default_view;
    v->grid_size = 1.0 / grid_size;*/
}

void seduce_view_center_getf(SViewData *v, float *center)
{
	if(v == NULL)
		v = &sui_default_view;
	center[0] = -v->target[0];
	center[1] = -v->target[1];
	center[2] = -v->target[2];
}

void seduce_view_center_getd(SViewData *v, double *center)
{
	if(v == NULL)
		v = &sui_default_view;
	center[0] = -v->target[0];
	center[1] = -v->target[1];
	center[2] = -v->target[2];
}
 void seduce_view_camera_getf(SViewData *v, float *camera)
{
	if(v == NULL)
		v = &sui_default_view;
	camera[0] = -v->camera[0];
	camera[1] = -v->camera[1];
	camera[2] = -v->camera[2];
}

void seduce_view_camera_getd(SViewData *v, double *camera)
{
	if(v == NULL)
		v = &sui_default_view;
	camera[0] = -v->camera[0];
	camera[1] = -v->camera[1];
	camera[2] = -v->camera[2];
}

void seduce_view_camera_vector_getf(SViewData *v, float *camera, float x, float y)
{
	float f;
	if(v == NULL)
		v = &sui_default_view;
	camera[0] = v->model[0] * x + v->model[1] * y - v->model[2];
	camera[1] = v->model[4] * x + v->model[5] * y - v->model[6];
	camera[2] = v->model[8] * x + v->model[9] * y - v->model[10];
/*	camera[0] = v->matrix[0] * x + v->matrix[1] * y + v->matrix[2] - v->camera[0];
	camera[1] = v->matrix[4] * x + v->matrix[5] * y + v->matrix[6] - v->camera[1];
	camera[2] = v->matrix[8] * x + v->matrix[9] * y + v->matrix[10] - v->camera[2];*/
	f = sqrt(camera[0] * camera[0] + camera[1] * camera[1] + camera[2] * camera[2]);
	camera[0] /= f;
	camera[1] /= f;
	camera[2] /= f;
}

void seduce_view_camera_vector_getd(SViewData *v, double *camera, double x, double y)
{
	double f;
	if(v == NULL)
		v = &sui_default_view;
	camera[0] = v->model[0] * x + v->model[1] * y - v->model[2];
	camera[1] = v->model[4] * x + v->model[5] * y - v->model[6];
	camera[2] = v->model[8] * x + v->model[9] * y - v->model[10];
/*	camera[0] = v->model[0] * x + v->model[1] * y + v->model[2] - v->camera[0];
	camera[1] = v->model[4] * x + v->model[5] * y + v->model[6] - v->camera[1];
	camera[2] = v->model[8] * x + v->model[9] * y + v->model[10] - v->camera[2];*/
	f = sqrt(camera[0] * camera[0] + camera[1] * camera[1] + camera[2] * camera[2]);
	camera[0] /= f;
	camera[1] /= f;
	camera[2] /= f;
}

double seduce_view_distance_camera_get(SViewData *v)
{
	if(v == NULL)
		v = &sui_default_view;
	return v->distance;
}

uint seduce_view_axis_get(void)
{
    uint *a = NULL;
    *a = 0;
    return 0;
/*	return sui_default_view.axis;*/
}


void seduce_view_model_matrixd(SViewData *v, double *matrix)
{
	uint i;
	if(v == NULL)
		v = &sui_default_view;
	for(i = 0; i < 16; i++)
		matrix[i] = (float)v->model[i];
}

void seduce_view_model_matrixf(SViewData *v, float *matrix)
{
	uint i;
	if(v == NULL)
		v = &sui_default_view;
	for(i = 0; i < 16; i++)
		matrix[i] = (float)v->model[i];
}

void seduce_view_sprite_matrixf(SViewData *v, float *matrix)
{
	float f;
	uint i;
	if(v == NULL)
		v = &sui_default_view;
	
	f = sqrt((float)v->model[0] * (float)v->model[0] + (float)v->model[4] * (float)v->model[4] + (float)v->model[8] * (float)v->model[8]);
	matrix[0] = (float)v->model[0] / f;
	matrix[4] = (float)v->model[4] / f;
	matrix[8] = (float)v->model[8] / f;
	matrix[3] = 0.0;

	f = sqrt((float)v->model[1] * (float)v->model[1] + (float)v->model[5] * (float)v->model[5] + (float)v->model[9] * (float)v->model[9]);
	matrix[1] = (float)v->model[1] / f;
	matrix[5] = (float)v->model[5] / f;
	matrix[9] = (float)v->model[9] / f;
	matrix[7] = 0.0;

	f = sqrt((float)v->model[2] * (float)v->model[2] + (float)v->model[6] * (float)v->model[6] + (float)v->model[10] * (float)v->model[10]);
	matrix[2] = (float)v->model[2] / f;
	matrix[6] = (float)v->model[6] / f;
	matrix[10] = (float)v->model[10] / f;
	matrix[11] = 0.0;

	matrix[12] = 0.0;
	matrix[13] = 0.0;
	matrix[14] = 0.0;
	matrix[15] = 1.0;
}

extern float sui_screen_mode_focus_plane;
extern RMatrix r_matrix_state; 

void seduce_view_set(SViewData *v, RMatrix *m)
{
	double cam_pos[3] ;
	float vantage[3];
	if(v == NULL)
		v = &sui_default_view;
	if(m == NULL)
		m = r_matrix_get();
	r_matrix_matrix_mult(m, v->model);
}


void seduce_view_projectiond(SViewData *v, double *output, double x, double y)
{
	if(v == NULL)
		v = &sui_default_view;
	output[0] = v->model[0] * x * v->distance;
	output[1] = v->model[4] * x * v->distance;
	output[2] = v->model[8] * x * v->distance;
	output[0] += v->model[1] * y * v->distance;
	output[1] += v->model[5] * y * v->distance;
	output[2] += v->model[9] * y * v->distance;
	output[0] -= v->target[0];
	output[1] -= v->target[1];
	output[2] -= v->target[2];
}

void seduce_view_projectionf(SViewData *v, float *output, float x, float y)
{
	if(v == NULL)
		v = &sui_default_view;
	output[0] = (float)v->model[0] * x * (float)v->distance;
	output[1] = (float)v->model[4] * x * (float)v->distance;
	output[2] = (float)v->model[8] * x * (float)v->distance;
	output[0] += (float)v->model[1] * y * (float)v->distance;
	output[1] += (float)v->model[5] * y * (float)v->distance;
	output[2] += (float)v->model[9] * y * (float)v->distance;
	output[0] -= (float)v->target[0];
	output[1] -= (float)v->target[1];
	output[2] -= (float)v->target[2];
}


void seduce_view_projection_vertexf(SViewData *v, float *output, float *vertex, float x, float y)
{
	float dist, z;
	if(v == NULL)
		v = &sui_default_view;
	z = v->model[2] * (vertex[0] - v->target[0]) + v->model[6] * (vertex[1] - v->target[1]) + v->model[10] * (vertex[2] - v->target[2]);
	dist = (v->distance - z);
	if(dist < 0)
		dist = v->distance;
	output[0] = v->model[0] * x * dist;
	output[1] = v->model[4] * x * dist;
	output[2] = v->model[8] * x * dist;
	output[0] += v->model[1] * y * dist;
	output[1] += v->model[5] * y * dist;
	output[2] += v->model[9] * y * dist;
	output[0] += v->model[2] * z;
	output[1] += v->model[6] * z;
	output[2] += v->model[10] * z;
	output[0] -= v->target[0];
	output[1] -= v->target[1];
	output[2] -= v->target[2];
}

void seduce_view_projection_vertexd(SViewData *v, double *output, double *vertex, double x, double y)
{
	double dist, z;
	if(v == NULL)
		v = &sui_default_view;

	z = v->model[2] * (vertex[0] - v->target[0]) + v->model[6] * (vertex[1] - v->target[1]) + v->model[10] * (vertex[2] - v->target[2]);
	dist = (v->distance - z);
	if(dist < 0)
		dist = v->distance;
	output[0] = v->model[0] * x * dist;
	output[1] = v->model[4] * x * dist;
	output[2] = v->model[8] * x * dist;
	output[0] += v->model[1] * y * dist;
	output[1] += v->model[5] * y * dist;
	output[2] += v->model[9] * y * dist;
	output[0] += v->model[2] * z;
	output[1] += v->model[6] * z;
	output[2] += v->model[10] * z;
	output[0] -= v->target[0];
	output[1] -= v->target[1];
	output[2] -= v->target[2];
}


void seduce_view_projection_screend(SViewData *v, double *output, double x, double y, double z)
{
	if(v == NULL)
		v = &sui_default_view;
	output[0] = (v->model[0] * x) + (v->model[4] * y) + (v->model[8] * z) + v->model[12];
	output[1] = (v->model[1] * x) + (v->model[5] * y) + (v->model[9] * z) + v->model[13];
	output[2] = (v->model[2] * x) + (v->model[6] * y) + (v->model[10] * z) + v->model[14];
	output[0] = -output[0] / output[2];
	output[1] = -output[1] / output[2];
}


void seduce_view_projection_screenf(SViewData *v, float *output, float x, float y, float z)
{
	if(v == NULL)
		v = &sui_default_view;
	output[0] = (v->model[0] * x) + (v->model[4] * y) + (v->model[8] * z) + v->model[12];
	output[1] = (v->model[1] * x) + (v->model[5] * y) + (v->model[9] * z) + v->model[13];
	output[2] = (v->model[2] * x) + (v->model[6] * y) + (v->model[10] * z) + v->model[14];
	output[0] = -output[0] / output[2];
	output[1] = -output[1] / output[2];
}


double seduce_view_projection_screen_distanced(SViewData *v, double space_x, double space_y, double space_z, double screen_x, double screen_y)
{
	float out[3];
	if(v == NULL)
		v = &sui_default_view;
	f_transform3f(out, v->model, space_x, space_y, space_z);
	out[0] = (out[0] / out[2]) + out[0];
	out[1] = (out[1] / out[2]) + out[1];
	return out[0] * out[0] + out[1] * out[1];
}


void seduce_view_projection_planed(SViewData *v, double *dist, uint axis, double pointer_x, double pointer_y , double depth)
{
/*	double a[3], b[3], r;
	if(v == NULL)
		v = &sui_default_view;
	a[0] = v->model[0] * v->distance * pointer_x;
	a[1] = v->model[4] * v->distance * pointer_x;
	a[2] = v->model[8] * v->distance * pointer_x;
	a[0] += v->model[1] * v->distance * pointer_y;
	a[1] += v->model[5] * v->distance * pointer_y;
	a[2] += v->model[9] * v->distance * pointer_y;
	a[0] += v->position[0];
	a[1] += v->position[1];
	a[2] += v->position[2];
	b[0] = v->model[2] * v->distance;
	b[1] = v->model[6] * v->distance;
	b[2] = v->model[10] * v->distance;
	b[0] += v->position[0];
	b[1] += v->position[1];
	b[2] += v->position[2];
	r = (b[axis] - depth) / (b[axis] - a[axis]);
	dist[0] = b[0] - (b[0] - a[0]) * r;
	dist[1] = b[1] - (b[1] - a[1]) * r;
	dist[2] = b[2] - (b[2] - a[2]) * r;
	dist[axis] = depth;*/
}

double seduce_view_projection_lined(SViewData *v, double *dist, uint axis, double pointer_x, double pointer_y, double *pos)
{
/*	double a[3], b[3], r, r2, r3;
	if(v == NULL)
		v = &sui_default_view;
	a[0] = v->model[0] * v->distance * pointer_x;
	a[1] = v->model[4] * v->distance * pointer_x;
	a[2] = v->model[8] * v->distance * pointer_x;
	a[0] += v->model[1] * v->distance * pointer_y;
	a[1] += v->model[5] * v->distance * pointer_y;
	a[2] += v->model[9] * v->distance * pointer_y;
	b[0] = v->model[2] * v->distance;
	b[1] = v->model[6] * v->distance;
	b[2] = v->model[10] * v->distance;
	a[0] -= b[0];
	a[1] -= b[1];
	a[2] -= b[2];
	b[0] += v->position[0] - pos[0];
	b[1] += v->position[1] - pos[1];
	b[2] += v->position[2] - pos[2];
	r = sqrt(b[(axis + 1) % 3] * b[(axis + 1) % 3] + b[(axis + 2) % 3] * b[(axis + 2) % 3]);
	r2 = sqrt(a[(axis + 1) % 3] * a[(axis + 1) % 3] + a[(axis + 2) % 3] * a[(axis + 2) % 3]);
	r3 = b[axis] + (a[axis] * r / r2);
	if(dist != NULL)
	{
		a[axis] += r3;
		f_transform3d(a, v->model, pos[0], pos[1], pos[2]);
		a[0] = (a[0] / a[2]) + pointer_x;
		a[1] = (a[1] / a[2]) + pointer_y;
		*dist = a[0] * a[0] + a[1] * a[1];
	}
	return r3;*/
    return 0;
}



/*
extern void r_matrix_projection_screend(RMatrix *matrix, double *output, double x, double y, double z);
extern void r_matrix_projection_screenf(RMatrix *matrix, float *output, float x, float y, float z);
extern void r_matrix_projection_worldf(RMatrix *matrix, float *output, float x, float y, float z);
extern void r_matrix_projection_vertexd(RMatrix *matrix, double *output, double *vertex, double x, double y);
extern void r_matrix_projection_vertexf(RMatrix *matrix, float *output, float *vertex, float x, float y);

extern boolean r_matrix_projection_surfacef(RMatrix *matrix, float *output, float *pos, uint axis, float x, float y);
extern boolean r_matrix_projection_axisf(RMatrix *matrix, float *output, float *pos, uint axis, float x, float y);
extern boolean r_matrix_projection_vectorf(RMatrix *matrix, float *output, float *pos, float *vec, float x, float y);
*/

boolean seduce_view_edge_test(double *a, double *b, double x, double y)
{
/*	double temp, r;
	x = -x;
	y = -y;
	if(a[2] > 0 || b[2] > 0)
		return FALSE;
	if((a[0] - b[0]) * (x - b[0]) + (a[1] - b[1]) * (y - b[1]) < 0)
		return FALSE;
	if((b[0] - a[0]) * (x - a[0]) + (b[1] - a[1]) * (y - a[1]) < 0)
		return FALSE;
	r = sqrt((b[1] - a[1]) * (b[1] - a[1]) + -(b[0] - a[0]) * -(b[0] - a[0]));
	temp = (x - a[0]) * ((b[1] - a[1]) / r) + (y - a[1]) * (-(b[0] - a[0]) / r);
	if(temp > 0.008 || temp < -0.008 || r < 0.0001)
		return FALSE;
	return TRUE;*/
    return TRUE;
}
