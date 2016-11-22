/*
 * BlockFileHandler.h
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#ifndef SRC_FILEHANDLERS_BlockFileHandler_H_
#define SRC_FILEHANDLERS_BlockFileHandler_H_

#include <string>
#include <vector>

#include "FileHandler.h"

#define INVALID_BSIZE -1

class BlockFileHandler : public FileHandler{
	char bSize;
	std::vector<char> byteMap;
	ulint currRelPos;//rel pos

public:
	BlockFileHandler(std::string path);
	BlockFileHandler(std::string path, uint bSize);
	virtual ~BlockFileHandler();
	int write(const std::vector<char> &data);
	int write(const std::vector<char> &data,uint relPos);
	void read(std::vector<char> &data);
	uint read(std::vector<char> &data,uint relPos);
	virtual ulint writeNext(const std::vector<char> &data);
	virtual bool readNext(std::vector<char> &data);
	virtual uint tellg();
	void deleteBlock(uint relPos);
	virtual bool eof();
	virtual void restartBuffersToBeginning();
protected:
	ulint calculateOffset(ulint relPos);
	virtual int writeBin(uint relPos, const std::vector<char>& serializedData);
private:
	uint blockSizeInBytes();
	void rewriteByteMap();
};

#endif /* SRC_FILEHANDLERS_BlockFileHandler_H_ */
