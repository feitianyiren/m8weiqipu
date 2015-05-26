#ifndef __UITLS__H__
#define __UITLS__H__

#include <stdlib.h>

//-----------------------------------------------------------------------------
// by liigo
//-----------------------------------------------------------------------------

//带有内存缓冲机制的内存块
class BufferedMem
{
private:
	char* m_pData;
	size_t m_DataSize, m_BufferSize;

public:

	BufferedMem(int BufferSize = 0);
	~BufferedMem() { free(m_pData); }

	void* GetData() { return (m_DataSize > 0 ? m_pData : NULL); }
	size_t GetDataSize() { return m_DataSize; }
	void* GetOffsetData(size_t offset) { return (m_DataSize > 0 ? ((char*)m_pData + offset) : NULL); }

	void Empty();
	size_t AppendMem(const void* pData, size_t size);
	size_t AppendZeroMem(size_t size);
	size_t AppendPointer(const void* p) { return AppendMem((void*)&p, sizeof(p)); }
	size_t AppendInt(unsigned int v) { return AppendMem(&v, sizeof(v)); }
	size_t AppendShort(unsigned short v) { return AppendMem(&v, sizeof(v)); }
	size_t AppendChar(char v) { return AppendMem(&v, sizeof(v)); }
	size_t AppendByte(unsigned char v) { return AppendMem(&v, sizeof(v)); }

	bool LoadFromeFile(const char* filename);
	bool SaveToFile(const char* filename);
	void CutUnusedBuffer();

private:
	void Init(int bufferSize);
	void ReallocIfNeed(size_t newAppendSize);
};

class MemBlocks
{
public: 
	MemBlocks(size_t blockBufferSize = 1024, int blockSize = -1);
	~MemBlocks();
	const void* NewMemBlock(int defaultBlockSize = -1);
	void FreeAll();
	
private:
	size_t m_blockBufferSize;
	int m_defaultBlockSize;
	BufferedMem m_memlist; //an array of BufferedMem*
};


//-----------------------------------------------------------------------------

struct _KV
{
	void* key;
	void* value;
	_KV* next;
};

typedef size_t (*PFN_HashKV_GetHashValue) (void* key);
typedef bool (*PFN_HashKV_IsKeyEquals) (void* key1, void* key2);
typedef void (*PFN_HashKV_DeleteKeyValue) (void* key, void* value);

class HashKV;

/*
Sample codes:

  HashKV hashkv;
  KVIterator i = hashkv.Iterator();
  while(i.Next())
  {
		void* key = i.GetKey();
		void* value = i.GetValue();
  }
*/
class KVIterator
{
private:
	HashKV* m_pHashKV;
	_KV* m_pKV;
	size_t m_index;
public:
	KVIterator();
	KVIterator(HashKV* pHashKV);
	KVIterator(const KVIterator& other);
	const KVIterator& operator= (const KVIterator& other);

	bool Next();
	void* GetKey();
	void* GetValue();

	void Reset();
};

class HashKV
{
	friend class KVIterator;
public:
	HashKV(int nPresumedElementCount = -1);
	~HashKV ();

	void Set (void* key, void* value);
	bool Get (void* key, void** ppValue);
	bool IsExist (void* key);
	bool Remove (void* key);
	void RemoveAll();
	size_t GetCount ();

	KVIterator Iterator() { return KVIterator(this); }

	//all can be NULL
	void SetFunctions ( PFN_HashKV_GetHashValue pfnGetKeyHashValue,
						PFN_HashKV_IsKeyEquals pfnIsKeyEquals,
						PFN_HashKV_DeleteKeyValue pfnDeleteKeyValue);

private:
	void InitMembers();
	void InitHashTable(int nPresumedElementCount);
	_KV* GetKVs (void* key, size_t* pnHash = NULL) const;
	_KV* GetKV (void* key) const;

private:
	size_t m_nCount, m_nHashTableSize;
	_KV** m_pHashTable;

	PFN_HashKV_GetHashValue m_pfnGetHashValue;
	PFN_HashKV_IsKeyEquals m_pfnIsKeyEquals;
	PFN_HashKV_DeleteKeyValue m_pfnDeleteKeyValue;
};

size_t FN_HashKV_DefaultGetHashValue(void* key);
bool FN_HashKV_DefaultIsKeyEquals(void* key1, void* key2);

size_t FN_HashKV_TextGetHashValue(void* key);
bool FN_HashKV_TextIsKeyEquals(void* key1, void* key2);


//以文本(const char*)为键(key)的哈希表
class TextHashMap : public HashKV
{
public:
	TextHashMap(int nPresumedElementCount = -1) : HashKV(nPresumedElementCount)
	{
		SetFunctions(FN_HashKV_TextGetHashValue, FN_HashKV_TextIsKeyEquals, NULL);
	}
};


//void testTextHashMap();

//-----------------------------------------------------------------------------

typedef void* (*PFN_malloc) (size_t memsize);

//从文件中读取数据，并返回数据首地址。
//如果pfnMalloc非NULL，将调用它分配足够的内存，用以存在文件数据，并返回该内存首地址。
//如果pfnMalloc为NULL但pDestBuffer非NULL，则文件数据将被写到pDestBuffer，并返回pDestBuffer。
//如果pDataSize非NULL，将写入文件内容长度（数据长度）。
//如果pfnMalloc和pDestBuffer均为NULL，将返回NULL，但pDataSize仍正常写出。
//如果nSuffixBytes不为0，将在文件数据结尾添加指定个数的0字节数据。
//在文件不存在或文件内容长度为0的情况下，将返回NULL。
void* LoadFileData(const char* filename, PFN_malloc pfnMalloc, void* pDestBuffer = 0, size_t* pDataSize = 0, size_t nSuffixBytes = 0);

//保存数据到文件. 如果指定文件已存在, 将会覆盖其内容.
bool SaveDataToFile(const char* filename, const void* data, size_t datalen);

//strings: '\0'结尾的多个字符串的连续排列
//count: 字符串的个数
void PrintStrings(const char* strings, size_t count, const char* description);

void WriteStringsToBufferedMem(const char* strings, size_t count, BufferedMem& mem);

//requires free() the cloned string, if not NULL
char* strclone(const char* s, size_t n);

//-----------------------------------------------------------------------------

// 与C标准库中的 assert() 相比, __assert() 断言失败并中断后, 仍可继续执行, 不会粗暴地结束程序
// __break_if() 用于设置条件断点, 条件成立时自动中断

#if defined(_DEBUG) || !defined(NDEBUG)
	#define __assert(exp)  do{ if(!(exp)){__assert_failed(#exp, __FILE__, __LINE__); /*__asm int 3*/} }while(0)
	#define __assert_notbreak(exp)  do{ if(!(exp)){__assert_failed(#exp, __FILE__, __LINE__);} }while(0)
	#define __break_if(exp)  do{ if(exp){__asm int 3} }while(0)
	void __assert_failed(const char* info, const char* file, unsigned int line);
#endif

#ifdef NDEBUG
	#undef  __assert
	#define __assert(exp) (void*)(0)
	#undef  __assert_notbreak
	#define __assert_notbreak(exp) (void*)(0)
	#undef  __break_if
	#define __break_if(exp) (void*)(0)
#endif



#endif //__UITLS__H__





















