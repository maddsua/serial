#ifndef __LIB_MADDSUA_SERIAL__
#define __LIB_MADDSUA_SERIAL__

#include <string>
#include <vector>
#include <stdint.h>
#include <mutex>
#include <thread>

namespace Serial {

	struct SerialDevice {
		uint16_t port;
		std::string devpath;
	};

	std::vector<SerialDevice> devices();

	enum PortStatus {
		PORTSTAT_OK = 0,
		PORTSTAT_NOT_CONNECTED = -1,
		PORTSTAT_PORT_ERROR = -2,
		PORTSTAT_SETT_ERR = -3,
		PORTSTAT_READ_ERR = -4,
		PORTSTAT_WRITE_ERR = -5,
	};

	struct PortStats {
		size_t transferTX = 0;
		size_t transferRX = 0;
	};

	class Port {
		private:
			std::mutex threadLock;
			std::thread* asyncReader = nullptr;
			void reader();

			void* hPort = nullptr;
			uint32_t portSpeed = 9600;
			uint16_t portidx = 0;

			PortStatus portStatus = PORTSTAT_NOT_CONNECTED;
			int64_t apiError = 0;

			std::vector<uint8_t> bufferRx;

			PortStats portStats;

		public:
			Port(uint16_t port);
			~Port();

			PortStatus status();
			PortStats stats();
			bool connected();

			uint16_t getPortIdx();

			bool setSpeed(uint32_t bSpeed);

			bool available();
			std::vector<uint8_t> read();
			bool write(std::vector<uint8_t>& data);
	};

	std::vector<uint32_t> speeds();

};

#endif
