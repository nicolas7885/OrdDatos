/*
 * VLRFileHandler.h
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#ifndef SRC_FILEHANDLERS_VLRSECFILEH_H_
#define SRC_FILEHANDLERS_VLRSECFILEH_H_

#include <string>
#include <vector>

#include "SecFileHandler.h"
#include "VLRFileHandler.h"

class VLRegistry;

typedef unsigned long int regSize_t;


class VLRSecFileH: public VLRFileHandler {
	SecFileHandler file;
public:
	VLRSecFileH(std::string path);
	VLRSecFileH(std::string path, std::string format);
	virtual ~VLRSecFileH();
	bool read(ulint relPos, VLRegistry &reg);
	virtual ulint writeNext(const VLRegistry & reg);
	virtual bool readNext(VLRegistry &reg);
	virtual uint tellg();
	virtual bool get(uint relPos, int id, VLRegistry& result);
	void deleteReg(ulint relPos);
	virtual void restartBuffersToBeginning();

private:
	void read(VLRegistry& reg);
	void unserializeReg(std::vector<char> serializedData, VLRegistry& reg);
};

#endif /* SRC_FILEHANDLERS_VLRSECFILEH_H_ */
