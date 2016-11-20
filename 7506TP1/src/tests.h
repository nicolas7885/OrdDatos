/*
 * tests.h
 *
 *  Created on: Sep 22, 2016
 *      Author: nicolas
 */

#ifndef SRC_TESTS_H_
#define SRC_TESTS_H_

#include <string>
#include <vector>

#include "FileHandlers/BlockFileHandler.h"
#include "FileHandlers/VLRFileHandler.h"
#include "RelationalAlgebra.h"
#include "VLRegistries/Field.h"
#include "VLRegistries/VLRegistry.h"

using namespace std;

#define FORMAT "i1,i2,i4,sD,d,dt"

void fillRegistry(VLRegistry& reg) {
	//set format
	Field field;
	field.value.i1 = 1;
	field.type = I1;
	reg.setField(1, field);
	field.value.i2 = 512;
	field.type = I2;
	reg.setField(2, field);
	field.value.i4 = 1024;
	field.type = I4;
	reg.setField(3, field);
	field.s = "AABBCCDD";
	field.type = SD;
	reg.setField(4, field);
	field.s = "aaaammdd";
	field.type = D;
	reg.setField(5, field);
	field.s = "aaaammdd-hhmmss";
	field.type = DT;
	reg.setField(6, field);
}

void loadBlock(const int cantRegAAgregar, BlockFileHandler& handler) {
	vector<VLRegistry> block;
	for (int i = 1; i <= cantRegAAgregar; i++) {
		//create reg
		VLRegistry reg(i, FORMAT);
		//set format
		fillRegistry(reg);
		block.push_back(reg);
	}
	handler.write(block);
}

void createBinBlockFile(string path) {
	BlockFileHandler handler(path, 4, FORMAT);
	const int cantBloquesAAgregar = 2;
	const int cantRegAAgregar = 20;
	for (int i = 0; i < cantBloquesAAgregar; i++) {
		loadBlock(cantRegAAgregar, handler);
	}
}

string obtainCombinedFormat(BlockFileHandler& blockHandler1,
		VLRFileHandler& vlrHandler) {
	string combinationFormat = blockHandler1.getFormatAsString() + ",i4,"
			+ vlrHandler.getFormatAsString();
	return combinationFormat;
}

void doMockSelection(RelationalAlgebra& proccessor, VLRFileHandler& vlrHandler,
		BlockFileHandler& selectionHandler) {
	Field compareValue;
	compareValue.type = I4;
	compareValue.value.i4 = 10;
	condition_t condition = { LOWER, compareValue, 0 };
	proccessor.selectionOperator(vlrHandler, selectionHandler, condition);
}

/*test: secuential integration test. if it returns -1 error,
 *check files for bugs even if 0*
 *srry about lazy test framework*/
