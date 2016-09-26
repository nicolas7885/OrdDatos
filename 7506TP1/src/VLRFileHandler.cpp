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

/*attempts to read the reg at relpos, if reg at relpos valid, writes it into reg
 * and returns true. If reg at relpos deleted or invalid, returns false.
 * Does nothing and returns false if EOF is reached before reading into reg.*/
bool VLRFileHandler::read(ulint relPos, VLRegistry& reg) {
	fs.seekp(calculateOffset(relPos));
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

void VLRFileHandler::deleteReg(ulint relPos) {
	PointerToFree prevFreePointer={0,0,firstFreePtr};
	ulint nextFreePointerPos=0;
	ulint searchPos=firstFreePtr;
	while (!this->eof() && !nextFreePointerPos && searchPos) {
		PointerToFree currFreePointer = readPointerToFree(searchPos);
		if (prevFreePointer.pointerToNext>relPos) {
			nextFreePointerPos=prevFreePointer.pointerToNext;
		} else {
			searchPos = currFreePointer.pointerToNext;//if relPos==0 write in eof
			prevFreePointer = currFreePointer;
		}
	}
	updateLinkedList(prevFreePointer,relPos);
	fs.seekp(calculateOffset(relPos));
	readType();
	//if(readType()!=DATA_TYPE_REG) //error
	ulint size=readSize();
	writePointerToFree(relPos,size,nextFreePointerPos);
}

/*read the next free Pointer at currPos.
 * If succesful returns the struture*/
PointerToFree VLRFileHandler::readPointerToFree(ulint relPos) {
	PointerToFree newPointer;
	newPointer.relPos=relPos;
	fs.seekp(calculateOffset(relPos));
	readType();
	//todo check type
	newPointer.size=readSize();
	fs.read((char*)&newPointer.pointerToNext,sizeof(newPointer.pointerToNext));
	return newPointer;
}

void VLRFileHandler::writePointerToFree(ulint freeSpacePos, ulint freeSpaceSize,ulint nextFreePointer) {
	std::vector<char> freeSpacePointer(freeSpaceSize);
	char* cp=reinterpret_cast<char*>(nextFreePointer);
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
	fs.write(&metadata[0], metadata.size());
}

void VLRFileHandler::updateLinkedList(const PointerToFree& prevFreePointer,
		const ulint nextPointer) {
	if (prevFreePointer.pointerToNext == firstFreePtr) {
		firstFreePtr = nextPointer;
		updateMetadata();
	} else {
		writePointerToFree(prevFreePointer.relPos, prevFreePointer.size,
				nextPointer);
	}
}

/*follows linked list of free spaces, until it finds a space big enough.
 * If found,updates the linked list accordingly to new insertion, and returns position of insertion.
 * If not found in list, returns the end of the file, to write there*/
ulint VLRFileHandler::findPosToWriteAndUpdateList(std::vector<char>& serializedData) {
	if (firstFreePtr == 0) {
		return std::ios_base::end; //write at end of file
	}
	ulint relPos = firstFreePtr;
	PointerToFree prevFreePointer={0,0,firstFreePtr};
	while (!this->eof()) {
		PointerToFree currFreePointer = readPointerToFree(relPos);
		ulint freeSize = currFreePointer.size;
		if (freeSize > serializedData.size()) {
			ulint freeSpacePointerOverhead = sizeof(char)
										+ sizeof(regSize_t);
			ulint freeSpacePointerSize = freeSpacePointerOverhead
					+ sizeof(currFreePointer.pointerToNext);
			ulint difference = freeSize - serializedData.size();
			if (difference >= freeSpacePointerSize) {
				//update free size after reg
				ulint freeSpacePos = relPos + freeSpacePointerOverhead
						+ serializedData.size();
				writePointerToFree(freeSpacePos,
						difference - freeSpacePointerOverhead,
						currFreePointer.pointerToNext);
				updateLinkedList(prevFreePointer, currFreePointer.relPos);
			} else {
				/*include internal fragmentation as part of reg
				 *to avoid loss of space(shouldnt influence data)*/
				serializedData.resize(freeSize);
				updateLinkedList(prevFreePointer,
						currFreePointer.pointerToNext);
			}
			return relPos;//exit
		} else {
			//go to next pointer
			relPos = currFreePointer.pointerToNext;
			if (relPos == 0) {
				return std::ios_base::end;
			}
			prevFreePointer = currFreePointer;
		}
	}
	return std::ios_base::end;
}

/*writes reg in next possible position. If its written
 * If write is succesful returns relPos where it ended in. */
ulint VLRFileHandler::writeNext(const VLRegistry& reg) {
	std::vector<char> serializedData;
	{
		VLRSerializer serializer;
		serializer.serializeReg(serializedData,reg);
	}
	ulint relPos = findPosToWriteAndUpdateList(serializedData);
	this->writeBin(relPos,serializedData,DATA_TYPE_REG);
	return relPos;
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
	return relPos+152;
}

/*writes data type, serializedData size, and serializedData  into relPos.
 * Writes over what was there.
 * There is no overflow so it returns 0, unless there is a writing error, then returns -1.*/
int VLRFileHandler::writeBin(uint off,
		const std::vector<char>& serializedData, char dataType) {
	fs.seekg(this->calculateOffset(off));
	fs.write(&dataType, sizeof(dataType));//data type
	ulint size= serializedData.size();
	fs.write((char*)&size, sizeof(size));//size
	fs.write(&serializedData[sizeof(char)], serializedData.size());//data
	if(!fs){
		std::cout<<"error writing"<<std::endl;
		return -1;
	}else{
		return 0;
	}
}

void VLRFileHandler::restartBuffersToBeginning() {
	fs.seekg(calculateOffset(0));
}

/*returns the size read,increments currRelPos by sizeof(regSize)*/
regSize_t VLRFileHandler::readSize(){
	//read size
	regSize_t size;
	fs.read((char*)&size, sizeof(regSize_t));
	return size;
}

/*returns the type and increments currRelPos by size of type*/
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
