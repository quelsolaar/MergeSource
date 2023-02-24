/* ------- Betray ------------

Betray is the main platform abstraction layer that lets you open a window that you can draw to and access user input, and other operat ing system specific functionality. Betray is designed to be implemented on any kind of polatform that supports some form of OpenGL (It would be popssibble to extend its capabilities to other graphics APIS too), it is therefor similar to GLUT and SDL. 

One of the main design goals with betray is to make it possible to write an applications that makes use of many different types of hardware setups, so that a single codebase can run on different hardware. If utilized corrrectly a single Betray application should be able to run on everything from a phone, to a laptop all the way to a sterioscopic headmounted display, or a multi user wall. The api is designed to try tto reduce the ammount of query needed to make the application run well on any hardware. For instance, all pointers are the same indipendently of if they are created using touch or one or more mice. The same goes for buttons and axis input. It is obviusly recomended that you try to test your application with as many different hardware setups as possible.

Betray is designed to be compiled in to any project, and therfore needs to be small, lightweight and not have any dependencies. On the other hand it is decirable for Betray to have as many dependencies to as many Hardware SDKs as possible in order to support a wide set of hardware. In order to solve this hardware paradox, Betray uses "imagine" to create a blugin interface where plugings can add functionality to betray. This way application developers can focus on building applications, and people who are interested in supporting hardware can write plugins that add functionality to Betray applications. Some functionality in Betrays API doesnt do anything until a plugin is present to provide the functionality (Like sound). A plugin can also expose settings to the applications using the settings API. If you are interested in writing a plugin for Betray, check out the separate betray_plugin_api.h.

Betray supports multi user inputs. This may be multiple game controlers, or more exotic setups like multi user multi touch walls. For most uses this does not matter to a developer, but in some cases (like matching a specific pointer activating a type-in feild to a specific keyboard) developers may want to take this in to account. All UI ellements in Seduce supports this.

The main loop:

In order to start betray and open a window the application needs to call betray_init, but this does not start the applications main loop. In order to start the main loop you first need to give betray a function pointer to call as the main loop using  betray_action_func_set and then call betray_launch_main_loop to launch the application main loop. If the window is closed, the betray_launch_main_loop function will return. When the main loop is called it gets called with a pointer to a structure named BInputState. This read only structure contains a lot of information about the current state, such as pointer positions and events. One of the most important parameters of this structure is "mode" that describes why the function was called. It can be either BAM_DRAW because the application is asked to redraw the graphics, BAM_EVENT because the application needs to respond to incomming events, or BAM_MAIN because time has elapsed. Ofther all three are cvalled for each frame, but this is not always the case. BAM_DRAW may not be called if the application is minimized, but can also be called multiple times for each frame to support thinjgs like sterioscopic rendering. The application can read out the parameter draw_id and draw_count in order to know how many draw calls are expected and what call is currently being requested. This is very useful since you may for instance only want to calculate shadow buffers once even if you are drawing two different outputs. For multi drawcals to work propperly it is important to make use of betray_view_vantage and betray_view_direction. BAM_EVENT may only be called if there are any input events worth prosessing, but should always call at least twice after any event due to the design of the pointer and key events. BAM_MAIN is always called even if the application is minimized. This makes it useful for things like processing and network tradfic.

The main design goal of using the same callback for all three calls is that it makes easy to implement high level widgets that, by taking the BInputStruct as a parameter can themselves figure out what to do. For exammple one may implement a function like:

booelan my_button(BInputState *input, float pos_x, float pos_y);

This function can now either draw a button, or detect if the button is being pressed separatly. The user do not have to describe the button more then once even if the button functionaly does two diffferent things. The UI library Seduce is built entierly arround this concept.

A simple sample program using betray is included in the source package and is called betray_test.c. This example uses desktop OpenGL. A different sample program called betray_relinquish_test.c uses the relinquish rendering wrapper and is therefore compatible with OpenGL ES 2.0

Betray depends on Forge, Imagine, and OpenGL.

Betray is currently implemented for windows (both 32 and 64 bit) and a iOS port exists (with less functionality obviusly). My plan is to port the layer to other platforms in the future. If you are interested in helping me let me know.

*/


