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

#include "../VLRegistries/Field.h"
#include "../VLRegistries/VLRegistry.h"

/*creates file handler for file in path. Opens and Reads metadata if mode==0.
 * Else creates new file.*/
FileHandler::FileHandler(std::string path,int mode)
:metadata(METADATA_SIZE){
	if(mode==0){
		fs.open(path.c_str(),std::ios_base::in | std::ios_base::out|std::ios::binary);
		fs.read(&metadata[0],METADATA_SIZE);//leo metadata
	}else{
		if(path.size())
			fs.open(path.c_str(),std::ios_base::in | std::ios_base::out|std::ios::binary|std::ios::trunc);
	}
	if(!fs || !fs.is_open())std::cout<<"file opening error"<<std::endl;
}


/*closes the file*/
FileHandler::~FileHandler() {
	saveMetadata();
	fs.close();
}

bool FileHandler::eof() {
	return fs.eof();
}

void FileHandler::saveMetadata() {
	fs.seekp(0);
	fs.write(&metadata[0], metadata.size());
}
