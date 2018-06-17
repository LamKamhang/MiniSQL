#pragma once
#include "../interface.h"
#include "BPTree.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <map>

class IndexManager
{
public:
    IndexManager();
    IndexManager(IndexManager &&) = default;
    IndexManager(const IndexManager &) = default;
    ~IndexManager();

    std::vector<TuplePtr> _select(const std::string &indexName, const condition &cond);

    std::vector<TuplePtr> _delete(const std::string &indexName, const condition &cond);

    bool _insert(const std::string &indexName, const condition &value, const TuplePtr &valueptr);

    bool _create(const std::string &indexName, const records &values, int index);
    
    bool _drop(const std::string &indexName);
private:
    std::vector<TuplePtr> int_select    (const std::string &indexName, const condition &cond);
    std::vector<TuplePtr> float_select  (const std::string &indexName, const condition &cond);
    std::vector<TuplePtr> string_select   (const std::string &indexName, const condition &cond);

    std::vector<TuplePtr> int_delete    (const std::string &indexName, const condition &cond);
    std::vector<TuplePtr> float_delete  (const std::string &indexName, const condition &cond);
    std::vector<TuplePtr> string_delete   (const std::string &indexName, const condition &cond);    

    typedef std::map<std::string, BPTree<int> *>            IntTrees;
    typedef std::map<std::string, BPTree<float> *>          FloatTrees;
    typedef std::map<std::string, BPTree<std::string> *>    StringTrees;

    IntTrees    intTrees;
    FloatTrees  floatTrees;
    StringTrees   stringTrees;

    enum {DEGREE = 200};
};