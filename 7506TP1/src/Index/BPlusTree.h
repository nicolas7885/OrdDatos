/*
 * BPlusTree.h
 *
 *  Created on: Nov 4, 2016
 *      Author: nicolas
 */

#ifndef SRC_INDEX_BPLUSTREE_H_
#define SRC_INDEX_BPLUSTREE_H_

#include <string>
#include <vector>

#include "../FileHandlers/BlockFileHandler.h"

struct pair_t;
class TreeNode;

typedef unsigned int uint;

class BPlusTree {
private:
	TreeNode* root;
	BlockFileHandler file;
	uint last;

public:
	//overrides previous index at file
	BPlusTree(std::string fileName);
	virtual ~BPlusTree();
	void insert(pair_t element);
	bool find(int key,uint& result);
	void write(std::vector<char>& nodeData, uint relPos);
	void read(std::vector<char>& nodeData, uint relPos);
	uint getNextPos();
};

#endif /* SRC_INDEX_BPLUSTREE_H_ */
