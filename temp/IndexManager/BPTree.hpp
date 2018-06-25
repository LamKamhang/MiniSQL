/*
 * File: BPTree.hpp
 * Version: 1.5
 * Author: kk
 * Created Date: Sat Jun  2 20:04:21 DST 2018
 * Modified Date: Mon Jun 11 22:37:24 DST 2018
 * Modified Date: Fri Jun 15 21:10:18 DST 2018
 * Modified Date: Sat Jun 16 01:07:47 DST 2018
 * Modified Date: Tue Jun 19 16:53:17 DST 2018
 * Modified Date: Sat Jun 23 17:21:37 DST 2018
 * -------------------------------------------
 * miniSQL的IndexManager需要用到的数据结构B+树定义
 * 实现B+树的基本操作，插入，删除，合并，分裂
 * 为了通用性，使用模板编程方式。
 * version 1.2 去除Union声明，联合中不能使用类，需要保持对内存相同的操作
 * version 1.3 修正inner node的split方式，上浮的key不保存在inner node中
 * version 1.4 无法修正remove的bug，对BPTree进行重构，
 *             结合bufferManager进行磁盘读写
 * version 1.5 修正了删除最后一个结点时，index头信息的丢失bug
 */

#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>
#include <iterator>

#include "../BufferManager/BufferManager.h"
#include "../interface.h"

using namespace MINISQL_BASE;

#define _cptr2float(x)  	(*(float*)(x))
#define _cptr2int(x)  		(*(int*)(x))
#define _cptr2uint(x) 		(*(unsigned int*)(x))
#define _data2cptr(x)  		((char*)(&x))

/*
 * class: IndexHead
 * Version: 1.0
 * Author: kk
 * Created Date: Tue Jun 19 16:53:17 DST 2018
 * ------------------------------------------
 * 存放BP树索引重要信息，作为索引头存放在block0的
 * 位置，读入文件的时候，首先读取block 0，读取BPTree
 * 信息，获取root和第一个叶子结点的bid
 */
class IndexHead
{
public:
    IndexHead(Block* pHead):
        pEmptyId    (pHead->data), 
        pNodeNum    (pHead->data+BLOCKSIZE-INT_SIZE*1),
        pRoot       (pHead->data+BLOCKSIZE-INT_SIZE*2), 
        pFirstLeaf  (pHead->data+BLOCKSIZE-INT_SIZE*3), 
        pKeySize    (pHead->data+BLOCKSIZE-INT_SIZE*4), 
        pEmptyNum   (pHead->data+BLOCKSIZE-INT_SIZE*5)
    {};

    // 从对应的指针中读取内容
    int getRoot()       {return _cptr2int(pRoot);};
    int getNodeNum()    {return _cptr2int(pNodeNum);};
    int getFirstLeaf()  {return _cptr2int(pFirstLeaf);};
    int getKeySize()    {return _cptr2int(pKeySize);};
    
    // 把内容拷贝到heap上指针的所指内容中
    void setRoot(int p)     {memcpy(pRoot,      _data2cptr(p), INT_SIZE);};
    void setNodeNum(int n)  {memcpy(pNodeNum,   _data2cptr(n), INT_SIZE);};
    void setFirstLeaf(int l){memcpy(pFirstLeaf, _data2cptr(l), INT_SIZE);};
    void setKeySize(int k)  {memcpy(pKeySize,   _data2cptr(k), INT_SIZE);};

    // 获取空快链，（删除结点的碎片）
    void getEmptyBlockList(list<int>& emptyList)
    {
        emptyList.clear();
        int EmptyNum = getEmptyNum();
        char *p = pEmptyId;
        for (int i = 0; i < EmptyNum; ++i)
        {
            emptyList.push_back(_cptr2int(p));
            p += INT_SIZE;
        }
    }

    // 清空空快链表
    void setEmptyBlockList(const list<int>& emptyList)
    {
        setEmptyNum(emptyList.size());
        char *p = pEmptyId;
        for (auto it = emptyList.begin(); it != emptyList.end(); ++it)
        {
            memcpy(p, _data2cptr(*it), INT_SIZE);
            p += INT_SIZE;
        }
    }

private:
    // 第一个空快的下标
    char* pEmptyId;

    // 存放到disk的重要信息，用于重建BPTree
    char* pNodeNum;
    char* pRoot;
    char* pFirstLeaf;
    char* pKeySize;

    // 为了方便获取空块链表，记录的空块数量
    char* pEmptyNum;

    // 私有函数，只能有public接口的函数调用去设置空块数量
    void setEmptyNum(int n){memcpy(pEmptyNum, _data2cptr(n), INT_SIZE);};
    int  getEmptyNum(){return _cptr2int(pEmptyNum);};
};

/*
 * class: TreeNode
 * Version: 1.5
 * Author: kk
 * Created Date: Sat Jun  2 20:04:21 DST 2018
 * Modified Date: Mon Jun 11 22:37:24 DST 2018
 * Modified Date: Fri Jun 15 21:10:18 DST 2018
 * Modified Date: Sat Jun 16 01:07:47 DST 2018
 * Modified Date: Tue Jun 19 16:53:17 DST 2018
 * Modified Date: Sat Jun 23 17:21:37 DST 2018
 * ------------------------------------------
 * B+树的结点类，节点类设计比较开放，其的数据都由
 * BPTree进行撰写，预留一个public的成员变量，让
 * BPTree进行设置
 * 需要记录该结点的id，以及父结点，sibling结点的id
 */
class TreeNode
{
public:
    // ignore the class of BPTree, left it to BPTree.
    char* pData;        // for BPTree to move, so it's funciton like stack pointer.
    BlockIDType id;

public:
    TreeNode():
        pData(NULL), id(-1),
        pCount(NULL), pParent(NULL), pIsLeaf(NULL), head(NULL)        
    {};

    TreeNode(Block* pBlock):
        pData(pBlock->data), id(pBlock->Offset),
        pCount (pBlock->data+BLOCKSIZE-INT_SIZE*1),
        pParent(pBlock->data+BLOCKSIZE-INT_SIZE*2),
        pIsLeaf(pBlock->data+BLOCKSIZE-INT_SIZE*3),
        head(pData)
    {};

    void attachTo(Block* pBlock)
    {
        pData   = pBlock->data;
        id      = pBlock->Offset;
        pCount  = pBlock->data + BLOCKSIZE - INT_SIZE*1;
        pParent = pBlock->data + BLOCKSIZE - INT_SIZE*2;
        pIsLeaf = pBlock->data + BLOCKSIZE - INT_SIZE*3;
        head    = pData;
    }

    // inline funciton, to avoid redefiniiton
    inline void reset()             {pData = head;};
    inline int getCount()           {return _cptr2int(pCount);};
    inline BlockIDType getParent()  {return _cptr2uint(pParent);};
    inline int isLeaf()             {return _cptr2int(pIsLeaf);};

