#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "forge.h"
#include "imagine.h"
#include "assemble_json.h"
#include "observatory_internal.h"

void observatory_headers_generate_html_link(char *link, ObsItem *file)
{
	char *text;
	uint i, j;
	for(i = 0; file->file[i] != 0 && file->file[i] != '.'; i++)
		link[i] = file->file[i];
	link[i++] = '_'; 

	if(file->name[0] != 0)
		text = file->name;
	else
		text = file->source;

	for(j = 0; text[j] != 0 && j < 32; j++)
	{
		if(text[j] <= 32)
			link[i + j] = '_';
		else
			link[i + j] = text[j];
	}
	i += j;
	link[i] = 0;
}

boolean observatory_headers_is_next(char *text, char *search)
{
	uint i;
	for(i = 0; text[i] == search[i] && search[i] != 0; i++);
	return search[i] == 0 && text[i] <= 32;
}

void observatory_headers_print_text(FILE *f, char *text, ObsItem *items, uint item_count)
{
	uint i, j, k;
	char link[256]; 
	for(i = 0; text[i] != 0; i++)
	{
		for(j = 0; j < item_count; j++)
		{
			if(items[j].name[0] != 0 && observatory_headers_is_next(&text[i], items[j].name))	
			{
				for(k = j; k != 0 && items[k].type != OBSERVATORY_IT_GROUP; k--);
				observatory_headers_generate_html_link(link, &items[k]);
				fprintf(f, "<a href=\"%s#%s\">", link, items[j].name);
				for(; text[i] > 32; i++)
					fputc(text[i], f);
				fprintf(f, "</a>");
			}
		}
		if(observatory_headers_is_next(&text[i], "http://"))	
		{
			fprintf(f, "<a href=\"");
			for(j = 0; text[j + i] > 32; j++)
				fputc(text[j + i], f);
			i += 7;
			fprintf(f, "\">");
			for(j = 0; text[j + i] > 32; j++)
				fputc(text[j + i], f);
			fprintf(f, "</a>");
			i += j;
		}
		if(text[i] > 32)
		{
			for(j = 0; text[i + j] > 32 && text[i + j] != '.'; j++);
			if(text[i + j] == '.' && (text[i + j + 1] == 'c' || text[i + j + 1] == 'h') && text[i + j + 2] <= 32)
			{
				fprintf(f, "<a href=\"");
				for(j = 0; text[j + i] > 32; j++)
					fputc(text[j + i], f);
				fprintf(f, "\">");
				for(j = 0; text[j + i] > 32; j++)
					fputc(text[j + i], f);
				fprintf(f, "</a>");
				i += j;
			}
		}
		if(text[i] == '\n')
			fprintf(f, "<BR>\n");
		else
			fputc(text[i], f);

	}
}

uint observatory_headers_find_in(char *text, char *search)
{
	uint i, j;
	for(i = 0; text[i] != 0; i++)
	{
		for(j = 0; text[i + j] == search[j] && search[j] != 0; j++);
		if(search[j] == 0)
			return i;
	}
	return -1;
}

char *observatory_headers_parce_section(char *text, uint *pos, char *start, char *end)
{
	uint i, j, k;
	for(i = 0; text[i + *pos] == start[i] && start[i] != 0; i++);
	if(start[i] != 0)
		return NULL;
	for(j = 0; text[i + j + *pos] != 0; j++)
	{
		for(k = 0; text[i + j + k + *pos] == end[k] && end[k] != 0; k++);
		if(end[k] == 0)
		{
			text[i + j + *pos] = 0;
			while(text[*pos + i] <= 32)
				i++;
			text = &text[*pos + i];
			*pos = i + j + k + *pos - 2;
			return text;
		}
	}
	return NULL;
}
/*
void observatory_headers_item_print(ObsItem *items, uint item, FILE *html)
{
	fprintf("");
}*/



void observatory_headers_extract_comment(char *text, ObsItem *items, uint *documented, uint *undocumented)
{
	char *p;
	uint i, j;
	items->comment = NULL;
	for(i = 0; text[i] != 0; i++)
	{
		j = i;
		if((p = observatory_headers_parce_section(text, &i, "/*", "*/")) != NULL)
		{
			text[j] = 0;
			items->comment = p;
			(*documented)++;
			return;
		}
		if((p = observatory_headers_parce_section(text, &i, "//", "\n")) != NULL)
		{
			text[j] = 0;
			items->comment = p;
			(*documented)++; 
			return;
		}
	}
	(*undocumented)++;
}


