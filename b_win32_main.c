#ifdef _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "betray.h"
#include "imagine.h"


#include "Shobjidl.h"

#include <io.h>

static boolean	input_focus = TRUE;
static int	b_window_size_x = 800;
static int	b_window_size_y = 600;
static int	b_window_center_x = 400;
static int	b_window_center_y = 300;
static int	b_window_pos_x = 800;
static int	b_window_pos_y = 600;

#ifdef BETRAY_CONTEXT_OPENGLES

EGLDisplay	sEGLDisplay;
EGLContext	sEGLContext;
EGLSurface	sEGLSurface;

#endif

static boolean	mouse_warp = FALSE;
static float	mouse_warp_move_x;
static float	mouse_warp_move_y;
static boolean	mouse_warp_move = FALSE;
static boolean	mouse_hide = FALSE;
static boolean	mouse_inside = FALSE;
static boolean	window_close = FALSE;
static boolean	window_minimized = FALSE;

static uint betray_user_id = -1;
static uint betray_device_mouse_id = -1;
static uint betray_device_keyboard_id = -1;
static uint betray_device_touch_id = -1;
static uint betray_mouse_id = -1;

HDC hDC;				/* device context */
HWND hWnd;				/* window */

#ifndef  WM_UNICHAR
#define WM_UNICHAR                      0x0109
#endif
extern void betray_plugin_callback_main(BInputState *input);
extern boolean betray_plugin_callback_event_pump(BInputState *input, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void betray_plugin_pointer_clean();
extern void betray_action(BActionMode mode);
extern void betray_reshape_view(uint x_size, uint y_size);

extern uint	betray_plugin_user_allocate(void);
extern uint betray_plugin_input_device_allocate(uint user_id, char *name);
extern uint betray_plugin_pointer_allocate(uint user_id, uint device_id, uint button_count, float x, float y, float z, float *origin, char *name, boolean draw);
extern void betray_plugin_pointer_set(uint id, float x, float y, float z, float *origin, boolean *buttons);
extern void betray_plugin_pointer_free(uint id);
extern void betray_plugin_button_set(uint user_id, uint id, boolean state, uint character);
extern void betray_plugin_button_release_all(BInputState *input);

void betray_reset_path(void)
{
	TCHAR path[MAX_PATH + 1] = {0};
	uint i;
	return;
	GetModuleFileName(0, path, MAX_PATH + 1);
	for(i = 0; path[i] != 0; i++);
	for(; i != 0 && path[i] != '\\' && path[i] != '\\'; i--)
		path[i] = 0;
	if(path[0] && SetCurrentDirectory(path))
	{	
		path[0] = 0;
	}else
		path[0] = 0;
}

void betray_timer_debug(char *text)
{
	static uint new_seconds = 0 , new_fractions = 0, old_seconds =0 , old_fractions = 0;
	imagine_current_time_get(&new_seconds, &new_fractions);
	printf("timer = %f %s\n", (float)imagine_delta_time_compute(new_seconds, new_fractions, old_seconds, old_fractions), text);
	old_seconds = new_seconds; 
	old_fractions = new_fractions;
}

boolean b_win32_system_wrapper_set_display(uint size_x, uint size_y, boolean full_screen) 
{  
	return TRUE;
}

uint betray_support_functionality(BSupportedFunctionality funtionality)
{
	uint array[] = {
				256, /*B_SF_USER_COUNT_MAX*/
				256, /*B_SF_POINTER_COUNT_MAX*/
				3, /*B_SF_POINTER_BUTTON_COUNT*/
				TRUE, /*B_SF_FULLSCREEN*/
				TRUE, /*B_SF_WINDOWED*/
				FALSE, /*B_SF_VIEW_POSITION*/
				FALSE, /*B_SF_VIEW_ROTATION*/
				TRUE, /*B_SF_MOUSE_WARP*/
				TRUE, /*B_SF_EXECUTE*/
				TRUE, /*B_SF_REQUESTER*/
				TRUE}; /*B_SF_CLIPBOARD*/
	if(funtionality >= B_SF_COUNT)
		return FALSE;
	return array[funtionality];
}


void *b_win32_requester_mutex = NULL;
char b_win32_requester_load[MAX_PATH] = {0};
char b_win32_requester_save[MAX_PATH] = {0};
void *b_win32_requester_load_id = NULL;
void *b_win32_requester_save_id = NULL;

typedef struct{
	void *id;
	uint type_count;
#ifdef  UNICODE
	short types[1];
#else
	char types[1];
#endif
}BetrayRequesterParam;

BetrayRequesterParam *betray_requester_string_build(char **types, uint type_count, void *id)
{
	BetrayRequesterParam *param;
	uint i, j, length;
#ifdef  UNICODE	
	short *output, *p;
#else
	char *output, *p;
#endif
	char *suported = "Supported files";
	char *all = "All files\0*.*\0\0";
	for(i = length = 0; i < type_count; i++)
		for(j = 0; types[i][j] != 0; j++)
			length++;
	length *= 2; // double.	
	if(type_count > 1)
	{
		for(i = 0; suported[i] != 0; i++);
		length += i + 1 +  // add space for supported
					length / 2 +  // add space for types
					type_count * 2 + // add space for *.
					type_count;  // add space ; separators and a termination
	}
	length += type_count * 4; // add termnination and *.
	length += 15; // size of all
	param = malloc((sizeof *param) + (sizeof *param->types) * (length - 1));
	output = p = param->types;
	param->type_count = type_count;
	param->id = id;
	if(type_count > 1)
	{
		for(i = 0; suported[i] != 0; i++)
			*p++ = suported[i];
		*p++ = 0;
		for(i = 0; i < type_count;)
		{		
			*p++ = '*';
			*p++ = '.';
			for(j = 0; types[i][j] != 0; j++)
				*p++ = types[i][j];
			if(++i < type_count)
				*p++ = ';';
		}
		*p++ = 0;
	}
	for(i = 0; i < type_count; i++)
	{		
		if(types[i][0] >= 'a' && types[i][0] <= 'z')
			*p++ = 'A' + types[i][0] - 'a';
		else
			*p++ = types[i][0];			
		for(j = 1; types[i][j] != 0; j++)
			*p++ = types[i][j];
		*p++ = 0;
		*p++ = '*';
		*p++ = '.';
		for(j = 0; types[i][j] != 0; j++)
			*p++ = types[i][j];
		*p++ = 0;
	}
	for(i = 0; i < 15; i++)	
		*p++ = all[i];
	*p++ = 0;
	return param;
}

void betray_requester_save_func(void *data)
{
	BetrayRequesterParam *param; 
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];
	uint i, j, k;
	param = data;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.hwndOwner = hWnd;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = param->types;
	ofn.nFilterIndex = 1;

	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if(GetSaveFileName(&ofn))
	{
		imagine_mutex_lock(b_win32_requester_mutex);
		for(i = 0; ofn.lpstrFile[i] != 0; i++)
			b_win32_requester_save[i] = ofn.lpstrFile[i];
		b_win32_requester_save[i] = 0;
		ofn.nFilterIndex--;
		if(ofn.nFilterIndex < param->type_count)
		{
			for(j = i; j > 0 && b_win32_requester_save[j] != '.' && b_win32_requester_save[j] != '/' && b_win32_requester_save[j] != '\\'; j--);
			if(b_win32_requester_save[j] != '.')
			{
				for(j = k = 0; j < ofn.nFilterIndex * 2 + 1; j++)
				{
					for(; param->types[k] != 0; k++);
					k++;
				}
				k += 2;
				b_win32_requester_save[i++] = '.';
				while(param->types[k] != 0)
					b_win32_requester_save[i++] = param->types[k++];
				b_win32_requester_save[i] = 0;
			}
		}
		b_win32_requester_save_id = param->id;
		imagine_mutex_unlock(b_win32_requester_mutex);
	}
	free(param);
	betray_reset_path();
}

