#include <stdio.h>
#include <stdlib.h>
#include "seduce.h"

extern void		sui_text_line_edit_mouse(SeduceRenderFont *font, float size, float spacing, float length, char *text, uint *scroll_start, uint *select_start, uint *select_end, boolean mouse_button, boolean mouse_button_last, float pointer_x);
extern void		sui_text_line_edit_draw(BInputState *input, SeduceRenderFont *font, float pos_x, float pos_y, float size, float spacing, float length, char *text, uint *scroll_start, uint select_start, uint select_end, float red, float green, float blue, float alpha);
extern void		sui_text_box_edit_mouse(char *text, STextBox *boxes, uint box_count, STextBlockMode *modes, uint mode_count, uint *select_start, uint *select_end, boolean mouse_button, boolean mouse_button_last, float pointer_x, float pointer_y);
extern void		sui_text_monospace_edit_mouse(char *text, float pos_x, float pos_y, float character_size, float space_size, float line_size, uint line_count, uint line_length, uint *scroll, uint *select_start, uint *select_end, boolean mouse_button, boolean mouse_button_last, float pointer_x, float pointer_y);


extern void seduce_text_edit_up(char *text, uint *select_start, uint *select_end);
extern void seduce_text_edit_down(char *text, uint *select_start, uint *select_end);
extern void seduce_text_edit_forward(char *text, uint *select_start, uint *select_end);
extern void seduce_text_edit_back(char *text, uint *select_start, uint *select_end);
extern void seduce_text_edit_delete(char *text, uint length, uint *select_start, uint *select_end, STextBlockMode *modes, uint mode_count);
extern void seduce_text_edit_end(char *text, uint length, uint *select_start, uint *select_end);
extern void seduce_text_edit_home(char *text, uint length, uint *select_start, uint *select_end);
extern void seduce_text_edit_backspace(char *text, uint length, uint *select_start, uint *select_end, STextBlockMode *modes, uint mode_count);
extern void seduce_text_edit_insert_character(char *text, uint length, uint *select_start, uint *select_end, uint32 character, STextBlockMode *modes, uint mode_count);
extern void seduce_text_edit_paste(char *text, uint length, uint *select_start, uint *select_end, STextBlockMode *modes, uint mode_count);
extern void seduce_text_edit_cut(char *text, uint length, uint *select_start, uint *select_end, STextBlockMode *modes, uint mode_count);
extern void seduce_text_edit_copy(char *text, uint length, uint *select_start, uint *select_end);


typedef struct{
	void *id;
	uint scroll_start;
	uint select_start; 
	uint select_end;
	uint pointer_active;
	char *copy;
	uint copy_allocated;
	void *done_func;
}STextEditState;




static STextEditState *seduce_text_state;

static uint sui_type_in_cursor = 0;
static char *sui_type_in_text = 0;
static char *sui_type_in_copy = 0;
static char *sui_return_text = 0;
static void (* sui_type_in_done_func)(void *user, char *text); 
/*
boolean seduce_text_button(BInputState *input, float pos_x, float pos_y, float center, float size, float spacing, const char *text, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select)
{
	if(input->mode == BAM_DRAW)
	{
		float f, length, brightness = 0;
		uint i;
		length = seduce_text_line_length(NULL, size, spacing, text, -1);
		center *= length;
		for(i = 0; i < input->pointer_count; i++)
		{
			if(sui_box_click_test(input->pointers[i].pointer_x, 
				input->pointers[i].pointer_y, 
				pos_x - center, 
				pos_y - size * 0.5, 
				length, size * 2.5))
			{
				if(brightness < 0.3)
					brightness = 0.5;
				if(input->pointers[i].button[0] || input->pointers[i].last_button[0])
					if(sui_box_click_test(input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0], pos_x - center, pos_y - size * 0.5, length, size * 2.5))
						brightness = 1.0;
			}
		}
		seduce_text_line_draw(NULL, pos_x - center, pos_y, size, spacing, text, 
			red + (red_select - red) * brightness, 
			green + (green_select - green) * brightness, 
			blue + (blue_select - blue) * brightness, 
			alpha + (alpha_select - alpha) * brightness, -1);
	}else if(input->mode == BAM_EVENT)
	{
		float length;
		uint i;
		length = seduce_text_line_length(NULL, size, spacing, text, -1);
		center *= length;
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
				if(sui_box_click_test(input->pointers[i].pointer_x, input->pointers[i].pointer_y, pos_x - center, pos_y - size * 0.5, length, size * 2.5) && 
				   sui_box_click_test(input->pointers[i].click_pointer_x[0], input->pointers[i].click_pointer_y[0], pos_x - center, pos_y - size * 0.5, length, size * 2.5))
					return TRUE;
	}
	return FALSE;
}
*/
boolean seduce_text_button(BInputState *input, void *id, float pos_x, float pos_y, float center, float size, float spacing, const char *text, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select)
{
	uint i;
	if(input->mode == BAM_DRAW)
	{
		boolean acticve = FALSE;
		float length;
		for(i = 0; i < input->pointer_count; i++)
			if(id == seduce_element_pointer_id(input, i, NULL))
				break;
		acticve = i < input->pointer_count;

		for(i = 0; i < input->user_count; i++)
			if(id == seduce_element_selected_id(i, NULL, NULL))
				if(betray_button_get(i, BETRAY_BUTTON_FACE_A))
				acticve = TRUE;

		if(center > 0.001)
		 {
			 length = seduce_text_line_length(NULL, size, spacing, text, -1);
			 pos_x = pos_x - length * center;
		}
		if(acticve)
			length = seduce_text_line_draw(NULL, pos_x, pos_y, size, spacing, text, red_select, green_select, blue_select, alpha_select, -1);
		else
			length = seduce_text_line_draw(NULL, pos_x, pos_y, size, spacing, text, red, green, blue, alpha, -1);

		seduce_element_add_rectangle(input, id, 0, pos_x, pos_y - size, length, size * 3.0);
	//	seduce_element_add_line(input, id, 0, a, b, size * 8.0);
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
			if(!input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, NULL))
					return TRUE;
		for(i = 0; i < input->user_count; i++)
			if(id == seduce_element_selected_id(i, NULL, NULL))
				if(betray_button_get(i, BETRAY_BUTTON_FACE_A))
					return TRUE;
	}
	return FALSE;
}


// uint seduce_text_button_list(BInputState *input, void *id, float pos_x, float pos_y, float center, float size, float spacing, const char **texts, uint text_count, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select);

#define SEDUCE_BUTTON_LIST_GAP_SIZE 4.0

