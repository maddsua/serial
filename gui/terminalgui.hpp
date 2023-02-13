
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

	struct uiElements {
		HWND terminal;
		HWND cmdInput;
		HWND comboSpeed;
		HWND comboPort;
		HWND comboLine;
		HWND btnSend;
		HWND btnClear;
		HWND timestamps;
		HWND echoCommands;
	};

	struct endlineoption {
		std::string title;
		std::string bytes;
	};

	struct uiData {
		std::vector <uint32_t> speeds;
		std::vector <uint32_t> ports;
		std::vector <endlineoption> endlines;

		std::vector <std::string> log;
		std::vector <std::string> cmdHistory;

		std::string buffIn;
		std::string buffOut;

		size_t sel_speed = 0;
		size_t sel_port = 0;
		size_t sel_endline = 0;
		size_t historyItem = 0;

		bool showTimestamps = true;
		bool echoCommands = true;
		//bool portWasChanged = false;

		bool viewHistory = false;
		bool useNewline = true;
		bool isExtended = false;
	};

	//	ini gui
	void uiInit(HWND* appwnd, uiElements* ui, uiData* data);

	//	set bombobox items
	void dropdown(HWND* combo, std::vector <std::string>* items, size_t focus, bool erase);

	//	display About Message, captain obvious
	void displayAboutMessage();

	//	save communacations log to a file
	void saveCommLog(HWND* appwnd, std::vector <std::string>* logdata);

	//	update available ports
	void updateComPorts(maddsua::serial* serial, uiElements* ui, uiData* data);

	//	reset communacations
	void resetComms(maddsua::serial* serial, uiElements* ui, uiData* data);

	//	print messages to a terminal
	void printComm(uiElements* ui, uiData* data, std::string message, bool incoming, int printMode);


#endif