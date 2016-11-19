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

#define CHUNK_SIZE 512
#define METADATA_SIZE CHUNK_SIZE
#define FORMAT_SIZE_POS 1

typedef unsigned int uint;
typedef unsigned long int ulint;
typedef std::vector<VLRegistry>::iterator blockIt_t;

class FileHandler {
protected:
	std::vector<char> metadata;
	std::fstream fs;
public:
	FileHandler(std::string path);
	FileHandler(std::string path, std::string format);
	virtual ~FileHandler();
	/*writes reg in next possible position
	 *Does nothing and returns -1 if EOF is reached before writing.
	 * If write is succesful returns num of block where it ended in.*/
	virtual ulint writeNext(const VLRegistry & reg)=0;
	/*reads the next valid registry*
	 * Does nothing and returns false if EOF is reached before reading into reg
	 * If EOF is reached stops reading, and returns true.*/
	virtual bool readNext(VLRegistry &reg)=0;
	/*gives the rel position of the last reg to be read*/
	virtual uint tellg()=0;
	/*obtains reg at relPos with id matching if possible and returns true.
	 * returns false if not found or out of bounds*/
	virtual bool get(uint relPos, int id, VLRegistry& result)=0;
	virtual bool eof();
	std::string getFormatAsString();
	void toCsv(std::string outputPath);
	void fromCsv(std::string sourcePath);
	virtual void restartBuffersToBeginning()=0;

protected:
	void setFormat(std::string format);
	std::vector<FieldType> getFormatAsTypes();
	void regToCsv(VLRegistry &reg, std::fstream& output);
};

#endif /* FILEHANDLER_H_ */
