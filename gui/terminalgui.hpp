
#ifndef _SERIALTERMINALAPPGUI
#define _SERIALTERMINALAPPGUI

	#include <windows.h>

	struct uiElements {
		HWND terminalwindow;
		HWND combospeed;
		HWND comboport;
		HWND commprompt;
		HWND senditbtn;
		HWND clearbtn;
		HWND newlinecheck;
		HWND extended;
		
		HWND atbtn_at;
		HWND atbtn_id;
		HWND atbtn_ok;
		HWND atbtn_prefix;
	};

	struct uiData {

	};


#endif