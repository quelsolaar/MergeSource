// we don't need much here
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <mach-o/dyld.h>
#include <OpenGL/gl.h>
#include <dlfcn.h>
#include "betray.h"

#include "macos/c-ocoa/c_ocoa_class_api.c"
#include "macos/c-ocoa/nsapplication.c"
#include "macos/c-ocoa/nswindow.c"
#include "macos/c-ocoa/nscolor.c"
#include "macos/c-ocoa/nsprocessinfo.c"
#include "macos/c-ocoa/nsmenu.c"
#include "macos/c-ocoa/nsmenuitem.c"
#include "macos/c-ocoa/nsscreen.c"
#include "macos/c-ocoa/nsarray.c"
#include "macos/c-ocoa/nsobject.c"
#include "macos/c-ocoa/nsview.c"
#include "macos/c-ocoa/nsopenglpixelformat.c"
#include "macos/c-ocoa/nsopenglcontext.c"
#include "macos/c-ocoa/nsstring.c"
#include "macos/c-ocoa/nsdate.c"
#include "macos/c-ocoa/nsevent.c"
#include "macos/c-ocoa/nspasteboard.c"
#include "macos/c-ocoa/uifont.c"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
// this is how they are defined originally
//#include <CoreGraphics/CGBase.h>
//#include <CoreGraphics/CGGeometry.h>
//typedef CGPoint NSPoint;
//typedef CGRect NSRect;

extern id NSApp;
extern id const NSDefaultRunLoopMode;
#endif

#if defined(__OBJC__) && __has_feature(objc_arc)
#define ARC_AVAILABLE
#endif

bool terminated = false;
uint32_t windowCount = 0;

// we gonna construct objective-c class by hand in runtime, so wow, so hacker!
//@interface AppDelegate : NSObject<NSApplicationDelegate>
//-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
//@end
//@implementation AppDelegate
//-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
//{
//	terminated = true;
//	return NSTerminateCancel;
//}
//@end
NSUInteger applicationShouldTerminate(id self, SEL _sel, id sender)
{
	printf("requested to terminate\n");
	//terminated = true;
	return 0;
}

//@interface WindowDelegate : NSObject<NSWindowDelegate>
//-(void)windowWillClose:(NSNotification*)notification;
//@end
//@implementation WindowDelegate
//-(void)windowWillClose:(NSNotification*)notification
//{
//	(void)notification;
//	assert(windowCount);
//	if(--windowCount == 0)
//		terminated = true;
//}
//@end

nsopenglcontext_t   betray_opengl_context_default;
nsopenglcontext_t   betray_opengl_context_current;
nsview_t            betray_content_view;
nswindow_t          betray_window;

uint betray_screen_screen_resolution_x = 0;
uint betray_screen_screen_resolution_y = 0;

uint betray_user_id;
uint betray_scroll_axis_id = -1;
uint betray_mouse_device_id = -1;
uint betray_mouse_pointer_id = -1;
uint betray_keyboard_device_id = -1;

extern int main(int argc, char** argv);

extern void betray_plugin_button_set(uint user_id, uint id, boolean state, uint character);
extern void betray_plugin_pointer_clean();
extern uint betray_plugin_user_allocate();
extern uint betray_plugin_input_device_allocate(uint user_id, char *name);
extern uint betray_plugin_pointer_allocate(uint user_id, uint device_id, uint button_count, float x, float y, float z, float *origin, char *name, boolean draw);
extern uint betray_plugin_axis_allocate(uint user_id, uint device_id, char *name, BAxisType type, uint axis_count);
extern void betray_plugin_axis_set(uint id, float axis_x, float axis_y, float axis_z);
extern void betray_plugin_callback_main(BInputState *input);
extern void betray_reshape_view(uint x_size, uint y_size);
extern void betray_action(BActionMode mode);
extern void betray_time_update();

void betray_url_launch(char* url)
{

}

void betray_device_init()
{
    betray_user_id = betray_plugin_user_allocate();
    betray_mouse_device_id = betray_plugin_input_device_allocate(0, "Mouse");
    betray_keyboard_device_id = betray_plugin_input_device_allocate(0, "Keyboard");
    betray_mouse_pointer_id = betray_plugin_pointer_allocate(betray_user_id, betray_mouse_device_id, 3, 0.0, 0.0, -1, NULL, "Mouse", FALSE);
    betray_scroll_axis_id = betray_plugin_axis_allocate(0, betray_mouse_device_id, "Scroll", B_AXIS_SCORLL, 2);
}

