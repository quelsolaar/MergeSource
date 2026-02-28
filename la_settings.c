
#include "la_includes.h"
#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_draw_overlay.h"
#include "la_particle_fx.h"


extern void parse_input(BInputState *input, void *user);

void layer_name_func(void *user, char *text)
{
	ENode *node;
	if((node = e_ns_get_node(0, udg_get_modeling_node())) != NULL &&
	   (node = e_ns_get_node(0, e_nso_get_link_id(e_nso_get_link(node, V_NT_GEOMETRY)))) != NULL) 
	{
		EGeoLayer *layer;
		if((layer = e_nsg_get_layer_by_name(node, text)) != NULL)
		{
			if(VN_G_LAYER_VERTEX_XYZ == e_nsg_get_layer_type(layer))
				udg_set_modeling_layer(e_nsg_get_layer_id(layer));
		}else
		{
			verse_send_g_layer_create(e_ns_get_node_id(node), -1, text, VN_G_LAYER_VERTEX_XYZ, 0, 0);
			udg_set_modeling_layer(e_nsg_get_layer_id(layer));
		}
	}
}

void la_edit_func(BInputState *input, void *user);
void la_draw_force_update_persuade(void);


/*	SUIViewElement element[10];
	static char *t, text[36];
	uint i;
	if(input->mode == BAM_MAIN)
	{
		verse_callback_update(0);
		return;
	}
	element[0].type = S_VET_BOOLEAN;
	element[0].text = "dispalay Flares";
	element[0].param.checkbox = imagine_setting_integer_get("DISPLAY_SILLY_FLARES", TRUE);

	element[1].type = S_VET_BOOLEAN;
	element[1].text = "display SDS";
	element[1].param.checkbox = imagine_setting_integer_get("RENDER_AS_SDS", TRUE);

	element[2].type = S_VET_SLIDER;
	element[2].text = "complexity";
	element[2].param.slider = sqrt(sui_get_setting_double("GEOMETRY_COMPLEXITY", 1) / 1000.0);

	element[3].type = S_VET_INTEGER;
	element[3].text = "maximum Tesselation";
	element[3].param.integer = imagine_setting_integer_get("MAX_TESS_shape", 2);

	element[4].type = S_VET_INTEGER;
	element[4].text = "minimum Tesselation";
	element[4].param.integer = sui_get_setting_int("MIN_TESS_shape", 0);

	element[5].type = S_VET_TEXT;
	element[5].text = "author";
	t = sui_get_setting_text("AUTHOR", "eskil");
	for(i = 0; i < 32 && t[i] != 0; i++)
		text[i] = t[i];
	text[i] = 0;
	element[5].param.text.text = text;
	element[5].param.text.length = 32;

	element[6].type = S_VET_INTEGER;
	element[6].text = "revole section";
	element[6].param.integer = sui_get_setting_int("REVOLVE_INTERSECTIONS", 8);

	element[7].type = S_VET_INTEGER;
	element[7].text = "undo buffer";
	element[7].param.integer = sui_get_setting_int("UNDO_MEMORY_FOOT_PRINT", 5000000);

	element[8].type = S_VET_BOOLEAN;
	element[8].text = "Geometry preview";
	element[8].param.checkbox = sui_get_setting_int("GEOMETRY_PREVIEW", TRUE);

	if(input->mode == BAM_DRAW)
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glPushMatrix();
		seduce_view_set(NULL, NULL);
		la_do_owerlay();
		glPushMatrix();
		la_pfx_select_vertex();
		glPopMatrix();
		glPopMatrix();
		la_pfx_draw(FALSE);

		glPushMatrix();
		glTranslatef(0, 0, -1);
		glDisable(GL_DEPTH_TEST);
	}
	if(input->mode == BAM_DRAW)
	{
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}
	sui_set_setting_int("DISPLAY_SILLY_FLARES",		element[0].param.checkbox);
	sui_set_setting_int("RENDER_AS_SDS",			element[1].param.checkbox);
	sui_set_setting_double("GEOMETRY_COMPLEXITY",	element[2].param.slider * element[2].param.slider * 1000.0);
	sui_set_setting_int("MAX_TESS_LEVEL",			element[3].param.integer);
	sui_set_setting_int("MIN_TESS_LEVEL",			element[4].param.integer);
	sui_set_setting_text("AUTHOR",					element[5].param.text.text);
	sui_set_setting_int("REVOLVE_INTERSECTIONS",	element[6].param.integer);
	sui_set_setting_int("UNDO_MEMORY_FOOT_PRINT",	element[7].param.integer);
	sui_set_setting_int("GEOMETRY_PREVIEW",			element[8].param.checkbox);
#ifdef PERSUADE_H
	p_geo_set_sds_shape(element[3].param.integer);
	p_geo_set_sds_force_shape(element[4].param.integer);
	p_geo_set_sds_mesh_factor(element[2].param.slider * element[2].param.slider * 1000.0);
#endif
	la_draw_force_update_persuade();*/


