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

#define SPSTAT_DISABLED	(5)
#define SPSTAT_IGNORED	(4)
#define SPSTAT_BUSY		(3)
#define SPSTAT_ACTIVE		(2)
#define SPSTAT_AVAILABLE	(1)
#define SPSTAT_DISCONN	(0)
#define SPSTAT_IOERROR	(-1)
#define SPSTAT_SETPERR	(-2)

namespace maddsua {

	class serial {

		public:
			struct portEntry {
				bool excluded = false;
				bool focus = false;
				int portIndex = 0;
				int status = SPSTAT_DISCONN;

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
				int comport = 0;
				bool excluded = false;
				bool cooldown = false;
				std::string status;
				std::string id;
			};

			struct portAttribs {
				bool enabled = true;
				bool ignored = false;
			};

			serial(int maxPorts, bool parallel) {
				running = true;
				textmode = true;
				parallelOps = parallel;
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
			std::vector <int> getSpeeds();

			std::vector <int> dataAvail();

			std::vector <readablePortEntry> stats();
			readablePortEntry stats(uint32_t comport);
			std::vector <int> portsActive();
			std::vector <int> portsFree();
			std::vector <int> portsAvailable();

			bool setPortState(uint32_t comport, portAttribs attribs);

			bool write(uint32_t comport, std::string data);
			std::string read(uint32_t comport);

			bool setFocus(uint32_t comport);
			bool clearFocus();

			bool write(std::string data);
			std::string read();

		private:
			int serialSpeed;
			int activatePorts;
			bool textmode;
			bool parallelOps;
			bool running;
			void ioloop();
			std::thread daemon;
			std::vector <portEntry> pool;
			readablePortEntry stats(portEntry& entry);
	};

}

#endif