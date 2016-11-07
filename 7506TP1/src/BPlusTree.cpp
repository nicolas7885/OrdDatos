/*
 * BPlusTree.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: nicolas
 */

#include "BPlusTree.h"

//overrides previous index at file
BPlusTree::BPlusTree(std::string fileName)
:file(fileName,std::ios_base::in | std::ios_base::out | std::ios_base::binary),
 last(0){
	//todo be able to read existing index
	root=new LeafNode(this);
}

BPlusTree::~BPlusTree(){
	//todo save root into file if modified
	//todo save last pos
	delete root;
}

void BPlusTree::insert(pair element){
	root->insert(element);
	if(root->shouldSplit()){
		InnerNode* newRoot= new InnerNode(this,root->getHeight()+1,last+1);
		root->split(newRoot);
		delete root;
		root=newRoot;
	}
}

/*return true and writes into rersult if found. else returns false*/
bool BPlusTree::find(int key,uint& result){
	return root->find(key,result);
}

/*writes node into appropiate pos*/
void BPlusTree::write(std::vector<int>& nodeData, uint relPos) {
	file.seekp(relPos*NODE_SIZE);
	file.write((char*)&nodeData[0],NODE_SIZE);
}

/*gets serialized data of node in relPos and stroes in node data*/
void BPlusTree::read(std::vector<int>& nodeData, uint relPos) {
	file.seekg(relPos*NODE_SIZE);
	file.read((char*)&nodeData[0],NODE_SIZE);
}

uint BPlusTree::getNextPos() {
	return ++last;
}

//checks if file exists
bool BPlusTree::checkFileExistance(std::string fileName){
	std::ifstream fs(fileName);
	return fs;
}

