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
			{
				input->pointers[0].button[0] = TRUE;
				betray_plugin_button_set(betray_user_id, betray_buttons[BETRAY_ENUM_BUTTON_FACE_A].system_code, TRUE, -1);
			}
			if(uMsg == WM_LBUTTONUP || uMsg == WM_NCLBUTTONUP)
			{
				input->pointers[0].button[0] = FALSE;
				betray_plugin_button_set(betray_user_id, betray_buttons[BETRAY_ENUM_BUTTON_FACE_A].system_code, FALSE, -1);
			}
			if(uMsg == WM_RBUTTONDOWN)
			{
				input->pointers[0].button[1] = TRUE;
				betray_plugin_button_set(betray_user_id, betray_buttons[BETRAY_ENUM_BUTTON_FACE_B].system_code, TRUE, -1);
			}
			if(uMsg == WM_RBUTTONUP || uMsg == WM_NCRBUTTONUP)
			{
				input->pointers[0].button[1] = FALSE;
				betray_plugin_button_set(betray_user_id, betray_buttons[BETRAY_ENUM_BUTTON_FACE_B].system_code, FALSE, -1);
			}
			if(uMsg == WM_MBUTTONDOWN)
			{
				input->pointers[0].button[2] = TRUE;
				betray_plugin_button_set(betray_user_id, betray_buttons[BETRAY_ENUM_BUTTON_FACE_X].system_code, TRUE, -1);
			}
			if(uMsg == WM_MBUTTONUP || uMsg == WM_NCMBUTTONUP)
			{
				input->pointers[0].button[2] = FALSE;
				betray_plugin_button_set(betray_user_id, betray_buttons[BETRAY_ENUM_BUTTON_FACE_X].system_code, FALSE, -1);
			}
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

void betray_set_icon(int width, int height, uint8* rgba_pixels)
{
	uint8 *pixels;
	ICONINFO icon_info = { 0 };
    HBITMAP bitmap;
	HICON icon;
	uint i;
	pixels = malloc((sizeof *pixels) * width * height * 4);
	for(i = 0; i < width * height * 4; i += 4)
	{
		pixels[i + 0] = rgba_pixels[i + 2];
		pixels[i + 1] = rgba_pixels[i + 1];
		pixels[i + 2] = rgba_pixels[i + 0];
		pixels[i + 3] = rgba_pixels[i + 3];
	}
	bitmap = CreateBitmap(width, height, 1, 32, pixels);
	free(pixels);
    if(bitmap == NULL)
        return NULL;
    icon_info.fIcon = TRUE;
    icon_info.hbmColor = bitmap;
    icon_info.hbmMask = bitmap;
    icon = CreateIconIndirect(&icon_info);
    DeleteObject(bitmap);
    SendMessage(betray_plugin_windows_window_handle_get(), WM_SETICON, ICON_SMALL, (LPARAM)icon);
    SendMessage(betray_plugin_windows_window_handle_get(), WM_SETICON, ICON_BIG, (LPARAM)icon);
    SendMessage(GetWindow(betray_plugin_windows_window_handle_get(), GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)icon);
    SendMessage(GetWindow(betray_plugin_windows_window_handle_get(), GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)icon);
}

#include "hxa.h"
int hxa_load_png(HXAFile* file, char* file_name);
void hxa_print_layer(HXALayer *layers, unsigned int length);
void hxa_print_layer_runlength(HXALayer *layers, unsigned int length);

void unpack(unsigned char *output, unsigned char *input, unsigned int input_length, unsigned int channels)
{
	unsigned int i, j, k, count;
	for(i = 0; i < input_length; i++)
	{
		count = input[i++];
		for(j = 0; j < count; j++)
			for(k = 0; k < channels; k++)
				*output++ = input[i + k];
		i += channels;
		if(i >= input_length)
			return;
		count = input[i++];
		for(j = 0; j < count * channels; j++)
			*output++ = input[i++];
	}
}

