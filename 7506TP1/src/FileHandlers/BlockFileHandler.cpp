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
BlockFileHandler::BlockFileHandler(std::string path, uint bSize)
:FileHandler(path,1),
 bSize(bSize),
 byteMap(blockSizeInBytes()),
 currRelPos(0){
	restartBuffersToBeginning();
	metadata[METADATA_BSIZE_POS]=bSize;
	saveMetadata();
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
int BlockFileHandler::write(const std::vector<char> &data) {
	int relPos=0;
	std::vector<char>::iterator freeBlockSpaceIt=byteMap.begin();
	for(; freeBlockSpaceIt!=byteMap.end() && *freeBlockSpaceIt!=0; freeBlockSpaceIt++){
		relPos++;
	}
	if(freeBlockSpaceIt==byteMap.end()){
		return -1;//no free space
	}else{
		return writeBin(relPos, data);
	}
}

/*if out of bounds returns-2, else calls writeBin*/
int BlockFileHandler::write(const std::vector<char> &data, uint relPos) {
	if(relPos>=byteMap.size()) return -2;//bounds check
	return writeBin(relPos, data);
}

/*attempts to read the next block and put the information into data.
 * Does nothing if EOF is reached before adding anything to data.
 * If EOF is reached stops reading. Also reads empty blocks*/
void BlockFileHandler::read(std::vector<char>& data) {
	if(!fs.eof()){
		data.resize(blockSizeInBytes());
		fs.read(&data[0],blockSizeInBytes());//todo error if eof
		currRelPos++;
	}
}

/*pre: relPos is in map, and its valid
 * post:reads the block at the position given
 * returns num of bytes read that hold data*/
uint BlockFileHandler::read(std::vector<char>& data, uint relPos) {
	if(relPos<byteMap.size() && byteMap[relPos]!=0){
		//		fs.seekp(calculateOffset(relPos));//todo should be seekg only
		fs.seekg(calculateOffset(relPos));
		currRelPos=relPos;
		this->read(data);
		uint bytesRead=* reinterpret_cast<unsigned int*>(&data[0]);
		data.erase(data.begin(),data.begin()+sizeof(bytesRead));
		return bytesRead;
	}
	return 0;
}

/*attempts to write the reg into current block. Must be single reg.
 * If overflow, goes to next block.
 * Does nothing and returns -1 if EOF is reached before writing.
 * If write is succesful returns num of block where it ended in.
 * Does not attempt to write into almost full blocks(BLOCK_CHARGE_PERCENTAGE)*/
ulint BlockFileHandler::writeNext(const std::vector<char> & reg){
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
		std::vector<char> block;
		uint size=this->read(block,relPos);
		block.resize(size);//recover size whithout padding
		//update number of reg in block
		if(block.size()){
			block[0]=block[0]+1;
		}else{
			block.push_back(1);
		}
		block.insert(block.end(),reg.begin(),reg.end());//append data
		if(this->write(block,relPos)==0)
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
 * Does nothing and returns false if EOF is reached before reading a block into data
 * If EOF is reached stops reading, and returns true.*/
bool BlockFileHandler::readNext(std::vector<char>& data) {
	//find next not-empty block
	while(!this->eof() && byteMap[currRelPos]==0){
		currRelPos++;
	}
	if(!this->eof()){
		//get next block
		this->read(data,currRelPos);
		return true;
	}else{
		return false;
	}
}

/*pre: relPos is in byteMap and is valid.note: first pos is 0
 * post: the block is replaced by 0. byteMap corrected*/
void BlockFileHandler::deleteBlock(uint relPos) {
	if(relPos<byteMap.size() && byteMap[relPos]){
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
	uint size=data.size();
	if(size+sizeof(int)>blockSizeInBytes())
		return -1;
	//update byte map
	long int percentage=size*100;
	percentage/=blockSizeInBytes();
	if(percentage==0 && size!=0) percentage=1;//if its almost empty still mark as occupied
	byteMap[relPos]=(char) percentage;
	//prepare data
	std::vector<char> block(blockSizeInBytes());//to get 0 filled vector
	char* cp=reinterpret_cast<char*>(&size);
	std::copy(cp,cp+sizeof(size),block.begin());//add size of data
	std::copy(data.begin(),data.end(),block.begin()+sizeof(size));
	//write and update
	fs.seekp(calculateOffset(relPos));
	fs.write(&block[0], blockSizeInBytes());
	if(!fs){
		std::cout<<"error writing"<<std::endl;
	}else{
		rewriteByteMap();
		currRelPos=relPos+1;
	}
	return 0;
}

uint BlockFileHandler::blockSizeInBytes() {
	uint blockSizeInBytes = 0x01;
	blockSizeInBytes <<= bSize;
	blockSizeInBytes *= CHUNK_SIZE;
	return blockSizeInBytes;
}

/*returns relPos of get pointer*/
uint BlockFileHandler::tellg() {
	if(!currRelPos)
		return currRelPos;
	else
		return currRelPos-1;
}

void BlockFileHandler::rewriteByteMap() {
	fs.seekp(METADATA_SIZE);
	fs.write(&byteMap[0], blockSizeInBytes());
}


void BlockFileHandler::restartBuffersToBeginning() {
	currRelPos = 0;
}
