/*
 * File: IndexManager.cpp
 * Version: 1.1
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * Modified Date: Thu Jun 21 21:06:52 DST 2018
 * -------------------------------------------
 * miniSQL的IndexManager的类函数实现
 * version 1.0: 基本操作的实现
 * version 1.1: 修改了select 以及 delete的接口，使得能够判断异常状态
 */

IndexManager::IndexManager(BufferManager& bm)
{
    intBPTree = new BPTree<int>(bm);
    floatBPTree = new BPTree<float>(bm);
    stringBPTree = new BPTree<std::string>(bm);
}

IndexManager::~IndexManager()
{
    if(intBPTree->isChanged())
        intBPTree->wiriteBack();
    if(floatBPTree->isChanged())
        floatBPTree->wiriteBack();
    if(stringBPTree->isChanged())
        stringBPTree->wiriteBack();
   
    delete intBPTree;
    delete floatBPTree;
    delete stringBPTree;
}

/*
class condition
{
public:
	string attributeName;
	int type;
	string oprt;
	int intValue;
	float floatValue;
	string stringValues;
};

// 属性的类型/值的类型
enum AttributeType {
    None,
    Integer,
    Float, 
    String,
};
typedef AttributeType SqlValueType;

// 操作符的类型
enum Operator{
    ERROR,
    EQ,
    NE,
    LE,
    GE,
    LT,
    GT,
};
*/
bool IndexManager::_select(const std::string &indexName, const condition &cond, std::vector<TuplePtr> &tuplePtrs)
{
    tuplePtrs.resize(0);
    GetBPTree(indexName, type);
    switch(cond.oprt)
    {
        case EQ:
            switch(type)
            {
                case Integer:   tuplePtrs.push_back(intBPTree->_search(cond.intValue));    break;
                case Float:     tuplePtrs.push_back(floatBPTree->_search(cond.floatValue));  break;
                case String:    tuplePtrs.push_back(stringBPTree->_search(cond.stringValues)); break;
            }
            break;
        case LE:
        case GE:
        case LT:
        case GT:
        case NE:
        default: return false;
    }

    return true;
}

bool IndexManager::_delete(const std::string &indexName, const condition &cond, std::vector<TuplePtr> &tuplePtrs)
{
    bool canDo = _select(indexName, condition, tuplePtrs);
    switch(cond.oprt)
    {
        case EQ:
            switch(type)
            {
                case Integer:   intBPTree->_delete(cond.intValue);    break;
                case Float:     floatBPTree->_delete(cond.floatValue);  break;
                case String:     stringBPTree->_delete(cond.stringValues);break;
            }
            break;
        case LE:
        case GE:
        case LT:
        case GT:
        case NE:
        default: return false;
    }
    return true;
}

// update <table_name> set <attribute> = <new_value> where <conditions>
// bool _update(const std::string &indexName, const condition &oldValue, const condition &newValue, const TuplePtr &newTuplePtr);

/*
class condition
{
public:
	string attributeName;
	int type;
	string oprt;
	int intValue;
	float floatValue;
	string stringValues;
};
*/
bool IndexManager::_insert(const std::string &indexName, const condition &value, const TuplePtr &valueptr)
{
    GetBPTree(indexName, value.type);
    switch(value.type)
    {
        case Integer:   intBPTree->_insert(value.intValue, valueptr);    break;
        case Float:     floatBPTree->_insert(value.floatValue, valueptr);    break;
        case String:    stringBPTree->_insert(value.stringValues, valueptr); break;
        default: return false;
    }
    return true;
}

/*
class records
{
public:
	int attriNum;//属性的个数
	vector<attribute> attributes;//属性信息
	int recordNum;//记录的个数
	vector<miniRecord> list;//记录的数据
}

class miniRecord
{
public:
	condition cond[32];//记录内容 
	int conditionNum;//属性数目 
	int pos;//所在块的偏移 
	int blockNum;//所在块的编号 
}
*/
bool IndexManager::_create(const std::string &indexName, const records &values, int index)
{
    FILE *fp = fopen(indexName.c_str(), "r");
    if (fp != NULL)
    {
        std::cerr << "ERROR: INDEX EXIST!" << std::endl;
        fclose(fp);
        return false;
    }
    fclose(fp);

    SqlValueType type = values.attributes[index].type;
    const std::vector<miniRecord> &_list = values.list;
    switch(type)
    {
        case Integer:   
            intBPTree->createNewFile(indexName, type);  
            for (int i = 0; i < values.recordNum; ++i)
            {
                intBPTree->_insert(_list[i].cond[index].intValue, TuplePtr((unsigned int)(_list[i].blockNum), (unsigned int)(_list[i].pos)));
            }
            break;
        case Float:     
            floatBPTree->createNewFile(indexName, type);
            for (int i = 0; i < values.recordNum; ++i)
            {
                floatBPTree->_insert(_list[i].cond[index].floatValue, TuplePtr((unsigned int)(_list[i].blockNum), (unsigned int)(_list[i].pos)));
            }
            break;
        case String:    
            stringBPTree->createNewFile(indexName, type);
            for (int i = 0; i < values.recordNum; ++i)
            {
                stringBPTree->_insert(_list[i].cond[index].stringValues, TuplePtr((unsigned int)(_list[i].blockNum), (unsigned int)(_list[i].pos)));
            }
            break;
        default: return false;
    }
    return true;
}

bool IndexManager::_drop(const std::string &indexName)
{
    // check whether the index exists.
    FILE* fp = fopen(indexName.c_str(), "r");
    if (fp == NULL)
    {
        std::cerr << "ERROR: INDEX FILE " << indexName << "NOT EXIST!" << std::endl;
        return false;
    }
    fclose(fp);

    // release any block about the file
    bm.DeleteFileBlock(indexName);
    // delete file in the disk!
    remove(filename.c_str());

    // update BPTree info to avoid rewrite into disk. 
    if (intBPTree->indexName == indexName)
        intBPTree->_release();
    else if(floatBPTree->indexName == indexName)
        floatBPTree->_release();
    else if (stringBPTree->indexName == indexName)
        stringBPTree->_release();
    
    return true;
}

// private function.
void IndexManager::GetBPTree(std::string indexName, SqlValueType type)
{  
    switch(type)
    {
        case Integer:   intBPTree->readFromFile(indexName); break;
        case Float:     floatBPTree->readFromFile(indexName);break;
        case String:    stringBPTree->readFromFile(indexName);break;
        default: return false;
    }
    return true;
}