uint seduce_text_button_list(BInputState *input, void *id, float pos_x, float pos_y, float length, STextBlockAlignmentStyle style, float size, float spacing, float line_size, const char **texts, uint text_count, uint selected, float red, float green, float blue, float alpha, float red_select, float green_select, float blue_select, float alpha_select)
{
	uint i, part;
	if(text_count > 64)
	{
		printf("Seduce ERROR: seduce_text_button_list called with more then 64 strings of text (%u)", text_count);
		text_count = 64;
	}
	if(input->mode == BAM_DRAW)
	{
		float lengths[64], sum = 0;
		boolean acticve[64] = {FALSE}, colapse = FALSE;
		float x_start, x, a[3], b[3], gap;
		char letter[2] = {0, 0};
		gap = size * SEDUCE_BUTTON_LIST_GAP_SIZE;
		for(i = 0; i < input->pointer_count; i++)
			acticve[i] = FALSE;
		if(selected < text_count)
			acticve[selected] = TRUE;
		for(i = 0; i < input->pointer_count; i++)
			if(id == seduce_element_pointer_id(input, i, &part))
				acticve[part] = TRUE;

		for(i = 0; i < input->user_count; i++)
			if(id == seduce_element_selected_id(i, NULL, &part))
				if(betray_button_get(i, BETRAY_BUTTON_FACE_A))
					acticve[part] = TRUE;

		for(i = 0; i < text_count; i++)
		{
			lengths[i] = seduce_text_line_length(NULL, size, spacing, texts[i], -1);
			sum += lengths[i];
		}
		if(gap * (text_count - 1) + sum > length)
		{
			colapse = TRUE;
			sum = 0;
			for(i = 0; i < text_count; i++)
			{
				letter[0] = texts[i][0];
				lengths[i] = seduce_text_line_length(NULL, size, spacing, letter, -1);
				sum += lengths[i];
			}
		}
		switch(style)
		{
			case SEDUCE_TBAS_LEFT :
				x_start = pos_x;
			break;
			case SEDUCE_TBAS_RIGHT :
				x_start = pos_x - sum - gap * (float)(text_count - 1);
			break;
			case SEDUCE_TBAS_CENTER :
				x_start = pos_x - (sum + gap * (float)(text_count - 1)) * 0.5;
			break;
			case SEDUCE_TBAS_STRETCH :
				x_start = pos_x;
				gap = (length - sum) / (float)(text_count - 1);
			break;
		}
		x = x_start;
		for(i = 0; i < text_count; i++)
		{
			a[0] = x;
			a[1] = b[1] = pos_y;
			a[2] = b[2] = 0;
			if(colapse)
			{
				letter[0] = texts[i][0];
				if(acticve[i])
					seduce_text_line_draw(NULL, x, pos_y, size, spacing, letter, red_select, green_select, blue_select, alpha_select, -1);
				else
					seduce_text_line_draw(NULL, x, pos_y, size, spacing, letter, red, green, blue, alpha, -1);
			}else
			{
				if(acticve[i])
					seduce_text_line_draw(NULL, x, pos_y, size, spacing, texts[i], red_select, green_select, blue_select, alpha_select, -1);
				else
					seduce_text_line_draw(NULL, x, pos_y, size, spacing, texts[i], red, green, blue, alpha, -1);
			}
			b[0] = a[0] + lengths[i];
			seduce_element_add_rectangle(input, id, i, x, pos_y - size, lengths[i], size * 3.0);
		//	seduce_element_add_line(input, id, i, a, b, size * 4.0);
			x += lengths[i] + gap;
		}
		x = x_start;
		for(i = 0; i < text_count - 1; i++)
		{
			x += lengths[i] + gap * 0.5;
			r_primitive_line_2d(x, pos_y + size * 0.75 - line_size, x, pos_y + size * 0.75 + line_size, red_select, green_select, blue_select, alpha_select);
			x += gap * 0.5;
		}
		r_primitive_line_flush();
	}
	if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
			if(!input->pointers[i].button[0] && input->pointers[i].last_button[0])
				if(id == seduce_element_pointer_id(input, i, &part))
					return part;
		for(i = 0; i < input->user_count; i++)
			if(id == seduce_element_selected_id(i, NULL, &part))
				if(betray_button_get(i, BETRAY_BUTTON_FACE_A))
					return part;
	}
	return -1;
}

void seduce_text_edit_init()
{
	uint i, count;
	count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	seduce_text_state = malloc((sizeof *seduce_text_state) * count);
	for(i = 0; i < count; i++)
	{
		seduce_text_state[i].id = NULL;
		seduce_text_state[i].scroll_start = 0;
		seduce_text_state[i].select_start = 0;
		seduce_text_state[i].select_end = 0;
		seduce_text_state[i].pointer_active = -1;
		seduce_text_state[i].done_func = NULL;
		seduce_text_state[i].copy = NULL;
		seduce_text_state[i].copy_allocated = 0;
	}
}

void seduce_text_activate(uint user_id, void *id,  char *text, uint buffer_size, void *done_func, void *user, boolean copy, uint pointer_id)
{
	char *t;
	uint i;
	if(user_id >= betray_support_functionality(B_SF_USER_COUNT_MAX))
		return;
	seduce_text_state[user_id].pointer_active = pointer_id;
	if(seduce_text_state[user_id].id == id)
		return;
	seduce_text_state[user_id].id = id;
	seduce_text_state[user_id].scroll_start = 0;
	seduce_text_state[user_id].select_start = 0;
	seduce_text_state[user_id].select_end = 0;
	seduce_text_state[user_id].done_func = done_func;
	if(copy)
	{
		if(buffer_size < 32)
			buffer_size = 32;
		t = seduce_text_state[user_id].copy = malloc((sizeof *seduce_text_state[user_id].copy) * buffer_size);
		seduce_text_state[user_id].copy_allocated = buffer_size;
		for(i = 0; text != NULL && i < buffer_size - 1 && text[i] != 0; i++)
			seduce_text_state[user_id].copy[i] = text[i];
		seduce_text_state[user_id].copy[i] = 0;
	}else
	{
		seduce_text_state[user_id].copy = NULL;
		seduce_text_state[user_id].copy_allocated = 0;
	}
}


void seduce_text_deactivate(uint user_id)
{
	if(user_id >= betray_support_functionality(B_SF_USER_COUNT_MAX))
		return;
	seduce_text_state[user_id].id = NULL;
	seduce_text_state[user_id].pointer_active = -1;
	if(seduce_text_state[user_id].copy != NULL)
		free(seduce_text_state[user_id].copy);
	seduce_text_state[user_id].copy = NULL;
}

boolean seduce_text_edit_active(void *id)
{
	uint i, count;
	count = betray_support_functionality(B_SF_USER_COUNT_MAX);
	if(id == NULL)
	{		
		for(i = 0; i < count && seduce_text_state[i].id == id; i++);
		return i < count;
	}else
		for(i = 0; i < count; i++)
			if(seduce_text_state[i].id == id)
				return TRUE;
	return FALSE;
}



// uint seduce_text_block_draw3(float pos_x, float pos_y, float line_size, float height, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count)
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

extern void seduce_text_block_select_draw(BInputState *input, float pos_x, float pos_y, float line_size, float height, float line_spacing, STextBlockAlignmentStyle style, const char *text, uint pos, STextBlockMode *modes, uint mode_count, uint select_start, uint select_end);

