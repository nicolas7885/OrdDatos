/*
 * TreeTesr.h
 *
 *  Created on: Nov 5, 2016
 *      Author: nicolas
 */

#ifndef SRC_TREETEST_H_
#define SRC_TREETEST_H_

#include "BPlusTree.h"
#include <limits>
#include <iostream>

void insertManyNodes(const int amountOfNodes) {
	bool failed=false;
	BPlusTree tree("treeTest.bin");
	for (int i = 0; i < amountOfNodes; i++) {
		pair_t aPair = {(uint)i*2, i };
		tree.insert(aPair);
	}
	int i = amountOfNodes / 2;//todo randomize
	uint expectedResult =i*2;
	uint result=0;
	if (!tree.find(i, result)){
		std::cout << "insert "<<amountOfNodes<<" (ordered) failed(not found)"
				<< std::endl;
		failed=true;
	}
	if (result != expectedResult){
		std::cout << "insert "<<amountOfNodes<<" (ordered) failed(value wrong)"
				<< std::endl;
		failed=true;
	}
	if(!failed)std::cout <<"insert "<<amountOfNodes<<" (ordered) success"<<std::endl;
}

void insertManyNodesUnordered(const int amountOfNodes) {
	bool failed=false;
	BPlusTree tree("treeTest.bin");
	for (int i = 0; i < amountOfNodes; i++) {
		pair_t aPair = {(uint)i*2,amountOfNodes - i };
		tree.insert(aPair);
	}
	int i = amountOfNodes / 2;//todo randomize
	uint expectedResult = i*2;
	uint result=0;
	if (!tree.find(amountOfNodes - i , result)){
		std::cout << "insert "<<amountOfNodes<<" (unordered) failed(not found)"
				<< std::endl;
		failed=true;
	}
	if (result != expectedResult){
		std::cout << "insert "<<amountOfNodes<<" (unordered) failed(value wrong)"
				<< std::endl;
		failed=true;
	}
	if(!failed)std::cout <<"insert "<<amountOfNodes<<" (unordered) success"<<std::endl;
}

void runTreeTests(){
	//scopes left over from before refactoring
	// warning N and M depend on a specific implementation of the file.
	//If write and read change N and M should too
	{
		//insert one, recover it.
		bool failed=false;
		BPlusTree tree("treeTest.bin");
		pair_t first={std::numeric_limits<uint>::max(),1};
		tree.insert(first);
		uint result;
		if(!tree.find(first.key,result)){
			std::cout<<"insert one failed(not found)"<<std::endl;
			failed=true;
		}
		if(result!=std::numeric_limits<uint>::max()){
			std::cout<<"insert one failed(value wrong)"<<std::endl;
			failed=false;
		}
		if(!failed) std::cout <<"insert one success"<<std::endl;
		//insert with same key, replaces it.
		first.value=0;
		tree.insert(first);
		tree.find(first.key,result);
		if(result!=0){
			std::cout<<"replace insert failed(value wrong)"<<std::endl;
			failed=true;
		}
		if(!failed)std::cout <<"replace insert success"<<std::endl;
	}
	{
		//put many elements in(should be one node only), in order, can find one of them
		const int amountOfNodes=M-2;
		insertManyNodes(amountOfNodes);
	}
	{
		//put many elements in, out of order, can find one of them
		const int amountOfNodes=M-2;
		insertManyNodesUnordered(amountOfNodes);
	}
	{
		//split leaf, can still find one element
		const int amountOfNodes=M;
		insertManyNodes(amountOfNodes);
	}
	{
		//put many nodes in, in order, can find one element of them
		const int amountOfNodes=M*2-2;
		insertManyNodes(amountOfNodes);
	}
	{
		//put many nodes in, out of order, can find one of them
		const int amountOfNodes=M*2-2;
		insertManyNodesUnordered(amountOfNodes);
	}
	{
		//split inner node, can still find one element
		const int amountOfNodes=M*(N/2)+M/2;//dependent on current size
		insertManyNodes(amountOfNodes);
	}
	{
		//put many many nodes in, can find one of them
		const int amountOfNodes=M*N;
		insertManyNodes(amountOfNodes);
	}
}



#endif /* SRC_TREETEST_H_ */
