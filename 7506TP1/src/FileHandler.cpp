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
#include <string>
#include <sstream>
#include <algorithm>
#include "VLRegistry.h"
#include "VLRSerializer.h"
#include "VLRUnserializer.h"
#include "Field.h"

#define CHUNK_SIZE 512
#define METADATA_SIZE CHUNK_SIZE
#define BLOCK_CHARGE_PERCENTAGE 80

/*creates file handler for file in path. Reads metadata and byteMap.
 * */
FileHandler::FileHandler(std::string path)
:metadata(METADATA_SIZE),
 fs(path){
	if(!fs || !fs.is_open())std::cout<<"file openeing error"<<std::endl;
	restartBuffersToBeginning();
	fs.read(&metadata[0],METADATA_SIZE);//leo metadata
	bSize=metadata[0];
	byteMap.resize(blockSizeInBytes());
	fs.read(&byteMap[0],(int)blockSizeInBytes());//leo mapa bytes
}

/*pre: path is a valid path, bSize is block size, between 1 and 4, format is valid
 * post: creates and opens new block file at path, with blocks of 2^bSize *512 bytes
 * 	with the given format, and an empty bit-map. If file exists, its overriden
 * 	Available blocks in bitmap depend on number of fields in format and bSize */
FileHandler::FileHandler(std::string path, uint bSize, std::string format)
:bSize(bSize),
 metadata(METADATA_SIZE),
 byteMap(blockSizeInBytes()),
 fs(path.c_str(),std::ios_base::in | std::ios_base::out|std::ios::binary|std::ios::trunc){
	if(!fs || !fs.is_open())std::cout<<"file creation error"<<std::endl;
	restartBuffersToBeginning();
	metadata[0]=bSize;
	setFormat(format);

	fs.write(&metadata[0], metadata.size());
	rewriteByteMap();//initializes byteMap as empty
	fs.seekp(calculateOffset(0),std::ios_base::beg);
}

FileHandler::~FileHandler() {
	fs.close();
}

/*attempts to write data into a single block.
 * If there is no more space available returns -1.
 * If possible, finds first empty block and writes it there,
 * then updates the byte map, and returns 0.*/
