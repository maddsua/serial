//  2023 maddsua | https://github.com/maddsua
//
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#include <windows.h>
#include <CommCtrl.h>
#include <stdio.h>

#include <thread>
#include <vector>
#include <string>
#include <algorithm>

#include "../lib/serial.hpp"

#include "interface.hpp"
#include "terminal.hpp"

const std::vector <uint32_t> serialSpeeds = {
	110, 300, 600, 1200, 2400, 4800, 9600,
	14400, 19200, 38400, 56000, 57600,
	115200, 128000, 256000
};


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK keyboardEvents(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC mainevents;


//	-------		main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	WNDCLASSEXA wc = {0};
	MSG msg;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
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

	//	set default line endings
	data.endlines = {
		{"CR+LF", "\r\n"},
		{"No endline", {/* empty line */}},
		{"CR only", "\r"},
		{"LF only", "\n"}
	};

	static auto serial = new maddsua::serial(8, false);	//	!!!	change it to scanSerialPorts

	//	get menus
	ui.menu_main = GetMenu(hwnd);
	ui.menu_hexStyle = GetSubMenu(ui.menu_main, 1);	//	(1) - index of that menu by the resource file

	static HBRUSH hbrBkgnd = 0;
	
	switch(Message) {
			
		case WM_CREATE: {
						
			//	get available serial speeds from the serial api
			//	ports are not gonna be ready just yet so we will get them by the times later
			data.speeds = serial->getSpeeds();

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