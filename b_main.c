
/*#include "opengl3.0.h"*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "forge.h"
#include "imagine.h"
#include "betray.h"

extern boolean b_sdl_system_wrapper_set_display(uint *size_x, uint *size_y, boolean *full_screen);
extern boolean b_glut_system_wrapper_set_display(uint size_x, uint size_y, boolean full_screen);
extern boolean b_win32_system_wrapper_set_display(uint size_x, uint size_y, boolean full_screen);
extern boolean b_android_init_display(uint *window_size_x, uint *window_size_y, char *name);
extern void betray_plugin_button_set(uint user_id, uint id, boolean state, uint character);
extern void *betray_plugin_windows_window_handle_get(void);
extern void *betray_plugin_windows_device_context_handle_get(void);
extern void betray_key_codes_init(void);
extern boolean betray_activate_context(void *context);
extern void *b_create_context();
extern void	betray_device_init();
#ifdef _WIN32
#pragma comment(lib, "OpenGL32.lib")
extern void APIENTRY betray_glBindFramebufferEXT(GLenum target, GLuint framebuffer);
#else
extern void betray_glBindFramebufferEXT(GLenum target, GLuint framebuffer);
#endif
boolean betray_capture = FALSE;
void (*b_context_func)(void) = NULL;
BContextType betray_context_type;
uint BGlobal_draw_state_fbo;

typedef struct{
	uint		x_size;
	uint		y_size;
	boolean		fullscreen;
}GraphicsMode;

#ifdef _WIN32 

typedef struct{
	void (*event_pump)(BInputState *input, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	uint event;
	boolean exclusive;
}BEventCallback;

#endif

typedef struct{
	uint (*sound_create)(uint type, uint stride, uint length, uint frequency, void *data, char *name);
	void (*sound_destroy)(uint sound);
	uint (*sound_play)(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete);
	boolean (*sound_is_playing)(uint play);
	void (*sound_set)(uint play, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient);
	void (*sound_stop)(uint play);
	uint (*stream_create)(uint frequency, float *pos, float *vector,  float volume, boolean ambient);
	void (*stream_destroy)(uint stream);
	void (*stream_feed)(uint stream, uint type, uint stride, uint length, void *data);
	uint (*stream_buffer_left)(uint stream);
	void (*stream_set)(uint stream, float *pos, float *vector,  float volume, boolean ambient);
	void (*listener)(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound);
	uint (*read)(void *data, uint type, uint buffer_size);
	uint channels;
	float *vectors;
}BAudioCallback;


typedef struct{
	void (*func)(float *pos);
	boolean modify;
}BVantage;

typedef struct{
	boolean (*func)(BInputState *input); 
	void *context;
	char *name;
}BImageWarp;

typedef struct{
	void (**main_loop)(BInputState *input);
	uint main_loop_count;
#ifdef _WIN32 
	BEventCallback *event_pump;
#endif
	uint event_pump_count ;
	BVantage *view_vantage;
	uint view_vantage_count ;
	void (**view_direction)(float *matrix);
	uint view_direction_count;
	BAudioCallback *audio;
	uint audio_count;
	BImageWarp *image_warp; 
	uint image_warp_count;
}BCallbacks;

typedef struct{
	BSettingType type;
	char name[64];
	union{
		boolean trigger;
		boolean toggle;
		char *string;
		struct{
			uint selected;
			uint select_count;
			char **options;
		}select;
		float number_float;
		int number_int;
		float slider;
		float dimentions[3];
		float color[3];
		float matrix[16];
	}data;
}BSetting;

typedef struct{
	uint x_size;
	uint y_size;
	float *vantage;
	boolean vantage_modify;
	float *matrix;
	void *context;
}BDrawState;

typedef struct{
	uint *sound_ids;
	uint sound_count;
	uint sound_allocated;
	uint *source_ids;
	uint source_count;
	uint source_allocated;
	uint *stream_ids;
	uint stream_count;
	uint stream_allocated;
	float master_volume;
}BSSound;

typedef struct{
	uint id;
	uint user_id;
	char name[64];
}BDevice;


struct{
	BInputState		input;
	boolean			input_clear;
	BActionMode		action_mode;
	GraphicsMode	screen_mode;
	boolean			sterioscopic;
	char			caption[512];
	void			(*action_func)(BInputState *input, void *user);
	void			*action_func_data;
	void			(*context_func)(void);
	uint32			time[2];
	BCallbacks		plugins;
	BSetting		*settings;
	uint			setting_count;
	BDrawState		draw_state;
	BSSound			sounds;
	BDevice			*devices;
	uint			device_allocated;
	uint			device_count;
	BButtonEvent    held_buttons[B_MAX_EVENT_COUNT];
	uint			held_button_count;
}BGlobal;

float BGlobal_occular_distance = 0.05;


extern void betray_desktop_size_get(uint *size_x, uint *size_y);
extern boolean b_win32_screen_mode(uint size_x, uint size_y);
extern void b_win32_window_close();

extern boolean b_init_display_opengl(uint size_x, uint size_y, boolean full_screen, uint samples, char *caption, boolean *sterioscopic);
extern boolean b_win32_init_display_opengles2(uint size_x, uint size_y, boolean full_screen, uint samples, char *caption, boolean *sterioscopic);

void betray_reshape_view(uint x_size, uint y_size)
{
	BGlobal.screen_mode.x_size = x_size;
	BGlobal.screen_mode.y_size = y_size;
}

void betray_draw_state_reset(void)
{
	BGlobal.draw_state.x_size = BGlobal.screen_mode.x_size;
	BGlobal.draw_state.y_size = BGlobal.screen_mode.y_size;
	BGlobal_draw_state_fbo = 0;
	BGlobal.draw_state.vantage = NULL;
	BGlobal.draw_state.vantage_modify = TRUE;
	BGlobal.draw_state.matrix = NULL;
}


/* Error source: */

#define GL_DEBUG_SOURCE_API                                     0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM                           0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER                         0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY                             0x8249
#define GL_DEBUG_SOURCE_APPLICATION                             0x824A
#define GL_DEBUG_SOURCE_OTHER                                   0x824B

/* Error type: */

#define GL_DEBUG_TYPE_ERROR                                     0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR                       0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR                        0x824E
#define GL_DEBUG_TYPE_PORTABILITY                               0x824F
#define GL_DEBUG_TYPE_PERFORMANCE                               0x8250
#define GL_DEBUG_TYPE_OTHER                                     0x8251
#define GL_DEBUG_TYPE_MARKER                                    0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP                                0x8269
#define GL_DEBUG_TYPE_POP_GROUP                                 0x826A

#define GL_DEBUG_SEVERITY_HIGH                                  0x9146
#define GL_DEBUG_SEVERITY_MEDIUM                                0x9147
#define GL_DEBUG_SEVERITY_LOW                                   0x9148

#ifdef _WIN32
uint (APIENTRY *betray_glGetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, char *messageLog);
#else
uint (*betray_glGetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, char *messageLog);
#endif

void b_debug_message() 
{
	char message_log[2049];
	uint types, severities, sources, ids;
	GLsizei lengths;
    return;
//	betray_glGetDebugMessageLog = wglGetProcAddress("glGetDebugMessageLog");
	if(betray_glGetDebugMessageLog == NULL)
		return;
	while(betray_glGetDebugMessageLog(1, 2048, &sources, &types, &ids, &severities, &lengths, message_log))
	{
		message_log[lengths] = 0;
		switch(sources)
		{
			case GL_DEBUG_SOURCE_API :
				printf("OpenGL API ");
			break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM :
				printf("OpenGL WINDOW ");
			break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER :
				printf("OpenGL COMPILER ");
			break;
			case GL_DEBUG_SOURCE_THIRD_PARTY :
				printf("OpenGL THIRD PARTY ");
			break;
			case GL_DEBUG_SOURCE_APPLICATION :
				printf("OpenGL APPLICATION ");
			break;
			case GL_DEBUG_SOURCE_OTHER :
				printf("OpenGL UNDEFINES ");
			break;
		}
		switch(sources)
		{
			case GL_DEBUG_TYPE_ERROR :
				printf("ERROR: ");
			break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR :
				printf("DEPRECATED BEHAVIOR: ");
			break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR :
				printf("UNDEFINED BEHAVIOR: ");
			break;
			case GL_DEBUG_TYPE_PORTABILITY :
				printf("PORTABILITY ERROR: ");
			break;
			case GL_DEBUG_TYPE_PERFORMANCE :
				printf("PERFORMANCE ISSUE: ");
			break;
			case GL_DEBUG_TYPE_OTHER :
				printf(" ISSUE: ");
			break;
			case GL_DEBUG_TYPE_MARKER :
				printf("MARKER: ");
			break;
			case GL_DEBUG_TYPE_PUSH_GROUP :
				printf("PUSH GROUP: ");
			break;
			case GL_DEBUG_TYPE_POP_GROUP :
				printf("POP GROUP: ");
			break;
		}
		printf(message_log);
		switch(sources)
		{
			case GL_DEBUG_SEVERITY_LOW :
				printf(" - SEVERITY LOW ");
			break;
			case GL_DEBUG_SEVERITY_MEDIUM :
				printf(" - SEVERITY MEDIUM ");
			break;
			case GL_DEBUG_SEVERITY_HIGH :
				printf(" - SEVERITY HIGH ");
			break;
		}
		printf("\n");

	}
}


