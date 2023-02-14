//  2023 maddsua | https://github.com/maddsua
//
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#include <windows.h>
#include <CommCtrl.h>
#include <stdio.h>
#include <dir.h>
#include <dirent.h>

#include <thread>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>
#include <fstream>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "../lib/serial.hpp"

#include "app.hpp"
#include "terminal.hpp"

const std::vector <uint32_t> serialSpeeds = {
	110, 300, 600, 1200, 2400, 4800, 9600,
	14400, 19200, 38400, 56000, 57600,
	115200, 128000, 256000
};


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK keyboardEvents(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC mainevents;

std::string preparePath(std::string tree);
bool saveConfiguration(appData* data);
bool loadConfiguration(appData* data);


//	-------		main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	WNDCLASSEXA wc = {0};
	MSG msg;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(0xffffff);
	wc.lpszMenuName  = APP_MAIN_MENU_ID;
	wc.lpszClassName = "terminalMainWindow";
	wc.hIcon		 = LoadIconA(hInstance, "APPICON");
	wc.hIconSm		 = LoadIconA(hInstance, "APPICON");

	if (!RegisterClassExA(&wc)) {
		MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	
	//	calc window position (1/8 left; 1/10 top)
    auto winPosx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowSizeX);
    auto winPosy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowSizeY / 1.2);

	HWND hwnd = CreateWindowExA(WS_EX_CLIENTEDGE, "terminalMainWindow", APP_TITLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, winPosx, winPosy, windowSizeX, windowSizeY, NULL, NULL, hInstance, NULL);

	if (!hwnd) {
		MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 2;
	}

	while (GetMessage(&msg, NULL, 0, 0)) {
	
		// skip msg translation to prevent sound	
		if (msg.wParam != VK_RETURN && msg.wParam != VK_ESCAPE)
			TranslateMessage(&msg);

		DispatchMessage(&msg);
	}
	
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

	static uiElements ui;
	static appData data;

	static auto serial = new maddsua::serial(8, false);	//	!!!	change it to scanSerialPorts

	//static HBRUSH hbrBkgnd = 0;
	
	switch(Message) {
			
		case WM_CREATE: {
						
			//	get available serial speeds from the serial api
			//	ports are not gonna be ready just yet so we will get them by the times later
			data.speeds = serial->getSpeeds();

			//	set default line endings
			data.endlines = {
				{"CR+LF", "\r\n"},
				{"No endline", {/* empty line */}},
				{"CR only", "\r"},
				{"LF only", "\n"}
			};

			//	load user config
			loadConfiguration(&data);

			//	get menus
			ui.menu_main = GetMenu(hwnd);
			ui.menu_hexStyle = GetSubMenu(ui.menu_main, 1);	//	(1) - index of that menu by the resource file

			uiInit(&hwnd, &ui, &data);
			
			SetTimer(hwnd, TIMER_DATAREAD, TIMEOUT_DATAREAD, NULL);
			SetTimer(hwnd, TIMER_PORTSLIST, TIMEOUT_PORTSLIST, NULL);

			//	redirect keypress event for input form
			mainevents = (WNDPROC)SetWindowLongPtr(ui.command, GWLP_WNDPROC, (LONG_PTR)keyboardEvents);

			break;
		}
		
		case WM_COMMAND: {
				
			switch (HIWORD(wParam)) {
			
				//	dropdowns
				case LBN_SELCHANGE: {
					
					switch(LOWORD(wParam)) {
						
						case GUI_DROP_PORT: {

							//	select a different port
							size_t temp = SendMessageW(ui.combo_port, CB_GETCURSEL, 0, 0);
							//	exit if it's the same
							if (temp == data.sel_port) break;

							//	assing port
							data.sel_port = temp;
							//	reset
							serial->clearFocus();

						} break;
						
						case GUI_DROP_SPEED: {

							//	select different speed
							size_t temp = SendMessageW(ui.combo_speed, CB_GETCURSEL, 0, 0);	
							//	exit if it's the same
							if (temp == data.sel_speed) break;

							//	apply new speed
							data.sel_speed = temp;
							serial->setSpeed(data.speeds.at(data.sel_speed));
							serial->clearFocus();

						} break;
						
						case GUI_DROP_LINE: {
							data.sel_endline = SendMessageW(ui.combo_lineEnding, CB_GETCURSEL, 0, 0);
						} break;
						
					}

				} break;
				
				//	buttons
				case BN_CLICKED: {
					
					switch(LOWORD(wParam)) {
												
						//	send
						case GUI_BUTTON_SEND: {
							sendMessage(serial, &ui, &data);
						} break;
						

						//	checkboxes

						case GUI_CHECK_HEXMODE : {
							data.hexMode = SendMessageA(ui.check_hexMode, BM_GETCHECK, 0, 0);
							serial->setmode(!data.hexMode);
						} break;
						
						
						//	context menus
						case MENUITEM_FILE_SVLOG: {
							saveCommLog(&hwnd, &data.log);
						} break;
						
						case MENUITEM_FILE_EXIT: {
							PostMessage(hwnd, WM_CLOSE, 0, 0);
						} break;


						case MENUITEM_CLEAR: {

							//	clear log
							data.log.clear();
							//	erase texts
							SetWindowText(ui.command, NULL);
							SetWindowText(ui.terminal, NULL);
							//	reset serial comms
							serial->clearFocus();

						} break;

						case SUBMENU_HEXSTYLE_SHORT: {

							data.hexStyleFull = false;
							selectSubmenu_hexStyle(&ui, SUBMENU_HEXSTYLE_SHORT);

						} break;

						case SUBMENU_HEXSTYLE_FULL: {

							data.hexStyleFull = true;
							selectSubmenu_hexStyle(&ui, SUBMENU_HEXSTYLE_FULL);

						} break;

						case MENUITEM_SPECCHARS: {
							//	invert the state
							data.specialCharsSupport = !data.specialCharsSupport;
							checkMainMenuItem(&ui, MENUITEM_SPECCHARS, data.specialCharsSupport);
						} break;

						case MENUITEM_TIMESTAMP: {
							//	invert the state
							data.showTimestamps = !data.showTimestamps;
							checkMainMenuItem(&ui, MENUITEM_TIMESTAMP, data.showTimestamps);
						} break;

						case MENUITEM_ECHOCMD: {
							//	invert the state
							data.echoInputs = !data.echoInputs;
							checkMainMenuItem(&ui, MENUITEM_ECHOCMD, data.echoInputs);
						} break;


						case MENUITEM_ABOUT: {
							displayAboutMessage();
						} break;

						case MENUITEM_HELP: {
							displayHelpMessage();
						} break;	


						//	custom events
						case KEYBOARD_ARROWS: {
							historyRecall(&ui, &data, lParam);
						} break;

						case KEYBOARD_ESCAPE: {
							resetCommandPrompt(&ui, &data);
						} break;
					}
					
				} break;
			}
			
		} break;
		
		case WM_SETFOCUS: {
			SetFocus(ui.command);
		} break;
		
		case WM_TIMER: {

			switch (wParam) {

				case TIMER_DATAREAD: {

					//	exit if we can't access a port just yet
					if (data.sel_port >= data.ports.size()) break;
					
					auto input = serial->read(data.ports.at(data.sel_port));
					if (!input.size()) break;

					printComm(&ui, &data, input, true);

				} break;

				case TIMER_PORTSLIST: {

					updateComPorts(serial, &ui, &data);
					updateStatusBar(serial, &ui, &data);

				} break;
				
			
				default: break;
			}

		} break;
		
		
		case WM_DESTROY: {

			delete serial;
			saveConfiguration(&data);
			PostQuitMessage(0);
			//DestroyWindow(ui.GUI_BUTTON_SEND);

		} break;

		/*case WM_CTLCOLORSTATIC: {
			HDC hdcStatic = (HDC) wParam;
			SetTextColor(hdcStatic, RGB(255,255,255));
			SetBkColor(hdcStatic, RGB(0,0,0));

			if (hbrBkgnd == NULL) hbrBkgnd = CreateSolidBrush(RGB(0,0,0));

			return (INT_PTR)hbrBkgnd;
		}*/
		
		
		default: return DefWindowProc(hwnd, Message, wParam, lParam);
	}

	return 0;
}


