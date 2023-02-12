//	2023 maddsua's serial lib
//	v1.0.0
//	https://github.com/maddsua

#ifndef _maddsuaseriallib
#define _maddsuaseriallib

#include <stdint.h>
#include <windows.h>
#include <string>
#include <array>
#include <vector>
#include <thread>
#include <mutex>


#define PORTS_COMSMAX		(256)
#define PORT_FIRST			(1)
#define PORT_COOLDOWN_MS	(3000)
#define PORT_CD_FAST_MS		(1000)
#define PORT_TEXT_WAIT		(300)
#define PORT_CHUNK			(64)

#define WINERR_DEV_NOTFOUND	(433)

#define SPSTAT_DISABLED		(5)
#define SPSTAT_IGNORED		(4)
#define SPSTAT_BUSY			(3)
#define SPSTAT_ACTIVE		(2)
#define SPSTAT_AVAILABLE	(1)
#define SPSTAT_DISCONN		(0)
#define SPSTAT_IOERROR		(-1)
#define SPSTAT_SETPERR		(-2)

namespace maddsua {

	class serial {

		public:
			struct portEntry {
				int32_t port = 0;
				int32_t status = SPSTAT_DISCONN;

				size_t transferTX = 0;
				size_t transferRX = 0;
				time_t cooldown = 0;
				time_t linePending = 0;

				bool excluded = false;
				bool focus = false;

				HANDLE portHandle = nullptr;

				std::string buffTX;
				std::string buffRX;
				std::string buffRXTemp;
				std::string deviceID;
			};

			struct portEntryInfo {
				uint32_t port = 0;
				int32_t status = SPSTAT_DISCONN;

				size_t transferTX = 0;
				size_t transferRX = 0;
				size_t dataAvailable = 0;

				bool excluded = false;
				bool cooldown = false;
				bool focus = false;

				std::string id;
			};

			serial(uint32_t maxPorts, bool parallel);
			~serial();

			std::vector <uint32_t> getSpeeds();
			bool setSpeed(uint32_t baudrate);

			std::vector <portEntryInfo> stats();
			portEntryInfo stats(uint32_t comport);

			bool write(uint32_t comport, std::string data);
			bool write(portEntry& entry, std::string data);
			std::string read(uint32_t comport);
			std::string read(portEntry& entry);

			bool setFocus(uint32_t comport);
			bool clearFocus();

		private:
			int serialSpeed;
			int activatePorts;
			bool textmode;
			bool parallelOps;
			bool running;
			void ioloop();
			std::thread daemon;
			std::vector <portEntry> pool;
			portEntryInfo stats(portEntry& entry);
	};

}

#endif