    inline void setCount(int c)             {memcpy(pCount,_data2cptr(c),INT_SIZE);};
    inline void setParent(BlockIDType pid)  {memcpy(pParent,_data2cptr(pid),INT_SIZE);};
    inline void setLeaf(int k)              {memcpy(pIsLeaf,_data2cptr(k),INT_SIZE);};

    inline void increaseCount(int k){int c = getCount();c+=k;setCount(c);};
    inline void decreaseCount(int k){int c = getCount();c-=k;setCount(c);};
private:
    char* pCount;       // how many keys in the node
    char* pParent;      // the block id of parent(if there no parent, id is 0, since block 0 is for indexHead)
    char* pIsLeaf;      // the block id of sibling(0 means the last leaf, and -something means it is an inner node)

    char* head;         // this is the pointer head of the pData.
};

/*
 * class: NodeSearchParse
 * Version: 1.0
 * Author: kk
 * Created Date: Tue Jun 19 16:53:17 DST 2018
 * ------------------------------------------
 * 搜索时需要的暂存结点搜索信息,
 * tupleOffset是相当于之前B+树的指针，现在改为
 * bid和offset的结构
 * offsetInNode则是搜索值在所在node的偏移量
 * blockOffset是所在块
 * indexName则是所在哪棵B+树上
 * iffound是表明是否找到
 */
struct NodeSearchParse
{
    OffsetType tupleOffset;
    unsigned int offsetInNode;
    BlockIDType blockOffset;
    string indexName;
    bool isFound;

    void printNSP(void)
    {
        if(!isFound)
        {
            std::cout<<"Not Found"<<std::endl;;
            return;
        }
        std::cout<<"---------------------"<<std::endl;
        std::cout<<"IndexName: "<<indexName<<std::endl;
        std::cout<<"Block Offset: "<<blockOffset<<std::endl;
        std::cout<<"Tupple Offset: "<<tupleOffset.blockID;
        std::cout <<" " << tupleOffset.offset <<std::endl;
        std::cout<<"Offset in Node: "<<offsetInNode<<std::endl;
        std::cout<<"---------------------"<<std::endl;
    }
};

/*
 * class: BPTree
 * Version: 1.5
 * Author: kk
 * Created Date: Sat Jun  2 20:04:21 DST 2018
 * Modified Date: Mon Jun 11 22:37:24 DST 2018
 * Modified Date: Fri Jun 15 21:10:18 DST 2018
 * Modified Date: Sat Jun 16 01:07:47 DST 2018
 * Modified Date: Tue Jun 19 16:53:17 DST 2018
 * Modified Date: Sat Jun 23 17:21:37 DST 2018
 * -------------------------------------------
 * 模板B+树，version 1.3增添了Buffermanager的接口
 * 但降低了自由度。
 * version 1.5: 修正了一些delete的bug
 */
template <typename T>
class BPTree 
{
public:
    std::string indexName;  // coresponding to the file name
private:
    BufferManager& bm;      
    int degree;             // Degree of node, do not need to write into disk.
    BlockIDType root;       // need to write into disk.[#]
    BlockIDType firstLeaf;  // need to write into disk.[#]

    int keySize;            // need to write into disk.[#]
    int indexSize;          // can calculate by keysize and offsetsize(tupleptrsize)
    int nodeNum;            // need to write into disk.[#]
    bool Changed;           // check whether the (need_write_variable) need to write into disk.
public:
    // constructor and destructor
    // this constructor is for test, manmal to adjust the degree.
    BPTree(const std::string &indexName, const int keySize, 
           BufferManager& bm,  int nodeDegree = 0);

    // the normal interface.
    BPTree(BufferManager& bm);

    ~BPTree();

    /* Inline function */    
    inline BlockIDType getRoot()        {return root;}; 
    inline BlockIDType getFirstLeaf()   {return firstLeaf;};
    inline bool isChanged()             {return Changed;};

    // some basic funciton for using BPTree.

    // create a new BPTree and create its corresponding file.
    void _create(const std::string &filename, int keySize);

    // delete one key in the bptree
    bool _delete(const T &oldkey);

    // insert one key into a bptree, but need one more info --> its tuple virtue pointer.
    bool _insert(const T &newkey, const TuplePtr &newtupleptr);

    // modify some key, simply combind delete and insert.
    bool _modify(const T &oldkey, const T &newkey, const TuplePtr &newtupleptr);
    
    // search for the key's corresponding tuple virtue pointer.
    // notice that, TuplePtr is equal to NONE means there no such key.
    TuplePtr _search(const T &key);

    // find the key in some block.
    TuplePtr block_search(Block* pBlock, const T &key);

    // have some problems.
    bool range_search(const T lvalue, const int lclosed, 
                      const T rvalue, const int rclosed,
                      std::vector<TuplePtr>& tuplePtrs);

    // delete the bptree and remove the file in file system.
    bool _drop(void);

    // undo and unbind the BPTree.
    bool _release(void);

    // initial the BPTree.
    void init(void);

    // write the BPTree info into block.
    void writeBack();

    // get BPtree from file.
    void readFromFile(const std::string &filename);

private:
    // fragment block, for efficient operatoration.
    list<int> emptyList;

    Block* createNewNode(void);

    bool insert_in_leaf(Block* pBlock, const T newkey, const TuplePtr &newtupleptr);

    bool insert_in_internal(Block* pBlock, const T newkey, const BlockIDType bidLeft, const BlockIDType bidRight);

    void adjust_after_delete(Block* pNode, char* delPos);

    void adjust_parent_info(BlockIDType id, BlockIDType newp);

    Block* getKeyNode(T key);

    int search_son_offset(TreeNode& parent, const BlockIDType id);
    bool parse_in_block(Block* pBlock, T key, NodeSearchParse& res);

    inline T getKey(char* p);

    inline void setKey(char* p, T newkey);

    OffsetType getOffset(char* p);

    void setOffset(char* p, OffsetType offset);   


#ifdef OUTPUT
public:
    void printNode(Block* pBlock)
    {
        TreeNode a(pBlock);
        if(a.isLeaf()>=0){
            for(int i=0;i<a.getCount();a.pData+=indexSize,i++)
            {
                T key =  getKey(a.pData);
                std::cout<<key<<" ";
            }
            std::cout<<std::endl;
        }
        else
        {
            a.pData+=TPTR_SIZE;
            for(int i=0;i<a.getCount();a.pData+=indexSize,i++)
            {
                T key = getKey(a.pData);
                std::cout<<key<<" ";
            }
            std::cout<<std::endl;
        }
    };
    

    void showLeaf(void)
    {
        TreeNode a;
        int nextLeaf =firstLeaf;
        do{
            Block* pb = bm.GetBlock(indexName, nextLeaf);
            a.attachTo(pb);
            std::cout<<"[";
            for(int i=0;i<a.getCount();a.pData+=indexSize,i++)
            {
                T key =  getKey(a.pData);
                std::cout<<key<<" ";
            }
            std::cout<<"] ";
            nextLeaf = a.isLeaf();
        }while(a.isLeaf()>0);
        std::cout<<"\n"<<std::endl;
        
    };

