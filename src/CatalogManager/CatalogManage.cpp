#include "CatalogManage.h"



CatalogManage::CatalogManage()
{
	tableNum = 0;
	indexNum = 0;
}


CatalogManage::~CatalogManage()
{
}

int CatalogManage::getTableNum()
{
	return tableNum;
}
int CatalogManage::getIndexNum()
{
	return indexNum;
}
bool CatalogManage::isTable(string tableName)
{
	for (int i = 0; i < tableNum; i++)
	{
		if (tables[i].tableName == tableName)
			return true;
	}
	return false;
}
table CatalogManage::getTable(string tableName)
{
	table t;
	for (int i = 0; i < tableNum; i++)
	{
		if (tables[i].tableName == tableName)
			return tables[i];
	}
	return t;
}

bool CatalogManage::isAttribute(string tableName, string AN)
{
	table tableTest = getTable(tableName);
	for (int i = 0; i < tableTest.attributes.size(); i++)
	{
		if (tableTest.attributes[i].name == AN)
			return true;
	}
	return false;
}

bool CatalogManage::isIndex(string indexName)
{
	for (int i = 0; i < indexNum; i++)
	{
		if (indices[i].indexName == indexName)
			return true;
	}
	return false;
}

bool CatalogManage::isIndex(string tableName, string attributeName)
{
	for (int i = 0; i < indexNum; i++)
	{
		if (indices[i].tableName == tableName&&indices[i].attributeName==attributeName)
			return true;
	}
	return false;
}

void CatalogManage::createTable(table& newTable)
{
	if (!isTable(newTable.tableName))
	{
		tableNum++;
		tables.push_back(newTable);
	}
}

index CatalogManage::getIndex(string tableName, string attributeName)
{
	index i;
	for (int i = 0; i < indexNum; i++)
	{
		if (indices[i].tableName == tableName&&indices[i].attributeName == attributeName)
		{
			return indices[i];
		}
	}
	return i;
}

void CatalogManage::dropTable(string tableName)
{
	if (isTable(tableName))
	{
		for (int i = 0; i < tableNum; i++)
		{
			if (tables[i].tableName == tableName)
			{
				tables.erase(tables.begin() + i);
				tableNum--;
			}
		}
		for (int i = 0; i < indexNum; i++)
		{
			if (indices[i].tableName == tableName)
			{
				indices.erase(indices.begin() + i);
				indexNum--;
			}
		}
	}
}

void CatalogManage::createIndex(index& newIndex)
{
	if (!isIndex(newIndex.indexName))
	{
		indexNum++;
		indices.push_back(newIndex);
	}
}

void CatalogManage::dropIndex(string indexName)
{
	if (isIndex(indexName))
	{
		for (int i = 0; i < indexNum; i++)
		{
			if (indices[i].indexName == indexName)
			{
				indices.erase(indices.begin() + i);
				indexNum--;
			}
		}
	}
}

