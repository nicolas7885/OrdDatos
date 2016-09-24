/*
 * VLRFileHandler.cpp
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#include "VLRFileHandler.h"

#include <string>
#include <iostream>
#include "VLRegistry.h"

VLRFileHandler::VLRFileHandler(std::string path)
:FileHandler(path){
	//todo read firstFreeRelPos bytes into firstFreeRelPos
	char* cp=&metadata[FORMAT_SIZE_POS+metadata[FORMAT_SIZE_POS]];
	firstFreeRelPos=*reinterpret_cast<long unsigned int*>(cp);
	fs.seekp(calculateOffset(0),std::ios_base::beg);
}

VLRFileHandler::VLRFileHandler(std::string path, std::string format)
:FileHandler(path,format),
 firstFreeRelPos(0){
	//todo write firstFreeRelPos into metadata
	char* cp= reinterpret_cast<char*>(&firstFreeRelPos);
	for(uint i=0; i<sizeof(firstFreeRelPos); i++){
		metadata[i+FORMAT_SIZE_POS+metadata[FORMAT_SIZE_POS]]=*(cp+i);
	}
	fs.write(&metadata[0], metadata.size());
	fs.seekp(calculateOffset(0),std::ios_base::beg);
}

VLRFileHandler::~VLRFileHandler() {
	// nothing to do
}

int VLRFileHandler::writeNext(VLRegistry& reg) {
	return 0;
}

bool VLRFileHandler::readNext(VLRegistry& reg) {
	return true;
}

ulint VLRFileHandler::calculateOffset(ulint relPos) {
	return relPos+512;
}

/*writes serializedData into relPos. Writes over what was there.
 * There is no overflow so it returns 0, unless there is a writing error, then returns -1.*/
int VLRFileHandler::writeBin(uint relPos,
		const std::vector<char>& serializedData) {
	fs.seekg(calculateOffset(relPos), std::ios_base::beg);
	fs.write(&serializedData[0], serializedData.size());
	if(!fs){
		std::cout<<"error writing"<<std::endl;
		return -1;
	}else{
		return 0;
	}
}

void VLRFileHandler::restartBuffersToBeginning() {
}
