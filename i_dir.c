#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "forge.h"

#ifdef _WIN32
#define IMAGINE_WINDOWS_PATH_LENGHT_MAX (256 * 128 - 1)
#include <string.h> 
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <wchar.h>

typedef struct{
	char			d_name[IMAGINE_WINDOWS_PATH_LENGHT_MAX];
}idirent;
 
typedef struct{
	WIN32_FIND_DATAW data; 
	HANDLE			handle;
	idirent			ent;
	uint			drive_letter;
}DIR;


DIR *opendir(char *path) 
{ 
	DIR *dir;
	if(path != NULL && path[0] != 0)
	{
		unsigned int pos, i;
		WIN32_FIND_DATAW data; 
		HANDLE handle; 
		wchar_t unicode_path[IMAGINE_WINDOWS_PATH_LENGHT_MAX];
		char *post_fix = "\\*.*";
		for(i = pos = 0; i < IMAGINE_WINDOWS_PATH_LENGHT_MAX - 5 && path[pos] != 0; i++)
			unicode_path[i] = f_utf8_to_uint32(path, &pos);
		for(pos = 0; post_fix[pos] != 0; i++)
			unicode_path[i] = (wchar_t)post_fix[pos++];
		unicode_path[i] = 0;
		if((handle = FindFirstFileW(unicode_path, &data)) != INVALID_HANDLE_VALUE)
		{ 
			dir = malloc(sizeof *dir);
			dir->handle = handle;
			dir->data = data;
			dir->ent.d_name[0] = 0;
			dir->drive_letter = -1;
			return dir;
		}
		return NULL;
	}else
	{
		dir = malloc(sizeof *dir);
		dir->handle = NULL;
		dir->ent.d_name[0] = 64;
		dir->ent.d_name[1] = ':';
		dir->ent.d_name[2] = '/';
		dir->ent.d_name[3] = 0;
		dir->drive_letter = GetLogicalDrives() * 2;
		return dir;
	}
}

void closedir(DIR *dir) 
{
	if(dir->drive_letter == -1)
		FindClose(dir->handle);
	free(dir);
}

idirent *readdir(DIR *dir) 
{
	if(dir->drive_letter == -1)
	{
		uint i, pos;
		if(FindNextFileW(dir->handle, &dir->data) != TRUE)
			return NULL;
		for(i = pos = 0; dir->data.cFileName[i] != 0 && pos < IMAGINE_WINDOWS_PATH_LENGHT_MAX - 6; i++)
			pos += f_uint32_to_utf8(dir->data.cFileName[i], &dir->ent.d_name[pos]);
		dir->ent.d_name[pos] = 0;
		return &dir->ent;
	}else
	{
		uint letter, drive, i;
		drive = dir->drive_letter;
		letter = dir->ent.d_name[0] - 64;
		letter++;
		for(i = 0; i < letter; i++)
			drive /= 2;

		for(i = letter; i < 32; i++)
		{
			if(drive % 2 == 1)
			{
				dir->ent.d_name[0] = 64 + i;
				return &dir->ent;
			}
			drive /= 2;
		}
		return NULL;
	}
}


void *imagine_path_watch(char *path, boolean subfoleders)
{

//	return FindFirstChangeNotificationW(path, subfoleders, FILE_NOTIFY_CHANGE_FILE_NAME);

}

	#define DIR_ROOT_PATH "/"
	#define DIR_HOME_PATH "."
#else

    #include <sys/types.h>
    #include <sys/dir.h>
	#include <dirent.h>
	#include <sys/statvfs.h>

	#define DIR_ROOT_PATH "/"
	#define DIR_HOME_PATH "."

#define idirent struct dirent
#endif

void imagine_drive_test()
{
	DIR *d;
	idirent *ent;
	return;
	d = opendir(DIR_HOME_PATH);
	printf("d = %p -%s-\n", d, DIR_HOME_PATH);
	if(d != NULL)
	{
		ent = readdir(d);
		if(ent != NULL)
		{
			while(TRUE)
			{
				printf("-%s%s-\n", DIR_HOME_PATH, ent->d_name);
				ent = readdir(d);
				if(ent == NULL)
					break;
			}
		}
		closedir(d);
	}
}


