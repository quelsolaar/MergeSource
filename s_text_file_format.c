
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "seduce.h"
#include "imagine.h"

uint8 seduce_font_read_uint8(uint8 *array, uint *pos)
{
	uint8 out;
	out = array[*pos];
	(*pos)++;
	return out;
}

uint16 seduce_font_read_uint16(uint8 *array, uint *pos)
{
	uint16 out;
	out = ((uint16) array[*pos]) << 8;
	(*pos)++;
	out |= (uint16) array[*pos];
	(*pos)++;
	return out;
}

uint32 seduce_font_read_uint32(uint8 *array, uint *pos)
{
	uint32 out;
	out = ((uint32) array[*pos]) << 24;
	(*pos)++;
	out |= ((uint32) array[*pos]) << 16;
	(*pos)++;
	out |= ((uint32) array[*pos]) << 8;
	(*pos)++;
	out |= (uint32) array[*pos];
	(*pos)++;
	return out;
}


float seduce_font_read_float(uint8 *array, uint *pos)
{
	union { uint32 integer; real32 real; } punt;
	punt.integer = ((uint32) array[*pos]) << 24;
	(*pos)++;
	punt.integer |= ((uint32) array[*pos]) << 16;
	(*pos)++;
	punt.integer |= ((uint32) array[*pos]) << 8;
	(*pos)++;
	punt.integer |= (uint32) array[*pos];
	(*pos)++;
	return punt.real;
}


void seduce_font_write_uint32(uint8 *array, uint *pos, uint32 value)
{
	array[*pos] = (value >> 24) & 0xFF;
	(*pos)++;
	array[*pos] = (value >> 16) & 0xFF;
	(*pos)++;
	array[*pos] = (value >> 8) & 0xFF;
	(*pos)++;
	array[*pos] = value & 0xFF;
	(*pos)++;
}


void seduce_font_write_float(uint8 *array, uint *pos, float value)
{
	union { uint32 integer; real32 real; } punt;
	punt.real = value;
	array[*pos] = (punt.integer >> 24) & 0xFF;
	(*pos)++;
	array[*pos] = (punt.integer >> 16) & 0xFF;
	(*pos)++;
	array[*pos] = (punt.integer >> 8) & 0xFF;
	(*pos)++;
	array[*pos] = punt.integer & 0xFF;
	(*pos)++;
}

SeduceFontCharacter *seduce_character_find(SeduceRenderFont *font, uint32 character)
{
	SeduceFontBranch *branch;
	uint base = 0, generation, pos = 0;

	branch = font->root;
	if(branch->character_code_start > character)
		return NULL;
	if(branch->character_code_end <= character)
		return NULL;
	base = 1;
	for(generation = 1; TRUE; generation *= 2)
	{
		base += generation;
		pos *= 2;
		if(branch[base + pos].character_code_end >= character)
		{
			if(branch[base + pos + 1].character_code_start > character)
				return NULL;
			pos++;
		}
		if(branch[base + pos].glyph_start != -1)
 			return &font->glyphs[font->root[base + pos].glyph_start + character - font->root[base + pos].character_code_start];
	}
	return NULL;
}


