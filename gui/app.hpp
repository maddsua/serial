//  2023 maddsua | https://github.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#ifndef _SERIALTERMINAL
#define _SERIALTERMINAL

	#include <vector>
	#include <string>
	#include <windows.h>

	#include "rescodes.hpp"

	#define APP_NAME		"Serial Terminal"
	#define APP_VERSION		"2.0.1"
	#define APP_DESC		"Serial Terminal (COM monitor)"
	#define VER_AUTHSTAMP	"2023 maddsua"
	#define APP_COPYRIGHT	"https://github.com/maddsua"

	/*		internals		*/
	#define windowSizeX				640
	#define windowSizeY				480
	#define userinputBuffer			128
	#define ascinumbShift			48
	#define TTOUT					10
	#define NNLTTOUT				50
	#define comlongmsg				128
	#define commsgbuff				512
	#define comlogbuff				640
	#define atcomlen				32
	#define CYCLE_PRINT				666
	#define winapi_textedit_OVF		28000

	/*		GUI		*/
	#define GUI_LOGWIN				11
	#define GUI_COMPROM				12
	#define GUI_COMBO_PORT			13
	#define GUI_COMBO_SPEED			14
	#define GUI_BTN_SEND			15
	#define GUI_BTN_CLR				16
	#define GUI_BTN_UPD				17
	#define GUI_CHK_NLN				18
	#define GUI_CHK_QKAT			19

	#define GUI_AT_PREF				20
	#define GUI_AT_AT				21
	#define GUI_AT_OK				22
	#define GUI_AT_ID				23


	/*		serial IO		*/
	#define comSyncDelayMs			1
	#define comMsgReadSize			32
	#define comMsgSendSize			(userinputBuffer)
	#define scanSerialPorts			64
	#define comPortNamePrefix		"COM"
	#define comPortPathPrefix		"\\\\.\\"
	#define portNameLen				8
	#define portPathLen				16
	#define defSerialSpeed			6


	void serialIO(char* port, unsigned int speed, char* bufferIn, char* bufferOut, int* commstat, bool placeNewLine);

	void log(HWND hEdit, LPCSTR newText);
	void quickcmd(HWND term, const char* cmd, bool usenl,  const char* port, std::vector <std::string>* datalog, char* comm);
	void metalog(const char* input, const char* port, char* result, bool isInput);
	//void dropdown(HWND combo, char** items, unsigned int length, unsigned int focus, bool erase);
	void dropdown(HWND combo, std::vector <std::string>* items, size_t focus, bool erase);
	
	bool SaveLogFile(std::vector <std::string>* commlog, char* filepath);
	
	unsigned int scanPorts(std::vector <std::string>* items);
	
	char*** create3d(unsigned int dim_1, unsigned int dim_2, unsigned int dim_3);
	char** create2d(unsigned int dim_1, unsigned int dim_2);
	void clear3d(char*** array, unsigned int dim_1, unsigned int dim_2);
	void clear2d(char** array, unsigned int dim_1);


#endif
