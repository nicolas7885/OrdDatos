/*
 * VLRFileHandler.h
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#ifndef SRC_FILEHANDLERS_SecFileHandler_H_
#define SRC_FILEHANDLERS_SecFileHandler_H_

#include "FileHandler.h"

typedef unsigned long int regSize_t;

struct PointerToFree{
	ulint pos;
	ulint size;
	ulint pointerToNext;
};


class SecFileHandler: public FileHandler {
	ulint firstFreePtr;
	uint lastRelPos;

public:
	SecFileHandler(std::string path, int mode=0);
	virtual ~SecFileHandler();
	bool read(ulint relPos, std::vector<char> &reg);
	virtual ulint writeNext(const std::vector<char> & reg);
	virtual bool readNext(std::vector<char> &reg);
	virtual uint tellg();
	void deleteReg(ulint relPos);
	virtual void restartBuffersToBeginning();

protected:
	virtual ulint calculateOffset(ulint relPos);

private:
	regSize_t readSize();
	char readType();
	void read(std::vector<char>& reg);
	void writePointerToFree(ulint freeSpacePos, ulint freeSpaceSize,ulint nextFreePointer);
	PointerToFree readPointerToFree(ulint relPos);
	void updateMetadata();
	void updateLinkedList(const PointerToFree& prevFreePointer,const ulint nextPointer);
	ulint findPosToWriteAndUpdateList(std::vector<char>& serializedData);

	virtual int writeBin(uint pos, const std::vector<char>& serializedData,const char dataType);
	regSize_t replaceRegWithFreePointer(ulint pos, ulint nextFreePointerPos);
	ulint findPointerInsertionPos(ulint pos, PointerToFree& prevFreePointer);
};

#endif /* SRC_FILEHANDLERS_SecFileHandler_H_ */
