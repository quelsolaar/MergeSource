#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "betray.h"

#define BETRAY_AUDIO_MIXER

#ifdef BETRAY_AUDIO_MIXER

#define BETRAY_READ_BUFFER_SIZE (11025 * 16)

typedef struct{
	uint sound;
	uint progress;
	uint progress_add;
	uint time_add;
	float pos[3];
	float vector[3];
	float speed;
	float volume;
	float play_distance;
	float play_speed;
	float play_volume;
	float play_vec[3];	
	float prev_play_speed;
	float prev_play_volume;
	float prev_play_vec[3];
	uint16 *data;
	uint data_place;
	uint data_left;
	boolean loop;
	boolean active;
	boolean deleted;
	boolean ambient;
	boolean auto_deleted;
}BSourceStorage;

BSourceStorage *b_source_storage = NULL;
uint  b_source_storage_count = 0;
uint  b_source_storage_hole = 0;
uint  b_source_storage_alloc = 0;

FILE *debug_output = NULL;

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
float b_listener_speed_of_sound = 0.1;

uint betray_audio_util_sound_create(uint type, uint stride, uint length, uint frequency, void *data, char *name)
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

void betray_audio_util_sound_destroy(uint sound)
{
	if(sound >= b_sound_storage_count)
		return;
	free(b_sound_storage[b_sound_storage_count].data);
}

void betray_audio_util_sound_set(uint source, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient)
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

boolean betray_audio_util_sound_is_playing(uint source)
{
	return b_source_storage[source].active;
}

void betray_audio_util_time_progress(BSourceStorage *s, float time)
{
	float f, speed, listener[3] = {0, 0, 0}, vec[3];

	s->prev_play_speed = s->play_speed;
	s->prev_play_volume = s->play_volume;
	s->prev_play_vec[0] = s->play_vec[0];
	s->prev_play_vec[1] = s->play_vec[1];
	s->prev_play_vec[2] = s->play_vec[2];
	s->play_volume = s->volume * b_sound_volume;
	if(s->ambient)
	{
		s->play_vec[0] = s->pos[0];
		s->play_vec[1] = s->pos[1];
		s->play_vec[2] = s->pos[2];
		if(s->data != NULL)
			s->play_speed = s->speed;
		else
			s->play_speed = s->speed * 44100.0 / (float)b_sound_storage[s->sound].frequency;
	}else
	{
		vec[0] = b_listener_pos[0] - s->pos[0];
		vec[1] = b_listener_pos[1] - s->pos[1];
		vec[2] = b_listener_pos[2] - s->pos[2];

		s->play_vec[0] = b_listener_matrix[0] * vec[0] + b_listener_matrix[1] * vec[1] + b_listener_matrix[2] * vec[2];
		s->play_vec[1] = b_listener_matrix[3] * vec[0] + b_listener_matrix[4] * vec[1] + b_listener_matrix[5] * vec[2];
		s->play_vec[2] = b_listener_matrix[6] * vec[0] + b_listener_matrix[7] * vec[1] + b_listener_matrix[8] * vec[2];

		f = sqrt(s->play_vec[0] * s->play_vec[0] + s->play_vec[1] * s->play_vec[1] + s->play_vec[2] * s->play_vec[2]);
		if(f > b_listener_scale)
			s->play_volume /= (f / b_listener_scale);
		
		s->play_vec[0] /= f;
		s->play_vec[1] /= f;
		s->play_vec[2] /= f;
		if(s->data != NULL)
			s->play_speed = s->speed;
		else
		{
			speed = (s->speed + (s->play_distance - f) / time * b_listener_speed_of_sound) * (float)b_sound_storage[s->sound].frequency / 44100.0;
			if(speed < s->speed * 0.1)
				speed = s->speed * 0.1;
			speed *= (float)b_sound_storage[s->sound].frequency / 44100.0;
		}
		s->play_distance = f;
		s->play_speed = speed;
		s->pos[0] += s->vector[0] * time;
		s->pos[1] += s->vector[1] * time;
		s->pos[2] += s->vector[2] * time;
	}
	s->play_volume -= b_sound_volume_silence_cutoff;
}

