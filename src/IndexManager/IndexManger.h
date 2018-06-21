/*
 * File: IndexManager.h
 * Version: 1.1
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * Modified Date: Thu Jun 21 21:06:52 DST 2018
 * -------------------------------------------
 * miniSQL的IndexManager的类头文件声明
 * version 1.0: 基本操作的实现
 * version 1.1: 修改了select 以及 delete的接口，使得能够判断异常状态
 */

#pragma once
#include "../interface.h"
#include "BPTree.hpp"
#include "../BufferManager/BufferManager.h"

#include <string>
#include <vector>
#include <iostream>

/*
 * class: IndexManager
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * -------------------------------------------
 * IndexManager类声明
 */
class IndexManager
{
private:
    // 不允许拷贝构造
    IndexManager(IndexManager &&) = default;
    IndexManager(const IndexManager &) = default;
public:
    IndexManager(BufferManager& bm);
    ~IndexManager();

    /*
	 * Function: _select
	 * Argument: first arg: indexName.
	 *			 second arg: the condition, but just one condition.
	 *		     third arg: return ptrs for record.
	 * Version: 1.1
	 * Author: kk
	 * Created Date: Sat Jun 16 01:07:47 DST 2018
	 * Modified Date: Thu Jun 21 21:06:52 DST 2018
	 * -------------------------------------------
	 * miniSQL的IndexManager的类函数实现
	 * version 1.1: 修改为返回值是bool型，将所需返回的TuplePtr放在了最后作为引用变量
	 */
    bool _select(const std::string &indexName, const condition &cond, std::vector<TuplePtr> &tuplePtrs);

    // 区间搜索的接口定义有点麻烦，暂定
    //bool range_select(const std::string &indexName, ...)
    
    /*
	 * Function: _delete
	 * Argument: first arg: indexName.
	 *			 second arg: the condition, but just one condition.
	 *		     third arg: return ptrs for record.
	 * Version: 1.1
	 * Author: kk
	 * Created Date: Sat Jun 16 01:07:47 DST 2018
	 * Modified Date: Thu Jun 21 21:06:52 DST 2018
	 * -------------------------------------------
	 * miniSQL的IndexManager的类函数实现
	 * version 1.1: 修改为返回值是bool型，将所需返回的TuplePtr放在了最后作为引用变量
	 */
    bool _delete(const std::string &indexName, const condition &cond, std::vector<TuplePtr> &tuplePtrs);

    // update <table_name> set <attribute> = <new_value> where <conditions>
    // bool _update(const std::string &indexName, const condition &oldValue, const condition &newValue, const TuplePtr &newTuplePtr);

    /*
	 * Function: _insert
	 * Argument: first arg: indexName.
	 *			 second arg: this is a value(...why not use another type...too strange).
	 *		     third arg: the tupleptr belongs to the value
	 * Version: 1.0
	 * Author: kk
	 * Created Date: Sat Jun 16 01:07:47 DST 2018
	 * -------------------------------------------
	 * miniSQL的IndexManager的类函数实现
	 */
    bool _insert(const std::string &indexName, const condition &value, const TuplePtr &valueptr);

    /*
	 * Function: _create
	 * Argument: first arg: indexName.
	 *			 second arg: these are the records in the whole table.
	 *		     third arg: this is the index of the index_attribute.
	 *			the type records is too complicated... just look up in the cpp... amazing!!!
	 * Version: 1.0
	 * Author: kk
	 * Created Date: Sat Jun 16 01:07:47 DST 2018
	 * -------------------------------------------
	 * miniSQL的IndexManager的类函数实现
	 */
    bool _create(const std::string &indexName, const records &values, int index);
 
    /*
	 * Function: _drop
	 * Argument: first arg: indexName.
	 * Version: 1.0
	 * Author: kk
	 * Created Date: Sat Jun 16 01:07:47 DST 2018
	 * -------------------------------------------
	 * miniSQL的IndexManager的类函数实现
	 */   
    bool _drop(const std::string &indexName);
private:
    /*
	 * Function: GetBPTree
	 * Argument: first arg: indexName.
	 *			 second arg: the BPtree's type
	 * Version: 1.0
	 * Author: kk
	 * Created Date: Sat Jun 16 01:07:47 DST 2018
	 * -------------------------------------------
	 * 内部函数，用来更新im当前的私有成员变量
	 */ 
    void GetBPTree(std::string indexName, SqlValueType type);

    BPTree<int> *intBPTree;
    BPTree<float> *floatBPTree;
    BPTree<std::string> *stringBPTree;

    BufferManager& bm;
};
