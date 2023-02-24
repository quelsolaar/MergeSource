#include <stdlib.h>
//#include "Carbon/Carbon.h"
#include "betray.h"

char *betray_button_unused = "Unnamed";

BButton *betray_buttons = NULL;

void betray_key_codes_init(void)
{
	uint i;
	betray_buttons = (BButton *)malloc((sizeof *betray_buttons) * BETRAY_ENUM_BUTTON_COUNT);
	for(i = 0; i < BETRAY_ENUM_BUTTON_COUNT; i++)
	{
		betray_buttons[i].name = betray_button_unused;
		betray_buttons[i].system_code = 256 * 256 + i;
	}

#if 0
kVK_Return                    = 0x24,
  kVK_Tab                       = 0x30,
  kVK_Space                     = 0x31,
  kVK_Delete                    = 0x33,
  kVK_Escape                    = 0x35,
  kVK_Command                   = 0x37,
  kVK_Shift                     = 0x38,
  kVK_CapsLock                  = 0x39,
  kVK_Option                    = 0x3A,
  kVK_Control                   = 0x3B,
  kVK_RightCommand              = 0x36,
  kVK_RightShift                = 0x3C,
  kVK_RightOption               = 0x3D,
  kVK_RightControl              = 0x3E,
  kVK_Function                  = 0x3F,
  kVK_F17                       = 0x40,
  kVK_VolumeUp                  = 0x48,
  kVK_VolumeDown                = 0x49,
  kVK_Mute                      = 0x4A,
  kVK_F18                       = 0x4F,
  kVK_F19                       = 0x50,
  kVK_F20                       = 0x5A,
  kVK_F5                        = 0x60,
  kVK_F6                        = 0x61,
  kVK_F7                        = 0x62,
  kVK_F3                        = 0x63,
  kVK_F8                        = 0x64,
  kVK_F9                        = 0x65,
  kVK_F11                       = 0x67,
  kVK_F13                       = 0x69,
  kVK_F16                       = 0x6A,
  kVK_F14                       = 0x6B,
  kVK_F10                       = 0x6D,
  kVK_F12                       = 0x6F,
  kVK_F15                       = 0x71,
  kVK_Help                      = 0x72,
  kVK_Home                      = 0x73,
  kVK_PageUp                    = 0x74,
  kVK_ForwardDelete             = 0x75,
  kVK_F4                        = 0x76,
  kVK_End                       = 0x77,
  kVK_F2                        = 0x78,
  kVK_PageDown                  = 0x79,
  kVK_F1                        = 0x7A,
  kVK_LeftArrow                 = 0x7B,
  kVK_RightArrow                = 0x7C,
  kVK_DownArrow                 = 0x7D,
  kVK_UpArrow                   = 0x7E


//	betray_buttons[BETRAY_ENUM_BUTTON_CANCEL].system_code = AKEYCODE_CANCEL;
//	betray_buttons[BETRAY_ENUM_BUTTON_BACK].system_code = ;
	betray_buttons[BETRAY_ENUM_BUTTON_TAB].system_code = kVK_Tab;
//	betray_buttons[BETRAY_ENUM_BUTTON_CLEAR].system_code = kVK_Clear;
	betray_buttons[BETRAY_ENUM_BUTTON_RETURN].system_code = kVK_Return;
	betray_buttons[BETRAY_ENUM_BUTTON_SHIFT].system_code = kVK_Shift;
	betray_buttons[BETRAY_ENUM_BUTTON_CONTROL].system_code = kVK_Control;
	betray_buttons[BETRAY_ENUM_BUTTON_MENU].system_code = kVK_Function;
//	betray_buttons[BETRAY_ENUM_BUTTON_PAUSE].system_code = kVK_Pause;
	betray_buttons[BETRAY_ENUM_BUTTON_CAPS_LOCK].system_code = kVK_CapsLock;
	betray_buttons[BETRAY_ENUM_BUTTON_ESCAPE].system_code = kVK_Escape;
	betray_buttons[BETRAY_ENUM_BUTTON_SPACE].system_code = kVK_Space;
//	betray_buttons[BETRAY_ENUM_BUTTON_PREV].system_code = AKEYCODE_PRIOR;
//	betray_buttons[BETRAY_ENUM_BUTTON_NEXT].system_code = AKEYCODE_NEXT;
	betray_buttons[BETRAY_ENUM_BUTTON_END].system_code = kVK_End;
	betray_buttons[BETRAY_ENUM_BUTTON_HOME].system_code = kVK_Home;
	betray_buttons[BETRAY_ENUM_BUTTON_LEFT].system_code = kVK_LeftArrow;
	betray_buttons[BETRAY_ENUM_BUTTON_UP].system_code = kVK_UpArrow;
	betray_buttons[BETRAY_ENUM_BUTTON_RIGHT].system_code = kVK_RightArrow;
	betray_buttons[BETRAY_ENUM_BUTTON_DOWN].system_code = kVK_DownArrow;
//	betray_buttons[BETRAY_ENUM_BUTTON_SELECT].system_code = AKEYCODE_SELECT;
//	betray_buttons[BETRAY_ENUM_BUTTON_PRINT].system_code = AKEYCODE_PRINT;
//	betray_buttons[BETRAY_ENUM_BUTTON_EXECUTE].system_code = AKEYCODE_EXECUTE;
//	betray_buttons[BETRAY_ENUM_BUTTON_SCREENSHOT].system_code = AKEYCODE_SNAPSHOT;
//	betray_buttons[BETRAY_ENUM_BUTTON_INSERT].system_code = kVK_Insert;
	betray_buttons[BETRAY_ENUM_BUTTON_DELETE].system_code = kVK_Delete;
//	betray_buttons[BETRAY_ENUM_BUTTON_BACKSPACE].system_code = kVK_Backspace;
//	betray_buttons[BETRAY_ENUM_BUTTON_HELP].system_code = AKEYCODE_HELP;
	betray_buttons[BETRAY_ENUM_BUTTON_0].system_code = kVK_ANSI_0;
	betray_buttons[BETRAY_ENUM_BUTTON_1].system_code = kVK_ANSI_1;
	betray_buttons[BETRAY_ENUM_BUTTON_2].system_code = kVK_ANSI_2;
	betray_buttons[BETRAY_ENUM_BUTTON_3].system_code = kVK_ANSI_3;
	betray_buttons[BETRAY_ENUM_BUTTON_4].system_code = kVK_ANSI_4;
	betray_buttons[BETRAY_ENUM_BUTTON_5].system_code = kVK_ANSI_5;
	betray_buttons[BETRAY_ENUM_BUTTON_6].system_code = kVK_ANSI_6;
	betray_buttons[BETRAY_ENUM_BUTTON_7].system_code = kVK_ANSI_7;
	betray_buttons[BETRAY_ENUM_BUTTON_8].system_code = kVK_ANSI_8;
	betray_buttons[BETRAY_ENUM_BUTTON_9].system_code = kVK_ANSI_9;
	betray_buttons[BETRAY_ENUM_BUTTON_A].system_code = kVK_ANSI_A;
	betray_buttons[BETRAY_ENUM_BUTTON_B].system_code = kVK_ANSI_B;
	betray_buttons[BETRAY_ENUM_BUTTON_C].system_code = kVK_ANSI_C;
	betray_buttons[BETRAY_ENUM_BUTTON_D].system_code = kVK_ANSI_D;
	betray_buttons[BETRAY_ENUM_BUTTON_E].system_code = kVK_ANSI_E;
	betray_buttons[BETRAY_ENUM_BUTTON_F].system_code = kVK_ANSI_F;
	betray_buttons[BETRAY_ENUM_BUTTON_G].system_code = kVK_ANSI_G;
	betray_buttons[BETRAY_ENUM_BUTTON_H].system_code = kVK_ANSI_H;
	betray_buttons[BETRAY_ENUM_BUTTON_I].system_code = kVK_ANSI_I;
	betray_buttons[BETRAY_ENUM_BUTTON_J].system_code = kVK_ANSI_J;
	betray_buttons[BETRAY_ENUM_BUTTON_K].system_code = kVK_ANSI_K;
	betray_buttons[BETRAY_ENUM_BUTTON_L].system_code = kVK_ANSI_L;
	betray_buttons[BETRAY_ENUM_BUTTON_M].system_code = kVK_ANSI_M;
	betray_buttons[BETRAY_ENUM_BUTTON_N].system_code = kVK_ANSI_N;
	betray_buttons[BETRAY_ENUM_BUTTON_O].system_code = kVK_ANSI_O;
	betray_buttons[BETRAY_ENUM_BUTTON_P].system_code = kVK_ANSI_P;
	betray_buttons[BETRAY_ENUM_BUTTON_Q].system_code = kVK_ANSI_Q;
	betray_buttons[BETRAY_ENUM_BUTTON_R].system_code = kVK_ANSI_R;
	betray_buttons[BETRAY_ENUM_BUTTON_S].system_code = kVK_ANSI_S;
	betray_buttons[BETRAY_ENUM_BUTTON_T].system_code = kVK_ANSI_T;
	betray_buttons[BETRAY_ENUM_BUTTON_U].system_code = kVK_ANSI_U;
	betray_buttons[BETRAY_ENUM_BUTTON_V].system_code = kVK_ANSI_V;
	betray_buttons[BETRAY_ENUM_BUTTON_W].system_code = kVK_ANSI_W;
	betray_buttons[BETRAY_ENUM_BUTTON_X].system_code = kVK_ANSI_X;
	betray_buttons[BETRAY_ENUM_BUTTON_Y].system_code = kVK_ANSI_Y;
	betray_buttons[BETRAY_ENUM_BUTTON_Z].system_code = kVK_ANSI_Z;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD0].system_code = kVK_ANSI_Keypad0;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD1].system_code = kVK_ANSI_Keypad1;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD2].system_code = kVK_ANSI_Keypad2;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD3].system_code = kVK_ANSI_Keypad3;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD4].system_code = kVK_ANSI_Keypad4;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD5].system_code = kVK_ANSI_Keypad5;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD6].system_code = kVK_ANSI_Keypad6;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD7].system_code = kVK_ANSI_Keypad7;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD8].system_code = kVK_ANSI_Keypad8;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD9].system_code = kVK_ANSI_Keypad9;
	betray_buttons[BETRAY_ENUM_BUTTON_MULTIPLY].system_code = kVK_ANSI_KeypadMultiply;
	betray_buttons[BETRAY_ENUM_BUTTON_ADD].system_code = kVK_ANSI_KeypadPlus;
	betray_buttons[BETRAY_ENUM_BUTTON_SUBTRACT].system_code = kVK_ANSI_KeypadMinus;
	betray_buttons[BETRAY_ENUM_BUTTON_DIVIDE].system_code = kVK_ANSI_KeypadDivide;
	betray_buttons[BETRAY_ENUM_BUTTON_F1].system_code = kVK_F1;
	betray_buttons[BETRAY_ENUM_BUTTON_F2].system_code = kVK_F2; 
	betray_buttons[BETRAY_ENUM_BUTTON_F3].system_code = kVK_F3;
	betray_buttons[BETRAY_ENUM_BUTTON_F4].system_code = kVK_F4;
	betray_buttons[BETRAY_ENUM_BUTTON_F5].system_code = kVK_F5;
	betray_buttons[BETRAY_ENUM_BUTTON_F6].system_code = kVK_F6;
	betray_buttons[BETRAY_ENUM_BUTTON_F7].system_code = kVK_F7;
	betray_buttons[BETRAY_ENUM_BUTTON_F8].system_code = kVK_F8;
	betray_buttons[BETRAY_ENUM_BUTTON_F9].system_code = kVK_F9;
	betray_buttons[BETRAY_ENUM_BUTTON_F10].system_code = kVK_F10;
	betray_buttons[BETRAY_ENUM_BUTTON_F11].system_code = kVK_F11;
	betray_buttons[BETRAY_ENUM_BUTTON_F12].system_code = kVK_F12;
