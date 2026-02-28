#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "imagine.h"
#include "opa_internal.h"


#define OPA_FILE_NAME_MAX_SIZE 64

typedef struct{
	char *define;
	char *replacement;
	uint partent;
	uint active_generation;
	boolean macro;
	boolean pre_space;
	boolean post_space;
}OPAPreProcessorDefine;

typedef struct{
	OPAPreProcessorDefine *defines;
	uint define_count;
	char file_path[1024];
	char *file_name;
	char *output;
	uint output_allocated;
	uint progress;
}OPAProcessorDefineList;



boolean opa_value_name_character(char character)
{
	if(character >= 'A' && character <= 'Z')
		return TRUE;
	if(character >= 'a' && character <= 'z')
		return TRUE;
	if(character >= '0' && character <= '9')
		return TRUE;
	if(character == '_')
		return TRUE;
	return FALSE;
}

boolean opa_preprosesso_compare(char *buffer, char *compare)
{
	uint i;
	for(i = 0; buffer[i] == compare[i] && compare[i] != 0; i++);
	return compare[i] == 0 && buffer[i] <= ' ';
}

uint opa_preprosesso_end_find(char *buffer)
{
	uint i, blocks;
	for(i = blocks = 0; buffer[i] != 0; i++)
	{
		if(buffer[i] == '#')
		{
			if(opa_preprosesso_compare(&buffer[i], "#if"))
			{
				i += 3;
				blocks++;
			}
			if(opa_preprosesso_compare(&buffer[i], "#else"))
			{
				i += 5;
				if(blocks == 0)
					return i; 
			}
			if(opa_preprosesso_compare(&buffer[i], "#endif"))
			{
				i += 6;
				if(blocks > 0)
					blocks--;
				else
					return i; 
			}
		}
	}
	return i; 
}

uint opa_preprosesso_trigraphs(char *buffer)
{
	uint read, write;
	for(read = write = 0; 0 != buffer[read];)
	{
		if(buffer[read] == '?' && buffer[read + 1] == '?')
		{
			switch(buffer[read + 2])
			{
				case '(' :
					buffer[write++] = '[';
				break;
				case ')' :
					buffer[write++] = ']';
				break;
				case '<' :
					buffer[write++] = '{';
				break;
				case '>' :
					buffer[write++] = '}';
				break;
				case '=' :
					buffer[write++] = '#';
				break;
				case '/' :
					buffer[write++] = '\\';
				break;
				case '\'' :
					buffer[write++] = '^';
				break;
				case '!' :
					buffer[write++] = '|';
				break;
				case '-' :
					buffer[write++] = '~';
				break;
			}
			read += 3;
		}else
			buffer[write++] = buffer[read++];
	}
	buffer[write++] = 0;
	return write;
}

