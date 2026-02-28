#define SEDUCE_GEO_PREVIEW_DEFINE 1

#include "la_includes.h"

#include "la_geometry_undo.h"
#include "la_tool.h"
#include "la_projection.h"

#include "la_pop_up.h"
#include "hxa_enough.h"
/*
struct{
	void	*ring;
	void	*empty_star;
	void	*manipulator_star;
	void	*edge_star;
	void	*polygon_star;
	void	*browser_star;
	void	*select_star;
	char	text[48];
	void	*text_button; 
	void	*slider;
	void	*complexity_slider;
	void	*treshold_slider;
	char	*mouse[3];
	float	mouse_pos[3];
	float	popup_pos[3];
}GlobalPopupData;*/

void la_intro_draw(void *user);
void la_geometry_load_obj(char *file);
void la_geometry_save_obj(char *file);


void connect_type_in_func(void *user, char *text)
{
	e_vc_connect(text, "unknown", "none", NULL);
}

extern void la_edit_func(BInputState *input, void *user);

extern void geometry_save(void);
extern void geometry_load(char *file);

extern void start_macro_record(char *file);
extern void start_macro_play_back(char *file);
extern void stop_macro(void);

extern void save_node_vnf(ENode *node, uint *node_count, FILE *file);
extern void test_ref_creator(void);
extern void draw_settings_menu(BInputState *input, void *user);

void la_geometry_load_obj(char *file);
uint lo_preview_texture_get(ENode *node);
void lo_preview_texture_get_pos(ENode *node, float *pos);
void lo_preview_texture_set_pos(ENode *node, float *pos);

boolean la_pu_object_selection(BInputState *input, boolean active)
{
	float center[3] = {0, 0, 0}, pos[3], aspect, color[3] = {1, 0.0, 0.2};
	static float time = 0, slider_pos = 1, slider = 20.0, scroll;
	static uint row_count = 20.0;
	static char search[64];
	static uint grab_id = -1;
 	uint i = 0, texture_id, count = 0;
	float size = 0.5, f, delta, text_size, text_length;
	char *name;
	ENode *node;
	seduce_animate(input, &time, active, 1.0);
	if(time < 0.01)
		return FALSE;
	aspect = betray_screen_mode_get(NULL, NULL, NULL);

	for(node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY); node != NULL; node = e_ns_get_node_next(e_ns_get_node_id(node) + 1, 0, V_NT_GEOMETRY))
		if(lo_preview_texture_get(node) != -1)
			count++;
	center[0] = input->pointers[0].pointer_x;
	center[1] = input->pointers[0].pointer_y;

	if(input->mode == BAM_EVENT)
		if(!input->pointers[0].button[0] && !input->pointers[0].last_button[0])
			grab_id = -1;

	delta = input->delta_time * 5.0;
	if(delta > 1.0)
		delta = 1.0;
	for(node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY); node != NULL; node = e_ns_get_node_next(e_ns_get_node_id(node) + 1, 0, V_NT_GEOMETRY))
	{
		texture_id = lo_preview_texture_get(node);
		if(texture_id != -1 && e_search_node(node, search))
		{
			if(input->mode == BAM_DRAW)
			{
				lo_preview_texture_get_pos(node, pos);
				pos[0] = pos[0] * (1 - delta) + delta * ((float)(i % row_count) * 2.0 / (float)row_count - 1.0);
				pos[1] = pos[1] * (1 - delta) + delta * ((float)(i / row_count) * -2.0 / (float)row_count + aspect - (2 / (float)row_count));
				pos[2] = pos[2] * (1 - delta) + delta * (2.0 / (float)row_count);
				lo_preview_texture_set_pos(node, pos);
				seduce_background_image_draw(input, node, pos[0], pos[1] + scroll, 0, pos[2], pos[2], 0, 0, 1, 1, time, center, texture_id);
				name = e_ns_get_node_name(node);
				text_length = seduce_text_line_length(NULL, SEDUCE_T_SIZE, SEDUCE_T_SPACE, name, -1);
				text_size = 1.0;
				if(text_length > pos[2] * 0.9)
					text_size = pos[2] * 0.9 / text_length;
		//		seduce_text_line_draw(NULL, pos[0] + pos[2] * 0.5 - 0.5 * text_length * text_size, pos[1] + SEDUCE_T_SIZE * 0.5, text_size * SEDUCE_T_SIZE, SEDUCE_T_SPACE, name, 1, 1, 1, 1, -1);
					
			}
			if(input->mode == BAM_EVENT)
			{
				if(node == seduce_element_pointer_id(input, 0, NULL))
				{
					if(input->pointers[0].button[0] && !input->pointers[0].last_button[0])
					{
						grab_id = e_ns_get_node_id(node);
					}
				}

				if(grab_id == e_ns_get_node_id(node) && !input->pointers[0].button[0] && input->pointers[0].last_button[0])
				{
					scroll = 0;
					search[0] = 0;
					udg_set_modeling_node(e_ns_get_node_id(node));

					return TRUE;
				}
			}
			i++;
		}
	}


	if(input->mode == BAM_EVENT)
	{
		if(input->pointers[0].button[0])
		{
			scroll += input->pointers[0].delta_pointer_y; 
			f = input->pointers[0].click_pointer_y[0] - input->pointers[0].pointer_y;
			if(f > 0.05 || f < -0.5)
				grab_id = -1;
		}

		if(betray_button_get(-1, BETRAY_BUTTON_SCROLL_UP))
			scroll += (2.0 / (float)row_count);
		if(betray_button_get(-1, BETRAY_BUTTON_SCROLL_DOWN))
			scroll -= (2.0 / (float)row_count);
		f = ((float)(i / row_count) * -2.0 / (float)row_count + aspect - (2 / (float)row_count)) - (2.0 / (float)row_count);
	//	if(f + scroll > -aspect)
	//		scroll = aspect - f;
		if(f + scroll > -aspect)
			scroll = -(f + aspect);
		if(scroll < 0)
			scroll = 0;
	}
	if(input->mode == BAM_DRAW)
	{
		SeduceBackgroundObject *object;
	/*	seduce_background_quad_draw(input, &grab_id, 0,
											-1.05, -1.05 * aspect, -0.05, 
											1.05, -1.05 * aspect, -0.05, 
											1.05, -1.05 * (aspect - 0.05), -0.05, 
											-1.05, -1.05 * (aspect - 0.05), -0.05, 
											0, 0, 1,
											0.5, 0.5, 0.5, 0.7);*/

		object = seduce_background_object_allocate();
		seduce_background_square_add(object, NULL, 0,
									-1.05, -1.05 * aspect, -0.05,
									2.1, 0.05,
									0.5, 0.5, 0.5, 0.7);
		seduce_primitive_surface_draw(input, object, time);

		seduce_background_polygon_flush(input, center, time);
		seduce_primitive_background_object_free(object);
	}

	if(seduce_widget_button_icon(input, &time, SEDUCE_OBJECT_CLOSE, 1 - 0.025, -aspect + 0.025,  0.025, time, color))
	{		

		scroll = 0;
		search[0] = 0;
		return TRUE;
	}

	seduce_widget_slider_line(input, &row_count, &slider, 0, -aspect + 0.025, 1 - 0.05, -aspect + 0.025, 0.035, 0.1, 1.0, 40, time, color, FALSE);
	row_count = slider + 0.5;
	if(row_count < 1)
		row_count = 1;
	seduce_text_edit_line(input, &search, NULL, search, 64, 0.025 - 1.0, -aspect + 0.0125, 1, 0.025 * 0.5, seduce_translate("Search"), TRUE, NULL, NULL, 0.2, 0.6, 1.0, 1, 1, 0, 0, 1);
	return FALSE;
}


