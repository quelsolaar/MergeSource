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
	T_TYPE_STRUCT,
	T_TYPE_COUNT,
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
								"T_TYPE_RAW",
								"T_TYPE_STRING"};

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
								sizeof(uint8),
								sizeof(char)};



extern boolean testify_pack_buffer_clear(THandle *handle);
extern void testify_pack_buffer_add(THandle *handle, uint8 *data, uint length);
/* --------- */

boolean testify_pack_debug_header(THandle *handle, uint type, uint vector_length, char *name, char *file, uint line, int value_int, double value_double, char *value_string)
{
	/* Check if the data is breaking the MTU limit*/
	if(handle->write_raw_progress != 0)
	{
		if(type == T_TYPE_RAW)
			return FALSE;
		printf("Testify error: on line %u in file: %s\n", line, file);
		printf("-Loading non raw data in to a raw stream that have not been completed.\n");
		exit(0);		
	}


	if(handle->write_buffer_pos + testify_type_size[type] > handle->write_buffer_size && handle->type == T_HT_PACKET_PEER)
	{
		printf("Testify error: on line %u in file: %s\n", line, file);
		printf("-Adding more data then can fit in a single UDP packet without calling send\n");
		exit(0);
	}




	/* send or save data if buffer is filling up */
	if(handle->write_buffer_pos + testify_type_size[type] > handle->write_buffer_size - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return FALSE; /* destination is gone */

	/* include the data description header */
	if(handle->debug_descriptor)
	{
		uint i;
		handle->write_buffer[handle->write_buffer_pos++] = type;
		handle->write_buffer[handle->write_buffer_pos++] = (vector_length >> 24) & 0xFF;
		handle->write_buffer[handle->write_buffer_pos++] = (vector_length >> 16) & 0xFF;
		handle->write_buffer[handle->write_buffer_pos++] = (vector_length >> 8)  & 0xFF;
		handle->write_buffer[handle->write_buffer_pos++] = vector_length & 0xFF;
		for(i = 0; name[i] != 0; i++)
			handle->write_buffer[handle->write_buffer_pos++] = name[i];
		handle->write_buffer[handle->write_buffer_pos++] = 0;
	}

	/* save the text version of the packet */
	if(handle->text_copy != NULL)
	{

		if(vector_length > 1 || type == T_TYPE_RAW)
			fprintf(handle->text_copy, "%s %s length %u\n", testify_type_strings[type], name, value_int);
		else if(type == T_TYPE_STRING)
			fprintf(handle->text_copy, "%s %s = %s\n", testify_type_strings[type], name, value_string);
		else if(type >= T_TYPE_REAL32)
			fprintf(handle->text_copy, "%s %s = %f\n", testify_type_strings[type], name, value_double);
		else
			fprintf(handle->text_copy, "%s %s = %i\n", testify_type_strings[type], name, value_int);
	}
	return TRUE;
}

void testify_unpack_debug_header(THandle *handle, uint type, uint vector_length, char *name, char *file, uint line)
{
	/* Check if the data is breaking the MTU limit*/
/*	if(handle->read_buffer_pos + testify_type_size[type] > testify_buffer_size[handle->type] && handle->type == T_HT_PACKET_PEER)
	{
		printf("UNRAVEL error on line %u in file: %s\n", line, file);
		printf("-Adding more data then can fit in a single UDP packet without calling send\n");
		exit(0);
	}

	/* read the the data description header */
	if(handle->debug_descriptor)
	{
		char read[32];
		uint i, j, read_type, length, start;

		if(handle->read_raw_progress != 0)
		{
			if(type == T_TYPE_RAW)
				return;
			printf("Testify error: on line %u in file: %s\n", line, file);
			printf("-Reading non raw data from a raw stream that have not been completed.\n");
			exit(0);		
		}

		start = handle->read_buffer_pos;

		if(handle->read_buffer_pos + 5 > handle->read_buffer_used)
		{
			printf("TESTIFY Error: Trying to read data that isnt there in file %s on line %u %u %u\n", file, line, handle->read_buffer_pos + 5, handle->read_buffer_used);
			exit(0);
		}
		read_type = handle->read_buffer[handle->read_buffer_pos++];
		length  = ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 24;
		length |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 16;
		length |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 8;
		length |= handle->read_buffer[handle->read_buffer_pos++];


		for(i = 0; handle->read_buffer[handle->read_buffer_pos + i] != 0 && i < 31; i++)
			read[i] = handle->read_buffer[handle->read_buffer_pos + i];
		read[i] = 0;

		for(j = 0; name[j] == read[j] && name[j] != 0; j++);

		if(name[j] != read[j]/* || read_type != type*/)
		{
			printf("TESTIFY read error on line %u in file: %s (%u)\n", line, file, handle->read_buffer_pos);
			printf("-Reading %s -> -%s-\n", testify_type_strings[read_type], read);
			printf("-Expecting %s -> -%s-\n", testify_type_strings[type], name);
			if(handle->file_name != NULL)
				printf("File Name: %s\n", handle->file_name);
			for(i = 0; i < handle->read_buffer_used && i < handle->read_buffer_pos + 100; i++)
			{
				char character[2] = {0, 0};
				character[0] = handle->read_buffer[i];
				if(i == handle->read_buffer_pos)
					printf("Buffer [%u] = %s (%u) <------------------\n", i, character, (uint)handle->read_buffer[i]);
				else
					printf("Buffer [%u] = %s (%u)\n", i, character, (uint)handle->read_buffer[i]);
			}
			exit(0);
		}

		if(length != vector_length)
		{
			uint j;
			printf("TESTIFY read error on line %u in file: %s\n", line, file);
			for(j = 0; j < 22; j++)
				printf(" %u ", (uint)handle->read_buffer[start + j]);
			printf("\n");
			printf("-Reading %s with a vector length of %u %u\n", name, length, (uint)handle->read_buffer_pos);
			printf("-Expecting a length of %u\n", vector_length);
			exit(0);
		}
		handle->read_buffer_pos += i + 1;
	}
}



/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_uint8_internal(THandle *handle, uint8 value, char *name, char *file, uint line)
#else
void testify_pack_uint8_internal(THandle *handle, uint8 value)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_UINT8, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_UINT8] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	handle->write_buffer[handle->write_buffer_pos++] = value;
}

