/*
 * Field2.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#include "Field.h"

#include <string>

FieldType Field::typeFromString(const std::string& field) {
	//todo error if none
	//todo use macros for each if, and to undo case sensitivity
	FieldType fieldId ;
	if (field == "i1")
		fieldId = I1;

	if (field == "i2")
		fieldId = I2;

	if (field == "i4")
		fieldId = I4;

	if (field == "i8")
		fieldId = I8;

	if (field == "sL")
		fieldId = SL;
	if (field == "sl")
		fieldId = SL;

	if (field == "sD")
		fieldId = SD;
	if (field == "sd")
		fieldId = SD;

	if (field == "d")
		fieldId = D;

	if (field == "dt")
		fieldId = DT;
	if (field == "dT")
		fieldId = DT;

	return fieldId;
}

bool Field::compareFields(Field& compared,Field& reference,CmpMode mode){
	if(compared.type!=reference.type)
		return false;

	switch(reference.type){
	case I1:
		return Field::compare(compared.value.i1,reference.value.i1,mode);
	case I2:
		return Field::compare(compared.value.i2,reference.value.i2,mode);
	case I4:
		return Field::compare(compared.value.i4,reference.value.i4,mode);
	case I8:
		return Field::compare(compared.value.i8,reference.value.i8,mode);
	case SD:
	case SL:
	case D:
	case DT:
		return compare(compared.s,reference.s,mode);
	default:
		return false;
	}
}
