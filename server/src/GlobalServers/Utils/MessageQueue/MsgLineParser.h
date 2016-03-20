/*-------------------------------------------------------------------------
	created:	2010/08/16  16:31
	filename: 	e:\Project_SVN\Server\Utils\MessageQueue\MsgLineParser.h
	author:		Deeple
	
	purpose:	
---------------------------------------------------------------------------*/


#pragma once 

#include <string>
#include <vector>
using namespace std;

class MsgAna
{
public:
	MsgAna(string& s);
	MsgAna();
	~MsgAna();
public:
	string GetHead();
	string GetString(int nPos);
	int GetInt(int nPos);
	float GetFloat(int nPos);
	wstring GetWString(int nPos);

	void SetString(const char* strVal, int nPos);
	void SetInt(int iVal, int nPos);
	void SetFloat(float fVal, int nPos);
	void SetWString(const wchar_t* pWstr, int nPos);

	size_t GetParamCount();

	string Format();

private:
	string WStringToParam(wstring wstr);
	wstring ParamToWString(string param);

	string StringToParam(string str);
	string ParamToString(string param);

public:
	char m_chBuffer[8192];
	vector<string> m_Params;
};