#ifdef TESTIFY_DEBUG
uint8 testify_unpack_uint8_internal(THandle *handle, char *name, char *file, uint line)
#else					
uint8 testify_unpack_uint8_internal(THandle *handle)
#endif						
{
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_UINT8] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_UINT8, 1, name, file, line);
#endif
	/* write out the data */
	return handle->read_buffer[handle->read_buffer_pos++];
}


extern void		testify_pack_vector_uint8_internal(THandle *handle, uint8 value, char *name, uint length, char *file, uint line);

/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_int8_internal(THandle *handle, int8 value, char *name, char *file, uint line)
#else
void testify_pack_int8_internal(THandle *handle, int8 value)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_INT8, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_INT8] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	handle->write_buffer[handle->write_buffer_pos++] = value;
}

#ifdef TESTIFY_DEBUG
int8 testify_unpack_int8_internal(THandle *handle, char *name, char *file, uint line)
#else					
int8 testify_unpack_int8_internal(THandle *handle)
#endif					
{
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_INT8] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_INT8, 1, name, file, line);
#endif
	/* write out the data */
	return (int8)handle->read_buffer[handle->read_buffer_pos++];
}

/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_uint16_internal(THandle *handle, uint16 value, char *name, char *file, uint line)
#else
void testify_pack_uint16_internal(THandle *handle, uint16 value)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_UINT16, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_UINT16] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	handle->write_buffer[handle->write_buffer_pos++] = (value & 0xFF00) >> 8;
	handle->write_buffer[handle->write_buffer_pos++] = value & 0xFF;
}

#ifdef TESTIFY_DEBUG
uint16 testify_unpack_uint16_internal(THandle *handle, char *name, char *file, uint line)
#else					
uint16 testify_unpack_uint16_internal(THandle *handle)
#endif						
{
	uint16 data;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_UINT16] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_UINT16, 1, name, file, line);
#endif
	/* write out the data */
	data = ((uint16) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	data |= (uint16) handle->read_buffer[handle->read_buffer_pos++];
	return data;
}

/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_int16_internal(THandle *handle, int16 value, char *name, char *file, uint line)
#else
void testify_pack_int16_internal(THandle *handle, int16 value)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_INT16, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_INT16] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	handle->write_buffer[handle->write_buffer_pos++] = (value & 0xFF00) >> 8;
	handle->write_buffer[handle->write_buffer_pos++] = value & 0xFF;
}

