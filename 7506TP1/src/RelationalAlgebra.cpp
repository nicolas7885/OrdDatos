/*
 * RelationalAlgebra.cpp
 *
 *  Created on: Sep 27, 2016
 *      Author: nicolas
 */

#include "RelationalAlgebra.h"

#include <algorithm>
#include <sstream>

#include "Index/BNode.h"
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
void RelationalAlgebra::unionOperator(VLRFileHandler& input1, VLRFileHandler& input2,
		VLRFileHandler& output) {
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
void RelationalAlgebra::projectionOperator(VLRFileHandler& input,
		VLRFileHandler& output, std::string selectionFields) {
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
void RelationalAlgebra::productOperator(VLRFileHandler& input1,
		VLRFileHandler& input2, VLRFileHandler& output) {
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
void RelationalAlgebra::selectionOperator(VLRFileHandler& input, VLRFileHandler& output,
		condition_t condition){
	input.restartBuffersToBeginning();
	VLRegistry reg;
	while(input.readNext(reg)){
		if(compare(reg,condition))
			output.writeNext(reg);
	}
}

#include "Index/BPlusTree.h"

void RelationalAlgebra::buildIndex(VLRFileHandler& input2,BPlusTree& bTree) {
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
void RelationalAlgebra::differenceOperator(VLRFileHandler& input1,
		VLRFileHandler& input2, VLRFileHandler& output) {
	BPlusTree bTree("tempIndex.bin");
	buildIndex(input2,bTree);
	//check no equal reg in dif for each
	input1.restartBuffersToBeginning();
	VLRegistry reg;
	while(input1.readNext(reg)){
		bool shouldInclude=true;
		Field f=reg.getField(0);
		uint relPos; VLRegistry reg2;
		if(bTree.find(f.value.i4,relPos) && input2.get(relPos,f.value.i4,reg2) && reg==reg2){
				shouldInclude=false;
		}
		if(shouldInclude)
			output.writeNext(reg);
	}
}

/*pre: input 1 and 2 open and valid. Output has appropiate format.
 * input 2 has unique id corresponding to (external) id stored at fieldNumber of input 1.
 * Both are of defualt id type type
 * post: writes to output the combination of reg from input 1 and input 2 that have at fieldNumber
 * of input 1 an external reference to input 2.
 * Reg of output consist of fields from input 1 and its corresponding fields of input 2
 * Does not repeat external reference field*/
void RelationalAlgebra::naturalJoin(VLRFileHandler& input1,
		VLRFileHandler& input2, VLRFileHandler& output, uint fieldNumber) {
	BPlusTree bTree("tempIndex.bin");
	buildIndex(input2,bTree);
	input1.restartBuffersToBeginning();
	VLRegistry reg;
	while(input1.readNext(reg) && fieldNumber<reg.getNumOfFields()){
		Field f=reg.getField(fieldNumber);
		uint relPos; VLRegistry reg2;
		if(f.type==I4 && bTree.find(f.value.i4,relPos) && input2.get(relPos,f.value.i4,reg2)){
			VLRegistry outputReg=reg;
			for(uint i=1; i<reg2.getNumOfFields();i++){
				outputReg.addNewField(reg2.getField(i));
			}
			output.writeNext(outputReg);
		}
	}
}

//todo agrupation
//todo intersection