STypeInState seduce_text_box_edit(BInputState *input, void *id, char *text, uint buffer_size, STextBox *boxes, uint box_count, STextBlockMode *modes, uint mode_count)
{
	float array[12], output[3];
	char *t;
	uint i, j, pos, next, user_count;
	if(input->mode == BAM_DRAW)
	{
		seduce_element_add_surface(input, id);
		for(i = 0; i < box_count; i++)
			seduce_element_add_rectangle(input, id, i, boxes[i].pos_x, boxes[i].pos_y - boxes[i].height, boxes[i].line_size, boxes[i].height);


//		seduce_element_add_line(input, id, 0, array, &array[3], size * 2.0);
		
		pos = 0;
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				if(seduce_text_state[i].done_func != NULL)
					t = seduce_text_state[i].copy;
				else
					t = text;
				for(j = 0; j < box_count; j++)
				{
					next = seduce_text_block_draw(boxes[j].pos_x, boxes[j]. pos_y, boxes[j].line_size, boxes[j].height, boxes[j].line_spacing, boxes[j].style, t, pos, modes, mode_count);
					if(seduce_text_state[i].select_start < seduce_text_state[i].select_end)
						seduce_text_block_select_draw(input, boxes[j].pos_x, boxes[j].pos_y, boxes[j].line_size, boxes[j].height, boxes[j].line_spacing, boxes[j].style, t, pos, modes, mode_count, seduce_text_state[i].select_start, seduce_text_state[i].select_end);
					else
						seduce_text_block_select_draw(input, boxes[j].pos_x, boxes[j].pos_y, boxes[j].line_size, boxes[j].height, boxes[j].line_spacing, boxes[j].style, t, pos, modes, mode_count, seduce_text_state[i].select_end, seduce_text_state[i].select_start);
					pos = next;
				}
				break;
			}
		}
		if(i == input->user_count)
		{
			for(j = 0; j < box_count; j++)
				pos = seduce_text_block_draw(boxes[j].pos_x, boxes[j]. pos_y, boxes[j].line_size, boxes[j].height, boxes[j].line_spacing, boxes[j].style, text, pos, modes, mode_count);
		}
	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
				if(/*seduce_text_state[input->pointers[i].user_id].id = id &&*/ id == seduce_element_pointer_id(input, i, NULL))
					seduce_text_activate(input->pointers[i].user_id, id,  text, buffer_size, NULL, NULL, FALSE, i);

		
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].pointer_active < input->pointer_count)
			{
				if(seduce_text_state[i].done_func != NULL)
					t = seduce_text_state[i].copy;
				else
					t = text;
				j = seduce_text_state[i].pointer_active;
				if(seduce_element_surface_project(input, id, output, 2, input->pointers[j].pointer_x, input->pointers[j].pointer_y))
					sui_text_box_edit_mouse(t, boxes, box_count, modes, mode_count, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->pointers[j].button[0], input->pointers[j].last_button[0], output[0], output[1]);
				if(!input->pointers[j].button[0] && !input->pointers[j].last_button[0])
					seduce_text_state[i].pointer_active = -1;
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				for(j = 0; j < input->button_event_count; j++)
				{
					if(input->button_event[j].state == TRUE && input->button_event[j].user_id == i)
					{
						if(seduce_text_state[i].done_func != NULL)
						{
							t = seduce_text_state[i].copy;
							buffer_size = seduce_text_state[i].copy_allocated;
						}
						else
							t = text;
						if(input->button_event[j].button == BETRAY_BUTTON_PASTE)
							seduce_text_edit_paste(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, modes, mode_count);
						else if(input->button_event[j].button == BETRAY_BUTTON_CUT)
							seduce_text_edit_cut(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, modes, mode_count);
						else if(input->button_event[j].button == BETRAY_BUTTON_COPY)
							seduce_text_edit_copy(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_RIGHT)
							seduce_text_edit_forward(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_LEFT)
							seduce_text_edit_back(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_DELETE)
							seduce_text_edit_delete(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, modes, mode_count);
						else if(input->button_event[j].button == BETRAY_BUTTON_END)
							seduce_text_edit_end(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_HOME)
							seduce_text_edit_home(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_BACKSPACE)
							seduce_text_edit_backspace(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, modes, mode_count);
						else if(input->button_event[j].character > 31 && input->button_event[j].character < 256)
							seduce_text_edit_insert_character(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->button_event[j].character, modes, mode_count);
					}
				}
			}
		}
	}
	for(i = 0; i < input->user_count; i++)
		if(seduce_text_state[i].id == id)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}

extern uint		seduce_text_box_draw_monospace(SeduceRenderFont *font, float pos_x, float pos_y, float character_size, float space_size, float line_size, const char *text, uint line_count, uint line_length, uint *scroll, STextBlockMode *modes, uint mode_count);
extern uint		seduce_text_box_draw_monospace_select(BInputState *input, float pos_x, float pos_y, float character_size, float space_size, float line_size, const char *text, uint line_count, uint line_length, uint *scroll, uint select_start, uint select_end);
extern void		seduce_text_monospace_edit_scroll(char *text, uint line_count, uint line_length, uint *scroll, uint curser);


