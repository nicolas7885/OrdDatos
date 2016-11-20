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
	VLRegistry();
	virtual ~VLRegistry();
	Field getField(int id) const;
	void setField(int id,const Field& field);
	uint getNumOfFields() const;
	void addEmptyField(FieldType type = I8);
	void addNewField(const Field& field);

	friend bool operator==(const VLRegistry& lhs, const VLRegistry& rhs){
		if(lhs.getNumOfFields()!=rhs.getNumOfFields()){
			return false;
		}
		for(uint i=0; i<lhs.getNumOfFields(); i++){
			Field f1=lhs.getField(i);
			Field f2=rhs.getField(i);
			if(!Field::compareFields(f1,f2,EQUAL)){
				return false;
			}
		}
		return true;
	}

	friend bool operator!=(const VLRegistry& lhs, const VLRegistry& rhs){ return !(lhs == rhs); }
private:
	void addId(int id);
};

#endif /* VLREGISTRY_H_ */
