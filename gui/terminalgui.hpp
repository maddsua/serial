
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
		std::vector <std::string> speeds;
		std::vector <std::string> ports;

		std::string buffIn;
		std::string buffOut;

		int32_t commstat;

		size_t sel_speed;
		size_t sel_port;
		size_t historyItem;

		bool viewHistory;
		bool useNewline;
		bool isExtended;
	};


#endif