#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "forge.h"
#include "imagine.h"
#include "assemble_json.h"
#include "observatory_internal.h"


size_t observatory_release_extract_string_length(char *file, size_t size)
{
	size_t i = 0, j = 0;
	while(TRUE)
	{
		while(file[i] != '\"' && i < size)
		{
			if(file[i] == '\\')
				i++;
			j++;
			i++;
		}
		while(TRUE)
		{
			i++;
			if(i == size)
				return j;
			if(file[i] > ' ' && file[i] != '\\' )
			{
				if(file[i] == '\"')
				{
					i++;
					break;
				}
				return j + 1;
			}
		}
	}	
}

char *observatory_release_extract_string(char *file, size_t size)
{
	size_t i = 0, j = 0, length;
	char *string;
	length = observatory_release_extract_string_length(file, size);
	string = malloc(length + 1);
	while(TRUE)
	{
		while(file[i] != '\"' && i < size)
		{
			if(file[i] == '\\')
			{
				i++;
				if(file[i] == 'n')
					string[j++] = '\n';
				else if(file[i] == 't')
					string[j++] = '\t';
				else
					string[j++] = file[i];
			}else
				string[j++] = file[i];
			i++;
		}
		while(TRUE)
		{
			i++;
			if(i == size)
			{
				string[j] = 0;
				return string;
			}
			if(file[i] > ' ' && file[i] != '\\' )
			{
				if(file[i] == '\"')
				{
					i++;
					break;
				}
				string[j] = 0;
				return string;
			}
		}
	}	
}

char *observatory_release_extract(char *file, size_t size, char *string)
{
	char first, *output;
	uint i, j;
	first = string[0];
	for(i = 0; i < size; i++)
	{
		if(file[i] == first)	
		{
			for(j = 1; string[j] != 0 && string[j] == file[i + j]; j++);
			if(string[j] == 0)
			{
				for(i += j; file[i] != 0 && (file[i] <= ' ' || file[i] == '='); i++);
				if(file[i] == '\"')
				{
					return observatory_release_extract_string(&file[i + 1], size - (i + 1));
				}else
				{
					for(j = 0; file[i + j] > ' ' && i + j < size; j++);
					output = malloc(j + 1);
					memcpy(output, &file[i], j);
					output[j] = 0;
					return output;
				}
			}
		}
	}
	return NULL;
}


char *observatory_release_extract_module(ObsState *state, uint module_id, char *string)
{
	size_t size;
	uint i, j, k;
	char *file, *output;
	if(state->modules[module_id].header[0] != 0)
	{
		file = observatory_files_read(state, state->modules[module_id].header, &size);
		if(file != NULL)
			if((output = observatory_release_extract(file, size, string)) != NULL)
				return output;
	}
	for(i = 0; i < state->source_file_count; i++)
	{
		if(!f_text_filter_case_insensitive(&state->source_files[i].path[state->source_files[i].file_start], ".json"))
		{
			for(j = 0; j < state->modules[module_id].prefix_count; j++)
			{
				for(k = 0; state->modules[module_id].prefixes[j][k] != 0 && state->modules[module_id].prefixes[j][k] == state->source_files[i].path[state->source_files[i].file_start + k]; k++);
				if(state->modules[module_id].prefixes[j][k] == 0)
				{
					file = state->source_files[i].file;
					if(file == NULL)
						file = state->source_files[i].file = f_text_load(state->source_files[i].path, &state->source_files[i].size);
					if(file != NULL)
						if((output = observatory_release_extract(file, state->source_files[i].size, string)) != NULL)
							return output;
				}
			}	
		}
	}
	return NULL;
}

size_t observatory_release_size(ObsState *state, uint module_id)
{
	size_t *size, sum = 0;
	uint i, j, k;
	char *file, *output;
	if(state->modules[module_id].header[0] != 0)
	{
		file = observatory_files_read(state, state->modules[module_id].header, &size);
		sum = size;
	}
	for(i = 0; i < state->source_file_count; i++)
	{
		for(j = 0; j < state->modules[module_id].prefix_count; j++)
		{
			for(k = 0; state->modules[module_id].prefixes[j][k] != 0 && state->modules[module_id].prefixes[j][k] == state->source_files[i].path[state->source_files[i].file_start + k]; k++);
			if(state->modules[module_id].prefixes[j][k] == 0)
			{
				if(state->source_files[i].file == NULL)
					state->source_files[i].file = f_text_load(state->source_files[i].path, &state->source_files[i].size);
				sum += state->source_files[i].size;
			}
		}	
	}
	return sum;
}

