#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#include <winnt.h>
#include <winreg.h>
#include <mmsystem.h>
#include <process.h>
#include "forge.h"

void *imagine_mutex_create()
{
	CRITICAL_SECTION *mutex;
	mutex = malloc(sizeof *mutex);
	InitializeCriticalSection(mutex);
	return mutex;
}

void imagine_mutex_destroy(void *mutex)
{
    DeleteCriticalSection(mutex);
    free(mutex);
}

void imagine_mutex_lock(void *mutex)
{
	EnterCriticalSection(mutex);
}

boolean imagine_mutex_lock_try(void *mutex)
{
	return TryEnterCriticalSection(mutex);
}

void imagine_mutex_unlock(void *mutex)
{
	LeaveCriticalSection(mutex);
}
#else
#include <pthread.h>
#include <unistd.h>
#include "forge.h"

void *imagine_mutex_create()
{
    pthread_mutex_t *mutex;
    mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    return mutex;
}
void imagine_mutex_destroy(void *mutex)
{
    pthread_mutex_destroy(mutex);
    free(mutex);
}
void imagine_mutex_lock(void *mutex)
{
    pthread_mutex_lock(mutex);
}

boolean imagine_mutex_lock_try(void *mutex)
{
    return pthread_mutex_trylock(mutex);
}

void imagine_mutex_unlock(void *mutex)
{
    pthread_mutex_unlock(mutex);
}

#endif

/* Signals */
#ifdef _WIN32

void *imagine_signal_create()
{
//	HANDLE *p;
	CONDITION_VARIABLE *p;
	p = malloc(sizeof *p);
//	p[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	InitializeConditionVariable(p);
	return p;
}

void imagine_signal_destroy(void *signal)
{
//	CloseHandle(&((HANDLE *)signal));
	free(signal);
}

boolean imagine_signal_wait(void *signal, void *mutex)
{
	SleepConditionVariableCS(signal, mutex, INFINITE);
	return TRUE;
//	return WAIT_OBJECT_0 == WaitForSingleObject(&((HANDLE *)signal), INFINITE);
}

boolean imagine_signal_activate(void *signal)
{
	WakeConditionVariable(signal);
	return TRUE;
//	SetEvent(&((HANDLE *)signal));
}

boolean imagine_signal_activate_all(void *signal)
{
	WakeAllConditionVariable(signal);
	return TRUE;
}
#else


void *imagine_signal_create()
{
	pthread_cond_t *p;
	p = malloc(sizeof *p);
	pthread_cond_init(p, NULL);
	return p;
}

void magine_signal_destroy(void *signal)
{
    pthread_cond_destroy(signal);
	free(signal);
}

boolean imagine_signal_wait(void *signal, void *mutex)
{
	pthread_cond_wait(signal, mutex);
	return TRUE;
}

boolean imagine_signal_activate(void *signal)
{
    pthread_cond_signal(signal);
	return TRUE;
}

boolean imagine_signal_activate_all(void *signal)
{
    pthread_cond_broadcast(signal);
	return TRUE;
}

#endif

typedef struct{
	void *mutex;
	char *file;
	uint line;
	boolean locked;
	boolean deleted;
}IDebugLock;

IDebugLock *i_debug_locks = NULL;
uint i_debug_lock_count = 0;
uint i_debug_lock_allocated = 0;
void *i_debug_lock_mutex = NULL;

void *imagine_mutex_create_debug()
{
	void *mutex;
	if(i_debug_lock_mutex == NULL)
		i_debug_lock_mutex = imagine_mutex_create();

	mutex = imagine_mutex_create();
	imagine_mutex_lock(i_debug_lock_mutex);
	if(i_debug_lock_count == i_debug_lock_allocated)
	{
		i_debug_lock_allocated += 256;
		i_debug_locks = realloc(i_debug_locks, (sizeof *i_debug_locks) * i_debug_lock_allocated);  
	}
	i_debug_locks[i_debug_lock_count].mutex = mutex;
	i_debug_locks[i_debug_lock_count].file = "No file";
	i_debug_locks[i_debug_lock_count].line = 0;
	i_debug_locks[i_debug_lock_count].deleted = FALSE;
	i_debug_locks[i_debug_lock_count++].locked = FALSE;
	if(i_debug_lock_count == 4)
		i_debug_lock_count += 0;
	imagine_mutex_unlock(i_debug_lock_mutex);
	return mutex;
}

