#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "forge.h"
#include "imagine.h"
#include "assemble_json.h"
#include "observatory_internal.h"

char *observatory_html_header;
char *observatory_html_path = "";

void observatory_html_init()
{
	observatory_html_header = f_text_load("./observatory_header.html", NULL);
}


void observatory_html_set_path(char *path)
{
	observatory_html_path = path;
}

extern void observatory_headers_menu(FILE *f, ObsItem *items, uint item_count, uint active_module, ObsModule *modules, uint module_count);

void observatory_html_string_to_link(char *copy, char *file_name)
{
	uint i, j;
	for(i = j = 0; j < 1023 - 5 && file_name[i] != 0; i++)
	{
		if(file_name[i] >= 'A' && file_name[i] <= 'Z')
			copy[j++] = file_name[i] - 'A' + 'a';
		else if(file_name[i] >= 'a' && file_name[i] <= 'z')
			copy[j++] = file_name[i];
		else if(file_name[i] <= ' ' || file_name[i] == '.' || file_name[i] == ',' || file_name[i] == '_' || file_name[i] == '-')
			copy[j++] = '_';
		else
			i += 0;
	}
	while(j > 0 && copy[j - 1] == '_')
		j--;
	copy[j++] = '.';
	copy[j++] = 'h';
	copy[j++] = 't';
	copy[j++] = 'm';
	copy[j++] = 'l';
	copy[j++] = 0;
}


FILE *observatory_html_create_page(char *file_name, ObsState *state, uint active_module, boolean cards)
{
	char copy[1024];
	uint i, j;
	FILE *f;
	for(j = 0; j < 1023 && observatory_html_path[j] != 0; j++)
		copy[j] = observatory_html_path[j];
	observatory_html_string_to_link(&copy[j], file_name);
	f = fopen(copy, "w");
	fprintf(f, "%s\n", observatory_html_header);
	observatory_headers_menu(f, state->items, state->item_count, active_module, state->modules, state->module_count);
	if(active_module < state->module_count)
		observatory_html_set_accent_color(state->modules[active_module].color[0], state->modules[active_module].color[1], state->modules[active_module].color[2]);
	fprintf(f, "</div>\n");
	if(cards)
		fprintf(f, "<article>\n");	
	return f;
}

void observatory_html_complete(FILE *f, boolean cards)
{
	if(cards)
		fprintf(f, "</article>\n");	
	fprintf(f, "</body>\n</html>\n");
	fclose(f);
}

uint observatory_html_card_color[3];

void observatory_html_set_accent_color(uint red, uint green, uint blue)
{
	observatory_html_card_color[0] = red;
	observatory_html_card_color[1] = green;
	observatory_html_card_color[2] = blue;
}


void observatory_html_card_begin(FILE *f, char *banner)
{  
	fprintf(f, "<div class=\"card\" style=\"outline-color: rgb(%u,%u,%u);\">", observatory_html_card_color[0], observatory_html_card_color[1], observatory_html_card_color[2]);
	if(banner != NULL)
		fprintf(f, "<img src=\"%s\" width=\"100%%\">", banner);
	fprintf(f, "<div class=\"cardbox\">");
}

void observatory_html_card_end(FILE *f)
{
	fprintf(f, "</div></div>\n");
}

void observatory_html_headline(FILE *f, char *headline_one, char *headline_two)
{
	if(headline_two == NULL)
		fprintf(f, "%s<div class=\"line\"></div>\n", headline_one);
	else
		fprintf(f, "<span style=\"color: rgb(%u,%u,%u);\">%s:</span>%s<div class=\"line\"></div>\n", observatory_html_card_color[0], observatory_html_card_color[1], observatory_html_card_color[2], headline_one, headline_two);
}

void observatory_html_text(FILE *f, char *text)
{
	char buffer[256];
	uint i, j;
	fprintf(f, "<p>\n");
	for(i = 0; text[i] != 0;)
	{
		if(text[i] != '\n')
		{
			for(j = 0; text[i + j] != 0 && text[i + j] != '\n' && j < 256 - 1; j++)
				buffer[j] = text[i + j];
			buffer[j] = 0;
			fprintf(f, "%s", buffer);
			i += j;
		}else
		{
			fprintf(f, "\n</p><p>\n");			
			for(i++; text[i + j] == '\n'; i++);
		}			
	}
	fprintf(f, "</p>\n");
}

uint observatory_html_bullet_point_id = 0;
uint observatory_html_bullet_point_count = 0;