LRESULT CALLBACK keyboardEvents(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

		case WM_KEYDOWN: {

			switch (wParam) {

				case VK_RETURN: return CallWindowProc(WndProc, wnd, WM_COMMAND, GUI_BUTTON_SEND, 0);

				case VK_UP: return CallWindowProc(WndProc, wnd, WM_COMMAND, KEYBOARD_ARROWS, (LPARAM)HISTORY_FORWARD);

				case VK_DOWN: return CallWindowProc(WndProc, wnd, WM_COMMAND, KEYBOARD_ARROWS, (LPARAM)HISTORY_BACKWARD);

				case VK_ESCAPE: return CallWindowProc(WndProc, wnd, WM_COMMAND, KEYBOARD_ESCAPE, 0);
			
				default: break;
			}

		} break;

		case WM_CHAR: {
			
			switch (wParam) {

				case 1: {
					SendMessage(wnd, EM_SETSEL, 0, -1);
				} break;
			
				default: break;
			}

		} break;
	
		default: break;
	}
	
   return CallWindowProc(mainevents, wnd, msg, wParam, lParam);
}

std::string preparePath(std::string tree) {

	tree = std::regex_replace(tree, std::regex("[\\\\\\/]+"), "\\");

	std::string userdir = std::string(std::getenv("userprofile"));
	if (!userdir.size()) userdir = std::getenv("%HOMEPATH%");

	if (!userdir.size()) {
		return {};
	}

	auto createIfDontexist = [](std::string path) {
		auto dir = opendir(path.c_str());
		if (dir) {
			closedir(dir);
			return true;
		}
		if (mkdir(path.c_str())) return false;
		return true;
	};

	auto hierrarchy = tree.find_first_of('\\');
	while(hierrarchy != std::string::npos) {
		if (!createIfDontexist(userdir + tree.substr(0, hierrarchy))) return {};
		hierrarchy = tree.find_first_of('\\', hierrarchy + 1);
	}

	return userdir + tree;
}

