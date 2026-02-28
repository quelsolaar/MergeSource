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

#define opa_min_max_macro(data_type) \
		{\
			data_type *m; \
			float f; \
			array_length -= array_length % options->columns; \
			for(i = 0; i < options->columns; i++) \
			{ \
				if(options->column_data[i] == OPA_CD_PLOT_2D &&  i + 1 < options->columns) \
				{ \
					m = (data_type *)&memmory[offset]; \
					for(j = 0; j < array_length; j += options->columns) \
					{ \
						f = (float)m[j]; \
						if(options->min[0] > f) \
							options->min[0] = f; \
						if(options->max[0] < f) \
							options->max[0] = f; \
						f = (float)m[j + 1]; \
						if(options->min[1] > f ) \
							options->min[1] = f; \
						if(options->max[1] < f) \
							options->max[1] = f; \
					} \
				} \
				if(options->column_data[i] == OPA_CD_PLOT_1D) \
				{ \
					m = (data_type *)&memmory[offset]; \
					for(j = 0; j < array_length; j += options->columns) \
					{ \
						f = (float)m[j]; \
						if(options->min[0] > f) \
							options->min[0] = f; \
						if(options->max[0] < f) \
							options->max[0] = f; \
					} \
					options->min[1] = 0; \
					if(options->max[1] < array_length) \
						options->max[1] = array_length; \
				} \
			} \
		}

void opa_plot_scale_type(BInputState *input, OPAProject *project, uint8 *memmory, uint type, uint memmory_id, uint indirection, uint array_length, uint offset, OPADisplayOptions *options)
{
	uint i, j;
	if(type >= OPA_TYPE_COUNT)
	{
		if(project->types[type].construct == OPA_C_STRUCT ||
			project->types[type].construct == OPA_C_UNION)
		{
			for(i = 0; i < array_length; i++)
			{
				for(j = 0; j < project->types[type].member_count; j++)
				{
					if((project->types[project->types[type].members[j].base_type].construct == OPA_C_STRUCT ||
						project->types[project->types[type].members[j].base_type].construct == OPA_C_UNION) && 
						project->types[type].members[j].indirection == 0)
					{
						if(project->types[type].members[j].options.show)
							opa_plot_scale_type(input, project, memmory,
								project->types[type].members[j].base_type, 
								memmory_id, 
								project->types[type].members[j].indirection, 
								project->types[type].members[j].array_length, 
								offset + project->types[type].size_of * (i + options->scroll) + project->types[type].members[j].offset, 
								&project->types[type].members[j].options);

					}else
					{
						opa_plot_scale_type(input, project, memmory,
							project->types[type].members[j].base_type, 
							memmory_id, 
							project->types[type].members[j].indirection, 
							project->types[type].members[j].array_length, 
							offset + project->types[type].size_of * (i + options->scroll) + project->types[type].members[j].offset, 
							&project->types[type].members[j].options);
					}
				}
			}
		}else if(indirection == 0 && project->types[type].construct != OPA_TYPE_VOID && project->types[type].construct != OPA_TYPE_FUNCTION && project->types[type].construct != OPA_C_ENUM)
			opa_plot_scale_type(input, project, memmory,
										project->types[type].members[0].base_type, 
										memmory_id, 
										project->types[type].members[0].indirection, 
										project->types[type].members[0].array_length, 
										0, 
										options);
	}else switch(type)
	{
		default :
		break;
		case OPA_TYPE_SINGED_CHAR :	
		opa_min_max_macro(int8);
		break;
		case OPA_TYPE_UNSINGED_CHAR :
		opa_min_max_macro(uint8);
		break;
		case OPA_TYPE_SIGNED_SHORT :
		opa_min_max_macro(int16);
		break;
		case OPA_TYPE_UNSIGNED_SHORT :
		opa_min_max_macro(int16);
		break;
		case OPA_TYPE_SINGED_INT :
		opa_min_max_macro(int32);
		break;
		case OPA_TYPE_UNSINGED_INT :
		opa_min_max_macro(uint32);
		break;
		case OPA_TYPE_SIGNED_LONG_LONG :
		opa_min_max_macro(int64);
		break;
		case OPA_TYPE_UNSIGNED_LONG_LONG :
		opa_min_max_macro(uint64);
		break;
		case OPA_TYPE_FLOAT :
		j = 0;
		opa_min_max_macro(float);
		if(array_length > 17)
			i = j;
		break;
		case OPA_TYPE_DOUBLE :
		opa_min_max_macro(double);
		break;
	}
}

