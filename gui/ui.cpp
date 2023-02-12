#include "terminalgui.hpp"
#include "rescodes.hpp"
#include "app.hpp"

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

void dropdown(HWND combo, std::vector <std::string>* items, size_t focus, bool erase) {
	
	if (erase) {
		size_t contlen = SendMessage(combo, CB_GETCOUNT, 0, 0);
		for (size_t i = 0; i < contlen; i++)
			SendMessage(combo, CB_DELETESTRING, 0, 0);
	}
	
	for (int i = 0; i < items->size(); i++) {
		std::string temp = items->at(i);
		SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)temp.c_str());
	}

	SendMessage(combo, CB_SETCURSEL , focus, 0);
}

void displayAboutMessage() {

	std::string msg = std::string(APP_NAME) + " v" + APP_VERSION +
		"\nA serial port communication utility\n\n" +
		VER_AUTHSTAMP + "\n" + APP_COPYRIGHT;
		
	MessageBoxA(NULL, msg.c_str(), "About...", 0);
}
