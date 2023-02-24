/* -------------- Imagine ------------
Imagine is a platform layer that wraps some basic operating system functionality. It is the smaller sibling of Betray the much larger platform layer. The difference is that Imagine doesnt require a display, drawing or any user input. It contains things like treading, file system traversal, plugin loading and settings management. */


#if defined __cplusplus		/* Declare as C symbols for C++ users. */
extern "C" {
#endif

#ifndef IMAGINE_H
#define IMAGINE_H
#include <stdlib.h>
#include <stdio.h>
#ifndef uint
typedef unsigned int uint;
#endif
#ifndef boolean
typedef unsigned char boolean;
#endif
#ifndef uint64
typedef unsigned long long uint64;
#endif
#ifndef int64
typedef long long int64;
#endif
#ifndef uint32
typedef unsigned int uint32;
#endif
#ifndef uint8
typedef unsigned char uint8;
#endif

//#include "i_atomics_internal.h"

#ifdef _WIN32
#define ILibExport _declspec (dllexport)
#else
#define ILibExport
#endif
/* ---------- Library loading --------
Imagine allows the loading of dynamic libraries and the description of function call interfaces between libraries and host applications. The host starts by creating an interface and then load function pointers in to it. It then loads a library that can access these functions pointers to comunicate with the host application. The same interface can be used for multiple loaded libraries.*/

typedef void IInterface;
typedef void ILib;

extern IInterface 	*imagine_library_interface_create(); /* Create an interface. */
extern void		imagine_library_interface_destroy(IInterface *i);  /* Destroy an interface. */
extern void		imagine_library_interface_register(IInterface *i, void *funtion_pointer, char *name); /* load a function pointer with a name in to identy fy it in to an interface*/
extern int		imagine_library_interface_count(IInterface *i); /* Find ount hown many functions are available in the interface.*/
extern char		*imagine_library_interface_list(IInterface *i, uint number); /* Get the name of a specific function in the interface. */
extern void		*imagine_library_interface_get_by_number(IInterface *i, uint number); /* Get the function pointer belonging to a specific number from an interface. */
extern void		*imagine_library_interface_get_by_name(IInterface *i, char *name); /* Look up a function pointer by name from an interface. Returns NULL is not available */
extern ILib		*imagine_library_load(char *path, IInterface *i, char *interface_name); /* Load a library. */
extern void		imagine_library_unload(ILib *lib, boolean interface_delete); /* Deletes a library. if interface_delete is set top TRUE, the interface used to load the library will als be deleted*/

/*ILibExport IInterface *(*imagine_lib_main)(IInterface *exe_interface); *//* library entry point */

/* ----- Multi Treading ----- 
Imagine has full thread supprot that lets you create threads and thread safe Mutex locks. */



extern void		*imagine_mutex_create(); /* Creates a Mutex. Mutex locks are unlocked when created.*/
extern void		imagine_mutex_lock(void *mutex); /* Lock a mutex. If the lock is already locked, the thread will wait on the lock until the lock is unlocked so that it can lock it */
extern boolean	imagine_mutex_lock_try(void *mutex); /* The thread will atempt to lock the thread, if the lock is already locked if will returne FALSE and fail, If is is not locked, it will lock the mutex and return TRUE */
extern void		imagine_mutex_unlock(void *mutex); /* Un locks the Mutex */
extern void		imagine_mutex_destroy(void *mutex); /* Destroys the mutex */

extern void		*imagine_signal_create(); /* Creates a signal bocker*/
extern void		imagine_signal_destroy(void *signal); /* Destroys a signal blocker */
extern boolean	imagine_signal_wait(void *signal, void *mutex); /*Sets a thread to wait on the blocker for another thread to a activate it.*/
extern boolean	imagine_signal_activate(void *signal); /*Activates the blocker so that one or more threads waiting on the signal will be released */
extern boolean	imagine_signal_activate_all(void *signal); /*Activates the blocker so that all threads waiting on the signal will be released */
extern void		imagine_thread(void (*func)(void *data), void *data, char *name); /* launches a thread that will execute the function pointer. The void pointer will be given as i parameter. Onec the function returns the thread will be deleted.*/

#ifndef IMAGINE_ATOMIC_EMULATION
typedef void * volatile IAtomicPointer; /* An atomic pointer */
typedef unsigned long long volatile IAtomicInteger; /* An atomic 64bit integer */
typedef unsigned long long volatile IAtomicUsers; /* An atomic lock that letss multiple users dread but only one write. */
#endif

#ifdef IMAGINE_INTERNAL

extern void		imagine_atomic_pointer_init(IAtomicPointer pointer, void *value);
extern void		*imagine_atomic_pointer_read(IAtomicPointer pointer);
extern void		imagine_atomic_pointer_write(IAtomicPointer pointer, void *value);
extern boolean	imagine_atomic_pointer_compare_and_exchange(IAtomicPointer pointer, void *expected, void *new_value);
extern void		imagine_atomic_pointer_free(IAtomicPointer pointer);

extern void		imagine_atomic_integer_init(IAtomicInteger pointer, uint64 value);
extern uint64	imagine_atomic_integer_read(IAtomicInteger pointer);
extern void		imagine_atomic_integer_write(IAtomicPointer pointer, uint64 value);
extern boolean	imagine_atomic_integer_compare_and_exchange(IAtomicInteger pointer, uint64 expected, uint64 new_value);
extern void		imagine_atomic_integer_free(IAtomicInteger pointer);

extern void		imagine_atomic_users_init(IAtomicUsers user);
extern void		imagine_atomic_users_read_aquire(IAtomicUsers user);
extern void		imagine_atomic_users_read_release(IAtomicUsers user);
extern void		imagine_atomic_users_write_aquire(IAtomicUsers user);
extern void		imagine_atomic_users_write_release(IAtomicUsers user);
extern boolean	imagine_atomic_users_write_aquire_try(IAtomicUsers user);
extern void		imagine_atomic_users_hold_aquire(IAtomicUsers user);
extern void		imagine_atomic_users_hold_to_write(IAtomicUsers user);
extern void		imagine_atomic_users_hold_release(IAtomicUsers user);
extern void		imagine_atomic_users_free(IAtomicUsers user);

#endif

#ifdef IMAGINE_MUTEX_DEBUG_MODE

#define imagine_mutex_create() imagine_mutex_create_debug() /* Replaces magine_mutex_create. */
#define imagine_mutex_destroy(n) imagine_signal_destroy_debug(n, __FILE__, __LINE__) /* Replaces imagine_signal_destroy. */
#define imagine_mutex_lock(n) imagine_mutex_lock_debug(n, __FILE__, __LINE__) /* Replaces imagine_mutex_lock. */
#define imagine_mutex_lock_try(n) imagine_mutex_lock_try_debug(n, __FILE__, __LINE__) /* Replaces imagine_mutex_lock_try. */
#define imagine_mutex_unlock(n) imagine_mutex_unlock_debug(n, __FILE__, __LINE__) /* Replaces imagine_mutex_unlock. */
#define imagine_signal_wait(n, m) imagine_signal_wait_debug(n, m, __FILE__, __LINE__) /* Replaces imagine_mutex_unlock. */

extern void		*imagine_mutex_create_debug(); /* debug version of magine_mutex_create */
extern void		imagine_signal_destroy_debug(void *mutex, char *file, uint line); /* debug version of magine_mutex_destroy */
extern void		imagine_mutex_lock_debug(void *mutex, char *file, uint line); /* debug version of imagine_mutex_lock */
extern boolean	imagine_mutex_lock_try_debug(void *mutex, char *file, uint line);/* debug version of imagine_mutex_lock_try */
extern void		imagine_mutex_unlock_debug(void *mutex, char *file, uint line);/* debug version of imagine_mutex_unlock */
extern void		imagine_signal_wait_debug(void *signal, void *mutex, char *file, uint line);/* debug version of imagine_mutex_unlock */
extern boolean	imagine_mutex_is_locked_debug(void *mutex); /* check if a lock is locked */

extern void		imagine_mutex_print_debug();/* prints out all currently locked mutexes and where they have been locked. */
#else
#define imagine_mutex_print_debug()
#define imagine_mutex_is_locked_debug() TRUE
#endif

/* ----- Timing ----- 
Imagine has full thread supprot that lets you create threads and thread safe Mutex locks. */

extern void		imagine_current_time_get(uint32 *seconds, uint32 *fractions); /*  get time in seconds and fractions of seconds */
extern double	imagine_delta_time_compute(uint new_seconds, uint new_fractions, uint old_seconds, uint old_fractions); /* computes the ammount of time that has elapsed between two time messurements.*/

extern int64	imagine_current_system_time_get(); /* Returns the current system time. */
extern void		imagine_current_date_local(int64 time, uint *seconds, uint *minutes, uint *hours, uint *week_days, uint *month_days, uint *month, uint *year); /* Converts a system time stamp into the curent local time and date information. */

extern void		imagine_sleepi(uint seconds, uint nano_seconds);
extern void		imagine_sleepd(double time);

/* ------- Execution ------- 
Imagine allows applications to launch other applications. These applications will run concurrent to the launching application*/

extern boolean	imagine_execute(const char *command); /* Execute command on platform */

/* --------- Settings storage -----
Imagines setting system is a convenient system for storing application settings in an XML file. An application can both set and get a settings value. If an application is using any of the "get" functions requesting a non existent setting, then the setting will be automaticaly created using the default value. This makes the system very robust as a deleter file, or even part of a file, can be recreated by the setting sytem system. An application also has the abillity to provide comments to make the settings file easier for humans to read.*/

extern boolean	imagine_setting_boolean_get(const char *setting, boolean default_value, const char *comment); /* Get boolean setting. The function will return default_value if the setting is not available, and create the setting in the setting li*/
extern void		imagine_setting_boolean_set(const char *setting, boolean value, const char *comment); /* set boolean setting */
extern int		imagine_setting_integer_get(const char *setting, int default_value, const char *comment); /* get uint setting */
extern void		imagine_setting_integer_set(const char *setting, int value, const char *comment); /* set uint setting */
extern double	imagine_setting_double_get(const char *setting, double default_value, const char *comment); /* get double setting */
extern void		imagine_setting_double_set(const char *setting, double value, const char *comment); /* set double setting */
extern char		*imagine_setting_text_get(const char *setting, char *default_text, const char *comment); /* get text setting */
extern void		imagine_setting_text_set(const char *setting, char *text, const char *comment); /* set text setting */ 
extern void		imagine_settings_save(const char *file_name); /* saves all settings to a file */
extern boolean	imagine_settings_load(const char *file_name); /* loads all settings from a file */
extern boolean  imagine_setting_test(const char *setting); /* Test if a setting exists */

/* ---- Directory listing --------
Under window this code will create a imaginary root directory containing all volumes. It is therefor possible to list all files in all volumes.*/

#ifdef _WIN32
	#define IMAGINE_DIR_ROOT_PATH "/" /* Defines the root path on Windows. */
	#define IMAGINE_LIBRARY_EXTENTION "dll" /* Defines the name of a library on Windows. */
	#define IMAGINE_DIR_SLASH '\\' /* Defines the slash direction. */
#else
	#define IMAGINE_DIR_ROOT_PATH "/" /* Defines the root path on Unix based platforms. */
	#define IMAGINE_LIBRARY_EXTENTION "lib" /* Defines the name of a library on Windows. */
	#define IMAGINE_DIR_SLASH '/' /* Defines the slash direction. */
#endif
#define IMAGINE_DIR_HOME_PATH "." /* Defines the path to the applications Home directory.*/

extern boolean imagine_path_search(char *file, boolean partial, char *path, boolean folders, uint number, char *out_buffer, uint out_buffer_size); /* Searches for a file recursevly in a path, and writes its location to the out_buffer. If "partial" is set the search will also yeild results wher the seacr string only makes up part of the file name.*/

typedef void IDir;


extern IDir		*imagine_path_dir_open(char *path); /* Opens a path for traversial. If the path is not legal or not a directry the fuction will return NULL. */
extern boolean	imagine_path_dir_next(IDir *d, char *file_name_buffer, uint buffer_size); /* Writes the name of the next member of the directory to file_name_buffer. Returns FALSE if there are no files left in the directory to write out. */
extern void		imagine_path_dir_close(IDir *d); /* Closes a directory. */

extern boolean	imagine_path_is_dir(char *path); /* Returns True if the path is a valid directory. */

extern boolean	imagine_path_file_stats(char *path, size_t *size, uint64 *create_time, uint64 *modify_time); /* Outputs stats about a file. */
extern boolean	imagine_path_volume_stats(char *path, size_t *block_size, size_t *free_size, size_t *used_size, size_t *total_size); /* Outputs information about a volumes, its block size, and how many block are used and free.  */

extern int		imagine_path_rename(char *old_name, char *new_name); /*Re name a path. */
extern int		imagine_path_remove(char *path); /* remove a file. */
extern int		imagine_path_make_dir(char *path); /* Create a directory. */
extern FILE		*imagine_path_open(char *path, char *mode); /* Same as fopen but UTF8. */
extern uint8	*imagine_path_load(char *path, size_t *size); /* Load a file in to a buffer. Writes the size of the buffer to "size" (optional).*/

/* ---- Inter procers Memory Share --------
This code lets one application host a region of memory for another clinet application. The client application can send commands to the host application.*/

typedef void IMemShare;

extern IMemShare	*imagine_memory_share_host_create(size_t size, char *id); /* creates a host. id is tht test string id used to identify the memory. */
extern boolean		imagine_memory_share_host_wait_for_connect(IMemShare *share); /* Once the host has been set up, this function can be called to check if the clint has connected.*/
extern void			imagine_memory_share_host_command(IMemShare *share, char *(*command_func)(IMemShare *share, uint command, void *parameter_buffer, void *return_buffer, void *user), void *user); /* this function lets the host side wait for the client to call a command. The function will lock, and is best used form a thread dedicated to serving the client. */

extern IMemShare	*imagine_memory_share_client_create(size_t *size, char *id); /* creats a client and connects to a host. Will return NULL if there is no hos or if the operation fails. */
extern boolean		imagine_memory_share_client_command(IMemShare *share, uint command, void *parameters, size_t param_size, void *return_buffer, size_t return_size); /* sends a command to the host, copying param_size bytes from parameters, and writing back, return_size bytes to return_buffer. Returns TRUE if sucsesfull of FALSE if it fails. If it fails call imagine_memory_share_alive to see if host is alive*/


extern void			*imagine_memory_share_buffer_get(IMemShare *share, size_t *size); /* returns the shared memory buffer, and its size. */

extern boolean		imagine_memory_share_alive(IMemShare *share); /* returns true if the other side of the client/host connection is still active. */
extern void			imagine_memory_share_destroy(IMemShare *share); /* destroys a shared memory region, and discconects */

/**/

typedef struct{
#ifdef _WIN32
	void *file_handle;
	void *mapping_handle;
#endif
	uint8 *mapping; /* pointer to the mapping. */
	size_t size; /* size of the file and mapping. */
}ImagineFilemapping; 

typedef enum{
	IMAGINE_FMCT_OPEN_READ, /* Open an existing file for reading. */
	IMAGINE_FMCT_OPEN_READ_WRITE, /* Open an existing file for reading and writing. */
	IMAGINE_FMCT_CREATE_READ_WRITE, /* Create anew file open for reading and writing. Will fail if the file already exists. */
	IMAGINE_FMCT_COUNT
}ImagineFileMappingCreateMode; /* parameter for imagine_file_mapping_create */

extern boolean imagine_file_mapping_create(ImagineFilemapping *file, char *file_name, size_t size, ImagineFileMappingCreateMode mode); /* Open/create a file */
extern boolean imagine_file_mapping_resize(ImagineFilemapping *file, size_t size); /* Resize an exisitng handle. */
extern void imagine_file_mapping_destroy(ImagineFilemapping *file); /* Close the file. */


#endif
#if defined __cplusplus
}
#endif