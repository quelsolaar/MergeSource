#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define F_MEMORY_INTERNAL
#define F_NO_MEMORY_DEBUG
#include "forge_memory_debugger.h"

extern void f_debug_mem_print(unsigned int min_allocs);

#define FALSE 0
#define TRUE !FALSE

#define F_MEMORY_MAGIC_NUMBER 0xCF
#define F_MEMORY_INITIALIZATION 0xCD
#define F_MEMORY_FREED 0xCE

FILE *forge_memory_log_file = NULL;
boolean forge_memory_active = TRUE;
unsigned char *forge_memory_stack_pointer = NULL;
size_t forge_memory_stack_size = 0;

typedef struct{
	size_t size;
	void *buf;
	char *comment;
	boolean active;
}ForgeMemAllocBuf;

typedef struct{
	unsigned int line;
	char file[256];
	ForgeMemAllocBuf *allocs;
	unsigned int alloc_count;
	size_t alloc_alocated;
	size_t size;
	size_t alocated;
	size_t freed;
}ForgeMemAllocLine;

ForgeMemAllocLine *f_alloc_lines;
unsigned int f_alloc_line_count = 0;

#define FORGE_FREE_POINTER_BUFFER_SIZE 1024

typedef struct{
	unsigned int alloc_line;
	char alloc_file[256];
	unsigned int free_line;
	char free_file[256];
	size_t size;
	void *pointer; 
	boolean realloc;
	boolean active;
}ForgeMemFreeBuf;

ForgeMemFreeBuf *f_freed_memory;
unsigned int f_freed_memory_count = 0;
unsigned int f_freed_memory_store = 1024;

void *f_alloc_mutex = NULL;
int (*f_alloc_mutex_lock)(void *mutex) = NULL;
int (*f_alloc_mutex_unlock)(void *mutex) = NULL;

void f_debug_mem_thread_safe_init(int (*lock)(void *mutex), int (*unlock)(void *mutex), void *mutex)
{
	f_alloc_mutex = mutex;
	f_alloc_mutex_lock = lock;
	f_alloc_mutex_unlock = unlock;
}


void f_debug_mem_stack_pointer_set(void *lowest_stack_pinter, size_t stack_size_in_bytes)
{
	forge_memory_stack_pointer = lowest_stack_pinter;
	forge_memory_stack_size = stack_size_in_bytes;
}

void f_debug_mem_active(boolean active)
{
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	forge_memory_active = active;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
}

void f_debug_mem_log(void *file_pointer)
{
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	forge_memory_log_file = file_pointer;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
}

void f_debug_mem_reset(void)
{
	unsigned int i;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		f_alloc_lines[i].alocated = 0;
		f_alloc_lines[i].size = 0;
	}
}

boolean f_debug_mem_check_bounds()
{
	boolean output = FALSE;
	size_t size;
	unsigned char *buf;
	size_t i, j, k;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].active)
			{
				buf = f_alloc_lines[i].allocs[j].buf;
				size = f_alloc_lines[i].allocs[j].size;
				for(k = 0; k < FORGE_MEMORY_OVER_ALLOC - FORGE_MEMORY_PRE_PADDIG; k++)
					if(buf[size + k] != F_MEMORY_MAGIC_NUMBER)
						break;
				if(k < FORGE_MEMORY_OVER_ALLOC - FORGE_MEMORY_PRE_PADDIG)
				{
					if(f_alloc_lines[i].allocs[j].comment == NULL)
						printf("FORGE Mem debugger error: Memory Overrun of allocation made on line %u in file %s\n", f_alloc_lines[i].line, f_alloc_lines[i].file);
					else
						printf("FORGE Mem debugger error: Memory Overrun of allocation made on  line %u in file %s /* %s */\n", f_alloc_lines[i].line, f_alloc_lines[i].file, f_alloc_lines[i].allocs[j].comment);
					FORGE_CALL_ON_ERROR
					output = TRUE;
				}
				buf = ((unsigned char*)f_alloc_lines[i].allocs[j].buf) - FORGE_MEMORY_PRE_PADDIG;
				size = f_alloc_lines[i].allocs[j].size;
				for(k = 0; k < FORGE_MEMORY_PRE_PADDIG; k++)
					if(buf[k] != F_MEMORY_MAGIC_NUMBER)
						break;
				if(k < FORGE_MEMORY_PRE_PADDIG)
				{
					if(f_alloc_lines[i].allocs[j].comment == NULL)
						printf("FORGE Mem debugger error: Memory underrun of allocation made on line %u in file %s\n", f_alloc_lines[i].line, f_alloc_lines[i].file);
					else
						printf("FORGE Mem debugger error: Memory underrun of allocation made on line %u in file %s /* %s */\n", f_alloc_lines[i].line, f_alloc_lines[i].file, f_alloc_lines[i].allocs[j].comment);
					FORGE_CALL_ON_ERROR
					output = TRUE;
				}
			}
		}
	}