STypeInState seduce_text_monospace_edit(BInputState *input, void *id, char *text, uint buffer_size, void *font, float pos_x, float pos_y, float character_size, float space_size, float line_size,  uint line_count, uint line_length, uint *scroll, STextBlockMode *modes, uint mode_count)
{
	float array[12], output[3];
	char *t;
	uint i, j, k, pos, next, user_count;
	if(input->mode == BAM_DRAW)
	{
		seduce_element_add_surface(input, id);
		seduce_element_add_rectangle(input, id, 0, pos_x, pos_y - line_size * line_count, space_size * line_length, line_size * line_count);
		seduce_text_box_draw_monospace(font, pos_x, pos_y, character_size, space_size, line_size, text, line_count, line_length, scroll, modes, mode_count);

		pos = 0;

		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				seduce_text_box_draw_monospace_select(input, pos_x, pos_y, character_size, space_size, line_size, text,line_count, line_length, scroll, seduce_text_state[i].select_start, seduce_text_state[i].select_end);
				break;
			}
		}


	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
				if(id == seduce_element_pointer_id(input, i, NULL))
					seduce_text_activate(input->pointers[i].user_id, id,  text, buffer_size, NULL, NULL, FALSE, i);

		
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].pointer_active < input->pointer_count)
			{

				j = seduce_text_state[i].pointer_active;
				if(seduce_element_surface_project(input, id, output, 2, input->pointers[j].pointer_x, input->pointers[j].pointer_y))
					sui_text_monospace_edit_mouse(text, pos_x, pos_y, character_size, space_size, line_size, line_count, line_length, scroll, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->pointers[j].button[0], input->pointers[j].last_button[0], output[0], output[1]);
				if(!input->pointers[j].button[0] && !input->pointers[j].last_button[0])
					seduce_text_state[i].pointer_active = -1;
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				for(j = 0; j < input->button_event_count; j++)
				{
					if(input->button_event[j].state == TRUE && input->button_event[j].user_id == i)
					{
						if(seduce_text_state[i].done_func != NULL)
							t = seduce_text_state[i].copy;
						else
							t = text;
						if(input->button_event[j].button == BETRAY_BUTTON_SCROLL_DOWN /*_LEFT*/)
						{
							if(scroll[0] != 0)
								scroll[0]--;
						}else if(input->button_event[j].button == BETRAY_BUTTON_SCROLL_UP /*_RIGHT*/)
						{
							scroll[0]++;
						}else				
					/*	if(input->button_event[j].button == BETRAY_BUTTON_SCROLL_DOWN)
						{
							if(scroll[1] != 0)
								scroll[1]--;
						}else if(input->button_event[j].button == BETRAY_BUTTON_SCROLL_UP)
						{
							uint count = 1;
							scroll[1]++;
							for(k = 0; text[k] != 0; k++)
							{
								if(text[k] == '\n')
									count++;
							}
							if(count <= line_count)
								scroll[1] = 0; 
							else if(scroll[1] + line_count > count)
								scroll[1] = count - line_count;
						}else*/
						{
							if(input->button_event[j].button == BETRAY_BUTTON_PASTE)
								seduce_text_edit_paste(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, modes, mode_count);
							else if(input->button_event[j].button == BETRAY_BUTTON_CUT)
								seduce_text_edit_cut(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, modes, mode_count);
							else if(input->button_event[j].button == BETRAY_BUTTON_COPY)
								seduce_text_edit_copy(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
							else if(input->button_event[j].button == BETRAY_BUTTON_UP)
								seduce_text_edit_up(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
							else if(input->button_event[j].button == BETRAY_BUTTON_DOWN)
								seduce_text_edit_down(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
							else if(input->button_event[j].button == BETRAY_BUTTON_RIGHT)
								seduce_text_edit_forward(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
							else if(input->button_event[j].button == BETRAY_BUTTON_LEFT)
								seduce_text_edit_back(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
							else if(input->button_event[j].button == BETRAY_BUTTON_DELETE)
								seduce_text_edit_delete(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, modes, mode_count);
							else if(input->button_event[j].button == BETRAY_BUTTON_END)
								seduce_text_edit_end(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
							else if(input->button_event[j].button == BETRAY_BUTTON_HOME)
								seduce_text_edit_home(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
							else if(input->button_event[j].button == BETRAY_BUTTON_BACKSPACE)
								seduce_text_edit_backspace(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, modes, mode_count);
							else if(input->button_event[j].button == BETRAY_BUTTON_TAB)
								seduce_text_edit_insert_character(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, '\t', modes, mode_count);
							else if(input->button_event[j].button == BETRAY_BUTTON_RETURN)
								seduce_text_edit_insert_character(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, '\n', modes, mode_count);
							else if(input->button_event[j].character > 31 && input->button_event[j].character < 256)
								seduce_text_edit_insert_character(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->button_event[j].character, modes, mode_count);
							if(seduce_text_state[i].select_start == seduce_text_state[i].select_end)
								seduce_text_monospace_edit_scroll(text, line_count, line_length, scroll, seduce_text_state[i].select_start);
						}
					}
				}
			}
		}
	}
	for(i = 0; i < input->user_count; i++)
		if(seduce_text_state[i].id == id)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}




STypeInState seduce_text_edit_line(BInputState *input, void *id, SeduceRenderFont *font, char *text, uint buffer_size, float pos_x, float pos_y, float length, float size, char *label, boolean left, void (*done_func)(void *user, char *text), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha)
{
	float array[6], output[3], right_pad = 0;
	char *t;
	uint i, j, user_count;
	if(!left)
	{
		if(text[0] != 0)
			 right_pad = length - seduce_text_line_length(font, size, SEDUCE_T_SPACE, text, -1);
		else
			right_pad = length - seduce_text_line_length(font, size, SEDUCE_T_SPACE, label, -1);
		pos_x += right_pad;
	}

	if(input->mode == BAM_DRAW)
	{
		seduce_element_add_rectangle(input, id, 0, pos_x - right_pad, pos_y - size, length, size * 3.0);
		seduce_element_add_surface(input, id);
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				if(seduce_text_state[i].done_func != NULL)
					t = seduce_text_state[i].copy;
				else
					t = text;
				seduce_text_line_draw(NULL, pos_x, pos_y, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], length) + seduce_text_state[i].scroll_start);
				sui_text_line_edit_draw(input, font, pos_x, pos_y, size, SEDUCE_T_SPACE, length, t, &seduce_text_state[i].scroll_start, seduce_text_state[i].select_start, seduce_text_state[i].select_end, active_red, active_green, active_blue, active_alpha);
				break;
			}
		}
		if(i == input->user_count)
		{
			if(label != NULL && (text == NULL || text[0] == 0))
				t = label;
			else
				t = text;

			for(i = 0; i < input->user_count && id != seduce_element_pointer_id(input, i, NULL); i++);

			if(i < input->user_count)
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, t, active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, t, length));
			else
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, t, red, green, blue, alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, t, length));
		}
	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
				if(/*seduce_text_state[input->pointers[i].user_id].id != id && */id == seduce_element_pointer_id(input, i, NULL))
					seduce_text_activate(input->pointers[i].user_id, id,  text, buffer_size, done_func, user, done_func != NULL, i);


		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].pointer_active < input->pointer_count && seduce_text_state[i].id == id)
			{
				if(seduce_text_state[i].done_func != NULL)
					t = seduce_text_state[i].copy;
				else
					t = text;
				j = seduce_text_state[i].pointer_active;
				if(seduce_element_surface_project(input, id, output, 2, input->pointers[j].pointer_x, input->pointers[j].pointer_y))
					sui_text_line_edit_mouse(font, size, SEDUCE_T_SPACE, length, t, &seduce_text_state[i].scroll_start, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->pointers[j].button[0], input->pointers[j].last_button[0], output[0] - pos_x);
				if(!input->pointers[j].button[0] && !input->pointers[j].last_button[0])
					seduce_text_state[i].pointer_active = -1;
			}
		}

		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				for(j = 0; j < input->button_event_count; j++)
				{
					if(input->button_event[j].state == TRUE && input->button_event[j].user_id == i)
					{
						if(seduce_text_state[i].done_func != NULL)
							t = seduce_text_state[i].copy;
						else
							t = text;
						if(input->button_event[j].button == BETRAY_BUTTON_PASTE)
							seduce_text_edit_paste(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_CUT)
							seduce_text_edit_cut(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_COPY)
							seduce_text_edit_copy(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_RIGHT)
							seduce_text_edit_forward(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_LEFT)
							seduce_text_edit_back(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_DELETE)
							seduce_text_edit_delete(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_END)
							seduce_text_edit_end(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_HOME)
							seduce_text_edit_home(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_BACKSPACE)
							seduce_text_edit_backspace(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_RETURN) 
						{
							if(seduce_text_state[i].done_func != NULL)
							{
								void (* done_func)(void *user, char *text);
								done_func = seduce_text_state[i].done_func;
								done_func(user, t);
								free(seduce_text_state[i].copy);
							}
							seduce_text_state[i].id = NULL;
							return S_TIS_DONE;
						}
						else if(input->button_event[j].character > 31 && input->button_event[j].character < 256)
							seduce_text_edit_insert_character(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->button_event[j].character, NULL, 0);
					}
				}
			}
		}
	}
	for(i = 0; i < input->user_count; i++)
		if(seduce_text_state[i].id == id)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}

STypeInState seduce_text_edit_obfuscated(BInputState *input, void *id, char *text, uint buffer_size, float pos_x, float pos_y, float length, float size, char *label, boolean left, void (*done_func)(void *user, char *text), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha)
{
	float /*array[6], */output[3];
	char *t, *password;
	uint i, j, user_count;
	if(input->mode == BAM_DRAW)
	{
	/*	array[0] = pos_x;
		array[1] = pos_y + size * 0.5;
		array[2] = 0;
		array[3] = pos_x + length;
		array[4] = pos_y + size * 0.5;
		array[5] = 0;*/
	//	seduce_element_add_line(input, id, 0, array, &array[3], size * 2.0);
		seduce_element_add_rectangle(input, id, 0, pos_x, pos_y - size, length, size * 3.0);
		seduce_element_add_surface(input, id);
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				if(seduce_text_state[i].done_func != NULL)
					t = seduce_text_state[i].copy;
				else
					t = text;
				for(j = 0; t[j] != 0; j++);
				password = malloc(sizeof *password * ++j);
				for(j = 0; t[j] != 0; j++)
					password[j] = '*';
				password[j] = 0;
				seduce_text_line_draw(NULL, pos_x, pos_y, size, SEDUCE_T_SPACE, &password[seduce_text_state[i].scroll_start], active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(NULL, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], length) + seduce_text_state[i].scroll_start);
				sui_text_line_edit_draw(input, NULL, pos_x, pos_y, size, SEDUCE_T_SPACE, length, password, &seduce_text_state[i].scroll_start, seduce_text_state[i].select_start, seduce_text_state[i].select_end, active_red, active_green, active_blue, active_alpha);
				free(password);
				break;
			}
		}
		if(i == input->user_count)
		{
			password = NULL;
			if(label != NULL && text[0] == 0)
				t = label;
			else
			{
				for(j = 0; text[j] != 0; j++);
				password = malloc(sizeof *password * ++j);
				for(j = 0; text[j] != 0; j++)
					password[j] = '*';
				password[j] = 0;
				t = password;
			}
			for(i = 0; i < input->user_count && id != seduce_element_pointer_id(input, i, NULL); i++);
			if(i < input->user_count)
				seduce_text_line_draw(NULL, pos_x, pos_y, size, SEDUCE_T_SPACE, t, active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(NULL, size, SEDUCE_T_SPACE, t, length));
			else
				seduce_text_line_draw(NULL, pos_x, pos_y, size, SEDUCE_T_SPACE, t, red, green, blue, alpha, seduce_text_line_hit_test(NULL, size, SEDUCE_T_SPACE, t, length));
			if(password != NULL)
				free(password);
		}

	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
				if(/*seduce_text_state[input->pointers[i].user_id].id != id && */id == seduce_element_pointer_id(input, i, NULL))
					seduce_text_activate(input->pointers[i].user_id, id,  text, buffer_size, done_func, user, done_func != NULL, i);
		


		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].pointer_active < input->pointer_count && seduce_text_state[i].id == id)
			{
				if(seduce_text_state[i].done_func != NULL)
					t = seduce_text_state[i].copy;
				else
					t = text;
				for(j = 0; t[j] != 0; j++);
				password = malloc(sizeof *password * ++j);
				for(j = 0; t[j] != 0; j++)
					password[j] = '*';
				password[j] = 0;
				j = seduce_text_state[i].pointer_active;
				if(seduce_element_surface_project(input, id, output, 2, input->pointers[j].pointer_x, input->pointers[j].pointer_y))
					sui_text_line_edit_mouse(NULL, size, SEDUCE_T_SPACE, length, password, &seduce_text_state[i].scroll_start, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->pointers[j].button[0], input->pointers[j].last_button[0], output[0] - pos_x);
				free(password);
				if(!input->pointers[j].button[0] && !input->pointers[j].last_button[0])
					seduce_text_state[i].pointer_active = -1;
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				for(j = 0; j < input->button_event_count; j++)
				{
					if(input->button_event[j].state == TRUE && input->button_event[j].user_id == i)
					{
						if(seduce_text_state[i].done_func != NULL)
							t = seduce_text_state[i].copy;
						else
							t = text;
						if(input->button_event[j].button == BETRAY_BUTTON_PASTE)
							seduce_text_edit_paste(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_CUT)
							seduce_text_edit_cut(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_COPY)
							seduce_text_edit_copy(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_RIGHT)
							seduce_text_edit_forward(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_LEFT)
							seduce_text_edit_back(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_DELETE)
							seduce_text_edit_delete(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_END)
							seduce_text_edit_end(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_HOME)
							seduce_text_edit_home(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_BACKSPACE)
							seduce_text_edit_backspace(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_RETURN) 
						{
							if(seduce_text_state[i].done_func != NULL)
							{
								void (* done_func)(void *user, char *text);
								done_func = seduce_text_state[i].done_func;
								done_func(user, t);
								free(seduce_text_state[i].copy);
							}
							seduce_text_state[i].id = NULL;
							return S_TIS_DONE;
						}
						else if(input->button_event[j].character > 31 && input->button_event[j].character < 256)
							seduce_text_edit_insert_character(t, buffer_size, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->button_event[j].character, NULL, 0);
					}
				}
			}
		}
	}
	for(i = 0; i < input->user_count; i++)
		if(seduce_text_state[i].id == id)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}