    void showEmpty()
    {
        list<int>::iterator it;
        for(it=emptyList.begin();it!=emptyList.end();it++)
        {
            std::cout<<*it<<" ";
        }
        std::cout<<std::endl;
    };
#endif

};


/***********************************************************************
 ************************funtion specialization*************************
 ***********************************************************************/

template <>
inline float BPTree<float>::getKey(char *p)
{ return _cptr2float(p); }

template <>
inline int BPTree<int>::getKey(char *p)
{ return _cptr2int(p); }

template <>
inline std::string BPTree<std::string>::getKey(char *p)
{ return std::string(p); }


template <typename T>
inline OffsetType BPTree<T>::getOffset(char* p)
{ return *(OffsetType*)p; }


template <typename T>
inline void BPTree<T>::setKey(char* p,T newkey)
{ memcpy(p, _data2cptr(newkey), keySize); }
    
template <>
inline void BPTree<std::string>::setKey(char *p, std::string newkey)
{ memcpy(p, newkey.c_str(), keySize); }


template <typename T>
inline void BPTree<T>::setOffset(char* p, OffsetType newtupleptr)
{ memcpy(p,_data2cptr(newtupleptr), TPTR_SIZE); }





/***********************************************************************
 ************************BPTree Implementation**************************
 ***********************************************************************/


template <typename T>
BPTree<T>::~BPTree()
{
    if (Changed)
        writeBack();
}


template <typename T>
BPTree<T>::BPTree(BufferManager& bm):
    indexName(""), bm(bm), Changed(false)
{
}

/* Used for test */
/*
 * function: BPTree
 * Version: 1.5
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 模板B+树，version 1.3增添了Buffermanager的接口
 * 但降低了自由度。
 * version 1.5: 修正了一些delete的bug
 * 最大的degree设为(BLOCKSIZE-TPTR_SIZE-INT_SIZE*3)/(indexSize) - 2
 * 一个block里首先除了包含正常的数据外，
 * 还包含了root, firstleaf, nodenum以及keysize和emptynum的信息， 
 * 最后-2则是为了split
 */
template <typename T>
BPTree<T>::BPTree(const std::string &indexName, const int keySize, 
                  BufferManager& bm, 
                  int nodeDegree):
    indexName(indexName), bm(bm), degree(nodeDegree), 
    root(1), firstLeaf(1), keySize(keySize), 
    indexSize(keySize+TPTR_SIZE), nodeNum(1), Changed(false)
{
    FILE* fp = fopen(indexName.c_str(), "w");
    if(fp==NULL)
    {
        std::cerr << "ERROR::CANNOT OPEN FILE!" << std::endl;
        this->indexName = "";
        return;
    }
    fclose(fp);

    if (degree == 0)
    {
        degree = (BLOCKSIZE-TPTR_SIZE-INT_SIZE*3)/(indexSize) - 2;
    }

    // 往block里写入BPTree的信息，让Buffermanager进行写入disk
    Block* pHead = bm.GetNewBlock(indexName, 0);
    bm.SetWritten(pHead);

    IndexHead head(pHead);
    
    head.setNodeNum(nodeNum);      //NodeNumber
    head.setRoot(root);             //root node id
    head.setFirstLeaf(firstLeaf);    //first Leaf id
    head.setKeySize(keySize);

    emptyList.clear();

    head.setEmptyBlockList(emptyList);
    
    // root node.
    Block* pBlock = bm.GetNewBlock(indexName, 1);
    bm.SetWritten(pBlock);
    TreeNode node(pBlock);
    node.setCount(0);   // 当前没有key值
    node.setParent(0);  // 没有父节点（root）
    node.setLeaf(0);    // 非负数表示叶子结点，其中0表示最后一个叶子结点
    Changed = false;
}

/*
 * function: getKeyNode
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 根据key的值返回对应的block进行下一步分析.
 */
template <typename T>
Block* BPTree<T>::getKeyNode(T key)
{
    // search from root.
    Block* pBlock = bm.GetBlock(indexName, root);
    if(pBlock==NULL)
    {
        std::cerr<<"ERROR::BUFFER CANNOT READ!"<<std::endl;
        exit(-1);
    }
    bm.SetWritten(pBlock);

    TreeNode node(pBlock);
    int bid = root;
    while(node.isLeaf()<0)
    {
        node.pData += TPTR_SIZE;   //Move to first key
        for(int i=node.getCount();i>0;node.pData += indexSize,i--)
        {
            if(getKey(node.pData)>key)
            {
                break;
            }
        }
        node.pData -= TPTR_SIZE;
        bid = getOffset(node.pData).blockID;
        pBlock = bm.GetBlock(indexName, bid);
        bm.SetWritten(pBlock);
        node.attachTo(pBlock);
    } 
    return pBlock;
}


/*
 * function: insert_in_leaf
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 在叶子结点中插入
 */
template <typename T>
bool BPTree<T>::insert_in_leaf(Block* pBlock, const T newkey,const TuplePtr &newtupleptr)
{
    TreeNode node(pBlock);
    bm.SetWritten(pBlock);

    // 指向最后一个key_index
    char* p = node.pData+(node.getCount()-1) * indexSize;
    while(p>=node.pData)
    {
        if(getKey(p) < newkey)
        {
            break;
        }
        else if(getKey(p) == newkey)
        {
            std::cerr << "ERROR::KEY IS NOT UNIQUE!" << std::endl;
            return false;
        }
        memcpy(p+indexSize,p,indexSize);
        p-=indexSize;
    }
    p+=indexSize;
    setKey(p, newkey);
    setOffset(p+keySize, newtupleptr);

    node.increaseCount(1);

#ifdef OUTPUT
    std::cout<<"Insert In Leaf "<<pBlock->Offset<<": "<<newkey<<std::endl;
    printNode(pBlock);
#endif
    return true;
}

/*
 * function: insert_in_internal
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 在内部结点插入，与叶子结点插入的不同是先跳过第一个Offset
 */
template <typename T>
bool BPTree<T>::insert_in_internal(Block* pBlock,T newkey,BlockIDType bidLeft,BlockIDType bidRight)
{
    TreeNode node(pBlock);
    bm.SetWritten(pBlock);

    // 指向最后一个key
    char* p = node.pData+(node.getCount()-1) * indexSize+TPTR_SIZE;
    // 开始的第一个key
    char* end = node.pData+TPTR_SIZE;
    while(p>=end)
    {
        if(getKey(p) <= newkey)
        {
            break;
        }
        memcpy(p+indexSize,p,indexSize);
        p-=indexSize;
    }
    p+=indexSize;
    setKey(p,newkey);
    setOffset(p+keySize,TuplePtr(bidRight, -1));
    setOffset(p-TPTR_SIZE, TuplePtr(bidLeft, -1));

    node.increaseCount(1);
#ifdef OUTPUT
    std::cout<<"Insert In Internal "<<pBlock->Offset<<": "<<newkey<<std::endl;
    printNode(pBlock);
#endif
    return true;
}

