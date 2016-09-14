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
	for(uint i=0; i<format.size(); i++){
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
			short int* sip;
			sip=reinterpret_cast<short int*>(cp);
			field.value.i2=*sip;
			break;
		case I4:
			cp=&(*dataIt);//todo check if it works
			dataIt+=sizeof(int);
			int* ip;
			ip=reinterpret_cast<int*>(cp);
			field.value.i4=*ip;
			break;
		case I8:
			cp=&(*dataIt);//todo check if it works
			dataIt+=sizeof(long int);
			long int* lip;
			lip=reinterpret_cast<long int*>(cp);
			field.value.i8=*lip;
			break;
		case SD:
			while(*dataIt != 0){
				field.s+=*dataIt;
				dataIt++;
			}
			dataIt++;//avoid null character
			break;
		case SL:
			char length;
			length=*dataIt;
			dataIt++;
			for(int i=1; i<=length; i++){
				field.s+=*dataIt;
				dataIt++;
			}
			break;
		case D:
			for(int i=1; i<=DATE_SIZE; i++){
				field.s+=*dataIt;
				dataIt++;
			}
			break;
		case DT:
			for(int i=1; i<=DATETIME_SIZE; i++){
				field.s+=*dataIt;
				dataIt++;
			}
			break;
		}
		output.setField(i,field);
	}
}
