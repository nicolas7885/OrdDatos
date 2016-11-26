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
#include <algorithm>

#include "FileHandlers/VLRBlockFileH.h"
#include "FileHandlers/VLRSecFileH.h"
#include "RelationalAlgebra.h"
#include "VLRegistries/Field.h"
#include "VLRegistries/VLRegistry.h"

using namespace std;

#define FORMAT "i1,i2,i4,sD,d,dt"

void fillRegistry(VLRegistry& reg, string format) {
	//set format
	std::replace(format.begin(), format.end(), ',', ' ');
	std::stringstream ss(format);
	std::string singleFieldString;uint i=1;
	while(ss>>singleFieldString){
		FieldType fieldType=Field::typeFromString(singleFieldString);
		Field field;
		field.type=fieldType;
		switch(fieldType){
		case I1:
			field.value.i1 = 1;
			break;
		case I2:
			field.value.i2 = 512;
			break;
		case I4:
			field.value.i4 = 1025;
			break;
		case I8:
			field.value.i8= 2048;
			break;
		case SL:
		case SD:
			field.s = "AABBCCDD";
			break;
		case D:
			field.s = "aaaammdd";
			break;
		case DT:
			field.s = "aaaammdd-hhmmss";
			break;

		}
		reg.setField(i, field);
		i++;
	}
}

void loadBlock(const int cantRegAAgregar, VLRBlockFileH& handler, string format) {
	vector<VLRegistry> block;
	for (int i = 1; i <= cantRegAAgregar; i++) {
		//create reg
		VLRegistry reg(i, format);
		//set format
		fillRegistry(reg, format);
		block.push_back(reg);
	}
	handler.write(block);
}

void createBinBlockFile(string path) {
	VLRBlockFileH handler(path, 4, FORMAT);
	const int cantBloquesAAgregar = 2;
	const int cantRegAAgregar = 20;
	for (int i = 0; i < cantBloquesAAgregar; i++) {
		loadBlock(cantRegAAgregar, handler, FORMAT);
	}
}

string obtainCombinedFormat(VLRBlockFileH& blockHandler1,
		VLRSecFileH& vlrHandler) {
	string combinationFormat = blockHandler1.getFormatAsString() + ",i4,"
			+ vlrHandler.getFormatAsString();
	return combinationFormat;
}

void doMockSelection(RelationalAlgebra& proccessor, VLRSecFileH& vlrHandler,
		VLRBlockFileH& selectionHandler) {
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
	VLRBlockFileH blockHandler1(binBlockFile1);
	blockHandler1.toCsv(basicFile);

	//2 read block file(smaller blocks also) from csv, delete block & output
	//out: 1 to 10, then 1 to 20
	VLRBlockFileH newBlockHandler(binBlockFile2, 0, FORMAT);
	newBlockHandler.fromCsv(basicFile);
	newBlockHandler.toCsv(ReadAndDeleteFile);
	newBlockHandler.deleteBlock(1);
	newBlockHandler.toCsv(ReadAndDeleteFile);

	//3 add another block and output
	//out: 1 to 10, then 1 to 5, then 1 to 20
	loadBlock(cantRegBloqueEspecial,newBlockHandler, FORMAT);
	newBlockHandler.toCsv(ReadAndDeleteAndPutNewFile);

	//4 read vlr file from csv, and output to other
	//out: 1 to 20 twice
	VLRSecFileH vlrHandler(binVLRFile1, FORMAT);
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
	fillRegistry(reg,FORMAT);
	vlrHandler.writeNext(reg);
	VLRegistry reg2(127, FORMAT);
	fillRegistry(reg2,FORMAT);
	vlrHandler.writeNext(reg2);
	VLRegistry reg3(98, FORMAT);
	fillRegistry(reg3,FORMAT);
	vlrHandler.writeNext(reg3);
	vlrHandler.toCsv(ReadAndDeleteAndPutNewFileVlr);

	RelationalAlgebra proccessor;
	{//7 do union and output
		//out: output of 1 and then output of 6
		string binUnionFileName="test4B.bin";
		string unionFileName="test7Csv";
		VLRBlockFileH unionHandler(binUnionFileName, 0, FORMAT);

		proccessor.unionOperator(blockHandler1,vlrHandler,unionHandler);
		unionHandler.toCsv(unionFileName);
	}
	{//8 do basic selection and output
		//out: reg from output 6(test) lower than compareValue
		string selectionFileName="test8Csv";
		string binSelectionFileName="testSel.bin";
		VLRBlockFileH selectionHandler(binSelectionFileName, 1, FORMAT);
		doMockSelection(proccessor, vlrHandler, selectionHandler);
		selectionHandler.toCsv(selectionFileName);
	}
	{//9 do projection and output
		//out: output from test 6 with projection applied
		string binProjectionFileName="testPJB.bin";
		string projectionFileName="test9Csv";
		VLRBlockFileH projectionHandler(binProjectionFileName, 0, "i1,d,sD");
		string selectionFields="0,1,5,4";
		proccessor.projectionOperator(vlrHandler,projectionHandler, selectionFields);
		projectionHandler.toCsv(projectionFileName);
	}
	{//10 do product and output
		//out: combinations of reg from output
		string binProductFileName="test4B.bin";
		string productFileName="test10Csv";
		string combinationFormat = obtainCombinedFormat(blockHandler1,vlrHandler);
		VLRSecFileH productHandler(binProductFileName,combinationFormat);
		proccessor.productOperator(blockHandler1,vlrHandler, productHandler);
		productHandler.toCsv(productFileName);
	}
	{//11 do difference and output
		string binSelectionFileName="testSel.bin";
		VLRBlockFileH selectionHandler(binSelectionFileName);
		string binDifferenceFileName="test4B.bin";
		string differenceFileName="test11Csv";
		VLRBlockFileH differenceHandler(binDifferenceFileName,0, FORMAT);
		proccessor.differenceOperator(blockHandler1,selectionHandler, differenceHandler);
		differenceHandler.toCsv(differenceFileName);
	}
	{//12 natural join and output
		//prepare test files
		string input1Name="testNatJoinI1.bin";
		string input2Name="testNatJoinI2.bin";
		string binNatJoinFileName="testNatJoin.bin";
		string output="test12Csv";
		string format1="i4,sD,i4";
		VLRBlockFileH input1(input1Name,0,format1);
		int cantReg = 10;
		for (int i = 1; i <= cantReg; i++) {
			VLRegistry reg(i, format1);
			Field f;
			f.value.i4=i; f.type=I4;
			reg.setField(1,f);
			f.s="bb"; f.type=SD;
			reg.setField(2,f);
			f.value.i4=cantReg+1-i; f.type=I4;
			reg.setField(3,f);
			input1.writeNext(reg);
		}
		string format2="sD";
		VLRBlockFileH input2(input2Name,0,format2);
		for(int i=1; i<=cantReg; i++){
			VLRegistry reg(i, format2);
			stringstream ss; ss<<"aaa"<<i;
			Field f; f.s=ss.str(); f.type=SD;
			reg.setField(1,f);
			input2.writeNext(reg);
		}
		string formatOutput=format1+","+format2;
		VLRBlockFileH NatJoinHandler(binNatJoinFileName,1,formatOutput);
		//execute
		proccessor.naturalJoin(input1,input2,NatJoinHandler,1);
		NatJoinHandler.toCsv(output);
	}
}



#endif /* SRC_TESTS_H_ */
