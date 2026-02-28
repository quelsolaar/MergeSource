#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "imagine.h"
#include "testify.h"
#include "opa_internal.h"


#define OPA_FILE_NAME_MAX_SIZE 64

void opa_parse_type_init(OPAConstruct *construct)
{
	construct->type_name[0] = 0;
	construct->member_count = 0;
	construct->members = NULL;
	construct->size_of = 0;
}
uint opa_parse_allocate_type(OPAProject *project, OPAConstruct *construct)
{
	if(project->type_count == project->type_allocated);
	{		
		project->type_allocated += 16;
		project->types = realloc(project->types, (sizeof *project->types) * project->type_allocated);
	}	
	project->types[project->type_count++] = *construct;
	return project->type_count - 1;
}

/*

typedef enum{
	OPA_TT_DELETED,
	OPA_TT_SPACE,
	OPA_TT_NAME,
	OPA_TT_OPERATOR,
	OPA_TT_SINGLE_CHARACTER_START,
	OPA_TT_SEMICOLON,
	OPA_TT_COLON,
	OPA_TT_COMMA,
	OPA_TT_PERIOD,
	OPA_TT_OPEN_PARENTHESIS,
	OPA_TT_CLOSE_PARENTHESIS,
	OPA_TT_OPEN_SCOPE,
	OPA_TT_CLOSE_SCOPE,
	OPA_TT_OPEN_BRACKET,
	OPA_TT_CLOSE_BRACKET,
	OPA_TT_INTEGER,
	OPA_TT_REAL,
	OPA_TT_KEYWORD_AUTO,
	OPA_TT_KEYWORD_BREAK,
	OPA_TT_KEYWORD_CASE,
	OPA_TT_KEYWORD_CHAR,
	OPA_TT_KEYWORD_CONST,
	OPA_TT_KEYWORD_CONTINUE,
	OPA_TT_KEYWORD_DEFAULT,
	OPA_TT_KEYWORD_DO,
	OPA_TT_KEYWORD_DOUBLE,
	OPA_TT_KEYWORD_ELSE,
	OPA_TT_KEYWORD_ENUM,
	OPA_TT_KEYWORD_EXTERN,
	OPA_TT_KEYWORD_FLOAT,
	OPA_TT_KEYWORD_FOR,
	OPA_TT_KEYWORD_GOTO,
	OPA_TT_KEYWORD_IF,
	OPA_TT_KEYWORD_INT,
	OPA_TT_KEYWORD_LONG,
	OPA_TT_KEYWORD_REGISTER,
	OPA_TT_KEYWORD_RETURN,
	OPA_TT_KEYWORD_SHORT,
	OPA_TT_KEYWORD_SIGNED,
	OPA_TT_KEYWORD_SIZEOF,
	OPA_TT_KEYWORD_STATIC,
	OPA_TT_KEYWORD_STRUCT,
	OPA_TT_KEYWORD_SWITCH,
	OPA_TT_KEYWORD_TYPEDEF,
	OPA_TT_KEYWORD_UNION,
	OPA_TT_KEYWORD_UNSIGNED,
	OPA_TT_KEYWORD_VOID,
	OPA_TT_KEYWORD_VOLATILE,
	OPA_TT_KEYWORD_WHILE
}OPATokenType;
*/

