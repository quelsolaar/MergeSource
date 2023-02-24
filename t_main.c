#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define TESTIFY_INTERNAL
#include "testify.h"

#ifdef TESTIFY_DEBUG
#define TESTIFY_HEADER_SIZE_MAX 64
#else
#define TESTIFY_HEADER_SIZE_MAX 0
#endif
/*
typedef unsigned int	uint;

typedef unsigned char	boolean;
typedef signed char		int8;
typedef unsigned char	uint8;
typedef short			int16;
typedef unsigned short	uint16;
typedef int				int32;
typedef unsigned int	uint32;
typedef long			int64;
typedef unsigned long	uint64;
typedef float			real32;
typedef double			real64;
*/

char *testify_debug_magic_number = "UnRaVeLdEbUg";
/*
typedef enum{
	T_TYPE_UINT8,
	T_TYPE_INT8,
	T_TYPE_UINT16,
	T_TYPE_INT16,
	T_TYPE_UINT32,
	T_TYPE_INT32,
	T_TYPE_UINT64,
	T_TYPE_INT64,
	T_TYPE_REAL32,
	T_TYPE_REAL64,
	T_TYPE_STRING,
	T_TYPE_UINT8_VECTOR,
	T_TYPE_INT8_VECTOR,
	T_TYPE_UINT16_VECTOR,
	T_TYPE_INT16_VECTOR,
	T_TYPE_UINT32_VECTOR,
	T_TYPE_INT32_VECTOR,
	T_TYPE_UINT64_VECTOR,
	T_TYPE_INT64_VECTOR,
	T_TYPE_REAL32_VECTOR,
	T_TYPE_REAL64_VECTOR,
	T_TYPE_STRING_VECTOR
}UTypes;*/
/*
extern void		testify_put_uint8(FILE *f, uint8 value, char *name);
extern void		testify_put_int8(FILE *f, int8 value, char *name);
extern void		testify_put_uint16(FILE *f, uint16 value, char *name);
extern void		testify_put_int16(FILE *f, int16 value, char *name);
extern void		testify_put_uint32(FILE *f, uint32 value, char *name);
extern void		testify_put_int32(FILE *f, int32 value, char *name);
extern void		testify_put_uint64(FILE *f, uint64 value, char *name);
extern void		testify_put_int64(FILE *f, int64 value, char *name);
extern void		testify_put_real32(FILE *f, float value, char *name);
extern void		testify_put_real64(FILE *f, double value, char *name);
extern void		testify_put_string(FILE *f, char *value, char *name);

typedef struct{
	FILE *f;
	FILE *text;
	boolean debug;
}UFile;
*/
/*
const char *testify_type_strings[] = {"T_TYPE_UINT8",
								"T_TYPE_INT8",
								"T_TYPE_UINT16",
								"T_TYPE_INT16",
								"T_TYPE_UINT32",
								"T_TYPE_INT32",
								"T_TYPE_UINT64",
								"T_TYPE_INT64",
								"T_TYPE_REAL32",
								"T_TYPE_REAL64",
								"T_TYPE_STRING"
								"T_TYPE_UINT8_VECTOR",
								"T_TYPE_INT8_VECTOR",
								"T_TYPE_UINT16_VECTOR",
								"T_TYPE_INT16_VECTOR",
								"T_TYPE_UINT32_VECTOR",
								"T_TYPE_INT32_VECTOR",
								"T_TYPE_UINT64_VECTOR",
								"T_TYPE_INT64_VECTOR",
								"T_TYPE_REAL32_VECTOR",
								"T_TYPE_REAL64_VECTOR",
								"T_TYPE_STRING_VECTOR"};*/
/*
const uint testify_type_size[] = {sizeof(uint8),
								sizeof(int8),
								sizeof(uint16),
								sizeof(int16),
								sizeof(uint32),
								sizeof(int32),
								sizeof(uint64),
								sizeof(int64),
								sizeof(real32),
								sizeof(real64),
								sizeof(char)};
*/

extern void testify_socket_destroy(uint32 socket);

