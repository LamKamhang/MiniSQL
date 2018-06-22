/*
 * File: BPTree.hpp
 * Version: 1.4
 * Author: kk
 * Created Date: Sat Jun  2 20:04:21 DST 2018
 * Modified Date: Mon Jun 11 22:37:24 DST 2018
 * Modified Date: Fri Jun 15 21:10:18 DST 2018
 * Modified Date: Sat Jun 16 01:07:47 DST 2018
 * -------------------------------------------
 * miniSQL的IndexManager需要用到的数据结构B+树定义
 * 实现B+树的基本操作，插入，删除，合并，分裂
 * 为了通用性，使用模板编程方式。
 * version 1.2 去除Union声明，联合中不能使用类，需要保持对内存相同的操作
 * version 1.3 修正inner node的split方式，上浮的key不保存在inner node中
 * version 1.4 修正delete的错误以及二分搜索的性能问题, 
 *             增加B+树从文件中读入以及写入文件的功能，成为文件B+树
 */

#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include "../interface.h"

using namespace MINISQL_BASE;


/*
 * class: BPTreeNode
 * Version: 1.0
 * Author: kk
 * Created Date: Sat Jun  2 20:04:21 DST 2018
 * -------------------------------------------
 * B+树的结点类，暂时不是很清楚具体的结构应该怎样，
 * 所以采用模板类的方式进行设计，实现通用性。
 * 结点的接口包括：
 * 构造函数（degree， isLeaf）
 * 搜索，增加，移除，是否为根节点，是否为叶结点，分裂
 * bool search(const T &key, int &index) const
 *      返回的下标在index变量里，返回值表示是否找到
 *      相同键值的key，从0开始
 */

template<typename T>
class BPTreeNode {
public:
    // constructor and destructors
    BPTreeNode() = default;

    BPTreeNode(int degree, bool isLeaf);

    ~BPTreeNode() {};

    bool search(const T &key, int &index) const;

    int add(const T &key);

    int add(const T &key, TuplePtr offset);

    BPTreeNode *split(T &key);

    void removeAt(int index);

    bool isRoot() const { return parent == nullptr; }

    bool isLeaf() const { return leaf; }
    
    int getDegree() const { return degree; }

    int getCount() const { return cnt; }
    
    std::vector<T> keys;    
    std::vector<BPTreeNode<T> *> children;
    std::vector<TuplePtr> keyOffset;
    BPTreeNode *parent, *sibling;
    int cnt;

#   ifdef DEBUG
    void showKeys(int id) {
        std::cout << "Keys [" << id << "]: ";
        for (int i = 0; i < cnt; i++) {
            std::cout << keys[i] << " ";
        }
        std::cout << std::endl;
    }
#   endif

private:
    /* 
     * degree为扇出数
     * cnt为该结点的key值数量
     */
    int degree;
    bool leaf;
    bool binarySearch(const T &key, int &index) const;
};

template<typename T>
BPTreeNode<T>::BPTreeNode(int degree, bool isLeaf) : 
    degree(degree), leaf(isLeaf), cnt(0), parent(nullptr), sibling(nullptr) 
{
    // add one more space for split
    keys.resize(degree);
    if (isLeaf)
        keyOffset.resize(degree);
    else
        children.resize(degree + 1);
}

template<typename T>
bool BPTreeNode<T>::search(const T &key, int &index) const {
    if (cnt == 0) {
        index = 0;
        return false;
    }
    if (keys[0] > key) {
        index = 0;
        return false;
    }
    if (keys[cnt - 1] < key) {
        index = cnt;
        return false;
    }
    return binarySearch(key, index);
}

template<typename T>
bool BPTreeNode<T>::binarySearch(const T &key, int &index) const {
    int left = 0, right = cnt - 1;
    while (left < right) {
        index = (right + left) / 2;
        if (keys[index] < key) 
        {
            left = ++index;
        }
        else if (keys[index] > key)
        {
            right = index;
        }
        else
        {
            return true;
        }
    }
    return false;
}

