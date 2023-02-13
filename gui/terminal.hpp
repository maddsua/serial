
#ifndef _SERIALTERMINALAPP
#define _SERIALTERMINALAPP

	#include <windows.h>
	#include <CommCtrl.h>

	#include <vector>
	#include <string>
	#include <algorithm>

	#include "interface.hpp"
	#include "../lib/serial.hpp"

	#define TERMINAL_MAX_TEXTLEN	(28000)
	#define TERMINAL_CUT_OVERFLOW	(1000)

	#define HISTORY_FORWARD			(-1)
	#define HISTORY_BACKWARD		(1)

	//	save communacations log to a file
	void saveCommLog(HWND* appwnd, std::vector <std::string>* logdata);

	//	update available ports
	void updateComPorts(maddsua::serial* serial, uiElements* ui, appData* data);

	//	print messages to a terminal
	void printComm(uiElements* ui, appData* data, std::string message, bool incoming);

	void updateStatusBar(maddsua::serial* serial, uiElements* ui, appData* data);

	void sendMessage(maddsua::serial* serial, uiElements* ui, appData* data);

	void historyRecall(uiElements* ui, appData* data, int step);

	std::string bytesToHex(std::string bytes, bool fullStyle);


#endif