extern void la_geometry_load_hxa(char *file_name);
extern void hxa_util_triangulate_node(HXANode *node, unsigned int max_sides);
extern float *hxa_unreferenced_vertex_array_build(HXANode *node, char **names, unsigned char *component_ids, float *defaults, unsigned intcomponent_count);
extern void hxa_util_normal_corner(HXANode *node);

LAPUEmptyReturn la_pu_empty(BInputState *input, boolean active)
{
	static SUIPUElement *element = NULL;
	static uint element_allocations = 0;
	static float x, y, static_start = 0, rotate = 0, time = 0;
	static char *types[2] = {"obj", "hxa"};
	float brightness = 0.5, tmp[2], aspect;
	BInputState fake_input;
	BInputPointerState *pointers;  
	uint start = 0, ring, count = 4, i, j, layer_count = 0, texture_id;
	ENode *node;
	EGeoLayer *layer;
	char *path;

	seduce_animate(input, &time, active, 4.0); 
	path = betray_requester_save_get(&element[1]);
	node = e_ns_get_node(0, udg_get_modeling_node());
	if(path != NULL)
	{
		if(f_text_filter_case_insensitive(path, ".array"))
		{
		/*	FILE* file;
			float defaults[6] = {0, 0, 0, 0, 0, 1}, *array;
			unsigned char component_ids[6] = {0, 1, 2, 0, 1, 2};
			char *names[6] = {HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_HARD_BASE_VERTEX_LAYER_NAME, HXA_CONVENTION_SOFT_LAYER_NORMALS, HXA_CONVENTION_SOFT_LAYER_NORMALS, HXA_CONVENTION_SOFT_LAYER_NORMALS};
			HXANode *hxa_node;
			uint i;
			hxa_node = malloc(sizeof *hxa_node);
			enough_to_hxa_node_geometry(node, hxa_node);
			hxa_util_triangulate_node(hxa_node, 3);
			hxa_util_normal_corner(hxa_node);
			array = hxa_unreferenced_vertex_array_build(hxa_node, names, component_ids, defaults, 6);
			file = fopen(path, "w");
			if(file != NULL)
			{
				fprintf(file, "float array[%u * 6] = {%f, %f, %f, %f, %f, %f\n", hxa_node->content.geometry.edge_corner_count, array[0], array[1], array[2], array[3], array[4], array[5]);
				for(i = 1; i < hxa_node->content.geometry.edge_corner_count; i++)
					fprintf(file, "\t\t, %f, %f, %f, %f, %f, %f\n", array[i * 6], array[i * 6 + 1], array[i * 6 + 2], array[i * 6 + 3], array[i * 6 + 4], array[i * 6 + 5]);
				fprintf(file, "};\n");
				fclose(file);
			}*/
		}
		else if(f_text_filter_case_insensitive(path, ".hxa"))
			enough_to_hxa_one(path, node);
		else
			la_geometry_save_obj(path);
	}
	path = betray_requester_save_get(&element[2]);
	if(path != NULL)
		enough_to_hxa_all(path);
	path = betray_requester_load_get(element);
	if(path != NULL)
	{
		if(f_text_filter_case_insensitive(path, ".hxa"))
			la_geometry_load_hxa(path);
		else
			la_geometry_load_obj(path);
	} 


	if(time < 0.01)
		return LA_PUER_NONE;
	
	node = e_ns_get_node(0, udg_get_modeling_node());
	if(node != NULL	&& V_NT_GEOMETRY == e_ns_get_node_type(node))
		for(layer = e_nsg_get_layer_next(node, 0); layer != NULL; layer = e_nsg_get_layer_next(node, e_nsg_get_layer_id(layer) + 1))
			if(VN_G_LAYER_VERTEX_XYZ == e_nsg_get_layer_type(layer))
				layer_count++;

	if((layer_count + 10) > element_allocations)
	{
		element_allocations = (layer_count + 10);
		if(element != NULL)
			free(element);
		element = malloc((sizeof *element) * element_allocations);
	}

	aspect = betray_screen_mode_get(NULL, NULL, NULL);
	start = static_start;
	element[0].type = S_PUT_TOP;
	element[0].text = seduce_translate("Settings");
	element[1].type = S_PUT_TOP;
	element[1].text = seduce_translate("Add Layer");
	element[2].type = S_PUT_TOP;
	element[2].text = seduce_translate("Load");
	element[3].type = S_PUT_TOP;
	element[3].text = seduce_translate("Save");
	element[4].type = S_PUT_TOP;
	element[4].text = seduce_translate("Save All");	
	element[5].type = S_PUT_TOP;
	element[5].text = seduce_translate("Rename");
	element[6].type = S_PUT_TOP;
	element[6].text = seduce_translate("New");
	element[7].type = S_PUT_ANGLE;
	element[7].text = seduce_translate("Undo");
	element[7].data.angle[0] = 45;
	element[7].data.angle[1] = 135;
	element[8].type = S_PUT_ANGLE;
	element[8].text = seduce_translate("Redo");
	element[8].data.angle[0] = 225;
	element[8].data.angle[1] = 315;
	element[9].type = S_PUT_ANGLE;
	element[9].text = seduce_translate("Select Geometry");
	element[9].data.angle[0] = 135;
	element[9].data.angle[1] = 225;
	count = 10;
	i = 0;

	node = e_ns_get_node(0, udg_get_modeling_node());
	if(node != NULL	&& V_NT_GEOMETRY == e_ns_get_node_type(node))
	{
		for(layer = e_nsg_get_layer_next(node, 0); layer != NULL; layer = e_nsg_get_layer_next(node, e_nsg_get_layer_id(layer) + 1))
		{
			if(VN_G_LAYER_VERTEX_XYZ == e_nsg_get_layer_type(layer))
			{
				element[count].type = S_PUT_TOP;
				element[count].text = e_nsg_get_layer_name(layer);
				count++;
			}
		}
	}


	rotate += betray_time_delta_get() * 20;


	if(input->pointers[0].button[2] == TRUE)
	{
		x = input->pointers[0].click_pointer_x[2];
		y = input->pointers[0].click_pointer_y[2];
	} 
	if(input->mode == BAM_DRAW)	
	{
		r_matrix_push(NULL);
		r_matrix_translate(NULL, x, y, 0);
	}


	fake_input = *input;
	fake_input.pointers = pointers = malloc((sizeof *pointers) * input->pointer_count);
	for(i = 0; i < input->pointer_count; i++)
	{
		pointers[i] = input->pointers[i];
		pointers[i].button[0] = !pointers[i].button[2];
		pointers[i].last_button[0] = !pointers[i].last_button[2];
	}
	ring = seduce_popup(&fake_input, element, element, count, time, TRUE);
	free(pointers);
	if(input->mode == BAM_DRAW)	
		r_matrix_pop(NULL);
	switch(ring)
	{
		case 0 :
			return LA_PUER_SETTINGS;
		break;
		case 1 :
			udg_create_new_modeling_layer();
		break;
		case 2 :
			betray_requester_load(types, 2, element); 
		break;
		case 3 :
			betray_requester_save(types, 2, &element[1]);
		break;
		case 4 :
			betray_requester_save(types, 2, &element[2]);
		break;
		case 5 :
			return LA_PUER_RENAME;
		break;
		case 6 :
			udg_create_new_modeling_node();
			return LA_PUER_RENAME;
		break;
		case 7 :
			udg_undo_geometry();
		break;
		case 8 :
			udg_redo_geometry();
		break;
		case 9 :
			return LA_PUER_OBJECT_SELECT;
		break;
	}
	count = 10;
	if(ring >= count && node != NULL && V_NT_GEOMETRY == e_ns_get_node_type(node))
	{
		
		for(layer = e_nsg_get_layer_next(node, 0); layer != NULL; layer = e_nsg_get_layer_next(node, e_nsg_get_layer_id(layer) + 1))
		{
			if(VN_G_LAYER_VERTEX_XYZ == e_nsg_get_layer_type(layer))
			{
				if(count == ring)
					udg_set_modeling_layer(e_nsg_get_layer_id(layer));
				count++;
			}
		}
	}
	return LA_PUER_NONE;
/*	if(input->mode == BAM_DRAW)
	{
		if(preview)
			e_ns_set_custom_func(0, V_NT_GEOMETRY, lo_geo_preview_func);
		glPushMatrix();
		glTranslatef(-0.8 + 0.9 / 16.0, y - 0.06, 0);
		glScalef(1.6 / (16.0 * 1.6), 1.6 / (16.0 * 1.6), 1.6 / (16.0 * 1.6));
		i = 1;
		glPushMatrix();
		tmp[0] = -(input->pointers[0].pointer_x + 0.8 - 1.0 / 16.0) / (1.6 / (16.0 * 1.6));
		tmp[1] =  -(-input->pointers[0].pointer_y + y - 0.06) / (1.6 / (16.0 * 1.6));
		brightness = 0.5;
		if(tmp[0] * tmp[0] + tmp[1] * tmp[1] < 0.8 * 0.8)
			brightness = 1.0;
		seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE / 2.0 * (16.0 * 1.6), SEDUCE_T_SPACE, "NEW", -1) * -0.5, 0, SEDUCE_T_SIZE / 2.0 * (16.0 * 1.6), SEDUCE_T_SPACE, "NEW", brightness, brightness, brightness, brightness, -1);
		glPopMatrix();
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		for(node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY); node != NULL; node = e_ns_get_node_next(e_ns_get_node_id(node) + 1, 0, V_NT_GEOMETRY))
		{
			if((float)(i / 16) * 1.6 * (1.6 / (16.0 * 1.6)) - y > aspect + 0.2)
				break;
			if((float)(i / 16) * 1.6 * (1.6 / (16.0 * 1.6)) - y > -aspect - 0.2)
			{
				glPushMatrix();
				glTranslatef((float)(i % 16) * 1.6, -1.6 * (float)(i / 16), 0);
				tmp[0] = (float)(i % 16) * 1.6 - (input->pointers[0].pointer_x + 0.8 - 1.0 / 16.0) / (1.6 / (16.0 * 1.6));
				tmp[1] = (float)(i / 16) * 1.6 - (-input->pointers[0].pointer_y + y - 0.06) / (1.6 / (16.0 * 1.6));
				brightness = 0.1;
				if(preview)
				{
					e_ns_set_custom_data(node, 0, lo_preview_geometry_update(node, e_ns_get_custom_data(node, 0)));
					if(tmp[0] * tmp[0] + tmp[1] * tmp[1] < 0.8 * 0.8)
					{
						brightness = 0.4;
						seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE / 2.0 * (16.0 * 1.6), SEDUCE_T_SPACE, e_ns_get_node_name(node), -1) * -0.5, - 0.8, SEDUCE_T_SIZE / 2.0 * (16.0 * 1.6), SEDUCE_T_SPACE, e_ns_get_node_name(node), 1.0, 1.0, 1.0, 1.0, -1);
					}
					glRotatef(rotate, 0, 1, 0);
					if(!lo_preview_geometry_draw(e_ns_get_custom_data(node, 0), brightness))
						seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE / 2.0 * (16.0 * 1.6), SEDUCE_T_SPACE, "EMPTY", -1) * -0.5, 0, SEDUCE_T_SIZE / 2.0 * (16.0 * 1.6), SEDUCE_T_SPACE, "EMPTY", brightness, brightness, brightness, brightness, -1);
				}else
				{
					if(tmp[0] * tmp[0] + tmp[1] * tmp[1] < 0.8 * 0.8)
						brightness = 0.4;
					seduce_text_line_draw(NULL, seduce_text_line_length(NULL, SEDUCE_T_SIZE / 3.0 * (16.0 * 1.6), SEDUCE_T_SPACE, e_ns_get_node_name(node), -1) * -0.5, 0, SEDUCE_T_SIZE / 3.0 * (16.0 * 1.6), SEDUCE_T_SPACE, e_ns_get_node_name(node), brightness, brightness, brightness, brightness, -1);
				}

				glPopMatrix();
			}
			i++;
		}
		if(input->pointers[0].pointer_y > aspect * 0.9 && y > aspect * 0.8)
			y -= betray_time_delta_get() * 2;
		if(input->pointers[0].pointer_y < aspect * -0.9 && aspect * 0.8 / (1.6 / (16.0 * 1.6)) < (float)(i / 16) * 1.6 - (y - 0.06) / (1.6 / (16.0 * 1.6)))
			y += betray_time_delta_get() * 2;
		glPopMatrix();

		glPopMatrix();
		glPushMatrix();
		glPushMatrix();
		glEnable(GL_DEPTH_TEST);
	}else if(input->pointers[0].button[2] == FALSE && input->pointers[0].last_button[2] == TRUE)
	{
		tmp[0] = -(input->pointers[0].pointer_x + 0.8 - 1.0 / 16.0) / (1.6 / (16.0 * 1.6));
		tmp[1] = -(-input->pointers[0].pointer_y + y - 0.06) / (1.6 / (16.0 * 1.6));
		if(tmp[0] * tmp[0] + tmp[1] * tmp[1] < 0.8 * 0.8)
			udg_create_new_modeling_node();
		i = 1;
		for(node = e_ns_get_node_next(0, 0, V_NT_GEOMETRY); node != NULL; node = e_ns_get_node_next(e_ns_get_node_id(node) + 1, 0, V_NT_GEOMETRY))
		{
			tmp[0] = (float)(i % 16) * 1.6 - (input->pointers[0].pointer_x + 0.8 - 1.0 / 16.0) / (1.6 / (16.0 * 1.6));
			tmp[1] = (float)(i / 16) * 1.6 - (-input->pointers[0].pointer_y + y - 0.06) / (1.6 / (16.0 * 1.6));
			if(tmp[0] * tmp[0] + tmp[1] * tmp[1] < 0.8 * 0.8)
			{
				udg_set_modeling_node(e_ns_get_node_id(node));
				lo_preview_geometry_center(e_ns_get_custom_data(node, 0));
			}
			i++;
		}
	}
	if(ring > 3)
		return TRUE;
	else
		return FALSE;*/
}