int FileHandler::write(const std::vector<VLRegistry> &data) {
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
int FileHandler::write(const std::vector<VLRegistry> &data, uint relPos) {
	if(relPos>=byteMap.size()) return -2;//bounds check
	std::vector<char> serializedData;
	VLRSerializer serializer;
	serializer.serializeBlock(serializedData,data);
	return writeBin(relPos, serializedData);
}

/*attempts to read the next block and put the information into data.
 * Does nothing if EOF is reached before adding anything to data.
 * If EOF is reached stops reading. Also reads empty blocks*/
void FileHandler::read(std::vector<VLRegistry>& data) {
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
void FileHandler::read(std::vector<VLRegistry>& data, uint relPos) {
	if(relPos<byteMap.size() && byteMap[relPos]!=0){
		fs.seekp(calculateOffset(relPos),std::ios_base::beg);
		currRelPos=relPos;
		this->read(data);
	}
}

/*attempts to write the reg into current block. If overflow, goes to next block.
 * Does nothing and returns -1 if EOF is reached before writing.
 * If write is succesful returns num of block where it ended in.
 * Does not attempt to write into almost full blocks(BLOCK_CHARGE_PERCENTAGE)*/
int FileHandler::writeNext(VLRegistry & reg){
	bool notWritten=true;
	uint relPos=currRelPos;
//	while(byteMap[currRelPos]>BLOCK_CHARGE_PERCENTAGE){
//		currRelPos++;
//		if(currRelPos>=byteMap.size())
//			return -1;
//	}//avoid almost full blocks
	while(notWritten && !this->eof()){
		if(currRelPos>0){
			relPos=currRelPos-1;//try last block used
		}else{
			relPos=currRelPos;//if first then try there
		}
		std::vector<VLRegistry> block;
		read(block,relPos);
		block.push_back(reg);
		if(write(block,relPos)==0)
			notWritten=false;
	}
	if(notWritten)
		return -1;
	else
		return relPos;
}

/*attempts to read the next non empty block and put the information into data.
 * Does nothing and returns false if EOF is reached before adding anything to data
 * If EOF is reached stops reading, and returns true.*/
bool FileHandler::readNext(VLRegistry& reg) {
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
void FileHandler::deleteBlock(uint relPos) {
	if(byteMap[relPos]){
		std::vector<char> emptyData;
		writeBin(relPos,emptyData);
	}
}

/*pre:valid file opened.
 * post: returns the current format of the file as csv string*/
std::string FileHandler::getFormatAsString() {
	std::stringstream format;
	for(int i=1; i<=metadata[1]; i++){
		switch(metadata[1+i]){
		case I1:
			format<<"i1";
			break;
		case I2:
			format<<"i2";
			break;
		case I4:
			format<<"i4";
			break;
		case I8:
			format<<"i8";
			break;
		case SD:
			format<<"sD";
			break;
		case SL:
			format<<"sL";
			break;
		case D:
			format<<"d";
			break;
		case DT:
			format<<"dT";
			break;
		}
		if(i<metadata[1])format<<",";
	}
	return format.str();
}

/*pre: valid file opened.
 * post: creates or overrides outputPath with a csv containing the registries.
 * Each registry occupies one line, with each field separated by a coma.*/
void FileHandler::toCsv(std::string outputPath) {
	std::fstream output(outputPath.c_str(),std::ios_base::out|std::ios::trunc);
	restartBuffersToBeginning();
	VLRegistry reg;
	while(this->readNext(reg)){
		regToCsv(reg, output);
	}
	output.close();
}

/*pre: valid file opened. csv has same format as handler
 * sourcePath is a csv file created by a handler of this same program, or respects its format
 * post: creates or overrides outputPath with a binary file containing the registries,
 * sets the metadata accordingly.*/
void FileHandler::fromCsv(std::string sourcePath) {
	std::fstream input(sourcePath.c_str(),std::ios_base::in);//open csv
	restartBuffersToBeginning();
	for(std::string line; std::getline(input,line);){
		std::replace(line.begin(), line.end(), ',', ' ');
		//todo accept string with whitespaces and commas
		std::stringstream ss(line);
		int id; ss>>id;
		VLRegistry reg(id,getFormatAsString());
		std::vector<FieldType> format=getFormatAsTypes();
		std::vector<FieldType>::iterator it=format.begin();
		int i=1;
		for(; it!=format.end(); it++){
			Field field;
			field.type=*it;
			switch(*it){
			case I1:
				ss >> field.value.i1;
				break;
			case I2:
				ss >> field.value.i2;
				break;
			case I4:
				ss >> field.value.i4;
				break;
			case I8:
				ss >> field.value.i8;
				break;
			case SD:
			case SL:
			case D:
			case DT:
				ss >> field.s;
				break;
			}
			reg.setField(i,field);
			i++;
		}
		writeNext(reg);
	}
	input.close();
}

bool FileHandler::eof() {
	if(currRelPos<byteMap.size())
		return fs.eof();
	else
		return true;
}

/******************************************private*********************************************/
void FileHandler::restartBuffersToBeginning() {
	bufferPos = 0;
	currRelPos = 0;
	readBuffer.clear();
}


/*attempts to write data into the specified block.
 * if overflow returns -1,if succesful returns 0*/
int FileHandler::writeBin(uint relPos,const std::vector<char>& data) {
	if(!fs)std::cout<<"file broken??"<<std::endl;
	if(data.size()>blockSizeInBytes())
		return -1;

	long int percentage=data.size()*100;
	percentage/=blockSizeInBytes();//something wrong in percentage
	if(percentage==0) percentage=1;//if its almost empty still mark as occupied
	byteMap[relPos]=(char) percentage;

	fs.seekg(calculateOffset(relPos), std::ios_base::beg);
	std::vector<char> block(blockSizeInBytes());//to get 0 filled vector
	std::copy(data.begin(),data.end(),block.begin());
	fs.write(&block[0], blockSizeInBytes());
	if(!fs)std::cout<<"error writing"<<std::endl;
	rewriteByteMap();
	currRelPos=relPos+1;
	return 0;
}

uint FileHandler::blockSizeInBytes() {
	uint blockSizeInBytes = 0x01;
	blockSizeInBytes <<= bSize;
	blockSizeInBytes *= CHUNK_SIZE;
	return blockSizeInBytes;
}

void FileHandler::rewriteByteMap() {
	fs.seekg(METADATA_SIZE,std::ios_base::beg);
	fs.write(&byteMap[0], blockSizeInBytes());
}

long unsigned int FileHandler::calculateOffset(uint relPos) {
	long unsigned int offset = metadata.size()+ byteMap.size();
	offset += relPos * blockSizeInBytes();
	return offset;
}

void FileHandler::setFormat(std::string format) {
	std::replace(format.begin(), format.end(), ',', ' ');
	std::stringstream ss(format);
	std::string field;
	while (ss >> field) {
		char fieldId = typeFromString(field);//auto cast here
		metadata[1]++;
		metadata[metadata[1]+1] = fieldId;
	}
}

std::vector<FieldType> FileHandler::getFormatAsTypes() {
	std::vector<FieldType> types;
	for(int i=0; i<metadata[1]; i++){
		types.push_back(static_cast<FieldType>(metadata[2+i]));
	}
	return types;
}

void FileHandler::regToCsv(VLRegistry &reg, std::fstream& output) {
	for (int j = 0; j < reg.getNumOfFields(); j++) {
		Field field = reg.getField(j);
		switch (field.type) {
		case I1:
			output << field.value.i1;
			break;
		case I2:
			output << field.value.i2;
			break;
		case I4:
			output << field.value.i4;
			break;
		case I8:
			output << field.value.i8;
			break;
		case SD:
		case SL:
		case D:
		case DT:
			output << field.s;
			break;
		}
		if (j < reg.getNumOfFields() - 1)
			output << ',';
	}
	output<<std::endl;
}
