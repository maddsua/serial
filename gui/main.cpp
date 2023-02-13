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

#include "app.hpp"
#include "terminalgui.hpp"

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
	wc.lpszMenuName  = "MAINMENU";
	wc.lpszClassName = "*windowClass";
	wc.hIcon		 = LoadIconA(hInstance, "APPICON");
	wc.hIconSm		 = LoadIconA(hInstance, "APPICON");

	if (!RegisterClassExA(&wc)) {
		MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	
	//	calc window position (1/8 left; 1/10 top)
    int winPosx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowSizeX);
    int winPosy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowSizeY / 1.2);

	HWND hwnd = CreateWindowExA(WS_EX_CLIENTEDGE, "*windowClass", APP_NAME, WS_VISIBLE | WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, winPosx, winPosy, windowSizeX, windowSizeY, NULL, NULL, hInstance, NULL);

	if (!hwnd) {
		MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 2;
	}

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
	
		// skip msg translation to prevent sound	
		if (msg.wParam != VK_RETURN)
			TranslateMessage(&msg);

		DispatchMessage(&msg);
	}
	
	return msg.wParam;
}


//	-------		input form enter key press
LRESULT CALLBACK keyboardEvents(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	//	enter key press
	if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
		return CallWindowProc(WndProc, wnd, WM_COMMAND, GUI_BTN_SEND, 0);
	
	//	ctrl+A shortcut
	} else if (msg == WM_CHAR && wParam == 1) {
		SendMessage(wnd, EM_SETSEL, 0, -1);
		return 0;
	}
	
	//	up/down arrows for cmd history
	else if (msg == WM_KEYDOWN && wParam == VK_UP) {
		return CallWindowProc(WndProc, wnd, WM_COMMAND, ICEV_CMDLIST, (LPARAM)0);

	} else if (msg == WM_KEYDOWN && wParam == VK_DOWN) {
		return CallWindowProc(WndProc, wnd, WM_COMMAND, ICEV_CMDLIST, (LPARAM)1);
	}
	
	//	other
	else return CallWindowProc(mainevents, wnd, msg, wParam, lParam);

   return 0;
}