void windowWillClose(id self, SEL _sel, id notification)
{
	printf("window will close\n");
	assert(windowCount);
	if(--windowCount == 0)
		terminated = true;
}


//extern boolean b_win32_init_display_opengles2(uint size_x, uint size_y, boolean full_screen, uint samples, char *caption, boolean *sterioscopic);

boolean b_init_display_opengl(uint size_x, uint size_y, boolean full_screen, uint samples, char *caption, boolean *sterioscopic)
{
	nsapplication_t nsapp = nsapplication_sharedApplication();
	nsapplication_setActivationPolicy(nsapp, 0);

    c_ocoa_class_definition_t* pAppDelegateDefinition = c_ocoa_create_class_definition("BetrayUiAppDelegate");
    c_ocoa_add_class_definition_method( pAppDelegateDefinition, "applicationShouldTerminate:", (void*)applicationShouldTerminate, "L@:@" );
    c_ocoa_finish_class_definition( pAppDelegateDefinition );

    nsobject_t appDelegate = c_ocoa_alloc_object_of_class( pAppDelegateDefinition );
    nsobject_init( appDelegate );
    nsobject_autorelease( appDelegate );

    nsapplication_setDelegate( nsapp, appDelegate );

	// only needed if we don't use [NSApp run]
	//[NSApp finishLaunching];
	nsapplication_finishLaunching( nsapp );

	//id menubar = [[NSMenu alloc] init];
	nsmenu_t menubar = nsmenu_init( nsmenu_alloc() );
	nsmenu_autorelease( menubar );

	//id appMenuItem = [[NSMenuItem alloc] init];
	nsmenuitem_t appMenuItem = nsmenuitem_init( nsmenuitem_alloc() );
	nsmenuitem_autorelease( appMenuItem );

	//[menubar addItem:appMenuItem];
	//nsmenu_addItem( menubar, appMenuItem );

	//[NSApp setMainMenu:menubar];
	//nsapplication_setMainMenu( nsapp, menubar );

	//id appMenu = [[NSMenu alloc] init];
	nsmenu_t appMenu = nsmenu_init( nsmenu_alloc() );
	nsmenu_autorelease( appMenu );

	//id appName = [[NSProcessInfo processInfo] processName];
	nsprocessinfo_t processInfo = nsprocessinfo_processInfo();
	nsstring_t appName = nsprocessinfo_processName(processInfo);

	//id quitTitle = [@"Quit " stringByAppendingString:appName];
	nsstring_t quitTitlePrefix = nsstring_stringWithUTF8String( "Quit " );
    nsstring_t quitTitle = nsstring_stringByAppendingString( quitTitlePrefix, appName );
    
	//id quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
	nsstring_t quitMenuItemKey = nsstring_stringWithUTF8String( "q" );
    nsmenuitem_t quitMenuItem = nsmenuitem_initWithTitle( nsmenuitem_alloc(), quitTitle, sel_registerName("terminate:"), quitMenuItemKey );

	//[appMenu addItem:quitMenuItem];
	//nsmenu_addItem( appMenu, quitMenuItem );

	//[appMenuItem setSubmenu:appMenu];
	//nsmenuitem_setSubmenu( appMenuItem, appMenu );

    nsapplication_finishLaunching( nsapp );


    nsscreen_t mainScreen = nsscreen_mainScreen();
    nsarray_t screens = nsscreen_screens();
    unsigned long long screenCount = nsarray_count( screens );
    nsscreen_t specificScreen = nsarray_objectAtIndex( screens, 0u );
    CGRect frameRect = nsscreen_frame( mainScreen );

    betray_screen_screen_resolution_x = frameRect.member1.member0;
    betray_screen_screen_resolution_y = frameRect.member1.member1;

    betray_window = nswindow_initWithContentRect( nswindow_alloc(), frameRect, 15, 2, NO );
    
    nsobject_autorelease( betray_window );

    nswindow_setReleasedWhenClosed( betray_window, NO );

    betray_content_view = nswindow_contentView( betray_window );
    nsview_setWantsBestResolutionOpenGLSurface( betray_content_view, YES );

    CGPoint point = {0, 0};

    if(size_x != 0)
      point.member0 = (betray_screen_screen_resolution_x - size_x) / 2;
    if(size_y != 0)
       point.member1 = (betray_screen_screen_resolution_y + size_y) / 2;

    nswindow_cascadeTopLeftFromPoint( betray_window, point );

    for(uint32_t i = 0; i < 3; i++)
    {
        uint32_t glAttributes[] = {
		8, 24,
		11, 8,
		5,
		73,
		72,
		55, 1,
        56, 4,
        99, 0x3200, // or 0x1000 0x3200 0x4100
     // 99, 0x4100, // or 0x1000 0x3200 0x4100
		0};
        uint32_t version[] = {0x4100, 0x3200, 0x1000};
        glAttributes[12] = version[i];
        
        nsopenglpixelformat_t pixelFormat = nsopenglpixelformat_initWithAttributes( nsopenglpixelformat_alloc(), glAttributes );
        nsobject_autorelease( pixelFormat );
        
        betray_opengl_context_default = nsopenglcontext_initWithFormat( nsopenglcontext_alloc(), pixelFormat, nil );
        if( betray_opengl_context_default != NULL )
        {
            break;
        }
    }

    if( betray_opengl_context_default == NULL )
    {
        return FALSE;
    }

    betray_opengl_context_current = betray_opengl_context_default;

    nsobject_autorelease( betray_opengl_context_current );
    nsopenglcontext_setView( betray_opengl_context_current, betray_content_view );

    nsstring_t windowTitle = nsstring_stringWithUTF8String( caption );
    nswindow_setTitle( betray_window, windowTitle );

    nswindow_makeKeyAndOrderFront( betray_window, betray_window );
    nswindow_setAcceptsMouseMovedEvents( betray_window, YES );
    nswindow_setBackgroundColor( betray_window, nscolor_blackColor() );

    nsapplication_activateIgnoringOtherApps( nsapp, YES );

    nsopenglcontext_makeCurrentContext( betray_opengl_context_current );
    nsapplication_setPresentationOptions( nsapp, (1<<10) );

    return TRUE;
}

