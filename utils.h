#ifndef __UITLS__H__
#define __UITLS__H__

#include <stdlib.h>

//-----------------------------------------------------------------------------
// by liigo
//-----------------------------------------------------------------------------

//�����ڴ滺����Ƶ��ڴ��
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


//���ı�(const char*)Ϊ��(key)�Ĺ�ϣ��
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

//���ļ��ж�ȡ���ݣ������������׵�ַ��
//���pfnMalloc��NULL���������������㹻���ڴ棬���Դ����ļ����ݣ������ظ��ڴ��׵�ַ��
//���pfnMallocΪNULL��pDestBuffer��NULL�����ļ����ݽ���д��pDestBuffer��������pDestBuffer��
//���pDataSize��NULL����д���ļ����ݳ��ȣ����ݳ��ȣ���
//���pfnMalloc��pDestBuffer��ΪNULL��������NULL����pDataSize������д����
//���nSuffixBytes��Ϊ0�������ļ����ݽ�β���ָ��������0�ֽ����ݡ�
//���ļ������ڻ��ļ����ݳ���Ϊ0������£�������NULL��
void* LoadFileData(const char* filename, PFN_malloc pfnMalloc, void* pDestBuffer = 0, size_t* pDataSize = 0, size_t nSuffixBytes = 0);

//�������ݵ��ļ�. ���ָ���ļ��Ѵ���, ���Ḳ��������.
bool SaveDataToFile(const char* filename, const void* data, size_t datalen);

//strings: '\0'��β�Ķ���ַ�������������
//count: �ַ����ĸ���
void PrintStrings(const char* strings, size_t count, const char* description);

void WriteStringsToBufferedMem(const char* strings, size_t count, BufferedMem& mem);

//requires free() the cloned string, if not NULL
char* strclone(const char* s, size_t n);

//-----------------------------------------------------------------------------

// ��C��׼���е� assert() ���, __assert() ����ʧ�ܲ��жϺ�, �Կɼ���ִ��, ����ֱ��ؽ�������
// __break_if() �������������ϵ�, ��������ʱ�Զ��ж�

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





















