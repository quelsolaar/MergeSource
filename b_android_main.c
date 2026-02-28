#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "betray.h"
#include "imagine.h"
#include <android/log.h>
#include <android/configuration.h>


static boolean	input_focus = TRUE;
static int	b_window_size_x = 800;
static int	b_window_size_y = 600;
static int	b_window_center_x = 400;
static int	b_window_center_y = 300;
static int	b_window_pos_x = 800;
static int	b_window_pos_y = 600;

uint betray_plugin_axis_allocate(uint user_id, uint draw_id, uint device_id, uint button_count, float x, float y, char *name);
void betray_plugin_axis_set(uint id, float axis_x, float axis_y, float axis_z);

#ifdef BETRAY_CONTEXT_OPENGLES
/*
EGLDisplay	sEGLDisplay;
EGLContext	sEGLContext;
EGLSurface	sEGLSurface;
*/
#endif

extern void betray_plugin_callback_main(BInputState *input);
extern void betray_plugin_pointer_clean();
extern void betray_action(BActionMode mode);
extern void betray_reshape_view(uint x_size, uint y_size);
extern uint betray_plugin_pointer_allocate(uint user_id, uint device_id, uint button_count, float x, float y, float z, float *origin, char *name, boolean draw);
extern void betray_plugin_pointer_set(uint id, float x, float y, float z, float *origin, boolean *buttons);
extern void betray_plugin_pointer_free(uint id);
extern void betray_plugin_button_set(uint user_id, uint id, boolean state, uint character);

uint betray_device_id;

void betray_glBindFramebufferEXT(GLenum target, GLuint framebuffer)
{

}

boolean b_win32_system_wrapper_set_display(uint size_x, uint size_y, boolean full_screen) 
{  
}

uint betray_support_functionality(BSupportedFunctionality funtionality)
{
	uint array[] = {
				1, /*B_SF_USER_COUNT_MAX*/
				16, /*B_SF_POINTER_COUNT_MAX*/
				1, /*B_SF_POINTER_BUTTON_COUNT*/
				TRUE, /*B_SF_FULLSCREEN*/
				FALSE, /*B_SF_WINDOWED*/
				FALSE, /*B_SF_VIEW_POSITION*/
				FALSE, /*B_SF_VIEW_ROTATION*/
				FALSE, /*B_SF_MOUSE_WARP*/
				FALSE, /*B_SF_EXECUTE*/
				FALSE, /*B_SF_REQUESTER*/
				TRUE}; /*B_SF_CLIPBOARD*/
	if(funtionality >= B_SF_COUNT)
		return FALSE;
	return array[funtionality];
}



void betray_requester_save_func(void *data)
{
}
char *betray_requester_save_get(void *id)
{
	return NULL;
}

void betray_requester_save(char **types, uint type_count, void *id)
{
}

void betray_requester_load_func(void *data)
{
}

char *betray_requester_load_get(void *id)
{
	return NULL;
}

void betray_requester_load(char **types, uint type_count, void *id)
{
}

char *betray_clipboard_get()
{
	return NULL;
}

void betray_clipboard_set(char *text)
{
}

boolean	betray_button_get(uint user_id, uint button)
{
}

void betray_button_get_up_down(uint user_id, boolean *press, boolean *last_press, uint button)
{
}

void betray_button_keyboard(uint user_id, boolean show)
{
}


void b_win32_window_close()
{

}

void betray_desktop_size_get(uint *size_x, uint *size_y) 
{
}

 
boolean betray_activate_context(void *context)
{
}

void *b_create_context() 
{
}

boolean b_init_display_opengl(uint size_x, uint size_y, boolean fullscreenflag, uint samples, char* title, boolean *sterioscopic) 
{

}

#ifdef BETRAY_CONTEXT_OPENGLES