//	-------		app itself
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {

	static uiElements ui;
	static uiData data;
		data.endlines = {
			{"CR+LF", "\r\n"},
			{"No endline", {/* empty line */}},
			{"CR only", "\r"},
			{"LF only", "\n"}
		};

	static auto serial = new maddsua::serial(8, false);	//	!!!	change it to scanSerialPorts
	
	switch(Message) {
			
		case WM_CREATE: {
						
			//	get available serial speeds from the serial api
			//	ports are not gonna be ready just yet so we will get them by the times later
			data.speeds = serial->getSpeeds();

			uiInit(&hwnd, &ui, &data);
			
			SetTimer(hwnd, TIMER_DATAREAD, TIMEOUT_DATAREAD, NULL);
			SetTimer(hwnd, TIMER_PORTSLIST, TIMEOUT_PORTSLIST, NULL);

			//	redirect keypress event for input form
			mainevents = (WNDPROC)SetWindowLongPtr(ui.cmdInput, GWLP_WNDPROC, (LONG_PTR)keyboardEvents);

			break;
		}
		
		case WM_COMMAND: {
				
			switch (HIWORD(wParam)) {
			
				case LBN_SELCHANGE: {
					
					switch(LOWORD(wParam)) {
						
						case GUI_COMBO_PORT: {
							
							//	select a different port
							size_t temp = SendMessageW(ui.comboPort, CB_GETCURSEL, 0, 0);
							//	exit if it's the same
							if (temp == data.sel_port) break;

							//	assing port
							data.sel_port = temp;
							//	reset
							resetComms(serial, &ui, &data);
														
							break;
						}

						case GUI_COMBO_SPEED: {
							
							//	select different speed
							size_t temp = SendMessageW(ui.comboSpeed, CB_GETCURSEL, 0, 0);	
							//	exit if it's the same
							if (temp == data.sel_speed) break;

							//	apply new speed
							data.sel_speed = temp;
							serial->setSpeed(data.speeds.at(data.sel_speed));
							resetComms(serial, &ui, &data);

							break;
						}
					}

					break;
				}
				
				//	buttons
				case BN_CLICKED: {
					
					switch(LOWORD(wParam)) {
												
						//	send
						case GUI_BTN_SEND: {
							
							data.viewHistory = false;
							
							//	get command trom input control
							char userCommand[commsgbuff];
							GetWindowTextA(ui.cmdInput, userCommand, commsgbuff);
							
							//	process command
							if (strlen(userCommand) > 0) {
								
								//	add command to history
								if (data.cmdHistory.size() > 0) {
									
									bool foundCmd = false;
									int foundCmdIndex;
									
									for (int i = 0; i < data.cmdHistory.size(); i++) {
										
										if (userCommand == data.cmdHistory[i]) {
											foundCmd = true;
											foundCmdIndex = i;
										}
									}
									
									if (foundCmd) std::swap(data.cmdHistory[foundCmdIndex], data.cmdHistory[data.cmdHistory.size() - 1]);
										else data.cmdHistory.push_back(userCommand);
									
								} else {
									data.cmdHistory.push_back(userCommand);
								}
								
								//	display command
								if (data.useNewline) {
									
									//	add new line sign
									strcat(userCommand, "\n");
									
									//	add port info
									//char logtmp[comlogbuff];
									//metalog(userCommand, data.ports[data.sel_port].c_str(), logtmp, true);
									
									//	display and write log
									//log(ui.terminalwindow, logtmp);
									//data.commLog.push_back(logtmp);
								}
								
								//	copy command to output buffer
								//strcpy(bufferOut, userCommand);
								
								//	clear command prompt
								SetWindowText(ui.cmdInput, 0);
							}
							
							break;
						}


						//	checkboxes
						case CHECKBOX_TIMESTAMP: 
							data.showTimestamps = SendMessageA(ui.timestamps, BM_GETCHECK, 0, 0);
						break;
						
						case CHECKBOX_ECHOCMD: 
							data.echoCommands = SendMessageA(ui.echoCommands, BM_GETCHECK, 0, 0);
						break;
						
						
						//	context menus
						case CONTEXT_ABOUT:
							displayAboutMessage();
						break;
											
						case CONTEXT_FILE_SVLOG: 
							saveCommLog(&hwnd, &data.log);
						break;
						
						case CONTEXT_FILE_EXIT: 
							PostMessage(hwnd, WM_CLOSE, 0, 0);
						break;

						case CONTEXT_CLEAR: {
							
							//	clear log
							data.log.clear();
							
							//	erase texts
							SetWindowText(ui.cmdInput, 0);
							SetWindowText(ui.terminal, 0);

							//	reset serial comms
							resetComms(serial, &ui, &data);
							
							break;
						}


						//	custom events
						case ICEV_CMDLIST: {
							
							if (data.cmdHistory.size() > 0) {
							
								//	open history of scroll trough it
								if (!data.viewHistory) {
									
									data.historyItem = data.cmdHistory.size() - 1;
									data.viewHistory = true;
								} else{
								
									if (lParam == 0) {
										data.historyItem--;
									}
									else{
										data.historyItem++;
									}
								}
								
									//	set index in range
									if (data.historyItem < 0) {
										data.historyItem = 0;

									} else if (data.historyItem >= data.cmdHistory.size()) {
										data.historyItem = data.cmdHistory.size() - 1;
									}
								
								//	paste cmd
								SetWindowTextA(ui.cmdInput, data.cmdHistory[data.historyItem].c_str());
									
								//	set text curcor position
								int TextLen = SendMessage(ui.cmdInput, WM_GETTEXTLENGTH, 0, 0);
								SendMessage(ui.cmdInput, EM_SETSEL, (WPARAM)TextLen, (LPARAM)TextLen);
							}
							
							break;
						}
					}
					
					break;
				}
			}
			
			break;
		}
		
		case WM_SETFOCUS: 
			SetFocus(ui.cmdInput);
		break;
		
		case WM_TIMER: {

			if (wParam == TIMER_DATAREAD) {

				//	exit if we can't access a port just yet
				if (data.sel_port >= data.ports.size()) break;

				auto input = serial->read(data.ports[data.sel_port]);

				if (!input.size()) break;

				printComm(&ui, &data, input, true, 0);

				/*switch (data.commstat) {
					case 2:
						log(ui.terminalwindow, "___ Port is not connected ___\n");
					break;

					case 3:
						log(ui.terminalwindow, "___ Port busy ___\n");
					break;

					case 4:
						log(ui.terminalwindow, "___ Port config error ___\n");
					break;

					case 6:
						log(ui.terminalwindow, "___ Port has been disconnected ___\n");
					break;
				
					default:
					break;
				}*/

			} else if (wParam == TIMER_PORTSLIST) {

				updateComPorts(serial, &ui, &data);
			}

			break;
		}
		
		case WM_DESTROY: {

			delete serial;
			PostQuitMessage(0);

			break;
		}
		
		
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}

	return 0;
}
