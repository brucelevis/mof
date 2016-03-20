
#ifndef _TOOLS_H
#define _TOOLS_H

#ifndef LINUX_
#include <windows.h>
#endif  // LINUX_

#include "Macros.h"
#include "Var.h"

#include <string>
#include <vector>

using namespace std;

// tools
// 辅助函数

// 测试对象号是否为空
extern bool IsNull(const PERSISTID & pid);
// 返回（0到scale-1）随机整数，scale最大32768
extern int RandomInt(int scale);
// 返回（0.0到scale）随机浮点数
extern float RandomFloat(float scale);
// 返回（0到scale-1）随机整数，scale最大32768 * 32768
extern int RandomBigInt(int scale);

// 整数转换为字符串
extern std::string IntAsString(int value);
// 浮点数转换为字符串
extern std::string FloatAsString(float value, int decimals = 0);
// 对象号转换为字符串
extern std::string ObjectAsString(const PERSISTID & pid);

// 字符串转换为整数
extern int StringAsInt(const char * value);
// 字符串转换为浮点数
extern float StringAsFloat(const char * value);

// 字符串转换为对象号
extern PERSISTID StringAsObject(const char * value);

// 字符串转换为整数
extern int StringAsInt(const std::string & value);
// 字符串转换为浮点数
extern float StringAsFloat(const std::string & value);

//
// 字符串转换为对象号
extern PERSISTID StringAsObject(const std::string & value);

// 整数转换为宽字符串
extern std::wstring IntAsWideStr(int value);
// 浮点数转换为宽字符串
extern std::wstring FloatAsWideStr(float value, int decimals = 0);
// 对象号转换为宽字符串
extern std::wstring ObjectAsWideStr(const PERSISTID & pid);

// 宽字符串转换为整数
extern int WideStrAsInt(const wchar_t * value);
// 宽字符串转换为浮点数
extern float WideStrAsFloat(const wchar_t * value);

// 宽字符串转换为对象号
extern PERSISTID WideStrAsObject(const wchar_t * value);

// 宽字符串转换为整数
extern int WideStrAsInt(const std::wstring & value);
// 宽字符串转换为浮点数
extern float WideStrAsFloat(const std::wstring & value);

// 宽字符串转换为对象号
extern PERSISTID WideStrAsObject(const std::wstring & value);

// 转换为大写的字符串
extern std::string ToUpper(const char * str);
// 转换为小写的字符串
extern std::string ToLower(const char * str);
// 取子串，pos从0开始
extern std::string Substr(const char * str, size_t pos, size_t len);
// 取用空格分割的子串，index从0开始，表示第几个
extern std::string Parse(const char * str, size_t index);
// 取用空格分割的子串，直到字符串结束
extern std::string StrTail(const char * str, size_t index);
// 在字符串中删除用空格分割的子串
extern std::string StrRemoveSub(const char * str, const char * sub);
// 在字符串中删除用空格分割的子串
extern std::string StrRemovePos(const char * str, size_t index);
// 查找用空格分割的子串在字符串第一次出现的位置，返回-1表示未找到
extern int StrFindPos(const char * str, const char * find);

// 宽字符版本的函数，同上
extern std::wstring ToUpperW(const wchar_t * str);
extern std::wstring ToLowerW(const wchar_t * str);
extern std::wstring SubstrW(const wchar_t * str, size_t pos, size_t len);
extern std::wstring ParseW(const wchar_t * str, size_t index);
extern std::wstring StrTailW(const wchar_t * str, size_t index);
extern std::wstring StrRemoveSubW(const wchar_t * str, const char * sub);
extern std::wstring StrRemovePosW(const wchar_t * str, size_t index);
extern int StrFindPosW(const wchar_t * str, const char * find);

// 返回系统的毫秒
extern int GetTicks();
// 取与前一个毫秒时间的差值
extern int Elapse(int prev);
// 取线段的方向
extern float LineOrient(float x1, float z1, float x2, float z2);
// 取两点之间的距离
extern float Get2DotDistance(float x1, float z1, float x2, float z2);

// 向量中时候有指定的成员
template <class MemberType>
bool VectorHaveMemer( std::vector<MemberType> &vec , MemberType &member )
{
	for( int i = 0 , j = vec.size( ) ; i < j ; ++i )
	{
		if( vec[i] == member )	return true ;
	}
	return false ;
}

// 在程序中插入调试用的语句，在Release版本下会被忽略
#ifndef NDEBUG
	#define Debug(p) p
#else
	#define Debug(p) (void(0))
#endif // NDEBUG

#endif // _TOOLS_H