/*
typedef struct{
	SeducePanelElementType type;
	char *text;
	union{
		boolean checkbox; 
		int		integer;
		uint	uinteger;
		double	real;
		float	slider;
		struct{
			uint	icon;
			boolean	active;
		}button;
		struct{
			char	*text;
			uint	length;
			uint	lines;
		}text;
		struct{
			char	**text;
			uint	count;
			uint	active;
		}select;
		struct{
			char	*text;
			uint	length;
		}password;
		float	*color;
		struct{
			uint	current;
			uint	count;
			boolean	add;
			boolean	del;
		}split_multi;
		struct{
			void (*func)(BInputState *input, void *user, double x_pos, double y_pos, double width, double length);
			void *user;
			float length;
		}custom;
		struct{
			uint icon;
			void (*func)(BInputState *input, float time, void *user);
			void *user;
			boolean displace;
		}popup;
		struct{
			boolean open;
			float timer;
		}sections;
	}param;
}SeducePanelElement;*/

extern void seduce_widget_list_element_test(BInputState *input, float time);
#define LA_SETTINGS_COUNT 9

boolean la_draw_settings_menu(BInputState *input, boolean active)
{
	static SeducePanelElement *element = NULL;
	static uint init = FALSE, selected = -1, element_count = 0;
	static float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	static float timer = 0;
//	float background_color[4] = {0.7, 0.7, 0.7, 0.7}, color[4] = {0.7, 0.7, 0.7, 0.7};
	float background_color[4] = {0.2, 0.6, 1.0, 0.8}, color[4] = {1.0, 1.0, 1.0, 0.7};
	uint output;
	seduce_animate(input, &timer, active, 1.0);

	if(timer < 0.0001)
		return TRUE;
	
	r_matrix_set(&la_interface_matrix);

	if(element_count != LA_SETTINGS_COUNT + betray_settings_count())
	{
		char *comment;
		element_count = LA_SETTINGS_COUNT + betray_settings_count();
		if(element != NULL)
			free(element);
		element = malloc((sizeof *element) * element_count);

		comment = "Revolve  uintersection count.";
		element[0].type = SEDUCE_PET_UNSIGNED_INTEGER;
		element[0].text = seduce_translate("Revolve Sections"); 
		element[0].param.uinteger = imagine_setting_integer_get("REVOLVE_INTERSECTIONS", 8, comment);
		element[0].description = comment;

		comment = "Render smooth surface";
		element[1].type = SEDUCE_PET_BOOLEAN;
		element[1].text = seduce_translate("Dispaly Subdivision surface"); 
		element[1].param.active = imagine_setting_boolean_get("RENDER_AS_SDS", TRUE, comment);	
		element[1].description = comment;	

		comment = "Geometry complexity.";
		element[2].type = SEDUCE_PET_REAL_BOUND;
		element[2].text = seduce_translate("Geometry complexity");
		element[2].param.real.value = imagine_setting_double_get("GEOMETRY_COMPLEXITY", 40, comment);
		element[2].description = comment;

		comment = "Maximum level of tesselation.";
		element[3].type = SEDUCE_PET_UNSIGNED_INTEGER;
		element[3].text = seduce_translate("Max Teselation"); 
		element[3].param.uinteger = imagine_setting_integer_get("MAX_TESS_LEVEL", 3, comment);
		element[3].description = comment;
		
		comment = "Minimum level of tesselation.";
		element[4].type = SEDUCE_PET_UNSIGNED_INTEGER;
		element[4].text = seduce_translate("Min Tesselelation"); 
		element[4].param.uinteger = imagine_setting_integer_get("MIN_TESS_LEVEL", 0, comment);
		element[4].description = comment;
		
		comment = "Size of texture used for flares.";
		element[5].type = SEDUCE_PET_UNSIGNED_INTEGER;
		element[5].text = seduce_translate("Texture Size"); 
		element[5].param.uinteger = imagine_setting_integer_get("FLARE_TEXTURE_SIZE", 512, comment);
		element[5].description = comment;		

		comment = "Display flares";
		element[6].type = SEDUCE_PET_BOOLEAN;
		element[6].text = seduce_translate("Display Flares"); 
		element[6].param.active = imagine_setting_boolean_get("DISPLAY_SILLY_FLARES", TRUE, comment);
		element[6].description = comment;
		
		comment = "Maximum number of prolygons drawn befor SDS is turned off";
		element[7].type = SEDUCE_PET_UNSIGNED_INTEGER;
		element[7].text = seduce_translate("SDS limit"); 
		element[7].param.uinteger = imagine_setting_integer_get("SDS_LIMIT", 10000, comment);
		element[7].description = comment;	

		comment = "shape of FSAA. Requires restart";
		element[8].type = SEDUCE_PET_UNSIGNED_INTEGER;
		element[8].text = seduce_translate("AntiAliasing level"); 
		element[8].param.uinteger = imagine_setting_integer_get("AA_LEVEL", 3, comment);
		element[8].description = comment;	

	}
	seduce_settings_betray_set(&element[LA_SETTINGS_COUNT]);
	seduce_background_shape_matrix_interact(input, &init, matrix, TRUE, TRUE);
	r_matrix_push(NULL);
	r_matrix_matrix_mult(NULL, matrix);
//	seduce_widget_list_element_background(input, 0.2, 0.5, 0.4, SEDUCE_T_SIZE * 2.0, element, element_count, &selected, &init, timer);
//	seduce_widget_list_element_list(input, 0.2, 0.5, 0.4, SEDUCE_T_SIZE * 2.0, element, element_count, &selected, &init, timer);
	seduce_primitive_line_focal_depth_set(1.0);
	
	seduce_widget_list(input, &init, element, element_count, timer, "SETTINGS", color, background_color, SEDUCE_WLS_PANEL);

	r_matrix_pop(NULL);
	seduce_settings_betray_get(&element[LA_SETTINGS_COUNT]);
	imagine_setting_integer_set("REVOLVE_INTERSECTIONS", element[0].param.uinteger, NULL);
	imagine_setting_boolean_set("RENDER_AS_SDS", element[1].param.active, NULL);	
	imagine_setting_double_set("GEOMETRY_COMPLEXITY", element[2].param.real.value, NULL);
	imagine_setting_integer_set("MAX_TESS_LEVEL", element[3].param.uinteger, NULL);
	imagine_setting_integer_set("MIN_TESS_LEVEL", element[4].param.uinteger, NULL);
	imagine_setting_integer_set("FLARE_TEXTURE_SIZE", element[5].param.uinteger, NULL);
	imagine_setting_boolean_set("DISPLAY_SILLY_FLARES", element[6].param.active, NULL);
	imagine_setting_integer_set("SDS_LIMIT", element[7].param.uinteger, NULL);
	imagine_setting_integer_set("AA_LEVEL", element[8].param.uinteger, NULL);

	if(input->mode == BAM_EVENT && input->pointers[0].button[0] && !input->pointers[0].last_button[0] && NULL == seduce_element_pointer_id(input, 0, NULL))
	{
		seduce_background_particle_burst(input, input->pointers[0].pointer_x, input->pointers[0].pointer_y, 256, 0.3, 0);
		return FALSE;
	}
	return TRUE;

/*	element[0].type = SEDUCE_PET_BOOLEAN;
	element[0].text = "Hello!";
	element[0].param.checkbox = TRUE; 
	element[1].type = SEDUCE_PET_BOOLEAN;
	element[1].text = "Hello!";
	element[1].param.checkbox = TRUE; 
	element[2].type = SEDUCE_PET_BOOLEAN;
	element[2].text = "Hello!";
	element[2].param.checkbox = TRUE; 
	element[3].type = SEDUCE_PET_BOOLEAN;
	element[3].text = "Hello!";
	element[3].param.checkbox = TRUE; 
	seduce_widget_list_element_background(input, 0.2, 0.5, 0.4, 1.0, element, 4, &selected, NULL, 1);
	seduce_settings_panel(input, &timer, element, 4, 0, 0, 0.8, 1, timer);
	*/
}

