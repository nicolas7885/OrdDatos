//============================================================================
// Name        : 7506TP1.cpp
// Author      : nico
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "tests.h"
#include "FileHandler.h"
#include "VLRegistry.h"
using namespace std;

#define MODE_PARAM 1
#define MODE_NEW 1


int main(int argc, char* argv[]) {
	//todo modes
	//todo use arguments
	//test variables
	runTests();
	FileHandler blockHandler("test.bin");
	blockHandler.toCsv("testCsv");
	FileHandler newBlockHandler("tests2.bin",4,"i1,i2,i4,sD,d,dT");
	newBlockHandler.fromCsv("testCsv");
	//newBlockHandler.toCsv("test2.csv");
	return 0;
}
