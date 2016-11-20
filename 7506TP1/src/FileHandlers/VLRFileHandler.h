/*
 * VLRFileHandler.h
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#ifndef SRC_FILEHANDLERS_VLRFILEHANDLER_H_
#define SRC_FILEHANDLERS_VLRFILEHANDLER_H_

#include "FileHandler.h"

typedef unsigned long int regSize_t;

struct PointerToFree{
	ulint pos;
	ulint size;
	ulint pointerToNext;
};


class VLRFileHandler: public FileHandler {
	ulint firstFreePtr;
	uint lastRelPos;

public:
	VLRFileHandler(std::string path);
	VLRFileHandler(std::string path, std::string format);
	virtual ~VLRFileHandler();
	bool read(ulint relPos, VLRegistry &reg);
	virtual ulint writeNext(const VLRegistry & reg);
	virtual bool readNext(VLRegistry &reg);
	virtual uint tellg();
	virtual bool get(uint relPos, int id, VLRegistry& result);
	void deleteReg(ulint relPos);

protected:
	virtual ulint calculateOffset(ulint relPos);
	virtual void restartBuffersToBeginning();

private:
	regSize_t readSize();
	char readType();
	void read(VLRegistry& reg);
	void writePointerToFree(ulint freeSpacePos, ulint freeSpaceSize,ulint nextFreePointer);
	PointerToFree readPointerToFree(ulint relPos);
	void updateMetadata();
	void updateLinkedList(const PointerToFree& prevFreePointer,const ulint nextPointer);
	ulint findPosToWriteAndUpdateList(std::vector<char>& serializedData);

	virtual int writeBin(uint pos, const std::vector<char>& serializedData,const char dataType);
	regSize_t replaceRegWithFreePointer(ulint pos, ulint nextFreePointerPos);
	ulint findPointerInsertionPos(ulint pos, PointerToFree& prevFreePointer);
};

#endif /* SRC_FILEHANDLERS_VLRFILEHANDLER_H_ */
