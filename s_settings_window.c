#include <math.h>
#include <stdlib.h>
#include "seduce.h"
#include "s_draw_3d.h"
/*
typedef enum{
	SEDUCE_PET_BOOLEAN,
	SEDUCE_PET_TRIGGER,
	SEDUCE_PET_INTEGER,
	SEDUCE_PET_UNSIGNED_INTEGER,
	SEDUCE_PET_INTEGER_BOUND,
	SEDUCE_PET_REAL,
	SEDUCE_PET_REAL_BOUND,
	SEDUCE_PET_2D_POS,
	SEDUCE_PET_3D_POS,
	SEDUCE_PET_4D_POS,
	SEDUCE_PET_QUATERNION,
	SEDUCE_PET_2D_NORMAL,
	SEDUCE_PET_3D_NORMAL,
	SEDUCE_PET_2X2MATRIX,
	SEDUCE_PET_3X3MATRIX,
	SEDUCE_PET_4X4MATRIX,
	SEDUCE_PET_TEXT,
	SEDUCE_PET_PASSWORD,
	SEDUCE_PET_TEXT_BUFFER,
	SEDUCE_PET_COLOR_RGB,
	SEDUCE_PET_COLOR_RGBA,
	SEDUCE_PET_TIME,
	SEDUCE_PET_DATE,
	SEDUCE_PET_SELECT,
	SEDUCE_PET_SECTION_START,
	SEDUCE_PET_SECTION_END,
	SEDUCE_PET_CUSTOM,
	SEDUCE_PET_COUNT
}ForgeDataElementType;

typedef struct{
	ForgeDataElementType type;
	char *text;
	char *description;
	union{
		boolean active; 
		boolean trigger; 
		int		integer;
		uint	uinteger;
		struct{
			double value;
			double max;
			double min;
		}real;
		float color[4];
		double vector[4];
		double matrix[16];
		char text[64];
		struct{
			char	*text_buffer;
			uint	buffer_size;
		}buffer;
		double time;
		struct{
			uint16	year;
			uint8	month;
			uint8	day;
		}date;
		struct{
			char	**text;
			uint	count;
			uint	active;
		}select;
	}param;
}ForgeDataElement;*/


boolean seduce_widget_share[SEDUCE_PET_COUNT] = {TRUE, /*SEDUCE_PET_BOOLEAN*/
												FALSE, /*SEDUCE_PET_TRIGGER*/
												TRUE, /*SEDUCE_PET_INTEGER*/
												TRUE, /*SEDUCE_PET_UNSIGNED_INTEGER*/
												TRUE, /*SEDUCE_PET_INTEGER_BOUND*/
												TRUE, /*SEDUCE_PET_REAL*/
												TRUE, /*SEDUCE_PET_REAL_BOUND*/
												FALSE, /*SEDUCE_PET_2D_POS*/
												FALSE, /*SEDUCE_PET_3D_POS*/
												FALSE, /*SEDUCE_PET_4D_POS*/
												TRUE, /*SEDUCE_PET_QUATERNION*/
												TRUE, /*SEDUCE_PET_2D_NORMAL*/
												TRUE, /*SEDUCE_PET_3D_NORMAL*/
												TRUE, /*SEDUCE_PET_2X2MATRIX*/
												TRUE, /*SEDUCE_PET_3X3MATRIX*/
												TRUE, /*SEDUCE_PET_4X4MATRIX*/
												TRUE, /*SEDUCE_PET_TEXT*/
												TRUE, /*SEDUCE_PET_PASSWORD*/
												FALSE, /*SEDUCE_PET_TEXT_BUFFER*/
												TRUE, /*SEDUCE_PET_COLOR_RGB*/
												TRUE, /*SEDUCE_PET_COLOR_RGBA*/
												TRUE, /*SEDUCE_PET_TIME*/
												TRUE, /*SEDUCE_PET_DATE*/
												TRUE, /*SEDUCE_PET_SELECT*/
												TRUE, /*SEDUCE_PET_POPUP*/
												TRUE, /*SEDUCE_PET_SECTION_START*/
												TRUE, /*SEDUCE_PET_SECTION_END*/
												TRUE}; /*SEDUCE_PET_CUSTOM*/

#define SEDUCE_PANEL_RIM_SIZE 2.0
#define SEDUCE_PANEL_LINE_SPACING 2.0


void seduce_widget_list_element_panel(BInputState *input, float pos_x, float pos_y, float width, float height, float scale, void *id, float *color, boolean active)
{
	if(active)
	{
		seduce_background_quad_draw(input, id, 0,
									pos_x, pos_y, 0, 
									pos_x + width, pos_y, 0, 
									pos_x + width, pos_y - (height - scale * SEDUCE_PANEL_RIM_SIZE) * 0.5, 0, 
									pos_x, pos_y - (height - scale * SEDUCE_PANEL_RIM_SIZE) * 0.5, 0, 
									0, 0, 1,
									color[0], color[1], color[2], color[3]);

		seduce_background_quad_draw(input, id, 0,
									pos_x, pos_y - (height - scale * SEDUCE_PANEL_RIM_SIZE) * 0.5, 0, 
									pos_x + width, pos_y - (height - scale * SEDUCE_PANEL_RIM_SIZE) * 0.5, 0, 
									pos_x + width, pos_y - (height) * 0.5, 0, 
									pos_x + scale, pos_y - (height) * 0.5, 0, 
									0, 0, 1,
									color[0], color[1], color[2], color[3]);
		seduce_background_quad_draw(input, id, 0,
									pos_x + width, pos_y - (height) * 0.5, 0, 
									pos_x + scale, pos_y - (height) * 0.5, 0, 
									pos_x, pos_y - (height + scale * SEDUCE_PANEL_RIM_SIZE) * 0.5, 0, 
									pos_x + width, pos_y - (height + scale * SEDUCE_PANEL_RIM_SIZE) * 0.5, 0, 
									0, 0, 1,
									color[0], color[1], color[2], color[3]);

		seduce_background_quad_draw(input, id, 0,
									pos_x, pos_y - (height + scale * SEDUCE_PANEL_RIM_SIZE) * 0.5, 0, 
									pos_x + width, pos_y - (height + scale * SEDUCE_PANEL_RIM_SIZE) * 0.5, 0, 
									pos_x + width, pos_y - height, 0, 
									pos_x, pos_y - height, 0, 
									0, 0, 1,
									color[0], color[1], color[2], color[3]);
	}else
		seduce_background_quad_draw(input, id, 0,
									pos_x, pos_y, 0, 
									pos_x + width, pos_y, 0, 
									pos_x + width, pos_y - height, 0, 
									pos_x, pos_y - height, 0, 
									0, 0, 1,
									color[0], color[1], color[2], color[3]);

}