#ifdef TESTIFY_DEBUG
int16 testify_unpack_int16_internal(THandle *handle, char *name, char *file, uint line)
#else					
int16 testify_unpack_int16_internal(THandle *handle)
#endif						
{
	int16 data;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_INT16] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_INT16, 1, name, file, line);
#endif
	/* write out the data */
	data = ((int16) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	data |= (int16) handle->read_buffer[handle->read_buffer_pos++];
	return data;
}



/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_uint32_internal(THandle *handle, uint32 value, char *name, char *file, uint line)
#else
void testify_pack_uint32_internal(THandle *handle, uint32 value)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_UINT32, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_UINT32] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 24) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 16) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 8) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = value & 0xFF;
}

#ifdef TESTIFY_DEBUG
uint32 testify_unpack_uint32_internal(THandle *handle, char *name, char *file, uint line)
#else					
uint32 testify_unpack_uint32_internal(THandle *handle)
#endif						
{
	uint32 data;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_UINT32] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used /*read_buffer_size*/)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_UINT32, 1, name, file, line);
#endif
	/* write out the data */
	data = ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 24;
	data |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 16;
	data |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	data |= (uint32) handle->read_buffer[handle->read_buffer_pos++];
	return data;
}



/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_int32_internal(THandle *handle, int32 value, char *name, char *file, uint line)
#else
void testify_pack_int32_internal(THandle *handle, int32 value)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_INT32, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_INT32] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 24) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 16) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 8) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = value & 0xFF;
}

#ifdef TESTIFY_DEBUG
int32 testify_unpack_int32_internal(THandle *handle, char *name, char *file, uint line)
#else					
int32 testify_unpack_int32_internal(THandle *handle)
#endif						
{
	int32 data;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_INT32] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_INT32, 1, name, file, line);
#endif
	/* write out the data */
	data = ((int32) handle->read_buffer[handle->read_buffer_pos++]) << 24;
	data |= ((int32) handle->read_buffer[handle->read_buffer_pos++]) << 16;
	data |= ((int32) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	data |= (int32) handle->read_buffer[handle->read_buffer_pos++];
	return data;
}



/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_uint64_internal(THandle *handle, uint64 value, char *name, char *file, uint line)
#else
void testify_pack_uint64_internal(THandle *handle, uint64 value)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_UINT64, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_UINT64] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 56) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 48) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 40) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 32) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 24) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 16) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 8) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = value & 0xFF;
}

#ifdef TESTIFY_DEBUG
uint64 testify_unpack_uint64_internal(THandle *handle, char *name, char *file, uint line)
#else					
uint64 testify_unpack_uint64_internal(THandle *handle)
#endif						
{
	uint64 data;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_UINT64] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_UINT64, 1, name, file, line);
#endif
	/* write out the data */
	data = ((uint64) handle->read_buffer[handle->read_buffer_pos++]) << 56;
	data |= ((uint64) handle->read_buffer[handle->read_buffer_pos++]) << 48;
	data |= ((uint64) handle->read_buffer[handle->read_buffer_pos++]) << 40;
	data |= ((uint64) handle->read_buffer[handle->read_buffer_pos++]) << 32;
	data |= ((uint64) handle->read_buffer[handle->read_buffer_pos++]) << 24;
	data |= ((uint64) handle->read_buffer[handle->read_buffer_pos++]) << 16;
	data |= ((uint64) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	data |= (uint64) handle->read_buffer[handle->read_buffer_pos++];
	return data;
}


/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_int64_internal(THandle *handle, int64 value, char *name, char *file, uint line)
#else
void testify_pack_int64_internal(THandle *handle, int64 value)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_INT64, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_INT64] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 56) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 48) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 40) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 32) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 24) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 16) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (value >> 8) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = value & 0xFF;
}

#ifdef TESTIFY_DEBUG
int64 testify_unpack_int64_internal(THandle *handle, char *name, char *file, uint line)
#else					
int64 testify_unpack_int64_internal(THandle *handle)
#endif						
{
	int64 data;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_INT64] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_INT64, 1, name, file, line);
