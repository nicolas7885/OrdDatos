/*
 * RelationalAlgebra.h
 *
 *  Created on: Sep 27, 2016
 *      Author: nicolas
 */

#ifndef SRC_RELATIONALALGEBRA_H_
#define SRC_RELATIONALALGEBRA_H_

#include "FileHandler.h"

enum CmpMode{
	GREATER,
	EQUAL,
	NOTEQUAL,
	LOWER
};

struct condition_t{
	CmpMode mode;
	Field value;
	uint pos;
};

class RelationalAlgebra {
public:
	RelationalAlgebra();
	virtual ~RelationalAlgebra();
	void unionOperator(FileHandler& input1,FileHandler& input2, FileHandler& output);
	void selectionOperator(FileHandler& input, FileHandler& output,condition_t condition);
	void projectionOperator(FileHandler& input, FileHandler& output, std::string selectionFields);
	void productOperator(FileHandler& input1,FileHandler& input2, FileHandler& output);

private:
	bool compare(const VLRegistry &reg,condition_t condition);

	template<typename T>
	bool compare(T base,T reference,CmpMode mode){
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
};

#endif /* SRC_RELATIONALALGEBRA_H_ */