void testify_handle_clear(THandle *handle, uint type)
{
	uint testify_buffer_size[] = {4096, /* T_HT_STREAMING_SERVER */
									4096, /* T_HT_STREAMING_CONNECTION */
									1500, /* T_HT_PACKET_PEER (MTU) */
									8192, /* T_HT_FILE_READ */
									8192, /* T_HT_FILE_WRITE */
									8192}; /* T_HT_BUFFER */
	handle->ip = 0;
	handle->port = 0;
	handle->socket = -1;
	handle->type = type;
	handle->read_buffer_size = testify_buffer_size[handle->type];
	if(type != T_HT_FILE_WRITE)
		handle->read_buffer = malloc((sizeof *handle->read_buffer) * handle->read_buffer_size);
	else
		handle->read_buffer = NULL;
	handle->read_buffer_used = 0;
	handle->read_buffer_pos = 0;
	handle->read_marker = -1;
	handle->read_raw_progress = 0;
	if(type == T_HT_BUFFER)
	{
		handle->write_buffer = handle->read_buffer;
		handle->write_buffer_size = handle->read_buffer_size;
	}else if(type != T_HT_FILE_READ)
	{
		handle->write_buffer_size = testify_buffer_size[handle->type];
		handle->write_buffer = malloc((sizeof *handle->read_buffer) * handle->write_buffer_size);
	}else
		handle->write_buffer = NULL;
	handle->write_buffer_pos = 0;
	handle->write_raw_progress = 0;
	handle->file = NULL;
	handle->text_copy = NULL;
	handle->debug_descriptor = FALSE;
	handle->connected = TRUE;
	handle->debug_header = FALSE;
	handle->file_name = NULL;
}

boolean testify_network_stream_connected(THandle *handle)
{
	return handle->connected;
}


void testify_debug_mode_set(THandle *handle, boolean debug, char *text_copy_name)
{
#ifndef TESTIFY_DEBUG
	if(debug || text_copy_name != NULL)
	{
		printf("UNRAVEL error, you cant testify_debug_mode_set with out having TESTIFY_DEBUG turned on\n");
		exit(0);
	}
#endif
	if(debug && handle->write_buffer != NULL && !handle->debug_descriptor && handle->type != T_HT_BUFFER)
	{
		uint i;
		for(i = 0; testify_debug_magic_number[i] != 0; i++)
			handle->write_buffer[handle->write_buffer_pos++] = testify_debug_magic_number[i];
	}
	handle->debug_descriptor = debug;
	if(text_copy_name != NULL)
	{
		handle->text_copy = fopen(text_copy_name, "w");
		if(handle->text_copy != NULL)
			fprintf(handle->text_copy, "Debug file for %s\n----------------------------------------------------\n", text_copy_name);
	}else if(handle->text_copy != NULL)
	{
		fclose(handle->text_copy);
		handle->text_copy = NULL;
	}
}


boolean	testify_network_address_create(THandle *handle, const char *host_name, uint16 port, THandleType type)
{
	return FALSE;
}

void testify_network_address_destroy(THandle *handle)
{

}

boolean	testify_network_wait_for_connection(THandle *handle, THandle *listen)
{
	return 0;
}

int	testify_data_receive(THandle *handle)
{
	return 0;
}

THandle *testify_file_load(char *path)
{
	THandle *handle;
	uint i;
	FILE *f;
	char *copy;
	if((f = fopen(path, "rb")) == NULL)
		return NULL;
	for(i = 0; path[i] != 0; i++);
	copy = malloc((sizeof *path) * (i + 1));
	for(i = 0; path[i] != 0; i++)
		copy[i] = path[i];
	copy[i] = 0;
	handle = malloc(sizeof *handle);
	testify_handle_clear(handle, T_HT_FILE_READ);
	handle->file_name = copy;
	handle->file = f;
	testify_unpack_buffer_get(handle);
	return handle;
}
	
THandle *testify_file_save(char *path)
{
	THandle *handle;
	uint i;
	FILE *f;
	char *copy;
	for(i = 0; path[i] != 0; i++);
	copy = malloc((sizeof *path) * (i + 5));
	sprintf(copy, "%s.tmp", path);
	if((f = fopen(copy, "wb")) == NULL)
	{
		free(copy);
		return NULL;
	}
	handle = malloc(sizeof *handle);
	testify_handle_clear(handle, T_HT_FILE_WRITE);
	for(i = 0; path[i] != 0; i++)
		copy[i] = path[i];
	copy[i] = 0;
	handle->file_name = copy;
	handle->file = f;
	return handle;
}

THandle *testify_buffer_create()
{
	THandle *handle;
	handle = malloc(sizeof *handle);
	testify_handle_clear(handle, T_HT_BUFFER);
	testify_unpack_buffer_get(handle);
	return handle;
}

