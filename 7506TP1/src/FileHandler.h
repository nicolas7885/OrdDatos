/*
 * FileHandler.h
 *
 *  Created on: Sep 8, 2016
 *      Author: nicolas
 */

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <fstream>
#include <string>
#include <vector>

#include "VLRegistry.h"
#include "Field.h"

#define INVALID_BSIZE -1
#define UNABLE_TO_OPEN -2

typedef unsigned int uint;
typedef std::vector<VLRegistry>::iterator blockIt_t;

class FileHandler {
	char bSize;
	std::vector<char> metadata;
	std::vector<char> byteMap;
	std::fstream fs;
	uint currRelPos;//rel pos
	std::vector<VLRegistry> readBuffer;
	uint bufferPos;

public:
	FileHandler(std::string path);
	FileHandler(std::string path, uint bSize, std::string format);
	virtual ~FileHandler();
	int write(const std::vector<VLRegistry> &data);
	int write(const std::vector<VLRegistry> &data,uint relPos);
	void read(std::vector<VLRegistry> &data);
	void read(std::vector<VLRegistry> &data,uint relPos);
	int writeNext(VLRegistry & reg);
	bool readNext(VLRegistry &reg);
	void deleteBlock(uint relPos);
	bool eof();
	std::string getFormatAsString();
	void toCsv(std::string outputPath);
	void fromCsv(std::string sourcePath);
private:
	uint blockSizeInBytes();
	void setFormat(std::string format);
	std::vector<FieldType> getFormatAsTypes();
	void rewriteByteMap();
	long unsigned int calculateOffset(uint relPos);
	int writeBin(uint relPos, const std::vector<char>& serializedData);
	void regToCsv(VLRegistry &reg, std::fstream& output);
	void restartBuffersToBeginning();
};

#endif /* FILEHANDLER_H_ */