void opa_type_size_compute(OPAProject *project)
{
	OPAConstruct *type;
	uint i, j, base_type;
	for(i = OPA_TYPE_COUNT; i < project->type_count; i++)
	{
		type = &project->types[i];
		if(type->construct == OPA_C_STRUCT || type->construct == OPA_C_BASE_TYPE)
		{
			type->alignment = 1;
			type->size_of = 0;
			for(j = 0; j < type->member_count; j++)
			{
				base_type = type->members[j].base_type;
				if(type->members[j].indirection != 0)
					base_type = OPA_TYPE_VOID;
				while(type->size_of % project->types[base_type].alignment != 0)
					type->size_of++;
				type->members[j].offset = type->size_of;
				type->size_of += type->members[j].array_length * project->types[base_type].size_of;
				if(type->alignment < project->types[base_type].alignment)
					type->alignment = project->types[base_type].alignment;
			}
			while(type->size_of % type->alignment != 0)
				type->size_of++;
		}else if(type->construct == OPA_C_UNION)
		{
			type->alignment = 1;
			type->size_of = 0;
			for(j = 0; j < type->member_count; j++)
			{
				base_type = type->members[j].base_type;
				if(type->members[j].indirection != 0)
					base_type = OPA_TYPE_VOID;
				type->members[j].offset = 0;
				if(type->size_of < type->members[j].array_length * project->types[base_type].size_of)
					type->size_of = type->members[j].array_length * project->types[base_type].size_of;
				if(type->alignment < project->types[base_type].alignment)
					type->alignment = project->types[base_type].alignment;
			}
			while(type->size_of % type->alignment != 0)
				type->size_of++;
		}
		else if(type->construct == OPA_C_ENUM)
		{
			type->alignment = project->types[OPA_TYPE_UNSINGED_INT].alignment;
			type->size_of = project->types[OPA_TYPE_UNSINGED_INT].size_of;
		}
	}
}

extern OPATokenFile *opa_parse_typedefs(OPAProject *project, OPATokenFile *tokens, OPAToken *token, OPAConstruct *construct);

OPAToken *opa_parse_member(OPAProject *project, OPATokenFile *tokens, OPAToken *token, OPAConstruct *construct)
{
	OPAMember *m;
	uint i, j;
	char *text = "que";

	if(token->type == OPA_TT_KEYWORD_SIGNED)
		token = token->next;
	if(token->type == OPA_TT_KEYWORD_UNSIGNED)
	{
		token = token->next;
		if(token->type == OPA_TT_KEYWORD_CHAR)
			text = "uchar";
		else if(token->type == OPA_TT_KEYWORD_SHORT)
			text = "ushort";
		else if(token->type == OPA_TT_KEYWORD_INT)
			text = "uint";
		else if(token->type == OPA_TT_KEYWORD_LONG)
		{
			if(((OPAToken *)token->next)->type == OPA_TT_KEYWORD_LONG)
			{
				text = "ulong long";
				token = token->next;
			}else
				text = "uint";
		}
	}else
	{
		if(token->type == OPA_TT_KEYWORD_CHAR)
			text = "char";
		else if(token->type == OPA_TT_KEYWORD_SHORT)
			text = "short";
		else if(token->type == OPA_TT_KEYWORD_INT)
			text = "int";
		else if(token->type == OPA_TT_KEYWORD_LONG)
		{
			if(((OPAToken *)token->next)->type == OPA_TT_KEYWORD_LONG)
			{
				text = "long long";
				token = token->next;
			}else
				text = "int";
		}else
			text = &tokens->buffer[token->start];
	}
	if(token->type == OPA_TT_KEYWORD_UNION || token->type == OPA_TT_KEYWORD_ENUM || token->type == OPA_TT_KEYWORD_STRUCT)
	{
		char *unnaned = "_unnamed";
		OPAConstruct c;
		opa_parse_type_init(&c);
		token = opa_parse_typedefs(project, tokens, token, &c);
		for(j = 0; unnaned[j] != 0; j++)
			c.type_name[j] = unnaned[j];
		c.type_name[j] = 0;
		i = opa_parse_allocate_type(project, &c);
		while(token->type != OPA_TT_CLOSE_SCOPE)
			token = token->prev;
		token = token->next;
	}else
	{
		token = token->next;
		for(i = 0; i < project->type_count; i++)
		{
			for(j = 0; project->types[i].type_name[j] != 0 && text[j] == project->types[i].type_name[j]; j++);
			if(project->types[i].type_name[j] == 0)
				break;
		}
		if(i == project->type_count)
		{
			for(i = 0; i < project->type_count; i++)
				printf("name[%u] = %s\n", i, project->types[i].type_name);
			printf("Error type %s undefined\n", text);
			exit(0);
		}
	}

	if(construct->member_count % 16 == 0)
		construct->members = realloc(construct->members, (sizeof *construct->members) * (construct->member_count + 16));
	m = &construct->members[construct->member_count++];
	m->base_type = i;
	m->offset = 0;
	m->indirection = 0;
	m->array_length = 1;
	m->value_name[0] = 0;
	while(token->type != OPA_TT_SEMICOLON)
	{
		if(token->type == OPA_TT_NAME && m->value_name[0] == 0)
		{
			for(i = 0; i < token->length; i++)
				m->value_name[i] = tokens->buffer[token->start + i];
			m->value_name[i] = 0;
		}

		if(token->type == OPA_TT_OPEN_PARENTHESIS)
		{
			m->base_type = OPA_TYPE_FUNCTION;
			m->indirection = 1;
		}
		if(token->type == OPA_TT_OPERATOR && m->base_type != OPA_TYPE_FUNCTION)
		{
			for(i = 0; i < token->length; i++)
				if(tokens->buffer[token->start + i] == '*')
					m->indirection++;
		}
		if(token->type == OPA_TT_INTEGER)
			m->array_length *= token->value.integer;
		token = token->next;
	}
	return token->next;
}


