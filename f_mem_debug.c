#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define F_MEMORY_INTERNAL
#define F_NO_MEMORY_DEBUG
#include "forge.h"

extern void f_debug_mem_print(uint min_allocs);

#define F_MEMORY_OVER_ALLOC 256
#define F_MEMORY_MAGIC_NUMBER 132
typedef struct{
	uint size;
	void *buf;
	char *comment;
}STMemAllocBuf;

typedef struct{
	uint line;
	char file[256];
	STMemAllocBuf *allocs;
	uint alloc_count;
	size_t alloc_alocated;
	size_t size;
	size_t alocated;
	size_t freed;
}STMemAllocLine;

STMemAllocLine f_alloc_lines[1024];
uint f_alloc_line_count = 0;

#define FORGE_FREE_POINTER_BUFFER_SIZE 1024

typedef struct{
	uint alloc_line;
	char alloc_file[256];
	uint free_line;
	char free_file[256];
	size_t size;
	void *pointer; 
	boolean realloc;
}STMemFreeBuf;

STMemFreeBuf f_freed_memory[FORGE_FREE_POINTER_BUFFER_SIZE];
uint f_freed_memory_current = 0;
uint f_freed_memory_count = 0;

void *f_alloc_mutex = NULL;
void (*f_alloc_mutex_lock)(void *mutex) = NULL;
void (*f_alloc_mutex_unlock)(void *mutex) = NULL;

void f_debug_memory_init(void (*lock)(void *mutex), void (*unlock)(void *mutex), void *mutex)
{
	f_alloc_mutex = mutex;
	f_alloc_mutex_lock = lock;
	f_alloc_mutex_unlock = unlock;
}


void *f_debug_memory_fopen(const char *file_name, const char *mode, char *file, uint line)
{
	FILE *f;
	f = fopen(file_name, mode);
	return f;
}

void f_debug_memory_fclose(void *f, char *file, uint line)
{
	fclose(f);
}


boolean f_debug_memory()
{
	boolean output = FALSE;
	uint i, j, k;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			uint8 *buf;
			size_t size;
			buf = f_alloc_lines[i].allocs[j].buf;
			size = f_alloc_lines[i].allocs[j].size;
			for(k = 0; k < F_MEMORY_OVER_ALLOC; k++)
				if(buf[size + k] != F_MEMORY_MAGIC_NUMBER)
					break;
			if(k < F_MEMORY_OVER_ALLOC)
			{
				if(f_alloc_lines[i].allocs[j].comment == NULL)
					printf("MEM ERROR: Overshoot at line %u in file %s\n", f_alloc_lines[i].line, f_alloc_lines[i].file);
				else
					printf("MEM ERROR: Overshoot at line %u in file %s /* %s */\n", f_alloc_lines[i].line, f_alloc_lines[i].file, f_alloc_lines[i].allocs[j].comment);
				{
					uint *X = NULL;
					X[0] = 0;
				}
				output = TRUE;
			}
		}
//		if(output)
//			exit(0);
	}
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return output;
}

void f_debug_mem_add(void *pointer, size_t size, char *file, uint line)
{
	uint i, j;
	for(i = 0; i < F_MEMORY_OVER_ALLOC; i++)
		((uint8 *)pointer)[size + i] = F_MEMORY_MAGIC_NUMBER;

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
				printf("MEM ERROR: Realloc returns NULL when trying to allocate %u bytes at line %u in file %s\n", size, line, file);
				exit(0);
			}
		}
		f_alloc_lines[i].allocs[f_alloc_lines[i].alloc_count].size = size;
		f_alloc_lines[i].allocs[f_alloc_lines[i].alloc_count].comment = NULL;
		f_alloc_lines[i].allocs[f_alloc_lines[i].alloc_count++].buf = pointer;
		f_alloc_lines[i].size += size;
		f_alloc_lines[i].alocated++;
	}else
	{
		if(i < 1024)
		{
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
			f_alloc_lines[i].size = size;
			f_alloc_lines[i].freed = 0;
			f_alloc_lines[i].alocated++;
			f_alloc_line_count++;
		}
	}
	for(i = 0; i < f_freed_memory_count; i++)
	{
		if(pointer == f_freed_memory[i].pointer)
		{
			f_freed_memory[i] = f_freed_memory[--f_freed_memory_count];
			break;
		}
	}
}

void *f_debug_mem_malloc(size_t size, char *file, uint line)
{
	void *pointer;
	uint i;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	f_debug_memory();
	pointer = malloc(size + F_MEMORY_OVER_ALLOC);

#ifdef F_MEMORY_PRINT 
	printf("Malloc %u bytes at pointer %p at %s line %u\n", size, pointer, file, line);
#endif
	if(pointer == NULL)
	{
		printf("MEM ERROR: Malloc returns NULL when trying to allocate %u bytes at line %u in file %s\n", size, line, file);
		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		f_debug_mem_print(0);
		exit(0);
	}
	for(i = 0; i < size + F_MEMORY_OVER_ALLOC; i++)
 		((uint8 *)pointer)[i] = F_MEMORY_MAGIC_NUMBER + 1;
	f_debug_mem_add(pointer, size, file, line);



	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return pointer;
}