float opa_plot_draw_memory_type(BInputState *input, THandle *handle, OPAProject *project, uint8 *memmory, uint type, uint memmory_id, uint indirection, uint array_length, uint offset, OPADisplayOptions *options, float *pos, float *color, RMatrix *camera)
{
	uint i, j;
	if(type >= OPA_TYPE_COUNT)
	{
		if(project->types[type].construct == OPA_C_STRUCT ||
			project->types[type].construct == OPA_C_UNION)
		{
			for(i = 0; i < array_length; i++)
			{
				for(j = 0; j < project->types[type].member_count; j++)
				{
					if((project->types[project->types[type].members[j].base_type].construct == OPA_C_STRUCT ||
						project->types[project->types[type].members[j].base_type].construct == OPA_C_UNION) && 
						project->types[type].members[j].indirection == 0)
					{
						if(project->types[type].members[j].options.show)
							opa_plot_draw_memory_type(input, handle, project, memmory,
								project->types[type].members[j].base_type, 
								memmory_id, 
								project->types[type].members[j].indirection, 
								project->types[type].members[j].array_length, 
								offset + project->types[type].size_of * (i + options->scroll) + project->types[type].members[j].offset, 
								&project->types[type].members[j].options, pos, color, camera);

					}else
					{
						opa_plot_draw_memory_type(input, handle, project, memmory,
							project->types[type].members[j].base_type, 
							memmory_id, 
							project->types[type].members[j].indirection, 
							project->types[type].members[j].array_length, 
							offset + project->types[type].size_of * (i + options->scroll) + project->types[type].members[j].offset, 
							&project->types[type].members[j].options, pos, color, camera);
					}
				}
			}
		}else if(indirection == 0 && project->types[type].construct != OPA_TYPE_VOID && project->types[type].construct != OPA_TYPE_FUNCTION && project->types[type].construct != OPA_C_ENUM)
			opa_plot_draw_memory_type(input, handle, project, memmory,
										project->types[type].members[0].base_type, 
										memmory_id, 
										project->types[type].members[0].indirection, 
										project->types[type].members[0].array_length, 
										0, 
										options, pos, color, camera);
	}else switch(type)
	{
		default :
		break;
		case OPA_TYPE_SINGED_CHAR :	
		break;
		case OPA_TYPE_UNSINGED_CHAR :
		break;
		case OPA_TYPE_SIGNED_SHORT :
		break;
		case OPA_TYPE_UNSIGNED_SHORT :
		break;
		case OPA_TYPE_SINGED_INT :
		break;
		case OPA_TYPE_UNSINGED_INT :
		break;
		case OPA_TYPE_SIGNED_LONG_LONG :
		break;
		case OPA_TYPE_UNSIGNED_LONG_LONG :
		break;
		case OPA_TYPE_FLOAT :
		{
			float *m, *base = NULL, pos[3];
			m = (float *)&memmory[offset];
			array_length -= array_length % options->columns;

			for(i = 0; i < options->columns; i++)
			{
				if(options->column_data[i] == OPA_CD_PLOT_3D &&  i + 2 < options->columns)
				{
					base = &m[i];
					break;
				}
			}
			for(i = 0; i < options->columns; i++)
			{
				if(options->column_data[i] == OPA_CD_PLOT_3D &&  i + 2 < options->columns)
				{
					
					m = (float *)&memmory[offset];
					if(m <= options->manipulator && m + array_length > options->manipulator && ((float *)options->manipulator - m) % options->columns == i) 
					{						
						if(seduce_manipulator_pos_xyz(input, camera, options->manipulator, &options->manipulator, NULL, FALSE, TRUE, TRUE, TRUE, 1, 1))
						{
							opa_request_memory_set(project, handle, memmory_id, (uint8 *)options->manipulator - memmory, sizeof(float) * 3, options->manipulator);
						}
					}

					if(input->mode == BAM_DRAW)
					{
						m = (float *)&memmory[offset];
						m = &m[i];
						for(j = 0; j < array_length; j += options->columns)
						{
							r_matrix_projection_screenf(camera, pos, m[j], m[j + 1], m[j + 2]);
							r_primitive_line_2d(pos[0] + 0.004, pos[1], 
													pos[0] - 0.004, pos[1], 0.4, 0.8, 0.0, 0);
							r_primitive_line_2d(pos[0], pos[1] + 0.004,  
												pos[0], pos[1] - 0.004, 0.4, 0.8, 0.0, 0);
						}
					}
					if(input->mode == BAM_EVENT)
					{
						for(j = 0; j < input->pointer_count; j++)
							if(input->pointers[j].button[0] && !input->pointers[j].last_button[0])
								break;
						if(j < input->pointer_count)
						{
							float click_x, click_y, best, f;
							click_x = input->pointers[j].pointer_x;
							click_y = input->pointers[j].pointer_y;
							best = 0.1;
							m = (float *)&memmory[offset];
							m = &m[i];
							for(j = 0; j < array_length; j += options->columns)
							{
								r_matrix_projection_screenf(camera, pos, m[j], m[j + 1], m[j + 2]);
								if(pos[2] < 0.0)
								{
									pos[0] -= click_x;
									pos[1] -= click_y;
									f = pos[0] * pos[0] + pos[1] * pos[1];
									if(f < best)
									{
										best = f;
										options->scroll = j / options->columns;
										options->manipulator = &m[j];
									}
								}
							}
						}
					}
				}
				if(options->column_data[i] == OPA_CD_PLOT_2D &&  i + 1 < options->columns)
				{
			
					float center[2], scale, steps, f, x, y, pos[3];
					m = (float *)&memmory[offset];
					m = &m[i];

					center[0] = options->down_left[0];
					center[0] += (options->max[0] - options->min[0] - options->min[0]) / (options->max[0] - options->min[0]) * (options->up_right[0] - options->down_left[0]) - (options->up_right[0] - options->down_left[0]);
					center[1] = options->down_left[1];
					center[1] += (options->max[1] - options->min[1] - options->min[1]) / (options->max[1] - options->min[1]) * (options->up_right[1] - options->down_left[1]) - (options->up_right[1] - options->down_left[1]);
					if((options->max[0] - options->min[0]) / (options->up_right[0] - options->down_left[0]) >
						(options->max[1] - options->min[1]) / (options->up_right[1] - options->down_left[1]))
					{
						scale = (options->up_right[0] - options->down_left[0]) / (options->max[0] - options->min[0]);
						f = 0.4 * (options->max[0] - options->min[0]) / (options->up_right[0] - options->down_left[0]);
					}else
					{
						scale = (options->up_right[1] - options->down_left[1]) / (options->max[1] - options->min[1]);
						f = 0.4 * (options->max[1] - options->min[1]) / (options->up_right[1] - options->down_left[1]);
					}
					
					if(m <= options->manipulator && m + array_length > options->manipulator && ((float *)options->manipulator - m) % options->columns == i) 
					{
						pos[0] = center[0] + ((float *)options->manipulator)[0] * scale;
						pos[1] = center[1] + ((float *)options->manipulator)[1] * scale;
						pos[2] = 0.0;
						if(seduce_manipulator_point_plane(input, NULL, pos, options->manipulator, NULL, FALSE, 2, 1))
						{
							((float *)options->manipulator)[0] = (pos[0] - center[0]) / scale;
							((float *)options->manipulator)[1] = (pos[1] - center[1]) / scale;
							opa_request_memory_set(project, handle, memmory_id, (uint8 *)options->manipulator - memmory, sizeof(float) * 2, options->manipulator);
						}
					}
					if(input->mode == BAM_EVENT)
					{
						for(j = 0; j < input->pointer_count; j++)
							if(input->pointers[j].button[0] && !input->pointers[j].last_button[0])
								break;
						if(j < input->pointer_count)
						{
							float click_x, click_y, best, f;
							click_x = input->pointers[j].pointer_x;
							click_y = input->pointers[j].pointer_y;
							best = 0.1;
							m = (float *)&memmory[offset];
							m = &m[i];
							for(j = 0; j < array_length; j += options->columns)
							{
								x = center[0] + m[j] * scale - click_x;
								y = center[1] + m[j + 1] * scale - click_y;									
								f = x * x + y * y;
								if(f < best)
								{
									best = f;
									options->scroll = j / options->columns;
									options->manipulator = &m[j - i];
								}
							}
						}
					}

					if(input->mode == BAM_DRAW)
					{


						if(f > 1.0)
						{
							steps = 0.01;
							while(f > 1.0)
							{
								f /= 10.0;
								steps *= 10.0; 
							}
						}else
						{
							steps = 0.1;
							while(f < 1.0)
							{
								f *= 10.0;
								steps /= 10.0; 
							}
						}
							
						//scale = (options->up_right[1] - options->down_left[1]) / (options->max[0] - options->min[0]);
						r_primitive_line_2d(options->down_left[0], center[1], 
											options->up_right[0], center[1], 1.0, 1.0, 1.0, 0);	
						r_primitive_line_2d(center[0], options->down_left[1], 
											center[0], options->up_right[1], 1.0, 1.0, 1.0, 0);	
				/*		r_primitive_line_2d(options->up_right[0], options->up_right[1], 
											options->up_right[0], options->down_left[1], 1.0, 1.0, 1.0, 0);	
						r_primitive_line_2d(options->down_left[0], options->up_right[1], 
											options->down_left[0], options->down_left[1], 1.0, 1.0, 1.0, 0);	*/

						steps *= scale;
						if(steps > 0.0001)
						{
							f = center[0] + steps;
							for(j = 1; f < options->up_right[0]; j++)
							{
								if(j % 10 == 0)
									r_primitive_line_2d(f, options->down_left[1], f, options->up_right[1], 0.2, 0.2, 0.0, 0);
								else if(j % 5 == 0)
									r_primitive_line_2d(f, options->down_left[1], f, options->up_right[1], 0.1, 0.1, 0.1, 0);
								else
									r_primitive_line_2d(f, options->down_left[1], f, options->up_right[1], 0.05, 0.05, 0.05, 0);
								f += steps;
							}
							f = center[0] - steps;
							for(j = 1; f > options->down_left[0]; j++)
							{
								if(j % 10 == 0)
									r_primitive_line_2d(f, options->down_left[1], f, options->up_right[1], 0.2, 0.2, 0.0, 0);
								else if(j % 5 == 0)
									r_primitive_line_2d(f, options->down_left[1], f, options->up_right[1], 0.1, 0.1, 0.1, 0);
								else
									r_primitive_line_2d(f, options->down_left[1], f, options->up_right[1], 0.05, 0.05, 0.05, 0);
								f -= steps;
							}		

							f = center[1] + steps;
							for(j = 1; f < options->up_right[1]; j++)
							{
								if(j % 10 == 0)
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.2, 0.2, 0.0, 0);
								else if(j % 5 == 0)
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.1, 0.1, 0.1, 0);
								else
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.05, 0.05, 0.05, 0);
								f += steps;
							}
							f = center[1] - steps;
							for(j = 1; f > options->down_left[1]; j++)
							{
								if(j % 10 == 0)
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.2, 0.2, 0.0, 0);
								else if(j % 5 == 0)
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.1, 0.1, 0.1, 0);
								else
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.05, 0.05, 0.05, 0);
								f -= steps;
							}		
						}
						for(j = 0; j < array_length; j += options->columns)
						{
							x = center[0] + m[j] * scale;
							y = center[1] + m[j + 1] * scale;
							
							r_primitive_line_fade_3d(x, y, 0,
												x, y, 0.01, 0.4, 0.8, 0.0, 0, 0.0, 0.0, 0.0, 0);
							r_primitive_line_2d(x + 0.004, y, 
												x - 0.004, y, 0.4, 0.4, 0.4, 0);
							r_primitive_line_2d(x, y + 0.004, 
												x, y - 0.004, 0.4, 0.4, 0.4, 0);
						}
					}
					if(options->first)
					{
						seduce_manipulator_square_cornered(input, NULL, options->down_left, options->up_right, options->down_left, NULL, FALSE, FALSE, 1, 1);
						options->first = FALSE;
					}
		//			
				}
				if(options->column_data[i] == OPA_CD_PLOT_1D)
				{
					if(options->first && options->max[0] - options->min[0] > 0.0001)
					{
						float height, scale, steps = 1.0, f;

						height = options->down_left[1];
						height += (options->max[0] - options->min[0] - options->min[0]) / (options->max[0] - options->min[0]) * (options->up_right[1] - options->down_left[1]) - (options->up_right[1] - options->down_left[1]);
						f = 0.4 * (options->max[0] - options->min[0]) / (options->up_right[1] - options->down_left[1]);
						if(f > 1.0)
						{
							steps = 0.01;
							while(f > 1.0)
							{
								f /= 10.0;
								steps *= 10.0; 
							}
						}else
						{
							steps = 0.1;
							while(f < 1.0)
							{
								f *= 10.0;
								steps /= 10.0; 
							}
						}
							
						scale = (options->up_right[1] - options->down_left[1]) / (options->max[0] - options->min[0]);

						if(m <= options->manipulator && m + array_length > options->manipulator && ((float *)options->manipulator - m) % options->columns == i) 
						{
							pos[0] = options->down_left[0] + (float)((float *)options->manipulator - m) / (float)array_length * (options->up_right[0] - options->down_left[0]);
							pos[1] = height + ((float *)options->manipulator)[0] * scale;

							if(input->mode == BAM_DRAW)
							{
								r_primitive_line_2d(pos[0], options->up_right[1], 
													pos[0], options->down_left[1], 1.0, 1.0, 1.0, 0);	
								r_primitive_line_2d(options->up_right[0], pos[1], 
													options->down_left[0], pos[1], 1.0, 1.0, 1.0, 0);	
							}

							pos[2] = 0.0;
							if(seduce_manipulator_point_axis(input, NULL, pos, options->manipulator, NULL, FALSE, 1, 1))
							{
								((float *)options->manipulator)[0] = (pos[1] - height) / scale;
								opa_request_memory_set(project, handle, memmory_id, (uint8 *)options->manipulator - memmory, sizeof(float), options->manipulator);
							}
						}

						if(input->mode == BAM_EVENT)
						{
							for(j = 0; j < input->pointer_count; j++)
								if(input->pointers[j].button[0] && !input->pointers[j].last_button[0] && 
									input->pointers[j].pointer_x > options->down_left[0] &&
									input->pointers[j].pointer_y > options->down_left[1] &&
									input->pointers[j].pointer_x < options->up_right[0] &&
									input->pointers[j].pointer_y < options->up_right[1])
									break;
							if(j < input->pointer_count)
							{
								options->scroll = ((input->pointers[j].pointer_x - options->down_left[0]) / (options->up_right[0] - options->down_left[0]) * array_length / options->columns);
								options->scroll *= options->columns;
								m = (float *)&memmory[offset];
								options->manipulator = &m[options->scroll];
							}
						}

						if(input->mode == BAM_DRAW)
						{

							r_primitive_line_2d(options->down_left[0], height, 
												options->up_right[0], height, 1.0, 1.0, 1.0, 0);	
							r_primitive_line_2d(options->up_right[0], options->up_right[1], 
												options->up_right[0], options->down_left[1], 1.0, 1.0, 1.0, 0);	
							r_primitive_line_2d(options->down_left[0], options->up_right[1], 
												options->down_left[0], options->down_left[1], 1.0, 1.0, 1.0, 0);	
							steps *= scale;
							f = height + steps;
							for(j = 1; f < options->up_right[1]; j++)
							{
								if(j % 10 == 0)
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.2, 0.2, 0.0, 0);
								else if(j % 5 == 0)
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.1, 0.1, 0.1, 0);
								else
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.05, 0.05, 0.05, 0);
								f += steps;
							}
							f = height - steps;
							for(j = 1; f > options->down_left[1]; j++)
							{
								if(j % 10 == 0)
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.2, 0.2, 0.0, 0);
								else if(j % 5 == 0)
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.1, 0.1, 0.1, 0);
								else
									r_primitive_line_2d(options->down_left[0], f, options->up_right[0], f, 0.05, 0.05, 0.05, 0);
								f -= steps;
							}			
						}
						seduce_manipulator_square_cornered(input, NULL, options->down_left, options->up_right, options->down_left, NULL, FALSE, FALSE, 1, 1);
						options->first = FALSE;
					}
					if(input->mode == BAM_DRAW)
					{
						float value, pos, add, height, scale;
						m = (float *)&memmory[offset];
						m = &m[i];
						value = m[0];
						pos = options->down_left[0];
						add = (options->up_right[0] - options->down_left[0]) / (float)array_length * (float)options->columns;
						height = options->down_left[1];
						scale = (options->up_right[1] - options->down_left[1]) / (options->max[0] - options->min[0]);

						height += (options->max[0] - options->min[0] - options->min[0]) / (options->max[0] - options->min[0]) * (options->up_right[1] - options->down_left[1]) - (options->up_right[1] - options->down_left[1]);
						for(j = options->columns; j < array_length; j += options->columns)
						{
							r_primitive_line_2d(pos, height + m[j - options->columns] * scale, 
												pos + add, height + m[j] * scale, 0.4, 0.8, 0.0, 0);
							pos += add;
						}
					}

				}
			}
		}
		break;
		case OPA_TYPE_DOUBLE :
		break;
	}
}



