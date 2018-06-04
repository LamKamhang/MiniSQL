#include "Interpreter.h"
#include<iostream>
#include<stdlib.h>
#include<vector>
using namespace std;



void Interpreter::init()
{
	cout << "Welcome to MiniSQL!"<<endl;
	cout << "MiniSQL->";
	fflush(stdin);
}

miniStatement miniInterpreter(char* in)
{
	miniStatement SQL;
	string splitInput;
	const char* d = " ";
	splitInput = strtok(in, d);
	if (splitInput.find("<>", 0) >= 0)
	{
		int k = splitInput.find("<>", 0);
		splitInput.insert(k + 1, " ");
		splitInput.insert(k, " ");
	}
	if (splitInput.find("<", 0) >= 0)
	{
		int k = splitInput.find("<", 0);
		if (!( splitInput.compare(k + 1, 1, ">") && splitInput.compare(k + 1, 1, "=") ))
		{
			splitInput.insert(k + 1, " ");
			splitInput.insert(k, " ");
		}
	}
	if (splitInput.find(">", 0) >= 0)
	{
		int k = splitInput.find(">", 0);
		if (!( splitInput.compare(k - 1, 1, "<") && splitInput.compare(k + 1, 1, "=") ))
		{
			splitInput.insert(k + 1, " ");
			splitInput.insert(k, " ");
		}
	}
	if (splitInput.find("=", 0) >= 0)
	{
		int k = splitInput.find("=", 0);
		splitInput.insert(k + 1, " ");
		splitInput.insert(k, " ");
	}
	else if (splitInput.find(">=", 0) >= 0)
	{
		int k = splitInput.find(">=", 0);
		splitInput.insert(k + 1, " ");
		splitInput.insert(k, " ");
	}
	else if (splitInput.find("<=", 0) >= 0)
	{
		int k = splitInput.find("<=", 0);
		splitInput.insert(k + 1, " ");
		splitInput.insert(k, " ");
	}
	while (1)
	{
		int pos = splitInput.find(",");
		if (pos == -1)
			break;
		else
			splitInput = splitInput.replace(splitInput.find(","), 1, " ");
	}
	vector<string> split;
	int i = 0;
	CatalogManage cm;
	while (!splitInput.empty())
	{
		split[i] = splitInput;
		splitInput = strtok(NULL, d);
		i++;
	}
	if (split[0] == "select")
	{
		if (split[1] == "*")
		{
			if (split[2] == "from")
			{
				if (split[4] == "where")
				{
					if (cm.isTable(split[3]))
					{
						SQL.flag = SELECT;
						SQL.MiniSelect.tableName = split[3];
						SQL.MiniSelect.conditionNum = 0;
						for (int j = 5; j < i; j = j + 4)
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].attributeName = split[j];
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt = split[j+1];
							if (strstr(split[j + 2].c_str(), "'"))
							{
								SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = CHAR;
								split[j + 2] = split[j + 2].erase(0, 1);
								split[j + 2] = split[j + 2].erase(split[j + 2].length() - 1, 1);
								SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].stringValues=split[j + 2];
								SQL.MiniSelect.conditionNum++;
							}
							else if (strstr(split[j + 2].c_str(), "."))
							{
								SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = FLOAT;
								SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].stringValues = split[j + 2];
								SQL.MiniSelect.conditionNum++;
							}
							else
							{
								SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = INT;
								SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].stringValues = split[j + 2];
								SQL.MiniSelect.conditionNum++;
							}
						}
						return SQL;
					}
					else
					{
						SQL.flag = ERROR;
						return SQL;
					}
				}
				else
				{
					if (i == 4)
					{
						SQL.flag = SELECT;
						SQL.MiniSelect.tableName = split[3];
						SQL.MiniSelect.conditionNum = 0;
						return SQL;
					}
					else
					{
						SQL.flag = ERROR;
						return SQL;
					}
				}
			}
			else
			{
				SQL.flag = ERROR;
				return SQL;
			}
		}
		else
		{
			int posFrom = -1;
			// 当选择不是全部属性的时候 即不是*的情况
			for (int j = 0; j < i; i++)
			{
				if (split[j] == "from")
					posFrom = j;
			}
			if (posFrom == -1 || posFrom == 1)
			{
				SQL.flag = ERROR;
				return SQL;
			}
			if (cm.isTable(split[posFrom + 1]))
			{
				SQL.MiniSelect.tableName = split[posFrom + 1];
				SQL.MiniSelect.conditionNum = 0;
				for (int j = posFrom + 3; j < i; j = j + 4)
				{
					SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].attributeName = split[j];
					SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt = split[j + 1];
					if (strstr(split[j + 2].c_str(), "'"))
					{
						SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = CHAR;
						split[j + 2] = split[j + 2].erase(0, 1);
						split[j + 2] = split[j + 2].erase(split[j + 2].length() - 1, 1);
						SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].stringValues = split[j + 2];
						SQL.MiniSelect.conditionNum++;
					}
					else if (strstr(split[j + 2].c_str(), "."))
					{
						SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = FLOAT;
						SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].stringValues = split[j + 2];
						SQL.MiniSelect.conditionNum++;
					}
					else
					{
						SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = INT;
						SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].stringValues = split[j + 2];
						SQL.MiniSelect.conditionNum++;
					}
				}
			}
			SQL.MiniSelect.attributeNum = 0;
			for (int j = 1; j < posFrom; j++)
			{
				if (cm.isAttribute(split[posFrom + 1], split[j]))
				{
					SQL.MiniSelect.attr[SQL.MiniSelect.attributeNum].name = split[j];
					SQL.MiniSelect.attributeNum++;
				}
				else
				{
					SQL.flag = ERROR;
					return SQL;
				}
			}
		}
		return SQL;
	}
	else if (split[0] == "delete")
	{
		if (split[1] == "from")
		{
			if (split[3] == "where")
			{
				if (cm.isTable(split[2]))
				{
					SQL.flag = DELETE;
					SQL.MiniDelete.tableName = split[3];
					SQL.MiniDelete.conditionNum = 0;
					for (int j = 4; j < i; j = j + 4)
					{
						SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].attributeName = split[j];
						SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].oprt = split[j + 1];
						if (strstr(split[j + 2].c_str(), "'"))
						{
							SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].type = CHAR;
							split[j + 2] = split[j + 2].erase(0, 1);
							split[j + 2] = split[j + 2].erase(split[j + 2].length() - 1, 1);
							SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].stringValues = split[j + 2];
							SQL.MiniDelete.conditionNum++;
						}
						else if (strstr(split[j + 2].c_str(), "."))
						{
							SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].type = FLOAT;
							SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].stringValues = split[j + 2];
							SQL.MiniDelete.conditionNum++;
						}
						else
						{
							SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].type = INT;
							SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].stringValues = split[j + 2];
							SQL.MiniDelete.conditionNum++;
						}
					}
					return SQL;
				}
				else
				{
					SQL.flag = ERROR;
					return SQL;
				}
			}
			else
			{
				if (i == 3)
				{
					SQL.flag = DELETE;
					SQL.MiniDelete.conditionNum = 0;
					SQL.MiniDelete.tableName = split[2];
					return SQL;
				}
				else
				{
					SQL.flag = ERROR;
					return SQL;
				}
			}
		}
		else
		{
			SQL.flag = ERROR;
			return SQL;
		}
	}

	else if (split[0] == "insert")
	{
		if (split[1] == "into")
		{
			if (split[3] == "values")
			{
				if (cm.isTable(split[2]))
				{
					SQL.flag = INSERT;
					SQL.MiniInsert.tableName = split[2];
					SQL.MiniInsert.insertNum = 0;
					split[4] = split[4].erase(0, 1);
					split[i - 1] = split[i - 1].erase(split[i - 1].length() - 1, 1);
					for (int j = 4; j < i; j++)
					{
						if (strstr(split[j].c_str(), "'"))
						{
							SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].type = CHAR;
							split[j] = split[j].erase(0, 1);
							split[j] = split[j].erase(split[j].length() - 1, 1);
							SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].stringValues = split[j];
							SQL.MiniInsert.insertNum++;
						}
						else if (strstr(split[j].c_str(), "."))
						{
							SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].type = FLOAT;
							SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].stringValues = split[j];
							SQL.MiniInsert.insertNum++;
						}
						else
						{
							SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].type = INT;
							SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].stringValues = split[j];
							SQL.MiniInsert.insertNum++;
						}
					}
					return SQL;
				}
				else
				{
					SQL.flag = ERROR;
					return SQL;
				}
			}
			else
			{
				SQL.flag = ERROR;
				return SQL;
			}
		}
		else
		{
			SQL.flag = ERROR;
			return SQL;
		}
	}

	else if (split[0] == "execfile")
	{
		if (i == 2)
		{
			SQL.flag = EXEFILE;
			SQL.MiniFile.fileName = split[1];
			return SQL;
		}
		else
		{
			SQL.flag = ERROR;
			return SQL;
		}
	}

	else if (split[0] == "create")
	{
		if (split[1] == "table")
		{
			if (!strcmp(split[3].c_str(), "(") && strcmp(split[i - 1].c_str(), ")"))
			{
				SQL.flag = CREATETABLE;
				SQL.MiniCreateTable.tableName = split[2];
				SQL.MiniCreateTable.attributeNum = 0;
				const char* dd = " ,'\n'()";	//为啥有括号
				char* tableSplit;
				tableSplit = strtok(in, dd);
				string tSplit[100];
				int j = 0;
				while (tableSplit)
				{
					tSplit[j] = tableSplit;
					tableSplit = strtok(NULL, dd);
					j++;
				}
				for (int k = 3; k < j; k++)
				{
					SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].name = tSplit[k];
					if (tSplit[k+1].find("char"))
					{
						tSplit[k + 1].erase(0, 4);
						tSplit[k + 1].erase(tSplit[k + 1].length() - 1, 1);
						SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].type = CHAR;
						SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].length = atoi(tSplit[k + 1].c_str());
						if (tSplit[k + 2] == "unique")
						{
							SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].unique = true;
							k = k + 2;
						}
						else
						{
							SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].unique = false;
							k = k + 1;
						}
						SQL.MiniCreateTable.attributeNum++;				
					}
					else if (tSplit[k + 1] == "int")
					{
						SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].type = INT;
						if (tSplit[k + 2] == "unique")
						{
							SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].unique = true;
							k = k + 2;
						}
						else
						{
							SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].unique = false;
							k = k + 1;
						}
						SQL.MiniCreateTable.attributeNum++;

					}
					else if (tSplit[k + 1] == "float")
					{
						SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].type = FLOAT;
						if (tSplit[k + 2] == "unique")
						{
							SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].unique = true;
							k = k + 2;
						}
						else
						{
							SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].unique = false;
							k = k + 1;
						}
						SQL.MiniCreateTable.attributeNum++;
					}
					else if (tSplit[k + 1] == "primary")
					{
						if (tSplit[k + 2].find("key"))
						{
							tSplit[k + 2].erase(0, 3);
							tSplit[k + 2].erase(tSplit[k + 2].length() - 1, 1);
							SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].primary = true;
						}
						else
						{
							SQL.flag = ERROR;
							return SQL;
						}
					}
				}
				return SQL;
			}
			else
			{
				SQL.flag = ERROR;
				return SQL;
			}
		}

		else if (split[1] == "index")
		{
			if (split[3] == "on")
			{
				string tName;
				string aName;
				int t = split[4].find("(");
				tName = split[4].erase(t, split[4].length()-1);
				aName = split[4].erase(0, t + 1);
				aName = aName.erase(aName.length() - 1, 1);
				if (cm.isTable(tName))
				{
					if (cm.isAttribute(tName,aName))
					{
						SQL.flag = CREATEINDEX;
						SQL.MiniCreateIndex.indexName = split[1];
						SQL.MiniCreateIndex.tableName = tName;
						SQL.MiniCreateIndex.attributeName = aName;
						return SQL;
					}
					else
					{
						SQL.flag = ERROR;
						return SQL;
					}
				}
				else
				{
					SQL.flag = ERROR;
					return SQL;
				}
				
			}
		}

		else if (split[1] == "database")
		{
			SQL.flag = CREATEDATABASE;
			SQL.MiniCreateDatabase.databaseName = split[2];
			return SQL;
		}

		else
		{
			SQL.flag = ERROR;
			return SQL;
		}
	}

	else if (split[0] == "drop")
	{
		if (split[1] == "table")
		{
			SQL.flag = DROPTABLE;
			SQL.MiniDropTable.tableName = split[2];
			return SQL;
		}
		else if (split[1] == "index")
		{
			SQL.flag = DROPINDEX;
			SQL.MiniDropIndex.indexName = split[2];
			return SQL;
		}
		else if (split[1] == "database")
		{
			SQL.flag = DROPDATABASE;
			SQL.MiniDropDatabase.databaseName = split[2];
			return SQL;
		}
		else
		{
			SQL.flag = ERROR;
			return SQL;
		}
	}

	else if (split[0] == "quit")
	{
		SQL.flag = QUIT;
		return SQL;
	}

	else if (split[0] == "use")
	{
		SQL.flag = USEDATABASE;
		SQL.MiniUseDatabase.databaseName = split[1];
		return SQL;
	}

	else
	{
		SQL.flag = ERROR;
		return SQL;
	}
}
