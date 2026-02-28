#include <math.h>
#include <stdlib.h>
#include "imagine.h"
#include "seduce.h"


typedef enum{
	SEDUCE_FRS_LOAD,
	SEDUCE_FRS_SAVE,	
	SEDUCE_FRS_DIRECTORY,
	SEDUCE_FRS_COUNT
}SeduceFileRequesterStyle;

typedef struct{
	char *name;
	char extention[16];
	float length;
	float focus;
	boolean directory;
	void *children;
	size_t child_count;
	size_t size;
	uint64 create_time;
	uint64 modify_time;
}SeduceFileRequesterFile;

SeduceFileRequesterFile *seduce_file_requester_scan_dir(uint *output_count, char *path, SeduceFileRequesterStyle style)
{
	char path_buffer[IMAGINE_PATH_LENGTH_MAX], *name;
	SeduceFileRequesterFile *file, *f;
	IDir *dir;
	uint i, j, length, allocated, count = 0;
	for(length = 0; path[length] != 0; length++)
	{
		if(length < IMAGINE_PATH_LENGTH_MAX - 1)
		{
			*output_count = 0;
			return NULL;
		}
	}
	memcpy(path_buffer, path, length);
	dir = imagine_path_dir_open(path);
	name = &path_buffer[length];
	allocated = 64;
	file = malloc((sizeof *file) * allocated);
	while(imagine_path_dir_next(dir, name, length - IMAGINE_PATH_LENGTH_MAX))
	{
		if(*name != '.')
		{
			if(count == allocated)
			{
				allocated *= 2;
				file = realloc(file, (sizeof *file) * allocated);
			}
			f = &file[count++];
			for(i = 0; name[i] != 0; i++);
			f->name = malloc(i + 1);
			memcpy(f->name, name, i + 1);
			for(j = i; i != 0 && name[j] != '.'; j--);
			i = 0;
			if(name[j] == '.')
				for(j++; i < 15 && name[j + i] != 0; i++)
					f->extention[i] = name[j + i];
			f->extention[0] = 0;
			f->length = 0;
			f->focus = 0;
			f->directory = imagine_path_is_dir(path);
			f->children = NULL;
			f->child_count = 0;
			imagine_path_file_stats(path, &f->size, &f->create_time, &f->modify_time);
		}
	}
	*output_count = count;
	return file;
}



IDir		*imagine_path_dir_open(char *path); /* Opens a path for traversial. If the path is not legal or not a directry the fuction will return NULL. */
extern boolean	imagine_path_dir_next(IDir *d, char *file_name_buffer, uint buffer_size); /* Writes the name of the next member of the directory to file_name_buffer. Returns FALSE if there are no files left in the directory to write out. */
extern void		imagine_path_dir_close(IDir *d); /* Closes a directory. */

extern boolean	imagine_path_is_dir(char *path); /* Returns True if the path is a valid directory. */

extern boolean	imagine_path_file_stats(char *path, size_t *size, uint64 *create_time, uint64 *modify_time); /* Outputs stats about a file. */
extern boolean	imagine_path_volume_stats(char *path, size_t *block_size, size_t *free_size, size_t *used_size, size_t *total_size); /* Outputs information about a volumes, its block size, and how many block are used and free.  */
