/*
 * BNode.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: nicolas
 */
#include "BNode.h"

typedef std::vector<int>::iterator intIt_t;
typedef std::vector<uint>::iterator uintIt_t;

TreeNode::TreeNode(BPlusTree* tree,uint height, uint pos)
:tree(tree),
 height(height),
 relPos(pos),
 load(0)
{}

TreeNode::~TreeNode(){}

uint TreeNode::getHeight(){
	return height;
}

TreeNode* TreeNode::read(BPlusTree* tree,uint relPos) {
	uint pos= relPos*NODE_SIZE;
	std::vector<int> data(NODE_SIZE/sizeof(int));
	tree->read(data,relPos);
	intIt_t dataIt=data.begin()+2;
	int load=data[1];
	TreeNode* node;
	if(data[0]==0){
		std::vector<pair> elements;
		for(int i=0; i<load; i++,dataIt+=2){
			pair p={(uint)*(dataIt+1),*dataIt};
			elements.push_back(p);
		}
		node=new LeafNode(tree,relPos,elements,*dataIt);
	}else{
		std::vector<int> keys(load);
		for(int i=0; i<load; i++,dataIt++){
			keys.push_back(*dataIt);
		}
		std::vector<uint> children(load+1);
		for(int i=0; i<load+1; i++,dataIt++){
			children.push_back(*dataIt);
		}
		node= new InnerNode(data[0],tree,relPos,keys,children);
	}
	return node;
}
/********************************************************************************/
/*creates new InnerNode as root of a tree*/
InnerNode::InnerNode(BPlusTree* tree, uint height, uint oldRoot)
:TreeNode(tree,height,0){
	children.push_back(oldRoot);
}

/*creates node with certain height, set of keys, and children. writes node in tree*/
InnerNode::InnerNode(BPlusTree* tree, uint height,uint pos,
		std::vector<int> &keys,std::vector<uint> &children)
:TreeNode(tree,height,pos),
 keys(keys),
 children(children){
	load=this->keys.size();
	this->write();
}

//todo chain delete
InnerNode::~InnerNode(){}

/*returns pos in which key should be inserted*/
int InnerNode::findKeyInsertPos(int key){
	if(keys.size()==0) return 0;//impossible
	for(uint i=0; i<keys.size(); i++){
		if(keys[i]>key) return i;
	}
	return keys.size();
}
/*inserts pair key/value into leaf in child nodes. Splits nodes if neccessary.*/
void InnerNode::insert(pair element){
	uint insertPos=findKeyInsertPos(element.key);
	TreeNode* insertionNode=TreeNode::read(tree,children[insertPos]);
	insertionNode->insert(element);
	if(insertionNode->shouldSplit()){
		insertionNode->split(this);
		if(!this->shouldSplit())
			this->write();//if child node split then this node changed
	}
	delete insertionNode;
}

/*inserts key and child into node.*/
void InnerNode::insert(int key,uint child){
	uint insertPos=findKeyInsertPos(key);
	if (insertPos==keys.size()){
		keys.push_back(key);
		children.push_back(child);
	}else{
		keys.insert(keys.begin()+insertPos,key);
		children.insert(children.begin()+insertPos+1,child);
	}
	load++;
}

/*writes in file the data of the node(complete rewrite)*/
void InnerNode::write() {
	std::vector<int> data;
	data.push_back(height);
	data.push_back(load);
	for(intIt_t it=keys.begin();it!=keys.end(); it++){
		data.push_back(*it);
	}
	for(uintIt_t it=children.begin();it!=children.end(); it++){
		data.push_back(*it);
	}
	data.resize(NODE_SIZE/sizeof(int));//should be same or bigger
	tree->write(data,relPos);
}

/*returns true if found and writes value to result.
 * if not, returns false and doesnt change result */
bool InnerNode::find(int key,uint& result) {
	uint nextPos=findKeyInsertPos(key);
	TreeNode* nextNode=TreeNode::read(tree,children[nextPos]);
	bool found=nextNode->find(key,result);
	delete nextNode;
	return found;
}

bool InnerNode::shouldSplit() {
	if(keys.size()==N){
		return true;
	}else{
		return false;
	}
}

/*splits node, creating new node, and adding key and child reference to parent.*/
void InnerNode::split(InnerNode* parent) {
	if(relPos==0)
		relPos=tree->getNextPos();
	int const halfSize=keys.size()/2;
	std::vector<int> brotherKeys(keys.begin()+halfSize+1,keys.end());//skip middle key
	int upKey=keys[halfSize];
	keys.resize(halfSize);
	std::vector<int> brotherChildren(children.begin()+halfSize+1,children.end());
	children.resize(halfSize+1);
	uint brotherPos=tree->getNextPos();
	InnerNode brother(height,tree,brotherPos,brotherKeys,brotherChildren);
	this->write();
	parent->insert(upKey,brotherPos);
}

/********************************************************************************/
/*creates empty leaf node(for root)*/
LeafNode::LeafNode(BPlusTree* tree)
:TreeNode(tree,0,0),
 next(0){}

/*creates leaf wtih set of elements. writes itself into tree*/
LeafNode::LeafNode(BPlusTree* tree, uint pos, std::vector<pair>& elements, uint next)
:TreeNode(tree,0,pos),
 elements(elements),
 next(next){
	this->write();
}

LeafNode::~LeafNode(){}

/*inserts element in node. writes node in tree*/
void LeafNode::insert(pair element){
	uint insertPos=findElementInsertPos(element.key);
	if(insertPos!=0 && elements[insertPos-1].key==element.key){
		//key alredy in tree, replace value
		elements[insertPos-1].value=element.value;
	}else{
		if(insertPos==elements.size()){
			elements.push_back(element);
		}else{
			elements.insert(elements.begin()+insertPos,element);
		}
		load++;
		this->write();
	}
}

/*changes next node*/
void LeafNode::changeNext(uint next){
	this->next=next;
}

/*writes in tree the data of the node(complete rewrite)*/
void LeafNode::write() {
	std::vector<int> data;
	data.push_back(height);
	data.push_back(load);
	for(std::vector<pair>::iterator it=elements.begin();it!=elements.end(); it++){
		data.push_back(it->key);
		data.push_back(it->value);
	}
	data.push_back(next);
	data.resize(NODE_SIZE/sizeof(int));//should be same or bigger
	tree->write(data,relPos);
}

/*returns true if found and writes value to result.
 * if not, returns false and doesnt change result */
bool LeafNode::find(int key, uint&result){
	for(uint i=0; i<elements.size(); i++){
		if(elements[i].key==key){
			result=elements[i].value;
			return true;
		}
	}
	return false;
}

/*returns true if node should be split*/
bool LeafNode::shouldSplit() {
	if(elements.size()==M){
		return true;
	}else{
		return false;
	}
}

/*splits node, creating new node, and adding key and child reference to parent.
 * If node was root, changes its relPos to next one available*/
void LeafNode::split(InnerNode* parent) {
	if(relPos==0)
		relPos=tree->getNextPos();
	std::size_t const halfSize=elements.size()/2;
	std::vector<pair> brotherElements(elements.begin()+halfSize,elements.end());
	elements.resize(halfSize);
	uint brotherPos=tree->getNextPos();
	LeafNode brother(tree,brotherPos,brotherElements,next);
	next=brotherPos;
	this->write();
	parent->insert(elements[halfSize-1].key,brotherPos);
}

int LeafNode::findElementInsertPos(int elementKey){
	if(elements.size()==0) return 0;
	for(uint i=0; i<elements.size(); i++){
		if(elements[i].key>elementKey) return i;
	}
	return elements.size();
}
