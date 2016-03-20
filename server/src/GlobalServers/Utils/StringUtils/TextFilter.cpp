/*--------------------------------------------------------------------
	created:	2011/03/22
	filename: 	c:\Documents and Settings\deeple\My Documents\Visual Studio 2005\Projects\TestCout\TestCout\StringFilter.cpp
	author:

	purpose:
--------------------------------------------------------------------*/

#include "TextFilter.h"
#include <fstream>
#include "../StringUtils/StringUtil.h"

TextFilter g_TextFilter;

_TxtFilterNode::~_TxtFilterNode()
{
	std::map<wchar_t, _TxtFilterNode*>::iterator it;

	for (it = child.begin(); it != child.end(); it++)
	{
		delete it->second;
	}
}

// 查找子
_TxtFilterNode* _TxtFilterNode::FindChild(wchar_t childname)
{
	std::map<wchar_t, _TxtFilterNode*>::iterator it = child.find(childname);

	if (it == child.end())
	{
		return NULL;
	}

	return it->second;
}

// 添加子
_TxtFilterNode* _TxtFilterNode::AddChild(wchar_t childname)
{
	if (FindChild(childname)) return NULL;

	_TxtFilterNode* newNode = new _TxtFilterNode(childname);
	child[childname] = newNode;

	return newNode;
}

////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
TextFilter::TextFilter()
{

}

TextFilter::~TextFilter()
{

}

// 加载所有
bool TextFilter::LoadFile(const char* filename)
{
	std::locale::global(std::locale(""));

	ifstream fin;
	fin.open(filename);

	const int LINE_LENGTH = 1024;
	char str[LINE_LENGTH];
	while( fin.getline(str, LINE_LENGTH) )
	{
		AddNode(StringAsWideStr(str).c_str());
	}

	return root.child.size() > 0;
}

// 添加词组
void TextFilter::AddNode(const wchar_t* word)
{
	if (!word) return;

	wchar_t val;

	_TxtFilterNode* current = &root;
	_TxtFilterNode* temp = NULL;

	while (val = *word++)
	{
		// 在current中查找
		temp = current->FindChild(val);

		if (temp)
		{
			current = temp;
		}
		else
		{
			current = current->AddChild(val);
		}
	}
}

// 查词
bool TextFilter::FindWord(const wchar_t* word)
{
	if (!word) return false;

	wchar_t val;

	_TxtFilterNode* current = &root;
	_TxtFilterNode* temp;

	while (val = *word++)
	{
		temp = current->FindChild(val);

		if (temp)
		{
			// 找到子
			current = temp;

			if (current->child.size() == 0)
			{
				return true;
			}
		}
		else
		{
			if (current != &root)
			{
				current = &root;
				word --;
			}
		}
	}

	return false;
}

// 替换词
bool TextFilter::ReplaceWord(wchar_t* word, wchar_t rep)
{
	if (!word) return false;

	bool ret = false;

	wchar_t val;

	_TxtFilterNode* current = &root;
	_TxtFilterNode* temp;

	int depth = 0;
	while (val = *word++)
	{
		temp = current->FindChild(val);

		if (temp)
		{
			depth ++;

			// 找到子
			current = temp;

			if (current->child.size() == 0)
			{
				// 找到关键字
				// return true;

				ret = true;

				// 根据depth 屏蔽字符

				for(int i=1; i <= depth; i++)
				{
					wchar_t* t = word - i;

					*t = rep;
				}

				current = &root;
				depth = 0;
			}
		}
		else
		{
			if (current != &root)
			{
				current = &root;
				depth = 0;
				word --;
			}
		}
	}

	return ret;
}

