#include <iostream>
#include <string>
#include <math.h>
#include <fstream>
#include "Catalog.h"
#include "Interpreter.h"
#include "API.h"

void InterpretManager::InputQuery()//catch the input query
{
	query.clear();
	string s = "";
	string temp;
	cin >> temp;
	while (temp != ";")
	{
		if (temp[temp.length() - 1] == ';')
		{
			s = s + " " + temp.substr(0, temp.length() - 1);
			break;
		}
		s = s + " " + temp;
		cin >> temp;
	}
	s = s + " ;";
	int i = 0;
	while (i < s.length())//add space to certain characters
	{
		if (s[i] == '(' || s[i] == ',' || s[i] == ')' || s[i] == '=')
		{
			if (s[i + 1] != ' ') s.insert(i + 1, " ");
			if (s[i - 1] != ' ') s.insert(i, " ");
			i++;
		}
		else if (s[i] == '>' || s[i] == '<')
		{
			if (s[i + 1] == '=')
			{
				if (s[i + 2] != ' ') s.insert(i + 2, " ");
				if (s[i - 1] != ' ') s.insert(i, " ");
				i += 3;
			}
			else
			{
				if (s[i + 1] != ' ') s.insert(i + 1, " ");
				if (s[i - 1] != ' ') s.insert(i, " ");
				i++;
			}
		}
		else if (s[i] == '!' && s[i + 1] == '=')
		{
			if (s[i + 2] != ' ') s.insert(i + 2, " ");
			if (s[i - 1] != ' ') s.insert(i, " ");
			i += 3;
		}
		else
		{
			i++;
		}
	}
	//   cout << s << "|" << endl;
	//spilt the query strings into different substrings using spaces
	string comma = " ";
	auto res = split(s, comma);
	for (auto str : res) {
		query.push_back(str);
	}
	vector<string>::iterator p;
	for (p = query.begin(); p != query.end(); ) {
		if (*p == "") p = query.erase(p);
		else {
			// cout << "std:" << *p << endl;
			p++;
		}
	}
}
static bool Str_to_Int(string s, int& result)//check if a string is a integer and convert
{
	stringstream check(s);
	if (!(check >> result)) return false;
	check >> result;
	return true;
}
static bool Str_to_Float(string s, float& result)//check if a string is a float and convert
{
	stringstream check(s);
	if (!(check >> result)) return false;
	check >> result;
	return true;
}
int InterpretManager::Interpret_Fetch()//fetch the query
{
	if (query[0] == "select")
	{
		Interpret_Select();
		cout << "[Message]Select execuated." << endl;
		return 1;
	}
	else if (query[0] == "drop")
	{
		Interpret_Drop();
		cout << "[Message]Drop execuated." << endl;
		return 1;
	}
	else if (query[0] == "insert")
	{
		Interpret_Insert();
		cout << "[Message]Insert execuated." << endl;
		return 1;
	}
	else if (query[0] == "create")
	{
		if (query[1] == "table") Interpret_Create_Table();
		else if (query[1] == "index") Interpret_Create_Index();
		else throw QueryException("[ERROR]Invalid query.");
		query.clear();
		cout << "[Message]Create execuated." << endl;
		return 1;
	}
	else if (query[0] == "delete")
	{
		Interpret_Delete();
		cout << "[Message]Delete execuated." << endl;
		query.clear();
		return 1;
	}
	else if (query[0] == "quit")
	{
		cout << "Quit the system..." << endl;
		Interpret_Quit();
		query.clear();
		return 0;
	}
	else if (query[0] == "execfile")
	{
		cout << "[Message]Reading the file..." << endl;
		Interpret_ExecFile();
		return 1;
	}
	else
	{
		query.clear();
		throw QueryException("[ERROR]Invalid query.");
		return 1;
	}
	query.clear();
	return 1;
}
void InterpretManager::Interpret_Select()
{
	int i = 1;
	vector<int> Column_Select;//columns to select
	CatalogManager cata;
	vector<int> Column_Where;//modifying columns with where clause 
	vector<where> w;//where clause values
	API api;
	while (query[i] != "from")//first get the table name
	{
		i++;
		if (query[i] == ";") throw QueryException("[ERROR]Invalid query.");
	}
	i++;
	Table* tb = cata.GetTable(query[i]);
	vector<Attribute> Table_Attr = tb->attr;
	i = 1;
	while (query[i] != "from")//get the attribute to select
	{
		if (i == 1 && query[i] == "*")
		{
			for (int j = 0; j < Table_Attr.size(); j++) Column_Select.push_back(j);
			i++;
			continue;
		}
		int col_num = tb->hasColumn(query[i]);
		if (col_num != -1) Column_Select.push_back(col_num);
		else throw QueryException("[ERROR]The attribute named" + query[i] + "doesn't exist.");
		i++;
		if (query[i] == ",") i++;
		else if (query[i] == "from") break;
		else throw  QueryException("[ERROR]Invalid query.");
	}
	i += 2;//query[i]=from query[i+1]=table name
	if (query[i] == ";")//no where clause
	{
		Table output = api.Select(*tb, w);
		output.Print(Column_Select);
		return;
	}
	if (query[i] != "where") throw  QueryException("[ERROR]Invalid query.");
	Where_Clause(i + 1, w, tb);
	Table output = api.Select(*tb, w);
	output.Print(Column_Select);
	return;

}
void InterpretManager::Interpret_Drop()
{
	CatalogManager cata;
	if (query[1] == "table")
	{
		Table* tb = cata.GetTable(query[2]);

		if (tb->index.size() > 0)
		{
			for (int j = 0; j < tb->index.size(); j++)
			{
				cata.Catalog_Drop_Index(query[2], tb->index[j].indexname);
			}
		}
		delete tb;
		cata.Catalog_Drop_Table(query[2]);
	}
	else if (query[1] == "index")
	{
		if(query.size() > 4) throw QueryException("[ERROR]Invalid query.");
		cata.Catalog_Drop_Index2(query[2]);
		for (int i = 0; i < tablename.name.size(); i++) {
			tablename.name.pop_back();
		}
		ifstream f_in("table_name");
		string s;
		while (getline(f_in, s)) {
			tablename.name.push_back(s);
		}
		f_in.close();
		for (int i = 0; i < tablename.name.size(); i++) {
			Table *t = cata.GetTable(tablename.name[i]);
			for (int j = 0; j < CheckIndex.iname.size(); j++) {
				if (CheckIndex.iname[j] == query[2]) {
					cata.Catalog_Drop_Index2(query[2]);
					break;
				}
			}
		}
		
	}
	else throw QueryException("[ERROR]Invalid query.");
	return;
}
void InterpretManager::Interpret_Insert()
{
	if (query[1] != "into" || query[3] != "values" || query[4] != "(") throw QueryException("[ERROR]Invalid query.");
	CatalogManager cata;
	Table* tb = cata.GetTable(query[2]);
	TableTuple* tp = new TableTuple();
	vector<Attribute> Table_Attr = tb->attr;
	int is_int;//this attribut is  an int
	float is_float;//this attribute is a float
	string is_char;//this attribute is a char(x)
	int i = 5;//travel the query
	int j = 0;//attribute column number
	Data* temp_i;
	try
	{
		int num = 0;
		while (query[i] != ";")
		{
			if(num == Table_Attr.size()) throw QueryException("[ERROR]Too more attributes!");
			if (Table_Attr[j].flag == -1 && Str_to_Int(query[i], is_int))
			{
				temp_i = new Data_i(is_int);
				num++;
			}
			else if (Table_Attr[j].flag == 0 && Str_to_Float(query[i], is_float))
			{
				temp_i = new Data_f(is_float);
				num++;
			}
			else if (Table_Attr[j].flag > 0 && Table_Attr[j].flag < 255 && query[i].length() - 2 <= Table_Attr[j].flag && query[i][0] == '\'')
			{
				string s = query[i];
				int j = 0;
				while (s[s.length() - 1] != '\'') {
					j++;
					if(query[i + j] == ";") throw QueryException("[ERROR]Incorrect data type of value " + s + ".");
					s += " " + query[i + j];
				}
				i += j;
				string Naked_S = s.substr(1, s.length() - 2);//query[i+2]="'xxxxxx'" Naked_S="xxxxx"
				temp_i = new Data_c(Naked_S);
				num++;
			}
			else throw QueryException("[ERROR]Incorrect data type of value " + query[i] + ".");
			tp->data.push_back(temp_i);
			j++;
			i++;
			if (query[i] == ",") i++;
			else
			{
				if (query[i] == ")" && query[i + 1] == ";") i++;
				else throw QueryException("[ERROR]Invalid query.");

			}
		}

	}
	catch (QueryException qe)
	{
		delete tb;
		delete tp;
		throw qe;
	}
	API api;
	api.Insert(*tb, *tp);
}
void InterpretManager::Interpret_Delete()
{
	if (query[1] != "from") throw QueryException("[ERROR]Invalid query.");
	CatalogManager cata;
	Table* tb = cata.GetTable(query[2]);
	vector<where> w;
	API api;
	if (query[3] == ";")
	{
		;
	}
	else
	{
		if (query[3] != "where") throw QueryException("[ERROR]Invalid query.");
		Where_Clause(4, w, tb);

	}
	api.Delete(*tb, w);
	delete tb;
}
void InterpretManager::Interpret_Quit()
{
	;
}
void InterpretManager::Interpret_Create_Table()
{
	//cout << "1" << endl;
	string Table_Name = query[2];
	if (query[3] != "(") throw QueryException("[ERROR]Invalid query.");
	int i = 4;
	vector<Attribute> attr(0);
	int primary_flag = -1;
	while (query[i] != ")" && i < query.size())
	{
		Attribute New_Attr;
		New_Attr.name = query[i];
		New_Attr.unique = 0;

		int char_len;
		if (query[i] == "primary" && query[i + 1] == "key" && query[i + 2] == "(" && query[i + 4] == ")")
		{
			if (primary_flag != -1) throw QueryException("[ERROR]Multiply primary key declarations.");
			for (int j = 0; j < attr.size(); j++)
			{
				if (query[i + 3] == attr[j].name)
				{
					primary_flag = j;
					attr[j].unique = 1;
					i += 5;
					break;
				}
				if (primary_flag == -1) throw QueryException("[ERROR]The attribute named" + query[i + 3] +"doesn't exist.");
			}
		}
		else
		{
			i++;
			if (query[i] == "int") { New_Attr.flag = -1; i++; }
			else if (query[i] == "float") { New_Attr.flag = 0; i++; }
			else if (query[i] == "char" && query[i + 1] == "(" && Str_to_Int(query[i + 2], char_len) && query[i + 3] == ")") { New_Attr.flag = char_len; i += 4; }
			else throw QueryException("ERROR: invalid query format!");
			if (query[i] == "unique") { New_Attr.unique = 1; i++; }
			if (query[i] == "primary" && query[i + 1] == "key")
			{
				if (primary_flag != -1) throw QueryException("[ERROR]Multiply primary key declarations.");
				primary_flag = attr.size();
				New_Attr.unique = 1;
				i += 2;
			}
			attr.push_back(New_Attr);
		}

		if (query[i] != ",")
		{
			if (query[i] == ")" && query[i + 1] == ";") continue;
			else throw QueryException("[ERROR]Invalid query.");
		}
		else i++;

	}
	//cout << "  " << Table_Name << endl; for (int i = 0; i < attr.num; i++) cout << attr.name[i] << " " << attr.flag[i] << " " << attr.unique[i] << endl;
	CatalogManager cata;
	vector<Index> ind(0);
	cata.Catalog_Create_Table(Table_Name, attr, primary_flag, ind);
	if(primary_flag != -1) cata.Catalog_Create_Index(Table_Name, attr[primary_flag].name, attr[primary_flag].name);
	return;
}

