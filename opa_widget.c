#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "forge.h"
#include "imagine.h"
#include "testify.h"
#include "opa_internal.h"
#include "seduce.h"


extern void opa_request_memory(OPAProject *project, uint64 pointer, uint type, uint parent_id, uint offset, uint indirection, float x, float y);
extern void opa_request_memory_set(OPAProject *project, THandle *handle, uint memory_id, uint offset, uint length, uint8 *data);
extern void opa_request_memory_allocate(OPAProject *project, THandle *handle, uint memory_id, uint offset, uint length);

#define opa_number_draw_macro(data_type, value_type, function_name, spacing) \
				{ \
					data_type *pointer; \
					value_type value; \
					pointer = &memmory[offset]; \
					for(i = 0; i < draw_length; i++) \
					{ \
						value = (value_type)pointer[i + options->scroll]; \
						state = function_name(input, &pointer[i + options->scroll], NULL, &value, x + SEDUCE_T_SIZE + (float)(i % options->columns) * spacing, y - (float)(i / options->columns) * SEDUCE_T_SIZE * 4.0,  0.1, SEDUCE_T_SIZE * 2, TRUE, NULL, NULL, 0.7, 0.7, 0.7, 1, 1, 1, 1, 1); \
						if(S_TIS_ACTIVE == state) \
							options->manipulator = &pointer[i + options->scroll]; \
						if(S_TIS_DONE == state) \
						{ \
							pointer[i + options->scroll] = (data_type)value; \
							update_start = (i + options->scroll) * sizeof(data_type) + offset; \
							update_length = sizeof(data_type); \
						} \
 					} \
					y -= (float)((draw_length + options->columns - 1) / options->columns) * SEDUCE_T_SIZE * 4; \
				}


float opa_widget_draw_expand(BInputState *input, float x, float y, float height, uint array_length, uint draw_length, OPADisplayOptions *options)
{	
	if(array_length > draw_length)
	{
		uint lines, visible;
		float pos[3];
		lines = (array_length + options->columns - 1) / options->columns;
		visible = (draw_length + options->columns - 1) / options->columns;
		if(input->mode == BAM_DRAW)
		{
			r_primitive_line_2d(0, y + SEDUCE_T_SIZE * 4 + height - height * (float)options->scroll / (float)array_length, 
								0, y + SEDUCE_T_SIZE * 4 + height - height * ((float)visible / (float)lines) - height * (float)options->scroll / (float)array_length, 1, 1, 1, 1.0);

			seduce_element_add_rectangle(input, &options->scroll, 0, x - 0.15, 
																y + SEDUCE_T_SIZE * 4 + height - height * ((float)visible / (float)lines) - height * (float)options->scroll / (float)array_length, 
																0.05, 
																height * ((float)visible / (float)lines));
		}
		pos[1] = y + SEDUCE_T_SIZE * 4 + height;
		pos[0] = pos[2] = 0.0;
		if(seduce_manipulator_point_axis(input, NULL, pos, &options->scroll, NULL, FALSE, 1, 0.0) && input->mode == BAM_EVENT)
		{
			pos[1] -= y + SEDUCE_T_SIZE * 4 + height;
			if(pos[1] >= 0.0001)
				options->scroll = 0;
			else
			{
				options->scroll = -pos[1] * (float)array_length / height;
				if(options->scroll + draw_length > array_length)
					options->scroll = array_length - draw_length;
				options->scroll = (options->scroll / options->columns) * options->columns;
			}
		}
	}
	if(array_length > options->columns)
	{
		if(input->mode == BAM_DRAW)
		{
			float f;
			uint i;
			seduce_element_add_rectangle(input, &options->expand, 0, x, y, options->width * options->columns, SEDUCE_T_SIZE * 4);
			if(options->expand)
			{
				for(i = 0; i < options->columns; i++)
				{
					f = x + options->width * ((float)i + 0.5);
					r_primitive_line_2d(f + 0.02, y + SEDUCE_T_SIZE * 1, f, y + SEDUCE_T_SIZE * 3, 0.4, 0.4, 0.4, 1.0);
					r_primitive_line_2d(f - 0.02, y + SEDUCE_T_SIZE * 1, f, y + SEDUCE_T_SIZE * 3, 0.4, 0.4, 0.4, 1.0);
				}
			}else
			{
				for(i = 0; i < options->columns; i++)
				{
					f = x + options->width * ((float)i + 0.5);
					r_primitive_line_2d(f + 0.02, y + SEDUCE_T_SIZE * 3, f, y + SEDUCE_T_SIZE * 1, 0.4, 0.4, 0.4, 1.0);
					r_primitive_line_2d(f - 0.02, y + SEDUCE_T_SIZE * 3, f, y + SEDUCE_T_SIZE * 1, 0.4, 0.4, 0.4, 1.0);
				}
			}
		}
		if(seduce_widget_button_invisible(input, &options->expand, x + options->width * (float)options->columns * 0.5, y + SEDUCE_T_SIZE * 2, 0.001, FALSE))
			options->expand = !options->expand;
		y -= SEDUCE_T_SIZE * 4;
	}
	return y;
}


