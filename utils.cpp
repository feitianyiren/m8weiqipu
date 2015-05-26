#include "utils.h"

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <limits.h>


//见头文件中的说明
void* LoadFileData(const char* filename, PFN_malloc pfnMalloc, void* pDestBuffer, size_t* pDataSize, size_t nSuffixBytes)
{
	FILE* pfile = fopen(filename, "rb");
	if(pfile)
	{
		fseek(pfile, 0, SEEK_END);
		long fsize = ftell(pfile);
		if(pDataSize) *pDataSize = (size_t)fsize;
		if(fsize == 0)
		{
			fclose(pfile);
			return NULL;
		}
		
		void* pData = NULL;
		if(pfnMalloc)
		{
			pData = pfnMalloc(fsize + nSuffixBytes);
		}
		else if(pDestBuffer)
		{
			pData = pDestBuffer;
		}
		else
		{
			fclose(pfile);
			return NULL;
		}

		__assert(pData);
		memset((char*)pData + fsize, 0, nSuffixBytes);

		fseek(pfile, 0, SEEK_SET);
		size_t readbytes = fread(pData, 1, fsize, pfile);
		__assert(readbytes == (size_t)fsize);
		fclose(pfile);
		return pData;
	}
	return NULL;
}

bool SaveDataToFile(const char* filename, const void* data, size_t datalen)
{
	FILE* pfile = fopen(filename, "wb");
	if(pfile)
	{
		fwrite(data, 1, datalen, pfile);
		fclose(pfile);
		return true;
	}
	return false;
}

//strings: '\0'结尾的多个字符串的连续排列
//count: 字符串的个数
void PrintStrings(const char* strings, size_t count, const char* description)
{
	__assert(strings && count >=0);
	const char* ps = strings;
	printf("%s:\n", description ? description : "Strings");
	for(size_t i = 0; i < count; i++)
	{
		printf("\t%s\n", ps);
		ps += strlen(ps) + sizeof('\0');
	}
	printf("End of %s.\n\n", description ? description : "Strings");
}

void WriteStringsToBufferedMem(const char* strings, size_t count, BufferedMem& mem)
{
	__assert(strings && count >=0);
	const char* ps = strings;
	for(size_t i = 0; i < count; i++)
	{
		mem.AppendPointer(ps);
		ps += strlen(ps) + sizeof('\0');
	}
}

//-----------------------------------------------------------------------------

BufferedMem::BufferedMem(int BufferSize /*= 0*/)
{
	Init(BufferSize);
}

void BufferedMem::Init(int bufferSize)
{
	if(bufferSize <= 0) bufferSize = 128;

	m_DataSize = 0;

	m_pData = (char*) malloc(bufferSize);
	m_BufferSize = bufferSize;
}

size_t BufferedMem::AppendMem(const void* pData, size_t size)
{
	__assert(pData);
	ReallocIfNeed(size);
	memcpy(m_pData + m_DataSize, pData, size);
	m_DataSize += size;
	return (m_DataSize - size);
}

size_t BufferedMem::AppendZeroMem(size_t size)
{
	ReallocIfNeed(size);
	memset(m_pData + m_DataSize, 0, size);
	m_DataSize += size;
	return (m_DataSize - size);
}

void BufferedMem::ReallocIfNeed(size_t newAppendSize)
{
	__assert(newAppendSize > 0 && m_pData);

	if(m_BufferSize - m_DataSize < newAppendSize)
	{
		do 
		{
			m_BufferSize *= 2;
		}
		while(m_BufferSize - m_DataSize < newAppendSize);

		m_pData = (char*) realloc(m_pData, m_BufferSize);
	}
}

void BufferedMem::CutUnusedBuffer()
{
	if(m_pData)
	{
		m_BufferSize = m_DataSize;
		m_pData = (char*) realloc(m_pData, m_BufferSize);
	}
}

bool BufferedMem::LoadFromeFile(const char* filename)
{
	FILE* pfile = fopen(filename, "rb");
	if(pfile)
	{
		fseek(pfile, 0, SEEK_END);
		long fsize = ftell(pfile);
		__assert(fsize > 0);
		ReallocIfNeed(fsize);
		fseek(pfile, 0, SEEK_SET);
		size_t readbytes = fread(m_pData, 1, fsize, pfile);
		__assert(readbytes == (size_t)fsize);
		m_DataSize = fsize;
		fclose(pfile);
		return true;
	}
	return false;
}

