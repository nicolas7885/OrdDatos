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

#include "FileHandlers/VLRFileHandler.h"
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
	void unionOperator(VLRFileHandler& input1,VLRFileHandler& input2, VLRFileHandler& output);
	void selectionOperator(VLRFileHandler& input, VLRFileHandler& output,condition_t condition);
	void projectionOperator(VLRFileHandler& input, VLRFileHandler& output, std::string selectionFields);
	void productOperator(VLRFileHandler& input1,VLRFileHandler& input2, VLRFileHandler& output);
	void differenceOperator(VLRFileHandler& input1,VLRFileHandler& input2, VLRFileHandler& output);
	void naturalJoin(VLRFileHandler& input1,VLRFileHandler& input2,
			VLRFileHandler& output, uint fieldNumber);
private:
	bool compare(const VLRegistry &reg,condition_t condition);
	void buildIndex(VLRFileHandler& input2, BPlusTree& bTree);
};

#endif /* SRC_RELATIONALALGEBRA_H_ */