STypeInState seduce_text_edit_double(BInputState *input, void *id, SeduceRenderFont *font, double *value, float pos_x, float pos_y, float length, float size, boolean left, void (*done_func)(void *user, double value), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha)
{
	float /*array[6],*/ output[3];
	char *t, buffer[256];
	uint i, j, k, l, user_count;
	if(input->mode == BAM_DRAW)
	{
	/*	array[0] = pos_x;
		array[1] = pos_y + size * 0.5;
		array[2] = 0;
		array[3] = pos_x + length;
		array[4] = pos_y + size * 0.5;
		array[5] = 0;
		seduce_element_add_line(input, id, 0, array, &array[3], size * 2.0);*/
		seduce_element_add_rectangle(input, id, 0, pos_x, pos_y - size, length, size * 3.0);
		seduce_element_add_surface(input, id);
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				t = seduce_text_state[i].copy;
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], length) + seduce_text_state[i].scroll_start);
				sui_text_line_edit_draw(input, font, pos_x, pos_y, size, SEDUCE_T_SPACE, length, t, &seduce_text_state[i].scroll_start, seduce_text_state[i].select_start, seduce_text_state[i].select_end, active_red, active_green, active_blue, active_alpha);
				break;
			}
		}
		if(i == input->user_count)
		{
			sprintf(buffer, "%f", *value);
			for(j = 0; 0 != buffer[j] && 46 != buffer[j]; j++);
			if(0 != buffer[j])
			{
				int pos;
				pos = -1;
				for(j++; 0 != buffer[j]; j++)
				{
					if(48 == buffer[j])
						pos++;
					else
						pos = 0;
				}
				if(pos > 0)
					buffer[j - pos] = 0;
			}
			for(i = 0; i < input->user_count && id != seduce_element_pointer_id(input, i, NULL); i++);
			if(i < input->user_count)
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, buffer, active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, buffer, length));
			else
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, buffer, red, green, blue, alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, buffer, length));
		}

	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
			{
				if(/*seduce_text_state[input->pointers[i].user_id].id != id && */id == seduce_element_pointer_id(input, i, NULL))
				{
					sprintf(buffer, "%f", *value);
					for(j = 0; 0 != buffer[j] && 46 != buffer[j]; j++);
					if(0 != buffer[j])
					{
						int pos;
						pos = -1;
						for(j++; 0 != buffer[j]; j++)
						{
							if(48 == buffer[j])
								pos++;
							else
								pos = 0;
						}
						if(pos > 0)
							buffer[j - pos] = 0;
					}
					seduce_text_activate(input->pointers[i].user_id, id, buffer, 256, done_func, user, TRUE, i);
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
			if(seduce_text_state[i].id == id)
				for(j = 0; j < input->pointer_count; j++)
					if(input->pointers[j].user_id == i)
						if(seduce_element_surface_project(input, id, output, 2, input->pointers[j].pointer_x, input->pointers[j].pointer_y))
							sui_text_line_edit_mouse(font, size, SEDUCE_T_SPACE, length, seduce_text_state[i].copy, &seduce_text_state[i].scroll_start, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->pointers[j].button[0], input->pointers[j].last_button[0], output[0] - pos_x);

		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				for(j = 0; j < input->button_event_count; j++)
				{
					if(input->button_event[j].state == TRUE && input->button_event[j].user_id == i)
					{
						t = seduce_text_state[i].copy;
						if(input->button_event[j].button == BETRAY_BUTTON_PASTE)
						{
							boolean negative = FALSE;
							boolean period = FALSE;

							seduce_text_edit_paste(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
							for(k = 0; t[k] != 0; k++)
							{
								if(t[k] == '-')
									negative = TRUE;
								if((t[k] <= 47 || t[k] >= 58) && (t[k] != '.' || period))
								{
									l = k;
									seduce_text_edit_delete(t, 256, &k, &l, NULL, 0);
									k--;
									seduce_text_state[i].select_end--;
								}
								if(t[k] != '.')
									period = TRUE;							
							}
							if(negative && t[0] != '-')
							{
								uint s = 0, e = 0;
								seduce_text_edit_insert_character(t, 256, &s, &e, '-', NULL, 0);
								seduce_text_state[i].select_start++;
								seduce_text_state[i].select_end++;
							}
						}else if(input->button_event[j].button == BETRAY_BUTTON_CUT)
							seduce_text_edit_cut(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_COPY)
							seduce_text_edit_copy(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_RIGHT)
							seduce_text_edit_forward(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_LEFT)
							seduce_text_edit_back(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_DELETE)
							seduce_text_edit_delete(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_END)
							seduce_text_edit_end(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_HOME)
							seduce_text_edit_home(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_BACKSPACE)
							seduce_text_edit_backspace(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_RETURN) 
						{
							double decimal = 0.1, v;
							v = 0;
							if(t[0] == '-')
								k = 1;
							else
								k = 0;
							for(; t[k] != 0 && t[k] != '.'; k++)
								if(t[k] > 47 && t[k] < 58)
									v = (t[k] - 48) + v * 10.0;
							if(t[k] == '.')
							{
								for(; t[k] != 0; k++)
								{
									if(t[k] > 47 && t[k] < 58)
									{
										v += decimal * (t[k] - 48);
										decimal /= 10.0;
									}
								}
							}
							if(t[0] == '-')
								v = -v;
							if(seduce_text_state[i].done_func != NULL)
							{
								void (* done_func)(void *user, double value);
								done_func = seduce_text_state[i].done_func;
								done_func(user, v);
								free(seduce_text_state[i].copy);
							}else
								*value = v;
							seduce_text_state[i].id = NULL;
							return S_TIS_DONE;
						}
						else if((input->button_event[j].character > 47 && 
								input->button_event[j].character < 58))
							seduce_text_edit_insert_character(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->button_event[j].character, NULL, 0);
						else if(input->button_event[j].character == '-' && t[0] != '-')
						{
							uint s = 0, e = 0;
							seduce_text_edit_insert_character(t, 256, &s, &e, '-', NULL, 0);
							seduce_text_state[i].select_start++;
							seduce_text_state[i].select_end++;
						}else if(input->button_event[j].character == '+' && t[0] == '-')
						{
							uint s = 0, e = 0;
							seduce_text_edit_delete(t, 256, &s, &e, NULL, 0);
							if(seduce_text_state[i].select_start > 0)
								seduce_text_state[i].select_start--;
							if(seduce_text_state[i].select_end > 0)
								seduce_text_state[i].select_end--;
						}
						else if(input->button_event[j].character == '.' ||
							input->button_event[j].character == ',')
						{
							uint l;
							for(k = 0; t[k] != 0 && t[k] != '.'; k++);
							if(t[k] == '.')
							{
								l = k;
								seduce_text_edit_delete(t, 256, &l, &k, NULL, 0);
								if(seduce_text_state[i].select_start >= l)
									seduce_text_state[i].select_start--;
								if(seduce_text_state[i].select_end >= l)
									seduce_text_state[i].select_end--;
							}
							seduce_text_edit_insert_character(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, '.', NULL, 0);
						}
					}
				}
			}
		}
	}
	for(i = 0; i < input->user_count; i++)
		if(seduce_text_state[i].id == id)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}


