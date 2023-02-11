#ifndef _maddsuaseriallib
#define _maddsuaseriallib

#include <stdint.h>
#include <windows.h>
#include <string>
#include <array>
#include <vector>
#include <thread>

#include <iostream>


#define PORTS_COMSMAX		(256)
#define PORT_FIRST			(1)
#define PORT_COOLDOWN_MS	(3000)
#define PORT_TEXT_WAIT		(300)
#define PORT_CHUNK			(64)

#define WINERR_DEV_NOTFOUND	(433)

#define PORTSTAT_DISABLED	(4)
#define PORTSTAT_IGNORED	(3)
#define PORTSTAT_BUSY		(2)
#define PORTSTAT_ACTIVE		(1)
#define PORTSTAT_DISCONN	(0)
#define PORTSTAT_IOERROR	(-1)
#define PORTSTAT_SETPERR	(-2)

namespace maddsua {

	class serial {

		public:
			struct portEntry {
				bool active = false;
				bool excluded = false;
				uint16_t portIndex = 0;
				int16_t status = PORTSTAT_DISCONN;

				time_t cooldown = 0;
				time_t linePending = 0;
				size_t transferTX = 0;
				size_t transferRX = 0;

				HANDLE portHandle = nullptr;

				std::string buffTX;
				std::string buffRX;
				std::string buffRXTemp;
				std::string deviceID;
			};

			struct readablePortEntry {
				size_t transferTX = 0;
				size_t transferRX = 0;
				size_t dataAvailable = 0;
				uint16_t comport = 0;
				bool excluded = false;
				bool cooldown = false;
				std::string status;
				std::string id;
			};

			struct portAttribs {
				bool enabled = true;
				bool ignored = false;
			};

			serial(uint16_t maxPorts) {
				running = true;
				textmode = true;
				serialSpeed = 9600;
				activatePorts = (maxPorts < PORTS_COMSMAX) ? maxPorts : PORTS_COMSMAX;

				//	create port port entries
				for (size_t i = 0; i < activatePorts; i++) {
					portEntry temp;
						temp.portIndex = (i + PORT_FIRST);
					pool.push_back(std::move(temp));
				}

				daemon = std::thread(ioloop, this);
			}
			~serial() {
				running = false;
				if (daemon.joinable()) daemon.join();
			}

			bool setSpeed(uint32_t baudrate);

			std::vector <uint16_t> dataAvail();

			std::vector <readablePortEntry> stats();
			readablePortEntry stats(uint16_t comport);

			bool setPortState(uint16_t comport, portAttribs attribs);

			bool write(uint16_t comport, std::string data);
			std::string read(uint16_t comport);

		private:
			uint32_t serialSpeed;
			uint16_t activatePorts;
			bool textmode;
			bool running;
			void ioloop();
			std::thread daemon;
			std::vector <portEntry> pool;
			readablePortEntry stats(portEntry& entry);
	};

}

#endif