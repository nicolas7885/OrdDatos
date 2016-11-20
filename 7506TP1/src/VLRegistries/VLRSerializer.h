/*
 * VLRSerializer.h
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#ifndef VLRSERIALIZER_H_
#define VLRSERIALIZER_H_

#include <vector>

#include "VLRegistry.h"

class VLRSerializer {
public:
	VLRSerializer();
	virtual ~VLRSerializer();
	void serializeReg(std::vector<char> &serializedData,const VLRegistry &reg);
	void serializeBlock(std::vector<char> &serializedData,const std::vector<VLRegistry> &data);
};

#endif /* VLRSERIALIZER_H_ */
