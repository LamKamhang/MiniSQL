/*
 * File: IndexManager.h
 * Version: 1.2
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * Modified Date: Thu Jun 21 21:06:52 DST 2018
 * Modified Date: Fri Jun 22 20:41:49 DST 2018
 * -------------------------------------------
 * miniSQL的IndexManager的类头文件声明
 * version 1.0: 基本操作的实现
 * version 1.1: 修改了select 以及 delete的接口，使得能够判断异常状态
 * version 1.2: 再次修正delete的接口，避免提前删除数据
 */

/*
 * 这样的设计是想尽可能减少indexManager的申请，
 * 在运行期间只定义一次indexmanager变量
 * 也就是在API的private中定义，API只定义一次
 * 这样的好处是：减小操作增加对构造和析构的开销
 * 更好的操作是BPTree做一个map，但这样对内存大小控制较难
 */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include "IndexManager.h"
IndexManager::IndexManager(BufferManager& bm):
    bm(bm)
{
    intBPTree = new BPTree<int>(bm);
    floatBPTree = new BPTree<float>(bm);
    stringBPTree = new BPTree<std::string>(bm);
}

IndexManager::~IndexManager()
{
    // 析构自动写
    if(intBPTree->isChanged())
        intBPTree->writeBack();
    if(floatBPTree->isChanged())
        floatBPTree->writeBack();
    if(stringBPTree->isChanged())
        stringBPTree->writeBack();
   
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
    tuplePtrs.clear();
    int type = cond.type;
    TuplePtr tmp;
    GetBPTree(indexName, type);
    switch(cond.oprt)
    {
        case EQ:
            switch(type)
            {
                case Integer:   tmp = intBPTree->_search(cond.intValue);    
                std::cout << cond.intValue << std::endl;
                break;
                case Float:     tmp = floatBPTree->_search(cond.floatValue);  break;
                case String:    tmp = stringBPTree->_search(cond.stringValues); break;
            }         
            if (tmp != NONE)
                tuplePtrs.push_back(tmp);
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

bool IndexManager::_delete(const std::string &indexName, const condition &value)
{
    bool res;
     GetBPTree(indexName, value.type);
     switch(value.type)
    {
        case Integer:   res = intBPTree->_delete(value.intValue);    break;
        case Float:     res = floatBPTree->_delete(value.floatValue);    break;
        case String:    res = stringBPTree->_delete(value.stringValues); break;
        default: res = false;
    }
    return res;
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
    bool res;
    GetBPTree(indexName, value.type);
    switch(value.type)
    {
        case Integer:   res = intBPTree->_insert(value.intValue, valueptr);    break;
        case Float:     res = floatBPTree->_insert(value.floatValue, valueptr);    break;
        case String:    res = stringBPTree->_insert(value.stringValues, valueptr); break;
        default: res = false;
    }
    return res;
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

class attribute
{
    string name;
	int type;
	int length;
	bool primary;
	bool unique;
	bool index;
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

    int type = values.attributes[index].type;
    int size = values.attributes[index].length;
    const std::vector<miniRecord> &_list = values.record_list;
    
    switch(type)
    {
        case Integer:   
            intBPTree->_create(indexName, size);  
            for (int i = 0; i < values.recordNum; ++i)
            {
                intBPTree->_insert(_list[i].cond[index].intValue, TuplePtr((unsigned int)(_list[i].blockNum), (unsigned int)(_list[i].pos)));
            }
            break;
        case Float:     
            floatBPTree->_create(indexName, size);
            for (int i = 0; i < values.recordNum; ++i)
            {
                floatBPTree->_insert(_list[i].cond[index].floatValue, TuplePtr((unsigned int)(_list[i].blockNum), (unsigned int)(_list[i].pos)));
            }
            break;
        case String:    
            stringBPTree->_create(indexName, size);
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

    // update BPTree info to avoid rewrite into disk. 
    if (intBPTree->indexName == indexName)
        intBPTree->_drop();
    else if(floatBPTree->indexName == indexName)
        floatBPTree->_drop();
    else if (stringBPTree->indexName == indexName)
        stringBPTree->_drop();
    
    return true;
}

// private function.
bool IndexManager::GetBPTree(const std::string &indexName, int type)
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