void seduce_font_load_construct_graph(SeduceRenderFont *font, uint *character_ids, uint character_count)
{
	SeduceFontBranch *branches;
	SeduceFontCharacter *characters;
	uint i, gaps = 1, tree_size, read_position,	write_position, branch_count, previous_gap_pos, current_charcter;
	
//	for(i = 0; i < character_count; i++)
//		printf("character_ids[%u] = %u %u\n", i, character_ids[i * 2], character_ids[i * 2 + 1]);
	for(i = 0; i < character_count - 1; i++)
		if(character_ids[i * 2 + 2] != character_ids[i * 2] + 1)
			gaps++;
	i = gaps;
	tree_size = 0;
	for(i = 1; i <= gaps; i *= 2)
		tree_size += i;
	tree_size += i;
	branch_count = i;
	font->root = branches = malloc((sizeof *branches) * tree_size);
	previous_gap_pos = 0;
	branches[tree_size - branch_count].character_code_start = current_charcter = character_ids[0];
	branches[tree_size - branch_count].glyph_start = 0;
	gaps = 0;
	for(i = 1; i < character_count; i++)
	{
		if(current_charcter + 1 != character_ids[i * 2])
		{
			branches[tree_size - branch_count + gaps].character_code_end = current_charcter + 1;
			gaps++;
			branches[tree_size - branch_count + gaps].character_code_start = character_ids[i * 2];
			branches[tree_size - branch_count + gaps].glyph_start = i;
		}
		current_charcter = character_ids[i * 2];
	}
	branches[tree_size - branch_count + gaps].character_code_end = current_charcter + 1;
	for(i = gaps; i < branch_count; i++)
	{
		branches[tree_size - branch_count + i].character_code_start = current_charcter + 1;
		branches[tree_size - branch_count + i].character_code_end = current_charcter + 1;
		branches[tree_size - branch_count + i].glyph_start = 0;
	}
	read_position = tree_size;
	write_position = tree_size - branch_count;
	for(; branch_count != 1; branch_count /= 2)
	{
		read_position -= branch_count;
		write_position -= branch_count / 2;
		for(i = 0; i < branch_count / 2; i++)
		{
			branches[write_position + i].character_code_start = branches[read_position + i * 2].character_code_start;
			branches[write_position + i].character_code_end = branches[read_position + i * 2 + 1].character_code_end;
			branches[write_position + i].glyph_start = -1;
		}
	}
	if(font->root->character_code_start < 33)
		font->root->character_code_start = 33;
}



void seduce_font_draw(SeduceRenderFont *font)
{
	SeduceFontCharacter *character;
	char *text = "Hello";
	uint i;
	RShader *shader; 

	
	r_primitive_line_3d(0, 0, 0.0, 1, 0, 0.0, 1, 0, 0, 1.0);
	r_primitive_line_3d(0, 0, 0.0, 0, 1, 0.0, 1, 0, 0, 1.0);
	r_primitive_line_flush();
	shader = r_shader_presets_get(P_SP_COLOR_UNIFORM);
	for(i = 0; text[i] != 0; i++)
	{
		character = seduce_character_find(font, (uint)text[i]);
		if(character != NULL)
			r_array_references_draw(font->pool, &character->sections, GL_LINES, NULL, NULL, 1);

	}

}

void *seduce_font_load_pff(SeduceRenderFont *font, char *file_name)
{
	SeduceFontCharacter *glyphs;
	
	RFormats vertex_format_types = R_FLOAT;
	FILE *f;
	uint i, j, size, largest, pos = 0, vertex_count, character_count, glyph_count, *charcter_ids, *ref_size, *ref;
	float *vertex_array, *vertex_section, **ref_sections;
	uint8 *buffer;

	f = fopen(file_name, "rb");
	if(f == NULL)
		return NULL;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	buffer = malloc(size);
	size = fread(buffer, 1, size, f);
	fclose(f);
	for(i = 0; i < 32; i++)
		font->name[i] = seduce_font_read_uint8(buffer, &pos);
	glyph_count = seduce_font_read_uint32(buffer, &pos);
	character_count = seduce_font_read_uint32(buffer, &pos);
	vertex_count = seduce_font_read_uint32(buffer, &pos);
	charcter_ids = malloc((sizeof *charcter_ids) * character_count * 2);
	for(i = 0; i < character_count * 2; i++)
		charcter_ids[i] = seduce_font_read_uint32(buffer, &pos);
	ref_size = malloc((sizeof *charcter_ids) * character_count);
	size = largest = 0;
	for(i = 0; i < glyph_count; i++)
	{
		ref_size[i] = seduce_font_read_uint32(buffer, &pos);
		size += ref_size[i];	
		if(largest < ref_size[i])
			largest = ref_size[i];
	}
	vertex_array = malloc((sizeof *vertex_array) * vertex_count * 2);
	for(i = 0; i < vertex_count * 2; i++)
		vertex_array[i] = seduce_font_read_float(buffer, &pos);
	i = 2;
	font->pool = r_array_allocate(vertex_count, &vertex_format_types, &i, 1, size + character_count * 2); 
	vertex_section = r_array_section_allocate_vertex(font->pool, vertex_count); 
	r_array_load_vertex(font->pool, vertex_section, vertex_array, 0, vertex_count);
	free(vertex_array);

	glyphs = malloc((sizeof *font->glyphs) * glyph_count);
	ref = malloc((sizeof *ref) * largest);
	for(i = 0; i < glyph_count; i++)
	{
		for(j = 0; j < ref_size[i]; j++)
		{
			ref[j] = seduce_font_read_uint32(buffer, &pos);
		}
 		glyphs[i].sections = r_array_section_allocate_reference(font->pool, ref_size[i]);
		if(glyphs[i].sections == 0)
			j = 0;
		r_array_load_reference(font->pool, glyphs[i].sections, vertex_section, ref, ref_size[i]); 
	}
	free(ref);
	for(i = 0; i < glyph_count; i++)
	{
		for(j = 0; j < 8; j++)
			glyphs[i].kerning[j] = seduce_font_read_float(buffer, &pos);
		glyphs[i].size = glyphs[i].kerning[1];
		for(j = 3; j < 8; j += 2)
			if(glyphs[i].size < glyphs[i].kerning[j])
				glyphs[i].size = glyphs[i].kerning[j];
	}

	font->glyphs = malloc((sizeof *font->glyphs) * character_count);
	for(i = 0; i < character_count; i++)
		font->glyphs[i] = glyphs[charcter_ids[i * 2 + 1]];
	free(glyphs);
	seduce_font_load_construct_graph(font, charcter_ids, character_count);
	free(charcter_ids);
	free(buffer);
	return font;
}

