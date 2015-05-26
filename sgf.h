#ifndef __SGF_H__
#define __SGF_H__

// SGF parser, by liigo
// email: com.liigo_at_gmail.com
// blog:  http://blog.csdn.net/liigo


#ifdef __cplusplus
	extern "C" {
#endif

typedef struct _tagSGFParseContext SGFParseContext;

typedef void (*PFN_ON_PROPERTY) (SGFParseContext* pContext, const char* szID, const char* szValue);
typedef void (*PFN_ON_NODE) (SGFParseContext* pContext, const char* szNodeHeader);
typedef void (*PFN_ON_NODE_END) (SGFParseContext* pContext);
typedef void (*PFN_ON_TREE) (SGFParseContext* pContext, const char* szTreeHeader, int treeIndex);
typedef void (*PFN_ON_TREE_END) (SGFParseContext* pContext, int treeIndex);

typedef struct _tagSGFParseContext
{
	void* pUserData;
	int treeIndex;

	PFN_ON_TREE pfnOnTree;
	PFN_ON_TREE_END pfnOnTreeEnd;
	PFN_ON_NODE pfnOnNode;
	PFN_ON_NODE_END pfnOnNodeEnd;
	PFN_ON_PROPERTY pfnOnProperty;

	char idBuffer[16];
	char* valueBuffer;
	int valueBufferSize;
}
SGFParseContext;

void initSGFParseContext(SGFParseContext* pContext,
						 PFN_ON_TREE pfnOnTree, PFN_ON_TREE_END pfnOnTreeEnd,
						 PFN_ON_NODE pfnOnNode, PFN_ON_NODE_END pfnOnNodeEnd, 
						 PFN_ON_PROPERTY pfnOnProperty, 
						 void* pUserData);

void cleanupSGFParseContext(SGFParseContext* pContext);

int parseSGF(SGFParseContext* pContext, const char* szCollection, int fromPos);

int parseGameTree(SGFParseContext* pContext, const char* szCollection, int fromPos);
int parseNodeSequence(SGFParseContext* pContext, const char* szCollection, int fromPos);
int parseNode(SGFParseContext* pContext, const char* szCollection, int fromPos);
int parseProperty(SGFParseContext* pContext, const char* szCollection, int fromPos);


#ifdef __cplusplus
	}
#endif

#endif //__SGF_H__
