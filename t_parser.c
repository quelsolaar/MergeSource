

#include "forge.h"
#include "testify.h"

#define TESTITY_POINTER_BIT 		0x10000000
#define TESTITY_DYNAMIC_LENGTH_BIT 0x20000000
#define TESTITY_DEFINED_TYPE_BIT 0x40000000
#define TESTITY_TYPE_BIT_MASK 0x0FFFFFFF

#define TESTITY_MEMBER_NAME_MAX_LENGTH 1024

typedef unsigned int uint;

typedef struct{
	char name[TESTITY_MEMBER_NAME_MAX_LENGTH];
	uint array;
	char *enabler;
	char *pointer;
	UTypes type;
	uint offset;
	void *next;
}TMember;

typedef struct{
	char name[16];
	TMember *member_first;
	boolean uses_allocations;
}TStruct;


typedef struct{
	char *string;
	UTypes type;
}StringType;



StringType testify_type_table[] = {"_Bool", T_TYPE_UINT8,
									 "char", T_TYPE_STRING,
									 "short", T_TYPE_INT16,
									 "short int", T_TYPE_INT16,
									 "int", T_TYPE_INT32,
									 "long", T_TYPE_INT32,
									 "long int", T_TYPE_INT32,
									 "long long", T_TYPE_INT64,
									 "long long int", T_TYPE_INT64,
									 "size_t", T_TYPE_UINT64,
									 "ssize_t", T_TYPE_INT64,
									 "ptrdiff_t", T_TYPE_INT64,
									 "uintptr_t", T_TYPE_UINT64,
									 "uint8_t", T_TYPE_UINT8, 
									 "uint16_t", T_TYPE_UINT16, 
									 "uint32_t", T_TYPE_UINT32,
									 "uint64_t", T_TYPE_UINT64,
									 "int8_t", T_TYPE_INT8,
									 "int16_t", T_TYPE_INT16,
									 "int32_t", T_TYPE_INT32,
									 "int64_t", T_TYPE_INT64,
									 "uint8", T_TYPE_UINT8, 
									 "uint16", T_TYPE_UINT16, 
									 "uint32", T_TYPE_UINT32,  
									 "uint64", T_TYPE_UINT64,  
									 "int8", T_TYPE_INT8,
									 "int16", T_TYPE_INT16,
									 "int32", T_TYPE_INT32,
									 "int64", T_TYPE_INT64,
									 "float", T_TYPE_REAL32,
									 "double", T_TYPE_REAL64,
									 "real32", T_TYPE_REAL32,
									 "float64", T_TYPE_REAL64};

 /* 	T_TYPE_UINT8,
	T_TYPE_INT8,
	T_TYPE_UINT16,
	T_TYPE_INT16,
	T_TYPE_UINT32,
	T_TYPE_INT32,
	T_TYPE_UINT64,
	T_TYPE_INT64,
	T_TYPE_REAL32,
	T_TYPE_REAL64,
	T_TYPE_STRING,
	T_TYPE_STRUCT,
	T_TYPE_COUNT*/

boolean testify_word_compare(char *text, uint *pos, char *word)
{
	uint i;
	text = &text[*pos];
	for(i = 0; text[i] == word[i] && text[i] != 0; i++);
	if(word[i] == 0)
	{
		*pos += i;
		return TRUE; 
	}
	return FALSE;
}

uint testify_whitespace_skip(char *text, uint i)
{
	while(text[i] != 0)
	{
		if(text[i] > ' ')
		{
			if(text[i] == '/')
			{
				if(text[i + 1] == '/')
				{
					while(text[i] != 0 && text[i] != '\n')
						i++;
					if(text[i] == 0)
						return i;
				}
				else if(text[i + 1] == '*')
				{
					while(text[i + 1] != 0 && (text[i] != '*' || text[i + 1] != '/'))
						i++;
					if(text[i + 1] == 0)
						return i + 1;
					i++;
				}
				else
					return i;				
			}else
				return i; 
		}
		i++;
	}
	return i;
}

