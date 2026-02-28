#include <stdio.h>
#include "forge.h"
#include "imagine.h"
	
#include "assemble_internal.h"
	
typedef enum{
	EPG_ST_ASSEMBLE,
	EPG_ST_IMAGE,
	EPG_ST_EMBED
}EPGSectionType;

typedef struct{
	char name[64];
	EPGSectionType type;
	AssembleTextElement *assemble_sections;
	uint sections_count;
	char *embed;
}EPGSection;

char *parse_command(int argc, char **argv, char *command, char *default_string)
{
	uint i, j;
	for(i = 1; i + 1 < argc; i++)
	{
		for(j = 0; argv[i][j] == command[j] && command[j] == 0; j++);
		if(argv[i][j] == command[j])
			return argv[i + 1];
	}
	return default_string;
}

uint parse_file(EPGSection **sections_pointer, uint section_count, char *path, uint start)
{
	char *file;
	size_t size;
	EPGSection *sections;
	uint i, j, sections_added = 0;
	sections = *sections_pointer;
	if(section_count % 1024 == 0)
		*sections_pointer = sections = realloc(sections, (section_count + 1024) * (sizeof *sections));
						
	for(i = 0; path[start + i] >= '0' && path[start + i] <= '9'; i++);
	for(j = 0; i < 64 - 1 && path[start + i] != '.' && path[start + i] != '\0'; i++)
	{
		sections[section_count].name[j] = path[start + i];
		if(sections[section_count].name[j] == '_')
			sections[section_count].name[j] = ' ';
		j++;
	}
	sections[section_count].name[j]  = 0;
			
	if(f_text_filter_case_insensitive(&path[start], ".png") ||
		f_text_filter_case_insensitive(&path[start], ".jpg") ||
		f_text_filter_case_insensitive(&path[start], ".jpeg") ||
		f_text_filter_case_insensitive(&path[start], ".gif"))
	{
		sections[section_count].type = EPG_ST_IMAGE;
		sections[section_count].embed = f_text_copy_allocate(path);
	//	printf(" image\n", &path[start]);
		sections_added = 1;
	}else if(f_text_filter_case_insensitive(&path[start], ".md"))
	{								
		file = f_text_load(path, &size);
		if(file != NULL)
		{
			sections[section_count].type = EPG_ST_ASSEMBLE;
			sections[section_count].assemble_sections = assemble_markdown_parse(file, &sections[section_count].sections_count);
			sections_added = 1;
		}
	}else if(f_text_filter_case_insensitive(&path[start], ".txt") ||
		f_text_filter_case_insensitive(&path[start], ".text"))
	{
		file = f_text_load(path, &size);
		if(file != NULL)
		{
			sections[section_count].type = EPG_ST_ASSEMBLE;
			sections[section_count].assemble_sections = assemble_text_parse(file, &sections[section_count].sections_count);
			sections_added = 1;
		}
	}else if(f_text_filter_case_insensitive(&path[start], ".html") ||
		f_text_filter_case_insensitive(&path[start], ".htm"))
	{
		sections[section_count].embed = f_text_copy_allocate(path);
	//	printf(" html\n", &path[start]);
	//	sections_added = 1;
	}					
	return sections_added;
}

