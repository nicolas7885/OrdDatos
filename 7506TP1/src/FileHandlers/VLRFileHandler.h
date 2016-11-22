/*
 * FileHandler.h
 *
 *  Created on: Sep 8, 2016
 *      Author: nicolas
 */


#ifndef VLRFILEHANDLER_H_
#define VLRFILEHANDLER_H_

#include <fstream>
#include <string>
#include <vector>

#include "../VLRegistries/Field.h"
#include "../VLRegistries/VLRegistry.h"

class FileHandler;

#define CHUNK_SIZE 512
#define METADATA_SIZE CHUNK_SIZE
#define FORMAT_SIZE_POS 1

typedef unsigned int uint;
typedef unsigned long int ulint;

class VLRFileHandler {
protected:
	FileHandler* fileP;
public:
	VLRFileHandler(FileHandler* file);
	virtual ~VLRFileHandler();
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

#endif /* VLRFILEHANDLER_H_ */
