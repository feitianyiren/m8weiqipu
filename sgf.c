#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <ctype.h>
#include <assert.h>

#include "sgf.h"

// SGF parser, by liigo
// email: com.liigo_at_gmail.com
// blog:  http://blog.csdn.net/liigo


int findchar(const char* sz, size_t n, char c)
{
	size_t i;
	if(n == -1)
	{
		char x;
		i = 0;
		while( x = sz[i] )
		{
			if(x == c) return i;
			i++;
		}
		return -1;
	}
	else
	{
		for(i = 0; i < n; i++)
		{
			if(sz[i] == c) return i;
		}
		return -1;
	}
}

//return the first non-space char's index
int skipSpaceChars(const char* s, char* endchars)
{
	const char* p = s;
	char c;
	assert(s);
	while(c = *p)
	{
		if(endchars)
		{
			if(findchar(endchars, -1, c) >= 0)
				return (p - s);
		}
		if(!isspace(c)) return (p - s);
		p++;
	}
	return (p - s);
}

//skipChars or endchars can be NULL, but can not all NULL
int skipChars(const char* s, char* skipChars, char* endchars)
{
	const char* p = s;
	char c;
	assert(s && (skipChars || endchars));
	while(c = *p)
	{
		if(skipChars)
		{
			if(findchar(skipChars, -1, c) == -1)
				return (p - s);
		}
		if(endchars)
		{
			if(findchar(endchars, -1, c) >= 0)
				return (p - s);
		}
		p++;
	}
	return (p - s);
}

int isTextPropertyID(const char* szID)
{
	static const char* textIDs[] = { "C", "N", "AP", "CA", "AN", "BR", "BT", "CP", "DT", "EV",
									 "GN", "GC", "ON", "OT", "PB", "PC", "PW", "RE", "RO", "RU",
									 "SO", "US", "WR", "WT", "FG", "LB" };
	static int n = sizeof(textIDs) / sizeof(textIDs[0]);
	int i;
	for(i = 0; i < n; i++)
	{
		if(_stricmp(szID, textIDs[i]) == 0)
			return 1;
	}
	return 0;
}

void getEnoughBuffer(SGFParseContext* pContext, int nNeedBufferSize)
{			
	if(pContext->valueBuffer == NULL || nNeedBufferSize > pContext->valueBufferSize)
	{
		int buffSize = pContext->valueBufferSize;
		if(buffSize == 0) buffSize = 1024;
		while(buffSize < nNeedBufferSize) buffSize *= 2;

		if(pContext->valueBuffer == NULL)
			pContext->valueBuffer = malloc(buffSize + 4);
		else
			pContext->valueBuffer = realloc(pContext->valueBuffer, buffSize + 4);
		pContext->valueBufferSize = buffSize;
	}
}

// [value]
int parsePropertyValue(SGFParseContext* pContext, const char* szCollection, int fromPos)
{
	const char* szFromPos = szCollection + fromPos;
	assert(szFromPos[0] == '[');

	if(!isTextPropertyID(pContext->idBuffer))
	{
		int rindex = findchar(szFromPos, -1, ']');
		int nNeedBufferSize = rindex - 1;
		assert(rindex >= 0);
		getEnoughBuffer(pContext, nNeedBufferSize);
		memcpy(pContext->valueBuffer, szFromPos + 1, nNeedBufferSize);
		pContext->valueBuffer[nNeedBufferSize] = '\0';

		if(pContext->pfnOnProperty)
			pContext->pfnOnProperty(pContext, pContext->idBuffer, pContext->valueBuffer);

		return (fromPos + rindex + 1);
	}
	else
	{
		//parse the text or simple-text value, consider the '\' escape character
		const char* s = szFromPos + 1;
		char c;
		int in_escape = 0;
		int valuelen = 0;
		getEnoughBuffer(pContext, 1024);
		pContext->valueBuffer[0] = '\0';
		while(1)
		{
			c = *s;
			assert(c);
			if(!in_escape)
			{
				if(c == '\\')
				{
					in_escape = 1;
				}
				else if(c == ']')
				{
					break;
				}
				else
				{
					getEnoughBuffer(pContext, valuelen + 1);
					pContext->valueBuffer[valuelen++] = c;
				}
			}
			else
			{
				//ignore the newline after '\'
				if(c != '\r' && c != '\n')
				{
					getEnoughBuffer(pContext, valuelen + 1);
					pContext->valueBuffer[valuelen++] = c;
				}
				else
				{
					char nc = *(s+1);
					if(nc)
					{
						if((c=='\r' && nc=='\n') || (c=='\n' && nc=='\r'))
							s++;
					}
				}
				in_escape = 0;
			}
			s++;
		}
		getEnoughBuffer(pContext, valuelen + 1);
		pContext->valueBuffer[valuelen] = '\0';

		if(pContext->pfnOnProperty) 
			pContext->pfnOnProperty(pContext, pContext->idBuffer, pContext->valueBuffer);

		return (s - szCollection + 1);
	}
}

