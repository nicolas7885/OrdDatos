/*
 * VLRBlockFileH.h
 *
 *  Created on: Sep 24, 2016
 *      Author: nicolas
 */

#ifndef SRC_FILEHANDLERS_VLRBLOCKFILEH_H_
#define SRC_FILEHANDLERS_VLRBLOCKFILEH_H_

#include <string>
#include <vector>

#include "../VLRegistries/VLRegistry.h"
#include "BlockFileHandler.h"
#include "VLRFileHandler.h"

#define INVALID_BSIZE -1

class VLRBlockFileH : public VLRFileHandler{
	BlockFileHandler file;
	std::vector<VLRegistry> readBuffer;
	uint bufferPos;

public:
	VLRBlockFileH(std::string path);
	VLRBlockFileH(std::string path, uint bSize, std::string format);
	virtual ~VLRBlockFileH();
	int write(const std::vector<VLRegistry> &data);
	int write(const std::vector<VLRegistry> &data,uint relPos);
	void read(std::vector<VLRegistry> &data);
	void read(std::vector<VLRegistry> &data,uint relPos);
	virtual ulint writeNext(const VLRegistry & reg);
	virtual bool readNext(VLRegistry &reg);
	virtual uint tellg();
	virtual bool get(uint relPos, int id, VLRegistry& result);
	void deleteBlock(uint relPos);
	virtual bool eof();
	virtual void restartBuffersToBeginning();

private:
	std::vector<char> serializeBlock(const std::vector<VLRegistry>& data);
	void unserializeBlock(const std::vector<char>& data,std::vector<VLRegistry>& block);
};

#endif /* SRC_FILEHANDLERS_VLRBLOCKFILEH_H_ */
