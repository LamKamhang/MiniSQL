#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#include "BufferManager.h"
using namespace std;
void BufferManager::InitBlock(Block* CurrBlock)
{
	CurrBlock->Offset = -1;		//-1表示未使用
	CurrBlock->FileName = "";
	CurrBlock->IsAccessed = false;
	CurrBlock->IsWritten = false;
	CurrBlock->IsLocked = false;
}

BufferManager::BufferManager():UsedBlock(0)
{
	for(int i=0; i<BUFFERSIZE; i++)
	{
		InitBlock(&Buffer[i]);
		Buffer[i].data = new char[BLOCKSIZE];
		memset(Buffer[i].data, 0, BLOCKSIZE);
	}
}



BufferManager::~BufferManager()
{
	if(!WriteBackAll())
	{
		std::cout << "Buffer Exception!!!" << endl;
		exit(1);
	}

	for(int i=0; i<BUFFERSIZE; i++)
	{
		delete[] Buffer[i].data;
 	}
}

bool BufferManager::WriteBackAll()
{
	for(int i=0; i<BUFFERSIZE; i++)
	{
        if(Buffer[i].Offset==-1)
            continue;
		if(! WriteBack(i))
		{
				std::cout << "Buffer Exception!!!" << endl;
				return false;
		}
	}

	return true;
}

Block* BufferManager::GetNewBlock(string FileName, int Offset)
{
	int Replaced;
	if (UsedBlock < BUFFERSIZE)
	{
		for (int i = 0; i<BUFFERSIZE; i++)
		{
			if (!Buffer[i].FileName.compare("") && Buffer[i].Offset == -1)
			{
				Replaced = i;
				UsedBlock++;
				break;
			}
		}
	}
	else
	{
		bool IsFound = false;
		while (!IsFound)
		{
			for (int i = 0; i<BUFFERSIZE; i++)
			{
				if (Buffer[i].IsLocked == true)
					continue;
				if (Buffer[i].IsAccessed == false)
				{
					Replaced = i;
					IsFound = true;
					break;
				}
				else
					Buffer[i].IsAccessed = false;
			}
		}
	}
	WriteBack(Replaced);
	memset(Buffer[Replaced].data, 0, BLOCKSIZE);
	FILE* fp;
	if((fp=fopen(FileName.c_str(),"rb+"))==NULL)
	{
		if((fp=fopen(FileName.c_str(),"wb+"))==NULL)
			return NULL;
	}
	fseek(fp, 0, SEEK_END);
	fwrite(Buffer[Replaced].data, BLOCKSIZE, 1, fp);
	fclose(fp);

	Buffer[Replaced].FileName = FileName;
	Buffer[Replaced].Offset = Offset;
	Buffer[Replaced].IsAccessed = true;

	return &Buffer[Replaced];
}

Block* BufferManager::GetBlock(string FileName, int Offset)
{
	if(!FileName.compare(""))//鏂囦欢鍚嶇┖
		return NULL;

	int Replaced = -1;
	for(int i=0; i<BUFFERSIZE; i++)
	{
		if(!FileName.compare(Buffer[i].FileName) && Offset==Buffer[i].Offset)
		{
			Buffer[i].IsAccessed = true;
			return &Buffer[i];
		}
		if(!Buffer[i].FileName.compare("") && Buffer[i].Offset==-1)
			Replaced = i;
	}

	if(Replaced == -1)
	{
		bool IsFound = false;
		while(!IsFound)
		{
			for(int i=0; i<BUFFERSIZE; i++)
			{
				if(Buffer[i].IsLocked == true)
					continue;
				if(Buffer[i].IsAccessed == false)
				{
					Replaced = i;
					IsFound = true;
					break;
				}
				else
					Buffer[i].IsAccessed = false;
			}
		}
	}
	else
		UsedBlock ++;

	if(ReplaceBlock(FileName, Offset, Replaced))
	{
		Buffer[Replaced].IsAccessed = true;
		return &Buffer[Replaced];
	}
	else
	{
		std::cout << "Buffer Exception!!!" << endl;
		return NULL;
	}



}

bool BufferManager::WriteBack(int Replaced)
{
	FILE* fp;
	if(Buffer[Replaced].IsWritten)
	{
		if(!Buffer[Replaced].FileName.compare("") || (Buffer[Replaced].Offset==-1))
		{
           // cout << "Error 1" << endl;
			return false;
		}

		if((fp=fopen(Buffer[Replaced].FileName.c_str(),"rb+"))==NULL)
        {
           // cout << "Error 2" << endl;
			return false;
        }

		fseek(fp, BLOCKSIZE*Buffer[Replaced].Offset, SEEK_SET);
		fwrite(Buffer[Replaced].data, BLOCKSIZE, 1, fp);
		fclose(fp);
	}

	InitBlock(&Buffer[Replaced]);
	return true;
}

bool BufferManager::ReplaceBlock(string FileName, int Offset, int Replaced)
{
	FILE* fp;
	if(!WriteBack(Replaced))
		return false;
	if((fp=fopen(FileName.c_str(),"rb"))==NULL)
			return false;
	fseek(fp, BLOCKSIZE*Offset, SEEK_SET);
	if(fread(Buffer[Replaced].data, 1, BLOCKSIZE, fp) < BLOCKSIZE)
	{
		fclose(fp);
		return false;
	}
	fclose(fp);

	Buffer[Replaced].FileName = FileName;
	Buffer[Replaced].Offset = Offset;

	return true;
}

bool BufferManager::DeleteFileBlock(string FileName)
{
	for(int i=0; i<BUFFERSIZE; i++)
	{
		if(!FileName.compare(Buffer[i].FileName))
		{
			UsedBlock --;
			InitBlock(&Buffer[i]);
		}
	}
	if(!remove(FileName.c_str()))
		return false;
	return true;
}



void BufferManager::SetLock(Block* CurrBlock)	//閿佹爣璁?
{
	CurrBlock->IsLocked = true;
}

void BufferManager::SetAccessed(Block* CurrBlock)
{
	CurrBlock->IsAccessed = true;
}

void BufferManager::SetWritten(Block* CurrBlock)
{
	CurrBlock->IsWritten = true;
}


