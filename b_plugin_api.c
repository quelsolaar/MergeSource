
#include <stdio.h>
#include "forge.h"
#include "imagine.h"
#define BETRAY_PLUGGIN_KEYS
#define BETRAY_PLUGGIN_DEFINES
#include "betray.h"

BInputState	*(*betray_plugin_get_input_state)(void) = NULL;
BContextType (*betray_plugin_context_type_get)() = NULL;
double		(*betray_plugin_screen_mode_get)(uint *x_size, uint *y_size, boolean *fullscreen) = NULL;  
void		(*betray_plugin_application_draw)(uint fbo, uint x_size, uint y_size, float *vantage, boolean vantage_modify, float *matrix) = NULL;
void		*(*betray_plugin_gl_proc_address_get)(void) = NULL;

void		(*betray_plugin_clipboard_set)(char *text) = NULL;
char		*(*betray_plugin_clipboard_get)(void) = NULL;
void		(*betray_plugin_callback_set_main)(void (*main_loop)(BInputState *input)) = NULL;
void		(*betray_plugin_callback_set_view_vantage)(void (*vantage)(float *pos), boolean modify) = NULL;
void		(*betray_plugin_callback_set_view_direction)(void (*matrix)(float *matrix)) = NULL;
#ifdef WIN32_
void		(*betray_plugin_callback_set_event_pump)(uint event, boolean exclusive, void (*event_loop)(BInputState *input, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)) = NULL;
#endif
void		(*betray_plugin_callback_set_image_warp)(boolean (*image_warp)(BInputState *input), char *name) = NULL;

uint		(*betray_plugin_audio_unit_create)(void) = NULL;
uint		(*betray_plugin_callback_set_audio_sound_create)(uint audio_unit_id, uint (*func)(uint type, uint stride, uint length, uint frequency, void *data, char *name)) = NULL;
void		(*betray_plugin_callback_set_audio_sound_destroy)(uint audio_unit_id, void (*func)(uint sound)) = NULL;
uint		(*betray_plugin_callback_set_audio_sound_play)(uint audio_unit_id, uint (*func)(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete)) = NULL;
void		(*betray_plugin_callback_set_audio_sound_set)(uint audio_unit_id, void (*func)(uint play, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient)) = NULL;
void		(*betray_plugin_callback_set_audio_sound_is_playing)(uint audio_unit_id, boolean (*func)(uint play)) = NULL;
void		(*betray_plugin_callback_set_audio_sound_stop)(uint audio_unit_id, void (*func)(uint play)) = NULL;
void		(*betray_plugin_callback_set_audio_stream_create)(uint audio_unit_id, uint (*func)(uint frequency, float *pos, float *vector,  float volume, boolean ambient)) = NULL;
void		(*betray_plugin_callback_set_audio_stream_destroy)(uint audio_unit_id, void (*func)(uint stream)) = NULL;
void		(*betray_plugin_callback_set_audio_stream_feed)(uint audio_unit_id, void (*func)(uint stream, uint type, uint stride, uint length, void *data)) = NULL;
void		(*betray_plugin_callback_set_audio_stream_buffer_left)(uint audio_unit_id, uint (*func)(uint stream)) = NULL;
void		(*betray_plugin_callback_set_audio_stream_set)(uint audio_unit_id, void (*func)(uint stream, float *pos, float *vector,  float volume, boolean ambient)) = NULL;
void		(*betray_plugin_callback_set_audio_listener)(uint audio_unit_id, void (*func)(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound)) = NULL;
void		(*betray_plugin_callback_set_audio_read)(uint audio_unit_id, uint (*func)(void *data, uint type, uint buffer_size), uint channels, float *vectors) = NULL;

void		(*betray_plugin_audio_master_volume_get)(float volume) = NULL;
uint		(*betray_plugin_user_allocate)(void) = NULL;
uint		(*betray_plugin_input_device_allocate)(uint user_id, char *name) = NULL;
uint		(*betray_plugin_input_device_free)(uint id) = NULL;
uint		(*betray_plugin_button_allocate)(uint code, char *name) = NULL;
void		(*betray_plugin_button_set)(uint user_id, uint id, boolean state, uint character) = NULL;
void		(*betray_plugin_button_free)(uint id) = NULL;
uint		(*betray_plugin_pointer_allocate)(uint user_id, uint device_id, uint button_count, float x, float y, float z, float *origin, char *name, boolean draw) = NULL;
uint		(*betray_plugin_pointer_set)(uint id, float x, float y, float z, float *origin, boolean *buttons) = NULL;
void		(*betray_plugin_pointer_free)(uint id) = NULL;
uint		(*betray_plugin_axis_allocate)(uint user_id, uint device_id,char *name, BAxisType type, uint axis_count) = NULL;
void		(*betray_plugin_axis_set)(uint id, float axis_x, float axis_y, float axis_z) = NULL;
void		(*betray_plugin_axis_free)(uint id) = NULL;