template<typename T>
BPTreeNode<T> *BPTreeNode<T>::split(T &key) {
    BPTreeNode<T> *newNode = new BPTreeNode<T>(degree, leaf);
    
    /* 
     * notice that: ceil[(degree - 1)/2] means
                                            degree  value
                                            2k+1 --> k
                                            2k   --> k
     * which is the same with degree / 2
     */
    int minimal = degree / 2;
    key = keys[minimal];    // 上浮的key

    // 分裂根据是否为leaf区分，keys对半分，新结点key个数 >= 旧结点key个数
    if (leaf) {
        for (int i = minimal; i < degree; i++) {
            newNode->keys[i - minimal] = keys[i];
            newNode->keyOffset[i - minimal] = keyOffset[i];
        }
        // keep the sibling link in the last pointer
        newNode->sibling = sibling;
        sibling = newNode;
		newNode->cnt = degree - minimal;
    } else {
        for (int i = minimal+1; i < degree; i++) {
            newNode->keys[i - minimal - 1] = keys[i];
            newNode->children[i - minimal - 1] = children[i];
            children[i]->parent = newNode;
            children[i] = nullptr;
        }
        // inner node have one more pointer than keys.
        newNode->children[degree - minimal - 1] = children[degree];
        children[degree]->parent = newNode;
        children[degree] = nullptr;
		newNode->cnt = degree - minimal - 1;
    }
    // the old node still has minimal keys,
    // while the new node has degree - minimal keys
    cnt = minimal;
    

    // the new node has the same parent with old node
    // we should also give its child identification to its parent.
    // children addition fix in the cascade insert function.
    newNode->parent = parent;
    return newNode;
}

template<typename T>
int BPTreeNode<T>::add(const T &key) {
    int index;
    bool keyExists = search(key, index);
    if (keyExists) {
        std::cerr << "Key is not unique!" << std::endl;
        exit(10);
    }
    for (int i = cnt; i > index; i--) {
        keys[i] = keys[i - 1];
        children[i + 1] = children[i];
    }
    keys[index] = key;
    // children[index + 1]这个孩子的处理留在外面
    // 因为根节点和inner结点对于这个处理不一致，
    // 为了方便起见则在调用处进行处理。
    // 这里把处理的下标作为返回值返回。
    children[index + 1] = nullptr;
    cnt++;
    return index;
}

template<typename T>
int BPTreeNode<T>::add(const T &key, TuplePtr offset) {
    int index;
    bool keyExists = search(key, index);
    if (keyExists) {
        std::cerr << "Key is not unique!" << std::endl;
        exit(10);
    }
    for (int i = cnt; i > index; i--) {
        keys[i] = keys[i - 1];
        keyOffset[i] = keyOffset[i - 1];
    }
    // 叶子结点的key与tuplePtr是捆绑的
    keys[index] = key;
    keyOffset[index] = offset;
    cnt++;
    return index;
}

template<typename T>
void BPTreeNode<T>::removeAt(int index) {
    if (leaf)
    {
        for (int i = index; i < cnt - 1; ++i)
        {
            keys[i] = keys[i + 1];
            keyOffset[i] = keyOffset[i+1];
        }
        // clear the content, for more safe.
        keyOffset[cnt-1] = TuplePtr();
        keys[cnt-1] = T();
    }
    else
    {
        // 对于inner node而言，在删除合并结点的时候，key的右指针是已经没用了
        for (int i = index; i < cnt - 1; ++i)
        {
            keys[i] = keys[i + 1];
            children[i + 1] = children[i + 2];
        }
        keys[cnt - 1] = T();
        children[cnt] = nullptr;
    }
    cnt--;
}



template<typename T>
struct NodeSearchParse {
    int index;
    BPTreeNode<T> *node;
};

template<typename T>
class BPTree {
public:
    typedef BPTreeNode<T> *TreeNode;

    BPTree(std::string fileName, int degree, bool read = false);

    ~BPTree();

    TreeNode getHeadNode() const { return head; }

    TuplePtr find(const T &key);

    std::vector<TuplePtr> findRange(const T &key, const Operator op);

    NodeSearchParse<T> findNode(const T &key);

    bool insert(const T &key, TuplePtr offset);

    TuplePtr remove(const T &key);

    std::vector<TuplePtr> removeRange(const T &key, const Operator op);
    
    bool remove(const T &key);

#   ifdef DEBUG
    void showTree() {
        std::cout << "Tree height: " << level << std::endl;
        int numofnodes = 1;
        std::queue<TreeNode> nodes;
        nodes.push(root);
        for (int i = 0; i < level; ++i)
        {            
            std::cout << "Level: " << i << std::endl;
            int temp = numofnodes;
            numofnodes = 0;
            for (int j = 0; j < temp; ++j)
            {
                TreeNode node = nodes.front();
                nodes.pop();
                if (node)
                {
                    if (node->isLeaf())
                    {
                        node->showKeys(j);
                    }
                    else
                    {
                        int numofchild = node->cnt + 1;
                        numofnodes += numofchild;
                        for (int k = 0; k < numofchild; ++k)
                        {
                            nodes.push(node->children[k]);
                        }
                        node->showKeys(j);
                    }
                }
            }
            std::cout << "------------------------------" << std::endl;
        }
        std::cout << "################################" << std::endl;
    }
#   endif

private:
    std::string fileName;
    TreeNode root, head;
    int level, keyCount, nodeCount, degree;

