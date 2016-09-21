//============================================================================
// Name        : 7506TP1.cpp
// Author      : nico
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "FileHandler.h"
#include <iostream>
using namespace std;

#define MODE_PARAM 1
#define MODE_NEW 1


int main(int argc, char* argv[]) {
	//test variables
	string format = "i1,i4,sD,d";
	const int cantRegAAgregar=1;
	//todo modes
	//todo use arguments
	vector<VLRegistry> block;
	FileHandler handler("testNew.bin", 1, format);
	for(int i=1; i<=cantRegAAgregar ; i++){
		//create reg
		VLRegistry reg(i,format);
		//set format
		Field field;
		field.value.i1=7;field.type=I1;
		reg.setField(1,field);
		field.value.i4=3;field.type=I4;
		reg.setField(2,field);
		field.s="AABBCCDD";field.type=SD;
		reg.setField(3,field);
		field.s="AABBCCCC";field.type=D;
		reg.setField(4,field);
		block.push_back(reg);
	}
	if(handler.write(block)<0) cout<<"overflow";
	block.clear();
	handler.read(block,0);
	//Field field=block[0].getField(0);
	//cout<<field.type<<": "<<field.value.i4;
	return 0;
}
