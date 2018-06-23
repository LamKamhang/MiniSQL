/*
 * File: BPTree.hpp
 * Version: 1.4
 * Author: kk
 * Created Date: Sat Jun  2 20:04:21 DST 2018
 * Modified Date: Mon Jun 11 22:37:24 DST 2018
 * Modified Date: Fri Jun 15 21:10:18 DST 2018
 * Modified Date: Sat Jun 16 01:07:47 DST 2018
 * Modified Date: Tue Jun 19 16:53:17 DST 2018
 * -------------------------------------------
 * miniSQL的IndexManager需要用到的数据结构B+树定义
 * 实现B+树的基本操作，插入，删除，合并，分裂
 * 为了通用性，使用模板编程方式。
 * version 1.2 去除Union声明，联合中不能使用类，需要保持对内存相同的操作
 * version 1.3 修正inner node的split方式，上浮的key不保存在inner node中
 * version 1.4 无法修正remove的bug，对BPTree进行重构，
 *             结合bufferManager进行磁盘读写
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

#define _cptr2float(x)  (*(float*)(x))
#define _cptr2int(x)  (*(int*)(x))
#define _cptr2uint(x) (*(unsigned int*)(x))
#define _data2cptr(x)  ((char*)(&x))

class NodeSearchParse
{
public:
    OffsetType tupleOffset;
    unsigned int offsetInNode;
    BlockIDType blockOffset;
    string indexName;
    bool isFound;

#define DEBUG
#ifdef DEBUG
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
#endif
};

class IndexHead{
public:
    IndexHead(Block* pHead):
        pEmptyId    (pHead->data), 
        pNodeNum    (pHead->data+BLOCKSIZE-INT_SIZE*1),
        pRoot       (pHead->data+BLOCKSIZE-INT_SIZE*2), 
        pFirstLeaf  (pHead->data+BLOCKSIZE-INT_SIZE*3), 
        pKeySize    (pHead->data+BLOCKSIZE-INT_SIZE*4), 
        pEmptyNum   (pHead->data+BLOCKSIZE-INT_SIZE*5)
    {};

    int getRoot()       {return _cptr2int(pRoot);};
    int getNodeNum()    {return _cptr2int(pNodeNum);};
    int getFirstLeaf()  {return _cptr2int(pFirstLeaf);};
    int getKeySize()    {return _cptr2int(pKeySize);};
    
    // 把内容拷贝到heap上指针的所指内容中
    void setRoot(int p)     {memcpy(pRoot,      _data2cptr(p), INT_SIZE);};
    void setNodeNum(int n)  {memcpy(pNodeNum,   _data2cptr(n), INT_SIZE);};
    void setFirstLeaf(int l){memcpy(pFirstLeaf, _data2cptr(l), INT_SIZE);};
    void setKeySize(int k)  {memcpy(pKeySize,   _data2cptr(k), INT_SIZE);};


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
    char* pEmptyId;

    char* pNodeNum;
    char* pRoot;
    char* pFirstLeaf;
    char* pKeySize;

    char* pEmptyNum;
    void setEmptyNum(int n){memcpy(pEmptyNum, _data2cptr(n), INT_SIZE);};
    int  getEmptyNum(){return _cptr2int(pEmptyNum);};
};


class TreeNode{
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


    inline void reset()             {pData = head;};
    inline int getCount()           {return _cptr2int(pCount);};
    inline BlockIDType getParent()  {return _cptr2uint(pParent);};
    inline int isLeaf()             {return _cptr2int(pIsLeaf);};

    inline void setCount(int c)             {memcpy(pCount,_data2cptr(c),4);};
    inline void setParent(BlockIDType pid)  {memcpy(pParent,_data2cptr(pid),4);};
    inline void setLeaf(int k)              {memcpy(pIsLeaf,_data2cptr(k),4);};

    inline void increaseCount(int k){int c = getCount();c+=k;setCount(c);};
    inline void decreaseCount(int k){int c = getCount();c-=k;setCount(c);};
private:
    char* pCount;       // how many keys in the node
    char* pParent;      // the block id of parent(if there no parent, id is 0, since block 0 is for indexHead)
    char* pIsLeaf;      // the block id of sibling(0 means the last leaf, and -something means it is an inner node)

    char* head;         // this is the pointer head of the pData.
};

/*
 *   B+ Tree 
 *   with key type T
 *     [ BLOCKSIZE-sizeof(OffsetType)-sizeof(int) * 2 ] / (keySize + TPTR_SIZE)  - 1 
 */
