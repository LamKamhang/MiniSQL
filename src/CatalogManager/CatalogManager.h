#pragma once
#include<string>
#include<vector>
#include<cassert>
#include<fstream>
#include<iostream>
using namespace std;

enum TYPE
{
	INT,
	CHAR,
	FLOAT
};

class index
{
public:
	string indexName;
	string tableName;
	string attributeName;
	index(string s1, string s2, string s3)
	{
		indexName = s1;
		tableName = s2;
		attributeName = s3;
	}
	index() {}

};

class attribute
{
public:
	string name;
	int type;
	int length;
	bool primary;
	bool unique;
	bool index;
	attribute() 
	{
		primary = false;
		unique = false;
		index = false;
	}
	~attribute() {}
	attribute(string n, int t, int l)
	{
		name = n;
		type = t;
		length = l;
		primary = false;
		unique = false;
		index = false;
	}
};

class table
{
public:
	string tableName;
	int attributeNum;
	int valueNum;
	vector<attribute> attributes;
	table() {}
	table(string tN, int aN)
	{
		tableName = tN;
		attributeNum = aN;
		valueNum = 0;
	}

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

class CatalogManager
{
public:
	int getTableNum();
	int getIndexNum();
	bool readTableFile();
	bool writeTableFile();
	bool isTable(string tableName);
	bool isAttribute(string tableName, string attributeName);
	bool createTable(miniCreateTable I);
	bool dropTable(string tableName);
	bool readIndexFile();
	bool writeIndexFile();
	bool isIndex(string indexName);
	bool isIndex(string tableName, string attributeName);
	string getIndex(string tableName, string attributeName);
	bool createIndex(miniCreateIndex I);
	bool dropIndex(string indexName);
	bool dropDatabase();
	CatalogManager();
	~CatalogManager();

private:
	vector<table> tables;
	vector<index> indices;
	int tableNum;
	int indexNum;
};

int  splitString(const string & strSrc, const std::string& strDelims, vector<string>& strDest);