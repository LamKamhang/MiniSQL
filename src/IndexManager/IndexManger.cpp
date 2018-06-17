#include "IndexManager.h"

IndexManager::IndexManager()
{
}

IndexManager::~IndexManager()
{
}

// select
std::vector<TuplePtr> IndexManager::_select(const std::string &indexName, const condition &cond)
{
    switch(cond.type)
    {
        case INT:       return int_select(indexName, cond);
        case FLOAT:     return float_select(indexName, cond);
        case CHAR:      return string_select(indexName, cond);
        default:        return std::vector<TuplePtr>{};
    }
}

std::vector<TuplePtr> IndexManager::int_select(const std::string &indexName, const condition &cond)
{
    if (intTrees.find(indexName) == intTrees.end())
    {
        intTrees[indexName] = BPTree(indexName, DEGREE);
    }

    switch (cond.op)
    {
        case EQ:    return std::vector<TuplePtr>{
                        intTrees[indexName]->find(cond.intValue)
                    };
        case LE:    return intTrees[indexName]->findRange(cond.intValue, LE);
        case GE:    return intTrees[indexName]->findRange(cond.intValue, GE);
        case LT:    return intTrees[indexName]->findRange(cond.intValue, LT);
        case GT:    return intTrees[indexName]->findRange(cond.intValue, GT);
        case NE:    return intTrees[indexName]->findRange(cond.intValue, NE);
        default:    return std::vector<TuplePtr>{};
    }
}

std::vector<TuplePtr> IndexManager::_delete(const std::string &indexName, const condition &cond)
{
    switch(cond.type)
    {
        case INT:       return int_delete(indexName, cond);
        case FLOAT:     return float_delete(indexName, cond);
        case CHAR:      return string_delete(indexName, cond);
        default:        return std::vector<TuplePtr>{};
    }
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
*/
bool IndexManager::_insert(const std::string &indexName, const condition &value, const TuplePtr &valueptr)
{
    switch(value.type)
    {
        case INT:   return int_insert(indexName, value, valueptr);
        case FLOAT: return float_insert(indexName, value, valueptr);
        case CHAR:  return string_insert(indexName, value, valueptr);
        default:    return false;
    }
}

bool IndexManager::_create(const std::string &indexName, const records &values, int index)
{
    switch(values.attributes[index].type)
    {
        case INT:       return int_create(indexName, values.list, index);
        case FLOAT:     return float_create(indexName, values.list, index);
        case CHAR:      return string_create(indexName, values.list, index);
        default:        return false;
    }
}

bool IndexManager::_drop(const std::string &indexName)
{
    if (intTrees.find(indexName) != intTrees.end())
        intTrees.erase(indexName);
    else if (floatTrees.find(indexName) != floatTrees.end())
        floatTrees.erase(indexName);
    else if (stringTrees.find(indexName) != stringTrees.end())
        stringTrees.erase(indexName);
    else
    {
        return false;
    }
    return true;
}