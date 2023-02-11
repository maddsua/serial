//  2023 maddsua | https://github.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal


#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "app.h"


uint32_t scanPorts(unsigned short int* splsarray);
uint32_t isInputPort(char* input);
uint32_t isInputSpeed(char* input);


const uint32_t serialSpeeds[] = {
	110, 300, 600, 1200, 2400, 4800, 9600,
	14400, 19200, 38400, 56000, 57600,
	115200, 128000, 256000
};


int main(int argc, char** argv) {
	
	printf("\n\t%s v%s\n\tUse help for more\n", APP_DESC, APP_VERSION);
	
	uint32_t thisPort = 9;
	uint32_t ThisPortSpeed = 9600;
	
	unsigned short int comPool[scanSerialPorts];
	uint32_t activePorts = scanPorts(comPool);
	
	char userInput [userinputBuffer];
	
	while(1) {
		
		memset(userInput, 0, sizeof (userInput));
		printf("\nSERTER> ");
		scanf ("%s", userInput);
				
		if (strstr(userInput, "connect") != NULL || strstr(userInput, "run") != NULL) {
			

				printf("\n Connecting...\n\n%s\n", hrline);
				uint32_t connstat = comTalk(thisPort, ThisPortSpeed);


				switch (connstat) {
					case 1:
						printf("\n Unable to connect: Port is not connected. (Error %i)\n", connstat);
					break;
					
					case 2:
						printf("\n Unable to connect: Port is busy. (Error %i)\n", connstat);
					break;

					case 3:
						printf("\n Unable to connect: Failed to set timeouts. (Error %i)\n", connstat);
					break;

					case 4:
						printf("\n Unable to connect: Failed to read port settings. (Error %i)\n", connstat);
					break;

					case 5:
						printf("\n Unable to connect: Failed to set port settings. (Error %i)\n", connstat);
					break;
				
					default:
						printf("\n Session terminated by user\n\n%s\n", hrline);
					break;
				}
			
		} else if (strstr(userInput, "selcom") != NULL || strstr(userInput, "port") != NULL) {
			
			activePorts = scanPorts(comPool);
			
			printf("\n Available ports:\n\n");
			
			unsigned short int nl = 0;
			for (int i = 0; i < activePorts; i++) {
				
				printf("\tCOM%i", comPool[i]);
				nl++;
				
				if (nl > 4) {
					nl = 0;
					printf("\n");
				}
			}
			
				printf("\n\nSERTER ? COM> ");
				scanf ("%s", userInput);
				
				uint32_t newPortIndex = isInputPort(userInput);
				if (newPortIndex) {
					
					uint32_t portListMatch = 0;
					for (short i = 0; i < scanSerialPorts; i++) {
						if (comPool[i] == newPortIndex) {
							portListMatch = 1;
							break;	
						}
					}
					
					if (portListMatch) {
						thisPort = newPortIndex;
						printf("\nCOM%i / %i baud selected\n", thisPort, ThisPortSpeed);

					} else printf("\n This port is not active now\n");
					
				} else printf("\n Cannot set this as a serial port\n");
			
		}
		else if (strstr(userInput, "sspeed") != NULL || strstr(userInput, "baud") != NULL) {
			
			printf("\n Set port speed:\n\n\t");
			
			unsigned short int nl = 0;
			for (int i = 0; i < (sizeof(serialSpeeds) / sizeof(uint32_t)); i++) {
				
				
				printf("  %i", serialSpeeds[i]);
				nl++;
				
				if (i < ((sizeof(serialSpeeds) / sizeof(uint32_t)) - 1)) 
					printf("  |");
				
				if (nl > 4) {
					nl = 0;
					printf("\n\t");
				}
			}
			
			printf("\nSERTER ? SPEED> ");
			scanf ("%s", userInput);
			
			uint32_t newPortSpeed = isInputSpeed(userInput);
			
			if (newPortSpeed) {
				ThisPortSpeed = newPortSpeed;
				printf("\nCOM%i / %i baud selected\n", thisPort, ThisPortSpeed);

			} else printf("\n Cannot set this as a serial speed\n");
			
		} else if (strstr(userInput, "help") != NULL) {
			printf("\n%s\n", msg_hlp);

		} else if (strstr(userInput, "cfg") != NULL) {
			printf("\n	COM%i / %i baud\n", thisPort, ThisPortSpeed);

		} else if (strstr(userInput, "exit") != NULL) {
			return 0;

		} else printf("\n Invalid command\n");
		
	}

	return 0;
}

uint32_t isInputPort(char* input) {
	
	if (strlen(input) > 6 || strlen(input) < 4) {
		return 0;
	}
	
	char serPID[3] = {0};
	strcpy(serPID, input + 3);

	uint32_t serIndex = atoi(serPID);
	
	if (serIndex > scanSerialPorts) return 0;
	
	return serIndex;
}
uint32_t isInputSpeed(char* input) {
	
	char portSpeed[8] = {0};
	strncpy(portSpeed, input, 8);
		
	uint32_t numPortSpeed = atoi(portSpeed);
	
	uint32_t speedMatch = 0;
	for (short i = 0; i < (sizeof(serialSpeeds) / sizeof(uint32_t)); i++) {
		if (serialSpeeds[i] == numPortSpeed) {
			speedMatch = 1;
			break;	
		}
	}	
	
	if (speedMatch) return numPortSpeed;
	
	return 0;
}
uint32_t scanPorts(unsigned short int* splsarray) {
	
	uint32_t splsarrayUtil = 0;
		
	for (int scancom = 1; scancom < scanSerialPorts; scancom++) {

		char compath[size_serialPortPath];
			sprintf(compath, "\\\\.\\COM%i", scancom);
		
		HANDLE Port  = CreateFile(compath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		
		if (Port != INVALID_HANDLE_VALUE) {
			splsarray[splsarrayUtil] = scancom;
			splsarrayUtil++;
		}
		
		CloseHandle(Port);
	}
	
	return splsarrayUtil;
}