void *testify_buffer_get(THandle *handle, uint32 *size)
{
	if(handle->type != T_HT_BUFFER)
	{
		printf("Unravel: Can't testify_buffer_get on non buffer handles\n");
		return NULL;
	}
	*size = handle->write_buffer_pos - handle->read_buffer_pos;
	return &handle->read_buffer[handle->read_buffer_pos];
}

void testify_buffer_set(THandle *handle, void *data, uint32 size)
{
	if(handle->type != T_HT_BUFFER)
	{
		printf("Unravel: Can't testify_buffer_set on non buffer handles\n");
		return;
	}
	if(handle->write_buffer_size - handle->write_buffer_pos < size)
	{
		handle->read_buffer_size = handle->write_buffer_size = handle->write_buffer_pos + size;
		handle->write_buffer = realloc(handle->write_buffer, handle->read_buffer_size);
		handle->read_buffer = handle->write_buffer;
	}
	memcpy(&handle->read_buffer[handle->write_buffer_pos], data, size);
	handle->write_buffer_pos += size;

}

uint64 testify_file_size(THandle *handle)
{
	uint64 pos, size;
	if(handle->type != T_HT_FILE_READ && handle->type != T_HT_FILE_WRITE)
	{
		printf("Unravel: Can't read file size on non file handles\n");
		return 0;
	}
#ifdef _WIN32 || _WIN64
	pos = _ftelli64(handle->file);
	_fseeki64(handle->file, 0, SEEK_END);
	size = _ftelli64(handle->file);
	_fseeki64(handle->file, pos, SEEK_SET);
#else
	pos = ftello(handle->file);
	fseeko(handle->file, 0, SEEK_END);
	size = ftello(handle->file);
	fseeko(handle->file, pos, SEEK_SET);
#endif
	return size;
}

void testify_file_position_set(THandle *handle, uint64 pos)
{
	if(handle->type != T_HT_FILE_READ && handle->type != T_HT_FILE_WRITE)
	{
		printf("Unravel: Can't read file size on non file handles\n");
		return;
	}	
	handle->read_buffer_pos = 0;
	handle->read_buffer_used = 0;
#ifdef _WIN32 || _WIN64
	_fseeki64(handle->file, pos, SEEK_SET);
#else
	fseeko(handle->file, pos, SEEK_SET);
#endif
}

uint64 testify_file_position_get(THandle *handle)
{
	uint64 pos;
	if(handle->type != T_HT_FILE_READ && handle->type != T_HT_FILE_WRITE)
	{
		printf("Unravel: Can't read file size on non file handles\n");
		return 0;
	}	
#ifdef _WIN32 || _WIN64
	pos = _ftelli64(handle->file) + handle->read_buffer_pos - handle->read_buffer_used;
#else
	pos = ftello(handle->file) + handle->read_buffer_pos - handle->read_buffer_used;
#endif
	return pos;
}

boolean testify_pack_buffer_clear(THandle *handle)
{
	int out = 0;
	if(handle->type == T_HT_FILE_WRITE)
	{
		out = fwrite(handle->write_buffer, (sizeof *handle->write_buffer), handle->write_buffer_pos, handle->file);
		handle->write_buffer_pos = 0;
	}

	if(handle->type == T_HT_STREAMING_SERVER || handle->type == T_HT_STREAMING_CONNECTION)
	{
		out = testify_network_stream_send_force(handle);
	}
	if(out == -1)
		return FALSE;
	if(handle->write_buffer_size - handle->write_buffer_pos < TESTIFY_MINIMUM_WRITE_SPACE)
	{
		handle->write_buffer_size += TESTIFY_MINIMUM_WRITE_SPACE;
		handle->write_buffer = realloc(handle->write_buffer, (sizeof *handle->write_buffer) * handle->write_buffer_size);
		if(handle->type == T_HT_BUFFER)
		{
			handle->read_buffer = handle->write_buffer;
			handle->read_buffer_size = handle->write_buffer_size;
		}
	}
	return TRUE;
}

void testify_pack_buffer_add(THandle *handle, uint8 *data, uint length)
{
	uint pos = 0, size;
	size = handle->write_buffer_size;
	while(pos < length)
	{
		for(; pos < length && handle->write_buffer_pos < handle->write_buffer_size; pos++)
			handle->write_buffer[handle->write_buffer_pos++] = data[pos];

		if(handle->write_buffer_pos == handle->write_buffer_size)
			if(!testify_pack_buffer_clear(handle))
				return; /* output no longer available */
	}
}

