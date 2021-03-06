//============================================================================
// Name        : 7506TP1.cpp
// Author      : nico
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "FileHandlers/VLRBlockFileH.h"
#include "FileHandlers/VLRSecFileH.h"
#include "RelationalAlgebra.h"
#include "VLRegistries/Field.h"

using namespace std;

#define SALIDA_NORMAL 0
#define SALIDA_ERROR 1
#define MAX_LARGO_NOMBRE 50

void imprimirAyuda(){
	cout<<"  Usage:"<<endl
			<<"\t -h \t Print this information."<<endl
			<<"\t -imp 'inputName' 'outputName' 'format', \t Imports csv file into variable length reg output with specified format(do not include field id)"<<endl
			<<"\t -imp 'inputName' 'outputName' 'format' 'blockSize', \t Imports csv file into block of 2^n*512b size output with specified format"<<endl
			<<"\t -exp 'inputName' 'outputName' \t Exports input file  to csv"<<endl
			<<"\t [operator] [arguments]"<<endl
			<<endl<<"  Operators:"<<endl
			<<"\t -u  -args-> 'input1Name' 'input2Name' 'outputName', \t Applies union operator"<<endl
			<<"\t -pd -args-> 'input1Name' 'input2Name' 'outputName', \t Applies product operator"<<endl
			<<"\t -pj -args-> 'inputName' 'projection format' 'outputName' , \t Applies projection operator"<<endl
			<<"\t \t Format specified with field numbers, separated with commas. Can repeat and transpose fields."<<endl
			<<"\t \t Example: -pj file1.bin 3,2,9,4  out.bin"<<endl
			<<"\t -s  -args-> 'inputName' 'outputName' 'field number' 'fieldType' 'fieldValue' 'op' \t Applies selection operator. "<<endl
			<<"\t \t Condition 'op' are: less , grtr , eq , neq . Value can be number or string accordingly"<<endl
			<<"\t \t Example: -s file1.bin  out.bin  0 i4 10 less"<<endl
			<<endl<<"  Adding a '-b <blockSize>' before an outputName makes it a block file, ex: '-b 3 out.bin'"<<endl<<endl;

}

void importFromCsv(int& currentArgument, char* argv[]) {
	//todo errors when wrong number of arg
	string input(argv[++currentArgument]);
	VLRFileHandler* handler;
	string next(argv[++currentArgument]);
	if (next=="-b") {
		int bSize=atoi(argv[++currentArgument]);
		string output(argv[++currentArgument]);
		string format(argv[++currentArgument]);
		handler = new VLRBlockFileH(output, bSize,format);
	} else {
		string output=argv[currentArgument];
		string format=argv[++currentArgument];
		handler = new VLRSecFileH(output, format);
	}
	handler->fromCsv(input);
	delete handler;
}

/*creates block or vlr handler. please destroy after*/
VLRFileHandler* getHandler(string name){
	fstream fs(name);
	char type;
	fs.read(&type,1);
	fs.close();
	if(type>=0 && type<=4)
		return new VLRBlockFileH(name);
	else
		return new VLRSecFileH(name);
}

void exportToCsv(int& currentArgument, char* argv[]) {
	//todo errors when wrong number of arg
	string input(argv[++currentArgument]);
	string output(argv[++currentArgument]);
	VLRFileHandler* handler=getHandler(input);
	handler->toCsv(output);
	delete handler;
}

/*creates block or vlr handler. please destroy after*/
VLRFileHandler* getNewHandler(int& currentArgument, char* argv[],string format){
	//todo errors when wrong number of arg
	string next(argv[++currentArgument]);
	if(next=="-b"){
		int bSize=atoi(argv[++currentArgument]);
		string handlerName=argv[++currentArgument];
		return new VLRBlockFileH(handlerName,bSize,format);
	}else{
		string handlerName=argv[currentArgument];
		VLRFileHandler* handler=new VLRSecFileH(handlerName,format);
		return handler;
	}

}

void performUnion(int& currentArgument, char* argv[]) {
	string input1Name(argv[++currentArgument]);
	string input2Name(argv[++currentArgument]);
	VLRFileHandler* input1 = getHandler(input1Name);
	VLRFileHandler* input2 = getHandler(input2Name);
	string format=input1->getFormatAsString();
	VLRFileHandler* output = getNewHandler(currentArgument, argv,format);
	RelationalAlgebra processor;
	processor.unionOperator(*input1, *input2, *output);
	delete input1;
	delete input2;
	delete output;
}

string obtainCombinedFormat(VLRFileHandler* handler1,
		VLRFileHandler* handler2) {
	string combinationFormat = handler1->getFormatAsString() + ",i4,"
			+ handler2->getFormatAsString();
	return combinationFormat;
}

