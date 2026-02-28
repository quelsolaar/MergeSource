#if defined __cplusplus		/* Declare as C symbols for C++ users. */
extern "C" {
#endif

extern BInputState	*(*betray_plugin_get_input_state)(void);
extern BContextType	(*betray_plugin_context_type_get)();
extern double		(*betray_plugin_screen_mode_get)(uint *x_size, uint *y_size, boolean *fullscreen);
extern void			(*betray_plugin_application_draw)(uint fbo, uint x_size, uint y_size, float *vantage, boolean vantage_modify, float *matrix); 
extern void			*(*betray_plugin_gl_proc_address_get)(void);
extern void			(*betray_plugin_clipboard_set)(char *text);
extern char			*(*betray_plugin_clipboard_get)(void);
extern void			(*betray_plugin_callback_set_main)(void (*main_loop)(BInputState *input));
extern void			(*betray_plugin_callback_set_view_vantage)(void (*vantage)(float *pos), boolean modify);
extern void			(*betray_plugin_callback_set_view_direction)(void (*matrix)(float *matrix));
extern void			(*betray_plugin_callback_set_event_pump)(uint event, boolean exclusive, void (*event_loop)(BInputState *input, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam));
extern void			(*betray_plugin_callback_set_image_warp)(boolean (*main_loop)(BInputState *input), char *name);
extern uint			(*betray_plugin_audio_unit_create)();
extern uint			(*betray_plugin_callback_set_audio_sound_create)(uint audio_unit_id, uint (*func)(uint type, uint stride, uint length, uint frequency, void *data, char *name));
extern void			(*betray_plugin_callback_set_audio_sound_destroy)(uint audio_unit_id, void (*func)(uint sound));
extern uint			(*betray_plugin_callback_set_audio_sound_play)(uint audio_unit_id, uint (*func)(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete));
extern void			(*betray_plugin_callback_set_audio_sound_set)(uint audio_unit_id, void (*func)(uint play, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient));
extern void			(*betray_plugin_callback_set_audio_sound_is_playing)(uint audio_unit_id, void (*func)(uint play));
extern void			(*betray_plugin_callback_set_audio_sound_stop)(uint audio_unit_id, void (*func)(uint play));
extern void			(*betray_plugin_callback_set_audio_stream_create)(uint audio_unit_id, uint (*func)(uint frequency, float *pos, float *vector,  float volume, boolean ambient));
extern void			(*betray_plugin_callback_set_audio_stream_destroy)(uint audio_unit_id, void (*func)(uint stream));
extern void			(*betray_plugin_callback_set_audio_stream_feed)(uint audio_unit_id, void (*func)(uint stream, uint type, uint stride, uint length, void *data));
extern void			(*betray_plugin_callback_set_audio_stream_buffer_left)(uint audio_unit_id, uint (*func)(uint stream));
extern void			(*betray_plugin_callback_set_audio_stream_set)(uint audio_unit_id, void (*func)(uint stream, float *pos, float *vector,  float volume, boolean ambient));
extern void			(*betray_plugin_callback_set_audio_listener)(uint audio_unit_id, void (*func)(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound));
extern void			(*betray_plugin_callback_set_audio_read)(uint audio_unit_id, uint (*func)(void *data, uint type, uint buffer_size), uint channels, float *vectors);
extern void			(*betray_plugin_audio_master_volume_get)(float volume);
extern uint			(*betray_plugin_user_allocate)(void);
extern uint			(*betray_plugin_input_device_allocate)(uint user_id, char *name);
extern uint			(*betray_plugin_input_device_free)(uint id);
extern uint			(*betray_plugin_button_allocate)(uint code, char *name);
extern void			(*betray_plugin_button_set)(uint user_id, uint id, boolean state, uint character);
extern void			(*betray_plugin_button_free)(uint id);
extern uint			(*betray_plugin_pointer_allocate)(uint user_id, uint device_id, uint button_count, float x, float y, float z, float *origin, char *name, boolean draw);
extern uint			(*betray_plugin_pointer_set)(uint id, float x, float y, float z, float *origin, boolean *buttons);
extern void			(*betray_plugin_pointer_free)(uint id);
extern uint			(*betray_plugin_axis_allocate)(uint user_id, uint device_id, char *name, BAxisType type, uint axis_count);
extern void			(*betray_plugin_axis_set)(uint id, float axis_x, float axis_y, float axis_z);
extern void			(*betray_plugin_axis_free)(uint id);

extern uint			(*betray_settings_create)(uint type, char *name, uint select_count, char **select_options);
extern uint			(*betray_settings_count)();
extern BSettingType	(*betray_settings_type)(uint id);
extern char			*(*betray_settings_name)(uint id);
extern boolean		(*betray_settings_toggle_get)(uint id);
extern void			(*betray_settings_toggle_set)(uint id, boolean toggle);
extern uint			(*betray_settings_select_get)(uint id);
extern void			(*betray_settings_select_set)(uint id, uint select);
extern uint			(*betray_settings_select_count_get)(uint id);
extern char			*(*betray_settings_select_name_get)(uint id, uint option);
extern float		(*betray_settings_number_float_get)(uint id);
extern void			(*betray_settings_number_float_set)(uint id, float number);
extern int			(*betray_settings_number_int_get)(uint id);
extern void			(*betray_settings_number_int_set)(uint id, int number);
extern float		(*betray_settings_slider_get)(uint id);
extern void			(*betray_settings_slider_set)(uint id, float slider);
extern void			(*betray_settings_2d_get)(uint id, float *x, float *y);
extern void			(*betray_settings_2d_set)(uint id, float x, float y);
extern void			(*betray_settings_3d_get)(uint id, float *x, float *y, float *z);
extern void			(*betray_settings_3d_set)(uint id, float x, float y, float z);
extern void			(*betray_settings_color_get)(uint id, float *red, float *green, float *blue);
extern void			(*betray_settings_color_set)(uint id, float red, float green, float blue);
extern void			(*betray_settings_4x4_matrix_get)(uint id, float *matrix);
extern void			(*betray_settings_4x4_matrix_set)(uint id, float *matrix);

extern void			*(*betray_plugin_windows_window_handle_get)(void);

#if defined __cplusplus
}
#endif