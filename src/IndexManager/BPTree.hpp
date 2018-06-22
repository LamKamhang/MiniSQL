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

#define _cptr2int(x)  (*(int*)(x))
#define _cptr2uint(x) (*(unsigned int*)(x))
#define _data2cptr(x)  ((char*)(&x))

class SR
{
public:
    OffsetType tupleOffset;
    OffsetType offsetInNode;
    BlockIDType blockOffset;
    string indexName;
    bool isFound;

#ifdef DEBUG
    void printSR(void)
    {
        if(!isFound)
        {
            cout<<"Not Found"<<endl;;
            return;
        }
        cout<<"---------------------"<<endl;
        cout<<"IndexName: "<<indexName<<endl;
        cout<<"Block Offset: "<<blockOffset<<endl;
        cout<<"Tupple Offset: "<<tupleOffset<<endl;
        cout<<"Offset in Node: "<<offsetInNode<<endl;
        cout<<"---------------------"<<endl;
    }
#endif
};

class IndexHead{
public:
    IndexHead(Block* pHead):
        pEmptyId    (pHead->data), 
        PNodeNum    (pHead->data+BLOCKSIZE-INT_SIZE*1),
        PRoot       (pHead->data+BLOCKSIZE-INT_SIZE*2), 
        pFirstLeaf  (pHead->data+BLOCKSIZE-INT_SIZE*3), 
        pKeyType    (pHead->data+BLOCKSIZE-INT_SIZE*4), 
        pEmptyNum   (pHead->data+BLOCKSIZE-INT_SIZE*5)
    {};

    int getRoot()       {return _cptr2int(pRoot);};
    int getNodeNum()    {return _cptr2int(pNodeNum);};
    int getFirstLeaf()  {return _cptr2int(pFirstLeaf);};
    int getKeyType()    {return _cptr2int(pKeyType);};
    
    // 把内容拷贝到heap上指针的所指内容中
    void setRoot(int p)     {memcpy(pRoot,      _data2cptr(p), INT_SIZE);};
    void setNodeNum(int n)  {memcpy(pNodeNum,   _data2cptr(n), INT_SIZE);};
    void setFirstLeaf(int l){memcpy(pFirstLeaf, _data2cptr(l), INT_SIZE);};
    void setKeyType(int k)  {memcpy(pKeyType,   _data2cptr(k), INT_SIZE);};


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

    void setEmptyBlockList(list<int>& emptyList)
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
    char* pKeyType;

    char* pEmptyNum;
    void setEmptyNum(int n){memcpy(pEmptyNum, _data2cptr(n), INT_SIZE);};
    int  getEmptyNum(){return _cptr2int(pEmptyNum);};
};


class TreeNode{
public:
    // ignore the class of BPTree, left it to BPTree.
    char* pData;
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
    char* pCount;
    char* pParent;
    char* pIsLeaf;

    char* head;
};

class KV
{
public:
    int intKey;
    std::string stringKey;
    float floatKey;
    KV(int& ik):intKey(ik){};
    KV(float& fk):floatKey(fk){};
    KV(const std::string& sk):stringKey(sk){};
    KV(){};
    void assignTo(int& k)               {k = intKey;};
    void assignTo(float& k)             {k=floatKey;};
    void assignTo(const std::string& k) {k=stringKey;};
};

/*
 *   B+ Tree 
 *   with key type T
 *     [ BLOCKSIZE-sizeof(OffsetType)-sizeof(int) * 2 ] / (keySize + offsetSize)  - 1 
 */
template <class T>
class BPTree 
{

public:
    string indexName;
private:
    int degree;             // Degree of node
    BlockIDType root;
    BlockIDType firstLeaf;
    BufferManager& bm;
    //Index Info
    SqlValueType keytype;
    int keySize;
    int offsetSize;
    int indexSize;
    int nodeNum;
    bool Changed;
public:
    BPTree(string filename,int nodeDegree,SqlValueType type,BufferManager& bm_in);
    BPTree(BufferManager& bm);
    ~BPTree();
    /* Inline function */
    
    inline BlockIDType getRoot(){return root;}; 
    inline BlockIDType getFirstLeaf(){return firstLeaf;};
    inline bool isChanged(void){return Changed;};