char *observatory_headers_load_file(char *path, uint *size)
{
	char *text, merge_source[1024];
	FILE *f;
	if((f = fopen(path, "r")) == NULL)
	{
		sprintf(merge_source, "../../Mergesource/%s", path);
		f = fopen(merge_source, "r");

		if(f == NULL)
		{
			sprintf(merge_source, "../Mergesource/%s", path);
			f = fopen(merge_source, "r");
		}
	}
	if(f != NULL)
	{
		fseek(f, 0, SEEK_END);
		*size = ftell(f);
		rewind(f);
		text = malloc(*size + 1);
		*size = fread(text, 1, *size, f);
		text[*size] = 0;
		fclose(f);
		return text;
/*
		struct stat st;
		if(stat(path, &st) == 0)
        {

			*size = st.st_size;
			printf("Size = %u %s\n", *size, path);
			text = malloc((sizeof *text) * (*size + 1));
			*size = fread(text, (sizeof *text), *size, f);
			text[*size - 1] = 0;
			fclose(f);
			return text;
		}*/
	}
	printf("Obeservatory ERROR: file %s not found\n", path);
	return NULL;
}

void observatory_headers_menu(FILE *f, ObsItem *items, uint item_count, uint active_module, ObsModule *modules, uint module_count)
{
	static char *intro = NULL;
	static char *midtro = NULL;
	uint i, j, group, size, minutes, hours, month_days, month, year, type;
	char link[1024], link2[1024], *header;
	header = modules[active_module].header;
	fprintf(f, "<dl>\n");
	observatory_html_string_to_link(link, "index");
	fprintf(f, "<dt><a href=\"%s\">Home</a></dt>\n", link);
	observatory_html_string_to_link(link, "work_log");
	fprintf(f, "<dt><a href=\"%s\">Work Log</a></dt>\n", link);
	fprintf(f, "</dl>APIs:<dl>\n");
	for(type = 0; type < 2; type++)
	{
		for(i = 0; i < module_count; i++)
		{
			if((modules[i].header[0] == 0 && type == 1) || (modules[i].header[0] != 0 && type == 0))
			{
				sprintf(link2, "%s_index", modules[i].name);
				observatory_html_string_to_link(link, link2);
				fprintf(f, "<dt><a href=\"%s\" style=\"color:rgb(%u,%u,%u);\">%s</a></dt>", link, (modules[i].color[0] * 2) / 3, (modules[i].color[1] * 2) / 3, (modules[i].color[2] * 2) / 3, modules[i].name);

				if(modules[i].header == header)
				{
					if(i == 4)
						i += 0;
					group = -1;
					for(j = 0; j < item_count; j++)
					{
						if(items[j].file == header)
						{
							if(items[j].type == OBSERVATORY_IT_GROUP)
								group = j;
							if(items[j].type == OBSERVATORY_IT_FUNCTION && group != -1)
							{
								observatory_headers_generate_html_link(link2, &items[group]);
								observatory_html_string_to_link(link, link2);
								if(items[group].name[0] != 0)
									fprintf(f, "<dd><a href=\"%s\">%s</a></dd>", link, items[group].name);
								else
									fprintf(f, "<dd><a href=\"%s\">%s</a></dd>", link, items[group].source);
								group = -1;
							}
						}
					}
					if(modules[i].header[0] != 0)
					{
						observatory_html_string_to_link(link, "defines");
						fprintf(f, "<dd><a href=\"%s\">Defines</a></dd>", link);
					}
				}
			}
		}
		if(type == 0)
		{
			fprintf(f, "</dl>Applications:<dl>\n");
		}
	}
	imagine_current_date_local(imagine_current_system_time_get(), NULL, &minutes, &hours, NULL, &month_days, &month, &year);
	fprintf(f, "<dt>%u/%u/%u %02u:%02u<dt>", year, month + 1, month_days, hours, minutes);
	fprintf(f, "</dl>\n");
}

