#pragma once
#include <string>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "../interface.h"
#include "../CatalogManager/CatalogManager.h"
using namespace MINISQL_BASE;

class miniSelect
{
public:
	std::string tableName;
	attribute attr[32];
	condition cond[32];
	int conditionNum;
	int attributeNum;
};

class miniDelete
{
public:
	std::string tableName;
	condition cond[32];
	int conditionNum;
};

class miniInsert
{
public:
	std::string tableName;
	condition cond[32];
	int insertNum;
};

class miniFile
{
public:
	std::string fileName;
};


class miniDropTable
{
public:
	std::string tableName;
};

class miniDropIndex
{
public:
	std::string indexName;
};

class miniCreateDatabase
{
public:
	std::string databaseName;
};

class miniDropDatabase
{
public:
	std::string databaseName;
};

class miniUseDatabase
{
public:
	std::string databaseName;
};

struct miniStatement
{
	miniStatement()=default;
	~miniStatement()=default;
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
miniStatement miniInterpreter(const std::string &in);

