#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include "RecordManager.h"
using namespace std;

void records::insert(Block* b,int pos)
{
	RecordManager rm;
	int c;
	int i;
	float f;
	std::string s;
	miniRecord r;
	r.conditionNum=attriNum;
	r.pos=pos;
	r.blockNum=b->Offset; 
	for(c=0;c<attriNum;c++)
	{
		r.cond[c].type=attributes[c].type;
		switch (attributes[c].type)
		{
		case 1://int
			i = rm.getInt(b, pos);
			r.cond[c].intValue = i;
			pos += sizeof(int);
			break;
		case 2://float
			f = rm.getFloat(b, pos);
			r.cond[c].floatValue = f;
			pos += sizeof(float);
			break;
		case 3://std::string
			s = rm.getString(b, pos);
			r.cond[c].stringValues = s;
			pos += attributes[c].length;
			break;
		}
		pos++;//跳过分隔符 
	}
	record_list.push_back(r); 
	recordNum++;
}

records RecordManager::SelectForCreateIndex(miniCreateIndex I,table t)
{
	records record(t);
	Block* b;
	BufferManager bm;
	b=bm.GetBlock(I.tableName,0);
	char* data; 
	int i,offset=1;
	int tempos,pos=0;
	int num=t.attributeNum;
	while(b)//记录未被读空 
	{
		data=b->data;
		while(data[pos-1]!=-2)
		{
			record.insert(b,pos);
			pos=FindNextRecord(b,pos);
		}
		b=bm.GetBlock(I.tableName,offset++);
	}
	return record;	
}
	
records RecordManager::DeleteByTuples(std::vector<TuplePtr> &tps, miniDelete I, table t)
{
	BufferManager bm;
	Block* b;
	records r(t);
	TuplePtr tp;
	int pos;
	int i, k;
	int order[32];
	for (i = 0; i<I.conditionNum; i++)
	{
		order[i] = located(I.cond[i].attributeName, t);
	}
	int tempos;
	for (k = 0; k < tps.size(); k++)
	{
		tp = tps[k];
		b = bm.GetBlock(I.tableName, tp.blockID);
		pos = tp.offset;
		for (i = 0; i<I.conditionNum; i++)
		{
			tempos = findAttri(b, pos, order[i]);
			if (!cmpAttri(b, tempos, I.cond[i]))
				break;
		}
		if (i == I.conditionNum)//符合条件 
		{
			r.insert(b, pos);
			fullblack(b,pos);
			b->IsWritten = 1;
		}
	}
	return r;
}
records RecordManager::SelectByTuples(vector<TuplePtr> &tps, miniSelect I, table t)
{
	BufferManager bm;
	Block* b;
	records r(t);
	TuplePtr tp;
	int pos;
	int i, k;
	int order[32];
	for (i = 0; i<I.conditionNum; i++)
	{
		order[i] = located(I.cond[i].attributeName, t);
	}
	int tempos;
	for (k = 0; k < tps.size(); k++)
	{
		tp = tps[k];
		b = bm.GetBlock(I.tableName, tp.blockID);
		pos = tp.offset;
		for (i = 0; i<I.conditionNum; i++)
		{
			tempos = findAttri(b, pos, order[i]);
			if (!cmpAttri(b, tempos, I.cond[i]))
				break;
		}
		if (i == I.conditionNum)//符合条件 
		{
			r.insert(b, pos);
		}
	}
	return r;
}

int RecordManager::LenOfRecord(miniInsert I,table t)
{
	int lenR=0;
    int i;
    int k;
    for(i=0;i<I.insertNum-1;i++)
    {
    	switch(I.cond[i].type)
    	{
    	case Integer://int 
       		lenR+=sizeof(int);
    		break;
    	case Float://float
    		lenR+=sizeof(float);
    		break;    		
    	case String://string
			lenR+=t.attributes[i].length;
			break;
		}
    }
    return lenR;
}


TuplePtr RecordManager::InsertRecord(miniInsert I,table t)
{
	TuplePtr tp;
	int i=1,k;
	Block* b;
	BufferManager bm;
	b=bm.GetBlock(I.tableName,0);
    int pos=FindEnd(b->data);
    if(pos!=0)b->data[pos++]=253;
    int lenR=LenOfRecord(I,t);//待测试 
    while(lenR+pos>=1024*4)
    {
    	if(!(b=bm.GetBlock(I.tableName,i++)))
    	{
    		b=bm.GetNewBlock(I.tableName,i-1);
    	}
    	pos=FindEnd(b->data);
    }
	tp.blockID = b->Offset;
	tp.offset = pos;
    for(i=0;i<I.insertNum-1;i++)
    {
    	switch(I.cond[i].type)
    	{
    	case Integer://int 
    		setInt(b,pos,I.cond[i].intValue);
    		pos+=sizeof(int);
    		break;
    	case Float://float
    		setFloat(b,pos,I.cond[i].floatValue);
    		pos+=sizeof(float);
    		break;    		
    	case String://string
			setString(b,pos,I.cond[i].stringValues);
			pos+=t.attributes[i].length;
			break;
		}
		b->data[pos++]=',';
    }
    b->data[pos-1]=254;
    b->IsWritten=1;
    return tp;
}

