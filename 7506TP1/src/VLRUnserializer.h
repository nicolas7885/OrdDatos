/*
 * VLRUnserializer.h
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#ifndef SRC_VLRUNSERIALIZER_H_
#define SRC_VLRUNSERIALIZER_H_

#include <vector>

#include "Field.h"
#include "VLRegistry.h"

typedef std::vector<char>::iterator dataIt_t;

class VLRUnserializer {
	std::vector<FieldType> format;
public:
	VLRUnserializer(std::vector<FieldType> format);
	virtual ~VLRUnserializer();
	void unserializeReg(VLRegistry &output,const std::vector<char> &serializedData,dataIt_t &begin);
	void unserializeBlock(std::vector<VLRegistry> &block, const std::vector<char> &serializedData);
};

#endif /* SRC_VLRUNSERIALIZER_H_ */