void la_pu_vertex(BInputState *input, uint vertex, boolean active)
{	
/*	static double value;
	double pos[3], output[3];
	glDisable(GL_DEPTH_TEST);
	glPopMatrix();
	glPopMatrix();
	glPushMatrix();
	udg_get_vertex_pos(pos, vertex);
	p_get_projection_screen(output, pos[0], pos[1], pos[2]);
	glTranslatef(-output[0], -output[1], -1);
	input->pointers[0].pointer_x += output[0];
	input->pointers[0].pointer_y += output[1];
	input->pointers[0].click_pointer_x[0] += output[0];
	input->pointers[0].click_pointer_y[0] += output[2];
	input->pointers[0].pointer_x -= output[0];
	input->pointers[0].pointer_y -= output[1];
	input->pointers[0].click_pointer_x[0] -= output[0];
	input->pointers[0].click_pointer_y[0] -= output[2];
	udg_set_select(vertex, value);
	glPopMatrix();
	glPushMatrix();
	glPushMatrix();
	glEnable(GL_DEPTH_TEST);*/
}

void me_create_tree();

boolean la_pu_manipulator(BInputState *input, boolean active)
{
	static BInputState fake_input;
	BInputPointerState *pointers;  
	static SUIPUElement element[29];
	static float x, y, time = 0;
	uint ring, i, j;

	seduce_animate(input, &time, active, 4.0);
	
	if(time < 0.01)
		return FALSE;

	element[0].type = S_PUT_ANGLE;
	element[0].text = seduce_translate("Reverse Selection");
	element[0].data.angle[0] = 45;
	element[0].data.angle[1] = 75;
	element[1].type = S_PUT_ANGLE;
	element[1].text = seduce_translate("Duplicate");
	element[1].data.angle[0] = 75;
	element[1].data.angle[1] = 105;
	element[2].type = S_PUT_ANGLE;
	element[2].text = seduce_translate("Detach");
	element[2].data.angle[0] = 105;
	element[2].data.angle[1] = 135;
	element[3].type = S_PUT_ANGLE;
	element[3].text = seduce_translate("Collapse");
	element[3].data.angle[0] = 45 + 180;
	element[3].data.angle[1] = 75 + 180;
	element[4].type = S_PUT_ANGLE;
	element[4].text = seduce_translate("Full Crease");
	element[4].data.angle[0] = 75 + 180;
	element[4].data.angle[1] = 105 + 180;
	element[5].type = S_PUT_ANGLE;
	element[5].text = seduce_translate("No Crease");
	element[5].data.angle[0] = 105 + 180;
	element[5].data.angle[1] = 135 + 180;

	element[6].type = S_PUT_TOP;
	element[6].text = seduce_translate("Flip");
	element[7].type = S_PUT_TOP;
	element[7].text = seduce_translate("Smooth Selection");
	element[8].type = S_PUT_TOP;
	element[8].text = seduce_translate("Poly gone");
	element[9].type = S_PUT_TOP;
	element[9].text = seduce_translate("Mirror");

	element[10].type = S_PUT_TOP;
	element[10].text = seduce_translate("Flatten");
	element[11].type = S_PUT_TOP;
	element[11].text = seduce_translate("Slice");
	element[12].type = S_PUT_TOP;
	element[12].text = seduce_translate("Slice Off");
	element[13].type = S_PUT_TOP;
	element[13].text = seduce_translate("Weld");
	element[14].type = S_PUT_TOP;
	element[14].text = seduce_translate("Center Geometry");
	element[15].type = S_PUT_BOTTOM;
	element[15].text = seduce_translate("Center Manipulator");
	element[16].type = S_PUT_BOTTOM;
	element[16].text = seduce_translate("Create Tag");
	element[17].type = S_PUT_BOTTOM;
	element[17].text = seduce_translate("Triangulate");
	element[18].type = S_PUT_BOTTOM;
	element[18].text = seduce_translate("Auto Crease");
	element[19].type = S_PUT_BOTTOM;
	element[19].text = seduce_translate("Find Quads");

	element[20].type = S_PUT_BOTTOM;
	element[20].text = seduce_translate("Cut");
	element[21].type = S_PUT_BOTTOM;
	element[21].text = seduce_translate("Copy");
	element[22].type = S_PUT_BOTTOM;
	element[22].text = seduce_translate("Paste");
	element[23].type = S_PUT_BOTTOM;
	element[23].text = seduce_translate("Copy to new");
	element[24].type = S_PUT_BOTTOM;
	element[24].text = seduce_translate("Revert to base");
	element[25].type = S_PUT_BOTTOM;
	element[25].text = seduce_translate("Vertex delete");
	element[26].type = S_PUT_BOTTOM;
	element[26].text = seduce_translate("Model Cleanup");
	element[27].type = S_PUT_BOTTOM;
	element[27].text = seduce_translate("Symmetry");
	element[28].type = S_PUT_BOTTOM;
	element[28].text = seduce_translate("Reset Axis Space");

	if(input->pointers[0].button[2] == TRUE)
	{
		x = input->pointers[0].click_pointer_x[2];
		y = input->pointers[0].click_pointer_y[2];
	}

	if(input->mode == BAM_DRAW)	
	{
		r_matrix_push(NULL);
		r_matrix_translate(NULL, x, y, 0);
	}

	fake_input = *input;
	fake_input.pointers = pointers = malloc((sizeof *pointers) * input->pointer_count);
	for(i = 0; i < input->pointer_count; i++)
	{
		pointers[i] = input->pointers[i];
		pointers[i].button[0] = !pointers[i].button[2];
		pointers[i].last_button[0] = !pointers[i].last_button[2];
	}
	ring = seduce_popup(&fake_input, element, element, 29, time, TRUE);
	free(pointers);
	if(input->mode == BAM_DRAW)	
		r_matrix_pop(NULL);
//	ring = -1;//s_popup(input, x, y, element, 28, &time, active, 1, 1, 1, 0.2, 0.6, 1.0);
	if(active)
	{
		switch(ring)
		{
			case 0 :
			{
				uint32 i, vertex_count;
				double *vertex;
				udg_get_geometry(&vertex_count, NULL, &vertex, NULL, NULL);
				for(i = 0; i < vertex_count; i++)
					if(vertex[i * 3] != V_REAL64_MAX)
						udg_set_select(i, 1 - udg_get_select(i));
			}
			return TRUE;
			case 1 :
			//	me_create_tree();
				la_t_duplicate_selected_polygons();
			return TRUE;
			case 2 :
			//	me_create_tree();
				la_t_detach_selected_polygons();
			return TRUE;
			case 3 :
			//	me_create_tree();
				la_t_collapse_selected_vertexes();
			return TRUE;
			case 4 :
				la_t_crease_selected(-1);
			return TRUE;
			case 5 :
				la_t_crease_selected(0);
			return TRUE;
			case 6 :
				la_t_flip_selected_polygons();
			return TRUE;
			case 7 :
				la_t_smooth_select();
			return TRUE;
			case 8 :
				la_t_delete_selection();
			return TRUE;
			case 9 :
			{
				double pos[3], vector[3];
				la_t_tm_get_pos(pos);
				la_t_tm_get_vector(vector);
				la_t_mirror(pos, vector);
				la_t_flip_selected_polygons();
			}
			return TRUE;
			case 10 :
			{
				double pos[3], vector[3];
				la_t_tm_get_pos(pos);
				la_t_tm_get_vector(vector);
				la_t_flatten(pos, vector);
			}
			return TRUE;
			case 11 :
			{
				double pos[3], vector[3];
				la_t_tm_get_pos(pos);
				la_t_tm_get_vector(vector);
				la_t_slice(pos, vector, FALSE);
			}
			return TRUE;
			case 12 :
			{
				double pos[3], vector[3];
				la_t_tm_get_pos(pos);
				la_t_tm_get_vector(vector);
				la_t_slice(pos, vector, TRUE);
			}
			return TRUE;
			case 13 :
			{
				la_t_weld_selected_vertexes();
			}
			return TRUE;
			case 14 :
			{
				la_t_center_geometry();
			}
			return TRUE;
			case 15 :
			{
				la_t_center_manipulator();
			}
			return TRUE;
			case 16 :
			{
				double pos[3];
				la_t_tm_get_pos(pos);
				udg_create_tag(pos);
			}
			return TRUE;
			case 17 :
			{
				la_t_poly_triangulate();
			}
			return TRUE;
			case 18 :
			{
				la_t_poly_auto_crease();
			}
			return TRUE;
			case 19 :
			{
				la_t_poly_find_quads();
			}
			return TRUE;
			case 20 :
			{
				double pos[3];
				la_t_tm_get_pos(pos);
				la_t_copy(pos);
				la_t_delete_selection();
			}
			return TRUE;
			case 21 :
			{
				double pos[3];
				la_t_tm_get_pos(pos);
				la_t_copy(pos);

			}
			break;
			case 22 :
			{
				double pos[3];
				la_t_tm_get_pos(pos);
				la_t_paste(pos);
			}
			return TRUE;
			case 23 :
				la_t_copy_to_new_geometry();
			return TRUE;
			case 24 :
				la_t_revert_to_base();
			return TRUE;
			case 25 :
				la_t_model_vertex_delete();
			return TRUE;
			case 26 :
				la_t_model_cleanup();
			return TRUE;
			case 27 :
			{
				double pos[3], vector[3];
				la_t_tm_get_pos(pos);
				la_t_tm_get_vector(vector);
				la_t_symmetry(pos, vector);
			}
			return TRUE;
			case 28 :
				la_view_axis_matrix_reset();
			return TRUE;			
		}
	}
	return FALSE;
}

