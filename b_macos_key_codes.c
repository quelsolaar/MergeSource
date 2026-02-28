#ifdef __APPLE_CC__

#include <stdlib.h>
#include "betray.h"

char *betray_button_unused = "Unnamed";

BButton *betray_buttons = NULL;

void betray_key_codes_init(void)
{
	uint i;
	betray_buttons = malloc((sizeof *betray_buttons) * BETRAY_ENUM_BUTTON_COUNT);
	for(i = 0; i < BETRAY_ENUM_BUTTON_COUNT; i++)
	{
		betray_buttons[i].name = betray_button_unused;
		betray_buttons[i].system_code = 256 * 256 + i;
	}
//	betray_buttons[BETRAY_ENUM_BUTTON_CANCEL].system_code = VK_CANCEL;
//	betray_buttons[BETRAY_ENUM_BUTTON_BACK].system_code = VK_BACK;
	betray_buttons[BETRAY_ENUM_BUTTON_TAB].system_code = 48;
//	betray_buttons[BETRAY_ENUM_BUTTON_CLEAR].system_code = VK_CLEAR;
	betray_buttons[BETRAY_ENUM_BUTTON_RETURN].system_code = 36;
	betray_buttons[BETRAY_ENUM_BUTTON_SHIFT].system_code = 56;
	betray_buttons[BETRAY_ENUM_BUTTON_CONTROL].system_code = 59;
	betray_buttons[BETRAY_ENUM_BUTTON_MENU].system_code = 58;
//	betray_buttons[BETRAY_ENUM_BUTTON_PAUSE].system_code = VK_PAUSE;
	betray_buttons[BETRAY_ENUM_BUTTON_CAPS_LOCK].system_code = 57;
	betray_buttons[BETRAY_ENUM_BUTTON_ESCAPE].system_code = 53;
	betray_buttons[BETRAY_ENUM_BUTTON_SPACE].system_code = 49;
//	betray_buttons[BETRAY_ENUM_BUTTON_PREV].system_code = VK_PRIOR;
//	betray_buttons[BETRAY_ENUM_BUTTON_NEXT].system_code = VK_NEXT;
	betray_buttons[BETRAY_ENUM_BUTTON_END].system_code = 119;
	betray_buttons[BETRAY_ENUM_BUTTON_HOME].system_code = 115;
	betray_buttons[BETRAY_ENUM_BUTTON_LEFT].system_code = 123;
	betray_buttons[BETRAY_ENUM_BUTTON_UP].system_code = 126;
	betray_buttons[BETRAY_ENUM_BUTTON_RIGHT].system_code = 124;
	betray_buttons[BETRAY_ENUM_BUTTON_DOWN].system_code = 125;
//	betray_buttons[BETRAY_ENUM_BUTTON_SELECT].system_code = VK_SELECT;
//	betray_buttons[BETRAY_ENUM_BUTTON_PRINT].system_code = VK_PRINT;
//	betray_buttons[BETRAY_ENUM_BUTTON_EXECUTE].system_code = VK_EXECUTE;
	betray_buttons[BETRAY_ENUM_BUTTON_SCREENSHOT].system_code = 105;
	betray_buttons[BETRAY_ENUM_BUTTON_INSERT].system_code = 114;
	betray_buttons[BETRAY_ENUM_BUTTON_DELETE].system_code = 117;
	betray_buttons[BETRAY_ENUM_BUTTON_BACKSPACE].system_code = 51;
	betray_buttons[BETRAY_ENUM_BUTTON_HELP].system_code = 114;
	betray_buttons[BETRAY_ENUM_BUTTON_0].system_code = 29;
	betray_buttons[BETRAY_ENUM_BUTTON_1].system_code = 18;
	betray_buttons[BETRAY_ENUM_BUTTON_2].system_code = 19;
	betray_buttons[BETRAY_ENUM_BUTTON_3].system_code = 20;
	betray_buttons[BETRAY_ENUM_BUTTON_4].system_code = 21;
	betray_buttons[BETRAY_ENUM_BUTTON_5].system_code = 23;
	betray_buttons[BETRAY_ENUM_BUTTON_6].system_code = 22;
	betray_buttons[BETRAY_ENUM_BUTTON_7].system_code = 26;
	betray_buttons[BETRAY_ENUM_BUTTON_8].system_code = 28;
	betray_buttons[BETRAY_ENUM_BUTTON_9].system_code = 25;
	betray_buttons[BETRAY_ENUM_BUTTON_A].system_code = 0;
	betray_buttons[BETRAY_ENUM_BUTTON_B].system_code = 11;
	betray_buttons[BETRAY_ENUM_BUTTON_C].system_code = 8;
	betray_buttons[BETRAY_ENUM_BUTTON_D].system_code = 2;
	betray_buttons[BETRAY_ENUM_BUTTON_E].system_code = 14;
	betray_buttons[BETRAY_ENUM_BUTTON_F].system_code = 3;
	betray_buttons[BETRAY_ENUM_BUTTON_G].system_code = 5;
	betray_buttons[BETRAY_ENUM_BUTTON_H].system_code = 4;
	betray_buttons[BETRAY_ENUM_BUTTON_I].system_code = 34;
	betray_buttons[BETRAY_ENUM_BUTTON_J].system_code = 38;
	betray_buttons[BETRAY_ENUM_BUTTON_K].system_code = 40;
	betray_buttons[BETRAY_ENUM_BUTTON_L].system_code = 37;
	betray_buttons[BETRAY_ENUM_BUTTON_M].system_code = 46;
	betray_buttons[BETRAY_ENUM_BUTTON_N].system_code = 45;
	betray_buttons[BETRAY_ENUM_BUTTON_O].system_code = 31;
	betray_buttons[BETRAY_ENUM_BUTTON_P].system_code = 35;
	betray_buttons[BETRAY_ENUM_BUTTON_Q].system_code = 12;
	betray_buttons[BETRAY_ENUM_BUTTON_R].system_code = 15;
	betray_buttons[BETRAY_ENUM_BUTTON_S].system_code = 1;
	betray_buttons[BETRAY_ENUM_BUTTON_T].system_code = 17;
	betray_buttons[BETRAY_ENUM_BUTTON_U].system_code = 32;
	betray_buttons[BETRAY_ENUM_BUTTON_V].system_code = 9;
	betray_buttons[BETRAY_ENUM_BUTTON_W].system_code = 13;
	betray_buttons[BETRAY_ENUM_BUTTON_X].system_code = 7;
	betray_buttons[BETRAY_ENUM_BUTTON_Y].system_code = 16;
	betray_buttons[BETRAY_ENUM_BUTTON_Z].system_code = 6;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD0].system_code = 82;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD1].system_code = 83;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD2].system_code = 84;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD3].system_code = 85;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD4].system_code = 86;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD5].system_code = 87;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD6].system_code = 88;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD7].system_code = 89;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD8].system_code = 91;
	betray_buttons[BETRAY_ENUM_BUTTON_NUMPAD9].system_code = 92;
	betray_buttons[BETRAY_ENUM_BUTTON_MULTIPLY].system_code = 67;
	betray_buttons[BETRAY_ENUM_BUTTON_ADD].system_code = 69;
	betray_buttons[BETRAY_ENUM_BUTTON_SUBTRACT].system_code = 78;
	betray_buttons[BETRAY_ENUM_BUTTON_DIVIDE].system_code = 75;
                                                        
    betray_buttons[BETRAY_ENUM_BUTTON_PERIOD].system_code = 47;
    betray_buttons[BETRAY_ENUM_BUTTON_COMMA].system_code = 43;
                                                               
	betray_buttons[BETRAY_ENUM_BUTTON_F1].system_code = 122;
	betray_buttons[BETRAY_ENUM_BUTTON_F2].system_code = 120;
	betray_buttons[BETRAY_ENUM_BUTTON_F3].system_code = 99;
	betray_buttons[BETRAY_ENUM_BUTTON_F4].system_code = 118;
	betray_buttons[BETRAY_ENUM_BUTTON_F5].system_code = 96;
	betray_buttons[BETRAY_ENUM_BUTTON_F6].system_code = 97;
	betray_buttons[BETRAY_ENUM_BUTTON_F7].system_code = 98;
	betray_buttons[BETRAY_ENUM_BUTTON_F8].system_code = 100;
	betray_buttons[BETRAY_ENUM_BUTTON_F9].system_code = 101;
	betray_buttons[BETRAY_ENUM_BUTTON_F10].system_code = 109;
	betray_buttons[BETRAY_ENUM_BUTTON_F11].system_code = 103;
	betray_buttons[BETRAY_ENUM_BUTTON_F12].system_code = 111;
