#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstdio>
#include "API/API.h"

using namespace MINISQL_BASE;
using namespace std; 

int main(void)
{
	API a;
	miniStatement m;
	records r;
	char data[200];
	while(1)
	{
		cin.getline(data,200);
		m=miniInterpreter(data);
		switch(m.flag)
		{
			case SYNTAX_ERROR:
				break;
			case SELECT:
				r=a.Select(m.MiniSelect);
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
	return 0;
} 
