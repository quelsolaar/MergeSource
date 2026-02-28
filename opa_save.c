#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "imagine.h"
#include "testify.h"
#include "opa_internal.h"
#include "seduce.h"



void opa_save_indirection(FILE *file, uint array_length, uint current, char *prefix, boolean pointer, char *value)
{
	if(array_length == 1)
	{
		if(pointer)
			fprintf(file, "\t*%s = %s;\n", prefix, value);
		else
			fprintf(file, "\t%s = %s;\n", prefix, value);
	}
	else
		fprintf(file, "\t%s[%u] = %s;\n", prefix, current, value);
}

void opa_save_export_type(FILE *file, OPAProject *project, uint8 *memmory, uint type, uint indirection, uint array_length, uint offset, char *prefix, boolean pointer)
{
	STypeInState state;
	char name[256];
	uint i, j, k;

	if(indirection != 0 || type == OPA_TYPE_VOID || type == OPA_TYPE_FUNCTION)
	{
		for(j = 0; j < array_length; j++)
		{
			if(project->types[OPA_TYPE_VOID].size_of == 8)
			{
				uint64 *p;
				p = &memmory[offset + 8 * j];
				if(*p == 0)
				{
					opa_save_indirection(file, array_length, j, prefix, pointer, "NULL");
				}else
				{
					for(k = 0; k < project->memory_count && project->memmory[k].pointer != p[0]; k++);
					if(k < project->memory_count)
					{
						char malloc_buf[1024];
						sprintf(malloc_buf, "malloc(sizeof(%s) * %u)", project->types[project->memmory[k].type].type_name, project->memmory[k].data_size / project->types[project->memmory[k].type].size_of);
						opa_save_indirection(file, array_length, j, prefix, pointer, malloc_buf);
						opa_save_export_type(file, project, project->memmory[k].data, project->memmory[k].type, project->memmory[k].indirection, project->memmory[k].data_size / project->types[project->memmory[k].type].size_of, 0, prefix, TRUE);
					}else
						opa_save_indirection(file, array_length, j, prefix, pointer, "NULL");						
				}
			}else
			{
				uint32 *p;
				p = &memmory[offset + 4 * j];
				if(*p == 0)
				{
					opa_save_indirection(file, array_length, j, prefix, pointer, "NULL");
				}else
				{
					for(k = 0; k < project->memory_count && project->memmory[k].pointer != p[0]; k++);
					if(k < project->memory_count)
					{
						char malloc_buf[1024];
						sprintf(malloc_buf, "malloc(sizeof(%s) * %u)", project->types[project->memmory[k].type].type_name, project->memmory[k].data_size / project->types[project->memmory[k].type].size_of);
						opa_save_indirection(file, array_length, j, prefix, pointer, malloc_buf);
						opa_save_export_type(file, project, project->memmory[k].data, project->memmory[k].type, project->memmory[k].indirection, project->memmory[k].data_size / project->types[project->memmory[k].type].size_of, 0, prefix, TRUE);
					}else
						opa_save_indirection(file, array_length, j, prefix, pointer, "NULL");						
				}
			}
		}	
	}else
	{
		name[0] = 0;
		if(type >= OPA_TYPE_COUNT)
		{
			if(project->types[type].construct == OPA_C_ENUM)
			{
				int *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					for(j = 0; j < project->types[type].member_count; j++)
						if(p[0] == project->types[type].members[j].enum_value)
							break;
					if(j < project->types[type].member_count)
						fprintf(file, "\t%s = %s;\n", prefix, project->types[type].members[j].value_name);
					else
						fprintf(file, "\t%s = %i;\n", prefix, p[0]);
				}
				else for(i = 0; i < array_length; i++)
				{
					for(j = 0; j < project->types[type].member_count; j++)
						if(p[i] == project->types[type].members[j].enum_value)
							break;
					if(j < project->types[type].member_count)
						fprintf(file, "\t%s[%u] = %s;\n", prefix, i, project->types[type].members[j].value_name);
					else
						fprintf(file, "\t%s[%u] = %i\n", prefix, i, p[i]);
				}
			}else if(project->types[type].construct == OPA_C_STRUCT ||
				project->types[type].construct == OPA_C_UNION)
			{
				char *nextfix, *arrayfix, *addfix;
				for(i = 0; prefix[i] != 0; i++);
				nextfix = malloc(i + 1024);				
				for(i = 0; prefix[i] != 0; i++)
					nextfix[i] = prefix[i];
				arrayfix = &nextfix[i];

				for(i = 0; i < array_length; i++)
				{
					if(array_length != 1)
					{
						sprintf(arrayfix, "[%u].", i);
						for(j = 0; arrayfix[j] != 0; j++);
						addfix = &arrayfix[j];
					}
					else
					{
						if(pointer)
						{
							arrayfix[0] = '-';
							arrayfix[1] = '>';
							addfix = &arrayfix[2];
						}else
						{
							arrayfix[0] = '.';
							addfix = &arrayfix[1];
						}
					}

					for(j = 0; j < project->types[type].member_count; j++)
					{
						for(k = 0; project->types[type].members[j].value_name[k] != 0; k++)
							addfix[k] = project->types[type].members[j].value_name[k];
						addfix[k] = 0;



						if((project->types[project->types[type].members[j].base_type].construct == OPA_C_STRUCT ||
							project->types[project->types[type].members[j].base_type].construct == OPA_C_UNION) && 
							project->types[type].members[j].indirection == 0)
						{

						opa_save_export_type(file, project, memmory, project->types[type].members[j].base_type,
																project->types[type].members[j].indirection, 
																project->types[type].members[j].array_length, 
																offset + project->types[type].size_of * i + project->types[type].members[j].offset, 
																nextfix, FALSE);

						}else
						{

						opa_save_export_type(file, project, memmory, project->types[type].members[j].base_type,
																project->types[type].members[j].indirection, 
																project->types[type].members[j].array_length, 
																offset + project->types[type].size_of * i + project->types[type].members[j].offset, 
																nextfix, FALSE);
						}
					}
				}
				free(nextfix);
			}else
				opa_save_export_type(file, project, memmory, project->types[type].members[0].base_type,
														project->types[type].members[0].indirection, 
														project->types[type].members[0].array_length, 
														offset, 
														prefix, FALSE);
		}else switch(type)
		{
			default :
			break;
			case OPA_TYPE_SINGED_CHAR :
			{
				char *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %i;\n", prefix, (int)p[0]);
					else
						fprintf(file, "\t%s = %i;\n", prefix, (int)p[0]);
				}
				else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %i;\n", prefix, i, (int)p[i]);
			}
			break;
			case OPA_TYPE_UNSINGED_CHAR :
			{
				char *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %i;\n", prefix, (int)p[0]);
					else
						fprintf(file, "\t%s = %i;\n", prefix, (int)p[0]);
				}
				else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %i;\n", prefix, i, (int)p[i]);
			}
			break;
			case OPA_TYPE_SIGNED_SHORT :
			{
				int16 *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %i;\n", prefix, (int)p[0]);
					else
						fprintf(file, "\t%s = %i;\n", prefix, (int)p[0]);
				}
				else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %i;\n", prefix, i, (int)p[i]);
			}
			break;
			case OPA_TYPE_UNSIGNED_SHORT :
			{
				uint16 *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %i;\n", prefix, (int)p[0]);
					else
						fprintf(file, "\t%s = %i;\n", prefix, (int)p[0]);
				}
				else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %i;\n", prefix, i, (int)p[i]);
			}
			break;
			case OPA_TYPE_SINGED_INT :
			{
				int *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %i;\n", prefix, p[0]);
					else
						fprintf(file, "\t%s = %i;\n", prefix, p[0]);
				}
				else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %i;\n", prefix, i, p[i]);
			}
			break;
			case OPA_TYPE_UNSINGED_INT :
			{
				uint *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %u;\n", prefix, p[0]);
					else
						fprintf(file, "\t%s = %u;\n", prefix, p[0]);
				}
				else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %u;\n", prefix, i, p[i]);
			}
			break;
			case OPA_TYPE_SIGNED_LONG_LONG :
			{
				int64 *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %llu;\n", prefix, p[0]);
					else
						fprintf(file, "\t%s = %llu;\n", prefix, p[0]);
				}
				else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %llu;\n", prefix, i, p[i]);
			}
			break;
			case OPA_TYPE_UNSIGNED_LONG_LONG :
			{
				uint64 *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %lli;\n", prefix, p[0]);
					else
						fprintf(file, "\t%s = %lli;\n", prefix, p[0]);
				}else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %lli;\n", prefix, i, (uint)p[i]);
			}
			break;
			case OPA_TYPE_FLOAT :
			{
				float *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %f;\n", prefix, (float)p[0]);
					else
						fprintf(file, "\t%s = %f;\n", prefix, (float)p[0]);
				}else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %f;\n", prefix, i, (float)p[i]);
			}
			break;
			case OPA_TYPE_DOUBLE :
			{
				double *p;
				p = &memmory[offset];
				if(array_length == 1)
				{
					if(pointer)
						fprintf(file, "\t*%s = %f;\n", prefix, (float)p[0]);
					else
						fprintf(file, "\t%s = %f;\n", prefix, (float)p[0]);
				}else for(i = 0; i < array_length; i++)
					fprintf(file, "\t%s[%u] = %f;\n", prefix, i, (float)p[i]);
			}
			break;
		}
	}
}