void betray_launch_main_loop(void)
{
    //nsautoreleasepool_t pool = nsautoreleasepool_alloc( nsautoreleasepool_init() );
    
    static uint32 last_modifyer_keys = 0;
    BInputState *input;
    uint i, j, event_timer = 0;
    input = betray_get_input_state();
	printf("entering runloop\n");

    nsapplication_t nsapp = nsapplication_sharedApplication();

	while(!terminated)
    {
        nsdate_t distantPast = nsdate_distantPast();
        nsevent_t event = nsapplication_nextEventMatchingMask(nsapp, 0xFFFF, distantPast, NSDefaultRunLoopMode, YES);
        
        while( event != NULL )
        {
            unsigned long long eventType = nsevent_type(event);
          switch(eventType)
			{
				//case NSMouseMoved:
				//case NSLeftMouseDragged:
				//case NSRightMouseDragged:
				//case NSOtherMouseDragged:
				case 5:
				case 6:
				case 7:
				case 27:
				{
                    nsview_t currentWindowContentView = nswindow_contentView(betray_window);
                    CGRect adjustFrame = nsview_frame(betray_content_view);
                    CGPoint p = nswindow_mouseLocationOutsideOfEventStream(betray_window);
                    
					// map input to content view rect
					if(p.member0 < 0)
                        p.member0 = 0;
					else if(p.member0 > adjustFrame.member1.member0)
                        p.member0 = adjustFrame.member1.member0;
					if(p.member1 < 0)
                        p.member1 = 0;
					else if(p.member1 > adjustFrame.member1.member1)
                        p.member1 = adjustFrame.member1.member1;
                    input->pointers[0].pointer_x = (float)p.member0 / (float)adjustFrame.member1.member0 * 2.0 - 1.0;
                    input->pointers[0].pointer_y = (-1.0 + (float)p.member1 / (float)adjustFrame.member1.member1 * 2.0) * (float)adjustFrame.member1.member1 / (float)adjustFrame.member1.member0;

					// map input to pixels
					CGRect r = {p.member0, p.member1, 0, 0};
                    r = nsview_convertRectToBacking(currentWindowContentView, r);
					p = r.member0;

					printf("mouse moved to %f %f\n", p.member0, p.member1);
					
				}
                break;
				//case NSLeftMouseDown:
                case 1:
                    nsapplication_terminate(nsapp, nsapp);
                    input->pointers[0].button[0] = TRUE;
					printf("mouse left key down\n");
				break;
				//case :
				case 2:
                    input->pointers[0].button[0] = FALSE;
					printf("mouse left key up\n");
				break;
				//case NSRightMouseDown:
                case 3:
                    input->pointers[0].button[1] = TRUE;
					printf("mouse right key down\n");
				break;
				//case NSRightMouseUp:
                case 4:
                    input->pointers[0].button[1] = FALSE;
					printf("mouse right key up\n");
				break;
				//case NSOtherMouseDown:
				case 25:
				{
					// number == 2 is a middle button
                    long long number = nsevent_buttonNumber(event);
                    if(number < B_POINTER_BUTTONS_COUNT)
                        input->pointers[0].button[number] = TRUE;
					printf("mouse other key down : %i\n", (int)number);
				}
                break;
				//case NSOtherMouseUp:
				case 26:
				{
					long long number = nsevent_buttonNumber(event);
                    if(number < B_POINTER_BUTTONS_COUNT)
                        input->pointers[0].button[number] = FALSE;
				}
                break;
				//case NSScrollWheel:
				case 22:
				{
                    double delta_x, delta_y;
                    BOOL precision_scrolling;
                    delta_x = nsevent_scrollingDeltaX(event);
                    delta_x = nsevent_scrollingDeltaY(event);
                    precision_scrolling = nsevent_hasPreciseScrollingDeltas(event);
					if(precision_scrolling)
					{
						delta_x *= 0.1; // similar to glfw
						delta_y *= 0.1;
                    }
                    if(delta_x >= 0.001)
                        betray_plugin_button_set(0, BETRAY_BUTTON_SCROLL_UP, TRUE, -1);
                    if(delta_y <= -0.001)
                        betray_plugin_button_set(0, BETRAY_BUTTON_SCROLL_DOWN, TRUE, -1);
                    if(delta_y >= 0.001)
                        betray_plugin_button_set(0, BETRAY_BUTTON_SCROLL_LEFT, TRUE, -1);
                    if(delta_y <= -0.001)
                        betray_plugin_button_set(0, BETRAY_BUTTON_SCROLL_RIGHT, TRUE, -1);
                    betray_plugin_axis_set(betray_scroll_axis_id, (float)delta_x, (float)delta_y, 0);
				}
                break;
				//case NSFlagsChanged:
				case 12:
				{
                    uint32 bits;
                    unsigned long long modifiers = nsevent_modifierFlags(event);

                    bits = (modifiers & 0xffff0000UL) >> 16;

                    if((bits & 2) != (last_modifyer_keys & 2))
						betray_plugin_button_set(0, BETRAY_BUTTON_SHIFT, bits & 2, -1);
					if((bits & 4) != (last_modifyer_keys & 4))
						betray_plugin_button_set(0, BETRAY_BUTTON_CONTROL, bits & 4, -1);
					if((bits & 8) != (last_modifyer_keys & 8))
						betray_plugin_button_set(0, BETRAY_BUTTON_ALT, bits & 8, -1);
					last_modifyer_keys = bits;


					break;
				}
				//case NSKeyDown:
				case 10:
				{
                    uint pos = 0;
                    nsstring_t inputText = nsevent_characters(event);
                    const char * inputTextUTF8 = nsstring_UTF8String(inputText);
                    uint16_t keyCode = nsevent_keyCode(event);
					if((last_modifyer_keys & 16) && (last_modifyer_keys & 2) && keyCode == BETRAY_BUTTON_Z)
					{
						betray_plugin_button_set(0, BETRAY_BUTTON_REDO, TRUE, -1);
					}else if((last_modifyer_keys & 16) && keyCode == BETRAY_BUTTON_Z)
					{
						betray_plugin_button_set(0, BETRAY_BUTTON_UNDO, TRUE, -1);
					}else if((last_modifyer_keys & 16) && keyCode == BETRAY_BUTTON_X)
					{
						betray_plugin_button_set(0, BETRAY_BUTTON_CUT, TRUE, -1);
					}else if((last_modifyer_keys & 16) && keyCode == BETRAY_BUTTON_C)
					{
						betray_plugin_button_set(0, BETRAY_BUTTON_COPY, TRUE, -1);
					}else if((last_modifyer_keys & 16) && keyCode == BETRAY_BUTTON_V)
					{
						betray_plugin_button_set(0, BETRAY_BUTTON_PASTE, TRUE, -1);
					}else
						betray_plugin_button_set(0, keyCode, TRUE, f_utf8_to_uint32(inputTextUTF8, &pos));
					printf("key down %u, text '%s'\n", keyCode, inputTextUTF8);
				}
                break;
				//case NSKeyUp:
				case 11:
				{
                    uint pos = 0;
                    nsstring_t inputText = nsevent_characters(event);
                    const char * inputTextUTF8 = nsstring_UTF8String(inputText);
                    uint16_t keyCode = nsevent_keyCode(event);
                    betray_plugin_button_set(0, keyCode, FALSE, f_utf8_to_uint32(inputTextUTF8, &pos));
                    printf("key up %u\n", keyCode);
					break;
				}
				default:
					break;
			}

            nsapplication_sendEvent(nsapp, event);

			// if user closes the window we might need to terminate asap
			if(terminated)
				break;

            event = nsapplication_nextEventMatchingMask(nsapp, 0xFFFF, distantPast, NSDefaultRunLoopMode, YES);
        }
 
		// do runloop stuff
		//[openGLContext update]; // probably we only need to do it when we resize the window
	//	((void (*)(id, SEL))objc_msgSend)(betray_opengl_context_current, updateSel);

		//[openGLContext makeCurrentContext];
	//	((void (*)(id, SEL))objc_msgSend)(betray_opengl_context_current, makeCurrentContextSel);

        CGRect rect = nsview_frame(betray_content_view);
        rect = nsview_convertRectToBacking(betray_content_view, rect);

        betray_reshape_view(rect.member1.member0, rect.member1.member1);
        for(i = 0; i < input->pointer_count; i++)
        {
            input->pointers[i].delta_pointer_x += input->pointers[i].pointer_x;
            input->pointers[i].delta_pointer_y += input->pointers[i].pointer_y;
            input->pointers[i].delta_pointer_z += input->pointers[i].pointer_z;
            for(j = 0; j < input->pointers[i].button_count && j < B_POINTER_BUTTONS_COUNT; j++)
            {
                if(input->pointers[i].button[j] && !input->pointers[i].last_button[j])
                {
                    input->pointers[i].click_pointer_x[j] = input->pointers[i].pointer_x;
                    input->pointers[i].click_pointer_y[j] = input->pointers[i].pointer_y;
                    input->pointers[i].click_pointer_z[j] = input->pointers[i].pointer_z;
                }
            }
        }
        betray_plugin_callback_main(input);
      //  if(event_timer < 3)
        {
            betray_action(BAM_EVENT);
            event_timer++;
        }
        betray_action(BAM_DRAW);
#ifdef BETRAY_CONTEXT_OPENGL
        glFinish();
        //[NSApp updateWindows];
        nsopenglcontext_flushBuffer(betray_opengl_context_current);
        nsopenglcontext_update(betray_opengl_context_current);
        nsapplication_updateWindows(nsapp);
#endif
#ifdef BETRAY_CONTEXT_OPENGLES
        eglSwapBuffers(sEGLDisplay, sEGLSurface);
#endif
        input->frame_number++;
        betray_time_update();
        betray_action(BAM_MAIN);
        input->button_event_count = 0;
        betray_plugin_pointer_clean();
        for(i = 0; i < input->pointer_count; i++)
        {
            input->pointers[i].delta_pointer_x = -input->pointers[i].pointer_x;
            input->pointers[i].delta_pointer_y = -input->pointers[i].pointer_y;
            for(j = 0; j < input->pointers[i].button_count; j++)
                input->pointers[i].last_button[j] = input->pointers[i].button[j];
        }
    }

	printf("gracefully terminated\n");

    //nsautoreleasepool_release(pool);
}

