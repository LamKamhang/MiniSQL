/*Record Manager*/
/*记录的存储是以逗号为间隔符，换行号为记录的间隔符,最后以/00为结尾*/
class RecordManager
{
public:
	RecordManager(){};
	~RecordManager(){};
	//mode中，需要有表的创建与删除，记录的读取，新增，删除
	/*对于API而言，需要给一张表的信息（catelog)以及插入/删除/选择的信息（interpreter）*/
	bool InsertRecord(miniInsert I);
	bool DeleteRecord(miniDelete I,table t);//直接根据条件进行删除 
	bool DeleteRecordByBlock(Block* b,miniDelete I,table t);//根据所在block以及条件进行删除 
	bool DeleteRecordByPos(Block* b,int pos,miniDelete I,table t);//根据block以及偏移量pos删除具体某一条record 
	records SelectRecord(miniSelect I,table t);//select返回所有符合条件的记录 
	records SelectRecordByBlock(Block* b,miniSelect I,table t);//返回block块中符合条件的记录 
	records SelectRecordByPos(Block* b,int pos,miniSelect I,table t);//根据block以及偏移量pos返回具体一条记录 
//	TuplePtr SelectForIndexInserted(miniInsert I,table t);
	records SelectForCreateIndex(miniCreateIndex I,table t);
//	records SelectByTuples(TuplePtr tp,miniSelect I,table t);
//	void DeleteByTuples(TuplePtr tp,miniDelete I);

private:
	int getInt(Block *block, int posBegin);
	void setInt(Block *block, int posBegin,int num);
	float getFloat(Block *block, int posBegin);
	void setFloat(Block *block, int posBegin,float num);
	std::string getString(Block *block, int posBegi);
	void setString(Block *block, int posBegin, std::string str);
	int FindEnd(char* data);
	int located(string name,table t);
	int findAttri(Block *block, int posBegin,int order);
	bool cmpAttri(Block *block, int posBegin,condition c);
	int FindNextRecord(Block *block, int posBegin);
	void fullblack(Block *block, int posBegin);
};

