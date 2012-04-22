#ifndef __HtmlParser_H__
#define __HtmlParser_H__

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

namespace liigo
{

//HtmlParser类，用于解析HTML文本
//by liigo, @2010-2012

//MemBuffer: 内存缓冲区类，
#define MEM_DEFAULT_BUFFER_SIZE  256
class MemBuffer
{
public:
	//nBufferSize指定缓存区初始大小(字节数), 为-1表示使用默认初始大小(MEM_DEFAULT_BUFFER_SIZE)
	//nBufferSize为0时暂不分配缓存区内存，延迟到下一次写入数据时再分配
	MemBuffer(size_t nBufferSize = -1);
	MemBuffer(const MemBuffer& other); //从other对象复制数据，缓存区是自行分配的
	~MemBuffer(); //析构时清理缓存区释放内存，除非已经detach()
	const MemBuffer& operator= (const MemBuffer& other); //清空数据后再把other内的数据复制进来

public:
	//向缓存区复制数据块，写入现有数据的末尾，必要时会自动扩充缓存区
	//返回新写入的数据块首地址在缓存区中的偏移量
	size_t appendData(void* pData, size_t nSize);
	//取数据首地址(在数据长度为0时返回NULL)，此地址非NULL时也就是缓存区首地址
	//在appendXXX()或resetSize()或exchange()或operator=调用之后可能会导致数据首地址发生改变
	void* getData() const { return (m_nDataSize == 0 ? NULL : m_pBuffer); }
	//取指定偏移处数据地址，偏移offset应小于getDataSize()，否则不保证返回的地址有效
	void* getOffsetData(int offset) const { return (m_nDataSize == 0 ? NULL : ((unsigned char*)m_pBuffer + offset)); }
	//取数据长度
	size_t getDataSize() const { return m_nDataSize; }
	//重置数据长度，新长度可以为任意值，必要时会自动扩充缓存区
	void resetDataSize(size_t size = 0);
	//清理缓存区，释放内存
	void clean();
	//放弃管理缓存区和其中的数据，用户应自行负责用free()释放数据:
	//数据首地址为detach()前getData()返回的地址，数据长度为detach()前getDataSize()返回的长度
	void detach();
	//交换两个对象(this & other)各自管理的所有内容（包括数据和缓存区）
	void exchange(MemBuffer& other);

	//添加基本类型数据
	size_t appendInt(int i) { return appendData(&i, sizeof(i)); }
	size_t appendChar(char c) { return appendData(&c, sizeof(c)); }
	//把指针p本身的值（而非p指向的数据）添加到缓存区
	size_t appendPointer(void* p) { return appendData(&p, sizeof(p)); }

private:
	//要求缓存区中至少留出长度为size的未使用空间
	//返回未使用空间的首地址，即现有数据的末尾
	void* require(size_t size);

private:
	unsigned char* m_pBuffer; //缓存区首地址
	size_t m_nDataSize, m_nBufferSize; //数据长度，缓存区长度
};

enum HtmlNodeType
{
	NODE_UNKNOWN = 0,
	NODE_START_TAG, //开始节点，如 <a>
	NODE_CLOSE_TAG, //结束节点，如 </a>
	NODE_CONTENT,   //文本
	NODE_REMARKS,   //注释 <!-- -->
};

enum HtmlTagType
{
	TAG_UNKNOWN = 0,
	TAG_SCRIPT, TAG_STYLE, //出于解析需要必须识别,内部特别处理
	TAG_A, TAG_B, TAG_BODY, TAG_BR, TAG_DIV, TAG_FONT, TAG_HR, TAG_I, TAG_IMG, TAG_META, 
	TAG_P, TAG_SPAN, TAG_TITLE, 
	//TAG_COLOR, TAG_BGCOLOR, //非标准HTML标签, 可以这样使用: <color=red>, 等效于 <color color=red>
};

struct HtmlNodeProp
{
	char* szName;
	char* szValue;
};

#define MAX_HTML_TAG_LENGTH  15 //节点名称的最大字符长度

struct HtmlNode
{
	HtmlNodeType type;
	HtmlTagType  tagType;
	char tagName[MAX_HTML_TAG_LENGTH+1];
	char* text;
	int propCount;
	HtmlNodeProp* props;
	void* pUser; //user customized, default to NULL
};


class HtmlParser
{
public:
	HtmlParser() {}
	~HtmlParser() { freeHtmlNodes(); }

public:
	//html
	void parseHtml(const char* szHtml, bool parseProps = false);

	//nodes
	int getHtmlNodeCount();
	HtmlNode* getHtmlNodes(int i);
	//props
	const HtmlNodeProp* getNodeProp(const HtmlNode* pNode, const char* szPropName);
	const char* getNodePropStringValue(const HtmlNode* pNode, const char* szPropName, const char* szDefaultValue = NULL);
	int getNodePropIntValue(const HtmlNode* pNode, const char* szPropName, int defaultValue = 0);
	//debug
	void dumpHtmlNodes(FILE* f = stdout);
protected:
	//允许子类覆盖, 以便识别更多结点(提高解析质量), 或者识别更少结点(提高解析速度)
	//默认仅识别涉及HTML基本结构和信息的有限几个TAG: A,IMG,META,BODY,TITLE
	virtual HtmlTagType identifyHtmlTag(const char* szTagName, HtmlNodeType nodeType);
	//允许子类覆盖, 以便更好的解析节点属性, 或者部分解析甚至干脆不解析节点属性(提高解析速度)
	//可以根据节点名称(pNode->tagName)或节点类型(pNode->tagType)判断是否需要解析属性
	virtual void parseNodeProps(HtmlNode* pNode);

private:
	HtmlNode* newHtmlNode();
	void freeHtmlNodes();

private:
	MemBuffer m_HtmlNodes;
};

} //namespace liigo

#endif //__HtmlParser_H__
