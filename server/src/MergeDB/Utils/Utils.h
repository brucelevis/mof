//
//  Utils.h
//  GoldOL
//
//  Created by Mike Chang on 12-10-16.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#ifndef GoldOL_Utils_h
#define GoldOL_Utils_h
//#include "Defines.h"
#include<math.h>

#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
using namespace std;

class Utils {
    private:
    Utils();
    virtual ~Utils();

public:
    static std::string ws2s(const std::wstring& ws);
    static std::wstring s2ws(const std::string& s);
    static std::string currentDate();
    static std::string currentTime();
    static time_t mktimeFromWday(int wday, int hour, int min);
    static time_t mktimeFromDate(int year, int mon, int m_day, int hour, int min);
    static time_t mktimeFromToday(int hour, int min = 0, int sec = 0);
    static time_t mktimeTheNextOnhour();    //获取下一个整点的时刻
    static bool sameDate(const char* str);
    static int parseDate(const char* str);
    //------------------UTF8字符串处理相关---linshusen-----------------------
    static std::string sLabelSplitStr;          //多彩字符串，不同颜色Label之间的分隔符
    static std::string sContentColorSplitStr;   //多彩字符串，文字内容和颜色值之间的分隔符
    static size_t Utf8StringSize(const std::string& str);
    static std::string Utf8SubStr(const std::string& str, size_t start, size_t n = (size_t)-1);
    /*
     判断字符串中是否有Emoji表情。
     经过测试，当有Emoji表情时，返回true。当有一些特殊符号(如：▼①)时，也会返回true。
     所以这个函数可以检测Emoji表情和一些特殊符号
     ---linshusen
     */
    static bool isUtf8StringContainEmoji(const std::string& str);
    static std::wstring Utf8ToUtf16(const std::string& utf8);//utf8转为utf16

private:
    // Moves the iterator to next unicode character in the string, returns number of bytes skipped
    template<typename _Iterator1, typename _Iterator2>
    static inline size_t IncUtf8StringIterator(_Iterator1& it, const _Iterator2& last)
    {
        if(it == last) return 0;
        unsigned char c;
        size_t res = 1;
        for(++it; last != it; ++it, ++res)
        {
            c = *it;
            if(!(c&0x80) || ((c&0xC0) == 0xC0)) break;
        }

        return res;
    }

    template<typename _Iterator>
    static inline size_t MultIncUtf8StringIterator(_Iterator& it, const _Iterator& last, size_t count)
    {
        size_t res = 0;
        for(size_t i = 0; i < count; i++)
        {
            if(it == last) break;
            res += IncUtf8StringIterator(it, last);
        }

        return res;
    }

    static inline std::string::const_iterator Utf8PositionToIterator(const std::string& str, size_t pos)
    {
        std::string::const_iterator res = str.begin();
        MultIncUtf8StringIterator(res, str.end(), pos);
        return res;
    }

//~~~~~~~~~~~~~~    J杰----用来防止发送的消息重复发送的消息锁
public:
    static std::vector<int> lockMsgList;
    //如果发现消息锁列表里 没有 的话就把消息放进列表里并返回true;
    //如果 有 的话就返回false;
    static bool lockTcpMsg(int msgType);
    //解锁，如果消息锁列表里 有 的话就把消息删除并返回true;
    //如果 没有 的话就返回false;
    static bool unLockTcpMsg(int msgType);
    //清空消息锁列表
    static void clearLockTcpMsg();
//~~~~~~~~~~~~~~
//    static int checkCopyOpen(int _copyID,int _lastCopyID);


    //TalkingData
    static std::string SwordToString(int _sword);
    static std::string EquipPartToString(int part);
    static std::string PvpRankToString(int _rank);

    enum CharacterProperties
    /// ASCII character properties.
    {
        ACP_CONTROL  = 0x0001,
        ACP_SPACE    = 0x0002,
        ACP_PUNCT    = 0x0004,
        ACP_DIGIT    = 0x0008,
        ACP_HEXDIGIT = 0x0010,
        ACP_ALPHA    = 0x0020,
        ACP_LOWER    = 0x0040,
        ACP_UPPER    = 0x0080,
        ACP_GRAPH    = 0x0100,
        ACP_PRINT    = 0x0200
    };
    static const int CHARACTER_PROPERTIES[128];