/*
 * function: _insert
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 插入一个新值
 */
template <typename T>
bool  BPTree<T>::_insert(const T &newkey, const TuplePtr &newtupleptr)
{
/*+-----------------------------------------------------------------+
 *+                        normal insert                            +
 *+-----------------------------------------------------------------+*/
 
    Changed = true;
    Block* pBlock = getKeyNode(newkey);
    TreeNode node(pBlock);
    bool res = insert_in_leaf(pBlock,newkey,newtupleptr);
    
    if(!res)
        return false;

    // 检查是否需要进行调整
    if(node.getCount()<degree)
    {
        return true;
    }

/*+-----------------------------------------------------------------+
 *+                             split                               +
 *+-----------------------------------------------------------------+*/
    bm.SetLock(pBlock);

    Block* pNewBlock = createNewNode();

    bm.SetWritten(pNewBlock);
    bm.SetLock(pNewBlock); 

    int splitPos = (degree+1)/2;
    memcpy(pNewBlock->data,pBlock->data+splitPos*indexSize,(node.getCount()-splitPos)*indexSize);

    TreeNode newNode(pNewBlock);
    bm.SetWritten(pNewBlock);
    bm.SetWritten(pBlock);

    newNode.setCount(node.getCount()-splitPos);
    node.setCount(splitPos);
    newNode.setLeaf(node.isLeaf());
    node.setLeaf(newNode.id);
    newNode.setParent(node.getParent());

    // 上浮的key
    T upperkey = getKey(newNode.pData);
    
/*+-----------------------------------------------------------------+
 *+                    case 1: node is root                         +
 *+-----------------------------------------------------------------+*/
    if(node.getParent() == 0) //分裂节点为根节点
    {
        // 创建一个新结点作为根
        Block* pRootBlock=createNewNode();

        TreeNode newRoot(pRootBlock);
        bm.SetWritten(pRootBlock);
    
        newRoot.setCount(0);
        // 一定不是叶子结点了
        newRoot.setLeaf(-1);

        root = newRoot.id;  //更新root

        node.setParent(root);
        newNode.setParent(root);
    
        // 上浮的时候，把key的左右两个node的结点id都上浮
        insert_in_internal(pRootBlock,upperkey,node.id,newNode.id);
        bm.UnLock(pBlock);
        bm.UnLock(pNewBlock);  //解锁
#ifdef OUTPUT
        std::cout<<"Split "<<pBlock->Offset<<std::endl;
        std::cout<<"Left:";
        printNode(pBlock);
        std::cout<<"id="<<node.id<<" ";
        std::cout<<"Parent="<<node.getParent()<<std::endl;
       
        std::cout<<"Right:";
        printNode(pNewBlock);
        std::cout<<"id="<<newNode.id<<" ";
        std::cout<<"Parent="<<newNode.getParent()<<std::endl;
#endif
        return true;
    }
    bm.UnLock(pBlock);
    bm.UnLock(pNewBlock);  //解锁
    

/*+-----------------------------------------------------------------+
 *+                 case 2: node is not root                        +
 *+-----------------------------------------------------------------+*/
#ifdef OUTPUT
    std::cout<<"Split "<<pBlock->Offset<<std::endl;
    std::cout<<"left: id="<<node.id<<" "<<std::endl;
    printNode(pBlock);
    std::cout<<"Parent="<<node.getParent()<<std::endl;
    std::cout<<"right: id="<<newNode.id<<" "<<std::endl;;
    printNode(pNewBlock);
    std::cout<<"Parent="<<newNode.getParent()<<std::endl;
#endif

    pBlock = bm.GetBlock(indexName,node.getParent());

    insert_in_internal(pBlock, upperkey, node.id, newNode.id);
#ifdef OUTPUT
    std::cout<<"in parent node: ";
    printNode(pBlock);
#endif
    
    // 父结点
    node.attachTo(pBlock);
    bm.SetWritten(pBlock);
    bm.SetLock(pBlock);

    Block* parent = NULL;

    // cascade update, using a loop to avoid recursion
    while(node.getCount()==degree)
    {
        
        pNewBlock = createNewNode();
        bm.SetWritten(pNewBlock);
        bm.SetLock(pNewBlock);

        newNode.attachTo(pNewBlock);

        int splitPos = (degree)/2;

        upperkey = getKey(node.pData+(splitPos)*indexSize+TPTR_SIZE);

        memcpy(newNode.pData,node.pData+(splitPos+1)*indexSize,(node.getCount()-splitPos-1)*indexSize+TPTR_SIZE);
        newNode.setCount(node.getCount()-splitPos-1);
        node.setCount(splitPos);
        newNode.setParent(node.getParent());
        newNode.setLeaf(-1);

        // update parent's info
        char* p = newNode.pData;
        for(int i=newNode.getCount();i>=0;i--)
        {
            adjust_parent_info(getOffset(p).blockID,newNode.id);
            p+=indexSize;
        }
        
        // if the node is root.
        if(node.getParent() == 0)
        {
            Block* pRootBlock = createNewNode();
            bm.SetWritten(pRootBlock);

            TreeNode newRoot(pRootBlock);
            newRoot.setParent(0);
            newRoot.setCount(0);
            newRoot.setLeaf(-1);
            root = newRoot.id;

            node.setParent(root);
            newNode.setParent(root);

            insert_in_internal(pRootBlock,upperkey,node.id,newNode.id);
            bm.UnLock(pBlock);
            bm.UnLock(pNewBlock);
            return true;
        }
        parent = bm.GetBlock(indexName,node.getParent());
        bm.SetWritten(parent);
        bm.UnLock(pBlock);
        bm.UnLock(pNewBlock);

        insert_in_internal(parent,upperkey,node.id,newNode.id);

        node.attachTo(parent);
        bm.SetLock(parent);
    }
    if(parent)
        bm.UnLock(parent);
    
    return true;
}

/*
 * function: parse_in_block
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 在块中分析key的信息，将查询结果以NodeSearchParse返回
 */
template <typename T>
bool BPTree<T>::parse_in_block(Block* pBlock,T key, NodeSearchParse& res)
{
    TreeNode node(pBlock);
    int i=0;
    int j = node.getCount()-1;
    int middle = 0;
    res.isFound=false;
    while(i<=j)
    {
        middle = (i+j)>>1;
        T val = getKey(node.pData+middle*indexSize);
        if(val>key)
            j = middle - 1;
        else if(val<key)
            i = middle + 1;
        else if(val==key)  {
            res.isFound=true;
            break;
        }
    }
    res.indexName = indexName;
    res.blockOffset = node.id;
    res.tupleOffset = getOffset(node.pData+indexSize*middle+keySize);
    res.offsetInNode = middle*indexSize;
    return res.isFound;
}

