/*
 * VLRSerializer.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#include "VLRSerializer.h"

#include <vector>
#include <cstring>
#include "VLRegistry.h"

#define DATE_SIZE 8
#define DATETIME_SIZE 15

VLRSerializer::VLRSerializer() {}

VLRSerializer::~VLRSerializer() {}

std::vector<char> VLRSerializer::serializeReg(const VLRegistry& reg) {
	std::vector<char> serializedData;
	int numOfFields=reg.getNumOfFields();
	//todo error in case theres too many
	serializedData.push_back(numOfFields);

	for(int i=0; i<numOfFields; i++){
		Field field=reg.getField(i);
		std::vector<char> fieldData;
		char* cp;
		long int dataSize=0;
		switch(field.type){
		case I1:
			char* cp2=&(field.value.i1);
			cp=cp2;
			dataSize=sizeof(char);
			break;
		case I2:
			short int* sip=&(field.value.i2);
			cp=reinterpret_cast<char*>(sip);
			dataSize=sizeof(short int);
			break;
		case I4:
			int* ip=&(field.value.i4);
			cp=reinterpret_cast<char*>(ip);
			dataSize=sizeof(int);
			break;
		case I8:
			long int* lip=&(field.value.i8);
			cp=reinterpret_cast<char*>(lip);
			dataSize=sizeof(long int);
			break;
		case SD:
			cp=field.value.sD.c_str();
			dataSize=strlen(cp);
			break;
		case SL:
			cp=field.value.sL.c_str();
			dataSize=strlen(cp);
			//todo error in case its too long
			fieldData.push_back(dataSize);
			break;
		case D:
			cp=field.value.d.c_str();
			//todo error in case size is wrong
			dataSize=DATE_SIZE;
			break;
		case DT:
			cp=field.value.dt.c_str();
			//todo error in case size is wrong
			dataSize=DATETIME_SIZE;
			break;
		}
		for(int j=0; j<dataSize; j++){
			fieldData.push_back(*cp);
			cp++;
		}
		serializedData.insert(serializedData.end(),fieldData.begin(),fieldData.end());
	}
	return serializedData;
}
