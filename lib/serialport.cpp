#include "./libserial.hpp"
#include <set>
#include <windows.h>

//	fucking microsoft forgot to add this code
#define WINERR_DEV_NOTFOUND	(433)

using namespace Serial;

std::set<uint32_t> serialSpeeds = {
	110, 150, 300, 600, 1200,
	1800, 2400, 4800, 7200,
	9600, 14400, 19200, 38400,
	56000, 57600, 115200, 128000
};

Port::Port(uint16_t port) {

	this->portidx = port;

	char filePath[16];
	snprintf(filePath, sizeof(filePath), "\\\\.\\COM%i", this->portidx);
	this->hPort = CreateFileA(filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (this->hPort == INVALID_HANDLE_VALUE) {
		auto errcode = GetLastError();
		this->apiError = errcode;
		this->portStatus = (errcode == ERROR_FILE_NOT_FOUND || errcode == WINERR_DEV_NOTFOUND) ? PORTSTAT_NOT_CONNECTED : PORTSTAT_PORT_ERROR;
		return;
	}

	COMMTIMEOUTS timeouts = {1, 1, 1, 1, 1};
	if (!SetCommTimeouts(this->hPort, &timeouts)) {
		this->apiError = GetLastError();
		this->portStatus = PORTSTAT_SETT_ERR;
		return;
	}

	DCB settings;
	if (!GetCommState(this->hPort, &settings)) {
		this->apiError = GetLastError();
		this->portStatus = PORTSTAT_SETT_ERR;
		return;
	}

	settings.BaudRate = this->portSpeed;
	settings.ByteSize = 8;
	settings.StopBits = ONESTOPBIT;
	settings.Parity = NOPARITY;

	if (!SetCommState(this->hPort, &settings)) {
		this->apiError = GetLastError();
		this->portStatus = PORTSTAT_SETT_ERR;
		return;
	}

	this->portStatus = PORTSTAT_OK;
}

Port::~Port() {
	if (this->hPort == INVALID_HANDLE_VALUE) return;
	EscapeCommFunction(this->hPort, CLRDTR);
	PurgeComm(this->hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
	CloseHandle(this->hPort);
}

bool Port::setSpeed(uint32_t bSpeed) {

	DCB settings;
	if (!GetCommState(this->hPort, &settings)) {
		this->apiError = GetLastError();
		this->portStatus = PORTSTAT_SETT_ERR;
		return false;
	}

	settings.BaudRate = bSpeed;

	if (!SetCommState(this->hPort, &settings)) {
		this->apiError = GetLastError();
		this->portStatus = PORTSTAT_SETT_ERR;
		return false;
	}

	this->portSpeed = bSpeed;

	return true;
}

uint16_t Port::getPortIdx() {
	return this->portidx;
}

bool Port::available() {
	return this->bufferRx.size();
}

std::vector<uint8_t> Port::read() {
	std::lock_guard<std::mutex>lock(threadLock);
	auto temp = this->bufferRx;
	this->bufferRx.clear();
	return temp;
}

bool Port::write(std::vector<uint8_t>& data) {
	
	if (this->hPort == INVALID_HANDLE_VALUE || this->portStatus != PORTSTAT_OK) return false;

	uint32_t bytesWritten = 0;

	if (!WriteFile(this->hPort, data.data(), data.size(), (DWORD*)&bytesWritten, NULL)) {
		this->apiError = GetLastError();
		this->portStatus = PORTSTAT_READ_ERR;
		return false;
	}
	
	this->transferTX += bytesWritten;
	return true;
}
