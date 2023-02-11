//  2023 maddsua | https://github.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal

#ifndef SERIALTERMINAL_PRIVATE_H
#define SERIALTERMINAL_PRIVATE_H


#include <stdint.h>


	#define APP_VERSION		"1.0.5"
	#define APP_DESC		"Serial Terminal CLI"


	/*		internals		*/
	#define serilPortIndexLenn		2
	#define userinputBuffer			128
	#define ascinumbShift			48
	#define hrline					"----------------"


	/*		serial IO		*/
	#define tempRecBuffer			512
	#define comSyncDelayMs			1
	#define comMsgReadSize			32
	#define comMsgSendSize			(userinputBuffer)
	#define scanSerialPorts			32
	#define comPortNamePrefix		"COM"
	#define comPortPathPrefix		"\\\\.\\"
	#define size_serialPortName		8
	#define size_serialPortPath		16
	#define serialSpeedsTotal		15

	#define msg_hlp					" https://github.com/maddsua\r\n \r\n Commands:\r\n\r\n\thelp\t\t->\tThis command\r\n\t\r\n\tconnect, run\t->\tConnect to the serial port\r\n\tselcom, port\t->\tSelect serial (COM) port\r\n\tsspeed, baud\t->\tSelect serial speed\r\n\t\r\n\tconfig, cfg\t->\tShow connection config\r\n\r\n\texit\t\t->\t?????????"


	int comTalk(unsigned int serialIndex, unsigned int serialSpeed);


#endif