#if !defined(BETRAY_H)
#define	BETRAY_H

#include "forge.h"

/* ----------------------------------------------------------------------------------------- */



//#define BETRAY_WIN32_SYSTEM_WRAPPER

/* On Windows, we also need the main platform header file. */
#if defined _WIN32
#include <windows.h>
#define BETRAY_CONTEXT_OPENGL
//#define BETRAY_CONTEXT_OPENGLES
#ifdef BETRAY_CONTEXT_OPENGL
#include <GL/gl.h>
#endif
#ifdef BETRAY_CONTEXT_OPENGLES
#include "EGL/egl.h"
#include "GLES2/gl2.h"
#endif
#define extern extern __declspec(dllexport)
#endif

#if defined __ANDROID__
#define BETRAY_CONTEXT_OPENGLES
#include "EGL/egl.h"
#include "GLES2/gl2.h"
#endif

#if defined __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define BETRAY_CONTEXT_OPENGLES
#include <OpenGLES/ES2/gl.h>
#elif TARGET_OS_MAC
/* We need OpenGL, which is a bit harder to locate on Macintosh systems. */
#define BETRAY_CONTEXT_OPENGL
#include <OpenGL/gl.h>
#endif
#endif

#include "b_keys.h"

#if defined (__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define BETRAY_CUSTOM_MAIN
#endif
#endif

//FK: on some platforms we need to have a custom main (betray_main)
#ifndef BETRAY_CUSTOM_MAIN
#define betray_main main
#endif

/* ----------------------------------------------------------------------------------------- */

#define B_POINTER_BUTTONS_COUNT 16  /* Maximum number of buttons ona pointing device */
#define B_MAX_EVENT_COUNT 64 /* maximum number of simultanius events per frame */
#define B_BUTTONS_COUNT (256 + 16)  /**/

/* ------ Pointer Structure ------
This structure describes a single pointer */

typedef struct{
	float	pointer_x; /* Current X position of the pointer ranging form -1 to +1 */
	float	pointer_y; /* Current Y position of the pointer ranging form -Aspect ratio to + aspect ratio */
	float	pointer_z; /* Current Z position of the pointer ranging form -Aspect ratio to + aspect ratio */
	float	click_pointer_x[B_POINTER_BUTTONS_COUNT]; /* X Position of last click event for each button on the device */
	float	click_pointer_y[B_POINTER_BUTTONS_COUNT];
	float	click_pointer_z[B_POINTER_BUTTONS_COUNT];
	float	delta_pointer_x; /* X Delta movement */
	float	delta_pointer_y; /* Y Delta movement */
	float	delta_pointer_z; /* Z Delta movement */
	float	origin[3]; /* Origin of pointer */
	boolean	button[B_POINTER_BUTTONS_COUNT]; /* Current button state */
	boolean	last_button[B_POINTER_BUTTONS_COUNT]; /* Button state of previous frame.  Use full in combination with the button parameter. Example: if(input->button[0] && !input->last_button) equals a click.*/
	uint	button_count; /* number of buttons on the device */
	char	name[32]; /* name of the device Example "Mouse"*/
	uint	user_id; /* The id of the user operating the pointer */
	uint	device_id; /*Device id of the pointer */
	boolean draw; /* Should applications draw this pointer to make it visible to the user */
}BInputPointerState; 

/* ------ Axis Enum -------
This enum describes the type of decice that is outputing a axis value */