#ifdef FORGE_USE_AFTER_FREE_CHECK
	for(i = 0; i < f_freed_memory_count && i < f_freed_memory_store; i++)
	{
		buf = f_freed_memory[i].pointer;
		size = f_freed_memory[i].size;
		for(k = 0; k < size && buf[k] == F_MEMORY_FREED; k++);
		if(k < size)
		{
			if(f_freed_memory[i].realloc)
				printf("FORGE Mem debugger error: Pointer that was reallocated on line %u in file %s, and freed on line %u in file %s was written to %u bytes in to the allocation after being freed.\n", 
					f_freed_memory[i].alloc_line, f_freed_memory[i].alloc_file,
					f_freed_memory[i].free_line, f_freed_memory[i].free_file, (unsigned int)k);
			else
				printf("FORGE Mem debugger error: Pointer that was allocated on line %u in file %s, and freed on line %u in file %s was written to %u bytes in to the allocation after being freed.\n", 
					f_freed_memory[i].alloc_line, f_freed_memory[i].alloc_file,
					f_freed_memory[i].free_line, f_freed_memory[i].free_file, (unsigned int)k);
			FORGE_CALL_ON_ERROR
		}
	}
#endif
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return output;
}

void f_debug_mem_add(void *pointer, size_t size, char *file, unsigned int line)
{
	unsigned int i, j;
	unsigned char *pre;
	pre = ((unsigned char *)pointer) - FORGE_MEMORY_PRE_PADDIG;
	for(i = 0; i < FORGE_MEMORY_PRE_PADDIG; i++)
		((unsigned char *)pre)[i] = F_MEMORY_MAGIC_NUMBER;

	for(i = 0; i < FORGE_MEMORY_OVER_ALLOC - FORGE_MEMORY_PRE_PADDIG; i++)
		((unsigned char *)pointer)[size + i] = F_MEMORY_MAGIC_NUMBER;

	for(i = 0; i < f_alloc_line_count; i++)
	{
		if(line == f_alloc_lines[i].line)
		{
			for(j = 0; file[j] != 0 && file[j] == f_alloc_lines[i].file[j] ; j++);
			if(file[j] == f_alloc_lines[i].file[j])
				break;
		}
	}
	if(i < f_alloc_line_count)
	{
		if(f_alloc_lines[i].alloc_alocated == f_alloc_lines[i].alloc_count)
		{
			f_alloc_lines[i].alloc_alocated += 1024;
			f_alloc_lines[i].allocs = realloc(f_alloc_lines[i].allocs, (sizeof *f_alloc_lines[i].allocs) * f_alloc_lines[i].alloc_alocated);
			if(f_alloc_lines[i].allocs == NULL)
			{			
				printf("FORGE Mem debugger error: Realloc returns NULL when trying to allocate %u bytes at line %u in file %s\n", (unsigned int)size, line, file);
				FORGE_CALL_ON_ERROR
				return;
			}
		}
		f_alloc_lines[i].allocs[f_alloc_lines[i].alloc_count].size = size;
		f_alloc_lines[i].allocs[f_alloc_lines[i].alloc_count].comment = NULL;
		f_alloc_lines[i].allocs[f_alloc_lines[i].alloc_count].active = forge_memory_active;
		f_alloc_lines[i].allocs[f_alloc_lines[i].alloc_count++].buf = pointer;
		if(forge_memory_active)
		{
			f_alloc_lines[i].size += size;
			f_alloc_lines[i].alocated++;
		}
	}else
	{
		if(i % 1024 == 0)
			f_alloc_lines = realloc(f_alloc_lines, (sizeof *f_alloc_lines) * (i + 1024));
		f_alloc_lines[i].line = line;
		for(j = 0; j < 255 && file[j] != 0; j++)
			f_alloc_lines[i].file[j] = file[j];
		f_alloc_lines[i].file[j] = 0;
		f_alloc_lines[i].alloc_alocated = 256;
		f_alloc_lines[i].allocs = malloc((sizeof *f_alloc_lines[i].allocs) * f_alloc_lines[i].alloc_alocated);
		f_alloc_lines[i].allocs[0].size = size;
		f_alloc_lines[i].allocs[0].buf = pointer;
		f_alloc_lines[i].allocs[0].comment = NULL;
		f_alloc_lines[i].alloc_count = 1;
		f_alloc_lines[i].freed = 0;
		if(forge_memory_active)
		{
			f_alloc_lines[i].alocated = 1;
			f_alloc_lines[i].size = size;
		}else
		{
			f_alloc_lines[i].alocated = 0;
			f_alloc_lines[i].size = 0;
		}
		f_alloc_line_count++;
	}
}

