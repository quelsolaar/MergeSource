#include "la_includes.h"
#include "la_geometry_undo.h"
#include "la_projection.h"
#include "la_draw_overlay.h"
#include "la_tool.h"
#include "la_pop_up.h"
#include "la_particle_fx.h"
#include "persuade2.h"
#include "deceive.h"

#define THREADED FALSE

uint connection;

extern void set_defult_settings(void);

extern void draw_qube(void);
extern void init_owerlay(void);
extern void draw_owerlay(uint connection);
extern void init_draw_settings(void);
extern void init_draw_line(void);
extern void init_transform_manipulator(void);
extern void la_intro_init(void);
extern void init_key_input(void);
extern void parse_input(BInputState *input, void *user);
extern void la_intro_draw(void *user);
extern void la_edit_func(BInputState *input, void *user);
extern void draw_persuade_surface_init();

void la_context_update(void)
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
//	la_pfx_image_init(imagine_setting_integer_get("FLARE_TEXTURE_SIZE", 512, "Size of texture used for flares."));
/*	sui_text_screen_mode_update();*/
}
extern void seduce_print_hxa(char *file_name);

int main(int argc, char **argv)
{
//	seduce_print_hxa("light2.hxa");
	imagine_settings_load("la_config.cfg");
	seduce_translate_load("la_translation.txt");
	f_debug_mem_thread_safe_init(imagine_mutex_lock, imagine_mutex_unlock, imagine_mutex_create()); 
	betray_init(B_CT_OPENGL, argc, argv, imagine_setting_integer_get("XRES", 0, "X reslution in pixels, set to zero to use screen resolution"),
										imagine_setting_integer_get("YRES", 0, "Y reslution in pixels, set to zero to use screen resolution"),
										imagine_setting_integer_get("AASAMPLES", 4, "shape of FSAA. Requires restart"), 
										imagine_setting_boolean_get("FULLSCREEN", FALSE, "Run Stellar in fullscreen (TRUE) or windowed mode (FALSE)"), seduce_translate("Loq Airou"));
	deceive_set_arg(argc, argv);
	r_init(betray_gl_proc_address_get());
	seduce_init();
	seduce_view_slide(NULL, TRUE);
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);	
	enough_init();
	la_do_init();
	init_key_input();
	init_undo();
	la_intro_init();
	la_t_init_draw_line();
	la_t_init_edge_connector();
	la_t_tm_init();
	la_context_update();
	lo_projection_cache_init();
	la_pfx_image_init(512);
	la_pfx_init(imagine_setting_integer_get("PARTICLE_COUNT", 512, "Number of FX particles."));
	seduce_primitive_line_color_set(0.2, 0.6, 1, 
									0.3, 0.3, 0.3, 
									1.0, 0.2, 0.6);
#ifdef PERSUADE_H
	persuade_init(4);
/*	p_geo_set_sds_level(imagine_setting_integer_get("MAX_TESS_LEVEL", 2, "Maximum shape of tesselation."));
	p_geo_set_sds_force_shape(imagine_setting_integer_get("MIN_TESS_LEVEL", 0, "Minimum shape of tesselation."));
	p_geo_set_sds_mesh_factor(imagine_setting_double_get("GEOMETRY_COMPLEXITY", 40, "Geometry complexity."));*/
#endif
	draw_persuade_surface_init();

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);/*GL_AMBIENT_AND_DIFFUSE*/

	deceive_set_intro_draw_func(la_intro_draw, NULL);
	betray_gl_context_update_func_set(la_context_update);
	betray_action_func_set(deceive_intro_handler, la_edit_func);
	betray_launch_main_loop();	
	imagine_settings_save("la_config.cfg");
	seduce_translate_save("la_translation.txt");
	f_debug_mem_print(1); 
	return 0;
}
