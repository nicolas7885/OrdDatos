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
#include "VLRSerializer.h"
#include "VLRUnserializer.h"

#define DATA_TYPE_REG 1
#define DATA_TYPE_POINTER_TO_FREE 2
#define FIRST_FREE_POINTER_POS 256+FORMAT_SIZE_POS

VLRFileHandler::VLRFileHandler(std::string path)
:FileHandler(path){
	char* cp=&metadata[FIRST_FREE_POINTER_POS];
	firstFreePtr=*reinterpret_cast<long unsigned int*>(cp);
	restartBuffersToBeginning();
}

VLRFileHandler::VLRFileHandler(std::string path, std::string format)
:FileHandler(path,format),
 firstFreePtr(0){
	fs.write(&metadata[0], metadata.size());
	restartBuffersToBeginning();
}

VLRFileHandler::~VLRFileHandler() {
	// nothing to do
}

/*rel pos is 0 at the start of the data segment
 * attempts to read the reg at relpos, if reg at relpos valid, writes it into reg
 * and returns true. If reg at relpos deleted or invalid, returns false.
 * Does nothing and returns false if EOF is reached before reading into reg.*/
bool VLRFileHandler::read(ulint relPos, VLRegistry& reg) {
	ulint pos= calculateOffset(relPos);
	fs.seekp(pos);
	char dataType = readType();
	if(this->eof())
		return false;
	if(dataType == DATA_TYPE_REG){
		read(reg);
		return true;
	}else{
		return false;
	}
}

/* pre: relPos was obtained from a write or read operation
 * rel pos is 0 at the start of the data segment
 * post:deletes the reg found at relPos. */
void VLRFileHandler::deleteReg(ulint relPos) {
	restartBuffersToBeginning();
	ulint pos= calculateOffset(relPos);
	PointerToFree prevFreePointer={0,0,firstFreePtr};
	ulint nextFreePointerPos=0;
	ulint searchPos=firstFreePtr;
	//find last free pointer before pos to be deleted
	while (!this->eof() && !nextFreePointerPos && searchPos) {
		PointerToFree currFreePointer = readPointerToFree(searchPos);
		if (prevFreePointer.pointerToNext>pos) {
			nextFreePointerPos=prevFreePointer.pointerToNext;
		} else {
			searchPos = currFreePointer.pointerToNext;//if next pointer==0 exits while
			prevFreePointer = currFreePointer;
		}
	}
	updateLinkedList(prevFreePointer,pos);
	fs.seekg(pos);
	readType();
	//if(readType()!=DATA_TYPE_REG) //error
	regSize_t size=readSize();
	writePointerToFree(pos,size,nextFreePointerPos);
}

/*read the next free Pointer at currPos.
 * If succesful returns the struture*/
PointerToFree VLRFileHandler::readPointerToFree(ulint pos) {
	PointerToFree newPointer;
	newPointer.pos=pos;
	fs.seekp(pos);
	readType();
	//todo check type
	newPointer.size=readSize();
	fs.read((char*)&newPointer.pointerToNext,sizeof(newPointer.pointerToNext));
	return newPointer;
}

void VLRFileHandler::writePointerToFree(ulint freeSpacePos, ulint freeSpaceSize,ulint nextFreePointer) {
	std::vector<char> freeSpacePointer(freeSpaceSize);
	char* cp=reinterpret_cast<char*>(&nextFreePointer);
	for(uint i=0; i<sizeof(nextFreePointer); i++){
		freeSpacePointer[i]=*(cp+i);
	}
	this->writeBin(freeSpacePos, freeSpacePointer, DATA_TYPE_POINTER_TO_FREE);
}

void VLRFileHandler::updateMetadata() {
	//update metadata
	char* cp = reinterpret_cast<char*>(&firstFreePtr);
	for (uint i = 0; i < sizeof(firstFreePtr); i++) {
		metadata[i + FIRST_FREE_POINTER_POS] = *(cp + i);
	}
	fs.clear();
	fs.seekp(0);
	fs.write(&metadata[0], metadata.size());
}

void VLRFileHandler::updateLinkedList(const PointerToFree& prevFreePointer,
		const ulint nextPointer) {
	if (prevFreePointer.pointerToNext == firstFreePtr) {
		firstFreePtr = nextPointer;
		updateMetadata();
	} else {
		writePointerToFree(prevFreePointer.pos, prevFreePointer.size,
				nextPointer);
	}
}

/*follows linked list of free spaces, until it finds a space big enough.
 * If found,updates the linked list accordingly to new insertion, and returns position of insertion.
 * If not found in list, returns the end of the file, to write there*/
