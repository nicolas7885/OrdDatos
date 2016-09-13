/*
 * VLRSerializer.h
 *
 *  Created on: Sep 12, 2016
 *      Author: nicolas
 */

#ifndef VLRSERIALIZER_H_
#define VLRSERIALIZER_H_

#include <vector>

class VLRegistry;

class VLRSerializer {
public:
	VLRSerializer();
	virtual ~VLRSerializer();
	std::vector<char> serializeReg(const VLRegistry &reg);
};

#endif /* VLRSERIALIZER_H_ */