bool BufferedMem::SaveToFile(const char* filename)
{
	FILE* pfile = fopen(filename, "wb");
	if(pfile)
	{
		size_t wrotebytes = 0;
		if(m_DataSize > 0)
		{
			wrotebytes = fwrite(m_pData, 1, m_DataSize, pfile);
		}
		fclose(pfile);
		return (wrotebytes == m_DataSize);
	}
	return false;
}

void BufferedMem::Empty()
{
	m_DataSize = 0;
}

//-----------------------------------------------------------------------------

MemBlocks::MemBlocks(size_t blockBufferSize /*= 1024*/, int defaultBlockSize /*= -1*/)
	: m_memlist(36), m_blockBufferSize(blockBufferSize), m_defaultBlockSize(defaultBlockSize)
{
	__assert(m_defaultBlockSize <= 0 || m_defaultBlockSize <= m_blockBufferSize);
}

const void* MemBlocks::NewMemBlock(int blockSize /*= -1*/)
{
	if(blockSize <= 0)
	{
		__assert(m_blockBufferSize > 0);
		blockSize = m_defaultBlockSize;
	}

	BufferedMem* pLastMem = NULL;
	if(m_memlist.GetDataSize() > 0)
	{
		pLastMem = *(BufferedMem**) ((char*)m_memlist.GetData() + m_memlist.GetDataSize() - sizeof(BufferedMem*));
	}

	//确认每一个BufferedMem*中的内存大小均不大于m_blockBufferSize, 从而保证之前分配出的地址始终有效
	if(pLastMem && pLastMem->GetDataSize() + blockSize <= m_blockBufferSize)
	{
		//append to the last BufferedMem*
		size_t offset = pLastMem->AppendZeroMem(blockSize);
		return pLastMem->GetOffsetData(offset);
	}
	else
	{
		//create new BufferedMem*
		BufferedMem* pNewMem = new BufferedMem(m_blockBufferSize);
		size_t offset = pNewMem->AppendZeroMem(blockSize);
		m_memlist.AppendPointer(pNewMem);
		return pNewMem->GetOffsetData(offset);
	}
}

void MemBlocks::FreeAll()
{
	BufferedMem** ppMem = (BufferedMem**) m_memlist.GetData();
	for(int i = (int)m_memlist.GetDataSize() / sizeof(BufferedMem*) - 1; i >= 0; i--)
	{
		ppMem[i]->Empty();
	}
}

MemBlocks::~MemBlocks()
{
	FreeAll();
}

//-----------------------------------------------------------------------------

HashKV::HashKV(int nPresumedElementCount)
{
	InitMembers();
	InitHashTable(nPresumedElementCount);
	SetFunctions(FN_HashKV_DefaultGetHashValue, FN_HashKV_DefaultIsKeyEquals, NULL);
}

HashKV::~HashKV ()
{
	RemoveAll ();
	if(m_pHashTable)
		free(m_pHashTable);
}


void HashKV::InitMembers()
{
	m_nCount = 0;
	m_pHashTable = NULL;
	m_nHashTableSize = 0;
	
	m_pfnGetHashValue = NULL;
	m_pfnIsKeyEquals = NULL;
	m_pfnDeleteKeyValue = NULL;
}

void HashKV::InitHashTable(int nPresumedElementCount)
{
	RemoveAll ();
	
	if (nPresumedElementCount <= 0)
        nPresumedElementCount = 1024;

	int nHashSize = (int)(nPresumedElementCount * 3 * 0.618);
	static int s_nPrimeNums [] =
    {
        257, 509, 1021, 2053, 4093, 8191, 16381, 32771, 65521, 131071, 262007, 525013, 
			1050239, 2100001, 4201123, 8402357, 16805323, 33610001, 67220089, INT_MAX
    };
	
    int i = 0;
	while (s_nPrimeNums [i] < nHashSize)
        i++;

    nHashSize = s_nPrimeNums [i];
	
	if (m_pHashTable)
        delete[] m_pHashTable;
	m_pHashTable = (_KV**) malloc(sizeof(_KV*) * nHashSize);
	memset (m_pHashTable, 0, sizeof(_KV*) * nHashSize);
	
	m_nHashTableSize = nHashSize;
}