void betray_gl_context_update_func_set(void (*context_func)(void))
{
	BGlobal.context_func = context_func;
}

boolean betray_screen_mode_set(uint x_size, uint y_size, uint samples, boolean fullscreen)
{
	uint desktop_x, desktop_y;

	if(BGlobal.screen_mode.x_size == x_size &&
		BGlobal.screen_mode.y_size == y_size &&
		BGlobal.screen_mode.fullscreen == fullscreen)
		return TRUE;

	betray_desktop_size_get(&desktop_x, &desktop_y);
	if(x_size == 0 || y_size == 0)
	{
		x_size = desktop_x;
		y_size = desktop_y;
	}

	#ifdef BETRAY_SDL_SYSTEM_WRAPPER
	if(b_sdl_system_wrapper_set_display(&x_size, &y_size, &fullscreen) != TRUE)
		return FALSE;
	#endif 

	#ifdef BETRAY_GLUT_SYSTEM_WRAPPER
	if(b_glut_system_wrapper_set_display(x_size, y_size, fullscreen) != TRUE)
		return FALSE;
	#endif

	#ifdef BETRAY_GLFW_SYSTEM_WRAPPER
	if(b_glfw_system_wrapper_set_display(x_size, y_size, fullscreen) != TRUE)
		return FALSE;
	#endif

	#ifdef BETRAY_WIN32_SYSTEM_WRAPPER

	if(BGlobal.screen_mode.fullscreen && fullscreen)
		b_win32_screen_mode(x_size, y_size);

	if(BGlobal.screen_mode.fullscreen && !fullscreen)
		if(BGlobal.screen_mode.x_size != desktop_x || BGlobal.screen_mode.y_size != desktop_y)
			b_win32_screen_mode(desktop_x, desktop_y);

	if(BGlobal.screen_mode.fullscreen && fullscreen)
		if(BGlobal.screen_mode.x_size != x_size || BGlobal.screen_mode.y_size != y_size)
			b_win32_screen_mode(x_size, y_size);

	b_win32_window_close();
	#ifdef BETRAY_CONTEXT_OPENGL
	if(betray_context_type == B_CT_OPENGL)
		if(!b_init_display_opengl(x_size, y_size, fullscreen, samples, BGlobal.caption, &BGlobal.sterioscopic))
			return FALSE;
	#endif

	#ifdef BETRAY_CONTEXT_OPENGLES
	if(betray_context_type == B_CT_OPENGLES2)
		if(!b_win32_init_display_opengles2(x_size, y_size, fullscreen, samples, BGlobal.caption, &BGlobal.sterioscopic))
			return FALSE;
	#endif
	#endif

	BGlobal.screen_mode.x_size = x_size;
	BGlobal.screen_mode.y_size = y_size;
	BGlobal.screen_mode.fullscreen = fullscreen;
	betray_reshape_view(x_size, y_size);
	if(BGlobal.context_func != NULL)
	{
		BGlobal.context_func();
	}
	return TRUE;
}


double betray_screen_mode_get(uint *x_size, uint *y_size, boolean *fullscreen)
{
	if(x_size != NULL)
		*x_size = BGlobal.draw_state.x_size;
	if(y_size != NULL)
		*y_size = BGlobal.draw_state.y_size;
	if(fullscreen != NULL)
		*fullscreen = BGlobal.screen_mode.fullscreen;
	return (double)BGlobal.draw_state.y_size / (double)BGlobal.draw_state.x_size;
}

uint betray_safe_area_setting[2] = {-1, -1};

void betray_screen_mode_safe_get(BetraySafeArea safe_area, float *left, float *right, float *top, float *bottom)
{
	if(left != NULL)
		*left = BGlobal.settings[betray_safe_area_setting[0]].data.slider * -1.0; 
	if(right != NULL)
		*right = BGlobal.settings[betray_safe_area_setting[0]].data.slider * 1.0;
	if(top != NULL)
		*top = BGlobal.settings[betray_safe_area_setting[1]].data.slider * (float)BGlobal.draw_state.y_size / (float)BGlobal.draw_state.x_size;
	if(bottom != NULL)
		*bottom = BGlobal.settings[betray_safe_area_setting[1]].data.slider * -(float)BGlobal.draw_state.y_size / (float)BGlobal.draw_state.x_size;
}


extern void pxc_facial_tracking(float *view, float delta_time);

void betray_view_vantage(float *pos)
{
	float tmp[3] = {0, 0, 1};
	if(BGlobal.draw_state.vantage != NULL)
	{
		if(!BGlobal.draw_state.vantage_modify)
		{
			pos[0] = BGlobal.draw_state.vantage[0];
			pos[1] = BGlobal.draw_state.vantage[1];
			pos[2] = BGlobal.draw_state.vantage[2];
		}
	}
	if(BGlobal.plugins.view_vantage_count != 0)
	{
		uint i;
		if(BGlobal.draw_state.vantage_modify)
		{
			for(i = 0; i < BGlobal.plugins.view_vantage_count; i++)
			{
				if(!BGlobal.plugins.view_vantage[i].modify)
				{
					BGlobal.plugins.view_vantage[i].func(pos);
					break;
				}
			}
		}
		for(i = 0; i < BGlobal.plugins.view_vantage_count; i++)
		{
			if(BGlobal.plugins.view_vantage[i].modify)
			{
				BGlobal.plugins.view_vantage[i].func(tmp);
				pos[0] += tmp[0];
				pos[1] += tmp[1];
				pos[2] += tmp[2];
			}
		}
	}
	if(BGlobal.draw_state.vantage != NULL)
	{
		if(BGlobal.draw_state.vantage_modify)
		{
			pos[0] += BGlobal.draw_state.vantage[0];
			pos[1] += BGlobal.draw_state.vantage[1];
			pos[2] += BGlobal.draw_state.vantage[2];
		}
	}
}

void betray_view_direction(float *matrix) 
{
	float m[16], m2[16];
	uint i, j;
	if(BGlobal.draw_state.matrix == NULL)
	{
		matrix[0] = 1;
		matrix[1] = 0;
		matrix[2] = 0;
		matrix[3] = 0;
		matrix[4] = 0;
		matrix[5] = 1;
		matrix[6] = 0;
		matrix[7] = 0;
		matrix[8] = 0;
		matrix[9] = 0;
		matrix[10] = 1;
		matrix[11] = 0;
		matrix[12] = 0;
		matrix[13] = 0;
		matrix[14] = 0;
		matrix[15] = 1;
	}else
		for(i = 0; i < 16; i++)
			matrix[i] = BGlobal.draw_state.matrix[i];
	for(i = 0; i < BGlobal.plugins.view_direction_count; i++)
	{
		BGlobal.plugins.view_direction[i](m);
		f_matrix_multiplyf(m2, m, matrix);
		for(j = 0; j < 16; j++)
			matrix[j] = m2[j];
	}
}

extern void b_sdl_init_display(uint *size_x, uint *size_y, boolean *full_screen, char *caption);
extern void b_glut_init_display(int argc, char **argv, uint size_x, uint size_y, boolean full_screen, char *caption);

void betray_plugin_callback_set_main(void (*main_loop)(BInputState *input))
{
	if(BGlobal.plugins.main_loop_count % 16 == 0)
		BGlobal.plugins.main_loop = (void (**)(BInputState *))realloc(BGlobal.plugins.main_loop, (sizeof *BGlobal.plugins.main_loop) * (BGlobal.plugins.main_loop_count + 16));
	BGlobal.plugins.main_loop[BGlobal.plugins.main_loop_count++] = main_loop;
}

void betray_plugin_callback_main(BInputState *input)
{
	uint i;
	for(i = 0; i < BGlobal.plugins.main_loop_count; i++)
		BGlobal.plugins.main_loop[i](input);
}

#ifdef _WIN32
void betray_plugin_callback_set_event_pump(uint event, boolean exclusive, void (*event_loop)(BInputState *input, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam))
{
	if(BGlobal.plugins.event_pump_count % 16 == 0)
		BGlobal.plugins.event_pump = realloc(BGlobal.plugins.event_pump, (sizeof *BGlobal.plugins.event_pump) * (BGlobal.plugins.event_pump_count + 16));	
	BGlobal.plugins.event_pump[BGlobal.plugins.event_pump_count].event_pump = event_loop;
	BGlobal.plugins.event_pump[BGlobal.plugins.event_pump_count].exclusive = exclusive;
	BGlobal.plugins.event_pump[BGlobal.plugins.event_pump_count++].event = event;
}
boolean betray_plugin_callback_event_pump(BInputState *input, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uint i;
	for(i = 0; i < BGlobal.plugins.event_pump_count; i++)
	{
		if(BGlobal.plugins.event_pump[i].event == wParam || BGlobal.plugins.event_pump[i].event == -1)
		{
			BGlobal.plugins.event_pump[i].event_pump(input, hWnd, uMsg, wParam, lParam);
			return BGlobal.plugins.event_pump[i].exclusive;
		}
	}
	return FALSE;
}
#endif

