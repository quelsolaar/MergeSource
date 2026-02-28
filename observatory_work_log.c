#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "forge.h"
#include "imagine.h"
#include "assemble_json.h"
#include "observatory_internal.h"

void observatory_work_log_add(AJsonValue *array, char *type, char *change, int64 time_stamp)
{
	AJsonValue *object;
	char *c;
	uint i;
	for(object = assemble_json_value_array_first_get(array); object != NULL; object = assemble_json_value_get_next(object))
	{
		if(A_JT_OBJECT == assemble_json_value_type_get(object))
		{
			c = assemble_json_object_member_search_name_get_string(object, "log");
			if(c != NULL && f_text_compare(c, change))
			{
				c = assemble_json_object_member_search_name_get_string(object, "type");
				if(c != NULL && f_text_compare(c, type))
					return;
			}
		}

	}
	object = assemble_json_value_array_add_to_create(array, NULL, A_JT_OBJECT, NULL); /* Creates a new value and inserts it to a object value under the name "name". The new value will be of type "type". If the value is of type A_JT_NUMBER, the "number" param will be used to set its value. If the value is of type A_JT_STRING, the "string" param will be used to set its value. The function will return a pointer to the new object.*/
	printf("%s\n", change);
	assemble_json_object_member_add_create(object, "log", A_JT_STRING, change); 
	assemble_json_object_member_add_create(object, "type", A_JT_STRING, type);
	assemble_json_object_member_add_decimal_create(object, "time", time_stamp, 0);
}

void observatory_work_log_update(ObsState *state, AJsonValue *array, uint module_id, int64 time_stamp)
{
	char release_name[256], *version_name;
	uint i, type, types[4] = {OBSERVATORY_IT_DEFINE, OBSERVATORY_IT_STRUCT, OBSERVATORY_IT_ENUM, OBSERVATORY_IT_FUNCTION};
	char *type_names[4] = {"define", "struct", "enum", "function"};
	for(i = 0; i < state->modules[module_id].todo_count; i++)
		if(state->modules[module_id].todo_parsed[i].type == OBSERVER_TDT_DONE)
			observatory_work_log_add(array, "done", state->modules[module_id].todo_parsed[i].string, time_stamp);
	for(type = 0; type < 4; type++)
		for(i = 0; i < state->item_count; i++)		
			if(state->items[i].file == state->modules[module_id].header && state->items[i].type == types[type])
				observatory_work_log_add(array, type_names[type], state->items[i].name, time_stamp);
	for(i = 0; i < state->source_file_count; i++)
		if(oservatory_files_file_in_module(state, module_id, &state->source_files[i]))
			observatory_work_log_add(array, "file", &state->source_files[i].path[state->source_files[i].file_start], time_stamp);

	version_name = observatory_release_extract_module(state, module_id, state->modules[module_id].release_define);
	if(version_name != NULL)
	{
		observatory_work_log_add(array, "release", version_name, time_stamp);
		free(version_name);
	}

}