void HashKV::RemoveAll()
{
	if (m_pHashTable)
	{
		_KV *pKV, *pTempKV;
		for (size_t i = 0; i < m_nHashTableSize; i++)
		{
			pKV = m_pHashTable [i];
			while (pKV)
			{
				if (m_pfnDeleteKeyValue)
					m_pfnDeleteKeyValue(pKV->key, pKV->value);
				pTempKV = pKV;
				pKV = pKV->next;
				free(pTempKV);
			}
		}

		memset (m_pHashTable, 0, sizeof (_KV*) * m_nHashTableSize);
	}
	
	m_nCount = 0;
}


bool HashKV::Remove (void* key)
{
	size_t nHash;
	_KV* pKVs = GetKVs (key, &nHash);
	if (pKVs == NULL)
        return false;  // not found
	
    if (m_pfnIsKeyEquals (key, pKVs->key))
    {
        _KV* pTempKV = pKVs;
        if (m_pfnDeleteKeyValue)
            m_pfnDeleteKeyValue (pKVs->key, pKVs->value);
		m_pHashTable[nHash] = pKVs->next;
        free(pTempKV);
        m_nCount--;
        return true;
    }
	
	_KV* pKV = pKVs->next; _KV* pPrevKV = pKVs;
	while (pKV)
	{
		if (m_pfnIsKeyEquals (key, pKV->key))
		{
			_KV* pTempKV  = pKV;
			pPrevKV->next = pKV->next;
			
			if (m_pfnDeleteKeyValue)
				m_pfnDeleteKeyValue (pTempKV->key, pTempKV->value);
			free (pTempKV);
			m_nCount--;
			return true;
		}
		
		pPrevKV = pKV;
		pKV = pKV->next;
	}
	
    return false;
}


void HashKV::Set (void* key, void* value)
{
	if (m_pHashTable == NULL)
		InitHashTable (m_nHashTableSize);
	
	_KV* pKV = GetKV (key);
	if (pKV)
	{
		if (m_pfnDeleteKeyValue)
			m_pfnDeleteKeyValue(pKV->key, pKV->value);
		pKV->key = key;
		pKV->value = value;
	}
	else
	{
		_KV* pNewKV = (_KV*) malloc(sizeof(_KV));
		pNewKV->key = key;
		pNewKV->value = value;
		pNewKV->next = NULL;
		
		size_t nHash;
		_KV* pKVs = GetKVs (key, &nHash);
		if (pKVs)
		{
			//add pNewKV to pKVs's end
			pKV = pKVs;
			while(pKV && pKV->next != NULL) pKV = pKV->next;
			pKV->next = pNewKV;
		}
		else
			m_pHashTable [nHash] = pNewKV;
		
		m_nCount++;
	}
}

bool HashKV::IsExist (void* key)
{
    return (GetKV (key) != NULL);
}

bool HashKV::Get (void* key, void** ppValue)
{
	_KV* pKV = GetKV (key);
	if (pKV)
	{
		if (ppValue)
            *ppValue = pKV->value;
		return true;
	}
	else
	{
		//if (ppValue)
        //    *ppValue = NULL;
		return false;
	}
}

_KV* HashKV::GetKVs (void* key, size_t* pnHash) const
{
	size_t nHash = m_pfnGetHashValue(key) % m_nHashTableSize; 
	if (pnHash)
		*pnHash = nHash;
	
	if (m_pHashTable == NULL || m_nCount == 0)
        return NULL;
    else
		return m_pHashTable [nHash];
}

_KV* HashKV::GetKV (void* key) const
{
	for (_KV* pKV = GetKVs (key); pKV != NULL; pKV = pKV->next)
	{
		if (m_pfnIsKeyEquals (pKV->key, key))
			return pKV;
	}
	return NULL;
}