    bool _delete(T oldkey);
    void _insert(T newkey,OffsetType toffset);    // Insert an index record
    bool _modify(T oldkey,T newkey,OffsetType newoffset);
    void _drop(string filename);
    void _release(void);
    OffsetType _search(T key);

    void init(void);

    void wiriteBack();
    void readFromFile(string filename);
    void createNewFile(string filename,SqlValueType type);
    bool range_search(T lower_bound,int Op1,T upper_bound,int Op2,list<OffsetType>& offsetList);
    Block* createNewNode(void);
    OffsetType block_search(Block* pBlock,T key);

    
    /*
    bool _delete(T oldkey);                       // Delete an index record
    void _drop();                                // Delete the whole tree
    bool _modify(T oldkey,T newkey,OffsetType newoffset=-1);  // Modify an index record*/

private:
    /*void deleteNode(BlockId bid);               // Delete a tree recursively, only used by _drop()
    void AdjustParent(BlockId bid);           // Adjust internalNode after deletetion
    int getSize(SqlValueType type);*/

    list<int> emptyList;    //空块

    void insert_in_leaf(Block* pBlock,T newkey,OffsetType newoffset);
    char* insert_in_internal(Block* pBlock,T newkey,BlockIDType bidLeft,BlockIDType bidRight);
    void adjustAfterDelete(Block* pNode,char* delPos);
    Block* getKeyNode(T key);
    void adjustParentInfo(BlockIDType id,BlockIDType newp);
    int searchSon(TreeNode& parent,BlockIDType id);
    //void convertKey(string key);

    T getKey(char* p);
    void writeKey(char* p,T newkey);
    OffsetType getOffset(char* p);
    void writeOffset(char* p,OffsetType offset);
    
    bool searchBlock(Block* pBlock,T key,SR& sr);


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
                cout<<key<<" ";
            }
            cout<<endl;
        }
        else
        {
            a.pData+=offsetSize;
            for(int i=0;i<a.getCount();a.pData+=indexSize,i++)
            {
                T key = getKey(a.pData);
                cout<<key<<" ";
            }
            cout<<endl;
        }
    };
    

    void showLeaf(void)
    {
        TreeNode a;
        int nextLeaf =firstLeaf;
        do{
            Block* pb = bm.GetBlock(indexName, nextLeaf);
            a.attachTo(pb);
            cout<<"[";
            for(int i=0;i<a.getCount();a.pData+=indexSize,i++)
            {
                T key =  getKey(a.pData);
                cout<<key<<" ";
            }
            cout<<"] ";
            nextLeaf = a.isLeaf();
        }while(a.isLeaf()>0);
        cout<<"\n"<<endl;
        
    };

    void showEmpty()
    {
        list<int>::iterator it;
        for(it=emptyList.begin();it!=emptyList.end();it++)
        {
            cout<<*it<<" ";
        }
        cout<<endl;
    };
#endif

};





/*=================================
=            BPTree            =
=================================*/



template <class T>
BPTree<T>::~BPTree()
{
    //cout<<nodeNum<<" "<<root<<" "<<firstLeaf<<" "<<keytype<<endl;
}


/**
 *  Constructor fot B+ Tree
 *  @ param int the degree of tree
 *  @int size of key in byte
 */

template <class T>
BPTree<T>::BPTree(BufferManager& bm_in):bm(bm_in)
{
    indexName = "";
    Changed = false;
}

/* Used for test */
template <class T>
BPTree<T>::BPTree(string filename,int nodeDegree,SqlValueType type,BufferManager& bm_in):bm(bm_in){
    TreeNode node;
    indexName = filename;
    bm = bm_in;
    keytype = type;
    offsetSize = sizeof(OffsetType);

    if(type==SQL_INT)
    {   
        keySize = sizeof(int);
    }
    else if(type==SQL_FLOAT)
    {
        keySize = sizeof(float);
    }
    else 
    {
        keySize = keytype + 1;
    }
    indexSize = keySize+offsetSize;
    degree = (BLOCKSIZE-sizeof(OffsetType)-sizeof(int)*3)/(indexSize) - 2;
    //degree = 5;
    root = 1;
    firstLeaf = 1;
    nodeNum = 1;

    FILE* fp = fopen(filename.c_str(), "w");
    if(fp==NULL)
    {
        std::cerr << "File Create Error" << std::endl;
    }
    fclose(fp);

    Block* pHead = bm.GetNewBlock(filename, 0);
    bm.SetWritten(pHead);

    IndexHead head(pHead);
    
    head.setNodeNum(1);   //NodeNumber
    head.setRoot(1);  //root node id
    head.setFirstLeaf(1);    //first Leaf id
    head.setKeyType(keytype);
    emptyList.clear();

    head.setEmptyBlockList(emptyList);
    //cout<<*(node.pCount)<<" "<<*(node.pParent)<<endl;
    
    Block* pBlock = bm.GetNewBlock(filename,1);
    bm.SetWritten(pBlock);
    node.attachTo(pBlock);
    node.setCount(0);
    node.setParent(-1);
    node.setLeaf(0);
    Changed = false;
    //cout<<*(node.pCount)<<" "<<*(node.pParent)<<endl;
}


