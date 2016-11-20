/*
 * BPlusTree.h
 *
 *  Created on: Nov 4, 2016
 *      Author: nicolas
 */

#ifndef SRC_INDEX_BPLUSTREE_H_
#define SRC_INDEX_BPLUSTREE_H_

#include <string>
#include <fstream>
#include <vector>

#include "../Index/BNode.h"

typedef unsigned int uint;

class BPlusTree {
private:
	TreeNode* root;
	std::fstream file;
	uint last;

public:
	//overrides previous index at file
	BPlusTree(std::string fileName);
	virtual ~BPlusTree();
	void insert(pair_t element);
	bool find(int key,uint& result);
	void write(std::vector<int>& nodeData, uint relPos);
	void read(std::vector<int>& nodeData, uint relPos);
	uint getNextPos();

private:
	bool checkFileExistance(std::string fileName);

};

#endif /* SRC_INDEX_BPLUSTREE_H_ */
