
#ifndef _SERIALTERMINALAPPGUI
#define _SERIALTERMINALAPPGUI

	#include <windows.h>
	#include <CommCtrl.h>

	#include <vector>
	#include <string>
	#include <algorithm>

	#include "rescodes.hpp"
	#include "app.hpp"

	#include "../lib/serial.hpp"

	struct uiElements {
		HWND terminal;
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
		std::vector <uint32_t> speeds;
		std::vector <uint32_t> ports;

		std::vector <std::string> log;
		std::vector <std::string> cmdHistory;

		std::string buffIn;
		std::string buffOut;

		size_t sel_speed = 0;
		size_t sel_port = 0;
		size_t historyItem = 0;

		bool viewHistory = false;
		bool useNewline = true;
		bool isExtended = false;
	};

	void uiInit(HWND* appwnd, uiElements* ui, uiData* data);
	void dropdown(HWND* combo, std::vector <std::string>* items, size_t focus, bool erase);
	void displayAboutMessage();
	void saveLogDialog(HWND* appwnd, std::vector <std::string>* logdata);
	void updateComPorts(maddsua::serial* serial, uiElements* ui, uiData* data);
	void resetComms(maddsua::serial* serial, uiElements* ui, uiData* data);


#endif