template <class T>
Block* BPTree<T>::getKeyNode(T key)
{
    TreeNode node;
    Block* pBlock = bm.GetBlock(indexName,root);
    if(pBlock==NULL)
    {
        cerr<<"Buffer Error"<<endl;
        exit(-1);
    }
    bm.SetWritten(pBlock);

    node.attachTo(pBlock);
    int bid = root;
    while(node.isLeaf()<0)
    {
        node.pData += offsetSize;   //Move to first key
        for(int i=node.getCount();i>0;node.pData += indexSize,i--)
        {
            if(getKey(node.pData)>key)
            {
                break;
            }
        }
        node.pData -= offsetSize;
        bid = (int)getOffset(node.pData);
        pBlock = bm.GetBlock(indexName, bid);
        bm.SetWritten(pBlock);
        node.attachTo(pBlock);
    }
    return pBlock;
}



template <class T>
void BPTree<T>::insert_in_leaf(Block* pBlock,T newkey,OffsetType newoffset)
{
    TreeNode node;
    node.attachTo(pBlock);
    bm.SetWritten(pBlock);

    char* p = node.pData+(node.getCount()-1) * indexSize;
    while(p>=node.pData)
    {
        T k = getKey(p);
        if(k<=newkey)
        {
            break;
        }
        memcpy(p+indexSize,p,indexSize);
        p-=indexSize;
    }
    p+=indexSize;
    writeKey(p,newkey);
    writeOffset(p+keySize,newoffset);

    node.increaseCount(1);
#ifdef OUTPUT
    cout<<"Insert In Leaf "<<pBlock->Offset<<": "<<newkey<<endl;
    printNode(pBlock);
#endif
}

template <class T>
char* BPTree<T>::insert_in_internal(Block* pBlock,T newkey,BlockIDType bidLeft,BlockIDType bidRight)
{
    TreeNode node;
    node.attachTo(pBlock);
    bm.SetWritten(pBlock);
    char* p = node.pData+(node.getCount()-1) * indexSize+offsetSize;
    char* end = node.pData+offsetSize;
    while(p>=end)
    {
        T k = getKey(p);
        if(k<=newkey)
        {
            break;
        }
        memcpy(p+indexSize,p,indexSize);
        p-=indexSize;
    }
    p+=indexSize;
    writeKey(p,newkey);
    writeOffset(p+keySize,bidRight);
    writeOffset(p-offsetSize,bidLeft);

   // adjustParentInfo(bidLeft,pBlock->Offset);
   // adjustParentInfo(bidRight,pBlock->Offset);

    node.increaseCount(1);
#ifdef OUTPUT
    cout<<"Insert In Internal "<<pBlock->Offset<<": "<<newkey<<endl;
    printNode(pBlock);
#endif
    return p;
}