/*
 * function: _delete
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 删除结点，类似于插入。但情况更为复杂
 */
template <typename T>
bool BPTree<T>::_delete(const T &oldkey)
{
/*+-----------------------------------------------------------------+
 *+                 find the oldkey's block                         +
 *+-----------------------------------------------------------------+*/
    int minLeafNum = degree/2;
    Block* pBlock = getKeyNode(oldkey);
    NodeSearchParse res;
    if(false==parse_in_block(pBlock,oldkey,res))
    {
        std::cerr<<"ERROR::NO SUCH KEY!"<<oldkey<<std::endl;
        return false;
    }
    Changed = true;

#ifdef TEST_DELETE
    std::cout<<"find key: "<<oldkey<<std::endl;
    res.printNSP();
#endif
    TreeNode node(pBlock);
    bm.SetWritten(pBlock);

/*+-----------------------------------------------------------------+
 *+                 case 1: the node is root                        +
 *+-----------------------------------------------------------------+*/
    if(node.id==root)
    {
/*+-----------------------------------------------------------------+
 *+                 case 1.1: the last key in BPT                   +
 *+-----------------------------------------------------------------+*/
        if(node.getCount()==1)
        {
            node.decreaseCount(1);
            std::cout << "root:" << root << std::endl;
            init();

            return true;
        }
/*+-----------------------------------------------------------------+
 *+                 case 1.2: there are another keys                +
 *+-----------------------------------------------------------------+*/
        char* p = node.pData+res.offsetInNode;
        char* end = node.pData+(node.getCount()-1)*indexSize;
        while(p<end)
        {
            memcpy(p,p+indexSize,indexSize);    //记录向前移动
            p+=indexSize;
        }
        node.decreaseCount(1);
        return true;
    }
/*+-----------------------------------------------------------------+
 *+                    case 2: normal case                          +
 *+-----------------------------------------------------------------+*/
    if(node.getCount()>minLeafNum)
    {
        char* p = node.pData+res.offsetInNode;
        char* end = node.pData+(node.getCount()-1)*indexSize;
        while(p<end)
        {
            memcpy(p,p+indexSize,indexSize);    //记录向前移动
            p+=indexSize;
        }
        node.decreaseCount(1);
#ifdef OUTPUT
#ifdef TEST_DELETE
        std::cout<<"OK"<<std::endl;
        printNode(pBlock);
#endif
#endif
        return true;
    }
#ifdef OUTPUT
#ifdef TEST_DELETE
    std::cout<<"Block "<<node.id<<" Need to adjust"<<std::endl;
    printNode(pBlock);
#endif
#endif

/*+-----------------------------------------------------------------+
 *+                  case 3: cascade to adjust                      +
 *+-----------------------------------------------------------------+*/
    bm.SetLock(pBlock);

    Block* pParent = bm.GetBlock(indexName,node.getParent());   //获取父节点
    TreeNode parent(pParent);
#ifdef OUTPUT
#ifdef TEST_DELETE
    std::cout<<"Parent: "<<std::endl;
    printNode(pParent);
#endif
#endif
    bm.SetWritten(pParent);
    bm.SetLock(pParent);

    Block* pSibling;
    TreeNode sibling;

    int off = search_son_offset(parent,node.id);
    bool leftSibling = false;

/*+-----------------------------------------------------------------+
 *+            case 3.1: find the sibling to merge                  +
 *+-----------------------------------------------------------------+*/
    if(off==0)
    {
        pSibling = bm.GetBlock(indexName,getOffset(parent.pData+off+indexSize).blockID);
        leftSibling = false;
    }
    else if(off==parent.getCount()*indexSize)   //最后一个儿子
    {
        pSibling = bm.GetBlock(indexName,getOffset(parent.pData+off-indexSize).blockID);
        leftSibling = true;
    }
    else
    {
        Block* pl = bm.GetBlock(indexName,getOffset(parent.pData+off-indexSize).blockID);
        sibling.attachTo(pl);
        if(sibling.getCount()>minLeafNum)
        {
            leftSibling = true;
            pSibling = pl;
        }
        else
        {
            leftSibling = false;
            pSibling = bm.GetBlock(indexName,getOffset(parent.pData+off+indexSize).blockID);
        }
    }
#ifdef OUTPUT
#ifdef TEST_DELETE
    std::cout<<"Choose sibling: "<<pSibling->Offset<<std::endl;
    printNode(pSibling);
#endif
#endif
/*+-----------------------------------------------------------------+
 *+                case 3.2: merge with the node                    +
 *+-----------------------------------------------------------------+*/
    if(pSibling->Offset==8)
        int b=1;
    bm.SetLock(pSibling);
    bm.SetWritten(pSibling);
    sibling.attachTo(pSibling);
    char* pDivider;
/*+-----------------------------------------------------------------+
 *+              case 3.2.1: merge with the node                    +
 *+-----------------------------------------------------------------+*/
    if(sibling.getCount()>minLeafNum) 
    {
        if(!leftSibling)    //右兄弟       // OK
        {
            char* p = node.pData+res.offsetInNode;
            char* end = node.pData+(node.getCount()-1)*indexSize;
            while(p<end)
            {
                memcpy(p,p+indexSize,indexSize);    //记录向前移动
                p+=indexSize;
            }

            memcpy(end,sibling.pData,indexSize);// 移动一条
            
            p = sibling.pData;
            end = sibling.pData+(sibling.getCount()-1)*indexSize;
            while(p<end)
            {
                memcpy(p,p+indexSize,indexSize);
                p += indexSize;
            }
            sibling.decreaseCount(1);
            
            memcpy(parent.pData+off+TPTR_SIZE,sibling.pData,keySize);   //更新父节点key

        }
        else        //OK
        {
            char* p = node.pData + res.offsetInNode;   //删除点前段右移
            char* end = node.pData;

            while(p>end)
            {
                memcpy(p,p-indexSize,indexSize);
                p -= indexSize;
            }//完成右移

            memcpy(node.pData,sibling.pData+(sibling.getCount()-1)*indexSize,indexSize);
            //移动最后一条记录
            sibling.decreaseCount(1);
            memcpy(parent.pData+off-keySize,node.pData,keySize); //更新父节点key
        }
    }
/*+-----------------------------------------------------------------+
 *+              case 3.2.2: merge with the node                    +
 *+-----------------------------------------------------------------+*/
    else        
    {        
        if(!leftSibling)    //ok
        {
            pDivider = parent.pData+off+TPTR_SIZE;
            char* p = node.pData+res.offsetInNode;
            char* end = node.pData+(node.getCount()-1)*indexSize;
            while(p<end)    //删除点右侧向前移动
            {
                memcpy(p,p+indexSize,indexSize);    //记录向前移动
                p+=indexSize;
            }
            memcpy(end,sibling.pData,sibling.getCount()*indexSize);
            node.increaseCount(sibling.getCount()-1); //右兄弟并入，删除右兄弟
            node.setLeaf(sibling.isLeaf());
            emptyList.push_back(sibling.id);
        }
        else        //OK
        {
            pDivider = parent.pData+off-keySize;
            char* p = sibling.pData+sibling.getCount()*indexSize;   //左兄弟末尾
            memcpy(p, node.pData,res.offsetInNode);  //删除点前段

            memcpy(p+res.offsetInNode,node.pData+res.offsetInNode+indexSize,
                (node.getCount()-1)*indexSize-res.offsetInNode);
            sibling.increaseCount(node.getCount()-1);
            sibling.setLeaf(node.isLeaf());
            emptyList.push_back(node.id); //删除右节点
        }
        //合并至左边节点
        bm.UnLock(pBlock);
        bm.UnLock(pSibling);
#ifdef OUTPUT
#ifdef TEST_DELETE
        std::cout<<"Adjust Parent "<<pParent->Offset<<std::endl;
        printNode(pParent);
        T delkey = getKey(pDivider);
        std::cout<<"delete :"<<delkey<<std::endl;
#endif
#endif
        adjust_after_delete(pParent,pDivider); //调整
    }

    return true;
}

