//  2023 maddsua | https://github.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#include <stdio.h>
#include <windows.h>

#include <vector>
#include <string>
#include <fstream>

#include "app.hpp"


void metalog(const char* input, const char* port, char* result, bool isInput) {

	strcpy(result, port);
	
	if (isInput) {
		strcat(result, " <--- ");
	} else{
		strcat(result, " ---> ");
	}
	
	strcat(result, input);
}
/*
void quickcmd(HWND term, const char* cmd, bool usenl,  const char* port, std::vector <std::string>* datalog, char* comm){
	
	//	display command	
	if (usenl) {

		//	add port info
		char logtmp[comlogbuff];
		metalog(cmd, port, logtmp, true);
							
		//	display and write log
		log(term, logtmp);
		datalog->push_back(logtmp);
	}
						
	//	copy command to send buffer
	strcpy(comm, cmd);
}
*/