template <class T>
void  BPTree<T>::_insert(T newkey,OffsetType newoffset)
{
    Changed = true;
    Block* pBlock = getKeyNode(newkey);
    
    TreeNode node(pBlock);
    insert_in_leaf(pBlock,newkey,newoffset);
    if(node.getCount()<degree)
    {
        return ;
    }

    bm.SetLock(pBlock);

    Block* pNewBlock = createNewNode();

    bm.SetWritten(pNewBlock);
    bm.SetLock(pNewBlock);  //锁

    int splitPos = (degree+1)/2;
    memcpy(pNewBlock->data,pBlock->data+splitPos*indexSize,(node.getCount()-splitPos)*indexSize);

    TreeNode newNode(pNewBlock);
    bm.SetWritten(pNewBlock);

    node.attachTo(pBlock);
    bm.SetWritten(pBlock);

    newNode.setCount(node.getCount()-splitPos);
    node.setCount(splitPos);
    newNode.setLeaf(node.isLeaf());
    node.setLeaf(newNode.id);
    newNode.setParent(node.getParent());
#ifdef TEST_DELETE
    cout<<"A's parent: "<<node.getParent()<<endl;
    cout<<"B's parent: "<<newNode.getParent()<<endl;
#endif
    T upperkey = getKey(newNode.pData);
    

    if(node.getParent()<0) //分裂节点为根节点
    {

        Block* pRootBlock=createNewNode();

        TreeNode newRoot(pRootBlock);
        bm.SetWritten(pRootBlock);
    
        newRoot.setParent(-1);
        newRoot.setCount(0);
        newRoot.setLeaf(-1);

        root = newRoot.id;  //更新root

        node.setParent(root);
        newNode.setParent(root);
    
        insert_in_internal(pRootBlock,upperkey,node.id,newNode.id);
#ifdef OUTPUT
        cout<<"in new root: ";
        printNode(pRootBlock);
#endif
        bm.UnLock(pBlock);
        bm.UnLock(pNewBlock);  //解锁
#ifdef OUTPUT
        cout<<"Split "<<pBlock->Offset<<endl;
        cout<<"Left:";
        printNode(pBlock);
        cout<<"id="<<node.id<<" ";
        cout<<"Parent="<<node.getParent()<<endl;
       
        cout<<"Right:";
        printNode(pNewBlock);
        cout<<"id="<<newNode.id<<" ";
        cout<<"Parent="<<newNode.getParent()<<endl;
#endif
        return;
    }
    bm.UnLock(pBlock);
    bm.UnLock(pNewBlock);  //解锁
    
   
    
#ifdef OUTPUT
    cout<<"Split "<<pBlock->Offset<<endl;
    cout<<"left: id="<<node.id<<" "<<endl;
    printNode(pBlock);
    cout<<"Parent="<<node.getParent()<<endl;
    cout<<"right: id="<<newNode.id<<" "<<endl;;
    printNode(pNewBlock);
    cout<<"Parent="<<newNode.getParent()<<endl;
#endif

    pBlock = bm.GetBlock(indexName,node.getParent());

    insert_in_internal(pBlock, upperkey, node.id, newNode.id);
#ifdef OUTPUT
    cout<<"in parent node: ";
    printNode(pBlock);
#endif
    
    node.attachTo(pBlock);
    bm.SetWritten(pBlock);
    bm.SetLock(pBlock);

    Block* parent = NULL;
    while(node.getCount()==degree)
    {
        
        pNewBlock = createNewNode();
        bm.SetWritten(pNewBlock);
        bm.SetLock(pNewBlock);

        newNode.attachTo(pNewBlock);

        int splitPos = (degree)/2;

        upperkey = getKey(node.pData+(splitPos)*indexSize+offsetSize);  //移到上一层节点的key

        memcpy(newNode.pData,node.pData+(splitPos+1)*indexSize,(node.getCount()-splitPos-1)*indexSize+offsetSize);
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
        
        if(node.getParent()<0)
        {
            Block* pRootBlock = createNewNode();
            bm.SetWritten(pRootBlock);

            TreeNode newRoot(pRootBlock);
            newRoot.setParent(-1);
            newRoot.setCount(0);
            newRoot.setLeaf(-1);
            root = newRoot.id;

            node.setParent(root);
            newNode.setParent(root);

            insert_in_internal(pRootBlock,upperkey,node.id,newNode.id);
            bm.UnLock(pBlock);
            bm.UnLock(pNewBlock);
            return;
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


template <class T>
T BPTree<T>::getKey(char* p)
{
    
    if(keytype==SQL_INT||keytype==SQL_FLOAT)
    {
        T val;
        memcpy((char*)&val, p, keySize);
        return val;
    }
    else
    {
        KV k;
        k.stringKey = p;
        T val;
        k.assignTo(val);
        return val;
    }
}

template <class T>
OffsetType BPTree<T>::getOffset(char* p)
{
    //OffsetType offset;
    //memcpy((char*)&offset,p,offsetSize);
    //return offset;

    return *(OffsetType*)p;
}

template <class T>
void BPTree<T>::writeKey(char* p,T newkey)
{
    
    if(keytype==SQL_INT)
    {
        memcpy(p,(char*)&newkey,keySize);
    }
    else if(keytype==SQL_FLOAT)
    {
        memcpy(p,(char*)&newkey,keySize);
    }
    else    //stringn
    {
        KV k(newkey);
        memcpy(p, k.stringKey.c_str(),keySize);
    }
}


template <class T>
void BPTree<T>::writeOffset(char* p,OffsetType newoffset)
{
    memcpy(p,(char*)&newoffset,offsetSize);
}


template <class T>
bool BPTree<T>::searchBlock(Block* pBlock,T key,SR& sr)
{
    TreeNode node(pBlock);
    int i=0;
    int j = node.getCount()-1;
    int middle = 0;
    sr.isFound=false;
    while(i<=j)
    {
        middle = (i+j)>>1;
        T val = getKey(node.pData+middle*indexSize);
        if(val>key)
            j = middle - 1;
        else if(val<key)
            i = middle + 1;
        else if(val==key)  {
            sr.isFound=true;
            break;
        }
    }
    sr.indexName = indexName;
    sr.blockOffset = node.id;
    sr.tupleOffset = getOffset(node.pData+indexSize*middle+keySize);
    sr.offsetInNode = middle*indexSize;
    return sr.isFound;
}


template <class T>
bool BPTree<T>::_delete(T oldkey)
{/*
    Block* proot = bm.GetBlock(indexName,root);
    TreeNode no(proot);
    size_t o;
    if(1!=(o=getOffset(proot->data))&&no.isLeaf()<0)
        cout<<"Error"<<o<<endl<<endl;
    
    */
    int minLeafNum = degree/2;
    Block* pBlock = getKeyNode(oldkey);
    SR sr;
    if(false==searchBlock(pBlock,oldkey,sr))
    {
        //printNode(pBlock);
        cout<<"No this key:"<<oldkey<<endl;
        return false;
    }
    Changed = true;
#ifdef TEST_DELETE
    cout<<"find key: "<<oldkey<<endl;
    sr.printSR();
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
        char* p = node.pData+sr.offsetInNode;
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
        char* p = node.pData+sr.offsetInNode;
        char* end = node.pData+(node.getCount()-1)*indexSize;
        while(p<end)
        {
            memcpy(p,p+indexSize,indexSize);    //记录向前移动
            p+=indexSize;
        }
        node.decreaseCount(1);
#ifdef TEST_DELETE
        cout<<"OK"<<endl;
        printNode(pBlock);
#endif
        /*
        ////////!!!!!!!!!!!!
        proot = bm.GetBlock(indexName,root);
        no.attachTo(proot);
        if(1!=(o=getOffset(proot->data))&&no.isLeaf()<0)
            cout<<"Error!!!!!"<<o<<endl<<endl;
        else cout<<o<<endl;*/
        return true;
    }
#ifdef TEST_DELETE
    cout<<"Block "<<node.id<<" Need to adjust"<<endl;
    printNode(pBlock);
#endif
    //需要调整
    bm.SetLock(pBlock);

    Block* pParent = bm.GetBlock(indexName,node.getParent());   //获取父节点
    TreeNode parent(pParent);
#ifdef TEST_DELETE
    cout<<"Parent: "<<endl;
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
    cout<<"Choose sibling: "<<pSibling->Offset<<endl;
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
            char* p = node.pData+sr.offsetInNode;
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
            
            memcpy(parent.pData+off+offsetSize,sibling.pData,keySize);   //更新父节点key

        }
        else        //OK
        {
            char* p = node.pData + sr.offsetInNode;   //删除点前段右移
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
        /*
        ///////////////!!!!
        proot = bm.GetBlock(indexName,root);
        no.attachTo(proot);
        if(1!=(o=getOffset(proot->data))&&no.isLeaf()<0)
            cout<<"Error!!!!!"<<o<<endl<<endl;
        else cout<<o<<endl;
         */
    }
    else        
    {        
        if(!leftSibling)    //ok
        {
            pDivider = parent.pData+off+offsetSize;
            char* p = node.pData+sr.offsetInNode;
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
            cout<<"右节点并入左节点: "<<endl;
            printNode(pBlock);
#endif
        }
        else        //OK
        {
            pDivider = parent.pData+off-keySize;
            char* p = sibling.pData+sibling.getCount()*indexSize;   //左兄弟末尾
            memcpy(p, node.pData,sr.offsetInNode);  //删除点前段

            memcpy(p+sr.offsetInNode,node.pData+sr.offsetInNode+indexSize,
                (node.getCount()-1)*indexSize-sr.offsetInNode);
            sibling.increaseCount(node.getCount()-1);
            sibling.setLeaf(node.isLeaf());
            emptyList.push_back(node.id); //删除右节点
#ifdef TEST_DELETE
            cout<<"右节点并入左节点: "<<endl;
            printNode(pSibling);
#endif
        }
        //合并至左边节点
        bm.UnLock(pBlock);
        bm.UnLock(pSibling);
#ifdef TEST_DELETE
        cout<<"Adjust Parent "<<pParent->Offset<<endl;
        printNode(pParent);
#endif
#ifdef TEST_DELETE
        T delkey = getKey(pDivider);
        cout<<"delete :"<<delkey<<endl;
#endif
        adjustAfterDelete(pParent,pDivider); //调整
    }
    /*
    ////////////!!!!!
    proot = bm.GetBlock(indexName,root);
    no.attachTo(proot);
    if(1!=(o=getOffset(proot->data))&&no.isLeaf()<0)
        cout<<"Error!!!!!"<<o<<endl<<endl;
    else cout<<o<<endl;
    */
    return true;
}


template <class T>
void BPTree<T>::adjustAfterDelete(Block* pNode,char* delPos)
{
    int minInternal = (degree-1)/2;
    TreeNode node(pNode);
    if(1!=getOffset(node.pData))
        cout<<"Error!!!!!!"<<endl;
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
            
            char* end = node.pData+(node.getCount()-1)*indexSize+offsetSize;
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
        pDivider = parent.pData+offInParent+offsetSize;
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
            pDivider = parent.pData+offInParent+offsetSize;
        }
    }
    
    bm.SetLock(pSibling);
    bm.SetWritten(pSibling);
    sibling.attachTo(pSibling);