#endif
	/* write out the data */
	data = ((int64) handle->read_buffer[handle->read_buffer_pos++]) << 56;
	data |= ((int64) handle->read_buffer[handle->read_buffer_pos++]) << 48;
	data |= ((int64) handle->read_buffer[handle->read_buffer_pos++]) << 40;
	data |= ((int64) handle->read_buffer[handle->read_buffer_pos++]) << 32;
	data |= ((int64) handle->read_buffer[handle->read_buffer_pos++]) << 24;
	data |= ((int64) handle->read_buffer[handle->read_buffer_pos++]) << 16;
	data |= ((int64) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	data |= (int64) handle->read_buffer[handle->read_buffer_pos++];
	return data;
}

/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_real32_internal(THandle *handle, real32 value, char *name, char *file, uint line)
#else
void testify_pack_real32_internal(THandle *handle, real32 value)
#endif					
{
	union { uint32 integer; real32 real; } punt;
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_REAL32, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_REAL32] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	punt.real = value;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer >> 24) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer >> 16) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer >> 8) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = punt.integer & 0xFF;
}

#ifdef TESTIFY_DEBUG
real32 testify_unpack_real32_internal(THandle *handle, char *name, char *file, uint line)
#else					
real32 testify_unpack_real32_internal(THandle *handle)
#endif						
{
	union { uint32 integer; real32 real; } punt;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_REAL32] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_REAL32, 1, name, file, line);
#endif
	/* write out the data */
	punt.integer = ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 24;
	punt.integer |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 16;
	punt.integer |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	punt.integer |= (uint32) handle->read_buffer[handle->read_buffer_pos++];
	return punt.real;
}




/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_real64_internal(THandle *handle, real64 value, char *name, char *file, uint line)
#else
void testify_pack_real64_internal(THandle *handle, real64 value)
#endif					
{
	union { uint32 integer[2]; real64 real; } punt;
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_REAL64, 1, name, file, line, value, value, ""))
		return; /* destination is gone */
#else
	if(handle->write_buffer_pos + testify_type_size[T_TYPE_REAL64] > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return; /* destination is gone */
#endif
	punt.real = value;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer[0] >> 24) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer[0] >> 16) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer[0] >> 8) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = punt.integer[0] & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer[1] >> 24) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer[1] >> 16) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = (punt.integer[1] >> 8) & 0xFF;
	handle->write_buffer[handle->write_buffer_pos++] = punt.integer[1] & 0xFF;
}

#ifdef TESTIFY_DEBUG
real64 testify_unpack_real64_internal(THandle *handle, char *name, char *file, uint line)
#else					
real64 testify_unpack_real64_internal(THandle *handle)
#endif						
{
	union { uint32 integer[2]; real64 real; } punt;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_REAL64] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_REAL64, 1, name, file, line);
#endif
	/* write out the data */
	punt.integer[0] = ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 24;
	punt.integer[0] |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 16;
	punt.integer[0] |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	punt.integer[0] |= (uint32) handle->read_buffer[handle->read_buffer_pos++];
	punt.integer[1] = ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 24;
	punt.integer[1] |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 16;
	punt.integer[1] |= ((uint32) handle->read_buffer[handle->read_buffer_pos++]) << 8;
	punt.integer[1] |= (uint32) handle->read_buffer[handle->read_buffer_pos++];
	return punt.real;
}



#ifdef TESTIFY_DEBUG					
uint64 testify_pack_raw_internal(THandle *handle, uint8 *data, uint64 length, char *name, char *file, uint line)
#else
uint64 testify_pack_raw_internal(THandle *handle, uint8 *data, uint64 lengt)
#endif					
{
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_RAW, length, name, file, line, length, length, ""))
		return 0; /* destination is gone */
#else
	if(handle->write_buffer_pos + length > testify_buffer_size[handle->type] - TESTIFY_HEADER_SIZE_MAX)
		if(!testify_pack_buffer_clear(handle))
			return 0; /* destination is gone */
