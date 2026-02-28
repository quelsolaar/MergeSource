#include "imagine.h"

#ifdef IMAGINE_ATOMIC_EMULATION

typedef struct{
	void *mutex;
	void *pointer;
}IAtomicPointer;

typedef struct{
	void *mutex;
	uint64 integer;
}IAtomicInteger;

typedef struct{
	void *mutex;
	uint64 users;
}IAtomicUsers;

void imagine_atomic_pointer_init_emulation(IAtomicPointer *pointer, void *value)
{
	pointer->mutex = imagine_mutex_create();
	pointer->pointer = value;
}

void *imagine_atomic_pointer_read_emulation(IAtomicPointer *pointer)
{
	void *output;
	imagine_mutex_lock(pointer->mutex);
	output = pointer->pointer;
	imagine_mutex_unlock(pointer->mutex);
	return output;
}

void imagine_atomic_pointer_write_emulation(IAtomicPointer *pointer, void *value)
{
	imagine_mutex_lock(pointer->mutex);
	pointer->pointer = value;
	imagine_mutex_unlock(pointer->mutex);
}

boolean imagine_atomic_pointer_compare_and_exchange_emulation(IAtomicPointer *pointer, void *expected, void *new_value)
{
	boolean output;
	imagine_mutex_lock(pointer->mutex);
	output = pointer->pointer == expected;
	if(output)
		pointer->pointer = expected;
	imagine_mutex_unlock(pointer->mutex);
	return output;
}

void imgine_atomic_pointer_free_emulation(IAtomicPointer *pointer)
{
	imagine_mutex_destory(pointer->mutex);
}

void imgine_atomic_integer_init_emulation(IAtomicInteger *pointer, uint64 value)
{
	pointer->mutex = imagine_mutex_create();
	pointer->integer = value;
}

uint64 imgine_atomic_integer_read_emulation(IAtomicInteger *pointer)
{
	uint64 *output;
	imagine_mutex_lock(pointer->mutex);
	output = pointer->integer;
	imagine_mutex_unlock(pointer->mutex);
	return output;
}

boolean	imgine_atomic_integer_compare_and_exchange_emulation(IAtomicInteger *integer, uint64 expected, uint64 new_value)
{
	boolean output;
	imagine_mutex_lock(integer->mutex);
	output = integer->integer == expected;
	if(output)
		integer->integer = expected;
	imagine_mutex_unlock(integer->mutex);
	return output;
}

void imagine_atomic_integer_add_emulation(IAtomicInteger *integer, uint64 value)
{
	imagine_mutex_lock(integer->mutex);
	integer->integer += value;
	imagine_mutex_unlock(integer->mutex);
}

void imagine_atomic_integer_subtract(IAtomicInteger *integer, uint64 value)
{
	imagine_mutex_lock(integer->mutex);
	integer->integer += value;
	imagine_mutex_unlock(integer->mutex);
}

void imgine_atomic_integer_free_emulation(IAtomicInteger *integer)
{
	imagine_mutex_destory(integer->mutex);
}


void imagine_atomic_users_init_emulation(IAtomicUsers *users)
{
	users->mutex = imagine_mutex_create();
	users->users = 0;
}

void imagine_atomic_users_read_aquire_emulation(IAtomicUsers *users)
{
	while(1)
	{
		imagine_mutex_lock(users->mutex);
		if(users->users < 0x8000000000000000)
		{
			users->users++;
			imagine_mutex_unlock(users->mutex);
			return;
		}
		imagine_mutex_unlock(users->mutex);
	}
}

void imagine_atomic_users_read_release_emulation(IAtomicUsers *users)
{
	imagine_mutex_lock(users->mutex);
	users->users--;
	imagine_mutex_unlock(users->mutex);
}

void imagine_atomic_users_write_aquire_emulation(IAtomicUsers *users)
{
	imagine_mutex_lock(users->mutex);
	if(users->users == 0)
	{
		users->users = 0x8000000000000000;
		imagine_mutex_unlock(users->mutex);
		return;
	}
	while(1)
	{
		if(users->users < 0x8000000000000000)
		{
			users->users += 0x8000000000000000;
			while(users->users != 0x8000000000000000)
			{
				imagine_mutex_unlock(users->mutex);
				imagine_mutex_lock(users->mutex);
			}
			imagine_mutex_unlock(users->mutex);
			return;
		}
		imagine_mutex_unlock(users->mutex);
		imagine_mutex_lock(users->mutex);
	}
}

void imagine_atomic_users_write_release_emulation(IAtomicUsers *users)
{
	imagine_mutex_lock(users->mutex);
	users->users = 0;
	imagine_mutex_unlock(users->mutex);
}


void imagine_atomic_users_free_emulation(IAtomicInteger *pointer)
{
	imagine_mutex_destory(pointer->mutex);
}

#endif