//Property: id [value] { [value] }
int parseProperty(SGFParseContext* pContext, const char* szCollection, int fromPos)
{
	const char* szFromPos;
	int lindex;
	int nIDBufferSize = sizeof(pContext->idBuffer) - 1;
	assert(szCollection && fromPos >= 0);
	szFromPos = szCollection + fromPos;

	lindex = findchar(szFromPos, -1, '[');
	assert(lindex > 0 && lindex < nIDBufferSize);
	if(lindex > 0 && lindex < nIDBufferSize)
	{
		memcpy(pContext->idBuffer, szFromPos, lindex);
		pContext->idBuffer[lindex] = '\0';

		fromPos = parsePropertyValue(pContext, szCollection, fromPos + lindex);
		szFromPos = szCollection + fromPos;
		while(1)
		{
			fromPos += skipSpaceChars(szFromPos, NULL);
			if(szCollection[fromPos] != '[')
				break;
			fromPos = parsePropertyValue(pContext, szCollection, fromPos);
			szFromPos = szCollection + fromPos;
		}
		return fromPos;
	}
	return -1;
}

//Node: ; {property}
int parseNode(SGFParseContext* pContext, const char* szCollection, int fromPos)
{
	const char* szFromPos = szCollection + fromPos;
	assert(fromPos >= 0);
	//assert(szFromPos[0] == ';');

	if(pContext->pfnOnNode) 
		pContext->pfnOnNode(pContext, szFromPos);

	if(szFromPos[0] == ';')
	{
		fromPos++; szFromPos++;
	}

	while(1)
	{
		fromPos += skipSpaceChars(szFromPos, NULL);
		if(szCollection[fromPos] == '\0' || findchar(";)(", -1, szCollection[fromPos]) >= 0)
			break;
		fromPos = parseProperty(pContext, szCollection, fromPos);
		szFromPos = szCollection + fromPos;
	}
	return fromPos;
}

//NodeSequence: node{node}
int parseNodeSequence(SGFParseContext* pContext, const char* szCollection, int fromPos)
{
	const char* szFromPos = szCollection + fromPos;
	assert(fromPos >= 0);
	//assert(szFromPos[0] == ';');
	while(1)
	{
		fromPos = parseNode(pContext, szCollection, fromPos);
		fromPos += skipSpaceChars(szFromPos, NULL);
		szFromPos = szCollection + fromPos;
		if(szFromPos[0] != ';')
		{
			if(pContext->pfnOnNodeEnd)
				pContext->pfnOnNodeEnd(pContext);
			break;
		}
	}
	return fromPos;	
}

//GameTree: ( {[NodeSequence]|[GameTree]} )
//old GameTree: ( NodeSequence {GameTree} )
int parseGameTree(SGFParseContext* pContext, const char* szCollection, int fromPos)
{
	char c;
	const char* szFromPos = szCollection + fromPos;
	assert(fromPos >= 0);
	assert(szFromPos[0] == '(');

	pContext->treeIndex++;
	if(pContext->pfnOnTree)
		pContext->pfnOnTree(pContext, szFromPos, pContext->treeIndex);

	fromPos++; szFromPos++;
	fromPos += skipSpaceChars(szFromPos, NULL);

	c = szCollection[fromPos];
	while(1)
	{
		if(c == '(')
			fromPos = parseGameTree(pContext, szCollection, fromPos);
		else
			fromPos = parseNodeSequence(pContext, szCollection, fromPos);

		szFromPos = szCollection + fromPos;
		fromPos += skipSpaceChars(szFromPos, NULL);
		c = szCollection[fromPos];
		if(c == ')')
		{
			if(pContext->pfnOnTreeEnd)
				pContext->pfnOnTreeEnd(pContext, pContext->treeIndex);
			pContext->treeIndex--;
			break;
		}
	}

	return (fromPos + 1);
}

//SGFCollection: GameTree {GameTree}
int parseSGF(SGFParseContext* pContext, const char* szCollection, int fromPos)
{
	const char* szFromPos = szCollection + fromPos;
	assert(fromPos >= 0);
	assert(szFromPos[0] == '(');
	pContext->treeIndex = -1;
	while(1)
	{
		fromPos = parseGameTree(pContext, szCollection, fromPos);
		fromPos += skipSpaceChars(szFromPos, NULL);
		szFromPos = szCollection + fromPos;
		if(szFromPos[0] != '(')
			break;
	}
	return fromPos;	
}

void initSGFParseContext(SGFParseContext* pContext,
						 PFN_ON_TREE pfnOnTree, PFN_ON_TREE_END pfnOnTreeEnd,
						 PFN_ON_NODE pfnOnNode, PFN_ON_NODE_END pfnOnNodeEnd, 
						 PFN_ON_PROPERTY pfnOnProperty, 
						 void* pUserData)
{
	assert(pContext);
	pContext->pfnOnTree = pfnOnTree;
	pContext->pfnOnTreeEnd = pfnOnTreeEnd;
	pContext->pfnOnNode = pfnOnNode;
	pContext->pfnOnNodeEnd = pfnOnNodeEnd;
	pContext->pfnOnProperty = pfnOnProperty;
	pContext->pUserData = pUserData;
	pContext->idBuffer[0] = '\0';
	pContext->valueBuffer = NULL;
	pContext->valueBufferSize = 0;
	pContext->treeIndex = -1;
}

