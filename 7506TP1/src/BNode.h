/*
 * BNode.h
 *
 *  Created on: Nov 4, 2016
 *      Author: nicolas
 */

#ifndef SRC_BNODE_H_
#define SRC_BNODE_H_

#include <vector>
#include <fstream>

//NODE_SIZE must be multiple of sizeof(int) and a power of 2
#define NODE_SIZE 512
#define N (NODE_SIZE-(2+1)*sizeof(int))/(2*sizeof(int))
#define M (NODE_SIZE-3*sizeof(int))/sizeof(unsigned int)

/*implementacion de arbol para unsigned int como valor e int como indice.
 * Maneja overflows, pero no underflows(no borra elementos)*/
typedef unsigned int uint;

struct pair{
	uint value;
	int key;
};

class TreeNode{
protected:
	uint height;
	uint load;
	std::fstream& fs;
	uint pos;

public:
	TreeNode(uint height, std::fstream& fs, uint pos);
	virtual ~TreeNode();
	uint getHeight();
	/*inserts an element into the node,returns true if split needed*/
	virtual bool insert(pair element)=0;
	/*writes node data into corresponding file pos*/
	virtual void write()=0;
	/*returns true if found and writes value to result.
	 * if not, returns false and doesnt change result */
	virtual bool find(int key, uint&result)=0;

	static TreeNode* read(std::fstream& fs,uint relPos);
};

/********************************************************************************/
class InnerNode:public TreeNode{
	std::vector<int> keys;
	std::vector<uint> children;

public:
	InnerNode(int height, std::fstream& fs, uint pos, std::vector<int> &keys,std::vector<uint> &children);
	virtual ~InnerNode();
	int findKeyInsertPos(int key);
	bool insert(pair element);
	bool insert(int key,uint child);
	void write();
	bool find(int key, uint&result);
};

/********************************************************************************/
class LeafNode:public TreeNode{
	std::vector<pair> elements;
	uint next;

public:
	LeafNode(std::fstream& fs, uint pos);
	LeafNode(std::fstream& fs, uint pos,std::vector<pair>& elements, uint next);
	virtual ~LeafNode();
	bool insert(pair element);
	void changeNext(uint next);
	void write();
	bool find(int key, uint&result);

private:
	int findElementInsertPos(int elementKey);
};
#endif /* SRC_BNODE_H_ */
