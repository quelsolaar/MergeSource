/*
 * Betray back-end for native X11. By Emil Brink, who is by no means an
 * experienced Xlib programmer. Comments and improvements very welcome.
*/

#include <stdio.h>
#include <stdlib.h>

#include "betray.h"

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/keysym.h>

#define GLX_GLXEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glx.h>

static struct {
	void		(*context)(void);	/* User-supplied context-change callback function. */
	Display		*dpl;			/* The X11 display connection. */
	Window		win;			/* Our window. */
	GLXContext	ctx;			/* The OpenGL context connected to the window. */
	XVisualInfo	*visual;		/* The visual we use. Not very well-defined. */
	Atom		wmproto, wmdel;		/* Atoms for receiving the WM_DELETE_WINDOW message. */
	boolean		warp;			/* Flag changed by betray_set_mouse_warp(). */
	int		warp_x, warp_y;		/* Center coordinates of window, in window space. */
} x11info;

#if defined __GNUC__
#define	UNUSED(decl)	decl __attribute__((unused))
#else
#define	UNUSED(decl)	decl
#endif

/* ---------------------------------------------------------------------------------------------- */

void betray_set_context_update_func(void (*context_func)(void))
{
	x11info.context = context_func;
}

boolean betray_internal_set_display(uint size_x, uint size_y, UNUSED(boolean full_screen))
{
	betray_reshape_view(size_x, size_y);	/* Just resize, and be done. */
	x11info.warp_x = size_x / 2;
	x11info.warp_y = size_y / 2;

	return TRUE;
}

/* This is an atexit() callback, registered by internal_init_display(), below. Clean-up time. */
static void cb_exit(void)
{
	if(x11info.dpl != NULL)
	{
		XAutoRepeatOn(x11info.dpl);
		if(x11info.win != None)
			XDestroyWindow(x11info.dpl, x11info.win);
		XCloseDisplay(x11info.dpl);
	}
}

boolean betray_internal_init_display(UNUSED(int argc), UNUSED(char **argv), uint size_x, uint size_y, boolean full_screen, const char *caption)
{
	int		attrs[] = { GLX_RGBA, GLX_DOUBLEBUFFER,
					GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8,
					/*GLX_STENCIL_SIZE, 8*/ GLX_DEPTH_SIZE, 8,
					None };
	XTextProperty	prop;

	if((x11info.dpl = XOpenDisplay(NULL)) == NULL)
	{
		fprintf(stderr, "Betray: Unable to open X11 display connection, aborting\n");
		return FALSE;
	}
	if((x11info.win = XCreateSimpleWindow(x11info.dpl, DefaultRootWindow(x11info.dpl),
					  0, 0, size_x, size_y,
					  0,
					  BlackPixel(x11info.dpl, DefaultScreen(x11info.dpl)),
					  BlackPixel(x11info.dpl, DefaultScreen(x11info.dpl)))) == None)
	{
		fprintf(stderr, "Betray: Unable to open X11 window, aborting\n");
		return FALSE;
	}
	XSelectInput(x11info.dpl, x11info.win,
		     ButtonPressMask | ButtonReleaseMask |
		     KeyPressMask | KeyReleaseMask |
		     PointerMotionMask |
		     StructureNotifyMask);

	/* Set the window's caption. */
	XStringListToTextProperty((char **) &caption, 1, &prop);
	XSetWMName(x11info.dpl, x11info.win, &prop);
	/* Enable delete event from window manager. */
	if((x11info.wmproto = XInternAtom(x11info.dpl, "WM_PROTOCOLS", False)) == None)
	{
		fprintf(stderr, "Betray: Unable to look up atom WM_PROTOCOLS\n");
		return FALSE;
	}
	if((x11info.wmdel = XInternAtom(x11info.dpl, "WM_DELETE_WINDOW", False)) != None)
		XSetWMProtocols(x11info.dpl, x11info.win, &x11info.wmdel, 1);

	XMapWindow(x11info.dpl, x11info.win);

	
	/* At this point, we have a window. Now we need to attach a GL rendering context to it. */
	if((x11info.visual = glXChooseVisual(x11info.dpl, DefaultScreen(x11info.dpl), attrs)) == NULL)
	{
		fprintf(stderr, "Betray: Unable to choose a suitable X Visual\n");
		return FALSE;
	}
	if((x11info.ctx = glXCreateContext(x11info.dpl,
					   x11info.visual,
					   NULL,
					   True)) == NULL)
	{
		fprintf(stderr, "Betray: Unable to create X11 rendering context\n");
		return FALSE;
	}
	if(glXMakeCurrent(x11info.dpl, x11info.win, x11info.ctx) == False)
	{
		fprintf(stderr, "Betray: Unable to make X11 window current\n");
		return FALSE;
	}
	betray_internal_set_display(size_x, size_y, full_screen);
	atexit(cb_exit);
	XAutoRepeatOff(x11info.dpl);	/* This is needed to get sensible keyboard events. */

	return TRUE;
}

void * betray_get_gl_proc_address(void)
{
#if defined GLX_ARB_get_proc_address
	return glXGetProcAddressARB;
#else
	return glXGetProcAddress;
#endif
}

void betray_set_mouse_warp(boolean warp)
{
	x11info.warp = warp;
}

static int button_map(int button)
{
	static const int x2betray[] = { 0, 2, 1 };

	return x2betray[button];
}

