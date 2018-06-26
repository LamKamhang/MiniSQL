#include "IndexManager.h"
#include <string>
#include <vector>
#include "../interface.h"

using namespace MINISQL_BASE;
using namespace std;

/*
	int attriNum;//属性的个数
	std::vector<attribute> attributes;//属性信息
	int recordNum;//记录的个数
	std::vector<miniRecord> list;//记录的数据
    */

int main(int argc, char const *argv[])
{
    BufferManager bm;
    IndexManager im(bm);

    records values;
    attribute a;
    condition value;
    value.type = Integer;
    value.oprt = EQ;
    a.type = Integer;
    a.length = sizeof(int);
    values.attributes.push_back(a);

    //im._create("testIM", values, 0);
    //im._insert("testIM", value, TuplePtr(1, 100));

    // std::vector<TuplePtr> p;
    // bool res = im._select("testIM", value, p);
    // cout << "res: " << res << endl;

    // for (int i = 0; i < p.size(); i++)
    // {
    //     cout << p[i].blockID << " " << p[i].offset << endl;
    // }

    value.intValue = 3;
    im._insert("testIM", value, TuplePtr(1, 1320));

    // value.intValue = 0;
    //  im._delete("testIM", value);

    // std::vector<TuplePtr> p;
    // bool res = im._select("testIM", value, p);
    // cout << res << endl;

    // for (int i = 0; i < p.size(); i++)
    // {
    //     cout << p[i].blockID << " " << p[i].offset << endl;
    // }

    //im._drop("testIM");
    return 0;
}
