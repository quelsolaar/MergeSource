#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "forge.h"
#include "imagine.h"
#include "assemble_json.h"
#include "observatory_internal.h"

ObsFile *observatory_files_gather_recursivly(ObsFile *files, uint *count, char *file_name_buffer, uint used)
{
	char copy[1024];
	IDir *dir;
	uint i, j, progress, write;
	progress = *count;
	dir = imagine_path_dir_open(file_name_buffer); /* Opens a path for traversial. If the path is not legal or not a directry the fuction will return NULL. */
	while(imagine_path_dir_next(dir, &file_name_buffer[used], 1024 - used))
	{
		if(file_name_buffer[used] != '.')
		{
			if(imagine_path_is_dir(file_name_buffer))
			{
			//	printf("dir: %s\n",  file_name_buffer);
				if(!f_text_filter_case_insensitive(&file_name_buffer[used], "debug") &&
				   !f_text_filter_case_insensitive(&file_name_buffer[used], "deprecated"))
				{
					for(i = used; file_name_buffer[i] != 0; i++);
					file_name_buffer[i] = '/'; 
					file_name_buffer[i + 1] = 0; 
					files = observatory_files_gather_recursivly(files, &progress, file_name_buffer, i + 1);
					file_name_buffer[used] = 0; 
				}
			}else
			{
				if(!f_text_filter_case_insensitive(&file_name_buffer[used], "old"))
				{
					size_t modify_time;
					imagine_path_file_stats(file_name_buffer, NULL, NULL, &modify_time); /* Outputs stats about a file. */
					for(i = 0; i < progress; i++)
					{
						
						for(j = 0; file_name_buffer[used + j] != 0 && file_name_buffer[used + j] == files[i].path[files[i].file_start + j]; j++);
						if(file_name_buffer[used + j] == files[i].path[files[i].file_start + j])
						{
							if(files[i].time < modify_time)
							{
								for(j = 0; file_name_buffer[j] != 0 && j < 1024 - 1; j++)
									files[i].path[j] = file_name_buffer[j];
								files[i].path[j] = 0;
								files[i].file_start = used;
								files[i].time = modify_time;
							}							
					//		printf("duplicate\n");
							break;
						}
					}
					if(i == progress)
					{
						if(progress % 256 == 0)
							files = realloc(files, (sizeof *files) * (progress + 256));
						for(i = 0; file_name_buffer[i] != 0 && i < 1024 - 1; i++)
							files[progress].path[i] = file_name_buffer[i];
						files[progress].path[i] = 0;
						files[progress].file_start = used;
						files[progress].file = NULL;
						files[progress].time = modify_time;
					//	printf("%u %s\n",progress, &files[progress].path[files[progress].file_start]);
						progress++;
					}
				}
			}
		}
	}
	*count = progress;
	return files;
}

ObsFile *observatory_files_gather(ObsFile *file, uint *count, char *path)
{
	char file_name_buffer[1024];
	uint i;
	for(i = 0; i < 1024 - 1 && path[i] != 0; i++)
		file_name_buffer[i] = path[i];
	file_name_buffer[i] = 0;
	return observatory_files_gather_recursivly(file, count, file_name_buffer, i);
}

boolean oservatory_files_file_in_module(ObsState *state, uint module_id, ObsFile *file)
{
	uint i, j;
	for(i = 0; i < state->modules[module_id].prefix_count; i++)
	{
		for(j = 0; state->modules[module_id].prefixes[i][j] != 0 && state->modules[module_id].prefixes[i][j] == file->path[file->file_start + j]; j++);
		if(state->modules[module_id].prefixes[i][j] == 0)
			return TRUE;
	}
	if(state->modules[module_id].header[0] == 0)
		return FALSE;
	for(j = 0; state->modules[module_id].header[j] != 0 && state->modules[module_id].header[j] == file->path[file->file_start + j]; j++);
	if(state->modules[module_id].header[j] == 0)
		return TRUE;
	return FALSE;
}