void *f_debug_mem_malloc(size_t size, char *file, unsigned int line)
{
	unsigned char *pointer;
#ifdef FORGE_MEMORY_CHECK_ALWAYS
	f_debug_mem_check_bounds();
#endif
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	if(size == 0)
	{
		printf("FORGE Mem debugger warning: malloc size ZERO in file %s line %u\n", file, line);
		FORGE_CALL_ON_ERROR
		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		return NULL;
	}
	pointer = malloc(size + FORGE_MEMORY_OVER_ALLOC);

	if(forge_memory_log_file != NULL && forge_memory_active)
		fprintf(forge_memory_log_file, "malloc %u bytes at pointer %p at %s line %u\n", (unsigned int)size, pointer, file, line);

	if(pointer == NULL)
	{
#ifdef FORGE_MEMORY_NULL_ALLOCATION_ERROR
		if(size > (size_t)(~((unsigned int)0)))
			printf("FORGE Mem debugger warning: malloc returns NULL at line %u in file %s\n", line, file);
		else
			printf("FORGE Mem debugger warning: malloc returns NULL when trying to allocate %u bytes at line %u in file %s\n", (unsigned int)size, line, file);
		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		FORGE_CALL_ON_ERROR
#endif
		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		return NULL;
	}
	pointer += FORGE_MEMORY_PRE_PADDIG;
	memset(pointer, F_MEMORY_INITIALIZATION, size);
	f_debug_mem_add(pointer, size, file, line);
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return pointer;
}

void *f_debug_mem_calloc(size_t num, size_t size, char *file, unsigned int line)
{
	unsigned char *pointer;
#ifdef FORGE_MEMORY_CHECK_ALWAYS
	f_debug_mem_check_bounds();
#endif
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	size *= num;
	if(size == 0)
	{
		printf("FORGE Mem debugger warning: calloc size ZERO in file %s line %u\n", file, line);
		FORGE_CALL_ON_ERROR

		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		return NULL;
	}


	pointer = malloc(size + FORGE_MEMORY_OVER_ALLOC);

	if(forge_memory_log_file != NULL && forge_memory_active)
		fprintf(forge_memory_log_file, "Calloc %u bytes at pointer %p at %s line %u\n", (unsigned int)size, pointer, file, line);

	if(pointer == NULL)
	{
#ifdef FORGE_MEMORY_NULL_ALLOCATION_ERROR
		if(size > (size_t)(~((unsigned int)0)))
			printf("FORGE Mem debugger varning: Calloc returns NULL at line %u in file %s\n", line, file);
		else
			printf("FORGE Mem debugger varning: Calloc returns NULL when trying to allocate %u bytes at line %u in file %s\n", (unsigned int)size, line, file);
		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		FORGE_CALL_ON_ERROR
#endif

		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		return NULL;
	}
	pointer += FORGE_MEMORY_PRE_PADDIG;
	memset(pointer, 0, size);
	f_debug_mem_add(pointer, size, file, line);
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return pointer;
}

