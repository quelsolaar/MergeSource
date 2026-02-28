#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "seduce.h"

/*
extern void		sui_text_line_cursur_draw(float pos_x, float pos_y, float size, float curser_x, float curser_y);
extern void		sui_text_line_selection_draw(float pos_x, float pos_y, float size, float curser_start, float curser_end, float curser_y);*/
extern uint		sui_text_block_line_end(const char *text, float spacing, float letter_size, float size);
extern void		sui_text_block_pos(float letter_size, float letter_spacing, float line_size, float line_spacing, const char *text, uint curser, float *pos_x, float *pos_y);

boolean sui_util_edit_insert_character(char *text, uint size, uint curser_start, uint curser_end, char *insert, STextBlockMode *modes, uint mode_count)
{
	uint i, in_end, out_end;
	char a[1280], b[1280];
	if(curser_start > curser_end)
	{
		i = curser_start;
		curser_start = curser_end;
		curser_end = i;
	}
	for(out_end = 0; text[out_end] != 0; out_end++);
	for(in_end = 0; insert[in_end] != 0; in_end++);
	if(out_end + in_end + curser_start - curser_end > size)
		return FALSE;
	if(in_end > curser_end - curser_start)
	{	
		for(i = out_end + in_end; i >= curser_end + in_end; i--)
			text[i - (curser_end - curser_start)] = text[i - in_end];
		for(i = 0; i < in_end; i++)
			text[curser_start + i] = insert[i];
	}else
	{	
		for(i = curser_end; i < out_end; i++)
			text[i + in_end - (curser_end - curser_start)] = text[i];
		text[i + in_end - (curser_end - curser_start)] = 0;
		for(i = 0; i < in_end; i++)
			text[curser_start + i] = insert[i];
	}

	for(i = 0; i < mode_count; i++)
	{
		if(modes[i].character_position > curser_end)
			modes[i].character_position += in_end - (curser_end - curser_start);
		else if(modes[i].character_position >= curser_start)
			modes[i].character_position = in_end + curser_start;			
	}


	return TRUE;
}


void sui_text_line_edit_draw(BInputState *input, SeduceRenderFont *font, float pos_x, float pos_y, float size, float spacing, float length, char *text, uint *scroll_start, uint select_start, uint select_end, float red, float green, float blue, float alpha)
{
	static uint seed = 0;
	float curser_start, curser_end;
	uint end;
	end = seduce_text_line_hit_test(font, size, spacing, &text[*scroll_start], length) + *scroll_start;
	/* find and draw the selection */

	if(select_start <= *scroll_start) // does the select start before the beginning of the visable text
		curser_start = 0; // set the start to zero
	else
		curser_start = seduce_text_line_length(font, size, spacing, &text[*scroll_start], select_start - *scroll_start); // find the position of the begining of the select

	if(select_start == select_end) /* are we in cursor or select mode? */
	{
		seduce_background_negative_draw(pos_x + curser_start - 0.1 * size, pos_y - size * 0.6, pos_x + curser_start + 0.1 * size, pos_y - size * 0.5, pos_x + curser_start + 0.1 * size, pos_y + size * 1.6, pos_x + curser_start - 0.1 * size, pos_y + size * 1.5, 1.0);
	//	r_primitive_line_2d(pos_x + curser_start, pos_y - size * 0.5, pos_x + curser_start, pos_y + size * 0.5, red, green, blue, alpha); /* draw the cursor*/
	}else 
	{
		float center[3] = {0, 0, 0};
		if(select_end > end) // does the select start before the beginning of the visable text
			curser_end = length; // set the end to the length of the box
		else
			curser_end = seduce_text_line_length(font, size, spacing, &text[*scroll_start], select_end - *scroll_start); // find the position of the end of the select
		
		seduce_background_quad_draw(input, NULL, 0, pos_x + curser_start, pos_y - size * 0.5, 0,
																	pos_x + curser_start * 0.5 + curser_end * 0.5, pos_y - size * 0.7, 0,
																	pos_x + curser_start * 0.5 + curser_end * 0.5, pos_y + size * 1.9, 0,
																	pos_x + curser_start, pos_y + size * 1.7, 0,
																	0, 0, 1,
																	0, 0, 0, 0.3);
		seduce_background_quad_draw(input, NULL, 0, pos_x + curser_start * 0.5 + curser_end * 0.5, pos_y - size * 0.7, 0,
																	pos_x + curser_end, pos_y - size * 0.5, 0,
																	pos_x + curser_end, pos_y + size * 1.7,  0,
																	pos_x + curser_start * 0.5 + curser_end * 0.5, pos_y + size * 1.9, 0,
																	0, 0, 1,
																	0, 0, 0, 0.3);

		seduce_background_polygon_flush(input, center, 1.0);
/*
		pos_x += curser_start + (curser_end - curser_start) * f_randf(seed++);
		pos_y += size * 0.7;
		seduce_background_particle_spawn(input, pos_x, pos_y, 0.4 * f_randnf(seed + 1), 0.4 * f_randnf(seed + 2), 0.5, seduce_background_particle_color_allocate(text, red, green, blue));
		*/
	}
	if(select_end < *scroll_start && *scroll_start != 0) /* if we are at the begining of our view and there is more text scroll back */
		(*scroll_start)--;
	if(select_end > end && text[end] != 0)  /* if we are att the end of our view and there is more text scroll forward */
		(*scroll_start)++;
/*	r_primitive_line_2d(pos_x, pos_y - size, pos_x + length, pos_y - size, red, green, blue, alpha);
	r_primitive_line_flush();*/
}

