#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "../interface.h"
using namespace MINISQL_BASE;

class CatalogManager
{
public:
	int getTableNum();
	int getIndexNum();
	bool readTableFile();
	bool writeTableFile();
	bool isTable(std::string tableName);
	bool isAttribute(std::string tableName, std::string attributeName);
	bool createTable(miniCreateTable I);
	bool dropTable(std::string tableName);
	bool readIndexFile();
	bool writeIndexFile();
	bool isIndex(std::string indexName);
	bool isIndex(std::string tableName, std::string attributeName);
	table getTable(std::string tableName);
	std::string getIndex(std::string tableName, std::string attributeName);
	bool createIndex(miniCreateIndex I);
	bool dropIndex(std::string indexName);
	bool dropDatabase();
	CatalogManager();
	~CatalogManager();

private:
	std::vector<table> tables;
	std::vector<index> indices;
	int tableNum;
	int indexNum;
};

int  splitString(const std::string & strSrc, const std::string& strDelims, std::vector<std::string>& strDest);
