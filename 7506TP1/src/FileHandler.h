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
	uint currRelPos;//rel pos

public:
	FileHandler(std::string path);
	FileHandler(std::string path, std::string format);
	virtual ~FileHandler();
	/*writes reg in next possible position*/
	virtual int writeNext(VLRegistry & reg)=0;
	/*reads the next valid registry*/
	virtual bool readNext(VLRegistry &reg)=0;
	virtual bool eof();
	std::string getFormatAsString();
	void toCsv(std::string outputPath);
	void fromCsv(std::string sourcePath);

protected:
	void setFormat(std::string format);
	std::vector<FieldType> getFormatAsTypes();
	virtual int writeBin(uint relPos, const std::vector<char>& serializedData)=0;
	void regToCsv(VLRegistry &reg, std::fstream& output);
	virtual void restartBuffersToBeginning()=0;
};

#endif /* FILEHANDLER_H_ */
