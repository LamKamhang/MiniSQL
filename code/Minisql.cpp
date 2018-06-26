#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS
#define DEBUG

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector>
#include "API/API.h"

using namespace MINISQL_BASE;
using namespace std; 

#define Threshold 5

bool checkStatement(vector<string> &statements, string &line);

int main(void)
{
	API a;
	miniStatement m;
	char data[200];
	vector<string> statements;
	string line;
	string fileContent;
	string res;
	ifstream fin;
	stringstream ss;
	bool finish = false;

	cout << "minisql=# ";
	while (1)
	{
		getline(cin, line);
		res += line;
		finish = checkStatement(statements, res);
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
				fin.open(m.MiniFile.fileName);
				if (!fin)
				{
					cerr << "The file you are looking for does not exist!" << endl;
				}
				else
				{
					ss << fin.rdbuf();
					fin.close();
					fileContent = ss.str();
					checkStatement(statements, fileContent);
					continue;
				}
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
			default:
				cerr << "ERROR::TYPE ERROR!" << endl;
			}
		}
		if (counter > Threshold)
		{
			a.writeBack();
			counter = 0;
		}
#ifdef DEBUG
		cout << "counter: " << counter << endl;
#endif // DEBUG


		if (finish)
			cout << "minisql=# ";
		else
			cout << "minisql-# ";
		//vector <string>().swap(statements);
		statements.clear();
	}
	return 0;
} 


bool checkStatement(vector<string> &statements, string &line)
{
	bool res = false;
	std::string::size_type pos;

	while ((pos = line.find(";")) != std::string::npos)
	{
		res = true;
		statements.push_back(line.substr(0, pos + 1));
		line = line.substr(pos + 1);
	}
	return res;
}