void observatory_html_bullet_point_begin(FILE *f, char *headline)
{
	observatory_html_bullet_point_id = (observatory_html_bullet_point_id + 1) % 1000000;
	observatory_html_bullet_point_count = 0;
	if(headline != NULL)
		fprintf(f, "<p style=\"color: rgb(%u,%u,%u);\"><span style=\"color: gray;\">%s:</span></p><ul>\n", observatory_html_card_color[0], observatory_html_card_color[1], observatory_html_card_color[2], headline);
	else
		fprintf(f, "<ul>\n");
}


void observatory_html_bullet_point(FILE *f, char *text, char *url, char *hightlight, unsigned char red, unsigned char green, unsigned char blue)
{
	char buffer[256];
	uint i, j;
	fprintf(f, "<li style=\"color: rgb(%u,%u,%u);\"><span style=\"color: black;\">\n", observatory_html_card_color[0], observatory_html_card_color[1], observatory_html_card_color[2]);
	if(url != NULL)
	{
		if(f_text_filter_case_insensitive(url, ".htm"))
			fprintf(f, "<a href=\"%s\" style=\"color: rgb(%u,%u,%u);\" >", url, (observatory_html_card_color[0] * 2) / 3, (observatory_html_card_color[1] * 2) / 3, (observatory_html_card_color[2] * 2) / 3);
		else
			fprintf(f, "<a href=\"%s\" style=\"color: rgb(%u,%u,%u);\" download>", url, (observatory_html_card_color[0] * 2) / 3, (observatory_html_card_color[1] * 2) / 3, (observatory_html_card_color[2] * 2) / 3);
	}
/*	for(i = 0; text[i] != 0;)
	{
		for(j = 0; text[i] != 0 && j < 256 - 4; i++)
		{
			if(text[i] == '_')
			{
				buffer[j++] = ' ';
				buffer[j++] = '_';
				buffer[j++] = ' ';
			}else
				buffer[j++] = text[i];
		}
		buffer[j] = 0;
		fprintf(f, "%s", buffer);
	}*/
	fprintf(f, "%s\n", text);
	if(url != NULL)
		fprintf(f, "</a>");
	if(hightlight != NULL)
		fprintf(f, "</span><span style=\"color: rgb(%u,%u,%u);\">%s\n", red, green, blue, hightlight);
	fprintf(f, "</span></li>\n");
	if(observatory_html_bullet_point_count == 10)
		fprintf(f, "<div id=\"list%u\" style=\"display: none;\">", observatory_html_bullet_point_id);
	observatory_html_bullet_point_count++;
}
void observatory_html_bullet_point_end(FILE *f)
{
	if(observatory_html_bullet_point_count > 10)
	{
		fprintf(f, "</div>");
		fprintf(f, "<a id=\"show_more%u\" onclick=\"turn_on(\'%u\')\">%s</a>", observatory_html_bullet_point_id, observatory_html_bullet_point_id, "...<br>Show more");
		fprintf(f, "<a id=\"show_less%u\" onclick=\"turn_off(\'%u\')\" style=\"display: none;\">%s</a>", observatory_html_bullet_point_id, observatory_html_bullet_point_id, "Show less");
	}
	fprintf(f, "</ul>\n");
}

void observatory_html_key_value(FILE *f, char *key, char *value, char *url)
{
	fprintf(f, "<p><span style=\"color: gray;\">%s:</span>", key);
	if(url != NULL)
		fprintf(f, "<a href=\"%s\" style=\"color: rgb(%u,%u,%u);\">", url, (observatory_html_card_color[0] * 2) / 3, (observatory_html_card_color[1] * 2) / 3, (observatory_html_card_color[2] * 2) / 3);
	fprintf(f, "<span style=\"color: black;\">%s</span>\n", value);
	if(url != NULL)
		fprintf(f, "</a>");
	fprintf(f, "</p>");
}

void observatory_html_test(ObsState *state)
{
	float rgb[3];
	FILE *f;
	uint i, j;
	f = observatory_html_create_page("test_page.html", state, 0, TRUE);

	for(i = 0; i < 100; i++)
	{
		f_hsv_to_rgb(rgb, (float)i / 100.0, 0.5, 1.0);
		observatory_html_set_accent_color(rgb[0], rgb[1], rgb[2]);
		observatory_html_card_begin(f, "image.png");
		observatory_html_text(f, "The Ministry's procedure is unlike previous attempts at making an automated UV unwrapper.");	
		observatory_html_key_value(f, "something", "good", "no_url");
		observatory_html_bullet_point_begin(f, "My list");
		for(j = 0; j < i + f_randi(i) % 17; j++)
			observatory_html_bullet_point(f, "A thing", "no_page.html", NULL, 0, 0, 0);

		observatory_html_bullet_point_end(f);
		observatory_html_card_end(f);
	}
	observatory_html_complete(f, TRUE);
}