boolean f_debug_mem_remove(void *buf, char *file, uint line, boolean realloc, size_t *size)
{
	STMemFreeBuf  *f;
	uint i, j, k;

	f = &f_freed_memory[f_freed_memory_current];
	f_freed_memory_current = (f_freed_memory_current + 1) % FORGE_FREE_POINTER_BUFFER_SIZE;
	if(f_freed_memory_current > f_freed_memory_count)
		f_freed_memory_count = f_freed_memory_current;
	for(i = 0; i < 255 && file[i] != 0; i++)
		f->free_file[i] = file[i];
	f->free_file[i] = 0;
	f->free_line = line;
	f->realloc = realloc;
	f->pointer = buf;

	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].buf == buf)
			{
				for(k = 0; k < F_MEMORY_OVER_ALLOC; k++)
					if(((uint8 *)buf)[f_alloc_lines[i].allocs[j].size + k] != F_MEMORY_MAGIC_NUMBER)
						break;
				if(k < F_MEMORY_OVER_ALLOC)
				{
					uint *a = NULL;
					printf("MEM ERROR: Overshoot at line %u in file %s\n", f_alloc_lines[i].line, f_alloc_lines[i].file);
					exit(0);
					a[0] = 0;
				}
				for(k = 0; k < f_alloc_lines[i].allocs[j].size; k++)
					((uint8 *)buf)[k] = 255;
				f->alloc_line = f_alloc_lines[i].line;
				for(k = 0; k < 255 && f_alloc_lines[i].file[k] != 0; k++)
					f->alloc_file[k] = f_alloc_lines[i].file[k];
				f->alloc_file[k] = 0;
				f->size = f_alloc_lines[i].allocs[j].size;
				*size = f_alloc_lines[i].allocs[j].size;
				f_alloc_lines[i].size -= f_alloc_lines[i].allocs[j].size;
				f_alloc_lines[i].allocs[j] = f_alloc_lines[i].allocs[--f_alloc_lines[i].alloc_count];
				f_alloc_lines[i].freed++;
				return TRUE;
			}
		}	
	}
	for(i = 0; i < f_freed_memory_count; i++)
	{
		if(f != &f_freed_memory[i] && buf == f_freed_memory[i].pointer)
		{
			uint *a = NULL;
			if(f->realloc)
				printf("MEM ERROR: Pointer %p in file is freed twice! if was freed one line %u in %s, was reallocated to %u bytes long one line %u in file %s\n", f->pointer, f->free_line, f->free_file, f->size, f->alloc_line, f->alloc_file);
			else
				printf("MEM ERROR: Pointer %p in file is freed twice! if was freed one line %u in %s, was allocated to %u bytes long one line %u in file %s\n", f->pointer, f->free_line, f->free_file, f->size, f->alloc_line, f->alloc_file);

			return FALSE;
		}
	}
	return TRUE;
}



void f_debug_mem_free(void *buf, char *file, uint line)
{
	STMemFreeBuf b;
	size_t size = 0;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	if(!f_debug_mem_remove(buf, file, line, FALSE, &size))
	{
		uint *X = NULL;
		X[0] = 0;
	}
	

#ifdef F_MEMORY_PRINT 
	printf("Free %u bytes at pointer %p at %s line %u\n", size, buf, file, line);
#endif

	free(buf);
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
}

boolean f_debug_mem_comment(void *buf, char *comment)
{
	uint i, j, k;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].buf == buf)
			{
				f_alloc_lines[i].allocs[j].comment = comment;
				return TRUE;
			}
		}
	}
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return FALSE;
}


void *f_debug_mem_realloc(void *pointer, size_t size, char *file, uint line)
{
	size_t i, j = 0, k, move;
	void *pointer2;	

	if(pointer == NULL)
		return f_debug_mem_malloc(size, file, line);
	
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
		printf("FORGE Mem debugger error. Trying to reallocate pointer %p in %s line %u. Pointer is not allocated\n", pointer, file, line);
		for(i = 0; i < f_alloc_line_count; i++)
		{
			for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
			{
				uint *buf;
				buf = f_alloc_lines[i].allocs[j].buf;
				for(k = 0; k < f_alloc_lines[i].allocs[j].size; k++)
				{
					if(&buf[k] == pointer)
					{
						printf("Trying to reallocate pointer %u bytes (out of %u) in to allocation made in %s on line %u.\n", k, f_alloc_lines[i].allocs[j].size, f_alloc_lines[i].file, f_alloc_lines[i].line);
					}
				}
			}
		}
		exit(0);
	}
	move = f_alloc_lines[i].allocs[j].size;
	
	if(move > size)
		move = size;

	pointer2 = malloc(size + F_MEMORY_OVER_ALLOC);
	if(pointer2 == NULL)
	{
		printf("MEM ERROR: Realloc returns NULL when trying to allocate %u bytes at line %u in file %s\n", size, line, file);
		if(f_alloc_mutex != NULL)
			f_alloc_mutex_unlock(f_alloc_mutex);
		f_debug_mem_print(0);
		exit(0);
	}
	for(i = 0; i < size + F_MEMORY_OVER_ALLOC; i++)
 		((uint8 *)pointer2)[i] = F_MEMORY_MAGIC_NUMBER;
	memcpy(pointer2, pointer, move);

	f_debug_mem_add(pointer2, size, file, line);
	move = 0;
	f_debug_mem_remove(pointer, file, line, TRUE, &move);
