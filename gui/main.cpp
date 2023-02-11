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
#include "terminalgui.hpp"

std::vector <int> serialSpeeds = {
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
	wc.lpszClassName = "*windowClass";
	wc.hIcon		 = LoadIconA(hInstance, "APPICON");
	wc.hIconSm		 = LoadIconA(hInstance, "APPICON");

	if (!RegisterClassExA(&wc)) {
		MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	
	//	calc window position (1/8 left; 1/10 top)
    int winPosx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (windowSizeX);
    int winPosy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (windowSizeY / 1.2);

	HWND hwnd = CreateWindowExA(WS_EX_CLIENTEDGE, "*windowClass", APP_NAME, WS_VISIBLE | WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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

void uiInit(HWND* appwnd, uiElements* ui, uiData* data) {
	//	drop lists
	ui->comboport = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 420, 8, 80, 200, *appwnd, (HMENU)GUI_COMBO_PORT, NULL, NULL);
	dropdown(ui->comboport, &data->ports, data->sel_port, true);
		
	ui->combospeed = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 500, 8, 120, 200, *appwnd, (HMENU)GUI_COMBO_SPEED, NULL, NULL);  
	dropdown(ui->combospeed, &data->speeds, data->sel_speed, false);
	
	//	log
	ui->terminalwindow = CreateWindowA(WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 0, 40, 630, 300, *appwnd, (HMENU)GUI_LOGWIN, NULL, NULL);	
		
	//	input	
	ui->commprompt = CreateWindowA(WC_EDITA, NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER, 10, 350, 510, 24, *appwnd, (HMENU)GUI_COMPROM, NULL, NULL);			
	
	//	buttons
	ui->senditbtn = CreateWindowA("BUTTON", "Send", WS_VISIBLE | WS_CHILD, 530, 350, 80, 25, *appwnd, (HMENU)GUI_BTN_SEND, NULL, NULL);
	
	ui->clearbtn = CreateWindowA("BUTTON", "Clear&&Update", WS_VISIBLE | WS_CHILD, 530, 380, 80, 25, *appwnd, (HMENU)GUI_BTN_CLR, NULL, NULL);
	
	//	checkboxes
	ui->newlinecheck = CreateWindowA("BUTTON", "Use new line", WS_VISIBLE | WS_CHILD | BS_VCENTER | BS_AUTOCHECKBOX, 10, 10, 80, 16, *appwnd, (HMENU)GUI_CHK_NLN, NULL, NULL);
	SendMessageW(ui->newlinecheck, BM_SETCHECK, BST_CHECKED, 0);
	
	ui->extended = CreateWindowA("BUTTON", "AT controls", WS_VISIBLE | WS_CHILD | BS_VCENTER | BS_AUTOCHECKBOX, 100, 10, 75, 16, *appwnd, (HMENU)GUI_CHK_QKAT, NULL, NULL);
		
	//	AT-macro buttons
	ui->atbtn_at = CreateWindowA("BUTTON", "AT", WS_CHILD, 10, 380, 80, 25, *appwnd, (HMENU)GUI_AT_AT, NULL, NULL);
	ui->atbtn_id = CreateWindowA("BUTTON", "AT+ID", WS_CHILD, 95, 380, 80, 25, *appwnd, (HMENU)GUI_AT_ID, NULL, NULL);
	ui->atbtn_ok = CreateWindowA("BUTTON", "OK", WS_CHILD, 180, 380, 80, 25, *appwnd, (HMENU)GUI_AT_OK, NULL, NULL);
	ui->atbtn_prefix = CreateWindowA("BUTTON", "AT+", WS_CHILD, 265, 380, 80, 25, *appwnd, (HMENU)GUI_AT_PREF, NULL, NULL);
	
	//	set font
	for (int i = GUI_LOGWIN; i <= GUI_AT_ID; i++)
		SendDlgItemMessage(*appwnd, i, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE,0));
}

//	-------		app itself
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	
	static std::thread worker;

	static uiElements ui;
	static uiData data;
	
	data.speeds = {
		"110", "300", "600", "1200", "2400", "4800", "9600",
		"14400", "19200", "38400", "56000", "57600",
		"115200", "128000", "256000"
	};

	static char bufferIn[commsgbuff];
	static char bufferOut[commsgbuff];

	static char porttemp[125];
		
	