/*
 * function: adjust_after_delete
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 调整删除后冗余的信息
 */
template <typename T>
void BPTree<T>::adjust_after_delete(Block* pNode,char* delPos)
{
    int minInternal = (degree-1)/2;
    TreeNode node(pNode);

    bm.SetLock(pNode);
    bm.SetWritten(pNode);
    if(node.getCount()>minInternal)
    {
            char* p = delPos;
            char* end = node.pData+(node.getCount())*indexSize-keySize;
            while(p<end)    //向前移动
            {
                memcpy(p,p+indexSize,indexSize);
                p+=indexSize;
            }
            node.decreaseCount(1);
            bm.UnLock(pNode);
#ifdef OUTPUT
#ifdef TEST_DELETE
        printNode(pNode);
#endif
#endif
            return;
    }
    if(node.id==root)
    {
        if(node.getCount()==1)
        {
            bm.UnLock(pNode);
            emptyList.push_back(root);
            root = getOffset(node.pData).blockID;//唯一的儿子成为root
            adjust_parent_info(root, -1);
            return;
        }
        else
        {
            char* p = delPos;
            
            char* end = node.pData+(node.getCount()-1)*indexSize+TPTR_SIZE;
            while(p<end)    //向前移动
            {
                memcpy(p,p+indexSize,indexSize);
                p+=indexSize;
            }
            node.decreaseCount(1);
            bm.UnLock(pNode);
#ifdef OUTPUT
#ifdef TEST_DELETE
            printNode(pNode);
#endif
#endif
            return;
        }
    }


    Block* pParent = bm.GetBlock(indexName,node.getParent());
    bm.SetWritten(pParent);
    bm.SetLock(pParent);
    TreeNode parent(pParent);
    
    int offInParent = search_son_offset(parent,node.id); //该节点对应父节点中的指针偏移量

    Block* pSibling;
    TreeNode sibling;
    bool leftSibling = false;

    char* pDivider;
    if(offInParent==0)  //父节点的第一个儿子
    {
        pSibling = bm.GetBlock(indexName,getOffset(parent.pData+offInParent+indexSize).blockID);
        leftSibling = false;
        pDivider = parent.pData+offInParent+TPTR_SIZE;
    }
    else if(offInParent==parent.getCount()*indexSize)   //最后一个儿子
    {
        pSibling = bm.GetBlock(indexName,getOffset(parent.pData+offInParent-indexSize).blockID);
        leftSibling = true;
        pDivider = parent.pData+offInParent-keySize;
    }
    else
    {
        Block* pl = bm.GetBlock(indexName,getOffset(parent.pData+offInParent-indexSize).blockID);
        sibling.attachTo(pl);
        if(sibling.getCount()>minInternal)
        {
            leftSibling = true;
            pSibling = pl;
            pDivider = parent.pData+offInParent-keySize;
        }
        else
        {
            leftSibling = false;
            pSibling = bm.GetBlock(indexName,getOffset(parent.pData+offInParent+indexSize).blockID);
            pDivider = parent.pData+offInParent+TPTR_SIZE;
        }
    }
    
    bm.SetLock(pSibling);
    bm.SetWritten(pSibling);
    sibling.attachTo(pSibling);

#ifdef OUTPUT
#ifdef TEST_DELETE
    std::cout<<"sibling: "<<std::endl;
    printNode(pSibling);
#endif
#endif
    if(sibling.getCount()>minInternal)
    {
        if(leftSibling) //ok
        {
            char* p = delPos;
            char* end = node.pData+TPTR_SIZE;
            while(p>end)
            {
                memcpy(p,p-indexSize,indexSize);    //删除点右边右移
                p-=indexSize;
            }

            p+=keySize;
            memcpy(p,p-indexSize,TPTR_SIZE);   //最左端指针

            p = sibling.pData+(sibling.getCount())*indexSize;
            memcpy(node.pData+TPTR_SIZE,pDivider,keySize); //父节点分割的key放入节点
            memcpy(node.pData,p,TPTR_SIZE);       //兄弟节点最右边指针加入该节点
            memcpy(pDivider,p-keySize,keySize);                 //最右边key变为父节点中新的key

            adjust_parent_info(getOffset(node.pData).blockID,node.id);

            sibling.decreaseCount(1);
        }
        else        //ok
        {
            char* p = delPos;
            char* end = node.pData+(node.getCount()-1)*indexSize+TPTR_SIZE;
            while(p<end)
            {
                memcpy(p,p+indexSize,indexSize);    //删除点右边 左移
                p+=indexSize;
            }

            memcpy(end,pDivider,keySize);   //父节点key下移
            memcpy(end+keySize,sibling.pData,TPTR_SIZE);       //指针加入左边
            memcpy(pDivider,sibling.pData+TPTR_SIZE,keySize);  //右节点  最左边key上移至父节点

            adjust_parent_info(getOffset(sibling.pData).blockID,node.id );

            p = sibling.pData;
            end = sibling.pData+(sibling.getCount()-1)*indexSize;
            while(p<end)
            {
                memcpy(p,p+indexSize,indexSize);
                p+=indexSize;
            }
            memcpy(end,end+indexSize,TPTR_SIZE);   //最右指针
            sibling.decreaseCount(1);
        }
    }
    else
    {
        if(leftSibling) //ok
        {
            char* p = node.pData;
            char* siblingEnd = sibling.pData+sibling.getCount()*indexSize+TPTR_SIZE;
            memcpy(siblingEnd,pDivider,keySize);    //获取父节点key
            siblingEnd+=keySize;    //指向最后一个指针
            memcpy(siblingEnd,p,delPos-p);    //删除点左
            //调整父节点
            while(p<delPos)
            {
                adjust_parent_info(getOffset(p).blockID,sibling.id);
                p+=indexSize;
            }

            siblingEnd+=delPos-node.pData;
            p = delPos+indexSize+keySize;
            char* end = node.getCount()*indexSize+TPTR_SIZE+node.pData;
            memcpy(siblingEnd,p-keySize,end-p+keySize);    //删除点右
            while(p<end)
            {
                adjust_parent_info(getOffset(p).blockID,sibling.id);
                p+=indexSize;
            }
            sibling.increaseCount(node.getCount());
            emptyList.push_back(node.id);   
        }
        else    //ok
        {
            char* p = delPos;
            char* end = node.pData+(node.getCount()-1)*indexSize+TPTR_SIZE;
            while(p<end)
            {
                memcpy(p,p+indexSize,indexSize);
                p+=indexSize;
            }
            memcpy(end,pDivider,keySize);
            end+=keySize;

            p = sibling.pData;
            char* siblingEnd = sibling.pData+sibling.getCount()*indexSize+TPTR_SIZE;
            memcpy(end,sibling.pData,siblingEnd-p);     //合并入左节点
            //调整父节点
            while(p<siblingEnd)
            {
                adjust_parent_info(getOffset(p).blockID,node.id);
                p+=indexSize;
            }
            emptyList.push_back(sibling.id);
            node.increaseCount(sibling.getCount());
            
        }
        bm.UnLock(pSibling);
        bm.UnLock(pNode);
#ifdef TEST_DELETE
        T delkey = getKey(pDivider);
        std::cout<<"delete :"<<delkey<<std::endl;
#endif
        adjust_after_delete(pParent,pDivider);
    }


}

