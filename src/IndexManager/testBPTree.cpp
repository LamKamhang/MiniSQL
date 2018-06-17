#define DEBUG
#include <iostream>
#include "BPTree.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
    BPTree<int> testTree("test", 4);
	
	int data[]{
		2, 3, 5, 7, 11, 17, 19, 23, 29, 31
	}; 
	int size = sizeof(data) / sizeof(int);
    for (int i = 0; i < size; ++i)
    {
        testTree.insert(data[i], NONE);
    }
    testTree.insert(9, NONE);
    testTree.showTree();
    testTree.insert(10, NONE);
    testTree.showTree();
    testTree.insert(8, NONE);
    testTree.showTree();
    testTree.remove(23);
    testTree.showTree();
    testTree.remove(19);
    testTree.showTree();   
    return 0;
}
