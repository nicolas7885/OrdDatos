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

#define CHUNK_SIZE 512
#define METADATA_SIZE CHUNK_SIZE
#define FORMAT_SIZE_POS 1

typedef unsigned int uint;
typedef unsigned long int ulint;
class VLRFileHandler;

class FileHandler {
	friend class VLRFileHandler;
protected:
	std::vector<char> metadata;
	std::fstream fs;

public:
	FileHandler(std::string path,int mode=0);
	virtual ~FileHandler();
	/*writes reg in next possible position
	 *Does nothing and returns -1 if EOF is reached before writing.
	 * If write is succesful returns num of block where it ended in.*/
	virtual ulint writeNext(const std::vector<char> & data)=0;
	/*reads the next valid data*
	 * Does nothing and returns false if EOF is reached before reading into data
	 * If EOF is reached stops reading, and returns true.*/
	virtual bool readNext(std::vector<char> &data)=0;
	/*gives the rel position of the last to be read*/
	virtual uint tellg()=0;
	virtual bool eof();
	virtual void restartBuffersToBeginning()=0;

protected:
	void saveMetadata();
};

#endif /* FILEHANDLER_H_ */