/*
 * function: adjust_parent_info
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 调整父节点信息
 */
template <typename T>
void BPTree<T>::adjust_parent_info(BlockIDType id, BlockIDType newp)
{
    Block* pBlock=bm.GetBlock(indexName,id);
    bm.SetWritten(pBlock);
    char* p = pBlock->data+BLOCKSIZE-INT_SIZE*2;
    memcpy(p, _data2cptr(newp), sizeof(BlockIDType));
}

/*
 * function: search_son_offset
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 寻找子节点的偏移量（根据bid进行查找）
 */
template <typename T>
int BPTree<T>::search_son_offset(TreeNode& parent, const BlockIDType id)
{
    parent.reset();
    char* p = parent.pData;
    while(getOffset(p).blockID!=id)
    {
        p+=indexSize;
    }
    return (p-parent.pData);
}

/*
 * function: init
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 初始化BPTree, 并删除之前存留的BPTree
 */
template <typename T>
void BPTree<T>::init(void)
{
    FILE* fp = fopen(indexName.c_str(), "w");
    if(fp==NULL)
    {
        std::cerr << "ERROR::CANNOT OPEN FILE!" << std::endl;
        indexName = "";
        return;
    }
    fclose(fp);

    bm.DeleteFileBlock(indexName);
    Block* pHead = bm.GetNewBlock(indexName, 0);
    bm.SetWritten(pHead);
    IndexHead head(pHead);
    
    nodeNum = 1;
    root = 1;
    firstLeaf = 1;

    head.setNodeNum(nodeNum);
    head.setRoot(root);
    head.setFirstLeaf(firstLeaf);
    head.setKeySize(keySize);

    emptyList.clear();

    head.setEmptyBlockList(emptyList);
    
    
    // root node
    Block* pBlock = bm.GetNewBlock(indexName, root);
    bm.SetWritten(pBlock);
    TreeNode node(pBlock);
    node.setCount(0);
    node.setParent(0);
    node.setLeaf(0);
    Changed = false;
}

/*
 * function: _modify
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 修改BPTree中的值的信息，结合delete和insert
 */
template <typename T>
bool BPTree<T>::_modify(const T &oldkey, const T &newkey, const TuplePtr &newtupleptr)
{
    Block* pBlock = getKeyNode(oldkey);
    bm.SetWritten(pBlock);
    TreeNode node(pBlock);
    NodeSearchParse res;
    if(!parse_in_block(pBlock,oldkey,res))
    {
        std::cerr << "ERROR::CANNOT FIND SUCH KEY::" << oldkey << std::endl;
        return false;
    }
    Changed = true;
    char* p = res.offsetInNode+node.pData;
    char* end = node.pData+(node.getCount()-1)*indexSize;
    T l = getKey(node.pData);
    T r = getKey(end);
    if(newkey>=l&&newkey<=r)
    {
        if(newkey>=oldkey)
        {
            while(p<end&&newkey>getKey(p+indexSize))
            {
                memcpy(p,p+indexSize,indexSize);
                p+=indexSize;
            }
            setKey(p,newkey);
            setOffset(p+keySize,newtupleptr);
        }
        else
        {
            while(p>node.pData&&newkey<getKey(p-indexSize))
            {
                memcpy(p,p-indexSize,indexSize);
                p-=indexSize;
            }
            setKey(p,newkey);
            setOffset(p+keySize,newtupleptr);
        }
        return true;
    }

    _delete(oldkey);
   // showLeaf();
    _insert(newkey,newtupleptr);
    //showLeaf();
    return true;
}

/*
 * function: createNewNode
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 创建新的结点
 */
template <typename T>
Block* BPTree<T>::createNewNode(void)
{
    Block* pBlock;
    int id;
    if(emptyList.size()==0){
            nodeNum++;
            id = nodeNum;
            pBlock = bm.GetNewBlock(indexName,id);
    }
    else
    {
        id = emptyList.front();
        emptyList.pop_front();
        pBlock = bm.GetBlock(indexName,id);
    }
    return pBlock;
}

/*
 * function: _search
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 根据key值寻找其对应的tuple虚指针
 */
template <typename T>
OffsetType BPTree<T>::_search(const T &key)
{
    Block* pBlock = getKeyNode(key);
    return block_search(pBlock,key);
}

/*
 * function: block_search
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 在确定块中寻找key值
 */
template <typename T>
OffsetType BPTree<T>::block_search(Block* pBlock, const T &key)
{
    std::cout << "key:" << key << std::endl;
    TreeNode node(pBlock);

    int i=0;
    int j = (node.getCount()-1);
    int middle = 0;

    while(i<=j)
    {
        middle = (i+j)>>1;
        T val = getKey(node.pData+middle*indexSize);
        if(val>key)
            j = middle - 1;
        else if(val<key)
            i = middle + 1;
        else if(val==key)
        {
            std::cout << "find" << std::endl;
            return getOffset(node.pData+middle*indexSize+keySize);
        }
    }
    return NONE;
}

