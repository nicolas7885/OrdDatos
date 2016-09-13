/*
 * VLRUnserializer.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#include "VLRUnserializer.h"
#include <vector>
#include "Field.h"

VLRUnserializer::VLRUnserializer(std::vector<FieldType> format)
:format(format){
	this->format.insert(format.begin(),I4);//adds id to the format
}

VLRUnserializer::~VLRUnserializer() {}

/*pre: serializedData is a registry with the same format as the unserializer.
 * output has same format as unserializer
 * post:takes the serialized registry and turns it into a VLRegistry according to the specified format
 * of the unserializer*/
void VLRUnserializer::unserializeReg(VLRegistry &output, std::vector<char> serializedData) {
	//todo exceptions
	std::vector<char>::iterator dataIt=serializedData.begin();
	for(int i=0; i<format.size(); i++){
		Field field;
		field.type=format[i];
		char* cp;
		switch(field.type){
		case I1:
			field.value.i1=*dataIt;
			dataIt++;
			break;
		case I2:
			cp=&(*dataIt);//todo check if it works
			dataIt+=sizeof(short int);
			short int* sip=reinterpret_cast<short int*>(cp);
			field.value.i2=*sip;
			break;
		case I4:
			cp=&(*dataIt);//todo check if it works
			dataIt+=sizeof(int);
			int* ip=reinterpret_cast<char*>(cp);
			field.value.i4=*ip;
			break;
		case I8:
			cp=&(*dataIt);//todo check if it works
			dataIt+=sizeof(long int);
			long int* lip=reinterpret_cast<long int*>(cp);
			field.value.i8=*lip;
			break;
		case SD:
			field.value.sD=std::string();
			while(*dataIt != 0){
				field.value.sD+=*dataIt;
				dataIt++;
			}
			dataIt++;//avoid null character
			break;
		case SL:
			char length=*dataIt;
			dataIt++;
			field.value.sL=std::string();
			for(int i=1; i<=length; i++){
				field.value.sL+=*dataIt;
				dataIt++;
			}
			break;
		case D:
			field.value.d=std::string();
			for(int i=1; i<=DATE_SIZE; i++){
				field.value.d+=*dataIt;
				dataIt++;
			}
			break;
		case DT:
			//todo check if this works, needs change, or can be reduced
			field.value.dt=*dataIt;
			dataIt++;
			for(int i=1; i<=DATETIME_SIZE; i++){
				field.value.dt+=*dataIt;
				dataIt++;
			}
			break;
		}
		output.setField(i,field);
	}
}
