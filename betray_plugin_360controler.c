
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "betray_plugin_api.h"


#define XINPUT_GAMEPAD_DPAD_UP	0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN	0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT	0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT	0x0008
#define XINPUT_GAMEPAD_START	0x0010
#define XINPUT_GAMEPAD_BACK	0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB	0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB	0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER	0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER	0x0200
#define XINPUT_GAMEPAD_A	0x1000
#define XINPUT_GAMEPAD_B	0x2000
#define XINPUT_GAMEPAD_X	0x4000
#define XINPUT_GAMEPAD_Y	0x8000

typedef struct{
  WORD  wButtons;
  BYTE  bLeftTrigger;
  BYTE  bRightTrigger;
  SHORT sThumbLX;
  SHORT sThumbLY;
  SHORT xThumbRX;
  SHORT xThumbRY;
}XINPUT_GAMEPAD;

typedef struct{
  DWORD          dwPacketNumber;
  XINPUT_GAMEPAD Gamepad;
}XINPUT_STATE;

DWORD (__stdcall *XInputGetState)(DWORD dwUserIndex, XINPUT_STATE *pState) = NULL;

uint controller_code_id[4][20];
uint controller_setting_id;
uint controller_user_ids[4];
uint controller_device_ids[4];
boolean controller_state[4][20];