void observatory_headers_html_footer(FILE *f)
{
	static char *text = NULL;
	uint size;
	if(text == NULL)
		text = observatory_headers_load_file("DocGen_outro.html", &size);
	if(text != NULL)
		fprintf(f, text);
}


uint observatory_headers_parce_file(ObsState *state, uint module_id, ObsItem *items)
{
	char *text, *p, *path;
	uint i, j, k, item_count = 0;
	size_t size;
	FILE *f;

	path = state->modules[module_id].header;
	if(path[0] == 0)
		return 0;
	printf(" Parsing header: %s\n", path);
	if((text = observatory_files_read(state, path, &size)) != NULL)
	{
		for(i = 0; i < size; i++)
		{	
			if((p = observatory_headers_parce_section(text, &i, "#define", "\n")) != NULL)
			{
				items[item_count].file = path;
				items[item_count].type = OBSERVATORY_IT_DEFINE;
				items[item_count].source = p;
				observatory_headers_extract_comment(p, &items[item_count], &state->modules[module_id].documented, &state->modules[module_id].undocumented);
				for(j = 0; p[j] > 32 && j < OBSERVATORY_NAME_SIZE - 1; j++)
					items[item_count].name[j] = p[j];
				items[item_count].name[j] = 0;
				item_count++;
			}
			if((p = observatory_headers_parce_section(text, &i, "typedef struct{", "}")) != NULL)
			{
				items[item_count].file = path;
				items[item_count].type = OBSERVATORY_IT_STRUCT;
				items[item_count].source = p;
				items[item_count].comment = NULL;
				for(j = 0; text[i] != 0 && j < OBSERVATORY_NAME_SIZE - 1 && text[i] != ';'; i++)
					items[item_count].name[j++] = text[i];
				items[item_count].name[j] = 0;
				item_count++;
			}
			if((p = observatory_headers_parce_section(text, &i, "typedef enum{", ";")) != NULL)
			{
				items[item_count].file = path;
				items[item_count].type = OBSERVATORY_IT_ENUM;
				items[item_count].source = p;
				items[item_count].comment = NULL;
				for(j = k = 0; p[j] != '}' && p[j] != 0; j++);
				if(p[j] == '}')
				{
					for(j++; p[j + k] != 0 && k < OBSERVATORY_NAME_SIZE - 1; k++)
						items[item_count].name[k] = p[j + k];
				}
				items[item_count].name[k] = 0;
				item_count++;
			}
			if((p = observatory_headers_parce_section(text, &i, "extern", "\n")) != NULL)
			{
				items[item_count].file = path;
				items[item_count].type = OBSERVATORY_IT_FUNCTION;
				items[item_count].source = p;
				items[item_count].name[0] = 0;
				observatory_headers_extract_comment(p, &items[item_count], &state->modules[module_id].documented, &state->modules[module_id].undocumented);
				for(j = 0; p[j] > 32; j++);
				for(; p[j] <= 32 && p[j] != 0; j++);
				for(; p[j] == '*'; j++);
				for(k = 0; p[j + k] != '(' && p[j + k] != 0 && k < OBSERVATORY_NAME_SIZE - 1; k++)
					items[item_count].name[k] = p[j + k];
				items[item_count].name[k] = 0;
				if(p[j + k] != 0 && k < OBSERVATORY_NAME_SIZE - 1)
					item_count++;
			}
			if((p = observatory_headers_parce_section(text, &i, "/*", "*/")) != NULL || 
				(p = observatory_headers_parce_section(text, &i, "//", "\n")) != NULL)
			{
				items[item_count].file = path;
				items[item_count].type = OBSERVATORY_IT_GROUP;
				items[item_count].name[0] = 0;
				if(p[1] == '-')
				{
					for(j = 2; p[j] == '-'; j++);
					for(; p[j] <= 32 && p[j] != 0; j++);
					if(p[j] != 0)
					{
						for(k = 0; p[j] != '-' && p[j] != 0 && k < OBSERVATORY_NAME_SIZE - 1; j++)
							items[item_count].name[k++] = p[j];
						items[item_count].name[k] = 0;
						for(; (p[j] == '-' || p[j] <= 32) && p[j] != 0; j++);
						p = &p[j];
					}
				}
				items[item_count].source = p;
				items[item_count].comment = NULL;
				item_count++;
			}
		}
	}

	return item_count;

}

