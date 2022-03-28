#pragma once
#ifndef _API_H
#define _API_H
#include "base.h"
#include "RecordManager.h"
extern BufferManager bf;

class API
{
public:
	API() :rm(&bf){}
	~API();
	Table Select(Table& tableIn, vector<where> w);//return a table containing select results
	int Delete(Table& tableIn, vector<where> w);
	void Insert(Table& tableIn, TableTuple& singleTuper);
	bool DropTable(Table& tableIn);
	void DropIndex(Table& tableIn, int attr);
	bool CreateTable(Table& tableIn);
	bool CreateIndex(Table& tableIn, int attr);
private:
	RecordManager rm;
};


#endif 
