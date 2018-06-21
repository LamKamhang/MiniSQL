#pragma once
#ifndef INTERPRETER_H_ 

#define INTERPRETER_H_
#include"CatalogManage.h"
#include<string.h>
#include<string>
using namespace std;

enum Mini
{
	ERROR,
	SELECT,
	DELETE,
	INSERT,
	EXEFILE,
	CREATETABLE,
	CREATEINDEX,
	DROPTABLE,
	DROPINDEX,
	CREATEDATABASE,
	DROPDATABASE,
	USEDATABASE,
	QUIT
};

class Interpreter
{
public:
	void init();
};

class condition
{
public:
	string attributeName;
	int type;
	string oprt;
	int intValue;
	float floatValue;
	string stringValues;
};

class miniSelect
{
public:
	string tableName;
	attribute attr[32];
	condition cond[32];
	int conditionNum;
	int attributeNum;
};

class miniDelete
{
public:
	string tableName;
	condition cond[32];
	int conditionNum;
};

class miniInsert
{
public:
	string tableName;
	condition cond[32];
	int insertNum;
};

class miniFile
{
public:
	string fileName;
};

class miniCreateTable
{
public:
	miniCreateTable() {}
	~miniCreateTable() {}
	string tableName;
	int attributeNum;
	attribute attributes[32];
};

class miniCreateIndex
{
public:
	string indexName;
	string tableName;
	string attributeName;
};

class miniDropTable
{
public:
	string tableName;
};

class miniDropIndex
{
public:
	string indexName;
};

class miniCreateDatabase
{
public:
	string databaseName;
};

class miniDropDatabase
{
public:
	string databaseName;
};

class miniUseDatabase
{
public:
	string databaseName;
};

class miniStatement
{
public:
	miniStatement() {}
	~miniStatement() {}
	int flag;
	miniSelect MiniSelect;
	miniDelete MiniDelete;
	miniInsert MiniInsert;
	miniFile MiniFile;
	miniCreateTable MiniCreateTable;
	miniCreateIndex MiniCreateIndex;
	miniDropTable MiniDropTable;
	miniDropIndex MiniDropIndex;
	miniCreateDatabase MiniCreateDatabase;
	miniDropDatabase MiniDropDatabase;
	miniUseDatabase MiniUseDatabase;

};

miniStatement miniInterpreter(char* in);

int  splitString(const string & strSrc, const std::string& strDelims, vector<string>& strDest);

#endif