void observatory_files_module_file_list(FILE *f, ObsState *state, uint module_id)
{
	FILE *file;
	uint i, j, k;
	char buffer[1024];
	if(state->modules[module_id].prefix_count == 0)
		return;
	observatory_html_card_begin(f, NULL);
	observatory_html_headline(f, "File list:", NULL);
	observatory_html_bullet_point_begin(f, NULL);
	for(i = 0; i < state->source_file_count; i++)
	{
		if((state->modules[module_id].header[0] != 0 && f_text_compare(&state->source_files[i].path[state->source_files[i].file_start], state->modules[module_id].header)) ||
			oservatory_files_file_in_module(state, module_id, &state->source_files[i]))
		{
			sprintf(buffer, "./output/source/%s", &state->source_files[i].path[state->source_files[i].file_start]);
		//	file = imagine_path_open(buffer, "wt");
			file = fopen(buffer, "wt");
			if(file != NULL)
			{
				if(state->source_files[i].file == NULL)
					state->source_files[i].file = f_text_load(state->source_files[i].path, &state->source_files[i].size);
				j = fwrite(state->source_files[i].file, 1, state->source_files[i].size, file);
				fclose(file);
			}
			sprintf(buffer, "./source/%s", &state->source_files[i].path[state->source_files[i].file_start]);
			observatory_html_bullet_point(f, &state->source_files[i].path[state->source_files[i].file_start], buffer, NULL, 0, 0, 0);
		}
	}

	observatory_html_bullet_point_end(f);
	observatory_html_card_end(f);
}


void observatory_files_module_make_file_add_files(FILE *file, ObsState *state, uint module_id, char *type, char *ending)
{
	uint i, j, k, type_length;
	char c;
	for(type_length = 0; type[type_length] != 0; type_length++);
	for(i = 0; i < state->source_file_count; i++)
	{
		if(oservatory_files_file_in_module(state, module_id, &state->source_files[i]))
		{
			for(j = state->source_files[i].file_start; state->source_files[i].path[j] != 0; j++);
			if(j > type_length)
			{
				j -= type_length;
				for(k = 0; state->source_files[i].path[j + k] == type[k] && k < type_length; k++);
				if(k == type_length)
				{
					c = state->source_files[i].path[j];
					state->source_files[i].path[j] = 0;
					fprintf(file, "%s%s ", &state->source_files[i].path[state->source_files[i].file_start], ending);
					state->source_files[i].path[j] = c;
				}
			}			
		}
	}
}


boolean observatory_files_dependency_test(ObsState *state, uint module_id, uint dependency_id)
{
	uint j;
	if(dependency_id != module_id && state->modules[dependency_id].header[0] != 0)
	{
		for(j = 0; j < state->source_file_count; j++)
		{
			if(oservatory_files_file_in_module(state, module_id, &state->source_files[j]))
			{
				if(state->source_files[j].file == NULL)
					state->source_files[j].file = f_text_load(state->source_files[j].path, &state->source_files[j].size);
				if(f_text_filter(state->source_files[j].file, state->modules[dependency_id].header))
					return TRUE;
			}
		}
	}
	return FALSE;
}

