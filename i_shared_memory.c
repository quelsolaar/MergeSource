#include "stdlib.h"
#include "forge.h"
//#include "imagine.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
/*
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
*/




typedef enum{
	IMAGINE_MSS_INITIALIZING,
	IMAGINE_MSS_CONNECTED,
	IMAGINE_MSS_ACTIVE,
	IMAGINE_MSS_DISCONNECTED,
	IMAGINE_MSS_LOST,
	IMAGINE_MSS_COUNT
}IMemShareState;

typedef enum{
	IMAGINE_MSCS_HOST_READY, /* ready to send a command */
	IMAGINE_MSCS_CLIENT_CONNECTED, /* ready to send a command */
	IMAGINE_MSCS_IDLE, /* ready to send a command */
	IMAGINE_MSCS_PROCESSING, /* A command is sent and beeing proccessed buy the host*/
	IMAGINE_MSCS_RETURNING, /* A comand is done and being consumed by the clinet*/
	IMAGINE_MSCS_FAIL_RECOVERABLE,
	IMAGINE_MSCS_FAIL_TERMINAL,
}IMemShareCommandState;

typedef struct{
	IMemShareCommandState command_state;
	uint command;
	size_t param_size;
}ImagineMeomoryShareSync;


typedef struct{
	ImagineMeomoryShareSync sync;
	size_t size;
	uint64 magic_number;
	uint64 proccess_id;
	HANDLE sync_event;
}ImagineMeomoryShareSyncInit;

typedef struct{
	HANDLE this_process;
	HANDLE other_process;
	HANDLE sync_buffer_handle;
	volatile ImagineMeomoryShareSync *sync_mapping;
	HANDLE data_buffer_handle;
	volatile uint8 *data_mapping;
	size_t size;
	DWORD *process_id_array;
	uint process_id_array_size;
	IMemShareState state;
	uint8 local_sync_buffer[4096];
	HANDLE sync_event;
}IMemShare;

uint64 imagine_memory_share_compute_magic_number(char *id)
{
	union{uint8 bytes[8]; uint64 magic_number;}compund;
	uint i;
	compund.magic_number = 2021060619761227;
	for(i = 0; id[i] != 0; i++)
		compund.bytes[i % 8] ^= id[i];
	return compund.magic_number;
}

/*
"Civilization advances by extending the number of important operations which we can perform without thinking about them." 

~Alfred North Whitehead, An Introduction to Mathematics (1911)
UnmapViewOfFile(buffer);
CloseHandle(handle);

*/
/*
BOOL DuplicateHandle(
	HANDLE   hSourceProcessHandle,
	HANDLE   hSourceHandle,
	HANDLE   hTargetProcessHandle,
	LPHANDLE lpTargetHandle,
	0,
	FALSE,
	DUPLICATE_SAME_ACCESS);
	*/


void imagine_memory_share_destroy(IMemShare *share)
{
	if(share->data_buffer_handle != INVALID_HANDLE_VALUE)
		CloseHandle(share->data_buffer_handle);
	if(share->sync_buffer_handle != INVALID_HANDLE_VALUE)
		CloseHandle(share->sync_buffer_handle);
	if(share->sync_mapping != NULL)
		UnmapViewOfFile(share->sync_mapping);
	if(share->data_mapping != NULL)
		UnmapViewOfFile(share->data_mapping);
	if(share->process_id_array == NULL)
		free(share->process_id_array);
	free(share);
}

boolean imagine_memory_share_alive(IMemShare *share)
{
	DWORD used, *array;
	uint i, count, process; 
	array = share->process_id_array;
	if(!EnumProcesses(share->process_id_array, (sizeof *share->process_id_array) * share->process_id_array_size, &used))
		return FALSE;
	count = used / (sizeof *share->process_id_array);
	process = share->other_process;
	for(i = 0; i < count; i++)
		if(process == array[i])
			return TRUE;
	if(count == share->process_id_array_size)
	{
		share->process_id_array_size *= 2;
		free(array);
		share->process_id_array = malloc((sizeof *share->process_id_array) * share->process_id_array_size);
		return TRUE;
	}else
		return FALSE;
}