OPATokenFile *opa_parse_typedefs(OPAProject *project, OPATokenFile *tokens, OPAToken *token, OPAConstruct *construct)
{
	OPAToken *t;
	uint i, j;
	if(token->type == OPA_TT_KEYWORD_ENUM)
	{
		construct->construct = OPA_C_ENUM;
		construct->size_of = sizeof(int);
		for(i = 0; TRUE; i++)
		{
			while(token != NULL && token->type != OPA_TT_CLOSE_SCOPE && token->type != OPA_TT_NAME)
				token = token->next; 
			if(token == NULL)
				return;
			if(token->type == OPA_TT_CLOSE_SCOPE)
				break;
			if(construct->member_count % 16 == 0)
				construct->members = realloc(construct->members, (sizeof *construct->members) * (construct->member_count + 16));
			for(j = 0; j < token->length; j++)
				construct->members[construct->member_count].value_name[j] = tokens->buffer[token->start + j];
			construct->members[construct->member_count].value_name[j] = 0;
			construct->members[construct->member_count].enum_value = i;
			for(t = token->next; t != NULL; t = t->next)
			{
				for(j = 0; tokens->buffer[t->start + j] == tokens->buffer[token->start + j] && j < token->length; j++);
				if(j == token->length)
				{
					t->value.integer = construct->members[construct->member_count].enum_value;
					t->type = OPA_TT_INTEGER;
				}
			}			
			construct->member_count++;
			token = token->next; 
		}
		token = token->next;
		if(token->type == OPA_TT_NAME)
		{
			for(i = 0; i < token->length; i++)
				construct->type_name[i] = tokens->buffer[token->start + i];
			construct->type_name[i] = 0;
		}
		return token->next;
	}else if(token->type == OPA_TT_KEYWORD_STRUCT ||
		token->type == OPA_TT_KEYWORD_UNION)
	{	
		if(token->type == OPA_TT_KEYWORD_STRUCT)
			construct->construct = OPA_C_STRUCT;
		else
			construct->construct = OPA_C_UNION;
		token = token->next; // struct
		token = token->next; // {
		while(token->type != OPA_TT_CLOSE_SCOPE)
			token = opa_parse_member(project, tokens, token, construct);		
		token = token->next; // }
		if(token->type == OPA_TT_NAME)
		{
			for(i = 0; i < token->length; i++)
				construct->type_name[i] = tokens->buffer[token->start + i];
			construct->type_name[i] = 0;
		}
		return token->next;
	}else
	{
		construct->construct = OPA_C_REFERENCE_START;
		token = opa_parse_member(project, tokens, token, construct);
		construct->type_name[0] = 0;
	}
	return token->next;
}