/*
boolean b_win32_init_display_opengles2(uint size_x, uint size_y, boolean fullscreenflag, uint samples, char* title, boolean *sterioscopic) 
{
	PIXELFORMATDESCRIPTOR pfd;
	GLuint		PixelFormat;
	WNDCLASS	wc;
	RECT		WindowRect;
	char		*class_name = "OGLES";								// Set The Class Name

	EGLint aEGLAttributes[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint aEGLContextAttributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLConfig	aEGLConfigs[1];
	EGLint		cEGLConfigs;

	hDC = EGL_DEFAULT_DISPLAY;

	sEGLDisplay = eglGetDisplay(hDC);

	eglInitialize(sEGLDisplay, NULL, NULL);
	eglChooseConfig(sEGLDisplay, aEGLAttributes, aEGLConfigs, 1, &cEGLConfigs);


	WindowRect.left = 0;
	WindowRect.right = size_x;
	WindowRect.top = 0;
	WindowRect.bottom = size_y;

	if(!b_win32_instance)
	{
		b_win32_instance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
		wc.lpfnWndProc = (WNDPROC)WindowProc;					// WndProc Handles Messages
		wc.cbClsExtra = 0;									// No Extra Window Data
		wc.cbWndExtra = 0;									// No Extra Window Data
		wc.hInstance = b_win32_instance;							// Set The Instance
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
		wc.hbrBackground = NULL;									// No Background Required For GL
		wc.lpszMenuName = NULL;									// We Don't Want A Menu
		wc.lpszClassName = class_name;								// Set The Class Name

		if(!RegisterClass(&wc))									// Attempt To Register The Window Class
		{
			MessageBox(NULL,"BETRAY Error: Failed To Register The Window Class.","Betray ERROR",MB_OK|MB_ICONEXCLAMATION);
			return FALSE;											// Return FALSE
		}
	}

	if(fullscreenflag)
	{
		AdjustWindowRectEx(&WindowRect, WS_POPUP, FALSE, WS_EX_APPWINDOW);
		hWnd = CreateWindowEx(WS_EX_APPWINDOW, class_name, title, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, b_win32_instance, NULL);
	}
	else
	{
		AdjustWindowRectEx(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
		hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, class_name, title, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, b_win32_instance, NULL);
	}
	if(!hWnd)
	{
		MessageBox(NULL,"BETRAY Error: Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);


	sEGLSurface = eglCreateWindowSurface(sEGLDisplay, aEGLConfigs[0], (EGLNativeWindowType)hWnd, NULL);

	if(sEGLSurface == EGL_NO_SURFACE)
	{
		printf("BETRAY Error: to create EGL surface.\n");
		return FALSE;
	}
	sEGLContext = eglCreateContext(sEGLDisplay, aEGLConfigs[0], EGL_NO_CONTEXT, aEGLContextAttributes);

	if(sEGLContext == EGL_NO_CONTEXT)
	{
		printf("BETRAY Error: Failed to create EGL context.\n");
		return FALSE;
	}

	eglMakeCurrent(sEGLDisplay, sEGLSurface, sEGLSurface, sEGLContext);
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

#ifdef UNICODE
	{
		void *dll;
		unsigned int i;
		short u_text[32];
		char *text = "User32.dll";
		for(i = 0; i < 32 && text[i] != 0 ; i++)
			u_text[i] = (char)text[i];
		u_text[i] = 0;
		dll = LoadLibrary(u_text);
		if(dll != NULL)
		{
			text = "GetTouchInputInfo";
			for(i = 0; i < 32 && text[i] != 0 ; i++)
				u_text[i] = (char)text[i];
			u_text[i] = 0;
			betray_GetTouchInputInfo = (int (__stdcall *)(long, unsigned int, BetrayWinTouch *, int))GetProcAddress(dll, u_text);
			text = "CloseTouchInputHandle";
			for(i = 0; i < 32 && text[i] != 0 ; i++)
				u_text[i] = (char)text[i];
			u_text[i] = 0;
			betray_CloseTouchInputHandle = (int (__stdcall *)(long ))GetProcAddress(dll, u_text);
			text = "RegisterTouchWindow";
			for(i = 0; i < 32 && text[i] != 0 ; i++)
				u_text[i] = (char)text[i];
			u_text[i] = 0;
			betray_RegisterTouchWindow = (int (__stdcall *)(struct HWND__ *,unsigned long ))GetProcAddress(dll, u_text);
		}
		exit(0);
	}
#else
	{
		void *dll;
		dll = LoadLibrary("User32.dll");
		if(dll != NULL)
		{
			betray_GetTouchInputInfo = (int (__stdcall *)(long, unsigned int, BetrayWinTouch *, int))GetProcAddress(dll, "GetTouchInputInfo");
			betray_CloseTouchInputHandle = (int (__stdcall *)(long ))GetProcAddress(dll, "CloseTouchInputHandle");
			betray_RegisterTouchWindow = (int (__stdcall *)(struct HWND__ *,unsigned long ))GetProcAddress(dll, "RegisterTouchWindow");
		}
	}
#endif

	if(betray_RegisterTouchWindow != NULL)
		betray_RegisterTouchWindow(hWnd, BETRAY_TWF_FINETOUCH | BETRAY_TWF_FINETOUCH) ;
	return TRUE;
}
*/
#endif    

