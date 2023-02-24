#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 /* Supported bu both GCC and LLVM */

#define	imagine_atomic_integer_init(a, b) __atomic_store_n(&b, __ATOMIC_RELEASE)
#define	imagine_atomic_integer_read(a) __atomic_load_n(&a, __ATOMIC_ACQUIRE)
#define	imagine_atomic_integer_write(a, b) __atomic_store_n(&a, &b, __ATOMIC_RELEASE)
#define	imagine_atomic_integer_compare_and_exchange(a, b, c) __atomic_compare_exchange_n(&a, &b, c, TRUE, __ATOMIC_RELEASE, __ATOMIC_RELAXED)
#define	imagine_atomic_integer_free(a)

#define	imagine_atomic_pointer_init(a, b) __atomic_store_n(&b, __ATOMIC_RELEASE)
#define	imagine_atomic_pointer_read(a) __atomic_load_n(&a, __ATOMIC_ACQUIRE)
#define	imagine_atomic_pointer_write(a, b) __atomic_store_n(&a, &b, __ATOMIC_RELEASE)
#define	imagine_atomic_pointer_compare_and_exchange(a, b, c) __atomic_compare_exchange_n(&a, &b, c, TRUE, __ATOMIC_RELEASE, __ATOMIC_RELAXED)
#define	imagine_atomic_pointer_free(a)

#define imagine_atomic_users_init(a) (a = 0)
#define imagine_atomic_users_read_aquire(a)  {uint64 x; while((x = InterlockedCompareExchange64(a, 0, 0)) >= 0x8000000000000000 || x == InterlockedCompareExchangeAcquire64(a, x + 1, x);}
#define imagine_atomic_users_read_release(a) InterlockedDecrementRelease64(a)
#define imagine_atomic_users_write_aquire(a) {uint64 x; while((x = InterlockedCompareExchange64(a, 0, 0)) >= 0x8000000000000000 || x == InterlockedCompareExchangeAcquire64(a, x + 0x8000000000000000, x); while(InterlockedCompareExchange64(pointer, 0, 0) != 0x8000000000000000);}
#define imagine_atomic_users_write_release(a); InterlockedCompareExchangeRelease64(a, 0x8000000000000000, 0)
#define imagine_atomic_users_write_aquire_try(a); (0 == InterlockedCompareExchangeRelease64(a, 0, 0x8000000000000000))
#define imagine_atomic_users_free(a)

#else
#ifdef _WIN32

#include <winnt.h>

#define IMAGINE_USER_WRITE_USER_WRITE 0x8000000000000000
#define IMAGINE_USER_WRITE_USER_HOLD 0x4000000000000000

#define	imagine_atomic_integer_init(a, b) a = (b)
#define	imagine_atomic_integer_read(a) InterlockedCompareExchange64(&a, 0, 0)
#define	imagine_atomic_integer_write(a, b) InterlockedExchange(&a, b)
#define	imagine_atomic_integer_compare_and_exchange(a, b, c) (b == InterlockedCompareExchange64(&a, b, c))
#define	imagine_atomic_integer_free(a)

#define	imagine_atomic_pointer_init(a, b) a = (b)
#define	imagine_atomic_pointer_read(a) InterlockedCompareExchange64(&a, 0, 0)
#define	imagine_atomic_pointer_write(a, b) InterlockedExchange(&a, b)
#define	imagine_atomic_pointer_compare_and_exchange(a, b, c) (b == InterlockedCompareExchange64(&a, b, c))
#define	imagine_atomic_pointer_free(a)
/*
int imagine_atomic_users_read_to_write(uint64 *x)
{
	uint64 count;
	while(TRUE)
	{
		count = InterlockedCompareExchange64(x, 0, 0);
		if(count >= IMAGINE_USER_WRITE_USER_WRITE)
			return FALSE;
		if(InterlockedCompareExchangeAcquire64(x, count + IMAGINE_USER_WRITE_USER_WRITE - 1, count) == count)
			break;
	}
	while(IMAGINE_USER_WRITE_USER_WRITE != InterlockedCompareExchange64(x, 0, 0));
	return TRUE;
}*/