template <typename T>
class BPTree 
{
public:
    std::string indexName;  // coresponding to the file name
private:
    BufferManager& bm;
    int degree;             // Degree of node, do not need to write into disk.
    BlockIDType root;       // need to write into disk.
    BlockIDType firstLeaf;  // need to write into disk.
    //Index Info
    int keySize;            // need to write into disk.
    int indexSize;          // can calculate by keysize and offsetsize(tupleptrsize)
    int nodeNum;            // need to write into disk.
    bool Changed;           // check whether the (need_write_variable) need to write into disk.
public:
    BPTree(const std::string &indexName, const int keySize, 
                  BufferManager& bm, 
                  int nodeDegree = 0);
    BPTree(BufferManager& bm);
    ~BPTree();

    /* Inline function */    
    inline BlockIDType getRoot()        {return root;}; 
    inline BlockIDType getFirstLeaf()   {return firstLeaf;};
    inline bool isChanged()             {return Changed;};

    bool _delete(const T &oldkey);

    bool _insert(const T &newkey, const TuplePtr &newtupleptr);    // Insert an index record

    bool _modify(const T &oldkey, const T &newkey, const TuplePtr &newtupleptr);
    
    bool _drop(const std::string &indexName);

    bool _release(void);

    TuplePtr _search(const T &key);

    bool range_search(const T lvalue, const int lclosed, 
                      const T rvalue, const int rclosed,
                      std::vector<TuplePtr>& tuplePtrs);

    void init(void);

    void writeBack();
    void readFromFile(const std::string &filename);
    void createNewFile(const std::string &filename, int keySize);
    TuplePtr block_search(const Block* pBlock, const T &key);

private:
    list<int> emptyList;    //空块

    Block* createNewNode(void);

    void insert_in_leaf(Block* pBlock, const T newkey,const TuplePtr &newtupleptr);

    char* insert_in_internal(Block* pBlock, const T newkey, const BlockIDType bidLeft, const BlockIDType bidRight);

    void adjustAfterDelete(Block* pNode, char* delPos);

    Block* getKeyNode(T key);

    void adjustParentInfo(BlockIDType id, BlockIDType newp);

    int searchSon(TreeNode& parent, const BlockIDType id);
    //void convertKey(string key);

    T getKey(char* p);

    void setKey(char* p, T newkey);

    OffsetType getOffset(char* p);

    void setOffset(char* p, OffsetType offset);
    
    bool searchBlock(Block* pBlock, T key, NodeSearchParse& res);

#define BPT_TEST
#ifdef BPT_TEST
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





/*=================================
=            BPTree            =
=================================*/



template <typename T>
BPTree<T>::~BPTree()
{
    if (Changed)
        writeBack();
}


/**
 *  Constructor fot B+ Tree
 *  @ param int the degree of tree
 *  @int size of key in byte
 */

template <typename T>
BPTree<T>::BPTree(BufferManager& bm):
    indexName(""), bm(bm), Changed(false)
{
}

/* Used for test */
// 一个block里首先除了包含正常的数据外，还包含了root, firstleaf, nodenum以及keysize和emptynum的信息， 最后-2则是为了split
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
    // 往文件里写入BPTree的信息
    Block* pHead = bm.GetNewBlock(indexName, 0);
    bm.SetWritten(pHead);

    IndexHead head(pHead);
    
    head.setNodeNum(nodeNum);      //NodeNumber
    head.setRoot(root);             //root node id
    head.setFirstLeaf(firstLeaf);    //first Leaf id
    head.setKeySize(keySize);
    emptyList.clear();

    head.setEmptyBlockList(emptyList);
    //std::cout<<*(node.pCount)<<" "<<*(node.pParent)<<std::endl;
    