void la_draw_rename_done_func(void *user, char *text)
{
	
}


boolean la_draw_rename_menu(BInputState *input, boolean active)
{
	static SeducePanelElement element[2];
	static uint init = FALSE, selected = -1;
	static float matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	static float timer = 0;
//	float background_color[4] = {0.7, 0.7, 0.7, 0.7}, color[4] = {0.7, 0.7, 0.7, 0.7};
	float background_color[4] = {0.2, 0.6, 1.0, 0.8}, color[4] = {1.0, 1.0, 1.0, 0.7};
	uint output, i;
	uint32 node_id;
	seduce_animate(input, &timer, active, 1.0);

	element[0].type = SEDUCE_PET_TEXT;
	element[0].text = seduce_translate("Name"); 
	if(timer < 0.001)
	{
		node_id = udg_get_modeling_node();
		if(node_id != 0)
		{
			ENode *node;
			node = e_ns_get_node(0, node_id);
			if(node != NULL)
			{
				char *name;
				uint i;
				name = e_ns_get_node_name(node);
				for(i = 0 ; i < 63 && name[i] != 0; i++)
					element[0].param.text[i] = name[i];
				element[0].param.text[i] = 0;
			}else
				element[0].param.text[0] = 0;
		}else
			element[0].param.text[0] = 0;
	}
	if(timer < 0.0001)
		return TRUE;
	
	element[0].description = "Node name";

	element[1].type = SEDUCE_PET_OK_CANCEL;
	element[1].text = seduce_translate("Dispaly Subdivision surface"); 
	element[1].param.ok_cancel = SEDUCE_PEOCS_UNDECIDED;
	element[1].description = "";	


	r_matrix_set(&la_interface_matrix);
	seduce_background_shape_matrix_interact(input, &init, matrix, TRUE, TRUE);
	r_matrix_push(NULL);
	r_matrix_matrix_mult(NULL, matrix);
	seduce_primitive_line_focal_depth_set(1.0);	
	seduce_widget_list(input, &init, element, 2, timer, "NAME", color, background_color, SEDUCE_WLS_PANEL);
	r_matrix_pop(NULL);
	if(element[1].param.ok_cancel == SEDUCE_PEOCS_OK)
	{
		verse_send_node_name_set(udg_get_modeling_node(), element[0].param.text);
		return FALSE;
	}
	if(element[1].param.ok_cancel == SEDUCE_PEOCS_CANCEL)
		return FALSE;
	if(input->mode == BAM_EVENT && input->pointers[0].button[0] && !input->pointers[0].last_button[0] && NULL == seduce_element_pointer_id(input, 0, NULL))
	{
		seduce_background_particle_burst(input, input->pointers[0].pointer_x, input->pointers[0].pointer_y, 256, 0.3, 0);
		return FALSE;
	}
	return TRUE;

}