#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include "API.h"
#include <iomanip>
using namespace std;

bool API::Insert(miniInsert I)
{
	table t;
	int i;
	TuplePtr tp;
	t=cm.getTable(I.tableName);
	tp=rm.InsertRecord(I,t);
	string indexName;
	for(i=0;i<I.insertNum-1;i++)
	{
		if(cm.isIndex(I.tableName,t.attributes[i].name))
		{
			indexName=cm.getIndex(I.tableName, t.attributes[i].name);
			im._insert(indexName, I.cond[i], tp);
		}
	}
	return 1;
}

bool API::Create(miniCreateTable I)
{
	bm.GetNewBlock(I.tableName,0);
	cm.createTable(I);
	//index.create
	return 1;
}

bool API::Drop(miniDropTable I)
{
	bm.DeleteFileBlock(I.tableName);
	cm.dropTable(I.tableName);
	return 1;
}

bool API::Select(miniSelect I)
{
	int i;
	table t;
	string indexName;
	t=cm.getTable(I.tableName);
	records r;
	vector<TuplePtr> tp;
	int num=t.attributeNum;
	for(i=0;i<I.conditionNum;i++)
	{
		if(cm.isIndex(I.tableName,I.cond[i].attributeName))
		{
			indexName=cm.getIndex(I.tableName, I.cond[i].attributeName);
			break;
		}
	}
	if (i == I.conditionNum)
	r=rm.SelectRecord(I,t);
	else
	{
		im._select(indexName,I.cond[i],tp);
		r=rm.SelectByTuples(tp,I,t);
	}
	vector<int> wide;
	for (int i = 0; i < r.attributes.size(); i++)
	{
		cout << "+";
		for (int j = 0; j < r.attributes[i].length; j++)
		{
			if (r.attributes[i].name.length() >= r.attributes[i].length)
			{
				wide.push_back(r.attributes[i].name.length());
				for (int k = 0; k<r.attributes[i].name.length(); k++)
				{
					cout << "-";
				}
			}
			else
			{
				wide.push_back(r.attributes[i].length);
				for (int k = 0; k<r.attributes[i].length; k++)
				{
					cout << "-";
				}
			}
		}
	}
	cout << "+" << endl;
	cout << "|";
	for (int j = 0; j<r.attributes.size(); j++)
	{
		cout << setw(wide[j]) << r.attributes[j].name << "|";
	}
	cout << endl;
	for (int i = 0; i < r.attributes.size(); i++)
	{
		cout << "+";
		for (int j = 0; j < r.attributes[i].length; j++)
		{
			if (r.attributes[i].name.length() >= r.attributes[i].length)
			{
				for (int k = 0; k<r.attributes[i].name.length(); k++)
				{
					cout << "-";
				}
			}
			else
			{
				wide.push_back(r.attributes[i].length);
				for (int k = 0; k<r.attributes[i].length; k++)
				{
					cout << "-";
				}
			}
		}
	}
	cout << "+" << endl;
	for (int i = 0; i<r.record_list.size(); i++)
	{
		cout << "|";
		for (int j = 0; j<r.attributes.size(); j++)
		{
			if (r.record_list[i].cond[j].type == Integer)
			{
				cout << setw(wide[j]) << r.record_list[i].cond[j].intValue << "|";
			}
			else if (r.record_list[i].cond[j].type == Float)
			{
				cout << setw(wide[j]) << r.record_list[i].cond[j].floatValue << "|";
			}
			else
			{
				cout << setw(wide[j]) << r.record_list[i].cond[j].stringValues << "|";
			}
		}
		cout << endl;
	}
	for (int i = 0; i < r.attributes.size(); i++)
	{
		cout << "+";
		for (int j = 0; j < r.attributes[i].length; j++)
		{
			if (r.attributes[i].name.length() >= r.attributes[i].length)
			{
				for (int k = 0; k<r.attributes[i].name.length(); k++)
				{
					cout << "-";
				}
			}
			else
			{
				wide.push_back(r.attributes[i].length);
				for (int k = 0; k<r.attributes[i].length; k++)
				{
					cout << "-";
				}
			}
		}
	}
	cout << "+" << endl;
	return 1;
}
bool API::Delete(miniDelete I)
{
	Block* b;
	char* data;
	int i,j;
	table t;
	records r;
	string indexName;
	vector<TuplePtr> tp;
	t=cm.getTable(I.tableName);
	int num=t.attributeNum;
	for(i=0;i<I.conditionNum;i++)
	{
		if(cm.isIndex(I.tableName,I.cond[i].attributeName))
		{
			indexName=cm.getIndex(I.tableName,I.cond[i].attributeName);
			break;
		}
	}
	if(i==I.conditionNum)
	rm.DeleteRecord(I,t);
	else
	{
		im._select(indexName,I.cond[i],tp);
		r=rm.DeleteByTuples(tp,I,t);
		for (j = 0; j < r.recordNum; j++)
		{
			im._delete(indexName, r.record_list[j].cond[i]);
		}
	}
	return 1;
}

bool API::CreateIndex(miniCreateIndex I)
{
	records r;
	table t;
	int i;
	t=cm.getTable(I.tableName);
	if(cm.isIndex(I.indexName))
	{	
		return false;
	}
	else
	{
		cm.createIndex(I);
		r=rm.SelectForCreateIndex(I,t);
		for(i=0;i<t.attributeNum;i++)
		{
			if(I.attributeName==t.attributes[i].name)break;
		}
		im._create(I.indexName,r,i);
	}
	return true;
}

bool API::DropIndex(miniDropIndex I)
{
	cm.dropIndex(I.indexName);
	im._drop(I.indexName);
	return 1;
}
