//  2023 maddsua | https://github.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)

#include <windows.h>
#include <stdio.h>

#include <thread>
#include <mutex>

#include "app.hpp"

std::mutex iolock;

//	stats:	0 connected	1 stopped	2 disconnected	3 busy	4 timeouts error	5 sercfg error	6 io error
void serialIO(char* port, unsigned int speed, char* bufferIn, char* bufferOut, int* commstat, bool placeNewLine) {
	
	char serialPath[16] = {0};
		strcpy(serialPath, "\\\\.\\");
		strcat(serialPath, port);
	
	//	create port handle	(NON OVERLAPPED)
	HANDLE Port = CreateFileA(serialPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	
	
	//	check if port opened or abort
	if (Port == INVALID_HANDLE_VALUE) {
		
		DWORD lerr = GetLastError();
			
		//	port does not exist
		if (lerr == ERROR_FILE_NOT_FOUND) {
			*commstat = 2;
		}
		//	port busy
		else if (lerr == ERROR_ACCESS_DENIED) {
			*commstat = 3;
		}
		
		CloseHandle(Port);
		return;
	}
	
	
	COMMTIMEOUTS commtt = {0};
	
		//	get serial port timeouts
		if (!GetCommTimeouts(Port, &commtt)) {
		
			*commstat = 4;
		
			CloseHandle(Port);
			return;
		}
	
	//	set new timeouts in ms
	commtt.ReadIntervalTimeout = 1;
	commtt.ReadTotalTimeoutMultiplier = 1;
	commtt.ReadTotalTimeoutConstant = 1;
	commtt.WriteTotalTimeoutMultiplier = 1;
	commtt.WriteTotalTimeoutConstant = 1;

		//	set serial port timeouts
		if (!SetCommTimeouts(Port, &commtt)) {
		
			*commstat = 4;
			
			CloseHandle(Port);
			return;
		}
	
	
	// speed and struff settings
	DCB sercfg = {0};
	sercfg.DCBlength = sizeof(sercfg);
		
		if (!GetCommState(Port, &sercfg)) {
				
			*commstat = 5;
			
			CloseHandle(Port);
			return;
		}
			
	sercfg.BaudRate = speed;
	sercfg.ByteSize = 8;
	sercfg.StopBits = ONESTOPBIT;
	sercfg.Parity = NOPARITY;
		
		if (!SetCommState(Port, &sercfg)) {
				
			*commstat = 5;
			
			CloseHandle(Port);
			return;
		}


	char recStrBuffer[commsgbuff];
		memset(recStrBuffer, 0, sizeof(recStrBuffer));
		
	unsigned long poll = 0;

	while (!*commstat) {

		/*			print whole string			*/
		if (strlen(recStrBuffer) > 0) {
			
			if (strchr(recStrBuffer, '\n') != NULL || strlen(recStrBuffer) > comlongmsg || poll > NNLTTOUT) {
				
				std::lock_guard <std::mutex> guard(iolock);
					strcat(bufferIn, recStrBuffer);
					memset(recStrBuffer, 0, sizeof(recStrBuffer));
				
				poll = 0;
			}
			
			poll++;
		}
		
		/*		read		*/
			DWORD iSize;
			char receiveBuffer[comMsgReadSize];
			memset(receiveBuffer, 0, sizeof(receiveBuffer));
			
			if (!ReadFile(Port, &receiveBuffer, comMsgReadSize - 1, &iSize, NULL)) {
					
				*commstat = 6;
				
				CloseHandle(Port);
				return;
			}
			
			if (iSize > 0) {

				if (placeNewLine) {
					strcat(recStrBuffer, receiveBuffer);

				} else {
					
					std::lock_guard <std::mutex> guard(iolock);
						strcat(bufferIn, receiveBuffer);
				}

				memset(receiveBuffer, 0, sizeof(receiveBuffer));
			}

		/*		send		*/
		if (strlen(bufferOut) > 0) {
			
			DWORD sendsize = sizeof(char)*strlen(bufferOut);
			DWORD dwBytesWritten;
			
			std::lock_guard <std::mutex> guard(iolock);
				
				if (!WriteFile(Port, bufferOut, sendsize, &dwBytesWritten, NULL)) {
					
					if (GetLastError() != ERROR_IO_PENDING) {
						
						*commstat = 6;
						
						CloseHandle(Port);
						return;
					}
				}
				memset(bufferOut, 0, sizeof(bufferOut));
		}			
		
	}

	CloseHandle(Port);
	return;
}