void InterpretManager::Interpret_Create_Index()
{
	//cout << "2" << endl;
	if (query[3] != "on" || query[5] != "(" || query[7] != ")") throw QueryException("[ERROR]Invalid query.");
	CatalogManager cata;
	cout << "Index Name " << query[2] << endl << "Table Name " << query[4] << endl <<  "Column Name " << query[6] << endl;
	cata.Catalog_Create_Index2(query[4], query[6], query[2]);
	return;
}
void InterpretManager::Where_Clause(int pos, vector<where>& w, Table* tb)
{
	vector<Attribute> Table_Attr = tb->attr;
	int Column_Size = tb->attr.size();
	int i = pos;//position
	int k = 0;
	where temp_w;//add to the where vector
	while (query[i] != ";")
	{
		int j = 0;//travel the columns
		int is_int;//the modifyng attribute is int
		float is_float;//the modifying attribute is float
		string is_char;//the modifying attribute is char(x)
		int attr_type;
		j = tb->hasColumn(query[i]);
		if (j == -1) throw QueryException("[ERROR]The attribute named" + query[i] + "doesn't exist.");
		temp_w.column = j;
		//check the data type after the logic operator
		if (Table_Attr[j].flag == -1 && Str_to_Int(query[i + 2], is_int))
		{
			temp_w.d = new Data_i(is_int);

		}
		else if (Table_Attr[j].flag == 0 && Str_to_Float(query[i + 2], is_float))
		{
			temp_w.d = new Data_f(is_float);
		}
		else if (Table_Attr[j].flag > 0 && Table_Attr[j].flag < 255 && query[i + 2][0] == '\'')
		{
			k = 0;
			string s = query[i + 2];
			while (s[s.length() - 1] != '\'') {
				k++;
				if (query[i + 2 + k] == ";") throw QueryException("[ERROR]Incorrect data type in where clause.");
				s += " " + query[i + 2 + k];
			}
			if (s.length() - 2 > Table_Attr[j].flag) {
				throw QueryException("[ERROR]Incorrect data type in where clause.");
			}
			string Naked_S = s.substr(1, s.length() - 2);//query[i+2]="'xxxxxx'" Naked_S="xxxxx"
			temp_w.d = new Data_c(Naked_S);
		}
		else throw QueryException("[ERROR]Incorrect data type in where clause.");

		//check logic operator
		if (query[i + 1] == "=") temp_w.flag = 3;
		else if (query[i + 1] == ">") temp_w.flag = 6;
		else if (query[i + 1] == ">=") temp_w.flag = 5;
		else if (query[i + 1] == "!=") temp_w.flag = 4;
		else if (query[i + 1] == "<") temp_w.flag = 1;
		else if (query[i + 1] == "<=") temp_w.flag = 2;
		else throw QueryException("[ERROR]Incorrect data type in where clause.");
		w.push_back(temp_w);
		i += 3 + k;
		//cout << temp_w.flag << " " << temp_w.d << endl;
		if (query[i] == "and") i++;

	}
}
void InterpretManager::Interpret_ExecFile()
{
	string FileName = query[1];
	ifstream in(FileName);
	if (!in) throw QueryException("[ERROR]File named \"" + query[1] + "\" doesn't exist.");
	while (in.peek() != EOF)
	{
		try
		{
			query.clear();
			string s = "";
			string temp;
			in >> temp;
			if (temp == "") 
			{
				query.clear();
				in.close();
				return;
			}
			while (temp != ";")
			{
				if (temp[temp.length() - 1] == ';')
				{
					s = s + " " + temp.substr(0, temp.length() - 1);
					break;
				}
				s = s + " " + temp;
				in >> temp;
			}
			s = s + " ;";
			//cout << s << endl;
			int i = 0;
			while (i < s.length())//add space to certain characters
			{
				if (s[i] == '(' || s[i] == ',' || s[i] == ')' || s[i] == '=')
				{
					if (s[i + 1] != ' ') s.insert(i + 1, " ");
					if (s[i - 1] != ' ') s.insert(i, " ");
					i++;
				}
				else if (s[i] == '>' || s[i] == '<')
				{
					if (s[i + 1] == '=')
					{
						if (s[i + 2] != ' ') s.insert(i + 2, " ");
						if (s[i - 1] != ' ') s.insert(i, " ");
						i += 3;
					}
					else
					{
						if (s[i + 1] != ' ') s.insert(i + 1, " ");
						if (s[i - 1] != ' ') s.insert(i, " ");
						i++;
					}
				}
				else if (s[i] == '!' && s[i + 1] == '=')
				{
					if (s[i + 2] != ' ') s.insert(i + 2, " ");
					if (s[i - 1] != ' ') s.insert(i, " ");
					i += 3;
				}
				else
				{
					i++;
				}
			}

			string comma = " ";
			auto res = split(s, comma);
			for (auto str : res) {
				query.push_back(str);
			}
			vector<string>::iterator p;
			for (p = query.begin(); p != query.end(); ) {
				if (*p == "") p = query.erase(p);
				else {
					// cout << "std:" << *p << endl;
					p++;
				}
			}
			Interpret_Fetch();
		}
		catch(TableException te) {
			cout << te.what() << endl;
		}
		catch (QueryException qe) {
			cout << qe.what() << endl;
		}
	}
	query.clear();
	in.close();

}

