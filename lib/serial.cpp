#include "serial.hpp"

const std::array<int, 17> comSpeeds = {
	110, 150, 300, 600, 1200,
	1800, 2400, 4800, 7200,
	9600, 14400, 19200, 38400,
	56000, 57600, 115200, 128000
};

std::vector <int> maddsua::serial::getSpeeds() {
	return std::vector <int> (comSpeeds.begin(), comSpeeds.end());
};

bool maddsua::serial::setSpeed(int baudrate) {

	for (auto speed : comSpeeds) {
		if (baudrate == speed) {
			serialSpeed = baudrate;
			return true;
		}
	}

	return false;
}

void maddsua::serial::ioloop() {

	int requests = 0;
	time_t systime = GetTickCount64();
	char openPath[16];
	char rxTemp[PORT_CHUNK];
	DWORD rxBytesRead;

	while (running) {

		systime = timeGetTime();
		requests = 0;

		for (auto& entry : pool) {

			if (entry.excluded) {
				if (entry.portHandle) {
					CloseHandle(entry.portHandle);
					entry.portHandle = nullptr;
				}
				continue;
			}

			//	try to establish a connection or perform cleanup
			if (!entry.active && entry.cooldown < systime) {
				memset(openPath, 0, 16);
				snprintf(openPath, 15, "\\\\.\\COM%i", entry.portIndex);
				entry.portHandle = CreateFileA(openPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

				//	if disconnected or error, do cleanup
				if (entry.portHandle == INVALID_HANDLE_VALUE) {

					entry.linePending = 0;
					entry.transferRX = 0;
					entry.transferTX = 0;

					entry.buffRX.clear();
					entry.buffTX.clear();
					entry.deviceID.clear();

					auto errcode = GetLastError();
					if (errcode == ERROR_FILE_NOT_FOUND || errcode == WINERR_DEV_NOTFOUND) {
						//	port is clear
						entry.cooldown = 0;
						entry.status = PORTSTAT_DISCONN;

					} else {
						//	wait for now + PORT_COOLDOWN_MS seconds until next try
						entry.cooldown = (systime + PORT_COOLDOWN_MS);
						entry.status = PORTSTAT_BUSY;
					}

				} else if (!parallelOps) {
					CloseHandle(entry.portHandle);
					entry.status = PORTSTAT_AVAILABLE;
					entry.cooldown = (systime + PORT_CD_FAST_MS);
				}

				if (!parallelOps) continue;

				//	perform setup
				{
					EscapeCommFunction(entry.portHandle, CLRDTR);
					PurgeComm(entry.portHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);

					COMMTIMEOUTS timeouts = {1, 1, 1, 1, 1};
					if (!SetCommTimeouts(entry.portHandle, &timeouts)){
						CloseHandle(entry.portHandle);
						entry.portHandle = nullptr;
						entry.status = PORTSTAT_SETPERR;
						entry.cooldown = (systime + PORT_COOLDOWN_MS);
						continue;
					}

					DCB settings = {0};
					if (!GetCommState(entry.portHandle, &settings)) {
						CloseHandle(entry.portHandle);
						entry.portHandle = nullptr;
						entry.status = PORTSTAT_SETPERR;
						entry.cooldown = (systime + PORT_COOLDOWN_MS);
						continue;
					}

						settings.BaudRate = serialSpeed;
						settings.ByteSize = 8;
						settings.StopBits = ONESTOPBIT;
						settings.Parity = NOPARITY;

					if (!SetCommState(entry.portHandle, &settings)){
						CloseHandle(entry.portHandle);
						entry.portHandle = nullptr;
						entry.status = PORTSTAT_SETPERR;
						entry.cooldown = (systime + PORT_COOLDOWN_MS);
						continue;
					}

					//	setup complete
					entry.status = PORTSTAT_ACTIVE;
					entry.active = true;
				}

				//	skip this cycle
				//	it's unlikely that a device is already transmitting	
				continue;
			}

			//	IO operations
			if (!entry.active) continue;

			//	send data
			if (entry.buffTX.size()) {
				auto sendDataSize = entry.buffTX.size();
				if (!WriteFile(entry.portHandle, entry.buffTX.data(), entry.buffTX.size(), NULL, NULL)){
					CloseHandle(entry.portHandle);
					entry.portHandle = nullptr;
					entry.status = PORTSTAT_IOERROR;
					entry.cooldown = (systime + PORT_COOLDOWN_MS);
					entry.active = false;
					continue;
				}
				entry.transferTX += entry.buffTX.size();
				entry.buffTX.clear();
			}

			//	receive data
			if (!ReadFile(entry.portHandle, &rxTemp, PORT_CHUNK, &rxBytesRead, NULL)){
				CloseHandle(entry.portHandle);
				entry.portHandle = nullptr;
				entry.status = PORTSTAT_IOERROR;
				entry.cooldown = (systime + PORT_COOLDOWN_MS);
				entry.active = false;
				continue;
			}

			if (!rxBytesRead) continue;

			if (textmode) {

				entry.buffRXTemp.insert(entry.buffRXTemp.end(), rxTemp, rxTemp + rxBytesRead);
				entry.linePending = timeGetTime() + PORT_TEXT_WAIT;

				auto newline = entry.buffRXTemp.find_last_of('\n');
				if (newline != std::string::npos) {
					entry.buffRX = std::string(entry.buffRXTemp.begin(), entry.buffRXTemp.begin() + newline + 1);
					entry.buffRXTemp.erase(0, newline + 1);

				} else if (entry.linePending && (timeGetTime() > entry.linePending)) {
					entry.buffRX = entry.buffRXTemp;
					entry.buffRXTemp.clear();
				}

				if (!entry.buffRXTemp.size()) entry.linePending = 0;

			} else entry.buffRX.insert(entry.buffRX.end(), rxTemp, rxTemp + rxBytesRead);

			entry.transferRX += rxBytesRead;
		}

		Sleep(10);
	}

	//	close all when exiting
	for (auto& entry : pool) {
		if (entry.portHandle) CloseHandle(entry.portHandle);
	}
}


bool maddsua::serial::write(int comport, std::string data) {
	comport -= PORT_FIRST;
	if (comport >= pool.size()) return false;
	
	auto& entry = pool[comport];
	if (!entry.active) return false;

	entry.buffTX += data;
	return true;
}
std::string maddsua::serial::read(int comport) {
	comport -= PORT_FIRST;
	if (comport >= pool.size()) return {};

	auto& entry = pool[comport];
	if (!entry.active) return {};

	auto temp = entry.buffRX;
	entry.buffRX.clear();

	return temp;
}

std::string maddsua::serial::read() {

	if (portFocus < 0) return {};

	auto& entry = pool[portFocus];
	if (!entry.active) return {};

	auto temp = entry.buffRX;
	entry.buffRX.clear();

	return temp;
}
bool maddsua::serial::write(std::string data) {
	if (portFocus < 0) return {};

	auto& entry = pool[portFocus];
	if (!entry.active) return false;

	entry.buffTX += data;
	return true;
}

maddsua::serial::readablePortEntry maddsua::serial::stats(int comport) {
	comport -= PORT_FIRST;
	if (comport >= pool.size()) return {};

	auto& entry = pool[comport];
	return stats(entry);
}

maddsua::serial::readablePortEntry maddsua::serial::stats(portEntry& entry) {

	readablePortEntry temp;
		temp.comport = entry.portIndex;
		temp.cooldown = entry.cooldown > 0 ? true : false;
		temp.excluded = entry.excluded;
		temp.dataAvailable = entry.buffRX.size();
		temp.id = entry.deviceID;
		temp.transferRX = entry.transferRX;
		temp.transferTX = entry.transferTX;

	switch (entry.status) {
		case PORTSTAT_AVAILABLE:
			temp.status = "available";
		break;

		case PORTSTAT_BUSY:
			temp.status = "busy";
		break;

		case PORTSTAT_ACTIVE:
			temp.status = "active";
		break;

		case PORTSTAT_IOERROR:
			temp.status = "io error";
		break;

		case PORTSTAT_SETPERR:
			temp.status = "setup error";
		break;
	
		default:
			temp.status = "offline";
		break;
	}
	
	return temp;
}

std::vector <maddsua::serial::readablePortEntry> maddsua::serial::stats() {

	std::vector <readablePortEntry> result;

	for (auto entry : pool)
		result.push_back(stats(entry));

	return result;
}

std::vector <int> maddsua::serial::portsActive() {

	std::vector <int> result;

	for (auto entry : pool) {
		if (entry.active) result.push_back(entry.portIndex);
	}

	return result;
}

std::vector <int> maddsua::serial::portsFree() {

	std::vector <int> result;

	for (auto entry : pool) {
		if (entry.status == (parallelOps ? PORTSTAT_DISCONN : PORTSTAT_AVAILABLE)) result.push_back(entry.portIndex);
	}

	return result;
}

std::vector <int> maddsua::serial::dataAvail() {

	std::vector <int> result;

	for (auto entry : pool) {
		if (entry.buffRX.size())
			result.push_back(entry.portIndex);
	}

	return result;
}

bool maddsua::serial::setPortState(int comport, maddsua::serial::portAttribs attribs) {
	comport -= PORT_FIRST;
	if (comport >= pool.size()) return {};
	
	auto& entry = pool[comport];

	entry.excluded = ~attribs.enabled;

	return true;
}

bool maddsua::serial::setFocus(int comport) {

	comport -= PORT_FIRST;
	if (comport >= pool.size()) return false;

	portFocus = comport;

	return true;
}

void maddsua::serial::clearFocus() {
	portFocus = 0;
}