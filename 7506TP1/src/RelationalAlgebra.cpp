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
	//todo check if it works
	input1.restartBuffersToBeginning();
	while(!input1.eof()){
		VLRegistry reg;
		input1.readNext(reg);
		output.writeNext(reg);
	}
	input2.restartBuffersToBeginning();
	while(!input2.eof()){
		VLRegistry reg;
		input2.readNext(reg);
		output.writeNext(reg);
	}
}

/* pre: input output open. Output has selection Fields compatible format.
 * selection fields is format including id. each pos is the in vector number of the field.
 * field 0 is the corresponding to the id of the reg.
 * post:writes output with all the reg of input following selection Fields.*/
void RelationalAlgebra::projectionOperator(FileHandler& input,
		FileHandler& output, std::string selectionFields) {
	std::replace(selectionFields.begin(), selectionFields.end(), ',', ' ');
	std::stringstream ss(selectionFields);
	input.restartBuffersToBeginning();
	while(!input.eof()){
		VLRegistry oldReg;
		input.readNext(oldReg);
		VLRegistry newReg;
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
	while(!input1.eof()){
		VLRegistry reg1;
		input1.readNext(reg1);
		while(!input2.eof()){
			VLRegistry reg2;
			input2.readNext(reg2);
			//todo do something here
			//todo write to output something
		}
	}
}

/*pre: input and output opened and valid. They both have same format
 * filter takes a field and determines if it goes or not */
void RelationalAlgebra::selectionOperator(FileHandler& input,
		FileHandler& output, bool (*filter)(VLRegistry reg)) {
	input.restartBuffersToBeginning();
	while(!input.eof()){
		VLRegistry reg;
		input.readNext(reg);
		if(filter(reg))
			output.writeNext(reg);
	}
}
