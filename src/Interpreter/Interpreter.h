class miniSelect
{
public:
	string tableName;
	attribute attr[32];
	condition cond[32];
	int conditionNum;
	int attributeNum;
};

class miniDelete
{
public:
	string tableName;
	condition cond[32];
	int conditionNum;
};

class miniInsert
{
public:
	string tableName;
	condition cond[32];
	int insertNum;
};

class miniFile
{
public:
	string fileName;
};

class miniDropTable
{
public:
	string tableName;
};

class miniDropIndex
{
public:
	string indexName;
};

class miniCreateDatabase
{
public:
	string databaseName;
};

class miniDropDatabase
{
public:
	string databaseName;
};

class miniUseDatabase
{
public:
	string databaseName;
};

class miniStatement
{
public:
	miniStatement() {}
	~miniStatement() {}
	int flag;
	miniSelect MiniSelect;
	miniDelete MiniDelete;
	miniInsert MiniInsert;
	miniFile MiniFile;
	miniCreateTable MiniCreateTable;
	miniCreateIndex MiniCreateIndex;
	miniDropTable MiniDropTable;
	miniDropIndex MiniDropIndex;
	miniCreateDatabase MiniCreateDatabase;
	miniDropDatabase MiniDropDatabase;
	miniUseDatabase MiniUseDatabase;

};
miniStatement miniInterpreter(char* in);
