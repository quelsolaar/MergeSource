#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "imagine.h"
#include "betray.h"
#include "seduce.h"
#include "testify.h"


THandle *opa_watch_connection = NULL;
boolean opa_watch_thread = FALSE;
void *opa_watch_mutex = NULL;
boolean opa_watch_initialized = FALSE;


void opa_hand_shake(char *file_name)
{
	uint size, i;
	uint8 buffer[32], *b;
	struct {uint8 pad; signed char value;}*char_struct;
	struct {uint8 pad; unsigned char value;} *uchar_struct;
	struct {uint8 pad; signed short value;} *short_struct;
	struct {uint8 pad; unsigned short value;} *ushort_struct;
	struct {uint8 pad; signed int value;} *int_struct;
	struct {uint8 pad; unsigned int value;} *uint_struct;
	struct {uint8 pad; signed long long value;} *long_long_struct;
	struct {uint8 pad; unsigned long long value;} *ulong_long_struct;
	struct {uint8 pad; float value;} *float_struct;
	struct {uint8 pad; double value;} *double_struct;
	struct {uint8 pad; void *value;} *void_struct;
	struct {uint8 pad; void (*value)(void);} *func_struct;
 

	/* char */
	size = sizeof(signed char);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	char_struct = &buffer;
	b= (uint8 *)&char_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	size = sizeof(unsigned char);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	uchar_struct = &buffer;
	b= (uint8 *)&uchar_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");
		
	/* short */

	size = sizeof(signed short);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	short_struct = &buffer;
	b= (uint8 *)&short_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	size = sizeof(unsigned short);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	ushort_struct = &buffer;
	b= (uint8 *)&ushort_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	/* int */

	size = sizeof(signed int);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	int_struct = &buffer;
	b= (uint8 *)&int_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	size = sizeof(unsigned int);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	uint_struct = &buffer;
	b= (uint8 *)&uint_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	/* long long */

	size = sizeof(signed long long);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	long_long_struct = &buffer;
	b= (uint8 *)&long_long_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	size = sizeof(unsigned long long);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	ulong_long_struct = &buffer;
	b= (uint8 *)&ulong_long_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	/* float */

	size = sizeof(float);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	float_struct = &buffer;
	b = (uint8 *)&float_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");
		
	/* double */
	size = sizeof(double);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	double_struct = &buffer;
	b= (uint8 *)&double_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	/* void */

	size = sizeof(void*);
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	void_struct = &buffer;
	b= (uint8 *)&void_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");
		
	/* func */

	size = sizeof(void (*)(void));
	testify_pack_uint8(opa_watch_connection, size, "size");
	for(i = 0; i < 32; i++)
		buffer[i] = 0;
	func_struct = &buffer;
	b= (uint8 *)&func_struct->value;
	for(i = 0; i < size; i++)
		b[i] = ~0;		
	for(i = 0; i < 32 && buffer[i] == 0; i++);
	testify_pack_uint8(opa_watch_connection, i, "allignment");

	testify_pack_string(opa_watch_connection, file_name, "path");
	testify_network_stream_send_force(opa_watch_connection);
}



void opa_connect_func(void *data)
{
	char *file_name;
	THandle *handle = NULL;
	file_name = data;
	while(handle == NULL)
	{
		handle = testify_network_stream_address_create("localhost", 0xa113);
	}
	imagine_mutex_lock(opa_watch_mutex);
	opa_watch_connection = handle;
	testify_debug_mode_set(opa_watch_connection, TRUE, NULL);
	opa_hand_shake(file_name);
	free(file_name);
	imagine_mutex_unlock(opa_watch_mutex); 
}