unsigned char icon[] = {255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 44, 0, 0, 0, 0, 9, 255, 255, 255, 11, 159, 159, 159, 129, 107, 107, 107, 247, 101, 101, 101, 255, 127, 127, 127, 191, 173, 173, 173, 121, 205, 205, 205, 24, 255, 255, 255, 15, 0, 0, 0, 0, 94, 0, 0, 0, 0, 9, 255, 255, 255, 3, 255, 255, 255, 17, 187, 187, 187, 73, 129, 129, 129, 136, 117, 117, 117, 247, 98, 98, 98, 255, 129, 129, 129, 191, 231, 231, 231, 70, 0, 0, 0, 0, 141, 0, 0, 0, 0, 4, 255, 255, 255, 7, 171, 171, 171, 176, 36, 36, 36, 255, 0, 0, 0, 255, 1, 3, 3, 3, 255, 8, 25, 25, 25, 255, 51, 51, 51, 240, 108, 108, 108, 231, 125, 125, 125, 135, 213, 213, 213, 121, 255, 255, 255, 24, 255, 255, 255, 2, 0, 0, 0, 0, 87, 0, 0, 0, 0, 8, 255, 255, 255, 15, 231, 231, 231, 73, 168, 168, 168, 128, 111, 111, 111, 183, 77, 77, 77, 238, 31, 31, 31, 253, 14, 14, 14, 255, 0, 0, 0, 255, 1, 5, 5, 5, 255, 3, 110, 110, 110, 245, 246, 246, 246, 71, 0, 0, 0, 0, 140, 0, 0, 0, 0, 3, 243, 243, 243, 25, 79, 79, 79, 246, 0, 0, 0, 255, 5, 0, 0, 0, 255, 8, 14, 14, 14, 255, 41, 41, 41, 255, 78, 78, 78, 240, 109, 109, 109, 183, 167, 167, 167, 128, 230, 230, 230, 72, 255, 255, 255, 15, 0, 0, 0, 0, 80, 0, 0, 0, 0, 9, 255, 255, 255, 3, 254, 254, 254, 24, 203, 203, 203, 121, 111, 111, 111, 135, 100, 100, 100, 231, 54, 54, 54, 253, 23, 23, 23, 255, 3, 3, 3, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 22, 22, 22, 255, 162, 162, 162, 127, 0, 0, 0, 0, 140, 0, 0, 0, 0, 3, 213, 213, 213, 119, 39, 39, 39, 254, 0, 0, 0, 255, 8, 0, 0, 0, 255, 9, 3, 3, 3, 255, 23, 23, 23, 255, 53, 53, 53, 252, 98, 98, 98, 231, 108, 108, 108, 135, 200, 200, 200, 121, 254, 254, 254, 24, 255, 255, 255, 2, 0, 0, 0, 0, 73, 0, 0, 0, 0, 8, 255, 255, 255, 15, 217, 217, 217, 73, 151, 151, 151, 128, 99, 99, 99, 183, 70, 70, 70, 240, 35, 35, 35, 255, 12, 12, 12, 255, 0, 0, 0, 255, 8, 0, 0, 0, 255, 4, 4, 4, 4, 255, 109, 109, 109, 183, 255, 255, 255, 9, 0, 0, 0, 0, 138, 0, 0, 0, 0, 4, 255, 255, 255, 1, 137, 137, 137, 137, 15, 15, 15, 255, 0, 0, 0, 255, 12, 0, 0, 0, 255, 8, 12, 12, 12, 255, 35, 35, 35, 255, 70, 70, 70, 240, 97, 97, 97, 183, 151, 151, 151, 128, 235, 235, 235, 72, 255, 255, 255, 3, 0, 0, 0, 0, 67, 0, 0, 0, 0, 8, 255, 255, 255, 22, 192, 192, 192, 121, 95, 95, 95, 135, 91, 91, 91, 231, 46, 46, 46, 253, 19, 19, 19, 255, 1, 1, 1, 255, 0, 0, 0, 255, 12, 0, 0, 0, 255, 3, 83, 83, 83, 242, 254, 254, 254, 13, 0, 0, 0, 0, 138, 0, 0, 0, 0, 3, 255, 255, 255, 15, 121, 121, 121, 230, 0, 0, 0, 255, 16, 0, 0, 0, 255, 6, 1, 1, 1, 255, 19, 19, 19, 255, 66, 66, 66, 252, 191, 191, 191, 173, 255, 255, 255, 3, 0, 0, 0, 0, 65, 0, 0, 0, 0, 5, 234, 234, 234, 66, 118, 118, 118, 233, 32, 32, 32, 255, 10, 10, 10, 255, 0, 0, 0, 255, 16, 0, 0, 0, 255, 3, 37, 37, 37, 247, 204, 204, 204, 71, 0, 0, 0, 0, 138, 0, 0, 0, 0, 3, 236, 236, 236, 25, 66, 66, 66, 240, 0, 0, 0, 255, 19, 0, 0, 0, 255, 3, 62, 62, 62, 253, 214, 214, 214, 68, 0, 0, 0, 0, 64, 0, 0, 0, 0, 4, 255, 255, 255, 2, 118, 118, 118, 188, 7, 7, 7, 255, 0, 0, 0, 255, 18, 0, 0, 0, 255, 4, 18, 18, 18, 255, 153, 153, 153, 140, 255, 255, 255, 1, 0, 0, 0, 0, 137, 0, 0, 0, 0, 3, 207, 207, 207, 119, 37, 37, 37, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 6, 6, 0, 0, 255, 87, 0, 0, 255, 71, 0, 0, 255, 32, 0, 0, 255, 6, 0, 0, 255, 0, 0, 0, 255, 10, 0, 0, 0, 255, 4, 6, 6, 6, 255, 115, 115, 115, 187, 255, 255, 255, 2, 0, 0, 0, 0, 63, 0, 0, 0, 0, 3, 185, 185, 185, 81, 40, 40, 40, 252, 0, 0, 0, 255, 10, 0, 0, 0, 255, 6, 1, 0, 0, 255, 18, 0, 0, 255, 53, 0, 0, 255, 92, 0, 0, 255, 41, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 132, 132, 132, 230, 255, 255, 255, 15, 0, 0, 0, 0, 136, 0, 0, 0, 0, 4, 255, 255, 255, 1, 125, 125, 125, 137, 14, 14, 14, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 10, 30, 0, 0, 255, 156, 0, 0, 255, 167, 0, 0, 255, 159, 0, 0, 255, 136, 0, 0, 255, 98, 0, 0, 255, 57, 0, 0, 255, 19, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 7, 0, 0, 0, 255, 3, 39, 39, 39, 252, 185, 185, 185, 80, 0, 0, 0, 0, 62, 0, 0, 0, 0, 3, 255, 255, 255, 22, 119, 119, 119, 237, 0, 0, 0, 255, 8, 0, 0, 0, 255, 9, 9, 0, 0, 255, 40, 0, 0, 255, 79, 0, 0, 255, 120, 0, 0, 255, 151, 0, 0, 255, 164, 0, 0, 255, 170, 0, 0, 255, 91, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 77, 77, 77, 240, 249, 249, 249, 25, 0, 0, 0, 0, 136, 0, 0, 0, 0, 3, 255, 255, 255, 15, 114, 114, 114, 230, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 65, 0, 0, 255, 167, 0, 0, 255, 1, 170, 0, 0, 255, 9, 173, 0, 0, 255, 177, 0, 0, 255, 173, 0, 0, 255, 157, 0, 0, 255, 125, 0, 0, 255, 80, 0, 0, 255, 39, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 120, 120, 120, 237, 255, 255, 255, 22, 0, 0, 0, 0, 61, 0, 0, 0, 0, 3, 182, 182, 182, 128, 31, 31, 31, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 14, 2, 0, 0, 255, 23, 0, 0, 255, 61, 0, 0, 255, 104, 0, 0, 255, 145, 0, 0, 255, 168, 0, 0, 255, 177, 0, 0, 255, 175, 0, 0, 255, 171, 0, 0, 255, 167, 0, 0, 255, 169, 0, 0, 255, 130, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 44, 44, 44, 254, 217, 217, 217, 118, 0, 0, 0, 0, 136, 0, 0, 0, 0, 3, 231, 231, 231, 25, 61, 61, 61, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 15, 106, 0, 0, 255, 173, 0, 0, 255, 170, 0, 0, 255, 172, 0, 0, 255, 175, 0, 0, 255, 177, 0, 0, 255, 178, 0, 0, 255, 179, 0, 0, 255, 181, 0, 0, 255, 179, 0, 0, 255, 167, 0, 0, 255, 141, 0, 0, 255, 100, 0, 0, 255, 28, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 32, 32, 32, 255, 183, 183, 183, 127, 0, 0, 0, 0, 60, 0, 0, 0, 0, 3, 255, 255, 255, 22, 91, 91, 91, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 16, 4, 0, 0, 255, 71, 0, 0, 255, 124, 0, 0, 255, 157, 0, 0, 255, 175, 0, 0, 255, 181, 0, 0, 255, 180, 0, 0, 255, 178, 0, 0, 255, 177, 0, 0, 255, 176, 0, 0, 255, 174, 0, 0, 255, 172, 0, 0, 255, 170, 0, 0, 255, 157, 0, 0, 255, 25, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 16, 16, 16, 255, 141, 141, 141, 136, 255, 255, 255, 1, 0, 0, 0, 0, 135, 0, 0, 0, 0, 3, 197, 197, 197, 119, 33, 33, 33, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 8, 8, 0, 0, 255, 143, 0, 0, 255, 174, 0, 0, 255, 175, 0, 0, 255, 177, 0, 0, 255, 180, 0, 0, 255, 181, 0, 0, 255, 183, 0, 0, 255, 1, 182, 0, 0, 255, 7, 181, 0, 0, 255, 179, 0, 0, 255, 177, 0, 0, 255, 178, 0, 0, 255, 123, 0, 0, 255, 5, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 92, 92, 92, 233, 255, 255, 255, 22, 0, 0, 0, 0, 59, 0, 0, 0, 0, 3, 147, 147, 147, 127, 21, 21, 21, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 59, 0, 0, 255, 170, 0, 0, 255, 177, 0, 0, 255, 178, 0, 0, 255, 180, 0, 0, 255, 182, 0, 0, 255, 1, 183, 0, 0, 255, 5, 182, 0, 0, 255, 180, 0, 0, 255, 179, 0, 0, 255, 176, 0, 0, 255, 172, 0, 0, 255, 1, 62, 0, 0, 255, 1, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 126, 126, 126, 230, 255, 255, 255, 15, 0, 0, 0, 0, 134, 0, 0, 0, 0, 4, 255, 255, 255, 1, 114, 114, 114, 137, 12, 12, 12, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 10, 35, 0, 0, 255, 166, 0, 0, 255, 175, 0, 0, 255, 179, 0, 0, 255, 181, 0, 0, 255, 184, 0, 0, 255, 185, 0, 0, 255, 187, 0, 0, 255, 188, 0, 0, 255, 187, 0, 0, 255, 1, 185, 0, 0, 255, 5, 182, 0, 0, 255, 179, 0, 0, 255, 175, 0, 0, 255, 62, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 21, 21, 21, 255, 146, 146, 146, 126, 0, 0, 0, 0, 58, 0, 0, 0, 0, 3, 243, 243, 243, 66, 76, 76, 76, 241, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 11, 0, 0, 255, 142, 0, 0, 255, 180, 0, 0, 255, 181, 0, 0, 255, 184, 0, 0, 255, 186, 0, 0, 255, 187, 0, 0, 255, 1, 187, 0, 0, 255, 8, 186, 0, 0, 255, 185, 0, 0, 255, 183, 0, 0, 255, 180, 0, 0, 255, 176, 0, 0, 255, 177, 0, 0, 255, 101, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 71, 71, 71, 240, 242, 242, 242, 25, 0, 0, 0, 0, 134, 0, 0, 0, 0, 3, 255, 255, 255, 15, 108, 108, 108, 230, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 74, 0, 0, 255, 178, 0, 0, 255, 1, 182, 0, 0, 255, 5, 185, 0, 0, 255, 187, 0, 0, 255, 189, 0, 0, 255, 191, 0, 0, 255, 192, 0, 0, 255, 1, 192, 0, 0, 255, 3, 191, 0, 0, 255, 189, 0, 0, 255, 186, 0, 0, 255, 1, 149, 0, 0, 255, 2, 12, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 77, 77, 77, 240, 243, 243, 243, 66, 0, 0, 0, 0, 56, 0, 0, 0, 0, 4, 255, 255, 255, 3, 153, 153, 153, 188, 13, 13, 13, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 83, 0, 0, 255, 185, 0, 0, 255, 1, 188, 0, 0, 255, 2, 190, 0, 0, 255, 192, 0, 0, 255, 1, 192, 0, 0, 255, 10, 191, 0, 0, 255, 190, 0, 0, 255, 188, 0, 0, 255, 186, 0, 0, 255, 183, 0, 0, 255, 180, 0, 0, 255, 178, 0, 0, 255, 140, 0, 0, 255, 6, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 40, 40, 40, 254, 211, 211, 211, 118, 0, 0, 0, 0, 134, 0, 0, 0, 0, 3, 221, 221, 221, 25, 56, 56, 56, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 113, 0, 0, 255, 181, 0, 0, 255, 1, 185, 0, 0, 255, 5, 188, 0, 0, 255, 191, 0, 0, 255, 193, 0, 0, 255, 195, 0, 0, 255, 196, 0, 0, 255, 1, 197, 0, 0, 255, 7, 196, 0, 0, 255, 195, 0, 0, 255, 192, 0, 0, 255, 189, 0, 0, 255, 190, 0, 0, 255, 85, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 13, 13, 13, 255, 154, 154, 154, 187, 255, 255, 255, 3, 0, 0, 0, 0, 55, 0, 0, 0, 0, 3, 220, 220, 220, 68, 63, 63, 63, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 22, 0, 0, 255, 165, 0, 0, 255, 191, 0, 0, 255, 1, 194, 0, 0, 255, 2, 195, 0, 0, 255, 197, 0, 0, 255, 1, 196, 0, 0, 255, 10, 195, 0, 0, 255, 194, 0, 0, 255, 192, 0, 0, 255, 189, 0, 0, 255, 186, 0, 0, 255, 182, 0, 0, 255, 179, 0, 0, 255, 165, 0, 0, 255, 31, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 14, 14, 14, 255, 132, 132, 132, 136, 255, 255, 255, 1, 0, 0, 0, 0, 133, 0, 0, 0, 0, 3, 190, 190, 190, 119, 30, 30, 30, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 14, 10, 0, 0, 255, 148, 0, 0, 255, 180, 0, 0, 255, 182, 0, 0, 255, 188, 0, 0, 255, 191, 0, 0, 255, 194, 0, 0, 255, 197, 0, 0, 255, 198, 0, 0, 255, 199, 0, 0, 255, 200, 0, 0, 255, 201, 0, 0, 255, 200, 0, 0, 255, 199, 0, 0, 255, 1, 196, 0, 0, 255, 4, 195, 0, 0, 255, 170, 0, 0, 255, 23, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 64, 64, 64, 252, 222, 222, 222, 68, 0, 0, 0, 0, 54, 0, 0, 0, 0, 4, 255, 255, 255, 2, 122, 122, 122, 188, 7, 7, 7, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 106, 0, 0, 255, 197, 0, 0, 255, 194, 0, 0, 255, 197, 0, 0, 255, 199, 0, 0, 255, 200, 0, 0, 255, 201, 0, 0, 255, 1, 200, 0, 0, 255, 10, 199, 0, 0, 255, 197, 0, 0, 255, 195, 0, 0, 255, 192, 0, 0, 255, 189, 0, 0, 255, 185, 0, 0, 255, 180, 0, 0, 255, 177, 0, 0, 255, 67, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 120, 120, 120, 230, 255, 255, 255, 15, 0, 0, 0, 0, 132, 0, 0, 0, 0, 4, 255, 255, 255, 1, 106, 106, 106, 137, 11, 11, 11, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 12, 41, 0, 0, 255, 169, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 197, 0, 0, 255, 199, 0, 0, 255, 201, 0, 0, 255, 203, 0, 0, 255, 204, 0, 0, 255, 205, 0, 0, 255, 1, 204, 0, 0, 255, 6, 203, 0, 0, 255, 201, 0, 0, 255, 199, 0, 0, 255, 202, 0, 0, 255, 109, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 7, 7, 7, 255, 124, 124, 124, 187, 255, 255, 255, 2, 0, 0, 0, 0, 53, 0, 0, 0, 0, 3, 195, 195, 195, 81, 43, 43, 43, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 37, 0, 0, 255, 184, 0, 0, 255, 200, 0, 0, 255, 1, 202, 0, 0, 255, 1, 204, 0, 0, 255, 1, 205, 0, 0, 255, 12, 204, 0, 0, 255, 203, 0, 0, 255, 202, 0, 0, 255, 200, 0, 0, 255, 198, 0, 0, 255, 195, 0, 0, 255, 192, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 181, 0, 0, 255, 104, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 64, 64, 64, 240, 232, 232, 232, 25, 0, 0, 0, 0, 132, 0, 0, 0, 0, 3, 255, 255, 255, 11, 100, 100, 100, 230, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 75, 0, 0, 255, 178, 0, 0, 255, 181, 0, 0, 255, 187, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 199, 0, 0, 255, 202, 0, 0, 255, 204, 0, 0, 255, 206, 0, 0, 255, 207, 0, 0, 255, 208, 0, 0, 255, 1, 208, 0, 0, 255, 7, 207, 0, 0, 255, 206, 0, 0, 255, 204, 0, 0, 255, 203, 0, 0, 255, 189, 0, 0, 255, 38, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 44, 44, 44, 252, 196, 196, 196, 80, 0, 0, 0, 0, 52, 0, 0, 0, 0, 4, 255, 255, 255, 22, 125, 125, 125, 237, 1, 1, 1, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 7, 1, 0, 0, 255, 131, 0, 0, 255, 207, 0, 0, 255, 203, 0, 0, 255, 205, 0, 0, 255, 207, 0, 0, 255, 208, 0, 0, 255, 2, 208, 0, 0, 255, 12, 206, 0, 0, 255, 205, 0, 0, 255, 203, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 193, 0, 0, 255, 189, 0, 0, 255, 183, 0, 0, 255, 180, 0, 0, 255, 140, 0, 0, 255, 7, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 35, 35, 35, 254, 203, 203, 203, 118, 0, 0, 0, 0, 132, 0, 0, 0, 0, 3, 236, 236, 236, 73, 59, 59, 59, 248, 0, 0, 0, 255, 2, 0, 0, 0, 255, 14, 1, 0, 0, 255, 112, 0, 0, 255, 179, 0, 0, 255, 182, 0, 0, 255, 189, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 209, 0, 0, 255, 210, 0, 0, 255, 211, 0, 0, 255, 212, 0, 0, 255, 1, 211, 0, 0, 255, 7, 210, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 211, 0, 0, 255, 133, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 1, 1, 1, 255, 127, 127, 127, 237, 255, 255, 255, 22, 0, 0, 0, 0, 51, 0, 0, 0, 0, 3, 191, 191, 191, 128, 35, 35, 35, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 55, 0, 0, 255, 201, 0, 0, 255, 207, 0, 0, 255, 208, 0, 0, 255, 210, 0, 0, 255, 211, 0, 0, 255, 212, 0, 0, 255, 1, 212, 0, 0, 255, 13, 211, 0, 0, 255, 209, 0, 0, 255, 208, 0, 0, 255, 205, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 191, 0, 0, 255, 185, 0, 0, 255, 179, 0, 0, 255, 162, 0, 0, 255, 33, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 13, 13, 13, 255, 123, 123, 123, 136, 255, 255, 255, 1, 0, 0, 0, 0, 131, 0, 0, 0, 0, 3, 189, 189, 189, 133, 29, 29, 29, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 14, 9, 0, 0, 255, 142, 0, 0, 255, 177, 0, 0, 255, 183, 0, 0, 255, 190, 0, 0, 255, 196, 0, 0, 255, 200, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 210, 0, 0, 255, 211, 0, 0, 255, 212, 0, 0, 255, 214, 0, 0, 255, 215, 0, 0, 255, 1, 215, 0, 0, 255, 7, 214, 0, 0, 255, 213, 0, 0, 255, 211, 0, 0, 255, 210, 0, 0, 255, 204, 0, 0, 255, 55, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 36, 36, 36, 255, 191, 191, 191, 127, 0, 0, 0, 0, 50, 0, 0, 0, 0, 3, 255, 255, 255, 22, 98, 98, 98, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 6, 0, 0, 255, 154, 0, 0, 255, 214, 0, 0, 255, 210, 0, 0, 255, 213, 0, 0, 255, 214, 0, 0, 255, 215, 0, 0, 255, 1, 215, 0, 0, 255, 14, 214, 0, 0, 255, 213, 0, 0, 255, 212, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 198, 0, 0, 255, 193, 0, 0, 255, 186, 0, 0, 255, 178, 0, 0, 255, 171, 0, 0, 255, 63, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 113, 113, 113, 231, 255, 255, 255, 16, 0, 0, 0, 0, 131, 0, 0, 0, 0, 3, 159, 159, 159, 133, 18, 18, 18, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 15, 3, 0, 0, 255, 111, 0, 0, 255, 179, 0, 0, 255, 184, 0, 0, 255, 192, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 206, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 213, 0, 0, 255, 215, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 219, 0, 0, 255, 1, 218, 0, 0, 255, 7, 217, 0, 0, 255, 215, 0, 0, 255, 213, 0, 0, 255, 217, 0, 0, 255, 156, 0, 0, 255, 6, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 99, 99, 99, 233, 255, 255, 255, 22, 0, 0, 0, 0, 49, 0, 0, 0, 0, 3, 158, 158, 158, 128, 24, 24, 24, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 76, 0, 0, 255, 213, 0, 0, 255, 214, 0, 0, 255, 215, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 219, 0, 0, 255, 1, 218, 0, 0, 255, 14, 217, 0, 0, 255, 216, 0, 0, 255, 214, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 204, 0, 0, 255, 199, 0, 0, 255, 194, 0, 0, 255, 188, 0, 0, 255, 180, 0, 0, 255, 159, 0, 0, 255, 34, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 97, 97, 97, 245, 255, 255, 255, 18, 0, 0, 0, 0, 131, 0, 0, 0, 0, 3, 195, 195, 195, 73, 43, 43, 43, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 35, 0, 0, 255, 167, 0, 0, 255, 186, 0, 0, 255, 193, 0, 0, 255, 199, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 216, 0, 0, 255, 218, 0, 0, 255, 220, 0, 0, 255, 2, 221, 0, 0, 255, 5, 222, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 218, 0, 0, 255, 216, 0, 0, 255, 1, 76, 0, 0, 255, 1, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 24, 24, 24, 255, 157, 157, 157, 127, 0, 0, 0, 0, 48, 0, 0, 0, 0, 3, 227, 227, 227, 18, 73, 73, 73, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 14, 0, 0, 255, 176, 0, 0, 255, 219, 0, 0, 255, 217, 0, 0, 255, 219, 0, 0, 255, 220, 0, 0, 255, 221, 0, 0, 255, 1, 222, 0, 0, 255, 14, 221, 0, 0, 255, 220, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 205, 0, 0, 255, 200, 0, 0, 255, 195, 0, 0, 255, 188, 0, 0, 255, 184, 0, 0, 255, 99, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 7, 7, 7, 255, 128, 128, 128, 187, 255, 255, 255, 8, 0, 0, 0, 0, 131, 0, 0, 0, 0, 4, 255, 255, 255, 3, 110, 110, 110, 187, 6, 6, 6, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 105, 0, 0, 255, 191, 0, 0, 255, 193, 0, 0, 255, 200, 0, 0, 255, 205, 0, 0, 255, 209, 0, 0, 255, 213, 0, 0, 255, 216, 0, 0, 255, 218, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 223, 0, 0, 255, 224, 0, 0, 255, 225, 0, 0, 255, 1, 224, 0, 0, 255, 7, 223, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 178, 0, 0, 255, 14, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 75, 75, 75, 233, 229, 229, 229, 18, 0, 0, 0, 0, 46, 0, 0, 0, 0, 4, 255, 255, 255, 3, 154, 154, 154, 175, 11, 11, 11, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 98, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 224, 0, 0, 255, 225, 0, 0, 255, 1, 224, 0, 0, 255, 14, 223, 0, 0, 255, 222, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 217, 0, 0, 255, 213, 0, 0, 255, 211, 0, 0, 255, 207, 0, 0, 255, 202, 0, 0, 255, 197, 0, 0, 255, 191, 0, 0, 255, 164, 0, 0, 255, 25, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 57, 57, 57, 252, 208, 208, 208, 67, 0, 0, 0, 0, 133, 0, 0, 0, 0, 3, 211, 211, 211, 68, 56, 56, 56, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 25, 0, 0, 255, 170, 0, 0, 255, 196, 0, 0, 255, 200, 0, 0, 255, 206, 0, 0, 255, 210, 0, 0, 255, 214, 0, 0, 255, 218, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 224, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 1, 228, 0, 0, 255, 1, 227, 0, 0, 255, 1, 226, 0, 0, 255, 5, 224, 0, 0, 255, 221, 0, 0, 255, 226, 0, 0, 255, 100, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 12, 12, 12, 255, 156, 156, 156, 175, 255, 255, 255, 3, 0, 0, 0, 0, 45, 0, 0, 0, 0, 3, 228, 228, 228, 68, 70, 70, 70, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 25, 0, 0, 255, 195, 0, 0, 255, 224, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 226, 0, 0, 255, 227, 0, 0, 255, 228, 0, 0, 255, 1, 227, 0, 0, 255, 13, 226, 0, 0, 255, 225, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 208, 0, 0, 255, 202, 0, 0, 255, 197, 0, 0, 255, 195, 0, 0, 255, 91, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 10, 10, 10, 255, 144, 144, 144, 187, 255, 255, 255, 3, 0, 0, 0, 0, 133, 0, 0, 0, 0, 4, 255, 255, 255, 3, 143, 143, 143, 187, 10, 10, 10, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 94, 0, 0, 255, 199, 0, 0, 255, 201, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 215, 0, 0, 255, 219, 0, 0, 255, 222, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 229, 0, 0, 255, 230, 0, 0, 255, 2, 230, 0, 0, 255, 7, 229, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 197, 0, 0, 255, 26, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 70, 70, 70, 253, 228, 228, 228, 68, 0, 0, 0, 0, 44, 0, 0, 0, 0, 4, 255, 255, 255, 3, 131, 131, 131, 188, 8, 8, 8, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 123, 0, 0, 255, 229, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 228, 0, 0, 255, 229, 0, 0, 255, 230, 0, 0, 255, 1, 230, 0, 0, 255, 14, 229, 0, 0, 255, 228, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 213, 0, 0, 255, 208, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 163, 0, 0, 255, 16, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 79, 79, 79, 253, 237, 237, 237, 67, 0, 0, 0, 0, 135, 0, 0, 0, 0, 3, 236, 236, 236, 68, 78, 78, 78, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 15, 17, 0, 0, 255, 168, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 216, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 1, 232, 0, 0, 255, 6, 231, 0, 0, 255, 229, 0, 0, 255, 226, 0, 0, 255, 232, 0, 0, 255, 123, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 8, 8, 8, 255, 133, 133, 133, 187, 255, 255, 255, 3, 0, 0, 0, 0, 43, 0, 0, 0, 0, 3, 196, 196, 196, 68, 49, 49, 49, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 41, 0, 0, 255, 211, 0, 0, 255, 228, 0, 0, 255, 1, 230, 0, 0, 255, 4, 231, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 1, 231, 0, 0, 255, 12, 230, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 214, 0, 0, 255, 210, 0, 0, 255, 204, 0, 0, 255, 201, 0, 0, 255, 78, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 15, 15, 15, 255, 167, 167, 167, 174, 255, 255, 255, 3, 0, 0, 0, 0, 135, 0, 0, 0, 0, 4, 255, 255, 255, 3, 166, 166, 166, 175, 14, 14, 14, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 82, 0, 0, 255, 206, 0, 0, 255, 208, 0, 0, 255, 213, 0, 0, 255, 217, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 1, 234, 0, 0, 255, 1, 235, 0, 0, 255, 1, 235, 0, 0, 255, 4, 234, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 1, 213, 0, 0, 255, 2, 40, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 50, 50, 50, 252, 196, 196, 196, 68, 0, 0, 0, 0, 42, 0, 0, 0, 0, 4, 255, 255, 255, 15, 102, 102, 102, 190, 5, 5, 5, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 8, 1, 0, 0, 255, 146, 0, 0, 255, 235, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 2, 235, 0, 0, 255, 13, 233, 0, 0, 255, 232, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 219, 0, 0, 255, 214, 0, 0, 255, 210, 0, 0, 255, 209, 0, 0, 255, 157, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 84, 84, 84, 233, 247, 247, 247, 18, 0, 0, 0, 0, 137, 0, 0, 0, 0, 3, 246, 246, 246, 18, 83, 83, 83, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 9, 0, 0, 255, 161, 0, 0, 255, 213, 0, 0, 255, 1, 218, 0, 0, 255, 9, 222, 0, 0, 255, 226, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 1, 237, 0, 0, 255, 8, 236, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 236, 0, 0, 255, 146, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 5, 5, 5, 255, 104, 104, 104, 189, 255, 255, 255, 15, 0, 0, 0, 0, 41, 0, 0, 0, 0, 3, 199, 199, 199, 129, 42, 42, 42, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 59, 0, 0, 255, 224, 0, 0, 255, 232, 0, 0, 255, 1, 234, 0, 0, 255, 3, 236, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 1, 237, 0, 0, 255, 13, 236, 0, 0, 255, 235, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 220, 0, 0, 255, 216, 0, 0, 255, 212, 0, 0, 255, 205, 0, 0, 255, 64, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 28, 28, 28, 255, 171, 171, 171, 127, 0, 0, 0, 0, 139, 0, 0, 0, 0, 3, 171, 171, 171, 128, 27, 27, 27, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 67, 0, 0, 255, 209, 0, 0, 255, 215, 0, 0, 255, 219, 0, 0, 255, 223, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 1, 239, 0, 0, 255, 8, 238, 0, 0, 255, 237, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 233, 0, 0, 255, 225, 0, 0, 255, 58, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 43, 43, 43, 255, 199, 199, 199, 128, 0, 0, 0, 0, 40, 0, 0, 0, 0, 3, 255, 255, 255, 22, 105, 105, 105, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 5, 0, 0, 255, 169, 0, 0, 255, 238, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 2, 238, 0, 0, 255, 12, 237, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 216, 0, 0, 255, 217, 0, 0, 255, 147, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 109, 109, 109, 233, 255, 255, 255, 22, 0, 0, 0, 0, 139, 0, 0, 0, 0, 3, 255, 255, 255, 22, 108, 108, 108, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 4, 0, 0, 255, 151, 0, 0, 255, 220, 0, 0, 255, 219, 0, 0, 255, 223, 0, 0, 255, 227, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 1, 241, 0, 0, 255, 8, 240, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 239, 0, 0, 255, 169, 0, 0, 255, 5, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 106, 106, 106, 233, 255, 255, 255, 22, 0, 0, 0, 0, 39, 0, 0, 0, 0, 3, 166, 166, 166, 128, 27, 27, 27, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 80, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 2, 241, 0, 0, 255, 12, 240, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 217, 0, 0, 255, 206, 0, 0, 255, 50, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 44, 44, 44, 255, 202, 202, 202, 128, 0, 0, 0, 0, 141, 0, 0, 0, 0, 3, 202, 202, 202, 129, 44, 44, 44, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 52, 0, 0, 255, 209, 0, 0, 255, 220, 0, 0, 255, 224, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 1, 242, 0, 0, 255, 1, 244, 0, 0, 255, 1, 243, 0, 0, 255, 5, 242, 0, 0, 255, 241, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 1, 79, 0, 0, 255, 1, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 28, 28, 28, 255, 167, 167, 167, 127, 0, 0, 0, 0, 38, 0, 0, 0, 0, 3, 241, 241, 241, 18, 80, 80, 80, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 13, 0, 0, 255, 190, 0, 0, 255, 239, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 1, 243, 0, 0, 255, 1, 244, 0, 0, 255, 1, 243, 0, 0, 255, 13, 242, 0, 0, 255, 241, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 134, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 6, 6, 6, 255, 107, 107, 107, 189, 255, 255, 255, 14, 0, 0, 0, 0, 141, 0, 0, 0, 0, 4, 255, 255, 255, 15, 106, 106, 106, 189, 6, 6, 6, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 13, 1, 0, 0, 255, 137, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 228, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 1, 245, 0, 0, 255, 1, 244, 0, 0, 255, 1, 242, 0, 0, 255, 6, 240, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 190, 0, 0, 255, 12, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 81, 81, 81, 233, 243, 243, 243, 18, 0, 0, 0, 0, 36, 0, 0, 0, 0, 4, 255, 255, 255, 3, 163, 163, 163, 175, 14, 14, 14, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 103, 0, 0, 255, 240, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 2, 245, 0, 0, 255, 12, 244, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 203, 0, 0, 255, 36, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 52, 52, 52, 252, 201, 201, 201, 67, 0, 0, 0, 0, 143, 0, 0, 0, 0, 3, 200, 200, 200, 68, 53, 53, 53, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 37, 0, 0, 255, 205, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 2, 247, 0, 0, 255, 1, 246, 0, 0, 255, 1, 244, 0, 0, 255, 6, 242, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 102, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 14, 14, 14, 255, 164, 164, 164, 175, 255, 255, 255, 3, 0, 0, 0, 0, 35, 0, 0, 0, 0, 3, 234, 234, 234, 68, 75, 75, 75, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 24, 0, 0, 255, 207, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 2, 247, 0, 0, 255, 12, 246, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 117, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 9, 9, 9, 255, 135, 135, 135, 187, 255, 255, 255, 3, 0, 0, 0, 0, 143, 0, 0, 0, 0, 4, 255, 255, 255, 3, 134, 134, 134, 187, 9, 9, 9, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 120, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 232, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 1, 247, 0, 0, 255, 1, 248, 0, 0, 255, 1, 247, 0, 0, 255, 8, 246, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 207, 0, 0, 255, 24, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 76, 76, 76, 253, 234, 234, 234, 68, 0, 0, 0, 0, 34, 0, 0, 0, 0, 4, 255, 255, 255, 3, 139, 139, 139, 188, 9, 9, 9, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 125, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 2, 248, 0, 0, 255, 12, 247, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 229, 0, 0, 255, 196, 0, 0, 255, 23, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 73, 73, 73, 253, 231, 231, 231, 67, 0, 0, 0, 0, 145, 0, 0, 0, 0, 3, 229, 229, 229, 68, 72, 72, 72, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 24, 0, 0, 255, 199, 0, 0, 255, 232, 0, 0, 255, 231, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 1, 249, 0, 0, 255, 1, 248, 0, 0, 255, 1, 246, 0, 0, 255, 6, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 244, 0, 0, 255, 124, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 9, 9, 9, 255, 141, 141, 141, 187, 255, 255, 255, 3, 0, 0, 0, 0, 33, 0, 0, 0, 0, 3, 208, 208, 208, 68, 54, 54, 54, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 39, 0, 0, 255, 220, 0, 0, 255, 240, 0, 0, 255, 1, 244, 0, 0, 255, 4, 245, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 1, 249, 0, 0, 255, 1, 248, 0, 0, 255, 1, 246, 0, 0, 255, 9, 245, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 233, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 100, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 12, 12, 12, 255, 158, 158, 158, 174, 255, 255, 255, 3, 0, 0, 0, 0, 145, 0, 0, 0, 0, 4, 255, 255, 255, 3, 158, 158, 158, 175, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 101, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 1, 250, 0, 0, 255, 9, 249, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 241, 0, 0, 255, 240, 0, 0, 255, 220, 0, 0, 255, 38, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 55, 55, 55, 252, 208, 208, 208, 68, 0, 0, 0, 0, 32, 0, 0, 0, 0, 4, 255, 255, 255, 15, 111, 111, 111, 190, 6, 6, 6, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 148, 0, 0, 255, 245, 0, 0, 255, 239, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 2, 251, 0, 0, 255, 12, 249, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 185, 0, 0, 255, 13, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 77, 77, 77, 233, 233, 233, 233, 18, 0, 0, 0, 0, 147, 0, 0, 0, 0, 3, 234, 234, 234, 18, 76, 76, 76, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 14, 0, 0, 255, 188, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 251, 0, 0, 255, 9, 250, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 245, 0, 0, 255, 147, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 6, 6, 6, 255, 112, 112, 112, 189, 255, 255, 255, 15, 0, 0, 0, 0, 31, 0, 0, 0, 0, 3, 207, 207, 207, 129, 46, 46, 46, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 56, 0, 0, 255, 230, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 251, 0, 0, 255, 11, 250, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 81, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 25, 25, 25, 255, 162, 162, 162, 127, 0, 0, 0, 0, 149, 0, 0, 0, 0, 3, 162, 162, 162, 128, 25, 25, 25, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 83, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 2, 252, 0, 0, 255, 9, 250, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 230, 0, 0, 255, 56, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 47, 47, 47, 255, 206, 206, 206, 128, 0, 0, 0, 0, 30, 0, 0, 0, 0, 3, 255, 255, 255, 22, 112, 112, 112, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 4, 0, 0, 255, 168, 0, 0, 255, 245, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 2, 252, 0, 0, 255, 11, 251, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 171, 0, 0, 255, 6, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 101, 101, 101, 233, 255, 255, 255, 22, 0, 0, 0, 0, 149, 0, 0, 0, 0, 3, 255, 255, 255, 22, 101, 101, 101, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 7, 0, 0, 255, 173, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 4, 252, 0, 0, 255, 9, 250, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 244, 0, 0, 255, 168, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 113, 113, 113, 233, 255, 255, 255, 22, 0, 0, 0, 0, 29, 0, 0, 0, 0, 3, 176, 176, 176, 128, 30, 30, 30, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 75, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 3, 252, 0, 0, 255, 10, 251, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 229, 0, 0, 255, 62, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 40, 40, 40, 255, 195, 195, 195, 128, 0, 0, 0, 0, 151, 0, 0, 0, 0, 3, 194, 194, 194, 129, 40, 40, 40, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 64, 0, 0, 255, 230, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 251, 0, 0, 255, 8, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 74, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 30, 30, 30, 255, 176, 176, 176, 127, 0, 0, 0, 0, 28, 0, 0, 0, 0, 3, 255, 255, 255, 22, 87, 87, 87, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 10, 0, 0, 255, 187, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 2, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 251, 0, 0, 255, 9, 248, 0, 0, 255, 247, 0, 0, 255, 244, 0, 0, 255, 241, 0, 0, 255, 237, 0, 0, 255, 242, 0, 0, 255, 154, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 5, 5, 5, 255, 97, 97, 97, 189, 255, 255, 255, 14, 0, 0, 0, 0, 151, 0, 0, 0, 0, 4, 255, 255, 255, 15, 97, 97, 97, 189, 5, 5, 5, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 11, 2, 0, 0, 255, 155, 0, 0, 255, 242, 0, 0, 255, 238, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 253, 0, 0, 255, 1, 253, 0, 0, 255, 11, 252, 0, 0, 255, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 184, 0, 0, 255, 10, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 88, 88, 88, 233, 255, 255, 255, 22, 0, 0, 0, 0, 27, 0, 0, 0, 0, 3, 140, 140, 140, 127, 20, 20, 20, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 96, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 241, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 253, 0, 0, 255, 1, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 250, 0, 0, 255, 8, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 223, 0, 0, 255, 46, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 47, 47, 47, 252, 189, 189, 189, 67, 0, 0, 0, 0, 153, 0, 0, 0, 0, 3, 187, 187, 187, 68, 47, 47, 47, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 45, 0, 0, 255, 224, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 2, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 251, 0, 0, 255, 8, 249, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 241, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 96, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 20, 20, 20, 255, 140, 140, 140, 126, 0, 0, 0, 0, 26, 0, 0, 0, 0, 3, 240, 240, 240, 66, 73, 73, 73, 241, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 21, 0, 0, 255, 202, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 253, 0, 0, 255, 2, 252, 0, 0, 255, 9, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 243, 0, 0, 255, 133, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 8, 8, 8, 255, 127, 127, 127, 187, 255, 255, 255, 3, 0, 0, 0, 0, 153, 0, 0, 0, 0, 4, 255, 255, 255, 3, 125, 125, 125, 187, 8, 8, 8, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 135, 0, 0, 255, 244, 0, 0, 255, 239, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 251, 0, 0, 255, 9, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 199, 0, 0, 255, 21, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 73, 73, 73, 240, 240, 240, 240, 66, 0, 0, 0, 0, 24, 0, 0, 0, 0, 4, 255, 255, 255, 3, 148, 148, 148, 188, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 117, 0, 0, 255, 241, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 2, 253, 0, 0, 255, 252, 0, 0, 255, 1, 250, 0, 0, 255, 4, 249, 0, 0, 255, 247, 0, 0, 255, 244, 0, 0, 255, 241, 0, 0, 255, 1, 213, 0, 0, 255, 2, 29, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 66, 66, 66, 252, 223, 223, 223, 67, 0, 0, 0, 0, 155, 0, 0, 0, 0, 3, 223, 223, 223, 68, 65, 65, 65, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 30, 0, 0, 255, 213, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 3, 252, 0, 0, 255, 10, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 116, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 12, 12, 12, 255, 150, 150, 150, 187, 255, 255, 255, 3, 0, 0, 0, 0, 23, 0, 0, 0, 0, 3, 218, 218, 218, 68, 59, 59, 59, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 34, 0, 0, 255, 213, 0, 0, 255, 237, 0, 0, 255, 1, 241, 0, 0, 255, 5, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 3, 252, 0, 0, 255, 9, 251, 0, 0, 255, 250, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 243, 0, 0, 255, 113, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 13, 13, 13, 255, 156, 156, 156, 187, 255, 255, 255, 3, 0, 0, 0, 0, 155, 0, 0, 0, 0, 4, 255, 255, 255, 3, 157, 157, 157, 187, 13, 13, 13, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 113, 0, 0, 255, 244, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 2, 252, 0, 0, 255, 11, 251, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 236, 0, 0, 255, 235, 0, 0, 255, 210, 0, 0, 255, 32, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 60, 60, 60, 252, 217, 217, 217, 68, 0, 0, 0, 0, 22, 0, 0, 0, 0, 4, 255, 255, 255, 2, 118, 118, 118, 188, 7, 7, 7, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 137, 0, 0, 255, 239, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 10, 251, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 199, 0, 0, 255, 18, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 79, 79, 79, 240, 245, 245, 245, 65, 0, 0, 0, 0, 157, 0, 0, 0, 0, 3, 245, 245, 245, 66, 78, 78, 78, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 18, 0, 0, 255, 199, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 1, 251, 0, 0, 255, 11, 250, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 135, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 7, 7, 7, 255, 118, 118, 118, 187, 255, 255, 255, 2, 0, 0, 0, 0, 21, 0, 0, 0, 0, 3, 190, 190, 190, 81, 41, 41, 41, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 48, 0, 0, 255, 220, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 2, 251, 0, 0, 255, 9, 250, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 91, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 22, 22, 22, 255, 151, 151, 151, 126, 0, 0, 0, 0, 159, 0, 0, 0, 0, 3, 150, 150, 150, 127, 22, 22, 22, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 92, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 1, 250, 0, 0, 255, 12, 249, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 217, 0, 0, 255, 47, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 41, 41, 41, 252, 188, 188, 188, 80, 0, 0, 0, 0, 20, 0, 0, 0, 0, 3, 255, 255, 255, 22, 121, 121, 121, 237, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 3, 0, 0, 255, 155, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 1, 250, 0, 0, 255, 9, 248, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 180, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 94, 94, 94, 233, 255, 255, 255, 22, 0, 0, 0, 0, 159, 0, 0, 0, 0, 3, 255, 255, 255, 22, 94, 94, 94, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 8, 0, 0, 255, 181, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 1, 248, 0, 0, 255, 12, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 233, 0, 0, 255, 152, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 122, 122, 122, 237, 255, 255, 255, 22, 0, 0, 0, 0, 19, 0, 0, 0, 0, 3, 184, 184, 184, 128, 33, 33, 33, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 64, 0, 0, 255, 223, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 1, 249, 0, 0, 255, 1, 248, 0, 0, 255, 1, 246, 0, 0, 255, 7, 245, 0, 0, 255, 244, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 234, 0, 0, 255, 69, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 34, 34, 34, 255, 186, 186, 186, 127, 0, 0, 0, 0, 161, 0, 0, 0, 0, 3, 185, 185, 185, 128, 33, 33, 33, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 70, 0, 0, 255, 234, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 3, 247, 0, 0, 255, 11, 246, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 220, 0, 0, 255, 63, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 34, 34, 34, 255, 185, 185, 185, 127, 0, 0, 0, 0, 18, 0, 0, 0, 0, 3, 255, 255, 255, 22, 94, 94, 94, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 7, 0, 0, 255, 169, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 1, 248, 0, 0, 255, 1, 247, 0, 0, 255, 1, 246, 0, 0, 255, 8, 245, 0, 0, 255, 244, 0, 0, 255, 241, 0, 0, 255, 238, 0, 0, 255, 243, 0, 0, 255, 161, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 1, 1, 1, 255, 122, 122, 122, 237, 255, 255, 255, 22, 0, 0, 0, 0, 161, 0, 0, 0, 0, 3, 255, 255, 255, 22, 122, 122, 122, 237, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 3, 0, 0, 255, 162, 0, 0, 255, 242, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 3, 246, 0, 0, 255, 12, 244, 0, 0, 255, 243, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 227, 0, 0, 255, 166, 0, 0, 255, 7, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 94, 94, 94, 233, 255, 255, 255, 22, 0, 0, 0, 0, 17, 0, 0, 0, 0, 3, 149, 149, 149, 127, 22, 22, 22, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 83, 0, 0, 255, 223, 0, 0, 255, 224, 0, 0, 255, 229, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 2, 246, 0, 0, 255, 4, 245, 0, 0, 255, 243, 0, 0, 255, 241, 0, 0, 255, 238, 0, 0, 255, 1, 225, 0, 0, 255, 2, 50, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 41, 41, 41, 252, 189, 189, 189, 80, 0, 0, 0, 0, 163, 0, 0, 0, 0, 3, 187, 187, 187, 80, 41, 41, 41, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 50, 0, 0, 255, 225, 0, 0, 255, 237, 0, 0, 255, 1, 240, 0, 0, 255, 4, 242, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 1, 244, 0, 0, 255, 12, 243, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 80, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 22, 22, 22, 255, 151, 151, 151, 126, 0, 0, 0, 0, 16, 0, 0, 0, 0, 3, 244, 244, 244, 66, 79, 79, 79, 241, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 16, 0, 0, 255, 181, 0, 0, 255, 224, 0, 0, 255, 1, 229, 0, 0, 255, 6, 231, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 2, 245, 0, 0, 255, 2, 244, 0, 0, 255, 243, 0, 0, 255, 1, 241, 0, 0, 255, 5, 238, 0, 0, 255, 236, 0, 0, 255, 241, 0, 0, 255, 139, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 7, 7, 7, 255, 117, 117, 117, 187, 255, 255, 255, 2, 0, 0, 0, 0, 163, 0, 0, 0, 0, 4, 255, 255, 255, 2, 116, 116, 116, 187, 7, 7, 7, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 139, 0, 0, 255, 240, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 3, 242, 0, 0, 255, 8, 241, 0, 0, 255, 239, 0, 0, 255, 238, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 221, 0, 0, 255, 1, 177, 0, 0, 255, 2, 15, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 80, 80, 80, 240, 244, 244, 244, 66, 0, 0, 0, 0, 14, 0, 0, 0, 0, 4, 255, 255, 255, 3, 157, 157, 157, 188, 13, 13, 13, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 100, 0, 0, 255, 220, 0, 0, 255, 1, 224, 0, 0, 255, 7, 228, 0, 0, 255, 231, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 3, 242, 0, 0, 255, 8, 241, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 213, 0, 0, 255, 33, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 60, 60, 60, 252, 216, 216, 216, 67, 0, 0, 0, 0, 165, 0, 0, 0, 0, 3, 217, 217, 217, 68, 59, 59, 59, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 34, 0, 0, 255, 212, 0, 0, 255, 235, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 1, 241, 0, 0, 255, 13, 240, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 229, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 216, 0, 0, 255, 217, 0, 0, 255, 97, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 13, 13, 13, 255, 158, 158, 158, 187, 255, 255, 255, 3, 0, 0, 0, 0, 13, 0, 0, 0, 0, 3, 225, 225, 225, 68, 65, 65, 65, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 25, 0, 0, 255, 188, 0, 0, 255, 217, 0, 0, 255, 219, 0, 0, 255, 223, 0, 0, 255, 227, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 2, 240, 0, 0, 255, 7, 239, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 238, 0, 0, 255, 116, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 12, 12, 12, 255, 148, 148, 148, 187, 255, 255, 255, 3, 0, 0, 0, 0, 165, 0, 0, 0, 0, 4, 255, 255, 255, 3, 149, 149, 149, 187, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 115, 0, 0, 255, 237, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 1, 238, 0, 0, 255, 13, 237, 0, 0, 255, 236, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 216, 0, 0, 255, 215, 0, 0, 255, 184, 0, 0, 255, 24, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 66, 66, 66, 252, 224, 224, 224, 68, 0, 0, 0, 0, 12, 0, 0, 0, 0, 4, 255, 255, 255, 3, 127, 127, 127, 188, 8, 8, 8, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 115, 0, 0, 255, 216, 0, 0, 255, 214, 0, 0, 255, 219, 0, 0, 255, 223, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 1, 238, 0, 0, 255, 1, 239, 0, 0, 255, 1, 237, 0, 0, 255, 7, 236, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 196, 0, 0, 255, 20, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 73, 73, 73, 240, 241, 241, 241, 65, 0, 0, 0, 0, 167, 0, 0, 0, 0, 3, 239, 239, 239, 66, 72, 72, 72, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 21, 0, 0, 255, 196, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 4, 236, 0, 0, 255, 12, 235, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 223, 0, 0, 255, 220, 0, 0, 255, 216, 0, 0, 255, 211, 0, 0, 255, 212, 0, 0, 255, 112, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 8, 8, 8, 255, 126, 126, 126, 187, 255, 255, 255, 3, 0, 0, 0, 0, 11, 0, 0, 0, 0, 3, 189, 189, 189, 68, 46, 46, 46, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 37, 0, 0, 255, 192, 0, 0, 255, 210, 0, 0, 255, 214, 0, 0, 255, 218, 0, 0, 255, 222, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 1, 237, 0, 0, 255, 1, 236, 0, 0, 255, 1, 235, 0, 0, 255, 6, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 92, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 19, 19, 19, 255, 141, 141, 141, 126, 0, 0, 0, 0, 169, 0, 0, 0, 0, 3, 140, 140, 140, 127, 20, 20, 20, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 92, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 1, 234, 0, 0, 255, 14, 233, 0, 0, 255, 232, 0, 0, 255, 231, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 223, 0, 0, 255, 219, 0, 0, 255, 215, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 187, 0, 0, 255, 34, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 48, 48, 48, 252, 188, 188, 188, 68, 0, 0, 0, 0, 10, 0, 0, 0, 0, 4, 255, 255, 255, 15, 97, 97, 97, 190, 5, 5, 5, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 1, 0, 0, 255, 129, 0, 0, 255, 208, 0, 0, 255, 1, 213, 0, 0, 255, 9, 217, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 1, 234, 0, 0, 255, 8, 233, 0, 0, 255, 232, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 232, 0, 0, 255, 177, 0, 0, 255, 10, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 87, 87, 87, 233, 254, 254, 254, 22, 0, 0, 0, 0, 169, 0, 0, 0, 0, 3, 254, 254, 254, 22, 87, 87, 87, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 9, 0, 0, 255, 176, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 229, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 232, 0, 0, 255, 1, 232, 0, 0, 255, 14, 231, 0, 0, 255, 230, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 214, 0, 0, 255, 210, 0, 0, 255, 205, 0, 0, 255, 204, 0, 0, 255, 124, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 5, 5, 5, 255, 99, 99, 99, 189, 255, 255, 255, 15, 0, 0, 0, 0, 9, 0, 0, 0, 0, 3, 193, 193, 193, 129, 40, 40, 40, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 16, 48, 0, 0, 255, 192, 0, 0, 255, 203, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 216, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 231, 0, 0, 255, 232, 0, 0, 255, 231, 0, 0, 255, 1, 229, 0, 0, 255, 5, 227, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 70, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 30, 30, 30, 255, 177, 177, 177, 127, 0, 0, 0, 0, 171, 0, 0, 0, 0, 3, 175, 175, 175, 128, 30, 30, 30, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 69, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 228, 0, 0, 255, 229, 0, 0, 255, 1, 229, 0, 0, 255, 2, 228, 0, 0, 255, 227, 0, 0, 255, 1, 224, 0, 0, 255, 10, 223, 0, 0, 255, 219, 0, 0, 255, 217, 0, 0, 255, 213, 0, 0, 255, 208, 0, 0, 255, 203, 0, 0, 255, 198, 0, 0, 255, 187, 0, 0, 255, 46, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 41, 41, 41, 255, 194, 194, 194, 128, 0, 0, 0, 0, 8, 0, 0, 0, 0, 3, 255, 255, 255, 22, 101, 101, 101, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 4, 0, 0, 255, 137, 0, 0, 255, 200, 0, 0, 255, 201, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 215, 0, 0, 255, 219, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 227, 0, 0, 255, 228, 0, 0, 255, 229, 0, 0, 255, 1, 229, 0, 0, 255, 1, 228, 0, 0, 255, 1, 226, 0, 0, 255, 5, 223, 0, 0, 255, 228, 0, 0, 255, 156, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 113, 113, 113, 233, 255, 255, 255, 22, 0, 0, 0, 0, 171, 0, 0, 0, 0, 3, 255, 255, 255, 22, 112, 112, 112, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 3, 0, 0, 255, 154, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 225, 0, 0, 255, 226, 0, 0, 255, 227, 0, 0, 255, 1, 226, 0, 0, 255, 14, 225, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 208, 0, 0, 255, 202, 0, 0, 255, 196, 0, 0, 255, 194, 0, 0, 255, 133, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 101, 101, 101, 233, 255, 255, 255, 22, 0, 0, 0, 0, 7, 0, 0, 0, 0, 3, 160, 160, 160, 127, 25, 25, 25, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 60, 0, 0, 255, 187, 0, 0, 255, 195, 0, 0, 255, 200, 0, 0, 255, 205, 0, 0, 255, 210, 0, 0, 255, 214, 0, 0, 255, 216, 0, 0, 255, 220, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 2, 226, 0, 0, 255, 9, 227, 0, 0, 255, 226, 0, 0, 255, 225, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 221, 0, 0, 255, 211, 0, 0, 255, 50, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 48, 48, 48, 255, 206, 206, 206, 128, 0, 0, 0, 0, 173, 0, 0, 0, 0, 3, 206, 206, 206, 129, 46, 46, 46, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 50, 0, 0, 255, 209, 0, 0, 255, 219, 0, 0, 255, 220, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 3, 223, 0, 0, 255, 13, 221, 0, 0, 255, 220, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 207, 0, 0, 255, 202, 0, 0, 255, 196, 0, 0, 255, 190, 0, 0, 255, 182, 0, 0, 255, 58, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 25, 25, 25, 255, 161, 161, 161, 127, 255, 255, 255, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 243, 243, 243, 72, 85, 85, 85, 241, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 9, 0, 0, 255, 142, 0, 0, 255, 190, 0, 0, 255, 193, 0, 0, 255, 200, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 221, 0, 0, 255, 222, 0, 0, 255, 223, 0, 0, 255, 2, 223, 0, 0, 255, 6, 222, 0, 0, 255, 220, 0, 0, 255, 218, 0, 0, 255, 223, 0, 0, 255, 132, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 6, 6, 6, 255, 111, 111, 111, 189, 255, 255, 255, 14, 0, 0, 0, 0, 173, 0, 0, 0, 0, 4, 255, 255, 255, 15, 111, 111, 111, 189, 6, 6, 6, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 130, 0, 0, 255, 220, 0, 0, 255, 215, 0, 0, 255, 217, 0, 0, 255, 219, 0, 0, 255, 220, 0, 0, 255, 221, 0, 0, 255, 220, 0, 0, 255, 1, 219, 0, 0, 255, 13, 218, 0, 0, 255, 216, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 208, 0, 0, 255, 205, 0, 0, 255, 201, 0, 0, 255, 195, 0, 0, 255, 188, 0, 0, 255, 184, 0, 0, 255, 135, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 99, 99, 99, 232, 255, 255, 255, 16, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 169, 169, 169, 133, 24, 24, 24, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 15, 71, 0, 0, 255, 179, 0, 0, 255, 185, 0, 0, 255, 193, 0, 0, 255, 199, 0, 0, 255, 203, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 213, 0, 0, 255, 216, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 220, 0, 0, 255, 221, 0, 0, 255, 220, 0, 0, 255, 1, 219, 0, 0, 255, 2, 218, 0, 0, 255, 216, 0, 0, 255, 1, 197, 0, 0, 255, 2, 32, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 55, 55, 55, 252, 206, 206, 206, 67, 0, 0, 0, 0, 175, 0, 0, 0, 0, 3, 207, 207, 207, 68, 55, 55, 55, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 33, 0, 0, 255, 193, 0, 0, 255, 213, 0, 0, 255, 1, 215, 0, 0, 255, 18, 216, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 217, 0, 0, 255, 216, 0, 0, 255, 215, 0, 0, 255, 214, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 207, 0, 0, 255, 204, 0, 0, 255, 199, 0, 0, 255, 194, 0, 0, 255, 187, 0, 0, 255, 179, 0, 0, 255, 172, 0, 0, 255, 64, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 74, 74, 74, 241, 255, 255, 255, 17, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 116, 116, 116, 128, 13, 13, 13, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 12, 13, 0, 0, 255, 141, 0, 0, 255, 178, 0, 0, 255, 184, 0, 0, 255, 192, 0, 0, 255, 197, 0, 0, 255, 202, 0, 0, 255, 206, 0, 0, 255, 209, 0, 0, 255, 211, 0, 0, 255, 213, 0, 0, 255, 215, 0, 0, 255, 1, 217, 0, 0, 255, 1, 218, 0, 0, 255, 1, 217, 0, 0, 255, 6, 215, 0, 0, 255, 214, 0, 0, 255, 212, 0, 0, 255, 216, 0, 0, 255, 108, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 9, 9, 9, 255, 140, 140, 140, 187, 255, 255, 255, 3, 0, 0, 0, 0, 175, 0, 0, 0, 0, 4, 255, 255, 255, 3, 140, 140, 140, 187, 9, 9, 9, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 106, 0, 0, 255, 212, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 212, 0, 0, 255, 213, 0, 0, 255, 214, 0, 0, 255, 1, 213, 0, 0, 255, 13, 212, 0, 0, 255, 211, 0, 0, 255, 209, 0, 0, 255, 208, 0, 0, 255, 205, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 193, 0, 0, 255, 187, 0, 0, 255, 179, 0, 0, 255, 173, 0, 0, 255, 69, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 110, 110, 110, 231, 255, 255, 255, 16, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 185, 185, 185, 133, 28, 28, 28, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 14, 13, 0, 0, 255, 145, 0, 0, 255, 177, 0, 0, 255, 183, 0, 0, 255, 190, 0, 0, 255, 196, 0, 0, 255, 199, 0, 0, 255, 204, 0, 0, 255, 206, 0, 0, 255, 209, 0, 0, 255, 210, 0, 0, 255, 212, 0, 0, 255, 213, 0, 0, 255, 214, 0, 0, 255, 1, 214, 0, 0, 255, 7, 213, 0, 0, 255, 211, 0, 0, 255, 209, 0, 0, 255, 211, 0, 0, 255, 178, 0, 0, 255, 20, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 76, 76, 76, 253, 235, 235, 235, 67, 0, 0, 0, 0, 177, 0, 0, 0, 0, 3, 234, 234, 234, 68, 75, 75, 75, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 19, 0, 0, 255, 176, 0, 0, 255, 207, 0, 0, 255, 206, 0, 0, 255, 208, 0, 0, 255, 209, 0, 0, 255, 210, 0, 0, 255, 2, 210, 0, 0, 255, 12, 208, 0, 0, 255, 207, 0, 0, 255, 205, 0, 0, 255, 202, 0, 0, 255, 200, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 185, 0, 0, 255, 179, 0, 0, 255, 163, 0, 0, 255, 33, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 13, 13, 13, 255, 120, 120, 120, 136, 255, 255, 255, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 235, 235, 235, 72, 57, 57, 57, 246, 0, 0, 0, 255, 2, 0, 0, 0, 255, 13, 1, 0, 0, 255, 114, 0, 0, 255, 179, 0, 0, 255, 182, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 206, 0, 0, 255, 208, 0, 0, 255, 209, 0, 0, 255, 210, 0, 0, 255, 2, 210, 0, 0, 255, 6, 209, 0, 0, 255, 207, 0, 0, 255, 205, 0, 0, 255, 206, 0, 0, 255, 85, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 14, 14, 14, 255, 165, 165, 165, 174, 255, 255, 255, 3, 0, 0, 0, 0, 177, 0, 0, 0, 0, 4, 255, 255, 255, 3, 163, 163, 163, 175, 14, 14, 14, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 84, 0, 0, 255, 202, 0, 0, 255, 201, 0, 0, 255, 203, 0, 0, 255, 205, 0, 0, 255, 206, 0, 0, 255, 207, 0, 0, 255, 1, 207, 0, 0, 255, 12, 205, 0, 0, 255, 204, 0, 0, 255, 202, 0, 0, 255, 200, 0, 0, 255, 197, 0, 0, 255, 194, 0, 0, 255, 189, 0, 0, 255, 184, 0, 0, 255, 180, 0, 0, 255, 142, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 34, 34, 34, 254, 201, 201, 201, 118, 0, 0, 0, 0, 6, 0, 0, 0, 0, 3, 255, 255, 255, 13, 107, 107, 107, 242, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 77, 0, 0, 255, 179, 0, 0, 255, 181, 0, 0, 255, 186, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 198, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 1, 206, 0, 0, 255, 1, 207, 0, 0, 255, 1, 207, 0, 0, 255, 7, 206, 0, 0, 255, 204, 0, 0, 255, 202, 0, 0, 255, 204, 0, 0, 255, 157, 0, 0, 255, 10, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 80, 80, 80, 233, 241, 241, 241, 18, 0, 0, 0, 0, 179, 0, 0, 0, 0, 3, 241, 241, 241, 18, 80, 80, 80, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 9, 0, 0, 255, 154, 0, 0, 255, 200, 0, 0, 255, 198, 0, 0, 255, 200, 0, 0, 255, 202, 0, 0, 255, 203, 0, 0, 255, 1, 203, 0, 0, 255, 8, 202, 0, 0, 255, 201, 0, 0, 255, 199, 0, 0, 255, 197, 0, 0, 255, 194, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 181, 0, 0, 255, 1, 106, 0, 0, 255, 1, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 62, 62, 62, 240, 230, 230, 230, 25, 0, 0, 0, 0, 6, 0, 0, 0, 0, 4, 255, 255, 255, 8, 142, 142, 142, 184, 7, 7, 7, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 11, 41, 0, 0, 255, 169, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 196, 0, 0, 255, 199, 0, 0, 255, 201, 0, 0, 255, 202, 0, 0, 255, 203, 0, 0, 255, 1, 203, 0, 0, 255, 7, 202, 0, 0, 255, 201, 0, 0, 255, 199, 0, 0, 255, 197, 0, 0, 255, 194, 0, 0, 255, 62, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 27, 27, 27, 255, 168, 168, 168, 127, 0, 0, 0, 0, 181, 0, 0, 0, 0, 3, 166, 166, 166, 128, 27, 27, 27, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 61, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 195, 0, 0, 255, 197, 0, 0, 255, 198, 0, 0, 255, 199, 0, 0, 255, 2, 198, 0, 0, 255, 9, 196, 0, 0, 255, 194, 0, 0, 255, 191, 0, 0, 255, 188, 0, 0, 255, 185, 0, 0, 255, 180, 0, 0, 255, 178, 0, 0, 255, 68, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 117, 117, 117, 230, 255, 255, 255, 15, 0, 0, 0, 0, 7, 0, 0, 0, 0, 3, 185, 185, 185, 115, 29, 29, 29, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 11, 11, 0, 0, 255, 149, 0, 0, 255, 180, 0, 0, 255, 182, 0, 0, 255, 187, 0, 0, 255, 190, 0, 0, 255, 193, 0, 0, 255, 195, 0, 0, 255, 197, 0, 0, 255, 198, 0, 0, 255, 199, 0, 0, 255, 1, 199, 0, 0, 255, 7, 198, 0, 0, 255, 196, 0, 0, 255, 193, 0, 0, 255, 195, 0, 0, 255, 134, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 106, 106, 106, 233, 255, 255, 255, 22, 0, 0, 0, 0, 181, 0, 0, 0, 0, 3, 255, 255, 255, 22, 105, 105, 105, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 3, 0, 0, 255, 130, 0, 0, 255, 191, 0, 0, 255, 188, 0, 0, 255, 192, 0, 0, 255, 193, 0, 0, 255, 194, 0, 0, 255, 195, 0, 0, 255, 1, 194, 0, 0, 255, 9, 193, 0, 0, 255, 191, 0, 0, 255, 188, 0, 0, 255, 185, 0, 0, 255, 182, 0, 0, 255, 179, 0, 0, 255, 165, 0, 0, 255, 32, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 14, 14, 14, 255, 130, 130, 130, 136, 255, 255, 255, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 3, 218, 218, 218, 25, 55, 55, 55, 240, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 1, 0, 0, 255, 114, 0, 0, 255, 180, 0, 0, 255, 1, 184, 0, 0, 255, 6, 188, 0, 0, 255, 190, 0, 0, 255, 192, 0, 0, 255, 193, 0, 0, 255, 194, 0, 0, 255, 195, 0, 0, 255, 1, 194, 0, 0, 255, 6, 193, 0, 0, 255, 190, 0, 0, 255, 188, 0, 0, 255, 178, 0, 0, 255, 42, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 43, 43, 43, 255, 199, 199, 199, 128, 0, 0, 0, 0, 183, 0, 0, 0, 0, 3, 198, 198, 198, 129, 42, 42, 42, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 41, 0, 0, 255, 172, 0, 0, 255, 183, 0, 0, 255, 185, 0, 0, 255, 188, 0, 0, 255, 190, 0, 0, 255, 2, 190, 0, 0, 255, 5, 189, 0, 0, 255, 187, 0, 0, 255, 185, 0, 0, 255, 182, 0, 0, 255, 178, 0, 0, 255, 1, 140, 0, 0, 255, 2, 7, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 38, 38, 38, 254, 209, 209, 209, 118, 0, 0, 0, 0, 8, 0, 0, 0, 0, 3, 255, 255, 255, 15, 107, 107, 107, 230, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 74, 0, 0, 255, 177, 0, 0, 255, 1, 181, 0, 0, 255, 5, 184, 0, 0, 255, 186, 0, 0, 255, 188, 0, 0, 255, 189, 0, 0, 255, 190, 0, 0, 255, 1, 190, 0, 0, 255, 6, 189, 0, 0, 255, 186, 0, 0, 255, 183, 0, 0, 255, 184, 0, 0, 255, 109, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 5, 5, 5, 255, 102, 102, 102, 189, 255, 255, 255, 14, 0, 0, 0, 0, 183, 0, 0, 0, 0, 4, 255, 255, 255, 15, 102, 102, 102, 189, 5, 5, 5, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 104, 0, 0, 255, 179, 0, 0, 255, 178, 0, 0, 255, 182, 0, 0, 255, 184, 0, 0, 255, 185, 0, 0, 255, 1, 186, 0, 0, 255, 8, 185, 0, 0, 255, 183, 0, 0, 255, 181, 0, 0, 255, 179, 0, 0, 255, 175, 0, 0, 255, 177, 0, 0, 255, 102, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 69, 69, 69, 240, 239, 239, 239, 25, 0, 0, 0, 0, 8, 0, 0, 0, 0, 4, 255, 255, 255, 1, 112, 112, 112, 137, 12, 12, 12, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 17, 37, 0, 0, 255, 165, 0, 0, 255, 175, 0, 0, 255, 178, 0, 0, 255, 181, 0, 0, 255, 183, 0, 0, 255, 184, 0, 0, 255, 185, 0, 0, 255, 186, 0, 0, 255, 185, 0, 0, 255, 184, 0, 0, 255, 183, 0, 0, 255, 179, 0, 0, 255, 177, 0, 0, 255, 158, 0, 0, 255, 27, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 50, 50, 50, 252, 194, 194, 194, 67, 0, 0, 0, 0, 185, 0, 0, 0, 0, 3, 196, 196, 196, 68, 49, 49, 49, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 25, 0, 0, 255, 147, 0, 0, 255, 175, 0, 0, 255, 179, 0, 0, 255, 2, 180, 0, 0, 255, 6, 181, 0, 0, 255, 180, 0, 0, 255, 179, 0, 0, 255, 178, 0, 0, 255, 175, 0, 0, 255, 172, 0, 0, 255, 1, 63, 0, 0, 255, 1, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 124, 124, 124, 230, 255, 255, 255, 15, 0, 0, 0, 0, 9, 0, 0, 0, 0, 3, 195, 195, 195, 119, 32, 32, 32, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 8, 9, 0, 0, 255, 143, 0, 0, 255, 173, 0, 0, 255, 174, 0, 0, 255, 177, 0, 0, 255, 178, 0, 0, 255, 180, 0, 0, 255, 181, 0, 0, 255, 2, 180, 0, 0, 255, 5, 179, 0, 0, 255, 176, 0, 0, 255, 169, 0, 0, 255, 81, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 9, 9, 9, 255, 131, 131, 131, 187, 255, 255, 255, 3, 0, 0, 0, 0, 185, 0, 0, 0, 0, 4, 255, 255, 255, 3, 131, 131, 131, 187, 8, 8, 8, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 15, 28, 0, 0, 255, 72, 0, 0, 255, 116, 0, 0, 255, 153, 0, 0, 255, 173, 0, 0, 255, 180, 0, 0, 255, 179, 0, 0, 255, 177, 0, 0, 255, 174, 0, 0, 255, 172, 0, 0, 255, 170, 0, 0, 255, 169, 0, 0, 255, 156, 0, 0, 255, 26, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 15, 15, 15, 255, 138, 138, 138, 136, 255, 255, 255, 1, 0, 0, 0, 0, 9, 0, 0, 0, 0, 3, 229, 229, 229, 25, 60, 60, 60, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 15, 106, 0, 0, 255, 172, 0, 0, 255, 169, 0, 0, 255, 171, 0, 0, 255, 174, 0, 0, 255, 176, 0, 0, 255, 178, 0, 0, 255, 180, 0, 0, 255, 178, 0, 0, 255, 164, 0, 0, 255, 134, 0, 0, 255, 91, 0, 0, 255, 49, 0, 0, 255, 5, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 70, 70, 70, 253, 229, 229, 229, 67, 0, 0, 0, 0, 187, 0, 0, 0, 0, 3, 227, 227, 227, 68, 70, 70, 70, 253, 0, 0, 0, 255, 6, 0, 0, 0, 255, 6, 16, 0, 0, 255, 53, 0, 0, 255, 93, 0, 0, 255, 136, 0, 0, 255, 162, 0, 0, 255, 173, 0, 0, 255, 2, 168, 0, 0, 255, 3, 129, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 42, 42, 42, 254, 216, 216, 216, 118, 0, 0, 0, 0, 10, 0, 0, 0, 0, 3, 255, 255, 255, 15, 112, 112, 112, 230, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 65, 0, 0, 255, 166, 0, 0, 255, 167, 0, 0, 255, 171, 0, 0, 255, 174, 0, 0, 255, 169, 0, 0, 255, 149, 0, 0, 255, 113, 0, 0, 255, 70, 0, 0, 255, 31, 0, 0, 255, 5, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 4, 11, 11, 11, 255, 156, 156, 156, 174, 255, 255, 255, 3, 0, 0, 0, 0, 187, 0, 0, 0, 0, 4, 255, 255, 255, 3, 155, 155, 155, 175, 12, 12, 12, 255, 0, 0, 0, 255, 8, 0, 0, 0, 255, 8, 5, 0, 0, 255, 30, 0, 0, 255, 68, 0, 0, 255, 109, 0, 0, 255, 142, 0, 0, 255, 163, 0, 0, 255, 91, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 75, 75, 75, 240, 247, 247, 247, 25, 0, 0, 0, 0, 10, 0, 0, 0, 0, 4, 255, 255, 255, 1, 123, 123, 123, 137, 13, 13, 13, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 8, 30, 0, 0, 255, 153, 0, 0, 255, 152, 0, 0, 255, 126, 0, 0, 255, 85, 0, 0, 255, 47, 0, 0, 255, 13, 0, 0, 255, 0, 0, 0, 255, 9, 0, 0, 0, 255, 3, 74, 74, 74, 233, 228, 228, 228, 18, 0, 0, 0, 0, 189, 0, 0, 0, 0, 3, 229, 229, 229, 18, 73, 73, 73, 233, 0, 0, 0, 255, 12, 0, 0, 0, 255, 4, 12, 0, 0, 255, 45, 0, 0, 255, 26, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 129, 129, 129, 230, 255, 255, 255, 15, 0, 0, 0, 0, 11, 0, 0, 0, 0, 3, 204, 204, 204, 119, 35, 35, 35, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 5, 5, 0, 0, 255, 49, 0, 0, 255, 25, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 11, 0, 0, 0, 255, 3, 23, 23, 23, 255, 158, 158, 158, 127, 0, 0, 0, 0, 191, 0, 0, 0, 0, 3, 164, 164, 164, 129, 42, 42, 42, 255, 0, 0, 0, 255, 18, 0, 0, 0, 255, 4, 17, 17, 17, 255, 148, 148, 148, 136, 255, 255, 255, 1, 0, 0, 0, 0, 11, 0, 0, 0, 0, 3, 235, 235, 235, 25, 65, 65, 65, 240, 0, 0, 0, 255, 18, 0, 0, 0, 255, 4, 5, 5, 5, 255, 115, 115, 115, 238, 255, 255, 255, 22, 0, 0, 0, 0, 191, 0, 0, 0, 0, 6, 255, 255, 255, 15, 146, 146, 146, 127, 73, 73, 73, 233, 40, 40, 40, 255, 14, 14, 14, 255, 0, 0, 0, 255, 15, 0, 0, 0, 255, 3, 47, 47, 47, 254, 223, 223, 223, 118, 0, 0, 0, 0, 12, 0, 0, 0, 0, 3, 255, 255, 255, 15, 120, 120, 120, 230, 0, 0, 0, 255, 15, 0, 0, 0, 255, 6, 4, 4, 4, 255, 23, 23, 23, 255, 52, 52, 52, 240, 106, 106, 106, 182, 228, 228, 228, 70, 0, 0, 0, 0, 194, 0, 0, 0, 0, 8, 255, 255, 255, 22, 211, 211, 211, 120, 120, 120, 120, 134, 107, 107, 107, 231, 49, 49, 49, 240, 24, 24, 24, 255, 4, 4, 4, 255, 0, 0, 0, 255, 11, 0, 0, 0, 255, 3, 83, 83, 83, 244, 252, 252, 252, 25, 0, 0, 0, 0, 12, 0, 0, 0, 0, 4, 255, 255, 255, 1, 134, 134, 134, 137, 15, 15, 15, 255, 0, 0, 0, 255, 11, 0, 0, 0, 255, 8, 16, 16, 16, 255, 34, 34, 34, 252, 83, 83, 83, 238, 120, 120, 120, 182, 168, 168, 168, 120, 204, 204, 204, 24, 255, 255, 255, 2, 0, 0, 0, 0, 198, 0, 0, 0, 0, 8, 255, 255, 255, 15, 197, 197, 197, 24, 169, 169, 169, 120, 121, 121, 121, 183, 83, 83, 83, 238, 36, 36, 36, 253, 16, 16, 16, 255, 0, 0, 0, 255, 7, 0, 0, 0, 255, 4, 4, 4, 4, 255, 107, 107, 107, 183, 255, 255, 255, 7, 0, 0, 0, 0, 13, 0, 0, 0, 0, 3, 212, 212, 212, 120, 39, 39, 39, 255, 0, 0, 0, 255, 7, 0, 0, 0, 255, 9, 6, 6, 6, 255, 28, 28, 28, 255, 57, 57, 57, 240, 116, 116, 116, 231, 138, 138, 138, 134, 196, 196, 196, 72, 255, 255, 255, 17, 255, 255, 255, 2, 0, 0, 0, 0, 204, 0, 0, 0, 0, 9, 255, 255, 255, 3, 255, 255, 255, 17, 198, 198, 198, 72, 138, 138, 138, 134, 116, 116, 116, 231, 58, 58, 58, 240, 28, 28, 28, 255, 6, 6, 6, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 29, 29, 29, 255, 163, 163, 163, 115, 0, 0, 0, 0, 14, 0, 0, 0, 0, 3, 243, 243, 243, 23, 83, 83, 83, 237, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 1, 1, 1, 255, 18, 18, 18, 255, 41, 41, 41, 252, 91, 91, 91, 238, 131, 131, 131, 182, 182, 182, 182, 120, 217, 217, 217, 24, 255, 255, 255, 15, 0, 0, 0, 0, 212, 0, 0, 0, 0, 12, 255, 255, 255, 15, 220, 220, 220, 24, 183, 183, 183, 120, 134, 134, 134, 183, 93, 93, 93, 238, 42, 42, 42, 253, 18, 18, 18, 255, 11, 11, 11, 255, 33, 33, 33, 255, 105, 105, 105, 177, 255, 255, 255, 16, 0, 0, 0, 0, 15, 0, 0, 0, 0, 12, 144, 144, 144, 78, 55, 55, 55, 245, 15, 15, 15, 255, 13, 13, 13, 255, 32, 32, 32, 255, 67, 67, 67, 240, 94, 94, 94, 182, 145, 145, 145, 127, 209, 209, 209, 72, 255, 255, 255, 17, 255, 255, 255, 2, 0, 0, 0, 0, 218, 0, 0, 0, 0, 8, 255, 255, 255, 3, 255, 255, 255, 17, 210, 210, 210, 72, 146, 146, 146, 127, 90, 90, 90, 127, 169, 169, 169, 120, 255, 255, 255, 10, 0, 0, 0, 0, 17, 0, 0, 0, 0, 7, 202, 202, 202, 70, 116, 116, 116, 127, 107, 107, 107, 127, 193, 193, 193, 120, 245, 245, 245, 24, 255, 255, 255, 2, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 141, 0, 0, 0, 0, 4, 255, 255, 255, 15, 193, 193, 193, 17, 159, 159, 159, 17, 162, 162, 162, 17, 55, 162, 162, 162, 17, 4, 159, 159, 159, 17, 255, 255, 255, 17, 255, 255, 255, 3, 0, 0, 0, 0, 65, 0, 0, 0, 0, 4, 255, 255, 255, 15, 177, 177, 177, 17, 160, 160, 160, 17, 162, 162, 162, 17, 54, 162, 162, 162, 17, 4, 160, 160, 160, 17, 164, 164, 164, 17, 255, 255, 255, 15, 0, 0, 0, 0, 61, 0, 0, 0, 0, 5, 219, 219, 219, 78, 125, 125, 125, 231, 52, 52, 52, 238, 43, 43, 43, 238, 44, 44, 44, 238, 55, 44, 44, 44, 238, 5, 43, 43, 43, 238, 69, 69, 69, 238, 149, 149, 149, 175, 255, 255, 255, 10, 0, 0, 0, 0, 63, 0, 0, 0, 0, 5, 172, 172, 172, 78, 105, 105, 105, 231, 48, 48, 48, 238, 43, 43, 43, 238, 44, 44, 44, 238, 54, 44, 44, 44, 238, 6, 43, 43, 43, 238, 44, 44, 44, 238, 73, 73, 73, 231, 155, 155, 155, 128, 255, 255, 255, 10, 0, 0, 0, 0, 58, 0, 0, 0, 0, 3, 222, 222, 222, 78, 91, 91, 91, 245, 0, 0, 0, 255, 60, 0, 0, 0, 255, 4, 21, 21, 21, 255, 125, 125, 125, 177, 255, 255, 255, 10, 0, 0, 0, 0, 61, 0, 0, 0, 0, 3, 172, 172, 172, 78, 59, 59, 59, 245, 0, 0, 0, 255, 60, 0, 0, 0, 255, 4, 39, 39, 39, 255, 171, 171, 171, 177, 255, 255, 255, 10, 0, 0, 0, 0, 56, 0, 0, 0, 0, 3, 232, 232, 232, 78, 99, 99, 99, 245, 0, 0, 0, 255, 62, 0, 0, 0, 255, 4, 24, 24, 24, 255, 137, 137, 137, 177, 255, 255, 255, 10, 0, 0, 0, 0, 59, 0, 0, 0, 0, 3, 185, 185, 185, 78, 65, 65, 65, 245, 0, 0, 0, 255, 62, 0, 0, 0, 255, 4, 40, 40, 40, 255, 181, 181, 181, 177, 255, 255, 255, 11, 0, 0, 0, 0, 54, 0, 0, 0, 0, 4, 241, 241, 241, 78, 108, 108, 108, 245, 1, 1, 1, 255, 0, 0, 0, 255, 63, 0, 0, 0, 255, 4, 27, 27, 27, 255, 148, 148, 148, 177, 255, 255, 255, 10, 0, 0, 0, 0, 57, 0, 0, 0, 0, 3, 197, 197, 197, 78, 73, 73, 73, 245, 0, 0, 0, 255, 64, 0, 0, 0, 255, 4, 45, 45, 45, 255, 189, 189, 189, 175, 255, 255, 255, 2, 0, 0, 0, 0, 52, 0, 0, 0, 0, 4, 249, 249, 249, 66, 117, 117, 117, 245, 2, 2, 2, 255, 0, 0, 0, 255, 65, 0, 0, 0, 255, 4, 31, 31, 31, 255, 160, 160, 160, 177, 255, 255, 255, 10, 0, 0, 0, 0, 55, 0, 0, 0, 0, 3, 210, 210, 210, 78, 82, 82, 82, 245, 0, 0, 0, 255, 66, 0, 0, 0, 255, 3, 44, 44, 44, 253, 127, 127, 127, 80, 0, 0, 0, 0, 51, 0, 0, 0, 0, 4, 255, 255, 255, 11, 83, 83, 83, 190, 7, 7, 7, 255, 0, 0, 0, 255, 67, 0, 0, 0, 255, 4, 35, 35, 35, 255, 170, 170, 170, 177, 255, 255, 255, 10, 0, 0, 0, 0, 53, 0, 0, 0, 0, 3, 221, 221, 221, 78, 89, 89, 89, 245, 0, 0, 0, 255, 68, 0, 0, 0, 255, 3, 42, 42, 42, 244, 140, 140, 140, 78, 0, 0, 0, 0, 49, 0, 0, 0, 0, 4, 255, 255, 255, 11, 83, 83, 83, 178, 10, 10, 10, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 24, 34, 0, 0, 255, 102, 0, 0, 255, 110, 0, 0, 255, 114, 0, 0, 255, 118, 0, 0, 255, 122, 0, 0, 255, 125, 0, 0, 255, 128, 0, 0, 255, 131, 0, 0, 255, 134, 0, 0, 255, 137, 0, 0, 255, 139, 0, 0, 255, 141, 0, 0, 255, 143, 0, 0, 255, 145, 0, 0, 255, 147, 0, 0, 255, 149, 0, 0, 255, 151, 0, 0, 255, 152, 0, 0, 255, 153, 0, 0, 255, 154, 0, 0, 255, 156, 0, 0, 255, 157, 0, 0, 255, 158, 0, 0, 255, 1, 159, 0, 0, 255, 1, 160, 0, 0, 255, 4, 160, 0, 0, 255, 2, 159, 0, 0, 255, 158, 0, 0, 255, 1, 157, 0, 0, 255, 24, 156, 0, 0, 255, 155, 0, 0, 255, 154, 0, 0, 255, 153, 0, 0, 255, 152, 0, 0, 255, 150, 0, 0, 255, 148, 0, 0, 255, 146, 0, 0, 255, 145, 0, 0, 255, 142, 0, 0, 255, 140, 0, 0, 255, 138, 0, 0, 255, 135, 0, 0, 255, 133, 0, 0, 255, 130, 0, 0, 255, 127, 0, 0, 255, 124, 0, 0, 255, 120, 0, 0, 255, 116, 0, 0, 255, 111, 0, 0, 255, 109, 0, 0, 255, 87, 0, 0, 255, 9, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 39, 39, 39, 255, 180, 180, 180, 177, 255, 255, 255, 10, 0, 0, 0, 0, 51, 0, 0, 0, 0, 3, 231, 231, 231, 78, 98, 98, 98, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 24, 52, 0, 0, 255, 107, 0, 0, 255, 109, 0, 0, 255, 114, 0, 0, 255, 118, 0, 0, 255, 122, 0, 0, 255, 125, 0, 0, 255, 128, 0, 0, 255, 131, 0, 0, 255, 134, 0, 0, 255, 137, 0, 0, 255, 139, 0, 0, 255, 141, 0, 0, 255, 144, 0, 0, 255, 146, 0, 0, 255, 147, 0, 0, 255, 148, 0, 0, 255, 151, 0, 0, 255, 152, 0, 0, 255, 154, 0, 0, 255, 155, 0, 0, 255, 156, 0, 0, 255, 157, 0, 0, 255, 158, 0, 0, 255, 1, 159, 0, 0, 255, 1, 160, 0, 0, 255, 4, 160, 0, 0, 255, 2, 159, 0, 0, 255, 158, 0, 0, 255, 1, 157, 0, 0, 255, 24, 156, 0, 0, 255, 155, 0, 0, 255, 154, 0, 0, 255, 153, 0, 0, 255, 151, 0, 0, 255, 149, 0, 0, 255, 147, 0, 0, 255, 146, 0, 0, 255, 144, 0, 0, 255, 142, 0, 0, 255, 139, 0, 0, 255, 137, 0, 0, 255, 135, 0, 0, 255, 132, 0, 0, 255, 129, 0, 0, 255, 126, 0, 0, 255, 123, 0, 0, 255, 119, 0, 0, 255, 115, 0, 0, 255, 111, 0, 0, 255, 109, 0, 0, 255, 74, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 49, 49, 49, 245, 156, 156, 156, 78, 0, 0, 0, 0, 47, 0, 0, 0, 0, 4, 255, 255, 255, 11, 95, 95, 95, 178, 13, 13, 13, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 26, 33, 0, 0, 255, 147, 0, 0, 255, 177, 0, 0, 255, 181, 0, 0, 255, 186, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 216, 0, 0, 255, 219, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 1, 244, 0, 0, 255, 1, 245, 0, 0, 255, 2, 245, 0, 0, 255, 1, 244, 0, 0, 255, 2, 243, 0, 0, 255, 26, 242, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 223, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 207, 0, 0, 255, 203, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 190, 0, 0, 255, 185, 0, 0, 255, 179, 0, 0, 255, 174, 0, 0, 255, 108, 0, 0, 255, 7, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 44, 44, 44, 255, 189, 189, 189, 177, 255, 255, 255, 15, 0, 0, 0, 0, 49, 0, 0, 0, 0, 4, 240, 240, 240, 78, 107, 107, 107, 245, 1, 1, 1, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 26, 53, 0, 0, 255, 159, 0, 0, 255, 177, 0, 0, 255, 182, 0, 0, 255, 187, 0, 0, 255, 193, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 206, 0, 0, 255, 209, 0, 0, 255, 213, 0, 0, 255, 216, 0, 0, 255, 219, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 1, 244, 0, 0, 255, 1, 245, 0, 0, 255, 2, 245, 0, 0, 255, 1, 244, 0, 0, 255, 1, 243, 0, 0, 255, 1, 242, 0, 0, 255, 1, 240, 0, 0, 255, 23, 239, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 217, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 199, 0, 0, 255, 194, 0, 0, 255, 189, 0, 0, 255, 183, 0, 0, 255, 178, 0, 0, 255, 170, 0, 0, 255, 86, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 57, 57, 57, 245, 171, 171, 171, 78, 0, 0, 0, 0, 45, 0, 0, 0, 0, 4, 255, 255, 255, 11, 106, 106, 106, 178, 15, 15, 15, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 28, 31, 0, 0, 255, 147, 0, 0, 255, 177, 0, 0, 255, 178, 0, 0, 255, 184, 0, 0, 255, 188, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 200, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 210, 0, 0, 255, 213, 0, 0, 255, 216, 0, 0, 255, 219, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 4, 241, 0, 0, 255, 1, 240, 0, 0, 255, 1, 240, 0, 0, 255, 3, 239, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 1, 234, 0, 0, 255, 23, 232, 0, 0, 255, 230, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 223, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 198, 0, 0, 255, 195, 0, 0, 255, 190, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 177, 0, 0, 255, 176, 0, 0, 255, 105, 0, 0, 255, 5, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 53, 53, 53, 255, 173, 173, 173, 127, 0, 0, 0, 0, 48, 0, 0, 0, 0, 4, 248, 248, 248, 79, 115, 115, 115, 245, 2, 2, 2, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 28, 50, 0, 0, 255, 161, 0, 0, 255, 177, 0, 0, 255, 179, 0, 0, 255, 185, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 216, 0, 0, 255, 219, 0, 0, 255, 222, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 4, 241, 0, 0, 255, 1, 240, 0, 0, 255, 2, 239, 0, 0, 255, 26, 237, 0, 0, 255, 236, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 208, 0, 0, 255, 205, 0, 0, 255, 202, 0, 0, 255, 198, 0, 0, 255, 194, 0, 0, 255, 190, 0, 0, 255, 186, 0, 0, 255, 181, 0, 0, 255, 176, 0, 0, 255, 173, 0, 0, 255, 83, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 65, 65, 65, 245, 183, 183, 183, 78, 0, 0, 0, 0, 43, 0, 0, 0, 0, 4, 255, 255, 255, 11, 117, 117, 117, 178, 18, 18, 18, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 27, 0, 0, 255, 145, 0, 0, 255, 179, 0, 0, 255, 1, 185, 0, 0, 255, 23, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 226, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 2, 244, 0, 0, 255, 1, 245, 0, 0, 255, 2, 244, 0, 0, 255, 1, 243, 0, 0, 255, 1, 242, 0, 0, 255, 28, 241, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 183, 0, 0, 255, 179, 0, 0, 255, 178, 0, 0, 255, 102, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 39, 39, 39, 240, 139, 139, 139, 78, 0, 0, 0, 0, 46, 0, 0, 0, 0, 4, 255, 255, 255, 18, 116, 116, 116, 237, 3, 3, 3, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 28, 46, 0, 0, 255, 160, 0, 0, 255, 179, 0, 0, 255, 180, 0, 0, 255, 186, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 2, 244, 0, 0, 255, 1, 245, 0, 0, 255, 2, 244, 0, 0, 255, 1, 243, 0, 0, 255, 1, 242, 0, 0, 255, 27, 241, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 178, 0, 0, 255, 173, 0, 0, 255, 79, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 73, 73, 73, 245, 196, 196, 196, 78, 0, 0, 0, 0, 41, 0, 0, 0, 0, 4, 255, 255, 255, 11, 130, 130, 130, 178, 21, 21, 21, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 24, 0, 0, 255, 141, 0, 0, 255, 180, 0, 0, 255, 1, 185, 0, 0, 255, 26, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 2, 247, 0, 0, 255, 3, 246, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 1, 245, 0, 0, 255, 29, 244, 0, 0, 255, 243, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 183, 0, 0, 255, 179, 0, 0, 255, 177, 0, 0, 255, 96, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 48, 48, 48, 245, 154, 154, 154, 78, 0, 0, 0, 0, 44, 0, 0, 0, 0, 4, 255, 255, 255, 11, 80, 80, 80, 177, 6, 6, 6, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 41, 0, 0, 255, 157, 0, 0, 255, 180, 0, 0, 255, 1, 186, 0, 0, 255, 28, 190, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 1, 246, 0, 0, 255, 4, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 244, 0, 0, 255, 1, 242, 0, 0, 255, 26, 241, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 181, 0, 0, 255, 179, 0, 0, 255, 173, 0, 0, 255, 74, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 80, 80, 80, 245, 207, 207, 207, 78, 0, 0, 0, 0, 39, 0, 0, 0, 0, 4, 255, 255, 255, 11, 140, 140, 140, 178, 24, 24, 24, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 20, 0, 0, 255, 137, 0, 0, 255, 179, 0, 0, 255, 1, 184, 0, 0, 255, 23, 188, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 1, 247, 0, 0, 255, 1, 248, 0, 0, 255, 5, 249, 0, 0, 255, 2, 248, 0, 0, 255, 247, 0, 0, 255, 1, 246, 0, 0, 255, 29, 245, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 198, 0, 0, 255, 194, 0, 0, 255, 190, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 178, 0, 0, 255, 176, 0, 0, 255, 90, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 55, 55, 55, 245, 169, 169, 169, 78, 0, 0, 0, 0, 42, 0, 0, 0, 0, 4, 255, 255, 255, 11, 92, 92, 92, 178, 12, 12, 12, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 36, 0, 0, 255, 154, 0, 0, 255, 179, 0, 0, 255, 1, 184, 0, 0, 255, 24, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 1, 248, 0, 0, 255, 1, 249, 0, 0, 255, 4, 249, 0, 0, 255, 3, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 1, 245, 0, 0, 255, 27, 243, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 220, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 208, 0, 0, 255, 205, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 194, 0, 0, 255, 190, 0, 0, 255, 186, 0, 0, 255, 181, 0, 0, 255, 178, 0, 0, 255, 170, 0, 0, 255, 67, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 88, 88, 88, 245, 220, 220, 220, 78, 0, 0, 0, 0, 37, 0, 0, 0, 0, 4, 255, 255, 255, 11, 152, 152, 152, 178, 28, 28, 28, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 17, 0, 0, 255, 130, 0, 0, 255, 177, 0, 0, 255, 1, 182, 0, 0, 255, 25, 186, 0, 0, 255, 191, 0, 0, 255, 195, 0, 0, 255, 199, 0, 0, 255, 202, 0, 0, 255, 205, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 2, 250, 0, 0, 255, 1, 251, 0, 0, 255, 2, 251, 0, 0, 255, 1, 250, 0, 0, 255, 1, 249, 0, 0, 255, 31, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 208, 0, 0, 255, 204, 0, 0, 255, 200, 0, 0, 255, 197, 0, 0, 255, 193, 0, 0, 255, 189, 0, 0, 255, 185, 0, 0, 255, 180, 0, 0, 255, 176, 0, 0, 255, 172, 0, 0, 255, 84, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 63, 63, 63, 245, 180, 180, 180, 78, 0, 0, 0, 0, 40, 0, 0, 0, 0, 4, 255, 255, 255, 11, 105, 105, 105, 178, 15, 15, 15, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 31, 0, 0, 255, 148, 0, 0, 255, 177, 0, 0, 255, 1, 182, 0, 0, 255, 27, 187, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 199, 0, 0, 255, 203, 0, 0, 255, 206, 0, 0, 255, 209, 0, 0, 255, 213, 0, 0, 255, 216, 0, 0, 255, 219, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 5, 251, 0, 0, 255, 33, 250, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 204, 0, 0, 255, 201, 0, 0, 255, 196, 0, 0, 255, 193, 0, 0, 255, 188, 0, 0, 255, 184, 0, 0, 255, 178, 0, 0, 255, 176, 0, 0, 255, 166, 0, 0, 255, 60, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 97, 97, 97, 245, 228, 228, 228, 78, 0, 0, 0, 0, 36, 0, 0, 0, 0, 3, 172, 172, 172, 176, 32, 32, 32, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 14, 0, 0, 255, 122, 0, 0, 255, 174, 0, 0, 255, 1, 179, 0, 0, 255, 24, 184, 0, 0, 255, 188, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 200, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 210, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 1, 249, 0, 0, 255, 3, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 5, 252, 0, 0, 255, 4, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 1, 245, 0, 0, 255, 27, 243, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 205, 0, 0, 255, 202, 0, 0, 255, 198, 0, 0, 255, 194, 0, 0, 255, 191, 0, 0, 255, 186, 0, 0, 255, 182, 0, 0, 255, 176, 0, 0, 255, 173, 0, 0, 255, 168, 0, 0, 255, 76, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 76, 76, 76, 245, 203, 203, 203, 66, 0, 0, 0, 0, 38, 0, 0, 0, 0, 4, 255, 255, 255, 16, 127, 127, 127, 178, 18, 18, 18, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 26, 0, 0, 255, 141, 0, 0, 255, 174, 0, 0, 255, 1, 180, 0, 0, 255, 29, 184, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 4, 252, 0, 0, 255, 34, 251, 0, 0, 255, 250, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 214, 0, 0, 255, 212, 0, 0, 255, 208, 0, 0, 255, 205, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 194, 0, 0, 255, 190, 0, 0, 255, 185, 0, 0, 255, 181, 0, 0, 255, 175, 0, 0, 255, 173, 0, 0, 255, 160, 0, 0, 255, 54, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 1, 1, 1, 255, 112, 112, 112, 245, 243, 243, 243, 70, 0, 0, 0, 0, 35, 0, 0, 0, 0, 2, 67, 67, 67, 246, 0, 0, 0, 255, 4, 0, 0, 0, 255, 34, 13, 0, 0, 255, 114, 0, 0, 255, 171, 0, 0, 255, 170, 0, 0, 255, 175, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 218, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 2, 252, 0, 0, 255, 2, 253, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 1, 251, 0, 0, 255, 1, 249, 0, 0, 255, 32, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 182, 0, 0, 255, 178, 0, 0, 255, 173, 0, 0, 255, 169, 0, 0, 255, 162, 0, 0, 255, 70, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 6, 6, 6, 255, 125, 125, 125, 188, 255, 255, 255, 9, 0, 0, 0, 0, 37, 0, 0, 0, 0, 3, 218, 218, 218, 121, 52, 52, 52, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 34, 24, 0, 0, 255, 133, 0, 0, 255, 171, 0, 0, 255, 170, 0, 0, 255, 176, 0, 0, 255, 181, 0, 0, 255, 186, 0, 0, 255, 189, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 1, 253, 0, 0, 255, 3, 252, 0, 0, 255, 1, 251, 0, 0, 255, 1, 249, 0, 0, 255, 32, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 177, 0, 0, 255, 172, 0, 0, 255, 170, 0, 0, 255, 154, 0, 0, 255, 48, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 17, 17, 17, 255, 149, 149, 149, 142, 0, 0, 0, 0, 35, 0, 0, 0, 0, 2, 34, 34, 34, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 32, 85, 0, 0, 255, 168, 0, 0, 255, 165, 0, 0, 255, 170, 0, 0, 255, 176, 0, 0, 255, 181, 0, 0, 255, 185, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 3, 252, 0, 0, 255, 1, 253, 0, 0, 255, 4, 252, 0, 0, 255, 34, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 183, 0, 0, 255, 179, 0, 0, 255, 174, 0, 0, 255, 168, 0, 0, 255, 165, 0, 0, 255, 155, 0, 0, 255, 35, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 105, 105, 105, 245, 255, 255, 255, 18, 0, 0, 0, 0, 37, 0, 0, 0, 0, 3, 165, 165, 165, 131, 19, 19, 19, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 34, 2, 0, 0, 255, 111, 0, 0, 255, 168, 0, 0, 255, 166, 0, 0, 255, 171, 0, 0, 255, 177, 0, 0, 255, 182, 0, 0, 255, 186, 0, 0, 255, 190, 0, 0, 255, 195, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 205, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 2, 252, 0, 0, 255, 1, 253, 0, 0, 255, 4, 252, 0, 0, 255, 30, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 192, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 178, 0, 0, 255, 173, 0, 0, 255, 167, 0, 0, 255, 1, 141, 0, 0, 255, 2, 17, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 2, 146, 146, 146, 238, 0, 0, 0, 0, 35, 0, 0, 0, 0, 2, 59, 59, 59, 253, 0, 0, 0, 255, 4, 0, 0, 0, 255, 34, 26, 0, 0, 255, 133, 0, 0, 255, 169, 0, 0, 255, 170, 0, 0, 255, 175, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 2, 253, 0, 0, 255, 252, 0, 0, 255, 2, 252, 0, 0, 255, 1, 251, 0, 0, 255, 1, 250, 0, 0, 255, 32, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 183, 0, 0, 255, 179, 0, 0, 255, 173, 0, 0, 255, 168, 0, 0, 255, 167, 0, 0, 255, 92, 0, 0, 255, 7, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 134, 134, 134, 236, 255, 255, 255, 16, 0, 0, 0, 0, 37, 0, 0, 0, 0, 3, 203, 203, 203, 121, 36, 36, 36, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 33, 1, 0, 0, 255, 42, 0, 0, 255, 148, 0, 0, 255, 169, 0, 0, 255, 171, 0, 0, 255, 176, 0, 0, 255, 181, 0, 0, 255, 186, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 205, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 9, 252, 0, 0, 255, 34, 251, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 231, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 178, 0, 0, 255, 172, 0, 0, 255, 168, 0, 0, 255, 162, 0, 0, 255, 71, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 12, 12, 12, 255, 126, 126, 126, 142, 0, 0, 0, 0, 35, 0, 0, 0, 0, 3, 132, 132, 132, 175, 17, 17, 17, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 29, 27, 0, 0, 255, 141, 0, 0, 255, 174, 0, 0, 255, 173, 0, 0, 255, 179, 0, 0, 255, 184, 0, 0, 255, 188, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 200, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 1, 249, 0, 0, 255, 3, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 5, 252, 0, 0, 255, 4, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 1, 245, 0, 0, 255, 24, 243, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 213, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 186, 0, 0, 255, 183, 0, 0, 255, 177, 0, 0, 255, 172, 0, 0, 255, 1, 99, 0, 0, 255, 2, 4, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 50, 50, 50, 253, 177, 177, 177, 80, 255, 255, 255, 1, 0, 0, 0, 0, 37, 0, 0, 0, 0, 4, 255, 255, 255, 23, 127, 127, 127, 237, 3, 3, 3, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 31, 45, 0, 0, 255, 155, 0, 0, 255, 173, 0, 0, 255, 175, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 1, 251, 0, 0, 255, 1, 252, 0, 0, 255, 5, 252, 0, 0, 255, 33, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 241, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 208, 0, 0, 255, 205, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 194, 0, 0, 255, 190, 0, 0, 255, 186, 0, 0, 255, 181, 0, 0, 255, 176, 0, 0, 255, 172, 0, 0, 255, 169, 0, 0, 255, 77, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 78, 78, 78, 244, 219, 219, 219, 69, 0, 0, 0, 0, 35, 0, 0, 0, 0, 4, 255, 255, 255, 11, 101, 101, 101, 177, 14, 14, 14, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 33, 31, 0, 0, 255, 148, 0, 0, 255, 176, 0, 0, 255, 177, 0, 0, 255, 182, 0, 0, 255, 187, 0, 0, 255, 191, 0, 0, 255, 195, 0, 0, 255, 199, 0, 0, 255, 202, 0, 0, 255, 206, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 4, 251, 0, 0, 255, 1, 250, 0, 0, 255, 1, 248, 0, 0, 255, 1, 247, 0, 0, 255, 1, 245, 0, 0, 255, 24, 243, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 207, 0, 0, 255, 204, 0, 0, 255, 201, 0, 0, 255, 197, 0, 0, 255, 193, 0, 0, 255, 189, 0, 0, 255, 185, 0, 0, 255, 180, 0, 0, 255, 176, 0, 0, 255, 1, 107, 0, 0, 255, 2, 7, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 47, 47, 47, 255, 192, 192, 192, 175, 255, 255, 255, 2, 0, 0, 0, 0, 39, 0, 0, 0, 0, 4, 242, 242, 242, 78, 112, 112, 112, 245, 2, 2, 2, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 33, 52, 0, 0, 255, 162, 0, 0, 255, 176, 0, 0, 255, 178, 0, 0, 255, 183, 0, 0, 255, 188, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 200, 0, 0, 255, 203, 0, 0, 255, 206, 0, 0, 255, 210, 0, 0, 255, 213, 0, 0, 255, 216, 0, 0, 255, 219, 0, 0, 255, 222, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 4, 251, 0, 0, 255, 1, 250, 0, 0, 255, 1, 248, 0, 0, 255, 30, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 207, 0, 0, 255, 204, 0, 0, 255, 200, 0, 0, 255, 197, 0, 0, 255, 193, 0, 0, 255, 188, 0, 0, 255, 184, 0, 0, 255, 179, 0, 0, 255, 175, 0, 0, 255, 172, 0, 0, 255, 84, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 3, 61, 61, 61, 244, 177, 177, 177, 77, 0, 0, 0, 0, 37, 0, 0, 0, 0, 4, 255, 255, 255, 10, 89, 89, 89, 177, 11, 11, 11, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 37, 0, 0, 255, 154, 0, 0, 255, 179, 0, 0, 255, 1, 184, 0, 0, 255, 23, 189, 0, 0, 255, 193, 0, 0, 255, 196, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 210, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 3, 248, 0, 0, 255, 2, 249, 0, 0, 255, 250, 0, 0, 255, 3, 249, 0, 0, 255, 2, 248, 0, 0, 255, 247, 0, 0, 255, 1, 245, 0, 0, 255, 24, 244, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 178, 0, 0, 255, 1, 114, 0, 0, 255, 2, 9, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 42, 42, 42, 255, 186, 186, 186, 177, 255, 255, 255, 11, 0, 0, 0, 0, 41, 0, 0, 0, 0, 4, 235, 235, 235, 78, 103, 103, 103, 245, 1, 1, 1, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 29, 58, 0, 0, 255, 165, 0, 0, 255, 178, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 3, 249, 0, 0, 255, 1, 250, 0, 0, 255, 2, 249, 0, 0, 255, 1, 248, 0, 0, 255, 1, 247, 0, 0, 255, 30, 246, 0, 0, 255, 245, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 214, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 194, 0, 0, 255, 190, 0, 0, 255, 186, 0, 0, 255, 181, 0, 0, 255, 177, 0, 0, 255, 175, 0, 0, 255, 91, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 54, 54, 54, 244, 165, 165, 165, 77, 0, 0, 0, 0, 39, 0, 0, 0, 0, 4, 255, 255, 255, 10, 89, 89, 89, 189, 8, 8, 8, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 42, 0, 0, 255, 158, 0, 0, 255, 180, 0, 0, 255, 1, 186, 0, 0, 255, 25, 190, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 1, 247, 0, 0, 255, 1, 248, 0, 0, 255, 1, 247, 0, 0, 255, 3, 246, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 1, 244, 0, 0, 255, 28, 243, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 184, 0, 0, 255, 178, 0, 0, 255, 180, 0, 0, 255, 121, 0, 0, 255, 11, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 38, 38, 38, 255, 176, 176, 176, 177, 255, 255, 255, 10, 0, 0, 0, 0, 43, 0, 0, 0, 0, 3, 227, 227, 227, 78, 95, 95, 95, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 33, 64, 0, 0, 255, 169, 0, 0, 255, 179, 0, 0, 255, 181, 0, 0, 255, 186, 0, 0, 255, 190, 0, 0, 255, 195, 0, 0, 255, 199, 0, 0, 255, 202, 0, 0, 255, 206, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 2, 247, 0, 0, 255, 1, 246, 0, 0, 255, 1, 245, 0, 0, 255, 29, 244, 0, 0, 255, 243, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 179, 0, 0, 255, 177, 0, 0, 255, 98, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 47, 47, 47, 244, 151, 151, 151, 77, 0, 0, 0, 0, 41, 0, 0, 0, 0, 4, 253, 253, 253, 66, 122, 122, 122, 245, 3, 3, 3, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 29, 47, 0, 0, 255, 161, 0, 0, 255, 179, 0, 0, 255, 180, 0, 0, 255, 186, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 205, 0, 0, 255, 209, 0, 0, 255, 211, 0, 0, 255, 215, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 4, 245, 0, 0, 255, 1, 244, 0, 0, 255, 1, 243, 0, 0, 255, 29, 242, 0, 0, 255, 241, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 204, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 184, 0, 0, 255, 179, 0, 0, 255, 180, 0, 0, 255, 126, 0, 0, 255, 14, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 33, 33, 33, 255, 166, 166, 166, 177, 255, 255, 255, 10, 0, 0, 0, 0, 45, 0, 0, 0, 0, 3, 216, 216, 216, 78, 86, 86, 86, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 29, 69, 0, 0, 255, 171, 0, 0, 255, 179, 0, 0, 255, 181, 0, 0, 255, 186, 0, 0, 255, 191, 0, 0, 255, 195, 0, 0, 255, 199, 0, 0, 255, 202, 0, 0, 255, 206, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 5, 245, 0, 0, 255, 31, 244, 0, 0, 255, 243, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 218, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 200, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 187, 0, 0, 255, 183, 0, 0, 255, 179, 0, 0, 255, 178, 0, 0, 255, 104, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 37, 37, 37, 240, 134, 134, 134, 77, 0, 0, 0, 0, 43, 0, 0, 0, 0, 4, 245, 245, 245, 78, 113, 113, 113, 245, 2, 2, 2, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 25, 52, 0, 0, 255, 162, 0, 0, 255, 177, 0, 0, 255, 180, 0, 0, 255, 184, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 219, 0, 0, 255, 222, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 2, 240, 0, 0, 255, 1, 241, 0, 0, 255, 4, 241, 0, 0, 255, 1, 240, 0, 0, 255, 1, 239, 0, 0, 255, 1, 238, 0, 0, 255, 1, 236, 0, 0, 255, 20, 234, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 203, 0, 0, 255, 199, 0, 0, 255, 196, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 183, 0, 0, 255, 178, 0, 0, 255, 1, 129, 0, 0, 255, 2, 16, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 29, 29, 29, 255, 155, 155, 155, 177, 255, 255, 255, 10, 0, 0, 0, 0, 47, 0, 0, 0, 0, 3, 204, 204, 204, 78, 78, 78, 78, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 23, 74, 0, 0, 255, 170, 0, 0, 255, 177, 0, 0, 255, 181, 0, 0, 255, 186, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 206, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 1, 238, 0, 0, 255, 2, 239, 0, 0, 255, 240, 0, 0, 255, 6, 241, 0, 0, 255, 1, 240, 0, 0, 255, 1, 239, 0, 0, 255, 27, 238, 0, 0, 255, 237, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 223, 0, 0, 255, 220, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 182, 0, 0, 255, 177, 0, 0, 255, 176, 0, 0, 255, 108, 0, 0, 255, 6, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 52, 52, 52, 255, 170, 170, 170, 127, 0, 0, 0, 0, 45, 0, 0, 0, 0, 4, 238, 238, 238, 78, 105, 105, 105, 245, 1, 1, 1, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 26, 55, 0, 0, 255, 161, 0, 0, 255, 176, 0, 0, 255, 180, 0, 0, 255, 186, 0, 0, 255, 191, 0, 0, 255, 196, 0, 0, 255, 200, 0, 0, 255, 204, 0, 0, 255, 207, 0, 0, 255, 210, 0, 0, 255, 213, 0, 0, 255, 217, 0, 0, 255, 219, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 2, 242, 0, 0, 255, 1, 243, 0, 0, 255, 2, 242, 0, 0, 255, 1, 241, 0, 0, 255, 1, 240, 0, 0, 255, 1, 239, 0, 0, 255, 1, 237, 0, 0, 255, 23, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 198, 0, 0, 255, 194, 0, 0, 255, 189, 0, 0, 255, 184, 0, 0, 255, 178, 0, 0, 255, 176, 0, 0, 255, 130, 0, 0, 255, 19, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 26, 26, 26, 255, 144, 144, 144, 177, 255, 255, 255, 10, 0, 0, 0, 0, 49, 0, 0, 0, 0, 3, 193, 193, 193, 78, 70, 70, 70, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 26, 77, 0, 0, 255, 169, 0, 0, 255, 177, 0, 0, 255, 182, 0, 0, 255, 187, 0, 0, 255, 192, 0, 0, 255, 196, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 2, 242, 0, 0, 255, 1, 243, 0, 0, 255, 3, 242, 0, 0, 255, 28, 241, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 223, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 215, 0, 0, 255, 211, 0, 0, 255, 209, 0, 0, 255, 205, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 193, 0, 0, 255, 189, 0, 0, 255, 183, 0, 0, 255, 177, 0, 0, 255, 174, 0, 0, 255, 109, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 43, 43, 43, 255, 188, 188, 188, 177, 255, 255, 255, 14, 0, 0, 0, 0, 46, 0, 0, 0, 0, 3, 228, 228, 228, 78, 96, 96, 96, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 21, 56, 0, 0, 255, 132, 0, 0, 255, 138, 0, 0, 255, 144, 0, 0, 255, 149, 0, 0, 255, 154, 0, 0, 255, 158, 0, 0, 255, 161, 0, 0, 255, 165, 0, 0, 255, 168, 0, 0, 255, 172, 0, 0, 255, 175, 0, 0, 255, 177, 0, 0, 255, 180, 0, 0, 255, 183, 0, 0, 255, 185, 0, 0, 255, 187, 0, 0, 255, 188, 0, 0, 255, 190, 0, 0, 255, 192, 0, 0, 255, 194, 0, 0, 255, 1, 196, 0, 0, 255, 1, 197, 0, 0, 255, 1, 198, 0, 0, 255, 1, 199, 0, 0, 255, 4, 199, 0, 0, 255, 1, 198, 0, 0, 255, 1, 197, 0, 0, 255, 25, 196, 0, 0, 255, 195, 0, 0, 255, 194, 0, 0, 255, 193, 0, 0, 255, 191, 0, 0, 255, 189, 0, 0, 255, 187, 0, 0, 255, 185, 0, 0, 255, 183, 0, 0, 255, 182, 0, 0, 255, 179, 0, 0, 255, 176, 0, 0, 255, 174, 0, 0, 255, 170, 0, 0, 255, 167, 0, 0, 255, 164, 0, 0, 255, 160, 0, 0, 255, 156, 0, 0, 255, 152, 0, 0, 255, 147, 0, 0, 255, 141, 0, 0, 255, 137, 0, 0, 255, 115, 0, 0, 255, 21, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 22, 22, 22, 255, 133, 133, 133, 177, 255, 255, 255, 10, 0, 0, 0, 0, 51, 0, 0, 0, 0, 3, 180, 180, 180, 78, 62, 62, 62, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 25, 76, 0, 0, 255, 135, 0, 0, 255, 139, 0, 0, 255, 145, 0, 0, 255, 149, 0, 0, 255, 154, 0, 0, 255, 158, 0, 0, 255, 162, 0, 0, 255, 165, 0, 0, 255, 169, 0, 0, 255, 172, 0, 0, 255, 175, 0, 0, 255, 178, 0, 0, 255, 181, 0, 0, 255, 183, 0, 0, 255, 184, 0, 0, 255, 187, 0, 0, 255, 189, 0, 0, 255, 191, 0, 0, 255, 193, 0, 0, 255, 194, 0, 0, 255, 195, 0, 0, 255, 196, 0, 0, 255, 197, 0, 0, 255, 198, 0, 0, 255, 6, 199, 0, 0, 255, 1, 198, 0, 0, 255, 1, 197, 0, 0, 255, 25, 196, 0, 0, 255, 195, 0, 0, 255, 194, 0, 0, 255, 193, 0, 0, 255, 190, 0, 0, 255, 189, 0, 0, 255, 188, 0, 0, 255, 185, 0, 0, 255, 183, 0, 0, 255, 181, 0, 0, 255, 178, 0, 0, 255, 176, 0, 0, 255, 173, 0, 0, 255, 169, 0, 0, 255, 167, 0, 0, 255, 163, 0, 0, 255, 159, 0, 0, 255, 155, 0, 0, 255, 151, 0, 0, 255, 146, 0, 0, 255, 140, 0, 0, 255, 137, 0, 0, 255, 102, 0, 0, 255, 10, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 38, 38, 38, 255, 178, 178, 178, 177, 255, 255, 255, 10, 0, 0, 0, 0, 48, 0, 0, 0, 0, 3, 218, 218, 218, 78, 88, 88, 88, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 2, 4, 0, 0, 255, 6, 0, 0, 255, 1, 6, 0, 0, 255, 1, 7, 0, 0, 255, 2, 7, 0, 0, 255, 1, 8, 0, 0, 255, 2, 8, 0, 0, 255, 1, 9, 0, 0, 255, 5, 9, 0, 0, 255, 1, 10, 0, 0, 255, 16, 10, 0, 0, 255, 1, 9, 0, 0, 255, 5, 9, 0, 0, 255, 1, 8, 0, 0, 255, 3, 8, 0, 0, 255, 1, 7, 0, 0, 255, 1, 7, 0, 0, 255, 1, 6, 0, 0, 255, 2, 6, 0, 0, 255, 2, 2, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 4, 19, 19, 19, 255, 121, 121, 121, 177, 255, 255, 255, 10, 0, 0, 0, 0, 53, 0, 0, 0, 0, 3, 166, 166, 166, 78, 55, 55, 55, 245, 0, 0, 0, 255, 5, 0, 0, 0, 255, 2, 5, 0, 0, 255, 6, 0, 0, 255, 1, 6, 0, 0, 255, 1, 7, 0, 0, 255, 2, 7, 0, 0, 255, 1, 8, 0, 0, 255, 2, 8, 0, 0, 255, 1, 9, 0, 0, 255, 4, 9, 0, 0, 255, 1, 10, 0, 0, 255, 18, 10, 0, 0, 255, 1, 9, 0, 0, 255, 4, 9, 0, 0, 255, 1, 8, 0, 0, 255, 2, 8, 0, 0, 255, 1, 7, 0, 0, 255, 2, 7, 0, 0, 255, 1, 6, 0, 0, 255, 1, 6, 0, 0, 255, 2, 5, 0, 0, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 4, 33, 33, 33, 255, 168, 168, 168, 177, 255, 255, 255, 10, 0, 0, 0, 0, 50, 0, 0, 0, 0, 3, 205, 205, 205, 78, 79, 79, 79, 245, 0, 0, 0, 255, 66, 0, 0, 0, 255, 4, 16, 16, 16, 255, 109, 109, 109, 177, 255, 255, 255, 10, 0, 0, 0, 0, 55, 0, 0, 0, 0, 3, 151, 151, 151, 78, 48, 48, 48, 245, 0, 0, 0, 255, 66, 0, 0, 0, 255, 4, 30, 30, 30, 255, 157, 157, 157, 177, 255, 255, 255, 10, 0, 0, 0, 0, 52, 0, 0, 0, 0, 3, 194, 194, 194, 78, 71, 71, 71, 245, 0, 0, 0, 255, 64, 0, 0, 0, 255, 4, 13, 13, 13, 255, 97, 97, 97, 177, 255, 255, 255, 10, 0, 0, 0, 0, 57, 0, 0, 0, 0, 3, 136, 136, 136, 78, 40, 40, 40, 244, 0, 0, 0, 255, 64, 0, 0, 0, 255, 4, 26, 26, 26, 255, 146, 146, 146, 177, 255, 255, 255, 10, 0, 0, 0, 0, 54, 0, 0, 0, 0, 3, 181, 181, 181, 78, 63, 63, 63, 245, 0, 0, 0, 255, 62, 0, 0, 0, 255, 4, 10, 10, 10, 255, 86, 86, 86, 177, 255, 255, 255, 10, 0, 0, 0, 0, 59, 0, 0, 0, 0, 3, 122, 122, 122, 80, 42, 42, 42, 253, 0, 0, 0, 255, 62, 0, 0, 0, 255, 4, 23, 23, 23, 255, 134, 134, 134, 177, 255, 255, 255, 10, 0, 0, 0, 0, 56, 0, 0, 0, 0, 3, 169, 169, 169, 78, 56, 56, 56, 245, 0, 0, 0, 255, 60, 0, 0, 0, 255, 4, 12, 12, 12, 255, 86, 86, 86, 189, 255, 255, 255, 10, 0, 0, 0, 0, 60, 0, 0, 0, 0, 4, 255, 255, 255, 2, 187, 187, 187, 175, 44, 44, 44, 255, 0, 0, 0, 255, 60, 0, 0, 0, 255, 4, 19, 19, 19, 255, 123, 123, 123, 177, 255, 255, 255, 10, 0, 0, 0, 0, 58, 0, 0, 0, 0, 5, 158, 158, 158, 78, 79, 79, 79, 233, 36, 36, 36, 255, 31, 31, 31, 255, 32, 32, 32, 255, 55, 32, 32, 32, 255, 5, 31, 31, 31, 255, 42, 42, 42, 252, 103, 103, 103, 182, 252, 252, 252, 58, 0, 0, 0, 0, 62, 0, 0, 0, 0, 4, 255, 255, 255, 11, 187, 187, 187, 175, 63, 63, 63, 240, 31, 31, 31, 255, 57, 32, 32, 32, 255, 5, 31, 31, 31, 255, 52, 52, 52, 252, 130, 130, 130, 175, 255, 255, 255, 10, 0, 0, 0, 0, 60, 0, 0, 0, 0, 3, 249, 249, 249, 22, 208, 208, 208, 120, 199, 199, 199, 127, 56, 199, 199, 199, 127, 4, 200, 200, 200, 127, 206, 206, 206, 72, 255, 255, 255, 2, 0, 0, 0, 0, 64, 0, 0, 0, 0, 4, 255, 255, 255, 3, 224, 224, 224, 24, 201, 201, 201, 120, 199, 199, 199, 127, 56, 199, 199, 199, 127, 4, 201, 201, 201, 127, 220, 220, 220, 72, 255, 255, 255, 3, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 141, 0, 0, 0, 0, 7, 255, 255, 255, 3, 255, 255, 255, 24, 210, 210, 210, 121, 177, 177, 177, 128, 210, 210, 210, 72, 255, 255, 255, 3, 0, 0, 0, 0, 17, 0, 0, 0, 0, 6, 252, 252, 252, 22, 185, 185, 185, 122, 182, 182, 182, 122, 192, 192, 192, 24, 255, 255, 255, 15, 0, 0, 0, 0, 221, 0, 0, 0, 0, 11, 255, 255, 255, 15, 226, 226, 226, 73, 161, 161, 161, 128, 106, 106, 106, 183, 75, 75, 75, 240, 39, 39, 39, 255, 22, 22, 22, 255, 49, 49, 49, 252, 155, 155, 155, 175, 255, 255, 255, 16, 0, 0, 0, 0, 14, 0, 0, 0, 0, 12, 255, 255, 255, 1, 196, 196, 196, 79, 87, 87, 87, 233, 27, 27, 27, 255, 24, 24, 24, 255, 47, 47, 47, 240, 104, 104, 104, 231, 118, 118, 118, 135, 209, 209, 209, 121, 255, 255, 255, 24, 255, 255, 255, 2, 0, 0, 0, 0, 213, 0, 0, 0, 0, 9, 255, 255, 255, 3, 247, 247, 247, 24, 198, 198, 198, 121, 103, 103, 103, 135, 96, 96, 96, 231, 51, 51, 51, 253, 21, 21, 21, 255, 2, 2, 2, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 44, 44, 44, 255, 190, 190, 190, 115, 0, 0, 0, 0, 14, 0, 0, 0, 0, 3, 255, 255, 255, 11, 106, 106, 106, 235, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 13, 13, 13, 255, 38, 38, 38, 255, 75, 75, 75, 240, 104, 104, 104, 183, 160, 160, 160, 128, 225, 225, 225, 72, 255, 255, 255, 15, 0, 0, 0, 0, 206, 0, 0, 0, 0, 9, 255, 255, 255, 3, 255, 255, 255, 17, 213, 213, 213, 73, 147, 147, 147, 128, 94, 94, 94, 183, 68, 68, 68, 240, 33, 33, 33, 255, 11, 11, 11, 255, 0, 0, 0, 255, 6, 0, 0, 0, 255, 4, 4, 4, 4, 255, 124, 124, 124, 184, 255, 255, 255, 9, 0, 0, 0, 0, 13, 0, 0, 0, 0, 3, 205, 205, 205, 72, 37, 37, 37, 248, 0, 0, 0, 255, 6, 0, 0, 0, 255, 9, 2, 2, 2, 255, 22, 22, 22, 255, 51, 51, 51, 252, 94, 94, 94, 231, 104, 104, 104, 135, 197, 197, 197, 121, 246, 246, 246, 24, 255, 255, 255, 2, 0, 0, 0, 0, 199, 0, 0, 0, 0, 9, 255, 255, 255, 15, 220, 220, 220, 24, 183, 183, 183, 121, 133, 133, 133, 183, 92, 92, 92, 238, 43, 43, 43, 253, 18, 18, 18, 255, 1, 1, 1, 255, 0, 0, 0, 255, 10, 0, 0, 0, 255, 3, 93, 93, 93, 242, 255, 255, 255, 13, 0, 0, 0, 0, 12, 0, 0, 0, 0, 4, 255, 255, 255, 1, 156, 156, 156, 141, 18, 18, 18, 255, 0, 0, 0, 255, 10, 0, 0, 0, 255, 8, 11, 11, 11, 255, 33, 33, 33, 255, 67, 67, 67, 240, 93, 93, 93, 183, 144, 144, 144, 128, 210, 210, 210, 72, 255, 255, 255, 15, 0, 0, 0, 0, 195, 0, 0, 0, 0, 6, 162, 162, 162, 78, 126, 126, 126, 231, 57, 57, 57, 240, 28, 28, 28, 255, 6, 6, 6, 255, 0, 0, 0, 255, 14, 0, 0, 0, 255, 3, 46, 46, 46, 247, 220, 220, 220, 72, 0, 0, 0, 0, 12, 0, 0, 0, 0, 3, 255, 255, 255, 15, 133, 133, 133, 230, 0, 0, 0, 255, 14, 0, 0, 0, 255, 7, 1, 1, 1, 255, 18, 18, 18, 255, 42, 42, 42, 252, 86, 86, 86, 231, 120, 120, 120, 128, 255, 255, 255, 10, 0, 0, 0, 0, 192, 0, 0, 0, 0, 3, 168, 168, 168, 79, 46, 46, 46, 245, 0, 0, 0, 255, 18, 0, 0, 0, 255, 3, 23, 23, 23, 255, 161, 161, 161, 127, 0, 0, 0, 0, 12, 0, 0, 0, 0, 3, 248, 248, 248, 25, 76, 76, 76, 240, 0, 0, 0, 255, 18, 0, 0, 0, 255, 4, 21, 21, 21, 255, 111, 111, 111, 177, 255, 255, 255, 15, 0, 0, 0, 0, 190, 0, 0, 0, 0, 3, 255, 255, 255, 22, 102, 102, 102, 237, 0, 0, 0, 255, 12, 0, 0, 0, 255, 4, 2, 0, 0, 255, 20, 0, 0, 255, 14, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 5, 5, 5, 255, 114, 114, 114, 183, 255, 255, 255, 9, 0, 0, 0, 0, 11, 0, 0, 0, 0, 3, 219, 219, 219, 119, 44, 44, 44, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 2, 0, 0, 255, 25, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 12, 0, 0, 0, 255, 3, 40, 40, 40, 255, 192, 192, 192, 128, 0, 0, 0, 0, 190, 0, 0, 0, 0, 3, 159, 159, 159, 128, 25, 25, 25, 255, 0, 0, 0, 255, 9, 0, 0, 0, 255, 7, 10, 0, 0, 255, 41, 0, 0, 255, 78, 0, 0, 255, 118, 0, 0, 255, 151, 0, 0, 255, 81, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 87, 87, 87, 242, 255, 255, 255, 13, 0, 0, 0, 0, 10, 0, 0, 0, 0, 4, 255, 255, 255, 1, 142, 142, 142, 137, 16, 16, 16, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 8, 24, 0, 0, 255, 143, 0, 0, 255, 134, 0, 0, 255, 97, 0, 0, 255, 58, 0, 0, 255, 21, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 9, 0, 0, 0, 255, 3, 100, 100, 100, 233, 255, 255, 255, 22, 0, 0, 0, 0, 188, 0, 0, 0, 0, 3, 229, 229, 229, 18, 74, 74, 74, 233, 0, 0, 0, 255, 6, 0, 0, 0, 255, 8, 3, 0, 0, 255, 25, 0, 0, 255, 64, 0, 0, 255, 107, 0, 0, 255, 145, 0, 0, 255, 165, 0, 0, 255, 172, 0, 0, 255, 169, 0, 0, 255, 1, 122, 0, 0, 255, 2, 3, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 41, 41, 41, 247, 211, 211, 211, 72, 0, 0, 0, 0, 10, 0, 0, 0, 0, 3, 255, 255, 255, 15, 125, 125, 125, 230, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 58, 0, 0, 255, 165, 0, 0, 255, 168, 0, 0, 255, 172, 0, 0, 255, 170, 0, 0, 255, 157, 0, 0, 255, 126, 0, 0, 255, 82, 0, 0, 255, 43, 0, 0, 255, 10, 0, 0, 255, 0, 0, 0, 255, 6, 0, 0, 0, 255, 3, 25, 25, 25, 255, 159, 159, 159, 127, 0, 0, 0, 0, 187, 0, 0, 0, 0, 4, 255, 255, 255, 3, 156, 156, 156, 175, 11, 11, 11, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 9, 0, 0, 255, 45, 0, 0, 255, 86, 0, 0, 255, 130, 0, 0, 255, 161, 0, 0, 255, 177, 0, 0, 255, 180, 0, 0, 255, 178, 0, 0, 255, 174, 0, 0, 255, 171, 0, 0, 255, 169, 0, 0, 255, 1, 152, 0, 0, 255, 2, 21, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 21, 21, 21, 255, 150, 150, 150, 127, 0, 0, 0, 0, 10, 0, 0, 0, 0, 3, 242, 242, 242, 25, 71, 71, 71, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 98, 0, 0, 255, 171, 0, 0, 255, 168, 0, 0, 255, 171, 0, 0, 255, 173, 0, 0, 255, 176, 0, 0, 255, 179, 0, 0, 255, 1, 170, 0, 0, 255, 5, 146, 0, 0, 255, 106, 0, 0, 255, 62, 0, 0, 255, 23, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 75, 75, 75, 233, 231, 231, 231, 18, 0, 0, 0, 0, 186, 0, 0, 0, 0, 3, 230, 230, 230, 68, 70, 70, 70, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 5, 15, 0, 0, 255, 128, 0, 0, 255, 167, 0, 0, 255, 178, 0, 0, 255, 180, 0, 0, 255, 1, 179, 0, 0, 255, 1, 180, 0, 0, 255, 1, 178, 0, 0, 255, 6, 177, 0, 0, 255, 174, 0, 0, 255, 171, 0, 0, 255, 169, 0, 0, 255, 56, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 4, 4, 4, 255, 106, 106, 106, 183, 255, 255, 255, 7, 0, 0, 0, 0, 9, 0, 0, 0, 0, 3, 211, 211, 211, 119, 40, 40, 40, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 8, 5, 0, 0, 255, 136, 0, 0, 255, 173, 0, 0, 255, 172, 0, 0, 255, 176, 0, 0, 255, 178, 0, 0, 255, 179, 0, 0, 255, 180, 0, 0, 255, 1, 179, 0, 0, 255, 1, 180, 0, 0, 255, 1, 172, 0, 0, 255, 3, 156, 0, 0, 255, 62, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 12, 12, 12, 255, 157, 157, 157, 175, 255, 255, 255, 3, 0, 0, 0, 0, 184, 0, 0, 0, 0, 4, 255, 255, 255, 3, 133, 133, 133, 188, 8, 8, 8, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 87, 0, 0, 255, 177, 0, 0, 255, 1, 180, 0, 0, 255, 3, 183, 0, 0, 255, 184, 0, 0, 255, 185, 0, 0, 255, 1, 184, 0, 0, 255, 7, 183, 0, 0, 255, 181, 0, 0, 255, 178, 0, 0, 255, 175, 0, 0, 255, 176, 0, 0, 255, 94, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 84, 84, 84, 244, 253, 253, 253, 25, 0, 0, 0, 0, 8, 0, 0, 0, 0, 4, 255, 255, 255, 1, 132, 132, 132, 137, 14, 14, 14, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 8, 29, 0, 0, 255, 161, 0, 0, 255, 174, 0, 0, 255, 177, 0, 0, 255, 180, 0, 0, 255, 182, 0, 0, 255, 183, 0, 0, 255, 185, 0, 0, 255, 1, 184, 0, 0, 255, 3, 183, 0, 0, 255, 182, 0, 0, 255, 178, 0, 0, 255, 1, 147, 0, 0, 255, 2, 16, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 71, 71, 71, 253, 229, 229, 229, 68, 0, 0, 0, 0, 184, 0, 0, 0, 0, 3, 197, 197, 197, 68, 50, 50, 50, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 28, 0, 0, 255, 163, 0, 0, 255, 182, 0, 0, 255, 184, 0, 0, 255, 187, 0, 0, 255, 188, 0, 0, 255, 190, 0, 0, 255, 1, 189, 0, 0, 255, 5, 188, 0, 0, 255, 187, 0, 0, 255, 185, 0, 0, 255, 181, 0, 0, 255, 178, 0, 0, 255, 1, 134, 0, 0, 255, 2, 4, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 46, 46, 46, 254, 222, 222, 222, 118, 0, 0, 0, 0, 8, 0, 0, 0, 0, 3, 255, 255, 255, 15, 120, 120, 120, 230, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 67, 0, 0, 255, 176, 0, 0, 255, 177, 0, 0, 255, 181, 0, 0, 255, 184, 0, 0, 255, 186, 0, 0, 255, 188, 0, 0, 255, 189, 0, 0, 255, 190, 0, 0, 255, 1, 189, 0, 0, 255, 6, 188, 0, 0, 255, 186, 0, 0, 255, 182, 0, 0, 255, 183, 0, 0, 255, 91, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 9, 9, 9, 255, 133, 133, 133, 187, 255, 255, 255, 3, 0, 0, 0, 0, 182, 0, 0, 0, 0, 4, 255, 255, 255, 15, 104, 104, 104, 190, 5, 5, 5, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 114, 0, 0, 255, 189, 0, 0, 255, 187, 0, 0, 255, 190, 0, 0, 255, 192, 0, 0, 255, 194, 0, 0, 255, 1, 194, 0, 0, 255, 10, 193, 0, 0, 255, 192, 0, 0, 255, 190, 0, 0, 255, 188, 0, 0, 255, 185, 0, 0, 255, 181, 0, 0, 255, 178, 0, 0, 255, 161, 0, 0, 255, 25, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 17, 17, 17, 255, 148, 148, 148, 136, 255, 255, 255, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 3, 235, 235, 235, 25, 65, 65, 65, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 106, 0, 0, 255, 180, 0, 0, 255, 1, 184, 0, 0, 255, 5, 186, 0, 0, 255, 189, 0, 0, 255, 191, 0, 0, 255, 193, 0, 0, 255, 194, 0, 0, 255, 1, 194, 0, 0, 255, 7, 193, 0, 0, 255, 191, 0, 0, 255, 189, 0, 0, 255, 188, 0, 0, 255, 169, 0, 0, 255, 29, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 52, 52, 52, 252, 197, 197, 197, 68, 0, 0, 0, 0, 182, 0, 0, 0, 0, 3, 200, 200, 200, 129, 43, 43, 43, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 44, 0, 0, 255, 183, 0, 0, 255, 192, 0, 0, 255, 194, 0, 0, 255, 196, 0, 0, 255, 198, 0, 0, 255, 1, 199, 0, 0, 255, 11, 198, 0, 0, 255, 197, 0, 0, 255, 195, 0, 0, 255, 194, 0, 0, 255, 191, 0, 0, 255, 188, 0, 0, 255, 184, 0, 0, 255, 179, 0, 0, 255, 177, 0, 0, 255, 61, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 131, 131, 131, 230, 255, 255, 255, 15, 0, 0, 0, 0, 7, 0, 0, 0, 0, 3, 204, 204, 204, 119, 35, 35, 35, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 19, 8, 0, 0, 255, 143, 0, 0, 255, 180, 0, 0, 255, 182, 0, 0, 255, 186, 0, 0, 255, 190, 0, 0, 255, 193, 0, 0, 255, 195, 0, 0, 255, 196, 0, 0, 255, 197, 0, 0, 255, 198, 0, 0, 255, 199, 0, 0, 255, 198, 0, 0, 255, 197, 0, 0, 255, 195, 0, 0, 255, 192, 0, 0, 255, 195, 0, 0, 255, 116, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 6, 6, 6, 255, 105, 105, 105, 189, 255, 255, 255, 15, 0, 0, 0, 0, 180, 0, 0, 0, 0, 3, 255, 255, 255, 22, 107, 107, 107, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 3, 0, 0, 255, 137, 0, 0, 255, 200, 0, 0, 255, 197, 0, 0, 255, 200, 0, 0, 255, 201, 0, 0, 255, 202, 0, 0, 255, 203, 0, 0, 255, 202, 0, 0, 255, 1, 201, 0, 0, 255, 6, 199, 0, 0, 255, 197, 0, 0, 255, 194, 0, 0, 255, 191, 0, 0, 255, 187, 0, 0, 255, 181, 0, 0, 255, 1, 98, 0, 0, 255, 1, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 74, 74, 74, 240, 245, 245, 245, 25, 0, 0, 0, 0, 6, 0, 0, 0, 0, 4, 255, 255, 255, 1, 123, 123, 123, 137, 13, 13, 13, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 12, 34, 0, 0, 255, 166, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 189, 0, 0, 255, 193, 0, 0, 255, 196, 0, 0, 255, 198, 0, 0, 255, 200, 0, 0, 255, 201, 0, 0, 255, 202, 0, 0, 255, 203, 0, 0, 255, 1, 202, 0, 0, 255, 6, 200, 0, 0, 255, 198, 0, 0, 255, 197, 0, 0, 255, 187, 0, 0, 255, 45, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 44, 44, 44, 255, 201, 201, 201, 128, 0, 0, 0, 0, 180, 0, 0, 0, 0, 3, 169, 169, 169, 128, 27, 27, 27, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 65, 0, 0, 255, 198, 0, 0, 255, 201, 0, 0, 255, 203, 0, 0, 255, 204, 0, 0, 255, 206, 0, 0, 255, 1, 207, 0, 0, 255, 13, 206, 0, 0, 255, 205, 0, 0, 255, 204, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 197, 0, 0, 255, 193, 0, 0, 255, 189, 0, 0, 255, 184, 0, 0, 255, 180, 0, 0, 255, 136, 0, 0, 255, 5, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 42, 42, 42, 254, 216, 216, 216, 118, 0, 0, 0, 0, 6, 0, 0, 0, 0, 3, 255, 255, 255, 15, 113, 113, 113, 230, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 70, 0, 0, 255, 178, 0, 0, 255, 181, 0, 0, 255, 187, 0, 0, 255, 191, 0, 0, 255, 195, 0, 0, 255, 198, 0, 0, 255, 201, 0, 0, 255, 203, 0, 0, 255, 204, 0, 0, 255, 206, 0, 0, 255, 207, 0, 0, 255, 1, 206, 0, 0, 255, 7, 205, 0, 0, 255, 204, 0, 0, 255, 201, 0, 0, 255, 204, 0, 0, 255, 140, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 107, 107, 107, 233, 255, 255, 255, 22, 0, 0, 0, 0, 178, 0, 0, 0, 0, 3, 244, 244, 244, 18, 81, 81, 81, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 10, 0, 0, 255, 160, 0, 0, 255, 207, 0, 0, 255, 205, 0, 0, 255, 207, 0, 0, 255, 209, 0, 0, 255, 210, 0, 0, 255, 1, 210, 0, 0, 255, 13, 209, 0, 0, 255, 208, 0, 0, 255, 206, 0, 0, 255, 204, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 196, 0, 0, 255, 191, 0, 0, 255, 185, 0, 0, 255, 179, 0, 0, 255, 160, 0, 0, 255, 27, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 15, 15, 15, 255, 140, 140, 140, 136, 255, 255, 255, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 226, 226, 226, 25, 59, 59, 59, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 106, 0, 0, 255, 180, 0, 0, 255, 182, 0, 0, 255, 189, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 201, 0, 0, 255, 203, 0, 0, 255, 206, 0, 0, 255, 208, 0, 0, 255, 209, 0, 0, 255, 210, 0, 0, 255, 1, 210, 0, 0, 255, 7, 209, 0, 0, 255, 208, 0, 0, 255, 206, 0, 0, 255, 204, 0, 0, 255, 202, 0, 0, 255, 65, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 27, 27, 27, 255, 170, 170, 170, 127, 0, 0, 0, 0, 177, 0, 0, 0, 0, 4, 255, 255, 255, 3, 165, 165, 165, 175, 13, 13, 13, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 86, 0, 0, 255, 210, 0, 0, 255, 208, 0, 0, 255, 210, 0, 0, 255, 211, 0, 0, 255, 213, 0, 0, 255, 214, 0, 0, 255, 1, 213, 0, 0, 255, 13, 212, 0, 0, 255, 211, 0, 0, 255, 209, 0, 0, 255, 207, 0, 0, 255, 204, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 192, 0, 0, 255, 186, 0, 0, 255, 179, 0, 0, 255, 171, 0, 0, 255, 62, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 123, 123, 123, 231, 255, 255, 255, 16, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 195, 195, 195, 121, 32, 32, 32, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 23, 10, 0, 0, 255, 140, 0, 0, 255, 178, 0, 0, 255, 183, 0, 0, 255, 190, 0, 0, 255, 195, 0, 0, 255, 200, 0, 0, 255, 203, 0, 0, 255, 206, 0, 0, 255, 208, 0, 0, 255, 210, 0, 0, 255, 211, 0, 0, 255, 212, 0, 0, 255, 213, 0, 0, 255, 214, 0, 0, 255, 213, 0, 0, 255, 212, 0, 0, 255, 211, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 163, 0, 0, 255, 10, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 82, 82, 82, 233, 245, 245, 245, 18, 0, 0, 0, 0, 176, 0, 0, 0, 0, 3, 236, 236, 236, 68, 76, 76, 76, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 19, 0, 0, 255, 180, 0, 0, 255, 214, 0, 0, 255, 212, 0, 0, 255, 214, 0, 0, 255, 215, 0, 0, 255, 216, 0, 0, 255, 217, 0, 0, 255, 1, 216, 0, 0, 255, 13, 215, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 203, 0, 0, 255, 199, 0, 0, 255, 194, 0, 0, 255, 187, 0, 0, 255, 179, 0, 0, 255, 173, 0, 0, 255, 71, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 76, 76, 76, 239, 255, 255, 255, 17, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 124, 124, 124, 129, 15, 15, 15, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 15, 16, 0, 0, 255, 147, 0, 0, 255, 178, 0, 0, 255, 184, 0, 0, 255, 192, 0, 0, 255, 197, 0, 0, 255, 202, 0, 0, 255, 205, 0, 0, 255, 208, 0, 0, 255, 211, 0, 0, 255, 212, 0, 0, 255, 214, 0, 0, 255, 215, 0, 0, 255, 216, 0, 0, 255, 217, 0, 0, 255, 1, 216, 0, 0, 255, 6, 215, 0, 0, 255, 213, 0, 0, 255, 211, 0, 0, 255, 214, 0, 0, 255, 87, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 14, 14, 14, 255, 165, 165, 165, 175, 255, 255, 255, 3, 0, 0, 0, 0, 174, 0, 0, 0, 0, 4, 255, 255, 255, 3, 142, 142, 142, 188, 10, 10, 10, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 109, 0, 0, 255, 219, 0, 0, 255, 215, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 219, 0, 0, 255, 220, 0, 0, 255, 1, 219, 0, 0, 255, 14, 218, 0, 0, 255, 217, 0, 0, 255, 216, 0, 0, 255, 214, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 205, 0, 0, 255, 200, 0, 0, 255, 195, 0, 0, 255, 189, 0, 0, 255, 183, 0, 0, 255, 147, 0, 0, 255, 17, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 94, 94, 94, 245, 255, 255, 255, 18, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 151, 151, 151, 133, 18, 18, 18, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 87, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 193, 0, 0, 255, 199, 0, 0, 255, 203, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 213, 0, 0, 255, 215, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 219, 0, 0, 255, 220, 0, 0, 255, 1, 220, 0, 0, 255, 7, 219, 0, 0, 255, 217, 0, 0, 255, 215, 0, 0, 255, 217, 0, 0, 255, 184, 0, 0, 255, 20, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 78, 78, 78, 253, 235, 235, 235, 68, 0, 0, 0, 0, 174, 0, 0, 0, 0, 3, 207, 207, 207, 68, 56, 56, 56, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 33, 0, 0, 255, 199, 0, 0, 255, 219, 0, 0, 255, 1, 221, 0, 0, 255, 2, 222, 0, 0, 255, 223, 0, 0, 255, 1, 223, 0, 0, 255, 3, 222, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 1, 216, 0, 0, 255, 9, 213, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 202, 0, 0, 255, 197, 0, 0, 255, 189, 0, 0, 255, 183, 0, 0, 255, 74, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 12, 12, 12, 255, 158, 158, 158, 175, 255, 255, 255, 8, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 227, 227, 227, 73, 71, 71, 71, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 17, 0, 0, 255, 154, 0, 0, 255, 188, 0, 0, 255, 193, 0, 0, 255, 200, 0, 0, 255, 204, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 219, 0, 0, 255, 220, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 1, 223, 0, 0, 255, 7, 222, 0, 0, 255, 221, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 222, 0, 0, 255, 110, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 10, 10, 10, 255, 141, 141, 141, 187, 255, 255, 255, 3, 0, 0, 0, 0, 172, 0, 0, 0, 0, 4, 255, 255, 255, 2, 109, 109, 109, 188, 6, 6, 6, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 134, 0, 0, 255, 225, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 224, 0, 0, 255, 225, 0, 0, 255, 226, 0, 0, 255, 1, 226, 0, 0, 255, 14, 225, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 218, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 207, 0, 0, 255, 202, 0, 0, 255, 197, 0, 0, 255, 193, 0, 0, 255, 146, 0, 0, 255, 10, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 75, 75, 75, 233, 227, 227, 227, 18, 0, 0, 0, 0, 6, 0, 0, 0, 0, 4, 255, 255, 255, 3, 156, 156, 156, 175, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 78, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 200, 0, 0, 255, 206, 0, 0, 255, 210, 0, 0, 255, 213, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 223, 0, 0, 255, 224, 0, 0, 255, 225, 0, 0, 255, 226, 0, 0, 255, 1, 226, 0, 0, 255, 3, 225, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 1, 201, 0, 0, 255, 2, 34, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 57, 57, 57, 252, 209, 209, 209, 68, 0, 0, 0, 0, 172, 0, 0, 0, 0, 3, 179, 179, 179, 81, 37, 37, 37, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 50, 0, 0, 255, 213, 0, 0, 255, 224, 0, 0, 255, 1, 226, 0, 0, 255, 1, 228, 0, 0, 255, 1, 229, 0, 0, 255, 1, 228, 0, 0, 255, 1, 227, 0, 0, 255, 12, 226, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 217, 0, 0, 255, 213, 0, 0, 255, 209, 0, 0, 255, 204, 0, 0, 255, 198, 0, 0, 255, 191, 0, 0, 255, 64, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 25, 25, 25, 255, 160, 160, 160, 127, 0, 0, 0, 0, 8, 0, 0, 0, 0, 3, 231, 231, 231, 18, 75, 75, 75, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 10, 0, 0, 255, 153, 0, 0, 255, 199, 0, 0, 255, 201, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 226, 0, 0, 255, 227, 0, 0, 255, 228, 0, 0, 255, 2, 229, 0, 0, 255, 7, 228, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 223, 0, 0, 255, 228, 0, 0, 255, 135, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 6, 6, 6, 255, 111, 111, 111, 187, 255, 255, 255, 2, 0, 0, 0, 0, 170, 0, 0, 0, 0, 3, 255, 255, 255, 22, 116, 116, 116, 237, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 3, 0, 0, 255, 157, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 1, 232, 0, 0, 255, 1, 231, 0, 0, 255, 1, 229, 0, 0, 255, 12, 228, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 214, 0, 0, 255, 210, 0, 0, 255, 204, 0, 0, 255, 203, 0, 0, 255, 141, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 100, 100, 100, 233, 255, 255, 255, 22, 0, 0, 0, 0, 9, 0, 0, 0, 0, 3, 159, 159, 159, 128, 25, 25, 25, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 66, 0, 0, 255, 197, 0, 0, 255, 203, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 216, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 229, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 2, 231, 0, 0, 255, 7, 230, 0, 0, 255, 228, 0, 0, 255, 227, 0, 0, 255, 226, 0, 0, 255, 216, 0, 0, 255, 51, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 37, 37, 37, 252, 180, 180, 180, 80, 0, 0, 0, 0, 170, 0, 0, 0, 0, 3, 178, 178, 178, 128, 30, 30, 30, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 71, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 2, 234, 0, 0, 255, 13, 232, 0, 0, 255, 231, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 215, 0, 0, 255, 210, 0, 0, 255, 206, 0, 0, 255, 195, 0, 0, 255, 49, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 40, 40, 40, 255, 192, 192, 192, 128, 0, 0, 0, 0, 10, 0, 0, 0, 0, 3, 255, 255, 255, 22, 100, 100, 100, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 5, 0, 0, 255, 145, 0, 0, 255, 208, 0, 0, 255, 1, 213, 0, 0, 255, 9, 217, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 231, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 1, 234, 0, 0, 255, 8, 233, 0, 0, 255, 231, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 233, 0, 0, 255, 158, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 116, 116, 116, 237, 255, 255, 255, 22, 0, 0, 0, 0, 168, 0, 0, 0, 0, 3, 255, 255, 255, 22, 87, 87, 87, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 9, 0, 0, 255, 178, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 233, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 236, 0, 0, 255, 2, 236, 0, 0, 255, 13, 234, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 229, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 220, 0, 0, 255, 216, 0, 0, 255, 210, 0, 0, 255, 211, 0, 0, 255, 131, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 5, 5, 5, 255, 96, 96, 96, 189, 255, 255, 255, 14, 0, 0, 0, 0, 11, 0, 0, 0, 0, 3, 193, 193, 193, 128, 36, 36, 36, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 52, 0, 0, 255, 200, 0, 0, 255, 209, 0, 0, 255, 214, 0, 0, 255, 218, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 1, 235, 0, 0, 255, 1, 236, 0, 0, 255, 2, 236, 0, 0, 255, 7, 234, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 71, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 31, 31, 31, 255, 178, 178, 178, 127, 0, 0, 0, 0, 168, 0, 0, 0, 0, 3, 142, 142, 142, 127, 20, 20, 20, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 93, 0, 0, 255, 234, 0, 0, 255, 231, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 1, 238, 0, 0, 255, 13, 237, 0, 0, 255, 236, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 195, 0, 0, 255, 38, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 46, 46, 46, 252, 187, 187, 187, 67, 0, 0, 0, 0, 12, 0, 0, 0, 0, 4, 255, 255, 255, 22, 128, 128, 128, 237, 1, 1, 1, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 11, 1, 0, 0, 255, 134, 0, 0, 255, 215, 0, 0, 255, 214, 0, 0, 255, 219, 0, 0, 255, 223, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 1, 238, 0, 0, 255, 1, 239, 0, 0, 255, 1, 238, 0, 0, 255, 8, 237, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 231, 0, 0, 255, 235, 0, 0, 255, 179, 0, 0, 255, 9, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 89, 89, 89, 233, 255, 255, 255, 22, 0, 0, 0, 0, 166, 0, 0, 0, 0, 3, 241, 241, 241, 66, 74, 74, 74, 241, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 20, 0, 0, 255, 197, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 1, 240, 0, 0, 255, 12, 239, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 232, 0, 0, 255, 229, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 216, 0, 0, 255, 218, 0, 0, 255, 117, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 8, 8, 8, 255, 125, 125, 125, 187, 255, 255, 255, 3, 0, 0, 0, 0, 13, 0, 0, 0, 0, 3, 197, 197, 197, 80, 44, 44, 44, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 39, 0, 0, 255, 198, 0, 0, 255, 216, 0, 0, 255, 219, 0, 0, 255, 223, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 1, 241, 0, 0, 255, 1, 240, 0, 0, 255, 1, 239, 0, 0, 255, 6, 237, 0, 0, 255, 235, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 93, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 20, 20, 20, 255, 141, 141, 141, 126, 0, 0, 0, 0, 165, 0, 0, 0, 0, 4, 255, 255, 255, 3, 149, 149, 149, 188, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 116, 0, 0, 255, 239, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 2, 242, 0, 0, 255, 13, 241, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 221, 0, 0, 255, 220, 0, 0, 255, 190, 0, 0, 255, 26, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 65, 65, 65, 252, 223, 223, 223, 67, 0, 0, 0, 0, 14, 0, 0, 0, 0, 4, 255, 255, 255, 2, 124, 124, 124, 187, 7, 7, 7, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 120, 0, 0, 255, 222, 0, 0, 255, 219, 0, 0, 255, 224, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 1, 242, 0, 0, 255, 8, 241, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 199, 0, 0, 255, 20, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 74, 74, 74, 240, 241, 241, 241, 66, 0, 0, 0, 0, 164, 0, 0, 0, 0, 3, 217, 217, 217, 68, 61, 61, 61, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 32, 0, 0, 255, 214, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 1, 244, 0, 0, 255, 1, 243, 0, 0, 255, 1, 242, 0, 0, 255, 10, 240, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 221, 0, 0, 255, 222, 0, 0, 255, 101, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 13, 13, 13, 255, 155, 155, 155, 187, 255, 255, 255, 3, 0, 0, 0, 0, 15, 0, 0, 0, 0, 3, 222, 222, 222, 68, 63, 63, 63, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 27, 0, 0, 255, 194, 0, 0, 255, 223, 0, 0, 255, 225, 0, 0, 255, 228, 0, 0, 255, 231, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 1, 245, 0, 0, 255, 9, 244, 0, 0, 255, 243, 0, 0, 255, 242, 0, 0, 255, 241, 0, 0, 255, 238, 0, 0, 255, 236, 0, 0, 255, 240, 0, 0, 255, 115, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 12, 12, 12, 255, 150, 150, 150, 187, 255, 255, 255, 3, 0, 0, 0, 0, 162, 0, 0, 0, 0, 4, 255, 255, 255, 2, 119, 119, 119, 188, 7, 7, 7, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 139, 0, 0, 255, 243, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 1, 246, 0, 0, 255, 9, 245, 0, 0, 255, 244, 0, 0, 255, 243, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 1, 183, 0, 0, 255, 2, 16, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 78, 78, 78, 240, 243, 243, 243, 65, 0, 0, 0, 0, 16, 0, 0, 0, 0, 4, 255, 255, 255, 3, 155, 155, 155, 187, 13, 13, 13, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 103, 0, 0, 255, 225, 0, 0, 255, 224, 0, 0, 255, 229, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 2, 246, 0, 0, 255, 8, 245, 0, 0, 255, 243, 0, 0, 255, 241, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 214, 0, 0, 255, 33, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 62, 62, 62, 252, 218, 218, 218, 68, 0, 0, 0, 0, 162, 0, 0, 0, 0, 3, 191, 191, 191, 81, 42, 42, 42, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 50, 0, 0, 255, 226, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 3, 247, 0, 0, 255, 11, 246, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 225, 0, 0, 255, 83, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 22, 22, 22, 255, 149, 149, 149, 126, 0, 0, 0, 0, 18, 0, 0, 0, 0, 3, 244, 244, 244, 66, 78, 78, 78, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 16, 0, 0, 255, 186, 0, 0, 255, 229, 0, 0, 255, 1, 233, 0, 0, 255, 7, 236, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 2, 247, 0, 0, 255, 8, 246, 0, 0, 255, 245, 0, 0, 255, 244, 0, 0, 255, 241, 0, 0, 255, 238, 0, 0, 255, 243, 0, 0, 255, 139, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 7, 7, 7, 255, 119, 119, 119, 187, 255, 255, 255, 2, 0, 0, 0, 0, 160, 0, 0, 0, 0, 4, 255, 255, 255, 22, 123, 123, 123, 237, 1, 1, 1, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 10, 3, 0, 0, 255, 161, 0, 0, 255, 244, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 1, 248, 0, 0, 255, 12, 247, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 171, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 93, 93, 93, 233, 255, 255, 255, 22, 0, 0, 0, 0, 19, 0, 0, 0, 0, 3, 147, 147, 147, 127, 22, 22, 22, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 86, 0, 0, 255, 227, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 1, 248, 0, 0, 255, 3, 249, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 1, 245, 0, 0, 255, 6, 243, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 226, 0, 0, 255, 50, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 42, 42, 42, 252, 191, 191, 191, 80, 0, 0, 0, 0, 160, 0, 0, 0, 0, 3, 186, 186, 186, 128, 33, 33, 33, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 70, 0, 0, 255, 235, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 1, 250, 0, 0, 255, 12, 249, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 225, 0, 0, 255, 66, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 33, 33, 33, 255, 183, 183, 183, 127, 0, 0, 0, 0, 20, 0, 0, 0, 0, 3, 255, 255, 255, 22, 93, 93, 93, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 8, 0, 0, 255, 174, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 2, 250, 0, 0, 255, 1, 248, 0, 0, 255, 1, 247, 0, 0, 255, 7, 245, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 245, 0, 0, 255, 160, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 1, 1, 1, 255, 124, 124, 124, 237, 255, 255, 255, 22, 0, 0, 0, 0, 158, 0, 0, 0, 0, 3, 255, 255, 255, 22, 95, 95, 95, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 8, 0, 0, 255, 180, 0, 0, 255, 244, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 1, 251, 0, 0, 255, 12, 250, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 156, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 121, 121, 121, 237, 255, 255, 255, 22, 0, 0, 0, 0, 21, 0, 0, 0, 0, 3, 184, 184, 184, 128, 32, 32, 32, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 68, 0, 0, 255, 227, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 1, 251, 0, 0, 255, 1, 250, 0, 0, 255, 1, 248, 0, 0, 255, 7, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 235, 0, 0, 255, 68, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 34, 34, 34, 255, 188, 188, 188, 127, 0, 0, 0, 0, 158, 0, 0, 0, 0, 3, 153, 153, 153, 127, 23, 23, 23, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 91, 0, 0, 255, 240, 0, 0, 255, 1, 243, 0, 0, 255, 5, 245, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 1, 251, 0, 0, 255, 1, 249, 0, 0, 255, 9, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 221, 0, 0, 255, 49, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 40, 40, 40, 252, 187, 187, 187, 80, 0, 0, 0, 0, 22, 0, 0, 0, 0, 4, 255, 255, 255, 22, 121, 121, 121, 237, 1, 1, 1, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 12, 3, 0, 0, 255, 158, 0, 0, 255, 238, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 10, 251, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 240, 0, 0, 255, 244, 0, 0, 255, 181, 0, 0, 255, 8, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 96, 96, 96, 233, 255, 255, 255, 22, 0, 0, 0, 0, 156, 0, 0, 0, 0, 3, 245, 245, 245, 66, 80, 80, 80, 241, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 18, 0, 0, 255, 199, 0, 0, 255, 243, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 3, 252, 0, 0, 255, 10, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 235, 0, 0, 255, 240, 0, 0, 255, 137, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 7, 7, 7, 255, 117, 117, 117, 187, 255, 255, 255, 2, 0, 0, 0, 0, 23, 0, 0, 0, 0, 3, 187, 187, 187, 80, 39, 39, 39, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 51, 0, 0, 255, 223, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 3, 252, 0, 0, 255, 9, 251, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 240, 0, 0, 255, 90, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 23, 23, 23, 255, 151, 151, 151, 126, 0, 0, 0, 0, 155, 0, 0, 0, 0, 4, 255, 255, 255, 3, 157, 157, 157, 188, 14, 14, 14, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 113, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 253, 0, 0, 255, 2, 252, 0, 0, 255, 10, 251, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 213, 0, 0, 255, 34, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 59, 59, 59, 252, 216, 216, 216, 67, 0, 0, 0, 0, 24, 0, 0, 0, 0, 4, 255, 255, 255, 2, 115, 115, 115, 187, 7, 7, 7, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 140, 0, 0, 255, 241, 0, 0, 255, 237, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 2, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 250, 0, 0, 255, 8, 248, 0, 0, 255, 247, 0, 0, 255, 244, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 198, 0, 0, 255, 17, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 81, 81, 81, 240, 245, 245, 245, 66, 0, 0, 0, 0, 154, 0, 0, 0, 0, 3, 223, 223, 223, 68, 67, 67, 67, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 29, 0, 0, 255, 212, 0, 0, 255, 241, 0, 0, 255, 1, 244, 0, 0, 255, 4, 246, 0, 0, 255, 249, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 2, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 251, 0, 0, 255, 8, 249, 0, 0, 255, 247, 0, 0, 255, 244, 0, 0, 255, 241, 0, 0, 255, 237, 0, 0, 255, 241, 0, 0, 255, 118, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 11, 11, 11, 255, 148, 148, 148, 187, 255, 255, 255, 3, 0, 0, 0, 0, 25, 0, 0, 0, 0, 3, 215, 215, 215, 68, 59, 59, 59, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 35, 0, 0, 255, 215, 0, 0, 255, 239, 0, 0, 255, 1, 243, 0, 0, 255, 6, 245, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 253, 0, 0, 255, 2, 252, 0, 0, 255, 9, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 111, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 14, 14, 14, 255, 159, 159, 159, 187, 255, 255, 255, 3, 0, 0, 0, 0, 152, 0, 0, 0, 0, 4, 255, 255, 255, 3, 127, 127, 127, 188, 8, 8, 8, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 133, 0, 0, 255, 242, 0, 0, 255, 238, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 252, 0, 0, 255, 5, 250, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 1, 202, 0, 0, 255, 2, 21, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 73, 73, 73, 240, 239, 239, 239, 65, 0, 0, 0, 0, 26, 0, 0, 0, 0, 4, 255, 255, 255, 3, 146, 146, 146, 187, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 119, 0, 0, 255, 242, 0, 0, 255, 238, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 253, 0, 0, 255, 1, 253, 0, 0, 255, 7, 252, 0, 0, 255, 251, 0, 0, 255, 250, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 1, 211, 0, 0, 255, 2, 29, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 67, 67, 67, 252, 226, 226, 226, 68, 0, 0, 0, 0, 152, 0, 0, 0, 0, 3, 191, 191, 191, 68, 48, 48, 48, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 45, 0, 0, 255, 222, 0, 0, 255, 238, 0, 0, 255, 239, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 253, 0, 0, 255, 1, 252, 0, 0, 255, 1, 251, 0, 0, 255, 8, 250, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 97, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 20, 20, 20, 255, 138, 138, 138, 126, 0, 0, 0, 0, 28, 0, 0, 0, 0, 3, 238, 238, 238, 66, 72, 72, 72, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 22, 0, 0, 255, 204, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 2, 253, 0, 0, 255, 252, 0, 0, 255, 1, 251, 0, 0, 255, 8, 248, 0, 0, 255, 247, 0, 0, 255, 244, 0, 0, 255, 241, 0, 0, 255, 237, 0, 0, 255, 242, 0, 0, 255, 131, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 8, 8, 8, 255, 129, 129, 129, 187, 255, 255, 255, 3, 0, 0, 0, 0, 150, 0, 0, 0, 0, 4, 255, 255, 255, 15, 99, 99, 99, 190, 5, 5, 5, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 10, 2, 0, 0, 255, 153, 0, 0, 255, 241, 0, 0, 255, 237, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 4, 252, 0, 0, 255, 9, 250, 0, 0, 255, 249, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 188, 0, 0, 255, 11, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 87, 87, 87, 233, 253, 253, 253, 22, 0, 0, 0, 0, 29, 0, 0, 0, 0, 3, 137, 137, 137, 127, 19, 19, 19, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 99, 0, 0, 255, 241, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 245, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 3, 252, 0, 0, 255, 10, 251, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 220, 0, 0, 255, 44, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 48, 48, 48, 252, 192, 192, 192, 68, 0, 0, 0, 0, 150, 0, 0, 0, 0, 3, 196, 196, 196, 129, 40, 40, 40, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 62, 0, 0, 255, 227, 0, 0, 255, 235, 0, 0, 255, 237, 0, 0, 255, 241, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 3, 252, 0, 0, 255, 9, 251, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 75, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 30, 30, 30, 255, 175, 175, 175, 127, 0, 0, 0, 0, 30, 0, 0, 0, 0, 3, 253, 253, 253, 22, 85, 85, 85, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 11, 0, 0, 255, 188, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 252, 0, 0, 255, 3, 252, 0, 0, 255, 11, 251, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 151, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 5, 5, 5, 255, 100, 100, 100, 189, 255, 255, 255, 15, 0, 0, 0, 0, 148, 0, 0, 0, 0, 3, 255, 255, 255, 22, 102, 102, 102, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 6, 0, 0, 255, 169, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 252, 0, 0, 255, 10, 250, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 245, 0, 0, 255, 168, 0, 0, 255, 4, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 112, 112, 112, 233, 255, 255, 255, 22, 0, 0, 0, 0, 31, 0, 0, 0, 0, 3, 176, 176, 176, 128, 29, 29, 29, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 77, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 252, 0, 0, 255, 1, 251, 0, 0, 255, 11, 250, 0, 0, 255, 249, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 226, 0, 0, 255, 61, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 41, 41, 41, 255, 196, 196, 196, 128, 0, 0, 0, 0, 148, 0, 0, 0, 0, 3, 163, 163, 163, 128, 25, 25, 25, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 79, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 1, 250, 0, 0, 255, 9, 249, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 244, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 230, 0, 0, 255, 57, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 46, 46, 46, 255, 206, 206, 206, 128, 0, 0, 0, 0, 32, 0, 0, 0, 0, 3, 255, 255, 255, 22, 110, 110, 110, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 5, 0, 0, 255, 169, 0, 0, 255, 244, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 251, 0, 0, 255, 1, 251, 0, 0, 255, 12, 250, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 168, 0, 0, 255, 6, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 103, 103, 103, 233, 255, 255, 255, 22, 0, 0, 0, 0, 146, 0, 0, 0, 0, 3, 235, 235, 235, 18, 77, 77, 77, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 13, 0, 0, 255, 184, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 2, 248, 0, 0, 255, 7, 246, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 244, 0, 0, 255, 147, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 6, 6, 6, 255, 111, 111, 111, 189, 255, 255, 255, 14, 0, 0, 0, 0, 33, 0, 0, 0, 0, 3, 205, 205, 205, 129, 45, 45, 45, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 23, 58, 0, 0, 255, 230, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 249, 0, 0, 255, 250, 0, 0, 255, 249, 0, 0, 255, 248, 0, 0, 255, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 78, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 26, 26, 26, 255, 162, 162, 162, 127, 0, 0, 0, 0, 145, 0, 0, 0, 0, 4, 255, 255, 255, 3, 158, 158, 158, 175, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 99, 0, 0, 255, 229, 0, 0, 255, 228, 0, 0, 255, 232, 0, 0, 255, 236, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 247, 0, 0, 255, 3, 248, 0, 0, 255, 5, 247, 0, 0, 255, 246, 0, 0, 255, 245, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 1, 219, 0, 0, 255, 2, 40, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 54, 54, 54, 252, 205, 205, 205, 67, 0, 0, 0, 0, 34, 0, 0, 0, 0, 4, 255, 255, 255, 15, 109, 109, 109, 189, 6, 6, 6, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 9, 1, 0, 0, 255, 148, 0, 0, 255, 244, 0, 0, 255, 238, 0, 0, 255, 242, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 248, 0, 0, 255, 1, 248, 0, 0, 255, 1, 247, 0, 0, 255, 1, 246, 0, 0, 255, 10, 244, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 229, 0, 0, 255, 230, 0, 0, 255, 181, 0, 0, 255, 12, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 79, 79, 79, 233, 238, 238, 238, 18, 0, 0, 0, 0, 144, 0, 0, 0, 0, 3, 231, 231, 231, 68, 73, 73, 73, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 23, 0, 0, 255, 194, 0, 0, 255, 228, 0, 0, 255, 1, 232, 0, 0, 255, 7, 235, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 246, 0, 0, 255, 247, 0, 0, 255, 1, 247, 0, 0, 255, 1, 246, 0, 0, 255, 1, 244, 0, 0, 255, 6, 243, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 243, 0, 0, 255, 125, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 9, 9, 9, 255, 139, 139, 139, 187, 255, 255, 255, 3, 0, 0, 0, 0, 35, 0, 0, 0, 0, 3, 205, 205, 205, 68, 54, 54, 54, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 39, 0, 0, 255, 219, 0, 0, 255, 239, 0, 0, 255, 1, 241, 0, 0, 255, 2, 243, 0, 0, 255, 246, 0, 0, 255, 3, 247, 0, 0, 255, 9, 246, 0, 0, 255, 245, 0, 0, 255, 243, 0, 0, 255, 242, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 234, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 1, 96, 0, 0, 255, 1, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 13, 13, 13, 255, 160, 160, 160, 175, 255, 255, 255, 3, 0, 0, 0, 0, 142, 0, 0, 0, 0, 4, 255, 255, 255, 3, 136, 136, 136, 188, 9, 9, 9, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 116, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 228, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 244, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 1, 245, 0, 0, 255, 8, 243, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 206, 0, 0, 255, 24, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 76, 76, 76, 253, 233, 233, 233, 67, 0, 0, 0, 0, 36, 0, 0, 0, 0, 4, 255, 255, 255, 3, 138, 138, 138, 187, 9, 9, 9, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 125, 0, 0, 255, 241, 0, 0, 255, 237, 0, 0, 255, 240, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 245, 0, 0, 255, 246, 0, 0, 255, 1, 246, 0, 0, 255, 13, 245, 0, 0, 255, 243, 0, 0, 255, 242, 0, 0, 255, 240, 0, 0, 255, 239, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 230, 0, 0, 255, 226, 0, 0, 255, 225, 0, 0, 255, 191, 0, 0, 255, 23, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 73, 73, 73, 253, 231, 231, 231, 68, 0, 0, 0, 0, 142, 0, 0, 0, 0, 3, 202, 202, 202, 68, 52, 52, 52, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 35, 0, 0, 255, 201, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 228, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 243, 0, 0, 255, 2, 243, 0, 0, 255, 1, 242, 0, 0, 255, 1, 239, 0, 0, 255, 5, 237, 0, 0, 255, 235, 0, 0, 255, 238, 0, 0, 255, 102, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 14, 14, 14, 255, 162, 162, 162, 174, 255, 255, 255, 3, 0, 0, 0, 0, 37, 0, 0, 0, 0, 3, 232, 232, 232, 68, 74, 74, 74, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 24, 0, 0, 255, 206, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 243, 0, 0, 255, 3, 243, 0, 0, 255, 12, 242, 0, 0, 255, 241, 0, 0, 255, 240, 0, 0, 255, 238, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 229, 0, 0, 255, 226, 0, 0, 255, 221, 0, 0, 255, 223, 0, 0, 255, 113, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 9, 9, 9, 255, 137, 137, 137, 187, 255, 255, 255, 3, 0, 0, 0, 0, 140, 0, 0, 0, 0, 4, 255, 255, 255, 15, 107, 107, 107, 190, 6, 6, 6, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 131, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 224, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 234, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 241, 0, 0, 255, 3, 242, 0, 0, 255, 8, 241, 0, 0, 255, 239, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 238, 0, 0, 255, 188, 0, 0, 255, 13, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 80, 80, 80, 233, 240, 240, 240, 18, 0, 0, 0, 0, 38, 0, 0, 0, 0, 4, 255, 255, 255, 3, 161, 161, 161, 175, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 103, 0, 0, 255, 237, 0, 0, 255, 234, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 240, 0, 0, 255, 241, 0, 0, 255, 242, 0, 0, 255, 2, 241, 0, 0, 255, 12, 240, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 232, 0, 0, 255, 229, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 218, 0, 0, 255, 197, 0, 0, 255, 34, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 53, 53, 53, 252, 202, 202, 202, 68, 0, 0, 0, 0, 140, 0, 0, 0, 0, 3, 203, 203, 203, 129, 44, 44, 44, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 49, 0, 0, 255, 204, 0, 0, 255, 215, 0, 0, 255, 219, 0, 0, 255, 223, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 232, 0, 0, 255, 235, 0, 0, 255, 236, 0, 0, 255, 238, 0, 0, 255, 3, 240, 0, 0, 255, 8, 239, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 235, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 79, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 27, 27, 27, 255, 165, 165, 165, 127, 0, 0, 0, 0, 40, 0, 0, 0, 0, 3, 239, 239, 239, 18, 79, 79, 79, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 13, 0, 0, 255, 187, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 239, 0, 0, 255, 2, 240, 0, 0, 255, 13, 239, 0, 0, 255, 238, 0, 0, 255, 237, 0, 0, 255, 236, 0, 0, 255, 233, 0, 0, 255, 231, 0, 0, 255, 228, 0, 0, 255, 225, 0, 0, 255, 221, 0, 0, 255, 216, 0, 0, 255, 218, 0, 0, 255, 127, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 6, 6, 6, 255, 109, 109, 109, 189, 255, 255, 255, 15, 0, 0, 0, 0, 138, 0, 0, 0, 0, 3, 255, 255, 255, 22, 109, 109, 109, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 13, 3, 0, 0, 255, 144, 0, 0, 255, 214, 0, 0, 255, 213, 0, 0, 255, 218, 0, 0, 255, 222, 0, 0, 255, 226, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 234, 0, 0, 255, 235, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 2, 238, 0, 0, 255, 8, 237, 0, 0, 255, 235, 0, 0, 255, 234, 0, 0, 255, 231, 0, 0, 255, 236, 0, 0, 255, 167, 0, 0, 255, 5, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 105, 105, 105, 233, 255, 255, 255, 22, 0, 0, 0, 0, 41, 0, 0, 0, 0, 3, 167, 167, 167, 128, 26, 26, 26, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 80, 0, 0, 255, 231, 0, 0, 255, 1, 233, 0, 0, 255, 4, 235, 0, 0, 255, 236, 0, 0, 255, 237, 0, 0, 255, 238, 0, 0, 255, 1, 237, 0, 0, 255, 1, 236, 0, 0, 255, 1, 234, 0, 0, 255, 10, 232, 0, 0, 255, 230, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 220, 0, 0, 255, 216, 0, 0, 255, 212, 0, 0, 255, 200, 0, 0, 255, 47, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 45, 45, 45, 255, 204, 204, 204, 128, 0, 0, 0, 0, 138, 0, 0, 0, 0, 3, 172, 172, 172, 128, 29, 29, 29, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 63, 0, 0, 255, 202, 0, 0, 255, 209, 0, 0, 255, 213, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 224, 0, 0, 255, 227, 0, 0, 255, 230, 0, 0, 255, 231, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 1, 236, 0, 0, 255, 1, 235, 0, 0, 255, 1, 234, 0, 0, 255, 6, 233, 0, 0, 255, 230, 0, 0, 255, 229, 0, 0, 255, 222, 0, 0, 255, 58, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 42, 42, 42, 255, 198, 198, 198, 128, 0, 0, 0, 0, 42, 0, 0, 0, 0, 3, 255, 255, 255, 22, 103, 103, 103, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 5, 0, 0, 255, 167, 0, 0, 255, 234, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 235, 0, 0, 255, 1, 236, 0, 0, 255, 1, 235, 0, 0, 255, 1, 233, 0, 0, 255, 8, 232, 0, 0, 255, 230, 0, 0, 255, 228, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 219, 0, 0, 255, 215, 0, 0, 255, 210, 0, 0, 255, 1, 141, 0, 0, 255, 2, 3, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 110, 110, 110, 233, 255, 255, 255, 22, 0, 0, 0, 0, 136, 0, 0, 0, 0, 3, 248, 248, 248, 18, 84, 84, 84, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 16, 8, 0, 0, 255, 154, 0, 0, 255, 206, 0, 0, 255, 207, 0, 0, 255, 213, 0, 0, 255, 217, 0, 0, 255, 220, 0, 0, 255, 223, 0, 0, 255, 226, 0, 0, 255, 228, 0, 0, 255, 229, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 233, 0, 0, 255, 232, 0, 0, 255, 1, 231, 0, 0, 255, 6, 230, 0, 0, 255, 227, 0, 0, 255, 232, 0, 0, 255, 144, 0, 0, 255, 1, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 5, 5, 5, 255, 102, 102, 102, 189, 255, 255, 255, 14, 0, 0, 0, 0, 43, 0, 0, 0, 0, 3, 198, 198, 198, 129, 41, 41, 41, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 58, 0, 0, 255, 220, 0, 0, 255, 228, 0, 0, 255, 1, 230, 0, 0, 255, 2, 232, 0, 0, 255, 233, 0, 0, 255, 1, 233, 0, 0, 255, 1, 232, 0, 0, 255, 1, 231, 0, 0, 255, 11, 229, 0, 0, 255, 227, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 218, 0, 0, 255, 214, 0, 0, 255, 210, 0, 0, 255, 205, 0, 0, 255, 197, 0, 0, 255, 60, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 29, 29, 29, 255, 172, 172, 172, 127, 0, 0, 0, 0, 135, 0, 0, 0, 0, 4, 255, 255, 255, 3, 167, 167, 167, 175, 15, 15, 15, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 10, 76, 0, 0, 255, 198, 0, 0, 255, 201, 0, 0, 255, 207, 0, 0, 255, 211, 0, 0, 255, 215, 0, 0, 255, 219, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 1, 229, 0, 0, 255, 3, 230, 0, 0, 255, 231, 0, 0, 255, 230, 0, 0, 255, 1, 229, 0, 0, 255, 2, 228, 0, 0, 255, 226, 0, 0, 255, 1, 210, 0, 0, 255, 2, 40, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 49, 49, 49, 252, 194, 194, 194, 67, 0, 0, 0, 0, 44, 0, 0, 0, 0, 4, 255, 255, 255, 15, 100, 100, 100, 189, 5, 5, 5, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 7, 1, 0, 0, 255, 144, 0, 0, 255, 230, 0, 0, 255, 225, 0, 0, 255, 227, 0, 0, 255, 228, 0, 0, 255, 230, 0, 0, 255, 1, 230, 0, 0, 255, 15, 231, 0, 0, 255, 229, 0, 0, 255, 228, 0, 0, 255, 227, 0, 0, 255, 226, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 217, 0, 0, 255, 213, 0, 0, 255, 209, 0, 0, 255, 203, 0, 0, 255, 202, 0, 0, 255, 149, 0, 0, 255, 7, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 85, 85, 85, 233, 251, 251, 251, 22, 0, 0, 0, 0, 134, 0, 0, 0, 0, 3, 237, 237, 237, 68, 79, 79, 79, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 11, 15, 0, 0, 255, 159, 0, 0, 255, 197, 0, 0, 255, 200, 0, 0, 255, 205, 0, 0, 255, 210, 0, 0, 255, 214, 0, 0, 255, 218, 0, 0, 255, 221, 0, 0, 255, 222, 0, 0, 255, 225, 0, 0, 255, 1, 226, 0, 0, 255, 1, 228, 0, 0, 255, 1, 228, 0, 0, 255, 7, 227, 0, 0, 255, 226, 0, 0, 255, 224, 0, 0, 255, 222, 0, 0, 255, 227, 0, 0, 255, 121, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 8, 8, 8, 255, 131, 131, 131, 187, 255, 255, 255, 3, 0, 0, 0, 0, 45, 0, 0, 0, 0, 3, 193, 193, 193, 68, 49, 49, 49, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 39, 0, 0, 255, 207, 0, 0, 255, 224, 0, 0, 255, 1, 226, 0, 0, 255, 2, 227, 0, 0, 255, 228, 0, 0, 255, 1, 228, 0, 0, 255, 1, 227, 0, 0, 255, 1, 225, 0, 0, 255, 11, 223, 0, 0, 255, 222, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 208, 0, 0, 255, 202, 0, 0, 255, 197, 0, 0, 255, 193, 0, 0, 255, 73, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 19, 19, 19, 255, 136, 136, 136, 126, 0, 0, 0, 0, 133, 0, 0, 0, 0, 4, 255, 255, 255, 3, 144, 144, 144, 188, 10, 10, 10, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 88, 0, 0, 255, 190, 0, 0, 255, 193, 0, 0, 255, 200, 0, 0, 255, 205, 0, 0, 255, 209, 0, 0, 255, 213, 0, 0, 255, 215, 0, 0, 255, 218, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 224, 0, 0, 255, 3, 225, 0, 0, 255, 7, 224, 0, 0, 255, 222, 0, 0, 255, 220, 0, 0, 255, 221, 0, 0, 255, 193, 0, 0, 255, 25, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 70, 70, 70, 253, 227, 227, 227, 67, 0, 0, 0, 0, 46, 0, 0, 0, 0, 4, 255, 255, 255, 3, 129, 129, 129, 187, 8, 8, 8, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 120, 0, 0, 255, 224, 0, 0, 255, 220, 0, 0, 255, 222, 0, 0, 255, 223, 0, 0, 255, 224, 0, 0, 255, 225, 0, 0, 255, 1, 225, 0, 0, 255, 14, 224, 0, 0, 255, 223, 0, 0, 255, 221, 0, 0, 255, 219, 0, 0, 255, 216, 0, 0, 255, 214, 0, 0, 255, 211, 0, 0, 255, 207, 0, 0, 255, 202, 0, 0, 255, 197, 0, 0, 255, 192, 0, 0, 255, 153, 0, 0, 255, 14, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 70, 70, 70, 240, 234, 234, 234, 66, 0, 0, 0, 0, 132, 0, 0, 0, 0, 3, 221, 221, 221, 74, 60, 60, 60, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 23, 0, 0, 255, 160, 0, 0, 255, 187, 0, 0, 255, 193, 0, 0, 255, 199, 0, 0, 255, 204, 0, 0, 255, 208, 0, 0, 255, 212, 0, 0, 255, 214, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 219, 0, 0, 255, 221, 0, 0, 255, 222, 0, 0, 255, 1, 222, 0, 0, 255, 7, 221, 0, 0, 255, 220, 0, 0, 255, 218, 0, 0, 255, 216, 0, 0, 255, 219, 0, 0, 255, 98, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 11, 11, 11, 255, 153, 153, 153, 174, 255, 255, 255, 3, 0, 0, 0, 0, 47, 0, 0, 0, 0, 3, 226, 226, 226, 68, 69, 69, 69, 253, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 25, 0, 0, 255, 191, 0, 0, 255, 218, 0, 0, 255, 217, 0, 0, 255, 219, 0, 0, 255, 221, 0, 0, 255, 222, 0, 0, 255, 1, 222, 0, 0, 255, 14, 221, 0, 0, 255, 220, 0, 0, 255, 219, 0, 0, 255, 218, 0, 0, 255, 215, 0, 0, 255, 212, 0, 0, 255, 209, 0, 0, 255, 205, 0, 0, 255, 201, 0, 0, 255, 196, 0, 0, 255, 188, 0, 0, 255, 184, 0, 0, 255, 82, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 11, 11, 11, 255, 153, 153, 153, 188, 255, 255, 255, 9, 0, 0, 0, 0, 131, 0, 0, 0, 0, 3, 166, 166, 166, 133, 20, 20, 20, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 15, 1, 0, 0, 255, 96, 0, 0, 255, 180, 0, 0, 255, 184, 0, 0, 255, 191, 0, 0, 255, 197, 0, 0, 255, 202, 0, 0, 255, 206, 0, 0, 255, 210, 0, 0, 255, 212, 0, 0, 255, 214, 0, 0, 255, 216, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 219, 0, 0, 255, 2, 218, 0, 0, 255, 6, 216, 0, 0, 255, 214, 0, 0, 255, 216, 0, 0, 255, 174, 0, 0, 255, 14, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 73, 73, 73, 233, 226, 226, 226, 18, 0, 0, 0, 0, 48, 0, 0, 0, 0, 4, 255, 255, 255, 3, 152, 152, 152, 175, 11, 11, 11, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 97, 0, 0, 255, 216, 0, 0, 255, 213, 0, 0, 255, 216, 0, 0, 255, 217, 0, 0, 255, 218, 0, 0, 255, 219, 0, 0, 255, 1, 219, 0, 0, 255, 14, 217, 0, 0, 255, 216, 0, 0, 255, 215, 0, 0, 255, 213, 0, 0, 255, 210, 0, 0, 255, 208, 0, 0, 255, 204, 0, 0, 255, 199, 0, 0, 255, 194, 0, 0, 255, 187, 0, 0, 255, 181, 0, 0, 255, 151, 0, 0, 255, 23, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 102, 102, 102, 243, 255, 255, 255, 18, 0, 0, 0, 0, 131, 0, 0, 0, 0, 3, 177, 177, 177, 133, 25, 25, 25, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 14, 10, 0, 0, 255, 142, 0, 0, 255, 177, 0, 0, 255, 184, 0, 0, 255, 191, 0, 0, 255, 196, 0, 0, 255, 201, 0, 0, 255, 205, 0, 0, 255, 207, 0, 0, 255, 209, 0, 0, 255, 212, 0, 0, 255, 213, 0, 0, 255, 214, 0, 0, 255, 216, 0, 0, 255, 1, 216, 0, 0, 255, 4, 215, 0, 0, 255, 214, 0, 0, 255, 212, 0, 0, 255, 210, 0, 0, 255, 1, 75, 0, 0, 255, 1, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 24, 24, 24, 255, 157, 157, 157, 127, 0, 0, 0, 0, 50, 0, 0, 0, 0, 3, 226, 226, 226, 18, 72, 72, 72, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 8, 14, 0, 0, 255, 171, 0, 0, 255, 213, 0, 0, 255, 211, 0, 0, 255, 213, 0, 0, 255, 214, 0, 0, 255, 215, 0, 0, 255, 216, 0, 0, 255, 1, 215, 0, 0, 255, 13, 214, 0, 0, 255, 212, 0, 0, 255, 210, 0, 0, 255, 208, 0, 0, 255, 205, 0, 0, 255, 202, 0, 0, 255, 198, 0, 0, 255, 193, 0, 0, 255, 186, 0, 0, 255, 178, 0, 0, 255, 172, 0, 0, 255, 64, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 111, 111, 111, 243, 255, 255, 255, 18, 0, 0, 0, 0, 131, 0, 0, 0, 0, 3, 222, 222, 222, 72, 48, 48, 48, 246, 0, 0, 0, 255, 2, 0, 0, 0, 255, 12, 2, 0, 0, 255, 120, 0, 0, 255, 178, 0, 0, 255, 183, 0, 0, 255, 189, 0, 0, 255, 194, 0, 0, 255, 198, 0, 0, 255, 202, 0, 0, 255, 205, 0, 0, 255, 207, 0, 0, 255, 209, 0, 0, 255, 211, 0, 0, 255, 3, 212, 0, 0, 255, 7, 211, 0, 0, 255, 210, 0, 0, 255, 207, 0, 0, 255, 210, 0, 0, 255, 151, 0, 0, 255, 6, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 98, 98, 98, 233, 255, 255, 255, 22, 0, 0, 0, 0, 51, 0, 0, 0, 0, 3, 157, 157, 157, 128, 23, 23, 23, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 74, 0, 0, 255, 206, 0, 0, 255, 207, 0, 0, 255, 209, 0, 0, 255, 211, 0, 0, 255, 212, 0, 0, 255, 2, 212, 0, 0, 255, 13, 211, 0, 0, 255, 209, 0, 0, 255, 208, 0, 0, 255, 206, 0, 0, 255, 204, 0, 0, 255, 200, 0, 0, 255, 197, 0, 0, 255, 191, 0, 0, 255, 186, 0, 0, 255, 178, 0, 0, 255, 164, 0, 0, 255, 39, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 7, 7, 7, 255, 142, 142, 142, 184, 255, 255, 255, 8, 0, 0, 0, 0, 131, 0, 0, 0, 0, 3, 255, 255, 255, 13, 96, 96, 96, 242, 0, 0, 0, 255, 3, 0, 0, 0, 255, 12, 83, 0, 0, 255, 179, 0, 0, 255, 181, 0, 0, 255, 188, 0, 0, 255, 193, 0, 0, 255, 197, 0, 0, 255, 200, 0, 0, 255, 202, 0, 0, 255, 204, 0, 0, 255, 206, 0, 0, 255, 208, 0, 0, 255, 209, 0, 0, 255, 1, 209, 0, 0, 255, 3, 208, 0, 0, 255, 206, 0, 0, 255, 204, 0, 0, 255, 1, 197, 0, 0, 255, 2, 54, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 35, 35, 35, 255, 190, 190, 190, 127, 0, 0, 0, 0, 52, 0, 0, 0, 0, 3, 255, 255, 255, 22, 96, 96, 96, 233, 0, 0, 0, 255, 3, 0, 0, 0, 255, 6, 6, 0, 0, 255, 149, 0, 0, 255, 207, 0, 0, 255, 203, 0, 0, 255, 206, 0, 0, 255, 208, 0, 0, 255, 2, 209, 0, 0, 255, 13, 208, 0, 0, 255, 207, 0, 0, 255, 206, 0, 0, 255, 204, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 194, 0, 0, 255, 190, 0, 0, 255, 184, 0, 0, 255, 180, 0, 0, 255, 146, 0, 0, 255, 10, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 29, 29, 29, 254, 184, 184, 184, 114, 0, 0, 0, 0, 132, 0, 0, 0, 0, 4, 255, 255, 255, 8, 128, 128, 128, 183, 6, 6, 6, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 12, 47, 0, 0, 255, 171, 0, 0, 255, 180, 0, 0, 255, 185, 0, 0, 255, 190, 0, 0, 255, 194, 0, 0, 255, 197, 0, 0, 255, 200, 0, 0, 255, 202, 0, 0, 255, 203, 0, 0, 255, 204, 0, 0, 255, 205, 0, 0, 255, 1, 205, 0, 0, 255, 7, 204, 0, 0, 255, 202, 0, 0, 255, 199, 0, 0, 255, 203, 0, 0, 255, 128, 0, 0, 255, 2, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 1, 1, 1, 255, 126, 126, 126, 237, 255, 255, 255, 22, 0, 0, 0, 0, 53, 0, 0, 0, 0, 3, 189, 189, 189, 128, 34, 34, 34, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 53, 0, 0, 255, 193, 0, 0, 255, 200, 0, 0, 255, 201, 0, 0, 255, 203, 0, 0, 255, 204, 0, 0, 255, 205, 0, 0, 255, 1, 205, 0, 0, 255, 11, 203, 0, 0, 255, 202, 0, 0, 255, 201, 0, 0, 255, 198, 0, 0, 255, 195, 0, 0, 255, 192, 0, 0, 255, 188, 0, 0, 255, 182, 0, 0, 255, 181, 0, 0, 255, 112, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 53, 53, 53, 240, 213, 213, 213, 25, 0, 0, 0, 0, 133, 0, 0, 0, 0, 3, 177, 177, 177, 128, 26, 26, 26, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 10, 15, 0, 0, 255, 153, 0, 0, 255, 180, 0, 0, 255, 184, 0, 0, 255, 188, 0, 0, 255, 191, 0, 0, 255, 194, 0, 0, 255, 196, 0, 0, 255, 199, 0, 0, 255, 200, 0, 0, 255, 2, 201, 0, 0, 255, 7, 200, 0, 0, 255, 199, 0, 0, 255, 197, 0, 0, 255, 196, 0, 0, 255, 181, 0, 0, 255, 36, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 44, 44, 44, 252, 194, 194, 194, 80, 0, 0, 0, 0, 54, 0, 0, 0, 0, 4, 255, 255, 255, 22, 125, 125, 125, 237, 1, 1, 1, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 7, 2, 0, 0, 255, 125, 0, 0, 255, 199, 0, 0, 255, 195, 0, 0, 255, 198, 0, 0, 255, 200, 0, 0, 255, 201, 0, 0, 255, 1, 201, 0, 0, 255, 11, 200, 0, 0, 255, 199, 0, 0, 255, 198, 0, 0, 255, 196, 0, 0, 255, 193, 0, 0, 255, 189, 0, 0, 255, 185, 0, 0, 255, 180, 0, 0, 255, 179, 0, 0, 255, 74, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 107, 107, 107, 230, 255, 255, 255, 15, 0, 0, 0, 0, 133, 0, 0, 0, 0, 3, 232, 232, 232, 72, 54, 54, 54, 246, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 1, 0, 0, 255, 121, 0, 0, 255, 181, 0, 0, 255, 1, 185, 0, 0, 255, 6, 188, 0, 0, 255, 191, 0, 0, 255, 194, 0, 0, 255, 195, 0, 0, 255, 196, 0, 0, 255, 197, 0, 0, 255, 1, 197, 0, 0, 255, 6, 196, 0, 0, 255, 193, 0, 0, 255, 190, 0, 0, 255, 193, 0, 0, 255, 104, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 7, 7, 7, 255, 121, 121, 121, 187, 255, 255, 255, 2, 0, 0, 0, 0, 55, 0, 0, 0, 0, 3, 192, 192, 192, 80, 43, 43, 43, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 35, 0, 0, 255, 177, 0, 0, 255, 191, 0, 0, 255, 192, 0, 0, 255, 195, 0, 0, 255, 196, 0, 0, 255, 197, 0, 0, 255, 1, 197, 0, 0, 255, 10, 196, 0, 0, 255, 195, 0, 0, 255, 193, 0, 0, 255, 190, 0, 0, 255, 187, 0, 0, 255, 183, 0, 0, 255, 179, 0, 0, 255, 168, 0, 0, 255, 38, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 12, 12, 12, 255, 112, 112, 112, 136, 255, 255, 255, 1, 0, 0, 0, 0, 133, 0, 0, 0, 0, 3, 255, 255, 255, 13, 103, 103, 103, 242, 0, 0, 0, 255, 3, 0, 0, 0, 255, 9, 81, 0, 0, 255, 179, 0, 0, 255, 178, 0, 0, 255, 182, 0, 0, 255, 185, 0, 0, 255, 188, 0, 0, 255, 190, 0, 0, 255, 191, 0, 0, 255, 193, 0, 0, 255, 2, 192, 0, 0, 255, 6, 190, 0, 0, 255, 187, 0, 0, 255, 186, 0, 0, 255, 161, 0, 0, 255, 21, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 63, 63, 63, 252, 220, 220, 220, 67, 0, 0, 0, 0, 56, 0, 0, 0, 0, 4, 255, 255, 255, 2, 121, 121, 121, 187, 7, 7, 7, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 101, 0, 0, 255, 187, 0, 0, 255, 186, 0, 0, 255, 189, 0, 0, 255, 191, 0, 0, 255, 192, 0, 0, 255, 193, 0, 0, 255, 1, 192, 0, 0, 255, 9, 191, 0, 0, 255, 189, 0, 0, 255, 187, 0, 0, 255, 184, 0, 0, 255, 180, 0, 0, 255, 179, 0, 0, 255, 146, 0, 0, 255, 9, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 32, 32, 32, 254, 195, 195, 195, 118, 0, 0, 0, 0, 134, 0, 0, 0, 0, 4, 255, 255, 255, 8, 135, 135, 135, 183, 7, 7, 7, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 17, 43, 0, 0, 255, 168, 0, 0, 255, 176, 0, 0, 255, 179, 0, 0, 255, 182, 0, 0, 255, 185, 0, 0, 255, 186, 0, 0, 255, 187, 0, 0, 255, 188, 0, 0, 255, 189, 0, 0, 255, 187, 0, 0, 255, 186, 0, 0, 255, 183, 0, 0, 255, 180, 0, 0, 255, 179, 0, 0, 255, 79, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 4, 13, 13, 13, 255, 152, 152, 152, 187, 255, 255, 255, 3, 0, 0, 0, 0, 57, 0, 0, 0, 0, 3, 220, 220, 220, 68, 62, 62, 62, 252, 0, 0, 0, 255, 3, 0, 0, 0, 255, 17, 20, 0, 0, 255, 155, 0, 0, 255, 180, 0, 0, 255, 182, 0, 0, 255, 185, 0, 0, 255, 187, 0, 0, 255, 188, 0, 0, 255, 189, 0, 0, 255, 188, 0, 0, 255, 187, 0, 0, 255, 185, 0, 0, 255, 183, 0, 0, 255, 180, 0, 0, 255, 177, 0, 0, 255, 178, 0, 0, 255, 109, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 60, 60, 60, 240, 227, 227, 227, 25, 0, 0, 0, 0, 135, 0, 0, 0, 0, 3, 185, 185, 185, 128, 30, 30, 30, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 17, 12, 0, 0, 255, 148, 0, 0, 255, 174, 0, 0, 255, 175, 0, 0, 255, 179, 0, 0, 255, 181, 0, 0, 255, 182, 0, 0, 255, 183, 0, 0, 255, 184, 0, 0, 255, 183, 0, 0, 255, 182, 0, 0, 255, 180, 0, 0, 255, 177, 0, 0, 255, 178, 0, 0, 255, 138, 0, 0, 255, 11, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 76, 76, 76, 240, 241, 241, 241, 65, 0, 0, 0, 0, 58, 0, 0, 0, 0, 4, 255, 255, 255, 3, 152, 152, 152, 187, 12, 12, 12, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 76, 0, 0, 255, 176, 0, 0, 255, 1, 178, 0, 0, 255, 3, 181, 0, 0, 255, 182, 0, 0, 255, 183, 0, 0, 255, 1, 183, 0, 0, 255, 7, 181, 0, 0, 255, 179, 0, 0, 255, 177, 0, 0, 255, 173, 0, 0, 255, 174, 0, 0, 255, 70, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 113, 113, 113, 230, 255, 255, 255, 15, 0, 0, 0, 0, 135, 0, 0, 0, 0, 3, 238, 238, 238, 72, 59, 59, 59, 246, 0, 0, 0, 255, 2, 0, 0, 0, 255, 9, 1, 0, 0, 255, 114, 0, 0, 255, 173, 0, 0, 255, 171, 0, 0, 255, 174, 0, 0, 255, 176, 0, 0, 255, 177, 0, 0, 255, 178, 0, 0, 255, 179, 0, 0, 255, 1, 181, 0, 0, 255, 5, 175, 0, 0, 255, 159, 0, 0, 255, 129, 0, 0, 255, 45, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 21, 21, 21, 255, 145, 145, 145, 126, 0, 0, 0, 0, 60, 0, 0, 0, 0, 3, 242, 242, 242, 66, 75, 75, 75, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 7, 9, 0, 0, 255, 101, 0, 0, 255, 147, 0, 0, 255, 170, 0, 0, 255, 179, 0, 0, 255, 180, 0, 0, 255, 179, 0, 0, 255, 1, 178, 0, 0, 255, 7, 176, 0, 0, 255, 175, 0, 0, 255, 173, 0, 0, 255, 170, 0, 0, 255, 161, 0, 0, 255, 33, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 4, 13, 13, 13, 255, 121, 121, 121, 136, 255, 255, 255, 1, 0, 0, 0, 0, 135, 0, 0, 0, 0, 3, 255, 255, 255, 13, 108, 108, 108, 242, 0, 0, 0, 255, 3, 0, 0, 0, 255, 14, 73, 0, 0, 255, 169, 0, 0, 255, 167, 0, 0, 255, 169, 0, 0, 255, 173, 0, 0, 255, 176, 0, 0, 255, 178, 0, 0, 255, 170, 0, 0, 255, 150, 0, 0, 255, 111, 0, 0, 255, 66, 0, 0, 255, 28, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 3, 91, 91, 91, 233, 255, 255, 255, 22, 0, 0, 0, 0, 61, 0, 0, 0, 0, 3, 145, 145, 145, 127, 21, 21, 21, 255, 0, 0, 0, 255, 4, 0, 0, 0, 255, 10, 13, 0, 0, 255, 49, 0, 0, 255, 90, 0, 0, 255, 135, 0, 0, 255, 163, 0, 0, 255, 176, 0, 0, 255, 178, 0, 0, 255, 174, 0, 0, 255, 171, 0, 0, 255, 168, 0, 0, 255, 1, 136, 0, 0, 255, 2, 7, 0, 0, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 36, 36, 36, 254, 202, 202, 202, 118, 0, 0, 0, 0, 136, 0, 0, 0, 0, 4, 255, 255, 255, 8, 143, 143, 143, 184, 7, 7, 7, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 10, 36, 0, 0, 255, 158, 0, 0, 255, 169, 0, 0, 255, 168, 0, 0, 255, 155, 0, 0, 255, 127, 0, 0, 255, 85, 0, 0, 255, 46, 0, 0, 255, 12, 0, 0, 255, 0, 0, 0, 255, 7, 0, 0, 0, 255, 3, 31, 31, 31, 255, 181, 181, 181, 127, 0, 0, 0, 0, 62, 0, 0, 0, 0, 3, 255, 255, 255, 22, 90, 90, 90, 233, 0, 0, 0, 255, 7, 0, 0, 0, 255, 10, 4, 0, 0, 255, 28, 0, 0, 255, 67, 0, 0, 255, 109, 0, 0, 255, 145, 0, 0, 255, 164, 0, 0, 255, 169, 0, 0, 255, 170, 0, 0, 255, 99, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 66, 66, 66, 240, 236, 236, 236, 25, 0, 0, 0, 0, 137, 0, 0, 0, 0, 3, 188, 188, 188, 115, 30, 30, 30, 254, 0, 0, 0, 255, 2, 0, 0, 0, 255, 7, 9, 0, 0, 255, 112, 0, 0, 255, 99, 0, 0, 255, 59, 0, 0, 255, 23, 0, 0, 255, 3, 0, 0, 255, 0, 0, 0, 255, 10, 0, 0, 0, 255, 3, 119, 119, 119, 237, 255, 255, 255, 22, 0, 0, 0, 0, 63, 0, 0, 0, 0, 3, 181, 181, 181, 128, 32, 32, 32, 255, 0, 0, 0, 255, 10, 0, 0, 0, 255, 6, 11, 0, 0, 255, 43, 0, 0, 255, 79, 0, 0, 255, 121, 0, 0, 255, 53, 0, 0, 255, 0, 0, 0, 255, 3, 0, 0, 0, 255, 3, 119, 119, 119, 230, 255, 255, 255, 15, 0, 0, 0, 0, 137, 0, 0, 0, 0, 3, 220, 220, 220, 25, 55, 55, 55, 240, 0, 0, 0, 255, 3, 0, 0, 0, 255, 2, 4, 0, 0, 255, 0, 0, 0, 255, 13, 0, 0, 0, 255, 3, 40, 40, 40, 253, 183, 183, 183, 80, 0, 0, 0, 0, 64, 0, 0, 0, 0, 3, 255, 255, 255, 22, 119, 119, 119, 237, 0, 0, 0, 255, 13, 0, 0, 0, 255, 1, 2, 0, 0, 255, 4, 0, 0, 0, 255, 4, 14, 14, 14, 255, 130, 130, 130, 136, 255, 255, 255, 1, 0, 0, 0, 0, 137, 0, 0, 0, 0, 3, 255, 255, 255, 15, 108, 108, 108, 230, 0, 0, 0, 255, 17, 0, 0, 0, 255, 5, 5, 5, 5, 255, 45, 45, 45, 255, 149, 149, 149, 175, 255, 255, 255, 2, 0, 0, 0, 0, 65, 0, 0, 0, 0, 4, 201, 201, 201, 78, 83, 83, 83, 245, 15, 15, 15, 255, 0, 0, 0, 255, 17, 0, 0, 0, 255, 3, 39, 39, 39, 254, 211, 211, 211, 118, 0, 0, 0, 0, 138, 0, 0, 0, 0, 4, 255, 255, 255, 1, 117, 117, 117, 137, 13, 13, 13, 255, 0, 0, 0, 255, 13, 0, 0, 0, 255, 7, 16, 16, 16, 255, 38, 38, 38, 252, 86, 86, 86, 238, 124, 124, 124, 182, 196, 196, 196, 120, 255, 255, 255, 11, 0, 0, 0, 0, 67, 0, 0, 0, 0, 7, 235, 235, 235, 70, 133, 133, 133, 134, 110, 110, 110, 231, 53, 53, 53, 240, 26, 26, 26, 255, 5, 5, 5, 255, 0, 0, 0, 255, 13, 0, 0, 0, 255, 3, 71, 71, 71, 240, 242, 242, 242, 25, 0, 0, 0, 0, 139, 0, 0, 0, 0, 3, 199, 199, 199, 119, 32, 32, 32, 254, 0, 0, 0, 255, 9, 0, 0, 0, 255, 9, 6, 6, 6, 255, 28, 28, 28, 255, 60, 60, 60, 240, 120, 120, 120, 231, 142, 142, 142, 134, 202, 202, 202, 72, 255, 255, 255, 17, 255, 255, 255, 2, 0, 0, 0, 0, 71, 0, 0, 0, 0, 8, 255, 255, 255, 15, 208, 208, 208, 24, 175, 175, 175, 120, 126, 126, 126, 183, 87, 87, 87, 238, 38, 38, 38, 253, 17, 17, 17, 255, 0, 0, 0, 255, 10, 0, 0, 0, 255, 3, 126, 126, 126, 230, 255, 255, 255, 15, 0, 0, 0, 0, 139, 0, 0, 0, 0, 3, 229, 229, 229, 25, 64, 64, 64, 246, 0, 0, 0, 255, 5, 0, 0, 0, 255, 9, 1, 1, 1, 255, 19, 19, 19, 255, 44, 44, 44, 252, 96, 96, 96, 238, 136, 136, 136, 182, 187, 187, 187, 120, 227, 227, 227, 24, 255, 255, 255, 15, 0, 0, 0, 0, 78, 0, 0, 0, 0, 9, 255, 255, 255, 3, 255, 255, 255, 17, 203, 203, 203, 72, 143, 143, 143, 134, 121, 121, 121, 231, 60, 60, 60, 240, 29, 29, 29, 255, 10, 10, 10, 255, 0, 0, 0, 255, 5, 0, 0, 0, 255, 4, 16, 16, 16, 255, 148, 148, 148, 140, 255, 255, 255, 1, 0, 0, 0, 0, 139, 0, 0, 0, 0, 4, 255, 255, 255, 7, 135, 135, 135, 176, 15, 15, 15, 255, 0, 0, 0, 255, 1, 0, 0, 0, 255, 9, 12, 12, 12, 255, 35, 35, 35, 255, 71, 71, 71, 240, 97, 97, 97, 182, 152, 152, 152, 127, 216, 216, 216, 72, 255, 255, 255, 17, 255, 255, 255, 2, 0, 0, 0, 0, 85, 0, 0, 0, 0, 8, 255, 255, 255, 15, 227, 227, 227, 24, 190, 190, 190, 120, 96, 96, 96, 134, 91, 91, 91, 231, 47, 47, 47, 253, 19, 19, 19, 255, 0, 0, 0, 255, 2, 0, 0, 0, 255, 3, 77, 77, 77, 244, 227, 227, 227, 69, 0, 0, 0, 0, 141, 0, 0, 0, 0, 10, 255, 255, 255, 11, 137, 137, 137, 184, 67, 67, 67, 255, 54, 54, 54, 255, 105, 105, 105, 246, 109, 109, 109, 136, 200, 200, 200, 120, 253, 253, 253, 24, 255, 255, 255, 2, 0, 0, 0, 0, 93, 0, 0, 0, 0, 9, 255, 255, 255, 15, 218, 218, 218, 72, 152, 152, 152, 127, 104, 104, 104, 191, 76, 76, 76, 255, 49, 49, 49, 255, 111, 111, 111, 246, 178, 178, 178, 78, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 0, 0, 0, 1, 0, 0, 0, 0, 41, 0, 0, 0, 0};

