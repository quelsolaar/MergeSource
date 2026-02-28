#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "forge.h"
#include "imagine.h"
#include "assemble_json.h"
#include "observatory_internal.h"

void observatory_todo_parse_dates(ObsToDo *todo, char *text)
{
	uint64 deadline[3];
	uint dates, i, number_start;
	for(dates = i = 0; dates < 3 && text[i] != 0; i++)
	{
		if(text[i] >= '0' && text[i] < '9')
		{
			if(dates == 0)
				number_start = i;
			i += f_text_parce_decimal(&text[i], &deadline[dates++]);
		}
	}
	if(dates == 3)
	{
		text[number_start] = 0;
		todo->date[0] = (uint)deadline[0];
		todo->date[1] = (uint)deadline[1];
		todo->date[2] = (uint)deadline[2];
	}else
		todo->date[0] = todo->date[1] = todo->date[2] = 0;
}

boolean observatory_todo_parse(ObsState *state, uint module_id)
{
	ObsToDo *todo = NULL;
	size_t i, j, k, size;
	boolean section = FALSE;
	char *text;
	state->modules[module_id].todo_count = 0;
	state->modules[module_id].todo_parsed = NULL;
	if(state->modules[module_id].todo[0] == 0)
		return FALSE;
	text = observatory_files_read(state, state->modules[module_id].todo, &size);
	if(text == NULL)
	{
		printf("To do Error: %s file not found\n", state->modules[module_id].todo);
		return FALSE;
	}else
		printf("To do File for module %s file \n%s\n", state->modules[module_id].name, text);
	size--;
	for(i = 0; i < size; i++)
	{
		while(i < size && text[i] <= ' ')
			i++;
		if(i == size)
			break;
		if((text[i] == '+' || text[i] == '-') && text[i] != text[i + 1])
		{
			if(state->modules[module_id].todo_count % 64 == 0)
				state->modules[module_id].todo_parsed = realloc(state->modules[module_id].todo_parsed, (sizeof *state->modules[module_id].todo_parsed) * (state->modules[module_id].todo_count + 64));
			todo = &state->modules[module_id].todo_parsed[state->modules[module_id].todo_count++];
			todo->string = &text[i + 1];
			if(section && text[i - 1] != '\t')
				section = FALSE;
			for(j = 0; i + 1 + j < size && text[i + 1 + j] != '\n' && text[i + 1 + j] != 0; j++)
				if(text[i + 1 + j] < ' ')
					text[i + 1 + j] = ' ';
			text[i + 1 + j] = 0;
			todo->grouped = section;
			if(text[i] == '+')
				todo->type = OBSERVER_TDT_DONE;
			else
				todo->type = OBSERVER_TDT_TODO;
			observatory_todo_parse_dates(todo, &text[i + 1]);
			i += j;
		}else
		{
			for(j = i; j < size && text[j] != '\n' && text[j] != ':'; j++);
			if(text[j] == ':')
			{
				text[j] = 0;
				if(todo == NULL || todo->type != OBSERVER_TDT_HEADLINE)
				{
					if(state->modules[module_id].todo_count % 64 == 0)
						state->modules[module_id].todo_parsed = realloc(state->modules[module_id].todo_parsed, (sizeof *state->modules[module_id].todo_parsed) * (state->modules[module_id].todo_count + 64));
					todo = &state->modules[module_id].todo_parsed[state->modules[module_id].todo_count++];
				}
				todo->grouped = FALSE;
				todo->string = &text[i];
				todo->type = OBSERVER_TDT_HEADLINE;
				observatory_todo_parse_dates(todo, &text[i + 1]);
				section = TRUE;
				i = j;
			}else
				while(i < size && text[i] != '\n')
					i++;
		}

	}
}


boolean observatory_todo_deadline(FILE *f, ObsToDo *todo, uint *date)
{
	uint i;
	if(todo->date[0] != 0 || todo->date[1] != 0 || todo->date[2] != 0)
	{
		char buffer[64];
		for(i = 0; i < 3; i++)
		{
			if(todo->date[i] > date[i])
			{
				sprintf(buffer, "Deadline : %.4u/%.2u/%.2u\n", todo->date[0], todo->date[1] + 1, todo->date[2]);
				observatory_html_bullet_point(f, todo->string, NULL, buffer, 50, 255, 0);
				break;
			}else if(todo->date[i] < date[i])
			{
				sprintf(buffer, "Deadline Missed! : %.4u/%.2u/%.2u\n", todo->date[0], todo->date[1] + 1, todo->date[2]);
				observatory_html_bullet_point(f, todo->string, NULL, buffer, 255, 0, 50);
				break;
			}
		}
		return TRUE;
	}
	return FALSE;
}

void observatory_todo_bulletpoint(FILE *f, ObsToDo *todo, uint *date)
{
	uint i;
	if(todo->date[0] == 0 && todo->date[1] == 0 && todo->date[2] == 0)
	{
		observatory_html_bullet_point(f, todo->string, NULL, NULL, 0, 0, 0);
	}else
		observatory_todo_deadline(f, todo, date);
}