char *betray_requester_save_get(void *id)
{
	uint i;
	char *buf;
	if(b_win32_requester_mutex == NULL)
		b_win32_requester_mutex = imagine_mutex_create();
	imagine_mutex_lock(b_win32_requester_mutex);
	if(b_win32_requester_save[0] == 0 || b_win32_requester_save_id != id)
	{
		imagine_mutex_unlock(b_win32_requester_mutex);
		return NULL;
	}
	for(i = 0; b_win32_requester_save[i] != 0; i++);
	buf = malloc((sizeof *buf) * (i + 1));
	for(i = 0; b_win32_requester_save[i] != 0; i++)
		buf[i] = b_win32_requester_save[i];
	buf[i] = 0;
	b_win32_requester_save[0] = 0;
	imagine_mutex_unlock(b_win32_requester_mutex);
	b_win32_requester_save_id = NULL;
	return buf;
}


void betray_requester_save(char **types, uint type_count, void *id)
{
	if(b_win32_requester_mutex == NULL)
		b_win32_requester_mutex = imagine_mutex_create();
	imagine_thread(betray_requester_save_func, betray_requester_string_build(types, type_count, id), "Betray save requester");
}


void betray_requester_load_func(void *data)
{
	BetrayRequesterParam *param; 
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH];
	uint i;
	param = data;
	if(b_win32_requester_mutex == NULL)
		b_win32_requester_mutex = imagine_mutex_create();
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.hwndOwner = hWnd;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = param->types;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if(GetOpenFileName(&ofn))
	{
		imagine_mutex_lock(b_win32_requester_mutex);
		for(i = 0; ofn.lpstrFile[i] != 0 && i <  MAX_PATH - 1; i++)
			b_win32_requester_load[i] = ofn.lpstrFile[i];
		b_win32_requester_load[i] = 0;
		b_win32_requester_load_id = param->id;
		imagine_mutex_unlock(b_win32_requester_mutex);
	}
	free(param);
	betray_reset_path();
}

char *betray_requester_load_get(void *id)
{
	uint i;
	char *buf;
	if(b_win32_requester_load_id != id)
		return NULL;
	if(b_win32_requester_mutex == NULL)
		b_win32_requester_mutex = imagine_mutex_create();
	imagine_mutex_lock(b_win32_requester_mutex);
	if(b_win32_requester_load[0] == 0 || b_win32_requester_load_id != id)
	{
		imagine_mutex_unlock(b_win32_requester_mutex);
		return NULL;
	}	
	for(i = 0; b_win32_requester_load[i] != 0; i++);
	buf = malloc((sizeof *buf) * (i + 1));
	for(i = 0; b_win32_requester_load[i] != 0; i++)
	{
		if(b_win32_requester_load[i] == 92)
			buf[i] = 47;
		else
			buf[i] = b_win32_requester_load[i];
	}
	buf[i] = 0;
	b_win32_requester_load[0] = 0;
	imagine_mutex_unlock(b_win32_requester_mutex);
	b_win32_requester_load_id = NULL;
	return buf;
}

void betray_requester_load(char **types, uint type_count, void *id)
{
	if(b_win32_requester_mutex == NULL)
		b_win32_requester_mutex = imagine_mutex_create();
	imagine_thread(betray_requester_load_func, betray_requester_string_build(types, type_count, id), "Betray load requester");
}

char *betray_clipboard_get()
{
	if(OpenClipboard(NULL)) 
	{
		HANDLE hClipboardData;
		char *text, *buf;
		uint length;
		hClipboardData = GetClipboardData(CF_TEXT);
		if(hClipboardData == NULL)
			return NULL;
		text = (char*)GlobalLock(hClipboardData);
		if(text == NULL)
			return NULL;
		for(length = 0; text[length] != 0; length++);
		length++;
		buf = malloc((sizeof *buf) *length);
		for(length = 0; text[length] != 0; length++)
			buf[length] = text[length];
		buf[length] = 0;
		GlobalUnlock(hClipboardData);
		CloseClipboard();
		return buf;
	}
	return NULL;
}

void betray_clipboard_set(char *text)
{
	if(OpenClipboard(NULL))
	{
		HGLOBAL clipboard_data;
		uint length;
		char *copy;
		for(length = 0; text[length] != 0; length++);
		length++;
		EmptyClipboard();
		clipboard_data = GlobalAlloc(GMEM_DDESHARE, length);
		copy = (char*)GlobalLock(clipboard_data);
		for(length = 0; text[length] != 0; length++)
			copy[length] = text[length];
		copy[length] = 0; 
		GlobalUnlock(clipboard_data);
		SetClipboardData(CF_TEXT, clipboard_data);
		CloseClipboard();
	}
}

