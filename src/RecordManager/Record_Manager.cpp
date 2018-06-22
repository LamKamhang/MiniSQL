/*records SelectByTuples(TuplePtr tp,miniSelect I,table t)
{
	records record(t);
	int i=0;
	while(1)
	record.insert(tp.b,tp.pos);
}*/

/*
records RecordManager::SelectForCreateIndex(miniCreateIndex I,table t)
{
	records record(t);
	Block* b;
	b=Readout(I.tableName);//读取表的第一个block 
	char* data; 
	int i;
	int tempos,pos=0;
	int num=t.attributeNum;
	int order[num];
	for(i=0;i<num;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	while(b)//记录未被读空 
	{
		data=b->data;
		while(data[pos-1]!=0)
		{
			record.insert(b,pos);
			pos=FindNextRecord(block,pos);
		}
		b=getNextBlock(b);
	}
	return record;	
}
*/

/*TuplePtr RecordManager::SelectForIndexInserted(miniInsert I)
{
	Block* b;
	TuplePtr tp;
	b=Writein(I.tableName);
    int pos=FindEnd(b->data);
    if(pos!=0)b->data[pos++]='\n';
    int i;
    tp.pos=pos;
    tp.offset=b->offset;
    for(i=0;i<I.conditionNum;i++)
    {
    	switch(I.cond[i].type)
    	case 1://int 
    		setInt(b,pos,I.cond[i].intValue);
    		pos+=sizeof(int);
    		break;
    	case 2://float
    		setFloat(b,pos,I.cond[i].floatValue);
    		pos+=sizeof(float);
    		break;    		
    	case 3://string
			setString(b,pos,I.cond[i].stringValues);
			pos+=length(I.cond[i].stringValues);
			break;
		b->data[pos++]=',';
    }
    b->[pos-1]=0;
    //当前默认每块都能写进去,晚点根据情况再改 
    Writeback(b);
    return tp;
}*/

bool RecordManager::InsertRecord(miniInsert I)
{
	Block* b;
	BufferManager bm; 
	b=bm.GetBlock(I.tableName,0);
    int pos=FindEnd(b->data);
    if(pos!=0)b->data[pos++]='\n';
    int i;
    int posrecord=pos-1;
    for(i=0;i<I.insertNum;i++)
    {
    	switch(I.cond[i].type)
    	{
    	case 0://int 
    		setInt(b,pos,I.cond[i].intValue);
    		pos+=sizeof(int);
    		break;
    	case 1://float
    		setFloat(b,pos,I.cond[i].floatValue);
    		pos+=sizeof(float);
    		break;    		
    	case 2://string
			setString(b,pos,I.cond[i].stringValues);
			pos+=sizeof(I.cond[i].stringValues);
			break;
		}
		b->data[pos++]=',';
    }
    b->data[pos-1]=254;
    b->IsWritten=1;
    //当前默认每块都能写进去,晚点根据情况再改 
    //插入下一块,暂时没写好 
    return 1;
}

bool RecordManager::DeleteRecord(miniDelete I,table t)
{/*
	Block* b;
	b=Readout(I.tableName);//读取表的第一个block 
	char* data,*temp;
	int pos=0,tempos=0;
	int i;
	int num=t.attributeNum;
	int order[num];
	for(i=0;i<num;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	while(b)//记录未被读空 
	{
		data=b->data;
		while(data[pos-1]!=0)
		{
			for(i=0;i<num;i++)
			{
				tempos=findAttri(b,pos,order[i]);
				if(!cmpAttri(b,tempos,I.cond[i]))
				break;
			}
			if(i==num)//符合条件 
			{
				fullblack(b,pos);
			}
			else
			pos=FindNextRecord(b,pos);
		}
		b=GetNextBlock(b);
	}*/
}