boolean la_pu_edge(BInputState *input, uint *edge, boolean active)
{
	BInputState fake_input;
	BInputPointerState *pointers;  
	static SUIPUElement element[10];
	static float x, y, time = 0;
	uint ring, i, j;


	seduce_animate(input, &time, active, 4.0);
	
	if(time < 0.01)
		return FALSE;

	element[0].type = S_PUT_ANGLE;
	element[0].text = seduce_translate("Select Border");
	element[0].data.angle[0] = 0;
	element[0].data.angle[1] = 360 / 10.0 * 1.0;
	element[1].type = S_PUT_ANGLE;
	element[1].text = seduce_translate("Revolve");
	element[1].data.angle[0] = 360 / 10.0 * 1.0;
	element[1].data.angle[1] = 360 / 10.0 * 2.0;
	element[2].type = S_PUT_ANGLE;
	element[2].text = seduce_translate("Tube");
	element[2].data.angle[0] = 360 / 10.0 * 2.0;
	element[2].data.angle[1] = 360 / 10.0 * 3.0;
	
	element[3].type = S_PUT_ANGLE;
	element[3].text = seduce_translate("Wrap Around");
	element[3].data.angle[0] = 360 / 10.0 * 3.0;
	element[3].data.angle[1] = 360 / 10.0 * 4.0;

	element[4].type = S_PUT_ANGLE;
	element[4].text = seduce_translate("Select Hull");
	element[4].data.angle[0] = 360 / 10.0 * 4.0;
	element[4].data.angle[1] = 360 / 10.0 * 5.0;
	element[5].type = S_PUT_ANGLE;
	element[5].text = seduce_translate("Collapse");
	element[5].data.angle[0] = 360 / 10.0 * 5.0;
	element[5].data.angle[1] = 360 / 10.0 * 6.0;
	element[6].type = S_PUT_ANGLE;
	element[6].text = seduce_translate("Measure Grid");
	element[6].data.angle[0] = 360 / 10.0 * 6.0;
	element[6].data.angle[1] = 360 / 10.0 * 7.0;
	element[7].type = S_PUT_ANGLE;
	element[7].text = seduce_translate("Select Fill");
	element[7].data.angle[0] = 360 / 10.0 * 7.0;
	element[7].data.angle[1] = 360 / 10.0 * 8.0;
	element[8].type = S_PUT_ANGLE;	
	element[8].text = seduce_translate("Define Axis Space");
	element[8].data.angle[0] = 360.0 / 10.0 * 8.0;
	element[8].data.angle[1] = 360.0 / 10.0 * 9.0;
	element[9].type = S_PUT_ANGLE;	
	element[9].text = seduce_translate("Strip fill Hole");
	element[9].data.angle[0] = 360.0 / 10.0 * 9.0;
	element[9].data.angle[1] = 360.0 / 10.0 * 10.0;

	if(input->pointers[0].button[2] == TRUE)
	{
		x = input->pointers[0].click_pointer_x[2];
		y = input->pointers[0].click_pointer_y[2];
	}
	if(input->mode == BAM_DRAW)	
	{
		r_matrix_push(NULL);
		r_matrix_translate(NULL, x, y, 0);
	}

	fake_input = *input;
	fake_input.pointers = pointers = malloc((sizeof *pointers) * input->pointer_count);
	for(i = 0; i < input->pointer_count; i++)
	{
		pointers[i] = input->pointers[i];
		pointers[i].button[0] = !pointers[i].button[2];
		pointers[i].last_button[0] = !pointers[i].last_button[2];
		pointers[i].button[2] = FALSE;
		pointers[i].last_button[2] = FALSE;
	}
	ring = seduce_popup(&fake_input, element, element, 10, time, TRUE);
	free(pointers);
	if(input->mode == BAM_DRAW)	
		r_matrix_pop(NULL);
	if(active)
	{
		switch(ring)
		{
			case 0 :
				la_t_select_open_edge();
			return TRUE;
			case 1 :
				la_t_revolve(edge, imagine_setting_integer_get("REVOLVE_INTERSECTIONS", 8, "Revolve  uintersection count."));
			return TRUE;
			case 2 :
				la_t_tube(edge, imagine_setting_integer_get("REVOLVE_INTERSECTIONS", 8, "Revolve  uintersection count."));
			return TRUE;
			case 3 :
				la_t_wrap_around(edge, imagine_setting_integer_get("REVOLVE_INTERSECTIONS", 8, "Revolve  uintersection count."));
			return TRUE;
			case 4 :
			//	if(!la_t_edge_select(edge[0], edge[1]))
					la_t_select_hull(edge);
			return TRUE;
			case 5 :
				la_t_collapse_two_vertexes(edge[0], edge[1]);
			return TRUE;
			case 6 :
			{
				double a[3], b[3];
				udg_get_vertex_pos(a, edge[0]);
				udg_get_vertex_pos(b, edge[1]);
				a[0] -= b[0];
				a[1] -= b[1];
				a[2] -= b[2];
				seduce_view_grid_size_set(NULL, sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]));
				la_t_tm_get_pos(b);
				udg_set_grid_snap(b, sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]));
			}
			return TRUE;
			case 7 :
				la_t_vertex_select_fill(edge[0]);
			return TRUE;
			case 8 :
				la_view_axis_matrix_edge_set(edge[0], edge[1]);
			return TRUE;
			case 9 :
