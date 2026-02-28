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


char *opa_languange_keywords[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"};

/*
typedef struct{
	TokenType type;
	uint start;
	uint length;
	union{
		double real;
		int64 integer;
	}value;
	void *next;
	void *prev;
}Token;


typedef struct{
	Token *tokens;
	uint token_count;
	uint8 *buffer;
}TokenFile;*/


char *opa_tokenizer_string_parser(uint8 *characters, uint *length, uint *forward)
{
	char *out;
	uint i, j, k;


	for(*length = i = 0; ; i++)
	{
		if(characters[i] == '\"' && characters[i - 1] != '\\')
		{
			j = i + 1;
			while(characters[j] <= ' ' && characters[j] != 0)
				j++;
			if(characters[j] != '\"')
				break;
			i = j;
			(*length)--;
		}
		if(characters[i] == '\'' && characters[i - 1] != '\\')
			break;
		if(characters[i] == '\\' && characters[i - 1] != '\\')
		{
			if(characters[i + 1] >= '0' && characters[i + 1] <= '9')
			{
				char buffer[4];
				uint64 number = 0;
				for(j = 0; j < 3 && characters[i + 1 + j] >= '0' && characters[i + 1 + j] <= '9'; j++)
					buffer[j] = characters[i + 1 + j];
				f_text_parce_decimal(&characters[i + 2], &number);
				i += j;

			}else if(characters[i + 1] == 'u')
			{
				uint64 number = 0;
				i += f_text_parce_decimal(&characters[i + 2], &number);

			}else if(characters[i + 1] == 'x')
			{
				uint64 number = 0;
				i += f_text_parce_hex(&characters[i + 2], &number);
			}
			(*length)--;
		}
		(*length)++;
	}
	*forward = i + 1;
	if(i == *length)
		return &characters[1];
	out = malloc(*length + 1);

	for(k = i = 0; ; i++)
	{
		if(characters[i] == '\'' && characters[i - 1] != '\\')
			break;
		if(characters[i] == '\"' && characters[i - 1] != '\\')
		{
			j = i + 1;
			while(characters[j] <= ' ' && characters[j] != 0)
				j++;
			if(characters[j] != '\"')
				break;
			i = j;
		}else
		{
			if(characters[i] != '\\')
				out[k++] = characters[i];
			else
			{
				i++;
				if(characters[i] == 0)
					break;
				if(characters[i] == 'a')
					out[k++] = '\a';
				else if(characters[i] == 'b')
					out[k++] = '\b';
				else if(characters[i] == 'f')
					out[k++] = '\f';
				else if(characters[i] == 'n')
					out[k++] = '\n';
				else if(characters[i] == 'r')
					out[k++] = '\r';
				else if(characters[i] == 't')
					out[k++] = '\t';
				else if(characters[i] == 'u')
				{
					uint64 number = 0;
					i++;
					i += f_text_parce_decimal(&characters[i], &number) - 1;
					out[k++] = number;
				}else if(characters[i] == 'v')
					out[k++] = '\v';
				else if(characters[i] == 'x')
				{
					uint64 number = 0;
					i++;
					i += f_text_parce_hex(&characters[i], &number) - 1;
					out[k++] = number;
				}
				else if(characters[i] >= '0' && characters[i] <= '9')
				{
					char buffer[4];
					uint64 number = 0;
					for(j = 0; j < 3 && characters[i + j] >= '0' && characters[i + j] <= '9'; j++)
						buffer[j] = characters[i + j];
					buffer[j] = 0;
					f_text_parce_decimal(buffer, &number);
					i += j - 1;
					out[k++] = number;
				}else
					out[k++] = characters[i];
			}
		}
	}
	out[k] = 0;
	return out;
}

int main2()
{
	uint length, forward, i;
	char *text = "\"hello\\65G\"   \"hello\\x65G\"";
	char *out;
	uint64 integer_output;
	double real_output;
	boolean decimal;
	f_text_parce_real("76534.76", &integer_output, &real_output, &decimal);

	out = opa_tokenizer_string_parser(&text[1], &length, &forward);
	i = 0;
	{
		FILE *f;
		char *list[9] = {"\a", "\b", "\f", "\n", "\r", "\t", "\65", "\v", "\x8"};
		char *letter[9] = {"a", "b", "f", "n", "r", "t", "u", "v", "x"};
		f = fopen("Character_coneversion_list.txt", "w");
			fprintf(f, "string -%s- -%s-\n", text, out);


		for(i = 0; i < 9; i++)
		{
			fprintf(f, "charater[%u] %s %u\n", i + 'a', letter[i], (uint)list[i][0]);
		}
		fclose(f);
	}

}



void opa_tokenizer_tokenizer(OPATokenFile *tokens, uint8 *buffer, uint buffer_size)
{
	uint i, j, type, current = -1, line_number = 0;
	OPAToken *t;
	tokens->token_count = 0;
	tokens->buffer = buffer;
	tokens->tokens = malloc((sizeof *tokens->tokens) * 256);
	t = &tokens->tokens[tokens->token_count];
	t->line = line_number;
	t->type = OPA_TT_SPACE;
	t->start = i;
	t->length = 1;
	for(i = 0; buffer[i] != 0 && i < buffer_size; i++)
	{
	//	i += opa_tokenizer_debug(&buffer[i], &line_number);
		if(OPA_TT_SPACE != type)
		{
			t = &tokens->tokens[tokens->token_count++];
			t->line = line_number;
			t->type = OPA_TT_SPACE;
			t->start = i;
			t->length = 1;
		}
		if(buffer[i] > ' ')
		{
			if(buffer[i] >= 0 && buffer[i] <= 9)
			{
				boolean decimal;
				j = f_text_parce_real(&buffer[i] , &t->value.integer, &t->value.real, decimal);
				t->text = &buffer[i];
				t->length = j;
				i += j;
				if(decimal)
					t->type = OPA_TT_REAL;
				else
					t->type = OPA_TT_INTEGER;
			}else
			{
				if((buffer[i] >= 'A' && buffer[i] <= 'Z') ||
					(buffer[i] >= 'a' && buffer[i] <= 'z') ||
					buffer[i] == '_' )
				{
					t->text = &buffer[i];
					for(t->length = 0; (buffer[i] >= 'A' && buffer[i] <= 'Z') ||
										(buffer[i] >= 'a' && buffer[i] <= 'z') ||
										(buffer[i] >= '0' && buffer[i] <= '9') ||
										buffer[i] == '_'; t->length++);
					t->type = OPA_TT_NAME;
				}else
				{
					switch(buffer[i])
					{
						case '=' :
							if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_EQUAL;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_ASSIGN;
							}
						break;
						case '!' :
							if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_NOT;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_NOT_ASSIGN;
							}
						break;
						case '+' :
							if(buffer[i + 1] == '+')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_PLUS_PLUS;
							}else if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_PLUS_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_PLUS;
							}
						break;
						case '-' :
							if(buffer[i + 1] == '-')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MINUS_MINUS;
							}else if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MINUS_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MINUS;
							}
						break;
						case '*' :
							if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MULTIPLY_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MULTIPLY;
							}
						break;
						case '/' :
							if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_DIVIDE_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_DIVIDE;
							}
						break;
						case '%' :
							if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MODULO_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MODULO;
							}
						break;
						case '|' :
							if(buffer[i + 1] == '|')
							{
								if(buffer[i + 2] == '=')
								{
									i++;
									t->length = j;
									t->type = OPA_TT_OPERATOR_OR_OR_ASSIGN;
								}else
								{
									i++;
									t->length = j;
									t->type = OPA_TT_OPERATOR_OR_OR;
								}
							}else if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_OR_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_OR;
							}
						break;
						case '&' :
							if(buffer[i + 1] == '&')
							{
								if(buffer[i + 2] == '=')
								{
									i++;
									t->length = j;
									t->type = OPA_TT_OPERATOR_AND_AND_ASSIGN;
								}else
								{
									i++;
									t->length = j;
									t->type = OPA_TT_OPERATOR_AND_AND;
								}
							}else if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_AND_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_AND;
							}
						break;
						case '<' :
							if(buffer[i + 1] == '<')
							{
								if(buffer[i + 2] == '=')
								{
									i++;
									t->length = j;
									t->type = OPA_TT_OPERATOR_LEFT_SHIFT_ASSIGN;
								}else
								{
									i++;
									t->length = j;
									t->type = OPA_TT_OPERATOR_LEFT_SHIFT;
								}
							}else if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_LESS_THEN_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_LESS_THEN;
							}
						break;
						case '>' :
							if(buffer[i + 1] == '>')
							{
								if(buffer[i + 2] == '=')
								{
									i++;
									t->length = j;
							//		t->type = OPA_TT_OPERATOR_RIGHT_SHIFT_ASSIGN;
								}else
								{
									i++;
									t->length = j;
									t->type = OPA_TT_OPERATOR_RIGHT_SHIFT;
								}
							}else if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MORE_THEN_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_MORE_THEN;
							}
						break;
						case '^' :
							if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_XOR_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_XOR;
							}
						break;
						case '?' :
							if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_QUESTION_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_QUESTION;
							}
						break;
						case '~' :
							if(buffer[i + 1] == '=')
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_BITWISE_NOT_ASSIGN;
							}else
							{
								i++;
								t->length = j;
								t->type = OPA_TT_OPERATOR_BITWIZE_NOT;
							}
						break;
						case ';' :
							i++;
							t->length = j;
							t->type = OPA_TT_SEMICOLON;
						break;
						case ':' :
							i++;
							t->length = j;
							t->type = OPA_TT_COLON;
						break;
						case ',' :
							i++;
							t->length = j;
							t->type = OPA_TT_COMMA;
						break;
						case '.' :
							i++;
							t->length = j;
							t->type = OPA_TT_PERIOD;
						break;
						case '(' :
							i++;
							t->length = j;
							t->type = OPA_TT_OPEN_PARENTHESIS;
						break;
						case ')' :
							i++;
							t->length = j;
							t->type = OPA_TT_CLOSE_PARENTHESIS;
						break;
						case '{' :
							i++;
							t->length = j;
							t->type = OPA_TT_OPEN_SCOPE;
						break;
						case '}' :
							i++;
							t->length = j;
							t->type = OPA_TT_CLOSE_SCOPE;
						break;
						case '[' :
							i++;
							t->length = j;
							t->type = OPA_TT_OPEN_BRACKET;
						break;
						case ']' :
							i++;
							t->length = j;
							t->type = OPA_TT_CLOSE_BRACKET;
						break;
						default :
							printf("Error charcter %u error.", (uint)buffer[i]);
							i++;
						break;
					}
				}
			}
			if(t->type != OPA_TT_SPACE)
			{
				i += t->length;
				if(tokens->token_count % 256 == 255)
					tokens->tokens = realloc(tokens->tokens, (sizeof *tokens->tokens) * (tokens->token_count + 256));
				t = &tokens->tokens[++tokens->token_count];
				t->line = line_number;
				t->type = OPA_TT_SPACE;
				t->start = i;
				t->length = 1;
			}
		}
	}
	tokens->tokens->prev = NULL;
	tokens->tokens->next = &tokens->tokens[1];
	for(i = 1; i < tokens->token_count - 1; i++)
	{
		tokens->tokens[i].prev = &tokens->tokens[i - 1];
		tokens->tokens[i].next = &tokens->tokens[i + 1];
	}
	tokens->tokens[i].prev = &tokens->tokens[i - 1];
	tokens->tokens[i].next = NULL;
}