extern boolean seduce_text_block_length(float *output, float pos_x, float pos_y, float line_size, float height, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count, uint end);

/*
void sui_text_box_edit_draw2(float size, float spacing, float line_spacing, float length, char *text, uint lines, uint *line_start, uint select_start, uint select_end, boolean draw_curser)

{
	float curser_start, curser_end;
	float x_start, y_start, x_end, y_end;
	uint end, scroll_start = 0, i;

	for(i = 0; i < *line_start; i++)
		scroll_start += sui_text_block_line_end(&text[scroll_start], spacing, size, length);
//	seduce_text_block_draw_old(size, spacing, length, line_spacing, lines, &text[scroll_start]);

	if(!draw_curser)
		return;
	if(select_start > select_end)
	{
		i = select_end;
		select_end = select_start;
		select_start = i;
	}
	if(select_end < scroll_start)
		return;
	if(select_start < scroll_start)
		select_start = scroll_start;

	if(select_start < scroll_start)
		x_start = y_start = 0;
	else
		sui_text_block_pos(size, spacing, length, line_spacing, &text[scroll_start], select_start - scroll_start, &x_start, &y_start);

	if(select_start == select_end)
	{
	//	sui_text_line_cursur_draw(0, 0, size, x_start, y_start);	
		return;
	}

	sui_text_block_pos(size, spacing, length, line_spacing, &text[scroll_start], select_end - scroll_start, &x_end, &y_end);
	
//	for(i = (uint)((y_start + 0.5 * size) / (size * line_spacing)); i < lines; i++)
	for(i = 0; i < 100; i++)
	{
		if(y_start + 0.5 * size > y_end && y_start - 0.5 * size < y_end)
		{
	//		sui_text_line_selection_draw(0, 0, size, x_start, x_end, y_start);
			return;
		}else
		{
	//		sui_text_line_selection_draw(0, 0, size, x_start, length, y_start);
		}
		y_start -= size * line_spacing;
		x_start = 0;
	}

	r_matrix_push(NULL);

	r_matrix_pop(NULL);
}*/

void sui_text_line_edit_mouse(SeduceRenderFont *font, float size, float spacing, float length, char *text, uint *scroll_start, uint *select_start, uint *select_end, boolean mouse_button, boolean mouse_button_last, float pointer_x)
{
	if(mouse_button) /* is the mouser_matrix_pop(NULL); in action */
	{
		uint i;
		uint end;
///		uint seduce_text_line_hit_test(float letter_size, float letter_spacing, const char *text, float pos_x)
		end = seduce_text_line_hit_test(font, size, spacing, &text[*scroll_start], length) + *scroll_start;
		i = seduce_text_line_hit_test(font, size, spacing, &text[*scroll_start], pointer_x) + *scroll_start; /* find the place of the select*/
		if(!mouse_button_last) // just clicked
			*select_start = *select_end = i;
		else // keep clicking
			*select_end = i;

		if(*select_end < *scroll_start && scroll_start != 0) /* if we are at the begining of our view and there is more text scroll back */
			(*scroll_start)--;
		if(*select_end > end && text[end] != 0)  /* if we are att the end of our view and there is more text scroll forward */
			(*scroll_start)++;
	}	
}
/*
	sui_text_box_edit_mouse(size, spacing, length, line_spacing, text, lines, &line_start, &select_start, &select_end, mouse_button, last_mouse_button, pointer_x, pointer_y);
	seduce_text_block_hit_test(size, spacing, length, line_spacing, lines, text, pointer_x, pointer_y)
*/

