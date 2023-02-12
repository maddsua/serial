//	2023 maddsua's serial lib
//	v1.0.0
//	https://github.com/maddsua

#include <iostream>
#include <windows.h>

#include "serial.hpp"


int main() {

	auto serial = new maddsua::serial(8, true);
	serial->setSpeed(9600);

/*	int helloCount = 0;

	while (true) {

		auto stats = serial->stats();

		for (auto entry : stats) {

			if (idx.dataAvailable) {

				auto msg = serial->read(entry.port);
				std::cout << msg << std::endl;

				if (msg.find("Hello") != std::string::npos) helloCount++;

				if (helloCount > 2) {

					helloCount = 0;
					serial->write(entry.port, "ping\r\n");
					
					auto stats = serial->stats(entry.port);
					std::cout << "Data transfered: " << (stats.transferTX + stats.transferRX) << std::endl;
				}
			}
		
		}
		Sleep(1000);
	}*/

	while (true) {

		auto stats = serial->stats();

		for (auto entry : stats) {
			//std::cout << entry.status << std::endl;
			if (entry.port == 3 && !entry.focus) entry.focus = true;

			if (entry.focus) {
				if (entry.dataAvailable) {
					std::cout << serial->read(entry.port) << std::endl;
				}
			}
		}

		Sleep(1000);
	}
	
	return 0;
}