void opa_parse_source(OPAProject *project, OPATokenFile *tokens)
{
	OPAConstruct c;
	OPAToken *token, *t;
	uint i;
	token = tokens->tokens;
	for(i = 0; i < tokens->token_count && tokens->tokens[i].type == OPA_TT_DELETED; i++);
	if(i < tokens->token_count)
	{
		for(t = &tokens->tokens[i]; t != NULL; t = t->next)
		{
			printf("%p -> %p\n", t, t->next);
			if(t->type == OPA_TT_KEYWORD_TYPEDEF)
			{
				t = t->next;
				if(t == NULL)
					return;
				opa_parse_type_init(&c);
			/*	t = */opa_parse_typedefs(project, tokens, t, &c);
				opa_parse_allocate_type(project, &c);
			}
		}
	}
}

void opa_parse_print(OPAProject *project)
{
	uint i, j, k;
	for(i = 0; i < project->type_count; i++)
	{
		printf("-%s\n", project->types[i].type_name);

		printf("-%u (sizeof = %u allignment = %u)\n", i, project->types[i].size_of, project->types[i].alignment);

		if(project->types[i].construct == OPA_C_ENUM)
			for(j = 0; j < project->types[i].member_count; j++)
				printf("\t%s\n", project->types[i].members[j].value_name);

		if(project->types[i].construct == OPA_C_STRUCT || 
			project->types[i].construct == OPA_C_UNION)
		{
			if(project->types[i].construct == OPA_C_STRUCT)
				printf("Struct\n");
			else
				printf("Union\n");
			for(j = 0; j < project->types[i].member_count; j++)
			{
				
				printf("\t");
				printf("%s(%u) ", project->types[project->types[i].members[j].base_type].type_name, project->types[i].members[j].base_type);
				for(k = 0; k < project->types[i].members[j].indirection; k++)
					printf("*");
				printf("%s", project->types[i].members[j].value_name);
				if(project->types[i].members[j].array_length != 1)
					printf("[%u]", project->types[i].members[j].array_length);
				printf("(offset = %u)\n", project->types[i].members[j].offset);
			}
		}
	}
	k = i;
}