void controller_plugin_callback_main(BInputState *input)
{
	char *players[4] = {"Player One", "Player Two", "Player Three", "Player Four"};
	boolean last_state[20];
	XINPUT_STATE pState;
	float deadspace, vec[4];
	uint i, j;
	deadspace = betray_settings_slider_get(controller_setting_id);
	deadspace *= deadspace;
	for(j = 0; j < 4; j++)
	{
		if(ERROR_SUCCESS == XInputGetState(j, &pState))
		{
			if(controller_device_ids[j] == -1)
				controller_device_ids[j] = betray_plugin_input_device_allocate(controller_user_ids[j], players[j]);
			if(controller_code_id[j][0] == -1)
			{
				controller_code_id[j][0] = betray_plugin_axis_allocate(controller_user_ids[j], controller_device_ids[j], "Left Stick", B_AXIS_STICK, 2);
				controller_code_id[j][1] = betray_plugin_axis_allocate(controller_user_ids[j], controller_device_ids[j], "Right Stick", B_AXIS_STICK, 2);
				controller_code_id[j][2] = betray_plugin_axis_allocate(controller_user_ids[j], controller_device_ids[j], "Left Bumper", B_AXIS_BUMPER_LEFT, 1);
				controller_code_id[j][3] = betray_plugin_axis_allocate(controller_user_ids[j], controller_device_ids[j], "Right Bunper", B_AXIS_BUMPER_RIGHT, 1);

				controller_code_id[j][4] = betray_plugin_button_allocate(BETRAY_BUTTON_PLAY_PAUSE, "");
				controller_code_id[j][5] = betray_plugin_button_allocate(BETRAY_BUTTON_SHOLDER_LEFT_A, "");
				controller_code_id[j][6] = betray_plugin_button_allocate(BETRAY_BUTTON_SHOLDER_LEFT_B, "");
				controller_code_id[j][7] = betray_plugin_button_allocate(BETRAY_BUTTON_SHOLDER_RIGHT_A, "");
				controller_code_id[j][8] = betray_plugin_button_allocate(BETRAY_BUTTON_SHOLDER_RIGHT_B, "");

				controller_code_id[j][9] = betray_plugin_button_allocate(BETRAY_BUTTON_FACE_A, "");
				controller_code_id[j][10] = betray_plugin_button_allocate(BETRAY_BUTTON_FACE_B, "");
				controller_code_id[j][11] = betray_plugin_button_allocate(BETRAY_BUTTON_FACE_X, "");
				controller_code_id[j][12] = betray_plugin_button_allocate(BETRAY_BUTTON_FACE_Y, "");

				controller_code_id[j][13] = betray_plugin_button_allocate(BETRAY_BUTTON_LEFT, "");
				controller_code_id[j][14] = betray_plugin_button_allocate(BETRAY_BUTTON_UP, "");
				controller_code_id[j][15] = betray_plugin_button_allocate(BETRAY_BUTTON_RIGHT, "");
				controller_code_id[j][16] = betray_plugin_button_allocate(BETRAY_BUTTON_DOWN, "");

				controller_code_id[j][17] = betray_plugin_button_allocate(BETRAY_ENUM_BUTTON_BACK, "");
				controller_code_id[j][18] = betray_plugin_button_allocate(-1, "Left Stick Click");
				controller_code_id[j][19] = betray_plugin_button_allocate(-1, "Right Stick Click");
			}

			vec[0] = (float)pState.Gamepad.sThumbLX / 32768.0;
			vec[1] = (float)pState.Gamepad.sThumbLY / 32768.0;
			vec[2] = (float)pState.Gamepad.xThumbRX / 32768.0;
			vec[3] = (float)pState.Gamepad.xThumbRY / 32768.0;
			for(i = 0; i < 4; i++)
			{
				if(vec[i] > 0.0)
				{
					vec[i] = vec[i] - deadspace;
					if(vec[i] < 0.0)
						vec[i] = 0.0;
				}else
				{
					vec[i] = vec[i] + deadspace;
					if(vec[i] > 0.0)
						vec[i] = 0.0;
				}
				vec[i] /= (1.0 - deadspace);
			}
			betray_plugin_axis_set(controller_code_id[j][0], vec[0], vec[1], 0);
			betray_plugin_axis_set(controller_code_id[j][1], vec[2], vec[3], 0);
			betray_plugin_axis_set(controller_code_id[j][2], (float)((uint)pState.Gamepad.bLeftTrigger) / 255.0, 0, 0);
			betray_plugin_axis_set(controller_code_id[j][3], (float)((uint)pState.Gamepad.bRightTrigger) / 255.0, 0, 0);

			for(i = 0; i < 20; i++)
				last_state[i] = controller_state[j][i];

			controller_state[j][4] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
			controller_state[j][5] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
			controller_state[j][6] = (uint)pState.Gamepad.bLeftTrigger > 5;
			controller_state[j][7] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
			controller_state[j][8] = (uint)pState.Gamepad.bRightTrigger > 5;
			controller_state[j][9] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
			controller_state[j][10] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
			controller_state[j][11] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
			controller_state[j][12] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
			controller_state[j][13] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
			controller_state[j][14] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
			controller_state[j][15] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
			controller_state[j][16] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
			controller_state[j][17] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
			controller_state[j][18] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
			controller_state[j][19] = (pState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;

			for(i = 4; i < 20; i++)
			{
				if(controller_state[j][i] != last_state[i])
				{
					betray_plugin_button_set(controller_user_ids[j], controller_code_id[j][i], controller_state[j][i], -1);
				}
			}
		}else if(controller_device_ids[j] != -1)
		{
			betray_plugin_input_device_free(controller_device_ids[j]);
			controller_device_ids[j] = -1;
		}
	}
}

void betray_plugin_init(void)
{
	void *xinput_dll = NULL;
	uint i, j, *a = NULL;
#ifdef UNICODE
	short u_text[32];
	char *text = "XInput9_1_0.dll";
	for (i = 0; i < 32 && text[i] != 0; i++)
		u_text[i] = (char)text[i];
	u_text[i] = 0;

	xinput_dll = LoadLibrary(u_text);
#else
	xinput_dll = LoadLibrary("XInput9_1_0.dll");
#endif
	if(xinput_dll == NULL)
		return;
	XInputGetState = (DWORD (__stdcall *)(DWORD , XINPUT_STATE *))GetProcAddress(xinput_dll, "XInputGetState");
	if(XInputGetState == NULL)
		return;
	betray_plugin_callback_set_main(controller_plugin_callback_main);

	for(i = 0; i < 4; i++)
	{
		controller_code_id[i][0] = -1;
		controller_user_ids[i] = betray_plugin_user_allocate();
		controller_device_ids[i] = -1;
		for(j = 0; j < 20; j++)
			controller_state[i][j] = FALSE;
	}

	controller_setting_id = betray_settings_create(BETRAY_ST_SLIDER, "Controller deadspace", 0, NULL);
	betray_settings_slider_set(controller_setting_id, 0.4);

}