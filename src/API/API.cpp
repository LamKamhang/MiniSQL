/*API*/
class API
{
public:
	API();
	~API();
	//mode中，需要有表的创建与删除，记录的读取，新增，删除
	bool Insert(miniInsert I);
	bool Create(miniCreate I);
	bool Drop(miniDrop I);
	bool Select(miniSelect I);
	bool Delete(miniDelete I);
	bool CreateIndex();//暂缺 
	bool DropIndex();//暂缺 
private:
	
}



bool API::Insert(miniInsert I)
{
	RecordManger rm;
	rm.Insert(mininInsert I);
	/*缺少对index的insert，需要先检测index是否存在，然后再insert*/
}

bool API::CreateTable(_Instruction I)
{
	BufferManager bm;
	bm.GetNewBlock(I.tableName,0);
	/*缺少catelog的创建*/
}

bool API::Drop(_Instruction I)
{
	BufferManager bm;
	bm.DeleteFileBlock(I.tableName);
	/*缺少catelog的删除*/
}

char* API::Select(miniSelect I)
{
	CatelogManager cm; 
	RecordManager rm;
	int i;
	table t;
	t=cm.getTable(I.tableName);
	records r;
	int num=t.attributeNum;
	/*首先需要判断是否存在index
	.
	.
	.*/
	/*若没有index，则直接通过rm进行select*/
	r=rm.SelectRecord(I);
	/*打印
	.
	.
	.*/
}
bool API::Delete(_Instruction I)
{
	CatelogManager cm;
	Block* b;
	b=Readout(I.tableName);//读取表的第一个block 
	char* data,*temp;
	int i;
	table t;
	t=cm.getTable(I.tableName);
	int num=t.attributeNum;
	/*首先需要判断是否存在index
	.
	.
	.*/
	/*若没有index，则直接通过rm进行delect*/
	r=rm.DeleteRecord(I);
}
