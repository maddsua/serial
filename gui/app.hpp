//  2023 maddsua | https://github.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#ifndef _SERIALTERMINALAPPGUI
#define _SERIALTERMINALAPPGUI

	#include <stdint.h>
	#include <vector>
	#include <string>
	#include <windows.h>

	#include "rescodes.hpp"

	#define APP_NAME			"Serial Terminal"
	#define APP_VERSION			"4.0.0"
	#define APP_TITLE			APP_NAME " v" APP_VERSION
	#define APP_DESC			"A serial port communication utility"
	#define VER_AUTHSTAMP		"2023 maddsua"
	#define APP_COPYRIGHT		"https://github.com/maddsua"


	#define CONFIG_SAVE_TREE	"\\AppData\\Local\\maddsuadev\\serialterminal\\config.json"


	#define windowSizeX			640
	#define windowSizeY			460

	#define IO_DEFAULT_SPEED	(9600)


	struct uiElements {
		HWND terminal;
		HWND command;

		HWND combo_speed;
		HWND combo_port;
		HWND combo_lineEnding;

		HWND button_send;

		HWND check_hexMode;

		HWND statusbar;

		HMENU menu_main;
		HMENU menu_hexStyle;
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

		size_t sel_speed = 0;
		size_t sel_port = 0;
		size_t sel_endline = 0;
		size_t sel_history = 0;
		size_t historyItem = 0;

		bool showTimestamps = true;
		bool echoInputs = true;
		bool hexMode = false;
		bool hexStyleFull = false;
		bool specialCharsSupport = true;

		bool historyOpen = false;
	};
	

	//	init gui
	void uiInit(HWND* appwnd, uiElements* ui, appData* data);

	//	set bombobox items
	void dropdown(HWND* combo, std::vector <std::string>* items, size_t focus, bool erase);

	//	display About Message, captain obvious
	void displayAboutMessage();
	void displayHelpMessage();

	void selectSubmenu_hexStyle(uiElements* ui, size_t selectID);
	void checkMainMenuItem(uiElements* ui, size_t selectID, bool checked);

	void historyRecall(uiElements* ui, appData* data, int step);
	void resetCommandPrompt(uiElements* ui, appData* data);


#endif
