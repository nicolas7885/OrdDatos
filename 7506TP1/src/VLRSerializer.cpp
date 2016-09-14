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
#include "Field.h"

VLRSerializer::VLRSerializer() {}

VLRSerializer::~VLRSerializer() {}

/*adds to end of serializedData the serialized contents of reg
 * note: this has absolute coupling with VLRegistry*/
void VLRSerializer::serializeReg(std::vector<char>& serializedData, const VLRegistry& reg) {
	int numOfFields=reg.getNumOfFields();
	//todo error in case theres too many
	serializedData.push_back(numOfFields);

	for(int i=0; i<numOfFields; i++){
		Field field=reg.getField(i);
		std::vector<char> fieldData;
		const char* cp;
		long int dataSize=0;
		switch(field.type){
		case I1:
			char* cp2;
			cp2=&(field.value.i1);
			cp=cp2;
			dataSize=sizeof(char);
			break;
		case I2:
			short int* sip;
			sip=&(field.value.i2);
			cp=reinterpret_cast<char*>(sip);
			dataSize=sizeof(short int);
			break;
		case I4:
			int* ip;
			ip=&(field.value.i4);
			cp=reinterpret_cast<char*>(ip);
			dataSize=sizeof(int);
			break;
		case I8:
			long int* lip;
			lip=&(field.value.i8);
			cp=reinterpret_cast<char*>(lip);
			dataSize=sizeof(long int);
			break;
		case SD:
			cp=field.s.c_str();
			dataSize=strlen(cp);
			break;
		case SL:
			cp=field.s.c_str();
			dataSize=strlen(cp);
			//todo error in case its too long
			fieldData.push_back(dataSize);
			break;
		case D:
			cp=field.s.c_str();
			//todo error in case size is wrong
			dataSize=DATE_SIZE;
			break;
		case DT:
			cp=field.s.c_str();
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
}

/*adds to end of serialized data the serialized block*/
void VLRSerializer::serializeBlock(std::vector<char>& serializedData,
		const std::vector<VLRegistry>& data) {
	serializedData.push_back(data.size());
	for(uint i=0; i<data.size(); i++){
		serializeReg(serializedData,data[i]);
	}
}