void opa_parse_add_base_type(OPAProject *project, uint base_type, uint size_of, uint alignment, char *name)
{
	OPAConstruct c;
	uint i;
	opa_parse_type_init(&c);
	c.member_count = 1;
	c.members = malloc(sizeof *c.members);
	c.members->array_length = 1;
	c.members->base_type = base_type;
	c.members->indirection = 0;
	c.members->offset = 0;
	c.size_of = size_of;
	if(alignment == 0)
		i  = 0;
	c.alignment = alignment;
	for(i = 0; name[i] != 0; i++)
		c.type_name[i] = name[i];
	c.type_name[i] = 0;
	c.construct = OPA_C_BASE_TYPE;
	opa_parse_allocate_type(project, &c);
}
/*
void opa_parse(OPAProject *project, char *path)
{
	char file_name[1024], merged_path[1024], *source;
	OPATokenFile tokens;
	uint8 *buffer;
	uint i, j;
	project->types = NULL;
	project->type_count = 0;
	project->type_allocated = 0;
	project->includes = NULL;
	project->include_count = 0;
	for(i = 0; i < 1023 && path[i] != 0; i++)
		project->path[i] = merged_path[i] = path[i];
	project->path[i] = 0;

	opa_parse_add_base_type(project, OPA_TYPE_SINGED_CHAR, sizeof(char), "char");
	opa_parse_add_base_type(project, OPA_TYPE_UNSINGED_CHAR, sizeof(char), "uchar");
	opa_parse_add_base_type(project, OPA_TYPE_SIGNED_SHORT, sizeof(short), "short");
	opa_parse_add_base_type(project, OPA_TYPE_UNSIGNED_SHORT, sizeof(char), "ushort");
	opa_parse_add_base_type(project, OPA_TYPE_SINGED_INT, sizeof(int), "uint");
	opa_parse_add_base_type(project, OPA_TYPE_UNSINGED_INT, sizeof(int), "uint");
	opa_parse_add_base_type(project, OPA_TYPE_SIGNED_LONG_LONG, sizeof(long long), "long long");
	opa_parse_add_base_type(project, OPA_TYPE_UNSIGNED_LONG_LONG, sizeof(long long), "ulong long");
	opa_parse_add_base_type(project, OPA_TYPE_FLOAT, sizeof(float), "float");
	opa_parse_add_base_type(project, OPA_TYPE_DOUBLE, sizeof(double), "double");
	opa_parse_add_base_type(project, OPA_TYPE_VOID, sizeof(void*), "void");
	opa_parse_add_base_type(project, OPA_TYPE_FUNCTION, sizeof(float), "function");

	buffer = opa_preprosessor("../../Mergesource/stellar_structure.h");
	opa_tokenizer(&tokens, buffer);
	opa_parse_source(project, &tokens);
	opa_parse_print(project);
}
*/
boolean opa_parse_init(OPAProject *project, THandle *handle)
{
	char *type_names[] = {"char", "uchar", "short", "ushort", "uint", "uint", "long long", "ulong long", "float", "double", "void", "function"};
	char file_name[1024], merged_path[1024], *source;
	OPATokenFile tokens;
	uint8 *buffer, size[OPA_TYPE_COUNT], allignment[OPA_TYPE_COUNT];
	uint i, j;
	project->types = NULL;
	project->type_count = 0;
	project->type_allocated = 0;
	project->includes = NULL;
	project->include_count = 0;
	project->path[0] = 0;
	project->memmory = NULL;
	project->memory_allocated = 0;
	project->memory_count = 0;

	testify_restart_marker_set(handle); /* Sets the marker at the current spot in the stream, forcing the handle not to free any data from this point forward in order to be able to rewind to this point. */

	if(!testify_retivable(handle, OPA_TYPE_COUNT * 2))
	{
		testify_restart_marker_reset(handle);
		return FALSE;
	}

	for(i = 0; i < OPA_TYPE_COUNT; i++)
	{
		size[i] = testify_unpack_uint8(handle, "size");
		allignment[i] = testify_unpack_uint8(handle, "allignment");
	}
	if(!testify_retivable_terminated(handle))
	{
		testify_restart_marker_reset(handle);
		return FALSE;
	}
	testify_unpack_string(handle, project->path, 1024, "path");

	testify_restart_marker_release(handle);
	for(i = 0; i < OPA_TYPE_COUNT; i++)
		opa_parse_add_base_type(project, i, size[i], allignment[i], type_names[i]);
	opa_parse_add_base_type(project, OPA_TYPE_UNSINGED_CHAR, sizeof(char), sizeof(char), "boolean");
	opa_parse_add_base_type(project, OPA_TYPE_UNSINGED_INT, sizeof(int), sizeof(int), "STextBlockAlignmentStyle");
	
	buffer = opa_preprosessor(project->path);
	if(buffer != NULL)
	{
		opa_tokenizer(&tokens, buffer, -1);
		opa_parse_source(project, &tokens);	
		opa_type_size_compute(project);
		opa_parse_print(project);
	}
}

extern void opa_options_init(OPADisplayOptions *options);