/*
typedef struct{
	float pos_x;
	float pos_y;
	float line_size;
	float height;
	float line_spacing;
	STextBlockAlignmentStyle style;
}STextBox;
*/

uint seduce_text_block_hit_detect(float pos_x, float pos_y, float line_size, float height, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count, float pointer_x, float pointer_y);

void sui_text_box_edit_mouse(char *text, STextBox *boxes, uint box_count, STextBlockMode *modes, uint mode_count, uint *select_start, uint *select_end, boolean mouse_button, boolean mouse_button_last, float pointer_x, float pointer_y)
{
	if(mouse_button) /* is the mouser_matrix_pop(NULL); in action */
	{
		uint hit, i, pos = 0, found = 0;
		float closest = 1000000, f, dist;

		for(i = 0; i < box_count; i++)
		{
			if(pointer_x < boxes[i].pos_x + boxes[i].line_size &&
				pointer_x > boxes[i].pos_x &&
				pointer_y > boxes[i].pos_y - boxes[i].height &&
				pointer_y < boxes[i].pos_y)
			{
				found = i;
				break;
			}
			dist = 0;
			if(pointer_x > boxes[i].pos_x + boxes[i].line_size)
			{
				f = pointer_x - (boxes[i].pos_x + boxes[i].line_size);
				if(f > dist)
					dist = f;
			}
			if(pointer_x < boxes[i].pos_x)
			{
				f = boxes[i].pos_x - pointer_x;
				if(f > dist)
					dist = f;
			}

			if(pointer_y > boxes[i].pos_y)
			{
				f = pointer_y - boxes[i].pos_y;
				if(f > dist)
					dist = f;
			}
			if(pointer_y < boxes[i].pos_y - boxes[i].height)
			{
				f = boxes[i].pos_y - boxes[i].height - pointer_y;
				if(f > dist)
					dist = f;
			}

			if(dist < closest)
			{
				found = i;
				closest = dist;
			}
		}


		i = 0;
		for(i = 0; i < found + 1; i++)
		{
			if(found == i)
			{
				hit = seduce_text_block_hit_detect(boxes[i].pos_x, boxes[i].pos_y, boxes[i].line_size, boxes[i].height, boxes[i].line_spacing, boxes[i].style, text, pos, modes, mode_count, pointer_x, pointer_y);
			}else
			{
				pos += seduce_text_block_hit_detect(boxes[i].pos_x, boxes[i].pos_y, boxes[i].line_size, boxes[i].height, boxes[i].line_spacing, boxes[i].style, text, pos, modes, mode_count, boxes[i].pos_x + boxes[i].line_size + 1, boxes[i].pos_y - boxes[i].height - 1);
			}
		}
		if(!mouse_button_last) // just clicked
			*select_start = *select_end = hit;
		else
			*select_end = hit;
	}
}


void sui_text_monospace_edit_mouse(char *text, float pos_x, float pos_y, float character_size, float space_size, float line_size, uint line_count, uint line_length, uint *scroll, uint *select_start, uint *select_end, boolean mouse_button, boolean mouse_button_last, float pointer_x, float pointer_y)
{
	uint line = 0, character = 0, i = 0, j, count, id;
	if(mouse_button) /* is the mouser_matrix_pop(NULL); in action */
	{
		if(pos_y > pointer_y)
			line = (pos_y - pointer_y) / line_size;
		else if(scroll[1] != 0)
			scroll[1]--;

		if(pos_x < pointer_x)
			character = (pointer_x - pos_x) / space_size;

		if(line + scroll[1] != 0)
		{
			for(i = count = 0; text[i] != 0; i++)
			{
				if(text[i] == '\n')
				{
					count++;
					if(count == scroll[1] + line)
					{
						i++;
						break;
					}
				}
			}
		}
		for(j = 0; j < character && text[i] != 0; j++)
		{
			id = f_utf8_to_uint32(text, &i);
			if(id == '\n')
				break;
			if(id == '\t')
				j += 3;
		}
	
		if(!mouse_button_last) // just clicked
			*select_start = *select_end = i;
		else
			*select_end = i;
	}
}