void system_wrapper_lose_focus(void)
{
	input_focus = FALSE;
}

void betray_set_mouse_warp(boolean warp)
{
}


void betray_set_mouse_move(float x, float y)
{
}

uint betray_support_context(BContextType context_type)
{
	return context_type == B_CT_OPENGLES2 || context_type == B_CT_OPENGL_OR_ES;
}

extern uint BGlobal_draw_state_fbo;
/*
void APIENTRY betray_glBindFramebufferEXT(GLenum target, GLuint framebuffer)
{
	static void (APIENTRY *internal_glBindFramebufferEXT)(GLenum target, GLuint framebuffer) = NULL;
	if(internal_glBindFramebufferEXT == NULL)
		internal_glBindFramebufferEXT = (void (APIENTRY __stdcall *)(GLenum , GLuint))wglGetProcAddress("glBindFramebufferEXT");
	if(framebuffer == 0)
		internal_glBindFramebufferEXT(target, BGlobal_draw_state_fbo);
	else
		internal_glBindFramebufferEXT(target, framebuffer);
} 

void *betray_gl_proc_address_get_internal(const char *text)
{
#ifdef BETRAY_CONTEXT_OPENGL
	if(b_win32_opengl_context_current == b_win32_opengl_context)
	{
		uint i;
		char *extension = "glBindFramebuffer";
		for(i = 0; extension[i] == text[i] && extension[i] != 0; i++);
		if(extension[i] == 0)
			return betray_glBindFramebufferEXT;
	}
	return wglGetProcAddress(text);
#endif
}
*/
void *betray_gl_proc_address_get()
{
	return (void *)eglGetProcAddress;
}

extern void betray_time_update(void);
int my_nCmdShow;
/*
void betray_launch_main_loop(void)
{
	MSG msg;
	BInputState *input;
	boolean swap;

	input = betray_get_input_state();
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	SetCapture(hWnd);
	betray_mouse_init();
    while(!window_close)
	{
		uint i, j;
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else										// If There Are No Messages
		{
			POINT current_pos;
			if(mouse_hide)
				ShowCursor(!mouse_inside);
			betray_time_update();
			betray_mouse_init(); 
			GetCursorPos(&current_pos);
			if(mouse_warp_move)
			{
				SetCursorPos(current_pos.x + (int)(mouse_warp_move_x * (float)b_window_size_x * 0.5), current_pos.y + (int)(mouse_warp_move_y * (float)b_window_size_y  * 0.5));
			}


			if(mouse_warp)
				SetCursorPos(b_window_center_x, b_window_center_y);
			mouse_inside = TRUE;


			for(i = 0; i < input->pointer_count; i++)
			{
				if(input->pointers[i].pointer_x > 1.0)
				{
					input->pointers[i].pointer_x = 1.0;
					mouse_inside = FALSE;
				}
				if(input->pointers[i].pointer_x < -1.0)
				{
					input->pointers[0].pointer_x = -1.0;
					mouse_inside = FALSE;
				}
				if(input->pointers[i].pointer_y > (float)b_window_size_y / (float)b_window_size_x)
				{
					input->pointers[i].pointer_y = (float)b_window_size_y / (float)b_window_size_x;
					mouse_inside = FALSE;
				}
				if(input->pointers[i].pointer_y < -((float)b_window_size_y / (float)b_window_size_x))
				{
					input->pointers[i].pointer_y = -((float)b_window_size_y / (float)b_window_size_x);
					mouse_inside = FALSE;
				}
				input->pointers[i].delta_pointer_x += input->pointers[i].pointer_x;
				input->pointers[i].delta_pointer_y += input->pointers[i].pointer_y;
				for(j = 0; j < input->pointers[i].button_count && j < B_POINTER_BUTTONS_COUNT; j++)
				{
					if(input->pointers[i].button[j] == TRUE && input->pointers[i].last_button[j] == FALSE)
					{
						input->pointers[i].click_pointer_x[j] = input->pointers[i].pointer_x;
						input->pointers[i].click_pointer_y[j] = input->pointers[i].pointer_y;
					}
				}
			}
			if(mouse_warp_move)
				mouse_warp_move = FALSE;
			betray_plugin_callback_main(input);
			betray_action(BAM_EVENT);	
			betray_action(BAM_DRAW);
#ifdef BETRAY_CONTEXT_OPENGL
			SwapBuffers(hDC);
			glFinish();
#endif
#ifdef BETRAY_CONTEXT_OPENGLES
			eglSwapBuffers(sEGLDisplay, sEGLSurface);
#endif
			input->frame_number++;
			betray_action(BAM_MAIN);
			betray_event_reset(input);
			betray_plugin_pointer_clean();
			for(i = 0; i < input->pointer_count; i++)
			{
				input->pointers[i].delta_pointer_x = -input->pointers[i].pointer_x;
				input->pointers[i].delta_pointer_y = -input->pointers[i].pointer_y;
				for(j = 0; j < input->pointers[i].button_count; j++)
					input->pointers[i].last_button[j] = input->pointers[i].button[j];
			}
		}
	}
}*/

                       


