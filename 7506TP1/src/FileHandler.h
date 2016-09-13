/*
 * FileHandler.h
 *
 *  Created on: Sep 8, 2016
 *      Author: nicolas
 */

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <iostream>
#include <vector>
#include <string>

class VLRegistry;

#define INVALID_BSIZE -1
#define UNABLE_TO_OPEN -2

typedef unsigned int uint;

class FileHandler {
	char bSize;
	std::vector<char> metadata;
	std::vector<char> byteMap;
	std::fstream fs;
public:
	FileHandler(std::string path);
	FileHandler(std::string path, uint bSize, std::string format);
	virtual ~FileHandler();
	int write(const std::vector<VLRegistry> &data);
	int write(const std::vector<VLRegistry> &data,int relPos);
	void read(std::vector<VLRegistry> &data);
	void read(std::vector<VLRegistry> &data,int relPos);
	void deleteBlock(int relPos);
	bool eof();

private:
	int blockSizeInBytes();
	void setFormat(std::string format);
	void rewriteByteMap();
	long int calculateOffset(int relPos);
	int writeBin(int relPos, const std::vector<char>& serializedData);
};

#endif /* FILEHANDLER_H_ */
