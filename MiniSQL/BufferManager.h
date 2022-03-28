#ifndef _BUFFERMANAGER_H
#define _BUFFERMANAGER_H
#include "base.h"
#include <fstream>
struct insertPos {
	int Pos[2];
};
class buffer{
public:
	string filename;
	bool isWritten;
	bool isValid;
	bool isLocked;
	bool isLRU;
	int blockOffset;
	int LRUvalue;		
	char values[BLOCKSIZE + 1];
	buffer(){
		initialize();
	}
	void initialize(){
		isWritten = 0;
		isValid = 0;
		isLocked = 0;
		filename = "NULL";
		blockOffset = 0;
		LRUvalue = 0;
		memset(values,EMPTY,BLOCKSIZE); 
		values[BLOCKSIZE] = '\0';
	}
	string getdata(int pos, int length){
		string temp = "";
		if (pos >= 0) {
			for (int i = pos; i < pos + length; i++)
				temp += values[i];
		}
		return temp;
	}
};

class BufferManager
{
public:
	BufferManager();
	~BufferManager();
	void initialize(int i);
	void Writeback(int num);
	int getbufferNum(string name, int offset);
	void readBlock(string name, int offset, int num); 
	void writeBlock(int num);
	void useBlock(int num); 
	int getEmptyBuffer(string name, int i);
	insertPos getInsertPosition(Table& tableinfor);
	int IsDataInBuffer(string name, int offset);
	void UpdateLRU();
	buffer bufferBlock[BLOCKNUM];
};


#endif
