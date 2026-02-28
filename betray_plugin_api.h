
#define BETRAY_PLUGGIN_KEYS
#define BETRAY_PLUGGIN_DEFINES
#include "betray.h"

#include "b_plugin_api_internal.h"

#ifdef BETRAY_PLUGGIN_POINTERS

/* This is the api to use if you want to write a plugin for Bertray


No NOT use funtions from betray.h but do use defines and typedefs

To build a library you need to include this file, betray.h, b_keys.h, 
b_plugin_api.c, b_plugin_api_internal.h and the files for the libs 
Forge (forge.h and files starting with "f_") and Imagine (imagine.h 
and files starting with "i_").

Your plug-in will need to implement:
 
void betray_plugin_init(void)

In this function you can then set upp all callbacks you need. See
b_plugin_templet.c for sample code.

Betray plugins defining a callback for betray_plugin_callback_set_image_warp in betray_plugin_init will be assuiged their own full OpenGL context. All assets of the applications Context will be shared with this context. Plugins can use this secondary context to retrive the graphics from the application in order to modify it befoire drawing it to screen. Examples of uses for this is perspective correction, color correction, screenshot captiure and much more. 
*/

/* ------ betray state -------------------- 
Functions to get access to Betray State. */

extern BInputState	*betray_plugin_get_input_state(void); /* Access the current state. It is illegal for any plugion to modify this structure in any way */
extern BContextType	betray_plugin_context_type_get(); /* Find out what kind of context OpenGL is running */
extern double		betray_plugin_screen_mode_get(uint *x_size, uint *y_size, boolean *fullscreen);  /*Get the current screen mode of the application */
extern void			*betray_plugin_gl_proc_address_get(void); /* Get the  gl Proc address for the plugin context. Not that if the pluging ios drawing using OpenGL it will not use the same context as the host application. */

/* ----- Clipboard  -----------------------
Just like any betray application, a blugin has access to the clopboard. */

extern void		betray_plugin_clipboard_set(char *text); /* Set the clicpboard text. */
extern char		*betray_plugin_clipboard_get(void); /* Get the Clipboiard text */ 


/* ----- Callbacks ---------------------
A betray pluigin can set a number of plugins in order to influence how betray works */
 
extern void		betray_plugin_callback_set_main(void (*main_loop)(BInputState *input)); /* Set a callback for the main loop. If set, the callback will be called once evbery frame */
extern void		betray_plugin_callback_set_view_vantage(void (*vantage)(float *pos), boolean modify); /* Set a callback to update the vantage of the viewer to the screen. This can be used by tracking hardware to create a prespective correct display. If "Modify" is set the plugin will modify the vantage set by the application, if not itt will overwrite the vantage set by the application.*/
extern void		betray_plugin_callback_set_view_direction(void (*matrix)(float *matrix));  /* Set a callback to update the view direction 4X4 matrix. This is useful for hardware like headmounted displays. */
#if defined _WIN32
extern void		betray_plugin_callback_set_event_pump(uint event, boolean exclusive, void (*event_loop)(BInputState *input, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)); /* Set a callback to give you access to windows event pump. Obviuslyu windows only. */
#endif 

/* ----- Image manipulation ---------
A plugin is able to take over the main rendering loop and have the application render to one or more back buffewrs instead of the screen so that the plugin can modify the render output before drawing it to screen. If betray_plugin_callback_set_image_warp is called a separate OpenGL context will be created to the plugin so that it can set state and draw indipendently from the application. If the application sets the betray_plugin_callback_set_image_warpcallback, it needs to in that call back to call betray_plugin_application_draw to signal to the application that it needs it to draw its graphics to a specific FBO. betray_plugin_callback_set_image_warp can only be called once and only inside the betray_plugin_init function. */  

extern void		betray_plugin_callback_set_image_warp(boolean (*image_warp)(BInputState *input, char *name)); /* Set a callback to give you access to the image as a texture, and lets you return a different texture for screen drawing. If the given callback returns false, betray will ignore it and itself trigger the application to draw. This call must be called BEFORE any openGL calls can be made, since it is responcible for creating a separate OpenGL context.*/
extern void		betray_plugin_application_draw(uint fbo, uint x_size, uint y_size, float *vantage, boolean vantage_modify, float *matrix); /* issues a draw call to the application to draw to a specific FBO, to a specific screen size. If the aplication binds FBO 0, this FBO will be bound instead. The plugin can modify the vantage and transformation matrix in order to produce multiple different vantages in order to support cave, or sterioscopic applications. */

/* ----- Sound call backs --------
The sound API mirrors the betray sound API */

extern uint		betray_plugin_audio_unit_create(); /* you need to create a autio unit id in order to create callbacks*/

