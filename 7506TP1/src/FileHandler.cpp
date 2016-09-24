/*
 * FileHandler.cpp
 *
 *  Created on: Sep 8, 2016
 *      Author: nicolas
 */

#include "FileHandler.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include "Field.h"
#include "VLRegistry.h"

/*creates file handler for file in path. Reads metadata.
 * */
FileHandler::FileHandler(std::string path)
:metadata(METADATA_SIZE),
fs(path),
currRelPos(0){
	if(!fs || !fs.is_open())std::cout<<"file openeing error"<<std::endl;
	fs.read(&metadata[0],METADATA_SIZE);//leo metadata
}

/*pre: path is a valid path, format is valid
 * post: creates and opens new file at path, with the given format.
 * If file exists, its overriden. Derivate class should write metadata into file*/
FileHandler::FileHandler(std::string path, std::string format)
:metadata(METADATA_SIZE),
 fs(path.c_str(),std::ios_base::in | std::ios_base::out|std::ios::binary|std::ios::trunc){
	if(!fs || !fs.is_open())std::cout<<"file creation error"<<std::endl;
	setFormat(format);
	//make sure derivate writes metadata!!
}

/*closes the file*/
FileHandler::~FileHandler() {
	fs.close();
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
	this->restartBuffersToBeginning();
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
	this->restartBuffersToBeginning();
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
	return fs.eof();
}

/******************************************private*********************************************/
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