boolean imagine_path_search(char *file, boolean partial, char *path, boolean folders, uint number, char *out_buffer, uint out_buffer_size)
{
	uint i, j, found = 0;
	DIR *d;
	idirent *ent;
	d = opendir(path);
	if(d != NULL)
	{
		ent = readdir(d);
		if(ent != NULL)
		{		
			out_buffer_size--;
			while(TRUE)
			{
				if(file == NULL)
				{
					if(found == number)
					{
						for(i = 0; ent->d_name[i] != 0 && i < out_buffer_size; i++)
							out_buffer[i] = ent->d_name[i];
						out_buffer[i] = 0;
						closedir(d);
						return TRUE;
					}
					found++;
				}else
				{
					if(partial)
					{
						for(i = 0; ent->d_name[i] != 0; i++)
						{
							if(ent->d_name[i] == file[0])
							{
								for(j = 0; ent->d_name[i + j] != 0 && file[j] != 0 && (ent->d_name[i + j] == file[j]); j++);
								if(file[j] == 0)
								{
									if(found == number)
									{
										for(i = 0; ent->d_name[i] != 0 && i < out_buffer_size; i++)
											out_buffer[i] = ent->d_name[i];
										out_buffer[i] = 0;
										closedir(d);
										return TRUE;
									}
									found++;
									break;
								}
							}
						}
					}else
					{
						for(i = 0; ent->d_name[i] != 0 && ent->d_name[i] == file[i]; i++);
						if(ent->d_name[i] == file[i])
						{
							if(found == number)
							{
								for(i = 0; ent->d_name[i] != 0 && i < out_buffer_size; i++)
									out_buffer[i] = ent->d_name[i];
								out_buffer[i] = 0;
								closedir(d);
								return TRUE;
							}
							found++;
						}
					}
				}
				ent = readdir(d);
				if(ent == NULL)
					break;
			}
		}
		closedir(d);
	}
	return FALSE;
}


void *imagine_path_dir_open(char *path)
{
	return opendir(path);
}

boolean imagine_path_dir_next(DIR *d, char *file_name_buffer, uint buffer_size)
{
	uint i;
	idirent *ent;
	if(d != NULL)
	{
		ent = readdir(d);
		if(ent != NULL)
		{
			if(buffer_size != 0)
			{
				buffer_size--;
				for(i = 0; ent->d_name[i] != 0 && i < buffer_size; i++)
					file_name_buffer[i] = ent->d_name[i];
				file_name_buffer[i] = 0;
			}
			return TRUE;
		}
	}
	return FALSE;
}

boolean imagine_path_is_dir(char *path)
{
	DIR *d;
	d = opendir(path);
	if(d != NULL)
	{
		closedir(d);
		return TRUE;
	}
	return FALSE;
}

void imagine_path_dir_close(DIR *d)
{
	closedir(d);
}



boolean imagine_path_file_stats(char *file_name, size_t *size, uint64 *create_time, uint64 *modify_time)
{
	struct stat file_stats[80];
	if(!stat(file_name, file_stats))
	{
		if(size != NULL)
			*size = file_stats[0].st_size;
		if(create_time != NULL)
			*create_time = file_stats[0].st_ctime;
		if(modify_time != NULL)
			*modify_time = file_stats[0].st_mtime;
	}
	return TRUE;
}

#ifdef _WIN32

boolean imagine_path_volume_stats(char *path, size_t *block_size, size_t *free_size, size_t *used_size, size_t *total_size)
{
	char drive[] = {'C', ':', '\\', 0};
	DWORD sectors_per_cluster, bytes_per_sector, free_clusters, total_clusters;
	uint64 bs;
	drive[0] = path[0];
	if(GetDiskFreeSpaceA(drive, &sectors_per_cluster, &bytes_per_sector, &free_clusters, &total_clusters))
	{
		bs = (uint64)sectors_per_cluster * (uint64)bytes_per_sector;
		if(block_size != NULL)
			*block_size = sectors_per_cluster;
		if(free_size != NULL)
			*free_size = bs * (uint64)free_clusters;
		if(used_size != NULL)
			*used_size = bs * (uint64)(total_clusters - free_clusters);
		if(total_size != NULL)
			*total_size = bs * (uint64)total_clusters;
/*		printf("%s %u %u %u", drive, (uint)bs, 
							(uint)(bs * (uint64)free_clusters / (uint64)(1024 * 1024)),
							(uint)(bs * (uint64)total_clusters / (uint64)(1024 * 1024)));
*/		return TRUE;
	}
	if(block_size != NULL)
		*block_size = 4096;
	if(free_size != NULL)
		*free_size = 0;
	if(used_size != NULL)
		*used_size = 0;
	if(total_size != NULL)
		*total_size = 0;
	return FALSE;
}
#else

boolean imagine_path_volume_stats(char *path, size_t *block_size, size_t *free_size, size_t *used_size, size_t *total_size)
{
/*	struct statvfs buf;
	if(0 == statvfs(path, &buf))
	{
		*block_size = buf.f_bsize;
		*free_size = buf.f_bavail * buf.f_bsize
		*used_size = buf.f_blocks * buf.f_frsize - buf.f_blocks * buf.f_bsize
		*total_size = buf.f_blocks * buf.f_frsize;
	}*/
}
#endif