// uint opa_tokenizer_categorizer(uint8 *character, OPAToken *token, uint start, uint *line_nr)
uint opa_tokenizer_categorizer(uint8 character)
{
	if(character <= ' ')
		return OPA_TT_SPACE;
	switch(character)
	{
		case ';' :
		return OPA_TT_SEMICOLON;
		case ':' :
		return OPA_TT_COLON;
		case ',' :
		return OPA_TT_COMMA;
		case '.' :
		return OPA_TT_PERIOD;
		case '(' :
		return OPA_TT_OPEN_PARENTHESIS;
		case ')' :
		return OPA_TT_CLOSE_PARENTHESIS;
		case '{' :
		return OPA_TT_OPEN_SCOPE;
		case '}' :
		return OPA_TT_CLOSE_SCOPE;
		case '[' :
		return OPA_TT_OPEN_BRACKET;
		case ']' :
		return OPA_TT_CLOSE_BRACKET;
		case '=' :
		case '>' :
		case '<' :
		case '|' :
		case '&' :
		case '+' :
		case '-' :
		case '*' :
		case '/' :
		case '%' :
		case '?' :
		case '!' :
		return OPA_TT_OPERATOR;
		case '#' :
		return OPA_TT_HASHTAG;
		case '\n' :
		return OPA_TT_NEWLINE;
	}
	return OPA_TT_NAME;
}

