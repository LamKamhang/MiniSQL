#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include "CatalogManager.h"
#include "../interface.h"
using namespace MINISQL_BASE;
using namespace std;

int CatalogManager::getTableNum()
{
	return tableNum;
}

int CatalogManager::getIndexNum()
{
	return indexNum;
}

 
bool CatalogManager::readTableFile()
{
	ifstream infile;
	infile.open("table.txt");
	if (infile.is_open())
	{
		tables.clear();
		string s;
		string strDelims = " ";
		vector<string> strDest;
		bool isNewTable = true;
		tableNum = 0;
		table t;
		while (getline(infile, s))
		{
			if (s != "##")
			{
				if (isNewTable == true)   //this line is tableName
				{
					t.tableName = s;
					t.attributeNum = 0;
					t.attributes.clear();
					isNewTable = false;
				}
				else
				{
					attribute a;
					strDest.clear();
					int vN = splitString(s, strDelims, strDest);
					int aN = t.attributeNum;
					a.name = strDest[0];
					a.primary = false;
					a.unique = false;
					for (int i = 1; i < vN; i++)
					{
						if (strDest[i] == "int")
						{
							a.type = Integer;
							a.length = sizeof(int);	// 4 --> sizeof(int), 可兼容，并不是所有的int都一定是4
						}
						if (strDest[i] == "char")
						{
							a.type = String;
							a.length = atoi(strDest[i + 1].c_str());
						}
						if (strDest[i] == "float")
						{
							a.type = Float;
							a.length = sizeof(float);// 8--> sizeof(float), 同理
						}
						if (strDest[i] == "unique")
						{
							a.unique = true;
						}
						if (strDest[i] == "primary")
						{
							a.primary = true;
						}
					}
					t.attributes.push_back(a);
					t.attributeNum = t.attributes.size();
				}
			}
			else
			{
				tables.push_back(t);
				tableNum++;
				isNewTable = true;
			}
		}
		infile.close();
		return true;
	}
	else
	{
		return false;
	}

}

bool CatalogManager::writeTableFile()
{
	ofstream outfile;
	outfile.open("table.txt", std::ios::trunc);
	outfile.close();
	outfile.open("table.txt", std::ios::out | std::ios::app);
	if (outfile.is_open())
	{
		for (int tN = 0; tN < getTableNum(); tN++)
		{
			outfile << tables[tN].tableName << endl;
			for (int aN = 0; aN < tables[tN].attributeNum; aN++)
			{
				outfile << tables[tN].attributes[aN].name << " ";
				if (tables[tN].attributes[aN].type == Integer)
				{
					outfile << "int";
					if (tables[tN].attributes[aN].unique == true)
						outfile << " " << "unique";
					if (tables[tN].attributes[aN].primary == true)
						outfile << " " << "primary";
				}
				if (tables[tN].attributes[aN].type == Float)
				{
					outfile << "float";
					if (tables[tN].attributes[aN].unique == true)
						outfile << " " << "unique";
					if (tables[tN].attributes[aN].primary == true)
						outfile << " " << "primary";
				}
				if (tables[tN].attributes[aN].type == String)
				{
					outfile << "char" << " ";
					outfile << tables[tN].attributes[aN].length;
					if (tables[tN].attributes[aN].unique == true)
						outfile << " " << "unique";
					if (tables[tN].attributes[aN].primary == true)
						outfile << " " << "primary";
				}
				outfile << endl;
			}
			outfile << "##" << endl;
		}
		outfile.close();
		return true;
	}
	else
	{
		return false;
	}

}


bool CatalogManager::isTable(string tableName)
{
	readTableFile();
	for (int i = 0; i < getTableNum(); i++)
	{
		if (tables[i].tableName == tableName)
		{
			//cout << "Already there exists the table" << endl;
			return true;
		}
			
	}
	return false;
}


bool CatalogManager::isAttribute(string tableName, string attributeName)
{
	readTableFile();
	int i,j;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].tableName == tableName)
			break;
	}
	if (i == tableNum)
		return false;
	for (j = 0; j < tables[i].attributeNum; j++)
	{
		if (tables[i].attributes[j].name == attributeName)
			return true;
	}

	if (j == tables[i].attributeNum)	// why do judge
		return false;
	return false;
}

int CatalogManager::getAttributeNum(string tableName)
{
	if (isTable(tableName))
	{
		for (int i = 0; i < getTableNum(); i++)
		{
			if (tables[i].tableName == tableName)
				return tables[i].attributeNum;
		}
	}
	else
		return 0;
}

bool CatalogManager::createTable(miniCreateTable I)
{
	if (isTable(I.tableName))
	{
		//cout << "This table alreay exists." << endl;
		return false;
	}
	else
	{
		ofstream outfile;
		outfile.open("table.txt", std::ios::out | std::ios::app);
		if (outfile.is_open())
		{
			outfile << I.tableName << endl;
			for (int i = 0; i < I.attributeNum; i++)
			{
				outfile << I.attributes[i].name << " ";
				if (I.attributes[i].type == Integer)
				{
					outfile << "int";
					if (I.attributes[i].unique == true)
						outfile << " " << "unique";
					if (I.attributes[i].primary == true)
						outfile << " " << "primary";
				}
				if (I.attributes[i].type == Float)
				{
					outfile << "float";
					if (I.attributes[i].unique == true)
						outfile << " " << "unique";
					if (I.attributes[i].primary == true)
						outfile << " " << "primary";
				}
				if (I.attributes[i].type == String)
				{
					outfile << "char" << " ";
					outfile << I.attributes[i].length;
					if (I.attributes[i].unique == true)
						outfile << " " << "unique";
					if (I.attributes[i].primary == true)
						outfile << " " << "primary";
				}
				outfile << endl;
			}
			outfile << "##" << endl;
			outfile.close();
			return true;
		}
		else
			return false;
	}
}