SeduceRenderFont *seduce_font_storage = NULL;
uint seduce_font_storage_count = 0;
uint seduce_font_storage_allocated = 0;
uint seduce_font_storage_default = 0;

void seduce_font_load(char *name)
{
	if(seduce_font_storage_count == seduce_font_storage_allocated)
	{
		seduce_font_storage_allocated += 16;
		seduce_font_storage = realloc(seduce_font_storage, (sizeof *seduce_font_storage) * seduce_font_storage_allocated);
	}
	if(seduce_font_load_pff(&seduce_font_storage[seduce_font_storage_count], name) != NULL)
		seduce_font_storage_count++;
}

extern void seduce_font_verdana_init(SeduceRenderFont *font);
extern void seduce_font_eurasia_init(SeduceRenderFont *font);
extern void seduce_font_times_init(SeduceRenderFont *font);
extern void seduce_font_impact_init(SeduceRenderFont *font);
extern void seduce_font_arial_init(SeduceRenderFont *font);

void seduce_font_load_all()
{
	uint i;
	char path[124];
	seduce_font_storage_allocated = 16;
	seduce_font_storage = realloc(seduce_font_storage, (sizeof *seduce_font_storage) * seduce_font_storage_allocated);
	seduce_font_eurasia_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_verdana_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_times_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_impact_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_arial_init(&seduce_font_storage[seduce_font_storage_count++]);
/*	seduce_font_abel_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_lane_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_lato_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_luke_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_oswald_init(&seduce_font_storage[seduce_font_storage_count++]);
	seduce_font_sansumi_init(&seduce_font_storage[seduce_font_storage_count++]);*/
	for(i = 0; imagine_path_search(".pff", TRUE, IMAGINE_DIR_HOME_PATH, FALSE, i, path, 1024); i++)
		seduce_font_load(path);
//	exit(0);
}

uint seduce_font_count()
{
	return seduce_font_storage_count;
}

char *seduce_font_name(uint id)
{
	return seduce_font_storage[id].name;
}

SeduceRenderFont *seduce_font_get_by_id(uint id)
{
	if(id < seduce_font_storage_count)
		return &seduce_font_storage[id];
	return &seduce_font_storage[seduce_font_storage_default];
}

SeduceRenderFont *seduce_font_get_by_name(char *name)
{
	uint i, j;
	for(i = 0; i < seduce_font_storage_count; i++)
	{
		for(j = 0; name[j] != 0 && name[j] == seduce_font_storage[i].name[j]; j++);
		if(name[j] == seduce_font_storage[i].name[j])
			return &seduce_font_storage[i];
	}
    return NULL;
}

SeduceRenderFont *seduce_font_default_get()
{
	return &seduce_font_storage[seduce_font_storage_default];
}

void seduce_font_default_set(SeduceRenderFont *font)
{
	uint i;
	for(i = 0; i < seduce_font_storage_count; i++)
	{
		if(&seduce_font_storage[i] == font)
		{
			seduce_font_storage_default = i;
			return;
		}
	}
}