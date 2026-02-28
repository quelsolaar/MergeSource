#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "forge.h"
#include "imagine.h"
#include "assemble_json.h"
#include "observatory_internal.h"

#define OBSERVATORY_RELEASE 1

void observatory_main_state_gen()
{
	AJsonValue *object, *array;
	char *text;
	FILE *f;
	object = assemble_json_value_allcoate(A_JT_OBJECT, 0, "state"); 
	assemble_json_object_member_add_create(object, "name", A_JT_STRING, "Quel Solaar Source Code"); 
	array = assemble_json_object_member_add_create(object, "source", A_JT_ARRAY, NULL); 
	assemble_json_value_array_add_to_create(array, NULL, A_JT_STRING, "c:/Mergesource/");
	array = assemble_json_object_member_add_create(object, "builds", A_JT_ARRAY, NULL); 
	assemble_json_value_array_add_to_create(array, NULL, A_JT_STRING, "c:/Source/");
	text = assemble_json_print_allocate(object);
	assemble_json_free(object);
	f = fopen("observatory_settings.json", "w");
	fprintf(f, "%s", text);
	fclose(f);
	free(text);
}

void observatory_main_templet_gen(char *name)
{
	AJsonValue *object, *array, *executable;
	char *text, buffer[256];
	FILE *f;
	object = assemble_json_value_allcoate(A_JT_OBJECT, 0, "module"); 
	array = assemble_json_object_member_add_create(object, "builds", A_JT_ARRAY, NULL); 
	executable = assemble_json_value_allcoate(A_JT_OBJECT, 0, "build");
	sprintf(buffer, "%s.exe", name);
	assemble_json_object_member_add_create(executable, "file", A_JT_STRING, buffer); 
	assemble_json_object_member_add_create(executable, "platform", A_JT_STRING, "win32"); 
	assemble_json_value_array_add(array, executable, NULL);
	array = assemble_json_object_member_add_create(object, "prefixes", A_JT_ARRAY, NULL); 
	sprintf(buffer, "%s_", name);
	assemble_json_value_array_add_to_create(array, NULL, A_JT_STRING, buffer);
	sprintf(buffer, "%c_", name[0]);
	assemble_json_value_array_add_to_create(array, NULL, A_JT_STRING, buffer);


	array = assemble_json_object_member_add_create(object, "plugins", A_JT_ARRAY, NULL); 
	sprintf(buffer, "%s_plugin_", name);
	assemble_json_value_array_add_to_create(array, NULL, A_JT_STRING, buffer);
	assemble_json_object_member_add_create(object, "plugin_help", A_JT_STRING, "PLUGIN_HELP"); 
	assemble_json_object_member_add_create(object, "plugin_owner", A_JT_STRING, "PLUGIN_OWNER"); 

	array = assemble_json_object_member_add_create(object, "assets", A_JT_ARRAY, NULL); 
	assemble_json_value_array_add_to_create(array, NULL, A_JT_STRING, "some_file.bin");
	assemble_json_object_member_add_create(object, "help", A_JT_STRING, "HELP_STRING"); 
	assemble_json_object_member_add_create(object, "deprecation_define", A_JT_STRING, "DEPRECATED"); 
	assemble_json_object_member_add_create(object, "release", A_JT_STRING, "SOMETHING_RELEASE");
	sprintf(buffer, "%s_todo.txt", name);
	assemble_json_object_member_add_create(object, "todo", A_JT_STRING, buffer); 
	sprintf(buffer, "%s.h", name);
	assemble_json_object_member_add_create(object, "header", A_JT_STRING, buffer); 
	assemble_json_object_member_add_create(object, "owner", A_JT_STRING, "Eskil Steenberg Hald"); 
	sprintf(buffer, "%s", name);
	if(buffer[0] >= 'a' && buffer[0] <= 'z')
		buffer[0] -= 'a' - 'A'; 
	assemble_json_object_member_add_create(object, "name", A_JT_STRING, "something"); 
	assemble_json_object_member_add_create(object, "magicmember", A_JT_STRING, "DocGenProfile"); 
	text = assemble_json_print_allocate(object);
	assemble_json_free(object);
	sprintf(buffer, "%s_observatory_module.json", name);
	f = fopen(buffer, "w");
	fprintf(f, "%s", text);
	fclose(f);
	free(text);
}

