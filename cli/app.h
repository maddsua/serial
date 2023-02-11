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

#define msg_title				""
#define msg_hlp					" https://gitlab.com/maddsua \n\n Commands:\n\n	help		->	This command\n	\n	connect, run	->	Connect to the serial port\n	selcom, port	->	Select serial (COM) port\n	sspeed, baud	->	Select serial speed\n	\n	config, cfg	->	Show connection config\n\n	exit		->	?????????"


int comTalk(unsigned int serialIndex, unsigned int serialSpeed);


#endif