    Block* pBlock = bm.GetNewBlock(indexName, 1);
    bm.SetWritten(pBlock);
    TreeNode node(pBlock);
    node.setCount(0);   // 当前没有key值
    node.setParent(0);  // 没有父节点（root）
    node.setLeaf(0);    // 非负数表示叶子结点，其中0表示最后一个叶子结点
    Changed = false;
    //std::cout<<*(node.pCount)<<" "<<*(node.pParent)<<std::endl;
}


template <typename T>
Block* BPTree<T>::getKeyNode(T key)
{
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



template <typename T>
void BPTree<T>::insert_in_leaf(Block* pBlock, const T newkey,const TuplePtr &newtupleptr)
{
    TreeNode node(pBlock);
    bm.SetWritten(pBlock);

    // 指向最后一个key_index
    char* p = node.pData+(node.getCount()-1) * indexSize;
    while(p>=node.pData)
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
    setOffset(p+keySize, newtupleptr);

    node.increaseCount(1);
#define OUTPUT
#ifdef OUTPUT
    std::cout<<"Insert In Leaf "<<pBlock->Offset<<": "<<newkey<<std::endl;
    printNode(pBlock);
#endif
}

template <typename T>
char* BPTree<T>::insert_in_internal(Block* pBlock,T newkey,BlockIDType bidLeft,BlockIDType bidRight)
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
    setOffset(p+keySize,bidRight);
    setOffset(p-TPTR_SIZE,bidLeft);

   // adjustParentInfo(bidLeft,pBlock->Offset);
   // adjustParentInfo(bidRight,pBlock->Offset);

    node.increaseCount(1);
#ifdef OUTPUT
    std::cout<<"Insert In Internal "<<pBlock->Offset<<": "<<newkey<<std::endl;
    printNode(pBlock);
#endif
    return p;
}


