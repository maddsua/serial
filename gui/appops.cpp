//  2023 maddsua | https://github.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#include <stdio.h>
#include <windows.h>

#include <vector>
#include <string>
#include <fstream>

#include "app.hpp"

bool SaveLogFile(std::vector <std::string>* commlog, char* filepath){
	
	std::ofstream localFile(filepath, std::ios::out);
	
	if (localFile.is_open()) {
		
		for (size_t i = 0; i < commlog->size(); i++)
			localFile << commlog->at(i);
			
		localFile.close();

	} else return false;
	
	return true;
}


void log(HWND hEdit, LPCSTR newText){

	//	get contents length
	unsigned int txtcontlen = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
	
	//	do some work if text fiekd is too big
	if(txtcontlen > winapi_textedit_OVF){
		
		//	erase first half of text field
		SendMessage(hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)(winapi_textedit_OVF / 2));	//	select 1/2 of text
		SendMessage(hEdit, EM_REPLACESEL, FALSE, 0);							// erase it
		
		//	get contents length again
		txtcontlen = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
	}
	
	//	append new text
	SendMessage(hEdit, EM_SETSEL, (WPARAM)txtcontlen, (LPARAM)txtcontlen);
	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)newText);
}

void metalog(const char* input, const char* port, char* result, bool isInput){

	strcpy(result, port);
	
	if (isInput) {
		strcat(result, " <--- ");
	} else{
		strcat(result, " ---> ");
	}
	
	strcat(result, input);
}

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



/*			winapi			*/
unsigned int scanPorts(std::vector <std::string>* items) {
	
	items->clear();
		
	for (int scancom = 1; scancom < scanSerialPorts; scancom++){
		
		bool found = false;
		
		char comname[portNameLen];
			sprintf(comname, "COM%i", scancom);

		char compath[portPathLen];
			sprintf(compath, "\\\\.\\%s", comname);
		
		HANDLE Port = CreateFileA(compath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		
		if(Port != INVALID_HANDLE_VALUE){
			
		//	found = true;
			items->push_back(comname);
		}
	/*	else{
			DWORD lerr = GetLastError();
			//	port is busy now
			if(lerr == ERROR_ACCESS_DENIED){
				found = true;
			}
		}
		
		if(found){
			strcpy(splsarray[splsarrayUtil], comname);
			splsarrayUtil++;
		}*/
		
		CloseHandle(Port);
	}
	
	return items->size();
}

