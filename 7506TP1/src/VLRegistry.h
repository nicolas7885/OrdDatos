/*
 * VLRegistry.h
 *
 *  Created on: Sep 8, 2016
 *      Author: nicolas
 */

#ifndef VLREGISTRY_H_
#define VLREGISTRY_H_

#include <string>
#include <vector>

#include "Field.h"

class VLRegistry {
	std::vector<Field> fields;
public:
	VLRegistry(int id, std::string format);
	virtual ~VLRegistry();
	Field getField(int id) const;
	void setField(int id,const Field& field);
	int getNumOfFields() const;
	void addEmptyField(FieldType type);
private:
	void addId(int id);
};

#endif /* VLREGISTRY_H_ */
