/*API*/
class API
{
public:
	API(){};
	~API(){};
	//mode中，需要有表的创建与删除，记录的读取，新增，删除
	bool Insert(miniInsert I);
	bool Create(miniCreateTable I);
	bool Drop(miniDropTable I);
	records Select(miniSelect I);
	bool Delete(miniDelete I);
	bool CreateIndex(miniCreateIndex I);
	bool DropIndex(miniDropIndex I);
private:
	
};

bool API::Insert(miniInsert I)
{
	//IndexManager im;
	RecordManager rm;
	rm.InsertRecord(I);
	CatalogManager cm; 
	/*int i;
	TuplePtr t;
	string indexName;
	for(i=0;i<I.insertNum;i++)
	{
		if(cm.isIndex(I.tableName,I.cond[i].attributeName))
		{
			indexName=cm.getIndex(I.tableName,I.cond[i].attributeName);
			t=SelectForIndexInserted(I);	
			im.insert(indexName, I.cond[i], t);
		}
	}	*/
	return 1;
}

bool API::Create(miniCreateTable I)
{
	BufferManager bm;
	bm.GetNewBlock(I.tableName,0);
	CatalogManager cm;
	cm.createTable(I);
}

bool API::Drop(miniDropTable I)
{
	BufferManager bm;
	bm.DeleteFileBlock(I.tableName);
	CatalogManager cm;
	cm.dropTable(I.tableName);
}

records API::Select(miniSelect I)
{/*
	CatalogManager cm; 
	RecordManager rm;
	int i;
	table t;
	string indexName;
	t=cm.getTable(I.tableName);
	records r;
	vector<TuplePtr> tp;
	int num=t.attributeNum;
	for(i=0;i<I.insertNum;i++)
	{
		if(cm.isIndex(I.tableName,I.cond[i].attributeName))
		{
			indexName=cm.getIndex(I.tableName,I.cond[i].attributeName);
			break;
		}
	}
	if(i==I.insertNum)
	r=rm.SelectRecord(I);
	else
	{
		tp=im._select(indexName,I.cond[i]);
		r=SelectByTuples(tp,I);
		//index_insert，名字，条件 
		//返回一个tuple 
		//再调用rm进行数据选择 
	}
	return r;
	*/
}
bool API::Delete(miniDelete I)
{/*
	CatalogManager cm;
	IndexManager im;
	RecordManager rm;
	Block* b;
	b=Readout(I.tableName);//读取表的第一个block 
	char* data,*temp;
	int i;
	table t;
	records r;
	string indexName;
	vector<TuplePtr> tp1,tp2;
	t=cm.getTable(I.tableName);
	int num=t.attributeNum;
	for(i=0;i<I.insertNum;i++)
	{
		if(cm.isIndex(I.tableName,I.cond[i].attributeName))
		{
			indexName=cm.getIndex(I.tableName,I.cond[i].attributeName);
			_select(indexName,I.cond[i],tp1);
		}
	}
	
	if(i==I.insertNum)
	r=rm.DeleteRecord(I);
	else
	{
		tp=im._delete(indexName,I.cond[i]);
		r=DeleteByTuples(tp,I);
	}
*/}

bool API::CreateIndex(miniCreateIndex I)
{
/*
	IndexManager im;
	CatalogManager cm;
	RecordManager rm;
	records r;

	if(cm.isIndex(I.indexName))
	{
		return false;
	}
	else
	{
		cm.createIndex(I);
		//indexName,type
		r=rm.SelectForCreateIndex(I);
		im.create(I.indexName,r,index的位置);
	}
	return 1;
*/
}

bool API::DropIndex(miniDropIndex I)
{/*
	CatalogManager cm;
	cm.dropIndex(I.indexName);
	//index_manager
	IndexManager im;
	im.drop(I.indexName);
	return 1;*/
}