template <typename T>
bool  BPTree<T>::_insert(const T &newkey, const TuplePtr &newtupleptr)
{
    Changed = true;
    Block* pBlock = getKeyNode(newkey);
    
    TreeNode node(pBlock);
    insert_in_leaf(pBlock,newkey,newtupleptr);
    if(node.getCount()<degree)
    {
        return true;
    }

    // 开始调整，split
    bm.SetLock(pBlock);

    Block* pNewBlock = createNewNode();

    bm.SetWritten(pNewBlock);
    bm.SetLock(pNewBlock);  //锁

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
#define TEST_DELETEs
#ifdef TEST_DELETE
    std::cout<<"A's parent: "<<node.getParent()<<std::endl;
    std::cout<<"B's parent: "<<newNode.getParent()<<std::endl;
#endif

    // 上浮的key
    T upperkey = getKey(newNode.pData);
    

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
#ifdef OUTPUT
        std::cout<<"in new root: ";
        printNode(pRootBlock);
#endif
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

    // 级联更新
    while(node.getCount()==degree)
    {
        
        pNewBlock = createNewNode();
        bm.SetWritten(pNewBlock);
        bm.SetLock(pNewBlock);

        newNode.attachTo(pNewBlock);

        int splitPos = (degree)/2;

        upperkey = getKey(node.pData+(splitPos)*indexSize+TPTR_SIZE);  //移到上一层节点的key

        memcpy(newNode.pData,node.pData+(splitPos+1)*indexSize,(node.getCount()-splitPos-1)*indexSize+TPTR_SIZE);
                   //新节点块头指针       //指向原节点分裂点右边的偏移量    //
        newNode.setCount(node.getCount()-splitPos-1);
        node.setCount(splitPos);
        newNode.setParent(node.getParent());
        newNode.setLeaf(-1);
        //printNode(pBlock);
        //printNode(pNewBlock);
        //更新子节点的父节点信息
        char* p = newNode.pData;
        for(int i=newNode.getCount();i>=0;i--)
        {
            adjustParentInfo((int)getOffset(p),newNode.id);
            p+=indexSize;
        }
        
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
}


// specialization
template <>
float BPTree<float>::getKey(char *p)
{ return _cptr2float(p); }

template <>
int BPTree<int>::getKey(char *p)
{ return _cptr2int(p); }

template <>
std::string BPTree<std::string>::getKey(char *p)
{ return std::string(p); }


template <typename T>
OffsetType BPTree<T>::getOffset(char* p)
{ return *(OffsetType*)p; }


template <typename T>
void BPTree<T>::setKey(char* p,T newkey)
{ memcpy(p, _data2cptr(newkey), keySize); }
    
template <>
void BPTree<std::string>::setKey(char *p, std::string newkey)
{ memcpy(p, newkey.c_str(), keySize); }


template <typename T>
void BPTree<T>::setOffset(char* p, OffsetType newtupleptr)
{ memcpy(p,_data2cptr(newtupleptr), TPTR_SIZE); }


template <typename T>
bool BPTree<T>::searchBlock(Block* pBlock,T key,NodeSearchParse& res)
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


template <typename T>
bool BPTree<T>::_delete(const T &oldkey)
{
    int minLeafNum = degree/2;
    Block* pBlock = getKeyNode(oldkey);
    NodeSearchParse res;
    if(false==searchBlock(pBlock,oldkey,res))
    {
        //printNode(pBlock);
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
    if(node.id==root)
    {
        if(node.getCount()==1)
        {
            //printNode(pBlock);
            node.decreaseCount(1);
            init();

            return true;
        }
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
#ifdef TEST_DELETE
        std::cout<<"OK"<<std::endl;
        printNode(pBlock);
#endif
        return true;
    }
#ifdef TEST_DELETE
    std::cout<<"Block "<<node.id<<" Need to adjust"<<std::endl;
    printNode(pBlock);
#endif
    //需要调整
    bm.SetLock(pBlock);

    Block* pParent = bm.GetBlock(indexName,node.getParent());   //获取父节点
    TreeNode parent(pParent);
#ifdef TEST_DELETE
    std::cout<<"Parent: "<<std::endl;
    printNode(pParent);
#endif
    bm.SetWritten(pParent);
    bm.SetLock(pParent);

    Block* pSibling;
    TreeNode sibling;

    int off = searchSon(parent,node.id);
    bool leftSibling = false;
    
    if(off==0)  //父节点的第一个儿子
    {
        pSibling = bm.GetBlock(indexName,getOffset(parent.pData+off+indexSize));
        leftSibling = false;
    }
    else if(off==parent.getCount()*indexSize)   //最后一个儿子
    {
        pSibling = bm.GetBlock(indexName,getOffset(parent.pData+off-indexSize));
        leftSibling = true;
    }
    else
    {
        Block* pl = bm.GetBlock(indexName,getOffset(parent.pData+off-indexSize));
        sibling.attachTo(pl);
        if(sibling.getCount()>minLeafNum)
        {
            leftSibling = true;
            pSibling = pl;
        }
        else
        {
            leftSibling = false;
            pSibling = bm.GetBlock(indexName,getOffset(parent.pData+off+indexSize));
        }
    }
#ifdef TEST_DELETE
    std::cout<<"Choose sibling: "<<pSibling->Offset<<std::endl;
    printNode(pSibling);
#endif
    if(pSibling->Offset==8)
        int b=1;
    bm.SetLock(pSibling);
    bm.SetWritten(pSibling);
    sibling.attachTo(pSibling);
    char* pDivider;
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

            //node.decreaseCount(1);
            memcpy(node.pData,sibling.pData+(sibling.getCount()-1)*indexSize,indexSize);
            //移动最后一条记录
            //node.increaseCount(1);
            sibling.decreaseCount(1);
            memcpy(parent.pData+off-keySize,node.pData,keySize); //更新父节点key
        }
    }
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
#ifdef TEST_DELETE
            std::cout<<"右节点并入左节点: "<<std::endl;
            printNode(pBlock);
#endif
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
#ifdef TEST_DELETE
            std::cout<<"右节点并入左节点: "<<std::endl;
            printNode(pSibling);
#endif
        }
        //合并至左边节点
        bm.UnLock(pBlock);
        bm.UnLock(pSibling);
#ifdef TEST_DELETE
        std::cout<<"Adjust Parent "<<pParent->Offset<<std::endl;
        printNode(pParent);
#endif
#ifdef TEST_DELETE
        T delkey = getKey(pDivider);
        std::cout<<"delete :"<<delkey<<std::endl;