table CatalogManager::getTable(string tableName)
{
	if (isTable(tableName))
	{
		for (int i = 0; i < getTableNum(); i++)
		{
			if (tables[i].tableName == tableName)
			{
				return tables[i];
			}
		}
	}
	return NONE_TABLE;
}


bool CatalogManager::dropTable(string tableName)
{
	if (isTable(tableName))
	{
		bool ret;
		for (int i = 0; i < getTableNum(); i++)
		{
			if (tables[i].tableName == tableName)
			{
				tables.erase(tables.begin() + i);
				tableNum = tables.size();
				ret = writeTableFile();
				break;
			}
		}
		bool rei = true;
		readIndexFile();
		for (int i = 0; i < getIndexNum(); i++)
		{
			//cout << "Begin checking the index" << endl;
			//cout << getIndexNum() << endl;
			//cout << indices[i].indexName << endl;
			//cout << indices[i].tableName << endl;
			//cout << indices[i].attributeName <<endl;
			if (indices[i].tableName == tableName)
			{
				//cout << "This is an index" << endl;
				indices.erase(indices.begin() + i);
				indexNum = indices.size();
				rei = writeIndexFile();
				break;
			}
		}
		if (ret && rei)
			return true;
		else
			return false;
	}
	else
		return false;
}

bool CatalogManager::readIndexFile()
{
	ifstream infile;
	infile.open("index.txt");
	if (infile.is_open())
	{
		indices.clear();
		string s;
		indexNum = 0;
		string strDelims = " ";
		vector<string> strDest;

		while (getline(infile, s))
		{
			index ind;
			strDest.clear();
			int vN = splitString(s, strDelims, strDest);
			ind.indexName = strDest[0];
			ind.tableName = strDest[1];
			ind.attributeName = strDest[2];
			indices.push_back(ind);
			indexNum = indices.size();
		}
		infile.close();
		return true;
	}
	else
	{
		return false;
	}
}

bool CatalogManager::writeIndexFile()
{
	ofstream outfile;
	outfile.open("index.txt", std::ios::trunc);
	outfile.close();
	outfile.open("index.txt", std::ios::out | std::ios::app);
	if (outfile.is_open())
	{
		for (int iN = 0; iN < getIndexNum(); iN++)
		{
			outfile << indices[iN].indexName << " ";
			outfile << indices[iN].tableName << " ";
			outfile << indices[iN].attributeName << endl;
		}
		outfile.close();
		return true;
	}
	else
	{
		return false;
	}
}

bool CatalogManager::isIndex(string indexName)
{
	readIndexFile();
	for (int i = 0; i < getIndexNum(); i++)
	{
		if (indices[i].indexName == indexName)
			return true;
	}
	return false;
}

bool CatalogManager::isIndex(string tableName, string attributeName)
{
	readIndexFile();
	for (int i = 0; i < getIndexNum(); i++)
	{
		if (indices[i].tableName == tableName && indices[i].attributeName == attributeName)
			return true;
	}
	return false;
}

string CatalogManager::getIndex(string tableName, string attributeName)
{
	readIndexFile();
	for (int i = 0; i < getIndexNum(); i++)
	{
		if (indices[i].tableName == tableName && indices[i].attributeName == attributeName)
			return indices[i].indexName;
	}
	return NULL;
}

bool CatalogManager::createIndex(miniCreateIndex I)
{
	if (isIndex(I.indexName))
	{
		return false;
	}
	if (!isTable(I.tableName))
	{
		return false;
	}
	if (!isAttribute(I.tableName, I.attributeName))
	{
		return false;
	}
	else
	{
		ofstream outfile;
		outfile.open("index.txt", std::ios::out | std::ios::app);
		if (outfile.is_open())
		{
			outfile << I.indexName << " ";
			outfile << I.tableName << " ";
			outfile << I.attributeName << endl;

			outfile.close();
			return true;
		}
		else
			return false;
	}
}

bool CatalogManager::dropIndex(string indexName)
{
	if (isIndex(indexName))
	{
		for (int i = 0; i < getIndexNum(); i++)
		{
			if (indices[i].indexName == indexName)
			{
				indices.erase(indices.begin() + i);
				indexNum = indices.size();
				break;
			}
		}
		if (writeIndexFile())
			return true;
		else
			return false;
	}
	else
		return false;
}

bool CatalogManager::dropDatabase()
{
	ofstream outfile;
	outfile.open("index.txt", std::ios::trunc);
	if (outfile.is_open())
		outfile.close();
	else
		return false;

	outfile.open("table.txt", std::ios::trunc);
	if (outfile.is_open())
		outfile.close();
	else
		return false;
	return true;
}

CatalogManager::CatalogManager()
{
	tableNum = 0;
	indexNum = 0;
}

CatalogManager::~CatalogManager()
{
}



int  splitString(const string & strSrc, const std::string& strDelims, vector<string>& strDest)
{
	typedef std::string::size_type ST;
	string delims = strDelims;
	std::string STR;
	if (delims.empty()) delims = "/n/r";

	ST pos = 0, LEN = strSrc.size();
	while (pos < LEN) {
		STR = "";
		while ((delims.find(strSrc[pos]) != std::string::npos) && (pos < LEN)) ++pos;
		if (pos == LEN) return strDest.size();
		while ((delims.find(strSrc[pos]) == std::string::npos) && (pos < LEN)) STR += strSrc[pos++];
		//std::cout << "[" << STR << "]";  
		if (!STR.empty()) strDest.push_back(STR);
	}
	return strDest.size();
}

