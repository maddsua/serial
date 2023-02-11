//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal


#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "app.h"


//	exit codes:
//	0	user exit
//	1	port does not exist
//	2	port busy
//	3	failed to set timeouts
//	4 	failed to read default port settings
//	5	failed to set port settings

int comTalk(uint32_t serialIndex, uint32_t serialSpeed) {

	uint32_t exitcode = 0;

	char serialPath[16];
	memset(serialPath, 0, sizeof (serialPath));
	sprintf(serialPath, "\\\\.\\COM%i", serialIndex);
	
	//	create port handle	(NON OVERLAPPED)
	HANDLE Port = CreateFile(serialPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	//	check if port opened or abort
	if (Port == INVALID_HANDLE_VALUE) {
		DWORD lerr = GetLastError ();

		//	port does not exist
		if (lerr == ERROR_FILE_NOT_FOUND) exitcode = 1;
		//	port busy
		else if (lerr == ERROR_ACCESS_DENIED) exitcode = 2;

		CloseHandle(Port);
		return exitcode;
	}

	//	read-set port timeouts
	COMMTIMEOUTS deftt = {0};
	COMMTIMEOUTS commtt = {0};

		commtt.ReadIntervalTimeout = 1;
		commtt.ReadTotalTimeoutMultiplier = 1;
		commtt.ReadTotalTimeoutConstant = 1;
		commtt.WriteTotalTimeoutMultiplier = 1;
		commtt.WriteTotalTimeoutConstant = 1;

	//	we are not interested in default timeouts, but if program fails to read or set new - abort operation
	if (!GetCommTimeouts(Port, &deftt) || !SetCommTimeouts(Port, &commtt)) {
		exitcode = 3;
		return exitcode;
	}

	// speed and struff settings
	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength = sizeof (dcbSerialParams);
		
	if (!GetCommState(Port, &dcbSerialParams)) {
		exitcode = 4;
		return exitcode;
	}

	dcbSerialParams.BaudRate = serialSpeed;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	if (!SetCommState(Port, &dcbSerialParams)) {	
		exitcode = 5;
		return exitcode;
	}

	//	cycle outside variables
	char sendBuffer [comMsgSendSize];
	char recStrBuffer [tempRecBuffer];
	memset(recStrBuffer, 0, sizeof (recStrBuffer));

	uint32_t flagSend = 0;
	uint32_t flagFullString = 0;		//	not used
	DWORD dwCommEvent = 0;


	//	show some tips
	printf("\n	Connected to COM%i at %i baud. Press Alt key to type in. ESC to exit.\n\n", serialIndex, serialSpeed);


	while(1) {

		//	user input
		if (GetKeyState(VK_MENU) & KF_UP) {

			memset(sendBuffer, 0, sizeof (sendBuffer));

			printf("\nCOM%i << ", serialIndex);
			scanf ("%s", sendBuffer);

			strcat(sendBuffer, "\n");

			flagSend = 1;
		}

		//	user exit
		if ((GetKeyState(VK_ESCAPE) & KF_UP)) {
			exitcode = 0;
			CloseHandle(Port);
			return exitcode;
		}


		//	print whole string
		if (strchr (recStrBuffer, '\n') != NULL || strlen(recStrBuffer) > 128) {
			printf("\nCOM%i >> %s", serialIndex, recStrBuffer);
			memset(recStrBuffer, 0, sizeof (recStrBuffer));
		}

		//	read
		DWORD iSize;
		char receiveBuffer[comMsgReadSize];
		memset(receiveBuffer, 0, sizeof (receiveBuffer));

		if (!ReadFile(Port, &receiveBuffer, comMsgReadSize - 1, &iSize, NULL)) {
			printf("\n\n	! Port read error !\n\n");
			return 1;
		}

		if (iSize > 0) {
			strcat(recStrBuffer, receiveBuffer);
			memset(receiveBuffer, 0, sizeof (receiveBuffer));
		}

		/*		send		*/
		if (flagSend) {
			
			flagSend = 0;
						
			DWORD sendsize = sizeof (char)*strlen(sendBuffer);
			DWORD dwBytesWritten;
				
			if (!WriteFile(Port, sendBuffer, sendsize, &dwBytesWritten, NULL)) {
				if (GetLastError () != ERROR_IO_PENDING) {
					printf("\n\n	! Port write error !\n\n");
					return 1;
				}
			}
		}
	}
			
	CloseHandle(Port);
	return 0;
}