void observatory_files_dependency_list(FILE *f, ObsState *state, uint module_id)
{
	uint i, j, count = 0;
	if(state->modules[module_id].prefix_count == 0)
		return;
	for(i = 0; i < state->module_count; i++)
	{
		if(observatory_files_dependency_test(state, module_id, i))
		{
			if(count == 0)
			{
				observatory_html_card_begin(f, NULL);
				observatory_html_headline(f, "Dependency list:", NULL);
				observatory_html_bullet_point_begin(f, NULL);
			}
			count++;
			observatory_html_set_accent_color(state->modules[i].color[0], state->modules[i].color[1], state->modules[i].color[2]);
			observatory_html_bullet_point(f, state->modules[i].name, NULL, NULL, 0, 0, 0);
		}
	}
	if(count != 0)
	{
		observatory_html_bullet_point_end(f);
		observatory_html_card_end(f);
		observatory_html_set_accent_color(state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
	}
}

void observatory_files_module_make_file(ObsState *state, uint module_id)
{
	uint i, j, k;
	FILE *file;
	char file_name[256];
	if(state->modules[module_id].header[0] != 0)
		return;
	sprintf(file_name, "%s_makefile.txt", state->modules[module_id].name);
	file = fopen(file_name, "w");
	fprintf(file, "CC=gcc\n");
	fprintf(file, "CFLAGS=-I.\n");
	fprintf(file, "DEPS = ");
	observatory_files_module_make_file_add_files(file, state, module_id, ".h", ".h");
	for(i = 0; i < state->module_count; i++)
		if(observatory_files_dependency_test(state, module_id, i))
			observatory_files_module_make_file_add_files(file, state, i, ".h", ".h");
	fprintf(file, "\nOBJ = ");
	observatory_files_module_make_file_add_files(file, state, module_id, ".c", ".o");
	for(i = 0; i < state->module_count; i++)
		if(observatory_files_dependency_test(state, module_id, i))
			observatory_files_module_make_file_add_files(file, state, i, ".c", ".o");
	fprintf(file, "\n");
	fprintf(file, "%%.o: %%.c $(DEPS)\n");
	fprintf(file, "$(CC) -c -o $@ $< $(CFLAGS)\n\n");
	fprintf(file, "%s: $(OBJ)\n", state->modules[module_id].name);
	fprintf(file, "$(CC) -o $@ $^ $(CFLAGS)\n");
	fclose(file);
}
/*
CC=gcc
CFLAGS=-I.
DEPS = hellomake.h
OBJ = hellomake.o hellofunc.o 

%.o: %.c $(DEPS)
$(CC) -c -o $@ $< $(CFLAGS)

hellomake: $(OBJ)
$(CC) -o $@ $^ $(CFLAGS)

*/

void observatory_files_documentation(FILE *f, ObsState *state)
{
	char buffer[64];
	uint i, percentage;
	float color;
	observatory_html_card_begin(f, NULL);
	observatory_html_headline(f, "Documentation:", NULL);
	observatory_html_bullet_point_begin(f, NULL);
	for(i = 0; i < state->module_count; i++)
	{
		if(state->modules[i].documented != 0 || 
		   state->modules[i].undocumented != 0)
		{
			observatory_html_set_accent_color(state->modules[i].color[0], state->modules[i].color[1], state->modules[i].color[2]);

			color = (float)(state->modules[i].documented) / (float)(state->modules[i].documented + state->modules[i].undocumented);
			percentage = (state->modules[i].documented * 1000) / (state->modules[i].documented + state->modules[i].undocumented);
			sprintf(buffer, "%u.%u%%", percentage / 10, percentage % 10);
			observatory_html_bullet_point(f, state->modules[i].name, NULL, buffer, (1 - color) * 255, color * 255, 0);
		}
	}
	observatory_html_bullet_point_end(f);
	observatory_html_card_end(f);
}


void observatory_files_help(FILE *f, ObsState *state, uint module_id)
{
	char *text;
	if(state->modules[module_id].help_string == NULL)
		return;
	text = observatory_release_extract_module(state, module_id, state->modules[module_id].help_string);
	if(text == NULL)
		return;
	observatory_html_card_begin(f, NULL);
	observatory_html_headline(f, "Help:", NULL);
	observatory_html_text(f, text);
	free(text);
	observatory_html_card_end(f);
}


extern char *observatory_files_read(ObsState *state, char *name, size_t *size)
{
	uint i, j;
	for(i = 0; i < state->source_file_count; i++)
	{	
		for(j = 0; name[j] != 0 && name[j] == state->source_files[i].path[state->source_files[i].file_start + j]; j++);
		if(name[j] == state->source_files[i].path[state->source_files[i].file_start + j])
		{
			if(state->source_files[i].file == NULL)
				state->source_files[i].file = f_text_load(state->source_files[i].path, &state->source_files[i].size);
			if(size != NULL)
				*size = state->source_files[i].size;
			return state->source_files[i].file;			
		}
	}
	return NULL;
}
