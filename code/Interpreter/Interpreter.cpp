#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include "Interpreter.h"
using namespace std;
/*void Interpreter::init()
{
	cout << "Welcome to MiniSQL!" << endl;
	cout << "MiniSQL->";
	fflush(stdin);
}*/

miniStatement miniInterpreter(char* in)
{
	miniStatement SQL;
	vector<string> split;
	int i = 0;
	CatalogManager cm;
	string d = " (),	\n";
	splitString(in, d, split);
	if (split[0] == "select")
	{
		int i = split[split.size() - 1].find(";");
		if (i != split[split.size() - 1].length() - 1)
		{
			SQL.flag = SYNTAX_ERROR;
			cout<<"Error! Check your input, please!"<<endl;
			return SQL;
		}
		else
		{
			split[split.size()-1] = split[split.size() - 1].erase(split[split.size() - 1].length() - 1, 1);
			if (split[1] == "*")
			{
				if (split[2] == "from")
				{
					if (split.size() == 4)
					{
						if (cm.isTable(split[3]))
						{
							SQL.flag = SELECT;
							SQL.MiniSelect.tableName = split[3];
							SQL.MiniSelect.conditionNum = 0;
							cout<<"Success!"<<endl;
							return SQL;
						}
						else
						{
							SQL.flag = SYNTAX_ERROR;
							cout<<"Error! Check your input, please!"<<endl;
							return SQL;
						}
					}
					else if (split[4] == "where")
					{
						if (cm.isTable(split[3]))
						{
							SQL.flag = SELECT;
							SQL.MiniSelect.tableName = split[3];
							SQL.MiniSelect.conditionNum = 0;
							for (int j = 5; j < split.size(); j = j + 4)
							{
								SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].attributeName = split[j];
								if(split[j + 1]=="=")
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=EQ;
								}									
								else if(split[j + 1]=="<>")
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=NE;
								}
								else if(split[j + 1]=="<=")
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=LE;
								}
								else if(split[j + 1]==">=")
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=GE;
								}
								else if(split[j + 1]=="<")
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=LT;
								}
								else if(split[j + 1]==">")
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=GT;
								}
								else
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=ERROR;
								}
								if (strstr(split[j + 2].c_str(), "'"))
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = String;
									split[j + 2] = split[j + 2].erase(0, 1);
									split[j + 2] = split[j + 2].erase(split[j + 2].length() - 1, 1);
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].stringValues = split[j + 2];
									SQL.MiniSelect.conditionNum++;
								}
								else if (strstr(split[j + 2].c_str(), "."))
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = Float;
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].floatValue = atof(split[j + 2].c_str());
									SQL.MiniSelect.conditionNum++;
								}
								else
								{
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = Integer;
									SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].intValue = atoi(split[j + 2].c_str());
									SQL.MiniSelect.conditionNum++;
								}
							}
							return SQL;
							cout<<"print select results"<<endl;
						}
						else
						{
							SQL.flag = SYNTAX_ERROR;
							cout<<"Error! Check your input, please!"<<endl;
							return SQL;
						}
					}
					else
					{
						SQL.flag = SYNTAX_ERROR;
						cout<<"Error! Check your input, please!"<<endl;
						return SQL;
					}
				}
				else
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;
					return SQL;
				}
			}
			else
			{
				int posFrom = -1;
				// 当选择不是全部属性的时候 即不是*的情况
				for (int j = 0; j < split.size(); j++)
				{
					if (split[j] == "from")
						posFrom = j;
				}
				if (posFrom == -1 || posFrom == 1)
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;
					return SQL;
				}
				if (cm.isTable(split[posFrom + 1]))
				{
					SQL.MiniSelect.tableName = split[posFrom + 1];
					SQL.MiniSelect.conditionNum = 0;
					for (int j = posFrom + 3; j < split.size(); j = j + 4)
					{
						SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].attributeName = split[j];
						SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].attributeName = split[j];
						if(split[j + 1]=="=")
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=EQ;
						}									
						else if(split[j + 1]=="<>" || split[j + 1] == "!=")
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=NE;
						}
						else if(split[j + 1]=="<=")
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=LE;
						}
						else if(split[j + 1]==">=")
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=GE;
						}
						else if(split[j + 1]=="<")
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=LT;
						}
						else if(split[j + 1]==">")
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=GT;
						}
						else
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].oprt=ERROR;
						}
						if (strstr(split[j + 2].c_str(), "'"))
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = String;
							split[j + 2] = split[j + 2].erase(0, 1);
							split[j + 2] = split[j + 2].erase(split[j + 2].length() - 1, 1);
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].stringValues = split[j + 2];
							SQL.MiniSelect.conditionNum++;
						}
						else if (strstr(split[j + 2].c_str(), "."))
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = Float;
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].floatValue = atof(split[j + 2].c_str());
							SQL.MiniSelect.conditionNum++;
						}
						else
						{
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].type = Integer;
							SQL.MiniSelect.cond[SQL.MiniSelect.conditionNum].intValue = atoi(split[j + 2].c_str());
							SQL.MiniSelect.conditionNum++;
						}
					}
				}
				else
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;
					return SQL;
				}
				SQL.MiniSelect.attributeNum = 0;
				for (int j = 1; j < posFrom; j++)
				{
					if (cm.isAttribute(split[posFrom + 1], split[j]))
					{
						SQL.flag = SELECT;
						SQL.MiniSelect.attr[SQL.MiniSelect.attributeNum].name = split[j];
						SQL.MiniSelect.attributeNum++;
						
					}
					else
					{
						SQL.flag = SYNTAX_ERROR;
						cout<<"Error! Check your input, please!"<<endl;
						return SQL;
					}
				}
				return SQL;
			}
		}	
	}

	else if (split[0] == "delete")
	{
		if (split[1] == "from")
		{
			int i = split[split.size()-1].find(";");
			if (i != split[split.size() - 1].length()-1)
			{
				SQL.flag = SYNTAX_ERROR;
				cout<<"Error! Check your input, please!"<<endl;					
				return SQL;
			}
			else
			{
				split[split.size()-1] = split[split.size() - 1].erase(split[split.size() - 1].length() - 1, 1);
				if(split.size()==3)
				{
					if(cm.isTable(split[2]))
					{
						SQL.flag = DELETE;
						SQL.MiniDelete.tableName = split[2];
						SQL.MiniDelete.conditionNum = 0;
						cout<<"Success!"<<endl;
						return SQL;
					}
					else
					{
						SQL.flag = SYNTAX_ERROR;
						cout<<"Error! Check your input, please!"<<endl;
						return SQL;
					}
				}
				else if (split[3] == "where")
				{									
					if (cm.isTable(split[2]))
					{
						SQL.flag = DELETE;
						SQL.MiniDelete.tableName = split[2];
						SQL.MiniDelete.conditionNum = 0;
						for (int j = 4; j < split.size(); j = j + 4)
						{
							SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].attributeName = split[j];
							if(split[j + 1]=="=")
							{
								SQL.MiniSelect.cond[SQL.MiniDelete.conditionNum].oprt=EQ;
							}									
							else if(split[j + 1]=="<>")
							{
								SQL.MiniSelect.cond[SQL.MiniDelete.conditionNum].oprt=NE;
							}
							else if(split[j + 1]=="<=")
							{
								SQL.MiniSelect.cond[SQL.MiniDelete.conditionNum].oprt=LE;
							}
							else if(split[j + 1]==">=")
							{
								SQL.MiniSelect.cond[SQL.MiniDelete.conditionNum].oprt=GE;
							}
							else if(split[j + 1]=="<")
							{
								SQL.MiniSelect.cond[SQL.MiniDelete.conditionNum].oprt=LT;
							}
							else if(split[j + 1]==">")
							{
								SQL.MiniSelect.cond[SQL.MiniDelete.conditionNum].oprt=GT;
							}
							else
							{
								SQL.MiniSelect.cond[SQL.MiniDelete.conditionNum].oprt=ERROR;
							}
							if (strstr(split[j + 2].c_str(), "'"))
							{
								SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].type = String;
								split[j + 2] = split[j + 2].erase(0, 1);
								split[j + 2] = split[j + 2].erase(split[j + 2].length() - 1, 1);
								SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].stringValues = split[j + 2];
								SQL.MiniDelete.conditionNum++;
							}
							else if (strstr(split[j + 2].c_str(), "."))
							{
								SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].type = Float;
								SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].floatValue = atof(split[j + 2].c_str());
								SQL.MiniDelete.conditionNum++;
							}
							else
							{
								SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].type = Integer;
								SQL.MiniDelete.cond[SQL.MiniDelete.conditionNum].intValue = atoi(split[j + 2].c_str());
								SQL.MiniDelete.conditionNum++;
							}
						}
						cout<<"Success!"<<endl;
						return SQL;
						
						
					}
					else
					{
						SQL.flag = SYNTAX_ERROR;
						cout<<"Error! Check your input, please!"<<endl;
						return SQL;
					}				
				}
				else
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;					
					return SQL;
				}
			}
		}
		else
		{
			SQL.flag = SYNTAX_ERROR;
			cout<<"Error! Check your input, please!"<<endl;
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
					if (split[split.size() - 1] != ";")
					{
						SQL.flag = SYNTAX_ERROR;
						cout<<"Error! Check your input, please!"<<endl;
						return SQL;
					}
					else
					{
						SQL.flag = INSERT;
						SQL.MiniInsert.tableName = split[2];
						SQL.MiniInsert.insertNum = 0;
						for (int j = 4; j < split.size(); j++)
						{
							if (strstr(split[j].c_str(), "'"))
							{
								SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].type = String;
								split[j] = split[j].erase(0, 1);
								split[j] = split[j].erase(split[j].length() - 1, 1);
								SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].stringValues = split[j];
								SQL.MiniInsert.insertNum++;
							}
							else if (strstr(split[j].c_str(), "."))
							{
								SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].type = Float;
								SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].floatValue = atof(split[j].c_str());
								SQL.MiniInsert.insertNum++;
							}
							else
							{
								SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].type = Integer;
								SQL.MiniInsert.cond[SQL.MiniInsert.insertNum].intValue = atoi(split[j].c_str());
								SQL.MiniInsert.insertNum++;
							}
						}
						cout<<"Success!"<<endl;
						return SQL;
					}				
				}
				else
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;
					return SQL;
				}
			}
			else
			{
				SQL.flag = SYNTAX_ERROR;
				cout<<"Error! Check your input, please!"<<endl;
				return SQL;
			}
		}
		else
		{
			SQL.flag = SYNTAX_ERROR;
			cout<<"Error! Check your input, please!"<<endl;
			return SQL;
		}
	}

	else if (split[0] == "execfile")
	{
		int i = split[1].find(";");
		if (i != split[1].length() - 1)
		{
			SQL.flag = SYNTAX_ERROR;
			cout<<"Error! Check your input, please!"<<endl;
			return SQL;
		}
		else
		{
			SQL.flag = EXEFILE;
			split[1] = split[1].erase(split[1].length() - 1, 1);
			SQL.MiniFile.fileName = split[1];
			cout<<"Success!"<<endl;
			return SQL;
		}
	}

	else if (split[0] == "create")
	{
		if (split[1] == "table")
		{
			if (split[split.size() - 1] != ";")	//判断有没有分号结尾
			{
				SQL.flag = SYNTAX_ERROR;
				cout<<"Error! Check your input, please!"<<endl;
				return SQL;
			}
			else
			{
				if (cm.isTable(split[2]))	  //检查表名是否已经存在
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;
					return SQL;
				}
				else
				{
					SQL.flag = CREATETABLE;
					SQL.MiniCreateTable.tableName = split[2];
					SQL.MiniCreateTable.attributeNum = 0;
					vector<string> primary;		//用于记录属性名，便于主键的bool值写入
					int j = 0;
					for (int k = 3; k < split.size()-1; k++)
					{					
						if (split[k] != "primary")
						{
							primary.insert(primary.end(), split[k]);
							j++;
							SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].name = split[k];
							if (split[k + 1] == "char")
							{
								SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].type = String;
								SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].length = atoi(split[k + 2].c_str());
								if (split[k + 3] == "unique")
								{
									SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].unique = true;
									k = k + 3;
								}
								else
								{
									SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].unique = false;
									k = k + 2;
								}
								SQL.MiniCreateTable.attributeNum++;
								
							}
							else if (split[k + 1] == "int")
							{
								SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].type = Integer;
								if (split[k + 2] == "unique")
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
							else if (split[k + 1] == "float")
							{
								SQL.MiniCreateTable.attributes[SQL.MiniCreateTable.attributeNum].type = Float;
								if (split[k + 2] == "unique")
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
							else
							{
								SQL.flag = SYNTAX_ERROR;
								cout<<"Error! Check your input, please!"<<endl;
								return SQL;
							}
						}											
						else if (split[k] == "primary")
						{
							if (split[k+1]==("key"))
							{
								int m;
								for (m = 0; m < j; m++)
								{
									if (split[k + 2] == primary[m])
									{
										SQL.MiniCreateTable.attributes[m].primary = true;
										break;
									}									
								}
								if (m > j)
								{
									SQL.flag = SYNTAX_ERROR;
									cout<<"Error! Check your input, please!"<<endl;
									return SQL;
								}
								k = k + 3;
							}
							else
							{
								SQL.flag = SYNTAX_ERROR;
								cout<<"Error! Check your input, please!"<<endl;
								return SQL;
							}
						}
					}
					cout<<"Success!"<<endl;
					return SQL;
				}
			}			
		}
		else if (split[1] == "index")
		{
			if (split[6]!=";")
			{
				SQL.flag = SYNTAX_ERROR;
				cout<<"Error! Check your input, please!"<<endl;
				return SQL;
			}
			else
			{
				if (split[3] == "on")
				{
					if (cm.isTable(split[4]))
					{
						if (cm.isAttribute(split[4], split[5]))
						{
							SQL.flag = CREATEINDEX;
							SQL.MiniCreateIndex.indexName = split[2];
							SQL.MiniCreateIndex.tableName = split[4];
							SQL.MiniCreateIndex.attributeName = split[5];
							cout<<"Success!"<<endl;
							return SQL;
						}
						else
						{
							SQL.flag = SYNTAX_ERROR;
							cout<<"Error! Check your input, please!"<<endl;
							return SQL;
						}
					}
					else
					{
						SQL.flag = SYNTAX_ERROR;
						cout<<"Error! Check your input, please!"<<endl;
						return SQL;
					}

				}
				else
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;
					return SQL;
				}
			}
			
		}
		else if (split[1] == "database")
		{
			int i = split[2].find(";");
			if (i != split[2].length() - 1)
			{
				SQL.flag = SYNTAX_ERROR;
				cout<<"Error! Check your input, please!"<<endl;
				return SQL;
			}
			else
			{
				SQL.flag = CREATEDATABASE;
				split[2] = split[2].erase(split[2].length() - 1, 1);
				SQL.MiniCreateDatabase.databaseName = split[2];
				cout<<"Success!"<<endl;
				return SQL;
			}
			
		}
		else
		{
			SQL.flag = SYNTAX_ERROR;
			cout<<"Error! Check your input, please!"<<endl;
			return SQL;
		}
	}

	else if (split[0] == "drop")
	{
		if (split[1] == "table")
		{
			int i = split[2].find(";");
			if (i != split[2].length() - 1)
			{
				SQL.flag = SYNTAX_ERROR;
				cout<<"Error! Check your input, please!"<<endl;
				return SQL;
			}
			else
			{				
				split[2] = split[2].erase(split[2].length() - 1, 1);
				if(!cm.isTable(split[2]))
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;
					return SQL;
				}
				else
				{
					SQL.flag = DROPTABLE;
					SQL.MiniDropTable.tableName = split[2];
					cout<<"Success!"<<endl;
					return SQL;
				}
				
			}
			
		}
		else if (split[1] == "index")
		{
			int i = split[2].find(";");
			if (i != split[2].length() - 1)
			{
				SQL.flag = SYNTAX_ERROR;
				cout<<"Error! Check your input, please!"<<endl;
				return SQL;
			}
			else
			{
				split[2] = split[2].erase(split[2].length() - 1, 1);
				if(cm.isIndex(split[2]))
				{
					SQL.flag = DROPINDEX;
					SQL.MiniDropIndex.indexName = split[2];
					return SQL;
				}
				else
				{
					SQL.flag = SYNTAX_ERROR;
					cout<<"Error! Check your input, please!"<<endl;
					return SQL;
				}
			}
			
		}
		else if (split[1] == "database")
		{
			int i = split[2].find(";");
			if (i != split[2].length() - 1)
			{
				SQL.flag = SYNTAX_ERROR;
				cout<<"Error! Check your input, please!"<<endl;
				return SQL;
			}
			else
			{
				SQL.flag = DROPDATABASE;
				split[2] = split[2].erase(split[2].length() - 1, 1);
				SQL.MiniDropDatabase.databaseName = split[2];
				return SQL;
			}
			
		}
		else
		{
			SQL.flag = SYNTAX_ERROR;
			cout<<"Error! Check your input, please!"<<endl;
			return SQL;
		}
	}

	else if (split[0] == "quit;")
	{
		SQL.flag = QUIT;
		cout<<"Bye"<<endl;
		return SQL;
	}

	else if (split[0] == "use")
	{
		int i = split[1].find(";");
		if (i != split[1].length() - 1)
		{
			SQL.flag = SYNTAX_ERROR;
			cout<<"Error! Check your input, please!"<<endl;
			return SQL;
		}
		else
		{
			SQL.flag = USEDATABASE;
			split[1] = split[1].erase(split[1].length() - 1, 1);
			SQL.MiniUseDatabase.databaseName = split[1];
			cout<<"Success!"<<endl;
			return SQL;
		}
		
	}

	else
	{
		SQL.flag = SYNTAX_ERROR;
		cout<<"Error! Check your input, please!"<<endl;
		return SQL;
	}
}