void betray_url_launch_func(uint16 *url)
{
	uint16 open[5] = {'o', 'p', 'e', 'n', 0};	
	ShellExecute(hWnd, open, url, NULL, NULL, TRUE);
	free(url);
}
void betray_url_launch(char *url)
{
	uint16 *unicode, open[5] = {'o', 'p', 'e', 'n', 0};
	uint i;
	for(i = 0; url[i] != 0; i++);
	unicode = malloc((sizeof *unicode) * (i + 1));
	for(i = 0; url[i] != 0; i++)
		unicode[i] = url[i];
	unicode[i] = 0;
	imagine_thread(betray_url_launch_func, unicode, "Betray URL launch");
}

extern void betray_time_update(void);


/*
 * Touch Input defines and functions
 */

/*
 * Touch input handle
 */
#define BETRAY_WM_TOUCH                        0x0240

typedef struct{
    LONG x;
    LONG y;
    HANDLE hSource;
    DWORD dwID;
    DWORD dwFlags;
    DWORD dwMask;
    DWORD dwTime;
	void *dwExtraInfo;
    DWORD cxContact;
    DWORD cyContact;
}BetrayWinTouch;


/*
 * Conversion of touch input coordinates to pixels
 */
#define BETRAY_TOUCH_COORD_TO_PIXEL(l)         ((l) / 100)

/*
 * Touch input flag values (TOUCHINPUT.dwFlags)
 */
#define BETRAY_TOUCHEVENTF_MOVE            0x0001
#define BETRAY_TOUCHEVENTF_DOWN            0x0002
#define BETRAY_TOUCHEVENTF_UP              0x0004
#define BETRAY_TOUCHEVENTF_INRANGE         0x0008
#define BETRAY_TOUCHEVENTF_PRIMARY         0x0010
#define BETRAY_TOUCHEVENTF_NOCOALESCE      0x0020
#define BETRAY_TOUCHEVENTF_PEN             0x0040
#define BETRAY_TOUCHEVENTF_PALM            0x0080

#define BETRAY_TWF_FINETOUCH       (0x00000001)
#define BETRAY_TWF_WANTPALM        (0x00000002)
/*
 * Touch input mask values (TOUCHINPUT.dwMask)
 */
#define BETRAY_TOUCHINPUTMASKF_TIMEFROMSYSTEM  0x0001  // the dwTime field contains a system generated value
#define BETRAY_TOUCHINPUTMASKF_EXTRAINFO       0x0002  // the dwExtraInfo field is valid
#define BETRAY_TOUCHINPUTMASKF_CONTACTAREA     0x0004  // the cxContact and cyContact fields are valid

BOOL (WINAPI *betray_GetTouchInputInfo)(LPARAM hTouchInput, uint cInputs, BetrayWinTouch *pInputs, int cbSize) = NULL;

BOOL (WINAPI *betray_CloseTouchInputHandle)(LPARAM hTouchInput) = NULL;

BOOL (WINAPI *betray_RegisterTouchWindow)(HWND hWnd, ULONG ulFlags) = NULL;
/*
BOOL WINAPI RegisterTouchWindow(
  _In_  HWND hWnd,
  _In_  ULONG ulFlags
);
*/
/*
WINUSERAPI BOOL WINAPI GetTouchInputInfo(
    LPARAM hTouchInput,
    uint cInputs,
    WinTouch *pInputs,
    int cbSize);

WINUSERAPI
BOOL
WINAPI
CloseTouchInputHandle( LPARAM hTouchInput);
*/

typedef struct{
	uint hardware_device;
	uint device_id;
	uint pointer_id;
}BetrayRawPointer;