size_t HashKV::GetCount ()
{
	return m_nCount;
}

void HashKV::SetFunctions (PFN_HashKV_GetHashValue pfnGetKeyHashValue, 
						   PFN_HashKV_IsKeyEquals pfnIsKeyEquals,
						   PFN_HashKV_DeleteKeyValue pfnDeleteKeyValue)
{
	m_pfnGetHashValue = (pfnGetKeyHashValue ? pfnGetKeyHashValue : FN_HashKV_DefaultGetHashValue);
	m_pfnIsKeyEquals = (pfnIsKeyEquals ? pfnIsKeyEquals : FN_HashKV_DefaultIsKeyEquals);
	m_pfnDeleteKeyValue = pfnDeleteKeyValue;
}

size_t FN_HashKV_DefaultGetHashValue(void* key)
{
	return (size_t)key;
}

bool FN_HashKV_DefaultIsKeyEquals(void* key1, void* key2)
{
	return (key1 == key2);
}

// BKDR Hash Function
// see: http://www.byvoid.com/blog/string-hash-compare/
size_t FN_HashKV_TextGetHashValue(void* key)
{
	const char* str = (const char*) key;
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

bool FN_HashKV_TextIsKeyEquals(void* key1, void* key2)
{
	return (strcmp((const char*)key1, (const char*)key2) == 0);
}

//-----------------------------------------------------------------------------
// KVIterator

KVIterator::KVIterator()
{
	m_pHashKV = NULL;
	Reset();
}

KVIterator::KVIterator(HashKV* pHashKV)
{
	__assert(pHashKV);
	m_pHashKV = pHashKV;
	Reset();
}

KVIterator::KVIterator(const KVIterator& other)
{
	*this = other;
}

const KVIterator& KVIterator::operator= (const KVIterator& other)
{
	m_pHashKV = other.m_pHashKV;
	m_pKV = other.m_pKV;
	m_index = other.m_index;
	return *this;
}

bool KVIterator::Next()
{
	__assert(m_pHashKV);

	if (m_pKV && m_pKV->next)
	{
		m_pKV = m_pKV->next;
		return true;
	}
	else
	{
		__assert(m_pHashKV);
		for (size_t i = m_index + 1; i < m_pHashKV->m_nHashTableSize; i++)
		{
			_KV* pKV = m_pHashKV->m_pHashTable[i];
			if (pKV)
			{
				m_index = i;
				m_pKV = pKV;
				return true;
			}
		}
		//the end, no more KVs
		m_index = m_pHashKV->GetCount();
		m_pKV = NULL;
		return false;
	}
}

void* KVIterator::GetKey()
{
	__assert(m_pKV);
	return m_pKV->key;
}

void* KVIterator::GetValue()
{
	__assert(m_pKV);
	return m_pKV->value;
}

void KVIterator::Reset()
{
	m_index = -1;
	m_pKV = NULL;
}

void testTextHashMap()
{
	TextHashMap map;
	map.Set("a", (void*)1);
	map.Set("b", (void*)2);
	map.Set("c", (void*)3);
	map.Set("aaa", (void*)5);
	map.Set("zdd", (void*)8);
	map.Set("a", (void*)6);
	map.Set("A", (void*)10);
	
	int v = 0;
	map.Get("aaa", (void**)&v);
	
	printf("count: %d\n", map.GetCount());
	
	KVIterator i = map.Iterator();
	while(i.Next())
	{
		printf("  %s = %d \n", (const char*)i.GetKey(), (int)i.GetValue());
	}
}

//-----------------------------------------------------------------------------

//requires free() the cloned string, if not NULL
char* strclone(const char* s, size_t n)
{
	if(s == NULL) return NULL;
	if(n == (size_t)-1)
		n = strlen(s);
	char* ns = (char*) malloc(n + 1);
	memcpy(ns, s, n);
	ns[n] = '\0';
	return ns;
}

//-----------------------------------------------------------------------------

#if _DEBUG || !NDEBUG
void __assert_failed(const char* info, const char* file, unsigned int line)
{
	printf("\007\nAssert failed: %s \n  at file \"%s\", %d line. \n", info, file, line);
}
#endif