ulint VLRFileHandler::findPosToWriteAndUpdateList(std::vector<char>& serializedData) {
	const ulint endOfFileOff = 0;
	if (firstFreePtr == 0) {
		return endOfFileOff; //write at end of file
	}
	ulint pos = firstFreePtr;//not 0
	PointerToFree prevFreePointer={0,0,firstFreePtr};
	while (!this->eof()) {
		PointerToFree currFreePointer = readPointerToFree(pos);
		ulint freeSize = currFreePointer.size;
		if (freeSize >= serializedData.size()) {
			ulint freeSpacePointerOverhead = sizeof(char)+ sizeof(regSize_t);

			ulint freeSpacePointerSize = freeSpacePointerOverhead+ sizeof(currFreePointer.pointerToNext);

			ulint difference = freeSize - serializedData.size();

			if (difference >= freeSpacePointerSize) {
				//update free size after reg
				ulint freeSpacePos = pos + freeSpacePointerOverhead+ serializedData.size();
				writePointerToFree(freeSpacePos,
						difference - freeSpacePointerOverhead,
						currFreePointer.pointerToNext);
				updateLinkedList(prevFreePointer, currFreePointer.pos);
			} else {
				/*include internal fragmentation as part of reg
				 *to avoid loss of space(shouldnt influence data)*/
				serializedData.resize(freeSize);
				updateLinkedList(prevFreePointer,currFreePointer.pointerToNext);
			}
			return currFreePointer.pos;//exit
		} else {
			//go to next pointer
			pos = currFreePointer.pointerToNext;
			if (pos == 0) {
				return endOfFileOff;
			}
			prevFreePointer = currFreePointer;
		}
	}
	return endOfFileOff;
}

/*writes reg in next possible position. If its written
 * If write is succesful returns pos where it ended in. */
ulint VLRFileHandler::writeNext(const VLRegistry& reg) {
	std::vector<char> serializedData;
	{
		VLRSerializer serializer;
		serializer.serializeReg(serializedData,reg);
	}
	ulint pos = findPosToWriteAndUpdateList(serializedData);
	if(pos<512){//avoid metadata
		fs.seekg (0, fs.end);
		pos = fs.tellg();
	}
	this->writeBin(pos,serializedData,DATA_TYPE_REG);
	return pos;
}

/*attempts to read the next valid registry*
 * Does nothing and returns false if EOF is reached before reading into reg.*/
bool VLRFileHandler::readNext(VLRegistry& reg) {
	while(!this->eof()){
		char dataType = readType();
		if(this->eof())
			return false;
		switch(dataType){
		case DATA_TYPE_REG:
			this->read(reg);
			return true;
			break;
		case DATA_TYPE_POINTER_TO_FREE:{
			regSize_t freeSize=readSize();
			fs.seekp(freeSize,std::ios_base::cur);
			break;
		}
		default:
			return false;
		}
	}
	return false;
}

ulint VLRFileHandler::calculateOffset(ulint relPos) {
	return relPos+metadata.size();
}

/*writes data type, serializedData size, and serializedData  into pos.
 * Writes over what was there.
 * There is no overflow so it returns 0, unless there is a writing error, then returns -1.*/
int VLRFileHandler::writeBin(uint pos,
		const std::vector<char>& serializedData, char dataType) {
	fs.seekg(pos);
	fs.write(&dataType, sizeof(dataType));//data type
	ulint size= serializedData.size();
	fs.write((char*)&size, sizeof(size));//size
	fs.write(&serializedData[0], serializedData.size());//data
	if(!fs){
		std::cout<<"error writing"<<std::endl;
		return -1;
	}else{
		return 0;
	}
}

void VLRFileHandler::restartBuffersToBeginning() {
	fs.clear();
	fs.seekg(calculateOffset(0));
}

/*reads & returns the size read*/
regSize_t VLRFileHandler::readSize(){
	//read size
	regSize_t size;
	fs.read((char*)&size, sizeof(regSize_t));
	return size;
}

/*reads & returns the type*/
char VLRFileHandler::readType() {
	char dataType;
	fs.read(&dataType, sizeof(dataType));
	return dataType;
}

void VLRFileHandler::read(VLRegistry& reg) {
	//read size
	regSize_t regSize = readSize();
	//read data
	std::vector<char> serializedData(regSize);
	fs.read(&serializedData[0], serializedData.size());
	//unserialize
	std::vector<FieldType> format = this->getFormatAsTypes();
	VLRUnserializer unserializer(format);
	dataIt_t dataIt = serializedData.begin();
	unserializer.unserializeReg(reg, dataIt);
}
