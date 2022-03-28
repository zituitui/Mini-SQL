#include "RecordManager.h"
#include "Interpreter.h"
#include "IndexManager.h"

int RecordManager::FindAttr(Table& t, int i) {
	if (t.attr[i].flag == -1) {
		return sizeof(int);
	}
	else if (t.attr[i].flag == 0) {
		return sizeof(float);
	}
	else return t.attr[i].flag + 1;
}

bool RecordManager::CreateTable(Table& t) {
	string filename = t.Tname + "_table";
	CatalogManager cata;
	if (!t.blockNum)t.blockNum++;
	else t.blockNum = 1;
	cata.changeblock(t.Tname, 1);
	ofstream file;
	file.open(filename);
	file.close();
	return true;
}

void RecordManager::DeleteTableInMemory(string s) {
	int i = bp->IsDataInBuffer(s,-1);
	if (i != -1) {
		bp->bufferBlock[i].initialize();
		DeleteTableInMemory(s);
	}
}

bool RecordManager::DropTable(Table& t) {
	string s = t.Tname;
	DeleteTableInMemory(s);
	s += "_table";
	remove(s.c_str());
	bp->UpdateLRU();
	return true;
}

string RecordManager::int2string(int x) {
	string s;
	char c;
	int flag, y;
	s = "";
	flag = 0;
	if (x < 0) {//if the fraction is negative, add a '-'
		flag = 1;
		x = -x;
	}
	while (x) {
		y = x % 10;
		c = y + '0';
		s = c + s;
		x = x / 10;
	}
	if (flag)s = "-" + s;
	return s;
}

int RecordManager::Delete(Table& t, vector<where> w) {
	string s, data, da;
	int i, j, k, l;
	int length = t.attr.size() + t.dataSize + 1;
	int rnum = BLOCKSIZE / length;
	int pos;
	int offset;
	int value_i;
	float value_f;
	char value_c[100];
	TableTuple *tu = NULL;
	Data *d;
	int ret;
	int flag = 0;
	s = t.Tname + "_table";
	IndexManager ind;
	for (i = 0; i < w.size(); i++) {
		int ans = ind.Find(t.Tname, w[i].d);
		if (ans == -1) continue;
		else {
			isSatisfied(*tu, w);
			bp->readBlock(s, i, t.blockNum - 1);
		}
	}
	for (i = 0; i < t.blockNum; i++) {
		j = bp->IsDataInBuffer(s, i);
		if (j == -1) {
			j = bp->getEmptyBuffer("", BLOCKNUM - 1);
			bp->readBlock(s, i, j);
			bp->UpdateLRU();
		}
		for (k = 0; k < rnum; k++) {
			tu = new TableTuple;
			data = bp->bufferBlock[j].getdata(k * length, length);
			pos = 0;
			if (data[0] == EMPTY) continue;
			pos++;
			for (l = 0; l < t.attr.size(); l++) {
				offset = FindAttr(t, l);
				switch (t.attr[l].flag) {
				case -1:
					memcpy(&value_i, &(data.c_str()[pos]), offset);
					pos += offset;
					if ((data.c_str()[pos] == EMPTY)) pos++;
					else pos++;
					d = new Data_i(value_i);
					tu->data.push_back(d);
					break;
				case 0:
					memcpy(&value_f, &(data.c_str()[pos]), offset);
					pos += offset;
					if ((data.c_str()[pos] == EMPTY)) pos++;
					else pos++;
					d = new Data_f(value_f);
					tu->data.push_back(d);
					break;
				default:
					memcpy(&value_c, &(data.c_str()[pos]), offset);
					pos += offset;
					if ((data.c_str()[pos] == EMPTY)) pos++;
					else pos++;
					da = value_c;
					d = new Data_c(da);
					tu->data.push_back(d);
					break;
				}
			}
			ret = isSatisfied(*tu, w);
			if (ret) {
				bp->writeBlock(j);
				bp->bufferBlock[j].values[k * length] = EMPTY;
				flag = 1;
			}
			delete tu;
		}
	}
	return flag;
}

bool RecordManager::isSatisfied_i(int x1, int x2, int logic) {
	switch (logic) {
	case 1:
		if (x1 >= x2) return false;
		break;
	case 2:
		if (x1 > x2) return false;
		break;
	case 3:
		if (x1 != x2) return false;
		break;
	case 4:
		if (x1 == x2) return false;
		break;
	case 5:
		if (x1 < x2) return false;
		break;
	case 6:
		if (x1 <= x2) return false;
		break;
	default:
		break;
	}
	return true;
}

bool RecordManager::isSatisfied_f(float x1, float x2, int logic) {
	switch (logic) {
	case 1:
		if (x1 >= x2) return false;
		break;
	case 2:
		if (x1 > x2) return false;
		break;
	case 3:
		if (x1 != x2) return false;
		break;
	case 4:
		if (x1 == x2) return false;
		break;
	case 5:
		if (x1 < x2) return false;
		break;
	case 6:
		if (x1 <= x2) return false;
		break;
	default:
		break;
	}
	return true;
}

