//	2023 maddsua's serial lib
//	v1.0.0
//	https://github.com/maddsua

#include "serial.hpp"


const std::array<uint32_t, 17> comSpeeds = {
	110, 150, 300, 600, 1200,
	1800, 2400, 4800, 7200,
	9600, 14400, 19200, 38400,
	56000, 57600, 115200, 128000
};


/*
     ██████ ██       █████  ███████ ███████ 
    ██      ██      ██   ██ ██      ██      
    ██      ██      ███████ ███████ ███████ 
    ██      ██      ██   ██      ██      ██ 
     ██████ ███████ ██   ██ ███████ ███████ 

*/
maddsua::serial::serial(uint32_t maxPorts, bool parallel) {
	running = true;
	textmode = true;
	parallelOps = parallel;
	serialSpeed = 9600;
	activatePorts = ((maxPorts + PORT_FIRST) < PORTS_COMSMAX) ? maxPorts : PORTS_COMSMAX;

	//	create port port entries
	for (size_t i = PORT_FIRST; i < activatePorts + PORT_FIRST; i++) {
		portEntry temp;
			temp.port = i;
		pool.push_back(std::move(temp));
	}

	daemon = std::thread(ioloop, this);
}
//	destructor
maddsua::serial::~serial() {
	running = false;
	if (daemon.joinable()) daemon.join();
}

void portShutdown(HANDLE& porthandle) {
	EscapeCommFunction(porthandle, CLRDTR);
	PurgeComm(porthandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
	if (CloseHandle(porthandle)) porthandle = nullptr;
}


/*
    ███████ ███████ ████████ ████████ ██ ███    ██  ██████  ███████ 
    ██      ██         ██       ██    ██ ████   ██ ██       ██      
    ███████ █████      ██       ██    ██ ██ ██  ██ ██   ███ ███████ 
         ██ ██         ██       ██    ██ ██  ██ ██ ██    ██      ██ 
    ███████ ███████    ██       ██    ██ ██   ████  ██████  ███████ 
*/
std::vector <uint32_t> maddsua::serial::getSpeeds() {
	return std::vector <uint32_t> (comSpeeds.begin(), comSpeeds.end());
};

bool maddsua::serial::setSpeed(uint32_t baudrate) {

	for (auto speed : comSpeeds) {
		if (baudrate == speed) {
			serialSpeed = baudrate;
			return true;
		}
	}

	return false;
}

/*
    ██████   █████  ███████ ███    ███  ██████  ███    ██ 
    ██   ██ ██   ██ ██      ████  ████ ██    ██ ████   ██ 
    ██   ██ ███████ █████   ██ ████ ██ ██    ██ ██ ██  ██ 
    ██   ██ ██   ██ ██      ██  ██  ██ ██    ██ ██  ██ ██ 
    ██████  ██   ██ ███████ ██      ██  ██████  ██   ████ 
*/
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
				if (entry.portHandle) portShutdown(entry.portHandle);
				continue;
			}

			//	discovery
			if (entry.status != SPSTAT_ACTIVE && entry.cooldown < systime) {
				snprintf(openPath, 15, "\\\\.\\COM%i", entry.port);
				entry.portHandle = CreateFileA(openPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

				//	if disconnected or error, do cleanup
				if (entry.portHandle == INVALID_HANDLE_VALUE) {

					auto errcode = GetLastError();
					if (errcode == ERROR_FILE_NOT_FOUND || errcode == WINERR_DEV_NOTFOUND) {
						//	port is clear
						entry.cooldown = 0;
						entry.status = SPSTAT_DISCONN;

					} else {
						//	wait for now + PORT_COOLDOWN_MS seconds until next try
						entry.cooldown = (systime + PORT_COOLDOWN_MS);
						entry.status = SPSTAT_BUSY;
					}

					entry.portHandle = nullptr;

					entry.linePending = 0;
					entry.transferRX = 0;
					entry.transferTX = 0;

					entry.buffRX.clear();
					entry.buffTX.clear();
					entry.deviceID.clear();

					continue;
				}

				if (parallelOps || entry.focus) {
					
					EscapeCommFunction(entry.portHandle, CLRDTR);
					PurgeComm(entry.portHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);

					COMMTIMEOUTS timeouts = {1, 1, 1, 1, 1};
					if (!SetCommTimeouts(entry.portHandle, &timeouts)){
						portShutdown(entry.portHandle);
						entry.status = SPSTAT_SETPERR;
						entry.cooldown = (systime + PORT_COOLDOWN_MS);
						continue;
					}

					DCB settings = {0};
					if (!GetCommState(entry.portHandle, &settings)) {
						portShutdown(entry.portHandle);
						entry.status = SPSTAT_SETPERR;
						entry.cooldown = (systime + PORT_COOLDOWN_MS);
						continue;
					}

						settings.BaudRate = serialSpeed;
						settings.ByteSize = 8;
						settings.StopBits = ONESTOPBIT;
						settings.Parity = NOPARITY;

					if (!SetCommState(entry.portHandle, &settings)) {
						portShutdown(entry.portHandle);
						entry.status = SPSTAT_SETPERR;
						entry.cooldown = (systime + PORT_COOLDOWN_MS);
						continue;
					}

					//	setup complete
					entry.status = SPSTAT_ACTIVE;

					continue;
				}
	
				portShutdown(entry.portHandle);

				entry.status = SPSTAT_AVAILABLE;
				entry.cooldown = (systime + PORT_CD_FAST_MS);
				continue;
			}


			//	IO operations
			if (entry.status != SPSTAT_ACTIVE) continue;

			//	send data
			if (entry.buffTX.size()) {
				auto sendDataSize = entry.buffTX.size();
				if (!WriteFile(entry.portHandle, entry.buffTX.data(), entry.buffTX.size(), NULL, NULL)){
					portShutdown(entry.portHandle);
					entry.status = SPSTAT_IOERROR;
					entry.cooldown = (systime + PORT_COOLDOWN_MS);
					continue;
				}
				entry.transferTX += entry.buffTX.size();
				entry.buffTX.clear();
			}

			//	receive data
			if (!ReadFile(entry.portHandle, &rxTemp, PORT_CHUNK, &rxBytesRead, NULL)){
				portShutdown(entry.portHandle);
				entry.status = SPSTAT_IOERROR;
				entry.cooldown = (systime + PORT_COOLDOWN_MS);
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
		if (entry.portHandle) {
			portShutdown(entry.portHandle);
		}
	}
}