void opa_request_memory(OPAProject *project, uint64 pointer, uint type, uint parent_id, uint offset, uint indirection, float x, float y)
{
	float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	uint i;

	for(i = 0; i < project->memory_count; i++)
	{
		if(project->memmory[i].pointer == pointer)
		{
			project->memmory[i].hidden = FALSE;
			return;
		}
	}

	if(project->memory_allocated == project->memory_count)
	{
		project->memory_allocated += 16;
		project->memmory = realloc(project->memmory, (sizeof *project->memmory) * project->memory_allocated);
	}
	project->memmory[project->memory_count].pointer = pointer;
	project->memmory[project->memory_count].original_type = type;
	project->memmory[project->memory_count].type = type;
	project->memmory[project->memory_count].indirection = indirection;
	project->memmory[project->memory_count].data = NULL;
	project->memmory[project->memory_count].data_size = project->types[type].size_of;
	for(i = 0; i < 16; i++)
		project->memmory[project->memory_count].matrix[i] = matrix[i];
	project->memmory[project->memory_count].matrix[12] = x;
	project->memmory[project->memory_count].matrix[13] = y;
	project->memmory[project->memory_count].offset = offset;
	project->memmory[project->memory_count].parent = parent_id;
	project->memmory[project->memory_count].line = 0;
	project->memmory[project->memory_count].path[0] = 0;
	project->memmory[project->memory_count].paused = FALSE;
	project->memmory[project->memory_count].hidden = FALSE;
	opa_options_init(&project->memmory[project->memory_count].options, "", 1);
	project->update_current = project->memory_count++;
}

void opa_parse_incomming(OPAProject *project, THandle *handle)
{
	while(testify_retivable(handle, 1))
	{
		testify_restart_marker_set(handle);
		switch(testify_unpack_uint8(handle, "command"))
		{
			case 0 :
			{
				uint i, j;
				uint64 pointer;
				char type_name[256];
				if(testify_retivable(handle, 8))
				{
					pointer = testify_unpack_uint64(handle, "pointer");
					if(testify_unpack_string(handle, type_name, 256, "type_name"))
					{
						testify_restart_marker_release(handle); 
						if(project->memory_count != 0 && project->memmory[0].pointer != pointer)
						{
							for(i = 0; i < project->memory_count; i++)
								if(project->memmory[i].data != NULL)
									free(project->memmory[i].data);
							project->memory_count = 0;
						}

						if(project->memory_count == 0)
						{
							for(i = 0; i < project->type_count; i++)
							{
								printf("%u %s\n", i, project->types[i].type_name);
								for(j = 0; project->types[i].type_name[j] != 0 && project->types[i].type_name[j] == type_name[j]; j++);
								if(project->types[i].type_name[j] == type_name[j])
									break;
							}
							if(i < project->type_count)
							{
								opa_request_memory(project, pointer, i, -1, 0, 0, 0, 0);
							/*	float matrix[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -0.5, 0.5, 0, 1};
								if(project->memory_count == project->memory_allocated)
								{
									project->memory_allocated += 16;
									project->memmory = realloc(project->memmory, (sizeof *project->memmory) * project->memory_allocated);
								}
								project->memmory[project->memory_count].data = NULL;
								project->memmory[project->memory_count].data_size = 0;
								project->memmory[project->memory_count].type = i;
								project->memmory[project->memory_count].pointer = pointer;
								for(j = 0; j < 16; j++)
									project->memmory[project->memory_count].matrix[j] = matrix[j];
								project->memory_count++;
								testify_pack_uint8(handle, 0, "command");
								testify_pack_uint32(handle, project->types[i].size_of, "length");
								testify_pack_uint8(handle, 0, "indirections");
								testify_pack_uint64(handle, pointer,"start");
								testify_network_stream_send_force(handle);*/
							}
						}

					}else
						testify_restart_marker_reset(handle); 
				}else
					testify_restart_marker_reset(handle); 
			}
			break;
			case 1 :
			{
				if(testify_retivable(handle, 8 + 8))
				{
					uint64 pointer, length, i, j, line;
					char path[256];
					pointer = testify_unpack_uint64(handle,  "pointer");
					length = testify_unpack_uint64(handle, "length");
					line = testify_unpack_uint32(handle, "line");	
					if(testify_unpack_string(handle, path, 256, "file"))
					{
						if(testify_retivable(handle, length))
						{
							for(i = 0; i < project->memory_count && project->memmory[i].pointer != pointer; i++);
							if(i == project->memory_count)
							{
								for(j = 0; j < length; j++)
									testify_unpack_uint8(handle, "data");
							}else
							{
								for(j = 0; j < 255 && path[j] != 0; j++)
									project->memmory[i].path[j] = path[j];
								project->memmory[i].path[j] = 0;
								project->memmory[i].line = line;
								if(length != project->memmory[i].data_size || project->memmory[i].data == NULL)
								{
									if(project->memmory[i].data != NULL)
										free(project->memmory[i].data);
									project->memmory[i].data = malloc(length);
									project->memmory[i].data_size = length;
								}
								for(j = 0; j < length; j++)
									project->memmory[i].data[j] = testify_unpack_uint8(handle, "data");
							}
							testify_restart_marker_release(handle); 
						}else
							testify_restart_marker_reset(handle); 
					}
				}else
					testify_restart_marker_reset(handle); 
			}
			break;
			default :
			{
				uint i;
				i = 0;
			}
			break;
		}
	}
}


