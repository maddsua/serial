//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#ifndef APPOPS
#define APPOPS


	void log(HWND hEdit, LPCSTR newText);
	void quickcmd(HWND term, const char* cmd, bool usenl,  const char* port, std::vector <std::string>* datalog, char* comm);
	void metalog(const char* input, const char* port, char* result, bool isInput);
	void dropdown(HWND combo, char** items, unsigned int length, unsigned int focus, bool erase);
	void dropdown(HWND combo, const int* items, unsigned int length, unsigned int focus, bool erase);
	
	bool SaveLogFile(std::vector <std::string>* commlog, char* filepath);
	
	unsigned int scanPorts(char** splsarray);
	
	char*** create3d(unsigned int dim_1, unsigned int dim_2, unsigned int dim_3);
	char** create2d(unsigned int dim_1, unsigned int dim_2);
	void clear3d(char*** array, unsigned int dim_1, unsigned int dim_2);
	void clear2d(char** array, unsigned int dim_1);


#endif