uint opa_tokenizer_debug(uint8 *buffer, uint *line)
{
	if(buffer[0] == '_')
	{
		char *line_string = "__OPALINESTART_";
		uint i;
		for(i = 1; buffer[i] == line_string[i] && line_string[i] != 0; i++);
		if(line_string[i] == 0)
		{
			for(*line = 0; buffer[i] >= '0' && buffer[i] <= '9'; i++)
				*line = *line * 10 + buffer[i] - '0';
			return i;
		}
	}
	return 0;
}

void opa_tokenizer_divide(OPATokenFile *tokens, uint8 *buffer, uint buffer_size)
{
	uint i, j, type, current = -1, line_number = 0;
	OPAToken *t;
	tokens->tokens = NULL;
	tokens->token_count = 0;
	tokens->buffer = buffer;

	// 
	for(i = 0; buffer[i] != 0 && i < buffer_size; i++)
	{
		i += opa_tokenizer_debug(&buffer[i], &line_number);

		type = opa_tokenizer_categorizer(buffer[i]);
		if(type != current || current >= OPA_TT_SINGLE_CHARACTER_START)
		{
			if(tokens->token_count % 256 == 0)
				tokens->tokens = realloc(tokens->tokens, (sizeof *tokens->tokens) * (tokens->token_count + 256));
			if(OPA_TT_SPACE != type)
			{
				t = &tokens->tokens[tokens->token_count++];
				t->line = line_number;
				t->type = type;
				t->start = i;
				t->length = 1;
			}
		}else if(OPA_TT_SPACE != type)
			t->length++;
		current = type;
	}
	tokens->tokens->prev = NULL;
	tokens->tokens->next = &tokens->tokens[1];
	for(i = 1; i < tokens->token_count - 1; i++)
	{
		tokens->tokens[i].prev = &tokens->tokens[i - 1];
		tokens->tokens[i].next = &tokens->tokens[i + 1];
	}
	tokens->tokens[i].prev = &tokens->tokens[i - 1];
	tokens->tokens[i].next = NULL;
}

