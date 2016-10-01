//============================================================================
// Name        : 7506TP1.cpp
// Author      : nico
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

#include "BlockFileHandler.h"
#include "VLRFileHandler.h"
#include "RelationalAlgebra.h"
#include "tests.h"

using namespace std;

#define SALIDA_NORMAL 0
#define SALIDA_ERROR 1
#define MAX_LARGO_NOMBRE 50

void imprimirAyuda(){
	cout<<"  Usage:"<<endl
			<<"\t -h \t Print this information."<<endl
			<<"\t -imp 'inputName' 'outputName' 'format', \t Imports csv file into variable length reg output with specified format"<<endl
			<<"\t -imp 'inputName' 'outputName' 'format' 'blockSize', \t Imports csv file into block of 2^n*512b size output with specified format"<<endl
			<<"\t -exp 'inputName' 'outputName' \t Exports input file  to csv"<<endl
			<<"\t [operator] [arguments]"<<endl
			<<endl<<"  Operators:"<<endl
			<<"\t -u  -args-> 'input1Name' 'input2Name' 'outputName', \t Applies union operator"<<endl
			<<"\t -pd -args-> 'input1Name' 'input2Name' 'outputName', \t Applies product operator"<<endl
			<<"\t -pj -args-> 'inputName' 'projection format' 'outputName' , \t Applies projection operator"<<endl
			<<"\t \t Format specified with field numbers, separated with commas. Can repeat and transpose fields."<<endl
			<<"\t \t Example: -pj file1.bin  out.bin  0,3,2,9,4"<<endl
			<<"\t -s  -args-> 'inputName' 'outputName' 'field number' 'fieldType' 'fieldValue' 'op' \t Applies selection operator. "<<endl
			<<"\t \t Condition 'op' are: less , grtr , eq , neq . Value can be number or string accordingly"<<endl
			<<"\t \t Example: -s file1.bin  out.bin  0 < 10"<<endl
			<<endl<<"  Adding a '-b <blockSize>' before an outputName makes it a block file, ex: '-b 3 out.bin'"<<endl<<endl;

}

void importFromCsv(int& currentArgument, char* argv[]) {
	//todo errors when wrong number of arg
	string input(argv[++currentArgument]);
	FileHandler* handler;
	string next(argv[++currentArgument]);
	if (next=="-b") {
		int bSize=atoi(argv[++currentArgument]);
		string output(argv[++currentArgument]);
		string format(argv[++currentArgument]);
		handler = new BlockFileHandler(output, bSize,format);
	} else {
		string output=argv[currentArgument];
		string format=argv[++currentArgument];
		handler = new VLRFileHandler(output, format);
	}
	handler->fromCsv(input);
	delete handler;
}

/*creates block or vlr handler. please destroy after*/
FileHandler* getHandler(string name){
	fstream fs(name);
	char type;
	fs.read(&type,1);
	fs.close();
	if(type>=0 && type<=4)
		return new BlockFileHandler(name);
	else
		return new VLRFileHandler(name);
}

void exportToCsv(int& currentArgument, char* argv[]) {
	//todo errors when wrong number of arg
	string input(argv[++currentArgument]);
	string output(argv[++currentArgument]);
	FileHandler* handler=getHandler(input);
	handler->toCsv(output);
	delete handler;
}

/*creates block or vlr handler. please destroy after*/
FileHandler* getNewHandler(int& currentArgument, char* argv[],string format){
	//todo errors when wrong number of arg
	string next(argv[++currentArgument]);
	if(next=="-b"){
		int bSize=atoi(argv[++currentArgument]);
		string handlerName=argv[++currentArgument];
		return new BlockFileHandler(handlerName,bSize,format);
	}else{
		string handlerName=argv[currentArgument];
		return new VLRFileHandler(handlerName,format);
	}

}

void performUnion(int& currentArgument, char* argv[]) {
	string input1Name(argv[++currentArgument]);
	string input2Name(argv[++currentArgument]);
	FileHandler* input1 = getHandler(input1Name);
	FileHandler* input2 = getHandler(input2Name);
	string format=input1->getFormatAsString();
	FileHandler* output = getNewHandler(currentArgument, argv,format);
	RelationalAlgebra processor;
	processor.unionOperator(*input1, *input2, *output);
}

string obtainCombinedFormat(FileHandler* handler1,
		FileHandler* handler2) {
	string combinationFormat = handler1->getFormatAsString() + ",i4,"
			+ handler2->getFormatAsString();
	return combinationFormat;
}

void performProduct(int& currentArgument, char* argv[]) {
	string input1Name(argv[++currentArgument]);
	string input2Name(argv[++currentArgument]);
	FileHandler* input1 = getHandler(input1Name);
	FileHandler* input2 = getHandler(input2Name);
	FileHandler* output = getNewHandler(currentArgument, argv,obtainCombinedFormat(input1,input2));
	RelationalAlgebra processor;
	processor.productOperator(*input1, *input2, *output);
}

void performProjection(int& currentArgument, char* argv[]) {
	string inputName(argv[++currentArgument]);
	FileHandler* input = getHandler(inputName);
	stringstream formatPos(argv[++currentArgument]);
	string format=input->getFormatAsString();
	//todo build actual format
	FileHandler* output = getNewHandler(currentArgument, argv, format);
	RelationalAlgebra processor;
	processor.projectionOperator(*input, *output, formatPos.str());
}

condition_t obtainCondition(char* argv[], int& currentArgument) {
	condition_t condition;
	condition.pos = atoi(argv[++currentArgument]);
	Field reference;
	reference.type = typeFromString(argv[++currentArgument]);
	switch (reference.type) {
	case I1:
		reference.value.i1 = atoi(argv[++currentArgument]);
		break;
	case I2:
		reference.value.i2 = atoi(argv[++currentArgument]);
		break;
	case I4:
		reference.value.i4 = atoi(argv[++currentArgument]);
		break;
	case I8:
		reference.value.i8 = atoi(argv[++currentArgument]);
		break;
	case SD:
	case SL:
	case D:
	case DT:
		reference.s = argv[++currentArgument];
		break;
	}
	condition.value = reference;
	string mode = argv[++currentArgument];
	if (mode == "less") {
		condition.mode = LOWER;
	} else if (mode == "grtr") {
		condition.mode = GREATER;
	} else if (mode == "eq") {
		condition.mode = EQUAL;
	} else if (mode == "neq") {
		condition.mode = NOTEQUAL;
	}

	return condition;
}

void performSelection(int& currentArgument, char* argv[]) {
	string inputName(argv[++currentArgument]);
	FileHandler* input = getHandler(inputName);
	FileHandler* output = getNewHandler(currentArgument, argv,input->getFormatAsString());
	condition_t condition = obtainCondition(argv, currentArgument);
	RelationalAlgebra processor;
	processor.selectionOperator(*input, *output, condition);
}

/*ugly interface, so hardcoded positions, repeated code and so on.*/
int main(int argc, char* argv[]) {
//	runTests();
	int currentArgument=1;
	string mode(argv[currentArgument]);
	if(mode== "-h"){
		imprimirAyuda();
	}else if(mode== "-imp"){
		importFromCsv(currentArgument, argv);
	}else if(mode == "-exp"){
		exportToCsv(currentArgument, argv);
	}else if(mode == "-u"){
		performUnion(currentArgument, argv);
	}else if(mode == "-pd"){
		performProduct(currentArgument, argv);
	}else if(mode == "-pj"){
		performProjection(currentArgument, argv);
	}else if(mode == "-s"){
		performSelection(currentArgument, argv);
	}

	return 0;
}