#ifdef TEST_DELETE
    cout<<"sibling: "<<endl;
    printNode(pSibling);
#endif
    if(sibling.getCount()>minInternal)
    {
        if(leftSibling) //ok
        {
            char* p = delPos;
            char* end = node.pData+offsetSize;
            while(p>end)
            {
                memcpy(p,p-indexSize,indexSize);    //删除点右边右移
                p-=indexSize;
            }

            p+=keySize;
            memcpy(p,p-indexSize,offsetSize);   //最左端指针

            p = sibling.pData+(sibling.getCount())*indexSize;
            memcpy(node.pData+offsetSize,pDivider,keySize); //父节点分割的key放入节点
            memcpy(node.pData,p,offsetSize);       //兄弟节点最右边指针加入该节点
            memcpy(pDivider,p-keySize,keySize);                 //最右边key变为父节点中新的key

            adjustParentInfo(getOffset(node.pData),node.id);

            sibling.decreaseCount(1);
        }
        else        //ok
        {
            char* p = delPos;
            char* end = node.pData+(node.getCount()-1)*indexSize+offsetSize;
            while(p<end)
            {
                memcpy(p,p+indexSize,indexSize);    //删除点右边 左移
                p+=indexSize;
            }

            memcpy(end,pDivider,keySize);   //父节点key下移
            memcpy(end+keySize,sibling.pData,offsetSize);       //指针加入左边
            memcpy(pDivider,sibling.pData+offsetSize,keySize);  //右节点  最左边key上移至父节点

            adjustParentInfo(getOffset(sibling.pData),node.id );

            p = sibling.pData;
            end = sibling.pData+(sibling.getCount()-1)*indexSize;
            while(p<end)
            {
                memcpy(p,p+indexSize,indexSize);
                p+=indexSize;
            }
            memcpy(end,end+indexSize,offsetSize);   //最右指针
            sibling.decreaseCount(1);
        }
    }
    else
    {
        if(leftSibling) //ok
        {
            char* p = node.pData;
            char* siblingEnd = sibling.pData+sibling.getCount()*indexSize+offsetSize;
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
            //int copySize = node.getCount()*indexSize+offsetSize-(delPos-node.pData)-indexSize;
            p = delPos+indexSize+keySize;
            char* end = node.getCount()*indexSize+offsetSize+node.pData;
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
            char* end = node.pData+(node.getCount()-1)*indexSize+offsetSize;
            while(p<end)
            {
                memcpy(p,p+indexSize,indexSize);
                p+=indexSize;
            }
            memcpy(end,pDivider,keySize);
            //node.increaseCount(1);
            end+=keySize;

            p = sibling.pData;
            char* siblingEnd = sibling.pData+sibling.getCount()*indexSize+offsetSize;
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
            //cout<<k;
            //printNode(pNode);
            
        }
        bm.UnLock(pSibling);
        bm.UnLock(pNode);
#ifdef TEST_DELETE
        T delkey = getKey(pDivider);
        cout<<"delete :"<<delkey<<endl;
#endif
        adjustAfterDelete(pParent,pDivider);
    }


}