void imagine_signal_destroy_debug(void *mutex, char *file, uint line)
{
	uint i;
	imagine_mutex_lock(i_debug_lock_mutex);
	for(i = 0; i < i_debug_lock_count && i_debug_locks[i].mutex != mutex; i++);
	if(i == i_debug_lock_count)
	{
		printf("Error: Imagine: trying to destroy a lock that hassent allocated: file %s line %u\n", file, line);	
		exit(0);		
	}
	if(i_debug_locks[i].deleted)
	{
		printf("Error: Imagine: trying to destroy a lock that has already been deleted: file %s line %u. Was already deleted at: file %s line %u\n", file, line, i_debug_locks[i].file, i_debug_locks[i].line);
 		exit(0);
	}
	i_debug_locks[i].file = file;
	i_debug_locks[i].line = line;
	i_debug_locks[i].deleted = TRUE;
	imagine_mutex_unlock(i_debug_lock_mutex);
}

void imagine_mutex_lock_debug(void *mutex, char *file, uint line)
{
	uint i;
	imagine_mutex_lock(i_debug_lock_mutex);
	for(i = 0; i < i_debug_lock_count && i_debug_locks[i].mutex != mutex; i++);
	if(i == i_debug_lock_count)
	{
		printf("Error: Imagine: trying to lock a lock that hassent allocated: file %s line %u\n", file, line);	
		exit(0);		
	}
	if(i_debug_locks[i].deleted)
	{
		printf("Error: Imagine: trying to lock a lock that has been deleted: file %s line %u\n", file, line);	
		exit(0);
	}
/*	if(i_debug_locks[i].locked)
	{
		printf("Warning: Imagine: trying to lock already locked lock: file %s line %u\n", file, line);
		if(i_debug_locks[i].file != file || i_debug_locks[i].line != line)
			printf("Locked in: file %s line %u\n", i_debug_locks[i].file, i_debug_locks[i].line);
	}*/
	imagine_mutex_unlock(i_debug_lock_mutex);

	imagine_mutex_lock(mutex);
	
	imagine_mutex_lock(i_debug_lock_mutex);
	i_debug_locks[i].file = file;
	i_debug_locks[i].line = line;
	i_debug_locks[i].locked = TRUE;
	imagine_mutex_unlock(i_debug_lock_mutex);
}

boolean imagine_mutex_lock_try_debug(void *mutex, char *file, uint line)
{
	boolean output;
	uint i;
	imagine_mutex_lock(i_debug_lock_mutex);
	for(i = 0; i < i_debug_lock_count && i_debug_locks[i].mutex != mutex; i++);
	if(i == i_debug_lock_count)
	{
		printf("Error: Imagine: trying to lock a lock that hassent allocated: file %s line %u\n", file, line);	
		exit(0);		
	}
	if(i_debug_locks[i].deleted)
	{
		printf("Error: Imagine: trying to lock a lock that has been deleted: file %s line %u\n", file, line);	
		exit(0);
	}
/*	if(i_debug_locks[i].locked)
	{
		printf("Warning: Imagine: trying to lock already locked lock: file %s line %u\n", file, line);
		if(i_debug_locks[i].file != file || i_debug_locks[i].line != line)
			printf("Locked in: file %s line %u\n", i_debug_locks[i].file, i_debug_locks[i].line);
	}*/
	imagine_mutex_unlock(i_debug_lock_mutex);
	output = imagine_mutex_lock_try(mutex);
	if(output)
	{		
		imagine_mutex_lock(i_debug_lock_mutex);
		i_debug_locks[i].file = file;
		i_debug_locks[i].line = line;
		i_debug_locks[i].locked = TRUE;
		imagine_mutex_unlock(i_debug_lock_mutex);
	}
	return output;
}

boolean imagine_signal_wait_debug(void *signal, void *mutex, char *file, uint line)
{
	boolean output;
	uint i;
	imagine_mutex_lock(i_debug_lock_mutex);
	for(i = 0; i < i_debug_lock_count && i_debug_locks[i].mutex != mutex; i++);
	if(i == i_debug_lock_count)
	{
		printf("Error: Imagine: trying to lock a lock that hassent allocated: file %s line %u\n", file, line);	
		exit(0);		
	}
	if(i_debug_locks[i].deleted)
	{
		printf("Error: Imagine: trying to lock a lock that has been deleted: file %s line %u\n", file, line);	
		exit(0);
	}
/*	if(i_debug_locks[i].locked)
	{
		printf("Warning: Imagine: trying to lock already locked lock: file %s line %u\n", file, line);
		if(i_debug_locks[i].file != file || i_debug_locks[i].line != line)
			printf("Locked in: file %s line %u\n", i_debug_locks[i].file, i_debug_locks[i].line);
	}*/
	if(!i_debug_locks[i].locked)
	{
		printf("Warning: Imagine: trying to wait on a lock that isnt already locked: file %s line %u\n", file, line);
		if(i_debug_locks[i].file != file || i_debug_locks[i].line != line)
			printf("Locked in: file %s line %u\n", i_debug_locks[i].file, i_debug_locks[i].line);
	}
	i_debug_locks[i].file = file;
	i_debug_locks[i].line = line;
	i_debug_locks[i].locked = FALSE;
	imagine_mutex_unlock(i_debug_lock_mutex);
	output = imagine_signal_wait(signal, mutex);
	imagine_mutex_lock(i_debug_lock_mutex);
	i_debug_locks[i].file = file;
	i_debug_locks[i].line = line;
	i_debug_locks[i].locked = TRUE;
	imagine_mutex_unlock(i_debug_lock_mutex);
	return output;
}