void observatory_main_modules_gather(ObsState *state)
{
	AJsonValue *root, *value, *array;
	char *text, *file, *magicmember[2] = {"DocGenProfile", "ObservatoryProfile"};
	ObsModule *m;
	float rgb[3];
	uint i, j, k;
	for(i = 0; i < state->source_file_count; i++)
	{
		if(f_text_filter_case_insensitive(&state->source_files[i].path[state->source_files[i].file_start], ".json"))
		{
			printf(" Parsing: %s", &state->source_files[i].path[state->source_files[i].file_start]);
			file = f_text_load(state->source_files[i].path, NULL);
			if(file != NULL)
			{
				
				root = assemble_json_parse(file, TRUE, A_JT_NUMBER_DECIMAL);
				if(root != NULL)
				{
				
					if(A_JT_OBJECT == assemble_json_value_type_get(root))
					{
						value = assemble_json_object_member_search_name_get_value(root, "magicmember", A_JT_STRING);
						if(value != NULL)
						{
							text = assemble_json_value_string_get(value);
							for(j = 0; j < 2; j++)
							{
								for(k = 0; text[k] != 0 && text[k] == magicmember[j][k]; k++);
								if(text[k] == magicmember[j][k])
									break;
							}
							if(j < 2)
							{
								printf("-----------------------------------------------------------\n%s\n", file);
								if(state->module_count % 16 == 0)
									state->modules = realloc(state->modules, (sizeof *state->modules) * (state->module_count + 16));
								m = &state->modules[state->module_count++];
								m->name[0] = 0;
								m->header[0] = 0;
								m->owner[0] = 0;
								m->todo[0] = 0;
								m->release_define[0] = 0;													
								assemble_json_object_member_search_name_get_string_copy(root, "name", m->name, 64);							
								assemble_json_object_member_search_name_get_string_copy(root, "header", m->header, 64);
								assemble_json_object_member_search_name_get_string_copy(root, "owner", m->owner, 64);
								assemble_json_object_member_search_name_get_string_copy(root, "todo", m->todo, 64);
								assemble_json_object_member_search_name_get_string_copy(root, "release", m->release_define, 64);
								assemble_json_object_member_search_name_get_string_copy(root, "help", m->help_string, 64);
								m->prefixes = NULL;
								m->prefix_count = 0;
								array = assemble_json_object_member_search_name_get_value(root, "prefixes", A_JT_ARRAY);	
								if(array != NULL)
									m->prefixes = assemble_json_value_array_get_allocate(array, A_JT_STRING, &m->prefix_count);
								m->assets = NULL;
								m->asset_count = 0;
								array = assemble_json_object_member_search_name_get_value(root, "assets", A_JT_ARRAY);	
								if(array != NULL)
									m->assets = assemble_json_value_array_get_allocate(array, A_JT_STRING, &m->asset_count);
									
								array = assemble_json_object_member_search_name_get_value(root, "builds", A_JT_ARRAY);
								if(array != NULL)
								{
									m->builds = malloc((sizeof *m->builds) * assemble_json_value_array_get_length(array));
									m->build_count = 0;
									for(value = assemble_json_value_array_first_get(array); value != NULL; value = assemble_json_value_get_next(value))
									{
										if(A_JT_OBJECT == assemble_json_value_type_get(value))
										{
											assemble_json_object_member_search_name_get_string_copy(root, "platform", m->builds[m->build_count].platform, 64);
											if(0 != assemble_json_object_member_search_name_get_string_copy(root, "file", m->builds[m->build_count].build, 64))
												m->build_count++;
										}
									}
								}
								m->documented = 0;
								m->undocumented = 0;
								m->release_array = NULL;
								printf(" Sucsess!\n");
							}else
								printf(" FAILED: Magic member incorrect\n");
						}else
							printf(" FAILED: Magic member not found\n");
					}else
						printf(" FAILED: No object\n");
					assemble_json_free(root); 
				}
				free(file);
			}else
				printf(" FAILED: Cant open file\n");
		}
	}
	if(state->module_count == 0)
		return;
	m = malloc((sizeof *state->modules) * state->module_count);
	state->header_count = 0;
	for(i = 0; i < state->module_count; i++)
	{
		if(state->modules[i].header[0] != 0)
		{
			m[state->header_count++] = state->modules[i];
		}
	}
	j = state->header_count;
	for(i = 0; i < state->module_count; i++)
		if(state->modules[i].header[0] == 0)
			m[j++] = state->modules[i];

	free(state->modules);
	state->modules = m;
	printf("Modules found:\n");
	for(i = 0; i < state->module_count; i++)
	{
		printf(" - %s\n", state->modules[i].name);
		f_hsv_to_rgb(rgb, (float)i / (float)state->module_count, 1.0, 1.0);
		state->modules[i].color[0] = (uint8)(rgb[0] * 255.0);
		state->modules[i].color[1] = (uint8)(rgb[1] * 255.0);
		state->modules[i].color[2] = (uint8)(rgb[2] * 255.0);
	}	
	i = 0;
}

