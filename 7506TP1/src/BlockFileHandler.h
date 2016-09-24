/*
 * BlockFileHandler.h
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#ifndef SRC_BLOCKFILEHANDLER_H_
#define SRC_BLOCKFILEHANDLER_H_

#include <iostream>
#include <string>
#include <vector>

#include "FileHandler.h"
#include "VLRegistry.h"

#define INVALID_BSIZE -1

class BlockFileHandler : public FileHandler{
	char bSize;
	std::vector<char> byteMap;
	std::vector<VLRegistry> readBuffer;
	uint bufferPos;

public:
	BlockFileHandler(std::string path);
	BlockFileHandler(std::string path, uint bSize, std::string format);
	virtual ~BlockFileHandler();
	int write(const std::vector<VLRegistry> &data);
	int write(const std::vector<VLRegistry> &data,uint relPos);
	void read(std::vector<VLRegistry> &data);
	void read(std::vector<VLRegistry> &data,uint relPos);
	virtual int writeNext(VLRegistry & reg);
	virtual bool readNext(VLRegistry &reg);
	void deleteBlock(uint relPos);
	virtual bool eof();
protected:
	ulint calculateOffset(ulint relPos);
	virtual int writeBin(uint relPos, const std::vector<char>& serializedData);
	virtual void restartBuffersToBeginning();
private:
	uint blockSizeInBytes();
	void rewriteByteMap();
};

#endif /* SRC_BLOCKFILEHANDLER_H_ */
