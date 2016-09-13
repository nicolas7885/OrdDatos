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

class VLRegistry;

class VLRUnserializer {
	std::vector<FieldType> format;
public:
	VLRUnserializer(std::vector<FieldType> format);
	virtual ~VLRUnserializer();
	void unserializeReg(VLRegistry &output, std::vector<char> serializedData);
};

#endif /* SRC_VLRUNSERIALIZER_H_ */
