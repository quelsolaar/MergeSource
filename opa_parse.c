#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "imagine.h"
#include "opa_internal.h"


#define OPA_FILE_NAME_MAX_SIZE 64
/*

typedef enum{
	OPA_TYPE_SINGED_CHAR,
	OPA_TYPE_UNSINGED_CHAR,
	OPA_TYPE_SIGNED_SHORT,
	OPA_TYPE_UNSIGNED_SHORT,
	OPA_TYPE_SINGED_INT,
	OPA_TYPE_UNSINGED_INT,
	OPA_TYPE_SIGNED_LONG_LONG,
	OPA_TYPE_UNSIGNED_LONG_LONG,
	OPA_TYPE_FLOAT,
	OPA_TYPE_DOUBLE,
	OPA_TYPE_VOID,
	OPA_TYPE_FUNCTION,
	OPA_TYPE_COUNT,	
}OPAType;

typedef struct{
	char value_name[32];
	uint offset;
	uint indirection;
	uint base_type;
	uint array_length;
}OPAMember;

typedef enum{
	OPA_C_STRUCT,
	OPA_C_UNION,
	OPA_C_ENUM
}OPAConstructType;

typedef struct{
	char type_name[642];
	OPAConstructType construct;
	uint member_count;
	OPAMember *members;
	uint array_length;
	uint struct_sizeof;
}OPAConstruct;

typedef struct{
	OPAConstruct *types;
	uint type_count;
	uint type_allocated;
}OPAProject;

*/


char *opa_file_load(char *file)
{
	char *buffer;
	uint size;
	FILE *f;
	f = fopen(file, "r");
	if(f == NULL)
		return NULL;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	buffer = malloc(size + 1);
	size = fread(buffer, 1, size, f);
	fclose(f);
	buffer[size] = 0;
	return buffer;
}




boolean opa_gapped_string_compare(char *unterminated, char *terminated, uint *pos)
{
	uint a = 0, b = 0;
	while(terminated[b] != 0)
	{
		for(; unterminated[a] != 0 && unterminated[a] <= ' '; a++);
		for(; terminated[b] != 0 && terminated[b] <= ' '; b++);
		while(unterminated[a] == terminated[b])
		{
			a++;
			b++;
		}
		if(terminated[b] > ' ')
			return FALSE;
	}
	if((unterminated[a] >= '0' && unterminated[a] <= '9') ||
		(unterminated[a] >= 'A' && unterminated[a] <= 'Z') ||
		(unterminated[a] >= 'a' && unterminated[a] <= 'z') ||
		unterminated[a] >= '_')
		return FALSE;
	*pos = a;
	return TRUE;
}


uint opa_file_parse_type(OPAProject *project, char *buffer, uint *pos)
{
	char *string[] = {"struct", "union", "enum"};
	uint i, j, k;
	for(i = 0; buffer[i] <= 32 && buffer[i] != 0; i++);
	*pos = i;
	if(buffer[i] == 0)
	{
		return OPA_C_ERROR;
	}
	for(k = 0; k < 3; k++)
	{
		for(j = 0; buffer[i + j] != 0 && buffer[i + j] == string[k][j]; j++);
		if(string[k][j] == 0)
		{
			*pos = i + j;
			return k;
		}
	}
	for(k = 0; k < project->type_count; k++)
	{
		if(opa_gapped_string_compare(&buffer[i], project->types[k].type_name, pos))
		{
			*pos += i;
			return OPA_C_REFERENCE_START + k;
		}
	}
	printf("Parse Error: OPA_C_ERROR\n");
	return OPA_C_ERROR;
}
void opa_file_parse_define(OPAProject *project, char *buffer)
{
	char *include = "#include", file_name[OPA_FILE_NAME_MAX_SIZE], *file, *typedef_string = "typedef";
	uint i, j, k, pos = 0;
	for(i = 0; buffer[i] != 0; i++)
	{
		if(buffer[i] == '#')
		{
			for(j = 0; include[j] != 0 && include[j] == buffer[i + j]; j++);
			if(include[j] == 0)
			{
				for(i += j; buffer[i] != 0 && buffer[i] != '<' && buffer[i] != '\"' && buffer[i] != '\n'; i++);
				if(buffer[i] == '\"')
				{
					char path[1024];
					i++;
					for(j = 0; project->path[j] != 0 && j < 1023; j++)
						path[j] = project->path[j];

					for(k = 0; buffer[i] != 0 && buffer[i] != '\"' && j < 1023 && k < OPA_FILE_NAME_MAX_SIZE - 1; j++)
						path[j] = file_name[k++] = buffer[i++];
					path[j] = file_name[k] = 0;
					for(j = 0; j < project->include_count; j++)
					{
						for(k = 0; file_name[k] != 0 && file_name[k] == project->includes[j * OPA_FILE_NAME_MAX_SIZE + k]; k++);
						if(file_name[k] == project->includes[j * OPA_FILE_NAME_MAX_SIZE + k])
							break;
					}
					if(j == project->include_count)
					{
						if(project->include_count % 16 == 0)
							project->includes = realloc(project->includes, (sizeof *project->includes) * (project->include_count + 16) * OPA_FILE_NAME_MAX_SIZE);
						for(k = 0; file_name[k] != 0; k++)
							project->includes[project->include_count * OPA_FILE_NAME_MAX_SIZE + k] = file_name[k];
						project->includes[project->include_count++ * OPA_FILE_NAME_MAX_SIZE + k] = 0;
						
						file = opa_file_load(path);
						if(file != NULL)
						{
							opa_file_parse_define(project, file);
							free(file);
							printf("file include = %s\n", file_name);
						}
					}
				//	file = opa_file_load(path);
				}
			}
		}
		for(j = 0; typedef_string[j] != 0 && typedef_string[j] == buffer[i + j]; j++);
		if(typedef_string[j] == 0)
		{
			printf("found typedef\n");
			opa_file_parse_type(project, &buffer[i + j], &pos);
		}
	}
}


void opa_parse_old(OPAProject *project, char *path)
{
	char file_name[1024], merged_path[1024], *source;
	IDir *dir;
	uint i, j;
	project->types = NULL;
	project->type_count = 0;
	project->type_allocated = 0;
	project->includes = NULL;
	project->include_count = 0;
	for(i = 0; i < 1023 && path[i] != 0; i++)
		project->path[i] = merged_path[i] = path[i];
	project->path[i] = 0;

	dir = imagine_path_dir_open(path);
	if(dir != NULL)
	{
		printf("Dir is open:\n");
		while(imagine_path_dir_next(dir, file_name, 1024))
		{
			for(j = 0; file_name[j] != 0 && file_name[j] != '.'; j++);
			if(file_name[j] == '.' && file_name[j + 1] == 'c' && file_name[j + 2] == 0)
			{
			//	printf("Path %s\n", file_name);
				for(j = 0; j + i < 1023 && file_name[j] != 0; j++)
					merged_path[i + j] = file_name[j];
				merged_path[i + j] = 0;
			//	printf("Path %s\n", merged_path);
				if(!imagine_path_is_dir(merged_path))
				{
			//		printf("Path %s\n", merged_path);
					source = opa_file_load(merged_path);
					if(source != NULL)
					{
						opa_file_parse_define(project, source);
						free(source);
					}
				}
			}
		}
	}
	imagine_path_dir_close(dir);
}