uint betray_audio_util_sound_play(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete)
{
	if(sound == -1)
		return -1;
	for(b_source_storage_hole = 0; b_source_storage_hole < b_source_storage_count && !b_source_storage[b_source_storage_hole].deleted; b_source_storage_hole++);
	if(b_source_storage_hole == b_source_storage_alloc)
	{
		b_source_storage_alloc += 32;
		b_source_storage = realloc(b_source_storage, (sizeof *b_source_storage) * b_source_storage_alloc);
	}
	b_source_storage[b_source_storage_hole].data = NULL;
	b_source_storage[b_source_storage_hole].sound = sound;
	b_source_storage[b_source_storage_hole].progress = 0;
	b_source_storage[b_source_storage_hole].progress_add = 0;
	b_source_storage[b_source_storage_hole].active = TRUE;
	b_source_storage[b_source_storage_hole].deleted = FALSE;
	b_source_storage[b_source_storage_hole].auto_deleted = auto_delete;
	betray_audio_util_sound_set(b_source_storage_hole, pos, vector, speed, volume, loop, ambient);
	betray_audio_util_time_progress(&b_source_storage[b_source_storage_hole], 0.01);
	betray_audio_util_time_progress(&b_source_storage[b_source_storage_hole], 0.0);
	b_source_storage_hole++;
	if(b_source_storage_hole > b_source_storage_count)
		b_source_storage_count = b_source_storage_hole;
	return b_source_storage_hole - 1;
}

void betray_audio_util_sound_stop(uint source)
{
	if(source >= b_source_storage_count)
		return;
	b_source_storage[source].active = FALSE;
	b_source_storage[source].deleted = TRUE;
	if(source + 1 == b_sound_storage_count)
	{
		for(b_sound_storage_count--; b_source_storage[b_sound_storage_count].deleted &&	b_sound_storage_count > 0; b_sound_storage_count--);
		if(b_sound_storage_count < b_source_storage_hole)
			b_source_storage_hole = b_sound_storage_count;
	}
	if(source < b_source_storage_hole)
		b_source_storage_hole = source;
}

uint betray_audio_util_stream_create(uint frequency, float *pos, float *vector,  float volume, boolean ambient)
{
	for(b_source_storage_hole = 0; b_source_storage_hole < b_source_storage_count && b_source_storage[b_source_storage_hole].deleted; b_source_storage_hole++);
	if(b_source_storage_hole == b_source_storage_alloc)
	{
		b_source_storage_alloc += 32;
		b_source_storage = realloc(b_source_storage, (sizeof *b_source_storage) * b_source_storage_alloc);
	}
	b_source_storage[b_source_storage_hole].data_place = 0;
	b_source_storage[b_source_storage_hole].data_left = 0;
	b_source_storage[b_source_storage_hole].data = malloc((sizeof *b_source_storage[b_source_storage_hole].data) * BETRAY_READ_BUFFER_SIZE);

	b_source_storage[b_source_storage_hole].sound = -1;
	b_source_storage[b_source_storage_hole].progress = 0;
	b_source_storage[b_source_storage_hole].progress_add = 0;
	b_source_storage[b_source_storage_hole].active = TRUE;
	b_source_storage[b_source_storage_hole].deleted = FALSE;
	betray_audio_util_sound_set(b_source_storage_hole, pos, vector, (float)frequency / 44100.0, volume, FALSE, ambient);
	betray_audio_util_time_progress(&b_source_storage[b_source_storage_hole], 0.01);
	betray_audio_util_time_progress(&b_source_storage[b_source_storage_hole], 0.0);
	b_source_storage_hole++;
	if(b_source_storage_hole > b_source_storage_count)
		b_source_storage_count = b_source_storage_hole;
	return b_source_storage_hole - 1;
}

void betray_audio_util_stream_destroy(uint stream)
{
	if(stream >= b_source_storage_count)
		return;
	b_source_storage[stream].active = FALSE;
	b_source_storage[stream].deleted = FALSE;
	if(b_source_storage[stream].data != NULL)
		free(b_source_storage[stream].data);
	b_source_storage[stream].data = NULL;
	if(stream + 1 == b_sound_storage_count)
	{
		for(b_sound_storage_count--; !b_source_storage[b_sound_storage_count].active &&	b_sound_storage_count > 0; b_sound_storage_count--);
		if(b_sound_storage_count < b_source_storage_hole)
			b_source_storage_hole = b_sound_storage_count;
	}
	if(stream < b_source_storage_hole)
		b_source_storage_hole = stream;
}

