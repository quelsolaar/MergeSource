

#include "betray_plugin_api.h"
#include <windows.h>
#include <winreg.h>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>

#define B_DLL_MAX_PATH 260

typedef enum tagNPResult
{
	NP_OK = 0,
    NP_ERR_DEVICE_NOT_PRESENT,
	NP_ERR_UNSUPPORTED_OS,
	NP_ERR_INVALID_ARG,
	NP_ERR_DLL_NOT_FOUND,
	NP_ERR_NO_DATA,
	NP_ERR_INTERNAL_DATA
} NPRESULT;

typedef struct{
	unsigned short wNPStatus;
	unsigned short wPFrameSignature;
	unsigned long  dwNPIOData;
	float fNPRoll;
	float fNPPitch;
	float fNPYaw;
	float fNPX;
	float fNPY;
	float fNPZ;
	float fNPRawX;
	float fNPRawY;
	float fNPRawZ;
	float fNPDeltaX;
	float fNPDeltaY;
	float fNPDeltaZ;
	float fNPSmoothX;
	float fNPSmoothY;
	float fNPSmoothZ;
}BTrackData;

//
// Typedef for pointer to the notify callback function that is implemented within
// the client -- this function receives head tracker reports from the game client API
//
typedef NPRESULT (__stdcall *PF_NOTIFYCALLBACK)( unsigned short, unsigned short );

typedef struct tagTrackIRSignature
{
	char DllSignature[200];
	char AppSignature[200];

} SIGNATUREDATA, *LPTRACKIRSIGNATURE;

NPRESULT (__stdcall *gpfNP_RegisterWindowHandle)(HWND) = NULL;
NPRESULT (__stdcall *gpfNP_UnregisterWindowHandle)(void) = NULL;
NPRESULT (__stdcall *gpfNP_RegisterProgramProfileID)(unsigned short) = NULL;
NPRESULT (__stdcall *gpfNP_QueryVersion)(unsigned short*) = NULL;
NPRESULT (__stdcall *gpfNP_RequestData)(unsigned short) = NULL;
NPRESULT (__stdcall *gpfNP_GetSignature)(LPTRACKIRSIGNATURE);
NPRESULT (__stdcall *gpfNP_GetData)(BTrackData *data);
//NPRESULT (__stdcall *PF_NP_REGISTERNOTIFY)(PF_NOTIFYCALLBACK);
NPRESULT (__stdcall *PF_NP_UNREGISTERNOTIFY)(void) = NULL;
NPRESULT (__stdcall *gpfNP_StartCursor)(void) = NULL;
NPRESULT (__stdcall *gpfNP_StopCursor)(void) = NULL;
NPRESULT (__stdcall *gpfNP_ReCenter)(void) = NULL;
NPRESULT (__stdcall *gpfNP_StartDataTransmission)(void) = NULL;
NPRESULT (__stdcall *gpfNP_StopDataTransmission)(void) = NULL;

HMODULE ghNPClientDLL = (HMODULE)NULL;

uint betray_settings_handle[3];


boolean trackir_dll_init( LPTSTR pszDLLPath)
{
	NPRESULT result = NP_OK;
    TCHAR szFullPath[MAX_PATH * 2];
    
    if(pszDLLPath == NULL)
        return FALSE;
    
    strcpy(szFullPath, pszDLLPath);

    if(lstrlen(szFullPath) > 0)        
	{
	   strcat(szFullPath, "\\");
	}
	
    #if defined(_WIN64) || defined(__amd64__)
	    strcat(szFullPath, "NPClient64.dll");
    #else	    
        strcat(szFullPath, "NPClient.dll");
    #endif

    ghNPClientDLL = LoadLibrary(szFullPath);
	if(NULL != ghNPClientDLL)
	{
		SIGNATUREDATA pSignature;
		SIGNATUREDATA verifySignature;
		NPRESULT vresult;

		gpfNP_GetSignature = GetProcAddress(ghNPClientDLL, "NP_GetSignature");
		strcpy(verifySignature.DllSignature, "precise head tracking\n put your head into the game\n now go look around\n\n Copyright EyeControl Technologies");
		strcpy(verifySignature.AppSignature, "hardware camera\n software processing data\n track user movement\n\n Copyright EyeControl Technologies");
		vresult = gpfNP_GetSignature(&pSignature);

		if( vresult == NP_OK )
		{
			if(strcmp(verifySignature.DllSignature,pSignature.DllSignature) == 0 &&
				strcmp(verifySignature.AppSignature,pSignature.AppSignature) == 0)
			{
				gpfNP_RegisterWindowHandle     = GetProcAddress( ghNPClientDLL, "NP_RegisterWindowHandle");
				gpfNP_UnregisterWindowHandle   = GetProcAddress( ghNPClientDLL, "NP_UnregisterWindowHandle");
				gpfNP_RegisterProgramProfileID = GetProcAddress( ghNPClientDLL, "NP_RegisterProgramProfileID");
				gpfNP_QueryVersion             = GetProcAddress( ghNPClientDLL, "NP_QueryVersion");
				gpfNP_RequestData              = GetProcAddress( ghNPClientDLL, "NP_RequestData");
				gpfNP_GetData                  = GetProcAddress( ghNPClientDLL, "NP_GetData");
				gpfNP_StartCursor              = GetProcAddress( ghNPClientDLL, "NP_StartCursor");
				gpfNP_StopCursor               = GetProcAddress( ghNPClientDLL, "NP_StopCursor");
				gpfNP_ReCenter	               = GetProcAddress( ghNPClientDLL, "NP_ReCenter");
				gpfNP_StartDataTransmission    = GetProcAddress( ghNPClientDLL, "NP_StartDataTransmission");
				gpfNP_StopDataTransmission     = GetProcAddress( ghNPClientDLL, "NP_StopDataTransmission");
				return TRUE;
			}
		}
	}
	return FALSE;
}