    void initBPTree();

    void readBPTree(const std::string &fileName);

    bool findKeyFromNode(TreeNode node, const T &key, NodeSearchParse<T> &res);

    void cascadeInsert(TreeNode node);

    bool cascadeDelete(TreeNode node);

    bool deleteBranchLL(TreeNode node, TreeNode parent, TreeNode sibling, int index);

    bool deleteBranchLR(TreeNode node, TreeNode parent, TreeNode sibling, int index);

    bool deleteBranchRL(TreeNode node, TreeNode parent, TreeNode sibling, int index);

    bool deleteBranchRR(TreeNode node, TreeNode parent, TreeNode sibling, int index);

    bool deleteLeafLL(TreeNode node, TreeNode parent, TreeNode sibling, int index);

    bool deleteLeafLR(TreeNode node, TreeNode parent, TreeNode sibling, int index);

    bool deleteLeafRL(TreeNode node, TreeNode parent, TreeNode sibling, int index);

    bool deleteLeafRR(TreeNode node, TreeNode parent, TreeNode sibling, int index);
};

template<typename T>
BPTree<T>::BPTree(std::string fileName, int degree, bool read) : 
    fileName(fileName), degree(degree), 
    keyCount(0), nodeCount(0), level(0), 
    root(nullptr), head(nullptr) 
{
    if (read)
    {
        readBPTree(fileName);
    }
    else
    {
        initBPTree(); 
    }
}

template<typename T>
BPTree<T>::~BPTree() {

}

template<typename T>
void BPTree<T>::initBPTree() {
    root = new BPTreeNode<T>(degree, true);
    keyCount = 0;
    level = 1;
    nodeCount = 1;
    head = root;
}

template<typename T>
bool BPTree<T>::findKeyFromNode(TreeNode node, const T &key, NodeSearchParse<T> &res) {
    int index;
    if (node->search(key, index)) {
        if (node->isLeaf()) {
            res.index = index;
        } else {
            node = node->children[index + 1];
            while (!node->isLeaf()) { node = node->children[0]; }
            res.index = 0;
        }
        res.node = node;
        return true;
    } else {
        if (node->isLeaf()) {
            res.node = node;
            res.index = index;
            return false;
        } else {
            return findKeyFromNode(node->children[index], key, res);
        }
    }
}

template<typename T>
TuplePtr BPTree<T>::find(const T &key) {
    NodeSearchParse<T> res;
    if (!root) { return NONE; }
    if (findKeyFromNode(root, key, res)) { return res.node->keyOffset[res.index]; }
    else { return NONE; }
}

template<typename T>
NodeSearchParse<T> BPTree<T>::findNode(const T &key) {
    NodeSearchParse<T> res;
    if (!root) { return res; }
    if (findKeyFromNode(root, key, res)) { return res; }
    else { return res; }
}

template<typename T>
bool BPTree<T>::insert(const T &key, TuplePtr offset) {
    NodeSearchParse<T> res;
    if (!root) { initBPTree(); }
    if (findKeyFromNode(root, key, res)) {
        std::cerr << "Insert duplicate key!" << std::endl;
        return false;
    }
    res.node->add(key, offset);
    // 溢出了
    if (res.node->cnt == degree) {
        cascadeInsert(res.node);
    }
    keyCount++;
    return true;
}

template<typename T>
void BPTree<T>::cascadeInsert(BPTree::TreeNode node) {
    T key;
    TreeNode sibling = node->split(key);
    nodeCount++;

    if (node->isRoot()) {
        TreeNode root = new BPTreeNode<T>(degree, false);
        level++;
        nodeCount++;
        this->root = root;
        node->parent = root;
        sibling->parent = root;
        root->add(key);
        root->children[0] = node;
        root->children[1] = sibling;
    } else {
        TreeNode parent = node->parent;
        int index = parent->add(key);

        parent->children[index + 1] = sibling;
        sibling->parent = parent;
        if (parent->cnt == degree) {
            cascadeInsert(parent);
        }
    }
}

