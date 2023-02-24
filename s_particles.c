#include <math.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include "seduce.h"

#define S_BACKGROUND_PARTICLE_COUNT 512
#define S_BACKGROUND_SPLAT_COLOR_COUNT 4

typedef struct{
	SPraticleType type;
	float pos[2];
	float vector[2];
	float age;
}SBackgroundParticle;

struct{
	uint particle_texture;
	void *particle_fbo;
	uint sprite_texture;
	uint click_texture;
	SBackgroundParticle particles[S_BACKGROUND_PARTICLE_COUNT];
	uint next_particle;
	void *particle_pool;
	float *particle_array;
	float rate;
	uint frame_id;
	uint draw_id;
}SeduceParticleGlobal;



void seduce_particle_init()
{
	RFormats vertex_format_types[3] = {R_FLOAT, R_FLOAT, R_FLOAT};
	uint vertex_format_size[3] = {4, 4, 4};
	static boolean init = FALSE;
	uint8 *array;
	uint i, j;
	float x, y, f, f2;
	if(init)
		return;
	init = TRUE;

	SeduceParticleGlobal.particle_texture = r_texture_allocate(R_IF_RGB_FLOAT16, 128, 128, 1, TRUE, FALSE, NULL);	
	SeduceParticleGlobal.particle_fbo = r_framebuffer_allocate(&SeduceParticleGlobal.particle_texture, 1, -1, RELINQUISH_TARGET_2D_TEXTURE);
	array = malloc((sizeof *array) * 256 * 256 * 4);
	for(i = 0; i < 256 * 256; i++)
	{
		if(i % 256 == 0 || i % 256 == 255 || i / 256 == 0 || i / 256 == 255)
			array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
		else
		{
			x = (float)(i % 256) / 64 - 2.0;
			y = (float)(i / 256) / 64 - 2.0;
			f = 1.0 / (1 + x * x + y * y);
			f = 255.0 * (f - 1.0 / 5.0) / (4.0 / 5.0);
			if(f > 0)
				array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = (uint8)f;
			else
				array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
		}
	}

	SeduceParticleGlobal.sprite_texture = r_texture_allocate(R_IF_RGBA_UINT8, 256, 256, 1, TRUE, FALSE, array);
	
	for(i = 0; i < 256 * 256; i++)
	{
		if(i % 256 == 0 || i % 256 == 255 || i / 256 == 0 || i / 256 == 255)
			array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
		else
		{
			x = (float)(i % 256) / 64 - 2.0;
			y = (float)(i / 256) / 64 - 2.0;
			f = sqrt(x * x + y * y);

			if(f > 0.5 && f < 0.98)
			{
				f = (f - 0.5) / 0.48;
				f = (cos(f * PI * 2) - 1) * -128.0;
				array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = f;
			}else
				array[i * 4 + 0] = array[i * 4 + 1] = array[i * 4 + 2] = array[i * 4 + 3] = 0;
		}
	}

	SeduceParticleGlobal.click_texture = r_texture_allocate(R_IF_RGBA_UINT8, 256, 256, 1, TRUE, FALSE, array);

	free(array);
	for(i = 0; i < S_BACKGROUND_PARTICLE_COUNT; i++)
	{
		SeduceParticleGlobal.particles[i].type = 0;
		SeduceParticleGlobal.particles[i].pos[0] = 0;
		SeduceParticleGlobal.particles[i].pos[1] = 0;
		SeduceParticleGlobal.particles[i].vector[0] = 0;
		SeduceParticleGlobal.particles[i].vector[1] = 0;
		SeduceParticleGlobal.particles[i].age = 1.1;
	}
	SeduceParticleGlobal.particle_pool = r_array_allocate(S_BACKGROUND_PARTICLE_COUNT * 6, vertex_format_types, vertex_format_size, 3, 0);
	SeduceParticleGlobal.particle_array = malloc((sizeof *SeduceParticleGlobal.particle_array) * S_BACKGROUND_PARTICLE_COUNT * 6 * 12);
}



void seduce_particle_spawn(BInputState *input, float pos_x, float pos_y, float vec_x, float vec_y, float start_age, uint type)
{
	static uint seed = 0;
	float f, pos[3];
	r_matrix_projection_screenf(r_matrix_get(), pos, pos_x, pos_y, 0);
	pos[0] = pos_x;
	pos[1] = pos_y;
	SeduceParticleGlobal.next_particle = (SeduceParticleGlobal.next_particle + 1) % S_BACKGROUND_PARTICLE_COUNT;
	SeduceParticleGlobal.particles[SeduceParticleGlobal.next_particle].type = type;
	SeduceParticleGlobal.particles[SeduceParticleGlobal.next_particle].age = start_age;
	SeduceParticleGlobal.particles[SeduceParticleGlobal.next_particle].pos[0] = pos[0];
	SeduceParticleGlobal.particles[SeduceParticleGlobal.next_particle].pos[1] = pos[1]; 
	SeduceParticleGlobal.particles[SeduceParticleGlobal.next_particle].vector[0] = vec_x; 
	SeduceParticleGlobal.particles[SeduceParticleGlobal.next_particle].vector[1] = vec_y; 
//	SBackgroundRender.rate++;
}