uint testify_parse_enabler(char *text)
{
	uint i, braces = 1;
	if(*text == '(')
	{
		for(i = 1; text[i] != 0; i++)
		{
			if(text[i] == '(')
				braces++;
			if(text[i] == ')')
				if(--braces == 0)
					return i + 1;
		}
		printf("TESTIFY ERROR: Missing ).\n");
		exit(0);
	}
	return 0;
}

uint testify_type_parse(TStruct *structs, uint *struct_count, TMember *members, uint *member_count, char *text)
{
	TStruct *current_struct;
	TMember *current_member, *last_member;
	uint i, j, is_signed = FALSE, is_unsigned = FALSE;

	current_member = &members[*member_count];
	current_member->enabler = NULL;
	i = testify_whitespace_skip(text, 0);

	if((j = testify_parse_enabler(&text[i])) != 0)
	{
		current_member->enabler = &text[i];
		i += j;
	}

	while(TRUE) /* quialifyers */
	{
		if(testify_word_compare(text, &i, "signed"))
				is_signed = TRUE;
		else if(testify_word_compare(text, &i, "unsigned"))
				is_unsigned = TRUE;
		else if(!testify_word_compare(text, &i, "const") &&
				!testify_word_compare(text, &i, "volotile") &&
				!testify_word_compare(text, &i, "static") &&
				!testify_word_compare(text, &i, "auto") &&
				!testify_word_compare(text, &i, "typedef") &&
				!testify_word_compare(text, &i, "register"))
			break;	
		while(text[i] <= ' ')
			i++;
	}

	if(testify_word_compare(text, &i, "struct"))
	{
		current_struct = &structs[*struct_count];
		(*struct_count)++;
		current_struct->name[0] = 0;
		current_struct->member_first = NULL;
		i = testify_whitespace_skip(text, i);
		if(text[i] != '{')
		{
			for(j = 0; text[j + i] > ' ' && text[j + i] != '{'; j++);
				current_struct->name[j] = text[j + i];
			current_struct->name[j] = 0;
			i = testify_whitespace_skip(text, i);	
			if(text[i] != '{')
			{	
				printf("TESTIFY ERROR: Expected { after %s.\n", current_struct->name);
				exit(0);
			}						
		}
		i++;
		while(text[i] != '}')
		{
			if(text[i] == 0)
			{	
				printf("TESTIFY ERROR: Un expected end of string. Missing }.\n");
				exit(0);
			}	
			i = testify_whitespace_skip(text, i);
			i += testify_type_parse(structs, struct_count, members, member_count, &text[i]);
			if(current_struct->member_first == NULL)
				last_member = current_struct->member_first = &members[*member_count - 1];
			else
			{
				last_member->next = &members[*member_count - 1];
				last_member = last_member->next;
			}	
			last_member->next = NULL;
			i = testify_whitespace_skip(text, i);	
		}
		i++;
		i = testify_whitespace_skip(text, i);
		if(text[i] != ';')
		{
			for(j = 0; text[i] > ' ' && text[i] != ';'; j++)
				current_struct->name[j] = text[i++];
			current_struct->name[j] = 0;
			i = testify_whitespace_skip(text, i);			
			if(text[i] != ';')
			{	
				printf("TESTIFY ERROR: Expected semicolon after \"%s\".\n", current_struct->name);
				exit(0);
			}			
		}
		current_struct->uses_allocations = FALSE;
		for(current_member = current_struct->member_first; current_member != NULL; current_member = current_member->next)
		{
			if((current_member->pointer != NULL && current_member->name[0] != '"' && (current_member->name[0] < '0' || current_member->name[0] > '9')) || (current_member->type >= T_TYPE_STRUCT && structs[current_member->type - T_TYPE_STRUCT].uses_allocations))
			{
				current_struct->uses_allocations = TRUE;
				break;
			}
		}
	}else
	{
		i = testify_whitespace_skip(text, i);	
		(*member_count)++;
		current_member->array = 1;
		current_member->pointer = NULL;
		current_member->name[0] = 0;
		current_member->next = NULL; 
		current_member->offset = 0;
		current_member->type = 0;

		for(j = 0; j < 33 && !testify_word_compare(text, &i, testify_type_table[j].string); j++);
		if(j == 33)
		{
			char error_buffer[32];
			for(j = 0; j < *struct_count; j++)
			{
				if(testify_word_compare(text, &i, structs[j].name))
				{
					current_member->type = T_TYPE_STRUCT + j;
					break;
				}
			}
			if(j == *struct_count)
			{
				for(j = 0; j < 31 && text[i + j] > ' '; j++)
					error_buffer[j] = text[i + j];
				error_buffer[j] = 0;			
				printf("TESTIFY ERROR: Undefined type\"%s\".\n", error_buffer);
				exit(0);
			}
		}else
		{
			current_member->type = testify_type_table[j].type;
			if(is_signed && ((current_member->type < T_TYPE_INT64 && current_member->type % 2 == 0)))
				current_member->type++;
			if(is_unsigned && ((current_member->type <= T_TYPE_INT64 && current_member->type % 2 == 1)))
				current_member->type--;
		}
		i = testify_whitespace_skip(text, i);
		

		if(text[i] == '*')
		{
			i++;			
			if(current_member->type != T_TYPE_STRING)
			{
				if(text[i] == '(')
				{
					current_member->pointer = &text[i];
					i += testify_parse_enabler(&text[i]);
					if(text[i] == 0)
					{
						printf("TESTIFY ERROR: Missing ).\n");
						exit(0);
					}
				}
			}else
				current_member->pointer = "";
		}
		i = testify_whitespace_skip(text, i);
		if((text[i] < 'A' || text[i] > 'Z') && (text[i] < 'a' || text[i] > 'z') && (text[i] < '0' || text[i] > '9') && text[i] != '"')
		{
			char error[2] = {0, 0};
			error[0] = text[i];
			printf("TESTIFY ERROR: Unexpected charcter: \"%s\".\n", error);
			exit(0);			
		}
		if(text[i] == '"')
		{
			current_member->name[0] = text[i++];
			for(j = 1; j < TESTITY_MEMBER_NAME_MAX_LENGTH - 2 && text[i] != '"'; j++)
				current_member->name[j] = text[i++];
			current_member->name[j++] = text[i++];
			current_member->name[j] = 0;
		}else
		{
			for(j = 0; j < TESTITY_MEMBER_NAME_MAX_LENGTH - 2 && (text[i] >= 'A' && text[i] <= 'Z') || (text[i] >= 'a' && text[i] <= 'z') || (text[i] >= '0' && text[i] <= '9') || text[i] == '_'; j++)
				current_member->name[j] = text[i++];
			current_member->name[j] = 0;
		}
		i = testify_whitespace_skip(text, i);
		if(text[i] == '[')
		{
			uint64 array;
			i = testify_whitespace_skip(text, ++i);
			if(text[i] < '0' || text[i] > '9')
			{
				printf("TESTIFY ERROR: Values between brackets of member %s are not numbers\n", current_member->name);
				exit(0);	
			}
			i += f_text_parce_decimal(&text[i], &array);
			i = testify_whitespace_skip(text, i);
			if(text[i] != ']')
			{
				printf("TESTIFY ERROR: Expected end bracket at end of member %s\n", current_member->name);
				exit(0);
			}
			i++;
			current_member->array = array;
		}			
	}
	i = testify_whitespace_skip(text, i);
	if(text[i] != ';')
	{
		printf("TESTIFY ERROR: Expected semicolon at end of member %s\n", current_member->name);
		exit(0);
	}
	i++;
	return testify_whitespace_skip(text, i);
}
/*
void testify_struct_sizeof(TStruct *structs, uint count)
{
	TMember *member;
	uint i, offset, size, alignment;
	for(i = 0; i < count; i++)
	{
		offset = 0;
		structs[i].size_of = 0;
		structs[i].alignment = 1;
		for(member = structs[i].member_first; member != NULL; member = member->next)
		{
			member->offset = offset;
			if(member->type & TESTITY_POINTER_BIT)
				size = alignment = sizeof(void *);
			else if((member->type & TESTITY_TYPE_BIT_MASK) >= T_TYPE_STRUCT)
			{
				size = structs[(member->type & TESTITY_TYPE_BIT_MASK) - T_TYPE_STRUCT].size_of;
				alignment = structs[(member->type & TESTITY_TYPE_BIT_MASK) - T_TYPE_STRUCT].alignment;
			}else 
			{
				uint type_sizes[] = {sizeof(uint8), // T_TYPE_UINT8
									sizeof(int8), // T_TYPE_INT8
									sizeof(uint16), // T_TYPE_UINT16
									sizeof(int16), // T_TYPE_INT16
									sizeof(uint32), // T_TYPE_UINT32
									sizeof(int32), // T_TYPE_INT32
									sizeof(uint64), // T_TYPE_UINT64
									sizeof(int64), // T_TYPE_INT64
									sizeof(float), // T_TYPE_REAL32
									sizeof(double), // T_TYPE_REAL64
									sizeof(uint8)}; // T_TYPE_STRING
				size = alignment = type_sizes[member->type & TESTITY_TYPE_BIT_MASK];
			}
			if(alignment > structs[i].alignment)
				structs[i].alignment = alignment;
			size *= member->array;
			offset += size;
		}
		structs[i].size_of = ((offset - 1 + structs[i].alignment) / structs[i].alignment) * structs[i].alignment;
	}
}
*/
void testify_struct_mem_alloc(char *text, TStruct **structs, TMember **members)
{
	uint i, member_count = 1, struct_count;
	for(i = struct_count = 0; text[i] != 0; i++)
	{
		if(text[i] == ';')
			member_count++;
		if(text[i] == 's' && testify_word_compare(text, &i, "struct"))
			struct_count++;
		if(text[i] == 'e' && testify_word_compare(text, &i, "enum"))
			struct_count++;	
	}
	*structs = malloc(sizeof(TStruct) * (struct_count) + sizeof(TMember) * (member_count - struct_count));
	*members = (TMember *)&(*structs)[struct_count];
}