typedef enum{
	B_AXIS_UNDEFINED,				/* Any axis that doesnt fit any pre definds description */
	B_AXIS_STICK,					/* Joystick */
	B_AXIS_SUBSTICK,				/* Thumb joystiq on top of joystick */
	B_AXIS_BUMPER_LEFT,				/* Left analog bumper top to bottom */
	B_AXIS_BUMPER_RIGHT,			/* Right analog bumper top to bottom */
	B_AXIS_PEDAL,					/* Pedals left to right ranging form 0 to 1*/
	B_AXIS_WHEEL,					/* Stearing Wheels */
	B_AXIS_CONTROLER_FORWARD,		/* Gyroscopc or other input defining forward vector of controler*/
	B_AXIS_CONTROLER_UP,			/* Gyroscopc or other input defining up vector of controler*/
	B_AXIS_CONTROLER_ACCELEROMETER, /* Accelerometer of controler */
	B_AXIS_CONTROLER_POS,			/* Pos of  controler */
	B_AXIS_SCREEN_FORWARD,			/* gyroscopc or other input defining forward vector of screen*/
	B_AXIS_SCREEN_UP,				/* gyroscopc or other input defining up vector of screen*/
	B_AXIS_SCREEN_ACCELEROMETER,	/* Accelerometer of screen */
	B_AXIS_SCREEN_POS,				/* Pos of screen */
	B_AXIS_OBJECT_FORWARD,			/* gyroscopc or other input defining forward vector of object beeing manipulated on screen*/
	B_AXIS_OBJECT_UP,				/* gyroscopc or other input defining up vector of object beeing manipulated on screen*/
	B_AXIS_OBJECT_ACCELEROMETER,	/* Accelerometer of object beeing manipulated on screen*/
	B_AXIS_OBJECT_POS,				/* Pos of object of object beeing manipulated on screen*/
	B_AXIS_ZOOM,					/* ZOOM */
	B_AXIS_SCORLL,					/* SCROLL */
	B_AXIS_SLIDER					/* SLIDER */
}BAxisType;

/* ----- Axis state -----
Teh current state of an axis */

typedef struct{
	float			axis[3]; /* position of axis ranged -1.0 to 1.0*/
	uint			axis_count; /* Number of axis in use (1-3) */
	BAxisType		axis_type; /* Axis description, see lis above for pre defined axis types */
	char			name[32]; /* Name of axis input */
	uint			user_id; /* User id of the Axis, Typicaly different if you have multiple contorllers */
	uint			device_id; /*Device id of the Axis, Typicaly different if you have multiple contorllers */
}BInputAxisState;

/* ----- Button state -----
The current state of a button*/

typedef struct{
	uint	button; /* the name of the button coresponding to the BETRAY_BUTTO_... defines. */
	uint	character; /* the unicode character this button corresponds to. -1 if not applicable*/
	boolean state; /* is the button going up or down. If yo want both its curent and previous state use the betray_button_get_up_down function*/
	uint	user_id; /* User id of the button.*/
	uint	device_id; /* Device id of the button */
}BButtonEvent;


/*------ Action Mode -------
For each frame the main loop is usualy called at least 3 times, once for udating the screen (BAM_DRAW) once for input events (MAM_EVENT) and once for computation (BAM_MAIN).  */

typedef enum{
	BAM_DRAW, /* re draw the frame but do not advance time. can be called multiple times for multi display or sterioscopic systems */
	BAM_EVENT, /* manage input, can be called multiple times if hardware has multipl separate input devices for multiple users, like more them one joypad */
	BAM_MAIN /* main idle loop, called even if the program is iconized and doesnt redraw or has input focus. Useful for keeping things like network conecctions alive.*/
}BActionMode;

/* ----- Input structure -----
A pointer to the BInputState structure is given when the main loop is executed, It contains the input state and executuon mode */

typedef struct{
	BInputPointerState *pointers; /* description of state of pointers (first is mouse pointer if availabel)*/
	uint			pointer_count; /* number of pointers currently active */
	BInputAxisState *axis; /* Analog input, joysticks, wheels, pedals, tracking....*/
	uint			axis_count; /* Number of axis curerently available */
	BButtonEvent	button_event[B_MAX_EVENT_COUNT]; /* Button events*/
	uint			button_event_count; /* Button Event count */ 
	double			delta_time; /* delta time for update */
	double			minute_time; /* timer in fractions of minutes that loops every minute, not syncronized with clock */
	uint			draw_id; /* current id if we have a multi dispalay output. By default zero. Useful in order to only have to render shadowmaps once for multiple displays*/
	uint			draw_count; /* The number of displays we will render */
	uint			user_count; /* the number of users */
	uint			frame_number; /*increments by one each renderd frame on all draw_ids*/
	BActionMode		mode; /* what is the purpouse of execution */
}BInputState;

