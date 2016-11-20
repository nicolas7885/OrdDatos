/*
 * RelationalAlgebra.h
 *
 *  Created on: Sep 27, 2016
 *      Author: nicolas
 */

#ifndef SRC_RELATIONALALGEBRA_H_
#define SRC_RELATIONALALGEBRA_H_

#include <sys/types.h>
#include <string>

#include "FileHandlers/FileHandler.h"
#include "Index/BPlusTree.h"
#include "VLRegistries/Field.h"

class VLRegistry;

struct condition_t{
	CmpMode mode;
	Field value;
	uint pos;
};

class RelationalAlgebra {
public:
	RelationalAlgebra();
	virtual ~RelationalAlgebra();
	//primary operators
	void unionOperator(FileHandler& input1,FileHandler& input2, FileHandler& output);
	void selectionOperator(FileHandler& input, FileHandler& output,condition_t condition);
	void projectionOperator(FileHandler& input, FileHandler& output, std::string selectionFields);
	void productOperator(FileHandler& input1,FileHandler& input2, FileHandler& output);
	void differenceOperator(FileHandler& input1,FileHandler& input2, FileHandler& output);
private:
	bool compare(const VLRegistry &reg,condition_t condition);
	void buildIndex(FileHandler& input2, BPlusTree& bTree);
};

#endif /* SRC_RELATIONALALGEBRA_H_ */
