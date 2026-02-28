
#include "la_includes.h"

#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_tool.h"
#include "la_pop_up.h"
#include "la_draw_overlay.h"
#include "la_particle_fx.h"
 
RMatrix la_interface_matrix;
RMatrix la_world_matrix;

#define VERTEX_SNAP_DISTANCE 0.0001
#define MAX_POPUP_TIME 100

typedef enum { 
	PIM_IDLE,
	PIM_DRAW,
	PIM_DRAW_SELECT,
	PIM_RESHAPE,
	PIM_SPLIT,
	PIM_DRAG_MANIPULATOR,
	PIM_DRAG_ONE_VERTEX,
	PIM_DRAG_ONE_TAG,
	PIM_GROUP_SELECT,
	PIM_SHOW_EMPTY_MENU,
	PIM_SHOW_OBJECT_SELECT,
	PIM_SHOW_VERTEX_MENU,
	PIM_SHOW_EDGE_MENU,
	PIM_SHOW_MANIPULATOR_MENU,
	PIM_SHOW_POLY_MENU,
	PIM_SHOW_SETTINGS,
	PIM_SHOW_RENAME,
	PIM_CHANGE_VIEW
}ParseInputMode;

struct{
	ParseInputMode	mode;
	uint			start_vertex;
	uint			closest_vertex;
	double			depth[3];
	uint			click_time;
}ParseInputData;

boolean draw_view_cage(void)
{
	return ParseInputData.mode != PIM_CHANGE_VIEW;
}

uint lo_pointer_polygon = -1;
void la_t_edge_fill_strip_debug();
void la_poly_csg();

static double debug_snap[3];

boolean la_popup_measure(BInputState *input, uint *edge)
{
	uint32 vertex_count, i;
	double *vertex, *v1, *v2, pos[3], dist, pre_dist, normal[3], selection;
	udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);
	if(edge[0] >= vertex_count || edge[1] >= vertex_count || udg_get_select(edge[1]) <= 0)
		return FALSE;
	v1 = &vertex[edge[0] * 3];
	v2 = &vertex[edge[1] * 3];
	pre_dist = dist = f_vector_normalized3d(normal, v1, v2);
	if(dist == 0)
		return FALSE;
	seduce_view_projection_screend(NULL, pos, (v1[0] + v2[0]) * 0.5, (v1[1] + v2[1]) * 0.5, (v1[2] + v2[2]) * 0.5);
	if(S_TIS_DONE == seduce_text_edit_double(input, la_popup_measure, NULL, &dist, pos[0], pos[1], 0.2, SEDUCE_T_SIZE, TRUE, NULL, NULL, 0.8, 0.8, 0.8, 0.8, 1, 1, 1, 1))
	{
		if(dist != pre_dist)
		{
			dist -= pre_dist;
			dist /= udg_get_select(edge[1]);
			for(i = 0; i < vertex_count; i++)
			{
				selection = udg_get_select(i);
				if(selection > 0)
				{
					udg_vertex_set(i, &vertex[i * 3], vertex[i * 3] + normal[0] * dist * selection, 
													vertex[i * 3 + 1] + normal[1] * dist * selection, 
													vertex[i * 3 + 2] + normal[2] * dist * selection);
				}
			}
		}
		return FALSE;
	}
	return TRUE;
}

