/*
 * BlockFileHandler.cpp
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#include "BlockFileHandler.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "Field.h"
#include "VLRegistry.h"
#include "VLRSerializer.h"
#include "VLRUnserializer.h"

#define BLOCK_CHARGE_PERCENTAGE 80
#define METADATA_BSIZE_POS 0

/*creates file handler for file in path. Reads metadata and byteMap.
 * */
BlockFileHandler::BlockFileHandler(std::string path)
:FileHandler(path),
 currRelPos(0){
	restartBuffersToBeginning();
	bSize=metadata[METADATA_BSIZE_POS];
	byteMap.resize(blockSizeInBytes());
	fs.read(&byteMap[0],(int)blockSizeInBytes());//leo mapa bytes
	fs.seekp(calculateOffset(0));
}

/*pre: path is a valid path, bSize is block size, between 1 and 4, format is valid
 * post: creates and opens new block file at path, with blocks of 2^bSize *512 bytes
 * 	with the given format, and an empty bit-map. If file exists, its overriden
 * 	Available blocks in bitmap depend on number of fields in format and bSize */
BlockFileHandler::BlockFileHandler(std::string path, uint bSize, std::string format)
:FileHandler(path,format),
 bSize(bSize),
 byteMap(blockSizeInBytes()),
 currRelPos(0){
	restartBuffersToBeginning();
	metadata[METADATA_BSIZE_POS]=bSize;

	fs.write(&metadata[0], metadata.size());
	rewriteByteMap();//initializes byteMap as empty
	fs.seekp(calculateOffset(0));
}

BlockFileHandler::~BlockFileHandler(){
	//nothing to be done
}

/*attempts to write data into a single block.
 * If there is no more space available returns -1.
 * If possible, finds first empty block and writes it there,
 * then updates the byte map, and returns 0.*/
int BlockFileHandler::write(const std::vector<VLRegistry> &data) {
	std::vector<char> serializedData;
	VLRSerializer serializer;
	serializer.serializeBlock(serializedData,data);
	int relPos=0;
	std::vector<char>::iterator freeBlockSpaceIt=byteMap.begin();
	for(; freeBlockSpaceIt!=byteMap.end() && *freeBlockSpaceIt!=0; freeBlockSpaceIt++){
		relPos++;
	}
	if(freeBlockSpaceIt==byteMap.end()){
		return -1;//no free space
	}else{
		return writeBin(relPos, serializedData);
	}
}

/*if out of bounds returns-2, else calls writeBin*/
int BlockFileHandler::write(const std::vector<VLRegistry> &data, uint relPos) {
	if(relPos>=byteMap.size()) return -2;//bounds check
	std::vector<char> serializedData;
	VLRSerializer serializer;
	serializer.serializeBlock(serializedData,data);
	return writeBin(relPos, serializedData);
}

/*attempts to read the next block and put the information into data.
 * Does nothing if EOF is reached before adding anything to data.
 * If EOF is reached stops reading. Also reads empty blocks*/
void BlockFileHandler::read(std::vector<VLRegistry>& data) {
	if(!fs.eof()){
		std::vector<char> serializedData(blockSizeInBytes());
		fs.read(&serializedData[0],blockSizeInBytes());//todo error if eof
		std::vector<FieldType> format=getFormatAsTypes();
		VLRUnserializer unserializer(format);
		unserializer.unserializeBlock(data,serializedData);
		currRelPos++;
	}
}

/*pre: relPos is in map, and its valid
 * post:reads the block at the position given*/
void BlockFileHandler::read(std::vector<VLRegistry>& data, uint relPos) {
	if(relPos<byteMap.size() && byteMap[relPos]!=0){
		fs.seekp(calculateOffset(relPos));
		currRelPos=relPos;
		this->read(data);
	}
}

/*attempts to write the reg into current block. If overflow, goes to next block.
 * Does nothing and returns -1 if EOF is reached before writing.
 * If write is succesful returns num of block where it ended in.
 * Does not attempt to write into almost full blocks(BLOCK_CHARGE_PERCENTAGE)*/
ulint BlockFileHandler::writeNext(const VLRegistry & reg){
	bool notWritten=true;
	uint relPos=currRelPos;
	while(notWritten && !this->eof()){
		if(currRelPos>0){
			relPos=currRelPos-1;//try last block used
		}else{
			relPos=currRelPos;//if first then try there
		}
		while(byteMap[relPos]>BLOCK_CHARGE_PERCENTAGE){
			relPos++;
			if(relPos>=byteMap.size())
				return -1;
		}//avoid almost full blocks
		std::vector<VLRegistry> block;
		read(block,relPos);
		block.push_back(reg);
		if(write(block,relPos)==0)
			notWritten=false;
		else
			std::cout<<"overflow"<<std::endl;
	}
	if(notWritten)
		return -1;
	else
		return relPos;
}

/*attempts to read the next non empty block and put the information into data.
 * Does nothing and returns false if EOF is reached before reading into reg
 * If EOF is reached stops reading, and returns true.*/
bool BlockFileHandler::readNext(VLRegistry& reg) {
	if(bufferPos<readBuffer.size()){
		reg=readBuffer[bufferPos];
		bufferPos++;
		return true;
	}else{
		while(!this->eof() && byteMap[currRelPos]==0){
			currRelPos++;
		}
		if(!this->eof()){
			this->read(readBuffer,currRelPos);
			bufferPos=0;
			return readNext(reg);
		}else{
			return false;
		}
	}
}

/*pre: relPos is in byteMap and is valid
 * post: the block is replaced by 0. byteMap corrected*/
void BlockFileHandler::deleteBlock(uint relPos) {
	if(byteMap[relPos]){
		std::vector<char> emptyData;
		writeBin(relPos,emptyData);
	}
}

bool BlockFileHandler::eof() {
	if(currRelPos<byteMap.size())
		return fs.eof();
	else
		return true;
}
/**********************************************private *************************************************/
ulint BlockFileHandler::calculateOffset(ulint relPos) {
	ulint offset = metadata.size()+ byteMap.size();
	offset += relPos * blockSizeInBytes();
	return offset;
}

/*attempts to write data into the specified block.
 * if overflow returns -1,if succesful returns 0*/
int BlockFileHandler::writeBin(uint relPos,const std::vector<char>& data) {
	if(data.size()>blockSizeInBytes())
		return -1;

	long int percentage=data.size()*100;
	percentage/=blockSizeInBytes();//something wrong in percentage
	if(percentage==0) percentage=1;//if its almost empty still mark as occupied
	byteMap[relPos]=(char) percentage;

	fs.seekg(calculateOffset(relPos));
	std::vector<char> block(blockSizeInBytes());//to get 0 filled vector
	std::copy(data.begin(),data.end(),block.begin());
	fs.write(&block[0], blockSizeInBytes());
	if(!fs)std::cout<<"error writing"<<std::endl;
	rewriteByteMap();
	currRelPos=relPos+1;
	return 0;
}

uint BlockFileHandler::blockSizeInBytes() {
	uint blockSizeInBytes = 0x01;
	blockSizeInBytes <<= bSize;
	blockSizeInBytes *= CHUNK_SIZE;
	return blockSizeInBytes;
}

void BlockFileHandler::rewriteByteMap() {
	fs.seekg(METADATA_SIZE);
	fs.write(&byteMap[0], blockSizeInBytes());
}


void BlockFileHandler::restartBuffersToBeginning() {
	bufferPos = 0;
	currRelPos = 0;
	readBuffer.clear();
}
