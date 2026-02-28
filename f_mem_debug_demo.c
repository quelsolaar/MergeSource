#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#ifdef DISABLED

#include "forge_memory_debugger.h" /* The forge memory debugger */

/* The worlds buggiest C program, and the debugger that can find the issues. */

int main()
{
	FILE *log;
	int *p, **p2;

	if((log = fopen("my_forge_log.txt", "w")) != NULL) /* log all allocations and frees. */
		f_debug_mem_log(log);

	p = malloc(sizeof *p);

	p2 = malloc((sizeof *p2) * 2);

	printf("memory content: %x\n", *p); /* read uninitialized memory. */

	printf("memory consumption: %u\n", (unsigned int)f_debug_mem_consumption()); /* print out how much memory has been allocated. */

	f_debug_mem_print(0); /* print a list of all allocations. */

	p[1] = 1138; /* buffer over run. */

	p[-1] = 2600; /* buffer under run. */

	free(p);

	p[0] = 7274; /* use after free. */   
   
	free(p); /* double free. */

	free(&p2[1]); /* free a pointer that doesnt point the start of an allocation.*/

	realloc(&p2[1], sizeof *p2); /* reallocte a pointer that doesnt point the start of an allocation. */

	malloc(~0); /* try to allocate an unreasonable ammount of memory, to force malloc to return NULL. L*/

	f_debug_mem_check_bounds(); /* find all errors. */

	{
		int x;
		*p2 = &x; /* store a pointer of a stack variable in to heap memory */

		free(&x); /* try to free stack variable */ 
	}

	f_debug_mem_check_stack_reference(); /* Find any allocation that contains a poiner to suspected stack memory */

	f_debug_mem_check_heap_reference(0); /* Find any allocations that cant be found any reference to in heap memory */

	f_debug_mem_log(NULL); /* stop logging */
	fclose(log);

	return 1;
}

/*

Expected output:

memory content: cdcdcdcd
memory consumption: 20
Memory repport:
----------------------------------------------
...\f_mem_debug_demo.c line: 15
 - Bytes allocated: 4
 - Allocations: 1
 - Frees: 0

...\f_mem_debug_demo.c line: 17
 - Bytes allocated: 16
 - Allocations: 1
 - Frees: 0

----------------------------------------------
FORGE Mem debugger error: Buffer underrun of allocation made on line 15 in file ...\f_mem_debug_demo.c
FORGE Mem debugger error: Buffer overrun of allocation made on line  15 in file ...\f_mem_debug_demo.c
FORGE Mem debugger error: Pointer 00000286BF6A1340 in file is freed twice! it was freed on line 29 in ...\f_mem_debug_demo.c, was allocated (4 bytes) on line 15 in file ...\f_mem_debug_demo.c
FORGE Mem debugger error: Trying to free pointer 00000286BF6A1340 that is not at the start (8 bytes in) of allocation made on line 33 in file ...\f_mem_debug_demo.c
FORGE Mem debugger error: Trying to reallocate pointer 00000286BF6F2C48 in ...\f_mem_debug_demo.c line 37. Pointer is not allocated.Trying to reallocate pointer 2 bytes (out of 16) in to allocation made in ...\f_mem_debug_demo.c on line 17.
FORGE Mem debugger varning: Malloc returns NULL at line 39 in file ...\f_mem_debug_demo.c
FORGE Mem debugger error: Pointer that was allocated on line 15 in file ...\f_mem_debug_demo.c, and freed on line 29 in file ...\f_mem_debug_demo.c was written to 0 bytes in to the allocation after being freed.
FORGE Mem debugger error: trying to free pointer, not allocated as far as FORGE knows on line 47 in file ...\f_mem_debug_demo.c. Very likly to be stack pointer (1664 bytes from known stack pointer)
FORGE Mem debugger varning: Suspected reference to stack variable 0 bytes in to in allocation made on line 17 in file ...\f_mem_debug_demo.c (71 bytes off known stack pointer)
FORGE Mem debugger varning: Cant find any reference in heap memory of 1 out of 1 allocations made on line 17 in file ...\f_mem_debug_demo.c

*/

#endif