uint betray_support_functionality(BSupportedFunctionality funtionality)
{
    uint array[] = {
        256, /*B_SF_USER_COUNT_MAX*/
        256, /*B_SF_POINTER_COUNT_MAX*/
        TRUE, /*B_SF_FULLSCREEN*/
        TRUE, /*B_SF_WINDOWED*/
        FALSE, /*B_SF_VIEW_POSITION*/
        FALSE, /*B_SF_VIEW_ROTATION*/
        FALSE, /*B_SF_MOUSE_WARP*/
        FALSE, /*B_SF_EXECUTE*/
        TRUE, /*B_SF_REQUESTER*/
        TRUE}; /*B_SF_CLIPBOARD*/
    if(funtionality >= B_SF_COUNT)
        return FALSE;
    return array[funtionality];
}

/*
char *betray_requester_save_get(void *id)
{
    return NULL;
}

void betray_requester_save(char **types, uint type_count, void *id)
{
}

char *betray_requester_load_get(void *id)
{
    return NULL;
}

void betray_requester_load(char **types, uint type_count, void *id)
{
}
*/

char *betray_clipboard_get()
{
    nspasteboard_t pasteboard = nspasteboard_generalPasteboard();
    nspasteboard_autorelease(pasteboard);

    nsstring_t pasteBoardType = nsstring_stringWithUTF8String("public.utf8-plain-text");
    nsstring_t content = nspasteboard_stringForType(pasteboard, pasteBoardType);

    if( content == NULL ){
        return NULL;
    }

    nsstring_autorelease(content);
    unsigned long length = nsstring_lengthOfBytesUsingEncoding(content, 4);
    
    char *cStr = (char *)malloc(sizeof(char) * (uint)length);
    if(!nsstring_getCString(content, cStr, length, 4)){
        free(cStr);
        return NULL;
    }
    
    return cStr;
}