void la_parse_input(BInputState *input)
{
	double output[3]; 
	static double snap[6] = {0, 0, 0, 0, 0, 0}, distance, selected_distance;
	static uint closest = -1, select_closest = -1, edge[2] = {-1, -1}, polygon = -1, tag = -1, measure_edge[2] = {-1, -1};
	static SUISnapType snap_type = SUI_ST_VERTEX_FAR;
	LAPUEmptyReturn empty_pop_up_return;

	if(input->mode == BAM_DRAW)
	{
		RMatrix *m;
		m = r_matrix_get();
		r_matrix_set(&la_world_matrix);
		r_matrix_set(m);
	}
	
	if(input->mode == BAM_MAIN)
	{
		la_t_group_update(input, ParseInputData.mode != PIM_IDLE && ParseInputData.mode != PIM_CHANGE_VIEW);
		imagine_sleepd(0.005);
	}

	if(input->mode == BAM_EVENT)
	{
		uint i;
		distance = 1E100;
		selected_distance = 1E100;
		snap_type = SUI_ST_NONE;
		if(ParseInputData.mode != PIM_CHANGE_VIEW &&
			ParseInputData.mode != PIM_DRAW_SELECT &&
			(ParseInputData.mode != PIM_DRAG_MANIPULATOR || input->pointers[0].button[1]) &&
			ParseInputData.mode != PIM_RESHAPE &&
			ParseInputData.mode != PIM_SPLIT &&
			ParseInputData.mode != PIM_SHOW_EMPTY_MENU &&
			ParseInputData.mode != PIM_SHOW_OBJECT_SELECT &&
			ParseInputData.mode != PIM_SHOW_VERTEX_MENU &&
			ParseInputData.mode != PIM_SHOW_EDGE_MENU &&
			ParseInputData.mode != PIM_SHOW_MANIPULATOR_MENU &&
			ParseInputData.mode != PIM_SHOW_POLY_MENU &&
			ParseInputData.mode != PIM_SHOW_SETTINGS &&
			ParseInputData.mode != PIM_SHOW_RENAME &&
			ParseInputData.mode != PIM_CHANGE_VIEW)
		{
			lo_projection_cache_update_test(&la_world_matrix);
			if(p_find_closest_vertex(&closest, &select_closest, &distance, &selected_distance, input->pointers[0].pointer_x, input->pointers[0].pointer_y, !input->pointers[0].button[2]))
			{
				udg_get_vertex_pos(snap, select_closest);
				if(selected_distance > VERTEX_SNAP_DISTANCE - (0.1 * VERTEX_SNAP_DISTANCE))
					snap_type = SUI_ST_VERTEX_FAR;
				else
					snap_type = SUI_ST_VERTEX_CLOSE;
				for(i = 0; i < 6; i++)
					if(snap[i] > 100 || snap[i] < -100 || snap[i] != snap[i])
						i = 100; 
			}
			if(snap_type == SUI_ST_VERTEX_FAR && (ParseInputData.mode != PIM_DRAW || input->pointers[0].button[1]))
			{
				boolean selected;
				if(ParseInputData.mode != PIM_SHOW_EDGE_MENU)
				{
					if(p_find_closest_edge(edge, snap, input->pointers[0].pointer_x, input->pointers[0].pointer_y))
						if((udg_get_select(edge[0]) < 0.01 && udg_get_select(edge[1]) < 0.01) || !input->pointers[0].button[2])
							snap_type = SUI_ST_LINE;
				for(i = 0; i < 6; i++)
					if(snap[i] > 100 || snap[i] < -100 || snap[i] != snap[i])
						i = 100; 
				}
				if(edge[0] == -1 && ParseInputData.mode != PIM_SHOW_POLY_MENU)
				{
					polygon = p_find_closest_polygon(input, snap, &snap[3], &selected);
					if(polygon != -1 && (!selected || !input->pointers[0].button[1]))
						snap_type = SUI_ST_TANGENT;
					lo_pointer_polygon = polygon;
				for(i = 0; i < 6; i++)
					if(snap[i] > 100 || snap[i] < -100 || snap[i] != snap[i])
						i = 100; 
				}
			}
		}
		la_t_paste_to_new_geometry();
	}
	
	switch(ParseInputData.mode)
	{
		case PIM_IDLE :
			if(input->mode == BAM_DRAW)
				lo_pfx_draw_selected(input, snap_type, select_closest, polygon, edge, input->pointers[0].button[0] && !input->pointers[0].last_button[0]);
			if(la_t_tm_grab(input))
			{
				ParseInputData.mode = PIM_DRAG_MANIPULATOR;
				return;
			}else if(input->mode == BAM_EVENT)
			{
				if(la_t_group_interact(input))
				{
					ParseInputData.mode = PIM_GROUP_SELECT;
					return;					
				}else if(input->pointers[0].button[0] == TRUE && input->pointers[0].last_button[0] == FALSE)
				{
					ParseInputData.click_time = 0;
					ParseInputData.start_vertex = -1;
					if(la_popup_measure != seduce_element_pointer_id(input, 0, NULL))
					{
						if(p_find_click_tag_lable(input->pointers[0].pointer_x, input->pointers[0].pointer_y))
							return;

						tag = p_find_click_tag(input->pointers[0].pointer_x, input->pointers[0].pointer_y);
						if(tag != -1)
						{
							udg_get_tag_pos(tag, ParseInputData.depth);
							ParseInputData.mode = PIM_DRAG_ONE_TAG;
							return;
						}
						if(selected_distance < VERTEX_SNAP_DISTANCE - (0.1 * VERTEX_SNAP_DISTANCE))
						{
							udg_get_vertex_pos(ParseInputData.depth, select_closest);
							ParseInputData.start_vertex = select_closest;
						}
						else
						{
							double center[3];
							if(edge[0] != -1 && edge[1] != -1)
							{
								if(udg_get_select(edge[0]) > 0.01 && udg_get_select(edge[1]) > 0.01)
								{
									ParseInputData.mode = PIM_RESHAPE;								
									la_t_reshape_hull_start(input, edge);
								}else
								{
									ParseInputData.mode = PIM_SPLIT;
									la_t_edge_splitter_start(input, edge);
								}
								return;
							}
							seduce_view_center_getd(NULL, center);
							seduce_view_projection_vertexd(NULL, ParseInputData.depth, center, input->pointers[0].pointer_x, input->pointers[0].pointer_y);
							ParseInputData.start_vertex = -1;
						}
						if(ParseInputData.start_vertex != -1 && udg_get_select(ParseInputData.start_vertex) > 0.5)
						{
                    		ParseInputData.mode = PIM_DRAG_ONE_VERTEX;
							grab_one_vertex(input, ParseInputData.start_vertex, ParseInputData.depth);
						}
						else
						{
							ParseInputData.mode = PIM_DRAW;
							la_t_new_draw_line(input->pointers[0].pointer_x, input->pointers[0].pointer_y);
						}
					}
				}else if(input->pointers[0].button[2] == TRUE && input->pointers[0].last_button[2] == FALSE)
				{
					ParseInputData.start_vertex = select_closest;
					if(la_t_tm_test_center(input))
						ParseInputData.mode = PIM_SHOW_MANIPULATOR_MENU;
					else if(selected_distance < VERTEX_SNAP_DISTANCE - (0.1 * VERTEX_SNAP_DISTANCE))
					{
						ParseInputData.mode = PIM_SHOW_VERTEX_MENU;
						la_pu_vertex(input, ParseInputData.start_vertex, TRUE);
					}
					else if(edge[0] != -1 && edge[1] != -1)
					{
						if(udg_get_select(edge[0]) > 0.0 && udg_get_select(edge[1]) == 0.5)
						{
							measure_edge[0] = edge[1];
							measure_edge[1] = edge[0];
						}else if(udg_get_select(edge[0]) == 0.0 && udg_get_select(edge[1]) > 0.5)
						{
							measure_edge[0] = edge[0];
							measure_edge[1] = edge[1];
						}else 
							ParseInputData.mode = PIM_SHOW_EDGE_MENU;
					}
					else if((polygon = p_find_closest_polygon(input, NULL, NULL, NULL)) != -1)
						ParseInputData.mode = PIM_SHOW_POLY_MENU;
					else
						ParseInputData.mode = PIM_SHOW_EMPTY_MENU;
				}
				if(input->pointers[0].button[1] == TRUE && input->pointers[0].last_button[1] == FALSE)
				{
					if(la_t_tm_test_center(input))
						la_t_tm_view_center();
					else
					{
						ParseInputData.mode = PIM_CHANGE_VIEW;
						seduce_view_change_right_button(NULL, input, TRUE, TRUE, TRUE);
					}
				}
			}
			if(input->mode == BAM_DRAW)
			{
				if(la_t_tm_test_center(input) == FALSE)
				{
					if((ParseInputData.mode == PIM_IDLE && selected_distance < VERTEX_SNAP_DISTANCE - (0.1 * VERTEX_SNAP_DISTANCE)) || (ParseInputData.mode == PIM_DRAW || ParseInputData.mode == PIM_DRAG_MANIPULATOR || ParseInputData.mode == PIM_DRAG_ONE_VERTEX) && input->pointers[0].button[1] == TRUE)
					{
						double closest[3];
						udg_get_vertex_pos(closest, select_closest);
						la_do_active_vertex(input, closest, 0.01 < udg_get_select(select_closest));
					}else
						la_do_draw_closest_edge(edge, input->pointers[0].pointer_x, input->pointers[0].pointer_y, FALSE);
				}
				
			}
			if(ParseInputData.mode != PIM_IDLE)
				la_parse_input(input);
		break;
		case PIM_DRAW :
			la_view_projection_vertex_with_axis(input, NULL, output, ParseInputData.depth, input->pointers[0].pointer_x, input->pointers[0].pointer_y, input->pointers[0].button[1], snap, snap_type);
			if(input->mode == BAM_DRAW)
			{
				boolean del = FALSE;
				double pos[3];
				if(selected_distance > VERTEX_SNAP_DISTANCE - (0.1 * VERTEX_SNAP_DISTANCE) && ParseInputData.start_vertex == -1 && input->mode == BAM_DRAW)
					del = la_t_draw_line_draw_delete_overlay();
				if(selected_distance < VERTEX_SNAP_DISTANCE - (0.1 * VERTEX_SNAP_DISTANCE) && !input->pointers[0].button[1])
					udg_get_vertex_pos(output, select_closest);
				la_t_draw_line_draw_overlay(ParseInputData.depth, output);
				seduce_view_projection_screend(NULL, pos, output[0], output[1], output[2]);
				seduce_primitive_line_focal_depth_set(-pos[2]);

			}
			if(input->mode == BAM_EVENT)
			{
				if(ParseInputData.start_vertex == -1)
					if(la_t_draw_select_menu_test())
						ParseInputData.mode = PIM_DRAW_SELECT;
				if((input->pointers[0].button[2] == FALSE &&input->pointers[0].last_button[2] == TRUE) || (input->pointers[0].button[0] == FALSE && input->pointers[0].last_button[0] == TRUE))
				{
					double pos[3];
					uint vertex[2];
					if(selected_distance < VERTEX_SNAP_DISTANCE - (0.1 * VERTEX_SNAP_DISTANCE) && input->pointers[0].button[1] != TRUE)
					{
						vertex[1] = select_closest;
						if(ParseInputData.start_vertex == select_closest)
						{
							if(la_t_tm_hiden())
							{
								la_t_tm_place(ParseInputData.depth[0], ParseInputData.depth[1], ParseInputData.depth[2]);
							}
							udg_set_select(select_closest, 1);
							if(!input->pointers[0].button[0])								
								ParseInputData.mode = PIM_IDLE;
							break;
						}else if(ParseInputData.start_vertex != -1)
						{
							vertex[0] = ParseInputData.start_vertex;
							if(la_t_edge_connector(vertex) == TRUE)
								udg_create_edge(ParseInputData.start_vertex, vertex[1]);
							ParseInputData.start_vertex = vertex[1];
							udg_get_vertex_pos(ParseInputData.depth, ParseInputData.start_vertex);
							if(!input->pointers[0].button[0])								
								ParseInputData.mode = PIM_IDLE;
							break;
						}
					}else
					{
						if(ParseInputData.start_vertex == -1 && input->pointers[0].button[1] != TRUE && input->pointers[0].last_button[2] != TRUE)
						{
							if(la_t_draw_line_test_delete() || la_t_draw_line_test_select(SM_SELECT))
								break;
							if(0.01 > (input->pointers[0].pointer_x - input->pointers[0].click_pointer_x[0]) * (input->pointers[0].pointer_x - input->pointers[0].click_pointer_x[0]) + (input->pointers[0].pointer_y - input->pointers[0].click_pointer_y[0]) * (input->pointers[0].pointer_y - input->pointers[0].click_pointer_y[0]))
							{
								polygon = p_find_closest_polygon(input, NULL, NULL, NULL);
								if(polygon == -1)
								{
									udg_clear_select(0);
								}								
								else
									la_t_poly_select(polygon);
								break;
							}
						}
						la_view_projection_vertex_with_axis(input, NULL, pos, ParseInputData.depth, input->pointers[0].pointer_x, input->pointers[0].pointer_y, input->pointers[0].button[1], snap, snap_type);
						vertex[1] = udg_find_empty_slot_vertex();
						udg_vertex_set(vertex[1], NULL, pos[0], pos[1], pos[2]);
						ParseInputData.depth[0] = pos[0];
						ParseInputData.depth[1] = pos[1];
						ParseInputData.depth[2] = pos[2];
					}
					if(ParseInputData.start_vertex == -1)
					{
						la_view_projection_vertex_with_axis(input, NULL, pos, ParseInputData.depth, input->pointers[0].click_pointer_x[0], input->pointers[0].click_pointer_y[0], FALSE, snap, snap_type);
						ParseInputData.start_vertex = udg_find_empty_slot_vertex();
						udg_vertex_set(ParseInputData.start_vertex, NULL, pos[0], pos[1], pos[2]);				
					}
					udg_create_edge(ParseInputData.start_vertex, vertex[1]);
					ParseInputData.start_vertex = vertex[1];
					if(input->pointers[0].button[2] == FALSE && input->pointers[0].last_button[2] == TRUE)
						undo_event_done();					
					if(!input->pointers[0].button[0])								
						ParseInputData.mode = PIM_IDLE;
				}
			}
			if(input->mode == BAM_DRAW)
			{
				la_do_draw(ParseInputData.depth, output, input->pointers[0].button[1], snap);
				if(ParseInputData.start_vertex == -1)
					la_t_draw_line_add(input->pointers[0].pointer_x, input->pointers[0].pointer_y, input->pointers[0].last_button[0]);

				if(input->pointers[0].button[1] && snap_type != SUI_ST_VERTEX_FAR && snap_type != SUI_ST_VERTEX_CLOSE)
				{
			/*		if(snap_type == SUI_ST_LENGTH)
						la_do_draw_snap_edge(edge);	
					if(snap_type == SUI_ST_TANGENT)
						la_do_active_polygon(snap);*/
				}
				if(snap_type == SUI_ST_VERTEX_CLOSE|| (input->pointers[0].button[1] && snap_type == SUI_ST_VERTEX_FAR))
				{
					double closest[3];
					udg_get_vertex_pos(closest, select_closest);
					la_do_active_vertex(input, closest, FALSE);
				}
			}
		break;
		case PIM_RESHAPE :
			if(input->mode == BAM_EVENT)
				la_t_reshape_hull(input);
			if(input->mode == BAM_DRAW)
				la_t_reshape_hull_draw();
		break;
		case PIM_SPLIT :
			if(input->mode == BAM_EVENT)
				la_t_edge_splitter(input);
		break;
		case PIM_DRAG_MANIPULATOR :
			{
				uint i;
				for(i = 0; i < 6; i++)
					if(snap[i] > 100 || snap[i] < -100 || snap[i] != snap[i])
						i = 100; 
				if(betray_button_get(-1, BETRAY_BUTTON_S))
					i = 100;
			}
			debug_snap[0] = snap[0];
			debug_snap[1] = snap[1];
			debug_snap[2] = snap[2];
			if(!la_t_tm_manipulate(input, snap, snap_type) && input->mode == BAM_EVENT)
				ParseInputData.mode = PIM_IDLE;
		break;
		case PIM_DRAG_ONE_VERTEX :
			{
				double vertex[3];
				static uint collapse = 0;
				udg_get_vertex_pos(snap, select_closest);
				la_view_projection_vertex_with_axis(input, NULL, vertex, ParseInputData.depth, input->pointers[0].pointer_x, input->pointers[0].pointer_y, input->pointers[0].button[1], snap, snap_type);
 
				if(input->mode == BAM_EVENT)
				{
                   grab_one_vertex(input, ParseInputData.start_vertex, vertex);
//                  udg_vertex_move(ParseInputData.start_vertex, vertex[0], vertex[1], vertex[2]);
                    if(input->pointers[0].button[0] == FALSE)
						ParseInputData.mode = PIM_IDLE;
				}else
				{
					la_do_draw(ParseInputData.depth, output, input->pointers[0].button[1], snap);
					la_t_draw_line_draw_overlay(ParseInputData.depth, vertex);

					if(input->pointers[0].button[1] && snap_type != SUI_ST_VERTEX_FAR && snap_type != SUI_ST_VERTEX_CLOSE)
					{
					//	if(snap_type == SUI_ST_LENGTH)
					//		la_do_draw_snap_edge(edge);	
					}
					if(snap_type == SUI_ST_VERTEX_CLOSE|| (input->pointers[0].button[1] && snap_type == SUI_ST_VERTEX_FAR))
					{
						double closest[3];
						udg_get_vertex_pos(closest, select_closest);
						la_do_active_vertex(input, closest, FALSE);
					}
				}
			}
		break;
		case PIM_DRAG_ONE_TAG :
			{
				double vec[3];
				static uint collapse = 0;
				udg_get_vertex_pos(snap, select_closest);
				if(input->mode == BAM_EVENT)
				{
					la_view_projection_vertex_with_axis(input, NULL, vec, ParseInputData.depth, input->pointers[0].pointer_x, input->pointers[0].pointer_y, input->pointers[0].button[1], snap, snap_type);
					udg_move_tag(tag, vec);
					if(input->pointers[0].button[0] == FALSE)
					{
						if(VERTEX_SNAP_DISTANCE > (input->pointers[0].pointer_x - input->pointers[0].click_pointer_x[0]) * (input->pointers[0].pointer_x - input->pointers[0].click_pointer_x[0]) + (input->pointers[0].pointer_y - input->pointers[0].click_pointer_y[0]) * (input->pointers[0].pointer_y - input->pointers[0].click_pointer_y[0]))
						{
							udg_move_tag(tag, ParseInputData.depth);
							udg_select_tag(tag, 1);
						}
						ParseInputData.mode = PIM_IDLE;
					}
				}else
				{
					la_do_xyz_lines(ParseInputData.depth, input->pointers[0].button[1]);
					if(select_closest != ParseInputData.start_vertex && selected_distance < VERTEX_SNAP_DISTANCE - (0.1 * VERTEX_SNAP_DISTANCE) && input->pointers[0].button[1] == TRUE)
						la_do_active_vertex(input, snap, FALSE);
				}
			}
		break;
		case PIM_DRAW_SELECT :
			{
				uint i;
				i = la_pu_select(input, TRUE);
				if(input->mode == BAM_EVENT)
				{
					if(i != -1)
						la_t_draw_line_test_select(i);
				}else
					la_t_draw_line_add(input->pointers[0].pointer_x, input->pointers[0].pointer_y, FALSE);
			}
		break;
		case PIM_CHANGE_VIEW :
			if(input->mode != BAM_EVENT)
				la_t_tm_grab(input);
			else
			{
				seduce_view_change_right_button(NULL, input, TRUE, TRUE, TRUE);
			//	if(input->pointers[0].button[1] == FALSE && input->pointers[0].last_button[1] == TRUE)
			//		la_draw_force_update_persuade();

				if(input->pointers[0].button[1] == FALSE && input->pointers[0].last_button[1] == TRUE)
					lo_projection_cache_update(&la_world_matrix);
				seduce_primitive_line_focal_depth_set(seduce_view_distance_camera_get(NULL));
			}
		break;
	}

	empty_pop_up_return = la_pu_empty(input, ParseInputData.mode == PIM_SHOW_EMPTY_MENU);

	if(empty_pop_up_return == LA_PUER_OBJECT_SELECT)
		ParseInputData.mode = PIM_SHOW_OBJECT_SELECT;
	if(empty_pop_up_return == LA_PUER_SETTINGS)
		ParseInputData.mode = PIM_SHOW_SETTINGS;
	if(empty_pop_up_return == LA_PUER_RENAME)
		ParseInputData.mode = PIM_SHOW_RENAME;


	if(measure_edge[0] != ~0 && !la_popup_measure(input, measure_edge))
	{
		measure_edge[0] = measure_edge[1] = ~0;
		ParseInputData.mode = PIM_IDLE;
	}

	if(la_pu_object_selection(input, ParseInputData.mode == PIM_SHOW_OBJECT_SELECT))
		ParseInputData.mode = PIM_IDLE;

	
	if(!la_draw_settings_menu(input, ParseInputData.mode == PIM_SHOW_SETTINGS) && ParseInputData.mode == PIM_SHOW_SETTINGS)
		ParseInputData.mode = PIM_IDLE;
	if(!la_draw_rename_menu(input, ParseInputData.mode == PIM_SHOW_RENAME) && ParseInputData.mode == PIM_SHOW_RENAME)
		ParseInputData.mode = PIM_IDLE;

/*	if(la_pu_vertex(input, ParseInputData.start_vertex, ParseInputData.mode == PIM_SHOW_VERTEX_MENU))
		ParseInputData.mode = PIM_IDLE;*/

	if(!input->pointers[0].button[2] && input->pointers[0].last_button[2] && input->mode == BAM_EVENT)
	{
		int i;
		i = 0;
	}
	if(ParseInputData.mode != PIM_DRAW_SELECT)
		la_pu_select(input, FALSE);
	if(la_pu_edge(input, edge, ParseInputData.mode == PIM_SHOW_EDGE_MENU))
		ParseInputData.mode = PIM_IDLE;
	if(la_pu_manipulator(input, ParseInputData.mode == PIM_SHOW_MANIPULATOR_MENU))
		ParseInputData.mode = PIM_IDLE;
	if(la_pu_polygon(input, polygon, ParseInputData.mode == PIM_SHOW_POLY_MENU))
		ParseInputData.mode = PIM_IDLE;

	if(input->mode == BAM_EVENT && ParseInputData.mode == PIM_SHOW_EMPTY_MENU)
		if((input->pointers[0].button[2] == FALSE && input->pointers[0].last_button[2] == TRUE) || (input->pointers[0].button[0] == FALSE && input->pointers[0].last_button[0] == TRUE))
			ParseInputData.mode = PIM_IDLE;



	if(input->mode == BAM_EVENT && ParseInputData.mode != PIM_SHOW_OBJECT_SELECT && ParseInputData.mode != PIM_SHOW_SETTINGS && ParseInputData.mode != PIM_SHOW_RENAME && input->pointers[0].button[0] == FALSE && input->pointers[0].last_button[0] == FALSE && input->pointers[0].button[1] == FALSE && input->pointers[0].last_button[1] == FALSE && input->pointers[0].button[2] == FALSE && input->pointers[0].last_button[2] == FALSE)
	{
		undo_event_done();
		ParseInputData.mode = PIM_IDLE;
		ParseInputData.click_time = 0;
	}
//	ParseInputData.mode = PIM_CHANGE_VIEW;
}