STypeInState seduce_text_edit_float(BInputState *input, void *id, SeduceRenderFont *font, float *value, float pos_x, float pos_y, float length, float size, boolean left, void (*done_func)(void *user, float value), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha)
{
	float/* array[6], */output[3];
	char *t, buffer[256];
	uint i, j, k, l, user_count;
	if(input->mode == BAM_DRAW)
	{
	/*	array[0] = pos_x;
		array[1] = pos_y + size * 0.5;
		array[2] = 0;
		array[3] = pos_x + length;
		array[4] = pos_y + size * 0.5;
		array[5] = 0;
		seduce_element_add_line(input, id, 0, array, &array[3], size * 2.0);*/
		seduce_element_add_rectangle(input, id, 0, pos_x, pos_y - size, length, size * 3.0);
		seduce_element_add_surface(input, id);
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				t = seduce_text_state[i].copy;
				for(j = 0; t[j] != 0 && j < seduce_text_state[i].scroll_start; j++);
				seduce_text_state[i].scroll_start = j;
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], length) + seduce_text_state[i].scroll_start);
				sui_text_line_edit_draw(input, font, pos_x, pos_y, size, SEDUCE_T_SPACE, length, t, &seduce_text_state[i].scroll_start, seduce_text_state[i].select_start, seduce_text_state[i].select_end, active_red, active_green, active_blue, active_alpha);
				break;
			}
		}
		if(i == input->user_count)
		{
			sprintf(buffer, "%f", *value);
			for(j = 0; 0 != buffer[j] && 46 != buffer[j]; j++);
			if(0 != buffer[j])
			{
				int pos;
				pos = -1;
				for(j++; 0 != buffer[j]; j++)
				{
					if(48 == buffer[j])
						pos++;
					else
						pos = 0;
				}
				if(pos > 0)
					buffer[j - pos] = 0;
			}
			for(i = 0; i < input->user_count && id != seduce_element_pointer_id(input, i, NULL); i++);
			if(i < input->user_count)
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, buffer, active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, buffer, length));
			else
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, buffer, red, green, blue, alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, buffer, length));
		}

	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
			{
				if(/*seduce_text_state[input->pointers[i].user_id].id != id && */id == seduce_element_pointer_id(input, i, NULL))
				{
					sprintf(buffer, "%f", *value);
					for(j = 0; 0 != buffer[j] && 46 != buffer[j]; j++);
					if(0 != buffer[j])
					{
						int pos;
						pos = -1;
						for(j++; 0 != buffer[j]; j++)
						{
							if(48 == buffer[j])
								pos++;
							else
								pos = 0;
						}
						if(pos > 0)
							buffer[j - pos] = 0;
					}
					seduce_text_activate(input->pointers[i].user_id, id, buffer, 256, done_func, user, TRUE, i);
				}
			}
		}

		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].pointer_active < input->pointer_count && seduce_text_state[i].id == id)
			{
				j = seduce_text_state[i].pointer_active;
				if(seduce_element_surface_project(input, id, output, 2, input->pointers[j].pointer_x, input->pointers[j].pointer_y))
					sui_text_line_edit_mouse(font, size, SEDUCE_T_SPACE, length, seduce_text_state[i].copy, &seduce_text_state[i].scroll_start, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->pointers[j].button[0], input->pointers[j].last_button[0], output[0] - pos_x);
				if(!input->pointers[j].button[0] && !input->pointers[j].last_button[0])
					seduce_text_state[i].pointer_active = -1;
			}
		}

		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				for(j = 0; j < input->button_event_count; j++)
				{
					if(input->button_event[j].state == TRUE && input->button_event[j].user_id == i)
					{
						t = seduce_text_state[i].copy;
						if(input->button_event[j].button == BETRAY_BUTTON_PASTE)
						{
							boolean negative = FALSE;
							boolean period = FALSE;

							seduce_text_edit_paste(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
							for(k = 0; t[k] != 0; k++)
							{
								if(t[k] == '-')
									negative = TRUE;
								if((t[k] <= 47 || t[k] >= 58) && (t[k] != '.' || period))
								{
									l = k;
									seduce_text_edit_delete(t, 256, &k, &l, NULL, 0);
									k--;
									seduce_text_state[i].select_end--;
								}
								if(t[k] != '.')
									period = TRUE;							
							}
							if(negative && t[0] != '-')
							{
								uint s = 0, e = 0;
								seduce_text_edit_insert_character(t, 256, &s, &e, '-', NULL, 0);
								seduce_text_state[i].select_start++;
								seduce_text_state[i].select_end++;
							}
						}else if(input->button_event[j].button == BETRAY_BUTTON_CUT)
							seduce_text_edit_cut(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_COPY)
							seduce_text_edit_copy(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_RIGHT)
							seduce_text_edit_forward(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_LEFT)
							seduce_text_edit_back(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_DELETE)
							seduce_text_edit_delete(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_END)
							seduce_text_edit_end(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_HOME)
							seduce_text_edit_home(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_BACKSPACE)
							seduce_text_edit_backspace(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_RETURN) 
						{
							float decimal = 0.1, v;
							v = 0;
							if(t[0] == '-')
								k = 1;
							else
								k = 0;
							for(; t[k] != 0 && t[k] != '.'; k++)
								if(t[k] > 47 && t[k] < 58)
									v = (t[k] - 48) + v * 10.0;
							if(t[k] == '.')
							{
								for(; t[k] != 0; k++)
								{
									if(t[k] > 47 && t[k] < 58)
									{
										v += decimal * (t[k] - 48);
										decimal /= 10.0;
									}
								}
							}
							if(t[0] == '-')
								v = -v;
							if(seduce_text_state[i].done_func != NULL)
							{
								void (* done_func)(void *user, double value);
								done_func = seduce_text_state[i].done_func;
								done_func(user, v);
								free(seduce_text_state[i].copy);
							}else
								*value = v;
							seduce_text_state[i].id = NULL;
							return S_TIS_DONE;
						}
						else if((input->button_event[j].character > 47 && 
								input->button_event[j].character < 58))
							seduce_text_edit_insert_character(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->button_event[j].character, NULL, 0);
						else if(input->button_event[j].character == '-' && t[0] != '-')
						{
							uint s = 0, e = 0;
							seduce_text_edit_insert_character(t, 256, &s, &e, '-', NULL, 0);
							seduce_text_state[i].select_start++;
							seduce_text_state[i].select_end++;
						}else if(input->button_event[j].character == '+' && t[0] == '-')
						{
							uint s = 0, e = 0;
							seduce_text_edit_delete(t, 256, &s, &e, NULL, 0);
							if(seduce_text_state[i].select_start > 0)
								seduce_text_state[i].select_start--;
							if(seduce_text_state[i].select_end > 0)
								seduce_text_state[i].select_end--;
						}
						else if(input->button_event[j].character == '.' ||
							input->button_event[j].character == ',')
						{
							uint l;
							for(k = 0; t[k] != 0 && t[k] != '.'; k++);
							if(t[k] == '.')
							{
								l = k;
								seduce_text_edit_delete(t, 256, &l, &k, NULL, 0);
								if(seduce_text_state[i].select_start >= l)
									seduce_text_state[i].select_start--;
								if(seduce_text_state[i].select_end >= l)
									seduce_text_state[i].select_end--;
							}
							seduce_text_edit_insert_character(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, '.', NULL, 0);
						}
					}
				}
			}
		}
	}
	for(i = 0; i < input->user_count; i++)
		if(seduce_text_state[i].id == id)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}