void opa_watch(void *pointer, char *type_name, char *file_name)
{

	uint64 int_pointer, length;
	static uint8 command, debug_code = 155;
	uint indirections, i;
	uint64 pointer_list[255];
	uint64 offset_list[255];
	int_pointer = (uint64)pointer;

	if(!, "Moon AI simulate") 
	{
		opa_watch_mutex = imagine_mutex_create();
		imagine_thread(opa_connect_func, f_text_copy_allocate(file_name), "opa watch thread");
		opa_watch_thread = TRUE;
	}
	if(!imagine_mutex_lock_try(opa_watch_mutex))
		return;
	if(opa_watch_connection == NULL)
	{		
		imagine_mutex_unlock(opa_watch_mutex); 
		return;
	}
	if(!testify_network_stream_connected(opa_watch_connection))
	{		
		opa_watch_connection = NULL;
		imagine_mutex_unlock(opa_watch_mutex); 
		imagine_thread(opa_connect_func, f_text_copy_allocate(file_name), "opa connect thread");
		return;
	}
	
	testify_pack_uint8(opa_watch_connection, 0, "command");
	testify_pack_uint64(opa_watch_connection, (uint64)pointer, "pointer");
	testify_pack_string(opa_watch_connection, type_name, "type_name");
	testify_network_stream_send_force(opa_watch_connection);

	while(testify_retivable(opa_watch_connection, 1))
	{
		testify_restart_marker_set(opa_watch_connection);

		if(!testify_retivable(opa_watch_connection, 1 + 4 + 1 + 8))
		{
			testify_restart_marker_reset(opa_watch_connection); 
			imagine_mutex_unlock(opa_watch_mutex); 
			return;
		}
		command = testify_unpack_uint8(opa_watch_connection, "command");
		length = testify_unpack_uint32(opa_watch_connection, "length");
		indirections = testify_unpack_uint8(opa_watch_connection, "indirections");
		pointer_list[0] = testify_unpack_uint64(opa_watch_connection, "start");
		if(indirections != 0)
		{
			if(!testify_retivable(opa_watch_connection, indirections * (8 + 8)))
			{
				testify_restart_marker_reset(opa_watch_connection); 
				imagine_mutex_unlock(opa_watch_mutex); 
				return;
			}
			for(i = 0; i < indirections; i++)
			{
				pointer_list[i + 1] = testify_unpack_uint64(opa_watch_connection, "pointer");
				offset_list[i] = testify_unpack_uint64(opa_watch_connection, "offset");
			}
		}
	
		for(i = 0; i < indirections && int_pointer == pointer_list[i]; i++)
		{
			void **p;
			int_pointer += offset_list[i];
			p = (void**)int_pointer;
			int_pointer = (uint64)p[0];
		}


		if(command == 0) /* data requested*/
		{
			if(i == indirections && int_pointer == pointer_list[i])
			{
				uint line = 0;
				char *file = "";
				uint64 size;
				uint8 *data;
				data = (uint8 *)int_pointer;
				testify_pack_uint8(opa_watch_connection, 1, "command");
				testify_pack_uint64(opa_watch_connection, (uint64)int_pointer, "pointer");
#ifdef F_MEMORY_DEBUG
				f_debug_mem_query((void *)int_pointer, &line, &file, &length);
#endif
				testify_pack_uint64(opa_watch_connection, length, "length");
				testify_pack_uint32(opa_watch_connection, line, "line");
				testify_pack_string(opa_watch_connection, file, "file");
				for(i = 0; i < length; i++)
						testify_pack_uint8(opa_watch_connection, data[i], "data");
				testify_network_stream_send_force(opa_watch_connection);
			}
		}else if(command == 1) /* data poke */
		{
			uint32 offset;
			uint8 *data;
			if(!testify_retivable(opa_watch_connection, 4 + length))
			{
				testify_restart_marker_reset(opa_watch_connection); 
				imagine_mutex_unlock(opa_watch_mutex); 
				debug_code = 1;
				return;
			}
			debug_code = 2;
			data = (uint8 *)int_pointer;
			if(i == indirections && int_pointer == pointer_list[i])
			{
				offset = testify_unpack_uint32(opa_watch_connection, "offset");
				for(i = 0; i < length; i++)
						data[i + offset] = testify_unpack_uint8(opa_watch_connection, "data");
			}else
			{
				testify_unpack_uint32(opa_watch_connection, "offset");
				for(i = 0; i < length; i++)
					testify_unpack_uint8(opa_watch_connection, "data");
			}
		}else if(command == 2) /* allocation */
		{
			void **allocation;
			uint32 offset;
			uint8 *data;
			if(!testify_retivable(opa_watch_connection, 4))
			{
				testify_restart_marker_reset(opa_watch_connection); 
				imagine_mutex_unlock(opa_watch_mutex); 
				return;
			}
			if(i == indirections && int_pointer == pointer_list[i])
			{
				data = (uint8 *)int_pointer;
				offset = testify_unpack_uint32(opa_watch_connection, "offset");
				allocation = &data[offset];
				*allocation = data = malloc(length);
				for(i = 0; i < length; i++)
					data[i] = 0;
			}else
				testify_unpack_uint32(opa_watch_connection, "offset");
		}
		testify_restart_marker_release(opa_watch_connection); 
	}
	imagine_mutex_unlock(opa_watch_mutex); 
}