#include "pch.h"
#include "Cube.h"


/**
* Our saved state data.
*/
struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
};

/**
* Shared state for our app.
*/
struct engine {
	struct android_app* app;
	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	const ASensor* gyroscopicSensor;
	ASensorEventQueue* sensorEventQueue;
	uint axis_up;
	uint axis_forward;
	float axis_matrix[16];
	float last_accelerometer[3];
	float delta_accelerometer[3];

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};

/**
* Initialize an EGL context for the current display.
*/


static int engine_init_display(struct engine* engine)
{
	// initialize OpenGL ES and EGL

	/*
	* Here specify the attributes of the desired configuration.
	* Below, we select an EGLConfig with at least 8 bits per color
	* component compatible with on-screen windows
	*/
	const EGLint attribs[] = {
	//	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	EGLint aEGLContextAttributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;


	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	//sqrt((double)d_sqrt(3894576));
	if(!eglInitialize(display, 0, 0))
		exit(0);
	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	if(!eglChooseConfig(display, attribs, &config, 1, &numConfigs))
		exit(0);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	if(!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
		exit(0);


	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	if(surface == EGL_NO_SURFACE)
	{
		uint *a = NULL;
		a[0] = 0;
	}
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, aEGLContextAttributes);
		//		eglCreateContext(display, config, );

	if(eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	{
		uint *a = NULL;
		a[0] = 0;
	}
	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;
	engine->state.angle = 0;
	engine->axis_matrix[0] = 1;
	engine->axis_matrix[1] = 0;
	engine->axis_matrix[2] = 0;
	engine->axis_matrix[3] = 0;
	engine->axis_matrix[4] = 0;
	engine->axis_matrix[5] = 1;
	engine->axis_matrix[6] = 0;
	engine->axis_matrix[7] = 0;
	engine->axis_matrix[8] = 0;
	engine->axis_matrix[9] = 0;
	engine->axis_matrix[10] = 1;
	engine->axis_matrix[11] = 0;
	engine->axis_matrix[12] = 0;
	engine->axis_matrix[13] = 0;
	engine->axis_matrix[14] = 0;
	engine->axis_matrix[15] = 1;
	engine->last_accelerometer[0] = 0;
	engine->last_accelerometer[1] = 0;
	engine->last_accelerometer[2] = 0;
	engine->delta_accelerometer[0] = 0;
	engine->delta_accelerometer[1] = 0;
	engine->delta_accelerometer[2] = 0;
	// Initialize GL state.
	//Cube_setupGL(w, h);
	return 0;
}

/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display(struct engine* engine)
{
	if(engine->display != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if(engine->context != EGL_NO_CONTEXT)
			eglDestroyContext(engine->display, engine->context);
		if(engine->surface != EGL_NO_SURFACE)
			eglDestroySurface(engine->display, engine->surface);
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
//	Cube_tearDownGL();
}

/**
* Process the next input event.
*/

int GetUnicodeChar(struct android_app* app, int eventType, int keyCode, int metaState)
{
	JavaVM* javaVM;
	JNIEnv* jniEnv;
	int unicodeKey;
	jint result;
	JavaVMAttachArgs attachArgs;
	jclass class_key_event;
	attachArgs.version = JNI_VERSION_1_6;
	attachArgs.name = "NativeThread";
	attachArgs.group = NULL;
	javaVM = app->activity->vm;
	jniEnv = app->activity->env;
	result = javaVM->AttachCurrentThread(&jniEnv, &attachArgs);
	if(result == JNI_ERR)
	    return 0;
	class_key_event = jniEnv->FindClass("android/view/KeyEvent");
	if(metaState == 0)
	{
	    jmethodID method_get_unicode_char, eventConstructor;
		jobject eventObj;
		method_get_unicode_char = jniEnv->GetMethodID(class_key_event, "getUnicodeChar", "()I");
		eventConstructor = jniEnv->GetMethodID(class_key_event, "<init>", "(II)V");
		eventObj = jniEnv->NewObject(class_key_event, eventConstructor, eventType, keyCode);
		unicodeKey = jniEnv->CallIntMethod(eventObj, method_get_unicode_char);
	}else
	{
		jmethodID method_get_unicode_char, eventConstructor;
		jobject eventObj;
		method_get_unicode_char = jniEnv->GetMethodID(class_key_event, "getUnicodeChar", "(I)I");
		eventConstructor = jniEnv->GetMethodID(class_key_event, "<init>", "(II)V");
		eventObj = jniEnv->NewObject(class_key_event, eventConstructor, eventType, keyCode);
		 unicodeKey = jniEnv->CallIntMethod(eventObj, method_get_unicode_char, metaState);
	}
	javaVM->DetachCurrentThread();
	return unicodeKey;
}

uint betray_android_pointer_allocations[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
	struct engine* engine = (struct engine*)app->userData;
	BInputState *input;
	uint32 key_val, unicode;
	input = betray_get_input_state();
	switch(AInputEvent_getType(event))
	{
		case AINPUT_EVENT_TYPE_MOTION:
			{
				uint count, i, j, id, max_pointer_count;
				max_pointer_count = betray_support_functionality(B_SF_POINTER_COUNT_MAX);
				for(i = 0; i < max_pointer_count; i++)
				{
					if(betray_android_pointer_allocations[i] != -1)
					{
						input->pointers[i].last_button[0] = input->pointers[betray_android_pointer_allocations[i]].button[0];
						input->pointers[i].button[0] = FALSE;
						input->pointers[i].delta_pointer_x = 0;
						input->pointers[i].delta_pointer_y = 0;
					}
				}
				if(AMOTION_EVENT_ACTION_UP != AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK)
				{
					count = AMotionEvent_getPointerCount(event);
					for(i = 0; i < count && i < betray_support_functionality(B_SF_POINTER_COUNT_MAX); i++)
					{
						id = AMotionEvent_getPointerId(event, i);
						for(j = 0; j < max_pointer_count && betray_android_pointer_allocations[j] != id; j++);
						if(j == max_pointer_count)
						{
							j = betray_plugin_pointer_allocate(0, betray_device_id, 1,
								2.0 * (AMotionEvent_getX(event, i) - ((float)engine->width / 2.0)) / (float)engine->width,
								-2.0 * (AMotionEvent_getY(event, i) - ((float)engine->height / 2.0)) / (float)engine->width,
								-1, NULL,"Touch", FALSE);
							if(j < max_pointer_count)
								betray_android_pointer_allocations[j] = id;
						}
						if(j < max_pointer_count)
						{
							BInputPointerState *p;
							float x, y;
							p = &input->pointers[j];
							x = 2.0 * (AMotionEvent_getX(event, i) - ((float)engine->width / 2.0)) / (float)engine->width;
							y = -2.0 * (AMotionEvent_getY(event, i) - ((float)engine->height / 2.0)) / (float)engine->width;
							p->delta_pointer_x = x - p->pointer_x;
							p->delta_pointer_y = y - p->pointer_y;
							p->pointer_x = x;
							p->pointer_y = y;
							if(!p->last_button[0])
							{
								p->click_pointer_x[0] = p->pointer_x;
								p->click_pointer_y[0] = p->pointer_y;
							}
							if(AMOTION_EVENT_ACTION_UP != AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK)
								p->button[0] = TRUE;
						}
					}
				}else
				{
					for(i = 0; i < max_pointer_count; i++)
					{
						if(betray_android_pointer_allocations[i] != -1)
						{
							input->pointers[i].button[0] = FALSE;
							betray_plugin_pointer_free(i);
							betray_android_pointer_allocations[i] = -1;
						}
					}
				}
			}
			break;
		case AINPUT_EVENT_TYPE_KEY:
			key_val = (uint32)AKeyEvent_getKeyCode(event);
			unicode = GetUnicodeChar(app, AINPUT_EVENT_TYPE_KEY, key_val, AKeyEvent_getMetaState(event));
			if(unicode == 0)
				unicode = -1;
			if(AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
				betray_plugin_button_set(0, key_val, TRUE, unicode);
			else
				betray_plugin_button_set(0, key_val, FALSE, unicode);
			break;
	}
	return 1;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
	struct engine* engine = (struct engine*)app->userData;
	switch(cmd)
	{
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			engine->app->savedState = malloc(sizeof(struct saved_state));
			*((struct saved_state*)engine->app->savedState) = engine->state;
			engine->app->savedStateSize = sizeof(struct saved_state);
			break;
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			if(engine->app->window != NULL)
				engine_init_display(engine);
			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			engine_term_display(engine);
			break;
		case APP_CMD_GAINED_FOCUS:
			// When our app gains focus, we start monitoring the accelerometer.
			if(engine->accelerometerSensor != NULL)
			{
				ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
				ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L / 60) * 1000);
			}
			if(engine->gyroscopicSensor != NULL)
			{
				ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->gyroscopicSensor);
				ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->gyroscopicSensor, (1000L / 60) * 1000);
			}
			engine->animating = TRUE;
			break;
		case APP_CMD_LOST_FOCUS:
			// When our app loses focus, we stop monitoring the accelerometer.
			// This is to avoid consuming battery while not being used.
			if(engine->accelerometerSensor != NULL)
				ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
			if(engine->gyroscopicSensor != NULL)
				ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->gyroscopicSensor);
			// Also stop animating.
			engine->animating = FALSE;
			break;
	}
}

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/