AJsonValue *observatory_work_log_generate_module(ObsState *state, FILE *f, uint module_id, AJsonValue *object, int64 read_time, int64 *next_time, boolean *new_day)
{
	char *enum_names[] = {"start", "done", "release", "define", "struct", "enum", "function", "file"};
	char *c, buffer[1024];
	uint i, type, start_month_days, start_month, start_year, read_month_days, read_month, read_year;
	boolean begin = FALSE;

	imagine_current_date_local(read_time, NULL, NULL, NULL, NULL, &start_month_days, &start_month, &start_year);	
	while(object != NULL)
	{
		if(A_JT_OBJECT == assemble_json_value_type_get(object))
		{
			read_time = assemble_json_object_member_search_name_get_integer(object, "time", 0);
			if(read_time != 0)
			{
				imagine_current_date_local(read_time, NULL, NULL, NULL, NULL, &read_month_days, &read_month, &read_year);
				if(read_month_days == start_month_days && read_month == start_month && read_year == start_year)
				{
					c = assemble_json_object_member_search_name_get_string(object, "log");
					if(c != NULL && assemble_json_object_member_search_name_enum_test(object, "type", enum_names, 8, &type))
					{
					//	printf("%s\n", c);
						if(type == 0)
						{
							object = NULL;
							break;
						}else
						{
							if(!begin)
							{
								if(*new_day)
								{
									observatory_html_set_accent_color(80, 80, 80);

									sprintf(buffer, "%u/%u/%u", start_year, start_month + 1, start_month_days);
									observatory_html_headline(f, "Log, stardate", buffer);
									*new_day = FALSE;
								}
								observatory_html_set_accent_color(state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
								sprintf(buffer, "%s - %s", state->modules[module_id].name, state->modules[module_id].owner);
								observatory_html_bullet_point_begin(f, buffer);
								begin = TRUE;
							}
							switch(type)
							{
								case 1 :
								observatory_html_bullet_point(f, c, NULL, "Done", state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
								break;
								case 2 :
								observatory_html_bullet_point(f, c, NULL, "Release", state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
								break;
								case 3 :
								observatory_html_bullet_point(f, c, NULL, "Define added", state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
								break;
								case 4 :
								observatory_html_bullet_point(f, c, NULL, "Struct added", state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
								break;
								case 5 :
								observatory_html_bullet_point(f, c, NULL, "Enum added", state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
								break;
								case 6 :
								observatory_html_bullet_point(f, c, NULL, "Function added", state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
								break;
								case 7 :
								sprintf(buffer, "./source/%s", c);
								observatory_html_bullet_point(f, c, buffer, "File added", state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
								break;
							}
						}
					}
				}else
				{
					if(read_time > *next_time)
						*next_time = read_time;
					break;
				}
			}
		}
		object = assemble_json_value_get_next(object);		
	}
	if(begin)
		observatory_html_bullet_point_end(f);
	return object;
}

void observatory_work_log_module(FILE *f, ObsState *state, uint module_id)
{
	AJsonValue *object;
	int64 read_time = 0xFFFFFFFFFFFFFFFF, next_time;
	boolean new_day;
	uint i;
	observatory_html_card_begin(f, NULL);
	object = assemble_json_value_array_first_get(state->modules[module_id].log_array);
	read_time = imagine_current_system_time_get() + 24 * 60 * 60;
	while(object != NULL)
	{
		new_day = TRUE;
		next_time = -1;
		object = observatory_work_log_generate_module(state, f, module_id, object, read_time, &next_time, &new_day);
		if(next_time == -1)
			break;
		read_time = next_time;
	}
	observatory_html_card_end(f);
}

void observatory_work_log(FILE *f, ObsState *state, uint days)
{
	AJsonValue **objects;
	int64 read_time = 0x0FFFFFFFFFFFFFFF, next_time;
	boolean new_day;
	uint i, j;
	objects = malloc((sizeof *objects) * state->module_count);
	observatory_html_card_begin(f, NULL);
	for(i = 0; i < state->module_count; i++)
		objects[i] = assemble_json_value_array_first_get(state->modules[i].log_array);
	read_time = imagine_current_system_time_get() + 24 * 60 * 60;
	for(i = j = 0; i <= days && j != state->module_count; i++)
	{
		new_day = TRUE;
		next_time = -1;
		for(j = 0; j < state->module_count; j++)
			if(objects[j] != NULL)
				objects[j] = observatory_work_log_generate_module(state, f, j, objects[j], read_time, &next_time, &new_day);	
		if(next_time == -1)
			break;
		read_time = next_time;
		for(j = 0; j < state->module_count && objects[j] == NULL; j++);
	}
	observatory_html_card_end(f);
	free(objects);
}

void observatory_work_log_load(ObsState *state, int64 time_stamp)
{
	FILE *file;
	AJsonValue *root = NULL, *module_obj;
	char *text;
	uint i;
	text = f_text_load("./observatory_work_logs.json", NULL);
	if(text != NULL)
	{
		printf("%s", text);
		root = assemble_json_parse(text, FALSE, A_JT_NUMBER_DECIMAL); 
		free(text);
	}else
		printf("no work log found!\n");
	if(root == NULL)
		root = assemble_json_value_allcoate(A_JT_OBJECT, 0, "modules");
	state->log_root = root;
	for(i = 0; i < state->module_count; i++)
	{
		module_obj = assemble_json_object_member_search_name_get_value(root, state->modules[i].name, A_JT_ARRAY);
		if(module_obj == NULL)
		{
			module_obj = assemble_json_object_member_add_create(root, state->modules[i].name, A_JT_ARRAY, NULL);
			observatory_work_log_update(state, module_obj, i, time_stamp);
			observatory_work_log_add(module_obj, "start", state->modules[i].name, time_stamp);
		}else
			observatory_work_log_update(state, module_obj, i, time_stamp);
		state->modules[i].log_array = module_obj;
	}
	text = assemble_json_print_allocate(root);
	file = fopen("./observatory_work_logs.json", "wt");
	if(file != NULL)
	{
		fprintf(file, "%s\n", text);
		fclose(file);
	}
	free(text);
}