/* ------- Button defines -----
Defines for all buttons. Note that they are variables, and are different on different platforms!*/

#ifdef BETRAY_BUTTON_BEGIN

#define BETRAY_BUTTON_CANCEL
#define BETRAY_BUTTON_BACK		
#define BETRAY_BUTTON_TAB
#define BETRAY_BUTTON_CLEAR
#define BETRAY_BUTTON_RETURN
#define BETRAY_BUTTON_SHIFT

#define BETRAY_BUTTON_MENU
#define BETRAY_BUTTON_PAUSE
#define BETRAY_BUTTON_CAPS_LOCK
#define BETRAY_BUTTON_ESCAPE

#define BETRAY_BUTTON_SPACE

#define BETRAY_BUTTON_PREV
#define BETRAY_BUTTON_NEXT

#define BETRAY_BUTTON_END
#define BETRAY_BUTTON_HOME

#define BETRAY_BUTTON_LEFT
#define BETRAY_BUTTON_UP
#define BETRAY_BUTTON_RIGHT
#define BETRAY_BUTTON_DOWN

#define BETRAY_BUTTON_SELECT
#define BETRAY_BUTTON_PRINT
#define BETRAY_BUTTON_EXECUTE
#define BETRAY_BUTTON_SCREENSHOT
#define BETRAY_BUTTON_INSERT
#define BETRAY_BUTTON_DELETE
#define BETRAY_BUTTON_BACKSPACE
#define BETRAY_BUTTON_HELP

#define BETRAY_BUTTON_0
#define BETRAY_BUTTON_1
#define BETRAY_BUTTON_2
#define BETRAY_BUTTON_3
#define BETRAY_BUTTON_4
#define BETRAY_BUTTON_5
#define BETRAY_BUTTON_6
#define BETRAY_BUTTON_7
#define BETRAY_BUTTON_8
#define BETRAY_BUTTON_9

#define BETRAY_BUTTON_A
#define BETRAY_BUTTON_B
#define BETRAY_BUTTON_C
#define BETRAY_BUTTON_D
#define BETRAY_BUTTON_E
#define BETRAY_BUTTON_F
#define BETRAY_BUTTON_G
#define BETRAY_BUTTON_H
#define BETRAY_BUTTON_I
#define BETRAY_BUTTON_J
#define BETRAY_BUTTON_K
#define BETRAY_BUTTON_L
#define BETRAY_BUTTON_M
#define BETRAY_BUTTON_N
#define BETRAY_BUTTON_O
#define BETRAY_BUTTON_P
#define BETRAY_BUTTON_Q
#define BETRAY_BUTTON_R	
#define BETRAY_BUTTON_S
#define BETRAY_BUTTON_T
#define BETRAY_BUTTON_U
#define BETRAY_BUTTON_V
#define BETRAY_BUTTON_W
#define BETRAY_BUTTON_X
#define BETRAY_BUTTON_Y
#define BETRAY_BUTTON_Z

#define BETRAY_BUTTON_NUM_0
#define BETRAY_BUTTON_NUM_1
#define BETRAY_BUTTON_NUM_2
#define BETRAY_BUTTON_NUM_3
#define BETRAY_BUTTON_NUM_4
#define BETRAY_BUTTON_NUM_5
#define BETRAY_BUTTON_NUM_6
#define BETRAY_BUTTON_NUM_7
#define BETRAY_BUTTON_NUM_8
#define BETRAY_BUTTON_NUM_9

#define BETRAY_BUTTON_MULTIPLY
#define BETRAY_BUTTON_ADD
#define BETRAY_BUTTON_SUBTRACT
#define BETRAY_BUTTON_DIVIDED

#define BETRAY_BUTTON_PERIOD
#define BETRAY_BUTTON_COMMA