uint testify_unpack_buffer_get(THandle *handle)
{
	size_t size, i, start;
	if(handle->read_marker != -1)
		start = handle->read_marker;
	else
		start = handle->read_buffer_pos;
	if(start > 0)
	{
		uint8 *buf;
		buf = handle->read_buffer;
		size = handle->read_buffer_used - start;
		if(size < 0)
			i = 0;
		for(i = 0; i < size; i++)
			buf[i] = buf[i + start];
		if(handle->read_buffer_pos < start)
		{
			printf("My error\n");
			exit(0);
		}
		if(handle->read_marker != -1)
			handle->read_marker -= start;

		handle->read_buffer_pos -= start;
		handle->read_buffer_used = size;
		if(handle->type == T_HT_BUFFER)
			handle->write_buffer_pos -= start;
	}

	if(handle->type != T_HT_BUFFER && handle->read_buffer_used + 1024 > handle->read_buffer_size)
	{
		handle->read_buffer_size *= 2;
		handle->read_buffer = realloc(handle->read_buffer, (sizeof *handle->read_buffer) * handle->read_buffer_size);
		for(i = handle->read_buffer_used; i < handle->read_buffer_size; i++)
			handle->read_buffer[i] = 69;
	}
	if(handle->read_buffer_used < handle->read_buffer_size)
	{
		size =  0;
		if(handle->type == T_HT_FILE_READ)
			size = fread(&handle->read_buffer[handle->read_buffer_used], 
							(sizeof *handle->read_buffer), 
							handle->read_buffer_size - handle->read_buffer_used, handle->file);
		if(handle->type == T_HT_STREAMING_SERVER || handle->type == T_HT_STREAMING_CONNECTION)
			size = testify_network_stream_receve(handle, &handle->read_buffer[handle->read_buffer_used], handle->read_buffer_size - handle->read_buffer_used);
		if(size > 0)
		{
			if(!handle->debug_header)
			{
				for(i = 0; testify_debug_magic_number[i] != 0 && handle->read_buffer[i] == testify_debug_magic_number[i]; i++);
				if(testify_debug_magic_number[i] == 0)
				{
					handle->debug_descriptor = TRUE;
					handle->read_buffer_pos += i;
				}
			}
			handle->read_buffer_used += size;
			return TRUE;
		}
	}
	if(handle->type == T_HT_BUFFER)
	{
		handle->read_buffer_used = handle->write_buffer_pos;
	}
	return FALSE;
}


void	testify_buffer_print(THandle *handle)
{
	char chararcters[2] = {0, 0};
	uint i;
	for(i = 0; i < handle->read_buffer_used; i++)
	{
		chararcters[0] = handle->read_buffer[i];
		printf("buffer[%u] = %u %s\n", i, handle->read_buffer[i], chararcters);
	}
}
THandleType	testify_type(THandle *handle)
{
	return handle->type;
}

void testify_free(THandle *handle)
{
	if(handle->type != T_HT_FILE_READ)
		testify_pack_buffer_clear(handle);
	if(handle->text_copy != NULL)
	{
		fprintf(handle->text_copy, "testify_free\n");
		fclose(handle->text_copy);
	}

	if(handle->read_buffer != NULL)
		free(handle->read_buffer);
	if(handle->type != T_HT_BUFFER && handle->write_buffer != NULL)
		free(handle->write_buffer);
	if(handle->file != NULL)
		fclose(handle->file);

	if(handle->socket != -1)
		testify_socket_destroy(handle->socket);

	if(handle->type == T_HT_FILE_WRITE)
	{
		uint i;
		char *buffer, *alt;
		for(i = 0; handle->file_name[i] != 0; i++);
		buffer = malloc((sizeof*buffer) * (i + 5));
		sprintf(buffer, "%s.tmp", handle->file_name);
		remove(handle->file_name);
		if(0 != rename(buffer, handle->file_name))
		{
			alt = malloc((sizeof*buffer) * (i + 32));
			sprintf(alt, "%s.emergency", handle->file_name);
			for(i = 0; 0 != rename(alt, handle->file_name) && i < 1024; i++)
				sprintf(alt, "%s.%uemergency", handle->file_name, i);
			free(alt);
		}
		free(buffer);
	}
	free(handle);
}
/*
void buffer_test()
{
	THandle *h, *h2;
	h = testify_buffer_create();
	testify_pack_string(h, "This is the text im trying to pipe!", "text");
	h2 = testify_file_save("My_Unravel_pipe_file.txt");
	testify_pipe(h, h2, -1);
	testify_free(h);
	testify_free(h2);
	exit(0);
}*/