#endif
        adjustAfterDelete(pParent,pDivider); //调整
    }

    return true;
}


template <typename T>
void BPTree<T>::adjustAfterDelete(Block* pNode,char* delPos)
{
    int minInternal = (degree-1)/2;
    TreeNode node(pNode);
    if(1!=getOffset(node.pData))
        std::cerr<<"ERROR::PDATA ERROR!"<<std::endl;
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
#ifdef TEST_DELETE
        printNode(pNode);
#endif
            return;
    }
    if(node.id==root)
    {
        if(node.getCount()==1)
        {
            //printNode(pNode);
            bm.UnLock(pNode);
            emptyList.push_back(root);
            root = (int)getOffset(node.pData);//唯一的儿子成为root
            adjustParentInfo(root,-1);
            /*
            Block* pNewRoot = bm.GetBlock(indexName,root);
            bm.SetWritten(pNewRoot);

            TreeNode newRoot(pNewRoot);
            newRoot.setParent(-1);
            */

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
            //printNode(pNode);
            bm.UnLock(pNode);
#ifdef TEST_DELETE
            printNode(pNode);
#endif
            return;
        }
    }


    Block* pParent = bm.GetBlock(indexName,node.getParent());
    bm.SetWritten(pParent);
    bm.SetLock(pParent);
    TreeNode parent(pParent);
    
    int offInParent = searchSon(parent,node.id); //该节点对应父节点中的指针偏移量

    Block* pSibling;
    TreeNode sibling;
    bool leftSibling = false;

    char* pDivider;
    if(offInParent==0)  //父节点的第一个儿子
    {
        pSibling = bm.GetBlock(indexName,getOffset(parent.pData+offInParent+indexSize));
        leftSibling = false;
        pDivider = parent.pData+offInParent+TPTR_SIZE;
    }
    else if(offInParent==parent.getCount()*indexSize)   //最后一个儿子
    {
        pSibling = bm.GetBlock(indexName,getOffset(parent.pData+offInParent-indexSize));
        leftSibling = true;
        pDivider = parent.pData+offInParent-keySize;
    }
    else
    {
        Block* pl = bm.GetBlock(indexName,getOffset(parent.pData+offInParent-indexSize));
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
            pSibling = bm.GetBlock(indexName,getOffset(parent.pData+offInParent+indexSize));
            pDivider = parent.pData+offInParent+TPTR_SIZE;
        }
    }
    
    bm.SetLock(pSibling);
    bm.SetWritten(pSibling);
    sibling.attachTo(pSibling);

#ifdef TEST_DELETE
    std::cout<<"sibling: "<<std::endl;
    printNode(pSibling);
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

            adjustParentInfo(getOffset(node.pData),node.id);

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

            adjustParentInfo(getOffset(sibling.pData),node.id );

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
                adjustParentInfo(getOffset(p),sibling.id);
                p+=indexSize;
            }

            siblingEnd+=delPos-node.pData;
            //int copySize = node.getCount()*indexSize+TPTR_SIZE-(delPos-node.pData)-indexSize;
            p = delPos+indexSize+keySize;
            char* end = node.getCount()*indexSize+TPTR_SIZE+node.pData;
            memcpy(siblingEnd,p-keySize,end-p+keySize);    //删除点右
            while(p<end)
            {
                adjustParentInfo(getOffset(p),sibling.id);
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
            //node.increaseCount(1);
            end+=keySize;

            p = sibling.pData;
            char* siblingEnd = sibling.pData+sibling.getCount()*indexSize+TPTR_SIZE;
            memcpy(end,sibling.pData,siblingEnd-p);     //合并入左节点
            //调整父节点
            while(p<siblingEnd)
            {
                adjustParentInfo(getOffset(p),node.id);
                p+=indexSize;
            }
            emptyList.push_back(sibling.id);
            node.increaseCount(sibling.getCount());
            //T k = getKey(pDivider);
            //std::cout<<k;
            //printNode(pNode);
            
        }
        bm.UnLock(pSibling);
        bm.UnLock(pNode);
#ifdef TEST_DELETE
        T delkey = getKey(pDivider);
        std::cout<<"delete :"<<delkey<<std::endl;
