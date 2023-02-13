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

void saveCommLog(HWND* appwnd, std::vector <std::string>* logdata) {

	OPENFILENAMEA ofn;
	memset(&ofn, 0, sizeof(ofn));

	char fpath[MAX_PATH];
	memset(fpath, 0, sizeof(fpath));
		
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = *appwnd;
	ofn.lpstrFilter = "Log Files (*.log)\0*.log\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = fpath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "txt";

	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (!GetSaveFileNameA(&ofn)) return;

	std::ofstream localFile(ofn.lpstrFile, std::ios::out);

	if (!localFile.is_open()) {
		MessageBoxA(*appwnd, "Save file failed.", "Error", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	for (auto record : *logdata) {
		localFile << record;
	}
	
	localFile.close();

}

void updateComPorts(maddsua::serial* serial, uiElements* ui, appData* data) {

	auto portsList = serial->list();
	std::vector <uint32_t> portsAvail;
	

	for (auto entry : portsList) {
		if (entry.status == SPSTAT_ACTIVE || entry.status == SPSTAT_AVAILABLE)
			portsAvail.push_back(entry.port);
	}

	if (portsAvail != data->ports) {
		
		//	get item that already selected
		if (data->sel_port < data->ports.size()) {

			auto selected = data->ports.at(data->sel_port);

			//	move dropdown selection
			data->sel_port = portsAvail.size() ? portsAvail.size() - 1 : 0;
			for (size_t i = 0; i < portsAvail.size(); i++) {
				if (portsAvail[i] == selected) {
					data->sel_port = i;
					break;
				}
			}
		}

		data->ports = portsAvail;

		std::vector <std::string> dropitems;
		for (auto item : data->ports)
			dropitems.push_back("COM" + std::to_string(item));

		//	rerender dropdown
		dropdown(&ui->comboPort, &dropitems, data->sel_port, true);
	}

	//	check if selected port is connected
	if (data->sel_port < data->ports.size()) {

		auto entry = serial->info(data->ports.at(data->sel_port));

		if (!entry.focus) {
			auto res = serial->setFocus(entry.port);
		}
	}	
}

void printComm(uiElements* ui, appData* data, std::string message, bool incoming) {

	//	add port name and data direction
	if (data->sel_port < data->ports.size()) {
		auto portInfo = std::string("COM") + std::to_string(data->ports.at(data->sel_port)) + (incoming ? "  --->  " : "  <---  ");
		message.insert(message.begin(), portInfo.begin(), portInfo.end());
	}

	//	add timestamps
	if (data->showTimestamps) {
		char timebuff[32];
		auto epoch = time(nullptr);
		auto timedata = gmtime(&epoch);
		strftime(timebuff, sizeof(timebuff), "[%H:%M:%S]  ", timedata);
		message.insert(message.begin(), timebuff, timebuff + strlen(timebuff));
	}

	if (!incoming) message += "\n";
	
	//	remove old data from terminal if it's too big
	size_t txtcontlen = SendMessage(ui->terminal, WM_GETTEXTLENGTH, 0, 0);
	if (txtcontlen > TERMINAL_MAX_TEXTLEN) {
		//	select part of a text from the beginning
		SendMessage(ui->terminal, EM_SETSEL, (WPARAM)0, (LPARAM)(TERMINAL_CUT_OVERFLOW));
		//	erase it
		SendMessage(ui->terminal, EM_REPLACESEL, 0, 0);
		//	get new content length
		txtcontlen = SendMessage(ui->terminal, WM_GETTEXTLENGTH, 0, 0);
	}
	
	//	move cusor to the end
	SendMessage(ui->terminal, EM_SETSEL, (WPARAM)txtcontlen, (LPARAM)txtcontlen);
	//	paste new text
	SendMessage(ui->terminal, EM_REPLACESEL, 0, (LPARAM)message.c_str());
	//	save new text in the log
	data->log.push_back(message);
}

void updateStatusBar(maddsua::serial* serial, uiElements* ui, appData* data) {

	//	exit if we can't access a port just yet
	if (data->sel_port >= data->ports.size()) {
		SetWindowTextA(ui->statusbar, "Terminal ready");
		return;
	}

	auto port = data->ports.at(data->sel_port);
	auto statusString = std::string("COM") + std::to_string(port) + " : " + serial->statusText(serial->info(port).status);
	SetWindowTextA(ui->statusbar, statusString.c_str());
}

void sendMessage(maddsua::serial* serial, uiElements* ui, appData* data) {

	char userinput[1024];
	if (!GetWindowTextA(ui->command, userinput, sizeof(userinput))) return;

	if (data->sel_port >= data->ports.size()) {
		SetWindowTextA(ui->statusbar, "No active port selected!");
		return;
	}

	for (auto itr = data->history.begin(); itr != data->history.end(); itr++) {
		if ((*itr) == userinput) {
			data->history.erase(itr);
			break;
		}
	}

	data->history.push_back(userinput);

	auto port = data->ports.at(data->sel_port);
	auto endline = data->endlines.at(data->sel_endline).bytes;

	if (data->echoCommands) printComm(ui, data, userinput, false);

	if (!serial->write(port, std::string(userinput) + endline)) {
		SetWindowTextA(ui->statusbar, "Failed to send the message!");
		return;
	}

	SetWindowText(ui->command, NULL);
}