void betray_clipboard_set(char *text)
{
    nspasteboard_t pasteboard = nspasteboard_generalPasteboard();
    nspasteboard_autorelease(pasteboard);
    nspasteboard_clearContents(pasteboard);

    nsstring_t str = nsstring_stringWithUTF8String(text);
    nsstring_autorelease(str);    

    nsstring_t pasteboardTypeString = nsstring_stringWithUTF8String("public.utf8-plain-text");
    nspasteboard_setString(pasteboard, str, pasteboardTypeString);
}

void betray_button_keyboard(uint user_id, boolean show)
{
}

void betray_desktop_size_get(uint *size_x, uint *size_y)
{
	if(betray_screen_screen_resolution_x == 0)
	{
        nsscreen_t mainScreen = nsscreen_mainScreen();
        nsarray_t screens = nsscreen_screens();
        uint array_length = nsarray_count(screens);
        
        nsscreen_t specific_screen = nsarray_objectAtIndex(screens, 0);
        CGRect rect = nsscreen_visibleFrame(specific_screen);

		betray_screen_screen_resolution_x = rect.member1.member0;
		betray_screen_screen_resolution_y = rect.member1.member1;
	}
    if(size_x != NULL)
        *size_x = betray_screen_screen_resolution_x;
    if(size_y != NULL)
        *size_y = betray_screen_screen_resolution_y;
}