void observatory_main_state_free(ObsState *setting)
{
	uint i;

	if(setting->name != NULL)
		free(setting->name);
	if(setting->builds_paths != NULL)
	{
		for(i = 0; i < setting->builds_path_count; i++)
			free(setting->builds_paths[i]);
		free(setting->builds_paths);
	}
	if(setting->source_paths != NULL)
	{
		for(i = 0; i < setting->source_path_count; i++)
			free(setting->source_paths[i]);
		free(setting->source_paths);
	}

	for(i = 0; i < setting->source_file_count; i++)
		if(setting->source_files[i].file != NULL)
			free(setting->source_files[i].file);
	if(0 != setting->source_file_count)
		free(setting->source_files);
	setting->source_files = NULL;
	setting->source_file_count = 0;

	for(i = 0; i < setting->build_file_count; i++)
		if(setting->build_files[i].file != NULL)
			free(setting->build_files[i].file);
	if(0 != setting->build_file_count)
		free(setting->build_files);
	setting->build_files = NULL;
	setting->build_file_count = 0;
}

void observatory_main_state_read(ObsState *setting)
{
	AJsonValue *root, *value, *member;
	char *text;
	size_t size;

	text = f_text_load("observatory_settings.json", &size);
	if(text == NULL)
	{
		printf("No observatory_settings.json file found, creating one with default settings.\n");
		observatory_main_state_gen();
		text = f_text_load("observatory_settings.json", &size);
	}
	if(text == NULL)
	{
		printf("Observatory Error: failed to read/write observatory_settins.json\n");
		exit(0);
	}
	root = assemble_json_parse(text, TRUE, A_JT_NUMBER_DECIMAL); 
	if(A_JT_OBJECT != assemble_json_value_type_get(root))
	{
		printf("Observatory Error: observatory_settings.json needs to have an object at it root.\n");
		exit(0);
	}
	value = assemble_json_object_member_search_name_get_value(root, "name", A_JT_STRING);
	if(value != NULL)
		setting->name = f_text_copy_allocate(assemble_json_value_string_get(value));
	else
		setting->name = NULL;

	value = assemble_json_object_member_search_name_get_value(root, "source", A_JT_ARRAY);
	if(value != NULL && assemble_json_value_array_get_length(value) != NULL)
	{
		setting->source_paths = malloc((sizeof *setting->source_paths) * assemble_json_value_array_get_length(value)); 
		setting->source_path_count = 0;
		for(member = assemble_json_value_array_first_get(value); member != NULL; member = assemble_json_value_get_next(member))
			if(A_JT_STRING == assemble_json_value_type_get(member))
				setting->source_paths[setting->source_path_count++] = f_text_copy_allocate(assemble_json_value_string_get(member));
	}
	value = assemble_json_object_member_search_name_get_value(root, "builds", A_JT_ARRAY);
	if(value != NULL && assemble_json_value_array_get_length(value) != NULL)
	{
		setting->builds_paths = malloc((sizeof *setting->builds_paths) * assemble_json_value_array_get_length(value)); 
		setting->builds_path_count = 0;
		for(member = assemble_json_value_array_first_get(value); member != NULL; member = assemble_json_value_get_next(member))
			if(A_JT_STRING == assemble_json_value_type_get(member))
				setting->builds_paths[setting->builds_path_count++] = f_text_copy_allocate(assemble_json_value_string_get(member));
	}
}




// observatory_todo_deadline(FILE *f, ObsToDo *todo, uint *date);




void observatory_main_gen_front_page(ObsState *state)
{
	FILE *f;
	f = observatory_html_create_page("index", state, -1, TRUE);
	observatory_todo_gen_deadlines(f, state);
	observatory_release_developer_status(f, state);
	observatory_files_documentation(f, state);
	observatory_release_list(f, state, 10);
	observatory_work_log(f, state, 14);
	observatory_html_complete(f, TRUE);
}

void observatory_main_gen_log_page(ObsState *state)
{
	FILE *f;
	f = observatory_html_create_page("work_log", state, -1, TRUE);
	observatory_work_log(f, state, ~0);
	observatory_html_complete(f, TRUE);
}

