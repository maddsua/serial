
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
		std::vector <int> portIndexes;

		std::vector <std::string> commLog;
		std::vector <std::string> cmdHistory;

		std::string buffIn;
		std::string buffOut;

		int commstat = 0;

		int sel_speed = 0;
		int sel_port = 0;
		size_t historyItem = 0;

		bool viewHistory = false;
		bool useNewline = true;
		bool isExtended = false;
	};

	void uiInit(HWND* appwnd, uiElements* ui, uiData* data);
	void dropdown(HWND combo, std::vector <std::string>* items, size_t focus, bool erase);
	void displayAboutMessage();
	void saveLogDialog(HWND* appwnd, std::vector <std::string>* logdata);


#endif