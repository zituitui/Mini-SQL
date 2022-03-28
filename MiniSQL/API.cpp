
#include "API.h"
#include "IndexManager.h"

API::~API()
{
}

Table API::Select(Table& tableIn, vector<where> w){
	return rm.Select(tableIn, w);
}

int API::Delete(Table& tableIn, vector<where> w)
{
	int res;
	res = rm.Delete(tableIn, w);
	return res;
}

void API::Insert(Table& tableIn, TableTuple& singleTuper)
{
	rm.InsertWithIndex(tableIn, singleTuper);
}

bool API::DropTable(Table& tableIn)
{
	bool res;
	res = rm.DropTable(tableIn);
	for (int i = 0; i < tableIn.index.size();i++) {
		DropIndex(tableIn, tableIn.index[i].location);
	}
	return res;
}

void API::DropIndex(Table& tableIn, int attr)
{
	IndexManager indexMA;
	string filename;
	filename = tableIn.Tname + to_string(attr) + "_index";
	indexMA.Drop(filename);
}

bool API::CreateTable(Table& tableIn)
{
	IndexManager indexMA;
	bool res;
	int i;
	ofstream f_out("table_name");
	streambuf* coutr;
	coutr = std::cout.rdbuf(f_out.rdbuf());
	cout << tableIn.Tname << endl;
	cout.rdbuf(coutr);
	tablename.name.push_back(tableIn.Tname);
	res = rm.CreateTable(tableIn);
	return true;	
}

bool API::CreateIndex(Table& tableIn, int attr)
{
	IndexManager indexMA;
	string file_name;
	file_name = tableIn.Tname + to_string(attr) + "_index";
	indexMA.Establish(file_name);
	vector<where> w;
	Table t(rm.Select(tableIn, w));
	vector<Attribute> att(1);
	att[0].flag = t.attr[attr].flag;
	att[0].name = t.attr[attr].name;
	att[0].unique = t.attr[attr].unique;
	Table tableForindex(t.Tname, att, t.index, t.blockNum, t.primary);
	for (int i = 0; i < tableForindex.T.size(); i++) {
		indexMA.Insert(file_name, tableForindex.T[i]->data[attr], i);
	}
	return true;
}