STypeInState seduce_text_edit_int(BInputState *input, void *id, SeduceRenderFont *font, int *value, float pos_x, float pos_y, float length, float size, boolean left, void (*done_func)(void *user, int value), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha)
{
	float /*array[6], */output[3];
	char *t, buffer[256];
	uint i, j, k, l, user_count;
	if(input->mode == BAM_DRAW)
	{
	/*	array[0] = pos_x;
		array[1] = pos_y + size * 0.5;
		array[2] = 0;
		array[3] = pos_x + length;
		array[4] = pos_y + size * 0.5;
		array[5] = 0;
		seduce_element_add_line(input, id, 0, array, &array[3], size * 2.0);*/
		seduce_element_add_rectangle(input, id, 0, pos_x, pos_y - size, length, size * 3.0);
		seduce_element_add_surface(input, id);

		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				t = seduce_text_state[i].copy;
				if(!left)
					pos_x += length - seduce_text_line_length(NULL, size, SEDUCE_T_SPACE, t, -1);
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], length) + seduce_text_state[i].scroll_start);
				sui_text_line_edit_draw(input, font, pos_x, pos_y, size, SEDUCE_T_SPACE, length, t, &seduce_text_state[i].scroll_start, seduce_text_state[i].select_start, seduce_text_state[i].select_end, active_red, active_green, active_blue, active_alpha);
				break;
			}
		}
		if(i == input->user_count)
		{
			sprintf(buffer, "%i", *value);
			if(!left)
				pos_x += length - seduce_text_line_length(font, size, SEDUCE_T_SPACE, buffer, -1);
			for(i = 0; i < input->user_count && id != seduce_element_pointer_id(input, i, NULL); i++);
			if(i < input->user_count)
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, buffer, active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, buffer, length));
			else
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, buffer, red, green, blue, alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, buffer, length));
		}
	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
			{
				if(/*seduce_text_state[input->pointers[i].user_id].id != id &&*/ id == seduce_element_pointer_id(input, i, NULL))
				{
					sprintf(buffer, "%i", *value);
					seduce_text_activate(input->pointers[i].user_id, id, buffer, 256, done_func, user, TRUE, i);
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].pointer_active < input->pointer_count && seduce_text_state[i].id == id)
			{
				j = seduce_text_state[i].pointer_active;
				if(seduce_element_surface_project(input, id, output, 2, input->pointers[j].pointer_x, input->pointers[j].pointer_y))
					sui_text_line_edit_mouse(font, size, SEDUCE_T_SPACE, length, seduce_text_state[i].copy, &seduce_text_state[i].scroll_start, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->pointers[j].button[0], input->pointers[j].last_button[0], output[0] - pos_x);
				if(!input->pointers[j].button[0] && !input->pointers[j].last_button[0])
					seduce_text_state[i].pointer_active = -1;
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				for(j = 0; j < input->button_event_count; j++)
				{
					if(input->button_event[j].state == TRUE && input->button_event[j].user_id == i)
					{
						t = seduce_text_state[i].copy;
						if(input->button_event[j].button == BETRAY_BUTTON_PASTE)
						{
							boolean negative = FALSE;
							seduce_text_edit_paste(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
							for(k = 0; t[k] != 0; k++)
							{
								if(t[k] == '-')
									negative = TRUE;
								if(t[k] <= 47 || t[k] >= 58)
								{
									l = k;
									seduce_text_edit_delete(t, 256, &k, &l, NULL, 0);
									k--;
									seduce_text_state[i].select_end--;
								}						
							}
							if(negative && t[0] != '-')
							{
								uint s = 0, e = 0;
								seduce_text_edit_insert_character(t, 256, &s, &e, '-', NULL, 0);
								seduce_text_state[i].select_start++;
								seduce_text_state[i].select_end++;
							}
						}else if(input->button_event[j].button == BETRAY_BUTTON_CUT)
							seduce_text_edit_cut(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_COPY)
							seduce_text_edit_copy(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_RIGHT)
							seduce_text_edit_forward(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_LEFT)
							seduce_text_edit_back(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_DELETE)
							seduce_text_edit_delete(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_END)
							seduce_text_edit_end(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_HOME)
							seduce_text_edit_home(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_BACKSPACE)
							seduce_text_edit_backspace(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_RETURN) 
						{
							int decimal = 0.1, v;
							v = 0;
							if(t[0] == '-')
								k = 1;
							else
								k = 0;
							for(; t[k] != 0; k++)
								if(t[k] > 47 && t[k] < 58)
									v = (t[k] - 48) + v * 10;
							if(t[0] == '-')
								v = -v;
							if(seduce_text_state[i].done_func != NULL)
							{
								void (* done_func)(void *user, int value);
								done_func = seduce_text_state[i].done_func;
								done_func(user, v);
								free(seduce_text_state[i].copy);
							}else
								*value = v;
							seduce_text_state[i].id = NULL;
							return S_TIS_DONE;
						}
						else if((input->button_event[j].character > 47 && input->button_event[j].character < 58))
							seduce_text_edit_insert_character(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->button_event[j].character, NULL, 0);
						else if(input->button_event[j].character == '-' && t[0] != '-')
						{
							uint s = 0, e = 0;
							seduce_text_edit_insert_character(t, 256, &s, &e, '-', NULL, 0);
							seduce_text_state[i].select_start++;
							seduce_text_state[i].select_end++;
						}else if(input->button_event[j].character == '+' && t[0] == '-')
						{
							uint s = 0, e = 0;
							seduce_text_edit_delete(t, 256, &s, &e, NULL, 0);
							if(seduce_text_state[i].select_start > 0)
								seduce_text_state[i].select_start--;
							if(seduce_text_state[i].select_end > 0)
								seduce_text_state[i].select_end--;
						}
					}
				}
			}
		}
	}
	for(i = 0; i < input->user_count; i++)
		if(seduce_text_state[i].id == id)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}