void betray_audio_util_stream_feed(uint stream, uint type, uint stride, uint length, void *data)
{
	uint i, data_place;
	int8 *pint8;
	int16 *pint16, *p;
	int32 *pint32;
	float *preal32;

	if(debug_output == NULL)
		debug_output = fopen("plugin_debug.txt", "w");
//	fprintf(debug_output, "length %u\n",  length);
	if(b_source_storage[stream].data_left + length > BETRAY_READ_BUFFER_SIZE)
		length = BETRAY_READ_BUFFER_SIZE - b_source_storage[stream].data_left;
	fprintf(debug_output, "length %u\n",  length);	
	p = b_source_storage[stream].data;
	data_place = b_source_storage[stream].data_place;
	switch(type)
	{
		case BETRAY_TYPE_INT8 :
			pint8 = data;
			for(i = 0; i < length; i++)
			{
				p[data_place] = (int16)pint8[i * stride] * 256;
				data_place = (data_place + 1) % BETRAY_READ_BUFFER_SIZE;
			}
		break;
		case BETRAY_TYPE_INT16 :
			pint16 = data;
			for(i = 0; i < length; i++)
			{
				p[data_place] = pint16[i * stride];
				data_place = (data_place + 1) % BETRAY_READ_BUFFER_SIZE;
			}
		break;
		case BETRAY_TYPE_INT32 :
			pint32 = data;
			for(i = 0; i < length; i++)
			{
				p[data_place] = (int16)(pint32[i * stride] / (256 * 256));
				data_place = (data_place + 1) % BETRAY_READ_BUFFER_SIZE;
			}
		break;
		case BETRAY_TYPE_FLOAT32 :
			preal32 = data;

			for(i = 0; i < length; i++)
			{
				p[data_place] = (int16)(preal32[i * stride] * (256.0 * 128.0 - 1.0));
				data_place = (data_place + 1) % BETRAY_READ_BUFFER_SIZE;
			}
		break;
	}
//	fprintf(debug_output, "Ingest %u samples from pos %u -> %u. Data left %u -> %u (adding %u)\n", length, b_source_storage[stream].data_place, data_place, b_source_storage[stream].data_left, b_source_storage[stream].data_left + length, length);
	b_source_storage[stream].data_place = data_place;
	b_source_storage[stream].data_left += length;
}

uint betray_audio_util_stream_buffer_left(uint stream)
{
	return b_source_storage[stream].data_left;
}

void betray_audio_util_stream_set(uint stream, float *pos, float *vector,  float volume, boolean ambient)
{
	betray_audio_util_sound_set(stream, pos, vector, b_source_storage[stream].speed, volume, FALSE, ambient);
}

void betray_audio_util_update_callback(void *data, uint length, uint padding, float *vec)
{
	float tmp[3], speed, prev_speed, f, gap;
	BSourceStorage *e;
	BSoundStorage *s;
	int16 *buf;
	uint i, j, k, p, dist, end;
	int volume, *fbuf, prev_volume, v;

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
				f = 0.5 + (e->play_vec[0] * vec[0] + e->play_vec[1] * vec[1] + e->play_vec[2] * vec[2]) / 2.0;
				f = 1.0 - (1.0 - f) * (1.0 - f);
				if(f > 1.0)
					f = 1.0;
				volume = (int)(e->play_volume * f * 32768.0);
				f = 0.5 + (e->prev_play_vec[0] * vec[0] + e->prev_play_vec[1] * vec[1] + e->prev_play_vec[2] * vec[2]) / 2.0;
				f = 1.0 - (1.0 - f) * (1.0 - f);
				if(f > 1.0)
					f = 1.0;
				prev_volume = (int)(e->play_volume * f * 32768.0);
			}else
			{
				volume = (int)(e->play_volume * 32768.0);
				prev_volume =(int)(e->prev_play_volume * 32768.0);
			}
			if(e->data != NULL)
			{
				if(e->data_left > 0)
				{
					if(debug_output == NULL)
						debug_output = fopen("plugin_debug.txt", "w");
					fprintf(debug_output, "Callback: speed %f ", e->play_speed);
					fprintf(debug_output, "place %u e->data_left %u", e->data_place, e->data_left);
					buf = e->data;
					end = length;
					p = e->data_place + BETRAY_READ_BUFFER_SIZE - (e->data_left);
					fprintf(debug_output, " P %u ", p);
					dist = (e->play_speed * (float)length);
					end = (dist * length);
					if(dist > e->data_left)
						dist = e->data_left;
					for(j = 0; j < length; j++)
					{
						v = (prev_volume * (length - j) + volume * j) / length;
						fbuf[j] += (buf[(p + (dist * j / length)) % BETRAY_READ_BUFFER_SIZE] * v) / 32768;
					//	fbuf[j] += (buf[(p + j) % BETRAY_READ_BUFFER_SIZE] * v) / 32768;
					}
					e->progress_add = dist;
					fprintf(debug_output, "dist %u (%u)\n", dist, length);
					fprintf(debug_output, "read %u -> %u - dist %u length %u\n", p % BETRAY_READ_BUFFER_SIZE, (p + (dist * j / length)) % BETRAY_READ_BUFFER_SIZE, dist, length);
				}
			}else
			{
				s = &b_sound_storage[e->sound];
				prev_speed = e->prev_play_speed; 
				speed = e->play_speed;

				if(speed > 10.0)
					speed = 10.0;
				if(prev_speed > 10.0)
					prev_speed = 10.0;
				if(speed < 0.01)
					speed = 0.01;
				if(prev_speed < 0.01)
					prev_speed = 0.01;
				if(speed > 0.0 && volume > 0.0)
				{
					if(e->loop)
					{
						uint d0, d1;
						buf = s->data;
						end = s->length;
						p = e->progress;
						d0 = (int)(prev_speed * (float)length);
						d1 = (int)((prev_speed + speed) * (float)length / 2.0);
						for(j = 0; j < length; j++)
						{
							dist = (d0 * (length - j) + d1 * j) / length;
							v = (prev_volume * (length - j) + volume * j) / length;
							fbuf[j] += (buf[(p + (dist * j / length)) % end] * v) / 32768;
						}
						dist = (d0 * (length - j) + d1 * j) / length;
						e->progress_add = (p + (dist * j / length)) % end;
					}else
					{
						uint d0, d1;
						buf = s->data;
						end = length;
						p = e->progress;
						d0 = (int)(prev_speed * (float)length);
						d1 = (int)((prev_speed + speed) * (float)length / 2.0);

						if(e->progress + d1 >= length)
							end = (length - e->progress) / speed;

						for(j = 0; j < end; j++)
						{
							dist = (d0 * (length - j) + d1 * j) / length;
							v = (prev_volume * (length - j) + volume * j) / length;
							fbuf[j] += (buf[(p + (dist * j / length))] * v) / 32768;
						}
						dist = (d0 * (length - j) + d1 * j) / length;
						e->progress_add = p + (dist * j / length);
						
					}
				}
			}
		}
	}
	for(i = 0; i < length; i++)
	{
		if(fbuf[i] > 32767)
			fbuf[i] = 32767;
		else if(fbuf[i] < -32766)
			fbuf[i] = -32766;
		((int16 *)data)[i * padding] = fbuf[i];
	}

	free(fbuf);
}