boolean f_debug_mem_remove(unsigned char *buf, char *file, unsigned int line, boolean realloced, size_t *size)
{
	ForgeMemFreeBuf  *f;
	unsigned int i, j, k;
	size_t distance, s;

#if defined(FORGE_DOUBLE_FREE_CHECK) || defined(FORGE_USE_AFTER_FREE_CHECK) 
	if(f_freed_memory_count % 1024 == 0 && f_freed_memory_count < f_freed_memory_store)
	{
		if(f_freed_memory_count == 0)
			f_freed_memory = malloc((sizeof *f_freed_memory) * 1024);
		else
			f_freed_memory = realloc(f_freed_memory, (sizeof *f_freed_memory) * (f_freed_memory_count + 1024));
	}
#ifdef FORGE_USE_AFTER_FREE_CHECK
	if(f_freed_memory_count >= f_freed_memory_store)
	{
		if(f_freed_memory[f_freed_memory_count % f_freed_memory_store].pointer != NULL)
		{
			free(((unsigned char *)f_freed_memory[f_freed_memory_count % f_freed_memory_store].pointer) - FORGE_MEMORY_PRE_PADDIG);	
			f_freed_memory[f_freed_memory_count % f_freed_memory_store].pointer = NULL;
		}
	}
#endif
	f = &f_freed_memory[f_freed_memory_count++ % f_freed_memory_store];
	for(i = 0; i < 255 && file[i] != 0; i++)
		f->free_file[i] = file[i];
	f->free_file[i] = 0;
	f->free_line = line;
	f->realloc = realloced;
	f->size = 0;
	f->pointer = buf;
	f->active = forge_memory_active;

#endif
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].buf == buf)
			{
				buf -= FORGE_MEMORY_PRE_PADDIG;
				for(k = 0; k < FORGE_MEMORY_PRE_PADDIG; k++)
					if(((unsigned char *)buf)[k] != F_MEMORY_MAGIC_NUMBER)
						break;
				if(k < FORGE_MEMORY_PRE_PADDIG)
				{
					unsigned int *a = NULL;
					printf("FORGE Mem debugger error: Buffer underrun of allocation made on line %u in file %s\n", f_alloc_lines[i].line, f_alloc_lines[i].file);
					FORGE_CALL_ON_ERROR;
				}
				for(k = 0; k < FORGE_MEMORY_OVER_ALLOC - FORGE_MEMORY_PRE_PADDIG; k++)
					if(((unsigned char *)buf)[f_alloc_lines[i].allocs[j].size + FORGE_MEMORY_PRE_PADDIG + k] != F_MEMORY_MAGIC_NUMBER)
						break;
				if(k < FORGE_MEMORY_OVER_ALLOC - FORGE_MEMORY_PRE_PADDIG)
				{
					unsigned int *a = NULL;
					printf("FORGE Mem debugger error: Buffer overrun of allocation made on line  %u in file %s\n", f_alloc_lines[i].line, f_alloc_lines[i].file);
					FORGE_CALL_ON_ERROR;
				}
				memset(buf, F_MEMORY_FREED, f_alloc_lines[i].allocs[j].size + FORGE_MEMORY_OVER_ALLOC);
				f->alloc_line = f_alloc_lines[i].line;
				for(k = 0; k < 255 && f_alloc_lines[i].file[k] != 0; k++)
					f->alloc_file[k] = f_alloc_lines[i].file[k];
				f->alloc_file[k] = 0;
				f->size = f_alloc_lines[i].allocs[j].size;
				*size = f_alloc_lines[i].allocs[j].size;
				f_alloc_lines[i].size -= f_alloc_lines[i].allocs[j].size;
				f_alloc_lines[i].allocs[j] = f_alloc_lines[i].allocs[--f_alloc_lines[i].alloc_count];
				if(forge_memory_active)
					f_alloc_lines[i].freed++;

#ifndef FORGE_USE_AFTER_FREE_CHECK
				free(buf);
#endif
				return TRUE;
			}
			if((unsigned char *)f_alloc_lines[i].allocs[j].buf < (unsigned char *)buf && (unsigned char *)f_alloc_lines[i].allocs[j].buf + f_alloc_lines[i].allocs[j].size > (unsigned char *)buf)
			{
				printf("FORGE Mem debugger error: Trying to free pointer %p that is not at the start (%i bytes in) of allocation made on line %u in file %s\n", f_freed_memory[i].pointer, (int)((unsigned char *)buf - (unsigned char *)f_alloc_lines[i].allocs[j].buf), f_freed_memory[i].free_line, f_freed_memory[i].free_file);
				FORGE_CALL_ON_ERROR;
				return FALSE;
			}
		}	
	}
	for(i = 0; i < f_freed_memory_count && i < f_freed_memory_store; i++)
	{
		if(f != &f_freed_memory[i] && buf == f_freed_memory[i].pointer)
		{
			unsigned int *a = NULL;
			if(f->realloc)
				printf("FORGE Mem debugger error: Pointer %p s freed twice! it was freed on line %u in %s, was reallocated (%u bytes) on line %u in file %s\n", f_freed_memory[i].pointer, f_freed_memory[i].free_line, f_freed_memory[i].free_file, (unsigned int)f_freed_memory[i].size, f_freed_memory[i].alloc_line, f_freed_memory[i].alloc_file);
			else
				printf("FORGE Mem debugger error: Pointer %p is freed twice! it was freed on line %u in %s, was allocated (%u bytes) on line %u in file %s\n", f_freed_memory[i].pointer, f_freed_memory[i].free_line, f_freed_memory[i].free_file, (unsigned int)f_freed_memory[i].size, f_freed_memory[i].alloc_line, f_freed_memory[i].alloc_file);
			FORGE_CALL_ON_ERROR;
			return FALSE;
		}
	}
	if(forge_memory_stack_size != 0)
	{
		if(forge_memory_stack_pointer <= buf && &forge_memory_stack_pointer[forge_memory_stack_size] > buf)
		{
			printf("FORGE Mem debugger error: Trying to free stack pointer on line %u in file %s.\n", line, file);
			FORGE_CALL_ON_ERROR;
			return TRUE;
		}
	}else
	{
		if(buf > (unsigned char *)&i)
			distance = buf - (unsigned char *)&i;
		else
			distance = (unsigned char *)&i - buf;
		if(distance < FORGE_STACK_SIZE)
		{
			printf("FORGE Mem debugger error: Trying to free pointer, not allocated as far as FORGE knows on line %u in file %s. Very likly to be stack pointer (%u bytes from known stack pointer)\n", line, file, (unsigned int)distance);
			FORGE_CALL_ON_ERROR;
			return TRUE;
		}
	}


	

	printf("FORGE Mem debugger varning: Trying to free pointer, not allocated as far as FORGE knows on line %u in file %s. \n", line, file);
	
	if(NULL != f_debug_mem_query_allocation(buf, &line, &file, &s))
		printf("FORGE Mem debugger error: Pointer is part of allocation allocated at line %u in file %s. \n", line, file);
	
	free(buf);

	return TRUE;
}