void observatory_todo_card(FILE *f, ObsState *state, uint module_id, ObsToDoType type)
{
	int64 time;
	uint i, j, list = FALSE, grouped = FALSE, date[3];
	if(state->modules[module_id].todo_count == 0)
		return;
	time = imagine_current_system_time_get();
	imagine_current_date_local(time, NULL, NULL, NULL, NULL, &date[2], &date[1], &date[0]); /* Converts a system time stamp into the curent local time and date information. */

	observatory_html_set_accent_color(state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
	observatory_html_card_begin(f, NULL);
	if(type == OBSERVER_TDT_TODO)
		observatory_html_headline(f, "To Do", state->modules[module_id].name);
	else
		observatory_html_headline(f, "Completed", state->modules[module_id].name);
	observatory_html_bullet_point_begin(f, NULL);

	for(i = 0; i < state->modules[module_id].todo_count; i++)
	{
		if(state->modules[module_id].todo_parsed[i].type == OBSERVER_TDT_HEADLINE)
		{
			for(j = i + 1; j < state->modules[module_id].todo_count && state->modules[module_id].todo_parsed[j].grouped && state->modules[module_id].todo_parsed[j].type != type; j++);
			if(j < state->modules[module_id].todo_count && state->modules[module_id].todo_parsed[j].grouped && state->modules[module_id].todo_parsed[j].type == type)
			{
				if(list)
					observatory_html_bullet_point_end(f);
				observatory_html_bullet_point_begin(f, state->modules[module_id].todo_parsed[i].string);
				grouped = list = TRUE;
			}
		}else if(state->modules[module_id].todo_parsed[i].type == type)
		{
			if(grouped && !state->modules[module_id].todo_parsed[i].grouped)
			{
				observatory_html_bullet_point_end(f);
				grouped = FALSE;
				observatory_html_bullet_point_begin(f, NULL);
			}else if(!list)
				observatory_html_bullet_point_begin(f, NULL);
			list = TRUE;
			observatory_todo_bulletpoint(f, &state->modules[module_id].todo_parsed[i], date);
		}
	}
	observatory_html_bullet_point_end(f);
	observatory_html_card_end(f);
}


void observatory_todo_gen_deadlines(FILE *f, ObsState *state)
{
	uint module_id, i, date[3];
	char buffer[256];
	observatory_html_card_begin(f, NULL);
	
	observatory_html_headline(f, "Deadlines:", NULL);
	imagine_current_date_local(imagine_current_system_time_get(), NULL, NULL, NULL, NULL, &date[2], &date[1], &date[0]); /* Converts a system time stamp into the curent local time and date information. */
	for(module_id = 0; module_id < state->module_count; module_id++)
	{
		for(i = 0; i < state->modules[module_id].todo_count && 
			((state->modules[module_id].todo_parsed[i].date[0] == 0 &&
			state->modules[module_id].todo_parsed[i].date[1] == 0 &&
			state->modules[module_id].todo_parsed[i].date[2] == 0) ||
			state->modules[module_id].todo_parsed[i].type != OBSERVER_TDT_TODO); i++);
		if(i < state->modules[module_id].todo_count)
		{
			observatory_html_set_accent_color(state->modules[module_id].color[0], state->modules[module_id].color[1], state->modules[module_id].color[2]);
			sprintf(buffer, "%s %s", state->modules[module_id].name, state->modules[module_id].owner);
			observatory_html_bullet_point_begin(f, buffer);
			for(; i < state->modules[module_id].todo_count; i++)
				if(state->modules[module_id].todo_parsed[i].type == OBSERVER_TDT_TODO)
					observatory_todo_deadline(f, &state->modules[module_id].todo_parsed[i], date);
			observatory_html_bullet_point_end(f);
		}
	}
	observatory_html_card_end(f);
}

/*
extern void observatory_html_set_path(char *path);
extern void observatory_html_string_to_link(char *copy, char *file_name);
extern FILE *observatory_html_create_page(char *file_name, ObsState *settings, uint active_module, boolean cards);
extern void observatory_html_complete(FILE *f, boolean cards);
extern void observatory_html_set_accent_color(uint red, uint green, uint blue);
extern void observatory_html_card_begin(FILE *f, char *banner);
extern void observatory_html_card_end(FILE *f);
extern void observatory_html_headline(FILE *f, char *headline_one, char *headline_two);
extern void observatory_html_text(FILE *f, char *text);
extern void observatory_html_bullet_point_begin(FILE *f, char *headline);
extern void observatory_html_bullet_point(FILE *f, char *text, char *url, char *hightlight, unsigned char red, unsigned char green, unsigned char blue);
extern void observatory_html_bullet_point_end(FILE *f);
extern void observatory_html_key_value(FILE *f, char *key, char *value, char *url);
extern void observatory_html_test(ObsState *settings);*/