char *testify_struct_test_string = "typedef struct{\n"
									"	char *\"Hello World\";\n"
									"	char *\"Hey boy Hey girl\"[12];\n"
									"	uint32 1337;\n"
									"	char *text_a;\n"
									"	char text_b[97];\n"
									"	char *text_c[97];\n"
									"	int	a;\n"
									"	float *(a * 56 + a)b[65];\n"
									"	float *(88)foo;\n"
									"	(a == 0)float bar[2];\n"
									"}MyOther;\n\n"
									"typedef struct{\n"
									"	int	a;\n"
									"	MyOther *b;\n"
									"}MyTypedef;\n"
									"typedef struct{\n"
									"	int	a;\n"
									"	int	x;\n"
									"	int	y;\n"
									"	char *\"Hello World\";"
									"	float b[324];\n"
									"	char c[324];\n"
									"}MyFinalTypedef;\n";
/**typedef struct{
	... members;
	... members;
	... members;
}a;


typedef struct{
	... members;
	... members;
	struct{
		... members;
		... members;
	}b;
	... members;
	... members;
}c;*/

char *testify_struct_name(char *name)
{
	char *buffer;
	uint i, j;
	for(i = j = 0; name[i] != 0; i++)
		if(name[i] >= 'A' && name[i] <= 'Z')
			j++;
	buffer = malloc(i + j + 1);
	for(i = j = 0; name[i] != 0; i++)
	{
		if(name[i] >= 'A' && name[i] <= 'Z')
		{
			buffer[j++] = '_';
			buffer[j++] = name[i] - 'A' + 'a'; 
		}else
			buffer[j++] = name[i];
	}
	buffer[j] = 0;
	return buffer;
}

