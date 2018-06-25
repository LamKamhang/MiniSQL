#include "iostream"
#include "string.h"
#include "cassert"
#include "vector"
#include "interface.H"
#include "interface.cpp"
#include "CatalogManager.h"
#include "CatalogManager.cpp"
#include "Interpreter.h"
#include "Interpreter.cpp"
#include "BufferManager1.h"
#include "BufferManager1.cpp"
#include "Record_Manager.H"
#include "Record_Manager.cpp"
#include "API.cpp"
using namespace std; 

int main(void)
{
	API a;
	miniStatement m;
	records r;
	char data[200];
	while(1)
	{
		gets(data);
		m=miniInterpreter(data);
		switch(m.flag)
		{
		case 1:
			r=a.Select(m.MiniSelect);
			break;
		case 2:
			a.Delete(m.MiniDelete);
			break;
		case 3:
			a.Insert(m.MiniInsert);
			break;
		case 5:
			a.Create(m.MiniCreateTable);
			break;
		case 6:
			a.CreateIndex(m.MiniCreateIndex);
			break;
		case 7:
			a.Drop(m.MiniDropTable);
			break;
		case 8:
			a.DropIndex(m.MiniDropIndex);
		case 12:
			break;
		default:break;
		}
	}
	return 0;
} 