bool RecordManager::DeleteRecordByBlock(Block* b,miniDelete I,table t)
{
	char* data,*temp;
	int tempos,pos=0,i;
	int num=t.attributeNum;
	int order[num];
	for(i=0;i<num;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	data=b->data;
	while(data[pos-1]!=0)
	{
		for(i=0;i<num;i++)
		{
			tempos=findAttri(b,pos,order[i]);
			if(!cmpAttri(b,tempos,I.cond[i]))
			break;
		}
		if(i==num)//符合条件 
		{
			/*填充空格*/
			fullblack(b,pos);
		}
		else
		pos=FindNextRecord(b,pos);
	}	
}

bool RecordManager::DeleteRecordByPos(Block* b,int pos,miniDelete I,table t)
{
	char* data;
	int tempos,i;
	int num=t.attributeNum;
	int order[num];
	for(i=0;i<num;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	for(i=0;i<num;i++)
	{
		tempos=findAttri(b,pos,order[i]);
		if(!cmpAttri(b,tempos,I.cond[i]))
		break;
	}
	if(i==num)//符合条件 
	{
		/*填充空格*/
		fullblack(b,pos);
	}
	return 1;
}

records RecordManager::SelectRecord(miniSelect I,table t)
{
	records record(t);
	Block* b;
	//b=Readout(I.tableName);//读取表的第一个block 
	char* data; 
	int i;
	int tempos,pos=0;
	int num=t.attributeNum;
	int order[num];
	for(i=0;i<num;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	while(b)//记录未被读空 
	{
		data=b->data;
		while(data[pos-1]!=0)
		{
			for(i=0;i<num;i++)
			{
				tempos=findAttri(b,pos,order[i]);
				if(!cmpAttri(b,tempos,I.cond[i]))
				break;
			}
			if(i==num)//符合条件 
			{
				record.insert(b,pos);
			}
			else
			pos=FindNextRecord(b,pos);
		}
	//	b=GetNextBlock(b);
	}
	return record;
}
records RecordManager::SelectRecordByBlock(Block* b,miniSelect I,table t)
{
	records record(t);
	char* data,*temp;
	int tempos,pos=0,i;
	int num=t.attributeNum;
	int order[num];
	for(i=0;i<num;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	data=b->data;
	while(data[pos-1]!=0)
	{
		for(i=0;i<num;i++)
		{
			tempos=findAttri(b,pos,order[i]);
			if(!cmpAttri(b,tempos,I.cond[i]))
			break;
		}
		if(i==num)//符合条件 
		{
			record.insert(b,pos);
		}
		else
		pos=FindNextRecord(b,pos);
	}	
	return record;	
}

records RecordManager::SelectRecordByPos(Block* b,int pos,miniSelect I,table t)
{
	records record(t);
	char* data,*temp;
	int i,tempos;
	int num=t.attributeNum;
	int order[num];
	for(i=0;i<num;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	data=b->data;
	for(i=0;i<num;i++)
	{
		tempos=findAttri(b,pos,order[i]);
		if(!cmpAttri(b,tempos,I.cond[i]))
		break;
	}
	if(i==num)//符合条件 
	{
		record.insert(b,pos);
	}
	return record;
}

/*定位到block最后一个字符*/
int RecordManager::FindEnd(char* data)
{
	int i=0;
	if(data[0]==0)return 0; 
	while(data[i]!=-2)i++;
	return i;
}

/*找到属性的位置（是表的第几个属性）*/
int RecordManager::located(string name,table t)
{
	int i;
	for(i=1;i<=t.attributeNum;i++)
	{
		if(t.attributes[i].name==name)
		break;
	}
	return i;
}

/*找到属性偏移*/ 
int RecordManager::findAttri(Block *block, int posBegin,int order)
{
	int i=0;
	char* p=block->data;
	for(i=0;i<order;i++)
	{
		while(p[posBegin++]!=',');
	}
	return posBegin;
}

/*比较属性*/
bool RecordManager::cmpAttri(Block *block, int posBegin,condition c)
{
	int i;
	float f;
	string s;
	switch (c.type)
	{
		case 1:
			i=getInt(block,posBegin);
			if(i==c.intValue)return 1;
			else return 0;
		case 2:
			f=getFloat(block,posBegin);
			if(f==c.floatValue)return 1;
			else return 0;
		case 3:
			s=getString(block,posBegin);
			if(s==c.stringValues)return 1;
			else return 0;
		default:
			return 0;
	}
}

/*根据当前偏移量返回下一条偏移*/
int RecordManager::FindNextRecord(Block *block, int posBegin)
{
	int i;
	while(block->data[posBegin+i]!='\n')i++;
	return posBegin+i+1;
}

/*用空格代替delete*/
void RecordManager::fullblack(Block *block, int posBegin)
{
	int i=0;
	char* p=block->data+posBegin;
	while(p[i]!='\n'&&p[i]!=0)p[i++]=' ';
	return; 
}

int RecordManager::getInt(Block *block, int posBegin)
{
	int *info = (int *)(block->data + posBegin);
	return *info;
}

void RecordManager::setInt(Block *block, int posBegin, int num)
{
	int *info = &num;
	for (int i = 0; i < sizeof(int); i++)
	{
		block->data[posBegin + i] = *((char*)info + i);
	}
}

float RecordManager::getFloat(Block *block, int posBegin)
{
	float *info = (float *)(block->data + posBegin);
	return *info;
}

void RecordManager::setFloat(Block *block, int posBegin, float num)
{
	float *info = &num;
	for (int i = 0; i < sizeof(float); i++)
	{
		block->data[posBegin + i] = *((char*)info + i);
	}
}

std::string RecordManager::getString(Block *block, int posBegin)
{
	int num=0; 
	char* str = (char*)block->data + posBegin;
	if(str[num]!='\n'&&str[num]!=',')num++;
	std::string theString(str, num);
	return theString;
}

void RecordManager::setString(Block *block, int posBegin, std::string str)
{
	strcpy((char *)(block->data + posBegin), str.c_str());
}