float seduce_widget_list_element_background(BInputState *input, float pos_x, float pos_y, float width, float scale, SeducePanelElement *element, uint element_count, uint *selected_element, void *id, float time)
{
	float start, add, center[2], x, y, color[4] = {0.5, 0.5, 0.5, 0.7}, dark[4] = {0.0, 0.0, 0.0, 0.9};
	uint i;
	center[0] = pos_x + width * 0.5;
	center[1] = pos_y - SEDUCE_T_SIZE;
	start = pos_y;

	if(input->mode != BAM_DRAW)
		return start;

	seduce_background_quad_draw(input, id, 0,
										pos_x, pos_y, 0, 
										pos_x + width, pos_y, 0, 
										pos_x + width, pos_y + scale * (SEDUCE_PANEL_LINE_SPACING / 2.0 - SEDUCE_PANEL_RIM_SIZE), 0, 
										pos_x, pos_y + scale * (SEDUCE_PANEL_LINE_SPACING / 2.0 - SEDUCE_PANEL_RIM_SIZE), 0, 
										0, 0, 1,
										0.5, 0.5, 0.5, 0.7);
	pos_y -= scale * SEDUCE_PANEL_RIM_SIZE / 2.0;
	for(i = 0; i < element_count; i++)
	{

		switch(element[i].type)
		{
			case SEDUCE_PET_BOOLEAN : 
				if(seduce_element_active(input, &element[i], NULL))
				{
					float on[4] = {0.2, 0.6, 1.0, 0.9};
					float off[4] = {1.0, 0.2, 0.4, 0.9};
					if(element[i].param.active)
						seduce_widget_list_element_panel(input, pos_x, pos_y, width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), scale, &element[i], on, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
					else
						seduce_widget_list_element_panel(input, pos_x, pos_y, width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), scale, &element[i], off, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				}else
					seduce_widget_list_element_panel(input, pos_x, pos_y, width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), scale, &element[i], color, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				pos_y -= scale * (SEDUCE_PANEL_LINE_SPACING + 1.0); 
			break;
			case SEDUCE_PET_TRIGGER :
			case SEDUCE_PET_INTEGER :
			case SEDUCE_PET_UNSIGNED_INTEGER :
			case SEDUCE_PET_INTEGER_BOUND :
			case SEDUCE_PET_REAL :
			case SEDUCE_PET_REAL_BOUND :
				seduce_widget_list_element_panel(input, pos_x, pos_y, width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), scale, &element[i], color, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				pos_y -= scale * (SEDUCE_PANEL_LINE_SPACING + 1.0); 
			break;
			case SEDUCE_PET_2D_POS :
			case SEDUCE_PET_3D_POS :
			case SEDUCE_PET_4D_POS :
			case SEDUCE_PET_QUATERNION :
			case SEDUCE_PET_2D_NORMAL :
			case SEDUCE_PET_3D_NORMAL : 
			case SEDUCE_PET_2X2MATRIX : 
			case SEDUCE_PET_3X3MATRIX :
			case SEDUCE_PET_4X4MATRIX :

			seduce_background_quad_draw(input, id, 0,
										pos_x, pos_y - scale * (SEDUCE_PANEL_LINE_SPACING / 2.0), 0, 
										pos_x + width, pos_y - scale * (SEDUCE_PANEL_LINE_SPACING / 2.0), 0, 
										pos_x + width, pos_y, 0, 
										pos_x, pos_y, 0, 
										0, 0, 1,
										0.5, 0.5, 0.5, 0.7);
			pos_y -= scale * SEDUCE_PANEL_LINE_SPACING * 0.5; 

			seduce_widget_list_element_panel(input, pos_x, pos_y, width, width * 0.5, scale, &element[i], dark, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
			if(element[i].type < SEDUCE_PET_QUATERNION)
			{
				for(x = pos_x + width / 2.0 + width / 16.0; x < pos_x + width - 0.001; x += width / 16.0)
				{
					for(y = pos_y - width / 16.0; y > pos_y - width / 2.0; y -= width / 16.0)
					{
						seduce_background_quad_draw(input, id, 0,
												x - width * 0.005, y - width * 0.005, 0, 
												x + width * 0.005, y - width * 0.005, 0, 
												x + width * 0.005, y + width * 0.005, 0, 
												x - width * 0.005, y + width * 0.005, 0, 
												0, 0, 1,
												0.5, 0.5, 0.5, 0.7);
					}
				}
			}
			pos_y -= width / 2.0; 
			seduce_background_quad_draw(input, id, 0,
										pos_x, pos_y - scale * (SEDUCE_PANEL_LINE_SPACING / 2.0), 0, 
										pos_x + width, pos_y - scale * (SEDUCE_PANEL_LINE_SPACING / 2.0), 0, 
										pos_x + width, pos_y, 0, 
										pos_x, pos_y, 0, 
										0, 0, 1,
										0.5, 0.5, 0.5, 0.7);
			pos_y -= scale * SEDUCE_PANEL_LINE_SPACING * 0.5; 
			break;
			case SEDUCE_PET_TEXT :
			case SEDUCE_PET_PASSWORD :
				seduce_widget_list_element_panel(input, pos_x, pos_y, width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), scale, &element[i], color, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				pos_y -= scale * (SEDUCE_PANEL_LINE_SPACING + 1.0); 
			break;
				
			case SEDUCE_PET_TEXT_BUFFER :
			{
				STextBlockMode mode;
				mode.character_position = 0;
				mode.font = seduce_font_default_get();
				mode.red  = 1;
				mode.green = 1; 
				mode.blue = 1;
				mode.alpha = 1;
				mode.letter_size = scale / 1.6;
				mode.letter_spacing = SEDUCE_T_SPACE;
				add = scale * (SEDUCE_PANEL_LINE_SPACING + 1.0) + seduce_text_block_height(width - scale * SEDUCE_PANEL_RIM_SIZE * 2.0, SEDUCE_T_LINE_SPACEING, SEDUCE_TBAS_LEFT, element[i].param.buffer.text_buffer, 0, &mode, 1, -1);
				seduce_widget_list_element_panel(input, pos_x, pos_y, width, add, scale, &element[i], dark, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				pos_y -= add;
			}
			break;
			case SEDUCE_PET_COLOR_RGB :
			case SEDUCE_PET_COLOR_RGBA :
				seduce_widget_list_element_panel(input, pos_x, pos_y, width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), scale, &element[i], element[i].param.color, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				pos_y -= scale * (SEDUCE_PANEL_LINE_SPACING + 1.0); 
			break;
			case SEDUCE_PET_TIME :
			case SEDUCE_PET_DATE :
			case SEDUCE_PET_SELECT :
			case SEDUCE_PET_POPUP :
			case SEDUCE_PET_SECTION_START :
			case SEDUCE_PET_SECTION_END :
				seduce_widget_list_element_panel(input, pos_x, pos_y, width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), scale, &element[i], color, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				pos_y -= scale * (SEDUCE_PANEL_LINE_SPACING + 1.0); 
			break;
			case SEDUCE_PET_IMAGE :
				pos_y -= width * element[i].param.image.aspect;
				seduce_background_image_draw(input, element[i].param.image.id, pos_x, pos_y, 0, width, width * element[i].param.image.aspect, 0, 0, 1, 1, time, center, element[i].param.image.texture_id);
			break;
			case SEDUCE_PET_CUSTOM :
				
			if(element[i].param.custom.fill)
			{
				seduce_widget_list_element_panel(input, pos_x, pos_y, width, width * element[i].param.custom.aspect, scale, &element[i], color, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				pos_y -= width * element[i].param.custom.aspect;	
			}else
			{
				float f;
				f = (width - scale * SEDUCE_PANEL_RIM_SIZE * 2);
				f = f * element[i].param.custom.aspect + scale * SEDUCE_PANEL_RIM_SIZE * 2;	
				seduce_widget_list_element_panel(input, pos_x, pos_y, width, f, scale, &element[i], color, seduce_element_active(input, &element[i], NULL)  || *selected_element == i);
				pos_y -= f;	
			}
			break;

		}
	}
	seduce_background_quad_draw(input, id, 0,
										pos_x, pos_y, 0, 
										pos_x + width, pos_y, 0, 
										pos_x + width, pos_y - scale * SEDUCE_PANEL_RIM_SIZE / 2.0, 0, 
										pos_x, pos_y - scale * SEDUCE_PANEL_RIM_SIZE / 2.0, 0, 
										0, 0, 1,
										0.5, 0.5, 0.5, 0.7);
	seduce_background_polygon_flush(input, NULL, time);
	return pos_y - scale * SEDUCE_PANEL_RIM_SIZE / 2.0;
}

boolean seduce_widget_list_victor_text(BInputState *input, float pos_x, float pos_y, float width, float scale, double *vector, char **vector_names,uint vector_length)
{
	boolean output = FALSE;
	float f;
	uint i;
	for(i = 0; i < vector_length; i++)
	{
		f = seduce_text_line_draw(NULL, pos_x + scale * SEDUCE_PANEL_RIM_SIZE, pos_y - scale * (float)(i + 2) * 2.0, scale / 1.6, SEDUCE_T_SPACE, vector_names[i], 1.0, 1.0, 1.0, 1.0, -1);		
		if(seduce_text_edit_double(input, &vector[i], NULL, &vector[i], pos_x + scale * SEDUCE_PANEL_RIM_SIZE + f + scale, pos_y - scale * (float)(i + 2) * 2.0, width / 2.0 - scale * SEDUCE_PANEL_RIM_SIZE - f - scale, scale / 1.6, TRUE, NULL, NULL, 0.2, 0.6, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0))
			output = TRUE;
	}
	return output;
}
void seduce_widget_list_element_list(BInputState *input, float pos_x, float pos_y, float width, float scale, SeducePanelElement *element, uint element_count, uint *selected_element, void *id, float time)
{
	uint i, j;
	float text_length;
	pos_y -= scale * (SEDUCE_PANEL_RIM_SIZE - SEDUCE_PANEL_LINE_SPACING);
	for(i = 0; i < element_count; i++)
	{
		pos_y -= scale * (SEDUCE_PANEL_LINE_SPACING + 1.0);	

		if(element[i].type >= SEDUCE_PET_2D_POS &&  element[i].type <= SEDUCE_PET_4X4MATRIX)
			text_length = seduce_text_line_draw(NULL, pos_x + scale * SEDUCE_PANEL_RIM_SIZE, pos_y - scale * SEDUCE_PANEL_RIM_SIZE, scale / 1.6, SEDUCE_T_SPACE, element[i].text, 1.0, 1.0, 1.0, 1.0, -1) + scale + scale * SEDUCE_PANEL_RIM_SIZE;		
		else
			text_length = seduce_text_line_draw(NULL, pos_x + scale * SEDUCE_PANEL_RIM_SIZE, pos_y, scale / 1.6, SEDUCE_T_SPACE, element[i].text, 0.0, 0.0, 0.0, 1.0, -1) + scale + scale * SEDUCE_PANEL_RIM_SIZE;		

		switch(element[i].type)
		{
			case SEDUCE_PET_BOOLEAN :
				if(element[i].param.active)
				{
					if(seduce_widget_toggle_icon(input, &element[i], &element[i].param.active, SUI_3D_OBJECT_CHECKBOXCHECKED, pos_x + width - scale * (0.5 + SEDUCE_PANEL_RIM_SIZE), pos_y + scale * 0.5, scale * 2.0, time))
					{
						float off[4] = {1.0, 0.2, 0.4, 0.9};
						j = seduce_background_particle_color_allocate(NULL, off[0], off[1], off[2]);
						seduce_background_particle_square(input, pos_x, pos_y + scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), 64, j);
					}
				}else
				{
					if(seduce_widget_toggle_icon(input, &element[i], &element[i].param.active, SUI_3D_OBJECT_CHECKBOXUNCHECKED, pos_x + width - scale * (0.5 + SEDUCE_PANEL_RIM_SIZE), pos_y + scale * 0.5, scale * 2.0, time))
					{
						float on[4] = {0.2, 0.6, 1.0, 0.9};
						j = seduce_background_particle_color_allocate(NULL, on[0], on[1], on[2]);
						seduce_background_particle_square(input, pos_x, pos_y + scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), width, scale * (SEDUCE_PANEL_LINE_SPACING + 1.0), 64, j);
					}
				}

			break;
			case SEDUCE_PET_TRIGGER :
				if(seduce_widget_button_icon(input, &element[i], SUI_3D_OBJECT_CHECKBOXCHECKED, pos_x + width - scale * (0.5 + SEDUCE_PANEL_RIM_SIZE), pos_y + scale * 0.5, scale, time, NULL))
					element[i].param.trigger = TRUE;
			break;
			case SEDUCE_PET_INTEGER :
				seduce_text_edit_int(input, &element[i], NULL, &element[i].param.integer, pos_x + scale * SEDUCE_PANEL_RIM_SIZE + text_length, pos_y, width - text_length - scale * SEDUCE_PANEL_RIM_SIZE *2.0, scale / 1.6, FALSE, NULL, NULL, 0, 0, 0, 1, 0.2, 0.6, 1.0, 1.0);
			break;
			case SEDUCE_PET_UNSIGNED_INTEGER :
				seduce_text_edit_uint(input, &element[i], NULL, &element[i].param.uinteger, pos_x + scale * SEDUCE_PANEL_RIM_SIZE + text_length, pos_y, width - text_length - scale * SEDUCE_PANEL_RIM_SIZE *2.0, scale / 1.6, FALSE, NULL, NULL, 0, 0, 0, 1, 0.2, 0.6, 1.0, 1.0);
			break;
			case SEDUCE_PET_INTEGER_BOUND :
			break;
			case SEDUCE_PET_REAL :
				seduce_text_edit_double(input, &element[i], NULL, &element[i].param.real.value, pos_x + scale * SEDUCE_PANEL_RIM_SIZE + text_length, pos_y, width - text_length - scale * SEDUCE_PANEL_RIM_SIZE *2.0, scale / 1.6, FALSE, NULL, NULL, 0, 0, 0, 1, 0.2, 0.6, 1.0, 1.0);

			break;
			case SEDUCE_PET_REAL_BOUND :
			{
				float color[4] = {1, 1, 1, 1}, value; 
				value = element[i].param.real.value;
				if(seduce_widget_slider_radial(input, &element[i], &value, pos_x + width - scale * (0.5 + SEDUCE_PANEL_RIM_SIZE), pos_y + scale * 0.5, scale * 2.0, 2, 0, 1, time, color))
					element[i].param.real.value = value;
			}
			break;
			case SEDUCE_PET_2D_POS :
			case SEDUCE_PET_3D_POS :
			case SEDUCE_PET_4D_POS :
			{
				char *vector_names[4] = {"X:", "Y:", "Z:", "W:"};
				float pos[4] = {0, 0, 0, 0}, f = 0, scaling = 1.0;
				uint axis_count;
				axis_count = 2 + element[i].type - SEDUCE_PET_2D_POS;
				seduce_widget_list_victor_text(input, pos_x, pos_y, width, scale, element[i].param.vector, vector_names, axis_count);
				for(j = 0; j < axis_count; j++)
				{
					pos[j] = element[i].param.vector[j];
					if(element[i].param.vector[j] > f)
						f = element[i].param.vector[0];
					if(-element[i].param.vector[j] > f)
						f = -element[i].param.vector[j];
				}
				for(scaling = 1.0; scaling < f - scaling * 0.1; scaling *= 10.0);

				r_matrix_push(NULL);
				r_matrix_translate(NULL, pos_x + width * 3.0 / 4.0, pos_y - width * 1.0 / 4.0, 0);
				r_matrix_scale(NULL, width / 4.0 / scaling, width / 4.0 / scaling, width / 4.0 / scaling);
				if((axis_count == 2 && seduce_manipulator_point_plane(input, NULL, pos, &element[i], NULL, FALSE, 2, scale / 0.0125)) ||
					(axis_count != 2 && seduce_manipulator_pos_xyz(input, NULL, pos, &element[i], NULL, FALSE, TRUE, TRUE, TRUE, scale, time)))		
				{
					for(j = 0; j < axis_count; j++)
					{			
						if(pos[j] > scaling)
							pos[j] = scaling;
						if(pos[j] < -scaling)
							pos[j] = -scaling;
					}
					element[i].param.vector[0] = pos[0];
					element[i].param.vector[1] = pos[1];
				}
				r_matrix_pop(NULL);
			}
			pos_y -= width * 0.5 - scale;	
			break;
			case SEDUCE_PET_QUATERNION :
			pos_y -= width * 0.5 - scale;
			break;
			case SEDUCE_PET_2D_NORMAL :
			{
				char *vector_names[2] = {"X:", "Y:"};
				float pos[3];
				if(seduce_widget_list_victor_text(input, pos_x, pos_y, width, scale, element[i].param.vector, vector_names, 2))
					f_normalize2d(element[i].param.vector);
				pos[0] = element[i].param.vector[0];
				pos[1] = element[i].param.vector[1];
				pos[2] = 0;
				r_matrix_push(NULL);
				r_matrix_translate(NULL, pos_x + width * 3.0 / 4.0, pos_y - width * 1.0 / 4.0, 0);
				r_matrix_scale(NULL, width / 4.0, width / 4.0, width / 4.0);
				if(seduce_manipulator_point_plane(input, NULL, pos, &element[i], NULL, FALSE, 2, scale / 0.0125))
				{
					f_normalize2d(element[i].param.vector);
					element[i].param.vector[0] = pos[0];
					element[i].param.vector[1] = pos[1];
				}
				r_matrix_pop(NULL);
			}
			pos_y -= width * 0.5 - scale;
			break;
			case SEDUCE_PET_3D_NORMAL :
			{
				char *vector_names[3] = {"X:", "Y:", "Z:"};
				float pos[3], normal[3];
				seduce_widget_list_victor_text(input, pos_x, pos_y, width, scale, element[i].param.vector, vector_names, 3);
				pos[0] = pos_x + width * 3.0 / 4.0;
				pos[1] = pos_y + width * 1.0 / 4.0;
				pos[2] = 0;
				normal[0] = element[i].param.vector[0];
				normal[1] = element[i].param.vector[1];
				normal[2] = 0;
				if(seduce_manipulator_normal(input, NULL, pos, normal, &element[i], width / 2.0, time))
				{
					element[i].param.vector[0] = normal[0];
					element[i].param.vector[1] = normal[1];
					element[i].param.vector[2] = normal[1];
				}
			}
			pos_y -= width * 0.5 - scale;	
			break;
			case SEDUCE_PET_2X2MATRIX :
			pos_y -= width * 0.5 - scale;	
			break;
			case SEDUCE_PET_3X3MATRIX :
			pos_y -= width * 0.5 - scale;	
			break;
			case SEDUCE_PET_4X4MATRIX :
			pos_y -= width * 0.5 - scale;	
			break;
			case SEDUCE_PET_TEXT :
			//	seduce_text_edit_int(input, &element[i], NULL, &element[i].param.integer, pos_x + scale * SEDUCE_PANEL_RIM_SIZE + text_length, pos_y, width - text_length - scale * SEDUCE_PANEL_RIM_SIZE *2.0, scale / 1.6, FALSE, NULL, NULL, 0, 0, 0, 1, 0.2, 0.6, 1.0, 1.0);
				seduce_text_edit_line(input, &element[i], NULL, element[i].param.text, 64, pos_x + scale * SEDUCE_PANEL_RIM_SIZE + text_length, pos_y, width - text_length - scale * SEDUCE_PANEL_RIM_SIZE *2.0, scale / 1.6, "", FALSE, NULL, NULL, 0, 0, 0, 1, 0.2, 0.6, 1.0, 1.0);

			break;
			case SEDUCE_PET_PASSWORD :
			break;
			case SEDUCE_PET_TEXT_BUFFER :
			{
				STextBlockMode mode;
				STextBox box;
				mode.character_position = 0;
				mode.font = seduce_font_default_get();
				mode.red  = 1;
				mode.green = 1; 
				mode.blue = 1;
				mode.alpha = 1;
				mode.letter_size = scale / 1.6;
				mode.letter_spacing = SEDUCE_T_SPACE;

				

				box.pos_x = pos_x + scale * SEDUCE_PANEL_RIM_SIZE;
				box.pos_y = pos_y;
				box.line_size = width - scale * SEDUCE_PANEL_RIM_SIZE * 2.0;
				box.height = seduce_text_block_height(width - scale * SEDUCE_PANEL_RIM_SIZE * 2.0, SEDUCE_T_LINE_SPACEING, SEDUCE_TBAS_LEFT, element[i].param.buffer.text_buffer, 0, &mode, 1, -1);
				box.line_spacing = SEDUCE_T_LINE_SPACEING;
				box.style = SEDUCE_TBAS_LEFT;
				seduce_text_box_edit(input, &element[i], element[i].param.buffer.text_buffer, element[i].param.buffer.buffer_size, &box, 1, &mode, 1);
				pos_y -= box.height;
			}
			break;
			case SEDUCE_PET_COLOR_RGB :
			{
				float color[4 * 3] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1}, hsv[3]; 
				char *ids;
				ids = (char *)element[i].param.color;
				color[0] = element[i].param.color[0];
				color[5] = element[i].param.color[1];
				color[10] = element[i].param.color[2];
				seduce_widget_slider_radial(input, &ids[0], &element[i].param.color[0], pos_x + width - SEDUCE_T_SIZE * 15.0, pos_y + SEDUCE_T_SIZE, scale * 2.0, 1, 0, 1, time, &color[0]);
				seduce_widget_slider_radial(input, &ids[1], &element[i].param.color[1], pos_x + width - SEDUCE_T_SIZE * 9.0, pos_y + SEDUCE_T_SIZE, scale * 2.0, 1, 0, 1, time, &color[1]);
				seduce_widget_slider_radial(input, &ids[2], &element[i].param.color[2], pos_x + width - SEDUCE_T_SIZE * 3.0, pos_y + SEDUCE_T_SIZE, scale * 2.0, 1, 0, 1, time, &color[2]);

			/*	seduce_widget_wheel_radial(input, &ids[3], &element[i].param.color[0], pos_x + width - SEDUCE_T_SIZE * 15.0, pos_y + SEDUCE_T_SIZE * -5.0, SEDUCE_T_SIZE * 4.5, scale * 0.5, time);
				f_rgb_to_hsv(hsv, element[i].param.color[0], element[i].param.color[1], element[i].param.color[2]);
				if(seduce_widget_slider_radial(input, &ids[4], &hsv[1], pos_x + width - SEDUCE_T_SIZE * 9.0, pos_y + SEDUCE_T_SIZE * -5.0, SEDUCE_T_SIZE * 4.5, scale * 0.5, 0, 1, time, element[i].param.color))		
					f_hsv_to_rgb(element[i].param.color, hsv[0], hsv[1], hsv[2]);
				f_rgb_to_hsv(hsv, element[i].param.color[0], element[i].param.color[1], element[i].param.color[2]);
				if(seduce_widget_slider_radial(input, &ids[5], &hsv[2], pos_x + width - SEDUCE_T_SIZE * 3.0, pos_y + SEDUCE_T_SIZE * -5.0, SEDUCE_T_SIZE * 4.5, scale * 0.5, 0, 1, time, element[i].param.color))		
					f_hsv_to_rgb(element[i].param.color, hsv[0], hsv[1], hsv[2]);	*/
			}
			break;
			case SEDUCE_PET_COLOR_RGBA :
			{
				float color[4 * 4] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1}, hsv[3]; 
				char *ids;
				ids = (char *)element[i].param.color;
				color[0] = element[i].param.color[0];
				color[5] = element[i].param.color[1];
				color[10] = element[i].param.color[2];
				color[12] = color[13] = color[14] = element[i].param.color[3];
				seduce_widget_slider_radial(input, &ids[0], &element[i].param.color[0], pos_x + width - SEDUCE_T_SIZE * 21.0, pos_y + SEDUCE_T_SIZE, scale * 2.0, 1, 0, 1, time, &color[0]);
				seduce_widget_slider_radial(input, &ids[1], &element[i].param.color[1], pos_x + width - SEDUCE_T_SIZE * 15.0, pos_y + SEDUCE_T_SIZE, scale * 2.0, 0.5, 0, 1, time, &color[4]);
				seduce_widget_slider_radial(input, &ids[2], &element[i].param.color[2], pos_x + width - SEDUCE_T_SIZE * 9.0, pos_y + SEDUCE_T_SIZE, scale * 2.0, 1, 0, 1, time, &color[8]);
				seduce_widget_slider_radial(input, &ids[3], &element[i].param.color[3], pos_x + width - SEDUCE_T_SIZE * 3.0, pos_y + SEDUCE_T_SIZE, scale * 2.0, 1, 0, 1, time, &color[12]);

			/*	seduce_widget_wheel_radial(input, &ids[3], &element[i].param.color[0], pos_x + width - SEDUCE_T_SIZE * 15.0, pos_y + SEDUCE_T_SIZE * -5.0, SEDUCE_T_SIZE * 4.5, scale * 0.5, time);
				f_rgb_to_hsv(hsv, element[i].param.color[0], element[i].param.color[1], element[i].param.color[2]);
				if(seduce_widget_slider_radial(input, &ids[4], &hsv[1], pos_x + width - SEDUCE_T_SIZE * 9.0, pos_y + SEDUCE_T_SIZE * -5.0, SEDUCE_T_SIZE * 4.5, scale * 0.5, 0, 1, time, element[i].param.color))		
					f_hsv_to_rgb(element[i].param.color, hsv[0], hsv[1], hsv[2]);
				f_rgb_to_hsv(hsv, element[i].param.color[0], element[i].param.color[1], element[i].param.color[2]);
				if(seduce_widget_slider_radial(input, &ids[5], &hsv[2], pos_x + width - SEDUCE_T_SIZE * 3.0, pos_y + SEDUCE_T_SIZE * -5.0, SEDUCE_T_SIZE * 4.5, scale * 0.5, 0, 1, time, element[i].param.color))		
					f_hsv_to_rgb(element[i].param.color, hsv[0], hsv[1], hsv[2]);	*/
			}

			break;
			case SEDUCE_PET_TIME :
			break;
			case SEDUCE_PET_DATE :
			break;
			case SEDUCE_PET_SELECT :

				seduce_text_line_draw(NULL, pos_x + width - 2.0 * scale * SEDUCE_PANEL_RIM_SIZE - seduce_text_line_length(NULL, scale / 1.6, SEDUCE_T_SPACE, element[i].param.select.text[element[i].param.select.active], -1), 
					pos_y, scale / 1.6, SEDUCE_T_SPACE, element[i].param.select.text[element[i].param.select.active], 0.0, 0.0, 0.0, 1.0, -1)/* + scale + scale * SEDUCE_PANEL_RIM_SIZE*/;		


				seduce_widget_select_radial(input, &element[i], &element[i].param.select.active, element[i].param.select.text, element[i].param.select.count, S_PUT_ANGLE, 
					pos_x + width - scale * (0.5 + SEDUCE_PANEL_RIM_SIZE), 
					pos_y + scale * 0.5, 
					scale * 2.0, scale * 2.5, 
					time, FALSE);
			break;
			case SEDUCE_PET_POPUP :
				seduce_popup_detect_icon(input, &element[i], element[i].param.popup.icon, 
										pos_x + width - scale * (0.5 + SEDUCE_PANEL_RIM_SIZE), 
										pos_y + scale * 0.5, 
										scale * 2.0,
										time, element[i].param.popup.func, element[i].param.popup.user, element[i].param.popup.displace, NULL);
			break;
			case SEDUCE_PET_IMAGE :
				pos_y += scale * (SEDUCE_PANEL_LINE_SPACING + 1.0);	
				pos_y -= width;
			break;
			case SEDUCE_PET_SECTION_START :
			break;
			case SEDUCE_PET_SECTION_END :
			break;
			case SEDUCE_PET_CUSTOM :
				if(element[i].param.custom.fill)
				{
					pos_y += scale * (SEDUCE_PANEL_LINE_SPACING);	
					element[i].param.custom.function(input, pos_x, pos_y, width, element[i].param.custom.user);
					pos_y -= width * element[i].param.custom.aspect;	
				}else
				{
					float f;

					f = (width - scale * SEDUCE_PANEL_RIM_SIZE * 2);
					
					pos_y += scale * (SEDUCE_PANEL_LINE_SPACING);
					element[i].param.custom.function(input, pos_x + scale * SEDUCE_PANEL_RIM_SIZE, pos_y - scale * SEDUCE_PANEL_RIM_SIZE, f, element[i].param.custom.user);
					pos_y += scale;	
					pos_y -= f * element[i].param.custom.aspect + scale * SEDUCE_PANEL_RIM_SIZE * 2;	
				}
			break;

		}
	}
}

