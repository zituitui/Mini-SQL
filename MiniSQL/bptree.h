#ifndef _BPTREE_H_
#define _BPTREE_H_
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "BufferManager.h"

#define BLOCKSIZE 4096
#define MESSAGE 24
#define FOUR_BYTE 4  

#define POINTERLENGTH 20


using namespace std;

enum nodetype { Internal, Leaf };
extern BufferManager bf;
class index {
public:
	int Number;			
	int keylength[3];	

private:
	int maxchild;
	int order;
	int type;
	string name;
public:
	index(string filename);
	~index() {};
	void initialize(Data* key, int Addr, int ktype);
	int find(Data* key);
	void insert(Data* key, int Addr);
	int* split(char* currentBlock, Data* mid, Data* key, int Addr, int leftpos, int rightpos);
	void Internal_insert(char* currentBlock, Data* mid, int leftpos, int rightpos);
	void SplitLeaf(char* block1, char* block2, char* currentBlock, Data* key, int Addr);
	void SplitInternal(char* block1, char* block2, char* currentBlock, Data* mid, int leftpos, int rightpos);
	void Delete(Data* key);
	int* Range(Data* key1, Data* key2);
};

#endif