#define DEBUG
#include <iostream>
#include "BPTree.hpp"
#include "../BufferManager/BufferManager.h"

using namespace std;

int main(int argc, char const *argv[])
{
    BufferManager bm;
#   ifdef _INT_TEST_
    BPTree<int> T(bm);
    T.createNewFile("testBPT1", sizeof(int));
    
    for (int i = 0; i < 10; i++)
    {
        T._insert(i, TuplePtr(1, i));
        T._insert(20-i, TuplePtr(1, 20-i));
    }
    T.showLeaf();

    T._modify(3, 50, TuplePtr(1, 50));
    T._delete(13);

    TuplePtr res = T._search(15);
    cout << res.blockID << " " << res.offset << endl;

    T.readFromFile("testBPT1");
    T.showLeaf();
#   endif

#   ifdef _FLOAT_TEST_
    BPTree<float> T(bm);
    T.createNewFile("testBPT2", sizeof(float));
    
    for (int i = 0; i < 10; i++)
    {
        T._insert(i*1.5, TuplePtr(1, i));
        T._insert(30-i*1.5, TuplePtr(1, 20-i));
    }
    T.showLeaf();

    T._modify(3*1.5, 50.0, TuplePtr(1, 50));
    T._delete(9*1.5);

    TuplePtr res = T._search(15.0);
    cout << res.blockID << " " << res.offset << endl;

    res = T._search(7.5);
    cout << res.blockID << " " << res.offset << endl;

    T.readFromFile("testBPT2");
    T.showLeaf();
#   endif

#define _STRING_TEST_
#   ifdef _STRING_TEST_
    BPTree<std::string> T(bm);
    T.createNewFile("testBPT3", 10);
    
    for (int i = 0; i < 10; i++)
    {
        T._insert("hello"+::to_string(i), TuplePtr(1, i));
        T._insert("hello"+::to_string(20-i), TuplePtr(1, 20-i));
    }
    T.showLeaf();

    T._modify("hello1", "hello50", TuplePtr(1, 50));
    T._delete("hello2");

    TuplePtr res = T._search("hello3");
    cout << res.blockID << " " << res.offset << endl;

    res = T._search("hello2");
    cout << res.blockID << " " << res.offset << endl;

    T.readFromFile("testBPT3");
    T.showLeaf();
#   endif
}