#define BETRAY_BUTTON_F1
#define BETRAY_BUTTON_F2
#define BETRAY_BUTTON_F3
#define BETRAY_BUTTON_F4
#define BETRAY_BUTTON_F5
#define BETRAY_BUTTON_F6
#define BETRAY_BUTTON_F7
#define BETRAY_BUTTON_F8
#define BETRAY_BUTTON_F9
#define BETRAY_BUTTON_F10
#define BETRAY_BUTTON_F11
#define BETRAY_BUTTON_F12
#define BETRAY_BUTTON_F13
#define BETRAY_BUTTON_F14
#define BETRAY_BUTTON_F15
#define BETRAY_BUTTON_F16
#define BETRAY_BUTTON_F17
#define BETRAY_BUTTON_F18
#define BETRAY_BUTTON_F19
#define BETRAY_BUTTON_F20
#define BETRAY_BUTTON_F21
#define BETRAY_BUTTON_F22
#define BETRAY_BUTTON_F23
#define BETRAY_BUTTON_F24

#define BETRAY_BUTTON_VOLUME_DOWN
#define BETRAY_BUTTON_VOLUME_UP
#define BETRAY_BUTTON_NEXT_TRACK
#define BETRAY_BUTTON_PREVIOUS_TRACK
#define BETRAY_BUTTON_STOP
#define BETRAY_BUTTON_PLAY_PAUSE

/* first row of face buttons */
#define BETRAY_BUTTON_FACE_A
#define BETRAY_BUTTON_FACE_B
#define BETRAY_BUTTON_FACE_C
#define BETRAY_BUTTON_FACE_D

/* second row of face buttons */
#define BETRAY_BUTTON_FACE_X
#define BETRAY_BUTTON_FACE_Y
#define BETRAY_BUTTON_FACE_Z
#define BETRAY_BUTTON_FACE_W

#define BETRAY_BUTTON_YES
#define BETRAY_BUTTON_NO
#define BETRAY_BUTTON_UNDO
#define BETRAY_BUTTON_REDO
#define BETRAY_BUTTON_CUT
#define BETRAY_BUTTON_COPY
#define BETRAY_BUTTON_PASTE
#define BETRAY_BUTTON_SELECT
#define BETRAY_BUTTON_SEARCH
#define BETRAY_BUTTON_SHOLDER_LEFT_A
#define BETRAY_BUTTON_SHOLDER_LEFT_B
#define BETRAY_BUTTON_SHOLDER_LEFT_C
#define BETRAY_BUTTON_SHOLDER_LEFT_D
#define BETRAY_BUTTON_SHOLDER_RIGHT_A
#define BETRAY_BUTTON_SHOLDER_RIGHT_B
#define BETRAY_BUTTON_SHOLDER_RIGHT_C
#define BETRAY_BUTTON_SHOLDER_RIGHT_D
#define BETRAY_BUTTON_SCROLL_UP
#define BETRAY_BUTTON_SCROLL_DOWN
#define BETRAY_BUTTON_SCROLL_LEFT
#define BETRAY_BUTTON_SCROLL_RIGHT
#define BETRAY_BUTTON_INVENTORY_NEXT
#define BETRAY_BUTTON_INVENTORY_PREVIOUS

#endif

/* ------ Context type ----- 
possible contexts that canbe opend, currently only OpenGL but others could be possible in the future */

typedef enum{
	B_CT_OPENGL, /* Normal OpenGL context */
	B_CT_OPENGLES2, /* OpenGL 2.0 ES Context*/
	B_CT_OPENGL_OR_ES /* Creating a context of either OpenGL Or OpenGLES*/
}BContextType;


/* ------- Functionality query Enum ------
Possible parameters to betray_support_functionality */

