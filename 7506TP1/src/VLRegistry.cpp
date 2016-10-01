/*
 * VLRegistry.cpp
 *
 *  Created on: Sep 8, 2016
 *      Author: nicolas
 */

#include "VLRegistry.h"

#include <algorithm>
#include <sstream>
#include "Field.h"

/*creates a registry with specified format, and id at field 0*/
VLRegistry::VLRegistry(int id, std::string format){
	addId(id);
	std::replace(format.begin(), format.end(), ',', ' ');
	std::stringstream ss(format);
	std::string fieldType;
	while (ss >> fieldType) {
		addEmptyField(typeFromString(fieldType));
	}
}

/*creates an empty registry, with no fields*/
VLRegistry::VLRegistry() {}

VLRegistry::~VLRegistry() {}

/* pre: there are at least id+1 fields
 * post:gets the field at id position*/
Field VLRegistry::getField(int id) const{
	//todo check bounds
	return fields[id];
}

/* pre: there are at least id+1 fields
 * post: sets the field at id position as field&*/
void VLRegistry::setField(int id, const Field& field) {
	//todo bounds check
	fields[id]=field;
}

/*adds a specified field with indicated type at the end of the registry.*/
void VLRegistry::addEmptyField(FieldType type) {
	Field field;
	field.type=type;
	switch (type) {
	case I1:
		field.value.i1 = 0;
		break;
	case I2:
		field.value.i2 = 0;
		break;
	case I4:
		field.value.i4 = 0;
		break;
	case I8:
		field.value.i8 = 0;
		break;
	case SD:
	case SL:
	case D:
	case DT:
		field.value.i8=0;
		field.s = "";
		break;
	}
	fields.push_back(field);
}

void VLRegistry::addId(int id) {
	Field field;
	field.type = I4;
	field.value.i4 = id;
	fields.push_back(field);
}

uint VLRegistry::getNumOfFields() const{
	return fields.size();
}


