#include "terminalgui.hpp"

#include <fstream>
#include <stdio.h>

void uiInit(HWND* appwnd, uiElements* ui, appData* data) {

	//	serial speeds dropdown
	//	it isn't gonna be updated coz speeds are hardcoded to the library
	//	so render it only once and then just use as intended
	ui->comboSpeed = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 500, 8, 120, 200, *appwnd, (HMENU)GUI_COMBO_SPEED, NULL, NULL);  
	{
		std::vector <std::string> temp;

		for (auto item : data->speeds) {
			temp.push_back(std::to_string(item) + " baud");
		}

		//	select 9600, bc it's the default
		for (size_t i = 1; i < data->speeds.size(); i++) {
			if (data->speeds.at(i) == SIO_DEFAULT_SPEED) {
				data->sel_speed = i;
				break;
			}
		}

		dropdown(&ui->comboSpeed, &temp, data->sel_speed, false);
	}

	//	port selector
	//	the items are gonna be assigned by the update timer callback
	//	so gonna leave it empty for now
	ui->comboPort = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 420, 8, 80, 200, *appwnd, (HMENU)GUI_COMBO_PORT, NULL, NULL);

	//	line ending selector
	ui->comboLineEnding = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 340, 8, 80, 200, *appwnd, (HMENU)GUI_COMBO_LINE, NULL, NULL);
	{
		std::vector <std::string> temp;
		for (auto item : data->endlines) temp.push_back(item.title);
		dropdown(&ui->comboLineEnding, &temp, data->sel_endline, false);
	}

	//	terminal window itself
	ui->terminal = CreateWindowA(WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 0, 40, 630, 300, *appwnd, (HMENU)GUI_LOGWIN, NULL, NULL);	
		
	//	input	
	ui->command = CreateWindowA(WC_EDITA, NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER, 10, 350, 568, 24, *appwnd, (HMENU)GUI_COMPROM, NULL, NULL);
	
	//	buttons
	ui->btnSend = CreateWindowA(WC_BUTTONA, "Send", WS_VISIBLE | WS_CHILD | BS_BITMAP, 588, 350, 25, 25, *appwnd, (HMENU)GUI_BTN_SEND, NULL, NULL);
	{
		HBITMAP	image_send = LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(ICON_BUTTON_SEND));
			if (image_send) SendMessage(ui->btnSend, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)image_send);
			else printf("didn't load: %i\n", GetLastError());
	}
		
	//	checkboxes
	ui->timestamps = CreateWindowA(WC_BUTTONA, "Show timestamps", WS_VISIBLE | WS_CHILD | BS_VCENTER | BS_AUTOCHECKBOX, 10, 10, 110, 16, *appwnd, (HMENU)CHECKBOX_TIMESTAMP, NULL, NULL);
	SendMessageW(ui->timestamps, BM_SETCHECK, BST_CHECKED, 0);
	
	ui->echoCommands = CreateWindowA(WC_BUTTONA, "Echo commands", WS_VISIBLE | WS_CHILD | BS_VCENTER | BS_AUTOCHECKBOX, 125, 10, 105, 16, *appwnd, (HMENU)CHECKBOX_ECHOCMD, NULL, NULL);
	SendMessageW(ui->echoCommands, BM_SETCHECK, BST_CHECKED, 0);

	//	status bar
	ui->statusbar = CreateWindowA(WC_STATICA, "Starting up...", WS_VISIBLE | WS_CHILD | SS_LEFT, 5, 410, 200, 16, *appwnd, (HMENU)GUI_STATUSBAR, NULL, NULL);

	//	set font
	for (size_t i = 0; i <= 1000; i++) {
		if (!GetDlgItem(*appwnd, i)) continue;
		SendDlgItemMessage(*appwnd, i, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(1, 0));
	}
}

void dropdown(HWND* combo, std::vector <std::string>* items, size_t focus, bool erase) {
	
	if (erase) {
		size_t contlen = SendMessage(*combo, CB_GETCOUNT, 0, 0);
		for (size_t i = 0; i < contlen; i++)
			SendMessage(*combo, CB_DELETESTRING, 0, 0);
	}

	for (auto entry : *items)
		SendMessage(*combo, CB_ADDSTRING, 0, (LPARAM)entry.c_str());
	
	SendMessage(*combo, CB_SETCURSEL , focus, 0);
}

void displayAboutMessage() {

	std::string msg = std::string(APP_NAME) + " v" + APP_VERSION +
		"\nA serial port communication utility\n\n" +
		VER_AUTHSTAMP + "\n" + APP_COPYRIGHT;
		
	MessageBoxA(NULL, msg.c_str(), "About...", 0);
}