#ifdef F_MEMORY_PRINT 
	printf("Relloc %u bytes at pointer %p to %u bytes at pointer %p at %s line %u\n", size, pointer, move, pointer2, file, line);
#endif
	free(pointer);

	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return pointer2;
}

void f_debug_mem_print(uint min_allocs)
{
	uint i, j;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	printf("Memory repport:\n----------------------------------------------\n");
	for(i = 0; i < f_alloc_line_count; i++)
	{
		if(min_allocs < f_alloc_lines[i].alocated - f_alloc_lines[i].freed)
		{
			printf("%s line: %u\n",f_alloc_lines[i].file, f_alloc_lines[i].line);
			printf(" - Bytes allocated: %u\n - Allocations: %u\n - Frees: %u\n\n", f_alloc_lines[i].size, f_alloc_lines[i].alocated, f_alloc_lines[i].freed);
			for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
				if(f_alloc_lines[i].allocs[j].comment != NULL)
					printf("\t\t comment %p : %s\n", f_alloc_lines[i].allocs[j].buf, f_alloc_lines[i].allocs[j].comment);
		}
	}
	printf("----------------------------------------------\n");
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
}


size_t f_debug_mem_footprint(uint min_allocs)
{
	uint i, j;
	size_t size = 0;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
		size += f_alloc_lines[i].size;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return size;
}

boolean f_debug_mem_query(void *pointer, uint *line, char **file, size_t *size)
{
	uint i, j;  
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].buf == pointer)
			{
				if(line != NULL)
					*line = f_alloc_lines[i].line;
				if(file != NULL)
					*file = f_alloc_lines[i].file;
				if(size != NULL)
					*size = f_alloc_lines[i].allocs[j].size;
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

boolean f_debug_mem_test(void *pointer, size_t size, boolean ignore_not_found)
{
	uint i, j;  
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
	{
		for(j = 0; j < f_alloc_lines[i].alloc_count; j++)
		{
			if(f_alloc_lines[i].allocs[j].buf >= pointer && ((uint8 *)f_alloc_lines[i].allocs[j].buf) + f_alloc_lines[i].allocs[j].size <= pointer)
			{
				if(((uint8 *)f_alloc_lines[i].allocs[j].buf) + f_alloc_lines[i].allocs[j].size < ((uint8 *)pointer) + size)
				{
					printf("MEM ERROR: Not enough memory to access pointer %p, %u bytes missing\n", pointer, (uint)(((uint8 *)f_alloc_lines[i].allocs[j].buf) + f_alloc_lines[i].allocs[j].size) - (uint)(((uint8 *)pointer) + size));
					if(f_alloc_mutex != NULL)
						f_alloc_mutex_unlock(f_alloc_mutex);
					return TRUE;
				}else
				{
					if(f_alloc_mutex != NULL)
						f_alloc_mutex_unlock(f_alloc_mutex);
					return FALSE;
				}
			}
		}
	}
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	if(ignore_not_found)
		return FALSE;

	for(i = 0; i < f_freed_memory_count; i++)
	{
		if(pointer >= f_freed_memory[i].pointer && ((uint8 *)f_freed_memory[i].pointer) + f_freed_memory[i].size >= ((uint8 *)pointer) + size)
		{
			printf("MEM ERROR: Pointer %p was freed on line %u in file %s\n", pointer, f_freed_memory[i].free_line, f_freed_memory[i].free_file);
		}
	}

	printf("MEM ERROR: No matching memory for pointer %p found!\n", pointer);
	return TRUE;
}


size_t f_debug_mem_consumption()
{
	uint i;
	size_t sum = 0;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);
	for(i = 0; i < f_alloc_line_count; i++)
		sum += f_alloc_lines[i].size;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
	return sum;
}

void f_debug_mem_reset()
{
	uint i;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_lock(f_alloc_mutex);	
#ifdef F_MEMORY_PRINT 
	printf("Memmory reset --------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
#endif
	for(i = 0; i < f_alloc_line_count; i++)
		free(f_alloc_lines[i].allocs);
	f_alloc_line_count = 0;
	if(f_alloc_mutex != NULL)
		f_alloc_mutex_unlock(f_alloc_mutex);
}

void exit_crash(uint i)
{
	uint *a = NULL;
	a[0] = 0;
} 