template <class T>
void BPTree<T>::adjustParentInfo(BlockIDType id,BlockIDType newp)
{
    Block* pBlock=bm.GetBlock(indexName,id);
    bm.SetWritten(pBlock);
    char* p = pBlock->data+BLOCKSIZE-sizeof(int)*2;
    memcpy(p,(char*)&newp,sizeof(BlockIDType));
}


template <class T>
int BPTree<T>::searchSon(TreeNode& parent,BlockIDType id)
{
    parent.reset();
    char* p = parent.pData;
    while(getOffset(p)!=id)
    {
        p+=indexSize;
    }
    return (p-parent.pData);
}

template <class T>
void BPTree<T>::init(void)
{
    FILE* fp = fopen(indexName.c_str(), "w+");
    if(fp==NULL)
    {
        cout<<"File Create Error"<<endl;
    }
    fclose(fp);

    Block* pHead = bm.GetNewBlock(indexName, 0);
    bm.SetWritten(pHead);
    bm.DeleteFileBlock(indexName);
    IndexHead head(pHead);
    
    head.setNodeNum(1);   //NodeNumber
    head.setRoot(1);  //root node id
    head.setFirstLeaf(1);    //first Leaf id
    head.setKeyType(keytype);
    nodeNum = 1;
    root = 1;
    firstLeaf = 1;
    
    
    emptyList.clear();

    head.setEmptyBlockList(emptyList);
    //cout<<*(node.pCount)<<" "<<*(node.pParent)<<endl;
    
    Block* pBlock = bm.GetNewBlock(indexName,1);
    TreeNode node(pBlock);
    bm.SetWritten(pBlock);
    node.attachTo(pBlock);
    node.setCount(0);
    node.setParent(-1);
    node.setLeaf(0);
    Changed = false;
}