IMemShare *imagine_memory_share_host_create(size_t size, char *id)
{
	ImagineMeomoryShareSyncInit *sync;
	IMemShare *share;
	char buffer[1024];
	uint i;
	share = malloc(sizeof *share);
	share->this_process = GetCurrentProcessId();
	share->other_process = 0;
	share->sync_buffer_handle = INVALID_HANDLE_VALUE;
	share->sync_mapping = NULL;
	share->data_buffer_handle = INVALID_HANDLE_VALUE;
	share->data_mapping = NULL;
	share->process_id_array_size = 1024;
	share->size = size;
	if(NULL == (share->process_id_array = malloc((sizeof *share->process_id_array) * share->process_id_array_size)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}	
	for(i = 0; id[i] != 0 && i < 1024 - 2; i++)
		buffer[i + 1] = id[i];
	buffer[i + 1] = 0;
	buffer[0] = 's';
	if(INVALID_HANDLE_VALUE == (share->sync_buffer_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, 4096, buffer)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}
	if(INVALID_HANDLE_VALUE == (share->sync_mapping = MapViewOfFile(share->sync_buffer_handle, FILE_MAP_ALL_ACCESS, 0, 0, 4096)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}

	buffer[0] = 'd';
	if(INVALID_HANDLE_VALUE == (share->data_buffer_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, size, buffer)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}

	if(INVALID_HANDLE_VALUE == (share->data_mapping = MapViewOfFile(share->data_buffer_handle, FILE_MAP_ALL_ACCESS, 0, 0, size)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}
	sync = (ImagineMeomoryShareSyncInit *)share->sync_mapping;
	sync->sync.command_state = IMAGINE_MSCS_HOST_READY;
	sync->sync.command = -1;
	sync->size = size;
	sync->proccess_id = share->this_process;
	sync->magic_number = imagine_memory_share_compute_magic_number(id);
	buffer[0] = 'e';
	share->sync_event = CreateEvent(NULL, TRUE, FALSE, buffer);
	share->state = IMAGINE_MSS_INITIALIZING;
	return share;
}



boolean imagine_memory_share_host_wait_for_connect(IMemShare *share)
{	
	if(share->sync_mapping->command_state == IMAGINE_MSCS_CLIENT_CONNECTED)
	{
		share->other_process = ((ImagineMeomoryShareSyncInit *)share->sync_mapping)->proccess_id;
		share->sync_mapping->command_state = IMAGINE_MSCS_IDLE;
		return TRUE;
	}
	return FALSE;
}


IMemShare *imagine_memory_share_client_create(char *id)
{
	ImagineMeomoryShareSyncInit *sync;
	IMemShare *share;
	char buffer[1024];
	uint i;
	share = malloc(sizeof *share);
	share->this_process = GetCurrentProcessId();
	share->other_process = 0;
	share->sync_buffer_handle = INVALID_HANDLE_VALUE;
	share->sync_mapping = NULL;
	share->data_buffer_handle = INVALID_HANDLE_VALUE;
	share->data_mapping = NULL;
	share->process_id_array_size = 1024;
	if(NULL == (share->process_id_array = malloc((sizeof *share->process_id_array) * share->process_id_array_size)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}	
	for(i = 0; id[i] != 0 && i < 1024 - 2; i++)
		buffer[i + 1] = id[i];
	buffer[i + 1] = 0;
	buffer[0] = 's';
	if(INVALID_HANDLE_VALUE == (share->sync_buffer_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, 4096, buffer)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}
	if(NULL == (share->sync_mapping = MapViewOfFile(share->sync_buffer_handle, FILE_MAP_ALL_ACCESS, 0, 0, 4096)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}
	sync = share->sync_mapping;
	if(sync->magic_number != imagine_memory_share_compute_magic_number(id))
	{
		imagine_memory_share_destroy(share);
		return NULL;	
	}
	share->other_process = sync->proccess_id;
	sync->proccess_id = share->this_process;
	share->size = sync->size;
	buffer[0] = 'd';
	if(INVALID_HANDLE_VALUE == (share->data_buffer_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, share->size, buffer)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}

	if(NULL == (share->data_mapping = MapViewOfFile(share->data_buffer_handle, FILE_MAP_ALL_ACCESS, 0, 0, share->size)))
	{
		imagine_memory_share_destroy(share);
		return NULL;
	}
	buffer[0] = 'e';
	share->sync_event = CreateEvent(NULL, TRUE, FALSE, buffer);
	share->state = IMAGINE_MSS_CONNECTED;
	sync->sync.command_state = IMAGINE_MSCS_CLIENT_CONNECTED;
	return share;
}


__inline boolean imagine_memory_share_sync(IMemShare *share, uint states)
{
	volatile int *command_state;
	uint i;
	command_state = &share->sync_mapping->command_state;
	for(i = 0; i < ~0 && (*command_state != states); i++); /* wait for return */
	if(i == ~0)
	{
		while(TRUE) // at most a tenth of a ssecond.
		{
			WaitForSingleObject(share->sync_event, 100000);
 			if(*command_state & states)
			{
				return TRUE;
			}
			if(!imagine_memory_share_alive(share))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

/*
* 
WaitForSingleObject(ghWriteEvent, INFINITE);  
IMAGINE_MSCS_IDLE,
IMAGINE_MSCS_PROCESSING,
IMAGINE_MSCS_FAIL_RECOVERABLE,
IMAGINE_MSCS_FAIL_TERMINAL,
IMAGINE_MSCS_COUNT
*/


boolean imagine_memory_share_client_command(IMemShare *share, uint command, void *parameters, size_t param_size, void *return_buffer, size_t return_size)
{
	volatile ImagineMeomoryShareSync *sync;
	uint i; 
	sync = share->sync_mapping;
	if(!imagine_memory_share_sync(share, IMAGINE_MSCS_IDLE))
		return FALSE;
	sync->command = command;
	sync->param_size = param_size;
	memcpy(&sync[1], parameters, param_size);
	sync->command_state = IMAGINE_MSCS_PROCESSING;
	SetEvent(share->sync_event);
	if(return_size == 0)
		return TRUE;
	if(imagine_memory_share_sync(share, IMAGINE_MSCS_RETURNING))
		return NULL;
	memcpy(return_buffer, &sync[1], return_size);
	sync->command_state = IMAGINE_MSCS_IDLE;
//	SetEvent(share->sync_event);
	return;
}

void *imagine_memory_share_buffer_get(IMemShare *share, size_t *size)
{
	if(size != NULL)
		*size = share->size;
	return share->data_mapping;
}

void imagine_memory_share_host_command(IMemShare *share, char *(*command_func)(IMemShare *share, uint command, void *parameter_buffer, void *return_buffer, void *user), void *user)
{
	volatile ImagineMeomoryShareSync *sync;
	uint i; 
	sync = share->sync_mapping;
	if(!imagine_memory_share_sync(share, IMAGINE_MSCS_PROCESSING))
		return;
	memcpy(share->local_sync_buffer, &sync[1], sync->param_size);
	command_func(share, sync->command, share->local_sync_buffer, &sync[1], user);
	sync->command_state = IMAGINE_MSCS_IDLE;
	SetEvent(share->sync_event);
}


/*

char *imagine_memory_share_host_command_func(IMemShare *share, uint command, void *parameter_buffer, void *return_buffer, void *user)
{
}

extern void		imagine_current_time_get(uint32 *seconds, uint32 *fractions); 
extern double	imagine_delta_time_compute(uint new_seconds, uint new_fractions, uint old_seconds, uint old_fractions); 

void main()
{
	char *id = "shared file";
	IMemShare *mem_share;
	mem_share = imagine_memory_share_client_create(id);
	if(mem_share == NULL)
	{
		printf("Host!\n");
		mem_share = imagine_memory_share_host_create(4096, id);
		while(!imagine_memory_share_host_wait_for_connect(mem_share));
		printf("Connected!\n");
		while(TRUE || imagine_memory_share_alive(mem_share))
		{
			imagine_memory_share_host_command(mem_share, imagine_memory_share_host_command_func, NULL);
		}
	}else
	{
		uint i, new_seconds, new_fractions, old_seconds, old_fractions;
		double f;
		printf("CLient!\n");
		while(TRUE)
		{
			imagine_current_time_get(&old_seconds, &old_fractions);
			for(i = 0; i < 10000000; i++)
			{
				imagine_memory_share_client_command(mem_share, 0, NULL, 0, NULL, 0);
			}
			imagine_current_time_get(&new_seconds, &new_fractions);
			f = imagine_delta_time_compute(new_seconds, new_fractions, old_seconds, old_fractions);
			printf("time %f\n", f);
		}
	}
	printf("Exiting!\n");
}

*/

/*
	// separate external and internal store

	login/out
	User create/delete.

	Create.
		non locking
			launcing checksuming, networking, signing, clean up and indexing threads
	Update
		Re sunc record.

	search
		Slow. wait for responce.	
	lookup
		client side.

	Alloc
		On request
		Predictive
	Cashe
		on request
		Predict from lookup/Search.
	UnCase
		FIFO
*/

typedef struct{
	HANDLE file_handle;
	HANDLE mapping_handle;
	uint8 *mapping;
	size_t size;
}ImagineFilemapping;

typedef enum{
	IMAGINE_FMCT_OPEN_READ,
	IMAGINE_FMCT_OPEN_READ_WRITE,
	IMAGINE_FMCT_CREATE_READ_WRITE,
	IMAGINE_FMCT_COUNT
}ImagineFileMappingCreateMode;


boolean imagine_file_mapping_set(ImagineFilemapping *file, ImagineFileMappingCreateMode mode)
{
	DWORD mapping_read_write;
	DWORD view_read_write;
	if(IMAGINE_FMCT_OPEN_READ)
	{
		mapping_read_write = PAGE_READONLY | SEC_COMMIT;
		view_read_write = FILE_MAP_READ;
	}else
	{
		mapping_read_write = PAGE_READONLY | SEC_COMMIT;
		view_read_write = FILE_MAP_ALL_ACCESS;
	}
	file->mapping_handle = CreateFileMapping(file->file_handle,          // current file handle
											 NULL,           // default security
											 mapping_read_write, // read/write permission
											 file->size / 0x100000000,        // size of mapping object, high
											 file->size & 0xFFFFFFFF,  // size of mapping object, low
											 NULL);          // name of mapping object

	if(file->mapping_handle == NULL)
	{
		DWORD error;
		error = GetLastError();
		CloseHandle(file->file_handle);
		return FALSE;	
	}
	file->mapping = MapViewOfFile(file->mapping_handle, view_read_write, 0, 0, file->size);
	if(NULL == file->mapping)
	{
		CloseHandle(file->file_handle);
		CloseHandle(file->mapping_handle);
		return FALSE;	
	}
	return TRUE;
}

boolean imagine_file_mapping_create(ImagineFilemapping *file, char *file_name, size_t size, ImagineFileMappingCreateMode mode)
{ 
	uint64 win_size;
	switch(mode)
	{
		case IMAGINE_FMCT_OPEN_READ :
		file->file_handle = CreateFileA(file_name, GENERIC_READ, 0/* exclusive access*/, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		break;
		case IMAGINE_FMCT_OPEN_READ_WRITE :
		file->file_handle = CreateFileA(file_name, GENERIC_WRITE | GENERIC_READ, 0/* exclusive access*/, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		break;
		case IMAGINE_FMCT_CREATE_READ_WRITE :
		file->file_handle = CreateFileA(file_name, GENERIC_WRITE | GENERIC_READ, 0/* exclusive access*/, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		break;
	}	
	if(file->file_handle == INVALID_HANDLE_VALUE)
		return FALSE;
	if(mode != IMAGINE_FMCT_CREATE_READ_WRITE)
	{
		if(!GetFileSizeEx(file->file_handle, &win_size))
		{
			CloseHandle(file->file_handle);
			return FALSE;	
		}
		size = win_size;
	}
	file->size = size;
	return imagine_file_mapping_set(file, mode);
}

boolean imagine_file_mapping_resize(ImagineFilemapping *file, size_t size)
{
	FlushViewOfFile(file->mapping, file->size);
	UnmapViewOfFile(file->mapping);
	CloseHandle(file->mapping_handle);
	file->size = size;
	return imagine_file_mapping_set(file, IMAGINE_FMCT_OPEN_READ_WRITE);
}

void imagine_file_mapping_destroy(ImagineFilemapping *file)
{
	FlushViewOfFile(file->mapping, file->size);
	UnmapViewOfFile(file->mapping);
	CloseHandle(file->mapping_handle);
	CloseHandle(file->file_handle);
}

/*
void main()
{
	ImagineFilemapping mapping;
	char *text = "some kind of message ";
	uint i;
	if(imagine_file_mapping_create(&mapping, "./new_file.txt", 1024 * 1024, FALSE))
	{
		for(i = 0; i < 1024 * 1024; i++)
			mapping.mapping[i] = text[i % 21];
		printf("file written!");
	}else
		printf("file failed to open\n");
}*/

#endif




void imagine_memory_share_destroy(IMemShare *share);
boolean imagine_memory_share_alive(IMemShare *share);
IMemShare *imagine_memory_share_host_create(size_t size, char *id);
boolean imagine_memory_share_host_wait_for_connect(IMemShare *share);
IMemShare *imagine_memory_share_client_create(char *id);
__inline boolean imagine_memory_share_sync(IMemShare *share, uint states);
boolean imagine_memory_share_client_command(IMemShare *share, uint command, void *parameters, size_t param_size, void *return_buffer, size_t return_size);
void *imagine_memory_share_buffer_get(IMemShare *share, size_t *size);
void imagine_memory_share_host_command(IMemShare *share, char *(*command_func)(IMemShare *share, uint command, void *parameter_buffer, void *return_buffer, void *user), void *user);

/*
	// separate external and internal store

	login/out
	User create/delete.

	Create.
		non locking
			launcing checksuming, networking, signing, clean up and indexing threads
	Update
		Re sunc record.

	search
		Slow. wait for responce.	
	lookup
		client side.

	Alloc
		On request
		Predictive
	Cashe
		on request
		Predict from lookup/Search.
	UnCase
		FIFO
*/