void opa_plot_option_clear(OPADisplayOptions *options)
{
	options->min[0] = 0;
	options->min[1] = 0;
	options->max[0] = 0;
	options->max[1] = 0;
	options->first = TRUE;
}

void opa_plot_draw(BInputState *input, THandle *handle, OPAProject *project, RMatrix *camera)
{
	float color[3] = {0 , 0, 0}, pos[3] = {0, 0, 0};
	uint i, j;

	for(i = 0; i < project->type_count; i++)
	{
		for(j = 0; j < project->types[i].member_count; j++)
		{
			project->types[i].members[0].options.first = TRUE;
			if(input->mode == BAM_DRAW)
				opa_plot_option_clear(&project->types[i].members[0].options);
		}
	}
	for(i = 0; i < project->memory_count; i++)
	{
		if(project->memmory[i].data != NULL)
		{
			project->memmory[i].options.first = TRUE;
			if(input->mode == BAM_DRAW)
				opa_plot_option_clear(&project->memmory[i].options);
		}
	}
	if(input->mode == BAM_DRAW)
		for(i = 0; i < project->memory_count; i++)
			if(project->memmory[i].data != NULL)
				opa_plot_scale_type(input, project, project->memmory[i].data, project->memmory[i].type, i, project->memmory[i].indirection, project->memmory[i].data_size / project->types[project->memmory[i].type].size_of, 0, &project->memmory[i].options);

	for(i = 0; i < project->memory_count; i++)
		if(project->memmory[i].data != NULL)
			opa_plot_draw_memory_type(input, handle, project, project->memmory[i].data, project->memmory[i].type, i, project->memmory[i].indirection, project->memmory[i].data_size / project->types[project->memmory[i].type].size_of, 0, &project->memmory[i].options, color, pos, camera);

	if(input->mode == BAM_DRAW)
		r_primitive_line_flush();
}