void opa_preprosesso_run(OPAProcessorDefineList *list, char *buffer, uint line_number, boolean debug)
{
	uint i, j, k, l, m;
	boolean new_line = FALSE;
	char *line = "__LINE__";
	char *key_word = "defined";
	for(i = 0; 0 != buffer[i];)
	{
		if(buffer[i] == '\n')
		{
			line_number++;
			i++;
			if(debug)
				new_line = TRUE;
			else
				list->output[list->progress++] = '\n';
		}else
		{
			if(buffer[i] == '\"')
			{
				if(debug && new_line)
				{
					sprintf(&list->output[list->progress], "\n__OPALINESTART_%u ", line_number);
					while(list->output[list->progress] != 0)
						list->progress++;
					new_line = FALSE;
				}
				list->output[list->progress++] = '\"';
				for(j = i + 1; buffer[j] != 0 && (buffer[j] != '\"' || buffer[j - 1] == '\\'); j++)
					list->output[list->progress++] = buffer[j];
				i = j;
			}

			for(j = 0; buffer[i + j] == key_word[j] && key_word[j] != 0; j++);
			if(key_word[j] == 0)
			{
				boolean parenthesis = FALSE;
				i += j;
				for(j = 0; buffer[i + j] != 0 && (buffer[i + j] == '(' || buffer[i + j] <= 32); j++)
					if(buffer[i + j] == '(')
						parenthesis = TRUE;
				i += j;
				if(buffer[i + j] == 0)
					break;
				for(j = 0; j < list->define_count; j++)
				{
					for(k = 0; list->defines[j].define[k] != 0 && buffer[i + k] == list->defines[j].define[k]; k++);
					if(list->defines[j].define[k] == 0)
						break;
				}
				if(j < list->define_count)
				{
					list->output[list->progress++] = '(';
					list->output[list->progress++] = '1';
					list->output[list->progress++] = ')';
					i += k;
					if(parenthesis)
						for(; buffer[i] != 0 && buffer[i] != ')'; i++);
					if(buffer[i] == 0)
						break;
					i++;
				}else
				{
					list->output[list->progress++] = '(';
					list->output[list->progress++] = '0';
					list->output[list->progress++] = ')';
					if(parenthesis)
						for(; buffer[i] != 0 && buffer[i - 1] != ')'; i++);
					else
						for(; buffer[i] != 0 && buffer[i] > 32; i++);
					if(buffer[i] == 0)
						break;
				}
			}else
			{
				for(j = 0; buffer[i + j] == line[j] && line[j] != 0; j++);
				if(line[j] == 0)
				{
					i += j;
					sprintf(&list->output[list->progress], "(%u)", line_number);
					while(list->output[list->progress] != 0)
						list->progress++;
				}else if(buffer[i] == '/' && buffer[i + 1] == '/')
				{
					if(debug)
					{
						if(debug && new_line)
						{
							sprintf(&list->output[list->progress], "\n__OPALINESTART_%u ", line_number);
							while(list->output[list->progress] != 0)
								list->progress++;
							new_line = FALSE;
						}
						list->output[list->progress++] = '/';
						list->output[list->progress++] = '*';
						for(i += 2 ;buffer[i] != '\n' && buffer[i] != 0; i++)
							list->output[list->progress++] = buffer[i];
						list->output[list->progress++] = '*';
						list->output[list->progress++] = '/';
					}else
					{
						for(i += 2; buffer[i] != '\n' && buffer[i] != 0; i++);
						if(buffer[i] == 0)
						{
							list->output[list->progress++] = 0;
							return;
						}
					}
				}else if(buffer[i] == '/' && buffer[i + 1] == '*')
				{
					if(debug)
					{
						if(debug && new_line)
						{
							sprintf(&list->output[list->progress], "\n__OPALINESTART_%u ", line_number);
							while(list->output[list->progress] != 0)
								list->progress++;
							new_line = FALSE;
						}
						for(;(buffer[i] != '*' || buffer[i + 1] != '/') && buffer[i] != 0; i++)
						{
							list->output[list->progress++] = buffer[i];
							if(buffer[i] != '\n')
								line_number++;
						}
						list->output[list->progress++] = '*';
						list->output[list->progress++] = '/';
						if(buffer[i] == 0 || buffer[i + 1] == 0)
						{
							list->output[list->progress++] = 0;
							return;
						}
						i += 2;
					}else
					{
						for(;(buffer[i] != '*' || buffer[i + 1] != '/') && buffer[i] != 0; i++)
							if(buffer[i] != '\n')
								line_number++;
						if(buffer[i] == 0)
						{
							list->output[list->progress++] = 0;
							return;
						}
						i += 2;
					}
				}else if(buffer[i] == '#')
				{
					if(opa_preprosesso_compare(&buffer[i], "#include"))
					{
						char path[1024];
						i += 8;
						while(buffer[i] != '\"' && buffer[i] != '<' && buffer[i] != 0)
							i++;
						for(j = 0; list->file_path[j] != 0; j++)
							path[j] = list->file_path[j];
						for(i++; buffer[i] != '\"' && buffer[i] != '>' && buffer[i] != 0 && buffer[i] != '\n'; i++)
							path[j++] = buffer[i];
						path[j++] = 0;
						if(buffer[i] != 0 && buffer[i] != '\n')
						{
							char *include_file, *save;
							printf("path = %s\n", path);
							if(debug)
							{
								sprintf(&list->output[list->progress], "\n__OPAFILENAME_%s \n", path);
								while(list->output[list->progress] != 0)
									list->progress++;
							}
							save = list->file_name;
							list->file_name = path;
							include_file = opa_file_load(path);
							if(include_file != 0)
							{
								opa_preprosesso_run(list, include_file, 0, debug);
								for(j = 0; list->output[j] != 0; j++);
								list->progress--;
								free(include_file);
							}
							list->file_name = save;
							if(debug)
							{
								sprintf(&list->output[list->progress], "\n__OPAFILENAME_%s \n", list->file_name);
								while(list->output[list->progress] != 0)
									list->progress++;
							}
							i++;
						}
					}else if(opa_preprosesso_compare(&buffer[i], "#define"))
					{
						uint current_define;
						i += 7;
						while(buffer[i] <= ' ' && buffer[i] != 0)
							i++;
						if(buffer[i] == 0)
						{
							list->output[list->progress++] = 0;
							return;
						}
						for(j = 0; buffer[i + j] > ' '; j++);

						if(list->define_count % 16 == 0)
							list->defines = realloc(list->defines, (sizeof *list->defines) * (list->define_count + 16));
						j++;
						list->defines[list->define_count].define = malloc(sizeof(char) * j);		
						for(j = 0; buffer[i + j] > ' ' && buffer[i + j] != '('; j++)
							list->defines[list->define_count].define[j] = buffer[i + j];
						list->defines[list->define_count].define[j] = 0;
						current_define = list->define_count++;
						
						printf("define found -%s-", list->defines[current_define].define);
						i += j;
						if(list->defines[current_define].macro = buffer[i] == '(') /* defines */
						{
							uint arguments = 0;
							
							i++;
							for(j = 0; buffer[i + j] != ')' && buffer[i + j] != 0; j++)
							{
								if(opa_value_name_character(buffer[i + j]))
								{
									if(list->define_count % 16 == 0)
										list->defines = realloc(list->defines, (sizeof *list->defines) * (list->define_count + 16));
									list->defines[list->define_count].partent = current_define;
									list->defines[list->define_count].active_generation = 0;
									list->defines[list->define_count].macro = FALSE;
									list->defines[list->define_count].replacement = NULL;
									list->defines[list->define_count].pre_space = TRUE;
									list->defines[list->define_count].post_space = TRUE;
									k = j;
									while(opa_value_name_character(buffer[i + k]))
										k++;
									list->defines[list->define_count].define = malloc(k - j + 1);
									for(k = 0; opa_value_name_character(buffer[i + j]); j++)
										list->defines[list->define_count].define[k++] = buffer[i + j];
									list->defines[list->define_count++].define[k] = 0;
									j--;
								}
							}
							i += j + 1;
						}
						list->defines[current_define].replacement = NULL;

						for(j = 0; buffer[i + j] <= ' ' && buffer[i + j] != '\n'; j++);
						if(buffer[i + j] != '\n')
						{
							i += j;
							list->defines[current_define].pre_space = (buffer[i] != '#' || buffer[i + 1] != '#');
							if(!list->defines[current_define].pre_space)
								i += 2;
							for(j = 0; buffer[i + j] != 0 && (buffer[i + j] != '\n' || buffer[i + j - 1] == '\\'); j++); /* FIX ME */
							j++;
							list->defines[current_define].replacement = malloc(sizeof(char) * j);
							for(j = k = 0; buffer[i + j] != 0 && (buffer[i + j] != '\n' || buffer[i + j - 1] == '\\'); j++)
							{
								if(buffer[i + j] != '\\' || buffer[i + j + 1] != '\n')
									list->defines[current_define].replacement[k++] = buffer[i + j];
							}
							i += j;
							list->defines[current_define].post_space = (k < 2 || list->defines[current_define].replacement[k - 1] != '#' || list->defines[current_define].replacement[k - 2] != '#');
							if(!list->defines[current_define].post_space)
								k -= 2;
							list->defines[current_define].replacement[k] = 0;
							printf(" -%s-", list->defines[current_define].replacement);

						}
						printf("\n", list->defines[current_define].define);
						list->defines[current_define].partent = -1;
						list->defines[current_define].active_generation = 1;


					}else if(opa_preprosesso_compare(&buffer[i], "#ifndef"))
					{
						i += 6;
						while(buffer[i] <= ' ' && buffer[i] != 0)
							i++;
						if(buffer[i] == 0)
						{
							list->output[list->progress++] = 0;
							return;
						}
						for(j = 0; j < list->define_count; j++)
						{
							for(k = 0; list->defines[j].define[k] != 0 && buffer[i + k] == list->defines[j].define[k]; k++);
							if(list->defines[j].define[k] == 0)
								break;
						}
						if(j < list->define_count)
						{
							i += k;
							i += opa_preprosesso_end_find(&buffer[i]);
						}else
						{
							while(buffer[i] != '\n' && buffer[i] != 0)
								i++;
						}
					}else if(opa_preprosesso_compare(&buffer[i], "#ifdef"))
					{
						i += 6;
						while(buffer[i] <= ' ' && buffer[i] != 0)
							i++;
						if(buffer[i] == 0)
						{
							list->output[list->progress++] = 0;
							return;
						}
						for(j = 0; j < list->define_count; j++)
						{
							for(k = 0; list->defines[j].define[k] != 0 && buffer[i + k] == list->defines[j].define[k]; k++);
							if(list->defines[j].define[k] == 0)
								break;
						}
						if(j < list->define_count)
						{
							while(buffer[i] != '\n' && buffer[i] != 0)
								i++;
						}else
						{
							i += k;
							i += opa_preprosesso_end_find(&buffer[i]);
						}
					}else if(opa_preprosesso_compare(&buffer[i], "#if"))
					{
						OPATokenFile tokens;
						char *copy;
						OPAProcessorDefineList sub_list;
						sub_list = *list;
						i += 4;
						for(j = 0; buffer[i + j] != 0 && buffer[i + j] != '\n'; j++);
						copy = malloc(j + 1);
						for(j = 0; buffer[i + j] != 0 && buffer[i + j] != '\n'; j++)
							copy[j] = buffer[i + j];
						copy[j] = 0;
						sub_list.output_allocated = j * 20;
						sub_list.output = malloc(sub_list.output_allocated + 1);
						sub_list.progress = 0;
						opa_preprosesso_run(&sub_list, copy, line_number, debug);
						sub_list.output[sub_list.progress] = 0;
						opa_tokenizer(&tokens, sub_list.output, -1);
						if(tokens.tokens->type == OPA_TT_INTEGER)
							printf("Intger = %u\n", tokens.tokens->value.integer);
						else
							printf("Intger = Error\n");
						free(sub_list.output);
						free(copy);
						i += j;
						if(tokens.tokens->type == OPA_TT_INTEGER && tokens.tokens->value.integer)
							i += opa_preprosesso_end_find(&buffer[i]);
					}else if(opa_preprosesso_compare(&buffer[i], "#else"))
					{
						i += 5;
						i += opa_preprosesso_end_find(&buffer[i]);
					}
					else if(opa_preprosesso_compare(&buffer[i], "#endif"))
					{
						i += 6;
					}
					else if(opa_preprosesso_compare(&buffer[i], "#error"))
					{
						char buf[1024];
						for(i += 6; buffer[i] != 0 && buffer[i] != '\"' && buffer[i] != '\n'; i++);
						if(buffer[i] == '\"')
						{
							i++;
							for(j = 0; buffer[i] != 0 && buffer[i] != '\"' && buffer[i] != '\n' && j < 1023; i++)
								buf[j++] = buffer[i];
							buf[j] = 0;
							printf("Error: %s\n", buf);
							for(; buffer[i] != 0 && buffer[i] != '\n'; i++);
						}
					}				
				}else
				{
					j = list->define_count;
					if(i == 0 || !opa_value_name_character(buffer[i - 1]))
					{
						for(j = 0; j < list->define_count; j++)
						{
							if(list->defines[j].active_generation == 1)
							{
								for(k = 0; list->defines[j].define[k] != 0 && buffer[i + k] == list->defines[j].define[k]; k++);
								if(list->defines[j].define[k] == 0 && !opa_value_name_character(buffer[i + k]))
									break;
							}
						}
					}
					if(j < list->define_count)
					{
						OPAProcessorDefineList list_copy;
						char *b;
						uint pre_length = 0, post_length = 0, param, parenthesis;
						for(k = 0; list->defines[j].define[k] != 0; k++);
						i += k;
						if(debug && new_line)
						{
							sprintf(&list->output[list->progress], "\n__OPALINESTART_%u ", line_number);
							while(list->output[list->progress] != 0)
								list->progress++;
							new_line = FALSE;
						}
				
						for(k = 0; list->defines[j].replacement[k] != 0; k++); /* compute add length */
						if(list->defines[j].pre_space)
						{
							if(list->progress == 0 || list->output[list->progress - 1] > ' ')
								list->output[list->progress++] = ' ';
						}else
						{
							for(pre_length = 0; pre_length <= list->progress && list->output[list->progress - pre_length - 1] > ' ' ; pre_length++);
							list->progress -= pre_length;
							printf("backing up %u\n", pre_length);
						}							
						if(!list->defines[j].post_space)
							while(buffer[i + post_length] > ' ')
								post_length++;

						b = malloc(pre_length + k + post_length + 1);
						for(k = l = 0; k < pre_length; k++)
							b[l++] = list->output[list->progress + k];
						for(k = 0; list->defines[j].replacement[k] != 0; k++)
							b[l++] = list->defines[j].replacement[k];
						for(k = 0; k < post_length; k++)
							b[l++] = buffer[i + k];
						b[l++] = 0;

						printf("Pre Processor line -%s-\n", b);

						list_copy = *list;
						list_copy.progress = 0;
						list_copy.output_allocated = l * 40;
						list_copy.output = malloc(list_copy.output_allocated);

						for(m = param = 0; m < list->define_count; m++)
						{
							if(list->defines[m].partent == j)
							{
								for(k = i; buffer[k] != 0 && buffer[k] != '('; k++);
								if(buffer[k] == '(')
								{
									k++;
									for(l = parenthesis = 0; l < param && (buffer[k] != ')' || parenthesis != 0) && buffer[k] != 0; k++)
									{
										if(buffer[k] == ')')
											parenthesis--;
										if(buffer[k] == '(')
											parenthesis++;
										if(parenthesis == 0 && buffer[k] == ',')
											l++;
									}
									if(l == param)
									{
										for(l = parenthesis = 0; buffer[k + l] != 0 && (parenthesis != 0 || (buffer[k + l] != ')' && buffer[k + l] != ',')); l++)
										{
											if(buffer[k + l] == ')')
												parenthesis--;
											if(buffer[k + l] == '(')
												parenthesis++;
										}

										list->defines[m].replacement = malloc(l + 1);
										for(l = parenthesis = 0; buffer[k + l] != 0 && (parenthesis != 0 || (buffer[k + l] != ')' && buffer[k + l] != ',')); l++)
										{
											if(buffer[k + l] == ')')
												parenthesis--;
											if(buffer[k + l] == '(')
												parenthesis++;
											list->defines[m].replacement[l] = buffer[k + l];
										}
										list->defines[m].replacement[l] = 0;
										list->defines[m].active_generation = 1;
										param++;
									}
								}
							}
						}
						if(list->defines[j].macro)
						{
							for(k = i; buffer[k] != 0 && buffer[k] != '('; k++);
							if(buffer[k] == '(')
							{
								parenthesis = 1;
								k++;
								for(; buffer[k] != 0 && parenthesis != 0; k++)
								{
									if(buffer[k] == ')')
										parenthesis--;
									if(buffer[k] == '(')
										parenthesis++;
								}
								i = k;
							}
						}
						list->defines[j].active_generation = 0;
						printf("Replacing -%s- ENTER\n %s\n", list->defines[j].define, b);
						opa_preprosesso_run(&list_copy, b, line_number, debug);
						printf("Replacing LEAVE\n %s\n", list_copy.output);
						list->defines[j].active_generation = 1;
						for(k = 0; k < list->define_count; k++)
						{
							if(list->defines[k].partent == j && list->defines[k].active_generation)
							{
								free(list->defines[k].replacement);
								list->defines[k].active_generation = 0;
							}
						}
				
 						printf("Pre Processor output -%s- (length = %u)\n", list_copy.output, list_copy.progress);

						for(k = 0; list_copy.output[k] != 0; k++)
							list->output[list->progress++] = list_copy.output[k];
						if(list->defines[j].post_space)
							list->output[list->progress++] = ' ';
						free(list_copy.output);
						free(b);
					}else
					{
						if(debug && new_line && buffer[i] >= ' ')
						{
							sprintf(&list->output[list->progress], "\n__OPALINESTART_%u ", line_number);
							while(list->output[list->progress] != 0)
								list->progress++;
							new_line = FALSE;
						}

						list->output[list->progress++] = buffer[i++];
					}
				}
			}
		}
	}
	list->output[list->progress++] = 0;
}

char *opa_preprosessor(char *file_name)
{
	OPAProcessorDefineList list;
	char *buffer;
	uint i;
	for(i = 0; i < 1023 && file_name[i] != 0; i++)
		list.file_path[i] = file_name[i];
	list.file_path[i] = 0;
	if(i == 0)
		return NULL;
	for(; i != 0 && file_name[i] != '/' && file_name[i] != '\\'; i--);
	if(i == 0)
		list.file_path[i] = 0;
	else
		list.file_path[i + 1] = 0;
	list.file_name = file_name;
	buffer = opa_file_load(file_name);
	if(buffer == NULL)
		return;
	printf("RAW:--\n");
	printf(buffer);
	printf("--\n");
	i = opa_preprosesso_trigraphs(buffer);
	printf("TriGraphs--\n");
	printf(buffer);
	printf("--\n");
	list.defines = NULL;
	list.define_count = 0;
	list.output_allocated = i * 40;
	list.output = malloc((sizeof * list.output) * list.output_allocated);
	list.progress = 0;
	opa_preprosesso_run(&list, buffer, 0, FALSE);

	printf("Post Pre--\n");
	printf(list.output);
	printf("--\n");
	free(buffer);
	return list.output;
}