void cleanupSGFParseContext(SGFParseContext* pContext)
{
	if(pContext->valueBuffer)
		free(pContext->valueBuffer);
	memset(pContext, 0, sizeof(SGFParseContext));
}

//
//-----------------------------------------------------------------------------
// 以下是测试代码

static void printheader(const char* s, int n)
{
	int i;
	for(i = 0; i < n; i++)
	{
		if(s[i] == '\0') break;
		printf("%c", s[i]);
	}
	printf(" ... \n");
	fflush(stdout);
}

static void onProperty(SGFParseContext* pContext, const char* szID, const char* szValue)
{
	printf("    onProperty: %s[%s] \n", szID, szValue);
	fflush(stdout);
}

static void onNode(SGFParseContext* pContext, const char* szNodeHeader)
{
	printf("  onNode: ");
	printheader(szNodeHeader, 32);
}

static void onNodeEnd(SGFParseContext* pContext)
{
	printf("  onNodeEnd \n");
}


static void onTree(SGFParseContext* pContext, const char* szTreeHeader, int treeIndex)
{
	printf("  onTree: ");
	printheader(szTreeHeader, 32);
}

static void onTreeEnd(SGFParseContext* pContext, int treeIndex)
{
	printf("  onTreeEnd \n");
}

//------------------

static void onProperty2(SGFParseContext* pContext, const char* szID, const char* szValue)
{
	printf("%s[%s] ", szID, szValue);
	//fflush(stdout);
}

static void onNode2(SGFParseContext* pContext, const char* szNodeHeader)
{
	printf("; ");
}

static void onNodeEnd2(SGFParseContext* pContext)
{
}

static void onTree2(SGFParseContext* pContext, const char* szTreeHeader, int treeIndex)
{
	int i;
	printf("\n");
	for(i = 0; i < treeIndex; i++)
		printf("  ");
	printf("[tree:%d]( ", treeIndex);
}

static void onTreeEnd2(SGFParseContext* pContext, int treeIndex)
{
	printf(") [end of tree %d]\n\n", treeIndex);
}

static void testParseSGFFile(SGFParseContext* pContext, const char* szFileName)
{
	int len = 0;
	void* data = NULL;
	FILE* pfile = fopen(szFileName, "r");
	printf("\n\n--- test parse sgf file: %s ---\n", szFileName);
	if(pfile)
	{
		fseek(pfile, 0, SEEK_END);
		len = ftell(pfile);
		assert(len > 0);
		fseek(pfile, 0, SEEK_SET);
		data = malloc(len);
		assert(data);
		fread(data, 1, len, pfile);

		parseSGF(pContext, data, 0);

		fclose(pfile);
		pfile = NULL;
	}
	else
	{
		printf("\n\n--- open sgf file error: %s ---\n\n", szFileName);
	}
}

////////////////////


int main(int argc, char *argv[])
{
	char* s;
	int x;
	SGFParseContext Context;
	//initSGFParseContext(&Context, onTree, onTreeEnd, onNode, onNodeEnd, onProperty, NULL);
	initSGFParseContext(&Context, onTree2, onTreeEnd2, onNode2, onNodeEnd2, onProperty2, NULL);

	//test parse property:
	{
		s = "A[a]B[b1][b2]X[xyz]";
		printf("\ntest parse property: ----- \n");
		x = parseProperty(&Context, s, 0);
		x = parseProperty(&Context, s, 4);
		s = "C[ab\\]cd]";
		x = parseProperty(&Context, s, 0);
	}
	//test parse node:
	{
		s = ";A[a]BB[bb]C[]";
		printf("\ntest parse node: ----- \n");
		x = parseNode(&Context, s, 0);
		s = ";A[a];BB[bb]C[]";
		x = parseNode(&Context, s, 0);
		x = parseNodeSequence(&Context, s, 0);
	}
	//test parse tree:
	{
		printf("\ntest parse tree: ----- \n");
		s = "(;A[a](;C[c](X[x])Z[z]);D[d](;E[e](F[ff])))";
		x = parseGameTree(&Context, s, 0);
	}

#if 1
	//parse real sgf file:
	{
		char filename[256];
		int i;

		for(i = 1; i <= 7; i++)
		{
			sprintf(filename, "%d.sgf", i);
			testParseSGFFile(&Context, filename);
		}

		for(i = 0; i <= 365; i++)
		{
			sprintf(filename, "WuQingYuan\\wqy (%d).sgf", i);
			testParseSGFFile(&Context, filename);
		}

		for(i = 1; i <= 465; i++)
		{
			sprintf(filename, "标准中国流645局\\S%05d.sgf", i);
			testParseSGFFile(&Context, filename);
		}
	}
#endif

	{
		char c;
		printf("\n----- any key to exit: ----- \n");
		fflush(stdout);
		scanf("%c", &c);
	}
}

