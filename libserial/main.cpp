#include <iostream>
#include <windows.h>

#include "serial.hpp"



int main() {

	auto com = new maddsua::serial(8);

	//maddsua::serial com;
		//com.setSpeed(9600);

	int helloCount = 0;

	while (true) {

		auto incoming = com->dataAvail();

		for (auto idx : incoming) {
		
			auto msg = com->read(idx);
			std::cout << msg << std::endl;

			if (msg.find("Hello") != std::string::npos) helloCount++;

			if (helloCount > 2) {

				helloCount = 0;
				com->write(idx, "ping\r\n");
				
				auto stats = com->stats(idx);
				std::cout << "Data transfered: " << (stats.transferTX + stats.transferRX) << std::endl;
			}
		}
		Sleep(10);
	}
	
	return 0;
}