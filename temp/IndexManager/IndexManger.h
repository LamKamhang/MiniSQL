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

#pragma once
#include "../interface.h"
#include "BPTree.hpp"
#include "../BufferManager/BufferManager.h"

#include <string>
#include <vector>
#include <iostream>

using namespace MINISQL_BASE;
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
	// 使用我的同时，需要给我传一个bufferManager的引用
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
	/* 暂时提供一个对于同一个属性的区间搜索，这个能够提高多索引的搜索效率。
	 * 需要提供索引的名字，他对应的左区间值，以及左区间是否为闭区间
	 * 					 他对应的右区间值，以及右区间是否为闭区间
	 * Usage example：需要搜索一个bno(int)的属性值, 他满足5 < bno <= 10
	 * 				  这时候，lvalue的值为5，lclosed为0
	 * 						 rvalue的值为10， rclosed为1
	 * 需要注意的是，我这里的接口设计不是太友好，需要必须从小往大的排列，
	 * 左值小于右值。而且由于condition没有定义一个空值，所以这个接口只能使用确切两个条件的情况下。
 	 */
	bool range_select(const std::string &indexName, const condition &lvalue, int lclosed, const condition &rvalue, int rclosed, std::vector<TuplePtr> &tuplePtrs);
	
	// 其他接口形式待商榷 
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
	 * version 1.2: 
	 */
    bool _delete(const std::string &indexName, const condition &value);

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
	bool GetBPTree(const std::string &indexName, SqlValueType type);

    BPTree<int> *intBPTree;
    BPTree<float> *floatBPTree;
    BPTree<std::string> *stringBPTree;

    BufferManager& bm;
};