extern void la_pu_browser(BInputState *input, uint node);

extern void la_inside_test(double *vertex, uint *ref, uint ref_length, uint vertex_count);

extern uint sui_3d_texture_reflection;
void sediuce_line_image_test();
void la_t_group_test();
void seduce_object_3d_test(BInputState *input);

void la_edit_func(BInputState *input, void *user)
{
	static boolean active = FALSE;
	if(input->mode == BAM_MAIN)
	{
		verse_callback_update(0);
		active = udg_update_geometry();
		seduce_view_update(NULL, input->delta_time);
	}
	if(input->mode == BAM_DRAW)
	{
		float view[3] = {0, 0, 1}, matrix[16], aspect;
		double dist;
		uint x, y;

		glClearColor(0, 0, 0, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		betray_view_vantage(view);
		aspect = betray_screen_mode_get(&x, &y, NULL);
		r_viewport(0, 0, x, y);
		r_matrix_identity(&la_interface_matrix);
		dist = seduce_view_distance_camera_get(NULL) * 0.001;
		r_matrix_frustum(&la_interface_matrix, -dist - view[0] * dist, dist - view[0] * dist, -dist * aspect - view[1] * dist, dist * aspect - view[1] * dist, dist * view[2], 100000.0 * dist); /* set frustum */
		la_world_matrix = la_interface_matrix;
		r_matrix_translate(&la_interface_matrix, -view[0], -view[1], -view[2]);

		betray_view_direction(matrix); 
		r_matrix_matrix_mult(&la_interface_matrix, matrix);
		seduce_view_set(NULL, &la_world_matrix);
		

		r_matrix_set(&la_world_matrix);
	/*	{
			double rand_x[3] = {1, 0.2, 0.6}, rand_y[3] = {-0.5, 0.4, -0.6};
			f_matrixxyd(la_axis_matrix, NULL, rand_x, rand_y);
			la_axis_matrix[0] = -la_axis_matrix[0];
			la_axis_matrix[1] = -la_axis_matrix[1];
			la_axis_matrix[2] = -la_axis_matrix[2];
			la_axis_matrix[8] = -la_axis_matrix[8];
			la_axis_matrix[9] = -la_axis_matrix[9];
			la_axis_matrix[10] = -la_axis_matrix[10];
		}
		r_primitive_line_3d(la_axis_matrix[0], la_axis_matrix[1], la_axis_matrix[2], 0, 0, 0, 1, 0, 0, 1);
		r_primitive_line_3d(la_axis_matrix[4], la_axis_matrix[5], la_axis_matrix[6], 0, 0, 0, 0, 1, 0, 1);
		r_primitive_line_3d(la_axis_matrix[8], la_axis_matrix[9], la_axis_matrix[10], 0, 0, 0, 0, 0, 1, 1);
		r_primitive_line_flush();*/
		la_do_owerlay(input);
		la_poly_csg();	
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		r_matrix_set(&la_world_matrix);
		r_primitive_line_flush(); 
		
		lo_t_hole_debug();
		la_t_group_draw(input);
		r_matrix_set(&la_interface_matrix);
		{
			r_matrix_set(NULL);
			r_matrix_identity(NULL);
			dist = seduce_view_distance_camera_get(NULL) * 0.001;
			r_matrix_frustum(NULL, -dist - view[0] * dist, dist - view[0] * dist, -dist * aspect - view[1] * dist, dist * aspect - view[1] * dist, dist * view[2], 100000.0 * dist); /* set frustum */
			r_matrix_translate(NULL, 0, 0, -1);	
			lo_projection_cache_draw();
		}
		
		seduce_object_3d_test(input);
		sediuce_line_image_test();	
	}	
	/*
if(input->mode == BAM_EVENT)
	seduce_view_change(NULL, input);/*
if(input->mode == BAM_DRAW)
{
	if(active)
	{
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0, 0, -1);
		seduce_text_line_draw(NULL, -0.5 * seduce_text_line_length(NULL, SEDUCE_T_SIZE * 2, 3, "SOME GEOMETRY", -1), 0, SEDUCE_T_SIZE * 2, 3, "SOME GEOMETRY", 1, 1, 1, 1, -1);
	}else
	{
		ENode *node;
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0, 0, -1);
		seduce_text_line_draw(NULL, -0.5 * seduce_text_line_length(NULL, SEDUCE_T_SIZE * 2, 3, "NO GEOMETRY", -1), 0, SEDUCE_T_SIZE * 2, 3, "NO GEOMETRY", 1, 1, 1, 1, -1);
		node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY);
		if(node != NULL)
			udg_set_modeling_node(e_ns_get_node_id(node));
	}
	glPopMatrix();
//	la_pfx_draw(FALSE);
}*/
	if(active)
	{
		la_parse_input(input);
	}else
	{
		ENode *node;
		if(input->mode == BAM_DRAW)
		{
			seduce_text_line_draw(NULL, -0.5 * seduce_text_line_length(NULL, SEDUCE_T_SIZE * 2, 3, "NO GEOMETRY", -1), 0, SEDUCE_T_SIZE * 2, 3, "NO GEOMETRY", 1, 1, 1, 1, -1);
			seduce_text_line_draw(NULL, -0.5 * seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Click to create a geometry node", -1), -0.1, SEDUCE_T_SIZE, SEDUCE_T_SPACE, "Click to create a geometry node", 1, 1, 1, 1, -1);
		}else
		{
			static boolean node_found = FALSE;
			if(input->pointers[0].button[0] == TRUE && input->pointers[0].last_button[0] == FALSE)
				udg_create_new_modeling_node();
			if(!node_found)
			{
				node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY);
				if(node != NULL)
				{
					udg_set_modeling_node(e_ns_get_node_id(node));
					node_found = TRUE;
				}
			}
		}
	}
/*	if(input->mode == BAM_DRAW)
	{
		uint32 vertex_count, i;
		double	*vertex;
		float	pos[3];
		udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);
		for(i = 0; i < vertex_count; i++)
		{
			if(vertex[i * 3] != V_REAL64_MAX)
			{
				r_matrix_projection_screenf(&la_world_matrix, pos, vertex[i * 3], vertex[i * 3 + 1], vertex[i * 3 + 2]);
				r_primitive_line_3d(pos[0] + 0.01, pos[1], 0, pos[0] - 0.01, pos[1], 0, 1, udg_get_select(i), 0, 1);	
				r_primitive_line_3d(pos[0], pos[1] + 0.01, 0, pos[0], pos[1] - 0.01, 0, 1, udg_get_select(i), 0, 1);	
			}
		}
		r_primitive_line_flush();
		glEnable(GL_DEPTH_TEST);
	}*/
	if(input->mode == BAM_DRAW)
	{
/*		ENode *node;
		node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY);
		if(node != NULL)
		{
			uint texture_id;
			texture_id = lo_preview_render(-1, node);
			r_primitive_image(-0.5, -0.5, -1, 1, 1, 0, 0, 1, 1, texture_id, 1, 1, 1, 1);
			r_texture_free(texture_id);	
		}*/
		lo_pfx_draw_local_space(input);
		la_pfx_test(input->delta_time);
		la_pfx_sparks(input->delta_time);
		la_pfx_manipulator_flare();


		lo_preview_update();
	/*	{
			SeduceBackgroundObject *obj;
			uint i;
			static uint texture_id = -1;
			float x, y;
			if(texture_id == -1)
				texture_id = seduce_line_image_gen(1024);
			obj = seduce_background_object_allocate();
			x = input->pointers[0].pointer_x; 
			y = input->pointers[0].pointer_y;
			seduce_background_square_add(obj, NULL, 0,
											-0.2, -4.0, 0, 
											0.4, 8.0,
											0.8, 0.8, 0.8, 0.6);											
			seduce_background_square_add(obj, &input->pointers[0].pointer_y, 0,
											-0.16, 0.0, 0, 
											0.32, 0.4,
											0.6, 1.0, 0.2, 0.8);
			seduce_background_square_add(obj, &input->pointers[0].pointer_z, 0,
											-0.16, -0.4, 0, 
											0.32, 0.4,
											0.2, 0.6, 1.0, 0.8);



			seduce_background_shadow_square_add(obj, -0.16, -0.4, 0.32, 0.8, 0.025);

			seduce_primitive_surface_draw(input, obj, 1);
			seduce_primitive_background_object_free(obj);
			seduce_background_image_draw(input, NULL, -0.2, -0.4, 0, 0.4, 0.8, 0, 0, 1, 1, 1, NULL, texture_id);
			
		}*/
	}
	seduce_element_endframe(input, FALSE);
}