#endif
	if(handle->type == T_HT_STREAMING_SERVER || 
		handle->type == T_HT_STREAMING_CONNECTION)
	{
		size_t size;
		if(handle->write_buffer_pos)
		{
			size = testify_network_stream_send(handle, handle->write_buffer, handle->write_buffer_pos);
			if(size == handle->write_buffer_pos)
				handle->write_buffer_pos = 0;
			else if(size != 0)
			{
				uint i;
				for(i = 0; i < handle->write_buffer_pos - size; i++)
					handle->write_buffer[i] = handle->write_buffer[size + i];
				handle->write_buffer_pos -= size;
				return 0;
			}
		}
		size = testify_network_stream_send(handle, &data[handle->write_raw_progress], length - handle->write_raw_progress);
		handle->write_raw_progress += size;
		if(handle->write_raw_progress == length)
			handle->write_raw_progress = 0;
		return size;

	}else if(handle->type == T_HT_PACKET_PEER)
	{
		memcpy(&handle->write_buffer[handle->write_buffer_pos++], data, length);
		handle->write_buffer_pos += length;
		return length;
	}else if(handle->type == T_HT_FILE_WRITE)
	{
		testify_pack_buffer_clear(handle);
		fwrite(data, 1, length, handle->file);
		return length;
	}
	return 0;
}


#ifdef TESTIFY_DEBUG
real64 testify_unpack_raw_internal(THandle *handle, uint8 *buffer, uint64 buffer_length, char *name, char *file, uint line)
#else					
real64 testify_unpack_raw_internal(THandle *handle, uint8 *buffer, uint64 buffer_length)
#endif						
{		
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor && handle->read_raw_progress == 0)
	{
		testify_unpack_buffer_get(handle);
		if(handle->read_buffer_pos + TESTIFY_HEADER_SIZE_MAX < handle->read_buffer_used)
		{
			testify_unpack_debug_header(handle, T_TYPE_RAW, 1, name, file, line);
		}else
			return 0;			
	}
#endif
	if(handle->type == T_HT_STREAMING_SERVER || 
		handle->type == T_HT_STREAMING_CONNECTION)
	{
	
		int64 progress, size;
		progress = handle->read_buffer_used - handle->read_buffer_pos;
		if(progress >= buffer_length)
		{
			memcpy(buffer, &handle->read_buffer[handle->read_buffer_pos], buffer_length);
			handle->read_buffer_pos += buffer_length;
			return buffer_length;
		}
		memcpy(buffer, &handle->read_buffer[handle->read_buffer_pos], progress);
		handle->read_buffer_pos = handle->read_buffer_used;
		size = testify_network_stream_receve(handle, &buffer[progress], buffer_length - progress);

			
		progress += size;
		if(progress == buffer_length)
			handle->read_raw_progress = 0;
		else
			handle->read_raw_progress = progress;
		return progress;

	}else if(handle->type == T_HT_PACKET_PEER)
	{
		memcpy(buffer, &handle->read_buffer[handle->read_buffer_pos], buffer_length);
		handle->read_buffer_pos += buffer_length;
		return buffer_length;
	}else if(handle->type == T_HT_FILE_READ)
	{
		if(handle->read_buffer_used - handle->read_buffer_pos >= buffer_length)
		{
			memcpy(buffer, &handle->read_buffer[handle->read_buffer_pos], buffer_length);
			handle->read_buffer_pos += buffer_length;
			handle->read_raw_progress = 0;
			return buffer_length;
		}else
		{
			uint64 progress, size;
			progress = handle->read_buffer_used - handle->read_buffer_pos;
			memcpy(buffer, &handle->read_buffer[handle->read_buffer_pos], progress);
			handle->read_buffer_pos = handle->read_buffer_used;
			return fread(&buffer[progress], 1, buffer_length - progress, handle->file) + progress;
		}
	}
	return 0;
}


/* --------------------------------------------------- */

#ifdef TESTIFY_DEBUG					
void testify_pack_string_internal(THandle *handle, char *string, char *name, char *file, uint line)
#else
void testify_pack_string_internal(THandle *handle, char *string)
#endif					
{
	uint length = 0, pos = 0, size;
#ifdef TESTIFY_DEBUG
	if(!testify_pack_debug_header(handle, T_TYPE_STRING, 1, name, file, line, 0, 0, string))
		return; /* destination is gone */
#endif
	if(string == NULL)
	{
		if(handle->write_buffer_pos == handle->write_buffer_size)
			if(!testify_pack_buffer_clear(handle))
				return;
		if(handle->write_buffer_pos < handle->write_buffer_size)
			handle->write_buffer[handle->write_buffer_pos++] = 0;
	}else
	{
		for(length = 0; string[length] != 0; length++);
		length++;
		size = handle->write_buffer_size;
		while(pos < length)
		{
			for(; pos < length && handle->write_buffer_pos < handle->write_buffer_size; pos++)
				handle->write_buffer[handle->write_buffer_pos++] = string[pos];

			if(handle->write_buffer_pos == handle->write_buffer_size)
				if(!testify_pack_buffer_clear(handle))
					return; /* output no longer available */
		}
	}
}

