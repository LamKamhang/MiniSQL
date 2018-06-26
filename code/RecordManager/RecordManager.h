#pragma once
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>
#include "../interface.h"
#include "../BufferManager/BufferManager.h"
#include "../Interpreter/Interpreter.h"
using namespace MINISQL_BASE;

/*Record Manager*/
/*记录的存储是以逗号为间隔符，换行号为记录的间隔符,最后以/00为结尾*/
class miniRecord
{
public:
	condition cond[32];//记录内容 
	int conditionNum;//属性数目 
	int pos;//所在块的偏移 
	int blockNum;//所在块的编号 
};

class records
{
public:
	records(){};
	~records(){};
	records(table t):
		attriNum(t.attributeNum), attributes(t.attributes), recordNum(0)
	{};
	void insert(Block* b,int pos);
	int attriNum;//属性的个数 
	std::vector<attribute> attributes;//属性信息 
	int recordNum;//记录的个数
	std::vector<miniRecord> record_list;//记录的数据
};

class RecordManager
{
public:
	RecordManager(){};
	~RecordManager(){};
	//mode中，需要有表的创建与删除，记录的读取，新增，删除
	/*对于API而言，需要给一张表的信息（catelog)以及插入/删除/选择的信息（interpreter）*/
	TuplePtr InsertRecord(miniInsert I,table t);
	bool DeleteRecord(miniDelete I,table t);//直接根据条件进行删除 
	bool DeleteRecordByPos(Block* b,int pos,miniDelete I,table t);//根据block以及偏移量pos删除具体某一条record 
	records SelectRecord(miniSelect I,table t);//select返回所有符合条件的记录 
	records SelectRecordByPos(Block* b,int pos,miniSelect I,table t);//根据block以及偏移量pos返回具体一条记录 
	records SelectForCreateIndex(miniCreateIndex I,table t);
	records SelectByTuples(std::vector<TuplePtr> &tps, miniSelect I, table t);
	records DeleteByTuples(std::vector<TuplePtr> &tps, miniDelete I, table t);
	int LenOfRecord(miniInsert I,table t);
	int getInt(Block *block, int posBegin);
	void setInt(Block *block, int posBegin,int num);
	float getFloat(Block *block, int posBegin);
	void setFloat(Block *block, int posBegin,float num);
	std::string getString(Block *block, int posBegi);
	void setString(Block *block, int posBegin, std::string str);
private:
	int cmpstring(std::string s1, std::string s2);
	int FindEnd(char* data);
	int located(std::string name,table t);
	int findAttri(Block *block, int posBegin,int order);
	bool cmpAttri(Block *block, int posBegin,condition c);
	int FindNextRecord(Block *block, int posBegin);
	void fullblack(Block *block, int posBegin);
};
//hi