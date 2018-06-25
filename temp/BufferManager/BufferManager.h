#ifndef BUFFERMANAGER1_H
#define BUFFERMANAGER1_H

#define BLOCKSIZE 4096 //4k
#define BUFFERSIZE 1024//1k

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <string>
using namespace std;
typedef struct
{
	string FileName;	//buffer中block所属文件文件名，""为当前block为空
	int Offset;			//是文件中的第几个block
	char* data;			//指向block数据
	bool IsAccessed;	//block是否被访问
	bool IsWritten;		//block是否被写过
	bool IsLocked;		//block是否被锁定
}Block;

class BufferManager
{
private:
	int UsedBlock;
	Block Buffer[BUFFERSIZE];	//buffer 中的全部block

private:
	bool ReplaceBlock(string FileName, int Offset, int i);	//i表示buffer中将要被替换的block
	void InitBlock(Block* CurrBlock);
	bool WriteBackAll();
	bool WriteBack(int Replaced);

	//void CleanWritten(Block* CurrBlock);
public:
	BufferManager();
	~BufferManager();
	Block* GetBlock(string FileName, int Offset);	//通过offset获取已存在的block，文件名与offset作为标识
	Block* GetNewBlock(string FileName, int Offset);//新建一个块，用文件名与offset初始化
	bool DeleteFileBlock(string FileName);	//删除buffer中指定文件全部block
	void SetWritten(Block* CurrBlock);	//写标记
	void SetAccessed(Block* CurrBlock);	//访问标记
	void SetLock(Block* CurrBlock);	//锁标记
	void UnLock(Block* CurrBlock){CurrBlock->IsLocked = false;};
};

#endif
