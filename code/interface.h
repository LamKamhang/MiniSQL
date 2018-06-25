/*
 * File: interface.h
 * Version: 1.1
 * Author: kk
 * Modified Date: Fri Jun  1 16:48:36 DST 2018
 * Modified Date: Mon Jun 11 22:37:24 DST 2018
 * -------------------------------------------
 * miniSQL 统一接口头文件
 * 重新定义了一些基础的类：
 *  - 值
 *  - 类型
 *  - 条件
 * 
 * v1.1 规范化头文件，避免重定义
 *      增加元组逻辑指针
 */

#pragma once
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include <string>
#include <Cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <cassert>

namespace MINISQL_BASE {
    // 块的大小以及最大块的数量
    // 阿洪你根据需要修改就好啦
    // 在bufferManager中定义
    // 定义的时候在namespae中定义
    // extern const int BlockSize;
    // extern const int MaxBlocks;

    typedef unsigned int BlockIDType;
    typedef struct TuplePtr OffsetType;

    enum{INT_SIZE = sizeof(int)};
    enum{FLOAT_SIZE = sizeof(float)};
    // enum{TPTR_SIZE = sizeof(MINISQL_BASE::TulpePtr)};//在下面定义了

    #define NONE    (TuplePtr(-1, -1))
    #define NONE_TABLE  (table("", 0))


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

    // MiniStatement type
    enum Mini{
        SYNTAX_ERROR,
        SELECT,
        DELETE,
        INSERT,
        EXEFILE,
        CREATETABLE,
        CREATEINDEX,
        DROPTABLE,
        DROPINDEX,
        CREATEDATABASE,
        DROPDATABASE,
        USEDATABASE,
        QUIT
    };

    /*
    * class: Attribute
    * Usage: 记录属性的信息
    * --------------------
    * 利用类的私有成员记录属性的名字，类型，以及约束(primary key / unique)
    * 通过函数操作进行成员的操纵 
    */
    class attribute
    {
    public:
        std::string name;
        int type;
        int length;
        bool primary;
        bool unique;
        bool index;
        attribute():
            primary(false), unique(false), index(false)
        {};
        ~attribute() {}
        attribute(std::string n, int t, int l):
            name(n), type(t), length(l), 
            primary(false), unique(false), index(false)
        {};
    };


    /*
    * class: table
    * Usage: 记录表格的信息
    * --------------------
    */
    class table
    {
    public:
        std::string tableName;
        int attributeNum;
        int valueNum;
        std::vector<attribute> attributes;
        table() {}
        table(const std::string &tN, int aN):
            tableName(tN), attributeNum(aN), valueNum(0)
        {};
        bool operator ==(const table &rhs)
        {
            return tableName == rhs.tableName;
        }
        bool operator !=(const table &rhs)
        {
            return tableName != rhs.tableName;
        }
    };

    /*
    * class: index
    * Usage: 记录索引的信息
    * --------------------
    */
    class index
    {
    public:
        std::string indexName;
        std::string tableName;
        std::string attributeName;
        index(const std::string &s1, const std::string &s2, const std::string &s3):
            indexName(s1), tableName(s2), attributeName(s3)
        {};
        index() = default;
    };


    /*
    * class: miniCreateTable
    * Usage: 记录索引的信息
    * --------------------
    */
    class miniCreateTable
    {
    public:
        miniCreateTable() {}
        ~miniCreateTable() {}
        std::string tableName;
        int attributeNum;
        attribute attributes[32];
    };

    /*
    * class: miniCreateIndex
    * Usage: 记录索引的信息
    * --------------------
    */
    class miniCreateIndex
    {
    public:
        std::string indexName;
        std::string tableName;
        std::string attributeName;
    };

    /*
    * structure: TuplePtr
    * Usage: 作为一条元组的定位信息
    * --------------------
    * 元组的逻辑指针，
    * 通过该结构能够定位到元组在哪个block
    * 以及在这个block中的偏移量是多少
    */
   struct TuplePtr{
       unsigned int blockID;
       unsigned int offset;
       TuplePtr() = default;
       TuplePtr(unsigned int bid, unsigned offset):
            blockID(bid), offset(offset)
       {
       };
       bool operator ==(const TuplePtr &rhs)
       {
           return blockID == rhs.blockID && offset == rhs.offset;
       }
       bool operator !=(const TuplePtr &rhs)
       {
           return blockID != rhs.blockID || offset != rhs.offset;
       }
       bool operator !()
       {
           return (*this == NONE);
       }
   };
   enum{TPTR_SIZE = sizeof(MINISQL_BASE::TuplePtr)};


    
    /*
    * class: condition
    * Usage: 记录索引的信息
    * --------------------
    */
    class condition
    {
    public:
        std::string attributeName;
        int type;
        Operator oprt;
        int intValue;
        float floatValue;
        std::string stringValues;
    };
}
