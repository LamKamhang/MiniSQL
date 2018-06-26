#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS
#pragma once
#include "../interface.h"
#include "../Interpreter/Interpreter.h"
#include "../RecordManager/RecordManager.h"
#include "../BufferManager/BufferManager.h"
#include "../IndexManager/IndexManager.h"

using namespace MINISQL_BASE;

/*API*/
class API
{
public:
	API():
		cm(), rm(),
		bm(), im(bm)
	{};
	~API(){};
	//mode中，需要有表的创建与删除，记录的读取，新增，删除
	bool Insert(miniInsert I);
	bool Create(miniCreateTable I);
	bool Drop(miniDropTable I);
	bool Select(miniSelect I);
	bool Delete(miniDelete I);
	bool CreateIndex(miniCreateIndex I);
	bool DropIndex(miniDropIndex I);
private:
	CatalogManager cm;
	RecordManager rm;
	BufferManager bm;
	IndexManager im;
};