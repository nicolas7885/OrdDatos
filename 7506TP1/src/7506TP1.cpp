//============================================================================
// Name        : 7506TP1.cpp
// Author      : nico
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "tests.h"
#include "BlockFileHandler.h"
#include "VLRFileHandler.h"
#include "FileHandler.h"

using namespace std;

#define MODE_PARAM 1
#define MODE_NEW 1
#define FORMAT  "i1,i2,i4,sD,d,dT"

int main(int argc, char* argv[]) {
	//todo modes
	//todo use arguments
	//todo test deletes
	runTests();
	BlockFileHandler blockHandler("test1.bin");
	blockHandler.toCsv("testCsv");
	BlockFileHandler newBlockHandler("test2.bin", 1, FORMAT);
	newBlockHandler.fromCsv("testCsv");
	newBlockHandler.toCsv("test2Csv");
	VLRFileHandler vlrHandler("testVLR.bin", FORMAT);
	vlrHandler.fromCsv("testCsv");
	//vlrHandler.deleteReg(6);
	vlrHandler.toCsv("testVLRCsv");
	return 0;
}
