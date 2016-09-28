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

#include "Field.h"
#include "VLRegistry.h"

RelationalAlgebra::RelationalAlgebra() {
	// TODO Auto-generated constructor stub

}

RelationalAlgebra::~RelationalAlgebra() {
	// TODO Auto-generated destructor stub
}

/*pre: input 1 and 2 opened and valid, have compatible format with output. Output opened and valid
 * post:reads all input 1 writing it into output. Then reads all input two and writes it into output*/
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
		for(int fieldNumber, numberOfFields=0;ss >> fieldNumber;numberOfFields++){
			if(oldReg.getNumOfFields()>fieldNumber){
				newReg.addEmptyField();
				Field f=oldReg.getField(fieldNumber);
				newReg.setField(numberOfFields,f);
			}
		}
		if(newReg.getNumOfFields()){
			output.writeNext(newReg);
		}
	}
}

void RelationalAlgebra::productOperator(FileHandler& input1,
		FileHandler& input2, FileHandler& output) {
	input1.restartBuffersToBeginning();
	VLRegistry reg1;
	while(input1.readNext(reg1)){
		VLRegistry reg2;
		while(input2.readNext(reg2)){
			//todo do something here
			//todo write to output something
		}
	}
}

bool RelationalAlgebra::compare(const VLRegistry &reg,condition_t condition){

	if(reg.getNumOfFields()<condition.pos)
		return false;

	Field compared=reg.getField(condition.pos);
	Field reference=condition.value;

	if(compared.type!=reference.type)
		return false;

	switch(reference.type){
	case I1:
		return compare(compared.value.i1,reference.value.i1,condition.mode);
	case I2:
		return compare(compared.value.i2,reference.value.i2,condition.mode);
	case I4:
		return compare(compared.value.i4,reference.value.i4,condition.mode);
	case I8:
		return compare(compared.value.i8,reference.value.i8,condition.mode);
	case SD:
	case SL:
	case D:
	case DT:
		return compare(compared.s,reference.s,condition.mode);
	default:
		return false;
	}
}

/*pre: input and output opened and valid. They both have same format
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
