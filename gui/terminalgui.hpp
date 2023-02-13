
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

	#define TERMINAL_MAX_TEXTLEN	(28000)
	#define TERMINAL_CUT_OVERFLOW	(1000)

	#define HISTORY_FORWARD			(-1)
	#define HISTORY_BACKWARD		(1)

	struct uiElements {
		HWND terminal;
		HWND command;

		HWND combo_speed;
		HWND combo_port;
		HWND combo_lineEnding;

		HWND button_send;

		HWND check_time;
		HWND check_echo;
		HWND check_textmode;

		HWND statusbar;
	};

	struct endlineoption {
		std::string title;
		std::string bytes;
	};

	struct appData {
		std::vector <uint32_t> speeds;
		std::vector <uint32_t> ports;
		std::vector <endlineoption> endlines;

		std::vector <std::string> log;
		std::vector <std::string> history;

		std::string buffIn;
		std::string buffOut;

		size_t sel_speed = 0;
		size_t sel_port = 0;
		size_t sel_endline = 0;
		size_t sel_history = 0;
		size_t historyItem = 0;

		bool showTimestamps = true;
		bool echoInputs = true;
		bool textmode = true;

		bool viewHistory = false;
		bool useNewline = true;
		bool isExtended = false;
	};

	//	ini gui
	void uiInit(HWND* appwnd, uiElements* ui, appData* data);

	//	set bombobox items
	void dropdown(HWND* combo, std::vector <std::string>* items, size_t focus, bool erase);

	//	display About Message, captain obvious
	void displayAboutMessage();

	//	save communacations log to a file
	void saveCommLog(HWND* appwnd, std::vector <std::string>* logdata);

	//	update available ports
	void updateComPorts(maddsua::serial* serial, uiElements* ui, appData* data);

	//	print messages to a terminal
	void printComm(uiElements* ui, appData* data, std::string message, bool incoming);

	void updateStatusBar(maddsua::serial* serial, uiElements* ui, appData* data);

	void sendMessage(maddsua::serial* serial, uiElements* ui, appData* data);

	void historyRecall(uiElements* ui, appData* data, int step);


#endif