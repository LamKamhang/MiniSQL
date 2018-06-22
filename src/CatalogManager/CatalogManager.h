class CatalogManager
{
public:
	int getTableNum();
	int getIndexNum();
	bool readTableFile();
	bool writeTableFile();
	bool isTable(string tableName);
	bool isAttribute(string tableName, string attributeName);
	bool createTable(miniCreateTable I);
	bool dropTable(string tableName);
	bool readIndexFile();
	bool writeIndexFile();
	bool isIndex(string indexName);
	bool isIndex(string tableName, string attributeName);
	table getTable(string tableName);
	string getIndex(string tableName, string attributeName);
	bool createIndex(miniCreateIndex I);
	bool dropIndex(string indexName);
	bool dropDatabase();
	CatalogManager();
	~CatalogManager();

private:
	vector<table> tables;
	vector<index> indices;
	int tableNum;
	int indexNum;
};

int  splitString(const string & strSrc, const std::string& strDelims, vector<string>& strDest);