void betray_plugin_callback_set_view_vantage(void (*vantage)(float *pos), boolean modify)
{
	if(BGlobal.plugins.view_vantage_count % 16 == 0)
		BGlobal.plugins.view_vantage = (BVantage *)realloc(BGlobal.plugins.view_vantage, (sizeof *BGlobal.plugins.view_vantage) * (BGlobal.plugins.view_vantage_count + 16));	
	BGlobal.plugins.view_vantage[BGlobal.plugins.view_vantage_count].func = vantage;
	BGlobal.plugins.view_vantage[BGlobal.plugins.view_vantage_count++].modify = modify;
}

void betray_plugin_callback_set_view_direction(void (*matrix)(float *matrix))
{
	if(BGlobal.plugins.view_direction_count % 16 == 0)
		BGlobal.plugins.view_direction = (void (**)(float *))realloc(BGlobal.plugins.view_direction, (sizeof *BGlobal.plugins.view_direction) * (BGlobal.plugins.view_direction_count + 16));	
	BGlobal.plugins.view_direction[BGlobal.plugins.view_direction_count++] = matrix;
}

void betray_plugin_callback_set_image_warp(boolean (*image_warp)(BInputState *input), char *name)
{
	if(BGlobal.plugins.image_warp_count % 16 == 0)
		BGlobal.plugins.image_warp = (BImageWarp *)realloc(BGlobal.plugins.image_warp, (sizeof *BGlobal.plugins.image_warp) * (BGlobal.plugins.image_warp_count + 16));
	BGlobal.plugins.image_warp[BGlobal.plugins.image_warp_count].context = b_create_context();
	BGlobal.plugins.image_warp[BGlobal.plugins.image_warp_count].name = name;
	betray_activate_context(BGlobal.plugins.image_warp[BGlobal.plugins.image_warp_count].context);
	BGlobal.plugins.image_warp[BGlobal.plugins.image_warp_count++].func = image_warp;
}

void betray_application_draw(uint fbo, uint x_size, uint y_size, float *vantage, boolean vantage_modify, float *matrix)
{
	if(BGlobal.action_func != NULL)
	{
		if(x_size == 0)
			BGlobal.draw_state.x_size = BGlobal.screen_mode.x_size;
		else
			BGlobal.draw_state.x_size = x_size;
		if(y_size == 0)
			BGlobal.draw_state.y_size = BGlobal.screen_mode.y_size;
		else
			BGlobal.draw_state.y_size = y_size;
		BGlobal_draw_state_fbo = fbo;
		BGlobal.draw_state.vantage = vantage;
		BGlobal.draw_state.vantage_modify = vantage_modify;
		BGlobal.draw_state.matrix = matrix;
		betray_activate_context(NULL);
		betray_glBindFramebufferEXT(0x8D40, fbo);
		BGlobal.action_func(&BGlobal.input, BGlobal.action_func_data);
//		glFinish();
		betray_activate_context(BGlobal.draw_state.context);
		betray_draw_state_reset();
		BGlobal.input.draw_id++;
	}
}

boolean	betray_button_get(uint user_id, uint button)
{
    BInputState *input;
    uint i;
    input = betray_get_input_state();
    for(i = 0; i < input->button_event_count; i++)
        if(input->button_event[i].state && input->button_event[i].button == button && (user_id == input->button_event[i].user_id || user_id == -1))
            return TRUE;
    return FALSE;
}

void betray_button_get_up_down(uint user_id, boolean *press, boolean *last_press, uint button)
{
    BInputState *input;
    uint i;
    input = betray_get_input_state();
    if(input->mode == BAM_EVENT)
    {
		if(last_press != NULL)
	        *last_press = *press;
        for(i = 0; i < input->button_event_count; i++)
            if(input->button_event[i].button == button && (user_id == input->button_event[i].user_id || user_id == -1))
                *press = input->button_event[i].state;
    }
}

#define BETRAY_AUDIO_UNIT_MAGIC_NUMBER 11382600

uint betray_plugin_audio_unit_create()
{
	if(BGlobal.plugins.audio_count % 16 == 0)
	{
		BGlobal.plugins.audio = (BAudioCallback *)realloc(BGlobal.plugins.audio, (sizeof *BGlobal.plugins.audio) * (BGlobal.plugins.audio_count + 16));
	}
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].sound_create = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].sound_destroy = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].sound_play = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].sound_set = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].sound_is_playing = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].sound_stop = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].stream_create = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].stream_destroy = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].stream_feed = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].stream_buffer_left = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].stream_set = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].listener = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].read = NULL;
	BGlobal.plugins.audio[BGlobal.plugins.audio_count].channels = 0;
	BGlobal.plugins.audio_count++;
	return BGlobal.plugins.audio_count - 1 + BETRAY_AUDIO_UNIT_MAGIC_NUMBER;
}

void betray_plugin_callback_set_audio_sound_create(uint audio_unit_id, uint (*func)(uint type, uint stride, uint length, uint frequency, void *data, char *name))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_sound_create set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].sound_create = func;
}

void betray_plugin_callback_set_audio_sound_destroy(uint audio_unit_id, void (*func)(uint sound))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_sound_destroy set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].sound_destroy = func;
}

void betray_plugin_callback_set_audio_sound_play(uint audio_unit_id, uint (*func)(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_sound_play set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].sound_play = func;
}


void betray_plugin_callback_set_audio_sound_set(uint audio_unit_id, void (*func)(uint play, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_sound_set set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].sound_set = func;
}

void betray_plugin_callback_set_audio_sound_is_playing(uint audio_unit_id, boolean (*func)(uint play))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_is_playing_set set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].sound_is_playing = func;
}

void betray_plugin_callback_set_audio_sound_stop(uint audio_unit_id, void (*func)(uint play))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_sound_stop set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].sound_stop = func;
}

void betray_plugin_callback_set_audio_listener(uint audio_unit_id, void (*func)(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_listener set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].listener = func;
}


uint betray_audio_sound_create(uint type, uint stride, uint length, uint frequency, void *data, char *name)
{
	uint i, id;
	for(id = 0; id < BGlobal.sounds.sound_count; id++)
		if(BGlobal.sounds.sound_ids[id * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] == FALSE)
			break;
	if(id == BGlobal.sounds.sound_count)
		BGlobal.sounds.sound_count++;
	if(id >= BGlobal.sounds.sound_allocated)
	{
		BGlobal.sounds.sound_allocated += 16;
		BGlobal.sounds.sound_ids = (uint *)realloc(BGlobal.sounds.sound_ids, (sizeof *BGlobal.sounds.sound_ids) * (BGlobal.plugins.audio_count + 1) * BGlobal.sounds.sound_allocated);
	}
	BGlobal.sounds.sound_ids[id * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] = TRUE;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].sound_create != NULL)
			BGlobal.sounds.sound_ids[id * (BGlobal.plugins.audio_count + 1) + i] = BGlobal.plugins.audio[i].sound_create(type, stride, length, frequency, data, name);
	return id;
}

void betray_audio_sound_destroy(uint sound)
{
	uint i;
	BGlobal.sounds.sound_ids[sound * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] = FALSE;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].sound_destroy != NULL)
			BGlobal.plugins.audio[i].sound_destroy(BGlobal.sounds.sound_ids[sound * (BGlobal.plugins.audio_count + 1) + i]);
}

uint betray_audio_sound_play(uint sound, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient, boolean auto_delete)
{
	uint i, id;
	if(auto_delete)
	{
		for(i = 0; i < BGlobal.plugins.audio_count; i++)
			BGlobal.plugins.audio[i].sound_play(BGlobal.sounds.sound_ids[sound * (BGlobal.plugins.audio_count + 1) + i], pos, vector, speed, volume, loop, ambient, auto_delete);
		return -1;
	}else
	{
		for(id = 0; id < BGlobal.sounds.source_count; id++)
			if(BGlobal.sounds.source_ids[id * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] == FALSE)
				break;
		if(id == BGlobal.sounds.source_allocated)
		{
			BGlobal.sounds.source_allocated += 32;
			BGlobal.sounds.source_ids = (uint *)realloc(BGlobal.sounds.source_ids, BGlobal.sounds.source_allocated * (sizeof *BGlobal.sounds.source_ids) * (BGlobal.plugins.audio_count + 1));
		}
		BGlobal.sounds.source_ids[id * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] = TRUE;
		for(i = 0; i < BGlobal.plugins.audio_count; i++)
			if(BGlobal.plugins.audio[i].sound_play != NULL)
				BGlobal.sounds.source_ids[id * (BGlobal.plugins.audio_count + 1) + i] = BGlobal.plugins.audio[i].sound_play(BGlobal.sounds.sound_ids[sound * (BGlobal.plugins.audio_count + 1) + i], pos, vector, speed, volume, loop, ambient, auto_delete);
		return id;
	}
}

