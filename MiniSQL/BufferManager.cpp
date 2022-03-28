#include "BufferManager.h"
#include "Catalog.h"
#include <stdio.h>

BufferManager::BufferManager() {
	initialize(0);
}

BufferManager::~BufferManager() {
	for (int i = 0; i < BLOCKNUM; i++) Writeback(i);
}

void BufferManager::initialize(int i) {
	Writeback(i);
	bufferBlock[i].initialize();
	if (i < BLOCKNUM - 1) initialize(i + 1);
}

void BufferManager::Writeback(int num) {
	if (bufferBlock[num].isWritten) {
		const char *name = bufferBlock[num].filename.c_str();
		fstream file;
		file.open(name, ios::binary | ios::out | ios::in);
		file.seekp(BLOCKSIZE * bufferBlock[num].blockOffset, ios::beg);
		file.write(bufferBlock[num].values, BLOCKSIZE);
		file.close();
		bufferBlock[num].initialize();
		UpdateLRU();
	}
}

int BufferManager::getbufferNum(string name, int offset) {
	int num = IsDataInBuffer(name, offset);
	if (num == -1) {
		num = getEmptyBuffer(name, BLOCKNUM - 1);
		readBlock(name, offset, num);
	}
	bufferBlock[num].LRUvalue = 0;
	UpdateLRU();
	return num;
}

void BufferManager::readBlock(string name, int offset, int num) {
	bufferBlock[num].initialize();
	bufferBlock[num].isValid = 1;
	bufferBlock[num].filename = name;
	bufferBlock[num].blockOffset = offset;
	UpdateLRU();
	fstream file;
	file.open(name, ios::binary | ios::out | ios::in);
	file.seekp(BLOCKSIZE * bufferBlock[num].blockOffset, ios::beg);
	file.read(bufferBlock[num].values, BLOCKSIZE);
	file.close();
}

void BufferManager::writeBlock(int num) {
	bufferBlock[num].isWritten = 1;
	useBlock(num);
}

void BufferManager::useBlock(int num) {
	bufferBlock[num].LRUvalue = 0;
	UpdateLRU();
}

int BufferManager::getEmptyBuffer(string name, int i) {
	if (i) {
		if (!bufferBlock[i].isValid) {
			bufferBlock[i].initialize();
			bufferBlock[i].isValid = 1;
			UpdateLRU();
			return i;
		}
		if (bufferBlock[i].isLRU && bufferBlock[i].filename != name) {
			Writeback(i);
			bufferBlock[i].initialize();
			bufferBlock[i].isValid = 1;
			UpdateLRU();
			return i;
		}
		else return getEmptyBuffer("", i - 1);
	}
	else {
		Writeback(0);
		bufferBlock[0].initialize();
		bufferBlock[0].isValid = 1;
		UpdateLRU();
		return 0;
	}
}

insertPos BufferManager::getInsertPosition(Table& t) {
	insertPos iPos;
	int num;
	if (t.blockNum == 0) { 
		t.blockNum++;
		num = getEmptyBuffer("", BLOCKNUM - 1);
		bufferBlock[num].initialize();
		bufferBlock[num].isValid = 1;
		bufferBlock[num].isWritten = 1;
		bufferBlock[num].filename = t.Tname + "_table";
		UpdateLRU();
		CatalogManager ca;
		ca.changeblock(t.Tname, t.blockNum);
		iPos.Pos[0] = num;
		iPos.Pos[1] = 0;
		return iPos;
	}
	string filename = t.Tname + "_table";
	int length = t.dataSize + 1 + t.attr.size();
	for (int blockOffset = 0; blockOffset < t.blockNum; blockOffset++) {
		num = getbufferNum(filename, blockOffset);
		int recordNum = BLOCKSIZE / length;
		for (int offset = 0; offset < recordNum; offset++) {
			if (bufferBlock[num].values[offset * length] == EMPTY) {
				iPos.Pos[0] = num;
				iPos.Pos[1] = offset * length;
				return iPos;
			}
		}
	}
	t.blockNum++;
	num = getEmptyBuffer("", BLOCKNUM - 1);
	bufferBlock[num].initialize();
	bufferBlock[num].isValid = 1;
	bufferBlock[num].isWritten = 1;
	bufferBlock[num].filename = t.Tname + "_table";
	bufferBlock[num].blockOffset = t.blockNum - 1;
	UpdateLRU();
	CatalogManager ca;
	ca.changeblock(t.Tname, t.blockNum);
	iPos.Pos[0] = num;
	iPos.Pos[1] = 0;
	return iPos;
}

int BufferManager::IsDataInBuffer(string name, int offset) {
	for (int num = 0; num < BLOCKNUM; num++)
		if (bufferBlock[num].filename == name && (bufferBlock[num].blockOffset == offset || offset == -1))	return num;
	return -1; 
}

void BufferManager::UpdateLRU() {
	int i, flag = 1;
	int theLRU = 0;
	for (i = 0; i < BLOCKNUM; i++) {
		if (!bufferBlock[i].isValid) flag = 0;
		if (bufferBlock[i].isLocked && bufferBlock[i].LRUvalue > bufferBlock[theLRU].LRUvalue) theLRU = i;
		bufferBlock[i].isLRU = false;
	}
	if (flag == 0) return;
	else bufferBlock[theLRU].isLRU = true;
}