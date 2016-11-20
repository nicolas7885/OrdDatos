/*
 * BNode.h
 *
 *  Created on: Nov 4, 2016
 *      Author: nicolas
 */

#ifndef SRC_INDEX_BNODE_H_
#define SRC_INDEX_BNODE_H_

#include <vector>

class BPlusTree;
class InnerNode;

//NODE_SIZE must be multiple of sizeof(int) and a power of 2
#define NODE_SIZE 512
#define N (NODE_SIZE-(2+1)*sizeof(int))/(2*sizeof(int))
#define M (NODE_SIZE-3*sizeof(int))/(sizeof(int)+sizeof(unsigned int))

/*implementacion de arbol para unsigned int como valor e int como indice.
 * Maneja overflows, pero no underflows(no borra elementos)*/
typedef unsigned int uint;

struct pair_t{
	uint value;
	int key;
};

class TreeNode{
protected:
	BPlusTree* tree;
	uint height;
	uint relPos;
	uint load;
public:
	TreeNode(BPlusTree* tree, uint height, uint pos);
	virtual ~TreeNode();
	uint getHeight();
	/*inserts an element into the node*/
	virtual void insert(pair_t element)=0;
	/*writes node data into corresponding file pos*/
	virtual void write()=0;
	/*returns true if found and writes value to result.
	 * if not, returns false and doesnt change result */
	virtual bool find(int key, uint&result)=0;
	/*returns true if node should be split*/
	virtual bool shouldSplit()=0;
	/*splits node, creating new node, and adding key and child reference to parent.*/
	virtual void split(InnerNode* parent)=0;
	/*reads new node from tree*/
	static TreeNode* read(BPlusTree* tree,uint relPos);
};

/********************************************************************************/
class InnerNode:public TreeNode{
	std::vector<int> keys;
	std::vector<uint> children;

public:
	InnerNode(BPlusTree* tree, uint height,uint oldRoot);
	InnerNode(BPlusTree* tree, uint height, uint pos, std::vector<int> &keys,std::vector<uint> &children);
	virtual ~InnerNode();
	int findKeyInsertPos(int key);
	void insert(pair_t element);
	void insert(int key,uint child);
	void write();
	bool find(int key, uint&result);
	bool shouldSplit();
	void split(InnerNode* parent);
};

/********************************************************************************/
class LeafNode:public TreeNode{
	std::vector<pair_t> elements;
	uint next;

public:
	LeafNode(BPlusTree* tree);
	LeafNode(BPlusTree* tree, uint pos,std::vector<pair_t>& elements, uint next);
	virtual ~LeafNode();
	void insert(pair_t element);
	void changeNext(uint next);
	void write();
	bool find(int key, uint&result);
	bool shouldSplit();
	void split(InnerNode* parent);

private:
	int findElementInsertPos(int elementKey);
};
#endif /* SRC_INDEX_BNODE_H_ */
