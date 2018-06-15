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

#include <string>
#include <Cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>

namespace MINISQL_BASE {
    // 块的大小以及最大块的数量
    // 阿洪你根据需要修改就好啦
    // 在bufferManager中定义
    // 定义的时候在namespae中定义
    extern const int BlockSize;
    extern const int MaxBlocks;
#define NONE    (TuplePtr(-1, -1))

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

    /*
    * class: Attribute
    * Usage: 记录属性的信息
    * --------------------
    * 利用类的私有成员记录属性的名字，类型，以及约束(primary key / unique)
    * 通过函数操作进行成员的操纵 
    */
    class Attribute {
        private:
            std::string name;
            unsigned int  charSize;   // this is the variable n of char(n)
            AttributeType type;
            bool primary;
            bool unique;
        public:
            // constructors and destructors
            Attribute():
                primary(false), unique(false), type(None)
            {
            };
            Attribute(std::string name, AttributeType type, unsigned int n = 0, bool primary = false, bool unique = false):
                name(name), type(type), charSize(n), primary(primary), unique(unique)
            {
                if (primary)
                    unique = true;
            }
            ~Attribute(){};

            // set some inner info.
            void setType(const AttributeType &type, const unsigned int charSize = 0) 
                {this->type = type; this->charSize = charSize;}
            void setName(const std::string &name)   {this->name = name;}
            void setPrimary(const bool &primary)    {if (this->primary = primary) unique = true;}
            void setUnique(const bool &unique)      {this->unique = unique;}

            // some methods to get inner info.
            AttributeType getType()     {return type;}
            unsigned int  getSize()     {return charSize;}
            const std::string& getName(){return name;}
            bool isPrimary()            {return primary;}
            bool isUnique()             {return unique;}
    };

    /*
    * class: SqlValue
    * Usage: 记录值的信息
    * --------------------
    * 利用类的统一不同类型的值
    * 利用统一接口实现关系操作
    */
    class SqlValue {
        private:
            std::string str;
            int i;
            float f;
            SqlValueType type;
            unsigned int charSize;

        public:
            // constructors and destructors
            SqlValue()                :           type(None)    {};
            SqlValue(int i)           : i(i),     type(Integer) {};
            SqlValue(float f)         : f(f),     type(Float)   {};
            SqlValue(std::string &str, unsigned int charSize): str(str), type(String), charSize(charSize) 
                {if(str.size() > charSize)  throw std::runtime_error("The str name size is overflow!");};
            SqlValue(const char *str,  unsigned int charSize) : str(str), type(String), charSize(charSize) 
                {if(strlen(str) > charSize) throw std::runtime_error("The str name size is overflow!");};
            ~SqlValue(){};

            // get sqlvalue info.
            const std::string getStr() 
            {
                if (type == String)
                    return str;
                else
                    throw std::runtime_error("The Value type is not String");
            }
            int getInteger()
            {
                if (type == Integer)
                    return i;
                else
                    throw std::runtime_error("The Value type is not Integer");
            }
            float getFloat()
            {
                if (type == Float)
                    return f;
                else
                    throw std::runtime_error("The Value type is not Float");
            }
            SqlValueType getType()
            {
                return type;
            }

            // set sqlValue info.
            void setStr(std::string &str, unsigned int charSize)
            {
                this->str = str;
                type = String;
                this->charSize = charSize;
                if (str.size() > charSize)
                    throw std::runtime_error("The str name size is overflow!");
            }
            void setStr(const char *str, unsigned int charSize)
            {
                this->str = std::string(str);
                type = String;
                this->charSize = charSize;
                if (strlen(str) > charSize)
                    throw std::runtime_error("The str name size is overflow!");
            }
            void setInteger(int i)
            {
                this->i = i;
                type = Integer;
            }
            void setFloat(float f)
            {
                this->f = f;
                type = Float;
            }

            // tool function
            void reset()
            {
                str.clear();
                i = 0;
                f = 0;
                charSize = 0;
                type = None;
            }
            std::string toStr() const
            {
                std::stringstream ss;
                switch(type)
                {
                    case String:    ss << str;  break;
                    case Integer:   ss << i;    break;
                    case Float:     ss << f;    break;
                    default: throw std::runtime_error("Undefined Type!");break;
                }
                return ss.str();
            }

            // relation operation
            bool operator <(const SqlValue &rhs) const
            {
                switch(type)
                {
                    case String:    return str < rhs.str;
                    case Integer:   return i < rhs.i;
                    case Float:     return f < rhs.f;
                    default: throw std::runtime_error("Undefined Type!");
                }
            }
            bool operator >(const SqlValue &rhs) const
            {
                switch(type)
                {
                    case String:    return str > rhs.str;
                    case Integer:   return i > rhs.i;
                    case Float:     return f > rhs.f;
                   default: throw std::runtime_error("Undefined Type!");
                }
            }
            bool operator ==(const SqlValue &rhs) const
            {
                switch(type)
                {
                    case String:    return str == rhs.str;
                    case Integer:   return i == rhs.i;
                    case Float:     return f == rhs.f;
                    default: throw std::runtime_error("Undefined Type!");
                }
            }
            bool operator !=(const SqlValue &rhs) const
            {
                switch(type)
                {
                    case String:    return str != rhs.str;
                    case Integer:   return i != rhs.i;
                    case Float:     return f != rhs.f;
                   default: throw std::runtime_error("Undefined Type!");
                }
            }
            bool operator <=(const SqlValue &rhs) const
            {
                switch(type)
                {
                    case String:    return str <= rhs.str;
                    case Integer:   return i <= rhs.i;
                    case Float:     return f <= rhs.f;
                    default: throw std::runtime_error("Undefined Type!");
                }
            }
            bool operator >=(const SqlValue &rhs) const
            {
                switch(type)
                {
                    case String:    return str >= rhs.str;
                    case Integer:   return i >= rhs.i;
                    case Float:     return f >= rhs.f;
                    default: throw std::runtime_error("Undefined Type!");
                }
            }
    };

    /*
    * class: Condition
    * Usage: 记录条件信息
    * --------------------
    * 利用类的统一不同条件
    */
    class Condition {
        private:
            std::string attr;
            Operator op;
            SqlValue val;
        public:
            // constructor and destructor.
            Condition() = default;
            Condition(const std::string &attr, const Operator op, const SqlValue &val):
                attr(attr), op(op), val(val)
            {
            };
            ~Condition(){};

            bool check(const SqlValue &lhs) const
            {
                switch(op)
                {
                    case EQ:return lhs == val;
                    case NE:return lhs != val;
                    case LE:return lhs <= val;
                    case GE:return lhs >= val;
                    case LT:return lhs <  val;
                    case GT:return lhs >  val;
                    default: throw std::runtime_error("CONDITION TYPE ERROR!");
                }
            }
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
       bool operator !()
       {
           return (*this == NONE);
       }
   };
}
