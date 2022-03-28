#ifndef Catalog_h
#define Catalog_h
#include "base.h"
#include "BufferManager.h"
extern BufferManager bf;

class CatalogManager {
public:
    CatalogManager() {};
    void Catalog_Create_Table(string s, vector<Attribute> attr, short primary, vector<Index> index);
    bool ExistTable(std::string s);
    Table* GetTable(std::string s);
    void Catalog_Create_Index(std::string tname, std::string aname, std::string iname);
    void Catalog_Create_Index2(std::string tname, std::string aname, std::string iname);
    void Catalog_Drop_Table(std::string t);
    void Catalog_Drop_Index(std::string tname, std::string iname);
    void Catalog_Drop_Index2(std::string iname);
    void changeblock(std::string tname, int bn);
    void Mem_Move_Attribute(vector<Attribute>& attr, char* Block, int& pos, int RW);
    void Mem_Move_Index(vector<Index>& ind, char* Block, int& pos, int RW);
};



#endif /* Catalog_h */