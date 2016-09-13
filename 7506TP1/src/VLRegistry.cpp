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

VLRegistry::VLRegistry(int id, std::string format){
	addId(id);
	std::replace(format.begin(), format.end(), ',', ' ');
	std::stringstream ss(format);
	std::string fieldType;
	while (ss >> fieldType) {
		addEmptyField(fromString(fieldType));
	}
}

VLRegistry::~VLRegistry() {
	// TODO Auto-generated destructor stub
}

Field VLRegistry::getField(int id) const{
	//todo check bounds
	return fields[id];
}

void VLRegistry::setField(int id, const Field& field) {
	//todo bounds check
	fields[id]=field;
}

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
		field.value.sD = "";
		break;
	case SL:
		field.value.sL = "";
		break;
	case D:
		field.value.d = "";
		break;
	case DT:
		field.value.dt = "";
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

int VLRegistry::getNumOfFields() const{
	return fields.size();
}