bool RecordManager::isSatisfied_c(string x1, string x2, int logic) {
	switch (logic) {
	case 1:
		if (x1 >= x2) return false;
		break;
	case 2:
		if (x1 > x2) return false;
		break;
	case 3:
		if (x1 != x2) return false;
		break;
	case 4:
		if (x1 == x2) return false;
		break;
	case 5:
		if (x1 < x2) return false;
		break;
	case 6:
		if (x1 <= x2) return false;
		break;
	default:
		break;
	}
	return true;
}

bool RecordManager::isSatisfied(TableTuple& row, vector<where> w) {
	int i;
	bool flag = true;
	if (&row == NULL) return true;
	for (i = 0; i < w.size(); i++) {
		if (w[i].d == NULL || (w[i].flag != 1 && w[i].flag != 2 && w[i].flag != 3 && w[i].flag != 4 && w[i].flag != 5 && w[i].flag != 6)) continue;
		switch (row[w[i].column]->flag) {
		case -1:
			if (!isSatisfied_i(((Data_i *)row[w[i].column])->x, ((Data_i *)w[i].d)->x, w[i].flag)) return false;
			break;
		case 0:
			if (!isSatisfied_f(((Data_f*)row[w[i].column])->x, ((Data_f*)w[i].d)->x, w[i].flag)) return false;
			break;
		default:
			if (!isSatisfied_c(((Data_c *)row[w[i].column])->x, ((Data_c *)w[i].d)->x, w[i].flag)) return false;
			break;
		}
	}
	return true;
}

Table RecordManager::Select(Table& t, vector<where>& w) {
	string s, data, da;
	int i, j, k, l;
	int length = t.attr.size() + t.dataSize + 1;
	int rnum = BLOCKSIZE / length;
	int pos;
	int offset;
	int value_i;
	float value_f;
	char value_c[100];
	TableTuple *tu = NULL;
	Data *d;
	int ret;
	int flag = 0;
	s = t.Tname + "_table";
	IndexManager ind;
	for (i = 0; i < w.size(); i++) {
		int ans = ind.Find(t.Tname, w[i].d);
		if (ans == -1) continue;
		else {
			isSatisfied(*tu, w);
			bp->readBlock(s, i, t.blockNum - 1);
		}
	}
	for (i = 0; i < t.blockNum; i++) {
		j = bp->IsDataInBuffer(s, i);
		if (j == -1) {
			j = bp->getEmptyBuffer("", BLOCKNUM - 1);
			bp->readBlock(s, i, j);
			bp->UpdateLRU();
		}
		for (k = 0; k < rnum; k++) {
			tu = new TableTuple;
			data = bp->bufferBlock[j].getdata(k * length, length);
			pos = 0;
			if (data[0] == EMPTY) continue;
			pos++;
			for (l = 0; l < t.attr.size(); l++) {
				offset = FindAttr(t, l);
				switch (t.attr[l].flag) {
				case -1:
					memcpy(&value_i, &(data.c_str()[pos]), offset);
					pos += offset;
					if ((data.c_str()[pos] == EMPTY)) pos++;
					else pos++;
					d = new Data_i(value_i);
					tu->data.push_back(d);
					break;
				case 0:
					memcpy(&value_f, &(data.c_str()[pos]), offset);
					pos += offset;
					if ((data.c_str()[pos] == EMPTY)) pos++;
					else pos++;
					d = new Data_f(value_f);
					tu->data.push_back(d);
					break;
				default:
					memcpy(&value_c, &(data.c_str()[pos]), offset);
					for (int k = 0; k <= offset; k++) {
						if (value_c[k] == EMPTY)value_c[k] = '\0';
					}
					pos += offset;
					if ((data.c_str()[pos] == EMPTY)) pos++;
					else pos++;
					da = value_c;
					d = new Data_c(da);
					tu->data.push_back(d);
					break;
				}
			}
			if (isSatisfied(*tu, w)) {
				t.addData(tu);
			}
			else delete tu;
		}
	}
	return t;
}

string RecordManager::FindUnique(Table& tableIn) {
	int i;
	string s = "";
	for (i = 0; i < tableIn.attr.size(); i++) {
		if (tableIn.attr[i].unique == 1) s += i + '0';
	}
	return s;
}