void betray_audio_sound_set(uint play, float *pos, float *vector, float speed, float volume, boolean loop, boolean ambient)
{
	uint i;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].sound_set != NULL)
			BGlobal.plugins.audio[i].sound_set(BGlobal.sounds.source_ids[play * (BGlobal.plugins.audio_count + 1) + i], pos, vector, speed, volume, loop, ambient);
}

boolean	betray_audio_sound_is_playing(uint play)
{
	uint i;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].sound_is_playing != NULL)
			if(BGlobal.plugins.audio[i].sound_is_playing(BGlobal.sounds.source_ids[play * (BGlobal.plugins.audio_count + 1) + i]))
				return TRUE;
	return FALSE;
}

void betray_audio_sound_stop(uint play)
{
	uint i;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].sound_stop != NULL)
			BGlobal.plugins.audio[i].sound_stop(BGlobal.sounds.source_ids[play * (BGlobal.plugins.audio_count + 1) + i]);
	BGlobal.sounds.source_ids[play * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] = FALSE;
}


void betray_plugin_callback_set_audio_stream_create(uint audio_unit_id, uint (*func)(uint frequency, float *pos, float *vector,  float volume, boolean ambient))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_stream_create set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].stream_create = func;
}

void betray_plugin_callback_set_audio_stream_destroy(uint audio_unit_id, void (*func)(uint stream))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_stream_destroy set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].stream_destroy = func;
}

void betray_plugin_callback_set_audio_stream_feed(uint audio_unit_id, void (*func)(uint stream, uint type, uint stride, uint length, void *data))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_stream_feed set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].stream_feed = func;
}

void betray_plugin_callback_set_audio_stream_buffer_left(uint audio_unit_id, uint (*func)(uint stream))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_stream_buffer_left set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].stream_buffer_left = func;
}

void betray_plugin_callback_set_audio_stream_set(uint audio_unit_id, void (*func)(uint stream, float *pos, float *vector,  float volume, boolean ambient))
{
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_stream_set set with an illegal audio unit id\n");
	else
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].stream_set = func;
}

uint betray_audio_stream_create(uint frequency, float *pos, float *vector,  float volume, boolean ambient)
{
	uint i, id;
	for(id = 0; id < BGlobal.sounds.stream_count; id++)
		if(BGlobal.sounds.stream_ids[id * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] == FALSE)
			break;
	if(id == BGlobal.sounds.stream_count)
		BGlobal.sounds.stream_count++;
	if(id == BGlobal.sounds.stream_allocated)
	{
		BGlobal.sounds.stream_allocated += 16;
		BGlobal.sounds.stream_ids = (uint *)realloc(BGlobal.sounds.stream_ids, (sizeof *BGlobal.sounds.stream_ids) * (BGlobal.plugins.audio_count + 1) * BGlobal.sounds.stream_allocated);
	}
	BGlobal.sounds.stream_ids[id * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] = TRUE;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
	{
		if(BGlobal.plugins.audio[i].stream_create != NULL)
			BGlobal.sounds.stream_ids[id * (BGlobal.plugins.audio_count + 1) + i] = BGlobal.plugins.audio[i].stream_create(frequency, pos, vector,  volume, ambient);
		else
			BGlobal.sounds.stream_ids[id * (BGlobal.plugins.audio_count + 1) + i] = -1;
	}
	return id;
}

void betray_audio_stream_destroy(uint stream)
{
	uint i;
	BGlobal.sounds.stream_ids[stream * (BGlobal.plugins.audio_count + 1) + BGlobal.plugins.audio_count] = FALSE;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].stream_destroy != NULL)
			BGlobal.plugins.audio[i].stream_destroy(BGlobal.sounds.stream_ids[stream * (BGlobal.plugins.audio_count + 1) + i]);
}

void betray_audio_stream_feed(uint stream, uint type, uint stride, uint length, void *data)
{
	uint i, id;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].stream_feed != NULL)
		{
			id = BGlobal.sounds.stream_ids[stream * (BGlobal.plugins.audio_count + 1) + i];
  			BGlobal.plugins.audio[i].stream_feed(BGlobal.sounds.stream_ids[stream * (BGlobal.plugins.audio_count + 1) + i], type, stride, length, data);
		}
		
}
uint betray_audio_stream_buffer_left(uint stream)
{
	uint i, size, left = -1;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
	{
		if(BGlobal.plugins.audio[i].stream_buffer_left!= NULL)
		{
			size = BGlobal.plugins.audio[i].stream_buffer_left(BGlobal.sounds.stream_ids[stream * (BGlobal.plugins.audio_count + 1) + i]);
			if(size < left)
				left = size;
		}
	}
	return left;
}

void betray_audio_stream_set(uint stream, float *pos, float *vector,  float volume, boolean ambient)
{
	uint i;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].stream_set != NULL)
			BGlobal.plugins.audio[i].stream_set(BGlobal.sounds.stream_ids[stream * (BGlobal.plugins.audio_count + 1) + i], pos, vector, volume, ambient);
}

void betray_audio_listener(float *pos, float *vector, float *forward, float *side, float scale, float speed_of_sound)
{
	uint i;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		BGlobal.plugins.audio[i].listener(pos, vector, forward, side, scale, speed_of_sound);
}

void betray_audio_master_volume_set(float volume)
{
	BGlobal.sounds.master_volume = volume;
}

float betray_audio_master_volume_get(void)
{
	return BGlobal.sounds.master_volume;
}

void betray_plugin_callback_set_audio_read(uint audio_unit_id, uint (*func)(void *data, uint type, uint buffer_size), uint channels, float *vectors)
{
	uint i;
	if(audio_unit_id < BETRAY_AUDIO_UNIT_MAGIC_NUMBER || audio_unit_id >= BETRAY_AUDIO_UNIT_MAGIC_NUMBER + BGlobal.plugins.audio_count)
		printf("Betray Error: betray_plugin_callback_set_audio_read set with an illegal audio unit id\n");
	else
	{
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].read = func;
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].channels = channels;
		BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].vectors = (float *)malloc((sizeof *BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].vectors) * 3 * channels);
		for(i = 0; i < channels * 3; i++)
			BGlobal.plugins.audio[audio_unit_id - BETRAY_AUDIO_UNIT_MAGIC_NUMBER].vectors[i] = vectors[i];
	}
}

uint betray_audio_read_units()
{
	uint i, count = 0;
	for(i = 0; i < BGlobal.plugins.audio_count; i++)
		if(BGlobal.plugins.audio[i].channels != 0)
			count++;
	return count;
}
uint betray_audio_read_channels_availables(uint unit_id)
{
	uint i, count = 0;
	for(i = 0; i < BGlobal.plugins.audio_count && count != unit_id; i++)
		if(BGlobal.plugins.audio[i].channels != 0)
			count++;
	if(count == unit_id)
		return BGlobal.plugins.audio[i].channels;
	return 0;
}

void betray_audio_read_channel_direction(uint unit_id, float *vec)
{
	uint i, j, count = 0;
	for(i = 0; i < BGlobal.plugins.audio_count && count != unit_id; i++)
		if(BGlobal.plugins.audio[i].channels != 0)
			count++;
	if(count == unit_id)
		for(j = 0; j < BGlobal.plugins.audio[i].channels; j++)
			vec[j] = BGlobal.plugins.audio[i].vectors[j];	
}

#define BETRAY_TYPE_INT8 0
#define BETRAY_TYPE_INT16 1
#define BETRAY_TYPE_INT32 2
#define BETRAY_TYPE_FLOAT32 3

uint betray_audio_read(uint unit_id, void *data, uint type, uint buffer_size)
{
	int8 *data8 = NULL;
	int16 *data16 = NULL;
	int32 *data32 = NULL;
	real32 *dataf32 = NULL;
	uint i, count = 0;
	for(i = 0; i < BGlobal.plugins.audio_count && count != unit_id; i++)
		if(BGlobal.plugins.audio[i].channels != 0)
			count++;
	if(count == unit_id)
		return BGlobal.plugins.audio[i].read(data, type, buffer_size);
	return 0;
}

extern char *betray_button_unused;

uint betray_plugin_user_allocate(void)
{
	BInputState *input;
	input = betray_get_input_state();
	input->user_count++;
	return input->user_count - 1;
}

uint betray_plugin_button_allocate(uint code, char *name)
{
	if(code < BETRAY_ENUM_BUTTON_KEY_END)
	{
		return betray_buttons[code].system_code;
	}
	for(code = BETRAY_ENUM_BUTTON_KEY_END; code < BETRAY_ENUM_BUTTON_COUNT; code++)
	{
		
		if(betray_buttons[code].name == betray_button_unused)
		{
			betray_buttons[code].name = name;
			return betray_buttons[code].system_code;
		}
	}
	return -1;
}

