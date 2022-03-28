#ifndef base_h
#define base_h
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include <stdlib.h> 
using namespace std;

#define BLOCKSIZE 4096
#define BLOCKNUM 100
#define EMPTY  '#'
#define NOTEMPTY '$'

struct {
	vector<std::string> name;
}tablename;
struct {
	vector<std::string> aname;
	vector<std::string> iname;
}CheckIndex;

class Data {
public:
	short flag;
};

class Data_i : public Data {
public:
	int x;
	Data_i() {
		x = 0;
		flag = -1;
	};
	Data_i(int i) {
		x = i;
		flag = -1;
	};
};

class Data_f : public Data {
public:
	float x;
	Data_f() {
		x = 0;
		flag = 0;
	};
	Data_f(float i) {
		x = i;
		flag = 0;
	};
};

class Data_c : public Data {
public:
	std::string x;
	Data_c(std::string c) {
		x = c;
		flag = (c.length()) ? c.length() : 1;
	};
};


struct Attribute {
	short flag;//data type
	std::string name;//attribute name
	bool unique;//unique
};

struct Index {
	short location;
	std::string indexname;
};
class TableTuple {
public:
	std::vector<Data*> data;
	TableTuple() {};
	TableTuple(const TableTuple& t);
	~TableTuple();
	int length() const {
		return (int)data.size();
	}
	Data* operator[](unsigned short i);
	void Print();
	void Print(vector<int>mask, int chart_len);
};
class Table {
	friend class CatalogManager;
public:
	std::string Tname;
	std::vector<Attribute> attr;//attributes
	std::vector<TableTuple*> T;//pointers to each TableTuple
	std::vector<Index> index;
	short primary;//the location of primary key. -1 means no primary key.
	int blockNum;//total number of blocks occupied in data file;
	int dataSize;
	Table(std::string s, vector<Attribute>aa, vector<Index>ii, int bn, int pri)
	{
		Tname = s;
		attr = aa;
		blockNum = bn;
		index = ii;
		if (pri == -1)
		{
			primary = -1;
			for (int i = 0; i < aa.size(); i++) { aa[i].unique = false; }
		}
		else
		{
			primary = pri;
			for (int i = 0; i < aa.size(); i++)
			{
				if (i == pri) aa[i].unique = true;
				else aa[i].unique = false;
			}
		}
		dataSize = 0;
		for (int i = 0; i < attr.size(); i++) {
			switch (attr[i].flag) {
			case -1:
				dataSize += sizeof(int); 
				break;
			case 0:
				dataSize += sizeof(float); 
				break;
			default:
				dataSize += attr[i].flag + 1; 
				break; 
			}
		}
	}
	Table(const Table& t);
	~Table();
	void setindex(Table* t, short i, std::string iname);
	void dropindex(std::string iname);
	void Print();
	void Print(vector<int>mask);
	void addData(TableTuple* t);
	int hasColumn(std::string name);
};
struct where{
	Data* d;
	int flag;
	int column;
};

class TableException : public std::exception {
public:
	TableException(std::string s) :text(s) {}
	std::string what() {
		return text;
	};
private:
	std::string text;
};

#endif 