void f_debug_mem_free(void *buf, char *file, unsigned int line)
{
	size_t size = 0;
#ifdef FORGE_MEMORY_CHECK_ALWAYS
	f_debug_mem_check_bounds();
#endif
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	if(f_debug_mem_remove(buf, file, line, FALSE, &size))
		if(forge_memory_log_file != NULL && forge_memory_active)
			fprintf(forge_memory_log_file, "Free %u bytes at pointer %p at %s line %u\n", (unsigned int)size, buf, file, line);

	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
}

boolean f_debug_mem_comment(void *buf, char *comment)
{
	unsigned int i, j;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].buf == buf)
			{
				f_alloc_lines[i].allocs[j].comment = comment;
				if(f_alloc_mutex != NULL)
					f_alloc_mutex_unlock(f_alloc_mutex);
				return TRUE;
			}
		}
	}
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return FALSE;
}


void *f_debug_mem_realloc(void *pointer, size_t size, char *file, unsigned int line)
{
	size_t i, j = 0, k, move;
	unsigned char *pointer2;	
#ifdef FORGE_MEMORY_CHECK_ALWAYS
	f_debug_mem_check_bounds();
#endif

	if(pointer == NULL)
	{
#ifdef FORGE_WARN_ON_REALLOC_NULL
		printf("FORGE Mem debugger warning: Realocating NULL in %s line %u. UB since C23.\n", file, line);
#endif
		return f_debug_mem_malloc(size, file, line);
	}
	
	if(size == 0)
	{
		printf("FORGE Mem debugger warning: realloc size ZERO in file %s line %u\n", file, line);
		FORGE_CALL_ON_ERROR
		return NULL;
	}

	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
			if(f_alloc_lines[i].allocs[j].buf == pointer)
				break;
		if(j < f_alloc_lines[i].alloc_count)
			break;
	}
	if(i == f_alloc_line_count)
	{
		printf("FORGE Mem debugger error: Trying to reallocate pointer %p in %s line %u. Pointer is not allocated.", pointer, file, line);
		for(i = 0; i < f_alloc_line_count; i++)
		{
			for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
			{
				unsigned int *buf;
				buf = f_alloc_lines[i].allocs[j].buf;
				for(k = 0; k < f_alloc_lines[i].allocs[j].size; k++)
				{
					if(&buf[k] == pointer)
					{
						printf("Trying to reallocate pointer %u bytes (out of %u) in to allocation made in %s on line %u.\n", (unsigned int)k, (unsigned int)f_alloc_lines[i].allocs[j].size, f_alloc_lines[i].file, f_alloc_lines[i].line);
						FORGE_CALL_ON_ERROR;
						if(f_alloc_mutex != NULL)
							f_alloc_mutex_unlock(f_alloc_mutex);
						return NULL;
					}
				}
			}
		}

		printf("\n");
		FORGE_CALL_ON_ERROR;

#ifdef FORGE_WARN_ON_REALLOC_NULL
		if(size == 0)
			printf("FORGE Mem debugger warning. Realocating pointer to zero size on line %u in file %s. UB since C23.\n", line, file);
#endif;
		return realloc(pointer, size);
	}
	if(size == 0)
	{
#ifdef FORGE_WARN_ON_REALLOC_NULL
		printf("FORGE Mem debugger warning. Realocating pointer to zero size on line %u in file %s. UB since C23.\n", line, file);
#endif
		f_debug_mem_free(pointer, file, line);
		return NULL;
	}


	pointer2 = malloc(size + FORGE_MEMORY_OVER_ALLOC);
	if(pointer2 == NULL)
	{
		printf("FORGE Mem debugger warning: Realloc returns NULL when trying to allocate %u bytes at line %u in file %s\n", (unsigned int)size, line, file);
		FORGE_CALL_ON_ERROR;
		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		return NULL;
	}
	memset(pointer2, F_MEMORY_MAGIC_NUMBER, FORGE_MEMORY_PRE_PADDIG);
	move = f_alloc_lines[i].allocs[j].size;
	if(move > size)
	{
		move = size;
		memcpy(&pointer2[FORGE_MEMORY_PRE_PADDIG], pointer, move);
		memset(&pointer2[FORGE_MEMORY_PRE_PADDIG + move], F_MEMORY_MAGIC_NUMBER, FORGE_MEMORY_OVER_ALLOC - FORGE_MEMORY_PRE_PADDIG);
	}else
	{
		memcpy(&pointer2[FORGE_MEMORY_PRE_PADDIG], pointer, move);
		memset(&pointer2[FORGE_MEMORY_PRE_PADDIG + move], F_MEMORY_INITIALIZATION, size - move);
		memset(&pointer2[FORGE_MEMORY_PRE_PADDIG + size], F_MEMORY_MAGIC_NUMBER, FORGE_MEMORY_OVER_ALLOC - FORGE_MEMORY_PRE_PADDIG);
	}
	pointer2 += FORGE_MEMORY_PRE_PADDIG;
	f_debug_mem_add(pointer2, size, file, line);
	move = 0;
	f_debug_mem_remove(pointer, file, line, TRUE, &move);
	if(forge_memory_log_file != NULL && forge_memory_active)
		fprintf(forge_memory_log_file, "Relloc %u bytes at pointer %p to %u bytes at pointer %p at %s line %u\n", (unsigned int)size, pointer, (unsigned int)move, pointer2, file, line);
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return pointer2;
}