void seduce_text_monospace_edit_scroll(char *text, uint line_count, uint line_length, uint *scroll, uint curser)
{
	uint i, line_start = 0, count = 0, length = 0;
	for(i = 0; i < curser; i++)
	{
		if(text[i] == '\n')
		{
			line_start = i + 1;
			count++;
		}
	}
	for(i = line_start; i < curser && text[i] != 0; f_utf8_to_uint32(text, &i))
		length++;	
	
	if(count < scroll[1])
		scroll[1] = count;
	if(count - scroll[1] >= line_count)
		scroll[1] = count - line_count + 1;

	if(length < scroll[0])
		scroll[0] = length;
	if(length - scroll[0] >= line_length)
		scroll[0] = length - line_length + 3;
}


void seduce_text_edit_up(char *text, uint *select_start, uint *select_end)
{
	uint i, line = -1, previous_line = -1, count;
	for(i = 0; i < *select_end; i++)
	{
		if(text[i] == '\n')
		{
			previous_line = line;
			line = i;
		}
	}
	if(previous_line != line)
	{
		count = i - line;
		if(line - previous_line > count)
			*select_start = *select_end = previous_line + count;
		else
			*select_start = *select_end = line - 1;
	}
}

void seduce_text_edit_down(char *text, uint *select_start, uint *select_end)
{
	uint i, j, line = 0, count;
	for(i = 0; i < *select_end; i++)
		if(text[i] == '\n')
			line = i;
	count = i - line;
	for(; text[i] != 0 && text[i] != '\n'; i++);
	if(text[i] == 0)
		return;
	for(j = 1; j < count && text[i + j] != '\n' && text[i + j] != 0; j++);
	if(text[i + j] == 0)
		j--;
	*select_start = *select_end = i + j;
}

void seduce_text_edit_forward(char *text, uint *select_start, uint *select_end)
{
	if(select_end > select_start)
		*select_start = *select_end;
	else
		*select_end = *select_start;
	if(text[*select_end] != 0)
		*select_end = *select_start = *select_start + 1;
}

void seduce_text_edit_back(char *text, uint *select_start, uint *select_end)
{
	if(*select_end < *select_start)
		*select_start = *select_end;
	else
		*select_end = *select_start;
	if(*select_end != 0)
		*select_end = *select_start = *select_start - 1;
}

void seduce_text_edit_delete(char *text, uint length, uint *select_start, uint *select_end, STextBlockMode *modes, uint mode_count)
{
	char insert[1] = {0};
	if(*select_end == *select_start && text[*select_end] != 0)
		(*select_end)++;
	sui_util_edit_insert_character(text, length, *select_start, *select_end, insert, modes, mode_count);
	if(*select_start > *select_end)
		*select_start = *select_end;
	else
		*select_end = *select_start;
}

void seduce_text_edit_end(char *text, uint length, uint *select_start, uint *select_end)
{
	uint i;
	for(i = 0; text[i] != 0; i++);
		*select_end = *select_start = i;
}

void seduce_text_edit_home(char *text, uint length, uint *select_start, uint *select_end)
{
	*select_end = *select_start = 0;
}

void seduce_text_edit_backspace(char *text, uint length, uint *select_start, uint *select_end, STextBlockMode *modes, uint mode_count)
{
	char insert[1] = {0};
	if(*select_end == *select_start && *select_end != 0)
		(*select_end)--;
	sui_util_edit_insert_character(text, length, *select_start, *select_end, insert, modes, mode_count);
	if(*select_start > *select_end)
		*select_start = *select_end;
	else
		*select_end = *select_start;
}

void seduce_text_edit_insert_character(char *text, uint length, uint *select_start, uint *select_end, uint32 character, STextBlockMode *modes, uint mode_count)
{
	char insert[7];
	insert[f_uint32_to_utf8(character, insert)] = 0;
	sui_util_edit_insert_character(text, length, *select_start, *select_end, insert, modes, mode_count);
	if(*select_start > *select_end)
		*select_start = *select_end = *select_end + 1;
	else
		*select_end = *select_start = *select_start + 1;
}


