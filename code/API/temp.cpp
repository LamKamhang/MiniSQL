#include "API.h"
#include <iomanip>
using namespace std;
size_t MINISQL_BASE::counter = 0;

bool API::Insert(miniInsert I)
{
	table t;
	int i;
	TuplePtr tp;
	t = cm.getTable(I.tableName);
	tp = rm.InsertRecord(I, t);
	if (tp.offset == -1)
		return 0;
	string indexName;
	for (i = 0; i < I.insertNum - 1; i++)
	{
		if (cm.isIndex(I.tableName, t.attributes[i].name))
		{
			indexName = cm.getIndex(I.tableName, t.attributes[i].name);
			im._insert(indexName, I.cond[i], tp);
		}
	}
	counter++;
	cout << "Insert successfully!" << endl;
	return 1;
}

bool API::Create(miniCreateTable I)
{
	bm.GetNewBlock(I.tableName, 0);
	cm.createTable(I);
	//index.create
	counter++;
	cout << "Create the table successfully!" << endl;
	return 1;
}

bool API::Drop(miniDropTable I)
{
	bm.DeleteFileBlock(I.tableName);
	cm.dropTable(I.tableName);
	counter++;
	cout << "Drop the table successfully!" << endl;
	return 1;
}

bool API::Select(miniSelect I)
{
	int i;
	table t;
	string indexName;
	t = cm.getTable(I.tableName);
	records r;
	vector<TuplePtr> tp;
	int num = t.attributeNum;
	for (i = 0; i < I.conditionNum; i++)
	{
		if (cm.isIndex(I.tableName, I.cond[i].attributeName))
		{
			indexName = cm.getIndex(I.tableName, I.cond[i].attributeName);
			break;
		}
	}
	if (i == I.conditionNum)
		r = rm.SelectRecord(I, t);
	else
	{
		im._select(indexName, I.cond[i], tp);
		r = rm.SelectByTuples(tp, I, t);
	}
	if (r.recordNum == 0)
	{
		cout << "Empty set!" << endl;
		return 1;
	}
	vector<int> wide;
	for (int i = 0; i < r.attributes.size(); i++)
	{
		cout << "+";
		int print_length;
		if (r.attributes[i].name.length() >= r.attributes[i].length)
		{
			print_length = r.attributes[i].name.length();
			wide.push_back(print_length);
			for (int k = 0; k < print_length; k++)
			{
				cout << "-";
			}
		}
		else
		{
			print_length = r.attributes[i].length;
			wide.push_back(print_length);
			for (int k = 0; k < print_length; k++)
			{
				cout << "-";
			}
		}
	}
	cout << "+" << endl;
	cout << "|";
	for (int j = 0; j < r.attributes.size(); j++)
	{
		cout << setw(wide[j]) << setiosflags(ios::left) << r.attributes[j].name << "|";
	}
	cout << endl;
	for (int i = 0; i < r.attributes.size(); i++)
	{
		cout << "+";
		int print_length;
		if (r.attributes[i].name.length() >= r.attributes[i].length)
		{
			print_length = r.attributes[i].name.length();
			//wide.push_back(print_length);
			for (int k = 0; k < print_length; k++)
			{
				cout << "-";
			}
		}
		else
		{
			print_length = r.attributes[i].length;
			//wide.push_back(print_length);
			for (int k = 0; k < print_length; k++)
			{
				cout << "-";
			}
		}
	}
	cout << "+" << endl;
	for (int i = 0; i < r.record_list.size(); i++)
	{
		cout << "|";
		for (int j = 0; j < r.attributes.size(); j++)
		{
			if (r.record_list[i].cond[j].type == Integer)
			{
				cout << setw(wide[j]) << setiosflags(ios::left) << r.record_list[i].cond[j].intValue << "|";
			}
			else if (r.record_list[i].cond[j].type == Float)
			{
				cout << setw(wide[j]) << setiosflags(ios::left) << r.record_list[i].cond[j].floatValue << "|";
			}
			else
			{
				cout << setw(wide[j]) << setiosflags(ios::left) << r.record_list[i].cond[j].stringValues << "|";
			}
		}
		cout << endl;
	}
	for (int i = 0; i < r.attributes.size(); i++)
	{
		cout << "+";
		int print_length;
		if (r.attributes[i].name.length() >= r.attributes[i].length)
		{
			print_length = r.attributes[i].name.length();
			//wide.push_back(print_length);
			for (int k = 0; k < print_length; k++)
			{
				cout << "-";
			}
		}
		else
		{
			print_length = r.attributes[i].length;
			//wide.push_back(print_length);
			for (int k = 0; k < print_length; k++)
			{
				cout << "-";
			}
		}
	}
	cout << "+" << endl;
	return 1;
}
bool API::Delete(miniDelete I)
{
	Block *b;
	char *data;
	int i, j;
	table t;
	records r;
	string indexName;
	vector<TuplePtr> tp;
	t = cm.getTable(I.tableName);
	int num = t.attributeNum;
	for (i = 0; i < I.conditionNum; i++)
	{
		if (cm.isIndex(I.tableName, I.cond[i].attributeName))
		{
			indexName = cm.getIndex(I.tableName, I.cond[i].attributeName);
			break;
		}
	}
	if (i == I.conditionNum)
		rm.DeleteRecord(I, t);
	else
	{
		im._select(indexName, I.cond[i], tp);
		r = rm.DeleteByTuples(tp, I, t);
		for (j = 0; j < r.recordNum; j++)
		{
			im._delete(indexName, r.record_list[j].cond[i]);
		}
	}
	counter++;
	cout << "Delete successfully!" << endl;
	return 1;
}

bool API::CreateIndex(miniCreateIndex I)
{
	records r;
	table t;
	int i;
	t = cm.getTable(I.tableName);
	if (cm.isIndex(I.indexName))
	{
		cout << "The index has already existed!" << endl;
		return false;
	}
	else
	{
		cm.createIndex(I);
		r = rm.SelectForCreateIndex(I, t);
		for (i = 0; i < t.attributeNum; i++)
		{
			if (I.attributeName == t.attributes[i].name)
				break;
		}
		im._create(I.indexName, r, i);
	}
	counter++;
	cout << "Create the index successfully!" << endl;
	return true;
}

bool API::DropIndex(miniDropIndex I)
{
	cm.dropIndex(I.indexName);
	im._drop(I.indexName);
	counter++;
	cout << "Drop the index successfully!" << endl;
	return 1;
}