/*
typedef enum{
	SEDUCE_PET_BOOLEAN,
	SEDUCE_PET_TRIGGER,
	SEDUCE_PET_INTEGER,
	SEDUCE_PET_UNSIGNED_INTEGER,
	SEDUCE_PET_INTEGER_BOUND,
	SEDUCE_PET_REAL,
	SEDUCE_PET_REAL_BOUND,
	SEDUCE_PET_2D_POS,
	SEDUCE_PET_3D_POS,
	SEDUCE_PET_4D_POS,
	SEDUCE_PET_QUATERNION,
	SEDUCE_PET_2D_NORMAL,
	SEDUCE_PET_3D_NORMAL,
	SEDUCE_PET_2X2MATRIX,
	SEDUCE_PET_3X3MATRIX,
	SEDUCE_PET_4X4MATRIX,
	SEDUCE_PET_TEXT,
	SEDUCE_PET_PASSWORD,
	SEDUCE_PET_TEXT_BUFFER,
	SEDUCE_PET_COLOR_RGB,
	SEDUCE_PET_COLOR_RGBA,
	SEDUCE_PET_TIME,
	SEDUCE_PET_DATE,
	SEDUCE_PET_SELECT,
	SEDUCE_PET_SECTION_START,
	SEDUCE_PET_SECTION_END,
	SEDUCE_PET_CUSTOM,
	SEDUCE_PET_COUNT
}ForgeDataElementType;

typedef struct{
	ForgeDataElementType type;
	char *text;
	char *description;
	union{
		boolean active; 
		boolean trigger;
		struct{
			int	value;
			int max;
			int min;
		}integer;
		uint	uinteger;
		struct{
			double value;
			double max;
			double min;
		}real;
		float color[4];
		double vector[4];
		double matrix[16];
		char text[64];
		struct{
			char	*text_buffer;
			uint	buffer_size;
		}buffer;
		double time;
		struct{
			uint16	year;
			uint8	month;
			uint8	day;
		}date;
		struct{
			char	**text;
			uint	count;
			uint	active;
		}select;
	}param;
}ForgeDataElement;*/

