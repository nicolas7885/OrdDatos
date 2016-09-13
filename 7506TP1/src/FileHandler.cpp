/*
 * FileHandler.cpp
 *
 *  Created on: Sep 8, 2016
 *      Author: nicolas
 */

#include "FileHandler.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include "VLRegistry.h"

#define CHUNK_SIZE 512
#define METADATA_SIZE CHUNK_SIZE

/*creates file handler for file in path. Reads metadata and byteMap.
 * */
FileHandler::FileHandler(std::string path)
:fs(path),
 metadata(METADATA_SIZE){
	fs.read(&metadata[0],METADATA_SIZE);//leo metadata
	bSize=metadata[0];
	byteMap.resize((uint)blockSizeInBytes);
	fs.read(&byteMap[0],(int)blockSizeInBytes);//leo mapa bytes
}

/*pre: path is a valid path, bSize is block size, between 1 and 4, format is valid
 * post: creates and opens new block file at path, with blocks of 2^bSize *512 bytes
 * 	with the given format, and an empty bit-map.
 * 	Available blocks in bitmap depend on number of fields in format and bSize */
FileHandler::FileHandler(std::string path, uint bSize, std::string format)
:bSize(bSize),
 metadata(METADATA_SIZE),
 byteMap(blockSizeInBytes),
 fs(path.c_str(),std::ios::binary|std::ios::trunc){
	metadata[0]=bSize;
	setFormat(format);

	fs.write(&metadata[0], metadata.size());
	rewriteByteMap();//initializes byteMap as empty
	fs.seekp(metadata.size()+byteMap.size(),std::ios_base::beg);//prepare for secuential reading
}



FileHandler::~FileHandler() {
	// TODO nothing for now
}

/*attempts to write data into a single block.
 * If there is no more space available returns -1.
 * If possible, finds first empty block and writes it there,
 * then updates the byte map, and returns 0.*/
int FileHandler::write(const std::vector<VLRegistry> &data) {
	std::vector<char> serializedData;
	//todo serialize
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
int FileHandler::write(const std::vector<VLRegistry> &data, int relPos) {
	if(relPos>=byteMap.size()) return -2;//bounds check
	std::vector<char> serializedData;
	//todo serialize
	return writeBin(relPos, serializedData);
}

/*attempts to read the next block and put the information into data.
 * Does nothing if EOF is reached before adding anything to data
 * If EOF is reached stops reading*/
void FileHandler::read(std::vector<VLRegistry>& data) {
	std::vector<char> serializedData(blockSizeInBytes());
	fs.read(&serializedData[0],(uint)blockSizeInBytes());
	//todo unserialize
}



/*pre: relPos is in map, and its valid
 * post:reads the block at the position given*/
void FileHandler::read(std::vector<VLRegistry>& data, int relPos) {
	fs.seekp(calculateOffset(relPos),std::ios_base::beg);
	this->read(data);
}

/*pre: relPos is in byteMap and is valid
 * post: the block is replaced by 0. byteMap corrected*/
void FileHandler::deleteBlock(int relPos) {
	if(byteMap[relPos]){
		std::vector<char> emptyData;
		writeBin(relPos,emptyData);
	}
}

/******************************************private*********************************************/

/*attempts to write data into the specified block.
 * if overflow returns -1,if succesful returns 0*/
int FileHandler::writeBin(int relPos,const std::vector<char>& data) {
	if(data.size()>blockSizeInBytes())
		return -1;

	long int percentage=data.size()*100;
	percentage/=blockSizeInBytes();
	byteMap[relPos]=(char) percentage;

	fs.seekg(calculateOffset(relPos), std::ios_base::beg);
	std::vector<char> block(blockSizeInBytes());
	std::copy(data.begin(),data.end(),block.begin());
	fs.write(&block[0], blockSizeInBytes());
	rewriteByteMap();
	return 0;
}

bool FileHandler::eof() {
	return fs.eof();
}

int FileHandler::blockSizeInBytes() {
	uint blockSizeInBytes = 0x01;
	blockSizeInBytes <<= bSize;
	blockSizeInBytes *= CHUNK_SIZE;
	return blockSizeInBytes;
}


void FileHandler::rewriteByteMap() {
	fs.seekg(METADATA_SIZE,std::ios_base::beg);
	fs.write(&byteMap[0], blockSizeInBytes());
}

long int FileHandler::calculateOffset(int relPos) {
	long int offset = metadata.size()+ byteMap.size();
	offset += relPos * blockSizeInBytes;
	return offset;
}

void FileHandler::setFormat(std::string format) {
	std::replace(format.begin(), format.end(), ',', ' ');
	std::stringstream ss(format);
	std::string field;
	while (ss >> field) {
		char fieldId = fromString(field);//auto cast here
		if (fieldId != 0) {
			metadata[1]++;
			metadata[metadata[1]] = fieldId;
		}
	}
}

