#include <iostream>
#include <fstream>
#include <string>
#include "API.h"
#include "Catalog.h"
static int Name_Length(char* begin, int pos)//calculate the length of a string
{
    int former = pos;
    int len = 0;
    while (begin[pos] != '\0')
    {
        len++;
        pos++;
    }

    return len + 1;
}

void CatalogManager::Mem_Move_Attribute(vector<Attribute>& attr, char* Block, int& pos, int RW)
{
    char temp[20];
    int Step;
    for (int i = 0; i < attr.size(); i++) 
    {
        if (RW == 0)
        {
            Step = Name_Length(Block, pos);
            memcpy(temp, &Block[pos], Step * sizeof(char));
            attr[i].name = temp;
            pos = pos + Step - 1 + sizeof(char);
        }
        else
        {
            memcpy(&Block[pos], attr[i].name.data(), attr[i].name.length() * sizeof(char));
            pos += (int)attr[i].name.length() * sizeof(char);
            memcpy(&Block[pos], "\0", sizeof(char));
            pos += sizeof(char);
        }

    }

    for (int i = 0; i < attr.size(); i++) 
    {
        if (RW == 0)
        {
            memcpy(&attr[i].flag, &Block[pos], sizeof(short));
            pos += sizeof(short);
        }
        else
        {
            memcpy(&Block[pos], &attr[i].flag, sizeof(short));
            pos += sizeof(short);
        }

    }

    for (int i = 0; i < attr.size(); i++)
    {
        if (RW == 0)
        {
            memcpy(&attr[i].unique, &Block[pos], sizeof(bool));
            pos += sizeof(bool);
        }
        else
        {
            memcpy(&Block[pos], &attr[i].unique, sizeof(bool));
            pos += sizeof(bool);
        }

    }
    return;
}

void CatalogManager::Mem_Move_Index(vector<Index>& ind, char* Block, int& pos, int RW)
{
    char temp[20];
    int Step;
    for (int i = 0; i < ind.size(); i++)
    {
        if (RW == 0)
        {
            Step = Name_Length(Block, pos);
            memcpy(temp, &Block[pos], Step * sizeof(char));
            ind[i].indexname = temp;
            pos = pos + Step + -1 + sizeof(char);
        }
        else
        {
            memcpy(&Block[pos], ind[i].indexname.data(), ind[i].indexname.length() * sizeof(char));
            pos += (int)ind[i].indexname.length() * sizeof(char);
            memcpy(&Block[pos], "\0", sizeof(char));
            pos += sizeof(char);
        }

    }

    for (int i = 0; i < ind.size(); i++) 
    {
        if (RW == 0)
        {
            memcpy(&ind[i].location, &Block[pos], sizeof(short));
            pos += sizeof(short);
        }
        else
        {
            memcpy(&Block[pos], &ind[i].location, sizeof(short));
            pos += sizeof(short);
        }
    }
    return;

}
void CatalogManager::Catalog_Create_Table(std::string s, vector<Attribute> attr, short primary, vector<Index> index){

    if (ExistTable(s))
        throw TableException("[ERROR]Table named" + s + " already exists.");
    if (primary >= 0)
        attr[primary].unique = true;
    std::string tname = s;
    s = "T_" + s;
    int i;

    string filename = s;
    fstream fout(filename.c_str(), ios::out);
    fout.close();

    int No = bf.getbufferNum(s, 0);

    char* Block = bf.bufferBlock[No].values;
    int pos = 0;
    int Attr_Num = attr.size();
    int Ind_Num = index.size();

    memcpy(&Block[pos], &Attr_Num, sizeof(int));
    pos += sizeof(int);
    memcpy(&Block[pos], &Ind_Num, sizeof(int));
    pos += sizeof(int);
    memset(&Block[pos], 0, sizeof(int));
    pos += sizeof(int);
    memcpy(&Block[pos], &primary, sizeof(short));
    pos += sizeof(short);
    Mem_Move_Attribute(attr, Block, pos, 1);
    Mem_Move_Index(index, Block, pos, 1);

    bf.writeBlock(No);

    Table* t = GetTable(s.substr(2, s.length() - 2));
    API api;
    api.CreateTable(*t);
    delete t;

}

bool CatalogManager::ExistTable(std::string s)
{
    s = "T_" + s;
    std::ifstream in(s);
    if (!in) {
        return false;
    }
    else {
        in.close();
        return true;
    }
}