void betray_audio_util_time_callback(uint length)
{

	BSourceStorage *s;
	uint i;
	for(i = 0; i < b_source_storage_count; i++)
	{
		if(b_source_storage[i].active)
		{
			s = &b_source_storage[i];
		//	s->progress += (uint)((float)length * (b_source_storage[i].play_speed + b_source_storage[i].prev_play_speed) / 2.0);
			s->progress = s->progress_add;
			if(s->sound != -1)
			{
				if(s->loop)
					s->progress = s->progress % b_sound_storage[s->sound].length;
				else if(s->progress >= b_sound_storage[s->sound].length)
				{
					s->active = FALSE;
					if(s->auto_deleted)
						s->deleted = FALSE;
				}
			}
			if(s->data != NULL)
			{
				if(s->data_left < s->progress_add)
					s->data_left = BETRAY_READ_BUFFER_SIZE;
				else
					s->data_left -= s->progress_add;

				if(debug_output == NULL)
					debug_output = fopen("plugin_debug.txt", "w");
				fprintf(debug_output, "betray_audio_util_time_callback(length = %u)\n", length);

			}
			if(s->active)
				betray_audio_util_time_progress(s, (float)length / 44100.0);
		}
	}
}

void betray_audio_util_listener(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound)
{
	b_listener_pos[0] = pos[0];
	b_listener_pos[1] = pos[1];
	b_listener_pos[2] = pos[2];
	b_listener_scale = scale * 1000.0;
	b_listener_speed_of_sound = speed_of_sound;
	b_listener_matrix[0] = side[0];
	b_listener_matrix[1] = side[1];
	b_listener_matrix[2] = side[2];
	b_listener_matrix[3] = side[1] * forward[2] - side[2] * forward[1];
	b_listener_matrix[4] = side[2] * forward[0] - side[0] * forward[2];
	b_listener_matrix[5] = side[0] * forward[1] - side[1] * forward[0];
	b_listener_matrix[6] = forward[0];
	b_listener_matrix[7] = forward[1];
	b_listener_matrix[8] = forward[2];
}

void betray_audio_util_master_volume_set(float volume)
{
	b_sound_volume = volume;
}

float betray_audio_util_master_volume_get()
{
	return b_sound_volume;
}

void betray_audio_util_master_volume_silence_cutoff(float volume)
{
	b_sound_volume_silence_cutoff = volume;
}

#endif