boolean observatory_release_array_string_test(AJsonValue *array, char *string)
{
	AJsonValue *member;
	char *compare;
	uint i;
	for(member = assemble_json_value_array_first_get(array); member != NULL; member = assemble_json_value_get_next(member))
	{
		if(A_JT_STRING == assemble_json_value_type_get(member))
		{
			compare = assemble_json_value_string_get(member);
			for(i = 0; compare[i] != 0 && compare[i] == string[i]; i++);
			if(compare[i] == string[i])
				return TRUE;
		}
	}
	return FALSE;
}

AJsonValue *observatory_release_array_string_release_test(AJsonValue *releases, char *asset, char *string)
{
	AJsonValue *release, *member, *array;
	char *compare;
	uint i;
	for(release = assemble_json_value_array_first_get(releases); release != NULL; release = assemble_json_value_get_next(release))
	{
		if(A_JT_OBJECT == assemble_json_value_type_get(release))
		{
			array = assemble_json_object_member_search_name_get_value(release, asset, A_JT_ARRAY);
			if(array != NULL && A_JT_ARRAY == assemble_json_value_type_get(array))
			{
				for(member = assemble_json_value_array_first_get(array); member != NULL; member = assemble_json_value_get_next(member))
				{
					if(A_JT_STRING == assemble_json_value_type_get(member))
					{
						compare = assemble_json_value_string_get(member);
						for(i = 0; compare[i] != 0 && compare[i] == string[i]; i++);
						if(compare[i] == string[i])
							return release;
					}
				}
			}
		}
	}
	return NULL;
}

extern AJsonValue	*assemble_json_value_array_add(AJsonValue *array, AJsonValue *insert, AJsonValue *after); /* Adds the value "insert" to the array. If "after is set to NULL", the insert will be added to the front of the array. If "after" is not NULL, it has to be a value that is already a member of the array, and the "insert" will be places after the "after" value in the array.*/
extern AJsonValue	*assemble_json_value_array_add_to_create(AJsonValue *array, AJsonValue *after, AJsonType type, void *data); /* Creates a new value and inserts it to a object value under the name "name". The new value will be of type "type". If the value is of type A_JT_NUMBER, the "number" param will be used to set its value. If the value is of type A_JT_STRING, the "string" param will be used to set its value. The function will return a pointer to the new object.*/
extern void			*assemble_json_value_array_get_allocate(AJsonValue *array, AJsonType output_array_type, uint *length); /* Allocates an array of A_JT_BOOLEAN, A_JT_STRING, A_JT_NUMBER_FLOAT, A_JT_NUMBER_DOUBLE, A_JT_NUMBER_DECIMAL */



extern AJsonValue	*assemble_json_value_get_next(AJsonValue *previous);


void observatory_release_load(ObsState *state)
{
	AJsonValue *root = NULL, *module_obj;
	char *text;
	uint i;
	text = f_text_load("observer_release_state.json", NULL);
	if(text != NULL)
	{
		root = assemble_json_parse(text, FALSE, A_JT_NUMBER_DECIMAL); 
		free(text);
	}
	if(root == NULL)
		root = assemble_json_value_allcoate(A_JT_OBJECT, 0, "modules");
	state->release_root = root;
	for(i = 0; i < state->module_count; i++)
	{
		module_obj = assemble_json_object_member_search_name_get_value(root, state->modules[i].name, A_JT_ARRAY);
		if(module_obj == NULL)
		{
			module_obj = assemble_json_object_member_add_create(root, state->modules[i].name, A_JT_ARRAY, NULL);
		}
		state->modules[i].release_array = module_obj;
	}
}