/*
 * function: writeBack
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 将bptree的信息写回block
 */
template <typename T>
void BPTree<T>::writeBack()
{
    Block* pHead = bm.GetBlock(indexName, 0);
    bm.SetWritten(pHead);
    IndexHead head(pHead);
    
    head.setNodeNum(nodeNum);   //NodeNumber
    head.setRoot(root);  //root node id
    head.setFirstLeaf(firstLeaf);    //first Leaf id
    head.setKeySize(keySize);

    head.setEmptyBlockList(emptyList);
    Changed = false;
    //std::cout<<*(node.pCount)<<" "<<*(node.pParent)<<std::endl;
}

/*
 * function: _create
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 建立BPTree，并创建相应文件存储
 */
template <typename T>
void BPTree<T>::_create(const std::string &filename, int keySize)
{
    if(Changed==true)
        writeBack();

    indexName = filename;
    this->keySize = keySize;

    indexSize = keySize+TPTR_SIZE;
    degree = (BLOCKSIZE-sizeof(OffsetType)-sizeof(int)*3)/(indexSize) - 2;

    root = 1;
    firstLeaf = 1;
    nodeNum = 1;

    FILE* fp = fopen(filename.c_str(), "w");
    if(fp==NULL)
    {
        std::cerr<<"ERROR::CANNOT OPEN FILE!"<<std::endl;
        exit(-1);
    }
    fclose(fp);

    Block* pHead = bm.GetNewBlock(filename, 0);
    bm.SetWritten(pHead);
    IndexHead head(pHead);
    
    head.setNodeNum(1);   //NodeNumber
    head.setRoot(1);  //root node id
    head.setFirstLeaf(1);    //first Leaf id
    head.setKeySize(keySize);

    emptyList.clear();
    head.setEmptyBlockList(emptyList);
    //std::cout<<*(node.pCount)<<" "<<*(node.pParent)<<std::endl;
    
    Block* pBlock = bm.GetNewBlock(filename,1);
    TreeNode node(pBlock);
    bm.SetWritten(pBlock);
    node.setCount(0);
    node.setParent(0);
    node.setLeaf(0);
}


/*
 * function: readFromFile
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 从磁盘中读入BPtree
 */
template <typename T>
void BPTree<T>::readFromFile(const std::string &filename)
{
    if(indexName==filename)
    {
        return;
    }
    else if(Changed==true)
        writeBack();

    indexName = filename;

    Block* pHead = bm.GetBlock(filename,0);
   // bm.SetWritten(pHead);

    IndexHead head(pHead);
    keySize = head.getKeySize();
    root = head.getRoot();
    std::cout << "read:root::" << root << std::endl;
    firstLeaf = head.getFirstLeaf();
    nodeNum = head.getNodeNum();

    indexSize = keySize+TPTR_SIZE;
    degree = (BLOCKSIZE-sizeof(OffsetType)-sizeof(int)*3)/(indexSize) - 2;
    head.getEmptyBlockList(emptyList);
}


/*
 * function: _drop
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 删除BPtree, 并删除对应的磁盘文件
 */
template <typename T>
bool BPTree<T>::_drop()
{
    FILE* fp = fopen(indexName.c_str(), "w");
    if(fp==NULL)
    {
        std::cerr<<"ERROR::CANNOT OPEN FILE!"<<std::endl;
        return false;
    }
    fclose(fp);
    remove(indexName.c_str());
    Changed = false;
    indexName = "";
    return true;
}

/*
 * function: _release
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * 将BPTree的修改信息撤销
 */
template <typename T>
bool BPTree<T>::_release(void)
{
    Changed = false;
    indexName = "";
    return true;
}


/*
 * function: range_search
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun 16 01:07:47 DST 2018
 * ------------------------------------------
 * BPTree的区间搜索，() // (] // [) // []
 * 尚未修改，仍有部分问题
 */
template <typename T>
bool BPTree<T>::range_search(const T lvalue, const int lclosed, 
                      const T rvalue, const int rclosed,
                      std::vector<TuplePtr>& tuplePtrs)
{
    Block* pBlock = getKeyNode(lvalue);

    if(lclosed>0&&rclosed>0)
    {
        Block* pBlock = getKeyNode(lvalue);
        TreeNode node;
        BlockIDType bid = pBlock->Offset;
        while(bid>0)
        {
            node.attachTo(pBlock);
            char* p = node.pData;;
            char* end = node.pData+indexSize*(node.getCount()-1);
            
            T val;
            while((val=getKey(p))<lvalue&&p<=end)
            {
                p+=indexSize;
            }
            if(p>end)
            {
                bid = node.isLeaf();    //next leaf
                pBlock = bm.GetBlock(indexName, bid);
                continue;
            }
            //找到第一个大于等于
            while(p<=end&&(val=getKey(p))<=rvalue)
            {
                tuplePtrs.push_back(getOffset(p+keySize));
                p+=indexSize;
            }
            if(p>end)
            {
                bid = node.isLeaf();    //next leaf
                pBlock = bm.GetBlock(indexName, bid);
                continue;
            }
            else
            {
                return true;    //结束搜索
            }
        }
        
    }
    else if(lclosed>0&&rclosed<=0)
    {
        Block* pBlock = getKeyNode(lvalue);
        TreeNode node;
        BlockIDType bid = pBlock->Offset;
        while(bid>0)
        {
            node.attachTo(pBlock);
            char* p = node.pData;;
            char* end = node.pData+indexSize*(node.getCount()-1);
            
            T val;
            while((val=getKey(p))<lvalue&&p<=end)
            {
                p+=indexSize;
            }
            if(p>end)
            {
                bid = node.isLeaf();    //next leaf
                if(bid<0) return true;
                pBlock = bm.GetBlock(indexName, bid);
                continue;
            }
            //找到第一个大于等于
            while(p<=end)
            {
                tuplePtrs.push_back(getOffset(p+keySize));
                p+=indexSize;
            }
            if(p>end)
            {
                bid = node.isLeaf();    //next leaf
                if(bid<0) return true;
                pBlock = bm.GetBlock(indexName, bid);
            }
        }
    }
    else if(lclosed<=0&&rclosed>0)
    {
        Block* pBlock = bm.GetBlock(indexName, firstLeaf);
        TreeNode node;
        BlockIDType bid = pBlock->Offset;
        while(bid>0)
        {
            node.attachTo(pBlock);
            char* p = node.pData;;
            char* end = node.pData+indexSize*(node.getCount()-1);
            T val;
            while((val=getKey(p))<=rvalue&&p<=end)
            {
                tuplePtrs.push_back(getOffset(p+keySize));
                p+=indexSize;
            }
            if(p>end)
            {
                bid = node.isLeaf();    //next leaf
                if(bid<0) return true;
                pBlock = bm.GetBlock(indexName, bid);
            }
            else return true;
        }
    }
    return false;  //不存在上下界
    
}