//				la_t_edge_fill_strip(edge[0], edge[1]);
				la_t_edge_fill_hole(edge[0], edge[1]);
			return TRUE;
			
		}
	}
	return FALSE;
}




boolean la_pu_polygon(BInputState *input, uint polygon, boolean active)
{
	static SUIPUElement element[11];
	static float x = 0, y = 0, time = 0;
	BInputState fake_input;
	BInputPointerState *pointers;  
	uint ring, i, j;

	seduce_animate(input, &time, active, 4.0);
	
	if(time < 0.01)
		return FALSE;
	element[0].type = S_PUT_ANGLE;
	element[0].text = seduce_translate("Mirror");
	element[0].data.angle[0] = -135.0 / 7.0;
	element[0].data.angle[1] = 135.0 / 7.0;
	element[1].type = S_PUT_ANGLE;
	element[1].text = seduce_translate("Flatten");
	element[1].data.angle[0] = 135.0 / 7.0;
	element[1].data.angle[1] = 135.0 / 7.0 * 3.0;
	element[2].type = S_PUT_ANGLE;
	element[2].text = seduce_translate("Deploy");
	element[2].data.angle[0] = 135.0 / 7.0 * 3.0;
	element[2].data.angle[1] = 135.0 / 7.0 * 5.0;
	element[3].type = S_PUT_ANGLE;
	element[3].text = seduce_translate("Slice");
	element[3].data.angle[0] = 135.0 / 7.0 * 5.0;
	element[3].data.angle[1] = 135.0 / 7.0 * 7.0;
	element[4].type = S_PUT_ANGLE;
	element[4].text = seduce_translate("Slice Off");
	element[4].data.angle[0] = 225.0 + 135.0 / 7.0 * 0.0;
	element[4].data.angle[1] = 225.0 + 135.0 / 7.0 * 2.0;
	element[5].type = S_PUT_ANGLE;
	element[5].text = seduce_translate("Split");
	element[5].data.angle[0] = 225.0 + 135.0 / 7.0 * 2.0;
	element[5].data.angle[1] = 225.0 + 135.0 / 7.0 * 4.0;
	element[6].type = S_PUT_ANGLE;
	element[6].text = seduce_translate("Define Axis Space");
	element[6].data.angle[0] = 225.0 + 135.0 / 7.0 * 4.0;
	element[6].data.angle[1] = 225.0 + 135.0 / 7.0 * 6.0;
	element[7].type = S_PUT_BOTTOM;
	element[7].text = seduce_translate("Fill Selection");
	element[8].type = S_PUT_BOTTOM;
	element[8].text = seduce_translate("Plane Select");
	element[9].type = S_PUT_BOTTOM;
	element[9].text = seduce_translate("Smooth Select");
	element[10].type = S_PUT_BOTTOM;
	element[10].text = seduce_translate("Convex Select");


	if(input->pointers[0].button[2] == TRUE)
	{
		x = input->pointers[0].click_pointer_x[2];
		y = input->pointers[0].click_pointer_y[2];
	}
	if(input->mode == BAM_DRAW)	
	{
		r_matrix_push(NULL);
		r_matrix_translate(NULL, x, y, 0);
	}

	fake_input = *input;
	fake_input.pointers = pointers = malloc((sizeof *pointers) * input->pointer_count);
	for(i = 0; i < input->pointer_count; i++)
	{
		pointers[i] = input->pointers[i];
		pointers[i].button[0] = !pointers[i].button[2];
		pointers[i].last_button[0] = !pointers[i].last_button[2];
	}
	ring = seduce_popup(&fake_input, element, element, 11, time, TRUE);
	free(pointers);
	if(input->mode == BAM_DRAW)	
		r_matrix_pop(NULL);
	if(active)
	{
		switch(ring)
		{
			case 0 :
			{
				uint *ref;
				double origo[3], vector[3];
				udg_get_geometry(NULL, NULL, NULL, &ref, NULL);
				la_t_face_vector(origo, vector, ref[polygon * 4], ref[polygon * 4 + 1], ref[polygon * 4 + 2]);
				la_t_mirror(origo, vector);
				la_t_flip_selected_polygons();
			}
			return TRUE;
			case 1 :
			{
				uint *ref;
				double origo[3], vector[3];
				udg_get_geometry(NULL, NULL, NULL, &ref, NULL);
				la_t_face_vector(origo, vector, ref[polygon * 4], ref[polygon * 4 + 1], ref[polygon * 4 + 2]);
				la_t_flatten(origo, vector);
			}
			return TRUE;
			case 2 :
				la_t_deploy(polygon);
			return TRUE;
			case 3 :
			{
				uint *ref;
				double origo[3], vector[3];
				udg_get_geometry(NULL, NULL, NULL, &ref, NULL);
				la_t_face_vector(origo, vector, ref[polygon * 4], ref[polygon * 4 + 1], ref[polygon * 4 + 2]);
				la_t_slice(origo, vector, FALSE);
			}
			return TRUE;
			case 4 :
			{
				uint *ref;
				double origo[3], vector[3];
				udg_get_geometry(NULL, NULL, NULL, &ref, NULL);
				la_t_face_vector(origo, vector, ref[polygon * 4], ref[polygon * 4 + 1], ref[polygon * 4 + 2]);
				la_t_slice(origo, vector, TRUE);
			}
			return TRUE;
			case 5 :
				la_t_poly_spliter(polygon);
			return TRUE;
			case 6 :
				la_view_axis_matrix_poly_set(polygon, la_t_poly_egde_test(polygon, x, y));
			return TRUE;
			case 7 :
				la_t_polygon_select_fill(polygon);
			return TRUE;
			case 8 :
				la_t_poly_surface_select(polygon, LA_PSST_PLANE);
			return TRUE;	
			case 9 :
				la_t_poly_surface_select(polygon, LA_PSST_SMOOTH);
			return TRUE;	
			case 10 :
				la_t_poly_surface_select(polygon, LA_PSST_CONVEX);
			return TRUE;			


		}
	}
	return FALSE;
}