boolean trackir_dll_locatate(LPTSTR pszPath)
{
	HKEY pKey = NULL;
	LPTSTR pszValue;
	DWORD dwSize;
	
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\NaturalPoint\\NATURALPOINT\\NPClient Location", 0, KEY_READ, &pKey) != ERROR_SUCCESS)
		return FALSE;

	if(RegQueryValueEx(pKey, "Path", NULL, NULL, NULL, &dwSize) == ERROR_SUCCESS)
	{
		pszValue = malloc(dwSize);
        if(RegQueryValueEx(pKey, "Path", NULL, NULL, (LPBYTE) pszValue, &dwSize) == ERROR_SUCCESS)
		{
			RegCloseKey(pKey);
			strcpy(pszPath, pszValue);
            free(pszValue);
			return TRUE;
		}else
			return FALSE;
	}
	RegCloseKey(pKey);
	return FALSE;
}

// roll, pitch, yaw
#define	NPRoll		1	// +/- 16383 (representing +/- 180) [data = input - 16383]
#define	NPPitch		2	// +/- 16383 (representing +/- 180) [data = input - 16383]
#define	NPYaw		4	// +/- 16383 (representing +/- 180) [data = input - 16383]

// x, y, z - remaining 6dof coordinates
#define	NPX			16	// +/- 16383 [data = input - 16383]
#define	NPY			32	// +/- 16383 [data = input - 16383]
#define	NPZ			64	// +/- 16383 [data = input - 16383]

// raw object position from imager
#define	NPRawX		128	// 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]
#define	NPRawY		256  // 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]
#define	NPRawZ		512  // 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]

void trackir_matrix_rotate(float *matrix, float angle, float x, float y, float z)
{
	float m[16], m2[16], f, a, c, d;
	uint i;
	f = sqrt(x * x + y * y + z * z);
	x /= f;
	y /= f;
	z /= f;
	angle *= PI * 2.0 / 360.0; 
	c = cos(angle);
	a = 1.0f - c;
	d = sin(angle);
	m[0] = x * x * a + c;
	m[1] = x * y * a + z * d;
	m[2] = x * z * a - y * d;
	m[3] = 0.0f;
	m[4] = y * x * a - z * d;
	m[5] = y * y * a + c;
	m[6] = y * z * a + x * d;
	m[7] = 0.0f;		
	m[8] = z * x * a + y * d;
	m[9] = z * y * a - x * d;
	m[10] = z * z * a + c;
	m[11] = 0.0f;
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
	f_matrix_multiplyf(m2, matrix, m);
	for(i = 0; i < 16; i++)
		matrix[i] = m2[i];
}


void trackir_view_direction_func(float *matrix)
{
	float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, m2[16], pos[3], scale;
	BTrackData data;
	uint i;
	for(i = 0; i < 16; i++)
		matrix[i] = m[i];
	data.wPFrameSignature = 0;
	gpfNP_GetData(&data);
	if(betray_settings_toggle_get(betray_settings_handle[0]))
	{
		trackir_matrix_rotate(matrix, data.fNPPitch * 180.0 / 16383.0, 1.0, 0, 0);
		trackir_matrix_rotate(matrix, data.fNPYaw * 180.0 / 16383.0, 0, -1.0, 0);
		trackir_matrix_rotate(matrix, data.fNPRoll * 180.0 / 16383.0, 0, 0, -1);
	}
	if(betray_settings_toggle_get(betray_settings_handle[1]))
	{
		scale = betray_settings_number_float_get(betray_settings_handle[2]);
		m[12] = scale * (float)((float)data.fNPX - 16383.0) / 16383.0;
		m[13] = scale * -(float)((float)data.fNPY - 16383.0) / 16383.0;
		m[14] = scale * -(float)((float)data.fNPZ - 16383.0) / 16383.0;
		f_matrix_multiplyf(m2, matrix, m);
		for(i = 0; i < 16; i++)
			matrix[i] = m2[i];
	}
}



void betray_plugin_init(void)
{
	unsigned short data_fields = 0;
	char path[B_DLL_MAX_PATH];
	HWND hWnd;
	hWnd = betray_plugin_windows_window_handle_get();

	if(!trackir_dll_locatate(path))
		return;

	if(!trackir_dll_init(path))
		return;

	data_fields |= NPPitch;
	data_fields |= NPYaw;
	data_fields |= NPRoll;
	data_fields |= NPX;
	data_fields |= NPY;
	data_fields |= NPZ;
	data_fields |= NPRawX;
	data_fields |= NPRawY;
	data_fields |= NPRawZ;
	gpfNP_RequestData(data_fields);
	gpfNP_RegisterProgramProfileID(3125);
	gpfNP_StopCursor();
	gpfNP_StartDataTransmission();
	betray_plugin_callback_set_view_direction(trackir_view_direction_func);
	betray_settings_handle[0] = betray_settings_create(BETRAY_ST_TOGGLE, "TrakIR Rotation", 0, NULL);
	betray_settings_handle[1] = betray_settings_create(BETRAY_ST_TOGGLE, "TrakIR Movement", 0, NULL);
	betray_settings_handle[2] = betray_settings_create(BETRAY_ST_NUMBER_FLOAT, "TrakIR Scale", 0, NULL);
	betray_settings_toggle_set(betray_settings_handle[0], TRUE);
	betray_settings_toggle_set(betray_settings_handle[1], TRUE);
	betray_settings_number_float_set(betray_settings_handle[2], 1.0);
}