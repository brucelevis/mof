/*--------------------------------------------------------------------
	created:	2011/03/22
	filename: 	c:\Documents and Settings\deeple\My Documents\Visual Studio 2005\Projects\TestCout\TestCout\StringFilter.h
	author:		
	
	purpose:	
--------------------------------------------------------------------*/

#pragma once

#include <string>
#include <map>

using namespace std;

// 文本过滤器节点
class _TxtFilterNode
{
public:
	_TxtFilterNode(){};
	_TxtFilterNode(wchar_t name){m_name = name;}
	~_TxtFilterNode();

	// 查找子
	_TxtFilterNode* FindChild(wchar_t childname);

	// 添加子
	_TxtFilterNode* AddChild(wchar_t childname);

public:
	wchar_t m_name;
	std::map<wchar_t, _TxtFilterNode*> child;
};

// 
class TextFilter
{
public:
	TextFilter();
	~TextFilter();

public:
	// 加载所有
	bool LoadFile(const char* filename);

	// 添加词组
	void AddNode(const wchar_t* word);

	// 查词
	bool FindWord(const wchar_t* word);

	// 替换词
	bool ReplaceWord(wchar_t* word, wchar_t rep=L'*');

public:
	_TxtFilterNode root;
};

extern TextFilter g_TextFilter;