/*
    ██   ██████      ██████  ██████  ███████ 
    ██  ██    ██    ██    ██ ██   ██ ██      
    ██  ██    ██    ██    ██ ██████  ███████ 
    ██  ██    ██    ██    ██ ██           ██ 
    ██   ██████      ██████  ██      ███████ ██
*/

bool maddsua::serial::write(uint32_t comport, std::string data) {

	for (auto& entry : pool) {
		if ((entry.port == comport && entry.status == SPSTAT_ACTIVE) || (entry.focus && entry.status == SPSTAT_ACTIVE)) {
			entry.buffTX += data;
			return true;
		}
	}

	return false;
}
bool maddsua::serial::write(portEntry& entry, std::string data) {

	if (entry.status == SPSTAT_ACTIVE) {
		entry.buffTX += data;
		return true;
	}

	return false;
}
std::string maddsua::serial::read(uint32_t comport) {

	for (auto& entry : pool) {
		if ((entry.port == comport && entry.status == SPSTAT_ACTIVE) || (entry.focus && entry.status == SPSTAT_ACTIVE)) {
			auto temp = entry.buffRX;
			entry.buffRX.clear();
			return temp;
		}
	}

	return {};
}
std::string maddsua::serial::read(portEntry& entry) {

	if (entry.status == SPSTAT_ACTIVE) {
		auto temp = entry.buffRX;
		entry.buffRX.clear();
		return temp;
	}

	return {};
}

/*
    ██ ███    ██ ███████  ██████  
    ██ ████   ██ ██      ██    ██ 
    ██ ██ ██  ██ █████   ██    ██ 
    ██ ██  ██ ██ ██      ██    ██ 
    ██ ██   ████ ██       ██████  
*/

maddsua::serial::portEntryInfo maddsua::serial::stats(uint32_t comport) {

	for (auto& entry : pool) {
		if (entry.port == comport) {
			return stats(entry);
		}
	}

	return {};
}

maddsua::serial::portEntryInfo maddsua::serial::stats(portEntry& entry) {

	portEntryInfo temp;
		temp.port = entry.port;
		temp.cooldown = entry.cooldown > 0 ? true : false;
		temp.excluded = entry.excluded;
		temp.dataAvailable = entry.buffRX.size();
		temp.id = entry.deviceID;
		temp.transferRX = entry.transferRX;
		temp.transferTX = entry.transferTX;
		temp.focus = entry.focus;
		temp.status = entry.status;
	
	return temp;
}

std::vector <maddsua::serial::portEntryInfo> maddsua::serial::stats() {

	std::vector <portEntryInfo> result;

	for (auto& entry : pool) {
		result.push_back(stats(entry));
		printf("COM%i:%i:%i\r\n", entry.port, entry.portHandle, entry.focus);
	}

	return result;
}

/*
    ███████  ██████   ██████ ██    ██ ███████ 
    ██      ██    ██ ██      ██    ██ ██      
    █████   ██    ██ ██      ██    ██ ███████ 
    ██      ██    ██ ██      ██    ██      ██ 
    ██       ██████   ██████  ██████  ███████ 

*/

bool maddsua::serial::setFocus(uint32_t comport) {

	clearFocus();

	for (auto& entry : pool) {
		if (entry.port == comport && entry.status == SPSTAT_AVAILABLE) {
			entry.focus = true;
			return true;
		}
	}

	return false;
}

bool maddsua::serial::clearFocus() {

	for (auto& entry : pool) {
		if (entry.focus) {
			portShutdown(entry.portHandle);
			entry.focus = false;
			entry.status = SPSTAT_AVAILABLE;
			entry.cooldown = (timeGetTime() + PORT_CD_FAST_MS);
			return true;
		}
	}

	return false;
}