uint la_pu_select(BInputState *input, boolean active)
{
	static float time = 0, x = 0, y = 0;
	static SUIPUElement element[4];
	BInputState fake_input;
	BInputPointerState *pointers;  
	uint ring, i, j;	
	seduce_animate(input, &time, active, 4.0);
	if(time < 0.01)
		return -1;
	element[0].type = S_PUT_ANGLE;
	element[0].text = seduce_translate("Select");
	element[0].data.angle[0] = 0;
	element[0].data.angle[1] = 90;
	element[1].type = S_PUT_ANGLE;
	element[1].text = seduce_translate("Deselect");
	element[1].data.angle[0] = 90;
	element[1].data.angle[1] = 180;
	element[2].type = S_PUT_ANGLE;
	element[2].text = seduce_translate("Sub");
	element[2].data.angle[0] = 180;
	element[2].data.angle[1] = 270;
	element[3].type = S_PUT_ANGLE;
	element[3].text = seduce_translate("Add");
	element[3].data.angle[0] = 270;
	element[3].data.angle[1] = 360;

	if(active)
	{
		x = input->pointers[0].click_pointer_x[0];
		y = input->pointers[0].click_pointer_y[0];
	}
	if(input->mode == BAM_DRAW)	
	{
		r_matrix_push(NULL);
		r_matrix_translate(NULL, x, y, 0);
	}
	
	fake_input = *input;
	fake_input.pointers = pointers = malloc((sizeof *pointers) * input->pointer_count);
	for(i = 0; i < input->pointer_count; i++)
	{
		pointers[i] = input->pointers[i];
		pointers[i].button[0] = !pointers[i].button[0];
		pointers[i].last_button[0] = !pointers[i].last_button[0];
	}
	ring = seduce_popup(&fake_input, element, element, 4, time, TRUE);
	free(pointers);
	if(input->mode == BAM_DRAW)
		r_matrix_pop(NULL);
	return ring;
}