STypeInState seduce_text_edit_uint(BInputState *input, void *id, SeduceRenderFont *font, uint *value, float pos_x, float pos_y, float length, float size, boolean left, void (*done_func)(void *user, uint value), void *user, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha)
{
	float/* array[6],*/ output[3];
	char *t, buffer[256];
	uint i, j, k, l, user_count;
	if(input->mode == BAM_DRAW)
	{
	/*	array[0] = pos_x;
		array[1] = pos_y + size * 0.5;
		array[2] = 0;
		array[3] = pos_x + length;
		array[4] = pos_y + size * 0.5;
		array[5] = 0;
		seduce_element_add_line(input, id, 0, array, &array[3], size * 2.0);*/
		seduce_element_add_rectangle(input, id, 0, pos_x, pos_y - size, length, size * 3.0);
		seduce_element_add_surface(input, id);
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				t = seduce_text_state[i].copy;

				if(!left)
					pos_x += length - seduce_text_line_length(NULL, size, SEDUCE_T_SPACE, t, -1);
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, &t[seduce_text_state[i].scroll_start], length) + seduce_text_state[i].scroll_start);
				sui_text_line_edit_draw(input, font, pos_x, pos_y, size, SEDUCE_T_SPACE, length, t, &seduce_text_state[i].scroll_start, seduce_text_state[i].select_start, seduce_text_state[i].select_end, active_red, active_green, active_blue, active_alpha);
				break;
			}
		}
		if(i == input->user_count)
		{
			sprintf(buffer, "%u", *value);
			if(!left)
				pos_x += length - seduce_text_line_length(font, size, SEDUCE_T_SPACE, buffer, -1);
			for(i = 0; i < input->user_count && id != seduce_element_pointer_id(input, i, NULL); i++);
			if(i < input->user_count)
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, buffer, active_red, active_green, active_blue, active_alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, buffer, length));
			else
				seduce_text_line_draw(font, pos_x, pos_y, size, SEDUCE_T_SPACE, buffer, red, green, blue, alpha, seduce_text_line_hit_test(font, size, SEDUCE_T_SPACE, buffer, length));
		}

	}else if(input->mode == BAM_EVENT)
	{
		for(i = 0; i < input->pointer_count; i++)
		{
			if(input->pointers[i].button[0] == TRUE && input->pointers[i].last_button[0] == FALSE)
			{
				if(/*seduce_text_state[input->pointers[i].user_id].id != id &&*/ id == seduce_element_pointer_id(input, i, NULL))
				{
					sprintf(buffer, "%u", *value);
					seduce_text_activate(input->pointers[i].user_id, id, buffer, 256, done_func, user, TRUE, i);
				}
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].pointer_active < input->pointer_count && seduce_text_state[i].id == id)
			{
				j = seduce_text_state[i].pointer_active;
				if(seduce_element_surface_project(input, id, output, 2, input->pointers[j].pointer_x, input->pointers[j].pointer_y))
					sui_text_line_edit_mouse(font, size, SEDUCE_T_SPACE, length, seduce_text_state[i].copy, &seduce_text_state[i].scroll_start, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->pointers[j].button[0], input->pointers[j].last_button[0], output[0] - pos_x);
				if(!input->pointers[j].button[0] && !input->pointers[j].last_button[0])
					seduce_text_state[i].pointer_active = -1;
			}
		}
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				for(j = 0; j < input->button_event_count; j++)
				{
					if(input->button_event[j].state == TRUE && input->button_event[j].user_id == i)
					{
						t = seduce_text_state[i].copy;
						if(input->button_event[j].button == BETRAY_BUTTON_PASTE)
						{
							seduce_text_edit_paste(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
							for(k = 0; t[k] != 0; k++)
							{
								if(t[k] <= 47 || t[k] >= 58)
								{
									l = k;
									seduce_text_edit_delete(t, 256, &k, &l, NULL, 0);
									k--;
									seduce_text_state[i].select_end--;
								}						
							}
						}else if(input->button_event[j].button == BETRAY_BUTTON_CUT)
							seduce_text_edit_cut(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_COPY)
							seduce_text_edit_copy(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_RIGHT)
							seduce_text_edit_forward(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_LEFT)
							seduce_text_edit_back(t, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_DELETE)
							seduce_text_edit_delete(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_END)
							seduce_text_edit_end(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_HOME)
							seduce_text_edit_home(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end);
						else if(input->button_event[j].button == BETRAY_BUTTON_BACKSPACE)
							seduce_text_edit_backspace(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, NULL, 0);
						else if(input->button_event[j].button == BETRAY_BUTTON_RETURN) 
						{
							uint v;
							v = 0;
							k = 0;
							for(; t[k] != 0; k++)
								if(t[k] > 47 && t[k] < 58)
									v = (t[k] - 48) + v * 10;
							if(seduce_text_state[i].done_func != NULL)
							{
								void (* done_func)(void *user, uint value);
								done_func = seduce_text_state[i].done_func;
								done_func(user, v);
								free(seduce_text_state[i].copy);
							}else
								*value = v;
							seduce_text_state[i].id = NULL;
							return S_TIS_DONE;
						}
						else if((input->button_event[j].character > 47 && input->button_event[j].character < 58))
							seduce_text_edit_insert_character(t, 256, &seduce_text_state[i].select_start, &seduce_text_state[i].select_end, input->button_event[j].character, NULL, 0);
					}
				}
			}
		}
	}
	for(i = 0; i < input->user_count; i++)
		if(seduce_text_state[i].id == id)
			return S_TIS_ACTIVE;
	return S_TIS_IDLE;
}


void sui_text_double_edit_print(double value, char *buffer)
{
	uint i;
	sprintf(buffer, "%f", value);
	for(i = 0; buffer[i] != 0; i++);
	for(i--; i > 0 && buffer[i] == '0'; i--);
	if(buffer[i] == '.')
	{
		buffer[i + 1] = '0';
		buffer[i + 2] = 0;
	}else
		buffer[i + 1] = 0;
}




STextBlockMode *seduce_text_block_set_mode(BInputState *input, void *id, uint user_id, STextBlockMode *blocks, uint *block_count, uint *block_allocated, void *font, float red, float green, float blue, float alpha, float letter_size, float letter_spacing)
{
	uint select_start = -1, select_end;
	STextBlockMode pre;
	uint i, start_block, end_block; 
	int dif;

	if(id != NULL)
	{
		for(i = 0; i < input->user_count; i++)
		{
			if(seduce_text_state[i].id == id)
			{
				select_start = seduce_text_state[i].select_start;
				select_end = seduce_text_state[i].select_end;
				break;
			}
		}
		if(select_start == -1)
		{
			if(input->user_count > user_id && seduce_text_state[i].id != NULL)
			{
				select_start = seduce_text_state[user_id].select_start;
				select_end = seduce_text_state[user_id].select_end;
			}else
				return blocks;
		}
	}

	if(select_start > select_end)
	{
		i = select_start;	
		select_start = select_end;
		select_end = i;
	}
	
	for(start_block = 0; start_block < *block_count && blocks[start_block].character_position < select_start; start_block++);
	for(end_block = 0; end_block < *block_count && blocks[end_block].character_position < select_end; end_block++);

	if(end_block > 0)
	{
		if(blocks[end_block].character_position == select_end)
			pre = blocks[end_block];
		else
			pre = blocks[end_block - 1];
	}else
		pre = blocks[0];

	dif = start_block - end_block;
	if(blocks[start_block].character_position != select_start)
	{
		dif++;
	}
	if(end_block >= *block_count || blocks[end_block].character_position >= select_end)
		dif++;
	if(dif < 0)
	{
		*block_count += dif;
		for(i = start_block; i < *block_count; i++)
			blocks[i] = blocks[(int)i - dif];
	}

	if(dif > 0)
	{
		*block_count += dif;
		if(*block_count > *block_allocated)
		{
			*block_allocated += 16;
			blocks = realloc(blocks, (sizeof *blocks) * *block_allocated);
		}
		for(i = *block_count - 1; start_block != i; i--)
			blocks[i] = blocks[(int)i - dif];
		
	}
	blocks[start_block].character_position = select_start;
	blocks[start_block].font = font;
	blocks[start_block].red = red; 
	blocks[start_block].green = green;
	blocks[start_block].blue = blue;
	blocks[start_block].alpha = alpha; 
	blocks[start_block].letter_size = letter_size;
	blocks[start_block].letter_spacing = letter_spacing;
	blocks[start_block + 1] = pre;
	blocks[start_block + 1].character_position = select_end;
	return blocks; 
}