void betray_set_icon_init()
{
	unsigned char *pixels;
	HICON hIcon;
	uint i;

	HXAFile file;
	return;
	file.node_array = NULL;
	file.node_count = 0;
	file.version = 0;
	pixels = malloc(256 * 256 * 4);

	unpack(pixels, icon, sizeof icon, 4);
	betray_set_icon(256, 256, pixels);
						

/*
	hxa_load_png(&file, "icon.png");

	hxa_print_layer_runlength(file.node_array->content.image.image_stack.layers, 
					file.node_array->content.image.resolution[0] *
					file.node_array->content.image.resolution[1]);

	betray_set_icon(file.node_array->content.image.resolution[0],
									 file.node_array->content.image.resolution[1], 
									 file.node_array->content.image.image_stack.layers->data.uint8_data);
*/
									 
									 /*	for(i = 0; i < 32 * 32 * 4; i++)
		pixels[i] = i;
	for(i = 0; i < 32 * 32 * 4; i++)
		mask[i] = 255;
	hIcon = CreateIconFromPixels(32, 32, pixels);
//	DrawIcon(betray_plugin_windows_device_context_handle_get(), 10, 20, hIcon); 
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    //This will ensure that the application icon gets changed too.
    SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(GetWindow(hWnd, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);*/
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
//	HGLRC (APIENTRY *betray_wglCreateContextAttribsARB)(HDC hDC, HGLRC hshareContext, const int *attribList);
	void *(APIENTRY *betray_wglCreateContextAttribsARB)(struct HDC__ *, HGLRC hshareContext, const int *attribList);
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
#ifdef FORGE_RELEASE_BUILD
		printf("OpenGL version: %s\n", glGetString(GL_VERSION));
		printf("OpenGL extensions:\n %s\n", glGetString(GL_EXTENSIONS));
#endif
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
#ifdef FORGE_RELEASE_BUILD
	if(context)
	{
		char *text;
		text = glGetString(GL_VERSION);
		printf("OpenGL version: %s\n", glGetString(GL_VERSION));
		printf("OpenGL extensions:\n %s\n", glGetString(GL_EXTENSIONS));
	}
#endif
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
			betray_CloseTouchInputHandle = (BOOL (__stdcall *)(LPARAM ))GetProcAddress(dll, "CloseTouchInputHandle");
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
betray_set_icon_init();
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
	static GLsync fence = (void *)~0;
	return;
	if(fence == (void *)~0)
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
int main(int argc, char **argv); /* extern but DO NOT put an extern before this. */

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