void observatory_headers_generate_html_enum_struct(FILE *f, ObsItem *item)
{
	char buffer[4096];
	uint i, j;
	boolean comment = FALSE;
	if(item->type == OBSERVATORY_IT_STRUCT)
		fprintf(f, "<p class=\"header\">Struct:</p> <p class=\"code\">%s<p>\n", item->name);
	else
		fprintf(f, "<p class=\"header\">Enum:</p> <p class=\"code\">%s<p>\n", item->name);

	fprintf(f, "<table border=\"1\">\n");
	comment = TRUE;

	for(i = 0; item->source[i] != 0; i++)
	{
		while(item->source[i] <= 32 && item->source[i] != 0)
			i++;
		if(item->source[i] == '}')
			break;
		if(item->source[i] == '/')
		{
			i++;
			if(item->source[i] == '/')
			{
				i++;
				for(j = 0; j < 4095 && item->source[i + j] != 0 && item->source[i + j] != '\n'; j++)
					buffer[j] = item->source[i + j];
				buffer[j] = 0;
				i += j;
				fprintf(f, "<td valign=top>\n");
				fprintf(f, "<font class=\"header\">Description:</font>\n");
				fprintf(f, "<font class=\"description\">%s</font>\n", buffer);
				fprintf(f, "</td></tr>\n");
				comment = TRUE;
			}
			if(item->source[i] == '*')
			{
				i++;
				for(j = 0; j < 4095 && item->source[i + j] != 0 && item->source[i + j] != '*'; j++)
					buffer[j] = item->source[i + j];
				buffer[j] = 0;
				i += j + 1;
				fprintf(f, "<td valign=top>\n");
				fprintf(f, "<font class=\"header\">Description:</font>\n");
				fprintf(f, "<font class=\"description\">%s</font>\n", buffer);
				fprintf(f, "</td></tr>\n");
				comment = TRUE;
			}
		}else if(item->source[i] != 0)
		{
			for(j = 0; item->source[i + j] != 0 && j < 4095 && item->source[i + j] != ';' && item->source[i + j] != ',' && item->source[i + j] != '/' && item->source[i + j] != '}' && (item->source[i + j] > 32 || item->type == OBSERVATORY_IT_STRUCT); j++)
				buffer[j] = item->source[i + j];
			buffer[j] = 0;
			if(!comment)
				fprintf(f, "</tr>\n");
			fprintf(f, "<tr><td valign=top><p class=\"code\">%s</p></td>\n", buffer);
			i += j;
		}
	}
	fprintf(f, "</table>\n");
}

void observatory_headers_module_index(FILE *f, ObsItem *items, uint count, ObsModule *modules, uint modules_count, uint active_module)
{
	char name[64], buffer[32], *path, link[1024], link2[1024];
	uint i, j, group = -1;
	boolean active = FALSE;
	if(count == 0 || modules[active_module].header[0] == 0)
		return;
	path = modules[active_module].header;
	observatory_html_card_begin(f, NULL);
	observatory_html_headline(f, modules[active_module].name, "Functionality");
	for(i = 0; i < count; i++)
	{
		if(items[i].file == path)
		{
			if(items[i].name[0] != 0 && items[i].type == OBSERVATORY_IT_GROUP)
			{
				if(active)
				{
					observatory_html_bullet_point_end(f);
				}
				for(j = i + 1; j < count && (items[j].name[0] == 0 || (items[j].type != OBSERVATORY_IT_GROUP && items[j].type != OBSERVATORY_IT_FUNCTION)); j++);
				if(j < count && items[j].type == OBSERVATORY_IT_FUNCTION)
				{
					group = i;	
					observatory_html_bullet_point_begin(f, items[i].name);
					active = TRUE;
				}else
					active = FALSE;
			}
			else if(items[i].name[0] != 0 && items[i].type == OBSERVATORY_IT_FUNCTION)
			{
				if(!active)
				{
					observatory_html_bullet_point_begin(f, NULL);
				}
				if(group != -1)
				{
					observatory_headers_generate_html_link(link2, &items[group]);
					observatory_html_string_to_link(link, link2);
					observatory_html_bullet_point(f, items[i].name, link, NULL, 0, 0, 0);
				}else
					observatory_html_bullet_point(f, items[i].name, NULL, NULL, 0, 0, 0);
				active = TRUE;
	
			}
		}
	}
	observatory_html_bullet_point_end(f);
	observatory_html_card_end(f);
}


