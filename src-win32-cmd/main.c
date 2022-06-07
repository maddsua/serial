//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal


#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "SerialTerminal_private.h"
#include "com/staticConfig.h"
#include "com/serialio.h"



unsigned int scanPorts(unsigned short int* splsarray);
unsigned int isInputPort(char* input);
unsigned int isInputSpeed(char* input);


unsigned int serialSpeeds[serialSpeedsTotal] = {
		
	110,
	300,
	600,
	1200,
	2400,
	4800,
	9600,
	14400,
	19200,
	38400,
	56000,
	57600,
	115200,
	128000,
	256000
	
};


int main(int argc, char** argv) {
	
	printf("\n\t%s v%i.%i.%i\n\tUse help for more\n", FILE_DESCRIPTION, VER_MAJOR, VER_MINOR, VER_RELEASE);
	
	unsigned int thisPort = 9;
	unsigned int ThisPortSpeed = 9600;
	
	unsigned short int comPool[scanSerialPorts];
	unsigned int activePorts = scanPorts(comPool);
	
	char userInput [userinputBuffer];
	
while(1){
	
	memset(userInput, 0, sizeof(userInput));
	printf("\nSERTER> ");
	scanf("%s", userInput);
	
	
	
	if(strstr(userInput, "connect") != NULL || strstr(userInput, "run") != NULL){
		
//		printf("\nConnect to COM%i / %i?	yes | no\n", thisPort, ThisPortSpeed);
		
//		printf("\nSERTER ? CONFIRM> ");
///		scanf("%s", userInput);
		
//		if(strstr(userInput, "yes") != NULL){

			printf("\n Connecting...\n\n%s\n", hrline);
			unsigned int connstat = comTalk(thisPort, ThisPortSpeed);
		
			if(connstat == 1){
				printf("\n Unable to connect: Port is not connected. (Error %i)\n", connstat);
			}
			else if(connstat == 2){
				printf("\n Unable to connect: Port is busy. (Error %i)\n", connstat);
			}
			else if(connstat == 3){
				printf("\n Unable to connect: Failed to set timeouts. (Error %i)\n", connstat);
			}
			else if(connstat == 4){
				printf("\n Unable to connect: Failed to read port settings. (Error %i)\n", connstat);
			}
			else if(connstat == 5){
				printf("\n Unable to connect: Failed to set port settings. (Error %i)\n", connstat);
			}
			else{
				printf("\n Session terminated by user\n\n%s\n", hrline);
			}
//		}
		
	}
	else if(strstr(userInput, "selcom") != NULL || strstr(userInput, "port") != NULL){
		
		activePorts = scanPorts(comPool);
		
		printf("\n Available ports:\n\n");
		
		unsigned short int nl = 0;
		for(int i = 0; i < activePorts; i++){
			
			printf("	COM%i", comPool[i]);
			nl++;
			
				if(nl > 4){
					nl = 0;
					printf("\n");
				}
		}
		
			printf("\n\nSERTER ? COM> ");
			scanf("%s", userInput);
			
			unsigned int newPortIndex = isInputPort(userInput);
			if(newPortIndex){
				
				unsigned int portListMatch = 0;
				for(short i = 0; i < scanSerialPorts; i++){
					if(comPool[i] == newPortIndex){
						portListMatch = 1;
						break;	
					}
				}
				
				if(portListMatch){
					thisPort = newPortIndex;
					printf("\nCOM%i / %i baud selected\n", thisPort, ThisPortSpeed);
				}
				else{
					printf("\n This port is not active now\n");
				}
				
			}
			else{
				printf("\n Cannot set this as a serial port\n");
			}
		
	}
	else if(strstr(userInput, "sspeed") != NULL || strstr(userInput, "baud") != NULL){
		
		printf("\n Set port speed:\n\n\t");
		
		unsigned short int nl = 0;
		for(int i = 0; i < serialSpeedsTotal; i++){
			
			
			printf("  %i", serialSpeeds[i]);
			nl++;
			
				if(i < (serialSpeedsTotal - 1)){
					printf("  |");
				}
				
					if(nl > 4){
						nl = 0;
						printf("\n\t");
					}
		}
		
		printf("\nSERTER ? SPEED> ");
		scanf("%s", userInput);
		
		unsigned int newPortSpeed = isInputSpeed(userInput);
		
		if(newPortSpeed){
			ThisPortSpeed = newPortSpeed;
			printf("\nCOM%i / %i baud selected\n", thisPort, ThisPortSpeed);
		}
		else{
				printf("\n Cannot set this as a serial speed\n");
		}
		
	}
	else if(strstr(userInput, "help") != NULL){
		printf("\n%s\n", msg_hlp);
	}
	else if(strstr(userInput, "cfg") != NULL){
		printf("\n	COM%i / %i baud\n", thisPort, ThisPortSpeed);
	}
	else if(strstr(userInput, "exit") != NULL){
		return 0;
	}
	else{
		printf("\n Invalid command\n");
	}
	
}	//	loop end
	return 0;
}




unsigned int isInputPort(char* input){
	
	if(strlen(input) > 6 || strlen(input) < 4){
		return 0;
	}
	
	char serPID[3] = {0};
	strcpy(serPID, input + 3);

	unsigned int serIndex = atoi(serPID);
	
	if(serIndex > scanSerialPorts){
		return 0;
	}
	
	return serIndex;
}


unsigned int isInputSpeed(char* input){
	
	char portSpeed[8] = {0};
		strncpy(portSpeed, input, 8);
		
	unsigned int numPortSpeed = atoi(portSpeed);
	
	unsigned int speedMatch = 0;
		for(short i = 0; i < serialSpeedsTotal; i++){
			if(serialSpeeds[i] == numPortSpeed){
				speedMatch = 1;
				break;	
			}
		}	
	
	if(speedMatch){
		return numPortSpeed;
	}
	
	return 0;
}





unsigned int scanPorts(unsigned short int* splsarray){
	
	unsigned int splsarrayUtil = 0;
		
	for(int scancom = 1; scancom < scanSerialPorts; scancom++){

		char compath[size_serialPortPath];
			sprintf(compath, "\\\\.\\COM%i", scancom);
		
		HANDLE Port  = CreateFile(compath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		
		if(Port != INVALID_HANDLE_VALUE){
			
			splsarray[splsarrayUtil] = scancom;
			splsarrayUtil++;
		}
		
		CloseHandle(Port);
	}
	
	return splsarrayUtil;
}