/* Grab events for this window from the X11 event queue, and handle them.
 * Returns 0 if we should quit, 1 to keep running.
*/
static int x11_event_handler(BInputState *input)
{
	XEvent	ev;

	/* Flush out any ClientMessage events, looking for WM_DELETE_WINDOW client message. We can't
	 * do this below, since XCheckWindowEvent() won't return non-masked types like ClientMessage.
	*/
	while(XCheckTypedEvent(x11info.dpl, ClientMessage, &ev))
	{
		if(ev.xclient.window == x11info.win &&
		   ev.xclient.message_type == x11info.wmproto &&
		   ev.xclient.format == 32 &&
		   (Atom) ev.xclient.data.l[0] == x11info.wmdel)
			return 0;
	}

	if(x11info.warp)
		input->pointer_x = input->pointer_y = 0.0f;

	while(XCheckWindowEvent(x11info.dpl, x11info.win,
				~0l,
				&ev))
	{
		switch(ev.type)
		{
		case ButtonPress:
			if(ev.xbutton.button >= 1 && ev.xbutton.button <= 3)
				input->mouse_button[button_map(ev.xbutton.button - 1)] = TRUE;
			break;
		case ButtonRelease:
			if(ev.xbutton.button >= 1 && ev.xbutton.button <= 3)
				input->mouse_button[button_map(ev.xbutton.button - 1)] = FALSE;
			break;
		case KeyPress:
			{
				char	buf[32];
				KeySym	sym;

				XLookupString(&ev.xkey, buf, sizeof buf, &sym, NULL);
				if(betray_is_type_in())
				{
					if(sym == XK_Return || sym == XK_KP_Enter)
						betray_end_type_in_mode(FALSE);
					else if(sym == XK_Escape)
						betray_end_type_in_mode(TRUE);
					else if(sym == XK_Delete)
					{
						betray_move_cursor(1);
						betray_delete_character();
					}
					else if(sym == XK_BackSpace)
						betray_delete_character();
					else if(sym == XK_Left)
						betray_move_cursor(-1);
					else if(sym == XK_Right)
						betray_move_cursor(1);
					else if(sym == XK_End)
						betray_move_cursor(32000);
					else if(sym == XK_Home)
						betray_move_cursor(-32000);
					else if(buf[0] >= ' ' && buf[1] == '\0')
						betray_insert_character(buf[0]);
				}
				else
					betray_internal_key_add(input, sym, TRUE);
			}
			break;
		case KeyRelease:
			if(!betray_is_type_in())
			{
				KeySym	sym = XLookupKeysym(&ev.xkey, 0);

				if(ev.xkey.state & (LockMask | ShiftMask))
					XConvertCase(sym, &sym, &sym);
				betray_internal_key_add(input, sym, FALSE);
			}
			break;
		case MotionNotify:
			{
				uint	w, h;

				/* In in warp-mode, disregard events from warp spot. Otherwise, we get plenty of
				 * false, negative, deltas after any movement, from the warp call. Badness.
				*/
				if(x11info.warp && ev.xmotion.x == x11info.warp_x && ev.xmotion.y == x11info.warp_y)
					break;
				/* Map the coordinates into the ranges used by Betray. This code is more
				 * or less directly lifted from the corresponding handler in the SDL backend.
				*/
				betray_get_screen_mode(&w, &h, NULL);
				input->delta_pointer_x = -input->pointer_x;
				input->pointer_x = (float) ev.xmotion.x / (float) w * 2.0f - 1.0f;
				input->delta_pointer_x += input->pointer_x;

				input->delta_pointer_y = -input->pointer_y;
				input->pointer_y = ((int) h / 2 - ev.xmotion.y) / (float) h * 2.0f * ((float) h / (float) w);
				input->delta_pointer_y += input->pointer_y;
/*				printf(" mouse at %d,%d -> (%g,%g), delta (%g,%g)\n", ev.xmotion.x, ev.xmotion.y,
				       input->pointer_x, input->pointer_y,
				       input->delta_pointer_x, input->delta_pointer_y);
*/			}
			break;
		case ConfigureNotify:
			betray_internal_set_display(ev.xconfigure.width, ev.xconfigure.height, FALSE);
			if(x11info.context != NULL)	/* If user has a context function registered, call it. */
				x11info.context();
			break;
		}
	}
	if(x11info.warp)
		XWarpPointer(x11info.dpl, x11info.win, x11info.win, 0, 0, 0, 0, x11info.warp_x, x11info.warp_y);
	return 1;
}

void betray_launch_main_loop(void)
{
	BInputState	*input;

	input = betray_get_input_state();

	while(x11_event_handler(input))
	{
		if(input->mouse_button[0] == FALSE && input->last_mouse_button[0] == FALSE)
		{
			input->click_pointer_x = input->pointer_x;
			input->click_pointer_y = input->pointer_y;
		}
		betray_time_update();
		betray_action(BAM_EVENT);
		betray_action(BAM_DRAW);
		input->delta_pointer_x = input->delta_pointer_y = 0;
		input->event_count = 0;
		glXSwapBuffers(x11info.dpl, x11info.win);
		input->time++;
		input->last_mouse_button[0] = input->mouse_button[0];
		input->last_mouse_button[1] = input->mouse_button[1];
		input->last_mouse_button[2] = input->mouse_button[2];
		betray_action(BAM_MAIN);
	}
	/* We don't bother with closing the window, the exit handler does that. */
}

#endif	/* BETRAY_X11_SYSTEM_WRAPPER */