void betray_plugin_button_release_all(BInputState *input)
{
	uint i, j;
	for(i = 0; i < input->button_event_count; i++)
	{
		if(input->button_event[i].state)
		{	
			input->button_event[i--] = input->button_event[--input->button_event_count];
		}else
		{
			for(j = 0; j < BGlobal.held_button_count; j++)
				if(BGlobal.held_buttons[j].user_id == input->button_event[i].user_id &&
					BGlobal.held_buttons[j].button == input->button_event[i].button)
					BGlobal.held_buttons[j--] = BGlobal.held_buttons[--BGlobal.held_button_count];
		}
	}
	for(i = 0; i < BGlobal.held_button_count && input->button_event_count < B_MAX_EVENT_COUNT; i++)
	{
		input->button_event[input->button_event_count++] = BGlobal.held_buttons[i];
	}
	BGlobal.held_button_count = 0;
}

void betray_plugin_button_set(uint user_id, uint id, boolean state, uint character)
{
	BInputState *input;
	uint i;
	input = betray_get_input_state();
	for(i = 0; i < BGlobal.held_button_count; i++)
	{
		if(BGlobal.held_buttons[i].user_id == user_id &&
			BGlobal.held_buttons[i].button == id)
		{
			BGlobal.held_buttons[i] = BGlobal.held_buttons[--BGlobal.held_button_count];
			break;
		}
	}			
	if(state && BGlobal.held_button_count >= B_MAX_EVENT_COUNT)
		return;
	if(input->button_event_count < B_MAX_EVENT_COUNT)
	{
		input->button_event[input->button_event_count].user_id = user_id;
		input->button_event[input->button_event_count].state = state;
		input->button_event[input->button_event_count].button = id;
		input->button_event[input->button_event_count++].character = character;
		if(state)
		{
			BGlobal.held_buttons[BGlobal.held_button_count] = input->button_event[input->button_event_count - 1];
			BGlobal.held_buttons[BGlobal.held_button_count].state = FALSE;
			BGlobal.held_button_count++;
		}
	}
}

void betray_plugin_button_free(uint id)
{
	uint code;
	for(code = BETRAY_ENUM_BUTTON_KEY_END; code < BETRAY_ENUM_BUTTON_COUNT; code++)
	{
		if(betray_buttons[code].system_code == id)
		{
			betray_buttons[code].name = betray_button_unused;
			return;
		}
	}
}

uint betray_plugin_pointer_allocate(uint user_id, uint device_id, uint button_count, float x, float y, float z, float *origin, char *name, boolean draw)
{
	uint i, code;
	for(code = 0; code < BGlobal.input.pointer_count && (BGlobal.input.pointers[code].name[0] != 0 || !BGlobal.input.pointers[code].last_button[0] || !BGlobal.input.pointers[code].button[0]); code++);
	if(code == 256)
		return -1;
	BGlobal.input.pointers[code].user_id = user_id;
	BGlobal.input.pointers[code].device_id = device_id;
	BGlobal.input.pointers[code].pointer_x = x;
	BGlobal.input.pointers[code].pointer_y = y;
	BGlobal.input.pointers[code].pointer_z = z;
	BGlobal.input.pointers[code].draw = draw;
	if(origin != NULL)
	{
		BGlobal.input.pointers[code].origin[0] = origin[0];
		BGlobal.input.pointers[code].origin[1] = origin[1];
		BGlobal.input.pointers[code].origin[2] = origin[2];
	}else
	{
		BGlobal.input.pointers[code].origin[0] = 0;
		BGlobal.input.pointers[code].origin[1] = 0;
		BGlobal.input.pointers[code].origin[2] = 0;
	}
	BGlobal.input.pointers[code].delta_pointer_x = -x;
	BGlobal.input.pointers[code].delta_pointer_y = -y;
	BGlobal.input.pointers[code].delta_pointer_z = -z;
	for(i = 0; i < B_POINTER_BUTTONS_COUNT; i++)
	{
		BGlobal.input.pointers[code].click_pointer_x[i] = x;
		BGlobal.input.pointers[code].click_pointer_y[i] = y;
		BGlobal.input.pointers[code].click_pointer_z[i] = y;
		BGlobal.input.pointers[code].button[i] = FALSE;
		BGlobal.input.pointers[code].last_button[i] = FALSE;
	}
	if(B_POINTER_BUTTONS_COUNT < button_count)
		button_count = B_POINTER_BUTTONS_COUNT;
	BGlobal.input.pointers[code].button_count = button_count;
	for(i = 0; i < 31 && name[i] != 0; i++)
		BGlobal.input.pointers[code].name[i] = name[i];
	BGlobal.input.pointers[code].name[i] = 0;
	if(BGlobal.input.pointer_count < code + 1)
		BGlobal.input.pointer_count = code + 1;
	return code;
}

void betray_plugin_pointer_set(uint id, float x, float y, float z, float *origin, boolean *buttons)
{
	uint i;
	if(id >= 256)
		return;
	BGlobal.input.pointers[id].pointer_x = x;
	BGlobal.input.pointers[id].pointer_y = y;
	BGlobal.input.pointers[id].pointer_z = z;
	if(origin != NULL)
	{
		BGlobal.input.pointers[id].origin[0] = origin[0];
		BGlobal.input.pointers[id].origin[1] = origin[1];
		BGlobal.input.pointers[id].origin[2] = origin[2];
	}
	for(i = 0; i < BGlobal.input.pointers[id].button_count; i++)
	{
	/*	if(buttons[i] && !BGlobal.input.pointers[id].button[i])
		{
			BGlobal.input.pointers[id].click_pointer_x[i] = x;
			BGlobal.input.pointers[id].click_pointer_y[i] = y;
		}*/
		BGlobal.input.pointers[id].button[i] = buttons[i];
	}
}

void betray_plugin_pointer_free(uint id)
{
	if(id >= 256)
		return;
	BGlobal.input.pointers[id].name[0] = 0;
	while(BGlobal.input.pointers[BGlobal.input.pointer_count - 1].name[0] == 0 && BGlobal.input.pointer_count != 0)
		BGlobal.input.pointer_count--;
}


void betray_plugin_pointer_clean()
{
	uint i;
	for(i = BGlobal.input.pointer_count; i != 0 && (BGlobal.input.pointers[i - 1].name[0] == 0 && !BGlobal.input.pointers[i - 1].last_button[0] && !BGlobal.input.pointers[i - 1].button[0]); i--);
	BGlobal.input.pointer_count = i;
}

uint betray_plugin_input_device_allocate(uint user_id, char *name)
{
	uint i;
	if(BGlobal.device_allocated == BGlobal.device_count)
	{
		BGlobal.device_allocated += 16;
		BGlobal.devices = realloc(BGlobal.devices, (sizeof *BGlobal.devices) * BGlobal.device_allocated);
	}
	BGlobal.devices[BGlobal.device_count].id = 0;
	for(i = 0; i <BGlobal.device_count; i++)
		if(BGlobal.devices[i].id >= BGlobal.devices[BGlobal.device_count].id)
			BGlobal.devices[BGlobal.device_count].id = BGlobal.devices[i].id + 1;
	for(i = 0; i < 64 - 1 && name[i] != 0; i++)
		BGlobal.devices[BGlobal.device_count].name[i] = name[i];
	BGlobal.devices[BGlobal.device_count].name[i] = 0;
	BGlobal.devices[BGlobal.device_count].user_id = user_id;
	return BGlobal.devices[BGlobal.device_count++].id;
}


void betray_plugin_input_device_free(uint id)
{
	uint i;
	for(i = 0; i <BGlobal.device_count; i++)
	{
		if(BGlobal.devices[i].id == id)
		{
			BGlobal.devices[i] = BGlobal.devices[--BGlobal.device_count];
			return;
		}
	}
}


uint betray_plugin_axis_allocate(uint user_id, uint device_id, char *name, BAxisType type, uint axis_count)
{
	uint i, id;
	for(id = 0; id < BGlobal.input.axis_count; id++)
		if(BGlobal.input.axis[id].name[0] == 0)
			break;
	if(id == BGlobal.input.axis_count && BGlobal.input.axis_count % 16 == 0)
		BGlobal.input.axis = (BInputAxisState *)realloc(BGlobal.input.axis, (sizeof *BGlobal.input.axis) * (BGlobal.input.axis_count + 16));
	BGlobal.input.axis[id].user_id = user_id;
	BGlobal.input.axis[id].axis[0] = 0;
	BGlobal.input.axis[id].axis[1] = 0;
	BGlobal.input.axis[id].axis[2] = 0;
	BGlobal.input.axis[id].axis_count = axis_count;
	BGlobal.input.axis[id].axis_type = type;
	BGlobal.input.axis[id].device_id = device_id;
	for(i = 0; i < 32 && name[i] != 0; i++)
		BGlobal.input.axis[id].name[i] = name[i];
	BGlobal.input.axis[id].name[i] = 0;
	if(BGlobal.input.axis_count < id + 1)
		BGlobal.input.axis_count = id + 1;
	return id;
}

void betray_plugin_axis_set(uint id, float axis_x, float axis_y, float axis_z)
{
	if(id < BGlobal.input.axis_count)
	{
		BGlobal.input.axis[id].axis[0] = axis_x;
		BGlobal.input.axis[id].axis[1] = axis_y;
		BGlobal.input.axis[id].axis[2] = axis_z;
	}
}