void seduce_particle_update(BInputState *input)
{
	static float particle_timer = 0;
	static uint seed = 0;
	float delta, f, vec[2], aspect;
	uint i, j;
	aspect = betray_screen_mode_get(NULL, NULL, NULL);
	particle_timer += input->delta_time;
	if(particle_timer > 0.0)
	{
		particle_timer = 0.0;
		for(i = 0; i < input->pointer_count; i++)
		{
			if(0.0001 < input->pointers[i].delta_pointer_x * input->pointers[i].delta_pointer_x + input->pointers[i].delta_pointer_y * input->pointers[i].delta_pointer_y)
			{
				seduce_particle_spawn(input, input->pointers[i].pointer_x + f_randnf(seed) * 0.1, input->pointers[i].pointer_y + f_randnf(seed + 1) * 0.1, 0, 0, 0, S_PT_LIGHT);
				seed++;
				particle_timer -= 0.015;
			}
		}
	}
	for(i = 0; i < input->pointer_count; i++)
	{
		if(input->pointers[i].button[0] && !input->pointers[i].last_button[0])
			seduce_particle_spawn(input, input->pointers[0].pointer_x, input->pointers[0].pointer_y, 0, 0, 0.0, S_PT_CLICK);
	}
	if(SeduceParticleGlobal.rate == 0)
		delta = input->delta_time * 0.1;
	else
		delta = 2.0 * SeduceParticleGlobal.rate / S_BACKGROUND_PARTICLE_COUNT;
	for(i = 0; i < S_BACKGROUND_PARTICLE_COUNT; i++)
	{
		if(SeduceParticleGlobal.particles[i].age < 1.0)
		{
			if(SeduceParticleGlobal.particles[i].type == S_PT_LIGHT)
				SeduceParticleGlobal.particles[i].age += delta;
			else
				SeduceParticleGlobal.particles[i].age += delta * 10.0;
			SeduceParticleGlobal.particles[i].pos[0] += SeduceParticleGlobal.particles[i].vector[0] * input->delta_time;
			SeduceParticleGlobal.particles[i].pos[1] += SeduceParticleGlobal.particles[i].vector[1] * input->delta_time / aspect;
			SeduceParticleGlobal.particles[i].vector[0] *= (1 -  input->delta_time * 1);
			SeduceParticleGlobal.particles[i].vector[1] *= (1 -  input->delta_time * 1);
		}
	}
	for(i = 0; i < input->pointer_count; i++)
	{
		if(!input->pointers[i].button[0] && 0.002 * 0.002 < input->pointers[i].delta_pointer_x * input->pointers[i].delta_pointer_x + input->pointers[i].delta_pointer_y * input->pointers[i].delta_pointer_y)
		{
			for(j = 0; j < S_BACKGROUND_PARTICLE_COUNT; j++)
			{
		//	if(SBackgroundRender.particles[j].type < S_PT_LIGHT)
				{
					vec[0] = (SeduceParticleGlobal.particles[j].pos[0] - input->pointers[i].pointer_x) * 10.0;
					vec[1] = (SeduceParticleGlobal.particles[j].pos[1] - input->pointers[i].pointer_y) * 10.0;
					f = 1.0 / (1.0 + vec[0] * vec[0] + vec[1] * vec[1]);
					vec[0] = SeduceParticleGlobal.particles[j].vector[0] * (1.0 - f) + f * input->pointers[i].delta_pointer_x / input->delta_time;
					vec[1] = SeduceParticleGlobal.particles[j].vector[1] * (1.0 - f) + f * input->pointers[i].delta_pointer_y / input->delta_time;
					delta = input->delta_time * 4.0;
					SeduceParticleGlobal.particles[j].vector[0] = SeduceParticleGlobal.particles[j].vector[0] * (1.0 - delta) + delta * vec[0];
					SeduceParticleGlobal.particles[j].vector[1] = SeduceParticleGlobal.particles[j].vector[1] * (1.0 - delta) + delta * vec[1];
				}
			}
		}
	}
	SeduceParticleGlobal.rate = 0;
}


