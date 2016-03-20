/*-------------------------------------------------------------------------
	created:	2010/08/16  16:10
	filename: 	e:\Project_SVN\Server\Utils\StringUtils\StringUtil.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/
#pragma once

#include "../Public/Tools.h"
#include "../ThreadPool/ThreadPool.h"
#include <wchar.h>

// 字符串数组
typedef std::vector<std::string> StringArray;
typedef std::vector<std::wstring> WStringArray;

// string ， UTF8， wstring 之间的转换
size_t  wcslen(const wchar_t * wcs);
char* itoa (int  val,  char  *buf, int  radix = 10);
int wcscmp_(const wchar_t *s1 , const wchar_t *s2);
int wcsicmp(const wchar_t *s1 , const wchar_t *s2);

//UNICODE码转为GB2312码
string WideStrAsString(wstring& in);
string WideStrAsString(const wchar_t* in);

//GB2312码转为UNICODE码 *****
wstring StringAsWideStr(string& in);
wstring StringAsWideStr(const char* in);

//UNICODE码转为utf8码 ***
string WideStrAsUtf8(wstring& in);
string WideStrAsUtf8(const wchar_t* in);

//utf8码转为UNICODE码 ****
wstring Utf8AsWideStr(string& in);
wstring Utf8AsWideStr(const char* in);

// utf8码转为GB2312码
string Utf8AsString(string& in);
string Utf8AsString(const char* in);

//GB2312码转为utf8码
string StringAsUtf8(string& in);
string StringAsUtf8(const char* in);

// 子串全部替换
string& replace_all(string& str, const string& old_value,const string& new_value);

// 字符串分割
void split_path(StringArray& paths, const char* szInput, char div,
				bool bSaveEmptyStr = true);

// 字符串分割宽串
void split_path(WStringArray& paths, const wchar_t* szInput, wchar_t div,
				bool bSaveEmptyStr = true);


// 提出两端空格
void Trim2SideSpace(std::string& szString);
char *trim(char *str);

// 获取ini文件名的数字, 如28001.ini, 得到28001数字
UI32 GetTypeFromFile(const char* szFileName);

// 小写转大写
void ToUpper(string& str);

//  二进制与字符串之间的转换
//  -----------------------------------------------------------------------
// 二进制数据转字符串
bool BinaryToText(std::string& out, char* buffer,  size_t nBufferLen);

// 二进制字符串转数据
bool TextToBinary(std::string& szIn,  char* buffer, size_t nBufferLen);

// 宽串转二进制串
string WstringAsHexChar(const wchar_t* pWstr);

// 二进制串转宽串
wstring HexCharAsWstring(const char* pStr);

// 字符串转二进制串
string StringAsHexChar(const char* pStr);

// 二进制串转字符串
string HexCharAsString(const char* pStr);

// 处理浮点数多余的0
void TrimFloatVal(string& val);

// 拆分字符串
inline StringArray StrSplit( string $input , string $delim )
{
	StringArray	result ;
	const char *input = $input.c_str( ) ;
	const char *delim = $delim.c_str( ) ;

	const char *begin	= input ;
	const	char *mark	= strpbrk( begin , delim ) ;

	while( mark )
	{
		result.push_back( $input.substr( begin - input , mark - begin ) ) ;
		begin	= mark + 1 ;
		mark	= strpbrk( begin , delim ) ;
	}
	if( *begin )
	{// 最后一个
		result.push_back( $input.substr( begin - input ) ) ;
	}
	return result ;
}
// 和并字符串
inline string StrMerge( StringArray input , char delim )
{
	string	result ;
	int	strNum	= input.size( ) ;
	if( strNum == 0 )		return "" ;
	--strNum ;
	for( int i = 0 ; i < strNum ; ++i )
	{
		result.append( input[i] ) ;
		result.append( 1 , delim ) ;
	}
	result.append( input[strNum] ) ;
	return result ;
}

//
//  自定义数据类型
//  -----------------------------------------------------------------------
// 数组值
class ArrayVal
{
public:
	ArrayVal();
	ArrayVal(const char* str);
	~ArrayVal();
	void operator =(const char* str);
	void operator = (const ArrayVal& another);
	UI32 operator [](UI16 val);
	const char* c_str(){return str_.c_str();}
public:
	vector<UI32> m_data;
	string str_;
};

// 浮点数组
class ArrayVal_F
{
public:
	ArrayVal_F();
	ArrayVal_F(const char* str);
	~ArrayVal_F();
	void operator =(const char* str);
	void operator = (const ArrayVal_F& another);
	float operator [](UI16 val);
	const char* c_str(){return str_.c_str();}
public:
	vector<float> m_data;
	string str_;
};

// 范围值
class RangeVal
{
public:
	RangeVal();
	RangeVal(const char* str);
	RangeVal(UI32 pFrom, UI32 pTo);
	~RangeVal();
	void operator = (const char* str);
	void operator = (const RangeVal& another);
	const char* c_str(){return str_.c_str();}
	UI16 Reification();
public:
	UI32 from;
	UI32 to;
	string str_;
};

class FunctionVal
{
public:
	FunctionVal();
	FunctionVal(const char* str);
	~FunctionVal();
	void operator = (const char* str);
	void operator = (const FunctionVal& another);
	const char* c_str(){return str_.c_str();}
public:
	string str_;
	string name;
	vector<string> m_data;
};

// 字符串申请器, 用来申请字符串
class CStringAllocator : public CriticalObject
{
public:
	CStringAllocator();
	~CStringAllocator();
public:
	string* Alloc();
	void Reuse(string* pstr);
private:
	vector<string*> m_List;
};

// 快速的json写入
class JsonFastWrite
{
public:
	JsonFastWrite(string& out_) : out(out_) {elements = 0; out.reserve(64); out = '{';}
	~JsonFastWrite(){};
public:
	void Write(const char* name, int val);
	void Write(const char* name, const char* val);
	void Write(const char* name, const wchar_t* val);
	void Write(const char* name, float val);
	void Write(const char* name, bool val);
	void WriteEnd();
private:
	int elements;
	string& out;
};

// 快速的Json解析
class JsonFastRead
{
public:
	JsonFastRead(const char* input);
	~JsonFastRead();

private:
    void Process(int name1, int name2, int val1, int val2);
    void CreateProp(char* name, char* val);

public:
    // 读取属性
	int ReadInt(const char* name);
	const char* ReadString(const char* name);
	float ReadFloat(const char* name);
	bool ReadBool(const char* name);

	// 修改
    bool SetInt(const char* name, int val);
	bool SetString(const char* name, const char* val);
	bool SetFloat(const char* name, float val);
	bool SetBool(const char* name, bool val);

	// 查找属性
	bool FindProp(const char* name);
	bool DeleteProp(const char* name);

	// 名字列表
	void GetPropNames(StringArray& sa);

    // 添加属性
    void AddInt(const char* name, int val);
    void AddString(const char* name, const char* val);
    void AddFloat(const char* name, float val);
    void AddBool(const char* name, bool val);

    // 输出为JSON
    void ToJson(string& out);
    string ToJson();

private:
	vector<CVar*> m_varList;
	char* m_szCopy;
};

// JSON 快速读取单个属性
int GetJsonInt(const char* val, const char* sec);
float GetJsonFloat(const char* val, const char* sec);
string GetJsonStr(const char* val, const char* sec);
bool GetJsonBool(const char* val, const char* sec);
