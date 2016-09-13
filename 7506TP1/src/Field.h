/*
 * Field2.h
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#ifndef FIELD_H_
#define FIELD_H_

#define DATE_SIZE 8
#define DATETIME_SIZE 15

union FieldValue{
	char i1;
	short int i2;
	int i4;
	long int i8;
	std::string sD;
	std::string sL;
	std::string d;
	std::string dt;
};

enum FieldType{I1,I2,I4,I8,SL,SD,D,DT};
FieldType fromString(const std::string& field);

struct Field {
	FieldValue value;
	FieldType type;
};

#endif /* FIELD_H_ */
