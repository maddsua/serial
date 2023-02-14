#include "terminal.hpp"

#include <fstream>

#include <stdio.h>

const uint8_t hex_table[16] = {
	'0','1','2','3','4','5','6','7',
	'8','9','a','b','c','d','e','f'
};

std::string bytesToHex(std::string bytes, bool fullStyle) {

	std::string result;

	char temp[5] = "0x00";

	for (size_t i = 0; i < bytes.size(); i++) {
		temp[2] = hex_table[(bytes[i] & 0xF0) >> 4];
		temp[3] = hex_table[bytes[i] & 0x0F];

		result.insert(result.end(), temp + (fullStyle ? 0 : 2), temp + 4);
		if (i < bytes.size() - 1) result += ' ';
	}

	return result;
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
		dropdown(&ui->combo_port, &dropitems, data->sel_port, true);
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

	//	message preformatting
	if (incoming && data->hexMode) message = bytesToHex(message, data->hexStyleFull);
	
	if (!incoming || data->hexMode) message += "\r\n";

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

	//	get input text
	char userinput[1024];
	if (!GetWindowTextA(ui->command, userinput, sizeof(userinput))) return;

	//	check if there are ports
	if (data->sel_port >= data->ports.size()) {
		SetWindowTextA(ui->statusbar, "No active port selected!");
		return;
	}

	//	echo to terminal
	if (data->echoInputs) printComm(ui, data, userinput, false);

	//	command history stuff
	for (auto itr = data->history.begin(); itr != data->history.end(); itr++) {
		if ((*itr) == userinput) {
			data->history.erase(itr);
			break;
		}
	}
	data->history.push_back(userinput);
	

	//	write data to com port
	auto port = data->ports.at(data->sel_port);
	auto endline = data->endlines.at(data->sel_endline).bytes;
	if (!serial->write(port, (data->specialCharsSupport ? restoreEscapedChars(userinput) : std::string(userinput)) + endline)) {
		SetWindowTextA(ui->statusbar, "Failed to send the message!");
		return;
	}

	//	clear command prompt
	resetCommandPrompt(ui, data);
}

struct unescape {
	char front;
	char escape;
};

const std::vector <unescape> escapedCharsTable = {
	{'n', '\n'},
	{'r', '\r'},
	{'t', '\t'},
	{'b', '\b'}
};

std::string restoreEscapedChars(std::string text) {

	for (size_t i = 0; i < text.size(); i++) {

		char escapechar = 0;

		for (auto item : escapedCharsTable) {
			if (text[i] == item.front) {
				escapechar = item.escape;
				break;
			}
		}

		if (i >= 2 && (escapechar && text.at(i - 1) == '\\')) {
			if (i >= 3 && text.at(i - 2) == '\\') {
				text.erase(i - 2, 1);
				continue;
			}
			text.replace(text.begin() + (i - 1), text.begin() + (i + 1), std::string(1, escapechar));
			i -= 2;
		}
	}

	return text;
}