typedef enum{
	B_SF_USER_COUNT_MAX, /* whats the maximum number of users */
	B_SF_POINTER_COUNT_MAX, /* whats the maximum number of pointers */
	B_SF_FULLSCREEN, /* Does the system support full screen */
	B_SF_WINDOWED,/* Does the system support window mode */
	B_SF_VIEW_POSITION, /* Does the system manipulate view vantage */
	B_SF_VIEW_ROTATION, /* Does the system manipulate the view direction*/
	B_SF_MOUSE_WARP, /* Does the syetem support mouse warp*/
	B_SF_EXECUTE, /* Does the system support execute */
	B_SF_REQUESTER, /* Can the system support save/load requesters */
	B_SF_CLIPBOARD, /*Does the syetem support cut and paste */
	B_SF_COUNT /*Number of members of this Enum */
}BSupportedFunctionality;

#ifndef BETRAY_PLUGGIN_DEFINES


/* ------- Functionality query ------
Thies functions make it possible for an application to query what  Betray can do on the current platform */

extern uint		betray_support_context(BContextType context_type); /*Returns TRUE or FALSE of the implementation can creat a specific type of DDraw context */
extern uint		betray_support_functionality(BSupportedFunctionality funtionality); /* Returns an integer value that tells the application what functionality is availabel on the platform */


/* ------- Display Functionality ------
Display, screen mode and view angle API (Must always be supported) */

extern void		betray_init(BContextType context_type, int argc, char **argv, uint window_size_x, uint window_size_y, uint samples, boolean window_fullscreen, char *name);
extern BContextType	betray_context_type_get(); /* Returns the type of context currently active. Can only be called after betray_init. If you want to know what context is possible to create use betray_support_context. This function is useful for finding out what context was created if betray was called with the parameter B_CT_OPENGL_OR_ES*/
extern boolean	betray_screen_mode_set(uint x_size, uint y_size, uint samples, boolean fullscreen); /* Change screen mode*/
extern double	betray_screen_mode_get(uint *x_size, uint *y_size, boolean *fullscreen); /* Get the current screen size, and if it is in fullscreen. Any pameters can be set to NULL, if you are not interested in any parameter. The fuinction returns the aspect ratio of the window/display*/

typedef enum{
	BETRAY_SA_ACTION, /* Any interaction */
	BETRAY_SA_TITLE /* Readable text */
}BetraySafeArea;

extern void		betray_screen_mode_safe_get(BetraySafeArea safe_area, float *left, float *right, float *top, float *bottom); /* Get the aspect of areas of the display that is safe to use for action and title. */
extern void		betray_view_vantage(float *pos); /* The head position compared to screen positioned at 0, 0, 0 with the width randing form -1.0 to 1.0 horizontaly. Should be supported in order to do prespective corect rendering for Sterio scopics and Headtracking. If no headtracking is active the data pointed to will be unaffected. */
extern void		betray_view_direction(float *matrix);  /* A 4X4 rotation matrix that should be applied to all drawing in order to support multi display solutions and headmounted disp�lays. */
extern BInputState *betray_get_input_state(void); /* A fiuinction that always returns the current input state. Useful for debugging.*/



/* ------ Main loop ------
Action Func (must always be suported)*/

extern void		betray_action_func_set(void (*input_compute_func)(BInputState *data, void *user_pointer), void *user_pointer); /* set a function pointer that will be the main loop of the application */
extern void		betray_launch_main_loop(void); /* Start the main loop */

/* ------ Mouse Warp ------
On some platfoms the mous can be warped. Use betray_support_functionality(B_SF_MOUSE_WARP); to qquery if the functionality is available*/

extern void		betray_set_mouse_warp(boolean warp); /* warp mouse to never hit the edges. Renders pointer_x/y void */
extern void		betray_set_mouse_move(float x, float y); /* Moves the mouse pointer to a specific x,Y coordinate */


/* ----- File requesters -------
Thes functions are able to launch Save and load requesters, if the get functions return a pointer you need to free it */

extern void		betray_requester_save(char **types, uint type_count, void *id); /* launches a requester to save something. The id is a identyfier for the caller. */
extern void		betray_requester_load(char **types, uint type_count, void *id); /* launches ar equester to load something. The id is a identyfier for the caller. */
extern char		*betray_requester_save_get(void *id); /* Returns a path from the save requester. Will only return a string if the id matches the id given to betray_requester_save.*/
extern char		*betray_requester_load_get(void *id); /* Returns a path from the load requester. Will only return a string if the id matches the id given to betray_requester_load. */