#define imagine_atomic_users_init(a) (a = 0)
#define imagine_atomic_users_read_aquire(a)  {uint64 x; while((x = InterlockedCompareExchange64(&a, 0, 0)) >= IMAGINE_USER_WRITE_USER_WRITE || x == InterlockedCompareExchangeAcquire64(&a, x + 1, x));}
#define imagine_atomic_users_read_release(a) InterlockedDecrementRelease64(a)
#define imagine_atomic_users_write_aquire(a) {uint64 x; while((x = InterlockedCompareExchange64(&a, 0, 0)) >= IMAGINE_USER_WRITE_USER_HOLD || x == InterlockedCompareExchangeAcquire64(&a, x + IMAGINE_USER_WRITE_USER_WRITE, x)); while(InterlockedCompareExchange64(&a, 0, 0) != IMAGINE_USER_WRITE_USER_WRITE);}
#define imagine_atomic_users_write_release(a) InterlockedCompareExchangeRelease64(&a, IMAGINE_USER_WRITE_USER_WRITE, 0)
#define imagine_atomic_users_write_to_read(a) InterlockedCompareExchangeRelease64(&a, IMAGINE_USER_WRITE_USER_WRITE, 1)
//#define imagine_atomic_users_read_to_write(a) InterlockedCompareExchangeRelease64(&a, IMAGINE_USER_WRITE_USER_WRITE, 1)
#define	imagine_atomic_users_write_aquire_try(a) (InterlockedCompareExchangeAcquire64(&a, IMAGINE_USER_WRITE_USER_WRITE, 0) == 0)
#define	imagine_atomic_users_hold_aquire(a) {uint64 x; while((x = InterlockedCompareExchange64(&a, 0, 0)) >= IMAGINE_USER_WRITE_USER_HOLD || x == InterlockedCompareExchangeAcquire64(&a, x + 1, x));}
#define	imagine_atomic_users_hold_to_write(a)

#define imagine_atomic_users_free(a)

#else

#define IMAGINE_ATOMICS_EMULATION

extern void		imagine_atomic_pointer_init_emulation(IAtomicPointer *pointer, void *value);
extern void		*imagine_atomic_pointer_read_emulation(IAtomicPointer *pointer);
extern void		imagine_atomic_pointer_write_emulation(IAtomicPointer *pointer, void *value);
extern boolean	imagine_atomic_pointer_compare_and_exchange_emulation(IAtomicPointer *pointer, void *expected, void *new_value);
extern void		imgine_atomic_pointer_free_emulation(IAtomicPointer *pointer);

extern void		imgine_atomic_integer_init_emulation(IAtomicInteger *pointer, uint64 value);
extern uint64	imgine_atomic_integer_read_emulation(IAtomicInteger *pointer);
extern boolean	imgine_atomic_integer_compare_and_exchange_emulation(IAtomicInteger *integer, uint64 expected, uint64 new_value);
extern void		imagine_atomic_integer_add_emulation(IAtomicInteger *pointer, uint64 value);
extern void		imagine_atomic_integer_subtract(IAtomicInteger *pointer, uint64 value);
extern void		imgine_atomic_integer_free_emulation(IAtomicInteger *pointer);

#define	imagine_atomic_integer_init(a, b) imagine_atomic_integer_init_emulation(&a, b)
#define	imagine_atomic_integer_read(a) imagine_atomic_integer_read_emulation(&a)
#define	imagine_atomic_integer_write(a, b) imagine_atomic_integer_write_emulation(&a, b)
#define	imagine_atomic_integer_compare_and_exchange(a, b, c) imagine_atomic_integer_compare_and_exchange_emulation(&a, b, c)
#define	imagine_atomic_integer_free(a) imgine_atomic_integer_free_emulation(&a)

#define	imagine_atomic_pointer_init(a, b) imagine_atomic_pointer_init_emulation(&a, b)
#define	imagine_atomic_pointer_read(a) imagine_atomic_pointer_read_emulation(&a)
#define	imagine_atomic_pointer_write(a, b) imagine_atomic_pointer_write_emulation(&a, b)
#define	imagine_atomic_pointer_compare_and_exchange(a, b, c) imagine_atomic_pointer_compare_and_exchange_emulation(&a, b, c)
#define	imagine_atomic_pointer_free(a) imgine_atomic_pointer_free_emulation(&a)

#define imagine_atomic_users_init(a) imagine_atomic_users_init_emulation(a)
#define imagine_atomic_users_read_aquire(a)  imagine_atomic_users_read_aquire_emulation(a)
#define imagine_atomic_users_read_release(a) imagine_atomic_users_read_release_emulation(a)
#define imagine_atomic_users_write_aquire(a) imagine_atomic_users_write_aquire_emulation(a)
#define imagine_atomic_users_write_release(a) imagine_atomic_users_write_release_emulation(a)
#define imagine_atomic_users_free(a) imagine_atomic_users_free_emulation(a)

#endif
#endif