int main(int argc, char **argv)
{
	char path[1024] = {'.', IMAGINE_DIR_SLASH}, buffer[1024], *file_name, *page_header, *file;
	FILE *output;
	IDir *root, *dir;
	uint i, j, k, start, section_count = 0, sections_added;
	boolean section_active = FALSE;
	EPGSection *sections = NULL;
	size_t size;


	file_name = parse_command(argc, argv, "-o", "index.html");
	output = fopen(file_name, "w");
	if(output == NULL)
	{
		printf("Error: could not open file %s\n", file_name);
		return 0;
	}

	file_name = parse_command(argc, argv, "-h", "page_header.html");
	page_header = f_text_load(file_name, &size);
	if(page_header == NULL)
	{
		printf("Error: could not open page header file %s\n", file_name);
		return 0;
	}
	fwrite(page_header, 1, size, output);
	file_name = parse_command(argc, argv, "-f", NULL); // front page
	if(file_name != NULL)
	{
	}else
	{
		file = f_text_load("front_page.txt", &size);
		if(file != NULL)
		{		
			sections = malloc((1024) * (sizeof *sections));
			sprintf(sections[section_count].name, "init");
			sections[section_count].type = EPG_ST_ASSEMBLE;
			sections[section_count].assemble_sections = assemble_text_parse(file, &sections[section_count].sections_count);
			section_count++;
		}
		file = f_text_load("front_page.md", &size);
		if(file != NULL)
		{		
			sections = malloc((1024) * (sizeof *sections));
			sprintf(sections[section_count].name, "init");
			sections[section_count].type = EPG_ST_ASSEMBLE;
			sections[section_count].assemble_sections = assemble_markdown_parse(file, &sections[section_count].sections_count);
			section_count++;
		}
	}
	file_name = parse_command(argc, argv, "-s", NULL);
	if(argc > 2)
	{
		printf(" - %s\n", file_name);
		sections_added = parse_file(&sections, section_count, file_name, 0);
	}else
	{
		file_name = parse_command(argc, argv, "-p", ".");
		root = imagine_path_dir_open(file_name);
		while(imagine_path_dir_next(root, &path[2], 1024 - 1))
		{
			if(imagine_path_is_dir(path))
			{
			//	printf("path %s\n", path);
				if(path[2] != '.' && path[2] != 'X' )
				{
					for(start = 2; path[start] != 0; start++);
					path[start++] = IMAGINE_DIR_SLASH;
					path[start] = 0;
					dir = imagine_path_dir_open(path);
					if(dir != NULL)
					{
						while(imagine_path_dir_next(dir, &path[start], 1024 - start))
						{
							if(path[start] != '.' && path[start] != 'X' && !imagine_path_is_dir(path))
							{
								printf(" - %s\n", path);
								sections_added = parse_file(&sections, section_count, path, start);
						//		
								for(i = 0; i < sections_added; i++)
								{
									if(!section_active)
									{
										for(k = 2; path[k] >= '0' && path[k] <= '9'; k++);
										for(j = 0; path[k] != IMAGINE_DIR_SLASH; j++)
										{
											if(path[k] == '_')
											{
												buffer[j] = ' ';
												k++;
											}else
												buffer[j] = path[k++];
										}
										buffer[j] = 0;
										if(buffer[0] >= 'a' && buffer[0] <= 'z')
											buffer[0] = buffer[0] + 'A' - 'a';
										fprintf(output, "<h4>%s</h4>\n", buffer);
										fprintf(output, "<ul>\n");
										section_active = TRUE;
									}
									fprintf(output, "<li><a onclick=\"display_section('%s')\" href=\"#%s\">%s</a></li>\n", sections[section_count].name, sections[section_count].name, sections[section_count].name);
									section_count++;
								}						
							}
						}
						imagine_path_dir_close(dir);
						if(section_active)
							fprintf(output, "</ul>\n");
						section_active = FALSE;
					}
				}
			}
		}
	}

//	AssembleTextElement *assemble_text_parse(char *text, uint *count);
	fprintf(output, "</div></div>\n");        
	fprintf(output, "<div style=\"border-width: 0px; border-style: solid; padding: 10px;\">\n"); 
	for(i = 0; i < section_count; i++)
	{
	   fprintf(output, "<div style=\"display:none;\" class=\"page\" id=\"%s\">\n", sections[i].name); 
		switch(sections[i].type)
		{
			case EPG_ST_ASSEMBLE :			
				if(sections[i].assemble_sections->length < 10)
					size = 0;
				size = assemble_html_print_size(sections[i].assemble_sections, sections[i].sections_count, NULL, NULL);
				file = malloc(size);
				assemble_html_print(sections[i].assemble_sections, sections[i].sections_count, file, NULL, NULL);
				fwrite(file, 1, size, output);
			break;
			case EPG_ST_IMAGE :			
				fprintf(output, "<img src=\"%s\" alt=\"%s\"> \n", sections[i].embed, sections[i].name); 
			break;

		}
	    fprintf(output, "</div>\n");   
	}
	fprintf(output, "</div>\n</body>\n<html>\n");
 
}