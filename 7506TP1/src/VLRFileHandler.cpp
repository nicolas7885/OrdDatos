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
#define POINTER_TO_FREE_SIZE 16

VLRFileHandler::VLRFileHandler(std::string path)
:FileHandler(path){
	char* cp=&metadata[FORMAT_SIZE_POS+metadata[FORMAT_SIZE_POS]];
	firstFreeRelPos=*reinterpret_cast<long unsigned int*>(cp);
	fs.seekp(calculateOffset(0));
}

VLRFileHandler::VLRFileHandler(std::string path, std::string format)
:FileHandler(path,format),
 firstFreeRelPos(0){
	char* cp= reinterpret_cast<char*>(&firstFreeRelPos);
	for(uint i=0; i<sizeof(firstFreeRelPos); i++){
		metadata[i+FORMAT_SIZE_POS+metadata[FORMAT_SIZE_POS]]=*(cp+i);
	}
	fs.write(&metadata[0], metadata.size());
	fs.seekp(calculateOffset(0));
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

void VLRFileHandler::writePointerToFree(ulint freeSpacePos, ulint freeSpaceSize,ulint nextFreePointer) {
	std::vector<char> freeSpacePointer(freeSpaceSize);
	char* cp=reinterpret_cast<char*>(nextFreePointer);
	for(uint i=0; i<sizeof(nextFreePointer); i++){
		freeSpacePointer[i]=*(cp+i);
	}
	this->writeBin(freeSpacePos, freeSpacePointer, DATA_TYPE_POINTER_TO_FREE);
}

/*read the next free Pointer at currPos.
 * If succesful returns the struture*/
PointerToFree VLRFileHandler::readPointerToFree(ulint relPos) {
	PointerToFree newPointer;
	newPointer.relPos=relPos;
	fs.seekp(relPos);
	readType();
	//todo check type
	newPointer.size=readSize();
	fs.read((char*)&newPointer.pointerToNext,sizeof(newPointer.pointerToNext));
	return newPointer;
}

/*writes reg in next possible position. If its written
 * If write is succesful returns relPos where it ended in. */
ulint VLRFileHandler::writeNext(const VLRegistry& reg) {
	VLRSerializer serializer;
	std::vector<char> serializedData;
	serializer.serializeReg(serializedData,reg);
	ulint relPos;
	if(firstFreeRelPos==0){
		relPos=std::ios_base::end;//write at end of file
	}else{
		bool spaceFound=false;
		relPos=firstFreeRelPos;
		PointerToFree prevFreePointer;
		while(!spaceFound && !this->eof()){
			PointerToFree currFreePointer=readPointerToFree(relPos);
			ulint freeSize=currFreePointer.size;
			if(freeSize>serializedData.size()){
				spaceFound=true;//to exit
				ulint freeSpacePointerOverhead=sizeof(char)+sizeof(regSize_t);
				ulint freeSpacePointerSize=freeSpacePointerOverhead+
						sizeof(currFreePointer.pointerToNext);
				ulint difference=freeSize-serializedData.size();
				if(difference>=freeSpacePointerSize){
					//update free size after reg
					ulint freeSpacePos=relPos+freeSpacePointerOverhead+serializedData.size();
					writePointerToFree(freeSpacePos,
							difference-freeSpacePointerOverhead,
							currFreePointer.pointerToNext);
					//update linked list(prev)
					writePointerToFree(prevFreePointer.relPos,
							prevFreePointer.size,
							currFreePointer.relPos);
				}else{
					/*include internal fragmentation as part of reg
					 *to avoid loss of space(shouldnt influence data)*/
					serializedData.resize(freeSize);
					//update linked list (prev)
					writePointerToFree(prevFreePointer.relPos,
							prevFreePointer.size,
							currFreePointer.pointerToNext);
				}
			}else{
				//go to next pointer
				relPos=currFreePointer.pointerToNext;
				if(relPos ==0){
					relPos=std::ios_base::end;
					spaceFound=true;
				}
				prevFreePointer=currFreePointer;
			}
		}
	}
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
	return relPos+512;
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