boolean b_win32_screen_mode(uint size_x, uint size_y)
{
    return FALSE;
}

boolean betray_activate_context(void *context)
{
    if(context == NULL)
        context = betray_opengl_context_default;
    if(context == betray_opengl_context_current)
        return TRUE;
    nsopenglcontext_makeCurrentContext(context);
    return TRUE;
}

void *b_create_context()
{
    return NULL;
}




void system_wrapper_lose_focus(void)
{
 //   input_focus = FALSE;
}

void betray_set_mouse_warp(boolean warp)
{
 //   mouse_warp = warp;
}


void betray_set_mouse_move(float x, float y)
{
 /*   BInputState *input;
    input = betray_get_input_state();
    mouse_warp_move_x = x - input->pointers[0].pointer_x;
    mouse_warp_move_y = input->pointers[0].pointer_y - y;
    mouse_warp_move = TRUE;*/
}

uint betray_support_context(BContextType context_type)
{
    return context_type == B_CT_OPENGL || context_type == B_CT_OPENGL_OR_ES;
}

extern uint BGlobal_draw_state_fbo;

void *betray_get_proc_address(const char *name)
{
    static const char * gl_lib_path = "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL";
    static void *lib = NULL;
    if(lib == NULL)
        lib = dlopen(gl_lib_path, RTLD_LAZY);
    void *addr = dlsym(lib, name);
    if(!addr)
    {
        size_t len;
        len = strlen(name);
        if(len > 3 && strcmp(name + (len - 3), "ARB") == 0)
        {
            char nameWithoutARB[len - 3 + 1];
            strncpy(nameWithoutARB, name, len - 3);
            nameWithoutARB[len - 3] = '\0';
            addr = dlsym(lib, name);
        }
    }
    return addr;
 }