float opa_widget_draw_grid(BInputState *input, float x, float y, uint draw_length, float size, OPADisplayOptions *options, void *data)
{
	if(draw_length > 1)
	{ 
		char buffer[16], *lables[OPA_CD_COUNT] = {"None", "Graph", "Plot 2D", "Plot 3D"};
		float x2, y2, pos[3] = {0, 0, 0};
		uint64 merge_pointer;
		uint i, j, type;

		for(i = 0; i < options->columns; i++)
		{
			sprintf(buffer, "%u", i);
			type = options->column_data[i];
			if(type >= OPA_CD_COUNT)
				type = 0;
			lables[0] = buffer;
			seduce_popup_text(input, &options->column_data[i], &type, lables, OPA_CD_COUNT, S_PUT_TOP, x + ((float)i + 0.5) * options->width , y + SEDUCE_T_SIZE * 0.5, 0.5, SEDUCE_T_SIZE, SEDUCE_T_SPACE, lables[type], 0.8, 0.8, 0.8, 1, 1, 1, 1, 1, FALSE);
			options->column_data[i] = type;
			//if(seduce_text_button(input, &options->column_data[i], x + ((float)i + 0.5) * options->width , y + SEDUCE_T_SIZE * 0.5, 0.5, SEDUCE_T_SIZE, SEDUCE_T_SPACE, buffer, 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
				;
		}


		y -= SEDUCE_T_SIZE * 4.0;
		if(input->mode == BAM_DRAW)
		{
			x2 = x + options->columns * options->width;
			for(i = 0; i < draw_length / options->columns; i++)
			{
				sprintf(buffer, "%u", i + options->scroll / options->columns);
				seduce_text_line_draw(NULL, x2 + SEDUCE_T_SIZE, y - (float)i * 4 * SEDUCE_T_SIZE, SEDUCE_T_SIZE, SEDUCE_T_SPACE, buffer, 0.8, 0.8, 0.8, 1, -1);
			}

			for(i = 0; i < options->columns; i++)
				r_primitive_line_2d(x + (float)i * options->width + 0.01, y + 4 * SEDUCE_T_SIZE, 
									x + (float)i * options->width + options->width - 0.01, y + 4 * SEDUCE_T_SIZE, 
												0.6, 0.7, 0.4, 1);

			for(i = 1; i < options->columns; i++)
			{
				for(j = 0; j < draw_length / options->columns - 1; j++)
				{
					x2 = x + (float)i * options->width;
					y2 = y - (float)j * 4 * SEDUCE_T_SIZE;
					r_primitive_line_2d(x2 - 0.004, y2,
										x2 + 0.004, y2,
												0.3, 0.3, 0.3, 1);
					r_primitive_line_2d(x2, y2 - 0.004,
										x2, y2 + 0.004,
												0.3, 0.3, 0.3, 1);
				}
			}
		}
		pos[0] = x + (float)options->columns * options->width; // + 0.025;
		pos[1] = y - (float)(draw_length / options->columns) * SEDUCE_T_SIZE * 2.0 + SEDUCE_T_SIZE * 4.0;
		if(input->mode == BAM_DRAW)
		{
			r_primitive_line_2d(pos[0], pos[1] + SEDUCE_T_SIZE * 2.0, pos[0], y + SEDUCE_T_SIZE * 2.0, 0.3, 0.3, 0.3, 1);
			r_primitive_line_2d(pos[0], pos[1] - SEDUCE_T_SIZE * 2.0, pos[0], y - (float)(draw_length / options->columns) * SEDUCE_T_SIZE * 4.0 + SEDUCE_T_SIZE * 6.0, 0.3, 0.3, 0.3, 1);
		}
		merge_pointer = (uint64)&options->width + (uint64)data;
		if(seduce_manipulator_point_axis(input, NULL, pos, (void *)merge_pointer, NULL, FALSE, 0, 1))
		{
			if(input->mode == BAM_EVENT)
			{
				pos[0] -= x;
				if(pos[0] < size + 0.025)
					pos[0] = size + 0.025;
				options->columns = (uint)(pos[0] / size);
				options->width = (pos[0] - 0.025) / options->columns;
				if(options->columns > draw_length)
					options->columns = draw_length;
			}
		}else if(input->mode == BAM_MAIN)
		{
		/*	options->width -= input->delta_time * 0.1;
			if(options->width < size)
				options->width = size;*/
		}
	}
	return y;
}

void opa_widget_draw_spline(float *a, float *b, float *c, float *d)
{
	float tmp1[3], tmp2[3], f;
	
	tmp1[0] = a[0];
	tmp1[1] = a[1];
	tmp1[2] = a[2];
	for(f = 0.025; f < 1.0; f += 0.025)
	{
		f_spline3df(tmp2, f, a, b, c, d);
		r_primitive_line_3d(tmp1[0], tmp1[1], tmp1[2], tmp2[0], tmp2[1], tmp2[2], 0.3, 0.3, 0.3, 0.3);
		f += 0.025;
		f_spline3df(tmp1, f, a, b, c, d);
		r_primitive_line_3d(tmp1[0], tmp1[1], tmp1[2], tmp2[0], tmp2[1], tmp2[2], 0.4, 0.4, 0.4, 0.3);
	}
}

