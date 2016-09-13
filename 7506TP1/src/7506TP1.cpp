//============================================================================
// Name        : 7506TP1.cpp
// Author      : nico
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "FileHandler.h"
using namespace std;

#define MODE_PARAM 1
#define MODE_NEW 1

int main(int argc, char* argv[]) {
	//todo modes
	//todo use arguments
	FileHandler handler("testNew.bin",2,"i1,i4,sD,d");
	return 0;
}
