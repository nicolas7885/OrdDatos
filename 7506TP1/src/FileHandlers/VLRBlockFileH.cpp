/*
 * VLRBlockFileH.cpp
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#include "VLRBlockFileH.h"

#include "../VLRegistries/Field.h"
#include "../VLRegistries/VLRSerializer.h"
#include "../VLRegistries/VLRUnserializer.h"

#define BLOCK_CHARGE_PERCENTAGE 80
#define METADATA_BSIZE_POS 0

/*creates file handler for file in path. Reads metadata and byteMap.
 * */
VLRBlockFileH::VLRBlockFileH(std::string path)
:VLRFileHandler(&file),
 file(path),
 bufferPos(0){
	restartBuffersToBeginning();
}

/*pre: path is a valid path, bSize is block size, between 1 and 4, format is valid
 * post: creates and opens new block file at path, with blocks of 2^bSize *512 bytes
 * 	with the given format, and an empty bit-map. If file exists, its overriden
 * 	Available blocks in bitmap depend on number of fields in format and bSize */
VLRBlockFileH::VLRBlockFileH(std::string path, uint bSize, std::string format)
:VLRFileHandler(&file),
 file(path,bSize),
 bufferPos(0){
	this->setFormat(format);
	restartBuffersToBeginning();
}

VLRBlockFileH::~VLRBlockFileH(){
	//nothing to be done
}

/*attempts to write data into a single block.
 * If there is no more space available returns -1.
 * If possible returns 0.*/
int VLRBlockFileH::write(const std::vector<VLRegistry> &data) {
	std::vector<char> serializedData = serializeBlock(data);
	return file.write(serializedData);
}



/*if out of bounds returns-2, else calls writeBin*/
int VLRBlockFileH::write(const std::vector<VLRegistry> &data, uint relPos) {
	std::vector<char> serializedData = serializeBlock(data);
	return file.write(serializedData,relPos);
}

/*attempts to read the next block and put the information into data.
 * Does nothing if EOF is reached before adding anything to data.
 * If EOF is reached stops reading. Also reads empty blocks*/
void VLRBlockFileH::read(std::vector<VLRegistry>& data) {
	if(!file.eof()){
		std::vector<char> serializedData;
		file.read(serializedData);
		unserializeBlock(serializedData,data);
		bufferPos=0;
	}
}

/*pre: relPos is in map, and its valid
 * post:reads the block at the position given*/
void VLRBlockFileH::read(std::vector<VLRegistry>& data, uint relPos) {
	std::vector<char> serializedData;
	file.read(serializedData,relPos);
	unserializeBlock(serializedData,data);
	bufferPos=0;
}

/*attempts to write the reg into current block. If overflow, goes to next block.
 * Does nothing and returns -1 if EOF is reached before writing.
 * If write is succesful returns num of block where it ended in.
 * Does not attempt to write into almost full blocks(BLOCK_CHARGE_PERCENTAGE)*/
ulint VLRBlockFileH::writeNext(const VLRegistry & reg){
	std::vector<char> serializedData;
	VLRSerializer serializer;
	serializer.serializeReg(serializedData,reg);
	return file.writeNext(serializedData);
}

/*attempts to read the next non empty block and put the information into data.
 * Does nothing and returns false if EOF is reached before reading into reg
 * If EOF is reached stops reading, and returns true.*/
bool VLRBlockFileH::readNext(VLRegistry& reg) {
	if(bufferPos<readBuffer.size()){
		//next reg in buffer
		reg=readBuffer[bufferPos];
		bufferPos++;
		return true;
	}else{
		//get next block and read first reg
		std::vector<char> serializedData;
		if(file.readNext(serializedData)){
			this->unserializeBlock(serializedData,readBuffer);
			bufferPos=0;
			return this->readNext(reg);
		}else{
			return false;
		}
	}
}

/*pre: relPos is in byteMap and is valid.note: first pos is 0
 * post: the block is replaced by 0.*/
void VLRBlockFileH::deleteBlock(uint relPos) {
	file.deleteBlock(relPos);
	readBuffer.clear();
	bufferPos=0;
}

bool VLRBlockFileH::eof() {
	return file.eof();
}


/*returns relPos of get pointer*/
uint VLRBlockFileH::tellg() {
	return file.tellg();
}

void VLRBlockFileH::restartBuffersToBeginning() {
	bufferPos = 0;
	readBuffer.clear();
	file.restartBuffersToBeginning();
}

bool VLRBlockFileH::get(uint relPos, int id, VLRegistry& result) {
	std::vector<VLRegistry> block;
	this->read(block,relPos);
	//todo use iterator
	for(uint i=0;i<block.size(); i++){
		if(block[i].getField(0).value.i4==id){
			result=block[i];
			return true;
		}
	}
	return false;
}


std::vector<char> VLRBlockFileH::serializeBlock(
		const std::vector<VLRegistry>& data) {
	std::vector<char> serializedData;
	VLRSerializer serializer;
	serializer.serializeBlock(serializedData, data);
	return serializedData;
}

void VLRBlockFileH::unserializeBlock(const std::vector<char>& data,
		std::vector<VLRegistry>& block) {
	std::vector<FieldType> format=getFormatAsTypes();
	VLRUnserializer unserializer(format);
	unserializer.unserializeBlock(block,data);
}
