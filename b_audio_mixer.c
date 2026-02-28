#include "betray.h"

#ifdef BETRAY_AUDIO_MIXER

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct{
	uint sound;
	uint progress;
	uint time_add;
	float pos[3];
	float vector[3];
	float speed;
	float volume;
	float play_distance;
	float play_speed;
	float play_volume;
	float play_vec[3];	
	boolean loop;
	boolean active;
	boolean ambient;
}BSourceStorage;

BSourceStorage *b_source_storage = NULL;
uint  b_source_storage_count = 0;
uint  b_source_storage_hole = 0;
uint  b_source_storage_alloc = 0;


typedef struct{
	uint16 *data;
	uint length;
	uint frequency;
}BSoundStorage;

BSoundStorage *b_sound_storage = NULL;
uint  b_sound_storage_count = 0;
uint  b_sound_storage_alloc = 0;
float b_sound_volume = 0.2;
float b_sound_volume_silence_cutoff = 0.01;

float b_listener_pos[3] = {0, 0, 0};
float b_listener_matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
float b_listener_scale = 1.0;

uint betray_audio_sound_create(uint type, uint stride, uint length, uint frequency, void *data, char *name)
{
	uint i, j;
	int8 *pint8;
	int16 *pint16, *p;
	int32 *pint32;
	float *preal32;
	if(data == NULL)
		return -1;
	if(b_sound_storage_count == b_sound_storage_alloc)
	{
		b_sound_storage_alloc += 32;
		b_sound_storage = realloc(b_sound_storage, (sizeof *b_sound_storage) * b_sound_storage_alloc);
	}
	p = malloc((sizeof *p) * length);
	
	switch(type)
	{
		case BETRAY_TYPE_INT8 :
			pint8 = data;
			for(i = 0; i < length; i++)
				p[i] = (int16)pint8[i * stride] * 256;
		break;
		case BETRAY_TYPE_INT16 :
			pint16 = data;
			for(i = 0; i < length; i++)
				p[i] = pint16[i * stride];
		break;
		case BETRAY_TYPE_INT32 :
			pint32 = data;
			for(i = 0; i < length; i++)
				p[i] = (int16)(pint32[i * stride] / (256 * 256));
		break;
		case BETRAY_TYPE_FLOAT32 :
			preal32 = data;
			for(i = 0; i < length; i++)
				p[i] = (int16)(preal32[i * stride] * (256.0 * 128.0 - 1.0));
		break;
	}
	b_sound_storage[b_sound_storage_count].data = p;
	b_sound_storage[b_sound_storage_count].length = length;
	b_sound_storage[b_sound_storage_count].frequency = frequency;
	b_sound_storage_count++;
	return b_sound_storage_count - 1;
}

void betray_audio_sound_destroy(uint sound)
{
	if(sound >= b_sound_storage_count)
		return;
	free(b_sound_storage[b_sound_storage_count].data);
}

void betray_audio_sound_set(uint source, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient)
{
	if(source >= b_source_storage_alloc)
		return;
	b_source_storage[source].pos[0] = pos[0];
	b_source_storage[source].pos[1] = pos[1];
	b_source_storage[source].pos[2] = pos[2];
	if(vector != NULL)
	{
		b_source_storage[source].vector[0] = vector[0];
		b_source_storage[source].vector[1] = vector[1];
		b_source_storage[source].vector[2] = vector[2];
	}else
	{
		b_source_storage[source].vector[0] = 0;
		b_source_storage[source].vector[1] = 0;
		b_source_storage[source].vector[2] = 0;
	}
	b_source_storage[source].speed = speed;
	b_source_storage[source].volume = volume;
	b_source_storage[source].loop = loop;
	b_source_storage[source].ambient = ambient;
}

void betray_audio_time_progress(BSourceStorage *s, float time)
{
	float f, listener[3] = {0, 0, 0}, vec[3];
	s->play_volume = s->volume * b_sound_volume;
	if(s->ambient)
	{
		s->play_vec[0] = s->pos[0];
		s->play_vec[1] = s->pos[1];
		s->play_vec[2] = s->pos[2];
		s->play_speed = s->speed * 44100.0 / (float)b_sound_storage[s->sound].frequency;
	}else
	{
		vec[0] = b_listener_pos[0] - s->pos[0];
		vec[1] = b_listener_pos[1] - s->pos[1];
		vec[2] = b_listener_pos[2] - s->pos[2];

	/*	s->play_vec[0] = b_listener_matrix[0] * vec[0] + b_listener_matrix[3] * vec[1] + b_listener_matrix[6] * vec[2];
		s->play_vec[1] = b_listener_matrix[1] * vec[0] + b_listener_matrix[4] * vec[1] + b_listener_matrix[7] * vec[2];
		s->play_vec[2] = b_listener_matrix[2] * vec[0] + b_listener_matrix[5] * vec[1] + b_listener_matrix[8] * vec[2];
	*/
		s->play_vec[0] = b_listener_matrix[0] * vec[0] + b_listener_matrix[1] * vec[1] + b_listener_matrix[2] * vec[2];
		s->play_vec[1] = b_listener_matrix[3] * vec[0] + b_listener_matrix[4] * vec[1] + b_listener_matrix[5] * vec[2];
		s->play_vec[2] = b_listener_matrix[6] * vec[0] + b_listener_matrix[7] * vec[1] + b_listener_matrix[8] * vec[2];

		f = sqrt(s->play_vec[0] * s->play_vec[0] + s->play_vec[1] * s->play_vec[1] + s->play_vec[2] * s->play_vec[2]);

	//	printf("betray_audio_time_progress %f %f\n", f, (float)sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]));
		if(f > b_listener_scale)
			s->play_volume /= (f / b_listener_scale);
		
		s->play_vec[0] /= f;
		s->play_vec[1] /= f;
		s->play_vec[2] /= f;
		s->play_speed = s->speed * (float)b_sound_storage[s->sound].frequency / 44100.0 + (s->play_distance - f) / b_listener_scale * 0.2;
	//	s->play_speed = 1.0;
		s->play_distance = f;
		s->pos[0] += s->vector[0] * time;
		s->pos[1] += s->vector[1] * time;
		s->pos[2] += s->vector[2] * time;
	}
	s->play_volume -= b_sound_volume_silence_cutoff;
}