void observatory_main_gen_module_front_page(ObsState *state, uint module_id)
{
	char buffer[1024];
	FILE *f;
	sprintf(buffer, "%s_index", state->modules[module_id].name);
	f = observatory_html_create_page(buffer, state, module_id, TRUE);
	observatory_headers_module_description(f, state->items, state->item_count, state->modules, state->module_count, module_id);
	observatory_work_log_module(f, state, module_id);
	observatory_todo_card(f, state, module_id, OBSERVER_TDT_DONE);
	observatory_todo_card(f, state, module_id, OBSERVER_TDT_TODO);
	observatory_files_help(f, state, module_id);
	observatory_files_module_file_list(f, state, module_id);
	observatory_files_dependency_list(f, state, module_id);
	observatory_headers_module_index(f, state->items, state->item_count, state->modules, state->module_count, module_id);
	observatory_release_module_list(f, state, module_id);
	observatory_html_complete(f, TRUE);
}

char *observatory_help_message = "This is where the help message i dont have time to write should go.";

void main(int argc, char **argv)
{
	ObsState state;
	FILE *f;
	ObsItem *items;
	char *executable = "observatory.exe";
	int64 system_time;
	uint i, count = 0;
	boolean release;
	float rgb[3];

	if(argc > 1)
	{
		for(i = 1; i < argc && !f_text_filter_case_insensitive(argv[i], "help"); i++)
		if(i < argc)
		{
			printf("%s\n", observatory_help_message);
		}else
			observatory_main_templet_gen(argv[i]);
		return TRUE;
	}
	if(argc > 1)
		executable = argv[0];
	printf("Start %s with option \"-help\" for additional features\n", executable);
	observatory_html_init();
	state.name = NULL;
	state.builds_paths = NULL;
	state.builds_path_count = 0;
	state.source_paths = NULL;
	state.source_path_count = 0;
	state.source_files = NULL;
	state.source_file_count = 0;
	state.build_files = NULL;
	state.build_file_count = 0;
	state.modules = NULL;
	state.module_count = 0;
	state.items = NULL;
	state.item_count = 0;
	
/*	observatory_main_templet_gen("forge");
	observatory_main_templet_gen("imagine");
	observatory_main_templet_gen("betray");
	observatory_main_templet_gen("relinquish");
	observatory_main_templet_gen("seduce");
	observatory_main_templet_gen("testify");*/
	observatory_main_state_free(&state);
	observatory_main_state_read(&state);
	for(i = 0; i < state.source_path_count; i++)
		state.source_files = observatory_files_gather(state.source_files, &state.source_file_count, state.source_paths[i]);
	for(i = 0; i < state.builds_path_count; i++)
		state.build_files = observatory_files_gather(state.build_files, &state.build_file_count, state.builds_paths[i]);
	observatory_main_modules_gather(&state);
	
	state.items = malloc((sizeof *state.items) * 10240);
	state.item_count = 0;
	for(i = 0; i < state.module_count; i++)
	{
		if(state.modules[i].header[0] != 0)
		{
			state.item_count += observatory_headers_parce_file(&state, i, &state.items[state.item_count]);
		}
	}
	for(i = 0; i < state.module_count; i++)
		observatory_todo_parse(&state, i);


	for(i = 0; i < state.module_count; i++)
		observatory_files_module_make_file(&state, i);
	observatory_release_load(&state);
	release = FALSE;
	system_time = imagine_current_system_time_get(); /* Returns the current system time. */
	for(i = 0; i < state.module_count; i++)
	{
		if(observatory_release_detect(&state, i, system_time))
		{
			observatory_release_save(&state);
			system_time++;
		}
	}
	observatory_work_log_load(&state, system_time);

	observatory_html_set_path("./output/");
	observatory_main_gen_front_page(&state);
	observatory_main_gen_log_page(&state);
	for(i = 0; i < state.module_count; i++)
	{
		observatory_main_gen_module_front_page(&state, i);
		observatory_header_defines(&state, i);
		observatory_header_functions(&state, i);

	}
		


/*	for(i = 0; i < state.module_count; i++)
		observatory_headers_generate_html(state.modules[i].header, state.items, state.item_count, state.modules, state.module_count);
	
	


	if((f = fopen("docs/index.html", "w")) != NULL)
	{
		char *text;
		observatory_headers_html_header(f, state.name, state.items, state.item_count, NULL, state.modules, state.module_count);
		text = observatory_headers_load_file("DocGen_index.html", &i);
		observatory_headers_print_text(f, text, state.items, state.item_count);
	//	fprintf(f, text);
		observatory_headers_html_footer(f);
		fclose(f);
	}*/
	exit(0);
/*	{
		uint *X = NULL;
		X[0] = 0;
	}*/
}