void opa_export(OPAProject *project, OPAMemory *memory, char *file_name)
{
	char buffer[1024], *t;
	FILE *f;
	uint i;
	f = fopen(file_name, "w");
	if(f == NULL)
		return;

	fprintf(f, " /* Function esported by OPA*/ \n\n");

	t = project->types[memory->type].type_name;
	for(i = 0; t[i] != 0 && i < 1024 - 1; i++)
	{
		if(t[i] >= 'A' && t[i] <= 'Z')
			buffer[i] = t[i] - 'A' + 'a';
		else
			buffer[i] = t[i];
	}
	buffer[i] = 0;
	fprintf(f, "void %s_set(%s *p)\n{\n", buffer, t);

	opa_save_export_type(f, project, memory->data, memory->type, memory->indirection, memory->data_size / project->types[memory->type].size_of, 0, "p", TRUE);

	fprintf(f, "}\n");
	fclose(f);
}


void opa_save(OPAProject *project, THandle *handle)
{
	char *path;
	uint i;

	for(i = 0; i < project->memory_count; i++)
	{
		if((path = betray_requester_load_get(&project->memmory[i].path[0])) != NULL)
		{
			uint size;
			FILE *f;
			f = fopen(path, "r");
			if(f == NULL)
			{
				return NULL;
			}
			fseek(f, 0, SEEK_END);
			size = ftell(f);
			if(project->memmory[i].data_size < size)
				size = project->memmory[i].data_size;
			rewind(f);
			size = fread(project->memmory[i].data, 1, size, f);
			fclose(f);
			opa_request_memory_set(project, handle, i, 0, size, project->memmory[i].data);

		}
		if((path = betray_requester_save_get(&project->memmory[i].path[0])) != NULL)
		{
			FILE *f;
			f = fopen(path, "wb");
			if(f == NULL)
				return;
			fwrite(project->memmory[i].data, 1, project->memmory[i].data_size, f);
			fclose(f);
		}
		if((path = betray_requester_save_get(&project->memmory[i].path[1])) != NULL)
			opa_export(project, &project->memmory[i], path);
	}
}