void seduce_widget_list_element_test(BInputState *input, float time)
{
	static SeducePanelElement element[SEDUCE_PET_COUNT];
	static uint init = FALSE, selected = -1;
	static float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	return;
	if(!init)
	{
		init = TRUE;
		element[SEDUCE_PET_BOOLEAN].type = SEDUCE_PET_BOOLEAN;
		element[SEDUCE_PET_BOOLEAN].text = "BOOLEAN"; 
		element[SEDUCE_PET_BOOLEAN].param.active = TRUE;
		element[SEDUCE_PET_TRIGGER].type = SEDUCE_PET_TRIGGER;
		element[SEDUCE_PET_TRIGGER].text = "TRIGGER"; 
		element[SEDUCE_PET_TRIGGER].param.trigger = TRUE;
		element[SEDUCE_PET_INTEGER].type = SEDUCE_PET_INTEGER;
		element[SEDUCE_PET_INTEGER].text = "INTEGER"; 
		element[SEDUCE_PET_INTEGER].param.integer = 1;
		element[SEDUCE_PET_UNSIGNED_INTEGER].type = SEDUCE_PET_UNSIGNED_INTEGER;
		element[SEDUCE_PET_UNSIGNED_INTEGER].text = "UNSIGNED INTEGER";
		element[SEDUCE_PET_UNSIGNED_INTEGER].param.uinteger = 1;
		element[SEDUCE_PET_INTEGER_BOUND].type = SEDUCE_PET_INTEGER_BOUND;
		element[SEDUCE_PET_INTEGER_BOUND].text = "INTEGER BOUND";
	//	element[SEDUCE_PET_INTEGER_BOUND].param.value = 01;
		element[SEDUCE_PET_REAL].type = SEDUCE_PET_REAL;
		element[SEDUCE_PET_REAL].text = "REAL";
		element[SEDUCE_PET_REAL].param.real.value = 0.5;
		element[SEDUCE_PET_REAL_BOUND].type = SEDUCE_PET_REAL_BOUND;
		element[SEDUCE_PET_REAL_BOUND].text = "REAL BOUND"; 
		element[SEDUCE_PET_REAL].param.real.value = 0.5;
		element[SEDUCE_PET_REAL].param.real.min = 0.0;
		element[SEDUCE_PET_REAL].param.real.max = 1.0;
		element[SEDUCE_PET_2D_POS].type = SEDUCE_PET_2D_POS;
		element[SEDUCE_PET_2D_POS].text = "2D POS"; 
		element[SEDUCE_PET_3D_POS].type = SEDUCE_PET_3D_POS;
		element[SEDUCE_PET_3D_POS].text = "3D POS"; 
		element[SEDUCE_PET_4D_POS].type = SEDUCE_PET_4D_POS;
		element[SEDUCE_PET_4D_POS].text = "4D POS"; 
		element[SEDUCE_PET_QUATERNION].type = SEDUCE_PET_QUATERNION;
		element[SEDUCE_PET_QUATERNION].text = "QUATERNION"; 
		element[SEDUCE_PET_2D_NORMAL].type = SEDUCE_PET_2D_NORMAL;
		element[SEDUCE_PET_2D_NORMAL].text = "2D NORMAL"; 
		element[SEDUCE_PET_3D_NORMAL].type = SEDUCE_PET_3D_NORMAL;
		element[SEDUCE_PET_3D_NORMAL].text = "3D NORMAL"; 
		element[SEDUCE_PET_2X2MATRIX].type = SEDUCE_PET_2X2MATRIX;
		element[SEDUCE_PET_2X2MATRIX].text = "2X2MATRIX"; 
		element[SEDUCE_PET_3X3MATRIX].type = SEDUCE_PET_3X3MATRIX;
		element[SEDUCE_PET_3X3MATRIX].text = "3X3MATRIX"; 
		element[SEDUCE_PET_4X4MATRIX].type = SEDUCE_PET_4X4MATRIX;
		element[SEDUCE_PET_4X4MATRIX].text = "4X4MATRIX"; 
		element[SEDUCE_PET_TEXT].type = SEDUCE_PET_TEXT;
		element[SEDUCE_PET_TEXT].text = "TEXT"; 
		element[SEDUCE_PET_PASSWORD].type = SEDUCE_PET_PASSWORD;
		element[SEDUCE_PET_PASSWORD].text = "PASSWORD"; 
		element[SEDUCE_PET_TEXT_BUFFER].type = SEDUCE_PET_TEXT_BUFFER;
		element[SEDUCE_PET_TEXT_BUFFER].text = "TEXT BUFFER"; 
		element[SEDUCE_PET_COLOR_RGB].type = SEDUCE_PET_COLOR_RGB;
		element[SEDUCE_PET_COLOR_RGB].text = "COLOR RGB"; 
		element[SEDUCE_PET_COLOR_RGBA].type = SEDUCE_PET_COLOR_RGBA;
		element[SEDUCE_PET_COLOR_RGBA].text = "COLOR RGBA"; 
		element[SEDUCE_PET_TIME].type = SEDUCE_PET_TIME;
		element[SEDUCE_PET_TIME].text = "TIME"; 
		element[SEDUCE_PET_DATE].type = SEDUCE_PET_DATE;
		element[SEDUCE_PET_DATE].text = "DATE"; 
		element[SEDUCE_PET_SELECT].type = SEDUCE_PET_SELECT;
		element[SEDUCE_PET_SELECT].text = "SELECT"; 
		element[SEDUCE_PET_SECTION_START].type = SEDUCE_PET_SECTION_START;
		element[SEDUCE_PET_SECTION_START].text = "SECTION START"; 
		element[SEDUCE_PET_SECTION_END].type = SEDUCE_PET_SECTION_END;
		element[SEDUCE_PET_SECTION_END].text = "SECTION END";
	}
	seduce_background_shape_matrix_interact(input, &init, matrix, TRUE, TRUE);
	r_matrix_push(NULL);
	r_matrix_matrix_mult(NULL, matrix);
	seduce_widget_list_element_background(input, 0.2, 0.5, 0.4, SEDUCE_T_SIZE * 2.0, element, SEDUCE_PET_SECTION_END + 1, &selected, &init, time);
	seduce_widget_list_element_list(input, 0.2, 0.5, 0.4, SEDUCE_T_SIZE * 2.0, element, SEDUCE_PET_SECTION_END + 1, &selected, &init, time);
	


	r_matrix_pop(NULL);

}
/*

float sui_widget_list_element_length(float length, float scale, SUIViewElement *element)
{
	switch(element->type)
	{
		case S_VET_LABEL :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_BOOLEAN :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_INTEGER :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_UNSIGNED_INTEGER :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_REAL :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_SLIDER :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_RADIAL :
			if(0.05 * scale < length * 0.5)
				return 0.1 * scale;
			else
				return 0.5 * length;
		case S_VET_TEXT :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_PASSWORD :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_LIST :
			return scale * SEDUCE_T_SIZE * 3.0 * element->param.list.length + scale * SEDUCE_T_SIZE * 1.0;
		case S_VET_COLOR_RGB :
			return scale * SEDUCE_T_SIZE * 12.0;
		case S_VET_COLOR_SLICES :
			return scale * SEDUCE_T_SIZE * 12.0;
		case S_VET_COLOR_WHEEL :
			return length;
		case S_VET_COLOR_HUE :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_COLOR_SATURATION :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_COLOR_VALUE :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_SELECT :
		case S_VET_CUSTOM :
		case S_VET_SECTION_START :
			return scale * SEDUCE_T_SIZE * 4.0;
		case S_VET_SECTION_END :
			return 0.0;
	}
	return 0;
}

void sui_widget_list(BInputState *input, float pos_x, float pos_y, float length, float scale, SUIViewElement *element, uint element_count, void *id, float time, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha)
{
	float hsv[3], low[3], high[3], f, color[3], *c;
	uint i, j;
	char *id_array;
	id_array = id;
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	pos_y -= scale * SEDUCE_T_SIZE * 2.0;
	for(i = 0; i < element_count; i++)
	{
		switch(element[i].type)
		{
			case S_VET_LABEL :
		//		seduce_text_line_draw(NULL, pos_x + length * 0.5 - seduce_text_line_length(NULL, SEDUCE_T_SIZE * 0.5, SEDUCE_T_SPACE, element[i].text, -1), pos_y, SEDUCE_T_SIZE, SEDUCE_T_SPACE, element[i].text, red, green, blue, alpha, -1);
			break;
			case S_VET_BOOLEAN :
				s_widget_boolean(input, pos_x, pos_y, length, scale, element[i].text, &element[i].param.checkbox, time, red, green, blue);
			break;
			case S_VET_INTEGER :
				sui_text_int_edit(input, pos_x, pos_y - 2.0 * SEDUCE_T_SIZE * scale, length, SEDUCE_T_SIZE * scale, &element[i].param.integer, &id_array[i], NULL, NULL, red, green, blue, alpha, active_red, active_green, active_blue, active_alpha);
			break;
			case S_VET_UNSIGNED_INTEGER :
				sui_text_uint_edit(input, pos_x, pos_y - 2.0 * SEDUCE_T_SIZE * scale, length, SEDUCE_T_SIZE * scale, &element[i].param.uinteger, &id_array[i], NULL, NULL, red, green, blue, alpha, active_red, active_green, active_blue, active_alpha);
			break;
			case S_VET_REAL :
				sui_text_double_edit(input, pos_x, pos_y - 2.0 * SEDUCE_T_SIZE * scale, length, SEDUCE_T_SIZE * scale, &element[i].param.real, &id_array[i], NULL, NULL, red, green, blue, alpha, active_red, active_green, active_blue, active_alpha);
			break;
			case S_VET_SLIDER :
				s_widget_slider_new(input, pos_x + 2.0 * SEDUCE_T_SIZE * scale, pos_y, pos_x + length - 2.0 * SEDUCE_T_SIZE * scale, pos_y, scale, time, &element[i].param.slider, &id_array[i + 1501], color, color, low, high);
			break;
			case S_VET_RADIAL :
				if(0.05 * scale < length * 0.5)
					seduce_widget_slider_radial(input, pos_x + length * 0.5, pos_y - 0.05 * scale + scale * SEDUCE_T_SIZE * 2.0, 0.05 * scale, scale, time, &element[i].param.slider, &id_array[i + 1501], red, green, blue, active_red, active_green, active_blue);
				else
					seduce_widget_slider_radial(input, pos_x + length * 0.5, pos_y - 0.5 * length + scale * SEDUCE_T_SIZE * 2.0, 0.5 * length, scale, time, &element[i].param.slider, &id_array[i + 1501], red, green, blue, active_red, active_green, active_blue);
			break;
			case S_VET_TEXT :
				seduce_text_edit_line(input, void *id, pos_x, pos_y - 2.0 * SEDUCE_T_SIZE * scale, length, scale * SEDUCE_T_SIZE, element[i].param.text.text, element[i].param.text.length, NULL, NULL, red, green, blue, alpha, active_red, active_green, active_blue, active_alpha);
			break;
			case S_VET_PASSWORD :
				sui_text_password_edit(input, pos_x, pos_y - 2.0 * SEDUCE_T_SIZE * scale, length, scale * SEDUCE_T_SIZE, element[i].param.password.text, element[i].param.password.length, NULL, NULL, red, green, blue, alpha, active_red, active_green, active_blue, active_alpha);
			break;
			case S_VET_LIST :
				for(j = 0; j < element[i].param.list.length; j++)
				{
					if(input->mode == BAM_DRAW)
					{
						if(sui_pointer_box_test(input, pos_x, pos_y - scale * SEDUCE_T_SIZE * 1.0, length, scale * SEDUCE_T_SIZE * 3.0))
						{
							if(element[i].param.list.active[j])
								seduce_text_line_draw(NULL, pos_x + scale * SEDUCE_T_SIZE * 4.0, pos_y - scale * SEDUCE_T_SIZE * 3.0 * (float)j, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, element[i].param.list.text[j], active_red, active_green, active_blue, active_alpha * 0.6, -1);
							else
								seduce_text_line_draw(NULL, pos_x + scale * SEDUCE_T_SIZE * 4.0, pos_y - scale * SEDUCE_T_SIZE * 3.0 * (float)j, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, element[i].param.list.text[j], red, green, blue, alpha * 0.6, -1);
						}else
						{
							if(element[i].param.list.active[j])
								seduce_text_line_draw(NULL, pos_x + scale * SEDUCE_T_SIZE * 4.0, pos_y - scale * SEDUCE_T_SIZE * 3.0 * (float)j, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, element[i].param.list.text[j], active_red, active_green, active_blue, active_alpha, -1);
							else
								seduce_text_line_draw(NULL, pos_x + scale * SEDUCE_T_SIZE * 4.0, pos_y - scale * SEDUCE_T_SIZE * 3.0 * (float)j, scale * SEDUCE_T_SIZE, SEDUCE_T_SPACE, element[i].param.list.text[j], red, green, blue, alpha, -1);
						}
					}else if(input->mode == BAM_EVENT)
						if(-1 != sui_pointer_box_click_test(input, pos_x, 0, pos_y - scale * SEDUCE_T_SIZE * 1.0, length, scale * SEDUCE_T_SIZE * 3.0))
							element[i].param.list.active[j] = !element[i].param.list.active[j];
				}
			break;
			case S_VET_COLOR_RGB :
				c = element[i].param.color;
				low[1] = high[1] = c[1];
				low[2] = high[2] = c[2];
				low[0] = 0;
				high[0] = 1;
				s_widget_slider_new(input, pos_x, pos_y, pos_x + length, pos_y, scale, time, &c[0], &id_array[i + 1501], color, color, low, high);
				low[0] = high[0] = c[0];
				low[1] = 0;
				high[1] = 1;
				s_widget_slider_new(input, pos_x, pos_y - scale * SEDUCE_T_SIZE * 4.0, pos_x + length, pos_y - scale * SEDUCE_T_SIZE * 4.0, scale, time, &c[1], &id_array[i + 1502], color, color, low, high);
				low[1] = high[1] = c[1];
				low[2] = 0;
				high[2] = 1;
				s_widget_slider_new(input, pos_x, pos_y - scale * SEDUCE_T_SIZE * 8.0, pos_x + length, pos_y - scale * SEDUCE_T_SIZE * 8.0, scale, time, &c[2], &id_array[i + 1503], color, color, low, high);
			break;
			case S_VET_COLOR_SLICES :
				s_widget_slider(input, pos_x, pos_y, length, scale, "slice Red", &element[i].param.color[0], &id_array[i + 5000], time, red, green, blue);
				s_widget_slider(input, pos_x, pos_y - scale * SEDUCE_T_SIZE * 4.0, length, scale, "slice Green", &element[i].param.color[1], &id_array[i + 10000], time, red, green, blue);
				s_widget_slider(input, pos_x, pos_y - scale * SEDUCE_T_SIZE * 8.0, length, scale, "slice Blue", &element[i].param.color[2], &id_array[i + 15000], time, red, green, blue);
			break;
			case S_VET_COLOR_WHEEL :
				s_widget_wheel(input, pos_x + length * 0.5, pos_y - length * 0.5 + scale * SEDUCE_T_SIZE * 2.0, length, time, element[i].param.color);
			//	pos_y -= length;
			break;
			case S_VET_COLOR_HUE :
				f_rgb_to_hsv(hsv, element[i].param.color[0], element[i].param.color[1], element[i].param.color[2]);
				s_widget_slider(input, pos_x, pos_y, length, scale, "Hue", hsv, &id_array[i + 15000], time, red, green, blue);
				f_hsv_to_rgb(element[i].param.color, hsv[0], hsv[1], hsv[2]);
			//	pos_y -= scale * SEDUCE_T_SIZE * 4.0;
			break;
			case S_VET_COLOR_SATURATION :
				f_rgb_to_hsv(hsv, element[i].param.color[0], element[i].param.color[1], element[i].param.color[2]);
				f_hsv_to_rgb(low, hsv[0], 0, hsv[2]);
				f_hsv_to_rgb(high, hsv[0], 1, hsv[2]);
				s_widget_slider_new(input, pos_x, pos_y, pos_x + length, pos_y, scale, time, &hsv[1], &id_array[i + 15000], color, color, low, high);
				f_hsv_to_rgb(element[i].param.color, hsv[0], hsv[1], hsv[2]);
			//	pos_y -= scale * SEDUCE_T_SIZE * 4.0;
			break;
			case S_VET_COLOR_VALUE :
				f_rgb_to_hsv(hsv, element[i].param.color[0], element[i].param.color[1], element[i].param.color[2]);
				f_hsv_to_rgb(low, hsv[0], hsv[1], 0);
				f_hsv_to_rgb(high, hsv[0], hsv[1], 1);
				s_widget_slider_new(input, pos_x, pos_y, pos_x + length, pos_y, scale, time, &hsv[2], &id_array[i + 15000], color, color, low, high);
				f_hsv_to_rgb(element[i].param.color, hsv[0], hsv[1], hsv[2]);
			//	pos_y -= scale * SEDUCE_T_SIZE * 4.0;
			break;
			case S_VET_SELECT :
				s_widget_select(input, pos_x, pos_y, length, scale, element[i].text, &element[i].param.select.select, element[i].param.select.count, element[i].param.select.text, &id_array[i], time, red, green, blue, red, green, blue);
			break;
			case S_VET_CUSTOM :
			break;
			case S_VET_SECTION_START :
				if(element[i].param.sections.timer > 0.001)
				{
					uint level = 1;
					for(j =	i + 1; j < element_count && level != 0; j++)
					{
						if(element[j].type == S_VET_SECTION_START)
							level++;
						if(element[j].type == S_VET_SECTION_END)
							level--;
					}
sui_widget_list(input, pos_x + scale * SEDUCE_T_SIZE * 4.0, pos_y, length - scale * SEDUCE_T_SIZE * 4.0, scale, &element[i + 1], j - (i + 1), id, time, red, green, blue, alpha, active_red, active_green, active_blue, active_alpha);
					i = j;
				}
			case S_VET_SECTION_END :
				return;
			break;
		}
		pos_y -= sui_widget_list_element_length(length, scale, &element[i]);
	}
}

void sui_widget_panel(BInputState *input, float pos_x, float pos_y, float length, float scale, SUIViewElement *element, uint element_count, void *id, float time, float red, float green, float blue, float alpha, float active_red, float active_green, float active_blue, float active_alpha)
{
	float  f, x = 0, y = 0, t = 0.0, split;
	uint i;

	time = time * 2.0;
	t = time - 1.0;
	if(t < 0.0)
		t = 0.0;
	if(time > 1.0)
		time = 1.0;

	x = 0;
	for(i = 0; i < element_count; i++)
	{
		f = seduce_text_line_length(NULL, SEDUCE_T_SIZE * scale, SEDUCE_T_SPACE, element[i].text, -1);
		if(f > x)
			x = f;
	}
	x += SEDUCE_T_SIZE * 4 * scale;
	split = x / length;

	if(input->mode == BAM_DRAW)
	{
		y = SEDUCE_T_SIZE * 4 * scale;
		for(i = 0; i < element_count; i++)
			y += sui_widget_list_element_length(length - x, scale, &element[i]);
		seduce_background_square_draw(input, NULL, pos_x, pos_y - y, length, y, split, 0.1, time);

		x -= SEDUCE_T_SIZE * 2 * scale - pos_x;
		for(i = 0; i < element_count; i++)	
		{
seduce_text_line_draw(NULL, x - seduce_text_line_length(NULL, SEDUCE_T_SIZE * scale, SEDUCE_T_SPACE, element[i].text, -1), 
				   pos_y - (float)i * 4.0 * SEDUCE_T_SIZE * scale - 4.5 * SEDUCE_T_SIZE * scale, SEDUCE_T_SIZE * scale, SEDUCE_T_SPACE, element[i].text, red, green, blue, t, -1);
					

		}

	}
	sui_widget_list(input, pos_x + length * split, pos_y, length * (1.0 - split), scale, element, element_count, id, time, red, green, blue, alpha, active_red, active_green, active_blue, active_alpha);
}

*/
/*
typedef enum{
	BETRAY_ST_TOGGLE,
	BETRAY_ST_SELECT,
	BETRAY_ST_NUMBER_FLOAT,
	BETRAY_ST_NUMBER_INT,
	BETRAY_ST_SLIDER,
	BETRAY_ST_2D,
	BETRAY_ST_3D,
	BETRAY_ST_COLOR,
	BETRAY_ST_4X4_MATRIX,
}BSettingType;

#ifndef BETRAY_PLUGGIN_DEFINES

extern uint		 betray_settings_count();
extern BSettingType betray_settings_type(uint id);
extern char		*betray_settings_name(uint id);

extern boolean	betray_settings_toggle_get(uint id);
extern void		betray_settings_toggle_set(uint id, boolean	toggle);

extern uint		betray_settings_select_get(uint id);
extern void		betray_settings_select_set(uint id, uint select);
extern uint		betray_settings_select_count_get(uint id);
extern char		*betray_settings_select_name_get(uint id, uint option);

extern float	betray_settings_number_float_get(uint id);
extern void		betray_settings_number_float_set(uint id, float number);

extern int		betray_settings_number_int_get(uint id);
extern void		betray_settings_number_int_set(uint id, int number);

extern float	betray_settings_slider_get(uint id);
extern void		betray_settings_slider_set(uint id, float slider);

extern void		betray_settings_2d_get(uint id, float *x, float *y);
extern void		betray_settings_2d_set(uint id, float x, float y);

extern void		betray_settings_3d_get(uint id, float *x, float *y, float *z);
extern void		betray_settings_3d_set(uint id, float x, float y, float z);

extern void		betray_settings_color_get(uint id, float *red, float *green, float *blue);
extern void		betray_settings_color_set(uint id, float red, float green, float blue);

extern void		betray_settings_4x4_matrix_get(uint id, float *matrix);
extern void		betray_settings_4x4_matrix_set(uint id, float *matrix);*/
/*
		r_matrix_push(&matrix);
		r_matrix_rotate(NULL, (value - 0.5) * 90.0, 1, 1, 0.2);
		r_matrix_matrix_mult(NULL, m3);
		seduce_background_square_draw(input, m3, -0.2, -0.4, 0.4, 0.8, 0.5, 0.1, amnimation);
		seduce_background_shape_matrix_interact(input, m3, m3, TRUE, TRUE);

		seduce_widget_slider_radial(input, 0.1, 0.15, 0.1, 0.5, amnimation,  &c[0], &c[1], c[0], c[1], c[2], c[0], c[1], c[2]);
		seduce_widget_slider_radial(input, 0.1, 0.0, 0.1, 0.5, amnimation,  &c[1], &c[2], c[0], c[1], c[2], c[0], c[1], c[2]);
		seduce_widget_slider_radial(input, 0.1, -0.15, 0.1, 0.5, amnimation,  &c[2], &c[3], c[0], c[1], c[2], c[0], c[1], c[2]);
		seduce_widget_slider_radial(input, 0.1, -0.3, 0.1, 0.5, amnimation,  &value,  &value, c[0], c[1], c[2], c[0], c[1], c[2]);

		seduce_widget_wheel_radial(input, c, c, -0.1, 0.15, 0.1, 0.5, amnimation);
		seduce_widget_button_icon(input, center, SUI_3D_OBJECT_MESSAGE, -0.1, -0.15,  0.1, amnimation, float *color);
		seduce_widget_toggle_icon(input, &toggle, &toggle, SUI_3D_OBJECT_HIGHLIGHT, -0.1, -0.3, 0.1, amnimation);
		seduce_widget_select_radial(input, -0.1, 0.0, 0.1, 0.5, amnimation, lables, 6, &selected, &selected);


		seduce_text_edit_line(input, text, 0.3, 0, 0.3, 0.01, text, 32, "type something", TRUE, NULL, NULL, 0.2, 0.6, 1, 0.6, 1, 1, 1, 1.6);		
		seduce_text_edit_obfuscated(input, password, 0.3, -0.05, 0.3, 0.01, password, 32, "Password", TRUE, NULL, NULL, 0.2, 0.6, 1, 0.6, 1, 1, 1, 1.6);
		seduce_text_edit_double(input, &dvalue, 0.3, -0.1, 0.3, 0.01, &dvalue, TRUE, NULL, NULL, 0.2, 0.6, 1, 0.6, 1, 1, 1, 1.6);
		seduce_text_edit_int(input, &ivalue, 0.3, -0.15, 0.3, 0.01, &ivalue, TRUE, NULL, NULL, 0.2, 0.6, 1, 0.6, 1, 1, 1, 1.6);
		seduce_text_edit_uint(input, &uvalue, 0.3, -0.2, 0.3, 0.01, &uvalue, TRUE, NULL, NULL, 0.2, 0.6, 1, 0.6, 1, 1, 1, 1.6);

*/