void opa_tokenizertoken_remove(OPAToken *t)
{

	if(t->next != NULL)
		((OPAToken *)t->next)->prev = t->prev;
	if(t->prev != NULL)
		((OPAToken *)t->prev)->next = t->next;
	t->type = OPA_TT_DELETED;
}

void opa_tokenizer_numbers(OPATokenFile *tokens)
{
	uint64 number = 0;
	OPAToken *t, *prev;
	uint i;
	
	for(i = 0; i < tokens->token_count  && tokens->tokens[i].type == OPA_TT_DELETED; i++);
	if(i < tokens->token_count)
	{
		for(t = &tokens->tokens[i]; t != NULL; t = t->next)
		{
			for(i = number = 0; i < t->length; i++)
			{
				if(tokens->buffer[i + t->start] < '0' || tokens->buffer[i + t->start] > '9')
					break;
				number *= 10;
				number += tokens->buffer[i + t->start] - '0';
			}
			if(i == t->length)
			{
				t->type = OPA_TT_INTEGER;
				t->value.integer = number;
				if(t->prev != NULL)
				{
					prev = t->prev;
					if(prev->type == OPA_TT_OPERATOR && prev->length == 1 && tokens->buffer[prev->start] == '-' && (prev->prev == NULL || ((OPAToken *)prev->prev)->type != OPA_TT_INTEGER))
					{
						t->value.integer = -t->value.integer;
						opa_tokenizertoken_remove(prev);
					}
				}
			}
		}
	}
}