switch(Message) {
		
	case WM_CREATE: {
					
		//	init vars
		//serialPorts = create2d(scanSerialPorts, portNameLen);
		scanPorts(&data.ports);
			
		data.useNewline = true;
		data.commstat = 0;
		
		//	default settings
		data.sel_speed = defSerialSpeed;
		
		if (data.ports.size()) data.sel_port = data.ports.size() - 1;
			else data.sel_port = 0;
		
		data.isExtended = false;
		data.viewHistory = false;

		uiInit(&hwnd, &ui, &data);
		
		//	start app
		memset(porttemp, 0, sizeof(porttemp));
		memcpy(porttemp, data.ports[data.sel_port].c_str(), data.ports[data.sel_port].size());
		worker = std::thread(serialIO, porttemp, serialSpeeds[data.sel_speed], bufferIn, bufferOut, &data.commstat, data.useNewline);
		SetTimer(hwnd, CYCLE_PRINT, TTOUT, NULL);

		//	ewdirect keypress input for input form
		mainevents = (WNDPROC)SetWindowLongPtr(ui.commprompt, GWLP_WNDPROC, (LONG_PTR)cmdEVs);

		break;
	}
	
	case WM_COMMAND:{
			
		switch(HIWORD(wParam)) {
		
			case LBN_SELCHANGE:{
				
				switch(LOWORD(wParam)) {
					
					case GUI_COMBO_PORT:{
						
						//	disconnect
						if (!data.commstat) {
							data.commstat = 1;
							worker.join();
						}
						
						//	clear
						memset(bufferOut, 0, sizeof(bufferOut));
						SetWindowText(ui.terminalwindow, 0);
						
						//	clear log
						data.commLog.clear();
						
						//	select
						data.sel_port = (int) SendMessageW(ui.comboport, CB_GETCURSEL, 0, 0);
						
						//	reconnect
						data.commstat = 0;
						memset(porttemp, 0, sizeof(porttemp));
						memcpy(porttemp, data.ports[data.sel_port].c_str(), data.ports[data.sel_port].size());
						worker = std::thread(serialIO, porttemp, serialSpeeds[data.sel_speed], bufferIn, bufferOut, &data.commstat, data.useNewline);
							
						break;
					}
					case GUI_COMBO_SPEED: {
						data.sel_speed = (int) SendMessageW(ui.combospeed, CB_GETCURSEL, 0, 0);		
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
						if (!data.commstat) {
							data.commstat = 1;
							worker.join();
						}
						
						//	update port list
						scanPorts(&data.ports);
						dropdown(ui.comboport, &data.ports, data.sel_port, true);
						
						//	flush buffers
						memset(bufferIn, 0, sizeof(bufferIn));
						memset(bufferOut, 0, sizeof(bufferOut));
						
						//	clear log
						data.commLog.clear();
						
						//	erase texts
						SetWindowText(ui.commprompt, 0);
						SetWindowText(ui.terminalwindow, 0);
						
						//	reconnect
						data.commstat = 0;
						memset(porttemp, 0, sizeof(porttemp));
						memcpy(porttemp, data.ports[data.sel_port].c_str(), data.ports[data.sel_port].size());
						worker = std::thread(serialIO, porttemp, serialSpeeds[data.sel_speed], bufferIn, bufferOut, &data.commstat, data.useNewline);

						break;
					}
					
					//	send button
					case GUI_BTN_SEND:{
						
						data.viewHistory = false;
						
						//	get command trom input control
						char userCommand[commsgbuff];
						GetWindowTextA(ui.commprompt, userCommand, commsgbuff);
						
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
								char logtmp[comlogbuff];
									metalog(userCommand, data.ports[data.sel_port].c_str(), logtmp, true);
								
								//	display and write log
								log(ui.terminalwindow, logtmp);
								data.commLog.push_back(logtmp);
							}
							
							//	copy command to output buffer
							strcpy(bufferOut, userCommand);
							
							//	clear command prompt
							SetWindowText(ui.commprompt, 0);
						}
						
						break;
					}
					
					//	AT-buttons
					case GUI_AT_PREF:{
						
						//	copy text to command prompt
						SetWindowTextA(ui.commprompt, "AT+");
						
						//	set focus
						SetFocus(ui.commprompt);
						
						//	set text curcor position
						int TextLen = SendMessage(ui.commprompt, WM_GETTEXTLENGTH, 0, 0);
						SendMessage(ui.commprompt, EM_SETSEL, (WPARAM)TextLen, (LPARAM)TextLen);
						
						break;
					}
					case GUI_AT_AT:{
						
						quickcmd(ui.terminalwindow, "AT\n", data.useNewline, data.ports[data.sel_port].c_str(), &data.commLog, bufferOut);
						break;
					}	
					case GUI_AT_OK:{

						quickcmd(ui.terminalwindow, "OK\n", data.useNewline, data.ports[data.sel_port].c_str(), &data.commLog, bufferOut);
						break;
					}
					case GUI_AT_ID:{

						quickcmd(ui.terminalwindow, "AT+ID\n", data.useNewline, data.ports[data.sel_port].c_str(), &data.commLog, bufferOut);
						break;
					}
					
				//	checkboxes	
					case GUI_CHK_NLN:{
						
						//	just get the flag
						data.useNewline = (bool) SendMessageW(ui.newlinecheck, BM_GETCHECK, 0, 0);
						break;
					}
					
					case GUI_CHK_QKAT:{
						
						unsigned short int showflag;
						
						if (data.isExtended) {
							data.isExtended = false;
							showflag = 0;
						}
						else{
							data.isExtended = true;
							showflag = 1;
						}
						
						//	draw extended controls
							ShowWindow(ui.atbtn_prefix, showflag);
							ShowWindow(ui.atbtn_ok, showflag);
							ShowWindow(ui.atbtn_id, showflag);
							ShowWindow(ui.atbtn_at, showflag);

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
							
							if (!SaveLogFile(&data.commLog, ofn.lpstrFile)) {
								MessageBoxA(hwnd, "Save file failed.", "Error", MB_OK | MB_ICONEXCLAMATION);
							}
						}
	
						break;
					}
					
					case CM_FILE_EXIT:{
						
						data.commstat = 1;
						PostMessage(hwnd, WM_CLOSE, 0, 0);
						break;
					}
					
					//	custom events
					case ICEV_CMDLIST: {
						
						if (data.cmdHistory.size() > 0) {
						
							//	open history of scroll trough it
							if (!data.viewHistory) {
								
								data.historyItem = data.cmdHistory.size() - 1;
								data.viewHistory = true;
							}
							else{
							
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
								}
								else if (data.historyItem >= data.cmdHistory.size()) {
									data.historyItem = data.cmdHistory.size() - 1;
								}
							
							//	paste cmd
							SetWindowTextA(ui.commprompt, data.cmdHistory[data.historyItem].c_str());
								
							//	set text curcor position
							int TextLen = SendMessage(ui.commprompt, WM_GETTEXTLENGTH, 0, 0);
							SendMessage(ui.commprompt, EM_SETSEL, (WPARAM)TextLen, (LPARAM)TextLen);
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
		
		SetFocus(ui.commprompt);
		break;
	}
	
	case WM_TIMER: {
		
		if (!data.commstat && strlen(bufferIn) > 0) {
			
			char logtmp[comlogbuff];
			
			if (data.useNewline) metalog(bufferIn, data.ports[data.sel_port].c_str(), logtmp, false);
			else strcpy(logtmp, bufferIn);

			log(ui.terminalwindow, logtmp);
			data.commLog.push_back(logtmp);
			
			memset(bufferIn, 0, sizeof(bufferIn)*sizeof(char));
		}

		switch (data.commstat) {
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
		}
				
		data.commstat = 0;
		
		break;
	}
	
	case WM_DESTROY: {
		
		//	close io thread
		data.commstat = 1;
		worker.join();
				
		//	exit
		PostQuitMessage(0);
		
		break;
	}
	
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
}

	return 0;
}
