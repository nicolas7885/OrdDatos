/*
 * VLRFileHandler.h
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#ifndef SRC_VLRFILEHANDLER_H_
#define SRC_VLRFILEHANDLER_H_

#include "FileHandler.h"

class VLRFileHandler: public FileHandler {
	ulint firstFreeRelPos;
public:
	VLRFileHandler(std::string path);
	VLRFileHandler(std::string path, std::string format);
	virtual ~VLRFileHandler();
	int write(ulint relPos, VLRegistry & reg);
	virtual int writeNext(VLRegistry & reg);
	virtual bool readNext(VLRegistry &reg);

protected:
	virtual ulint calculateOffset(ulint relPos);
	virtual int writeBin(uint relPos, const std::vector<char>& serializedData);
	virtual void restartBuffersToBeginning();

};

#endif /* SRC_VLRFILEHANDLER_H_ */
