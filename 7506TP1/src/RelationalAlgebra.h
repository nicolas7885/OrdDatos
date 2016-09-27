/*
 * RelationalAlgebra.h
 *
 *  Created on: Sep 27, 2016
 *      Author: nicolas
 */

#ifndef SRC_RELATIONALALGEBRA_H_
#define SRC_RELATIONALALGEBRA_H_

#include "FileHandler.h"

class RelationalAlgebra {
public:
	RelationalAlgebra();
	virtual ~RelationalAlgebra();
	void unionOperator(FileHandler& input1,FileHandler& input2, FileHandler& output);
	void selectionOperator(FileHandler& input, FileHandler& output,
			bool (*filter)(VLRegistry reg));
	void projectionOperator(FileHandler& input, FileHandler& output, std::string selectionFields);
	void productOperator(FileHandler& input1,FileHandler& input2, FileHandler& output);

};

#endif /* SRC_RELATIONALALGEBRA_H_ */
