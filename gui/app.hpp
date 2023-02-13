//  2023 maddsua | https://github.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#ifndef _SERIALTERMINAL
#define _SERIALTERMINAL

	#include <stdint.h>
	#include <vector>
	#include <string>
	#include <windows.h>

	#include "rescodes.hpp"

	#define APP_NAME		"Serial Terminal"
	#define APP_VERSION		"4.0.0"
	#define APP_DESC		"A serial port communication utility"
	#define VER_AUTHSTAMP	"2023 maddsua"
	#define APP_COPYRIGHT	"https://github.com/maddsua"

	/*		internals		*/
	#define windowSizeX				640
	#define windowSizeY				480
	//#define userinputBuffer		128
	//#define ascinumbShift			48
	//#define TTOUT					10
	//#define NNLTTOUT				50
	#define comlongmsg				128
	#define commsgbuff				512
	#define comlogbuff				640
	#define atcomlen				32
	//#define CYCLE_PRINT				666

	/*		serial IO		*/
	#define SIO_DEFAULT_SPEED		(9600)

	/*#define comSyncDelayMs			1
	#define comMsgReadSize			32
	#define comMsgSendSize			(userinputBuffer)
	#define scanSerialPorts			64
	#define comPortNamePrefix		"COM"
	#define comPortPathPrefix		"\\\\.\\"
	#define portNameLen				8
	#define portPathLen				16
	#define defSerialSpeed			6*/


	//void serialIO(char* port, unsigned int speed, char* bufferIn, char* bufferOut, int* commstat, bool placeNewLine);

	//void log(HWND hEdit, LPCSTR newText);
	//void quickcmd(HWND term, const char* cmd, bool usenl,  const char* port, std::vector <std::string>* datalog, char* comm);
	void metalog(const char* input, const char* port, char* result, bool isInput);
	//void dropdown(HWND combo, char** items, unsigned int length, unsigned int focus, bool erase);
	//void dropdown(HWND combo, std::vector <std::string>* items, size_t focus, bool erase);
	
	//bool SaveLogFile(std::vector <std::string>* commlog, char* filepath);
	
	//unsigned int scanPorts(std::vector <std::string>* items);


#endif