void f_debug_mem_print(unsigned int min_allocs)
{
	unsigned int i, j, alloc_count;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	
	printf("Memory repport: %u Bytes\n----------------------------------------------\n", (unsigned int)f_debug_mem_consumption()); // FIX ME
	for(i = 0; i < f_alloc_line_count; i++)
	{
		if(min_allocs < f_alloc_lines[i].alocated - f_alloc_lines[i].freed)
		{
			for(j = alloc_count = 0; j < f_alloc_lines[i].alloc_count; j++)
				if(f_alloc_lines[i].allocs[j].active)
					alloc_count++;
			if(alloc_count > 0)
			{
				printf("%s line: %u\n", f_alloc_lines[i].file, f_alloc_lines[i].line);
				printf(" - Bytes allocated: %u\n - Allocations: %u\n - Frees: %u\n\n", (unsigned int)f_alloc_lines[i].size, (unsigned int)alloc_count, (unsigned int)f_alloc_lines[i].freed);
				for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
					if(f_alloc_lines[i].allocs[j].comment != NULL)
						printf("\t\t comment %p : %s\n", f_alloc_lines[i].allocs[j].buf, f_alloc_lines[i].allocs[j].comment);
			}
		}	
	}
	printf("----------------------------------------------\n");
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
}


size_t f_debug_mem_footprint(unsigned int min_allocs)
{
	unsigned int i;
	size_t size = 0;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
		size += f_alloc_lines[i].size;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return size;
}