struct android_app* betray_android_app_state;
struct engine engine;

void android_start(struct android_app* state)
{

}


boolean b_android_init_display(uint *window_size_x, uint *window_size_y, char *name)
{
	memset(&engine, 0, sizeof(engine));
	betray_android_app_state->userData = &engine;
	betray_android_app_state->onAppCmd = engine_handle_cmd;
	betray_android_app_state->onInputEvent = engine_handle_input;
	engine.surface = EGL_NO_SURFACE;
	engine.app = betray_android_app_state;
	// Prepare to monitor accelerometer
	betray_device_id = betray_plugin_input_device_allocate(0, "Device");
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	engine.gyroscopicSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_GYROSCOPE);
	if(engine.gyroscopicSensor != NULL || engine.accelerometerSensor != NULL)
	{
		;
		engine.axis_up = betray_plugin_axis_allocate(0, device_id, "Orientation up", B_AXIS_SCREEN_UP, 3);
		engine.axis_forward = betray_plugin_axis_allocate(0, device_id, "Orinentation forward", B_AXIS_SCREEN_FORWARD, 3);
	}
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, betray_android_app_state->looper, LOOPER_ID_USER, NULL, NULL);
	if(betray_android_app_state->savedState != NULL)
	{
		// We are starting with a previous saved state; restore from it.
		engine.state = *(struct saved_state*)betray_android_app_state->savedState;
	}
	engine.animating = 1;
	
	while(engine.surface == EGL_NO_SURFACE)
	{
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		if((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
		{
			if (source != NULL)
				source->process(betray_android_app_state, source);
			if (betray_android_app_state->destroyRequested != 0)
				return FALSE;
		}
	}
	*window_size_x = engine.width;
	*window_size_y = engine.height;
	return TRUE;
}

void betray_launch_main_loop(void)
{
	while(TRUE)
	{
		// Read all pending events.
		int ident;
		int events;
		float vector[3], counter[3], m[16];
		struct android_poll_source* source;

		while((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
		{
			if(source != NULL)
				source->process(betray_android_app_state, source);
			if(ident == LOOPER_ID_USER)
			{
				ASensorEvent event;
				while(ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0)
				{
					switch(event.type)
					{	
						case ASENSOR_TYPE_ACCELEROMETER :
							f_matrix_reverse4f(m, engine.axis_matrix); 
							vector[0] = m[4] * 0.995 + 0.005 * event.acceleration.x;
							vector[1] = m[5] * 0.995 + 0.005 * event.acceleration.y;
							vector[2] = m[6] * 0.995 + 0.005 * event.acceleration.z;
							counter[0] = m[8];
							counter[1] = m[9];
							counter[2] = m[10];
							f_matrixyzf(m, NULL, vector, counter);
							f_matrix_reverse4f(engine.axis_matrix, m); 
							betray_plugin_axis_set(engine.axis_up, engine.axis_matrix[4], engine.axis_matrix[5], engine.axis_matrix[6]);
							betray_plugin_axis_set(engine.axis_forward, engine.axis_matrix[8], engine.axis_matrix[9], engine.axis_matrix[10]);
						break;
						case ASENSOR_TYPE_GYROSCOPE :
							vector[0] = engine.axis_matrix[4] + event.uncalibrated_gyro.x_uncalib * engine.axis_matrix[8] * 0.02 - event.uncalibrated_gyro.z_uncalib * engine.axis_matrix[0] * 0.02;
							vector[1] = engine.axis_matrix[5] + event.uncalibrated_gyro.x_uncalib * engine.axis_matrix[9] * 0.02 - event.uncalibrated_gyro.z_uncalib * engine.axis_matrix[1] * 0.02;
							vector[2] = engine.axis_matrix[6] + event.uncalibrated_gyro.x_uncalib * engine.axis_matrix[10] * 0.02 - event.uncalibrated_gyro.z_uncalib * engine.axis_matrix[2] * 0.02;
							
							counter[0] = engine.axis_matrix[8] + event.uncalibrated_gyro.y_uncalib * engine.axis_matrix[0] * 0.02;
							counter[1] = engine.axis_matrix[9] + event.uncalibrated_gyro.y_uncalib * engine.axis_matrix[1] * 0.02;
							counter[2] = engine.axis_matrix[10] + event.uncalibrated_gyro.y_uncalib * engine.axis_matrix[2] * 0.02;

							f_matrixyzf(engine.axis_matrix, NULL, vector, counter);
							betray_plugin_axis_set(engine.axis_up, engine.axis_matrix[4], engine.axis_matrix[5], engine.axis_matrix[6]);
							betray_plugin_axis_set(engine.axis_forward, engine.axis_matrix[8], engine.axis_matrix[9], engine.axis_matrix[10]);
						break;
					}
				}
			}
			if(betray_android_app_state->destroyRequested != 0)
				return;
		}

		betray_time_update();
		if(engine.animating)
		{
			BInputState *input;
			input = betray_get_input_state();
			betray_action(BAM_EVENT);
			if(engine.display != NULL)
			{
				betray_action(BAM_DRAW);
				eglSwapBuffers(engine.display, engine.surface);
			}
			input->frame_number++;
		//	betray_event_reset(input);
		}
		betray_action(BAM_MAIN);
	}
}

int main(int argc, char **argv); /*is extern but cant have exterin infront of it. */

void android_main(struct android_app* state)
{
	betray_android_app_state = state;
//	android_start(state);
	char *argument = "file.exe";
/*	{
		FILE *f;
		f = fopen("BETRAY_output.txt", "w");
		fprintf(f, "BETRAY HEY WE GOT SOMETHING!!!!\n");
		fclose(f);
	}*/
	main(1, &argument);
	engine_term_display(&engine);

}