void imagine_path_test()
{
	char file[128];
	uint i;
	void *dir;

	dir = imagine_path_dir_open("./");
	printf("dir Poninter %p\n", dir);
	if(dir != NULL)
	{
		for(i = 0; imagine_path_dir_next(dir, file, 128); i++)
			printf("FILE:%s\n", file);
		imagine_path_dir_close(dir);
	}
	exit(0);
}

/*
FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001   
#define FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002   
#define FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004   
#define FILE_NOTIFY_CHANGE_SIZE         0x00000008   
#define FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010   
#define FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020   
#define FILE_NOTIFY_CHANGE_CREATION     0x00000040   
#define FILE_NOTIFY_CHANGE_SECURITY     0x00000100   */

/*
*/

#ifdef _WIN32

#define IMAGINE_WINDOWS_PATH_LENGHT_MAX (256 * 128 - 1)


uint32 f_utf8_to_uint32(char *c, uint *pos); /* converts a string c at position pos to a unt32. pos will be modifyed to jump forward the number of bytes the character takes up. */
uint f_uint32_to_utf8(uint32 character, char *out); /*convets a 32 bit unicode charcter to UTF8. out param needs space for at least 6 8bit characters. returns the number of ytes used. */


char invalid_characters[] = {'>', '<', '\"', '|', '?', '*'};
// CON, PRN, AUX, NUL, COM1, COM2, COM3, COM4, COM5, COM6, COM7, COM8, COM9, LPT1, LPT2, LPT3, LPT4, LPT5, LPT6, LPT7, LPT8, and LPT9

int imagine_path_rename(char *old_name, char *new_name)
{
	wchar_t wide_oldname[IMAGINE_WINDOWS_PATH_LENGHT_MAX];
	wchar_t wide_newname[IMAGINE_WINDOWS_PATH_LENGHT_MAX];
	uint pos, i;
	for(i = pos = 0; i < IMAGINE_WINDOWS_PATH_LENGHT_MAX - 1 && old_name[pos] != 0; i++)
		wide_oldname[i] = f_utf8_to_uint32(old_name, &pos);
	wide_oldname[i] = 0;
	for(i = pos = 0; i < IMAGINE_WINDOWS_PATH_LENGHT_MAX - 1 && new_name[pos] != 0; i++)
		wide_newname[i] = f_utf8_to_uint32(new_name, &pos);
	wide_newname[i] = 0;
	return _wrename(wide_oldname, wide_newname);
}

int imagine_path_remove(char *path)
{
	wchar_t wide_name[IMAGINE_WINDOWS_PATH_LENGHT_MAX];
	uint pos = 0, i;
	for(i = pos = 0; i < IMAGINE_WINDOWS_PATH_LENGHT_MAX - 1 && path[pos] != 0; i++)
		wide_name[i] = f_utf8_to_uint32(path, &pos);
	wide_name[i] = 0;
	return _wremove(wide_name);
}

int imagine_path_make_dir(char *path)
{
	wchar_t wide_name[IMAGINE_WINDOWS_PATH_LENGHT_MAX];
	uint pos = 0, i;
	for(i = pos = 0; i < IMAGINE_WINDOWS_PATH_LENGHT_MAX - 1 && path[pos] != 0; i++)
		wide_name[i] = f_utf8_to_uint32(path, &pos);
	wide_name[i] = 0;
	return _wmkdir(wide_name);
}

FILE *imagine_path_open(char *path, char *mode)
{
	FILE *f = NULL;
	uint i;
	char encoding_mode[32], *p, *encoding = ", ccs=UTF-8";
	p = encoding_mode;
	for(i = 0; i < 3 && mode[i] > ' '; i++)
		*p++ = mode[i];
	for(i = 0; encoding[i] != 0; i++)
		*p++ = encoding[i];
	*p++ = 0;
	if(0 == fopen_s(&f, path, encoding_mode))
		return f;
	return NULL;
}


uint8 *imagine_path_load(char *path, size_t *size)
{
	char *buffer;
	uint allocation, i;
	FILE *f;
	f = fopen(path, "rb");
	if(f == NULL)
	{
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	allocation = ftell(f);
	if(allocation == 0)
	{
		fclose(f);
		return NULL;
	}
	rewind(f);
	buffer = malloc(allocation + 1);
	memset(buffer, 0, allocation + 1);
	fread(buffer, 1, allocation, f);
	fclose(f);
	buffer[allocation] = 0;
	if(size != NULL)
		*size = allocation;
	return buffer;
}
#else

int imagine_path_rename(char *old_name, char *new_name)
{
	return rename(old_name, new_name);
}

int imagine_path_remove(char *path)
{
	return remove(path);
}

int imagine_path_make_dir(char *path)
{
	return mkdir(path, S_IRWXU);
}

FILE *imagine_path_open(char *path, char *mode)
{
	return fopen(path, mode);
}

#endif