boolean imagine_mutex_is_locked_debug(void *mutex)
{
	uint i;
	imagine_mutex_lock(i_debug_lock_mutex);
	for(i = 0; i < i_debug_lock_count && i_debug_locks[i].mutex != mutex; i++);
	if(i == i_debug_lock_count)
	{
		printf("Error: Imagine: trying to see if alock is locked but the lock hassent been allocated.\n");	
		exit(0);		
	}
	i = i_debug_locks[i].locked;
	imagine_mutex_unlock(i_debug_lock_mutex);
	return (boolean)i;
}

void imagine_mutex_unlock_debug(void *mutex, char *file, uint line)
{
	uint i;
	imagine_mutex_lock(i_debug_lock_mutex);
	for(i = 0; i < i_debug_lock_count && i_debug_locks[i].mutex != mutex; i++);
	if(i == i_debug_lock_count)
	{
		printf("Error: Imagine: trying to lock a lock that hassent allocated: file %s line %u\n", file, line);	
		exit(0);		
	}
	if(i_debug_locks[i].deleted)
	{
		printf("Error: Imagine: trying to lock a lock that has been deleted: file %s line %u\n", file, line);	
		exit(0);
	}
	if(!i_debug_locks[i].locked)
	{
		printf("Error: Imagine: trying to unlock lock that hassent been lockled: file %s line %u\n", file, line);	
		exit(0);
	}
	i_debug_locks[i].file = file;
	i_debug_locks[i].line = line;
	i_debug_locks[i].locked = FALSE;
	imagine_mutex_unlock(i_debug_lock_mutex);
	imagine_mutex_unlock(mutex);
}


void imagine_mutex_print_debug()
{
	uint i;
	imagine_mutex_lock(i_debug_lock_mutex);
	printf("Printing list of all %u locks:\n", i_debug_lock_count);
	for(i = 0; i < i_debug_lock_count; i++)
	{
		if(!i_debug_locks[i].deleted)
		{
			if(i_debug_locks[i].locked)
				printf("Mutex at: file %s line %u LOCKED\n", i_debug_locks[i].file, i_debug_locks[i].line);
			else
				printf("Mutex at: file %s line %u unlocked\n", i_debug_locks[i].file, i_debug_locks[i].line);
		}
	}
	for(i = 0; i < i_debug_lock_count; i++)
		if(i_debug_locks[i].deleted)
			printf("Deleted Mutex at: file %s line %u\n", i_debug_locks[i].file, i_debug_locks[i].line);
	imagine_mutex_unlock(i_debug_lock_mutex);
}



/* Treads */

typedef struct{
    void	(*func)(void *data);
    void	*data;
}ImagineThreadParams;

#ifdef _WIN32

DWORD WINAPI i_win32_thread(LPVOID param)
{
	ImagineThreadParams *thread_param;
	thread_param = (ImagineThreadParams *)param; 
	thread_param->func(thread_param->data);
	free(thread_param);
	return TRUE;
}

#pragma pack(push, 8)
typedef struct {
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
}ImagineThreadNameParam;
#pragma pack(pop)

void imagine_thread(void (*func)(void *data), void *data, char *name)
{
	ImagineThreadNameParam info;
	ImagineThreadParams *thread_param;
	DWORD dwThreadID;
	thread_param = malloc(sizeof *thread_param);
	thread_param->func = func;
	thread_param->data = data;
	CreateThread(NULL, 0,  i_win32_thread, thread_param, 0, &info.dwThreadID);
#if defined(DEBUG) || defined(_DEBUG)
	info.dwType = 0x1000;
	info.dwFlags = 0;
	info.szName = name;
	RaiseException(0x406D1388 /*MS_VC_EXCEPTION*/, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);	
#endif
}

#else

void *i_thread_thread(void *param)
{
    ImagineThreadParams *thread_param;
    thread_param = (ImagineThreadParams *)param;
    thread_param->func(thread_param->data);
    free(thread_param);
    return NULL;
}

void imagine_thread(void (*func)(void *data), void *data, char *name)
{
    pthread_t thread_id;
    ImagineThreadParams *thread_param;
    thread_param = malloc(sizeof *thread_param);
    thread_param->func = func;
    thread_param->data = data;
    pthread_create(&thread_id, NULL, i_thread_thread, thread_param);
	pthread_setname_np(thread_id, name);
}
#endif