bool saveConfiguration(appData* data) {

	auto filepath = preparePath(CONFIG_SAVE_TREE);
	if (!filepath.size()) return false;

	std::ofstream configFile(filepath.c_str(), std::ios::out);
	if (!configFile.is_open()) return false;

	JSON appconfig = {
		{"showTimestamps", data->showTimestamps},
		{"echoInputs", data->echoInputs},
		{"hexMode", data->hexMode},
		{"hexStyleFull", data->hexStyleFull},
		{"specialCharsSupport", data->specialCharsSupport},
		{"sel_speed", data->sel_speed},
		{"sel_port", data->sel_port},
		{"sel_endline", data->sel_endline}
	};

	configFile << appconfig.dump();

	configFile.close();

	return true;
}

bool loadConfiguration(appData* data) {

	auto filepath = preparePath(CONFIG_SAVE_TREE);
	if (!filepath.size()) return false;

	std::ifstream configFile(filepath.c_str(), std::ios::in);
	if (!configFile.is_open()) return false;

	try {
		
		auto appconfig = JSON::parse(configFile);

		data->showTimestamps = appconfig["showTimestamps"].get<bool>();
		data->echoInputs = appconfig["echoInputs"].get<bool>();
		data->hexMode = appconfig["hexMode"].get<bool>();
		data->hexStyleFull = appconfig["hexStyleFull"].get<bool>();
		data->specialCharsSupport = appconfig["specialCharsSupport"].get<bool>();

		size_t temp;
		temp = appconfig["sel_speed"].get<size_t>();
			if (temp < data->speeds.size()) data->sel_speed = temp;
		temp = appconfig["sel_port"].get<size_t>();
			if (temp < data->ports.size()) data->sel_port = temp;
		temp = appconfig["sel_endline"].get<size_t>();
			if (temp < data->endlines.size()) data->sel_endline = temp;
		
	} catch(...) {
		configFile.close();
		puts("config load failed");
		return false;
	}
	puts("config load ok");
	configFile.close();
	return true;
}