void opa_parse_retrive_path(OPAProject *project, THandle *handle, uint current)
{
	if(project->memmory[current].parent == -1)
	{
		testify_pack_uint64(handle, project->memmory[current].pointer,"start");
		return;
	}
	opa_parse_retrive_path(project, handle, project->memmory[current].parent);	
	testify_pack_uint64(handle, project->memmory[current].pointer, "pointer");
	testify_pack_uint64(handle, project->memmory[current].offset, "offset");
}

void opa_request_memory_set(OPAProject *project, THandle *handle, uint memory_id, uint offset, uint length, uint8 *data)
{
	uint i, c;

	testify_pack_uint8(handle, 1, "command");
	testify_pack_uint32(handle, length, "length");
	c = memory_id;
	for(i = 0; project->memmory[c].parent != -1; i++)
		c = project->memmory[c].parent;
	testify_pack_uint8(handle, i, "indirections");
	opa_parse_retrive_path(project, handle, memory_id);
	testify_pack_uint32(handle, offset, "offset");
	for(i = 0; i < length; i++)
		testify_pack_uint8(handle, data[i], "data");
	testify_network_stream_send_force(handle);
}

void opa_request_memory_allocate(OPAProject *project, THandle *handle, uint memory_id, uint offset, uint length)
{
	uint i, c;

	testify_pack_uint8(handle, 2, "command");
	testify_pack_uint32(handle, length, "length");
	c = memory_id;
	for(i = 0; project->memmory[c].parent != -1; i++)
		c = project->memmory[c].parent;
	testify_pack_uint8(handle, i, "indirections");
	opa_parse_retrive_path(project, handle, memory_id);
	testify_pack_uint32(handle, offset, "offset");
	testify_network_stream_send_force(handle);
}

void opa_parse_retrive(OPAProject *project, THandle *handle)
{
	static uint counter = 0;
	uint i, c;
	counter++;
	if(counter < 100)
		return;
	if(project->memory_count == 0)
		return;
	counter = 0;
	project->update_current %= project->memory_count;
	c = project->update_current;
	if(!project->memmory[c].paused)
	{
		testify_pack_uint8(handle, 0, "command");
		testify_pack_uint32(handle, project->memmory[c].data_size, "length");
		for(i = 0; project->memmory[c].parent != -1; i++)	
			c = project->memmory[c].parent;
		testify_pack_uint8(handle, i, "indirections");
		opa_parse_retrive_path(project, handle, project->update_current);
		testify_network_stream_send_force(handle);
		project->update_current++;
	}
}