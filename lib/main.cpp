//	2023 maddsua's serial lib
//	v1.0.0
//	https://github.com/maddsua

#include <iostream>
#include <windows.h>

#include "serial.hpp"


int main() {

	auto com = new maddsua::serial(8, true);
	com->setSpeed(9600);

	int helloCount = 0;

	while (true) {

		auto incoming = com->stats();

		for (auto idx : incoming) {

			if (idx.dataAvailable) {

				auto msg = com->read(idx.port);
				std::cout << msg << std::endl;

				if (msg.find("Hello") != std::string::npos) helloCount++;

				if (helloCount > 2) {

					helloCount = 0;
					com->write(idx.port, "ping\r\n");
					
					auto stats = com->stats(idx.port);
					std::cout << "Data transfered: " << (stats.transferTX + stats.transferRX) << std::endl;
				}
			}
		
		}
		Sleep(1000);
	}
	
	return 0;
}