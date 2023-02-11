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

#include "app.hpp"

const int serialSpeedList[serialSpeeds] = {
	110, 300, 600, 1200, 2400, 4800, 9600,
	14400, 19200, 38400, 56000, 57600,
	115200, 128000, 256000
};


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK cmdEVs(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
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
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIconA(hInstance, "APPICON");
	wc.hIconSm		 = LoadIconA(hInstance, "APPICON");

	if (!RegisterClassExA(&wc)) {
		MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	//	calc window position (1/8 left; 1/10 top)
    int winPosx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowSizeX);
    int winPosy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowSizeY / 1.2);

	HWND hwnd = CreateWindowExA(WS_EX_CLIENTEDGE, "WindowClass", APP_NAME, WS_VISIBLE | WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
		winPosx, winPosy, windowSizeX, windowSizeY,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	while(GetMessage(&msg, NULL, 0, 0) > 0) {
	
		// skip msg translation to prevent sound	
		if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)
			goto dspmsg;
	
		TranslateMessage(&msg);
		
	dspmsg:
		DispatchMessage(&msg);
	}
	
	return msg.wParam;
}


//	-------		input form enter key press
LRESULT CALLBACK cmdEVs(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
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
	
	static std::thread worker;
	
	static char bufferIn[commsgbuff];
	static char bufferOut[commsgbuff];
	static int commstat;
	static bool placeNewLine;

	static HWND combospeed;
	static HWND comboport;
	static HWND terminalwindow;
	static HWND commprompt;
	static HWND senditbtn;
	static HWND clearbtn;
	static HWND newlinecheck;
	static HWND extended;
	
	static HWND atbtn_at;
	static HWND atbtn_id;
	static HWND atbtn_ok;
	static HWND atbtn_prefix;
	
	static char** serialPorts;
	static unsigned int portsReady;
	
	static unsigned int selspeed;
	static unsigned int selport;
	
	static bool isExtended;
	
	static std::vector <std::string> commLog;
	static std::vector <std::string> cmdHistory;
	
	static bool viewHistory;
	static int historyItem;
		
	
switch(Message) {
		
	case WM_CREATE: {
					
		//	init vars
		serialPorts = create2d(scanSerialPorts, portNameLen);
		portsReady = scanPorts(serialPorts);
			
		placeNewLine = true;
		commstat = 0;
		
		//	default settings
		selspeed = defSerialSpeed;
		
		if (portsReady != 0) selport = portsReady - 1;
			else selport = 0;
		
		isExtended = false;
		viewHistory = false;
		
		
		//	draw GUI
		//	drop lists
		comboport = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 420, 8, 80, 200, hwnd, (HMENU)GUI_COMBO_PORT, NULL, NULL);
			dropdown(comboport, serialPorts, portsReady, selport, true);
			
		combospeed = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 500, 8, 120, 200, hwnd, (HMENU)GUI_COMBO_SPEED, NULL, NULL);  
			dropdown(combospeed, serialSpeedList, serialSpeeds, selspeed, false);
		
		//	log
		terminalwindow = CreateWindowA(WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 0, 40, 630, 300, hwnd, (HMENU)GUI_LOGWIN, NULL, NULL);	
			
		//	input	
		commprompt = CreateWindowA(WC_EDITA, NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER, 10, 350, 510, 24, hwnd, (HMENU)GUI_COMPROM, NULL, NULL);			
		
		//	buttons
		senditbtn = CreateWindowA("BUTTON", "Send", WS_VISIBLE | WS_CHILD, 530, 350, 80, 25, hwnd, (HMENU)GUI_BTN_SEND, NULL, NULL);
		
		clearbtn = CreateWindowA("BUTTON", "Clear&&Update", WS_VISIBLE | WS_CHILD, 530, 380, 80, 25, hwnd, (HMENU)GUI_BTN_CLR, NULL, NULL);
		
		//	checkboxes
		newlinecheck = CreateWindowA("BUTTON", "Use new line", WS_VISIBLE | WS_CHILD | BS_VCENTER | BS_AUTOCHECKBOX, 10, 10, 80, 16, hwnd, (HMENU)GUI_CHK_NLN, NULL, NULL);
		SendMessageW(newlinecheck, BM_SETCHECK, BST_CHECKED, 0);
		
		extended = CreateWindowA("BUTTON", "AT controls", WS_VISIBLE | WS_CHILD | BS_VCENTER | BS_AUTOCHECKBOX, 100, 10, 75, 16, hwnd, (HMENU)GUI_CHK_QKAT, NULL, NULL);
			
		//	AT-macro buttons
		atbtn_at = CreateWindowA("BUTTON", "AT", WS_CHILD, 10, 380, 80, 25, hwnd, (HMENU)GUI_AT_AT, NULL, NULL);
		atbtn_id = CreateWindowA("BUTTON", "AT+ID", WS_CHILD, 95, 380, 80, 25, hwnd, (HMENU)GUI_AT_ID, NULL, NULL);
		atbtn_ok = CreateWindowA("BUTTON", "OK", WS_CHILD, 180, 380, 80, 25, hwnd, (HMENU)GUI_AT_OK, NULL, NULL);
		atbtn_prefix = CreateWindowA("BUTTON", "AT+", WS_CHILD, 265, 380, 80, 25, hwnd, (HMENU)GUI_AT_PREF, NULL, NULL);
		
		//	set font
		for (int i = GUI_LOGWIN; i <= GUI_AT_ID; i++)
			SendDlgItemMessage(hwnd, i, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE,0));
		
		//	start app
		worker = std::thread(serialIO, serialPorts[selport], serialSpeedList[selspeed], bufferIn, bufferOut, &commstat, placeNewLine);
		SetTimer(hwnd, CYCLE_PRINT, TTOUT, NULL);
		
		//	ewdirect keypress input for input form
		mainevents = (WNDPROC)SetWindowLongPtr(commprompt, GWLP_WNDPROC, (LONG_PTR)cmdEVs);

		break;
	}
	
	case WM_COMMAND:{
			
		switch(HIWORD(wParam)) {
		
			case LBN_SELCHANGE:{
				
				switch(LOWORD(wParam)) {
					
					case GUI_COMBO_PORT:{
						
						//	disconnect
						if (!commstat) {
							commstat = 1;
							worker.join();
						}
						
						//	clear
						memset(bufferOut, 0, sizeof(bufferOut));
						SetWindowText(terminalwindow, 0);
						
						//	clear log
						commLog.clear();
						
						//	select
						selport = (int) SendMessageW(comboport, CB_GETCURSEL, 0, 0);
						
						//	reconnect
						commstat = 0;
						worker = std::thread(serialIO, serialPorts[selport], serialSpeedList[selspeed], bufferIn, bufferOut, &commstat, placeNewLine);
							
						break;
					}
					case GUI_COMBO_SPEED: {
						selspeed = (int) SendMessageW(combospeed, CB_GETCURSEL, 0, 0);		
						break;
					}
				}
				break;
			}
			
			case BN_CLICKED:{
				
				switch(LOWORD(wParam)) {
					
					//	clear button
					case GUI_BTN_CLR:{
						
						//	disconnect
						if (!commstat) {
							commstat = 1;
							worker.join();
						}
						
						//	update port list
						portsReady = scanPorts(serialPorts);
						dropdown(comboport, serialPorts, portsReady, selport, true);
						
						//	flush buffers
						memset(bufferIn, 0, sizeof(bufferIn));
						memset(bufferOut, 0, sizeof(bufferOut));
						
						//	clear log
						commLog.clear();
						
						//	erase texts
						SetWindowText(commprompt, 0);
						SetWindowText(terminalwindow, 0);
						
						//	reconnect
						commstat = 0;
						worker = std::thread(serialIO, serialPorts[selport], serialSpeedList[selspeed], bufferIn, bufferOut, &commstat, placeNewLine);

						break;
					}
					
					//	send button
					case GUI_BTN_SEND:{
						
						viewHistory = false;
						
						//	get command trom input control
						char userCommand[commsgbuff];
						GetWindowTextA(commprompt, userCommand, commsgbuff);
						
						//	process command
						if (strlen(userCommand) > 0) {
							
							//	add command to history
							if (cmdHistory.size() > 0) {
								
								bool foundCmd = false;
								int foundCmdIndex;
								
								for (int i = 0; i < cmdHistory.size(); i++) {
									
									if (userCommand == cmdHistory[i]) {
										
										foundCmd = true;
										foundCmdIndex = i;
									}
								}
								
								if (foundCmd) std::swap(cmdHistory[foundCmdIndex], cmdHistory[cmdHistory.size() - 1]);
									else cmdHistory.push_back(userCommand);
								
							} else {
								cmdHistory.push_back(userCommand);
							}
							
							//	display command
							if (placeNewLine) {
								
								//	add new line sign
								strcat(userCommand, "\n");
								
								//	add port info
								char logtmp[comlogbuff];
									metalog(userCommand, serialPorts[selport], logtmp, true);
								
								//	display and write log
								log(terminalwindow, logtmp);
								commLog.push_back(logtmp);
							}
							
							//	copy command to output buffer
							strcpy(bufferOut, userCommand);
							
							//	clear command prompt
							SetWindowText(commprompt, 0);
						}
						
						break;
					}
					
					//	AT-buttons
					case GUI_AT_PREF:{
						
						//	copy text to command prompt
						SetWindowTextA(commprompt, "AT+");
						
						//	set focus
						SetFocus(commprompt);
						
						//	set text curcor position
						int TextLen = SendMessage(commprompt, WM_GETTEXTLENGTH, 0, 0);
						SendMessage(commprompt, EM_SETSEL, (WPARAM)TextLen, (LPARAM)TextLen);
						
						break;
					}
					case GUI_AT_AT:{
						
						quickcmd(terminalwindow, "AT\n", placeNewLine, serialPorts[selport], &commLog, bufferOut);
						break;
					}	
					case GUI_AT_OK:{

						quickcmd(terminalwindow, "OK\n", placeNewLine, serialPorts[selport], &commLog, bufferOut);
						break;
					}
					case GUI_AT_ID:{

						quickcmd(terminalwindow, "AT+ID\n", placeNewLine, serialPorts[selport], &commLog, bufferOut);
						break;
					}
					
				//	checkboxes	
					case GUI_CHK_NLN:{
						
						//	just get the flag
						placeNewLine = (bool) SendMessageW(newlinecheck, BM_GETCHECK, 0, 0);
						break;
					}
					
					case GUI_CHK_QKAT:{
						
						unsigned short int showflag;
						
						if (isExtended) {
							isExtended = false;
							showflag = 0;
						}
						else{
							isExtended = true;
							showflag = 1;
						}
						
						//	draw extended controls
							ShowWindow(atbtn_prefix, showflag);
							ShowWindow(atbtn_ok, showflag);
							ShowWindow(atbtn_id, showflag);
							ShowWindow(atbtn_at, showflag);

						break;
					}
					
				//	menus
					case CM_ABOUT:{
						
						char msgabout[256] = {0};
							sprintf(msgabout, "%s v%s\nA serial port communication utility\n\n%s\n%s", APP_NAME, APP_VERSION, VER_AUTHSTAMP, APP_COPYRIGHT);
							
						MessageBoxA(NULL, msgabout, "About...", 0);
						break;
					}
					
					case CM_FILE_SVLOG:{
						
						OPENFILENAMEA ofn = {0};
							char fpath[MAX_PATH] = {0};
							
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = "Log Files (*.log)\0*.log\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
						ofn.lpstrFile = fpath;
						ofn.nMaxFile = MAX_PATH;
						ofn.lpstrDefExt = "txt";

						ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
						
						if (GetSaveFileNameA(&ofn)) {
							
							if (!SaveLogFile(&commLog, ofn.lpstrFile)) {
								MessageBoxA(hwnd, "Save file failed.", "Error", MB_OK | MB_ICONEXCLAMATION);
							}
						}
	
						break;
					}
					
					case CM_FILE_EXIT:{
						
						commstat = 1;
						PostMessage(hwnd, WM_CLOSE, 0, 0);
						break;
					}
					
					//	custom events
					case ICEV_CMDLIST: {
						
						if (cmdHistory.size() > 0) {
						
							//	open history of scroll trough it
							if (!viewHistory) {
								
								historyItem = cmdHistory.size() - 1;
								viewHistory = true;
							}
							else{
							
								if (lParam == 0) {
									historyItem--;
								}
								else{
									historyItem++;
								}
							}
							
								//	set index in range
								if (historyItem < 0) {
									historyItem = 0;
								}
								else if (historyItem >= cmdHistory.size()) {
									historyItem = cmdHistory.size() - 1;
								}
							
							//	paste cmd
							SetWindowTextA(commprompt, cmdHistory[historyItem].c_str());
								
							//	set text curcor position
							int TextLen = SendMessage(commprompt, WM_GETTEXTLENGTH, 0, 0);
							SendMessage(commprompt, EM_SETSEL, (WPARAM)TextLen, (LPARAM)TextLen);
						}
						
						break;
					}
				}
				
				break;
			}
		}
		
		break;
	}
	
	case WM_SETFOCUS:{
		
		SetFocus(commprompt);
		break;
	}
	
	case WM_TIMER: {
		
		if (!commstat && strlen(bufferIn) > 0) {
			
			char logtmp[comlogbuff];
			
			if (placeNewLine) metalog(bufferIn, serialPorts[selport], logtmp, false);
			else strcpy(logtmp, bufferIn);

			log(terminalwindow, logtmp);
			commLog.push_back(logtmp);
			
			memset(bufferIn, 0, sizeof(bufferIn)*sizeof(char));
		}

		switch (commstat) {
			case 2:
				log(terminalwindow, "___ Port is not connected ___\n");
			break;

			case 3:
				log(terminalwindow, "___ Port busy ___\n");
			break;

			case 4:
				log(terminalwindow, "___ Port config error ___\n");
			break;

			case 6:
				log(terminalwindow, "___ Port has been disconnected ___\n");
			break;
		
			default:
			break;
		}
				
		commstat = 0;
		
		break;
	}
	
	case WM_DESTROY: {
		
		//	close io thread
		commstat = 1;
		worker.join();
		
		//	destroy ports array
		clear2d(serialPorts, scanSerialPorts);
		
		//	exit
		PostQuitMessage(0);
		
		break;
	}
	
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
}

	return 0;
}
