/*
 * Field2.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#include "Field.h"
#include <string>

FieldType fromString(const std::string& field) {
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

	if (field == "sD")
		fieldId = SD;

	if (field == "d")
		fieldId = D;

	if (field == "dT")
		fieldId = DT;

	return fieldId;
}