char *testify_expression(TStruct *structs, char *buffer, uint buffer_length)
{
	char *output;
	TMember *m;
	uint i, j, k;
	output = malloc(buffer_length * 4 + 1);
	for(i = j = 0; buffer[i] != 0 && i < buffer_length; i++)
	{
		for(m = structs->member_first; m != NULL; m = m->next)
		{
			for(k = 0; m->name[k] == buffer[i + k]; k++);
			if(m->name[k] == 0 && (buffer[i + k] < 'A' || buffer[i + k] > 'Z') && (buffer[i + k] < 'a' || buffer[i + k] > 'z') && buffer[i + k] != '_')
			{
				output[j++] = 'p';
				output[j++] = '-';
				output[j++] = '>';
				break;
			}					
		}
		output[j++] = buffer[i];
	}
	output[j] = 0;
	return output;
}

char *type_names[] = {"uint8", "int8", "uint16", "int16", "uint32", "int32", "uint64", "int64", "real32", "real64", "string"};



void testify_struct_struct_print(FILE *f, TStruct *structs, TStruct *struct_current)
{
	TMember *members;
	fprintf(f, "typedef struct{\n");
	for(members = struct_current->member_first; members != NULL; members = members->next)
	{
		if(members->name[0] != '"' && (members->name[0] < '0' || members->name[0] > '9'))
		{
			if(members->type < T_TYPE_STRING)
				fprintf(f, "\t%s ", type_names[members->type]);
			else if(members->type == T_TYPE_STRING)
				fprintf(f, "\tchar ");
			else
				fprintf(f, "\t%s ", structs[members->type - T_TYPE_STRUCT].name);	
			if(members->pointer != NULL)
				fprintf(f, "*");	
			if(members->array != 1)	
				fprintf(f, "%s[%u];\n", members->name, members->array);	
			else
				fprintf(f, "%s;\n", members->name);	
		}
	}
	fprintf(f, "}%s;\n\n", struct_current->name);
}