#define SEDUCE_BBETRAY_LINE_DISTANCE 0.04

extern boolean seduce_background_shape_draw2(BInputState *input, void *id, float a_x, float a_y, float b_x, float b_y, float c_x, float c_y, float d_x, float d_y, float timer, float normal_x, float normal_y, float *center);


void seduce_color_settings_panel(BInputState *input, boolean active)
{
	static float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -0.4, 0, 0, 1}, animation = 0.0;
	static float tilt[16] = {1, 0, 0.2, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	static float color[12] = {0.1, 0.1, 0.1, 0.1, 0.4, 0.4, 0.3, 0, 0.25, 0.5, 0.4, 0};
	RMatrix	*matrix;
	boolean toggle;
	int ivalue;
	float value, pos_x, pos_y, hsv[3];
	double dvalue; 
	char *text, *lables[32];
	uint i, j, count, selected;
	if(input->mode == BAM_MAIN)
	{
		if(active)
		{
			animation += input->delta_time * 3.0;
			if(animation > 1.0)
				animation = 1.0;
		}else
		{
			animation -= input->delta_time * 3.0;
			if(animation < 0.0)
				animation = 0.0;
		}
	}
	if(animation < 0.001)
		return;
	count = 9;
	matrix = r_matrix_get();
	r_matrix_push(matrix);
	r_matrix_matrix_mult(NULL, m);
	seduce_background_square_draw(input, m, -0.15, -0.5 * SEDUCE_BBETRAY_LINE_DISTANCE * (float)count - 0.05, 0.3, SEDUCE_BBETRAY_LINE_DISTANCE * (float)count + 0.1, 0.55, 0.1, animation);
	seduce_background_shape_matrix_interact(input, m, m, TRUE, TRUE);

	pos_y = SEDUCE_BBETRAY_LINE_DISTANCE * 0.5 * (float)(count - 1);

	for(i = 0; i < 3; i++)
	{
		text = "Surface Color";
		r_matrix_push(matrix);
		r_matrix_translate(matrix, -0.0, 0, 0.025);
		r_matrix_push(matrix);
		r_matrix_matrix_mult(NULL, tilt);
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE * animation, text, -1), pos_y - SEDUCE_T_SIZE * 0.5, SEDUCE_T_SIZE * animation, SEDUCE_T_SPACE, text, 0, 0, 0, 0.4 * animation, -1);
		r_matrix_pop(matrix);
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE * animation, text, -1), pos_y - SEDUCE_T_SIZE * 0.5, SEDUCE_T_SIZE * animation, SEDUCE_T_SPACE, text, 0.8, 0.8, 0.8, 1 * animation, -1);
		r_matrix_pop(matrix);

		pos_x = 0.06 + (float)((i + 0) % 2) * 0.05;
		seduce_widget_color_wheel_radial(input, &color[i * 4], &color[i * 4], pos_x, pos_y, 0.05, 0.5, animation);
		pos_y -= SEDUCE_BBETRAY_LINE_DISTANCE;

		text = "Surface Brightness";
		r_matrix_push(matrix);
		r_matrix_translate(matrix, -0.0, 0, 0.025);
		r_matrix_push(matrix);
		r_matrix_matrix_mult(NULL, tilt);
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE * animation, text, -1), pos_y - SEDUCE_T_SIZE * 0.5, SEDUCE_T_SIZE * animation, SEDUCE_T_SPACE, text, 0, 0, 0, 0.4 * animation, -1);
		r_matrix_pop(matrix);
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE * animation, text, -1), pos_y - SEDUCE_T_SIZE * 0.5, SEDUCE_T_SIZE * animation, SEDUCE_T_SPACE, text, 0.8, 0.8, 0.8, 1 * animation, -1);
		r_matrix_pop(matrix);

		pos_x = 0.06 + (float)((i + 1) % 2) * 0.05;
		f_rgb_to_hsv(hsv, color[i * 4], color[i * 4 + 1], color[i * 4 + 2]);
		if(seduce_widget_slider_radial(input, &color[i * 4 + 1], &hsv[2], pos_x, pos_y, 0.05, 0.5, 0, 1, animation,  NULL))
			f_hsv_to_rgb(&color[i * 4], hsv[2], hsv[2], hsv[2]);
		pos_y -= SEDUCE_BBETRAY_LINE_DISTANCE;

		text = "Surface Transparency";
		r_matrix_push(matrix);
		r_matrix_translate(matrix, -0.0, 0, 0.025);
		r_matrix_push(matrix);
		r_matrix_matrix_mult(NULL, tilt);
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE * animation, text, -1), pos_y - SEDUCE_T_SIZE * 0.5, SEDUCE_T_SIZE * animation, SEDUCE_T_SPACE, text, 0, 0, 0, 0.4 * animation, -1);
		r_matrix_pop(matrix);
		seduce_text_line_draw(NULL, -seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE * animation, text, -1), pos_y - SEDUCE_T_SIZE * 0.5, SEDUCE_T_SIZE * animation, SEDUCE_T_SPACE, text, 0.8, 0.8, 0.8, 1 * animation, -1);
		r_matrix_pop(matrix);

		pos_x = 0.06 + (float)((i + 0) % 2) * 0.05;

		seduce_widget_slider_radial(input, &color[i * 4 + 2], &color[i * 4 + 3], pos_x, pos_y, 0.05, 0.5, 0, 1, animation, NULL);
		pos_y -= SEDUCE_BBETRAY_LINE_DISTANCE;
	}
	seduce_background_color(color[0], color[1], color[2], color[3], 
							 color[4], color[5], color[6], color[7],  
							 color[8], color[9], color[10], color[11]);

	r_matrix_pop(matrix);
}