LONG WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
	static BetrayRawPointer raw_pointers[16];
    static PAINTSTRUCT ps;
    static int omx, omy, mx, my, raw_pointer_count;
	static boolean control = FALSE, shift = FALSE;
	BInputState *input;
	input = betray_get_input_state();
	if(betray_plugin_callback_event_pump(input, hWnd, uMsg, wParam, lParam))
		 return DefWindowProc(hWnd, uMsg, wParam, lParam);

    switch(uMsg)
	{
		case WM_SYSCOMMAND :
		    if((wParam & 0xFFF0) == SC_MINIMIZE)
				window_minimized = TRUE;
			 if((wParam & 0xFFF0) == SC_RESTORE)
				window_minimized = FALSE;
			break;
		
		case WM_INPUT: 
	/*	{
			uint dwSize;
			uint8 *lpb;
			RAWINPUT *raw;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, 
							sizeof(RAWINPUTHEADER));	
			raw = malloc(dwSize);
			if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize); 
			{
				if(raw->header.dwType == RIM_TYPEMOUSE) 
				{
					uint i;
					for(i = 0; i < raw_pointer_count && raw_pointers[i].hardware_device != raw->header.hDevice; i++);
					if(i == raw_pointer_count)
					{
						if(i == 16)
							return;
						raw_pointers[i].hardware_device = raw->header.hDevice;
						if(i != 0)
						{
							raw_pointers[i].device_id = betray_plugin_input_device_allocate(betray_user_id, "Pointer");
							raw_pointers[i].pointer_id = betray_plugin_pointer_allocate(betray_user_id, betray_device_mouse_id, 16, 0.0, 0.0, "mouse");
						}else
						{
							raw_pointers[i].device_id = betray_device_mouse_id;
							raw_pointers[i].pointer_id = betray_mouse_id;
						}
						raw_pointer_count++;
					}
					i = raw_pointers[i].pointer_id;

					input->pointers[i].pointer_x += (float)raw->data.mouse.lLastX / (float)b_window_size_x;
					input->pointers[i].pointer_y -= (float)raw->data.mouse.lLastY / (float)b_window_size_x;		
					if(input->pointers[i].pointer_x > 1.0)
						input->pointers[i].pointer_x = 1.0;		
					if(input->pointers[i].pointer_x < -1.0)
						input->pointers[i].pointer_x = -1.0;	
					if(input->pointers[i].pointer_y > (float)b_window_size_y / (float)b_window_size_x)
						input->pointers[i].pointer_y = (float)b_window_size_y / (float)b_window_size_x;		
					if(input->pointers[i].pointer_y < -(float)b_window_size_y / (float)b_window_size_x)
						input->pointers[i].pointer_y = -(float)b_window_size_y / (float)b_window_size_x;
					if(raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
						input->pointers[i].button[0] = TRUE;	
					if(raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
						input->pointers[i].button[0] = FALSE;	
					if(raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
						input->pointers[i].button[1] = TRUE;	
					if(raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
						input->pointers[i].button[1] = FALSE;	
					if(raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
						input->pointers[i].button[2] = TRUE;	
					if(raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
						input->pointers[i].button[2] = FALSE;
				}
			}
			free(raw);
		}*/
		return 0;
		case WM_SIZE :
			b_window_size_x = LOWORD(lParam);
			b_window_size_y = HIWORD(lParam);
			betray_reshape_view(b_window_size_x, b_window_size_y);
		return 0;
		break;
		case WM_MOVE :
			b_window_pos_x = (int)(short) LOWORD(lParam);   // horizontal position 
			b_window_pos_y = (int)(short) HIWORD(lParam);   // vertical position
		return 0;
		break;
	    case WM_LBUTTONDOWN :
	    case WM_LBUTTONUP :
	    case WM_RBUTTONDOWN :
	    case WM_RBUTTONUP :
	    case WM_MBUTTONDOWN :
	    case WM_MBUTTONUP :
	    case WM_MOUSEMOVE :
		case WM_NCRBUTTONUP :
		case WM_NCLBUTTONUP :
		case WM_NCMBUTTONUP :
//		case WM_XBUTTONUP :
//		case WM_XBUTTONDOWN :
/*		input->pointers[0].pointer_x = (float)(0x0000FFFF | lParam) / (float)b_window_size_x; 
		input->pointers[0].pointer_y = (float)((0xFFFF0000 | lParam) / 0x0000FFFF) / (float)b_window_size_y; 
*/	

		if(LOWORD(lParam) < b_window_size_x)
			input->pointers[0].pointer_x = (float)((signed short)LOWORD(lParam)) / (float)b_window_size_x * 2.0 - 1.0;
		if(HIWORD(lParam) < b_window_size_y)
			input->pointers[0].pointer_y = (1.0 - (float)((signed short)HIWORD(lParam)) / (float)b_window_size_y * 2.0) * (float)b_window_size_y / (float)b_window_size_x;
		if((GetMessageExtraInfo() & 0xFFFFFF00) != 0xFF515700)
		{
			if(uMsg == WM_LBUTTONDOWN)
				input->pointers[0].button[0] = TRUE;
			if(uMsg == WM_LBUTTONUP || uMsg == WM_NCLBUTTONUP)
				input->pointers[0].button[0] = FALSE;
			if(uMsg == WM_RBUTTONDOWN)
			{
				input->pointers[0].button[1] = TRUE;
			}if(uMsg == WM_RBUTTONUP || uMsg == WM_NCRBUTTONUP)
				input->pointers[0].button[1] = FALSE;
			if(uMsg == WM_MBUTTONDOWN)
				input->pointers[0].button[2] = TRUE;
			if(uMsg == WM_MBUTTONUP || uMsg == WM_NCMBUTTONUP)
				input->pointers[0].button[2] = FALSE;
/*			if(uMsg == WM_XBUTTONDOWN)
				input->pointers[0].button[3] = TRUE;
			if(uMsg == WM_XBUTTONUP)
				input->pointers[0].button[3] = FALSE;*/

			if(input->pointers[0].button[0] ||
				input->pointers[0].button[1] ||
				input->pointers[0].button[2])
				 SetCapture(hWnd);
			if(!input->pointers[0].button[0] &&
				!input->pointers[0].button[1] &&
				!input->pointers[0].button[2])
				ReleaseCapture();
		}		
		return 0;
		break;
		case /*WM_MOUSEWHEEL*/ 0x020A :
		{
			short dir;
			dir = wParam / (256 * 256);
			if(dir < 0)
				betray_plugin_button_set(betray_user_id, BETRAY_BUTTON_SCROLL_UP, TRUE, -1);
			if(dir > 0)
				betray_plugin_button_set(betray_user_id, BETRAY_BUTTON_SCROLL_DOWN, TRUE, -1);
			return 0;
		}
		case WM_SYSKEYDOWN :
			betray_plugin_button_set(betray_user_id, wParam, TRUE, lParam);
			return 0;
			break;
		case WM_KEYDOWN :
			if(wParam == VK_CONTROL)
				control = TRUE;
			if(wParam == VK_SHIFT)
				shift = TRUE;
			if(control && shift && wParam == 'Z')
			{
				betray_plugin_button_set(betray_user_id, BETRAY_BUTTON_REDO, TRUE, -1);
				betray_plugin_button_set(betray_user_id, wParam, TRUE, -1);
			}else if(control && wParam == 'Z')
			{
				betray_plugin_button_set(betray_user_id, BETRAY_BUTTON_UNDO, TRUE, -1);
				betray_plugin_button_set(betray_user_id, wParam, TRUE, -1);
			}else if(control && wParam == 'X')
			{
				betray_plugin_button_set(betray_user_id, BETRAY_BUTTON_CUT, TRUE, -1);
				betray_plugin_button_set(betray_user_id, wParam, TRUE, -1);
			}else if(control && wParam == 'C')
			{
				betray_plugin_button_set(betray_user_id, BETRAY_BUTTON_COPY, TRUE, -1);
				betray_plugin_button_set(betray_user_id, wParam, TRUE, -1);
			}else if(control && wParam == 'V')
			{
				printf("BETRAY_BUTTON_PASTE\n");
				betray_plugin_button_set(betray_user_id, BETRAY_BUTTON_PASTE, TRUE, -1);
				betray_plugin_button_set(betray_user_id, wParam, TRUE, -1);
			}else
				betray_plugin_button_set(betray_user_id, wParam, TRUE, lParam);
			return 0;
			break;
		case WM_CHAR :
		case WM_UNICHAR :
			betray_plugin_button_set(betray_user_id, -1, TRUE, wParam);
			return 0;
			break;
		case WM_SYSKEYUP:
			betray_plugin_button_set(betray_user_id, wParam, FALSE, lParam);
			return 0;
			break;
		case WM_KEYUP :
			if(wParam == VK_CONTROL)
				control = FALSE;
			if(wParam == VK_SHIFT)
				shift = FALSE;
			betray_plugin_button_set(betray_user_id, wParam, FALSE, lParam);
			return 0;
			break;
		case WM_KILLFOCUS :
				betray_plugin_button_release_all(input);	
			break;
		case WM_SETFOCUS :
			break;			
		case WM_SHOWWINDOW :
			window_minimized = !wParam;	
			if(window_minimized)
				betray_plugin_button_release_all(input);	
			return DefWindowProc(hWnd, uMsg, wParam, lParam); 
			break;
		case WM_PAINT :	
			break;
		case BETRAY_WM_TOUCH :
			if(betray_GetTouchInputInfo != NULL && betray_CloseTouchInputHandle != NULL)
			{
				static unsigned int *ids = NULL;
				unsigned int i, j, numInputs = (int) wParam; //Number of actual contact messages
				BetrayWinTouch *ti;
				boolean button;
				float x, y;

				if(ids == NULL)
				{
					betray_device_touch_id = betray_plugin_input_device_allocate(betray_user_id, "Touch");
					ids = malloc((sizeof *ids) * betray_support_functionality(B_SF_POINTER_COUNT_MAX));
					for(i = 0; i < betray_support_functionality(B_SF_POINTER_COUNT_MAX); i++)
						ids[i] = -1;
				}

				ti = malloc((sizeof *ti) * wParam);
				if(betray_GetTouchInputInfo(lParam, (int)wParam, ti, sizeof *ti))
				{
					for(i = 0; i < wParam; i++)
					{
					//	if((ti[i].dwFlags & BETRAY_TOUCHEVENTF_PRIMARY))
						{
							x = ((float)ti[i].x / 100.0) / (float)b_window_size_x * 2.0 - 1.0;
							y = (1.0 - ((float)ti[i].y / 100.0) / (float)b_window_size_y * 2.0) * (float)b_window_size_y / (float)b_window_size_x;
							if(ti[i].dwFlags & BETRAY_TOUCHEVENTF_DOWN)
							{
								j = betray_plugin_pointer_allocate(betray_user_id, betray_device_touch_id, 1, x, y, -1, NULL, "Touch", FALSE);
								ids[j] = ti[i].dwID;
								button = TRUE;
								
								betray_plugin_pointer_set(j, x, y, -1, NULL, &button);
							}
							if(ti[i].dwFlags & BETRAY_TOUCHEVENTF_MOVE)
							{	
								for(j = 1; j < betray_support_functionality(B_SF_POINTER_COUNT_MAX) && ids[j] != ti[i].dwID; j++);
								if(j < betray_support_functionality(B_SF_POINTER_COUNT_MAX))
								{
									button = TRUE;
									betray_plugin_pointer_set(j, x, y, -1, NULL, &button);
								}
							}
							if(ti[i].dwFlags & BETRAY_TOUCHEVENTF_UP)
							{
								for(j = 1; j < betray_support_functionality(B_SF_POINTER_COUNT_MAX) && ids[j] != ti[i].dwID; j++);
								if(j < betray_support_functionality(B_SF_POINTER_COUNT_MAX))
								{
									button = FALSE;
									betray_plugin_pointer_set(j, x, y, -1, NULL, &button);
									betray_plugin_pointer_free(j);
									ids[j] = -1;
								}
							}
						}
					}
				}
				betray_CloseTouchInputHandle(lParam);
				free(ti);
			}
			break;
	    case WM_CLOSE :
			window_close = TRUE;
			break;
		default :
	//		printf("not caught message %u\n", uMsg);
			break;
	}
    return DefWindowProc(hWnd, uMsg, wParam, lParam); 
}




