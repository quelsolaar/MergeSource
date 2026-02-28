#include <stdio.h>
#include <stdlib.h>

#include "seduce.h"

//#define S_TRANSLATION_UTIL
//#define S_TRANSLATION_TEST

typedef struct{
	char *text;
	uint checksum;
}SeduceLine;

SeduceLine *seduce_translate_data_base = NULL;
uint seduce_translate_data_base_count = 0;
uint seduce_translate_data_base_allocated = 0;


char *seduce_translate_add(char *text, uint checksum)
{
	uint i;
	for(i = 0; i < seduce_translate_data_base_count; i++)
		if(seduce_translate_data_base[i].checksum == checksum)
			return seduce_translate_data_base[i].text;

	if(seduce_translate_data_base_count == seduce_translate_data_base_allocated)
	{
		seduce_translate_data_base_allocated += 64;
		seduce_translate_data_base = realloc(seduce_translate_data_base, (sizeof *seduce_translate_data_base) * seduce_translate_data_base_allocated);
	}
	seduce_translate_data_base[seduce_translate_data_base_count].checksum = checksum;
	seduce_translate_data_base[seduce_translate_data_base_count].text = text;
	seduce_translate_data_base_count++;
	return text;
}


char *seduce_translate(char *text)
{
	uint i, checksum = 0;

	for(i = 0; text[i] != 0; i++)
		checksum += text[i] * ((i + 23) * (i + 65537));
	return seduce_translate_add(text, checksum);
}


void seduce_translate_load(char *file)
{
	FILE *f;
	boolean keep_going = TRUE;
	uint i, j, checksum;
	char text[4096], line[4096], *p;
	if((f = fopen(file, "r")) != NULL)
	{
		while(keep_going)
		{
			for(i = 0; i < 4095; i++)
			{
				text[i] = fgetc(f);
				if(text[i] == EOF || text[i] == '\n')
				{
					if(text[i] == EOF)
						keep_going = FALSE;
					break;
				}
			}
			text[i] = 0;
			if(1 == sscanf(text, "<id=%u", &checksum))
			{
				for(i = 0; text[i] != 0 && text[i] != '>'; i++);
				if(text[i] != 0)
				{
					i++;
					for(j = 0; text[i] != 0 && text[i] != 0; i++)
						line[j++] = text[i];
					line[j++] = 0;

					p = malloc((sizeof *line) * j);
					for(j = 0; line[j] != 0; j++)
					{
						if(line[j] == 36)
							p[j] = '\n';
						else
							p[j] = line[j];
					}
					p[j] = 0;
					seduce_translate_add(p, checksum);
				}
			}
		}
	}
}

/*
*/
void seduce_translate_save(char *file)
{
	char text[4096];
	uint i, j;
	FILE *f;
	if((f = fopen(file, "w")) != NULL)
	{
		for(i = 0; i < seduce_translate_data_base_count; i++)
		{
			for(j = 0; seduce_translate_data_base[i].text[j] != 0; j++)
			{
				if(seduce_translate_data_base[i].text[j] == '\n')
					text[j] = 36;
				else
					text[j] = seduce_translate_data_base[i].text[j];
			}
			text[j] = 0;
			fprintf(f, "<id=%u>%s\n", seduce_translate_data_base[i].checksum, text);
		}
		fclose(f);
	}
}

void seduce_translate_apply(char *file)
{
	char *array, *string, *compare = "seduce_translate(\"", *trans, *t;
	FILE *f;
	uint length, i, j, pos;
	if((f = fopen(file, "r")) != NULL)
	{
		fseek(f, 0 , SEEK_END);
		length = ftell(f);
		rewind(f);
		if(length == 0)
		{
			fclose(f);
			return;
		}
		array = (char*)malloc(length);
		string = (char*)malloc(length);

		length = fread(array, 1, length, f);
		fclose(f);
		if((f = fopen(file, "w")) != NULL)
		{
			pos = 0;
			for(i = 0; i < length; i++)
			{
				for(j = 0; compare[j] == array[i + j] && compare[j] != 0; j++);
				if(compare[j] == 0)
				{
					i += j;
					fwrite(&array[pos], (sizeof *array), i - pos, f);
					pos = i;
					for(j = 0; array[i + j] != 0 && array[i + j] != '\"'; j++);
					if(array[i + j] == '\"')
					{
						t = malloc((sizeof *t) * (j + 1));
						for(j = 0; array[i + j] != 0 && array[i + j] != '\"'; j++)
							t[j] = array[i + j];
						t[j] = 0;
						trans = seduce_translate(t);
						printf("Translating: %s -> %s\n", t, trans);
						array[i + j] = '\"';
						i += j;
						for(j = 0; trans[j] != 0; j++);
						fwrite(trans, (sizeof *trans), j, f);
						pos = i;
					}
				}
			}
			fwrite(&array[pos], (sizeof *array), i - pos, f);
			fclose(f);
		}
		free(array);
		free(string);
	}
}

#ifdef S_TRANSLATION_TEST

int main(int argc, char **argv)
{
	seduce_translate_load("test_translation.txt");
	printf(seduce_translate("Hello World!"));
	seduce_translate_save("test_translation.txt");
	return 1;
}

#endif