template<typename T>
bool BPTree<T>::remove(const T &key) {
    NodeSearchParse<T> res;
    if (!root) {
        std::cerr << "Dequeuing empty BPTree!" << std::endl;
        return false;
    }
    if (!findKeyFromNode(root, key, res)) {
        std::cerr << "Key not found!" << std::endl;
        return false;
    }
    if (res.node->isRoot()) {
        res.node->removeAt(res.index);
        keyCount--;
        return cascadeDelete(res.node);
    } else {
        if (res.index == 0 && head != res.node) {
            // cascadingly update parent node
            int index;
            TreeNode currentParent = res.node->parent;
            bool keyFound = currentParent->search(key, index);
            while (!keyFound) {
                if (!currentParent->parent) { break; }
                currentParent = currentParent->parent;
                keyFound = currentParent->search(key, index);
            }
            currentParent->keys[index] = res.node->keys[1];
            res.node->removeAt(res.index);
            keyCount--;
            return cascadeDelete(res.node);
        } else {
            res.node->removeAt(res.index);
            keyCount--;
            return cascadeDelete(res.node);
        }
    }
}

template<typename T>
bool BPTree<T>::cascadeDelete(BPTree::TreeNode node) {
    int minimal = degree / 2, minimalBranch = (degree - 1) / 2;
    if ((node->isLeaf() && node->cnt >= minimal) // leaf node
        || (node->isRoot() && node->cnt) // root node
        || (!node->isLeaf() && !node->isRoot() && node->cnt >= minimal) // branch node
            ) {
        return true; // no need to update
    }

    if (node->isRoot()) {
        if (root->isLeaf()) {
            // tree completely removed
            root = nullptr;
            head = nullptr;
        } else {
            // reduce level by one
            root = node->children[0];
            root->parent = nullptr;
        }
        delete node;
        nodeCount--;
        level--;
        return true;
    }


    TreeNode currentParent = node->parent, sibling;
    int index;

    if (node->isLeaf()) {
        // merge if it is leaf node
        currentParent->search(node->keys[0], index);
        if (currentParent->children[0] != node && currentParent->cnt == index + 1) {
            // rightest, also not first, merge with left sibling
            sibling = currentParent->children[index];
            if (sibling->cnt > minimal) {
                // transfer rightest of left to the leftest to meet the requirement
                return deleteLeafLL(node, currentParent, sibling, index);
            } else {
                // have to merge and cascadingly merge
                return deleteLeafLR(node, currentParent, sibling, index);
            }
        } else {
            // can merge with right brother
            if (currentParent->children[0] == node) {
                // on the leftest
                sibling = currentParent->children[1];
            } else {
                // normally
                sibling = currentParent->children[index + 2];
            }
            if (sibling->cnt > minimal) {
                // add the leftest of sibling to the right
                return deleteLeafRL(node, currentParent, sibling, index);
            } else {
                // merge and cascadingly delete
                return deleteLeafRR(node, currentParent, sibling, index);
            }
        }
    } else {
        // merge if it is branch node
        currentParent->search(node->children[0]->keys[0], index);
        if (currentParent->children[0] != node && currentParent->cnt == index + 1) {
            // can only be updated with left sibling
            sibling = currentParent->children[index];
            if (sibling->cnt > minimalBranch) {
                // add rightest key to the first node to avoid cascade operation
                return deleteBranchLL(node, currentParent, sibling, index);
            } else {
                // delete this and merge
                return deleteBranchLR(node, currentParent, sibling, index);
            }
        } else {
            // update with right sibling
            if (currentParent->children[0] == node) {
                sibling = currentParent->children[1];
            } else {
                sibling = currentParent->children[index + 2];
            }

            if (sibling->cnt > minimalBranch) {
                // add first key of sibling to the right
                return deleteBranchRL(node, currentParent, sibling, index);
            } else {
                // merge the sibling to current node
                return deleteBranchRR(node, currentParent, sibling, index);
            }
        }
    }
}

template<typename T>
bool BPTree<T>::deleteBranchLL(BPTree::TreeNode node, BPTree::TreeNode parent, BPTree::TreeNode sibling, int index) {
    node->children[node->cnt + 1] = node->children[node->cnt];
    for (int i = node->cnt; i > 0; i--) {
        node->children[i] = node->children[i - 1];
        node->keys[i] = node->keys[i - 1];
    }
    node->children[0] = sibling->children[sibling->cnt];
    node->keys[0] = parent->keys[index];
    parent->keys[index] = sibling->keys[sibling->cnt - 1];
    node->cnt++;
    sibling->children[sibling->cnt]->parent = node;
    sibling->removeAt(sibling->cnt - 1);
    return true;
}