void observatory_release_save(ObsState *state)
{
	FILE *file;
	char *text;
	uint size;
	size = assemble_json_print_size(state->release_root, 0);
	text = malloc(size + 1);
	size = assemble_json_print(text, state->release_root, 0);
	text[size++] = 0;
	file = fopen("observer_release_state.json", "wt");
	fwrite(text, 1, size, file);
	fclose(file);
	file = fopen("observer_release_state_backup.json", "wt");
	fwrite(text, 1, size, file);
	fclose(file);
	free(text);
}


void observatory_release_create(ObsState *state, uint module_id, char *name, int64 system_time)
{
	size_t *size, sum = 0;
	uint i, j, k;
	char *file;
	AJsonValue *release, *file_array, *function_array, *define_array, *done_array, *object;
	char *text = NULL, *header_file;

	release = assemble_json_value_allcoate(A_JT_OBJECT, 0, NULL);
	assemble_json_object_member_add_create(release, "name", A_JT_STRING, name);
	
	file_array = assemble_json_object_member_add_create(release, "files", A_JT_ARRAY, NULL); 
	function_array = assemble_json_object_member_add_create(release, "functions", A_JT_ARRAY, NULL); 
	define_array = assemble_json_object_member_add_create(release, "defines", A_JT_ARRAY, NULL); 
	done_array = assemble_json_object_member_add_create(release, "done", A_JT_ARRAY, NULL); 

	if(state->modules[module_id].header[0] != 0)
	{
		file = observatory_files_read(state, state->modules[module_id].header, &size);
		sum = size;
	}
	for(i = 0; i < state->source_file_count; i++)
	{
		if(oservatory_files_file_in_module(state, module_id, &state->source_files[i]))
		{
			if(state->source_files[i].file == NULL)
				state->source_files[i].file = f_text_load(state->source_files[i].path, &state->source_files[i].size);
			sum += state->source_files[i].size;

			if(NULL == observatory_release_array_string_release_test(state->modules[module_id].release_array, "files", &state->source_files[i].path[state->source_files[i].file_start]))
				assemble_json_value_array_add_to_create(file_array, NULL, A_JT_STRING, &state->source_files[i].path[state->source_files[i].file_start]);
		}	
	}

	for(i = 0; i < state->item_count; i++)
	{
		if(state->items[i].type == OBSERVATORY_IT_FUNCTION && state->items[i].file == state->modules[module_id].header)			
			if(NULL == observatory_release_array_string_release_test(state->modules[module_id].release_array, "functions", state->items[i].name))
				assemble_json_value_array_add_to_create(function_array, NULL, A_JT_STRING, state->items[i].name);
		if(state->items[i].type == OBSERVATORY_IT_DEFINE && state->items[i].file == state->modules[module_id].header)			
			if(NULL == observatory_release_array_string_release_test(state->modules[module_id].release_array, "defines", state->items[i].name))
				assemble_json_value_array_add_to_create(define_array, NULL, A_JT_STRING, state->items[i].name);
	}
	for(i = 0; i < state->modules[module_id].todo_count; i++)
		if(state->modules[module_id].todo_parsed[i].type == OBSERVER_TDT_DONE)			
			if(NULL == observatory_release_array_string_release_test(state->modules[module_id].release_array, "done", state->modules[module_id].todo_parsed[i].string))
				assemble_json_value_array_add_to_create(done_array, NULL, A_JT_STRING, state->modules[module_id].todo_parsed[i].string);

	assemble_json_object_member_add_decimal_create(release, "size", sum, 0);
	assemble_json_object_member_add_decimal_create(release, "time", system_time, 0);
	assemble_json_value_array_add(state->modules[module_id].release_array, release, NULL);
	text = assemble_json_print_allocate(state->modules[module_id].release_array);
	i = 0;
}