uint		(*betray_settings_create)(uint type, char *name, uint select_count, char **select_options) = NULL;
uint		(*betray_settings_count)() = NULL;
BSettingType(*betray_settings_type)(uint id) = NULL;
char		*(*betray_settings_name)(uint id) = NULL;
boolean		(*betray_settings_toggle_get)(uint id) = NULL;
void		(*betray_settings_toggle_set)(uint id, boolean toggle) = NULL;
uint		(*betray_settings_select_get)(uint id) = NULL;
void		(*betray_settings_select_set)(uint id, uint select) = NULL;
uint		(*betray_settings_select_count_get)(uint id) = NULL;
char		*(*betray_settings_select_name_get)(uint id, uint option) = NULL;
char		*(*betray_settings_string_get)(uint id) = NULL;
void		(*betray_settings_string_set)(uint id, char *string) = NULL;
float		(*betray_settings_number_float_get)(uint id) = NULL;
void		(*betray_settings_number_float_set)(uint id, float number) = NULL;
int			(*betray_settings_number_int_get)(uint id) = NULL;
void		(*betray_settings_number_int_set)(uint id, int number) = NULL;
float		(*betray_settings_slider_get)(uint id) = NULL;
void		(*betray_settings_slider_set)(uint id, float slider) = NULL;
void		(*betray_settings_2d_get)(uint id, float *x, float *y) = NULL;
void		(*betray_settings_2d_set)(uint id, float x, float y) = NULL;
void		(*betray_settings_3d_get)(uint id, float *x, float *y, float *z) = NULL;
void		(*betray_settings_3d_set)(uint id, float x, float y, float z) = NULL;
void		(*betray_settings_color_get)(uint id, float *red, float *green, float *blue) = NULL;
void		(*betray_settings_color_set)(uint id, float red, float green, float blue) = NULL;
void		(*betray_settings_4x4_matrix_get)(uint id, float *matrix) = NULL;
void		(*betray_settings_4x4_matrix_set)(uint id, float *matrix) = NULL;

void		*(*betray_plugin_windows_window_handle_get)(void) = NULL;
void		*(*betray_plugin_windows_device_context_handle_get)(void) = NULL;
extern		void betray_plugin_init(void);

ILibExport char *imagine_lib_name(void)
{
	return "Betray";
}

