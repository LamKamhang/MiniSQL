#define DEBUG
#include <iostream>
#include "BPTree.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
    BPTree<int> testTree("test", 4);
    
    for (int i = 0; i < 10; ++i)
    {
        testTree.insert(i, NONE);
    }
    testTree.showTree();
    return 0;
}