bool RecordManager::CheckRedundancy(Table& t, TableTuple& tup) {
	int i, j, mask;
	string s1, data, da;
	int k, l, m, n;
	int length = t.attr.size() + t.dataSize + 1;
	int rnum = BLOCKSIZE / length;
	int pos;
	int offset;
	int value_i;
	float value_f;
	const char* test_c;
	char value_c[256];
	int ret;
	int flag = 0, flagc;
	IndexManager ind;
	string s = FindUnique(t);
	s1 = t.Tname + "_table";
	n = 0;
	for (i = 0; i < tup.data.size(); i++) {
		int ans = ind.Find(t.Tname, tup.data[i]);
		if (ans == -1) continue;
		else {
			bp->readBlock(s, i, t.blockNum - 1);
		}
	}
	for (n = 0; n < s.size(); n++) {
		for (i = 0; i < t.blockNum; i++) {
			j = bp->IsDataInBuffer(s1, i);
			if (j == -1) {
				j = bp->getEmptyBuffer("", BLOCKNUM - 1);
				bp->readBlock(s1, i, j);
				bp->UpdateLRU();
			}
			for (k = 0; k < rnum; k++) {
				data = bp->bufferBlock[j].getdata(k * length, length);
				pos = 0;
				if (data[pos] == EMPTY) continue;
				pos++;
				for (l = 0; l < t.attr.size(); l++) {
					if (n == s.size()) break;
					mask = s[n] - '0';
					offset = FindAttr(t, l);
					switch (t.attr[l].flag) {
					case -1:
						memcpy(&value_i, &(data.c_str()[pos]), offset);
						pos += offset;
						if ((data.c_str()[pos] == EMPTY)) pos++;
						else pos++;
						if (l == mask) {
							if (((Data_i*)tup[mask])->x == value_i) {
								throw QueryException("Unique Value already exists. Insertion failed!");
								return false;
							}
							continue;
						}
						break;
					case 0:
						memcpy(&value_f, &(data.c_str()[pos]), offset);
						pos += offset;
						if ((data.c_str()[pos] == EMPTY)) pos++;
						else pos++;
						if (l == mask) {
							if (((Data_f*)tup[mask])->x == value_f) {
								throw QueryException("Unique Value already exists. Insertion failed!");
								return false;
							}
							continue;
						}
						break;
					default:
						memcpy(&value_c, &(data.c_str()[pos]), offset);
						for (int k = 0; k <= offset; k++) {
							if (value_c[k] == EMPTY)value_c[k] = '\0';
						}
						value_c[offset] = '\0';
						pos += offset;
						if ((data.c_str()[pos] == EMPTY)) pos++;
						else pos++;
						if (l == mask) {
							flagc = 1;
							test_c = ((Data_c*)tup[mask])->x.c_str();
							for (m = 0;; m++) {
								if (test_c[m] == '\0' && value_c[m] == '\0' || value_c[m] == EMPTY && test_c[m] == '\0') break;
								if (test_c[m] != value_c[m] || test_c[m] == '\0' || value_c[m] == '\0') {
									flagc = 0;
									break;
								}
							}
							if (flagc) {
								throw QueryException("Unique Value already exists. Insertion failed!");
								return false;
							}
							continue;
						}
						break;
					}
				}
			}
		}
	}
	return true;
}

void RecordManager::InsertWithIndex(Table& t, TableTuple& tup) {
	if (!CheckRedundancy(t, tup)) return;
	t.addData(&tup);
	char *charTuper;
	charTuper = TupertoChar(t, tup);
	insertPos iPos = bp->getInsertPosition(t);
	bp->writeBlock(iPos.Pos[0]);
	bp->bufferBlock[iPos.Pos[0]].values[iPos.Pos[1]] = NOTEMPTY;
	memcpy(&(bp->bufferBlock[iPos.Pos[0]].values[iPos.Pos[1] + 1]), charTuper, t.dataSize + t.attr.size());
	int length = t.dataSize + 1 + t.attr.size();
	IndexManager ind;
	int blockCapacity = BLOCKSIZE / length;
	for (int i = 0; i < t.index.size(); i++) {
		int addr = bp->bufferBlock[iPos.Pos[0]].blockOffset*blockCapacity + iPos.Pos[1] / length; //the TableTuple's addr in the data file
		for (int j = 0; j < t.index.size(); j++) {
			ind.Insert(t.Tname + to_string(t.index[j].location) + "_index", tup[t.index[i].location], addr);
		}
	}
	delete[] charTuper;
}

void RecordManager::AddPtr(Table& t, TableTuple& tup, int i, int pos, char* ptrRes) {
	if (t.attr[i].flag == -1) { //int
		pos--;
		char ch = '#';
		memcpy(ptrRes + pos, &ch, 1);
		pos -= sizeof(int);
		int value = ((Data_i*)tup[i])->x;
		memcpy(ptrRes + pos, &value, sizeof(int));
	}
	else if (t.attr[i].flag == 0) {//float
		pos--;
		char ch = '#';
		memcpy(ptrRes + pos, &ch, 1);
		pos -= sizeof(float);
		float value = ((Data_f*)tup[i])->x;
		memcpy(ptrRes + pos, &value, sizeof(float));
	}
	else { //string
		pos--;
		char ch = '#';
		memcpy(ptrRes + pos, &ch, 1);
		pos = pos - t.attr[i].flag - 1;
		string value(((Data_c*)tup[i])->x);
		while (value.size() < t.attr[i].flag) value.push_back(EMPTY);
		memcpy(ptrRes + pos, value.c_str(), t.attr[i].flag + 1);
	}
	if (i > 0) AddPtr(t, tup, i - 1, pos, ptrRes);
}

char* RecordManager::TupertoChar(Table& t, TableTuple& tup) {
	char* ptrRes;
	ptrRes = new char[(t.dataSize + 1 + t.attr.size()) * sizeof(char)];
	ptrRes[t.dataSize + t.attr.size()] = '\0';
	AddPtr(t, tup, t.attr.size() - 1, t.dataSize + t.attr.size(), ptrRes);
	return ptrRes;
}
