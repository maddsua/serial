#include "terminalgui.hpp"

#include <fstream>
#include <stdio.h>

void uiInit(HWND* appwnd, uiElements* ui, uiData* data) {

	//	drop lists
	ui->comboport = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 420, 8, 80, 200, *appwnd, (HMENU)GUI_COMBO_PORT, NULL, NULL);
		
	ui->combospeed = CreateWindowA(WC_COMBOBOXA, NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VSCROLL, 500, 8, 120, 200, *appwnd, (HMENU)GUI_COMBO_SPEED, NULL, NULL);  
	dropdown(&ui->combospeed, &data->speeds, data->sel_speed, false);
	
	//	log
	ui->terminal = CreateWindowA(WC_EDITA, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 0, 40, 630, 300, *appwnd, (HMENU)GUI_LOGWIN, NULL, NULL);	
		
	//	input	
	ui->commprompt = CreateWindowA(WC_EDITA, NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER, 10, 350, 510, 24, *appwnd, (HMENU)GUI_COMPROM, NULL, NULL);			
	
	//	buttons
	ui->senditbtn = CreateWindowA(WC_BUTTONA, "Send", WS_VISIBLE | WS_CHILD, 530, 350, 80, 25, *appwnd, (HMENU)GUI_BTN_SEND, NULL, NULL);
	
	ui->clearbtn = CreateWindowA(WC_BUTTONA, "Clear&&Update", WS_VISIBLE | WS_CHILD, 530, 380, 80, 25, *appwnd, (HMENU)GUI_BTN_CLR, NULL, NULL);
	
	//	checkboxes
	ui->newlinecheck = CreateWindowA(WC_BUTTONA, "Use new line", WS_VISIBLE | WS_CHILD | BS_VCENTER | BS_AUTOCHECKBOX, 10, 10, 80, 16, *appwnd, (HMENU)GUI_CHK_NLN, NULL, NULL);
	SendMessageW(ui->newlinecheck, BM_SETCHECK, BST_CHECKED, 0);
	
	ui->extended = CreateWindowA(WC_BUTTONA, "AT controls", WS_VISIBLE | WS_CHILD | BS_VCENTER | BS_AUTOCHECKBOX, 100, 10, 75, 16, *appwnd, (HMENU)GUI_CHK_QKAT, NULL, NULL);

	//	AT-macro buttons
	ui->atbtn_at = CreateWindowA(WC_BUTTONA, "AT", WS_CHILD, 10, 380, 80, 25, *appwnd, (HMENU)GUI_AT_AT, NULL, NULL);
	ui->atbtn_id = CreateWindowA(WC_BUTTONA, "AT+ID", WS_CHILD, 95, 380, 80, 25, *appwnd, (HMENU)GUI_AT_ID, NULL, NULL);
	ui->atbtn_ok = CreateWindowA(WC_BUTTONA, "OK", WS_CHILD, 180, 380, 80, 25, *appwnd, (HMENU)GUI_AT_OK, NULL, NULL);
	ui->atbtn_prefix = CreateWindowA(WC_BUTTONA, "AT+", WS_CHILD, 265, 380, 80, 25, *appwnd, (HMENU)GUI_AT_PREF, NULL, NULL);
	
	//	set font
	for (int i = GUI_LOGWIN; i <= GUI_AT_ID; i++)
		SendDlgItemMessage(*appwnd, i, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE,0));
}

void dropdown(HWND* combo, std::vector <std::string>* items, size_t focus, bool erase) {
	
	if (erase) {
		size_t contlen = SendMessage(*combo, CB_GETCOUNT, 0, 0);
		for (size_t i = 0; i < contlen; i++)
			SendMessage(*combo, CB_DELETESTRING, 0, 0);
	}
	
	for (int i = 0; i < items->size(); i++) {
		std::string listPorts = items->at(i);
		SendMessage(*combo, CB_ADDSTRING, 0, (LPARAM)listPorts.c_str());
	}

	SendMessage(*combo, CB_SETCURSEL , focus, 0);
}

void displayAboutMessage() {

	std::string msg = std::string(APP_NAME) + " v" + APP_VERSION +
		"\nA serial port communication utility\n\n" +
		VER_AUTHSTAMP + "\n" + APP_COPYRIGHT;
		
	MessageBoxA(NULL, msg.c_str(), "About...", 0);
}

void saveLogDialog(HWND* appwnd, std::vector <std::string>* logdata) {

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

void disconnectPort(maddsua::serial* serial, uiElements* ui, uiData* data) {

	//printf("Clear focus: %i\r\n", serial->clearFocus());
	serial->clearFocus();
}

void updateComPorts(maddsua::serial* serial, uiElements* ui, uiData* data) {

	auto portsList = serial->stats();
	std::vector <uint32_t> portsAvail;

	//printf("Entries: %i\r\n", portsList.size());

	/*for (size_t i = 0; i < portsList.size(); i++) {
		printf("COM: %i : %i\r\n", portsList[i].port, portsList[i].status);
	}*/
	

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
		dropdown(&ui->comboport, &dropitems, data->sel_port, true);
	}

	//	check if selected port is connected
	if (data->sel_port < data->ports.size()) {

		auto entry = serial->stats(data->ports.at(data->sel_port));

		if (!entry.focus) {
			printf("entry: %i\r\n", entry.port);
			auto res = serial->setFocus(entry.port);
		}
	}
	
}