ILibExport IInterface *imagine_lib_main(IInterface *exe_interface)
{
	betray_plugin_get_input_state =									imagine_library_interface_get_by_name(exe_interface, "betray_plugin_get_input_state");
	betray_plugin_context_type_get =								imagine_library_interface_get_by_name(exe_interface, "betray_plugin_context_type_get");
	betray_plugin_screen_mode_get =									imagine_library_interface_get_by_name(exe_interface, "betray_plugin_screen_mode_get");
	betray_plugin_application_draw =								imagine_library_interface_get_by_name(exe_interface, "betray_plugin_application_draw");
	betray_plugin_gl_proc_address_get =								imagine_library_interface_get_by_name(exe_interface, "betray_plugin_gl_proc_address_get");
	betray_plugin_clipboard_set =									imagine_library_interface_get_by_name(exe_interface, "betray_plugin_clipboard_set");
	betray_plugin_clipboard_get =									imagine_library_interface_get_by_name(exe_interface, "betray_plugin_clipboard_get");
	betray_plugin_callback_set_main =								imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_main");
	betray_plugin_callback_set_view_vantage =						imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_view_vantage");
	betray_plugin_callback_set_view_direction =						imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_view_direction");
#ifdef WIN32_
	betray_plugin_callback_set_event_pump =							imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_event_pump");
#endif
	betray_plugin_callback_set_image_warp =							imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_image_warp");

	betray_plugin_audio_unit_create =								imagine_library_interface_get_by_name(exe_interface, "betray_plugin_audio_unit_create");
	betray_plugin_callback_set_audio_sound_create =					imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_sound_create");
	betray_plugin_callback_set_audio_sound_destroy =				imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_sound_destroy");
	betray_plugin_callback_set_audio_sound_play =					imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_sound_play");
	betray_plugin_callback_set_audio_sound_set =					imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_sound_set");
	betray_plugin_callback_set_audio_sound_is_playing =				imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_sound_is_playing");
	betray_plugin_callback_set_audio_sound_stop =					imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_sound_stop");
	betray_plugin_callback_set_audio_stream_create =				imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_stream_create");
	betray_plugin_callback_set_audio_stream_destroy =				imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_stream_destroy");
	betray_plugin_callback_set_audio_stream_feed =					imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_stream_feed");
	betray_plugin_callback_set_audio_stream_buffer_left =			imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_stream_buffer_left");
	betray_plugin_callback_set_audio_stream_set =					imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_stream_set");
	betray_plugin_callback_set_audio_listener =						imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_listener");
	betray_plugin_callback_set_audio_read =							imagine_library_interface_get_by_name(exe_interface, "betray_plugin_callback_set_audio_read");

	betray_plugin_audio_master_volume_get =							imagine_library_interface_get_by_name(exe_interface, "betray_plugin_audio_master_volume_get");
	
	betray_plugin_user_allocate =									imagine_library_interface_get_by_name(exe_interface, "betray_plugin_user_allocate");
	betray_plugin_input_device_allocate =							imagine_library_interface_get_by_name(exe_interface, "betray_plugin_input_device_allocate");
	betray_plugin_input_device_free =								imagine_library_interface_get_by_name(exe_interface, "betray_plugin_input_device_free");
	betray_plugin_button_allocate =									imagine_library_interface_get_by_name(exe_interface, "betray_plugin_button_allocate");
	betray_plugin_button_set =										imagine_library_interface_get_by_name(exe_interface, "betray_plugin_button_set");
	betray_plugin_button_free =										imagine_library_interface_get_by_name(exe_interface, "betray_plugin_button_free");
	betray_plugin_pointer_allocate =								imagine_library_interface_get_by_name(exe_interface, "betray_plugin_pointer_allocate");
	betray_plugin_pointer_set =										imagine_library_interface_get_by_name(exe_interface, "betray_plugin_pointer_set");
	betray_plugin_pointer_free =									imagine_library_interface_get_by_name(exe_interface, "betray_plugin_pointer_free");
	betray_plugin_axis_allocate =									imagine_library_interface_get_by_name(exe_interface, "betray_plugin_axis_allocate");
	betray_plugin_axis_set =										imagine_library_interface_get_by_name(exe_interface, "betray_plugin_axis_set");
	betray_plugin_axis_free =										imagine_library_interface_get_by_name(exe_interface, "betray_plugin_axis_free");

	betray_settings_create =										imagine_library_interface_get_by_name(exe_interface, "betray_settings_create");
	betray_settings_count =											imagine_library_interface_get_by_name(exe_interface, "betray_settings_count");
	betray_settings_type =											imagine_library_interface_get_by_name(exe_interface, "betray_settings_type");
	betray_settings_name =											imagine_library_interface_get_by_name(exe_interface, "betray_settings_name");
	betray_settings_toggle_get =									imagine_library_interface_get_by_name(exe_interface, "betray_settings_toggle_get");
	betray_settings_toggle_set =									imagine_library_interface_get_by_name(exe_interface, "betray_settings_toggle_set");
	betray_settings_select_get =									imagine_library_interface_get_by_name(exe_interface, "betray_settings_select_get");
	betray_settings_select_set =									imagine_library_interface_get_by_name(exe_interface, "betray_settings_select_set");
	betray_settings_select_count_get =								imagine_library_interface_get_by_name(exe_interface, "betray_settings_select_count_get");
	betray_settings_select_name_get =								imagine_library_interface_get_by_name(exe_interface, "betray_settings_select_name_get");
	betray_settings_string_get =									imagine_library_interface_get_by_name(exe_interface, "betray_settings_string_get");
	betray_settings_string_set =									imagine_library_interface_get_by_name(exe_interface, "betray_settings_string_set");
	betray_settings_number_float_get =								imagine_library_interface_get_by_name(exe_interface, "betray_settings_number_float_get");
	betray_settings_number_float_set =								imagine_library_interface_get_by_name(exe_interface, "betray_settings_number_float_set");
	betray_settings_number_int_get =								imagine_library_interface_get_by_name(exe_interface, "betray_settings_number_int_get");
	betray_settings_number_int_set =								imagine_library_interface_get_by_name(exe_interface, "betray_settings_number_int_set");
	betray_settings_slider_get =									imagine_library_interface_get_by_name(exe_interface, "betray_settings_slider_get");
	betray_settings_slider_set =									imagine_library_interface_get_by_name(exe_interface, "betray_settings_slider_set");
	betray_settings_2d_get =										imagine_library_interface_get_by_name(exe_interface, "betray_settings_2d_get");
	betray_settings_2d_set =										imagine_library_interface_get_by_name(exe_interface, "betray_settings_2d_set");
	betray_settings_3d_get =										imagine_library_interface_get_by_name(exe_interface, "betray_settings_3d_get");
	betray_settings_3d_set =										imagine_library_interface_get_by_name(exe_interface, "betray_settings_3d_set");
	betray_settings_color_get =										imagine_library_interface_get_by_name(exe_interface, "betray_settings_color_get");
	betray_settings_color_set =										imagine_library_interface_get_by_name(exe_interface, "betray_settings_color_set");
	betray_settings_4x4_matrix_get =								imagine_library_interface_get_by_name(exe_interface, "betray_settings_4x4_matrix_get");
	betray_settings_4x4_matrix_set =								imagine_library_interface_get_by_name(exe_interface, "betray_settings_4x4_matrix_set");

	betray_plugin_windows_window_handle_get =						imagine_library_interface_get_by_name(exe_interface, "betray_plugin_windows_window_handle_get");
	betray_plugin_windows_device_context_handle_get =				imagine_library_interface_get_by_name(exe_interface, "betray_plugin_windows_device_context_handle_get");
	betray_plugin_init();
	return NULL;
}