void opa_tokenizer_real(OPATokenFile *tokens)
{
	uint64 number = 0;
	OPAToken *t, *prev, *next;
	uint i;
	for(i = 0; i < tokens->token_count  && tokens->tokens[i].type == OPA_TT_DELETED; i++);
	if(i < tokens->token_count)
	{
		for(t = &tokens->tokens[i]; t != NULL; t = t->next)
		{
			if(t->type == OPA_TT_PERIOD && t->next != NULL)
			{
				prev = t->prev;
				next = t->next;
				if(prev->type == OPA_TT_INTEGER && next->type == OPA_TT_INTEGER && next->value.integer >= 0)
				{
					double a;
					a = (double)next->value.integer;
					while(a >= 1.0)
						a *= 0.1;
					if(prev->value.integer > 0)
						a += (double)prev->value.integer;
					else
						a = (double)prev->value.integer - a;
					prev->value.real = a;
					prev->type = OPA_TT_REAL;
					opa_tokenizertoken_remove(t);
					opa_tokenizertoken_remove(next);
					t = prev;
				}
			}		
		}
	}
}




boolean opa_tokenizer_math(OPATokenFile *tokens, char *op)
{
	uint64 number = 0;
	OPAToken *t, *prev, *next;
	uint i;
	boolean action = FALSE;
	for(i = 0; i < tokens->token_count  && tokens->tokens[i].type == OPA_TT_DELETED; i++);
	if(i < tokens->token_count)
	{
		for(t = &tokens->tokens[i]; t != NULL; t = t->next)
		{
			if(t->type == OPA_TT_OPERATOR && tokens->buffer[t->start] == op[0] && (tokens->buffer[t->start + 1] == op[1] || (tokens->buffer[t->start + 1] <= 32 && op[1] <= 32)) && t->prev != NULL && t->next != NULL)
			{
				prev = t->prev;
				next = t->next;
				if((prev->type == OPA_TT_INTEGER || prev->type == OPA_TT_REAL) &&
					(next->type == OPA_TT_INTEGER || next->type == OPA_TT_REAL))
				{
					if(next->type == OPA_TT_REAL || prev->type == OPA_TT_REAL)
					{
						double a, b;
						if(next->type == OPA_TT_REAL)
							a = next->value.real;
						else
							a = (double)next->value.integer;
						if(prev->type == OPA_TT_REAL)
							b = prev->value.real;
						else
							b = (double)prev->value.integer;
						switch(op[0])
						{
							case '+' :
								a = b + a;
							break;
							case '-' :
								a = b - a;
							break;
							case '*' :
								a = b * a;
							break;
							case '/' :
								a = b / a;
							break;
							case '%' :
								a = b / a;
							break;

						}
						prev->value.real = a;
						prev->type = OPA_TT_REAL;
					}else
					{
						switch(op[0])
						{
							case '+' :
								prev->value.integer = prev->value.integer + next->value.integer;
							break;
							case '-' :
								prev->value.integer = prev->value.integer - next->value.integer;
							break;
							case '*' :
								prev->value.integer = prev->value.integer * next->value.integer;
							break;
							case '/' :
								prev->value.integer = prev->value.integer / next->value.integer;
							break;
							case '%' :
								prev->value.integer = prev->value.integer % next->value.integer;
							break;
							case '<' :
								if(op[1] == '<')
									prev->value.integer = prev->value.integer << next->value.integer;
								else
									prev->value.integer = prev->value.integer < next->value.integer;
							break;
							case '>' :
								if(op[1] == '>')
									prev->value.integer = prev->value.integer >> next->value.integer;
								else
									prev->value.integer = prev->value.integer > next->value.integer;
							break;
							case '=' :
								if(op[1] == '=')
									prev->value.integer = prev->value.integer == next->value.integer;
							break;
							case '!' :
								if(op[1] == '=')
									prev->value.integer = prev->value.integer != next->value.integer;
							break;
							case '&' :
								if(op[1] == '&')
									prev->value.integer = prev->value.integer && next->value.integer;
								else
									prev->value.integer = prev->value.integer & next->value.integer;
							break;
							case '|' :
								if(op[1] == '|')
									prev->value.integer = prev->value.integer || next->value.integer;
								else
									prev->value.integer = prev->value.integer | next->value.integer;
							break;
							case '^' :
								prev->value.integer = prev->value.integer ^ next->value.integer;
							break;
						}
					}
					opa_tokenizertoken_remove(t);
					opa_tokenizertoken_remove(next);
					t = prev;
					while(t->next != NULL && t->prev != NULL && ((OPAToken *)t->prev)->type == OPA_TT_OPEN_PARENTHESIS && ((OPAToken *)t->next)->type == OPA_TT_CLOSE_PARENTHESIS)
					{
						opa_tokenizertoken_remove(t->prev);
						opa_tokenizertoken_remove(t->next);
					}
					action = TRUE;
				}
			}		
		}
	}
	return action;
}