/* ---- Clip board -------- 
Gives the application accesss the the platforms Cut and Paste buffers*/

extern void		betray_clipboard_set(char *text); /* Sets the clipboard buffers to a text string */
extern char		*betray_clipboard_get(void); /* Retrives a text string containg the current content of a text string. Dont forget to free it! */

/* ---- Launch URL -------- 
Functionality for launching URLs */

extern void		betray_url_launch(char *url); /* launches a url using the default browser */

/* ---- Timers -------------
Messure time. Timers can also be found  in the BInputState structure.*/

extern float	betray_time_delta_get(void); /* get delta time of current time step*/

/* ----- OpenGL --------
These functions manages the applications interactions with the OpenGL Context*/

extern void		betray_gl_context_update_func_set(void (*context_func)(void)); /* This function lets you set a function pointer that will be called if OpenGL context is lost. May not happen ever on some platforms, but should alwayss be implemented.*/
extern void		*betray_gl_proc_address_get(void); /* Get the proc address for OpenGL extensions */

/* ------- Button and key -------
All keyboard and buttons can be read out using this API. All thes functiuons are convinience functions, as you can use the events in the BInputState structurte instead to get the same infromation. */

extern boolean	betray_button_get(uint user_id, uint button); /* was this button pressed this time step */
extern void		betray_button_get_up_down(uint user_id, boolean *press, boolean *last_press, uint button); /* what is the state of a button this and previous time step*/
extern boolean	betray_button_get_name(uint user_id, uint button, char *name, uint buffer_size); /* get the name of the key*/
extern void 	betray_button_keyboard(uint user_id, boolean show); /* Tell the system that a beyboard is needed. (god for syetms with on screen keyboards)*/

#endif

/* ----- Sound Clip ---- 
Betray supports full 3D sound playback of preloaded clips. The user loads a mono clip in to the 3D sound engine and can then trigger it to play and movce anywhere in the environment with full surround and dopler. Please note that all these functions are defered to plugins to actiulay record and play out the sound. */

#define BETRAY_TYPE_INT8 0
#define BETRAY_TYPE_INT16 1
#define BETRAY_TYPE_INT32 2
#define BETRAY_TYPE_FLOAT32 3

#ifndef BETRAY_PLUGGIN_DEFINES

extern uint		betray_audio_sound_create(uint type, uint stride, uint length, uint frequency, void *data, char *name); /* Load a sound clip in to betrays sound system. */
extern void		betray_audio_sound_destroy(uint sound); /* Unload a sound clip. */
extern uint		betray_audio_sound_play(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete); /* Play a sound at a particular speed, volume traeling along a vector. If looped the sound will keep replaying until stopped. The function returns a handle that can be used to stop, or modify the sound later. All sounds need to be deleted manulay once played, exept you the auto delete is set. Auto delete will automaticaly remove the sound once it has been played. If Auto delete is set to true you can not modify the sound after creating it. By setting the sound to ambient the position of the sound will be in the same space as the listener and will therfor not be afected by head movements of the listener.*/
extern void		betray_audio_sound_set(uint play, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient); /* change settings for an already playing sound*/
extern boolean	betray_audio_sound_is_playing(uint play); /* Returns true or false if a sound is still playing. */
extern void		betray_audio_sound_stop(uint play); /* Stop playing a sound */

/* ----- Sound Stream  ---- 
Betray supports full 3D sound playback of preloaded clips. The user loads a mono clip in to the 3D sound engine and can then trigger it to play and movce anywhere in the environment with full surround and dopler. Please note that all these functions are defered to plugins to actiulay record and play out the sound. */