    static int properties(int ch)
    {
        if (isAscii(ch))
            return CHARACTER_PROPERTIES[ch];
        else
            return 0;
    }


    static bool isAscii(int ch)
    {
        return (static_cast<int>(ch) & 0xFFFFFF80) == 0;
    }


    static bool hasProperties(int ch, int props)
    {
        return (properties(ch) & props) == props;
    }

    static bool isSpace(int ch)
    {
        return hasProperties(ch, ACP_SPACE);
    }

    static string itoa(int val, int radix = 10, char* buf = NULL);

    static int safe_atoi(const char* str, int def = 0);

    //取大于val的，能整除align的值，align必须时2的幂次方
    static int alignNum(int val, uint32_t align);

    static int numOfOne(int val, int start = 0, int len = sizeof(int));

    static bool sameIntVector(const vector<int>& a,const vector<int>& b);

    static string makeStr(const char * data, ...);

};

class StringTokenizer
/// A simple tokenizer that splits a string into
/// tokens, which are separated by separator characters.
/// An iterator is used to iterate over all tokens.
{
public:
	enum Options
	{
		TOK_IGNORE_EMPTY = 1, /// ignore empty tokens
		TOK_TRIM         = 2  /// remove leading and trailing whitespace from tokens
	};

	typedef std::vector<std::string>::const_iterator Iterator;

    /*
     bool userMultiSeparators 是否使用多个分隔符。
     如果true，separator里面的每个字符都是分隔符；
     如果false，separator整个作为一个分隔符；
     ----linshusen
     */
	StringTokenizer(const std::string& str, const std::string& separators, int options = 0, bool useMultiSeparators=true);
    /// Splits the given string into tokens. The tokens are expected to be
    /// separated by one of the separator characters given in separators.
    /// Additionally, options can be specified:
    ///   * TOK_IGNORE_EMPTY: empty tokens are ignored
    ///   * TOK_TRIM: trailing and leading whitespace is removed from tokens.
    /// An empty token at the end of str is always ignored. For example,
    /// a StringTokenizer with the following arguments:
    ///     StringTokenizer(",ab,cd,", ",");
    /// will produce three tokens, "", "ab" and "cd".

	~StringTokenizer();
    /// Destroys the tokenizer.

	Iterator begin() const;
	Iterator end() const;

	const std::string& operator [] (std::size_t index) const;
    /// Returns the index'th token.
    /// Throws a RangeException if the index is out of range.

	std::size_t count() const;
    /// Returns the number of tokens.

private:
	StringTokenizer(const StringTokenizer&);
	StringTokenizer& operator = (const StringTokenizer&);

	std::vector<std::string> _tokens;

};


//
// inlines
//


inline StringTokenizer::Iterator StringTokenizer::begin() const
{
	return _tokens.begin();
}


inline StringTokenizer::Iterator StringTokenizer::end() const
{
	return _tokens.end();
}


inline const std::string& StringTokenizer::operator [] (std::size_t index) const
{
	if (index >= _tokens.size())
    {
        //oh
    }//throw RangeException();
	return _tokens[index];
}


inline std::size_t StringTokenizer::count() const
{
	return _tokens.size();
}

//连接格式 支持vector list arrary
template <class It>
std::string StrJoin(It begin,It end,std::string sep=",")
{
    std::ostringstream sstream;
    It step = begin;
    while (step != end) {
        if (step != begin) {
            sstream<<sep;
        }
        sstream<<*step;
        step++;
    }
    return sstream.str();
}

string strFormat(string& str, const char* fmt, ...);
string strFormat(const char* fmt, ...);
string strvFormat(const char* fmt, va_list ap);

inline std::vector<std::string> StrSpilt(std::string str,std::string sep=",")
{
    std::vector<std::string> out;

    StringTokenizer token(str,sep);
    std::copy(token.begin(), token.end(), std::back_inserter(out));
    return out;
}

// 拷贝Src 到 Des
template <class Src,class Des>
void copy_all(Src src,Des des)
{
    std::copy(src.begin(),src.end(),des);
}

template <class T>
inline const T &
check_min (T &val, const T &minval)
{
    if (val < minval)
    {
        val = minval;
    }

    return val;
}

template <class T>
inline const T &
check_max (T &val, const T &maxval)
{
    if (val > maxval)
    {
        val = maxval;
    }

    return val;
}