void observatory_headers_module_description(FILE *f, ObsItem *items, uint count, ObsModule *modules, uint modules_count, uint active_module)
{
	char name[64], buffer[32], *path;
	uint i, j, group = -1;
	boolean active = FALSE;
	path = modules[active_module].header;
	for(i = 0; i < count; i++)
	{
		if(items[i].file == path)
		{
			if(items[i].name[0] != 0 && items[i].type == OBSERVATORY_IT_GROUP)
			{
				observatory_html_card_begin(f, NULL);
				observatory_html_headline(f, items[i].name, NULL);
				observatory_html_text(f, items[i].source);
				observatory_html_card_end(f);
				return;
			}
		}
	}
}




void observatory_headers_generate_html(char *path, ObsItem *items, uint count, ObsModule *modules, uint modules_count)
{
	FILE *f;
	char name[64], buffer[32], file[128] = {0};
	uint i, j, group = -1;
	boolean first;
	for(i = 0; path[i] != 0 && path[i] != '.' && i < 32; i++)
		buffer[i] = path[i];
	buffer[i] = 0;
	sprintf(name, "./docs/%s.html", buffer);
	if((f = fopen(name, "w")) != NULL)
	{
	//	observatory_headers_html_header(f, buffer, items, count, path, modules, modules_count);
		for(i = 0; i < count && (items[i].type != OBSERVATORY_IT_GROUP || items[i].file != path); i++);
		if(i < count)
		{
			fprintf(f, "<p><H2>%s</H2></p>\n", buffer);
			fprintf(f, "<p class=\"group\">\n");
			observatory_headers_print_text(f, items[i].source, items, count);
			fprintf(f, "</p>\n");
			i++;
		}else
			i = 0;
		fprintf(f, "<p class=\"header\">Contents:</p>");
		fprintf(f, "<dl>\n");
		for(; i < count; i++)
		{
			if(items[i].file == path)
			{
				if(items[i].type == OBSERVATORY_IT_GROUP)
					group = i;
				else if(items[i].name[0] != 0 && items[i].type == OBSERVATORY_IT_FUNCTION)
				{
					if(group != -1)
					{
						observatory_headers_generate_html_link(file, &items[group]);
						if(items[group].name[0] != 0)
							fprintf(f, "<dt><a href=\"%s\">%s</a></dt>\n", file, items[group].name);
						else
							fprintf(f, "<dt><a href=\"%s\">%s</a></dt>\n", file, items[group].source);
						group = -1;
					}
					fprintf(f, "<dd><a href=\"%s#%s\">%s</a></dd>\n", file, items[i].name, items[i].name);
				}
			}
		}
		fprintf(f, "<dt><a href=\"%s_defines.html\">Defines</a></dt>\n", buffer);
		fprintf(f, "</dl>\n");
		observatory_headers_html_footer(f);
		fclose(f);
	}
	f = NULL;
	for(i = 0; i < count; i++)
	{
		if(items[i].file == path)
		{
			if(items[i].type == OBSERVATORY_IT_GROUP)
				group = i;
			else if(items[i].name[0] != 0 && items[i].type == OBSERVATORY_IT_FUNCTION)
			{
				if(group != -1)
				{
					observatory_headers_generate_html_link(file, &items[group]);
					sprintf(name, "./docs/%s", file);
					if(f != NULL)
					{
						observatory_headers_html_footer(f);
						fclose(f);
					}
					if((f = fopen(name, "w")) != NULL)
					{
					//	observatory_headers_html_header(f, buffer, items, count, path, modules, modules_count);
						if(items[group].name[0] != 0)
							fprintf(f, "<h1><p>%s</p></h1>\n", items[group].name);
						fprintf(f, "<p class=\"group\">");
						observatory_headers_print_text(f, items[group].source, items, count);
						fprintf(f, "</p>\n");
						group = -1;
					}
				}
				if(f != NULL)
				{
					fprintf(f, "<H3><p id=\"%s\">%s</p></H3>\n", items[i].name, items[i].name);
					fprintf(f, "<p class=\"code\">%s</p>\n", items[i].source);
					if(items[i].comment != NULL)
					{
						fprintf(f, "<font class=\"header\">Description:</font>\n");
						fprintf(f, "<font class=\"description\">%s</font>\n", items[i].comment);
					}
					first = TRUE;
					for(j = 0; j < count; j++)
					{
						if(items[j].file == path && items[j].name[0] != 0 && (items[j].type == OBSERVATORY_IT_ENUM))
						{
							if(-1 != observatory_headers_find_in(items[i].source, items[j].name))
							{
								if(first)
									fprintf(f, "<p>Types:<p>\n");
								observatory_headers_generate_html_enum_struct(f, &items[j]);
								first = FALSE;
							}
						}
					}
				}
			}	
		}
	}
	if(f != NULL)
	{
		observatory_headers_html_footer(f);
		fclose(f);
	}
	sprintf(file, "docs/%s_defines.html", buffer);

	if((f = fopen(file, "w")) != NULL)
	{
	//	observatory_headers_html_header(f, buffer, items, count, path, modules, modules_count);
		for(i = 0; i < count; i++)
		{
			if(items[i].file == path)
			{
				if(items[i].type == OBSERVATORY_IT_GROUP)
					group = i;
				else if(items[i].name[0] != 0 && (items[i].type == OBSERVATORY_IT_DEFINE || items[i].type == OBSERVATORY_IT_STRUCT || items[i].type == OBSERVATORY_IT_ENUM))
				{
					if(group != -1)
					{
						observatory_headers_generate_html_link(file, &items[group]);
						if(items[group].name[0] != 0)
							fprintf(f, "<p class=\"code\">%s</p>\n", items[group].name);
						else
							fprintf(f, "<p class=\"code\">%s</p>\n", items[group].source);
						group = -1;
					}
					if(items[i].type == OBSERVATORY_IT_DEFINE)
					{
						if(items[i].comment == NULL)
						{
							fprintf(f, "<p class=\"code\">%s</p>\n", items[i].source);
						}else
						{
							fprintf(f, "<H3><p id=\"%s\">%s<p></H3>\n", items[i].name, items[i].name);
							fprintf(f, "<p class=\"code\">%s</p>\n", items[i].source);
							fprintf(f, "<font class=\"header\">Description:</font>\n");
							fprintf(f, "<font class=\"description\">%s</font>\n", items[i].comment);
						}
					}else
						observatory_headers_generate_html_enum_struct(f, &items[i]);
				}
			}
		}

		group = -1;
		observatory_headers_html_footer(f);
		fclose(f);
	}
}