void seduce_settings_betray_set(SeducePanelElement *element)
{
	float tmp[16];
	uint i, j;
	for(i = 0; i < betray_settings_count(); i++)
	{
		element[i].text = betray_settings_name(i);
		element[i].description = NULL;
		switch(betray_settings_type(i))
		{
			case BETRAY_ST_TRIGGER :
				element[i].type = SEDUCE_PET_TRIGGER;
				element[i].param.trigger = 0;
			break;
			case BETRAY_ST_TOGGLE :
				element[i].type = SEDUCE_PET_BOOLEAN;
				element[i].param.active = betray_settings_toggle_get(i);
			break;
			case BETRAY_ST_SELECT :
				element[i].type = SEDUCE_PET_SELECT;
				element[i].param.select.active = betray_settings_select_get(i);
				element[i].param.select.count = betray_settings_select_count_get(i);
				element[i].param.select.text = malloc((sizeof *element[i].param.select.text) * element[i].param.select.count);
				for(j = 0; j < element[i].param.select.count; j++)
					element[i].param.select.text[j] = betray_settings_select_name_get(i, j);
			break;
			case BETRAY_ST_NUMBER_FLOAT :
				element[i].type = SEDUCE_PET_REAL;
				element[i].param.real.value = betray_settings_number_float_get(i);
			break;
			case BETRAY_ST_NUMBER_INT :
				element[i].type = SEDUCE_PET_INTEGER;
				element[i].param.integer = betray_settings_number_int_get(i);
			break;
			case BETRAY_ST_SLIDER :
				element[i].type = SEDUCE_PET_REAL_BOUND;
				element[i].param.real.value = betray_settings_slider_get(i);
			break;
			case BETRAY_ST_2D :
				element[i].type = SEDUCE_PET_2D_POS;
				betray_settings_2d_get(i, &tmp[0], &tmp[1]);
				element[i].param.vector[0] = (double)tmp[0];
				element[i].param.vector[1] = (double)tmp[1];
			break;
			case BETRAY_ST_3D :
				element[i].type = SEDUCE_PET_3D_POS;
				betray_settings_3d_get(i, &tmp[0], &tmp[1], &tmp[2]);
				element[i].param.vector[0] = (double)tmp[0];
				element[i].param.vector[1] = (double)tmp[1];
				element[i].param.vector[2] = (double)tmp[2];
			break;
			case BETRAY_ST_COLOR :
				element[i].type = SEDUCE_PET_COLOR_RGB;
				betray_settings_color_get(i, &element[i].param.color[0], &element[i].param.color[1], &element[i].param.color[2]);
			break;
			case BETRAY_ST_4X4_MATRIX :
				element[i].type = SEDUCE_PET_4X4MATRIX;
				betray_settings_4x4_matrix_get(i, tmp);
				for(i = 0; i < 16; i++)
					element[i].param.vector[i] = (double)tmp[i];
			break;
		}
	}
}