void opa_tokenizer_print(OPATokenFile *tokens)
{
	char text[256];
	OPAToken *t;
	uint8 *buffer;
	uint i, j;
	for(i = 0; i < tokens->token_count  && tokens->tokens[i].type == OPA_TT_DELETED; i++);
	if(i < tokens->token_count)
	{
		for(t = &tokens->tokens[i]; t != NULL; t = t->next)
		{
			if(t->type == OPA_TT_INTEGER)
				printf(" integer[%u] = %lli\n", i, t->value.integer);
			else if(t->type == OPA_TT_REAL)
				printf(" real[%u] = %f\n", i, t->value.real);
			else
			{
				for(j = 0; j < t->length && j < 255; j++)
					text[j] = tokens->buffer[t->start + j];
				text[j] = 0;
				printf(" text[%u] = -%s- type: %u\n", i, text, t->type);
			}
			i++;
		}
	}

	for(i = 0; i < tokens->token_count  && tokens->tokens[i].type == OPA_TT_DELETED; i++);
	if(i < tokens->token_count)
	{
		for(t = &tokens->tokens[i]; t != NULL; t = t->next)
		{
			if(t->type == OPA_TT_INTEGER)
				printf("%lli ", t->value.integer);
			else if(t->type == OPA_TT_REAL)
				printf("%f ", t->value.real);
			else if(t->type >= OPA_TT_KEYWORD_AUTO)
				printf("%s ", opa_languange_keywords[t->type - OPA_TT_KEYWORD_AUTO]);
			else if(t->type == OPA_TT_SEMICOLON)
				printf(";\n");
			else
			{
				for(j = 0; j < t->length && j < 255; j++)
					text[j] = tokens->buffer[t->start + j];
				text[j] = 0;
				printf(" %s ", text);
			}
		}
	}
}



void opa_tokenizer(OPATokenFile *tokens, uint8 *buffer, uint buffer_size)
{
	boolean math = TRUE;
	opa_tokenizer_divide(tokens, buffer, buffer_size);
	opa_tokenizer_print(tokens);
	opa_tokenizer_numbers(tokens);
	opa_tokenizer_real(tokens);

	opa_tokenizer_keywords(tokens);

	while(math)
	{
		math = FALSE;
	//	math = opa_tokenizer_math(tokens, "sizeof") || math;
		math = opa_tokenizer_math(tokens, "!") || math;
		math = opa_tokenizer_math(tokens, "~") || math;
		math = opa_tokenizer_math(tokens, "*") || math;
		math = opa_tokenizer_math(tokens, "/") || math;
		math = opa_tokenizer_math(tokens, "%") || math;
		math = opa_tokenizer_math(tokens, "+") || math;
		math = opa_tokenizer_math(tokens, "-") || math;

		math = opa_tokenizer_math(tokens, "<<") || math;
		math = opa_tokenizer_math(tokens, ">>") || math;
		math = opa_tokenizer_math(tokens, "<") || math;
		math = opa_tokenizer_math(tokens, ">") || math;
		math = opa_tokenizer_math(tokens, "==") || math;
		math = opa_tokenizer_math(tokens, "!=") || math;
		math = opa_tokenizer_math(tokens, "&") || math;
		math = opa_tokenizer_math(tokens, "^") || math;
		math = opa_tokenizer_math(tokens, "|") || math;
		math = opa_tokenizer_math(tokens, "&&") || math;
		math = opa_tokenizer_math(tokens, "||") || math;
	//	math = opa_tokenizer_math(tokens, "?") || math;
	}

	opa_tokenizer_print(tokens);
}



void opa_tokenizer_test()
{
	OPATokenFile tokens;
	uint8 *buffer;
	buffer = opa_preprosessor("../../Mergesource/stellar_structure.h");
	opa_tokenizer(&tokens, buffer, -1);
}
