/*
 * Field2.h
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#ifndef FIELD_H_
#define FIELD_H_

#include <string>

#define DATE_SIZE 8
#define DATETIME_SIZE 15

enum FieldType{
	I1,
	I2,
	I4,
	I8,
	SL,
	SD,
	D,
	DT
};

enum CmpMode{
	GREATER,
	EQUAL,
	NOTEQUAL,
	LOWER
};

/*This is a little ugly class to handle fields,
 * you have to check its type and decide how to access it, horrible but simple*/
//todo change to class with methods to become each type
struct Field {
	union{
		char i1;
		short int i2;
		int i4;
		long int i8;
	} value;
	std::string s="";//I know its ugly but no boost allowed
	FieldType type;

	/*compares any two elements of type T that implement standard Comparison operators.*/
	template<typename T>
	static bool compare(T base,T reference,CmpMode mode){
		switch(mode){
		case GREATER:
			return base>reference;
		case EQUAL:
			return base==reference;
		case NOTEQUAL:
			return base!=reference;
		case LOWER:
			return base<reference;
		default:
			return false;
		}
	}

	static bool compareFields(Field& compared,Field& reference,CmpMode mode);

	static FieldType typeFromString(const std::string& field);
};
#endif /* FIELD_H_ */
