#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include "API.h"
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
	return 1;
}

bool API::Drop(miniDropTable I)
{
	bm.DeleteFileBlock(I.tableName);
	cm.dropTable(I.tableName);
	return 1;
}

records API::Select(miniSelect I)
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
		if(cm.isIndex(I.tableName,t.attributes[i].name))
		{
			indexName=cm.getIndex(I.tableName, t.attributes[i].name);
			break;
		}
	}
	if (i == I.conditionNum)
	r=rm.SelectRecord(I,t);
	else
	{
		im._select(indexName,I.cond[i],tp);
//		r=rm.SelectByTuples(tp,I,t);
		//index_insert，名字，条件 
		//返回一个tuple 
		//再调用rm进行数据选择 
	}
	return r;
}
bool API::Delete(miniDelete I)
{
	Block* b;
	char* data;
	int i;
	table t;
	records r;
	string indexName;
//	vector<TuplePtr> tp1,tp2;
	t=cm.getTable(I.tableName);
	int num=t.attributeNum;
/*	for(i=0;i<I.insertNum;i++)
	{
		if(cm.isIndex(I.tableName,I.cond[i].attributeName))
		{
			indexName=cm.getIndex(I.tableName,I.cond[i].attributeName);
			_select(indexName,I.cond[i],tp1);
		}
	}
	if(i==I.insertNum)*/
	rm.DeleteRecord(I,t);
/*	else
	{
		tp=im._delete(indexName,I.cond[i]);
		r=DeleteByTuples(tp,I);
	}*/
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