void seduce_text_edit_paste(char *text, uint length, uint *select_start, uint *select_end, STextBlockMode *modes, uint mode_count)
{
	uint i;
	char *paste;
	if(*select_start > *select_end)
	{
		i = *select_start;
		*select_start = *select_end;
		*select_end = i;
	}
	paste = betray_clipboard_get();
	if(sui_util_edit_insert_character(text, length, *select_start, *select_end, paste, modes, mode_count));
	{
		for(i = 0; 0 != paste[i]; i++);
		*select_start = *select_end = *select_start + i;
		if(*select_end > length)
			*select_end = length;
	}
}

void seduce_text_edit_cut(char *text, uint length, uint *select_start, uint *select_end, STextBlockMode *modes, uint mode_count)
{
	char character;
	uint i;
	char *paste;
	if(*select_start > *select_end)
	{
		i = *select_start;
		*select_start = *select_end;
		*select_end = i;
	}
	paste = betray_clipboard_get();
	character = text[*select_end];
	text[*select_end] = 0;
	betray_clipboard_set(&text[*select_start]);
	text[*select_end] = character;
	character = 0;
	sui_util_edit_insert_character(text, length, *select_start, *select_end, &character, modes, mode_count);
	*select_end = *select_start;
}

void seduce_text_edit_copy(char *text, uint length, uint *select_start, uint *select_end)
{
	char character;
	uint i;
	char *paste;
	if(*select_start > *select_end)
	{
		i = *select_start;
		*select_start = *select_end;
		*select_end = i;
	}
	paste = betray_clipboard_get();
	character = text[*select_end];
	text[*select_end] = 0;
	betray_clipboard_set(&text[*select_start]);
	text[*select_end] = character;
	character = 0;
}
/*
typedef struct{
	uint character_position;
	void *font;
	float red;
	float green; 
	float blue;
	float alpha;
	float letter_size;
	float letter_spacing;
}STextBlockMode;*/

STextBlockMode *seduce_text_syntax_highlight(char *text, uint start, uint end, char **key_words, float *colors, uint key_word_count, float red, float green, float blue, float alpha, uint *block_count)
{
	STextBlockMode *blocks = NULL;
	uint i, j, k;
	boolean test = TRUE;
	*block_count = 0;
	blocks = realloc(blocks, (sizeof *blocks) * 256);
	blocks[*block_count].character_position = 0;
	blocks[*block_count].red = red;
	blocks[*block_count].green = green;
	blocks[*block_count].blue = blue;
	blocks[*block_count].alpha = alpha;
	(*block_count)++;

	for(i = start; text[i] != 0; i++)
	{
		if(test)
		{
			for(j = 0; j < key_word_count; j++)
			{
				for(k = 0; key_words[j][k] == text[i + k] && key_words[j][k] != 0; k++);
				if(key_words[j][k] == 0)
				{
					if(text[i + k] != '_' && (text[i + k] < '0' || text[i + k] > 'z' || (text[i + k] > 'Z' && text[i + k] < 'a') || (text[i + k] > '9' && text[i + k] < 'A')))
					{
						if(blocks[*block_count - 1].character_position == i)
							(*block_count)--;
						else if(*block_count % 256 == 0)
							blocks = realloc(blocks, (sizeof *blocks) * (*block_count + 256));
						blocks[*block_count].character_position = i;
						blocks[*block_count].red = colors[j * 4 + 0];
						blocks[*block_count].green = colors[j * 4 + 1];
						blocks[*block_count].blue = colors[j * 4 + 2];
						blocks[*block_count].alpha = colors[j * 4 + 3];
						(*block_count)++;
						if(*block_count % 256 == 0)
							blocks = realloc(blocks, (sizeof *blocks) * (*block_count + 256));
						blocks[*block_count].character_position = i + k;
						blocks[*block_count].red = red;
						blocks[*block_count].green = green;
						blocks[*block_count].blue = blue;
						blocks[*block_count].alpha = alpha;
						(*block_count)++;
						i += k - 1;
						break;
					}
				}
			}
		}
		test = text[i] != '_' && (text[i] < '0' || text[i] > 'z' || (text[i] > 'Z' && text[i] < 'a') || (text[i] > '9' && text[i] < 'A'));
	}
	return blocks;
}