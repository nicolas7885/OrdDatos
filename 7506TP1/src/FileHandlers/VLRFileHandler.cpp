/*
 * FileHandler.cpp
 *
 *  Created on: Sep 8, 2016
 *      Author: nicolas
 */

#include "VLRFileHandler.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>

#include "FileHandler.h"
#include "../VLRegistries/Field.h"
#include "../VLRegistries/VLRegistry.h"

VLRFileHandler::VLRFileHandler(FileHandler* file)
:fileP(file){}

VLRFileHandler::~VLRFileHandler() {
}

/*pre:valid file opened.
 * post: returns the current format of the file as csv string*/
std::string VLRFileHandler::getFormatAsString() {
	std::stringstream format;
	for(int i=1; i<=fileP->metadata[FORMAT_SIZE_POS]; i++){
		switch(fileP->metadata[FORMAT_SIZE_POS+i]){
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
		if(i<fileP->metadata[FORMAT_SIZE_POS])format<<",";
	}
	return format.str();
}

/*pre: valid file opened.
 * post: creates or overrides outputPath with a csv containing the registries.
 * Each registry occupies one line, with each field separated by a coma.
 * At the end of the method restars file stream*/
void VLRFileHandler::toCsv(std::string outputPath) {
	std::fstream output(outputPath.c_str(),std::ios_base::out|std::ios::trunc);
	this->restartBuffersToBeginning();
	VLRegistry reg;
	while(this->readNext(reg)){
		regToCsv(reg, output);
	}
	output.close();
	this->restartBuffersToBeginning();
}

/*pre: valid file opened. csv has same format as handler
 * sourcePath is a csv file created by a handler of this same program, or respects its format
 * post: creates or overrides outputPath with a binary file containing the registries,
 * sets the metadata accordingly.At the end of the method restars file stream*/
void VLRFileHandler::fromCsv(std::string sourcePath) {
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
		this->writeNext(reg);
	}
	input.close();
	this->restartBuffersToBeginning();
}

bool VLRFileHandler::eof() {
	return fileP->eof();
}

/******************************************protected*********************************************/
void VLRFileHandler::setFormat(std::string format) {
	std::replace(format.begin(), format.end(), ',', ' ');
	std::stringstream ss(format);
	std::string field;
	while (ss >> field) {
		char fieldId = Field::typeFromString(field);//auto cast here
		fileP->metadata[FORMAT_SIZE_POS]++;
		fileP->metadata[fileP->metadata[FORMAT_SIZE_POS]+1] = fieldId;
	}
	fileP->saveMetadata();
}

std::vector<FieldType> VLRFileHandler::getFormatAsTypes() {
	std::vector<FieldType> types;
	for(int i=0; i<fileP->metadata[FORMAT_SIZE_POS]; i++){
		types.push_back(static_cast<FieldType>(fileP->metadata[2+i]));
	}
	return types;
}

void VLRFileHandler::regToCsv(VLRegistry &reg, std::fstream& output) {
	for (uint j = 0; j < reg.getNumOfFields(); j++) {
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