void seduce_particle_set(float *array, float size, float pos_x, float pos_y, float scroll_a_u, float scroll_a_v, float scroll_b_u, float scroll_b_v, float aspect, float red, float green, float blue, float alpha)
{
	static float x[6] = {-1, 1, 1, -1, 1, -1};
	static float y[6] = {-1, -1, 1, -1, 1, 1};
	static float u[6] = {0, 1, 1, 0, 1, 0};
	static float v[6] = {0, 0, 1, 0, 1, 1};
	uint i;
	for(i = 0; i < 6; i++)
	{
		*array++ = pos_x + x[i] * size;
		*array++ = pos_y + y[i] * size / aspect;
		*array++ = u[i];
		*array++ = v[i];
		*array++ = u[i] * 0.5 + scroll_a_u * 0.5;
		*array++ = v[i] * 0.5 + scroll_a_v * 0.5;
		*array++ = u[i] * 0.25 + scroll_b_u * 0.25;
		*array++ = v[i] * 0.25 + scroll_b_v * 0.25;
		*array++ = red;
		*array++ = green;
		*array++ = blue;
		*array++ = alpha;
	}
}


uint seduce_particle_texture_get()
{
	return SeduceParticleGlobal.particle_texture;
}

void seduce_particle_render(BInputState *input)
{
	float splat_color[] = {0, 0, 0, 1, 0, 0, 0};
	if(input->frame_number == SeduceParticleGlobal.frame_id &&
		input->draw_id == SeduceParticleGlobal.draw_id)
		return;
	SeduceParticleGlobal.frame_id = input->frame_number;
	SeduceParticleGlobal.draw_id = input->draw_id;
	if(input->mode == BAM_DRAW)
	{
		RMatrix matrix, *reset;
		float f, f2, aspect, rgb[3];
		uint i, j, x, y;
		aspect = betray_screen_mode_get(&x, &y, NULL);
		reset = r_matrix_get();
		r_matrix_set(&matrix);
		r_matrix_identity(&matrix);
		r_matrix_frustum(&matrix, -0.05, 0.05, -0.05, 0.05, 0.05, 10.0f);
		r_viewport(0, 0, 128, 128);
		r_framebuffer_bind(SeduceParticleGlobal.particle_fbo);
		r_framebuffer_clear(0.0, 0.0, 0.0, 0.0, TRUE, TRUE);
		j = 0;

		f_hsv_to_rgb(rgb, input->minute_time, 0.4, 0.1);
		for(i = 0; i < input->pointer_count; i++)
				r_primitive_image(input->pointers[i].pointer_x - 0.5, 
								(input->pointers[i].pointer_y - 0.5) / aspect, -1.0, 1, 1 / aspect, 0, 0, 1, 1, SeduceParticleGlobal.sprite_texture, rgb[0], rgb[1], rgb[2], 0);

		for(i = 0; i < S_BACKGROUND_PARTICLE_COUNT; i++)
		{
			if(SeduceParticleGlobal.particles[i].age < 1.0)
			{
				switch(SeduceParticleGlobal.particles[i].type)	
				{
					case S_PT_LIGHT :
					if(SeduceParticleGlobal.particles[i].age < 0.025)
						f = SeduceParticleGlobal.particles[i].age / 0.025;
					else
						f = 1 - (SeduceParticleGlobal.particles[i].age - 0.025) / (1.0 - 0.025);
					f *= 0.25;
					r_primitive_image(SeduceParticleGlobal.particles[i].pos[0] - 0.25, 
								(SeduceParticleGlobal.particles[i].pos[1] - 0.25) / aspect, -1.0, 0.5, 0.5 / aspect, 0, 0, 1, 1, SeduceParticleGlobal.sprite_texture, f_randf(i) * f * 0.5, f_randf(i + 1) * f, f_randf(i + 2) * f, 0);
					break;
					case S_PT_CLICK :
					f = (1.0 - SeduceParticleGlobal.particles[i].age);
					f *= f;
					f2 = 1.0 - f;
					f *= 0.1; 
					r_primitive_image(SeduceParticleGlobal.particles[i].pos[0] - f2, 
									(SeduceParticleGlobal.particles[i].pos[1] - f2) / aspect, -1.0, f2 * 2.0, f2 * 2.0 / aspect, 0, 0, 1, 1, SeduceParticleGlobal.click_texture, f_randf(i) * f, f_randf(i + 1) * f, f_randf(i + 2) * f, f);
					break;
				}	
			}
		}
		r_framebuffer_bind(NULL);
		r_matrix_set(NULL);
		r_viewport(0, 0, x, y);
		r_matrix_set(reset);
	}
}

