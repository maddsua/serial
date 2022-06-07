//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#include <stdio.h>
#include <windows.h>

#include <vector>
#include <string>
#include <fstream>


#include "staticConfig.hpp"



bool SaveLogFile(std::vector <std::string>* commlog, char* filepath){
	
	std::ofstream localFile(filepath, std::ios::out);
	
	if(localFile.is_open()){
		
		for(long i = 0; i < commlog->size(); i++){
			
			localFile << commlog->at(i);
		}
			
		localFile.close();
	}
	else{
		return false;
	}
	
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
	
	if(isInput){
		strcat(result, " <--- ");
	}
	else{
		strcat(result, " ---> ");
	}
	
	strcat(result, input);
	
}

void quickcmd(HWND term, const char* cmd, bool usenl,  const char* port, std::vector <std::string>* datalog, char* comm){
	
	//	display command	
	if(usenl){
							
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

void dropdown(HWND combo, char** items, unsigned int length, unsigned int focus, bool erase){
	
	if(erase){
		int contlen = SendMessage(combo, CB_GETCOUNT, 0, 0);
		
		for(int i = 0; i < contlen; i++){
			SendMessage(combo, CB_DELETESTRING, 0, 0);
		}
	}
	
	for(int i = 0; i < length; i++){					
		SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)items[i]);
	}
	SendMessage(combo, CB_SETCURSEL , focus, 0);
	
}

void dropdown(HWND combo, const int* items, unsigned int length, unsigned int focus, bool erase){
	
	if(erase){
		int contlen = SendMessage(combo, CB_GETCOUNT, 0, 0);
		
		for(int i = 0; i < contlen; i++){
			SendMessage(combo, CB_DELETESTRING, 0, 0);
		}
	}
	
	for(int i = 0; i < length; i++){
		
		char tempnumstr[64];
			sprintf(tempnumstr, "%i baud", items[i]);
		
		SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)tempnumstr);
	}
	SendMessage(combo, CB_SETCURSEL , focus, 0);
	
}


/*			winapi			*/
unsigned int scanPorts(char** splsarray){
	
	unsigned int splsarrayUtil = 0;
		
	for(int scancom = 1; scancom < scanSerialPorts; scancom++){
		
		bool found = false;
		
		char comname[portNameLen];
			sprintf(comname, "COM%i", scancom);

		char compath[portPathLen];
			sprintf(compath, "\\\\.\\%s", comname);
		
		HANDLE Port = CreateFile(compath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		
		if(Port != INVALID_HANDLE_VALUE){
			
		//	found = true;
			strcpy(splsarray[splsarrayUtil], comname);
			splsarrayUtil++;
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
	
	return splsarrayUtil;
}


/*			memory operations			*/
char*** create3d(unsigned int dim_1, unsigned int dim_2, unsigned int dim_3){

	//	allocate memory for 3 dimensions
	char*** array = (char***)malloc(sizeof(char ***)*dim_1);
		for (int x = 0; x < dim_1; x++){
			array[x] = (char**)malloc(sizeof(char**)*dim_2);
				for (int y = 0; y < dim_2; y++){
					array[x][y] = (char*)malloc(sizeof(char)*dim_3);
				}
		}


	//	fill with zeros for 146% sure
	for(int x = 0; x < dim_1; x++){
		for(int y = 0; y < dim_2; y++){
			for (int z = 0; z < dim_3; z++){
				array[x][y][z] = 0;
			}
		}
	}

	return array;
}

char** create2d(unsigned int dim_1, unsigned int dim_2){

	//	allocate memory for 2 dimensions
	char** array = (char**)malloc(sizeof(char**)*dim_1);
		for (int x = 0; x < dim_1; x++){
			array[x] = (char*)malloc(sizeof(char)*dim_2);
		}	


	//	fill with zeros for 146% sure
	for(int x = 0; x < dim_1; x++){
		for(int y = 0; y < dim_2; y++){
			array[x][y] = 0;
		}
	}

	return array;
}

void clear3d(char*** array, unsigned int dim_1, unsigned int dim_2){
	
	for(int x = 0; x < dim_1; x++){
		for (int y = 0; y < dim_2; y++){
			free(array[x][y]);
		}	
		free(array[x]);
	}
	free(array);
}

void clear2d(char** array, unsigned int dim_1){
	
	for(int x = 0; x < dim_1; x++){
		free(array[x]);
	}
	free(array);
}
