/*
 * VLRUnserializer.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#include "VLRUnserializer.h"

#include <vector>

#include "Field.h"
#include "VLRegistry.h"

VLRUnserializer::VLRUnserializer(std::vector<FieldType> &format)
:format(format){
	this->format.insert(this->format.begin(),I4);//adds id to the format
}

VLRUnserializer::~VLRUnserializer() {}

/*pre: serializedData is a registry with the same format as the unserializer.
 * output has same format as unserializer
 * post:takes the serialized registry and turns it into a VLRegistry according to the specified format
 * of the unserializer. dataIt points to last pos read*/
void VLRUnserializer::unserializeReg(VLRegistry &output,dataIt_t &dataIt) {
	initializeRegAsEmpty(output);
	//todo exceptions
	for(uint i=0; i<format.size(); i++){
		Field field;
		field.type=format[i];
		char const * cp;
		switch(field.type){
		case I1:
			field.value.i1=*dataIt;
			dataIt++;
			break;
		case I2:{
			cp=&(*dataIt);//todo check if it works
			dataIt+=sizeof(short int);
			short int const* sip;
			sip=reinterpret_cast<short int const*>(cp);
			field.value.i2=*sip;
			break;
		}
		case I4:{
			cp=&(*dataIt);//todo check if it works
			dataIt+=sizeof(int);
			int const* ip;
			ip=reinterpret_cast<int const*>(cp);
			field.value.i4=*ip;
			break;
		}
		case I8:{
			cp=&(*dataIt);//todo check if it works
			dataIt+=sizeof(long int);
			long int const* lip;
			lip=reinterpret_cast<long int const*>(cp);
			field.value.i8=*lip;
			break;
		}
		case SD:
			while(*dataIt != 0){
				field.s+=*dataIt;
				dataIt++;
			}
			dataIt++;//avoid null character
			break;
		case SL:{
			char length;
			length=*dataIt;
			dataIt++;
			for(int j=1; j<=length; j++){
				field.s+=*dataIt;
				dataIt++;
			}
			break;
		}
		case D:{
			for(int j=1; j<=DATE_SIZE; j++){
				field.s+=*dataIt;
				dataIt++;
			}
			break;
		}
		case DT:{
			for(int j=1; j<=DATETIME_SIZE; j++){
				field.s+=*dataIt;
				dataIt++;
			}
			break;
		}
		}
		output.setField(i,field);
	}
}

void VLRUnserializer::initializeRegAsEmpty(VLRegistry& newReg) {
	for (uint j = newReg.getNumOfFields(); j < format.size(); j++) {
		newReg.addEmptyField(format[j]);
	}
}

void VLRUnserializer::reziseBlock(uint newSize, std::vector<VLRegistry>& block) {
	VLRegistry newReg;
	block.resize(newSize, newReg);
}

/*pre: serializedData has the format indicated for unserializer.
 * post: fills block with the unserialized registries*/
void VLRUnserializer::unserializeBlock(std::vector<VLRegistry>& block,
		const std::vector<char>& serializedData) {
	reziseBlock((unsigned char)serializedData[0], block);
	dataIt_t dataIt=serializedData.begin();
	dataIt++;//avoid num of reg
	for(int i=0; i<serializedData[0]; i++){
		unserializeReg(block[i],dataIt);
	}
}