template <class T>
bool BPTree<T>::_modify(T oldkey,T newkey,OffsetType newoffset)
{
    Block* pBlock = getKeyNode(oldkey);
    bm.SetWritten(pBlock);
    TreeNode node(pBlock);
    SR sr;
    if(!searchBlock(pBlock,oldkey,sr)) return false;
    Changed = true;
    char* p = sr.offsetInNode+node.pData;
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
            writeKey(p,newkey);
            writeOffset(p+keySize,newoffset);
        }
        else
        {
            while(p>node.pData&&newkey<getKey(p-indexSize))
            {
                memcpy(p,p-indexSize,indexSize);
                p-=indexSize;
            }
            writeKey(p,newkey);
            writeOffset(p+keySize,newoffset);
        }
        return true;
    }

    _delete(oldkey);
   // showLeaf();
    _insert(newkey,newoffset);
    //showLeaf();
    return true;
}
template <class T>
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


template <class T>
OffsetType BPTree<T>::_search(T key)
{
    Block* pBlock = getKeyNode(key);
    return block_search(pBlock,key);
}


template <class T>
OffsetType BPTree<T>::block_search(Block* pBlock,T key)
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
    return 0;
}


template <class T>
void BPTree<T>::writeToDisk()
{
    Block* pHead = bm.GetBlock(indexName, 0);
    bm.SetWritten(pHead);
    IndexHead head(pHead);
    
    head.setNodeNum(nodeNum);   //NodeNumber
    head.setRoot(root);  //root node id
    head.setFirstLeaf(firstLeaf);    //first Leaf id
    head.setKeyType(keytype);

    head.setEmptyBlockList(emptyList);
    Changed = false;
    //cout<<*(node.pCount)<<" "<<*(node.pParent)<<endl;
}