template<typename T>
bool BPTree<T>::deleteBranchLR(BPTree::TreeNode node, BPTree::TreeNode parent, BPTree::TreeNode sibling, int index) {
    sibling->keys[sibling->cnt] = parent->keys[index]; // add one node
    parent->removeAt(index);
    sibling->cnt++;
    for (int i = 0; i < node->cnt; i++) {
        node->children[i]->parent = sibling;
        sibling->children[sibling->cnt + i] = node->children[i];
        sibling->keys[sibling->cnt + i] = node->keys[i];
    }
    // rightest children
    sibling->children[sibling->cnt + node->cnt] = node->children[node->cnt];
    sibling->children[sibling->cnt + node->cnt]->parent = sibling;
    sibling->cnt += node->cnt;

    delete node;
    nodeCount--;

    return cascadeDelete(parent);
}

template<typename T>
bool BPTree<T>::deleteBranchRL(BPTree::TreeNode node, BPTree::TreeNode parent, BPTree::TreeNode sibling, int index) {
    sibling->children[0]->parent = node;
    node->children[node->cnt + 1] = sibling->children[0];
    node->keys[node->cnt] = sibling->children[0]->keys[0];
    node->cnt++;

    if (node == parent->children[0]) {
        parent->keys[0] = sibling->keys[0];
    } else {
        parent->keys[index + 1] = sibling->keys[0];
    }

    sibling->children[0] = sibling->children[1];
    sibling->removeAt(0);
    return true;
}

template<typename T>
bool BPTree<T>::deleteBranchRR(BPTree::TreeNode node, BPTree::TreeNode parent, BPTree::TreeNode sibling, int index) {

    node->keys[node->cnt] = parent->keys[index];
    if (node == parent->children[0]) {
        parent->removeAt(0);
    } else {
        parent->removeAt(index + 1);
    }
    node->cnt++;
    for (int i = 0; i < sibling->cnt; i++) {
        sibling->children[i]->parent = node;
        node->children[node->cnt + i] = sibling->children[i];
        node->keys[node->cnt + i] = sibling->keys[i];
    }
    // rightest child
    sibling->children[sibling->cnt]->parent = node;
    node->children[node->cnt + sibling->cnt] = sibling->children[sibling->cnt];
    node->cnt += sibling->cnt;

    delete sibling;
    nodeCount--;

    return cascadeDelete(parent);
}

template<typename T>
bool BPTree<T>::deleteLeafLL(BPTree::TreeNode node, BPTree::TreeNode parent, BPTree::TreeNode sibling, int index) {
    for (int i = node->cnt; i > 0; i--) {
        node->keys[i] = node->keys[i - 1];
        node->keyOffset[i] = node->keyOffset[i - 1];
    }
    node->keys[0] = sibling->keys[sibling->cnt - 1];
    node->keyOffset[0] = sibling->keyOffset[sibling->cnt - 1];
    sibling->removeAt(sibling->cnt - 1);

    node->cnt++;
    parent->keys[index] = node->keys[0];

    return true;
}

template<typename T>
bool BPTree<T>::deleteLeafLR(BPTree::TreeNode node, BPTree::TreeNode parent, BPTree::TreeNode sibling, int index) {
    parent->removeAt(index);
    for (int i = 0; i < node->cnt; i++) {
        sibling->keys[i + sibling->cnt] = node->keys[i];
        sibling->keyOffset[i + sibling->cnt] = node->keyOffset[i];
    }
    sibling->cnt += node->cnt;
    sibling->sibling = node->sibling;

    delete node;
    nodeCount--;

    return cascadeDelete(parent);
}

template<typename T>
bool BPTree<T>::deleteLeafRL(BPTree::TreeNode node, BPTree::TreeNode parent, BPTree::TreeNode sibling, int index) {
    node->keys[node->cnt] = sibling->keys[0];
    node->keyOffset[node->cnt] = sibling->keyOffset[0];
    node->cnt++;
    sibling->removeAt(0);
    if (parent->children[0] == node) {
        parent->keys[0] = sibling->keys[0]; // if it is leftest, change key at index zero
    } else {
        parent->keys[index + 1] = sibling->keys[0]; // or next sibling should be updated
    }
    return true;
}

template<typename T>
bool BPTree<T>::deleteLeafRR(BPTree::TreeNode node, BPTree::TreeNode parent, BPTree::TreeNode sibling, int index) {
    for (int i = 0; i < sibling->cnt; i++) {
        node->keys[node->cnt + i] = sibling->keys[i];
        node->keyOffset[node->cnt + i] = sibling->keyOffset[i];
    }
    if (node == parent->children[0]) {
        parent->removeAt(0); // if leftest, merge with first sibling
    } else {
        parent->removeAt(index + 1); // or merge with next
    }
    node->cnt += sibling->cnt;
    node->sibling = sibling->sibling;
    delete sibling;
    nodeCount--;
    return cascadeDelete(parent);
}

