#include "./libserial.hpp"
#include <windows.h>

std::vector<Serial::SerialDevice> Serial::devices() {

	std::vector<SerialDevice> result;

	char portPath[32];
	char devPath[UINT8_MAX];

	for (uint16_t i = 0; i < UINT8_MAX; i++) {
		snprintf(portPath, sizeof(portPath), "COM%i", i);
		auto portResult = QueryDosDeviceA(portPath, devPath, sizeof(devPath));
		if (!portResult) continue;
		result.push_back({ i, devPath });
	}
	
	return std::move(result);
}