/*	betray_buttons[BETRAY_ENUM_BUTTON_F13].system_code = VK_F13;
	betray_buttons[BETRAY_ENUM_BUTTON_F14].system_code = VK_F14;
	betray_buttons[BETRAY_ENUM_BUTTON_F15].system_code = VK_F15;
	betray_buttons[BETRAY_ENUM_BUTTON_F16].system_code = VK_F16;
	betray_buttons[BETRAY_ENUM_BUTTON_F17].system_code = VK_F17;
	betray_buttons[BETRAY_ENUM_BUTTON_F18].system_code = VK_F18;
	betray_buttons[BETRAY_ENUM_BUTTON_F19].system_code = VK_F19;
	betray_buttons[BETRAY_ENUM_BUTTON_F20].system_code = VK_F20;
	betray_buttons[BETRAY_ENUM_BUTTON_F21].system_code = VK_F21;
	betray_buttons[BETRAY_ENUM_BUTTON_F22].system_code = VK_F22;
	betray_buttons[BETRAY_ENUM_BUTTON_F23].system_code = VK_F23;
	betray_buttons[BETRAY_ENUM_BUTTON_F24].system_code = VK_F24;*/
    betray_buttons[BETRAY_ENUM_BUTTON_SCROLL_UP].system_code = 116;
    betray_buttons[BETRAY_ENUM_BUTTON_SCROLL_DOWN].system_code = 121;
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

#endif