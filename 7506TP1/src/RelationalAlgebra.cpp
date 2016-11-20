/*
 * RelationalAlgebra.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: nicolas
 */

#include "RelationalAlgebra.h"

#include <algorithm>
#include <sstream>
#include <string>

#include "VLRegistries/Field.h"
#include "VLRegistries/VLRegistry.h"

RelationalAlgebra::RelationalAlgebra() {
	// TODO Auto-generated constructor stub

}

RelationalAlgebra::~RelationalAlgebra() {
	// TODO Auto-generated destructor stub
}

/*pre: input 1 and 2 opened and valid, have compatible format with output. Output opened and valid
 * post:reads all input 1 writing it into output. Then reads all input two and writes it into output.
 * Does not check for duplicates.*/
void RelationalAlgebra::unionOperator(FileHandler& input1, FileHandler& input2,
		FileHandler& output) {
	input1.restartBuffersToBeginning();
	VLRegistry reg;
	while(input1.readNext(reg)){
		output.writeNext(reg);
	}
	input2.restartBuffersToBeginning();
	while(input2.readNext(reg)){
		output.writeNext(reg);
	}
}

/* pre: input output open. Output has selection Fields compatible format.
 * selection fields is format including id. each pos is the in vector number of the field.
 * field 0 is the corresponding to the id of the reg. may repeat an input field number and/or commute them.
 * post:writes output with all the reg of input following selection Fields.*/
void RelationalAlgebra::projectionOperator(FileHandler& input,
		FileHandler& output, std::string selectionFields) {
	std::replace(selectionFields.begin(), selectionFields.end(), ',', ' ');
	input.restartBuffersToBeginning();
	VLRegistry oldReg;
	while(input.readNext(oldReg)){
		VLRegistry newReg;
		std::stringstream ss(selectionFields);
		for(uint fieldNumber, numberOfFields=0;ss >> fieldNumber;){
			if(fieldNumber<oldReg.getNumOfFields()){
				Field f=oldReg.getField(fieldNumber);
				newReg.addEmptyField(f.type);
				newReg.setField(numberOfFields++,f);
			}
		}
		if(newReg.getNumOfFields()){
			output.writeNext(newReg);
		}
	}
}

/* pre: input1, input2, output are valid open files.
 * Output can write the combination of any two registries.
 * (i.e. if block file, sum of registries size not bigger than block size)
 * post:for each registry in input 1, reads every registry in input2,
 *  and writes the "union" between both registries and writes it into output*/
void RelationalAlgebra::productOperator(FileHandler& input1,
		FileHandler& input2, FileHandler& output) {
	input1.restartBuffersToBeginning();
	VLRegistry reg1;
	while(input1.readNext(reg1)){
		input2.restartBuffersToBeginning();
		VLRegistry reg2;
		while(input2.readNext(reg2)){
			VLRegistry combination;
			for (uint i = 0; i < reg1.getNumOfFields(); i++) {
				Field field=reg1.getField(i);
				combination.addNewField(field);
			}
			for (uint i = 0; i < reg2.getNumOfFields(); i++) {
				Field field=reg2.getField(i);
				combination.addNewField(field);
			}
			output.writeNext(combination);
		}
	}
}

bool RelationalAlgebra::compare(const VLRegistry &reg,condition_t condition){

	if(reg.getNumOfFields()<condition.pos)
		return false;

	Field compared=reg.getField(condition.pos);
	Field reference=condition.value;

	return Field::compareFields(compared,reference,condition.mode);
}

/*pre: input and output open and valid. They both have same format
 * filter takes a field and determines if it goes or not */
void RelationalAlgebra::selectionOperator(FileHandler& input, FileHandler& output,
		condition_t condition){
	input.restartBuffersToBeginning();
	VLRegistry reg;
	while(input.readNext(reg)){
		if(compare(reg,condition))
			output.writeNext(reg);
	}
}

#include "Index/BPlusTree.h"

void RelationalAlgebra::buildIndex(FileHandler& input2,BPlusTree& bTree) {
	input2.restartBuffersToBeginning();
	uint regPos=input2.tellg();
	VLRegistry reg;
	while (input2.readNext(reg)) {
		Field f = reg.getField(0);
		pair_t p = { regPos, f.value.i4 };
		bTree.insert(p);
		regPos=input2.tellg();
	}
}

/*pre: input 1 and 2 opened and valid, have compatible format with output.
 * Input2 must have unique id's for the index. Else 2nd reg with same id replaces 1st.
 * post: Puts into output all the reg of input1 that are not in input2. Equality determined by id.*/
void RelationalAlgebra::differenceOperator(FileHandler& input1,
		FileHandler& input2, FileHandler& output) {
	/*todo difference
	 * idea: build a primary, exhaustive, index*/
	BPlusTree bTree("tempIndex.bin");
	buildIndex(input2,bTree);
	//check no equal reg in dif for each
	input1.restartBuffersToBeginning();
	VLRegistry reg;
	while(input1.readNext(reg)){
		bool shouldInclude=true;
		Field f=reg.getField(0);
		uint relPos;
		if(bTree.find(f.value.i4,relPos)){
			VLRegistry reg2;
			if(input2.get(relPos,f.value.i4,reg2) && reg==reg2)
				shouldInclude=false;
		}
		if(shouldInclude)
			output.writeNext(reg);
	}
}

//todo agrupation
//todo wierd union
//todo intersection