#ifdef TESTIFY_DEBUG
boolean testify_unpack_string_internal(THandle *handle, char *string, uint buffer_size, char *name, char *file, uint line)
#else					
boolean testify_unpack_string_internal(THandle *handle, char *string, uint buffer_size)
#endif						
{
	boolean marker;
	uint i, size;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_STRING] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_STRING, 1, name, file, line);
#endif
	/* read the data */
	marker = handle->read_marker != -1;
	if(!marker)
		handle->read_marker = handle->read_buffer_pos;

	while(TRUE)
	{
		for(size = 0; handle->read_buffer_pos + size < handle->read_buffer_used && handle->read_buffer[handle->read_buffer_pos + size] != 0; size++);

		if(handle->read_buffer_pos + size < handle->read_buffer_used)
		{
			--buffer_size;
			for(i = 0; i < buffer_size && handle->read_buffer[i + handle->read_buffer_pos] != 0; i++)
				string[i] = handle->read_buffer[i + handle->read_buffer_pos];
			string[i] = 0;
			handle->read_buffer_pos += size + 1;
			if(!marker)
				handle->read_marker = -1;
			return TRUE;
		}
		if(!testify_unpack_buffer_get(handle))
		{
			if(!marker)
				handle->read_marker = -1;	
			return FALSE;
		}
	}
}


#ifdef TESTIFY_DEBUG
char *testify_unpack_string_allocate_internal(THandle *handle, char *name, char *file, uint line)
#else					
char *testify_unpack_string_allocate_internal(THandle *handle)
#endif						
{
	char *string;
	boolean marker;
	uint i, size;
	/* send or save data if buffer is filling up */
	if(handle->read_buffer_pos + testify_type_size[T_TYPE_STRING] + TESTIFY_HEADER_SIZE_MAX >= handle->read_buffer_used)
		testify_unpack_buffer_get(handle);
#ifdef TESTIFY_DEBUG	
	/* include the data description header */
	if(handle->debug_descriptor)
		testify_unpack_debug_header(handle, T_TYPE_STRING, 1, name, file, line);
#endif
	/* read the data */
	marker = handle->read_marker != -1;
	if(!marker)
		handle->read_marker = handle->read_buffer_pos;

	while(TRUE)
	{
		for(size = 0; handle->read_buffer_pos + size < handle->read_buffer_used && handle->read_buffer[handle->read_buffer_pos + size] != 0; size++);

		if(handle->read_buffer_pos + size < handle->read_buffer_used)
		{
			if(size == 0)
			{
				handle->read_buffer_pos++;
				return NULL;
			}
			string = malloc((sizeof* string) * ++size);
			for(i = 0; handle->read_buffer[i + handle->read_buffer_pos] != 0; i++)
				string[i] = handle->read_buffer[i + handle->read_buffer_pos];
			string[i] = 0;
			handle->read_buffer_pos += size;
			if(!marker)
				handle->read_marker = -1;
			return string;
		}
		if(!testify_unpack_buffer_get(handle))
		{
			if(!marker)
				handle->read_marker = -1;	
			return NULL;
		}
	}
}

void testify_restart_marker_set(THandle *handle)
{
	handle->read_marker = handle->read_buffer_pos;
}

void testify_restart_marker_release(THandle *handle)
{
	handle->read_marker = -1;
}

void testify_restart_marker_reset_internal(THandle *handle, char *file, uint line)
{
	if(handle->read_marker == -1)
	{
		printf("UNRAVEL testify_restart_marker_reset error on line %u in file %s. Marker not set.\n", line, file);
		return;
	}
	handle->read_buffer_pos = handle->read_marker;
	handle->read_marker = -1;
}

