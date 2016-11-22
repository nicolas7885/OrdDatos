/*
 * VLRFileHandler.cpp
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#include "VLRSecFileH.h"

#include <string>
#include <iostream>

#include "../VLRegistries/VLRegistry.h"
#include "../VLRegistries/VLRSerializer.h"
#include "../VLRegistries/VLRUnserializer.h"

#define DATA_TYPE_REG 1
#define DATA_TYPE_POINTER_TO_FREE 2
#define FIRST_FREE_POINTER_POS 256+FORMAT_SIZE_POS

VLRSecFileH::VLRSecFileH(std::string path)
:VLRFileHandler(&file),
 file(path){
}

VLRSecFileH::VLRSecFileH(std::string path, std::string format)
:VLRFileHandler(&file),
 file(path,1){
	this->setFormat(format);
}

VLRSecFileH::~VLRSecFileH() {
	// nothing to do
}

/*rel pos is 0 at the start of the data segment
 * attempts to read the reg at relpos, if reg at relpos valid, writes it into reg
 * and returns true. If reg at relpos deleted or invalid, returns false.
 * Does nothing and returns false if EOF is reached before reading into reg.*/
bool VLRSecFileH::read(ulint relPos, VLRegistry& reg) {
	std::vector<char> serializedData;
	bool retVal=file.read(relPos,serializedData);
	unserializeReg(serializedData,reg);
	return retVal;
}

/* pre: relPos was obtained from a write or read operation
 * rel pos is 0 at the start of the data segment
 * post:deletes the reg found at relPos. */
void VLRSecFileH::deleteReg(ulint relPos) {
	file.deleteReg(relPos);
}

/*writes reg in next possible position. If its written
 * If write is succesful returns pos where it ended in. */
ulint VLRSecFileH::writeNext(const VLRegistry& reg) {
	std::vector<char> serializedData;
	VLRSerializer serializer;
	serializer.serializeReg(serializedData,reg);
	return file.writeNext(serializedData);
}

/*attempts to read the next valid registry*
 * Does nothing and returns false if EOF is reached before reading into reg.*/
bool VLRSecFileH::readNext(VLRegistry& reg) {
	std::vector<char> serializedData;
	if(file.readNext(serializedData)){
		unserializeReg(serializedData,reg);
		return true;
	}else{
		return false;
	}
}

bool VLRSecFileH::get(uint relPos, int id, VLRegistry& result) {
	VLRegistry temp;
	this->read(relPos,temp);
	if(temp.getField(0).value.i4==id){
		result=temp;
		return true;
	}else{
		return false;
	}
}

void VLRSecFileH::restartBuffersToBeginning() {
	file.restartBuffersToBeginning();
}

uint VLRSecFileH::tellg() {
	return file.tellg();
}

void VLRSecFileH::unserializeReg(std::vector<char> serializedData,
		VLRegistry& reg) {
	std::vector<FieldType> format = this->getFormatAsTypes();
	VLRUnserializer unserializer(format);
	dataIt_t dataIt = serializedData.begin();
	unserializer.unserializeReg(reg, dataIt);
}