/*	betray_buttons[BETRAY_ENUM_BUTTON_F13].system_code = AKEYCODE_F13;
	betray_buttons[BETRAY_ENUM_BUTTON_F14].system_code = AKEYCODE_F14;
	betray_buttons[BETRAY_ENUM_BUTTON_F15].system_code = AKEYCODE_F15;
	betray_buttons[BETRAY_ENUM_BUTTON_F16].system_code = AKEYCODE_F16;
	betray_buttons[BETRAY_ENUM_BUTTON_F17].system_code = AKEYCODE_F17;
	betray_buttons[BETRAY_ENUM_BUTTON_F18].system_code = AKEYCODE_F18;
	betray_buttons[BETRAY_ENUM_BUTTON_F19].system_code = AKEYCODE_F19;
	betray_buttons[BETRAY_ENUM_BUTTON_F20].system_code = AKEYCODE_F20;
	betray_buttons[BETRAY_ENUM_BUTTON_F21].system_code = AKEYCODE_F21;
	betray_buttons[BETRAY_ENUM_BUTTON_F22].system_code = AKEYCODE_F22;
	betray_buttons[BETRAY_ENUM_BUTTON_F23].system_code = AKEYCODE_F23;
	betray_buttons[BETRAY_ENUM_BUTTON_F24].system_code = AKEYCODE_F24;*/
	betray_buttons[BETRAY_ENUM_BUTTON_VOLUME_DOWN].system_code = kVK_VolumeDown;
	betray_buttons[BETRAY_ENUM_BUTTON_VOLUME_UP].system_code = kVK_VolumeUp;

    #if 0
	betray_buttons[BETRAY_ENUM_BUTTON_NEXT_TRACK].system_code = ;
	betray_buttons[BETRAY_ENUM_BUTTON_PREV_TRACK].system_code = AKEYCODE_MEDIA_PREVIOUS;
	betray_buttons[BETRAY_ENUM_BUTTON_STOP].system_code = kVK_Stop;
	betray_buttons[BETRAY_ENUM_BUTTON_PLAY_PAUSE].system_code = kVK_PlayPause;
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_A].system_code = AKEYCODE_BUTTON_A;
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_B].system_code = AKEYCODE_BUTTON_B;
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_C].system_code = AKEYCODE_BUTTON_C;
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_X].system_code = AKEYCODE_BUTTON_X;
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_Y].system_code = AKEYCODE_BUTTON_Y;
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_Z].system_code = AKEYCODE_BUTTON_Z;
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_LEFT_A].system_code = AKEYCODE_BUTTON_L1;
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_LEFT_B].system_code = AKEYCODE_BUTTON_L2;
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_RIGHT_A].system_code = AKEYCODE_BUTTON_R1;
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_RIGHT_B].system_code = AKEYCODE_BUTTON_R2;
    #endif
	betray_buttons[BETRAY_ENUM_BUTTON_CANCEL].name = "Cancel";
	betray_buttons[BETRAY_ENUM_BUTTON_BACK].name = "Back";
	betray_buttons[BETRAY_ENUM_BUTTON_TAB].name = "Tab";
	betray_buttons[BETRAY_ENUM_BUTTON_CLEAR].name = "Clear";
	betray_buttons[BETRAY_ENUM_BUTTON_RETURN].name = "Return";
	betray_buttons[BETRAY_ENUM_BUTTON_SHIFT].name = "Shift";
	betray_buttons[BETRAY_ENUM_BUTTON_CONTROL].name = "Control";
	betray_buttons[BETRAY_ENUM_BUTTON_MENU].name = "Menu";
	betray_buttons[BETRAY_ENUM_BUTTON_PAUSE].name = "Pause";
	betray_buttons[BETRAY_ENUM_BUTTON_CAPS_LOCK].name = "Capital";
	betray_buttons[BETRAY_ENUM_BUTTON_ESCAPE].name = "Escape";
	betray_buttons[BETRAY_ENUM_BUTTON_SPACE].name = "Space";
	betray_buttons[BETRAY_ENUM_BUTTON_PREV].name = "Previous";
	betray_buttons[BETRAY_ENUM_BUTTON_NEXT].name = "Next";
	betray_buttons[BETRAY_ENUM_BUTTON_END].name = "End";
	betray_buttons[BETRAY_ENUM_BUTTON_HOME].name = "Home";
	betray_buttons[BETRAY_ENUM_BUTTON_LEFT].name = "Left";
	betray_buttons[BETRAY_ENUM_BUTTON_UP].name = "Up";
	betray_buttons[BETRAY_ENUM_BUTTON_RIGHT].name = "Right";
	betray_buttons[BETRAY_ENUM_BUTTON_DOWN].name = "Down";
	betray_buttons[BETRAY_ENUM_BUTTON_SELECT].name = "Select";
	betray_buttons[BETRAY_ENUM_BUTTON_PRINT].name = "Print";
	betray_buttons[BETRAY_ENUM_BUTTON_EXECUTE].name = "Execute";
	betray_buttons[BETRAY_ENUM_BUTTON_SCREENSHOT].name = "Screenshot";
	betray_buttons[BETRAY_ENUM_BUTTON_INSERT].name = "Insert";
	betray_buttons[BETRAY_ENUM_BUTTON_DELETE].name = "Delete";
	betray_buttons[BETRAY_ENUM_BUTTON_HELP].name = "Help";
	betray_buttons[BETRAY_ENUM_BUTTON_0].name = "0";
	betray_buttons[BETRAY_ENUM_BUTTON_1].name = "1";
	betray_buttons[BETRAY_ENUM_BUTTON_2].name = "2";
	betray_buttons[BETRAY_ENUM_BUTTON_3].name = "3";
	betray_buttons[BETRAY_ENUM_BUTTON_4].name = "4";
	betray_buttons[BETRAY_ENUM_BUTTON_5].name = "5";
	betray_buttons[BETRAY_ENUM_BUTTON_6].name = "6";
	betray_buttons[BETRAY_ENUM_BUTTON_7].name = "7";
	betray_buttons[BETRAY_ENUM_BUTTON_8].name = "8";
	betray_buttons[BETRAY_ENUM_BUTTON_9].name = "9";
	betray_buttons[BETRAY_ENUM_BUTTON_A].name = "A";
	betray_buttons[BETRAY_ENUM_BUTTON_B].name = "B";
	betray_buttons[BETRAY_ENUM_BUTTON_C].name = "C";
	betray_buttons[BETRAY_ENUM_BUTTON_D].name = "D";
	betray_buttons[BETRAY_ENUM_BUTTON_E].name = "E";
	betray_buttons[BETRAY_ENUM_BUTTON_F].name = "F";
	betray_buttons[BETRAY_ENUM_BUTTON_G].name = "G";
	betray_buttons[BETRAY_ENUM_BUTTON_H].name = "H";
	betray_buttons[BETRAY_ENUM_BUTTON_I].name = "I";
	betray_buttons[BETRAY_ENUM_BUTTON_J].name = "J";
	betray_buttons[BETRAY_ENUM_BUTTON_K].name = "K";
	betray_buttons[BETRAY_ENUM_BUTTON_L].name = "L";
	betray_buttons[BETRAY_ENUM_BUTTON_K].name = "M";
	betray_buttons[BETRAY_ENUM_BUTTON_N].name = "N";
	betray_buttons[BETRAY_ENUM_BUTTON_O].name = "O";
	betray_buttons[BETRAY_ENUM_BUTTON_P].name = "P";
	betray_buttons[BETRAY_ENUM_BUTTON_Q].name = "Q";
	betray_buttons[BETRAY_ENUM_BUTTON_R].name = "R";
	betray_buttons[BETRAY_ENUM_BUTTON_S].name = "S";
	betray_buttons[BETRAY_ENUM_BUTTON_T].name = "T";
	betray_buttons[BETRAY_ENUM_BUTTON_U].name = "U";
	betray_buttons[BETRAY_ENUM_BUTTON_V].name = "V";
	betray_buttons[BETRAY_ENUM_BUTTON_W].name = "W";
	betray_buttons[BETRAY_ENUM_BUTTON_X].name = "X";
	betray_buttons[BETRAY_ENUM_BUTTON_Y].name = "Y";
	betray_buttons[BETRAY_ENUM_BUTTON_Z].name = "Z";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD0].name = "Num Pad 0";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD1].name = "Num Pad 1";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD2].name = "Num Pad 2";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD3].name = "Num Pad 3";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD4].name = "Num Pad 4";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD5].name = "Num Pad 5";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD6].name = "Num Pad 6";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD7].name = "Num Pad 7";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD8].name = "Num Pad 8";
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD9].name = "Num Pad 9";
	betray_buttons[BETRAY_ENUM_BUTTON_MULTIPLY].name = "Multiply";
	betray_buttons[BETRAY_ENUM_BUTTON_ADD].name = "Add";
	betray_buttons[BETRAY_ENUM_BUTTON_SUBTRACT].name = "Subtract";
	betray_buttons[BETRAY_ENUM_BUTTON_DIVIDE].name = "Divide";
	betray_buttons[BETRAY_ENUM_BUTTON_F1].name = "F1";
	betray_buttons[BETRAY_ENUM_BUTTON_F2].name = "F2";
	betray_buttons[BETRAY_ENUM_BUTTON_F3].name = "F3";
	betray_buttons[BETRAY_ENUM_BUTTON_F4].name = "F4";
	betray_buttons[BETRAY_ENUM_BUTTON_F5].name = "F5";
	betray_buttons[BETRAY_ENUM_BUTTON_F6].name = "F6";
	betray_buttons[BETRAY_ENUM_BUTTON_F7].name = "F7";
	betray_buttons[BETRAY_ENUM_BUTTON_F8].name = "F8";
	betray_buttons[BETRAY_ENUM_BUTTON_F9].name = "F9";
	betray_buttons[BETRAY_ENUM_BUTTON_F10].name = "F10";
	betray_buttons[BETRAY_ENUM_BUTTON_F11].name = "F11";
	betray_buttons[BETRAY_ENUM_BUTTON_F12].name = "F12";
	betray_buttons[BETRAY_ENUM_BUTTON_F13].name = "F13";
	betray_buttons[BETRAY_ENUM_BUTTON_F14].name = "F14";
	betray_buttons[BETRAY_ENUM_BUTTON_F15].name = "F15";
	betray_buttons[BETRAY_ENUM_BUTTON_F16].name = "F16";
	betray_buttons[BETRAY_ENUM_BUTTON_F17].name = "F17";
	betray_buttons[BETRAY_ENUM_BUTTON_F18].name = "F18";
	betray_buttons[BETRAY_ENUM_BUTTON_F19].name = "F19";
	betray_buttons[BETRAY_ENUM_BUTTON_F20].name = "F20";
	betray_buttons[BETRAY_ENUM_BUTTON_F21].name = "F21";
	betray_buttons[BETRAY_ENUM_BUTTON_F22].name = "F22";
	betray_buttons[BETRAY_ENUM_BUTTON_F23].name = "F23";
	betray_buttons[BETRAY_ENUM_BUTTON_F24].name = "F24";
	betray_buttons[BETRAY_ENUM_BUTTON_VOLUME_DOWN].name = "Volume down";
	betray_buttons[BETRAY_ENUM_BUTTON_VOLUME_UP].name = "Volume up";
	betray_buttons[BETRAY_ENUM_BUTTON_NEXT_TRACK].name = "Next track";
	betray_buttons[BETRAY_ENUM_BUTTON_PREV_TRACK].name = "Previous track";
	betray_buttons[BETRAY_ENUM_BUTTON_STOP].name = "Stop";
	betray_buttons[BETRAY_ENUM_BUTTON_PLAY_PAUSE].name = "Play/Pause";
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_A].name = "Face A";
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_B].name = "Face B";
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_C].name = "Face C";
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_D].name = "Face D";
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_X].name = "Face X";
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_Y].name = "Face Y";
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_Z].name = "Face Z";
	betray_buttons[BETRAY_ENUM_BUTTON_FACE_W].name = "Face W";
	betray_buttons[BETRAY_ENUM_BUTTON_YES].name = "Yes";
	betray_buttons[BETRAY_ENUM_BUTTON_NO].name = "No";
	betray_buttons[BETRAY_ENUM_BUTTON_UNDO].name = "Undo";
	betray_buttons[BETRAY_ENUM_BUTTON_REDO].name = "Redo";
	betray_buttons[BETRAY_ENUM_BUTTON_CUT].name = "Cut";
	betray_buttons[BETRAY_ENUM_BUTTON_COPY].name = "Copy";
	betray_buttons[BETRAY_ENUM_BUTTON_PASTE].name = "Paste";
	betray_buttons[BETRAY_ENUM_BUTTON_SEARCH].name = "Search";
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_LEFT_A].name = "Sholder Left 1";
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_LEFT_B].name = "Sholder Left 2";
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_LEFT_C].name = "Sholder Left 3";
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_LEFT_D].name = "Sholder Left 4";
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_RIGHT_A].name = "Sholder Right 1";
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_RIGHT_B].name = "Sholder Right 2";
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_RIGHT_C].name = "Sholder Right 3";
	betray_buttons[BETRAY_ENUM_BUTTON_SHOLDER_RIGHT_D].name = "Sholder Right 4";
	betray_buttons[BETRAY_ENUM_BUTTON_SCROLL_UP].name = "Scroll up";
	betray_buttons[BETRAY_ENUM_BUTTON_SCROLL_DOWN].name = "Scroll down";
	betray_buttons[BETRAY_ENUM_BUTTON_SCROLL_LEFT].name = "Scroll left";
	betray_buttons[BETRAY_ENUM_BUTTON_SCROLL_RIGHT].name = "Scroll right";
	betray_buttons[BETRAY_ENUM_BUTTON_INVENTORY_NEXT].name = "Inventory next";
	betray_buttons[BETRAY_ENUM_BUTTON_INVENTORY_PREVIOUS].name = "Inventory Previus";
#endif
}

boolean	betray_button_get_name(uint user_id, uint key, char *name, uint buffer_size)
{
	uint i, j;
	for(i = 0; i < BETRAY_ENUM_BUTTON_COUNT; i++)
		if(betray_buttons[i].system_code == key)
			break;

	if(i == BETRAY_ENUM_BUTTON_COUNT)
		return FALSE;

	buffer_size--;
	for(j = 0; j < buffer_size && betray_buttons[i].name[j] != 0; j++)
		name[j] = betray_buttons[i].name[j];
	name[j] = 0;
	return TRUE;
}
