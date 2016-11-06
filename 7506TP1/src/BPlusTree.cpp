/*
 * BPlusTree.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: nicolas
 */

#include "BPlusTree.h"

//overrides previous index at file
BPlusTree::BPlusTree(std::string fileName)
:file(fileName,std::ios_base::in | std::ios_base::out | std::ios_base::binary){
	//todo be able to read existing index
	root=new LeafNode(file,0);
}

BPlusTree::~BPlusTree(){
	//todo save root into file if modified
	delete root;
}

void BPlusTree::insert(pair element){
	root->insert(element);
}

/*return the value of the corresponding key, or the closest one to it.*/
bool BPlusTree::find(int key,uint& result){
	return root->find(key);
}

//checks if file exists
bool BPlusTree::checkFileExistance(std::string fileName){
	std::ifstream fs(fileName);
	return fs;
}