void *f_debug_mem_query_allocation(void *pointer, unsigned int *line, char **file, size_t *size)
{
	unsigned int i, j;  
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if((unsigned char*)f_alloc_lines[i].allocs[j].buf <= (unsigned char*)pointer &&
			   (unsigned char *)f_alloc_lines[i].allocs[j].buf + f_alloc_lines[i].allocs[j].size > (unsigned char *)pointer) /* technically UB */
			{
				if(line != NULL)
					*line = f_alloc_lines[i].line;
				if(file != NULL)
					*file = f_alloc_lines[i].file;
				if(size != NULL)
					*size = f_alloc_lines[i].allocs[j].size;
				if(f_alloc_mutex != NULL)
					f_alloc_mutex_unlock(f_alloc_mutex);
				return f_alloc_lines[i].allocs[j].buf;
			}
		}
	}
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return NULL;
}

boolean f_debug_mem_query_is_allocated(void *pointer, size_t size, boolean ignore_not_found)
{
	unsigned int i, j;  
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].buf >= pointer && ((unsigned char *)f_alloc_lines[i].allocs[j].buf) + f_alloc_lines[i].allocs[j].size <= (unsigned char *)pointer) /* technically UB */
			{
				if(((unsigned char *)f_alloc_lines[i].allocs[j].buf) + f_alloc_lines[i].allocs[j].size < ((unsigned char *)pointer) + size)
				{
					printf("FORGE Mem debugger error: Not enough memory to access pointer %p, %u bytes missing\n", pointer, (unsigned int)(((unsigned char *)f_alloc_lines[i].allocs[j].buf) + f_alloc_lines[i].allocs[j].size) - (unsigned int)(((unsigned char *)pointer) + size));
					if(f_alloc_mutex != NULL)
						f_alloc_mutex_unlock(f_alloc_mutex);
					return FALSE;
				}else
				{
					if(f_alloc_mutex != NULL)
						f_alloc_mutex_unlock(f_alloc_mutex);
					return TRUE;
				}
			}
		}
	}


	for(i = 0; i < f_freed_memory_count; i++)
	{
		if(pointer >= f_freed_memory[i].pointer && ((unsigned char *)f_freed_memory[i].pointer) + f_freed_memory[i].size >= ((unsigned char *)pointer) + size) /* technically UB */
		{
			printf("FORGE Mem debugger error: Pointer %p was freed on line %u in file %s\n", pointer, f_freed_memory[i].free_line, f_freed_memory[i].free_file);
		}
	}
	if(forge_memory_stack_size != 0 && pointer >= forge_memory_stack_pointer && &forge_memory_stack_pointer[forge_memory_stack_size] > pointer)
	{
		if(&forge_memory_stack_pointer[forge_memory_stack_size] <  &((unsigned char *)pointer)[size])
			printf("FORGE Mem debugger Eror: memmory is in stack, but does not fit.\n", pointer, f_freed_memory[i].free_line, f_freed_memory[i].free_file);
		else
			printf("FORGE Mem debugger warning: memmory is in stack.\n", pointer, f_freed_memory[i].free_line, f_freed_memory[i].free_file);
		return FALSE;
	}
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	if(ignore_not_found)
		return FALSE;
	printf("FORGE Mem debugger warning: No matching memory for pointer %p found!\n", pointer);
	return FALSE;
}


size_t f_debug_mem_consumption(void)
{
	unsigned int i;
	size_t sum = 0;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
		sum += f_alloc_lines[i].size;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return sum;
}

void exit_crash(unsigned int i)
{
	unsigned int *a = NULL;
	a[0] = 0;
} 