extern uint		betray_audio_stream_create(uint frequency, float *pos, float *vector,  float volume, boolean ambient); /* load sound data in to betray*/
extern void		betray_audio_stream_destroy(uint stream); /* Destroys a stream */
extern void		betray_audio_stream_feed(uint stream, uint type, uint stride, uint length, void *data); /* Feeds a stream, with */
extern uint		betray_audio_stream_buffer_left(uint stream); /* unload sound data */
extern void		betray_audio_stream_set(uint stream, float *pos, float *vector,  float volume, boolean ambient); /* change settings for an already playing sound*/


/* ----- Sound Listener  ---- 
In order to transform the position of the listener Betray betray has a number of functions that can be used to define the listener. */

extern void		betray_audio_listener(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound); /* Define listener position by possition, movement vector, Forward vector, right vector, scale. It also defines the speed of sound in order to propperly compute dopler. Objects traveling faster then the speed of sound in relation to the listener may not play acuratly but are expected to fail gracefully*/
extern void		betray_audio_master_volume_set(float volume); /* Set master volume */
extern float	betray_audio_master_volume_get(void); /* Get current volume */

/* ----- Sound Listener  ---- 
Betray has a fill recording API that lets you recourd sound sources. */

extern uint		betray_audio_read_units(); /* Returns the number of units available for recording. */
extern uint		betray_audio_read_channels_availables(uint unit_id); /* Returns the number of channels each recording unit records */
extern void		betray_audio_read_channel_directions(uint unit_id, float *vec); /* returns a 3d vector of the channel */
extern uint		betray_audio_read(uint unit_id, void *data, uint type, uint buffer_size); /* Reads out data from a channel on a recording device to the buffer defined as data. The "type" parameter defines the type of data to be read and the stride of the read data counted in the size of the specified type. The buffer_size defines the maximum number of samples to be read. So the buffer needs to be atleast the data types size times the buffer_size times the stride. The function returns the number of samples actiualy read. */

#endif



/* ------ Settings ------
Since a larg portion of betrays functionality may come in the from of plugins, Plugins needs a way to comunicate with applications to expose settings. Betray therfor has a settings api where an a pplication can query the settings, build an interface for the user, and then let theuser modify the settings. Since a plugging may be added after the release of a betray apoplication, it is important that this API is supported correctly so that the application can manage even future plugins. Seduce has dedicated functionality. */

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

#ifndef BETRAY_PLUGGIN_DEFINES

extern uint		 betray_settings_count(); /* Returns the number of settings available (This number will be static once betray_innit has been called)*/
extern BSettingType betray_settings_type(uint id); /* Returns the BSettingType of a specific setting*/
extern char		*betray_settings_name(uint id); /* Rewturns the name of a specific setting */

extern void     betray_settings_trigger(uint id);  /* Triggers a setting with the type BETRAY_ST_TOGGLE. */

extern boolean	betray_settings_toggle_get(uint id); /* Reads out the current state of a setting with the type BETRAY_ST_TOGGLE. */
extern void		betray_settings_toggle_set(uint id, boolean	toggle); /* Sets the state of a setting with the type BETRAY_ST_TOGGLE. */

extern uint		betray_settings_select_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_SELECT. */
extern void		betray_settings_select_set(uint id, uint select); /* Sets the state of a setting with the type BETRAY_ST_SELECT. */
extern uint		betray_settings_select_count_get(uint id); /* returns the number of options available for selection.*/
extern char		*betray_settings_select_name_get(uint id, uint option); /* Returns the names of each individual option available. */

extern char		*betray_settings_string_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_STRING. */
extern void		betray_settings_string_set(uint id, char *string); /* Sets the state of a setting with the type BETRAY_ST__STRING. */

extern float	betray_settings_number_float_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_NUMBER_FLOAT. */
extern void		betray_settings_number_float_set(uint id, float number); /* Sets the state of a setting with the type BETRAY_ST_NUMBER_FLOAT. */

extern int		betray_settings_number_int_get(uint id);/* Reads out the current state of a setting with the type BETRAY_ST_NUMBER_INT. */
extern void		betray_settings_number_int_set(uint id, int number); /* Sets the state of a setting with the type BETRAY_ST_NUMBER_INT. */

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

#endif

#undef extern

#endif