boolean testify_retivable_internal(THandle *handle, uint size)
{
	if(handle->type == T_HT_BUFFER)
		handle->read_buffer_used = handle->write_buffer_pos;
	
	if(handle->debug_descriptor)
	{
		char read[32];
		uint i, pos, colected = 0, read_type, length, add;
		pos = handle->read_buffer_pos;

		while(pos + 5 < handle->read_buffer_used && size > colected)
		{
			read_type = handle->read_buffer[pos++];
			length  = ((uint32) handle->read_buffer[pos++]) << 24;
			length |= ((uint32) handle->read_buffer[pos++]) << 16;
			length |= ((uint32) handle->read_buffer[pos++]) << 8;
			length |= handle->read_buffer[pos++];

			for(i = 0; pos < handle->read_buffer_used && handle->read_buffer[pos] != 0 && i < 31; i++)
				pos++; /* name */
			if(i == 31)
			{
				for(i = 0; i < handle->read_buffer_used && i < pos + 10000; i++)
				{
					char character[2] = {0, 0};
					character[0] = handle->read_buffer[i];
					if(i == pos)
						printf("Buffer [%u] = %s (%u) <------------------used %u pos %u\n", i, character, (uint)handle->read_buffer[i], handle->read_buffer_used, handle->read_buffer_pos);
					else
						printf("Buffer [%u] = %s (%u)\n", i, character, (uint)handle->read_buffer[i]);
				}
				exit(0);
			}
			pos++;
			if(pos == handle->read_buffer_used)
				return FALSE;
			if(length <= 1)
				add = testify_type_size[read_type];
			else
				add = testify_type_size[read_type] * length;
			pos += add;
			if(pos > handle->read_buffer_used)
				return FALSE;
			colected += add;
		}
	/*	if(size >= colected)
		{
			char character[2] = {0, 0};
			for(i = handle->read_buffer_pos; i < handle->read_buffer_used; i++)
			{
				character[0] = handle->read_buffer[i];
				printf("buffer[%u] %u %s\n", i, (uint)handle->read_buffer[i], character);
			}
		}*/
		return size <= colected;
	}
	return handle->read_buffer_pos + size <= handle->read_buffer_used;
}

boolean testify_retivable(THandle *handle, uint size)
{ 
	while(TRUE)
	{
		if(testify_retivable_internal(handle, size))
			return TRUE;
		if(!testify_unpack_buffer_get(handle))
			return FALSE;
	}
}


boolean testify_retivable_terminated_internal(THandle *handle)
{
	uint i;
	if(handle->debug_descriptor)
	{
		char read[32];
		uint pos, colected, read_type, length;
		pos = handle->read_buffer_pos;

		if(pos + 6 >= handle->read_buffer_used)
			return FALSE;

		read_type = handle->read_buffer[pos++];
		if(read_type != T_TYPE_STRING)
		{
			uint8 read[2048];
			char string[2048];
			uint *X = NULL, j;
			j = 0;
			if(pos > 50)
				j = pos - 50;
			for(i = 0; i < 2048 && i < handle->read_buffer_used; i++)
				read[i] = string[i] = handle->read_buffer[j++];
			printf("UNRAVEL Error: Trying to testify_retivable_terminated on non string\n");
			X[0] = 0;
			return FALSE;
		}
		length  = ((uint32) handle->read_buffer[pos++]) << 24;
		length |= ((uint32) handle->read_buffer[pos++]) << 16;
		length |= ((uint32) handle->read_buffer[pos++]) << 8;
		length |= handle->read_buffer[pos++];

		for(i = 0; pos < handle->read_buffer_used && handle->read_buffer[pos] != 0 && i < 31; i++)
			pos++;/* name */
		if(pos == handle->read_buffer_used)
			return FALSE;

		for(pos++ ; pos < handle->read_buffer_used && handle->read_buffer[pos] != 0; pos++);
		if(pos < handle->read_buffer_used)
			return TRUE;
		return FALSE;
	}
	for(i = handle->read_buffer_pos; i < handle->read_buffer_used && handle->read_buffer[i] != 0; i++);
	return handle->read_buffer[i] == 0;
}


boolean testify_retivable_terminated(THandle *handle)
{
	while(TRUE)
	{
		if(testify_retivable_terminated_internal(handle))
			return TRUE;
		if(!testify_unpack_buffer_get(handle))
			return FALSE;
	}
}