void seduce_settings_betray_get(SeducePanelElement *element)
{
	float tmp[16];
	uint i, j;
	for(i = 0; i < betray_settings_count(); i++)
	{
		switch(betray_settings_type(i))
		{
			case BETRAY_ST_TRIGGER :
				if(element[i].param.trigger)
					betray_settings_trigger(i); 
			break;
			case BETRAY_ST_TOGGLE :
				betray_settings_toggle_set(i, element[i].param.active);
			break;
			case BETRAY_ST_SELECT :
				betray_settings_select_set(i, element[i].param.select.active);
				free(element[i].param.select.text);
			break;
			case BETRAY_ST_NUMBER_FLOAT :
				betray_settings_number_float_set(i, element[i].param.real.value);
			break;
			case BETRAY_ST_NUMBER_INT :
				betray_settings_number_int_set(i, element[i].param.integer);
			break;
			case BETRAY_ST_SLIDER :
				betray_settings_slider_set(i, element[i].param.real.value);
			break;
			case BETRAY_ST_2D :
				tmp[0] = (float)element[i].param.vector[0];
				tmp[1] = (float)element[i].param.vector[1];
				betray_settings_2d_set(i, tmp[0], tmp[1]);
			break;
			case BETRAY_ST_3D :
				tmp[0] = (float)element[i].param.vector[0];
				tmp[1] = (float)element[i].param.vector[1];
				tmp[2] = (float)element[i].param.vector[2];
				betray_settings_3d_set(i, tmp[0], tmp[1], tmp[2]);
			break;
			case BETRAY_ST_COLOR :
				betray_settings_color_set(i, element[i].param.color[0], element[i].param.color[1], element[i].param.color[2]);
			break;
			case BETRAY_ST_4X4_MATRIX :
				for(i = 0; i < 16; i++)
					tmp[i] = (float)element[i].param.vector[i];
				betray_settings_4x4_matrix_set(i, tmp);
			break;
		}
	}
}