/*
extern FILE *observatory_html_create_page( char *file_name, ObsState *state, uint active_module, boolean cards);
extern void observatory_html_complete(FILE *f, boolean cards);
extern void observatory_html_set_accent_color(uint red, uint green, uint blue);
extern void observatory_html_card_begin(FILE *f, char *banner);
extern void observatory_html_card_end(FILE *f);
extern void observatory_html_headline(FILE *f, char *headline_one, char *headline_two);
extern void observatory_html_text(FILE *f, char *text);
extern void observatory_html_bullet_point_begin(FILE *f, char *headline);
extern void observatory_html_bullet_point(FILE *f, char *text, char *url);
extern void observatory_html_bullet_point_end(FILE *f);
extern void observatory_html_key_value(FILE *f, char *key, char *value, char *url);
extern void observatory_html_test(ObsState *state);
*/

void observatory_header_functions(ObsState *state, uint module_id)
{
	FILE *f = NULL;
	char name[64], buffer[32], file[128] = {0};
	uint i, j, group = -1;
	boolean first;

	for(i = 0; i < state->item_count; i++)
	{
		if(state->items[i].file == state->modules[module_id].header)
		{
			if(state->items[i].name[0] != 0 && state->items[i].type == OBSERVATORY_IT_GROUP)
			{
				if(f != NULL)
					observatory_html_complete(f, TRUE);
				f = NULL;
				group = i;

			}
			else if(state->items[i].name[0] != 0 && state->items[i].type == OBSERVATORY_IT_FUNCTION)
			{
				if(f == NULL && group != -1)
				{
					observatory_headers_generate_html_link(file, &state->items[group]);
					f = observatory_html_create_page(file, state, module_id, TRUE);
				}	
				if(f != NULL)
				{
					observatory_html_card_begin(f, NULL);
					observatory_html_headline(f, "Function", state->items[i].name);
					observatory_html_text(f, state->items[i].source);
					if(state->items[i].comment != NULL)
						observatory_html_text(f, state->items[i].comment);
					else
						observatory_html_text(f, "Description missing!");
					observatory_html_card_end(f);
				}

			/*	if(group != -1)
				{
					observatory_headers_generate_html_link(file, &items[group]);
					sprintf(name, "./docs/%s", file);
					if(f != NULL)
					{
						observatory_headers_html_footer(f);
						fclose(f);
					}
					if((f = fopen(name, "w")) != NULL)
					{
					//	observatory_headers_html_header(f, buffer, items, count, path, modules, modules_count);
						if(items[group].name[0] != 0)
							fprintf(f, "<h1><p>%s</p></h1>\n", items[group].name);
						fprintf(f, "<p class=\"group\">");
						observatory_headers_print_text(f, items[group].source, items, count);
						fprintf(f, "</p>\n");
						group = -1;
					}
				}
				if(f != NULL)
				{
					fprintf(f, "<H3><p id=\"%s\">%s</p></H3>\n", items[i].name, items[i].name);
					fprintf(f, "<p class=\"code\">%s</p>\n", items[i].source);
					if(items[i].comment != NULL)
					{
						fprintf(f, "<font class=\"header\">Description:</font>\n");
						fprintf(f, "<font class=\"description\">%s</font>\n", items[i].comment);
					}
					first = TRUE;
					for(j = 0; j < count; j++)
					{
						if(items[j].file == path && items[j].name[0] != 0 && (items[j].type == OBSERVATORY_IT_ENUM))
						{
							if(-1 != observatory_headers_find_in(items[i].source, items[j].name))
							{
								if(first)
									fprintf(f, "<p>Types:<p>\n");
								observatory_headers_generate_html_enum_struct(f, &items[j]);
								first = FALSE;
							}
						}
					}
				}*/
			}	
		}
	}
	if(f != NULL)
		observatory_html_complete(f, TRUE);
}

