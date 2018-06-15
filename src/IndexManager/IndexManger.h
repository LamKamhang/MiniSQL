#include "../interface.h"


bool insert(std::string indexName, condition value, TuplePtr tuple_ptr);
bool create(std::string indexName, std::vector<)

/*
TuplePtrs | _select: indexName, condition 
bool      | _insert: indexName, value, tuplePtr
bool      | _create: indexName, records, index
TuplePtrs | _delete: indexName, condition
bool      | _drop  : indexName,

*/
std::vector<TuplePtr>