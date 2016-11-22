/*
 * BPlusTree.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: nicolas
 */

#include "BPlusTree.h"

#include <algorithm>
#include <iterator>

#include "BNode.h"

//overrides previous index at file
BPlusTree::BPlusTree(std::string fileName)
:file(fileName,BLOCK_SIZE),
 last(0){
	root=new LeafNode(this);
	//todo be able to read existing index
}

BPlusTree::~BPlusTree(){
	//todo save last pos
	delete root;
}

void BPlusTree::insert(pair_t element){
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

/*writes node into appropiate pos. returns true if write succesfull, false if overflow*/
bool BPlusTree::write(std::vector<char>& nodeData, uint relPos) {
	if(file.write(nodeData,relPos))
		return false;
	else
		return true;
}

/*gets serialized data of node in relPos and stores in node data*/
void BPlusTree::read(std::vector<char>& nodeData, uint relPos) {
	file.read(nodeData,relPos);
}

uint BPlusTree::getNextPos() {
	return ++last;
}