#endif
        adjustAfterDelete(pParent,pDivider);
    }


}

template <typename T>
void BPTree<T>::adjustParentInfo(BlockIDType id, BlockIDType newp)
{
    Block* pBlock=bm.GetBlock(indexName,id);
    bm.SetWritten(pBlock);
    char* p = pBlock->data+BLOCKSIZE-sizeof(int)*2;
    memcpy(p,(char*)&newp,sizeof(BlockIDType));
}


template <typename T>
int BPTree<T>::searchSon(TreeNode& parent, const BlockIDType id)
{
    parent.reset();
    char* p = parent.pData;
    while(getOffset(p)!=id)
    {
        p+=indexSize;
    }
    return (p-parent.pData);
}

template <typename T>
void BPTree<T>::init(void)
{
    FILE* fp = fopen(indexName.c_str(), "w+");
    if(fp==NULL)
    {
        std::cout<<"File Create Error"<<std::endl;
    }
    fclose(fp);

    Block* pHead = bm.GetNewBlock(indexName, 0);
    bm.SetWritten(pHead);
    bm.DeleteFileBlock(indexName);
    IndexHead head(pHead);
    
    head.setNodeNum(1);   //NodeNumber
    head.setRoot(1);  //root node id
    head.setFirstLeaf(1);    //first Leaf id
    head.setKeySize(keySize);
    nodeNum = 1;
    root = 1;
    firstLeaf = 1;
    
    
    emptyList.clear();

    head.setEmptyBlockList(emptyList);
    //std::cout<<*(node.pCount)<<" "<<*(node.pParent)<<std::endl;
    
    Block* pBlock = bm.GetNewBlock(indexName,1);
    TreeNode node(pBlock);
    bm.SetWritten(pBlock);
    node.attachTo(pBlock);
    node.setCount(0);
    node.setParent(0);
    node.setLeaf(0);
    Changed = false;
}

template <typename T>
bool BPTree<T>::_modify(const T &oldkey, const T &newkey, const TuplePtr &newtupleptr)
{
    Block* pBlock = getKeyNode(oldkey);
    bm.SetWritten(pBlock);
    TreeNode node(pBlock);
    NodeSearchParse res;
    if(!searchBlock(pBlock,oldkey,res)) return false;
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


template <typename T>
OffsetType BPTree<T>::_search(const T &key)
{
    Block* pBlock = getKeyNode(key);
    return block_search(pBlock,key);
}


template <typename T>
OffsetType BPTree<T>::block_search(const Block* pBlock, const T &key)
{
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
            return getOffset(node.pData+middle*indexSize+keySize)+1;
        }
    }
    return NONE;
}


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


template <typename T>
void BPTree<T>::createNewFile(const std::string &filename, int keySize)
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
    TPTR_SIZE = sizeof(OffsetType);

    Block* pHead = bm.GetBlock(filename,0);
   // bm.SetWritten(pHead);

    IndexHead head(pHead);
    keySize = head.getKeySize();
    root = head.getRoot();
    firstLeaf = head.getFirstLeaf();
    nodeNum = head.getNodeNum();

    indexSize = keySize+TPTR_SIZE;
    degree = (BLOCKSIZE-sizeof(OffsetType)-sizeof(int)*3)/(indexSize) - 2;
    head.getEmptyBlockList(emptyList);
}

template <typename T>
bool BPTree<T>::_drop(const std::string &indexName)
{
    FILE* fp = fopen(indexName.c_str(), "w");
    if(fp==NULL)
    {
        std::cerr<<"ERROR::CANNOT OPEN FILE!"<<std::endl;
        return false;
    }
    fclose(fp);
    Changed = false;
    indexName = "";
    return true;
}

template <typename T>
bool BPTree<T>::_release(void)
{
    Changed = false;
    indexName = "";
    return true;
}

template <typename T>         //0:=  1:>  2:< 3:>=  4:<= 5 :<>, !!!-1 disable
bool BPTree<T>::range_search(const T lvalue, const int lclosed, 
                      const T rvalue, const int rclosed,
                      std::vector<TuplePtr>& tuplePtrs)
{
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

/*=====  End of BPTree  ======*/