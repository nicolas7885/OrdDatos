/*
 * tests.h
 *
 *  Created on: Sep 22, 2016
 *      Author: nicolas
 */

#ifndef SRC_TESTS_H_
#define SRC_TESTS_H_

#include "FileHandler.h"
#include "VLRegistry.h"
#include <iostream>

using namespace std;

void runTests() {
	//todo modes
	//todo use arguments
	//test variables
	string format = "i1,i2,i4,sD,d,dT";
	const int cantRegAAgregar = 5;
	//todo test open
	vector<VLRegistry> block;
	FileHandler handler("test.bin", 1, format);
	for (int i = 1; i <= cantRegAAgregar; i++) {
		//create reg
		VLRegistry reg(i, format);
		//set format
		Field field;
		field.value.i1 = 127;
		field.type = I1;
		reg.setField(1, field);
		field.value.i2 = 512;
		field.type = I2;
		reg.setField(2, field);
		field.value.i4 = 1024;
		field.type = I4;
		reg.setField(3, field);
		field.s = "AABBCCDD";
		field.type = SD;
		reg.setField(4, field);
		field.s = "aaaammdd";
		field.type = D;
		reg.setField(5, field);
		field.s = "aaaammdd-hhmmss";
		field.type = DT;
		reg.setField(6, field);
		block.push_back(reg);
	}
	if (handler.write(block) < 0)
		cout << "overflow";
}



#endif /* SRC_TESTS_H_ */