void testify_struct_code_print(FILE *f, TStruct *structs, TStruct *struct_current, char *prefix, boolean load)
{
	char *tabs = "\t\t\t\t\t";
	TMember *members, *m2;
	char *expression;
	uint i, j, k, tab_current;

	expression = testify_struct_name(struct_current->name);
	if(load)
	{
		if(struct_current->uses_allocations)
			fprintf(f, "%s *%s%s_load_internal(THandle *handle, %s *p, void ***pointers, uint *pointers_used)\n{\n", struct_current->name, prefix, expression, struct_current->name);
		else
			fprintf(f, "%s *%s%s_load_internal(THandle *handle, %s *p)\n{\n", struct_current->name, prefix, expression, struct_current->name);
	}else
		fprintf(f, "void %s%s_save(THandle *handle, %s *p)\n{\n", prefix, expression, struct_current->name);
	free(expression);
	for(members = struct_current->member_first; members != NULL && members->array == 1; members = members->next);
	if(members != NULL)
		fprintf(f, "\tuint array, array_length;\n");
	for(members = struct_current->member_first; members != NULL && members->pointer == NULL; members = members->next);
	if(members != NULL)
		fprintf(f, "\tuint pointer, pointer_length;\n");


	for(members = struct_current->member_first; members != NULL; members = members->next)
	{
		tab_current = 4;
		if(members->enabler != NULL)
		{
			expression = testify_expression(structs, members->enabler, testify_parse_enabler(members->enabler));
			fprintf(f, "\tif%s\n", expression);
			if(members->pointer != NULL)
				fprintf(f, "\t{\n");
			free(expression);
			tab_current--;
		}

		if(members->type < T_TYPE_STRING && load)
		{
			fprintf(f, "%sif(!testify_retivable(handle, sizeof(%s)", &tabs[tab_current], type_names[members->type]);			
			if(members->array != 1)
				fprintf(f, " * %u", members->array);
			if(members->pointer != NULL)
			{
				expression = testify_expression(structs, &members->pointer[1], testify_parse_enabler(members->pointer) - 2);
				fprintf(f, " * %s", expression);
			}
			fprintf(f, "))\n%s\treturn NULL;\n", &tabs[tab_current]);
		}
		if(members->array != 1 && (members->type != T_TYPE_STRING || members->pointer != NULL))
		{
			fprintf(f, "%sfor(array = 0; array < %u; array++)\n", &tabs[tab_current], members->array);
			if(members->pointer != NULL)
				fprintf(f, "%s{\n", &tabs[tab_current]);
			tab_current--;
		}
		if(members->pointer != NULL && members->type != T_TYPE_STRING)
		{
			expression = testify_expression(structs, &members->pointer[1], testify_parse_enabler(members->pointer) - 2);
			fprintf(f, "%spointer_length = %s;\n", &tabs[tab_current], expression);
			free(expression);

			if(load)
			{				
				fprintf(f, "%sif(*pointers_used %% 256 == 0)\n", &tabs[tab_current]);
				fprintf(f, "%s\t*pointers = realloc(*pointers, (sizeof *pointers) * (*pointers_used + 256));\n", &tabs[tab_current]);
				if(members->array != 1)
					fprintf(f, "%sp->%s[array] = (*pointers)[(*pointers_used)++] = malloc((sizeof *p->%s[0]) * pointer_length);\n", &tabs[tab_current], members->name, members->name);
				else
					fprintf(f, "%sp->%s = (*pointers)[(*pointers_used)++] = malloc((sizeof *p->%s) * pointer_length);\n", &tabs[tab_current], members->name, members->name);
			}

			fprintf(f, "%sfor(pointer = 0; pointer < pointer_length; pointer++)\n", &tabs[tab_current]);
			tab_current--;
		}		
		if(members->type == T_TYPE_STRING && load)
			fprintf(f, "%sif(!testify_retivable_terminated(handle))\n%s\treturn NULL;\n", &tabs[tab_current], &tabs[tab_current]);

		if(members->type >= T_TYPE_STRUCT)
		{
			expression = testify_struct_name(structs[members->type - T_TYPE_STRUCT].name);
			if(load)
				fprintf(f, "%s%s%s_load_internal(handle, &p->%s", &tabs[tab_current], prefix, expression, members->name);
			else
				fprintf(f, "%s%s%s_save(handle, &p->%s", &tabs[tab_current], prefix, expression, members->name);
			free(expression);	
		}else if(!load)
		{
			if(members->name[0] == '"' || (members->name[0] >= '0' && members->name[0] <= '9'))
				fprintf(f, "%stestify_pack_%s(handle, %s", &tabs[tab_current], type_names[members->type], members->name);
			else
				fprintf(f, "%stestify_pack_%s(handle, p->%s", &tabs[tab_current], type_names[members->type], members->name);
		}else if(members->type == T_TYPE_STRING && members->pointer == NULL)
		{
			if(members->name[0] != '"')
				fprintf(f, "%stestify_unpack_string(handle, p->%s", &tabs[tab_current],  members->name);
			else
				fprintf(f, "%stestify_unpack_string(handle, NULL", &tabs[tab_current]);	
		}else if(members->name[0] != '"' && (members->name[0] < '0' || members->name[0] > '9'))
		{

			fprintf(f, "%sp->%s", &tabs[tab_current],  members->name);
		}	
		if(members->name[0] != '"' && (members->name[0] < '0' || members->name[0] > '9'))
		{
			if(members->array != 1 && (members->type != T_TYPE_STRING || members->pointer != NULL))
				fprintf(f, "[array]");	
			if(members->pointer != NULL && members->type != T_TYPE_STRING)
				fprintf(f, "[pointer]");
		}
		if(members->type >= T_TYPE_STRUCT)	
		{
			if(load && structs[members->type - T_TYPE_STRUCT].uses_allocations)
				fprintf(f, ", pointers, pointers_used);\n");	
			else
				fprintf(f, ");\n");	
		}else 
		{
			if(load)
			{
				if(members->type == T_TYPE_STRING)
				{
					if(members->pointer == NULL)	
						fprintf(f, ", %u", members->array);
					else
					{
						if(members->name[0] == '"')
							fprintf(f, "%stestify_unpack_string(handle, NULL, 0", &tabs[tab_current]);
						else
							fprintf(f, " = testify_unpack_string_allocate(handle");
					}
				}else
				{	
					if(members->name[0] != '"' && (members->name[0] < '0' || members->name[0] > '9'))
						fprintf(f, " = testify_unpack_%s(handle", type_names[members->type]);
					else
						fprintf(f, "%stestify_unpack_%s(handle", &tabs[tab_current], type_names[members->type]);
				}
			}
			if(members->name[0] == '"')
				fprintf(f, ", %s);\n", members->name);
			else
				fprintf(f, ", \"%s\");\n", members->name);
		}

		if(members->pointer != NULL && members->type != T_TYPE_STRING)
			tab_current++;
		if(members->array != 1)
		{		
			tab_current++;
			if(members->pointer != NULL)
				fprintf(f, "%s}\n", &tabs[tab_current]);

		}
		if(members->enabler != NULL && members->pointer != NULL)
			fprintf(f, "\t}\n");
	}
	if(load)
		fprintf(f, "\treturn p;\n");


	fprintf(f, "}\n\n");

	expression = testify_struct_name(struct_current->name);

	if(load)
	{
		fprintf(f, "%s *%s%s_load(THandle *handle, %s *p)\n{\n", struct_current->name, prefix, expression, struct_current->name);
		fprintf(f, "\tvoid **pointers;\n");
		fprintf(f, "\tuint pointers_used = 0, i;\n");
		fprintf(f, "\ttestify_restart_marker_set(handle);\n");		
		fprintf(f, "\tif(p == NULL)\n\t{\n");
		fprintf(f, "\t\tpointers = realloc(pointers, (sizeof pointers) * (pointers_used + 256));\n");
		fprintf(f, "\t\tp = pointers[pointers_used++] = malloc(sizeof *p);\n\t}\n");			
		fprintf(f, "\tp = %s%s_load_internal(handle, p, &pointers, &pointers_used);\n",  prefix, expression);
		fprintf(f, "\tif(p == NULL)\n");
		fprintf(f, "\t\tfor(i = 0; i < pointers_used; i++)\n");
		fprintf(f, "\t\t\tfree(pointers[i]);\n");
		fprintf(f, "\tfree(pointers);\n");
		fprintf(f, "\tif(p != NULL)\n");
		fprintf(f, "\t\ttestify_restart_marker_release(handle);\n");
		fprintf(f, "\telse\n");
		fprintf(f, "\t\ttestify_restart_marker_reset(handle);\n");
		fprintf(f, "\treturn p;\n");
		fprintf(f, "}\n\n");
	}
	free(expression);
}


void testify_struct_generate(char *path, char *struct_definition, char *prefix)
{
	FILE *file;
	TStruct *structs = NULL;
	TMember *members = NULL;
	uint i, struct_count = 0, members_count = 0;
	return;
	printf("testing string:\n%s\n", struct_definition);
	testify_struct_mem_alloc(struct_definition, &structs, &members);
	for(i = 0; struct_definition[i] != 0; i += testify_type_parse(structs, &struct_count, members, &members_count, &struct_definition[i]));
//	testify_struct_sizeof(structs, struct_count);
	file = fopen(path, "w");
	if(file != NULL)
	{
		fprintf(file, "#include \"testify.h\"\n\n");
		fprintf(file, "\n/*\n%s\n*/\n", struct_definition);		
		for(i = 0; i < struct_count; i++)
			testify_struct_struct_print(file, structs, &structs[i]);
		for(i = 0; i < struct_count; i++)
		{
			testify_struct_code_print(file, structs, &structs[i], prefix, TRUE);
			testify_struct_code_print(file, structs, &structs[i], prefix, FALSE);
		}
		fclose(file);
	}
	exit(0);
}