void betray_button_keyboard(uint user_id, boolean show)
{
}


static HGLRC b_win32_opengl_context = NULL;
static HGLRC b_win32_opengl_context_current = NULL;
static HINSTANCE b_win32_instance = NULL;
static uint	b_win32_display_size_x = 0;
static uint	b_win32_display_size_y = 0;


HDC hDC;				/* device context */
HWND hWnd;				/* window */

HWND  betray_plugin_windows_window_handle_get()
{
	return hWnd;
}

HDC  betray_plugin_windows_device_context_handle_get()
{
	return hDC;
}

void betray_set_icon()
{
	uint8 pixels[32 * 32 * 3], mask[32 * 32 * 3];
	HICON hIcon;
	uint i;
	for(i = 0; i < 32 * 32 * 3; i++)
		pixels[i] = 0;
	for(i = 0; i < 32 * 32 * 3; i++)
		mask[i] = 255;
	hIcon = CreateIcon(NULL, 32, 32, 1, 24, mask, pixels);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, hIcon);

    //This will ensure that the application icon gets changed too.
    SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, hIcon);
    SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG, hIcon);
}

void b_win32_window_close()
{
	uint i;
	for(i = 0; i < 2; i++)
	{
	//	if(b_win32_opengl_context_current)
		{
#ifdef BETRAY_CONTEXT_OPENGL
		/*	if(B_CT_OPENGL == betray_context_type_get())
			{
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(b_win32_opengl_context[i]);
			}*/
#endif
#ifdef BETRAY_CONTEXT_OPENGLES
			if(B_CT_OPENGLES2 == betray_context_type_get())
			{
				eglMakeCurrent(sEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
				eglDestroySurface(sEGLDisplay, sEGLSurface);
				eglDestroyContext(sEGLDisplay, sEGLContext);
				eglTerminate(sEGLDisplay);
				break;
			}
#endif
		}
	}
	if(hDC && ReleaseDC(hWnd, hDC))
		hDC = NULL;
	if(hWnd && DestroyWindow(hWnd))
		hWnd = NULL;
	if(UnregisterClass("OpenGL", b_win32_instance))
		b_win32_instance = NULL;
}

void betray_desktop_size_get(uint *size_x, uint *size_y) 
{
	if(b_win32_display_size_x == 0)
	{
		b_win32_display_size_x = GetSystemMetrics(SM_CXSCREEN);
		b_win32_display_size_y = GetSystemMetrics(SM_CYSCREEN);
	}
	if(size_x != NULL)
		*size_x = b_win32_display_size_x;
	if(size_y != NULL)
		*size_y = b_win32_display_size_y;
}

boolean b_win32_screen_mode(uint size_x, uint size_y) 
{
	DEVMODE sreen_sttings;
	memset(&sreen_sttings, 0, sizeof(sreen_sttings));
	sreen_sttings.dmSize = sizeof(sreen_sttings);
	sreen_sttings.dmPelsWidth = size_x;
	sreen_sttings.dmPelsHeight = size_y;
	sreen_sttings.dmBitsPerPel = 32;
	sreen_sttings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	return ChangeDisplaySettings(&sreen_sttings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
}



#define	WGL_DRAW_TO_WINDOW_ARB		0x2001
#define	WGL_SUPPORT_OPENGL_ARB		0x2010
#define	WGL_DOUBLE_BUFFER_ARB		0x2011
#define	WGL_PIXEL_TYPE_ARB			0x2013
#define	WGL_TYPE_RGBA_ARB			0x202B
#define	WGL_COLOR_BITS_ARB			0x2014
#define	WGL_DEPTH_BITS_ARB			0x2022
#define	WGL_STENCIL_BITS_ARB		0x2023
#define	WGL_STEREO_ARB				0x2012
#define	WGL_SAMPLE_BUFFERS_ARB		0x2041
#define	WGL_SAMPLES_ARB				0x2042
 
boolean betray_activate_context(void *context)
{
//	glFinish();
	if(context == NULL)
		context = b_win32_opengl_context;
	if(b_win32_opengl_context_current == context)
		return FALSE;
	b_win32_opengl_context_current = context;
	return wglMakeCurrent(hDC, context);
}

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091 
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092 
#define WGL_CONTEXT_DEBUG_BIT_ARB     0x0001
#define WGL_CONTEXT_FLAGS_ARB         0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126 
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

void *b_create_context() 
{
	HGLRC (APIENTRY *betray_wglCreateContextAttribsARB)(HDC hDC, HGLRC hshareContext, const int *attribList);
	void *context;
	const int attrib_list[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
							WGL_CONTEXT_MINOR_VERSION_ARB, 0,
							WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
							0};
/*	const int attrib_list[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
							WGL_CONTEXT_MINOR_VERSION_ARB, 5,
							WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
							0};*/
	betray_wglCreateContextAttribsARB  = (void *(__stdcall *)(struct HDC__ *,HGLRC hshareContext, const int *attribList))wglGetProcAddress("wglCreateContextAttribsARB");

	if(betray_wglCreateContextAttribsARB != NULL)
	{
		context = betray_wglCreateContextAttribsARB(hDC, b_win32_opengl_context, attrib_list);
		if(NULL == context)
		{
			printf("Betray: ERROR: b_create_context\n");
			exit(0);
		}
		printf("OpenGL version: %s\n", glGetString(GL_VERSION));
		printf("OpenGL extensions:\n %s\n", glGetString(GL_EXTENSIONS));
	}else
	{
		context = wglCreateContext(hDC);
		if(NULL == context)
		{
			printf("Betray: ERROR: b_create_context\n");
			exit(0);
		}
		if(b_win32_opengl_context != NULL)
			wglShareLists(b_win32_opengl_context, context);
	}
	if(context)
	{
		char *text;
		text = glGetString(GL_VERSION);
		printf("OpenGL version: %s\n", glGetString(GL_VERSION));
		printf("OpenGL extensions:\n %s\n", glGetString(GL_EXTENSIONS));
	}
	return context;
}

__declspec(dllexport) char ovr_Initialize();

boolean b_init_display_opengl(uint size_x, uint size_y, boolean fullscreenflag, uint samples, char* title, boolean *sterioscopic) 
{
	PIXELFORMATDESCRIPTOR pfd;
	GLuint PixelFormat = 0;
	WNDCLASS wc;
	RECT WindowRect;
	char *class_name = "OpenGL";								// Set The Class Name
#ifdef UNICODE
	wchar_t uni[1024], uni_class_name[16];
	unsigned i;
#endif
	if(samples > 1)
	{
		GLvoid (APIENTRY *b_wglChoosePixelFormatARB)(HDC hdc,
											const int *piAttribIList,
											const FLOAT *pfAttribFList,
											uint  nMaxFormats,
											int *piFormats,
											uint *nNumFormats) = NULL;
		uint i, count = 0;
		int format[24];
		boolean output;
		output = b_init_display_opengl(size_x, size_y, fullscreenflag,  1,  title, sterioscopic);		
		b_wglChoosePixelFormatARB = (void (__stdcall *)(struct HDC__ *,const int *,const float *,unsigned int ,int *,unsigned int *))wglGetProcAddress("wglChoosePixelFormatARB");
		if(b_wglChoosePixelFormatARB != NULL)
		{
			int attrib_list[] = {WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
								WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
								WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
								WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
								WGL_COLOR_BITS_ARB, 32,
								WGL_DEPTH_BITS_ARB, 24,
								WGL_STENCIL_BITS_ARB, 8,
								WGL_SAMPLE_BUFFERS_ARB, 1,
								WGL_SAMPLES_ARB, 8,
								WGL_STEREO_ARB, TRUE,
								0};
			while(samples > 1 && PixelFormat == 0)
			{
				for(i = 0; i < 4 && PixelFormat == 0; i++)
				{
					if((i / 4) % 2 == 0) // stencil
					{
						attrib_list[11] = 24; 
						attrib_list[13] = 8;
					}else
					{
						attrib_list[11] = 32; 
						attrib_list[13] = 0;
					}
					attrib_list[17] = samples;
					attrib_list[19] = i % 2 == 0;

					b_wglChoosePixelFormatARB(hDC, attrib_list, NULL, 24, format, &count);
					if(count > 0)
						PixelFormat = format[0];
				}
				samples /= 2; 
			}
			b_win32_window_close();
		}else
			return output;
	}
	WindowRect.left = (b_win32_display_size_x - size_x) / 2;
	WindowRect.right = (b_win32_display_size_x - size_x) / 2 + size_x;
	WindowRect.top = (b_win32_display_size_y - size_y) / 2;
	WindowRect.bottom = (b_win32_display_size_y - size_y) / 2 + size_y;

	if(b_win32_instance == NULL)
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
#ifdef UNICODE
		for(i = 0; i < 1024 && title[i] !=0 ; i++)
			uni[i] = (char)title[i];
		uni[i] = 0;
		for(i = 0; i < 1024 && class_name[i] !=0 ; i++)
			uni_class_name[i] = (char)class_name[i];
		uni_class_name[i] = 0;
		hWnd = CreateWindowEx(WS_EX_APPWINDOW, class_name, uni, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, b_win32_instance, NULL);
#else
		hWnd = CreateWindowEx(WS_EX_APPWINDOW, class_name, title, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, b_win32_instance, NULL);
#endif
	}
	else
	{
		AdjustWindowRectEx(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
#ifdef UNICODE
		for(i = 0; i < 1024 && title[i] !=0 ; i++)
			uni[i] = (char)title[i];
		uni[i] = 0;
		for(i = 0; i < 1024 && class_name[i] !=0 ; i++)
			uni_class_name[i] = (char)class_name[i];
		uni_class_name[i] = 0;
		hWnd = CreateWindowEx(WS_EX_APPWINDOW /* | WS_EX_WINDOWEDGE */, class_name, uni, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, (b_win32_display_size_x - (WindowRect.right - WindowRect.left)) / 2, (b_win32_display_size_y - (WindowRect.bottom - WindowRect.top)) / 2, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, b_win32_instance, NULL);
#else
		hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, class_name, title, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, (b_win32_display_size_x - (WindowRect.right - WindowRect.left)) / 2, (b_win32_display_size_y - (WindowRect.bottom - WindowRect.top)) / 2, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, b_win32_instance, NULL);
#endif;
	}
	if(!hWnd)
	{
		MessageBox(NULL,"BETRAY Error: Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!(hDC = GetDC(hWnd)))
	{
		MessageBox(NULL,"BETRAY Error: Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cStencilBits = 8;
    pfd.cDepthBits = 24;
    pfd.cColorBits = 32;

	if(PixelFormat == 0)
	{
		if(!*sterioscopic || 0 == (PixelFormat = ChoosePixelFormat(hDC, &pfd)))
		{
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			PixelFormat = ChoosePixelFormat(hDC, &pfd);
			if(PixelFormat == 0)
			{
				printf("BETRAY Error: ChoosePixelFormat() failed: Cannot find a suitable pixel format.\n"); 
				return FALSE;
			}
			*sterioscopic = FALSE;
		}else
			*sterioscopic = TRUE;
	}

	if(!SetPixelFormat(hDC, PixelFormat, &pfd))
	{
		MessageBox(NULL,"BETRAY Error: Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
#ifdef BETRAY_CONTEXT_OPENGL
	
	
	if(!(b_win32_opengl_context = b_create_context()))
	{
		MessageBox(NULL,"BETRAY Error: Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!betray_activate_context(NULL))
	{
		MessageBox(NULL,"BETRAY Error: Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
#endif
/*	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);*/

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
			betray_GetTouchInputInfo = (int (__stdcall *)(long, unsigned int, BetrayWinTouch *, int))GetProcAddress(dll, "GetTouchInputInfo");
			betray_CloseTouchInputHandle = (int (__stdcall *)(long ))GetProcAddress(dll, "CloseTouchInputHandle");
			betray_RegisterTouchWindow = (int (__stdcall *)(struct HWND__ *,unsigned long ))GetProcAddress(dll, "RegisterTouchWindow");
		}
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
		betray_RegisterTouchWindow(hWnd, BETRAY_TWF_FINETOUCH/* | BETRAY_TWF_FINETOUCH*/) ;
	betray_set_icon();
	return TRUE;
}



#ifdef BETRAY_CONTEXT_OPENGLES


boolean b_win32_init_display_opengles2(uint size_x, uint size_y, boolean fullscreenflag, uint samples, char* title, boolean *sterioscopic) 
{
	PIXELFORMATDESCRIPTOR pfd;
	GLuint		PixelFormat;
	WNDCLASS	wc;
	RECT		WindowRect;
	char		*class_name = "OGLES";								// Set The Class Name


	/* EGL Configuration */

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
#endif    


void betray_device_init()
{
	betray_reset_path();
	betray_user_id = betray_plugin_user_allocate();
	betray_device_mouse_id = betray_plugin_input_device_allocate(betray_user_id, "Mouse");
	betray_device_keyboard_id = betray_plugin_input_device_allocate(betray_user_id, "Keyboard");
	betray_mouse_id = betray_plugin_pointer_allocate(betray_user_id, betray_device_mouse_id, GetSystemMetrics(SM_CMOUSEBUTTONS), 0.0, 0.0, -1, NULL, "mouse", FALSE);
}


void system_wrapper_lose_focus(void)
{
	input_focus = FALSE;
}

void betray_set_mouse_warp(boolean warp)
{
	mouse_warp = warp;
}


void betray_set_mouse_move(float x, float y)
{
	BInputState *input;
	input = betray_get_input_state();
	mouse_warp_move_x = x - input->pointers[0].pointer_x;
	mouse_warp_move_y = input->pointers[0].pointer_y - y;
	mouse_warp_move = TRUE;
}

uint betray_support_context(BContextType context_type)
{
	return context_type == B_CT_OPENGL || context_type == B_CT_OPENGL_OR_ES;
}

extern uint BGlobal_draw_state_fbo;

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

void *betray_gl_proc_address_get()
{
	return (void *(*)(const char *))betray_gl_proc_address_get_internal;
}

extern void betray_time_update(void);
int my_nCmdShow;

void betray_event_reset(BInputState *input)
{
	uint i, add = 0;
	for(i = 0; i < input->button_event_count; i++)
	{
		if(input->button_event[i].state)
		{
			if(input->button_event[i].button == BETRAY_BUTTON_SCROLL_UP ||
				input->button_event[i].button == BETRAY_BUTTON_SCROLL_DOWN ||
				input->button_event[i].button == BETRAY_BUTTON_SCROLL_RIGHT ||
				input->button_event[i].button == BETRAY_BUTTON_SCROLL_LEFT)
			{
				input->button_event[add] = input->button_event[i];
				input->button_event[add++].state = FALSE;
			}
		}
	}
	input->button_event_count = add;
}

extern void init_trackir_update();

void betray_mouse_init(void) 
{
/*	int			width, height;*/
	RECT		window;

	b_win32_display_size_x = GetSystemMetrics(SM_CXSCREEN);
	b_win32_display_size_y = GetSystemMetrics(SM_CYSCREEN);

	GetWindowRect(hWnd, &window);
	if(window.left < 0)
		window.left = 0;
	if(window.top < 0)
		window.top = 0;
	if(window.right >= b_win32_display_size_x)
		window.right = b_win32_display_size_x - 1;
	if(window.bottom >= b_win32_display_size_y - 1)
		window.bottom = b_win32_display_size_y - 1;

/*
	b_window_pos_x = window.left;
	b_window_pos_y = window.top;
*/
	b_window_center_x = (window.right + window.left) / 2;
	b_window_center_y = (window.top + window.bottom) / 2;

//	SetCursorPos(b_window_center_x, b_window_center_y);
//	SetCapture(hWnd);
//	ClipCursor(&window);
//	while(ShowCursor(FALSE) >= 0);
}

 typedef struct __GLsync *GLsync;
 
GLsync (__stdcall *b_glFenceSync)(GLenum condition, GLbitfield flags) = NULL;
GLenum (__stdcall *b_glClientWaitSync)(GLsync sync, GLbitfield flags, uint64 timeout) = NULL;
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117

void betray_end_of_frame_flush() /* leaking memeory! */
{
	static GLsync fence = ~0;
	if(fence == ~0)
	{
		b_glFenceSync = (GLsync (__stdcall *)(GLenum condition, GLbitfield flags))wglGetProcAddress("glFenceSync");
		b_glClientWaitSync = (GLenum (__stdcall *)(GLsync sync, GLbitfield flags, uint64 timeout))wglGetProcAddress("glClientWaitSync");
		fence = b_glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
	b_glClientWaitSync(fence, 0, 100000000);
}




void betray_launch_main_loop(void)
{
	boolean fullscreen, show_window = FALSE;
	MSG msg;
	BInputState *input;
	boolean swap;
	TCHAR path[MAX_PATH + 1] = {0};
	double latency_times[10];

	input = betray_get_input_state();
/*	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	SetCapture(hWnd);*/
	betray_mouse_init();

	betray_reset_path();
	
	GetModuleFileName(0, path, MAX_PATH + 1);
	   

/*	{
		RAWINPUTDEVICELIST devicelist[64];
		RAWINPUTDEVICE Rid[2];
		uint count = 64, i;
		count = GetRawInputDeviceList(devicelist, &count , sizeof(RAWINPUTDEVICELIST));
		for(i = 0; i < count; i++)
		{
			printf("device[%u]: %u  %u\n", i, devicelist[i].dwType, RIM_TYPEMOUSE);
		}
		
		Rid[0].usUsagePage = 0x01; 
		Rid[0].usUsage = 0x02; 
		Rid[0].dwFlags = RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
		Rid[0].hwndTarget = 0;
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
	}*/

    while(!window_close)
	{
		uint i, j;


/*	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(GetMessage(&msg, NULL, 0, 0) > 0)
				DispatchMessage(&msg);
		}*/
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else				// If There Are No Messages
		{
			POINT current_pos;
			if(mouse_hide)
				ShowCursor(!mouse_inside);
			betray_time_update();
			betray_mouse_init(); 
			if(mouse_warp_move)
			{
				GetCursorPos(&current_pos);
				SetCursorPos(current_pos.x + (int)(mouse_warp_move_x * (float)b_window_size_x * 0.5), current_pos.y + (int)(mouse_warp_move_y * (float)b_window_size_y  * 0.5));
				mouse_warp_move = FALSE;
			}
		/*	if(mouse_warp)
				SetCursorPos(b_window_center_x, b_window_center_y);
			GetCursorPos(&current_pos);
			input->pointers[0].pointer_x = (float)(current_pos.x - b_window_center_x) / (float)b_window_size_x * 2.0;
			betray_screen_mode_get(NULL, NULL, &fullscreen);
			if(fullscreen)
				input->pointers[0].pointer_y = (float)(current_pos.y - b_window_center_y) / (float)b_window_size_x * -2.0;
			else
				input->pointers[0].pointer_y = (float)(current_pos.y - b_window_center_y - 12) / (float)b_window_size_x * -2.0;*/
			input->pointers[0].button_count = 3;
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
					if(input->pointers[i].button[j] && !input->pointers[i].last_button[j])
					{
						input->pointers[i].click_pointer_x[j] = input->pointers[i].pointer_x;
						input->pointers[i].click_pointer_y[j] = input->pointers[i].pointer_y;
					}
				}
			}
			betray_plugin_callback_main(input);
			
			if(!window_minimized)
			{
				betray_action(BAM_EVENT);
				betray_action(BAM_DRAW);
				glFlush();
				betray_end_of_frame_flush();
				SwapBuffers(hDC);

#ifdef BETRAY_CONTEXT_OPENGL		
#endif
#ifdef BETRAY_CONTEXT_OPENGLES
				eglSwapBuffers(sEGLDisplay, sEGLSurface);
#endif
				if(!show_window)
				{
					show_window = TRUE;
					ShowWindow(hWnd, SW_SHOW);
					SetForegroundWindow(hWnd);
					SetFocus(hWnd);
					SetCapture(hWnd);
				}
			}
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
}

/*
void SetStdOutToNewConsole()
{
    int hConHandle;
    long lStdHandle;
    FILE *fp;

    // Allocate a console for this app
    AllocConsole();

    // Redirect unbuffered STDOUT to the console
    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen(hConHandle, "w");
    *stdout = *fp;

    setvbuf(stdout, NULL, _IONBF, 0);
}
*/
extern int main(int argc, char **argv);

int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
	my_nCmdShow = nCmdShow;
	LPWSTR *argv;
	int argc;
	AttachConsole(ATTACH_PARENT_PROCESS);
    HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hConOut != NULL)
	{
	    int fd = _open_osfhandle((intptr_t)hConOut, 0);
		_dup2(fd, 1);
	}
	argv = CommandLineToArgvW(lpszCmdLine, &argc);
//	SetStdOutToNewConsole();
	main(argc, argv);
	LocalFree(argv);
    return TRUE;
}

#endif