void betray_plugin_axis_free(uint id)
{
	uint i;
	if(id < BGlobal.input.axis_count)
	{
		BGlobal.input.axis[id].name[0] = 0;
		for(i = BGlobal.input.axis_count; i != 0 && BGlobal.input.axis[i - 1].name[0] == 0; i--);
		BGlobal.input.axis_count = i;
	}
}



uint betray_settings_create(uint type, char *name, uint select_count, char **select_options)
{
	uint i, j;
	if(BGlobal.setting_count % 16 == 0)
		BGlobal.settings = (BSetting *)realloc(BGlobal.settings, (sizeof *BGlobal.settings) * (BGlobal.setting_count + 16));
	BGlobal.settings[BGlobal.setting_count].type = (BSettingType)type;
	if(type == BETRAY_ST_SELECT)
	{
		BGlobal.settings[BGlobal.setting_count].data.select.select_count = select_count;
		BGlobal.settings[BGlobal.setting_count].data.select.options = (char **)malloc((sizeof *BGlobal.settings[BGlobal.setting_count].data.select.options) * select_count);
		for(i = 0; i < select_count; i++)
		{
			for(j = 0; select_options[i][j] != 0; j++);
			BGlobal.settings[BGlobal.setting_count].data.select.options[i] = (char *)malloc((sizeof *BGlobal.settings[BGlobal.setting_count].data.select.options[i]) * ++j);
			for(j = 0; select_options[i][j] != 0; j++)
				BGlobal.settings[BGlobal.setting_count].data.select.options[i][j] = select_options[i][j];
			BGlobal.settings[BGlobal.setting_count].data.select.options[i][j] = 0;
		}
	}

	if(type == BETRAY_ST_STRING)
	{
		BGlobal.settings[BGlobal.setting_count].data.string = malloc(1);
		BGlobal.settings[BGlobal.setting_count].data.string[0] = 0;
	}
	if(type == BETRAY_ST_TRIGGER)
		BGlobal.settings[BGlobal.setting_count].data.trigger = FALSE;
	for(i = 0; i < 63 && name[i] != 0; i++)
		BGlobal.settings[BGlobal.setting_count].name[i] = name[i];
	BGlobal.settings[BGlobal.setting_count++].name[i] = 0;
	return BGlobal.setting_count - 1;
}

uint betray_settings_count()
{
	return BGlobal.setting_count;
}

BSettingType betray_settings_type(uint id)
{
	return BGlobal.settings[id].type;
}

char *betray_settings_name(uint id)
{
	return BGlobal.settings[id].name;
}

void betray_settings_trigger(uint id)
{
	BGlobal.settings[id].data.trigger = TRUE;
}

boolean betray_settings_trigger_captiure(uint id)
{
	boolean output;
	output = BGlobal.settings[id].data.trigger;
	BGlobal.settings[id].data.trigger = FALSE;
	return output;
}

boolean	betray_settings_toggle_get(uint id)
{
	return BGlobal.settings[id].data.toggle;
}

void betray_settings_toggle_set(uint id, boolean toggle)
{
	BGlobal.settings[id].data.toggle = toggle;
}

uint betray_settings_select_get(uint id)
{
	return BGlobal.settings[id].data.select.selected;
}

void betray_settings_select_set(uint id, uint select)
{
	BGlobal.settings[id].data.select.selected = select;
}

uint betray_settings_select_count_get(uint id)
{
	return BGlobal.settings[id].data.select.select_count;
}

char *betray_settings_select_name_get(uint id, uint option)
{
	return BGlobal.settings[id].data.select.options[option];
}


char *betray_settings_string_get(uint id)
{
	return BGlobal.settings[id].data.string;
}

void betray_settings_string_set(uint id, char *string)
{
	free(BGlobal.settings[id].data.string);
	BGlobal.settings[id].data.string = f_text_copy_allocate(string);
}

float betray_settings_number_float_get(uint id)
{
	return BGlobal.settings[id].data.number_float;
}

void betray_settings_number_float_set(uint id, float number)
{
	BGlobal.settings[id].data.number_float = number;
}

int betray_settings_number_int_get(uint id)
{
	return BGlobal.settings[id].data.number_int;
}

void betray_settings_number_int_set(uint id, int number)
{
	BGlobal.settings[id].data.number_int = number;
}

float betray_settings_slider_get(uint id)
{
	return BGlobal.settings[id].data.slider;
}

void betray_settings_slider_set(uint id, float slider)
{
	BGlobal.settings[id].data.slider = slider;
}

void betray_settings_2d_get(uint id, float *x, float *y)
{
	*x = BGlobal.settings[id].data.dimentions[0];
	*y = BGlobal.settings[id].data.dimentions[1];
}

void betray_settings_2d_set(uint id, float x, float y)
{
	BGlobal.settings[id].data.dimentions[0] = x;
	BGlobal.settings[id].data.dimentions[1] = y;
}

void betray_settings_3d_get(uint id, float *x, float *y, float *z)
{
	*x = BGlobal.settings[id].data.dimentions[0];
	*y = BGlobal.settings[id].data.dimentions[1];
	*x = BGlobal.settings[id].data.dimentions[2];
}

void betray_settings_3d_set(uint id, float x, float y, float z)
{
	BGlobal.settings[id].data.dimentions[0] = x;
	BGlobal.settings[id].data.dimentions[1] = y;
	BGlobal.settings[id].data.dimentions[2] = z;
}

void betray_settings_color_get(uint id, float *red, float *green, float *blue)
{
	*red = BGlobal.settings[id].data.color[0];
	*green = BGlobal.settings[id].data.color[1];
	*blue = BGlobal.settings[id].data.color[2];
}

void betray_settings_color_set(uint id, float red, float green, float blue)
{
	BGlobal.settings[id].data.color[0] = red;
	BGlobal.settings[id].data.color[1] = green;
	BGlobal.settings[id].data.color[2] = blue;
}

void betray_settings_4x4_matrix_get(uint id, float *matrix)
{
	uint i;
	for(i = 0; i < 16; i++)
		matrix[i] = BGlobal.settings[id].data.matrix[i];
}

void betray_settings_4x4_matrix_set(uint id, float *matrix)
{
	uint i;
	for(i = 0; i < 16; i++)
		BGlobal.settings[id].data.matrix[i] = matrix[i];
}

#ifndef errno
extern int errno; 
#endif

