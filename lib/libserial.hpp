#ifndef __LIB_MADDSUA_SERIAL__
#define __LIB_MADDSUA_SERIAL__

#include <string>
#include <vector>
#include <stdint.h>
//#include <thread>

namespace Serial {

	enum PortStatus {
		PORTSTAT_OK = 0,
		PORTSTAT_NOT_CONNECTED = -1,
		PORTSTAT_PORT_ERROR = -2,
		PORTSTAT_SETT_ERR = -3,
	};

	class Port {
		private:
			void* hPort = nullptr;
			uint32_t portSpeed = 9600;
			uint16_t portidx = 0;
			PortStatus portStatus = PORTSTAT_NOT_CONNECTED;
			int64_t apiError = 0;
			size_t transferTX = 0;
			size_t transferRX = 0;
			size_t dataAvailable = 0;
			std::vector<uint8_t> bufferRx;

		public:
			Port(uint16_t port);
			~Port();

			uint16_t getPortIdx();

			bool setSpeed(uint32_t bSpeed);

			bool available();
			std::vector<uint8_t> read();
			bool write(std::vector<uint8_t>& data);
	};

};

#endif
