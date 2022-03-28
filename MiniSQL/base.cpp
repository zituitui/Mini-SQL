#include "base.h"
#include <string>
#include <iomanip>

TableTuple::TableTuple(const TableTuple& t) {
	int i;
	Data* d;
	for (i = 0; i < t.length(); i++) {
		if (t.data[i]->flag == -1) {
			d = new Data_i(((Data_i*)(t.data[i]))->x);
			data.push_back(d);
		}
		if (t.data[i]->flag == 0) {
			d = new Data_f(((Data_f*)(t.data[i]))->x);
			data.push_back(d);
		}
		if (t.data[i]->flag>0) {
			d = new Data_c(((Data_c*)(t.data[i]))->x);
			data.push_back(d);
		}
	}
}

TableTuple::~TableTuple() {
	int i;
	for (i = 0; i < data.size(); i++){
		if (data[i] == NULL);
		if (data[i]->flag == -1)
			delete (Data_i*)data[i];
		else if (data[i]->flag == 0) delete (Data_f*)data[i];
		else delete (Data_c*)data[i];
	}
}

void TableTuple::Print() {
	int i, j;
	for (i = 0; i < data.size(); i++) {
		if (data[i] == NULL)
			std::cout << "NULL           ";
		else if (data[i]->flag == -1) {
			std::cout << to_string(((Data_i*)data[i])->x);
			j = 5 - to_string(((Data_i*)data[i])->x).size();
			while (j-- > 0) cout << " ";
		}
		else if (data[i]->flag == 0) {
			float num = ((Data_f*)data[i])->x;
			std::cout << setprecision(4) << num;
			j = 5;
			while (j-- > 0) cout << " ";
		}
		else {
			std::cout << ((Data_c*)data[i])->x;
			j = 20 - ((Data_c*)data[i])->x.size();
			while (j-- > 0) cout << " ";
		}
	}
	std::cout << std::endl;
}

void TableTuple::Print(vector<int>mask, int chart_len) {
	int i, j;
	int cur_len = 0;
	std::cout << "©¦"; cur_len++;
	for (i = 0; i < mask.size(); i++) {
		if (data[mask[i]] == NULL)
		{
			std::cout << "NULL           ";
			cur_len += 16;
		}
		else if (data[mask[i]]->flag == -1) {
			std::cout << to_string(((Data_i*)data[mask[i]])->x);
			j = 5 - to_string(((Data_i*)data[mask[i]])->x).size();
			cur_len += 5;
			while (j-- >= 0) cout << " ";
		}
		else if (data[mask[i]]->flag == 0) {
			float num = ((Data_f*)data[mask[i]])->x;
			std::cout << setprecision(4) << num;
			j = 5;
			cur_len += 10;
			if (num >= 1000) j--;
			while (j-- >= 0) cout << " ";
			if (num < 100) cout << " ";
			if (num == (int)num) cout << "  ";
		}
		else {
			std::cout << ((Data_c*)data[mask[i]])->x;
			j = 18 - ((Data_c*)data[mask[i]])->x.size();
			cur_len += 20;
			while (j-- >= 0) cout << " ";
		}
	}
	for (; cur_len < chart_len; cur_len++) cout << " ";
	std::cout << "©¦";
	std::cout << std::endl;
}

Data* TableTuple::operator[](unsigned short i) {
	if (i >= data.size())
		throw std::out_of_range("out of range in t[i]");
	return data[i];
}



Table::Table(const Table& t) {
	attr = t.attr;
	Tname = t.Tname;
	primary = t.primary;
	index = t.index;
	blockNum = t.blockNum;
	int i;
	for (i = 0; i< t.T.size(); i++) {
		TableTuple* tp = new TableTuple(*(t.T[i]));
		addData(tp);
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


Table::~Table() {
	int i;
	for (i = 0; i< T.size(); i++)
		delete T[i];
}

void Table::addData(TableTuple* t) {
	if (t->length() != attr.size())
		throw TableException("[ERROR]Column size dismatches.");
	int i, j;
	for (i = 0; i<attr.size(); i++) {
		j = (*t)[i]->flag;
		if (j>attr[i].flag)
			throw TableException("[ERROR]Attribute dismatches.");
		else if (j<attr[i].flag && j <= 0)
			throw TableException("[ERROR]Attribute dismatches.");
	}
	T.push_back(t);
}

void Table::Print() {
	int i, j;
	for (i = 0; i < attr.size(); i++) {
		std::cout << attr[i].name;
		j = 15 - attr[i].name.size();
		while (j-- > 0) cout << " ";
	}
	std::cout << std::endl;
	for (i = 0; i < T.size(); i++) {
		T[i]->Print();
	}
};//display the whole table

void Table::Print(vector<int>mask) {
	int i, j;
	int chart_len = 0;
	for (i = 0; i < mask.size(); i++) {
		if (attr[mask[i]].flag == -1) {
			chart_len += 5;
		}
		else if (attr[mask[i]].flag == 0) {
			chart_len += 10;
		}
		else {
			chart_len += 20;
		}
	}
	chart_len += mask.size();
	std::cout << "©°";
	for (int k = 1; k <= chart_len; k++) std::cout << "©¤";
	std::cout << "©´" << endl;
	std::cout <<"©¦";
	for (i = 0; i < mask.size(); i++) {
		std::cout << attr[mask[i]].name;
		if (attr[mask[i]].flag == -1) {
			j = 5 - attr[mask[i]].name.size();
		}
		else if (attr[mask[i]].flag == 0) {
			j = 10 - attr[mask[i]].name.size();
		}
		else {
			j = 20 - attr[mask[i]].name.size();
		}
		while (j-- >= 0) cout << " ";
	}
	std::cout << "©¦";
	std::cout << std::endl;
	std::cout << "©À";
	for (int k = 1; k <= chart_len; k++) std::cout << "©¤";
	std::cout << "©È" << endl;
	for (i = 0; i < T.size(); i++) {
		T[i]->Print(mask, chart_len);
	}
	std::cout << "©¸";
	for (int k = 1; k <= chart_len; k++) std::cout << "©¤";
	std::cout << "©¼" << endl;
};


void Table::setindex(Table* t,short i, std::string iname) {
	short j;
	for (j = 0; j < index.size(); j++)
		if (iname == index[j].indexname)
			break;
	if (j < index.size())
		throw TableException("Index name has been used!");

	Index New_Ind;
	New_Ind.location = i;
	New_Ind.indexname = iname;
	index.push_back(New_Ind);
}

void Table::dropindex(std::string iname) {
	short j;
	for (j = 0; j < CheckIndex.iname.size(); j++) {
		if (iname != CheckIndex.iname[j]) {
			continue;
			cout << CheckIndex.iname[j] << endl;
		}
		else {
			CheckIndex.iname[j] = " ";
			throw TableException("Drop index successfully!");
		}
	}
	for (j = 0; j < index.size(); j++)
		if (iname == index[j].indexname)
			break;
	if (j == index.size())
		throw TableException("No index named " + iname + " on " + Tname + "!");

	std::string temps;

	index.erase(index.begin() + j);

}
int Table::hasColumn(std::string name)
{
	for (int i = 0; i < attr.size(); i++)
	{
		if (name == attr[i].name) return i;

	}
	return -1;
}