void betray_init(BContextType context_type, int argc, char **argv, uint window_size_x, uint window_size_y, uint samples, boolean window_fullscreen, char *name)
{
	char path[1024];
	uint i, j, display_mode_x, display_mode_y;
	IInterface *exe_interface;
//#ifndef _CONSOLE
	freopen("betray_stdout.txt", "w", stdout);
	setbuf(stdout, NULL);
	freopen("betray_stderr.txt", "w", stderr);
	setbuf(stderr, NULL);
//#endif
//	betray_context_type = context_type;

	BGlobal.context_func = NULL;
	for(i = 0; i < 511 && name[i] != 0; i++)
		BGlobal.caption[i] = name[i];
	BGlobal.caption[i] = 0;
	BGlobal.devices = NULL;
	BGlobal.device_allocated = 0;
	BGlobal.device_count = 0;

	betray_key_codes_init();

	betray_desktop_size_get(&display_mode_x, &display_mode_y);
	if(window_fullscreen)
	{
		if(window_size_x == 0 || window_size_x > display_mode_x)
			window_size_x = display_mode_x;
		if(window_size_y == 0 || window_size_y > display_mode_y)
			window_size_y = display_mode_y;
	}else
	{
		if(window_size_x == 0 || window_size_x + 100 > display_mode_x)
			window_size_x = display_mode_x - 100;
		if(window_size_y == 0 || window_size_y + 100 > display_mode_y)
			window_size_y = display_mode_y - 100;
	}
	BGlobal.sterioscopic = FALSE;
	BGlobal.input.minute_time = 0;
	BGlobal.input.pointer_count = 256;
	BGlobal.input.pointers = (BInputPointerState *)malloc((sizeof *BGlobal.input.pointers) * BGlobal.input.pointer_count);
	for(i = 0; i < BGlobal.input.pointer_count; i++)
	{
		BGlobal.input.pointers[i].pointer_x = 0;
		BGlobal.input.pointers[i].pointer_y = 0;
		BGlobal.input.pointers[i].pointer_z = -1;
		BGlobal.input.pointers[i].delta_pointer_x = 0;
		BGlobal.input.pointers[i].delta_pointer_y = 0;
		BGlobal.input.pointers[i].delta_pointer_z = 0;
		BGlobal.input.pointers[i].origin[0] = 0;
		BGlobal.input.pointers[i].origin[1] = 0;
		BGlobal.input.pointers[i].origin[2] = 0;
		for(j = 0; j < B_POINTER_BUTTONS_COUNT; j++)
		{
			BGlobal.input.pointers[i].click_pointer_x[j] = 0;
			BGlobal.input.pointers[i].click_pointer_y[j] = 0;
			BGlobal.input.pointers[i].button[j] = FALSE;
			BGlobal.input.pointers[i].last_button[j] = FALSE;
		}
		BGlobal.input.pointers[i].button_count = 1;
		BGlobal.input.pointers[i].name[0] = 0;
	}
	BGlobal.input.pointer_count = 0;

	BGlobal.input.axis_count = 0;
	BGlobal.input.axis = NULL;
	BGlobal.input.user_count = 0;
	BGlobal.settings = NULL;
	BGlobal.setting_count = 0;
	BGlobal.input.frame_number = 0;


	
	betray_device_init();
	#ifdef BETRAY_CONTEXT_OPENGL


		if(context_type == B_CT_OPENGL || context_type == B_CT_OPENGL_OR_ES)
		{
			if(!b_init_display_opengl(window_size_x, window_size_y, window_fullscreen,  samples, name, &BGlobal.sterioscopic))
			{
			#ifdef BETRAY_CONTEXT_OPENGLES
				if(betray_context_type != B_CT_OPENGL_OR_ES)
			#endif
				{
					printf("BETRAY Error: Failed to create OpenGL Context");
					exit(0);
				}
			}else
				betray_context_type = B_CT_OPENGL;
		}
	#endif
	#ifdef BETRAY_CONTEXT_OPENGLES
		if(betray_context_type == -1 && (context_type == B_CT_OPENGLES2 || context_type == B_CT_OPENGL_OR_ES))
		{
			if(!b_win32_init_display_opengles2(window_size_x, window_size_y, window_fullscreen,  samples, name, &BGlobal.sterioscopic))
			{
				if(betray_context_type == B_CT_OPENGL_OR_ES)
					printf("BETRAY Error: Failed to create either OpenGL or ES 2.0 Context");
				else
					printf("BETRAY Error: Failed to create OpenGL ES 2.0 Context");
				exit(0);
			}else
				betray_context_type = B_CT_OPENGLES2;
		}
		printf("betray_context_type %u %u \n", betray_context_type, B_CT_OPENGLES2);

	#endif

#ifdef __ANDROID__ FIX ME
//	b_android_init_display(&window_size_x, &window_size_y, name);
//	window_fullscreen = TRUE;
#endif


	imagine_current_time_get(&BGlobal.time[0], &BGlobal.time[1]);
	BGlobal.screen_mode.x_size = window_size_x;
	BGlobal.screen_mode.y_size = window_size_y;
	BGlobal.screen_mode.fullscreen = window_fullscreen;
//	

//	PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;
//printf("%s", glGetString(GL_EXTENSIONS));
//	if (WGLExtensionSupported("WGL_EXT_swap_control"))
/*	{
		const char *extension, *a, *string = "WGL_EXT_swap_control";
		uint i;
		extension = glGetString(GL_EXTENSIONS);
		if(extension != NULL)
		{
			for(a = extension; a[0] != 0; a++)
			{
				for(i = 0; string[i] != 0 && a[i] != 0 && string[i] == a[i]; i++);
				if(string[i] == 0)
				{
					uint (APIENTRY *wglSwapIntervalEXT)(int count);
					void *(*b_gl_GetProcAddress)(const char* proc);
					b_gl_GetProcAddress = betray_gl_proc_address_get();
					wglSwapIntervalEXT = b_gl_GetProcAddress("wglSwapIntervalEXT");
					if(wglSwapIntervalEXT != NULL)
						wglSwapIntervalEXT(1);
				}
			}
		}
	}*/

	BGlobal.plugins.main_loop = NULL;
	BGlobal.plugins.main_loop_count = 0;
#ifdef _WIN32
	BGlobal.plugins.event_pump = NULL;
	BGlobal.plugins.event_pump_count = 0;
#endif
	BGlobal.plugins.view_vantage = NULL;
	BGlobal.plugins.view_vantage_count = 0;
	BGlobal.plugins.view_direction = NULL;
	BGlobal.plugins.view_direction_count = 0;
	BGlobal.plugins.audio = NULL;
	BGlobal.plugins.audio_count = 0;
	BGlobal.plugins.image_warp = NULL; 
	BGlobal.plugins.image_warp_count = 0;

	BGlobal.sounds.sound_ids = NULL;
	BGlobal.sounds.sound_count = 0;
	BGlobal.sounds.sound_allocated = 0;
	BGlobal.sounds.source_ids = NULL;
	BGlobal.sounds.source_count = 0;
	BGlobal.sounds.source_allocated = 0;
	BGlobal.sounds.stream_ids = NULL;
	BGlobal.sounds.stream_count = 0;
	BGlobal.sounds.stream_allocated = 0;
	BGlobal.sounds.master_volume = 0.5;

	exe_interface = imagine_library_interface_create();
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_main, "betray_plugin_callback_set_main");

	imagine_library_interface_register(exe_interface, (void *)betray_get_input_state, "betray_plugin_get_input_state");
	imagine_library_interface_register(exe_interface, (void *)betray_context_type_get, "betray_plugin_context_type_get");
	imagine_library_interface_register(exe_interface, (void *)betray_screen_mode_get, "betray_plugin_screen_mode_get");
	imagine_library_interface_register(exe_interface, (void *)betray_application_draw, "betray_plugin_application_draw");
	imagine_library_interface_register(exe_interface, (void *)betray_gl_proc_address_get, "betray_plugin_gl_proc_address_get");
	imagine_library_interface_register(exe_interface, (void *)betray_clipboard_set, "betray_plugin_clipboard_set");
	imagine_library_interface_register(exe_interface, (void *)betray_clipboard_get, "betray_plugin_clipboard_get");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_main, "betray_plugin_callback_set_main");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_view_vantage, "betray_plugin_callback_set_view_vantage");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_view_direction, "betray_plugin_callback_set_view_direction");
#ifdef _WIN32
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_event_pump, "betray_plugin_callback_set_event_pump");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_windows_window_handle_get, "betray_plugin_windows_window_handle_get");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_windows_device_context_handle_get, "betray_plugin_windows_device_context_handle_get");
#endif
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_image_warp, "betray_plugin_callback_set_image_warp");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_audio_unit_create, "betray_plugin_audio_unit_create");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_sound_create, "betray_plugin_callback_set_audio_sound_create");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_sound_destroy, "betray_plugin_callback_set_audio_sound_destroy");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_sound_play, "betray_plugin_callback_set_audio_sound_play");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_sound_is_playing, "betray_plugin_callback_set_audio_sound_is_playing");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_sound_set, "betray_plugin_callback_set_audio_sound_set");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_sound_stop, "betray_plugin_callback_set_audio_sound_stop");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_stream_create, "betray_plugin_callback_set_audio_stream_create");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_stream_destroy, "betray_plugin_callback_set_audio_stream_destroy");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_stream_feed, "betray_plugin_callback_set_audio_stream_feed");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_stream_buffer_left, "betray_plugin_callback_set_audio_stream_buffer_left");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_stream_set, "betray_plugin_callback_set_audio_stream_set");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_listener, "betray_plugin_callback_set_audio_listener");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_callback_set_audio_read, "betray_plugin_callback_set_audio_read");
	imagine_library_interface_register(exe_interface, (void *)betray_audio_master_volume_get, "betray_plugin_audio_master_volume_get");

	imagine_library_interface_register(exe_interface, (void *)betray_plugin_user_allocate, "betray_plugin_user_allocate");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_input_device_allocate, "betray_plugin_input_device_allocate");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_input_device_free, "betray_plugin_input_device_free");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_button_allocate, "betray_plugin_button_allocate");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_button_set, "betray_plugin_button_set");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_button_free, "betray_plugin_button_free");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_pointer_allocate, "betray_plugin_pointer_allocate");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_pointer_set, "betray_plugin_pointer_set");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_pointer_free, "betray_plugin_pointer_free");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_axis_allocate, "betray_plugin_axis_allocate");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_axis_set, "betray_plugin_axis_set");
	imagine_library_interface_register(exe_interface, (void *)betray_plugin_axis_free, "betray_plugin_axis_free");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_create, "betray_settings_create");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_count, "betray_settings_count");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_type, "betray_settings_type");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_name, "betray_settings_name");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_trigger_captiure, "betray_settings_trigger");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_toggle_get, "betray_settings_toggle_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_toggle_set, "betray_settings_toggle_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_select_get, "betray_settings_select_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_select_set, "betray_settings_select_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_select_count_get, "betray_settings_select_count_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_select_name_get, "betray_settings_select_name_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_string_get, "betray_settings_string_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_string_set, "betray_settings_string_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_number_float_get, "betray_settings_number_float_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_number_float_set, "betray_settings_number_float_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_number_int_get, "betray_settings_number_int_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_number_int_set, "betray_settings_number_int_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_slider_get, "betray_settings_slider_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_slider_set, "betray_settings_slider_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_2d_get, "betray_settings_2d_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_2d_set, "betray_settings_2d_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_3d_get, "betray_settings_3d_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_3d_set, "betray_settings_3d_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_color_get, "betray_settings_color_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_color_set, "betray_settings_color_set");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_4x4_matrix_get, "betray_settings_4x4_matrix_get");
	imagine_library_interface_register(exe_interface, (void *)betray_settings_4x4_matrix_set, "betray_settings_4x4_matrix_set");
