#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include "API/API.h"

using namespace MINISQL_BASE;
using namespace std; 

bool checkStatement(vector<string> &statements, string &line);

int main(void)
{
	API a;
	miniStatement m;
	char data[200];
	vector<string> statements;
	string line;
	string res;

	while (1)
	{
		getline(cin, line);
		res += line;
		checkStatement(statements, res);
		for (size_t i = 0; i < statements.size(); ++i)
		{
			strcpy(data, statements[i].c_str());
			m = miniInterpreter(data);
			switch (m.flag)
			{
			case SYNTAX_ERROR:
				break;
			case SELECT:
				a.Select(m.MiniSelect);
				break;
			case DELETE:
				a.Delete(m.MiniDelete);
				break;
			case INSERT:
				a.Insert(m.MiniInsert);
				break;
			case EXEFILE:
				break;
			case CREATETABLE:
				a.Create(m.MiniCreateTable);
				break;
			case CREATEINDEX:
				a.CreateIndex(m.MiniCreateIndex);
				break;
			case DROPTABLE:
				a.Drop(m.MiniDropTable);
				break;
			case DROPINDEX:
				a.DropIndex(m.MiniDropIndex);
				break;
			case CREATEDATABASE:
				break;
			case DROPDATABASE:
				break;
			case USEDATABASE:
				break;
			case QUIT:
				return 0;
				break;
			}
		}
		//vector <string>().swap(statements);
		statements.clear();
	}

	


	while(1)
	{
		cin.getline(data,200);
		
	}
	return 0;
} 


bool checkStatement(vector<string> &statements, string &line)
{
	std::string::size_type pos;

	while ((pos = line.find(";")) != std::string::npos)
	{
		statements.push_back(line.substr(0, pos + 1));
		line = line.substr(pos + 1);
	}
	return true;
}