void observatory_header_defines(ObsState *state, uint module_id)
{
	char file_name[256], file[1024];
	uint i, group = -1;
	FILE *f;
	sprintf(file_name, "%s_defines.html", state->modules[module_id].name);
	f = observatory_html_create_page(file_name, state, module_id, TRUE);
	for(i = 0; i < state->item_count; i++)
	{
		if(state->items[i].file == state->modules[module_id].header)
		{
			if(state->items[i].type == OBSERVATORY_IT_GROUP)
				group = i;
			else if(state->items[i].name[0] != 0 && (state->items[i].type == OBSERVATORY_IT_DEFINE || state->items[i].type == OBSERVATORY_IT_STRUCT || state->items[i].type == OBSERVATORY_IT_ENUM))
			{
				if(group != -1)
				{
					observatory_headers_generate_html_link(file, &state->items[group]);
					if(state->items[group].name[0] != 0)
						fprintf(f, "<p class=\"code\">%s</p>\n", state->items[group].name);
					else
						fprintf(f, "<p class=\"code\">%s</p>\n", state->items[group].source);
					group = -1;
				}
				if(state->items[i].type == OBSERVATORY_IT_DEFINE)
				{
					if(state->items[i].comment == NULL)
					{
						fprintf(f, "<p class=\"code\">%s</p>\n", state->items[i].source);
					}else
					{
						fprintf(f, "<H3><p id=\"%s\">%s<p></H3>\n", state->items[i].name, state->items[i].name);
						fprintf(f, "<p class=\"code\">%s</p>\n", state->items[i].source);
						fprintf(f, "<font class=\"header\">Description:</font>\n");
						fprintf(f, "<font class=\"description\">%s</font>\n", state->items[i].comment);
					}
				}else
					observatory_headers_generate_html_enum_struct(f, &state->items[i]);
			}
		}
	}
	observatory_html_complete(f, TRUE);
}
