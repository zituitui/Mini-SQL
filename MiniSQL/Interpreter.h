#ifndef Interpreter_h
#define Interpreter_h
#include "base.h"
std::vector<std::string> split(const std::string& input, const std::string& regex)
{
	std::regex re(regex);
	std::sregex_token_iterator first{ input.begin(), input.end(), re, -1 }, last;
	return{ first, last };
}
class InterpretManager {
private:
	std::string s;//input string
	vector<string>query;
public:
	int Interpret_Fetch();
	void Interpret_Select();
	void Interpret_Drop();
	void Interpret_Create_Index();
	void Interpret_Create_Table();
	void Interpret_Insert();
	void Interpret_Delete();
	void Interpret_Quit();
	void Interpret_ExecFile();
	void Where_Clause(int pos, vector<where>& w, Table* tb);
	void InputQuery();//input the query
};


class QueryException :std::exception 
{
public:;
	   QueryException(std::string s) :text(s) {}
	   std::string what() {
		   return text;
	   };
private:
	std::string text;
};

#endif 