boolean f_debug_mem_check_stack_reference()
{
	boolean output = FALSE;
	size_t size;
	size_t i, j, k, distance, best = 1024 * 1024, found = ~0, **buf;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);

	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].active)
			{
				buf = f_alloc_lines[i].allocs[j].buf;
				size = f_alloc_lines[i].allocs[j].size / sizeof(void *);
				if(forge_memory_stack_size != 0)
				{
					for(k = 0; k < size; k++)
					{
						if(buf[k] >= forge_memory_stack_pointer && buf[k] < &forge_memory_stack_pointer[forge_memory_stack_size])
						{
							if(f_alloc_lines[i].allocs[j].comment == NULL)
								printf("FORGE Mem debugger varning: Suspected reference to stack variable %u bytes in to in allocation made on line %u in file %s (%u bytes off known stack pointer)\n", (unsigned int)(k * sizeof(void*)), f_alloc_lines[i].line, f_alloc_lines[i].file, (unsigned int)best);
							else
								printf("FORGE Mem debugger varning: Suspected reference to stack variable %u bytes in to in allocation made on line %u in file %s /* %s */ (%u bytes off known stack pointer)\n", (unsigned int)(k * sizeof(void*)), f_alloc_lines[i].line, f_alloc_lines[i].file, f_alloc_lines[i].allocs[j].comment, (unsigned int)best);

						}
					}
				}else
				{
					for(k = 0; k < size; k++)
					{
						if(buf[k] > &i)
							distance = buf[k] - &i;
						else
							distance = &i - buf[k];
						if(distance < best)
						{
							best = distance;
							found = k * sizeof(void *);
						}
					}
					if(found != ~0)
					{
						if(f_alloc_lines[i].allocs[j].comment == NULL)
							printf("FORGE Mem debugger varning: Suspected reference to stack variable %u bytes in to in allocation made on line %u in file %s (%u bytes off known stack pointer)\n", (unsigned int)found, f_alloc_lines[i].line, f_alloc_lines[i].file, (unsigned int)best);
						else
							printf("FORGE Mem debugger varning: Suspected reference to stack variable %u bytes in to in allocation made on line %u in file %s /* %s */ (%u bytes off known stack pointer)\n", (unsigned int)found, f_alloc_lines[i].line, f_alloc_lines[i].file, f_alloc_lines[i].allocs[j].comment, (unsigned int)best);
						output = TRUE;
					}
				}
			}
		}
	}
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return output;
}





boolean f_debug_find_pointer_in_memory(void *pointer)
{
	boolean output = FALSE;
	size_t size;
	size_t i, j, k;
	void **buf, *read;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			buf = f_alloc_lines[i].allocs[j].buf;
			size = f_alloc_lines[i].allocs[j].size / sizeof(void *);
			for(k = 0; k < size; k++)
			{
				memcpy(&read, &buf[k], sizeof(void *));
				if(pointer == read)
				{
					if(f_alloc_mutex != NULL)
						f_alloc_mutex_unlock(f_alloc_mutex);
					return TRUE;
				}
			}
		}
	}
	buf = (void **)forge_memory_stack_pointer;
	for(j = 0; j < forge_memory_stack_size / sizeof(void *) && buf[j] != pointer; j++);
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return j < forge_memory_stack_size / sizeof(void *);
}

void f_debug_mem_check_heap_reference(unsigned int minimum_allocations)
{
	boolean output = FALSE;
	size_t i, j, found, alloc_count;

	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);

	for(i = 0; i < f_alloc_line_count; i++)
	{
		if(f_alloc_lines[i].alloc_count >= minimum_allocations)
		{
			for(found = alloc_count = j = 0; j < f_alloc_lines[i].alloc_count; j++)
			{
				if(f_alloc_lines[i].allocs[j].active)
				{
					alloc_count++;
					if(f_debug_find_pointer_in_memory(f_alloc_lines[i].allocs[j].buf))
						found++;
				}
			}
			if(found != f_alloc_lines[i].alloc_count)
			{
				if(forge_memory_stack_size)
					printf("FORGE Mem debugger Error: Cant find any reference in heap memory or stack of %u out of %u allocations made on line %u in file %s\n", f_alloc_lines[i].alloc_count - (unsigned int)found, f_alloc_lines[i].alloc_count, f_alloc_lines[i].line, f_alloc_lines[i].file);
				else
					printf("FORGE Mem debugger Warning: Cant find any reference in heap memory of %u out of %u allocations made on line %u in file %s\n", f_alloc_lines[i].alloc_count - (unsigned int)found, f_alloc_lines[i].alloc_count, f_alloc_lines[i].line, f_alloc_lines[i].file);
			}
		}
	}
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
}