/* Execute */
#ifdef _WIN32

void imagine_sleepi(uint seconds, uint nano_seconds)
{
	Sleep(nano_seconds / 1000000 + seconds * 1000);
}

#else

void imagine_sleepi(uint seconds, uint nano_seconds)
{
	struct timespec times;
	times.tv_sec = 0;        /* seconds */
	times.tv_nsec = nano_seconds;
	nanosleep(&times, NULL); 
}

#endif

void imagine_sleepd(double time)
{
	imagine_sleepi((uint)time, (time - (double)((uint)time)) * 1000000000);
}

#ifdef IMAGINE_ATOMIC_EMULATION

void imagine_atomic_pointer_init(IAtomicPointer *pointer, void *value)
{
	pointer->mutex = imagine_mutex_create();
	pointer->pointer = value;
}

void *imagine_atomic_pointer_read(IAtomicPointer *pointer)
{
	void *output;
	imagine_mutex_lock(pointer->mutex);
	output = pointer->pointer;
	imagine_mutex_lock(pointer->mutex);
	return output;
}

void imagine_atomic_pointer_write(IAtomicPointer *pointer, void *value)
{
	imagine_mutex_lock(pointer->mutex);
	pointer->pointer = value;
	imagine_mutex_lock(pointer->mutex);
}

boolean imagine_atomic_pointer_compare_and_exchange(IAtomicPointer *pointer, void *expected, void *new_value)
{
	boolean output;
	imagine_mutex_lock(pointer->mutex);
	output = pointer->pointer == expected;
	if(output)
		pointer->pointer = expected;
	imagine_mutex_lock(pointer->mutex);
	return output;
}

void imgine_atomic_pointer_free(IAtomicPointer *pointer)
{
	imagine_mutex_destory(pointer->mutex);
}

void imgine_atomic_integer_init(IAtomicInteger *pointer, uint64 value)
{
	pointer->mutex = imagine_mutex_create();
	pointer->integer = value;
}

uint64 imgine_atomic_integer_read(IAtomicInteger *pointer)
{
	uint64 *output;
	imagine_mutex_lock(pointer->mutex);
	output = pointer->integer;
	imagine_mutex_lock(pointer->mutex);
	return output;
}

boolean	imgine_atomic_integer_compare_and_exchange(IAtomicInteger *integer, uint64 expected, uint64 new_value)
{
	boolean output;
	imagine_mutex_lock(integer->mutex);
	output = integer->integer == expected;
	if(output)
		integer->integer = expected;
	imagine_mutex_lock(integer->mutex);
	return output;
}

void imagine_atomic_integer_add(IAtomicInteger *pointer, uint64 value)
{
	imagine_mutex_lock(integer->mutex);
	integer->integer += value;
	imagine_mutex_lock(integer->mutex);
}

void imagine_atomic_integer_subtract(IAtomicInteger *pointer, uint64 value)
{
	imagine_mutex_lock(integer->mutex);
	integer->integer += value;
	imagine_mutex_lock(integer->mutex);
}

void imgine_atomic_integer_free(IAtomicInteger *pointer)
{
	imagine_mutex_destory(pointer->mutex);
}

#endif



/* Execute */
#ifdef _WIN32

boolean imagine_execute(char *command)
{
    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    ZeroMemory(&process_info, sizeof(process_info));
    return CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &startup_info, &process_info);
}

#else

boolean imagine_execute(char *command)
{
    uint id;
    id = fork();
    if(id == 0)
        return execl(command, NULL);
    return  FALSE;
}

#endif

//#define IMAGINE_THREAD_TEST

#ifdef IMAGINE_THREAD_TEST

typedef struct{
	void *mutex;
	void *condition;
	uint id;
}ImagineThreadTestParam;

void imagine_thread_test_func(void *data)
{
	ImagineThreadTestParam *params;
	uint i;
	params = (ImagineThreadTestParam *)data;
//	imagine_mutex_lock(params->mutex);
	while(TRUE)
	{
		printf("thread %u ->", params->id);
		for(i = 0; i < 10; i++)
			printf(" %u", i + 1);
		printf("\n", params->id);
		imagine_signal_activate(params->condition);
		imagine_signal_wait(params->condition, params->mutex);
	}
}

int main()
{
	ImagineThreadTestParam params[2];
	params[0].mutex = params[1].mutex = imagine_mutex_create();
	params[0].condition = params[1].condition = imagine_signal_create();
	params[0].id = 0;
	imagine_thread(imagine_thread_test_func, &params[0], "Imagine Tread test");
	params[1].id = 1;	
	imagine_thread_test_func(&params[1]);
}

#endif