boolean observatory_release_generate(FILE *f, ObsState *state, AJsonValue *release, uint module_id, boolean developer)
{
	AJsonDecimalNumber decimal_a, decimal_b;
	AJsonValue *previous, *value, *array;
	char *text, buffer[256], *array_names[4] = {"done", "files", "functions", "defines"}, *headline_names[4] = {"Completed tasks", "Added files", "Added functions", "Added defines"};
	uint i;
	observatory_html_set_accent_color(state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
	text = assemble_json_object_member_search_name_get_string(release, "name");
	for(i = 0; text[i] != 0 && i < 256 - 1; i++)
	{
		if(text[i] == '_')
			buffer[i] = ' ';
		else
			buffer[i] = text[i];
	}
	buffer[i] = 0;
	observatory_html_headline(f, "Release", buffer);
	if(developer && state->modules[module_id].owner[0] != 0)
		observatory_html_key_value(f, "Developer", state->modules[module_id].owner, NULL);

	if(assemble_json_value_get_is_array_member(release))
	{
		previous = assemble_json_value_get_next(release);
		if(previous != NULL)
		{
			value = assemble_json_object_member_search_name_get_value(release, "size", A_JT_NUMBER);
			if(value != NULL)
			{
				assemble_json_value_number_get_decimal(value, &decimal_a);
				value = assemble_json_object_member_search_name_get_value(previous, "size", A_JT_NUMBER);
				if(value != NULL)
				{
					assemble_json_value_number_get_decimal(value, &decimal_b);
					sprintf(buffer, "%lliBytes", decimal_a.integer - decimal_b.integer);
					observatory_html_key_value(f, "Size:", buffer, NULL);
				}
			}
		}
	}
	for(i = 0; i < 4; i++)
	{
		array = assemble_json_object_member_search_name_get_value(release, array_names[i], A_JT_ARRAY);
		if(array != NULL && (value = assemble_json_value_array_first_get(array)) != NULL)
		{
			observatory_html_bullet_point_begin(f, headline_names[i]);
			do {
				if(A_JT_STRING == assemble_json_value_type_get(value))
					observatory_html_bullet_point(f, assemble_json_value_string_get(value), NULL, NULL, 0, 0, 0);
			}while((value = assemble_json_value_get_next(value)) != NULL);
			observatory_html_bullet_point_end(f);	
		}
	}
}

void observatory_release_developer_status(FILE *f, ObsState *state)
{
	AJsonValue *release, *array, *value;
	char buffer[256];
	int64 two_weeks;
	uint i, j, k, release_count, project_count, last_size, new_size, sum, deadlines, done;
	observatory_html_set_accent_color(128, 128, 128);
	observatory_html_card_begin(f, NULL);
	observatory_html_headline(f, "Fortnight status", NULL);

	two_weeks = imagine_current_system_time_get() - 60 * 60 * 24 * 7 * 2; // two weeks ago!
	for(i = 0; i < state->module_count; i++)
	{
		for(j = 0; j < i && !f_text_compare(state->modules[i].owner, state->modules[j].owner); j++);
		if(j == i)
		{
			observatory_html_bullet_point_begin(f, state->modules[i].owner);
			observatory_html_bullet_point_end(f);
			release_count = sum = last_size = project_count = deadlines = done = 0;
			for(j = i; j < state->module_count; j++)
			{
				if(f_text_compare(state->modules[i].owner, state->modules[j].owner))
				{
					project_count++;
					for(release = assemble_json_value_array_first_get(state->modules[j].release_array); release != NULL; release = assemble_json_value_get_next(release))
					{
						new_size = assemble_json_object_member_search_name_get_integer(release, "size", last_size);
						if(last_size != 0 && last_size < new_size)
							sum += new_size - last_size;
						last_size = new_size;
						if(two_weeks < assemble_json_object_member_search_name_get_integer(release, "time", two_weeks + 1))
						{
							release_count++;
						}else
							break;
					}
				}
			}
			sprintf(buffer, "%u", project_count);
			observatory_html_key_value(f, "Projects", buffer, NULL);	
			if(release_count != 0)
			{
				sprintf(buffer, "%u", release_count);
				observatory_html_key_value(f, "Releses", buffer, NULL);			
				sprintf(buffer, "%u bytes", sum);
				observatory_html_key_value(f, "Contribution", buffer, NULL);
			}else
				observatory_html_key_value(f, "Releses", "None!", NULL);
			for(j = i; j < state->module_count; j++)
			{
				if(f_text_compare(state->modules[i].owner, state->modules[j].owner))
				{
					observatory_html_set_accent_color(state->modules[j].color[0], state->modules[j].color[1], state->modules[j].color[2]);
					for(release = assemble_json_value_array_first_get(state->modules[j].release_array); release != NULL; release = assemble_json_value_get_next(release))
					{
						if(two_weeks < assemble_json_object_member_search_name_get_integer(release, "time", two_weeks + 1))
						{
							array = assemble_json_object_member_search_name_get_value(release, "done", A_JT_ARRAY);
							if(array != NULL)
							{
								for(value = assemble_json_value_array_first_get(array); 
									value != NULL;
									value = assemble_json_value_get_next(value))
								{
									if(A_JT_STRING == assemble_json_value_type_get(value))
									{
										if(done == 0)
											observatory_html_bullet_point_begin(f, "Completed tasks");
										done++;
										observatory_html_bullet_point(f, assemble_json_value_string_get(value), NULL, NULL, 0, 0, 0);
									}
								}
							}
						}else
							break;
					}
				}
			}
			if(done != 0)
				observatory_html_bullet_point_end(f);

		}
	}
	observatory_html_card_end(f);
}

boolean observatory_release_list(FILE *f, ObsState *state, uint count)
{
	AJsonDecimalNumber decimal;
	AJsonValue *release, *value, *found;
	int64 min_time = 0, best_time;
	uint i, j, module_id, mod_count = 0;
	observatory_html_card_begin(f, NULL);
	observatory_html_headline(f, "LATEST RELEASES", NULL);

	min_time = 0x7FFFFFFFFFFFFFFF;
	for(j = 0; j < count; j++)
	{
		best_time = 0;
		found = NULL;
		for(i = 0; i < state->module_count; i++)
		{
			if(state->modules[i].release_array != NULL)
			{
				for(release = assemble_json_value_array_first_get(state->modules[i].release_array); release != NULL; release = assemble_json_value_get_next(release))
				{
					mod_count++;
					if(A_JT_OBJECT == assemble_json_value_type_get(release))
					{
						value = assemble_json_object_member_search_name_get_value(release, "time", A_JT_NUMBER);
						if(value != NULL)
						{
							assemble_json_value_number_get_decimal(value, &decimal);/* Sets the AJsonDecimalNumber of the value to "decimal", if of type A_JT_NUMBER, A_JT_NUMBER_FLOAT, A_JT_NUMBER_DOUBLE, or A_JT_NUMBER_DECIMAL. */
							
							
							if(best_time < decimal.integer)
							{
								if(decimal.integer < min_time)
								{
									found = release;
									best_time = decimal.integer;
									module_id = i;
								}
								break;
							}
						}
					}
				}
			}
		}
		if(found == NULL)
			return;
		observatory_release_generate(f, state, found, module_id, TRUE);
		min_time = best_time; 
	}
	observatory_html_card_end(f);
}

void observatory_release_module_list(FILE *f, ObsState *state, uint module_id)
{
	AJsonValue *release, *value;
	if(state->modules[module_id].release_define[0] == 0)
		return;
	observatory_html_card_begin(f, NULL);
	observatory_html_headline(f, "RELEASES", NULL);
	for(release = assemble_json_value_array_first_get(state->modules[module_id].release_array); release != NULL; release = assemble_json_value_get_next(release))
	{
		if(A_JT_OBJECT == assemble_json_value_type_get(release))
			observatory_release_generate(f, state, release, module_id, FALSE);
	}
	observatory_html_card_end(f);
}

boolean observatory_release_detect(ObsState *state, uint module_id, int64 system_time)
{
	AJsonValue *release;
	char *version_name = NULL, *text, release_name[1024];
	uint i;
	if(state->modules[module_id].release_define[0] == 0)
		return;	
	version_name = observatory_release_extract_module(state, module_id, state->modules[module_id].release_define);
	if(version_name == NULL)
		return;
	sprintf(release_name, "%s_%s", state->modules[module_id].name, version_name);
	free(version_name);

	release = assemble_json_value_array_first_get(state->modules[module_id].release_array);
	if(release == NULL)
	{	
		observatory_release_create(state, module_id, release_name, system_time);
 		return TRUE;
	}

	if(version_name != NULL)
	{
		text = assemble_json_object_member_search_name_get_string(release, "name");
		if(text != NULL)
		{
			for(i = 0; text[i] != 0 && text[i] == release_name[i]; i++);
			if(text[i] == release_name[i])
				return FALSE;
		}
		observatory_release_create(state, module_id, release_name, system_time);
		return TRUE;
	}
}