void betray_glBindFramebufferEXT(GLenum target, GLuint framebuffer)
{
    static void (*internal_glBindFramebufferEXT)(GLenum target, GLuint framebuffer) = NULL;
    if(internal_glBindFramebufferEXT == NULL)
        internal_glBindFramebufferEXT = (void (*)(GLenum , GLuint))betray_get_proc_address("glBindFramebufferEXT");
    if(framebuffer == 0)
        internal_glBindFramebufferEXT(target, BGlobal_draw_state_fbo);
    else
        internal_glBindFramebufferEXT(target, framebuffer);
}


GLubyte *betray_glGetString(GLenum name)
{
    if(name == GL_EXTENSIONS)
    {
     //   return "GL_ARB_vertex_array_bgra GL_ARB_draw_elements_base_vertex GL_ARB_fragment_coord_conventions GL_ARB_provoking_vertex GL_ARB_seamless_cube_map GL_ARB_texture_multisample GL_ARB_depth_clamp GL_ARB_sync GL_ARB_geometry_shader4 GL_ARB_uniform_buffer_object GL_ARB_draw_instanced GL_EXT_copy_buffer GL_NV_primitive_restart GL_ARB_texture_buffer_object GL_ARB_texture_rectangle GL_ARB_compatibility GL_ARB_framebuffer_object GL_ARB_vertex_array_object GL_NV_conditional_render GL_ARB_color_buffer_float GL_NV_depth_buffer_float GL_ARB_texture_float GL_EXT_packed_float GL_EXT_texture_shared_exponent GL_NV_half_float GL_EXT_half_float_pixel GL_EXT_texture_integer GL_EXT_texture_array GL_EXT_draw_buffers2 GL_EXT_texture_compression_rgtc GL_ARB_transform_feedback GL_EXT_framebuffer_sRGBde GL_ARB_pixel_buffer_object GL_EXT_texture_sRGB GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader GL_ARB_shading_language_100 GL_ARB_draw_buffers GL_ARB_texture_non_power_of_two GL_ARB_point_sprite GL_ATI_separate_stencil GL_EXT_stencil_two_side GL_ARB_vertex_buffer_object GL_ARB_occlusion_query GL_EXT_shadow_funcs GL_SGIS_generate_mipmap GL_NV_blend_equare GL_ARB_depth_texture GL_ARB_shadow GL_EXT_fog_coord GL_EXT_multi_draw_arrays GL_ARB_point_parameters GL_EXT_secondary_color GL_EXT_blend_func_separate GL_EXT_stencil_wrap GL_ARB_texture_env_crossbar GL_EXT_texture_lod_bias GL_ARB_texture_mirrored_repeat GL_ARB_window_pos GL_ARB_texture_compression GL_ARB_texture_cube_map ARB_multisample GL_ARB_multitexture GL_ARB_texture_env_add GL_ARB_texture_env_combine GL_ARB_texture_env_dot3 GL_ARB_texture_border_clamp GL_ARB_transpose_matrix EXT_texture3D GL_EXT_bgra GL_EXT_packed_pixels GL_EXT_rescale_normal GL_EXT_separate_specular_color GL_SGIS_texture_edge_clamp GL_SGIS_texture_lod GL_EXT_draw_range_elements GL_EXT_color_table GL_EXT_color_subtable GL_EXT_convolution GL_EXT_convolution_border_modes GL_EXT_color_matrix GL_EXT_histogram GL_EXT_blend_color GL_EXT_blend_minmax GL_EXT_blend_substract GL_EXT_vertex_array GL_EXT_polygon_offset GL_EXT_blend_logic_op GL_EXT_texture GL_EXT_copy_texture GL_EXT_subtexture GL_EXT_texture_object GL_ARB_framebuffer_object GL_ARB_vertex_buffer_object";
        return "GL_ARB_vertex_array_bgra GL_ARB_draw_elements_base_vertex GL_ARB_fragment_coord_conventions GL_ARB_provoking_vertex GL_ARB_seamless_cube_map GL_ARB_texture_multisample GL_ARB_depth_clamp GL_ARB_sync GL_ARB_geometry_shader4 GL_ARB_uniform_buffer_object GL_ARB_draw_instanced GL_EXT_copy_buffer GL_NV_primitive_restart GL_ARB_texture_buffer_object GL_ARB_texture_rectangle GL_ARB_compatibility GL_ARB_framebuffer_object GL_ARB_vertex_array_object GL_NV_conditional_render GL_ARB_color_buffer_float GL_NV_depth_buffer_float GL_ARB_texture_float GL_EXT_packed_float GL_EXT_texture_shared_exponent GL_NV_half_float GL_EXT_half_float_pixel GL_EXT_texture_integer GL_EXT_texture_array GL_EXT_draw_buffers2 GL_EXT_texture_compression_rgtc GL_ARB_transform_feedback GL_EXT_framebuffer_sRGBde GL_ARB_pixel_buffer_object GL_EXT_texture_sRGB GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader GL_ARB_shading_language_100 GL_ARB_draw_buffers GL_ARB_texture_non_power_of_two GL_ARB_point_sprite GL_ATI_separate_stencil GL_EXT_stencil_two_side GL_ARB_vertex_buffer_object GL_ARB_occlusion_query GL_EXT_shadow_funcs GL_SGIS_generate_mipmap GL_NV_blend_equare GL_ARB_depth_texture GL_ARB_shadow GL_EXT_fog_coord GL_EXT_multi_draw_arrays GL_ARB_point_parameters GL_EXT_secondary_color GL_EXT_blend_func_separate GL_EXT_stencil_wrap GL_ARB_texture_env_crossbar GL_EXT_texture_lod_bias GL_ARB_texture_mirrored_repeat GL_ARB_window_pos GL_ARB_texture_compression GL_ARB_texture_cube_map ARB_multisample GL_ARB_multitexture GL_ARB_texture_env_add GL_ARB_texture_env_combine GL_ARB_texture_env_dot3 GL_ARB_texture_border_clamp GL_ARB_transpose_matrix EXT_texture3D GL_EXT_bgra GL_EXT_packed_pixels GL_EXT_rescale_normal GL_EXT_separate_specular_color GL_SGIS_texture_edge_clamp GL_SGIS_texture_lod GL_EXT_draw_range_elements GL_EXT_color_table GL_EXT_color_subtable GL_EXT_convolution GL_EXT_convolution_border_modes GL_EXT_color_matrix GL_EXT_histogram GL_EXT_blend_color GL_EXT_blend_minmax GL_EXT_blend_substract GL_EXT_vertex_array GL_EXT_polygon_offset GL_EXT_blend_logic_op GL_EXT_texture GL_EXT_copy_texture GL_EXT_subtexture GL_EXT_texture_object GL_ARB_framebuffer_object GL_ARB_vertex_buffer_object GL_ARB_get_program_binary GL_ARB_separate_shader_objects GL_ARB_ES2_compatibility GL_ARB_shader_precision GL_ARB_vertex_attrib_64_bit GL_ARB_viewport_array GL_ARB_texture_query_lod GL_ARB_gpu_shader5 GL_ARB_gpu_shader_fp64 GL_ARB_shader_subroutine GL_ARB_texture_gather GL_ARB_draw_indirect GL_ARB_sample_shading GL_ARB_tessellation_shader GL_ARB_texture_buffer_object_rgb32 GL_ARB_texture_cube_map_array GL_ARB_transform_feedback2 GL_ARB_transform_feedback3 GL_ARB_draw_buffers_blend GL_ARB_shader_bit_encoding GL_ARB_blend_func_extended GL_ARB_explicit_attrib_location GL_ARB_occlusion_query2 GL_ARB_sampler_objects GL_ARB_texture_rgb10_a2ui GL_ARB_texture_swizzle GL_ARB_timer_query GL_ARB_instanced_arrays GL_ARB_vertex_type_2_10_10_10_rev GL_ARB_framebuffer_object GL_ARB_vertex_buffer_object";
    }
    else
        return glGetString(name);
}

void *betray_gl_proc_address_get_internal(const char *text)
{
#ifdef BETRAY_CONTEXT_OPENGL
    // if(b_win32_opengl_context_current == b_win32_opengl_context)
    {
        uint i;
        char *extension = "glBindFramebuffer";
        for(i = 0; extension[i] == text[i] && extension[i] != 0; i++);
        if(extension[i] == 0)
            return betray_glBindFramebufferEXT;
    }
    {
        uint i;
        char *extension = "glGetString";
        for(i = 0; extension[i] == text[i] && extension[i] != 0; i++);
        if(extension[i] == 0)
            return betray_glGetString;
    }
    return betray_get_proc_address(text);
#endif
    return NULL;
}

void *betray_gl_proc_address_get()
{
    return (void *(*)(const char *))betray_gl_proc_address_get_internal;
}