void runTests() {
	//test variables
	string binBlockFile1="test1B.bin";
	string binBlockFile2="test2B.bin";
	string basicFile="test1BCsv";
	string ReadAndDeleteFile="test2BCsv";
	const int cantRegBloqueEspecial=5;
	string ReadAndDeleteAndPutNewFile="test3BCsv";//read delete and put back another
	string binVLRFile1="test3VLR.bin";
	string ReadFileVlr="test4VLRCsv";//read
	string ReadAndDeleteFileVlr="test5VLRCsv";//read and delete
	string ReadAndDeleteAndPutNewFileVlr="test6VLRCsv";//read and delete and put back another

	//create a file, fill with stuff
	createBinBlockFile(binBlockFile1);

	//1 read and then output said file to csv
	//out: 1 to 20 twice
	BlockFileHandler blockHandler1(binBlockFile1);
	blockHandler1.toCsv(basicFile);

	//2 read block file(smaller blocks also) from csv, delete block & output
	//out: 1 to 10, then 1 to 20
	BlockFileHandler newBlockHandler(binBlockFile2, 0, FORMAT);
	newBlockHandler.fromCsv(basicFile);
	newBlockHandler.deleteBlock(1);
	newBlockHandler.toCsv(ReadAndDeleteFile);

	//3 add another block and output
	//out: 1 to 10, then 1 to 5, then 1 to 20
	loadBlock(cantRegBloqueEspecial,newBlockHandler);
	newBlockHandler.toCsv(ReadAndDeleteAndPutNewFile);

	//4 read vlr file from csv, and output to other
	//out: 1 to 20 twice
	VLRFileHandler vlrHandler(binVLRFile1, FORMAT);
	vlrHandler.fromCsv(basicFile);
	vlrHandler.toCsv(ReadFileVlr);

	//5 delete in dif places and output
	//out:3 to 20 then 2 to 20
	vlrHandler.deleteReg(0);
	ulint next=vlrHandler.tellg();
	vlrHandler.deleteReg(next);
	for(int i=3; i<=20; i++){
		VLRegistry reg;
		vlrHandler.readNext(reg);
	}
	next=vlrHandler.tellg();
	vlrHandler.deleteReg(next);
	vlrHandler.toCsv(ReadAndDeleteFileVlr);

	//6 add 2 reg and output
	//out: 255,127, then 2 to 20, then 1 to 20, then 127
	VLRegistry reg(255, FORMAT);
	fillRegistry(reg);
	vlrHandler.writeNext(reg);
	VLRegistry reg2(127, FORMAT);
	fillRegistry(reg2);
	vlrHandler.writeNext(reg2);
	VLRegistry reg3(98, FORMAT);
	fillRegistry(reg3);
	vlrHandler.writeNext(reg3);
	vlrHandler.toCsv(ReadAndDeleteAndPutNewFileVlr);

	RelationalAlgebra proccessor;
	{//7 do union and output
		//out: output of 1 and then output of 6
		string binUnionFileName="test4B.bin";
		string unionFileName="test7Csv";
		BlockFileHandler unionHandler(binUnionFileName, 0, FORMAT);

		proccessor.unionOperator(blockHandler1,vlrHandler,unionHandler);
		unionHandler.toCsv(unionFileName);
	}
	{//8 do basic selection and output
		//out: reg from output 6(test) lower than compareValue
		string selectionFileName="test8Csv";
		string binSelectionFileName="testSel.bin";
		BlockFileHandler selectionHandler(binSelectionFileName, 1, FORMAT);
		doMockSelection(proccessor, vlrHandler, selectionHandler);
		selectionHandler.toCsv(selectionFileName);
	}
	{//9 do projection and output
		//out: output from test 6 with projection applied
		string binProjectionFileName="testPJB.bin";
		string projectionFileName="test9Csv";
		BlockFileHandler projectionHandler(binProjectionFileName, 0, "i1,d,sD");
		string selectionFields="0,1,5,4";
		proccessor.projectionOperator(vlrHandler,projectionHandler, selectionFields);
		projectionHandler.toCsv(projectionFileName);
	}
	{//10 do product and output
		//out: combinations of reg from output
		string binProductFileName="test4B.bin";
		string productFileName="test10Csv";
		string combinationFormat = obtainCombinedFormat(blockHandler1,vlrHandler);
		VLRFileHandler productHandler(binProductFileName,combinationFormat);
		proccessor.productOperator(blockHandler1,vlrHandler, productHandler);
		productHandler.toCsv(productFileName);
	}
	{//11 do difference and output
		string binSelectionFileName="testSel.bin";
		BlockFileHandler selectionHandler(binSelectionFileName);
		string binDifferenceFileName="test4B.bin";
		string differenceFileName="test11Csv";
		BlockFileHandler differenceHandler(binDifferenceFileName,0, FORMAT);
		proccessor.differenceOperator(blockHandler1,selectionHandler, differenceHandler);
		differenceHandler.toCsv(differenceFileName);
	}
}



#endif /* SRC_TESTS_H_ */
