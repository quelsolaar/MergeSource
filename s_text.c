
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "seduce.h"

#define SUI_TEXTURE_DRAW_CASH_SIZE 512
#define SUI_TEXTURE_LETTER_SIZE 2.0
#define SUI_TEXTURE_LETTER_BASE (0.5)

extern boolean	betray_set_screen_mode(uint x_size, uint y_size, boolean fullscreen);
extern double	betray_screen_mode_get(uint *x_size, uint *y_size, boolean *fullscreen);
extern void		betray_set_frustum_mode(double far, double near, double fov);
extern void		betray_get_frustum_mode(double *far, double *near, double *fov);

extern void seduce_font_init();

void *sui_font_shader;
uint sui_font_shader_location_shader;

char *r_font_shader_vertex = 
"attribute vec2 vertex;\n"
"uniform block{\n"
"	vec2 pos;\n"
"};\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"void main()\n"
"{\n"
"	gl_Position = ModelViewProjectionMatrix * vec4(vertex + pos, 0.0, 1.0);\n"
"}\n";
char *r_font_shader_fragment = 
"uniform vec4 color;\n"
"void main()\n"
"{\n"
"	gl_FragColor = color;\n"
"}\n";


void seduce_text_init()
{
	char buf[2000];
	sui_font_shader = r_shader_create_simple(buf, 2000, r_font_shader_vertex, r_font_shader_fragment, "color font");
	r_shader_state_set_blend_mode(sui_font_shader, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	printf(buf);
	sui_font_shader_location_shader = r_shader_uniform_location(sui_font_shader, "color");
}

float seduce_text_kern(float *k, float *k2)
{
	float f, f2;
	f = k[1] - k2[0];
	f2 = k[3] - k2[2];
	if(f2 > f)
		f = f2;
	f2 = k[5] - k2[4];
	if(f2 > f)
		f = f2;
	f2 = k[7]- k2[6];
	if(f2 > f)
		f = f2;
	return f;
}


uint seduce_text_line_draw_monospace(SeduceRenderFont *font, float pos_x, float pos_y, float character_size, float space_size, const char *text, uint *pos, float red, float green, float blue, float alpha, uint start, uint end, uint length)
{
	RShader	*s;
	RMatrix	*m;
	SeduceFontCharacter *character = NULL;
	unsigned int j = 0, placement = 0;
	static void **sections = NULL;
	static float *data;
	uint32 character_id;
	uint location, block_size, calls = 0;
	float *c;
	if(text == NULL || *text == '\0')
		return 0.0;
	m = r_matrix_get();
	r_matrix_push(m);
	r_matrix_translate(m, pos_x, pos_y, 0);
	r_matrix_scale(m, character_size, character_size, 1.0);
	r_shader_set(sui_font_shader);
	r_shader_vec4_set(NULL, sui_font_shader_location_shader, red, green, blue, alpha);
	block_size = r_shader_uniform_block_size(sui_font_shader, 0);
	if(sections == NULL)
	{
		sections = malloc((sizeof *sections) * 1024);
		data = malloc(1024 * block_size);
	}
	block_size /= sizeof(float);
	space_size /= character_size;
	data[0] = 0;
	data[1] = 0;
	while(*pos < end && text[*pos] != 0 && placement < length)
	{
		while(*pos < end && text[*pos] != 0 && calls < 1024 && placement < length)
		{
			character_id = f_utf8_to_uint32(text, pos);
			if(placement >= start)
			{
				character = seduce_character_find(font, character_id);
				if(character != NULL)
				{
					data[calls * block_size] = ((float)(placement - start) + (1 - character->size) * 0.5) * space_size;
					data[calls * block_size + 1] = 0;
					sections[calls] = character->sections;
					calls++;
					if(calls == 1024)
						break;
				}
			}
			placement++;
			if(character_id == '\t')
				placement = ((placement + 3) / 4) * 4;
		}
		if(calls != 0)
			r_array_references_draw(font->pool, sections, GL_TRIANGLES, data, NULL, calls);
		calls = 0;
	}
	r_matrix_pop(m);
	return placement;
}

uint seduce_text_box_draw_monospace(SeduceRenderFont *font, float pos_x, float pos_y, float character_size, float space_size, float line_size, const char *text, uint line_count, uint line_length, uint *scroll, STextBlockMode *modes, uint mode_count)
{
	uint pos = 0, p, i, j, end, add, block = 0, indent;
	uint32 character_id;
	if(font == NULL)
		font = seduce_font_default_get();
	if(scroll[1] != 0)
	{
		for(i = 0; text[pos] != 0; pos++)
		{
			if(text[pos] == '\n')
			{
				if(++i == scroll[1])
				{
					pos++;
					break;
				}
			}
		}
	}
	if(text[pos] == 0)
		return pos;

	for(i = 0; i < line_count && text[pos] != 0; i++)
	{
		end = pos;
		add = 0;
		for(end = pos; text[end] != 0 && text[end] != '\n'; end++);
		character_id = 0;
		for(j = 0; j < scroll[0] && pos != end; j++)
		{
			character_id = f_utf8_to_uint32(text, &pos);
			if(character_id == '\t')
				j += 3;
		}
		if(character_id == '\t' && scroll[0] % 4 != 0)
			add = (4 - scroll[0] % 4);
		if(pos != end)
		{
			while(block + 1 < mode_count && modes[block + 1].character_position < end)
			{
				add += seduce_text_line_draw_monospace(font, pos_x + space_size * (float)add, pos_y - line_size * (float)(i + 1), character_size, space_size, text, &pos, modes[block].red, modes[block].green, modes[block].blue, modes[block].alpha, 0, modes[block + 1].character_position, line_length - add);
				block++;
			}
			seduce_text_line_draw_monospace(font, pos_x + space_size * (float)add, pos_y - line_size * (float)(i + 1), character_size, space_size, text, &pos, modes[block].red, modes[block].green, modes[block].blue, modes[block].alpha, 0, end, line_length - add);
			if(text[end] == 0)
				return end;
		}
		pos = end + 1;
	}
	return pos;
}

uint seduce_text_box_draw_monospace_find(const char *text, uint pos, uint scroll, uint line_length)
{
	uint i, j, character_id;
	for(i = j = 0; i < pos; j++)
	{
		character_id = f_utf8_to_uint32(text, &i);
		if(character_id == '\t')
			j += 3;
	}
//	if(j > scroll)
	{
//		j -= scroll;
		if(j < line_length)
			return j;
		return line_length;
	}
	return 0;

}

void seduce_text_box_draw_monospace_select(BInputState *input, float pos_x, float pos_y, float character_size, float space_size, float line_size, const char *text, uint line_count, uint line_length, uint *scroll, uint select_start, uint select_end)
{
	uint pos = 0, p, i, j, end, a, b, block = 0, indent;
	uint32 character_id;
	float center[3] = {0, 0, 0};
	

	if(select_start > select_end)
	{
		i = select_start;
		select_start = select_end;
		select_end = i; 
	}

	if(scroll[1] != 0)
	{
		for(i = 0; text[pos] != 0; pos++)
		{
			if(text[pos] == '\n')
			{
				if(++i == scroll[1])
				{
					pos++;
					break;
				}
			}
		}
	}
	if(/*text[pos] == 0 || */pos > select_end)
		return;

	for(i = 0; i < line_count; i++)
	{
		end = pos;
		for(end = pos; text[end] != 0 && text[end] != '\n'; end++);
		character_id = 0;
		for(j = 0; j < scroll[0] && pos != end; j++)
		{
			character_id = f_utf8_to_uint32(text, &pos);
			if(character_id == '\t')
				j += 3;
		}

		if(select_start <= end)
		{
			if(pos > select_start)
				a = 0; 
			else
				a  = seduce_text_box_draw_monospace_find(&text[pos], select_start - pos, scroll[0], line_length);
			if(select_start == select_end)
			{
				seduce_background_quad_draw(input, NULL, 0, 
														pos_x + space_size * (float)a, pos_y - line_size - character_size * 0.5, 0, 
														pos_x + space_size * (float)a, pos_y - character_size * 0.5, 0, 
														pos_x + space_size * (float)a - space_size * 0.2, pos_y - character_size * 0.5, 0, 
														pos_x + space_size * (float)a - space_size * 0.2, pos_y - line_size - character_size * 0.5, 0, 
														0.2, 0, 0,
														0, 0, 0, 0.0);
				seduce_background_quad_draw(input, NULL, 0, 
														pos_x + space_size * (float)a + space_size * 0.2, pos_y - line_size - character_size * 0.5, 0, 
														pos_x + space_size * (float)a + space_size * 0.2, pos_y - character_size * 0.5, 0, 
														pos_x + space_size * (float)a, pos_y - character_size * 0.5, 0, 
														pos_x + space_size * (float)a, pos_y - line_size - character_size * 0.5, 0, 
														-0.2, 0, 0,
														0, 0, 0, 0.0);
				break;
			}
			if(select_end < end)
				b = seduce_text_box_draw_monospace_find(&text[pos], select_end - pos, scroll[0], line_length);
			else
				b = seduce_text_box_draw_monospace_find(&text[pos], end - pos, scroll[0], line_length);

			seduce_background_quad_draw(input, NULL, 0,
													pos_x + space_size * (float)a, pos_y - line_size - character_size * 0.5, 0, 
													pos_x + space_size * (float)a, pos_y - character_size * 0.5, 0, 
													pos_x + space_size * (float)b, pos_y - character_size * 0.5, 0, 
													pos_x + space_size * (float)b, pos_y - line_size - character_size * 0.5, 0, 
													0, 0, 0,
													0, 0, 0, 0.0);

		}
		if(end >= select_end || text[end] == 0/* || text[end + 1] == 0*/)
			break;
		pos = end + 1;
		pos_y -= line_size;
	}
	seduce_background_polygon_flush(input, center, 1.0);

}



float seduce_text_line_draw_internal(SeduceRenderFont *font, float pos_x, float pos_y, float size, float spacing, float space_length_add, const char *text, float red, float green, float blue, float alpha, uint length)
{
	RShader	*s;
	RMatrix	*m;
	SeduceFontCharacter *character = NULL, *last;
	unsigned int i = 0, j = 0, k;
	float f, f2, pos;
	static void **sections = NULL;
	static float *data;
	uint location, block_size, calls = 0;
	float *c;
	if(text == NULL || *text == '\0')
		return 0.0;
	m = r_matrix_get();
	r_matrix_push(m);
	r_matrix_translate(m, pos_x, pos_y, 0);
	r_matrix_scale(m, size, size, 1.0);
	r_shader_set(sui_font_shader);
	r_shader_vec4_set(NULL, sui_font_shader_location_shader, red, green, blue, alpha);
	block_size = r_shader_uniform_block_size(sui_font_shader, 0);
	if(sections == NULL)
	{
		sections = malloc((sizeof *sections) * 1024);
		data = malloc(1024 * block_size);
	}
	space_length_add /= size;
	block_size /= sizeof(float);
	data[0] = 0;
	data[1] = 0;
	pos = -(1.0 + spacing);
	pos = -spacing;
	while(i < length && text[i] != 0)
	{
		while(i < length && calls < 1024)
		{
			last = character;
			character = seduce_character_find(font, f_utf8_to_uint32(text, &i));
			if(character != NULL)
			{
				if(last != NULL)
					pos += spacing + seduce_text_kern(last->kerning, character->kerning);
				else
					pos += spacing;
				data[calls * block_size] = pos;
				data[calls * block_size + 1] = 0;
				sections[calls] = character->sections;
				if(sections[calls] != NULL)
				{
					calls++;
					if(calls == 1024)
						break;
				}
			}else
			{
				if(last != NULL)
					pos += last->size;
				pos += 1 + space_length_add + spacing;
			}
			if(text[i] == 0)
				break;
		}
		r_array_references_draw(font->pool, sections, GL_TRIANGLES, data, NULL, calls);
		calls = 0;
	}
	if(character != NULL)
		pos += character->size;
	r_matrix_pop(m);
	return pos * size;
}


float seduce_text_line_draw(SeduceRenderFont *font, float pos_x, float pos_y, float size, float spacing, const char *text, float red, float green, float blue, float alpha, uint length)
{
	if(font == NULL)
		font = seduce_font_default_get();
	return seduce_text_line_draw_internal(font, pos_x, pos_y, size, spacing, 0, text, red, green, blue, alpha, length);
}

float seduce_text_line_length_internal(SeduceRenderFont *font, float size, float spacing, float space_length_add, const char *text, uint *length, float distance)
{
	SeduceFontCharacter *character = NULL, *last;
	unsigned int i = 0, j = 0, k, old_i = 0;
	float add, pos;
	uint location, block_size, calls = 0;
	float *c;
	if(font == NULL)
		font = seduce_font_default_get();
	if(text == NULL || *text == '\0')
	{
		*length = 0;
		return 0.0;
	}
	 
	space_length_add /= size;
	distance /= size;
	pos = -spacing;
	distance -= spacing;
	while(i < *length && text[i] != 0)
	{
		last = character;
		old_i = i;
		character = seduce_character_find(font, f_utf8_to_uint32(text, &i));
		if(character != NULL)
		{
			if(last != NULL)
				add = spacing + seduce_text_kern(last->kerning, character->kerning);
			else
				add = spacing;
		}else
		{
			add = 1 + space_length_add + spacing;
			if(last != NULL)
				add += last->size;
		}
	/*	if(text[i] == 0)
			break;*/
		if(distance < pos + add * 0.5)
		{
			*length = old_i;
			return pos * size;
		}
		pos += add;
		calls = 0;
	}
	*length = i;
	if(character != NULL)
		pos += character->size;
	return pos * size;
}


float seduce_text_line_length(SeduceRenderFont *font, float size, float spacing, const char *text, uint end)
{
	return seduce_text_line_length_internal(font, size, spacing, 0, text, &end, 10000000);
}

uint seduce_text_line_hit_test(SeduceRenderFont *font, float letter_size, float letter_spacing, const char *text, float pos_x)
{
	uint end = -1;
	seduce_text_line_length_internal(font, letter_size, letter_spacing, 0, text, &end, pos_x - letter_size);
	return end;
}

uint seduce_text_block_hit_test(SeduceRenderFont *font, float *size, uint *gaps, float letter_size, float letter_spacing, const char *text,  float pos_x, uint end)
{
	SeduceFontCharacter *character = NULL, *last;
	unsigned int i = 0, last_i = 0, space_count, space_pos = 0;
	float f, pos, last_pos = 0;
	if(font == NULL)
		font = seduce_font_default_get();
	if(text == NULL || *text == '\0')
		return 0.0;
	pos_x /= letter_size;
	pos = -(letter_spacing);
	space_count = 0;
	while(i < end && text[i] != 0)
	{
		last = character;
		last_i = i;
		character = seduce_character_find(font, f_utf8_to_uint32(text, &i));
		if(character != NULL)
		{
			if(last != NULL)
				pos += letter_spacing + seduce_text_kern(last->kerning, character->kerning);
			else
				pos += letter_spacing;
			if(pos_x < pos + character->size)
			{
				if(*gaps + space_count > 0)
				{
					*gaps += space_count;
					*size = last_pos;
					return space_pos;
				}else
				{
					*gaps += space_count;
					*size = pos;
					return last_i;
				}
			}
		}else
		{
			space_pos = last_i;
			space_count++;
			if(last != NULL)
				pos += last->size;
			last_pos = pos;
			pos += 1 + letter_spacing;
		}
	}
	if(character != NULL)
		pos += letter_spacing + character->size;
	else
		pos += 1 + letter_spacing;

	*gaps += space_count;
	*size = pos;
	return i;
}

uint seduce_text_block_draw(float pos_x, float pos_y, float line_size, float height, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count)
{
	uint i, j, next, gaps, start_block = 0, block, line_end, accum;
	float f, size, space_size, x, y, highest;
	y = 0;
	height = -height;
	for(start_block = 0; start_block + 1 < mode_count && pos > modes[start_block + 1].character_position; start_block++);
	for(i = 0; text[pos] != 0; i++)
	{
		accum = pos;
		if(NULL == seduce_character_find(modes[start_block].font, f_utf8_to_uint32(text, &accum)))
			pos = accum;
		else 
			accum = pos;
		x = line_size;
		highest = modes[start_block].letter_size;
		gaps = 0;
		for(block = start_block; ; block++)
		{
			if(block + 1 < mode_count)
				line_end = modes[block + 1].character_position - accum;
			else
				line_end = -1;
			next = seduce_text_block_hit_test(modes[block].font, &size, &gaps, modes[block].letter_size, modes[block].letter_spacing, &text[accum], x, line_end);
			x -= size * modes[block].letter_size;
			accum += next;
			if(next != line_end)
				break;
			if(highest < modes[block + 1].letter_size)	
				highest = modes[block + 1].letter_size;
		}
		j = accum;
		if(NULL == seduce_character_find(modes[start_block].font, f_utf8_to_uint32(text, &j)))
			gaps--;
		y -= line_spacing * highest;
		if(y < height)
			return pos;
		space_size = 0.0;
		switch(style)
		{
			case SEDUCE_TBAS_LEFT :
				x = 0;
			break;
			case SEDUCE_TBAS_RIGHT :
				x = x;
			break;
			case SEDUCE_TBAS_CENTER :
				x = x * 0.5;
			break;
			case SEDUCE_TBAS_STRETCH :
				if(text[accum] != 0)
					space_size = x / (float)gaps;
				x = 0;
			break;
		}


		for(start_block; start_block <= block; start_block++)
		{
			if(start_block + 1 < mode_count)
				line_end = modes[start_block + 1].character_position - pos;
			else
				line_end = -1;
			if(line_end > accum - pos)
				line_end = accum - pos;	
			x += seduce_text_line_draw_internal(modes[start_block].font, pos_x + x, pos_y + y, modes[start_block].letter_size, modes[start_block].letter_spacing, space_size, &text[pos], modes[start_block].red, modes[start_block].green, modes[start_block].blue, modes[start_block].alpha, line_end);
			pos += line_end;
			if(pos >= accum)
				break;
		}
	}
	return pos;
}



void seduce_text_block_select_draw(BInputState *input, float pos_x, float pos_y, float line_size, float height, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count, uint select_start, uint select_end)
{
	uint i, j, next, gaps, start_block = 0, block, line_end, line_start, accum;
	float f, size, space_size, x, y, highest, f_start, f_end;
	y = 0;
	height = -height;
	if(pos >= select_end)
		return;
	for(start_block = 0; start_block + 1 < mode_count && pos > modes[start_block + 1].character_position; start_block++);
	for(i = 0; text[pos] != 0; i++)
	{
		accum = pos;
		if(NULL == seduce_character_find(modes[start_block].font, f_utf8_to_uint32(text, &accum)))
			pos = accum;
		else 
			accum = pos;
		x = line_size;
		highest = modes[start_block].letter_size;
		gaps = 0;
		for(block = start_block; ; block++)
		{
			if(block + 1 < mode_count)
				line_end = modes[block + 1].character_position - accum;
			else
				line_end = -1;
			next = seduce_text_block_hit_test(modes[block].font, &size, &gaps, modes[block].letter_size, modes[block].letter_spacing, &text[accum], x, line_end);
			x -= size * modes[block].letter_size;
			accum += next;
			if(next != line_end)
				break;
			if(highest < modes[block + 1].letter_size)	
				highest = modes[block + 1].letter_size;
		}
		j = accum;
		if(NULL == seduce_character_find(modes[start_block].font, f_utf8_to_uint32(text, &j)))
			gaps--;
		y -= line_spacing * highest;
		if(y < height)
			return;
		switch(style)
		{
			case SEDUCE_TBAS_LEFT :
				x = 0;
				space_size = 0.0;
			break;
			case SEDUCE_TBAS_RIGHT :
				x = x;
				space_size = 0.0;
			break;
			case SEDUCE_TBAS_CENTER :
				x = x * 0.5;
				space_size = 0.0;
			break;
			case SEDUCE_TBAS_STRETCH :
				space_size = x / (float)gaps;
				x = 0;
			break;
		}
		for(start_block; start_block <= block; start_block++)
		{
			if(start_block + 1 < mode_count)
				line_end = modes[start_block + 1].character_position - pos;
			else
				line_end = -1;
			if(line_end > accum - pos)
				line_end = accum - pos;	
		//	x += seduce_text_line_draw_internal(modes[start_block].font, pos_x + x, pos_y + y, modes[start_block].letter_size, modes[start_block].letter_spacing, space_size, &text[pos], modes[start_block].red, modes[start_block].green, modes[start_block].blue, modes[start_block].alpha, line_end);
			if(pos + line_end > select_start)
			{

				if(pos < select_start)
				{
					line_start = select_start - pos;
					f_start = x + seduce_text_line_length_internal(modes[start_block].font, modes[start_block].letter_size, modes[start_block].letter_spacing, space_size, &text[pos], &line_start, line_size + 1);
				}else
					f_start = x;

				if(select_start == select_end)
				{
					seduce_background_quad_draw(input, NULL, 0, 
													pos_x + f_start, pos_y + y - modes[start_block].letter_size * 0.5, 0, 
													pos_x + f_start, pos_y + y + modes[start_block].letter_size * 1.5, 0, 
													pos_x + f_start + modes[start_block].letter_spacing * modes[start_block].letter_size * 0.5, pos_y + y + modes[start_block].letter_size * 1.5, 0, 
													pos_x + f_start + modes[start_block].letter_spacing * modes[start_block].letter_size * 0.5, pos_y + y - modes[start_block].letter_size * 0.5, 0, 
													-0.2, 0, 0,
													0, 0, 0, 1.0);
					seduce_background_quad_draw(input, NULL, 0, 
													pos_x + f_start + modes[start_block].letter_spacing * modes[start_block].letter_size * 0.5, pos_y + y - modes[start_block].letter_size * 0.5, 0, 
													pos_x + f_start + modes[start_block].letter_spacing * modes[start_block].letter_size * 0.5, pos_y + y + modes[start_block].letter_size * 1.5, 0, 
													pos_x + f_start + modes[start_block].letter_spacing * modes[start_block].letter_size, pos_y + y + modes[start_block].letter_size * 1.5, 0, 
													pos_x + f_start + modes[start_block].letter_spacing * modes[start_block].letter_size, pos_y + y - modes[start_block].letter_size * 0.5, 0, 
													0.2, 0, 0,
													0, 0, 0, 1.0);
				}else
				{
					if(pos + line_end > select_end)
					{
						if(line_end > select_end - pos)
							line_end = select_end - pos;	
						f_end = x + seduce_text_line_length_internal(modes[start_block].font, modes[start_block].letter_size, modes[start_block].letter_spacing, space_size, &text[pos], &line_end, line_size + 1);
					}else
					{
						if(pos + line_end >= accum)
							f_end = line_size;
						else
							f_end = x + seduce_text_line_length_internal(modes[start_block].font, modes[start_block].letter_size, modes[start_block].letter_spacing, space_size, &text[pos], &line_end, line_size + 1);	
					}
					seduce_background_quad_draw(input, NULL, 0,
													pos_x + f_start, pos_y + y - modes[start_block].letter_size * 0.5, 0, 
													pos_x + f_start, pos_y + y + modes[start_block].letter_size * 1.5, 0, 
													pos_x + f_end, pos_y + y + modes[start_block].letter_size * 1.5, 0, 
													pos_x + f_end, pos_y + y - modes[start_block].letter_size * 0.5, 0, 
													0, 0, 0,
													0, 0, 0, 0.0);
				}				
				if(pos + line_end >= select_end)
				{
					float center[3] = {0, 0, 0};
					seduce_background_polygon_flush(input,center, 1.0);
					return;
				}
			}
			x += seduce_text_line_length_internal(modes[start_block].font, modes[start_block].letter_size, modes[start_block].letter_spacing, space_size, &text[pos], &line_end, line_size + 1);			
			pos += line_end;
			if(pos >= accum)
				break;
		}
	}
	return;
}



boolean seduce_text_block_length(float *output, float pos_x, float pos_y, float line_size, float height, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count, uint end)
{
	uint i, j, next, gaps, start_block = 0, block, line_end, accum;
	float f, size, space_size, x = 0, y = 0, highest;
	height = -height;
	for(start_block = 0; start_block + 1 < mode_count && pos > modes[start_block + 1].character_position; start_block++);
	for(i = 0; text[pos] != 0; i++)
	{
		accum = pos;
		if(NULL == seduce_character_find(modes[start_block].font, f_utf8_to_uint32(text, &accum)))
			pos = accum;
		else 
			accum = pos;
		x = line_size;
		highest = modes[start_block].letter_size;
		gaps = 0;
		for(block = start_block; ; block++)
		{
			if(block + 1 < mode_count)
				line_end = modes[block + 1].character_position - accum;
			else
				line_end = -1;
			next = seduce_text_block_hit_test(modes[block].font, &size, &gaps, modes[block].letter_size, modes[block].letter_spacing, &text[accum], x, line_end);
			x -= size * modes[block].letter_size;
			accum += next;
			if(next != line_end)
				break;
			if(highest < modes[block + 1].letter_size)	
				highest = modes[block + 1].letter_size;
		}
		j = accum;
		if(NULL == seduce_character_find(modes[start_block].font, f_utf8_to_uint32(text, &j)))
			gaps--;

		y -= line_spacing * highest;
		if(y < height)
		{
			output[0] = pos_x + x;
			output[1] = pos_y + y;
			return FALSE;
		}
		if(accum >= end)
		{

			switch(style)
			{
				case SEDUCE_TBAS_LEFT :
					x = 0;
					space_size = 0.0;
				break;
				case SEDUCE_TBAS_RIGHT :
					x = x;
					space_size = 0.0;
				break;
				case SEDUCE_TBAS_CENTER :
					x = x * 0.5;
					space_size = 0.0;
				break;
				case SEDUCE_TBAS_STRETCH :
					space_size = x / (float)gaps;
					x = 0;
				break;
			}
			for(start_block; start_block <= block; start_block++)
			{
				if(start_block + 1 < mode_count)
					line_end = modes[start_block + 1].character_position - pos;
				else
					line_end = -1;
				if(line_end > accum - pos)
					line_end = accum - pos;	
				if(line_end > end - pos)
					line_end = end - pos;
				x += seduce_text_line_length_internal(modes[start_block].font, modes[start_block].letter_size, modes[start_block].letter_spacing, space_size, &text[pos], &line_end, line_size + 1);
				if(line_end == end - pos)
				{
					output[0] = pos_x + x;
					output[1] = pos_y + y;
					return TRUE;
				}
				pos += line_end;


				if(pos >= accum)
					break;
			}
		}else
		{
			pos = accum;
			for(start_block; start_block + 1 < mode_count && modes[start_block + 1].character_position <= pos; start_block++);
		}
	}
	output[0] = pos_x + x;
	output[1] = pos_y + y;
	return FALSE;
}


float seduce_text_block_height(float line_size, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count, uint end)
{
	float output[2];
	seduce_text_block_length(output, 0, 0, line_size, 100000000000, line_spacing, style, text, pos, modes, mode_count, end);
	return -output[1];
}


uint seduce_text_block_hit_detect(float pos_x, float pos_y, float line_size, float height, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count, float pointer_x, float pointer_y)
{
	uint i, j, next, gaps, start_block = 0, block, line_end, accum;
	float f, size, space_size, x, y, highest;

	if(pointer_y > pos_y)
		return 0;
	height = -height;
	y = 0;
	for(start_block = 0; start_block + 1 < mode_count && pos > modes[start_block + 1].character_position; start_block++);
	for(i = 0; text[pos] != 0; i++)
	{
		accum = pos;
		if(NULL == seduce_character_find(modes[start_block].font, f_utf8_to_uint32(text, &accum)))
			pos = accum;
		else 
			accum = pos;
		x = line_size;
		highest = modes[start_block].letter_size;
		gaps = 0;
		for(block = start_block; ; block++)
		{
			if(block + 1 < mode_count)
				line_end = modes[block + 1].character_position - accum;
			else
				line_end = -1;
			next = seduce_text_block_hit_test(modes[block].font, &size, &gaps, modes[block].letter_size, modes[block].letter_spacing, &text[accum], x, line_end);
			x -= size * modes[block].letter_size;
			accum += next;
			if(next != line_end)
				break;
			if(highest < modes[block + 1].letter_size)	
				highest = modes[block + 1].letter_size;
		}
		j = accum;
		if(NULL == seduce_character_find(modes[start_block].font, f_utf8_to_uint32(text, &j)))
			gaps--;

		y -= line_spacing * highest;
		if(y < height)
			return pos;
		if(pos_y + y - highest < pointer_y)
		{
			switch(style)
			{
				case SEDUCE_TBAS_LEFT :
					x = 0;
					space_size = 0.0;
				break;
				case SEDUCE_TBAS_RIGHT :
					x = x;
					space_size = 0.0;
				break;
				case SEDUCE_TBAS_CENTER :
					x = x * 0.5;
					space_size = 0.0;
				break;
				case SEDUCE_TBAS_STRETCH :
					space_size = x / (float)gaps;
					x = 0;
				break;
			}
			for(start_block; start_block <= block; start_block++)
			{
				if(start_block + 1 < mode_count)
					line_end = modes[start_block + 1].character_position - pos;
				else
					line_end = -1;
				if(line_end > accum - pos)
					line_end = accum - pos;

				j = line_end;
				x += seduce_text_line_length_internal(modes[start_block].font, modes[start_block].letter_size, modes[start_block].letter_spacing, space_size,  &text[pos], &j, pointer_x - x - pos_x);
				if(line_end != j)
					return pos + j;
				pos += line_end;
				if(pos >= accum)
					break;
			}
			return pos;
		}else
		{
			pos = accum;
			for(start_block; start_block + 1 < mode_count && modes[start_block + 1].character_position <= pos; start_block++);
		}
	}
	return FALSE;
}