uint betray_audio_sound_play(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient)
{
	if(sound == -1)
		return -1;
	for(b_source_storage_hole = 0; b_source_storage_hole < b_source_storage_count && b_source_storage[b_source_storage_hole].active; b_source_storage_hole++);
	if(b_source_storage_hole == b_source_storage_alloc)
	{
		b_source_storage_alloc += 32;
		b_source_storage = realloc(b_source_storage, (sizeof *b_source_storage) * b_source_storage_alloc);
	}
	b_source_storage[b_source_storage_hole].sound = sound;
	b_source_storage[b_source_storage_hole].progress = 0;
	b_source_storage[b_source_storage_hole].active = TRUE;
	betray_audio_sound_set(b_source_storage_hole, pos, vector, speed, volume, loop, ambient);
	betray_audio_time_progress(&b_source_storage[b_source_storage_hole], 0.01);
	betray_audio_time_progress(&b_source_storage[b_source_storage_hole], 0.0);
	b_source_storage_hole++;
	if(b_source_storage_hole > b_source_storage_count)
		b_source_storage_count = b_source_storage_hole;
	return b_source_storage_hole - 1;
}

void betray_audio_sound_stop(uint source)
{
	if(source >= b_source_storage_count)
		return;
	b_source_storage[source].active = FALSE;
	if(source + 1 == b_sound_storage_count)
	{
		for(b_sound_storage_count--; !b_source_storage[b_sound_storage_count].active &&	b_sound_storage_count > 0; b_sound_storage_count--);
		if(b_sound_storage_count < b_source_storage_hole)
			b_source_storage_hole = b_sound_storage_count;
	}
	if(source < b_source_storage_hole)
		b_source_storage_hole = source;
}


void betray_audio_update_callback(void *data, uint length, uint padding, float *vec)
{
	float volume, tmp[3], speed, f, gap, *fbuf;
	BSourceStorage *e;
	BSoundStorage *s;
	int16 *buf;
	uint i, j, k, end;
	fbuf = malloc((sizeof *fbuf) * length);
	for(i = 0; i < length; i++)
		fbuf[i] = 0;

	for(i = 0; i < b_source_storage_count; i++)
	{
		if(b_source_storage[i].active)
		{
			
			e = &b_source_storage[i];
			if(!e->ambient)
			{
				f = 0.8 + (e->play_vec[0] * vec[0] + e->play_vec[1] * vec[1] + e->play_vec[2] * vec[2]);
				if(f > 1.0)
					f = 1.0;
				volume = e->play_volume * f;
			}else
				volume = e->play_volume;
			s = &b_sound_storage[e->sound];
			speed = e->play_speed; 
			if(speed > 0.0 && volume > 0.0)
			{
				if(e->loop)
				{
					buf = s->data;
					end = s->length;
					f = e->progress;
					for(j = 0; j < length; j++)
					{
						k = (uint)f;
						gap = f - (float)k;
					/*	fbuf[j] += (((float)buf[k % end] * (1.0 - gap)) + 
									((float)buf[(k + 1) % end] * gap)) * volume;*/
						fbuf[j] += (float)buf[k % end] * volume;
						f += speed;
					}
					e->time_add = (float)length * speed;
				}else
				{
					buf = s->data;
					end = length;
					f = e->progress;

					if(e->progress + (uint)(speed * (float)length) >= s->length)
						end = (s->length - e->progress) / speed;
					for(j = 0; j < end; j++)
					{
						k = (uint)f;
						gap = f - (float)k;
					/*	fbuf[j] += (((float)buf[k % end] * (1.0 - gap)) + 
									((float)buf[(k + 1) % end] * gap)) * volume;*/
						fbuf[j] += (float)buf[k] * volume;
						f += speed;
					}
				}
			}
		}
	}
	for(i = 0; i < length; i++)
	{
		if(fbuf[i] > 32000)
			fbuf[i] = 32000;
		else if(fbuf[i] < -32000)
			fbuf[i] = -32000;
		((int16 *)data)[i * padding] = fbuf[i];
	}
	free(fbuf);
}