template <class T>
inline const T &
check_range (T &val, const T &minval, const T &maxval)
{
    assert(minval <= maxval);

    if (val < minval)
    {
        val = minval;
    }
    else if (val > maxval)
    {
        val = maxval;
    }

    return val;
}

inline int range_rand(int min, int max)
{
    assert(min <= max);

    if (min == max)
    {
        return min;
    }

    int interval = max - min;
    return min + rand() % (interval + 1);
}

inline float randf()
{
    return ((float)rand()) / RAND_MAX;
}

inline float range_randf(float min, float max)
{
    return min + randf() * (max - min);
}


typedef bool (*TraverseCallback)(void* traverseobj, void* param);

//根据概率获取随机索引，返回0 - num－1
int getRand(int* props, int num);
void getRands(int *props, int propnum, int getnum, int* getindices, bool isRepeat);


//安全删除指针
#define  SafeDelete(pData)	{ try { delete pData; } catch (...) { } pData=NULL; }

//安全删除指针
#define  SafeDeleteArray(pData)	{ try { delete [] pData; } catch (...) { } pData=NULL; }


typedef uint8_t     BYTE;
typedef uint16_t    WORD;
typedef uint32_t    DWORD;

#define MAKE_INT16(hig, low) ((WORD) (((BYTE) (low)) | ((WORD) ((BYTE) (hig))) << 8))
#define MAKE_INT32(hig, low) ((DWORD)(((WORD)(low)) | ((DWORD)((WORD)(hig))) << 16))
#define LOWORD(l)           ((WORD)((DWORD)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD)(w) >> 8))

//不定参字符串
#define MAKE_VA_STR_DEF(cstr) \
    std::vector<char> tmp;     \
    tmp.resize(128);            \
    va_list ap;                 \
    for (;;) {                  \
        va_start(ap, cstr );     \
        int n = vsnprintf(tmp.data(), tmp.size(), cstr, ap);\
        va_end(ap);             \
        if(n > -1 && n < tmp.size())\
            break;                  \
        else                        \
            tmp.resize(tmp.size()*2);\
    }                                \
    std::string str##cstr(tmp.data());
/////////////////////////////////////////
//和Utils::makeStr 完全一样，更短
inline string xystr(const char* str,...)
{
    MAKE_VA_STR_DEF(str);
    return strstr;
}
/////////////////////////////////////
namespace Json {
	class Value;
}
string xyJsonWrite(const Json::Value & val);

bool xyJsonSafeParse(const string &str, Json::Value &value);

int xyDiffday(time_t start, time_t end);

/***************************************************************
 函数：分页请求参数处理
 输入：  size    数组大小
        start   开始下标, 0开始
        num     数量
        maxNum  单页最大数量
 返回：
        first   begin()
        second  end() 哨兵
        num = second - first
 ****************************************************************/
std::pair<int,int> checkPageRange(int size,int start ,int num ,int maxNum = 50);

inline bool isFloatZero(float val)
{
    if ( fabs(val-0.0f) < 0.0000001) {
        return true;
    }
    return false;
}

// 配置表id加等级转成技能ID
int makeSkillId(int skillcfgid , int level);
// 技能ID反算出配置表技能ID与等级 first:skillcfgid,second:level
std::pair<int,int> calSkillCfgIdAndLevel(int skillid);



#ifdef __APPLE__
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <sys/param.h>
#else
#include <unistd.h>
#endif

inline string GetAppPath()
{
    string ret;
    char path[1024];
    path[0] = 0;


#if __APPLE__

    uint32_t buflen = 1024;
    _NSGetExecutablePath(path, &buflen);

    char* realp = realpath(path, NULL);
    if (realp)
    {
        ret = realp;

        free( realp );
    }

#else

    char pidname[128];
    sprintf(pidname,"/proc/%d/exe", getpid());
    int retlen = readlink(pidname, path, sizeof(path));
    if (retlen > 0 ) path[retlen] = 0;

    ret = path;
#endif

    // 去掉路径
    int n = ret.rfind('/');
    if (n != -1)
    {
        ret = ret.substr(0, n);
    }

    return ret;
}

/*
 *用来连接字串,传入char *类型数据
 */
string str_cat(const int &count, ...);

#endif
