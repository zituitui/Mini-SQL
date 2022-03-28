#ifndef _RECORDMANAGER_H
#define	_RECORDMANAGER_H
#include "Catalog.h"
#include <cmath>
#include <windows.h>
class RecordManager
{
public:
	RecordManager(BufferManager *bf) :bp(bf) {}
	~RecordManager() {};
	bool CreateTable(Table& tableIn);
	bool DropTable(Table& tableIn);
	int Delete(Table& tableIn, vector<where> w);
	Table Select(Table& tableIn, vector<where>& w);

	string int2string(int x);
	int FindAttr(Table& t, int i);
	void DeleteTableInMemory(string filename);
	string FindUnique(Table& tableIn);
	bool CheckRedundancy(Table& tableIn, TableTuple& singleTuper);
	bool isSatisfied_i(int x1, int x2, int logic);
	bool isSatisfied_f(float x1, float x2, int logic);
	bool isSatisfied_c(string x1, string x2, int logic);
	bool isSatisfied(TableTuple& row, vector<where> w); 
	void AddPtr(Table& t, TableTuple& tup, int i, int pos, char* ptrRes);
	void InsertWithIndex(Table& tableIn, TableTuple& singleTuper);
	char* TupertoChar(Table& tableIn, TableTuple& singleTuper);
private:
	BufferManager *bp;
};

#endif
