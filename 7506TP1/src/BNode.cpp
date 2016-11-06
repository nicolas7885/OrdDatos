/*
 * BNode.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: nicolas
 */
#include "BNode.h"

typedef std::vector<int>::iterator intIt_t;
typedef std::vector<uint>::iterator uintIt_t;

TreeNode::TreeNode(uint height, std::fstream& fs, uint pos)
:height(height),
 load(0),
 fs(fs),
 pos(pos)
{}

TreeNode::~TreeNode(){}

uint TreeNode::getHeight(){
	return height;
}

TreeNode* TreeNode::read(std::fstream& fs,uint relPos) {
	uint pos= relPos*NODE_SIZE;
	std::vector<int> data(NODE_SIZE/sizeof(int));
	fs.seekg(pos);
	fs.read((char*)&data[0],NODE_SIZE);
	intIt_t dataIt=data.begin()+2;
	int load=data[1];
	TreeNode* node;
	if(data[0]==0){
		std::vector<pair> elements;
		for(int i=0; i<load; i++,dataIt+=2){
			pair p={(uint)*(dataIt+1),*dataIt};
			elements.push_back(p);
		}
		node=new LeafNode(fs,relPos,elements,*dataIt);
	}else{
		std::vector<int> keys(load);
		for(int i=0; i<load; i++,dataIt++){
			keys.push_back(*dataIt);
		}
		std::vector<uint> children(load+1);
		for(int i=0; i<load+1; i++,dataIt++){
			children.push_back(*dataIt);
		}
		node= new InnerNode(data[0],fs,relPos,keys,children);
	}
	return node;
}
/********************************************************************************/

/*creates node with certain height, set of keys and children*/
InnerNode::InnerNode(int height, std::fstream& fs, uint pos,
		std::vector<int> &keys,std::vector<uint> &children)
:TreeNode(height,fs,pos),
 keys(keys),
 children(children){
	load=this->keys.size();
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
/*inserts pair key/value into leaf in child nodes. Splits nodes if neccessary.
 * returns true if node must be split.*/
bool InnerNode::insert(pair element){
	uint insertPos=findKeyInsertPos(element.key);
	TreeNode* insertionNode=TreeNode::read(fs,children[insertPos]);
	if(insertionNode->insert(element)){
		//node split needed
		//todo split node
		return true;
	}else{
		return false;
	}
}

/*inserts key and child into node. returns true if it must be split*/
bool InnerNode::insert(int key,uint child){
	uint insertPos=findKeyInsertPos(key);
	if (insertPos==keys.size()){
		keys.push_back(key);
		children.push_back(child);
	}else{
		keys.insert(keys.begin()+insertPos,key);
		children.insert(children.begin()+insertPos+1,child);
	}
	load++;
	if(keys.size()==N){
		//split needed
		return true;
	}else{
		return false;
	}
}

/*writes in file the data of the node*/
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
	fs.write((char*)&data[0],NODE_SIZE);
}

/*returns true if found and writes value to result.
 * if not, returns false and doesnt change result */
bool InnerNode::find(int key,uint& result) {
	//todo hidrate children and find in them
	return 0;
}

/********************************************************************************/
/*creates empty leaf node(for root)*/
LeafNode::LeafNode(std::fstream& fs, uint pos)
:TreeNode(0,fs,pos),
 next(0){}

/*creates leaf wtih set of elements*/
LeafNode::LeafNode(std::fstream& fs, uint pos, std::vector<pair>& elements, uint next)
:TreeNode(0,fs,pos),
 elements(elements),
 next(next){}

LeafNode::~LeafNode(){}

/*inserts element in node. Returns true if it needs to be split*/
bool LeafNode::insert(pair element){
	uint insertPos=findElementInsertPos(element.key);
	if(insertPos==elements.size()){
		elements.push_back(element);
	}else{
		elements.insert(elements.begin()+insertPos,element);
	}
	load++;

	if(elements.size()==M){
		//split needed
		return true;
	}else{
		return false;
	}
}

/*changes next node*/
void LeafNode::changeNext(uint next){
	this->next=next;
}

/*writes in file the data of the node*/
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
	fs.write((char*)&data[0],NODE_SIZE);
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

int LeafNode::findElementInsertPos(int elementKey){
	if(elements.size()==0) return 0;
	for(uint i=0; i<elements.size(); i++){
		if(elements[i].key>elementKey) return i;
	}
	return elements.size();
}