/*{
	uint i, j;
	if(BGlobal.setting_count % 16 == 0)
		BGlobal.settings = realloc(BGlobal.settings, (sizeof *BGlobal.settings) * (BGlobal.setting_count + 16));
	BGlobal.settings[BGlobal.setting_count].type = type;
	if(type == BETRAY_ST_SELECT)
	{
		BGlobal.settings[BGlobal.setting_count].data.select.select_count = select_count;
		BGlobal.settings[BGlobal.setting_count].data.select.options = malloc((sizeof *BGlobal.settings[BGlobal.setting_count].data.select.options) * select_count);
		for(i = 0; i < select_count; i++)
		{
			for(j = 0; select_options[i][j] != 0; j++);
			BGlobal.settings[BGlobal.setting_count].data.select.options[i] = malloc((sizeof *BGlobal.settings[BGlobal.setting_count].data.select.options[i]) * ++j);
			for(j = 0; select_options[i][j] != 0; j++)
				BGlobal.settings[BGlobal.setting_count].data.select.options[i][j] = select_options[i][j];
			BGlobal.settings[BGlobal.setting_count].data.select.options[i][j] = 0;
		}
	}
	for(i = 0; i < 63 && name[i] != 0; i++)
		BGlobal.settings[BGlobal.setting_count].name[i] = name[i];
	BGlobal.settings[BGlobal.setting_count++].name[i] = 0;
	return BGlobal.setting_count - 1;
}*/
	betray_settings_create(BETRAY_ST_TOGGLE, "Post Plugin", 0, NULL);
	
	for(i = 0; imagine_path_search("." /*the lack of comma is intentional*/ IMAGINE_LIBRARY_EXTENTION, TRUE, IMAGINE_DIR_HOME_PATH, FALSE, i, path, 1024); i++)
	{
		printf("Loading: %s ", path);
		if(NULL == imagine_library_load(path, exe_interface, "Betray"))
			printf("%s failed.\n", path);
		else
			printf("%s succeded.\n", path);
	}
	
 	betray_activate_context(NULL);
	if(BGlobal.plugins.image_warp_count != 0)
	{
		char **names;
		BGlobal.settings[0].type = BETRAY_ST_SELECT;
		BGlobal.settings[0].data.select.options = (char **)malloc((sizeof *names) * (BGlobal.plugins.image_warp_count + 1));
		for(i = 0; i < BGlobal.plugins.image_warp_count; i++)
			BGlobal.settings[0].data.select.options[i] = BGlobal.plugins.image_warp[i].name;
		BGlobal.settings[0].data.select.options[i] = "None";
		BGlobal.settings[0].data.select.select_count = BGlobal.plugins.image_warp_count + 1;
		BGlobal.settings[0].data.select.selected = BGlobal.plugins.image_warp_count;
	}else
	{
		for(i = 0; i < BGlobal.setting_count - 1; i++)
			BGlobal.settings[i] = BGlobal.settings[i + 1];
		BGlobal.setting_count--;
	}
	betray_safe_area_setting[0] = betray_settings_create(BETRAY_ST_SLIDER, "Horizontal safe area", 0, NULL);
	BGlobal.settings[betray_safe_area_setting[0]].data.slider = 1.0;
	betray_safe_area_setting[1] = betray_settings_create(BETRAY_ST_SLIDER, "Vertuical safe area", 0, NULL);
	BGlobal.settings[betray_safe_area_setting[1]].data.slider = 1.0;
	betray_draw_state_reset();
/*	{k
		char *text[5] = {"Option A", "Option B", "Option C", "Option D", "Option E"};
		betray_settings_create(BETRAY_ST_TOGGLE, "toggle", 0, NULL);
		betray_settings_create(BETRAY_ST_SELECT, "select", 5, text);
		betray_settings_create(BETRAY_ST_NUMBER_FLOAT, "float", 0, NULL);
		betray_settings_create(BETRAY_ST_NUMBER_INT, "int", 0, NULL);
		betray_settings_create(BETRAY_ST_SLIDER, "slider", 0, NULL);
		betray_settings_create(BETRAY_ST_2D, "2D", 0, NULL);
		betray_settings_create(BETRAY_ST_3D, "3D", 0, NULL);
		betray_settings_create(BETRAY_ST_COLOR, "color", 0, NULL);
		betray_settings_create(BETRAY_ST_4X4_MATRIX, "matrix", 0, NULL);
	}*/
}

BContextType betray_context_type_get()
{
	return betray_context_type;
}


BInputState *betray_get_input_state(void)
{
	return &BGlobal.input;
}

float betray_get_time(void)
{
	return (float)BGlobal.input.delta_time;
}

void betray_action_func_set(void (*action_func)(BInputState *data, void *user_pointer), void *user_pointer)
{
	BGlobal.action_func = action_func;
	BGlobal.action_func_data = user_pointer;
}

void betray_action(BActionMode mode)
{
	BGlobal.input.mode = mode;
	if(mode == BAM_DRAW)
	{
		uint i;
		i = BGlobal.plugins.image_warp_count;
		BGlobal.input.draw_id = 0;
		if(BGlobal.plugins.image_warp_count != 0 && BGlobal.settings[0].data.select.selected < BGlobal.plugins.image_warp_count)
		{
			BGlobal.draw_state.context = BGlobal.plugins.image_warp[BGlobal.settings[0].data.select.selected].context;
			betray_activate_context(BGlobal.plugins.image_warp[BGlobal.settings[0].data.select.selected].context);
			if(BGlobal.plugins.image_warp[BGlobal.settings[0].data.select.selected].func(&BGlobal.input))
				BGlobal.input.draw_id++;
			glFinish();
		}
		betray_draw_state_reset();
		betray_activate_context(NULL);
		if(0 == BGlobal.input.draw_id)
		{
			if(BGlobal.plugins.image_warp_count != 0)
				betray_glBindFramebufferEXT(0x8D40, 0);
			if(BGlobal.action_func != NULL)
				BGlobal.action_func(&BGlobal.input, BGlobal.action_func_data);
	//		glFinish();
		}
	//	b_debug_message();
	}else if(BGlobal.action_func != NULL)
		BGlobal.action_func(&BGlobal.input, BGlobal.action_func_data);
}

boolean betray_get_key(uint key)
{
	uint i;
	for(i = 0; i < BGlobal.input.button_event_count; i++)
		if(BGlobal.input.button_event[i].button == key && BGlobal.input.button_event[i].state)
			return TRUE;
	return FALSE;
}

void betray_get_key_up_down(boolean *press, boolean *last_press, uint key)
{
	uint i;
	*last_press = *press;
	for(i = 0; i < BGlobal.input.button_event_count; i++)
	{
		if(BGlobal.input.button_event[i].button == key)
		{
			*press = BGlobal.input.button_event[i].state;
		}
	}
}
/*
#if defined _WIN32

void betray_get_current_time(uint32 *seconds, uint32 *fractions)
{
	static LARGE_INTEGER frequency;
	static boolean init = FALSE;
	LARGE_INTEGER counter;

	if(!init)
	{
		init = TRUE;
		QueryPerformanceFrequency(&frequency);
	}

	QueryPerformanceCounter(&counter);
	if(seconds != NULL)
		*seconds = (uint32)(counter.QuadPart / frequency.QuadPart);
	if(fractions != NULL)
		*fractions = (uint32)((((ULONGLONG) 0xffffffffU) * (counter.QuadPart % frequency.QuadPart)) / frequency.QuadPart);
}

#else

#include <sys/time.h>

void betray_get_current_time(uint32 *seconds, uint32 *fractions)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	if(seconds != NULL)
	    *seconds = tv.tv_sec;
	if(fractions != NULL)
		*fractions = tv.tv_usec * 1E-6 * (double) (uint32)~0;
}

#endif
*/
uint		betray_framebuffer_save_frame_rate = 25;
double		betray_framebuffer_save_delta = 0.0;

float betray_time_delta_get(void)
{
//	return 1.0 / 30.0;
	if(betray_capture)
	{
	/*	if(betray_framebuffer_save_delta + BGlobal.delta_time > 1.0 / (double)betray_framebuffer_save_frame_rate)
			return 1.0 / (double)betray_framebuffer_save_frame_rate - betray_framebuffer_save_delta;
		else
			return BGlobal.delta_time;*/
			return 1.0 / (double)betray_framebuffer_save_frame_rate;
			
	}else
		return BGlobal.input.delta_time;
}

void betray_time_update(void)
{
	uint32 seconds, fractions;
	imagine_current_time_get(&seconds, &fractions);
	BGlobal.input.delta_time = (double)seconds - (double)BGlobal.time[0] + ((double)fractions - (double)BGlobal.time[1]) / (double)(0xffffffff);
	BGlobal.input.minute_time += BGlobal.input.delta_time / 60.0;
	if(BGlobal.input.minute_time >= 1.0)
		BGlobal.input.minute_time -= 1.0;
	BGlobal.time[0] = seconds;
	BGlobal.time[1] = fractions;
}
