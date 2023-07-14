#include <iostream>
#include <windows.h>

#include "./lib/libserial.hpp"

int main(int argc, char const *argv[])
{

	auto port = Serial::Port(5);

	std::cout << "Port status: " << port.status() << std::endl;

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		auto temp = port.read();
		if (!temp.size()) continue;
		std::cout << std::string(temp.begin(), temp.end()) << std::endl;
	}

	return 0;
}