void opa_widget_draw_pointer_line(BInputState *input, OPAProject *project, uint memmory_id, float x, float y, uint64 pointer)
{
	float pos[3], m[16], a[3], b[3], c[3], d[3];
	uint i;
	for(i = 0; i < project->memory_count; i++)
	{
		if(project->memmory[i].pointer <= pointer && project->memmory[i].pointer + project->memmory[i].data_size > pointer)
		{
			if(project->memmory[i].hidden)
				return;
			a[0] = b[0] = x;
			b[0] += 0.3;
			a[1] = b[1] = y;
			a[2] = b[2] = 0;
			f_transform3f(pos, project->memmory[i].matrix, -0.8, -SEDUCE_T_SIZE, 0);
			f_transforminv_scaled3f(c, project->memmory[memmory_id].matrix, pos[0], pos[1], pos[2]);
			f_transform3f(pos, project->memmory[i].matrix, -0.4, -SEDUCE_T_SIZE, 0);
			f_transforminv_scaled3f(d, project->memmory[memmory_id].matrix, pos[0], pos[1], pos[2]);
			opa_widget_draw_spline(a, b, c, d);
			return;
		}
	}
}

float opa_widget_draw_memory_type(BInputState *input, THandle *handle, OPAProject *project, uint8 *memmory, uint type, float x, float y, uint memmory_id, uint indirection, uint array_length, uint offset, OPADisplayOptions *options)
{
	STypeInState state;
	char name[256];
	uint i, j, update_start = -1, update_length = 0, draw_length;
	if(options->columns >= array_length)
		options->columns = array_length;
	//options->expand = FALSE;
	if(!options->expand)
		draw_length = options->columns;
	else
	{
		draw_length = 24 * options->columns;

	}
	if(draw_length + options->scroll > array_length)
		draw_length = array_length - options->scroll;

//	if(options->expand && array_length > columns)
//		array_length = columns;
	if(indirection != 0 || type == OPA_TYPE_VOID || type == OPA_TYPE_FUNCTION)
	{
		if(project->types[OPA_TYPE_VOID].size_of == 8)
		{
			for(j = 0; j < draw_length; j++)
			{
				uint64 *pointer;
				pointer = &memmory[offset + 8 * (j + options->scroll)];
				if(*pointer == 0)
				{
					static void *active_id = NULL;
					if(active_id != pointer && !seduce_element_active(input, pointer, NULL))
					{
						seduce_text_button(input, pointer, x + SEDUCE_T_SIZE, y, 0.0, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, "NULL", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1);
					}else
					{
						STypeInState state;
						static uint value;
						state =	seduce_text_edit_uint(input, pointer, NULL, &value, x + SEDUCE_T_SIZE, y,  0.1, SEDUCE_T_SIZE * 2, TRUE, NULL, NULL, 0.7, 0.7, 0.7, 1, 1, 1, 1, 1);
						if(state == S_TIS_IDLE && active_id == pointer)
							active_id = NULL;
						if(state == S_TIS_ACTIVE)
							active_id = pointer;
						if(state == S_TIS_DONE && value != 0)
							opa_request_memory_allocate(project, handle, memmory_id, (options->scroll) * 8 + offset, value * project->types[type].size_of);
					}
				}else
				{
					sprintf(name, "%llx", (uint64)*pointer);
					if(input->mode == BAM_DRAW)
						opa_widget_draw_pointer_line(input, project, memmory_id, x + 0.24, y + SEDUCE_T_SIZE * 1.5, *pointer);
					if(seduce_text_button(input, pointer, x + SEDUCE_T_SIZE, y, 0.0, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, name, 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
					{
						if(indirection != 0)
						{
							float pos[3];
							f_transform3f(pos, project->memmory[memmory_id].matrix, x + 0.4, y, 0);
							opa_request_memory(project, *pointer, OPA_TYPE_VOID, memmory_id, offset + sizeof(uint32) * j, indirection - 1, pos[0], pos[1]);
						}
					}
				}
				y -= SEDUCE_T_SIZE * 4;
			}
			y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * (float)draw_length, array_length, draw_length, options);
		}else
		{
			uint32 *pointer;
			for(j = 0; j < draw_length; j++)
			{
				pointer = &memmory[offset + 4 * (j + options->scroll)];
				if(*pointer == 0)
				{
					static void *active_id = NULL;
					if(active_id != pointer && !seduce_element_active(input, pointer, NULL))
					{
						seduce_text_button(input, pointer, x + SEDUCE_T_SIZE, y, 0.0, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, "NULL", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1);
					}else
					{
						STypeInState state;
						static uint value;
						state =	seduce_text_edit_uint(input, pointer, NULL, &value, x + SEDUCE_T_SIZE, y,  0.1, SEDUCE_T_SIZE * 2, TRUE, NULL, NULL, 0.7, 0.7, 0.7, 1, 1, 1, 1, 1);
						if(state == S_TIS_IDLE && active_id == pointer)
							active_id = NULL;
						if(state == S_TIS_ACTIVE)
							active_id = pointer;
						if(state == S_TIS_DONE && value != 0)
							opa_request_memory_allocate(project, handle, memmory_id, (options->scroll) * 4 + offset, value * project->types[type].size_of);
					}
				}else
				{
					sprintf(name, "%x", (uint)*pointer);
					if(input->mode == BAM_DRAW)
						opa_widget_draw_pointer_line(input, project, memmory_id, x + 0.12, y + SEDUCE_T_SIZE * 1.5, *pointer);
					if(seduce_text_button(input, pointer, x + SEDUCE_T_SIZE, y, 0.0, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, name, 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
					{
						if(indirection != 0)
						{
							float pos[3];
							f_transform3f(pos, project->memmory[memmory_id].matrix, x + 0.4, y, 0);
							opa_request_memory(project, *pointer, type, memmory_id, offset + sizeof(uint32) * j, indirection - 1, pos[0], pos[1]);
						}
					}
				}
				y -= SEDUCE_T_SIZE * 4;
			}
			y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * (float)draw_length, array_length, draw_length, options);
		}				
	}else
	{
		name[0] = 0;
		if(type >= OPA_TYPE_COUNT)
		{
			if(project->types[type].construct == OPA_C_ENUM)
			{
				char **lables, *l[64];
				unsigned int *pointer;	
				pointer = &memmory[offset + options->scroll * sizeof(unsigned int)];
				if(project->types[type].member_count <= 64)
					lables = l;
				else
					lables = malloc((sizeof *lables) * project->types[type].member_count);

				for(i = 0; i < project->types[type].member_count;  i++)
					lables[i] = project->types[type].members[i].value_name;

				for(i = 0; i < draw_length;  i++)
				{
					for(j = 0; j < project->types[type].member_count; j++)
						if(pointer[i] == project->types[type].members[j].enum_value)
							break;
					if(j < project->types[type].member_count)
					{
						sprintf(name, "%s (%u)", project->types[type].members[j].value_name, project->types[type].members[j].enum_value);
					}else
						sprintf(name, "Ilegal Enum (%u)", (uint)pointer[i]);
					j = &pointer[i];
					if(S_TIS_DONE == seduce_popup_text(input, &pointer[i], &j, lables, project->types[type].member_count, S_PUT_TOP, x, y, 0, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, name, 0.7, 0.7, 0.7, 1, 1, 1, 1, 1, TRUE) && j < project->types[type].member_count)
					{
						pointer[i] = project->types[type].members[j].enum_value;
						update_start = offset + i * sizeof(unsigned int) + options->scroll * sizeof(unsigned int);
						update_length = sizeof(unsigned int);
					}
					y -= SEDUCE_T_SIZE * 4;
				}
				if(project->types[type].member_count > 64)
					free(lables);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * (float)draw_length, array_length, draw_length, options);
			}else if(project->types[type].construct == OPA_C_STRUCT ||
				project->types[type].construct == OPA_C_UNION)
			{
				float start_y;
				y -= SEDUCE_T_SIZE * 4;
				start_y = y;
				for(i = 0; i < draw_length; i++)
				{
					for(j = 0; j < project->types[type].member_count; j++)
					{
						if((project->types[project->types[type].members[j].base_type].construct == OPA_C_STRUCT ||
							project->types[project->types[type].members[j].base_type].construct == OPA_C_UNION) && 
							project->types[type].members[j].indirection == 0)
						{
							if(seduce_text_button(input, &project->types[type].members[j].options.show, -0.05 - x, y, 1, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, project->types[type].members[j].value_name, 0.5, 0.7, 0.0, 1, 1, 1, 1, 1))
								project->types[type].members[j].options.show = !project->types[type].members[j].options.show;
							if(project->types[type].members[j].options.show)
								y = opa_widget_draw_memory_type(input, handle, project, memmory,
									project->types[type].members[j].base_type, 
									0.05 + x, y, memmory_id, 
									project->types[type].members[j].indirection, 
									project->types[type].members[j].array_length, 
									offset + project->types[type].size_of * (i + options->scroll) + project->types[type].members[j].offset, 
									&project->types[type].members[j].options);
							else
								y -= SEDUCE_T_SIZE * 4;

						}else
						{
							if(input->mode == BAM_DRAW)
							{
								seduce_text_line_draw(NULL, -0.05 - x - seduce_text_line_length(NULL, SEDUCE_T_SIZE * 2, SEDUCE_T_SPACE, project->types[type].members[j].value_name, -1) - SEDUCE_T_SIZE, y, SEDUCE_T_SIZE * 2, SEDUCE_T_SPACE, project->types[type].members[j].value_name, 0.5, 0.7, 0.0, 1, -1);
								if(x > -0.01)
								{
									r_primitive_line_2d(-0.01, y + SEDUCE_T_SIZE * 1.5, -0.04 - x, y + SEDUCE_T_SIZE * 1.5, 0.2, 0.2, 0.2, 0.2);
									r_primitive_line_2d(0.01, y + SEDUCE_T_SIZE * 1.5, 0.04 + x, y + SEDUCE_T_SIZE * 1.5, 0.2, 0.2, 0.2, 0.2);
								}
							}
							y = opa_widget_draw_memory_type(input, handle, project, memmory,
								project->types[type].members[j].base_type, 
								0.05 + x, y, memmory_id, 
								project->types[type].members[j].indirection, 
								project->types[type].members[j].array_length, 
								offset + project->types[type].size_of * (i + options->scroll) + project->types[type].members[j].offset, 
								&project->types[type].members[j].options);
						}
					}
				}
				y = opa_widget_draw_expand(input, x, y, start_y - y, array_length, draw_length, options);
			}else
				y = opa_widget_draw_memory_type(input, handle, project, memmory,
											project->types[type].members[0].base_type, 
											x, y, memmory_id, 
											project->types[type].members[0].indirection, 
											project->types[type].members[0].array_length, 
											0, 
											options);
		}else switch(type)
		{
			default :
			y -= SEDUCE_T_SIZE * 4;
			break;
			case OPA_TYPE_SINGED_CHAR :
			{
				char *pointer;
				int value;
				pointer = &memmory[offset];
				for(i = 0; i < array_length && pointer[i] != 0; i++);
				if(i < array_length)
				{
					if(array_length > 256 && options->expand)
					{
						STextBlockMode block;
						STextBox box;
						float size; 
						block.character_position = 0;
						block.font = seduce_font_default_get();
						block.red = 1.0;
						block.green = 1.0; 
						block.blue = 1.0;
						block.alpha = 1.0;
						block.letter_size = SEDUCE_T_SIZE;
						block.letter_spacing = SEDUCE_T_SPACE;
						size = seduce_text_block_height(0.6, 2, SEDUCE_TBAS_LEFT, pointer, 0, &block, 1, -1);
						box.pos_x = x;
						box.pos_y = y + SEDUCE_T_SIZE * 2;
						box.line_size = 0.6;
						box.height = 1000000000;
						box.line_spacing = 2;
						box.style = SEDUCE_TBAS_LEFT;
						if(S_TIS_IDLE !=seduce_text_box_edit(input, pointer, pointer, array_length, &box, 1, &block, 1))
						{
							update_start = offset;
							update_length = array_length;
						}
						y -= size;
					}else
					{
						if(S_TIS_IDLE != seduce_text_edit_line(input, pointer, NULL, pointer, array_length, x + SEDUCE_T_SIZE, y, 0.4, SEDUCE_T_SIZE * 2, "Terminated string", TRUE, NULL, NULL, 0.7, 0.7, 0.7, 1, 1, 1, 1, 1))
						{
							update_start = offset;
							update_length = array_length;
						}
						y -= SEDUCE_T_SIZE * 4;
					}
				}else
				{
					y = opa_widget_draw_grid(input, x, y, draw_length, 0.1, options, &memmory[offset]);
					opa_number_draw_macro(char, int, seduce_text_edit_int, options->width);
					y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
				}
			}
			break;
			case OPA_TYPE_UNSINGED_CHAR :
			{
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.1, options, &memmory[offset]);
				opa_number_draw_macro(unsigned char, uint, seduce_text_edit_int, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
			case OPA_TYPE_SIGNED_SHORT :
			{
				
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.1, options, &memmory[offset]);
				opa_number_draw_macro(short, int, seduce_text_edit_int, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
			case OPA_TYPE_UNSIGNED_SHORT :
			{
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.1, options, &memmory[offset]);
				opa_number_draw_macro(unsigned short, uint, seduce_text_edit_uint, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
			case OPA_TYPE_SINGED_INT :
			{
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.1, options, &memmory[offset]);
				opa_number_draw_macro(int, int, seduce_text_edit_int, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
			case OPA_TYPE_UNSINGED_INT :
			{
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.1, options, &memmory[offset]);
				opa_number_draw_macro(unsigned int, uint, seduce_text_edit_uint, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
			case OPA_TYPE_SIGNED_LONG_LONG :
			{
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.1, options, &memmory[offset]);
				opa_number_draw_macro(long long, int, seduce_text_edit_int, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
			case OPA_TYPE_UNSIGNED_LONG_LONG :
			{
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.1, options, &memmory[offset]);
				opa_number_draw_macro(unsigned long long, uint, seduce_text_edit_uint, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
			case OPA_TYPE_FLOAT :
			{
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.2, options, &memmory[offset]);
				opa_number_draw_macro(float, float, seduce_text_edit_float, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
			case OPA_TYPE_DOUBLE :
			{
				y = opa_widget_draw_grid(input, x, y, draw_length, 0.2, options, &memmory[offset]);
				opa_number_draw_macro(double, double, seduce_text_edit_double, options->width);
				y = opa_widget_draw_expand(input, x, y, SEDUCE_T_SIZE * 4 * ((draw_length + options->columns - 1) / options->columns), array_length, draw_length, options);
			}
			break;
		}
		if(update_start != -1)
		{
			opa_request_memory_set(project, handle, memmory_id, update_start, update_length, &memmory[update_start]);
			update_start = -1; 
		}
	}

	return y;
}

/*
Cast/delete/Load/save/print
*/

typedef struct{
	OPAProject *project;
	OPAMemory *memmory;
}OPAWidgetCastParam;

void opa_widget_draw_memory_cast_popup_func(BInputState *input, float time, void *user)
{
	OPAWidgetCastParam *param;
	uint i, columns;
	float y;
	param = (OPAWidgetCastParam *)user;

	columns = param->project->type_count / 12;
	if(columns < 1)
		columns = 1;
	if(columns > 6)
		columns = 6;
	columns = 6;

	i = (param->project->type_count + columns - 1) / columns;
	y = (i - 1) * 2.0 * SEDUCE_T_SIZE;
	if(input->mode == BAM_DRAW)
	{
		seduce_background_quad_draw(input, NULL, 0,
												-1, y + 4.0 * SEDUCE_T_SIZE, 0,
												1, y + 4.0 * SEDUCE_T_SIZE, 0,
												1, -y - 4.0 * SEDUCE_T_SIZE, 0,
												-1, -y - 4.0 * SEDUCE_T_SIZE, 0, 
														0, 0, 1,
														0, 0, 0, 1);
		seduce_background_polygon_flush(input, NULL, time);
	}
	y -= 1.0 * SEDUCE_T_SIZE;
	for(i = 0; i < param->project->type_count; i++)
	{
		if(param->memmory->original_type == i)
		{
			if(seduce_text_button(input, &param->project->types[i].type_name, time * ((float)(i % columns) - (float)(columns - 1) * 0.5) * 1.0 / 3.0, time * (-4.0 * SEDUCE_T_SIZE * (float)(i / columns) + y), 0.5, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, &param->project->types[i].type_name, 0.6, 1.0, 0.2, 1, 0.9, 1, 0.0, 1))
				param->memmory->type = i;
		}else
			if(seduce_text_button(input, &param->project->types[i].type_name, time * ((float)(i % columns) - (float)(columns - 1) * 0.5) * 1.0 / 3.0, time * (-4.0 * SEDUCE_T_SIZE * (float)(i / columns) + y), 0.5, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, &param->project->types[i].type_name, 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
				param->memmory->type = i;
	}
}

void opa_widget_draw_memory(BInputState *input, THandle *handle, OPAProject *project, OPAMemory *memmory, char *type_lables)
{
	OPAWidgetCastParam param; 
	char name[512];
	float x = 0, y = 0, pos, length, a[3] = {0, 0, 0}, b[3] = {0, 0, 0};
	uint i, size;
	uint8 *ids;
	if(input->mode == BAM_DRAW)
	{
		r_matrix_push(NULL);
		r_matrix_matrix_mult(NULL, memmory->matrix);
	}
	seduce_background_shape_matrix_interact(input, memmory->matrix, memmory->matrix, TRUE, FALSE);
	
	x = SEDUCE_T_SIZE;
	param.project = project;
	param.memmory = memmory;
	seduce_popup_detect_text(input, &memmory->type, x, y, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Cast", opa_widget_draw_memory_cast_popup_func, &param, FALSE, 0.8, 0.8, 0.8, 1, 1, 1, 1, 1);

	x += seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Cast", -1) + SEDUCE_T_SIZE;
	if(input->mode == BAM_DRAW)
		r_primitive_line_2d(x, -SEDUCE_T_SIZE * 0.0, x, SEDUCE_T_SIZE * 1.5, 0.3, 0.6, 0.0, 1.0);
	x += SEDUCE_T_SIZE;
	if(memmory->parent != -1)
	{
		if(seduce_text_button(input, &memmory->hidden, x, y, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Hide", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
			memmory->hidden = TRUE;
		x += seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Hide", -1) + SEDUCE_T_SIZE;
		if(input->mode == BAM_DRAW)
			r_primitive_line_2d(x, -SEDUCE_T_SIZE * 0.0, x, SEDUCE_T_SIZE * 1.5, 0.3, 0.6, 0.0, 1.0);
		x += SEDUCE_T_SIZE;
	}
	if(seduce_text_button(input, &memmory->path[0], x, y, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Load", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
		betray_requester_load(NULL, 0, &memmory->path[0]);
	x += seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Load", -1) + SEDUCE_T_SIZE;
	if(input->mode == BAM_DRAW)
		r_primitive_line_2d(x, -SEDUCE_T_SIZE * 0.0, x, SEDUCE_T_SIZE * 1.5, 0.3, 0.6, 0.0, 1.0);
	x += SEDUCE_T_SIZE;
	if(seduce_text_button(input, &memmory->path[1], x, y, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Save", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
		betray_requester_save(NULL, 0, &memmory->path[0]);
	x += seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Save", -1) + SEDUCE_T_SIZE;
	if(input->mode == BAM_DRAW)
		r_primitive_line_2d(x, -SEDUCE_T_SIZE * 0.0, x, SEDUCE_T_SIZE * 1.5, 0.3, 0.6, 0.0, 1.0);
	x += SEDUCE_T_SIZE;
	if(seduce_text_button(input, &memmory->path[2], x, y, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Export", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
		betray_requester_save(NULL, 0, &memmory->path[1]);
	x += seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Export", -1) + SEDUCE_T_SIZE;
	if(input->mode == BAM_DRAW)
		r_primitive_line_2d(x, -SEDUCE_T_SIZE * 0.0, x, SEDUCE_T_SIZE * 1.5, 0.3, 0.6, 0.0, 1.0);
	x += SEDUCE_T_SIZE;
	if(memmory->paused)
	{
		if(seduce_text_button(input, &memmory->paused, x, y, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "UnPause", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
			memmory->paused = FALSE;
		x += seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "UnPause", -1) + SEDUCE_T_SIZE;
	}else
	{
		if(seduce_text_button(input, &memmory->paused, x, y, 0, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Pause", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
			memmory->paused = TRUE;
		x += seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Pause", -1) + SEDUCE_T_SIZE;
	}

	
	sprintf(name, "%s", project->types[memmory->type].type_name, 5);
	seduce_text_button(input, memmory->matrix, -SEDUCE_T_SIZE, y, 1, SEDUCE_T_SIZE * 4.0, SEDUCE_T_SPACE, name, 0.8, 0.8, 0.8, 1, 1, 1, 1, 1);

	sprintf(name, "[%u / %u]", memmory->options.scroll, memmory->data_size / project->types[memmory->type].size_of);
//	seduce_text_button(input, memmory->matrix, x + SEDUCE_T_SIZE, y + SEDUCE_T_SIZE * 2, 0, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, "[", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1);
//	seduce_text_button(input, memmory->matrix, x + SEDUCE_T_SIZE, y + SEDUCE_T_SIZE * 2, 0, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, "]", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1);

	ids = &project->types[memmory->type].size_of;
	x = SEDUCE_T_SIZE;
	if(seduce_text_button(input, &ids[0], x, y + SEDUCE_T_SIZE * 4.5, 0, SEDUCE_T_SIZE * 4.0, SEDUCE_T_SPACE, "[", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1) && memmory->options.scroll > 0)
		memmory->options.scroll--;
	x += seduce_text_line_length(NULL, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, "[ ", -1) + SEDUCE_T_SIZE * 0.5;

		
	size = memmory->data_size / project->types[memmory->type].size_of;
	if(!memmory->options.expand && memmory->data_size / project->types[memmory->type].size_of > 1)
	{
		sprintf(name, "%u", memmory->options.scroll);
		if(!seduce_text_edit_active(&ids[2])) 
			length = seduce_text_line_length(NULL, SEDUCE_T_SIZE * 4.0, SEDUCE_T_SPACE, name, -1);
		else
			length = 0.15;
		if(S_TIS_DONE == seduce_text_edit_uint(input, &ids[2], NULL, &memmory->options.scroll, x, y + SEDUCE_T_SIZE * 4, length, SEDUCE_T_SIZE * 4.0, 0, NULL, NULL, 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
		{
			if(memmory->options.scroll > size - 1)
				memmory->options.scroll = size - 1;
			memmory->options.scroll -= memmory->options.scroll % memmory->options.columns; 
		}
		if(input->mode == BAM_DRAW)
		seduce_text_line_draw(NULL, x + length + SEDUCE_T_SIZE * 2, y + SEDUCE_T_SIZE * 4, SEDUCE_T_SIZE * 4.0, SEDUCE_T_SPACE, "/", 0.6, 0.8, 0.4, 0.4, -1);
		x += length + SEDUCE_T_SIZE * 6;
	}
	sprintf(name, "%u", size);
	if(!seduce_text_edit_active(&ids[1])) 
		length = seduce_text_line_length(NULL, SEDUCE_T_SIZE * 4.0, SEDUCE_T_SPACE, name, -1);
	else
		length = 0.15;
	if(S_TIS_DONE == seduce_text_edit_uint(input, &ids[1], NULL, &size, x, y + SEDUCE_T_SIZE * 4, length, SEDUCE_T_SIZE * 4.0, 0, NULL, NULL, 0.8, 0.8, 0.8, 1, 1, 1, 1, 1))
	{
		memmory->data = realloc(memmory->data, size * project->types[memmory->type].size_of);
		for(i = memmory->data_size; i < size * project->types[memmory->type].size_of; i++)
			memmory->data[i] = 0;
		memmory->data_size = size * project->types[memmory->type].size_of;
	}
	x += length + SEDUCE_T_SIZE * 2;
	if(seduce_text_button(input, &ids[3], x, y + SEDUCE_T_SIZE * 4.5, 0, SEDUCE_T_SIZE * 4.0, SEDUCE_T_SPACE, "]", 0.8, 0.8, 0.8, 1, 1, 1, 1, 1) && memmory->options.scroll + 1 < size)
		memmory->options.scroll++;
	if(memmory->data_size / project->types[memmory->type].size_of > 1)
	{

		if(memmory->options.expand)
			seduce_text_button(input, &memmory->options.expand, -SEDUCE_T_SIZE, y + SEDUCE_T_SIZE * 6, 1, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, "Colapse", 0.6, 0.6, 0.6, 1, 0.5, 1, 0.2, 1);
		else
			seduce_text_button(input, &memmory->options.expand, -SEDUCE_T_SIZE, y + SEDUCE_T_SIZE * 6, 1, SEDUCE_T_SIZE * 2.0, SEDUCE_T_SPACE, "Expand", 0.6, 0.6, 0.6, 1, 0.5, 1, 0.2, 1);		
	}

	/*
	Remove
	Cast
	*/
	x += SEDUCE_T_SIZE * 4;
	if(input->mode == BAM_DRAW)
	{
		if(memmory->path[0] != 0)
		{
			pos = seduce_text_line_draw(NULL, x, y + SEDUCE_T_SIZE * 8.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, "Pointer : ", 0.6, 0.7, 0.5, 0.4, -1);
			sprintf(name, "%llx", memmory->pointer);
			seduce_text_line_draw(NULL, x + pos, y + SEDUCE_T_SIZE * 8.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, name, 0.4, 0.4, 0.4, 0.4, -1);
			pos = seduce_text_line_draw(NULL, x, y + SEDUCE_T_SIZE * 6.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, "Size : ", 0.6, 0.7, 0.5, 0.4, -1);
			sprintf(name, "%u",  memmory->data_size);
			seduce_text_line_draw(NULL, x + pos, y + SEDUCE_T_SIZE * 6.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, name, 0.4, 0.4, 0.4, 0.4, -1);
			pos = seduce_text_line_draw(NULL, x, y + SEDUCE_T_SIZE * 4.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, "Line : ", 0.6, 0.7, 0.5, 0.4, -1);
			sprintf(name, "%u", memmory->line);
			seduce_text_line_draw(NULL, x + pos, y + SEDUCE_T_SIZE * 4.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, name, 0.4, 0.4, 0.4, 0.4, -1);
			pos = seduce_text_line_draw(NULL, x, y + SEDUCE_T_SIZE * 2.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, "Path : ", 0.6, 0.7, 0.5, 0.4, -1);
			seduce_text_line_draw(NULL, x + pos, y + SEDUCE_T_SIZE * 2.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, memmory->path, 0.4, 0.4, 0.4, 0.4, -1);
		}else
		{
			pos = seduce_text_line_draw(NULL, x, y + SEDUCE_T_SIZE * 4.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, "Pointer : ", 0.6, 0.7, 0.5, 0.4, -1);
			sprintf(name, "%llx", memmory->pointer);
			seduce_text_line_draw(NULL, x + pos, y + SEDUCE_T_SIZE * 4.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, name, 0.4, 0.4, 0.4, 0.4, -1);
			pos = seduce_text_line_draw(NULL, x, y + SEDUCE_T_SIZE * 2.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, "Size : ", 0.6, 0.7, 0.5, 0.4, -1);
			sprintf(name, "%u",  memmory->data_size);
			seduce_text_line_draw(NULL, x + pos, y + SEDUCE_T_SIZE * 2.5, SEDUCE_T_SIZE * 1.0, SEDUCE_T_SPACE, name, 0.4, 0.4, 0.4, 0.4, -1);
		}

		r_primitive_line_2d(-0.4, -SEDUCE_T_SIZE, -SEDUCE_T_SIZE, -SEDUCE_T_SIZE, 0.4, 0.8, 0.0, 1.0);
		r_primitive_line_2d(0.3, -SEDUCE_T_SIZE, SEDUCE_T_SIZE, -SEDUCE_T_SIZE, 0.4, 0.8, 0.0, 1.0);
		r_primitive_line_2d(0.0, -SEDUCE_T_SIZE, 0.0, memmory->length, 0.4, 0.8, 0.0, 1.0);
		a[1] = memmory->length;
		seduce_element_add_line(input, memmory->matrix, 0, a, b, 0.01);
	}
	if(memmory->data != NULL)
	{
		y = opa_widget_draw_memory_type(input, handle, project, memmory->data, memmory->type, 0, 0 - SEDUCE_T_SIZE * 4, memmory - project->memmory, memmory->indirection, memmory->data_size / project->types[memmory->type].size_of, 0, &memmory->options);
		if(input->mode == BAM_DRAW)
			memmory->length = y;
	}
	if(input->mode == BAM_DRAW)
	{
		r_primitive_line_flush();
		r_matrix_pop(NULL);
	}
}




void opa_widget_draw(BInputState *input, THandle *handle, OPAProject *project)
{
	char **type_lables;
	uint i;
	type_lables = malloc((sizeof *type_lables) * project->type_count);
	for(i = 0; i < project->type_count; i++)
		type_lables[i] = project->types[i].type_name;
	for(i = 0; i < project->memory_count; i++)
		if(!project->memmory[i].hidden)
			opa_widget_draw_memory(input, handle, project, &project->memmory[i], type_lables);
	free(type_lables);
}