void betray_audio_time_callback(uint length)
{

	BSourceStorage *s;
	uint i;
	for(i = 0; i < b_source_storage_count; i++)
	{
		if(b_source_storage[i].active)
		{
			s = &b_source_storage[i];
			s->progress += (uint)((float)length * b_source_storage[i].play_speed);
	
			if(s->loop)
				s->progress = s->progress % b_sound_storage[s->sound].length;
			else if(s->progress >= b_sound_storage[s->sound].length)
				s->active = FALSE;
			if(s->active)
			{
				betray_audio_time_progress(s, (float)length / 44100.0);
			/*	s->play_volume = b_sound_volume;
				s->play_vec[0] = listener[0] - b_source_storage[i].pos[0];
				s->play_vec[1] = listener[1] - b_source_storage[i].pos[1];
				s->play_vec[2] = listener[2] - b_source_storage[i].pos[2];
				f = sqrt(s->play_vec[0] * s->play_vec[0] + s->play_vec[1] * s->play_vec[1] + s->play_vec[2] * s->play_vec[2]);
				if(f > 0.1)
					s->play_volume /= (f * 10.0);
				s->play_vec[0] /= f;
				s->play_vec[1] /= f;
				s->play_vec[2] /= f;
				s->play_speed = 1.0 + s->play_distance - f;
				s->play_speed = 1.0;
				s->play_distance = f;*/
			}
		}
	}
}

void betray_audio_listener(float *pos, float *vector, float *forward, float *side, float scale)
{
	b_listener_pos[0] = pos[0];
	b_listener_pos[1] = pos[1];
	b_listener_pos[2] = pos[2];
	b_listener_scale = scale;
	b_listener_matrix[0] = side[0];
	b_listener_matrix[1] = side[1];
	b_listener_matrix[2] = side[2];
	b_listener_matrix[3] = side[1] * forward[2] - side[2] * forward[1];
	b_listener_matrix[4] = side[2] * forward[0] - side[0] * forward[2];
	b_listener_matrix[5] = side[0] * forward[1] - side[1] * forward[0];
	b_listener_matrix[6] = forward[0];
	b_listener_matrix[7] = forward[1];
	b_listener_matrix[8] = forward[2];

/*	b_listener_matrix[0] = 1;
	b_listener_matrix[1] = 0;
	b_listener_matrix[2] = 0;
	b_listener_matrix[3] = 0;
	b_listener_matrix[4] = 1;
	b_listener_matrix[5] = 0;
	b_listener_matrix[6] = 0;
	b_listener_matrix[7] = 0;
	b_listener_matrix[8] = 1;*/
}

void betray_audio_master_volume_set(float volume)
{
	b_sound_volume = volume;
}

float betray_audio_master_volume_get()
{
	return b_sound_volume;
}

void betray_audio_master_volume_silence_cutoff(float volume)
{
	b_sound_volume_silence_cutoff = volume;
}


extern void b_audio_win_init();

void betray_audio_init()
{
#ifdef BETRAY_WIN32_AUDIO_WRAPPER
	b_audio_win_init();
#endif
}

#endif

double betray_get_delta_time(void);

void betray_audio_test()
{
	static boolean init = FALSE;
	static uint sound, source, rand;
	static float time = 0;
	float pos[3] = {0.1, 0, 0}, vec[3] = {0, 0, 0}, f;
	if(!init)
	{
		short *buf;
		uint i;
		betray_audio_init();
		
		buf = malloc((sizeof *buf) * 100000);
		for(i = 0; i < 100000; i++)
			buf[i] = 32000.0 * sin((double)(i) * PI * 2 * 0.01);
		sound = betray_audio_sound_create(0, 1, 100000, 22050, buf, "test");
	//	source = betray_audio_sound_play(sound, pos, vec, 1.0, 1.0, TRUE);
	//	betray_audio_sound_play(sound, pos, vec, 1.0, 1.0, TRUE);
		init = TRUE;
	}
/*	f = betray_get_delta_time();*/
	b_audio_win_update_sound();
/*	rand++;
	if(rand % 100 == 0)
	{
		printf("second\n");
		vec[0] = sin(time) * 5;
		vec[2] = 0.1;//cos(time);
		time += f;
		pos[0] = (f_randf(rand * 4 + 0) - 0.5) * 5.0;
		pos[2] = (f_randf(rand * 4 + 1) - 0.5) * 5.0;
		vec[0] = (f_randf(rand * 4 + 2) - 0.5) * 0.05;
		vec[2] = (f_randf(rand * 4 + 3) - 0.5) * 0.05;
	//	betray_audio_sound_set(source, pos, vec, 1.0, 1.0, TRUE);
		betray_audio_sound_play(sound, pos, vec, 1.0, 1.0, FALSE);
	}*/
}