Table* CatalogManager::GetTable(std::string s)
{
    std::string s1 = "T_" + s;
    if (!ExistTable(s))
        throw TableException("[ERROR]Table named \"" + s + "\" doesn't exist.");

    int pri;
    int Block_Num;
    int No = bf.getbufferNum(s1, 0);
    char* Block = bf.bufferBlock[No].values;
    int pos = 0;
    int Attr_Num;
    int Ind_Num;

    memcpy(&Attr_Num, &Block[pos], sizeof(int));
    pos += sizeof(int);
    memcpy(&Ind_Num, &Block[pos], sizeof(int));
    pos += sizeof(int);
    memcpy(&Block_Num, &Block[pos], sizeof(int));
    pos += sizeof(int);
    memcpy(&pri, &Block[pos], sizeof(short));
    pos += sizeof(short);
    vector<Attribute> attr(Attr_Num);
    vector<Index> ind(Ind_Num);
    Mem_Move_Attribute(attr, Block, pos, 0);
    Mem_Move_Index(ind, Block, pos, 0);

    bf.useBlock(No);
    Table* tb = new Table(s, attr, ind, Block_Num, pri);
    return tb;
}


void CatalogManager::Catalog_Create_Index(std::string tname, std::string aname, std::string iname)
{
    Table* tb = GetTable(tname);
    try {
        int i;
        for (i = 0; i < tb->attr.size(); i++)
            if (tb->attr[i].name == aname)
                break;
        if (i == tb->attr.size())
            throw TableException("[ERROR]Attribute named \"" + aname +" \" doesn't exist.");
        if (tb->attr[i].unique == 0)
            throw TableException("[ERROR]The attribute is not unique.");
        tb->setindex(tb, i, iname);

        int No = bf.getbufferNum("T_" + tb->Tname, 0);

        char* Block = bf.bufferBlock[No].values;

        int pos = 0;
        vector<Attribute> attr = tb->attr;
        vector<Index> ind = tb->index;
        short primary = tb->primary;
        int bn = tb->blockNum;
        int Attr_Num = attr.size();
        int Ind_Num = ind.size();

        memcpy(&Block[pos], &Attr_Num, sizeof(int));
        pos += sizeof(int);
        memcpy(&Block[pos], &Ind_Num, sizeof(int));
        pos += sizeof(int);
        memcpy(&Block[pos], &bn, sizeof(int));
        pos += sizeof(int);
        memcpy(&Block[pos], &primary, sizeof(short));
        pos += sizeof(short);

        Mem_Move_Attribute(attr, Block, pos, 1);
        Mem_Move_Index(ind, Block, pos, 1);

        bf.writeBlock(No);


        delete tb;
    }
    catch (TableException e1) {
        delete tb;
        throw e1;
    }
}

void CatalogManager::Catalog_Create_Index2(std::string tname, std::string aname, std::string iname) {
    Table* t = GetTable(tname);
    int i;
    for (i = 0; i < t->attr.size(); i++) {
        if (t->attr[i].name == aname) {
            if(t->attr[i].unique == 0) throw TableException("[ERROR]The attribute named " + aname + " on table " + tname + " is not unique.");
            break;
        }
    }
    if (i == t->attr.size()) throw TableException("[ERROR]No attribute named " + aname + " on table " + tname + ".");
    for (int j = 0; j < CheckIndex.aname.size(); j++) {
        if (CheckIndex.iname[j] == iname) {
            throw TableException("[ERROR]Index name exists.");
        }
    }
    CheckIndex.aname.push_back(aname);
    CheckIndex.iname.push_back(iname);
    throw TableException("[Message]Create index execuated.");
}

void CatalogManager::Catalog_Drop_Table(std::string t)
{
    if (!ExistTable(t))
        throw TableException("[ERROR]Table named \"" + t + "\" doesn't exist.");
    for (int i = 0; i < BLOCKNUM; i++)bf.initialize(i);
    Table* tb = GetTable(t);
    API api;
    api.DropTable(*tb);
    remove(("T_" + t).c_str());
}

void CatalogManager::Catalog_Drop_Index(std::string tname, std::string iname)
{
    Table* tb = GetTable(tname);
    try {
        tb->dropindex(iname);
        Catalog_Drop_Table(tname);
        Catalog_Create_Table(tname, tb->attr, tb->primary, tb->index);
        delete tb;
    }
    catch (TableException e1) {
        delete tb;
        throw e1;
    }
}

void CatalogManager::Catalog_Drop_Index2(std::string iname)
{
    for (int i = 0; i < CheckIndex.iname.size(); i++) {
        if (CheckIndex.iname[i] == iname) {
            CheckIndex.iname[i] = "";
            remove((iname + ".index").c_str());
            cout << "[MESSAGE]Drop index successfully!" << endl;
            return;
        }
    }
    throw TableException("[ERROR]No such index!");
}

void CatalogManager::changeblock(std::string tname, int bn) {
    string s = "T_" + tname;
    int No = bf.getbufferNum(s, 0);
    char* begin = bf.bufferBlock[No].values;

    int pos = 0;
    pos += sizeof(int);
    pos += sizeof(int);
    memcpy(&begin[pos], &bn, sizeof(int));

    bf.writeBlock(No);

}