/**/
void performProduct(int& currentArgument, char* argv[]) {
	string input1Name(argv[++currentArgument]);
	string input2Name(argv[++currentArgument]);
	VLRFileHandler* input1 = getHandler(input1Name);
	VLRFileHandler* input2 = getHandler(input2Name);
	VLRFileHandler* output = getNewHandler(currentArgument, argv,obtainCombinedFormat(input1,input2));
	RelationalAlgebra processor;
	processor.productOperator(*input1, *input2, *output);
	delete input1;
	delete input2;
	delete output;
}

string getProjectionFormat(VLRFileHandler* handler, string positions){
	string format=handler->getFormatAsString();
	replace(format.begin(), format.end(), ',', ' ');
	replace(positions.begin(), positions.end(), ',', ' ');
	stringstream ss1(format);
	string field;
	vector<string> formatVector;
	while (ss1 >> field) {
		formatVector.push_back(field);
	}
	string finalFormat;
	stringstream ss2(positions);
	uint pos;
	while(ss2>>pos){
		if(pos<formatVector.size()+1 && pos>0){
			finalFormat+=formatVector[pos-1]+",";

		}
	}
	return finalFormat;
}

void performProjection(int& currentArgument, char* argv[]) {
	string inputName(argv[++currentArgument]);
	VLRFileHandler* input = getHandler(inputName);
	string formatPos(argv[++currentArgument]);
	string format=getProjectionFormat(input,formatPos);
	VLRFileHandler* output = getNewHandler(currentArgument, argv, format);
	RelationalAlgebra processor;
	processor.projectionOperator(*input, *output, "0,"+formatPos);//include id as first
	delete input;
	delete output;
}

condition_t obtainCondition(char* argv[], int& currentArgument) {
	condition_t condition;
	condition.pos = atoi(argv[++currentArgument]);
	Field reference;
	reference.type = Field::typeFromString(argv[++currentArgument]);
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
	VLRFileHandler* input = getHandler(inputName);
	VLRFileHandler* output = getNewHandler(currentArgument, argv,input->getFormatAsString());
	condition_t condition = obtainCondition(argv, currentArgument);
	RelationalAlgebra processor;
	processor.selectionOperator(*input, *output, condition);
	delete input;
	delete output;
}

void performDifference(int& currentArgument, char* argv[]){
	string input1Name(argv[++currentArgument]);
	string input2Name(argv[++currentArgument]);
	VLRFileHandler* input1 = getHandler(input1Name);
	VLRFileHandler* input2 = getHandler(input2Name);
	VLRFileHandler* output = getNewHandler(++currentArgument, argv,input1->getFormatAsString());
	RelationalAlgebra processor;
	processor.differenceOperator(*input1, *input2, *output);
	delete input1;
	delete input2;
	delete output;
}

void performNatJoin(int& currentArgument, char* argv[]){
	string input1Name(argv[++currentArgument]);
	string input2Name(argv[++currentArgument]);
	VLRFileHandler* input1 = getHandler(input1Name);
	VLRFileHandler* input2 = getHandler(input2Name);
	string combinedFormat= input1->getFormatAsString()+","+ input2->getFormatAsString();
	VLRFileHandler* output = getNewHandler(++currentArgument, argv,combinedFormat);
	RelationalAlgebra processor;
	processor.naturalJoin(*input1, *input2, *output, currentArgument);
	delete input1;
	delete input2;
	delete output;
}

/*ugly interface, so hardcoded positions, repeated code and so on.*/
void normalExecution(char* argv[]) {
	int currentArgument = 1;
	string mode(argv[currentArgument]);
	if (mode == "-h") {
		imprimirAyuda();
	} else if (mode == "-imp") {
		importFromCsv(currentArgument, argv);
	} else if (mode == "-exp") {
		exportToCsv(currentArgument, argv);
	} else if (mode == "-u") {
		performUnion(currentArgument, argv);
	} else if (mode == "-pd") {
		performProduct(currentArgument, argv);
	} else if (mode == "-pj") {
		performProjection(currentArgument, argv);
	} else if (mode == "-s") {
		performSelection(currentArgument, argv);
	} else if (mode == "-d") {
		performSelection(currentArgument, argv);
	} else if (mode == "-nj") {
		performNatJoin(currentArgument, argv);
	}
}

#include "tests.h" //comentar/descomentar para desactivar/activar tests
#include "Index/TreeTest.h" //comentar/descomentar para desactivar/activar tests

int main(int argc, char* argv[]) {
	runTests();//comentar/descomentar para desactivar/activar tests
	runTreeTests();//comentar para desactivar tests
	//normalExecution(argv); //descomentar/comentar para activar/desactivar uso por consola
	return 0;
}