extern void		betray_plugin_callback_set_audio_sound_create(uint audio_unit_id, uint (*func)(uint type, uint stride, uint length, uint frequency, void *data, char *name)); /* Set a call back for betray_audio_sound_create. */
extern void		betray_plugin_callback_set_audio_sound_destroy(uint audio_unit_id, void (*func)(uint sound)); /* Set a call back for betray_audio_sound_create. */
extern void		betray_plugin_callback_set_audio_sound_play(uint audio_unit_id, uint (*func)(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete)); /* Set a call back for betray_audio_sound_play. */
extern void		betray_plugin_callback_set_audio_sound_set(uint audio_unit_id, void (*func)(uint play, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient)); /* Set a call back for betray_audio_sound_set. */
extern void		betray_plugin_callback_set_audio_sound_is_playing(uint audio_unit_id, boolean (*func)(uint play)); /* Set a call back for betray_audio_sound_is_playing. */
extern void		betray_plugin_callback_set_audio_sound_stop(uint audio_unit_id, void (*func)(uint play)); /* Set a call back for betray_audio_sound_stop. */

extern void		betray_plugin_callback_set_audio_stream_create(uint audio_unit_id, uint (*func)(uint frequency, float *pos, float *vector,  float volume, boolean ambient)); /* Set a call back for betray_audio_stream_create. */
extern void		betray_plugin_callback_set_audio_stream_destroy(uint audio_unit_id, void (*func)(uint stream)); /* Set a call back for betray_audio_stream_destroy. */
extern void		betray_plugin_callback_set_audio_stream_feed(uint audio_unit_id, void (*func)(uint stream, uint type, uint stride, uint length, void *data)); /* Set a call back for betray_audio_stream_feed. */
extern void		betray_plugin_callback_set_audio_stream_buffer_left(uint audio_unit_id, uint (*func)(uint stream)); /* Set a call back for betray_audio_stream_buffre_left. */
extern void		betray_plugin_callback_set_audio_stream_set(uint audio_unit_id, void (*func)(uint stream, float *pos, float *vector,  float volume, boolean ambient)); /* Set a call back for betray_audio_stream_set. */

extern void		betray_plugin_callback_set_audio_listener(uint audio_unit_id, void (*func)(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound)) /* Set a call back for betray_audio_listener. */;
extern void		betray_plugin_callback_set_audio_read(uint audio_unit_id, uint (*func)(void *data, uint type, uint buffer_size), uint channels, float *vectors); /*  Set up a Audio recording unit, by defining the number of channels it can record, its verctors (array of 3 * channels floats) and the callback to be called by the application to get access to the recorded data.*/

extern float	betray_plugin_audio_master_volume_get(void); /* Returns the volume level set by the application from Zero to One*/


/*----------- Allocate inputs ------
When a input device wants to expose its functionality to Betray it needs to allocate new buttons, axis and pointers. Input devices can also callocate new users as one singler application may have many users whos input should be treated differently. If the input only provides functionalty for a single user, you can use the defalut user Zero. */

#define BETRAY_PLUGGIN_UNDEFINED_KEY

extern uint		betray_plugin_user_allocate(void);  /* Allocate a user */

extern uint		betray_plugin_input_device_allocate(uint user_id, char *name); /* Allocate input device. */
extern uint		betray_plugin_input_device_free(uint id); /* Free input device. */

extern uint		betray_plugin_button_allocate(uint code, char *name); /* Allocate a new button. The Code parameter refers to the Betray Key code the button corresponds to. If there is no Key code that fits, this parameter should be set to -1. Name refers to a string describing the button. The function returns an id to be used when the button is triggerd. */ 
extern void		betray_plugin_button_set(uint user_id, uint id, boolean press, uint character); /* Call this Function to set the current state of a button. */
extern void		betray_plugin_button_free(uint id); /* Frees the button id, if the device is nolonger active. */

extern uint		betray_plugin_pointer_allocate(uint user_id, uint device_id, uint button_count, float x, float y, float z, float *origin, char *name, boolean draw); /* Allocate a pointer id for anew pointer. */ 
extern uint		betray_plugin_pointer_set(uint id, float x, float y, float z, float *origin, boolean *buttons); /* Set the current state of the button.,*/
extern void		betray_plugin_pointer_free(uint id); /* Frees the pointer id, if the device is nolonger active. */

extern uint		betray_plugin_axis_allocate(uint user_id, uint device_id, char *name, BAxisType type, uint axis_count);  /* Allocates a new axis, and describes its user id, name type and axis count. */
extern void		betray_plugin_axis_set(uint id, float axis_x, float axis_y, float axis_z); /* Sets the current state of the axis. */
extern void		betray_plugin_axis_free(uint id); /* Frees the axis id, if the device is nolonger active. */