template <class T>
void BPTree<T>::createNewFile(string filename,SqlValueType type)
{
    if(Changed==true)
        writeToDisk();

    indexName = filename;
    keytype = type;

    offsetSize = sizeof(OffsetType);
    if(type==SQL_INT)
    {   
        keySize = sizeof(int);
    }
    else if(type==SQL_FLOAT)
    {
        keySize = sizeof(float);
    }
    else 
    {
        keySize = keytype + 1;
    }
    indexSize = keySize+offsetSize;
    degree = (BLOCKSIZE-sizeof(OffsetType)-sizeof(int)*3)/(indexSize) - 2;

    root = 1;
    firstLeaf = 1;
    nodeNum = 1;

    FILE* fp = fopen(filename.c_str(), "w");
    if(fp==NULL)
    {
        cout<<"File Create Error"<<endl;
    }
    fclose(fp);

    Block* pHead = bm.GetNewBlock(filename, 0);
    bm.SetWritten(pHead);
    IndexHead head(pHead);
    
    head.setNodeNum(1);   //NodeNumber
    head.setRoot(1);  //root node id
    head.setFirstLeaf(1);    //first Leaf id
    head.setKeyType(keytype);

    emptyList.clear();
    head.setEmptyBlockList(emptyList);
    //cout<<*(node.pCount)<<" "<<*(node.pParent)<<endl;
    
    Block* pBlock = bm.GetNewBlock(filename,1);
    TreeNode node(pBlock);
    bm.SetWritten(pBlock);
    node.setCount(0);
    node.setParent(-1);
    node.setLeaf(0);
}


template <class T>
void BPTree<T>::readFromFile(string filename)
{
    if(indexName==filename)
    {
        return;
    }
    else if(Changed==true)
        writeToDisk();

    indexName = filename;
    offsetSize = sizeof(OffsetType);

    Block* pHead = bm.GetBlock(filename,0);
   // bm.SetWritten(pHead);

    IndexHead head(pHead);
    keytype = head.getKeyType();
    root = head.getRoot();
    firstLeaf = head.getFirstLeaf();
    nodeNum = head.getNodeNum();

    if(keytype==SQL_INT)
    {   
        keySize = sizeof(int);
    }
    else if(keytype==SQL_FLOAT)
    {
        keySize = sizeof(float);
    }
    else 
    {
        keySize = keytype + 1;
    }
    indexSize = keySize+offsetSize;
    degree = (BLOCKSIZE-sizeof(OffsetType)-sizeof(int)*3)/(indexSize) - 2;
    head.getEmptyBlockList(emptyList);
}

template <class T>
void BPTree<T>::_drop(string filename)
{
    FILE* fp = fopen(filename.c_str(), "w");
    if(fp==NULL)
    {
        cout<<"File Create Error"<<endl;
    }
    fclose(fp);
    Changed = false;
    indexName = "";
}

template <class T>
void BPTree<T>::_release(void)
{
    Changed = false;
    indexName = "";
}

template <class T>         //0:=  1:>  2:< 3:>=  4:<= 5 :<>, !!!-1 disable
bool BPTree<T>::range_search(T lower_bound,int Op1,T upper_bound,int Op2,list<OffsetType>& offsetList)
{
    if(Op1>0&&Op2>0)
    {
        Block* pBlock = getKeyNode(lower_bound);
        TreeNode node;
        BlockIDType bid = pBlock->Offset;
        while(bid>0)
        {
            node.attachTo(pBlock);
            char* p = node.pData;;
            char* end = node.pData+indexSize*(node.getCount()-1);
            
            T val;
            while((val=getKey(p))<lower_bound&&p<=end)
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
            while(p<=end&&(val=getKey(p))<=upper_bound)
            {
                offsetList.push_back(getOffset(p+keySize));
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
    else if(Op1>0&&Op2<=0)
    {
        Block* pBlock = getKeyNode(lower_bound);
        TreeNode node;
        BlockIDType bid = pBlock->Offset;
        while(bid>0)
        {
            node.attachTo(pBlock);
            char* p = node.pData;;
            char* end = node.pData+indexSize*(node.getCount()-1);
            
            T val;
            while((val=getKey(p))<lower_bound&&p<=end)
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
                offsetList.push_back(getOffset(p+keySize));
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
    else if(Op1<=0&&Op2>0)
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
            while((val=getKey(p))<=upper_bound&&p<=end)
            {
                offsetList.push_back(getOffset(p+keySize));
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