bool RecordManager::DeleteRecord(miniDelete I,table t)
{
	Block* b;
	BufferManager bm;
	b=bm.GetBlock(I.tableName,0);//读取表的第一个block 
	char* data;
	int pos=0,tempos=0;
	int i,offset=1;
	int num=I.conditionNum;
	int order[32];
	for(i=0;i<num;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	while(b)//记录未被读空 
	{
		data=b->data;
		while(data[pos-1]!=-2)
		{
			while (data[pos] == -1)pos = FindNextRecord(b, pos);
			for(i=0;i<num;i++)
			{
				tempos=findAttri(b,pos,order[i]);
				if(!cmpAttri(b,tempos,I.cond[i]))
				break;
			}
			if(i==num)//符合条件 
			{
				fullblack(b,pos);
				b->IsWritten=1; 
			}
			pos=FindNextRecord(b,pos);
		}
		b=bm.GetBlock(I.tableName,offset++);
	}
	return 1;
}

bool RecordManager::DeleteRecordByPos(Block* b,int pos,miniDelete I,table t)
{
	char* data;
	int tempos,i;
	int num=I.conditionNum;
	int order[32];
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
		b->IsWritten=1; 
	}
	return 1;
}

records RecordManager::SelectRecord(miniSelect I,table t)
{
	Block* b;
	BufferManager bm;
	b=bm.GetBlock(I.tableName,0);//读取表的第一个block 
	char* data;
	int pos=0,tempos=0;
	records record(t);
	int i,offset=1;
	int num=t.attributeNum;
	int order[32];
	for(i=0;i<I.conditionNum;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	while(b)//记录未被读空 
	{
		data=b->data;
		while(data[pos-1]!=-2||pos==0)
		{
			while(data[pos]==-1)pos=FindNextRecord(b,pos);
			for(i=0;i<I.conditionNum;i++)
			{
				tempos=findAttri(b,pos,order[i]);
				if(!cmpAttri(b,tempos,I.cond[i]))
				break;
			}
			if(i==I.conditionNum)//符合条件 
			{
				record.insert(b,pos);//没有成功insert 
			}
			pos=FindNextRecord(b,pos);
		}
		b=bm.GetBlock(I.tableName,offset++);
	}
	return record;
}

records RecordManager::SelectRecordByPos(Block* b,int pos,miniSelect I,table t)
{
	records record(t);
	char* data,*temp;
	int i,tempos;
	int num=I.conditionNum;
	int order[32];
	for(i=0;i<I.conditionNum;i++)
	{
		order[i]=located(I.cond[i].attributeName,t);
	}
	data=b->data;
	for(i=0;i<I.conditionNum;i++)
	{
		tempos=findAttri(b,pos,order[i]);
		if(!cmpAttri(b,tempos,I.cond[i]))
		break;
	}
	if(i==num)//符合条件 
	{
		record.insert(b,pos);
		b->IsWritten=1;
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
	for(i=0;i<t.attributeNum;i++)
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
		while(p[posBegin]!=','&&p[posBegin]!=-3&&p[posBegin]!=-2)posBegin++;
	}
	return posBegin+1;
}

bool RecordManager::cmpAttri(Block *block, int posBegin,condition c)
{
	int i;
	float f;
	string s;
	switch (c.type)
	{
		case Integer:
			i=getInt(block,posBegin);
			switch (c.oprt)
			{
			case EQ:if (i == c.intValue)return 1; break;
			case NE:if (i != c.intValue)return 1; break;
			case LE:if (i <= c.intValue)return 1; break;
			case GE:if (i >= c.intValue)return 1; break;
			case LT:if (i <  c.intValue)return 1; break;
			case GT:if (i >  c.intValue)return 1; break;
			}
			return 0;
		case Float:
			f = getFloat(block, posBegin);
			switch (c.oprt)
			{
			case EQ:if (f == c.floatValue)return 1; break;
			case NE:if (f != c.floatValue)return 1; break;
			case LE:if (f <= c.floatValue)return 1; break;
			case GE:if (f >= c.floatValue)return 1; break;
			case LT:if (f <  c.floatValue)return 1; break;
			case GT:if (f >  c.floatValue)return 1; break;
			}
			return 0;
		case String:
			s=getString(block,posBegin);
			switch (c.oprt)
			{
			case EQ:if (s == c.stringValues)return 1; break;
			case NE:if (s != c.stringValues)return 1; break;
			case LE:if (s <= c.stringValues)return 1; break;
			case GE:if (s >= c.stringValues)return 1; break;
			case LT:if (s <  c.stringValues)return 1; break;
			case GT:if (s >  c.stringValues)return 1; break;
			}
			return 0;
		default:
			return 0;
	}
}

/*根据当前偏移量返回下一条偏移*/
int RecordManager::FindNextRecord(Block *block, int posBegin)
{
	int i=0;
	while(block->data[posBegin+i]!=-3&&block->data[posBegin+i]!=-2)i++;
	return posBegin+i+1;
}

/*用空格代替delete*/
void RecordManager::fullblack(Block *block, int posBegin)
{
	int i=0;
	char* p=block->data+posBegin;
	while (p[i] != -3 && p[i] != -2) { if (p[i] != ',')p[i] = 255; i++; }
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
	while(str[num]!=-3&&str[num]!=','&&str[num]!=-2&&str[num]!=0)num++;
	std::string theString(str, num+1);
	return theString;
}

void RecordManager::setString(Block *block, int posBegin, std::string str)
{
	strcpy_s((block->data + posBegin),strlen(str.c_str())+1, str.c_str());
}
