#pragma once
#include<string>
#include<vector>
using namespace std;

enum TYPE
{
	INT,
	CHAR,
	FLOAT
};

class value
{
public:
	vector<string> values;
};

class index
{
public:
	string indexName;
	string tableName;
	string attributeName;
	int col;
	index(string s1, string s2, string s3)
	{
		indexName = s1;
		tableName = s2;
		attributeName = s3;
	}
	index(){}
		
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
	attribute(){}
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
	table(){}
	table(string tN, int aN)
	{
		tableName = tN;
		attributeNum = aN;
		valueNum = 0;
	}
	
};

class CatalogManage
{
public:
	int getTableNum();
	int getIndexNum();
	bool isTable(string tableName);
	table getTable(string tableName);
	bool isAttribute(string tableName, string AN);
	bool isIndex(string indexName);
	bool isIndex(string tableName, string attributeName);
	void createTable(table& newTable);
	index getIndex(string tableName, string attributeName);
	void dropTable(string tableName);
	void createIndex(index& newIndex);
	void dropIndex(string indexName);
	CatalogManage();
	~CatalogManage();
private:
	vector<table> tables;
	vector<index> indices;
	int tableNum;
	int indexNum;
};