/* ------- Settings -------
In order to comunicate with a host application, a plugin can expose settings to betray. All betray applications are recomended to support a way for users to modify these settings. */

typedef enum{
	BETRAY_ST_TRIGGER, /*Creates a trigger event for a plugi. Useful for things like screen captiure. */
	BETRAY_ST_TOGGLE, /*A simple boolean to turn on or off functionality. */
	BETRAY_ST_SELECT, /*A Selector where the application can chose form one of a number of named optrions. */
	BETRAY_ST_STRING, /*A text string option. */
	BETRAY_ST_NUMBER_FLOAT, /*A floating point value. */
	BETRAY_ST_NUMBER_INT, /*A integer value. */
	BETRAY_ST_SLIDER, /*A Floating point value ranging from zero to one.*/
	BETRAY_ST_2D, /* A two dimational value ranged form, minus one to plus one. */
	BETRAY_ST_3D, /* A three dimational value ranged form, minus one to plus one. */ 
	BETRAY_ST_COLOR, /* A Color */
	BETRAY_ST_4X4_MATRIX, /* A 4x4 trnasfomration matrix */
}BSettingType;
extern uint		betray_settings_create(uint type, char *name, uint select_count, char **select_options); /* Create a new setting by giving it a name and a type. If the type is set to BETRAY_ST_SELECT, yopu must also give it the number of select options available and an array of strings describing the different options. */
extern uint		betray_settings_count(); /* Returns the number of settings available (This number will be static once betray_innit has been called)*/
extern BSettingType betray_settings_type(uint id); /* Returns the BSettingType of a specific setting*/
extern char		*betray_settings_name(uint id); /* Rewturns the name of a specific setting */


extern boolean	betray_settings_toggle_get(uint id); /* Reads out the current state of a setting with the type BETRAY_ST_TOGGLE. */
extern void		betray_settings_toggle_set(uint id, boolean	toggle); /* Sets the state of a setting with the type BETRAY_ST_TOGGLE. */

extern uint		betray_settings_select_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_SELECT. */
extern void		betray_settings_select_set(uint id, uint select); /* Sets the state of a setting with the type BETRAY_ST_SELECT. */
extern uint		betray_settings_select_count_get(uint id); /* returns the number of options available for selection.*/
extern char		*betray_settings_select_name_get(uint id, uint option); /* Returns the names of each individual option available. */

extern float	betray_settings_number_float_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_NUMBER_FLOAT. */
extern void		betray_settings_number_float_set(uint id, float number); /* Sets the state of a setting with the type BETRAY_ST_NUMBER_FLOAT. */

extern int		betray_settings_number_int_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_NUMBER_INT. */
extern void		betray_settings_number_int_set(uint id, int number); /* Sets the state of a setting with the type BETRAY_ST_NUMBER_INT. */

extern char		*betray_settings_string_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_STRING. */
extern void		betray_settings_string_set(uint id, char *string); /* Sets the state of a setting with the type BETRAY_ST__STRING. */

extern float	betray_settings_slider_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_SLIDER. */
extern void		betray_settings_slider_set(uint id, float slider); /* Sets the state of a setting with the type BETRAY_ST_SLIDER. */

extern void		betray_settings_2d_get(uint id, float *x, float *y);/* Reads out the current state of a setting with the type BETRAY_ST_2D. */
extern void		betray_settings_2d_set(uint id, float x, float y); /* Sets the state of a setting with the type BETRAY_ST_2D. */

extern void		betray_settings_3d_get(uint id, float *x, float *y, float *z); /* Reads out the current state of a setting with the type BETRAY_ST_3D. */
extern void		betray_settings_3d_set(uint id, float x, float y, float z); /* Sets the state of a setting with the type BETRAY_ST_3D. */

extern void		betray_settings_color_get(uint id, float *red, float *green, float *blue); /* Reads out the current state of a setting with the type BETRAY_ST_COLOR. */
extern void		betray_settings_color_set(uint id, float red, float green, float blue); /* Sets the state of a setting with the type BETRAY_ST_COLOR. */

extern void		betray_settings_4x4_matrix_get(uint id, float *matrix); /* Reads out the current state of a setting with the type BETRAY_ST_4X4_MATRIX. */
extern void		betray_settings_4x4_matrix_set(uint id, float *matrix); /* Sets the state of a setting with the type BETRAY_ST_4X4_MATRIX. */

/* ----- Windows specific ---- */

extern void		*betray_plugin_windows_window_handle_get(void); /* returns the window handle of the Windows window.*/


#endif
