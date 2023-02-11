
#ifndef _SERIALTERMINALAPPGUI
#define _SERIALTERMINALAPPGUI

	#include <windows.h>
	#include <CommCtrl.h>

	#include <vector>
	#include <string>

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

		std::vector <std::string> commLog;
		std::vector <std::string> cmdHistory;

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

	void uiInit(HWND* appwnd, uiElements* ui, uiData* data);
	void dropdown(HWND combo, std::vector <std::string>* items, size_t focus, bool erase);


#endif