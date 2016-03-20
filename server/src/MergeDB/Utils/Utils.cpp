//
//  Utils.cpp
//  GoldOL
//
//  Created by Mike Chang on 12-10-16.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "Utils.h"
#include "stdio.h"
#include "stdint.h"
#include "utf8.h"
#include <vector>
#include <list>
#include <iostream>
#include <string.h>
#include "json/json.h"


// 调整宽字符大小 Iter 遍历 bug 用len 绕过bug
std::string Utils::ws2s(const std::wstring& ws)
{
    std::string resultstring;
    resultstring.reserve(ws.size());
    utf8::utf16to8(ws.begin(),ws.begin() + ws.length(), std::back_inserter(resultstring));

    return resultstring;
}

//临时变量保存避免string预先分配内存bug
std::wstring Utils::s2ws(const std::string& s)
{
    std::vector<unsigned short> tmp;
    utf8::utf8to16(s.begin(), s.end(), std::back_inserter(tmp));
    std::wstring resultstring(tmp.begin(),tmp.end());
    //resultstring.resize(s.length());
    return resultstring;
}

const int Utils::CHARACTER_PROPERTIES[128] =
{
	/* 00 . */ ACP_CONTROL,
	/* 01 . */ ACP_CONTROL,
	/* 02 . */ ACP_CONTROL,
	/* 03 . */ ACP_CONTROL,
	/* 04 . */ ACP_CONTROL,
	/* 05 . */ ACP_CONTROL,
	/* 06 . */ ACP_CONTROL,
	/* 07 . */ ACP_CONTROL,
	/* 08 . */ ACP_CONTROL,
	/* 09 . */ ACP_CONTROL | ACP_SPACE,
	/* 0a . */ ACP_CONTROL | ACP_SPACE,
	/* 0b . */ ACP_CONTROL | ACP_SPACE,
	/* 0c . */ ACP_CONTROL | ACP_SPACE,
	/* 0d . */ ACP_CONTROL | ACP_SPACE,
	/* 0e . */ ACP_CONTROL,
	/* 0f . */ ACP_CONTROL,
	/* 10 . */ ACP_CONTROL,
	/* 11 . */ ACP_CONTROL,
	/* 12 . */ ACP_CONTROL,
	/* 13 . */ ACP_CONTROL,
	/* 14 . */ ACP_CONTROL,
	/* 15 . */ ACP_CONTROL,
	/* 16 . */ ACP_CONTROL,
	/* 17 . */ ACP_CONTROL,
	/* 18 . */ ACP_CONTROL,
	/* 19 . */ ACP_CONTROL,
	/* 1a . */ ACP_CONTROL,
	/* 1b . */ ACP_CONTROL,
	/* 1c . */ ACP_CONTROL,
	/* 1d . */ ACP_CONTROL,
	/* 1e . */ ACP_CONTROL,
	/* 1f . */ ACP_CONTROL,
	/* 20   */ ACP_SPACE | ACP_PRINT,
	/* 21 ! */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 22 " */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 23 # */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 24 $ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 25 % */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 26 & */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 27 ' */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 28 ( */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 29 ) */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2a * */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2b + */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2c , */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2d - */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2e . */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2f / */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 30 0 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 31 1 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 32 2 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 33 3 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 34 4 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 35 5 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 36 6 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 37 7 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 38 8 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 39 9 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 3a : */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3b ; */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3c < */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3d = */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3e > */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3f ? */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 40 @ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 41 A */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 42 B */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 43 C */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 44 D */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 45 E */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 46 F */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 47 G */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 48 H */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 49 I */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4a J */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4b K */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4c L */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4d M */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4e N */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4f O */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 50 P */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 51 Q */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 52 R */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 53 S */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 54 T */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 55 U */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 56 V */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 57 W */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 58 X */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 59 Y */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 5a Z */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 5b [ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 5c \ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 5d ] */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 5e ^ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 5f _ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 60 ` */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 61 a */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 62 b */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 63 c */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 64 d */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 65 e */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 66 f */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 67 g */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 68 h */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 69 i */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6a j */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6b k */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6c l */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6d m */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6e n */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6f o */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 70 p */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 71 q */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 72 r */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 73 s */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 74 t */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 75 u */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 76 v */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 77 w */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 78 x */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 79 y */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 7a z */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 7b { */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 7c | */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 7d } */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 7e ~ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 7f . */ ACP_CONTROL
};

std::string Utils::sLabelSplitStr = "╟╬╡";              //多彩字符串，不同颜色Label之间的分隔符
std::string Utils::sContentColorSplitStr = "╩╨╧";       //多彩字符串，文字内容和颜色值之间的分隔符

size_t Utils::Utf8StringSize(const std::string &str)
{
    size_t res = 0;
    std::string::const_iterator it = str.begin();
    for(; it != str.end(); IncUtf8StringIterator(it, str.end()))
        res++;

    return res;
}

std::string Utils::Utf8SubStr(const std::string& str, size_t start, size_t n)
{
	if (n == (size_t)-1)
		return std::string(Utf8PositionToIterator(str, start), str.end());
	else
		return std::string(
                           Utf8PositionToIterator(str, start),
                           Utf8PositionToIterator(str, start + n));
}
std::wstring Utils::Utf8ToUtf16(const std::string& utf8)
{
    std::vector<unsigned long> unicode;
    size_t i = 0;
    while (i < utf8.size())
    {
        unsigned long uni;
        size_t todo;
        unsigned char ch = utf8[i++];
        if (ch <= 0x7F)
        {
            uni = ch;
            todo = 0;
        }
        else if (ch <= 0xBF)
        {
            throw std::logic_error("not a UTF-8 string");
        }
        else if (ch <= 0xDF)
        {
            uni = ch&0x1F;
            todo = 1;
        }
        else if (ch <= 0xEF)
        {
            uni = ch&0x0F;
            todo = 2;
        }
        else if (ch <= 0xF7)
        {
            uni = ch&0x07;
            todo = 3;
        }
        else
        {
            throw std::logic_error("not a UTF-8 string");
        }
        for (size_t j = 0; j < todo; ++j)
        {
            if (i == utf8.size())
                throw std::logic_error("not a UTF-8 string");
            unsigned char ch = utf8[i++];
            if (ch < 0x80 || ch > 0xBF)
                throw std::logic_error("not a UTF-8 string");
            uni <<= 6;
            uni += ch & 0x3F;
        }
        if (uni >= 0xD800 && uni <= 0xDFFF)
        throw std::logic_error("not a UTF-8 string");
        if (uni > 0x10FFFF)
        throw std::logic_error("not a UTF-8 string");
        unicode.push_back(uni);
    }
    std::wstring utf16;
    for (size_t i = 0; i < unicode.size(); ++i)
    {
        unsigned long uni = unicode[i];
        if (uni <= 0xFFFF)
        {
            utf16 += (wchar_t)uni;
        }
        else
        {
            uni -= 0x10000;
            utf16 += (wchar_t)((uni >> 10) + 0xD800);
            utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
        }
    }
    return utf16;
}
/*
 判断字符串中是否有Emoji表情。
经过测试，当有Emoji表情时，返回true。当有一些特殊符号(如：▼①)时，也会返回true。
所以这个函数可以检测Emoji表情和一些特殊符号
    ---linshusen
*/
bool Utils::isUtf8StringContainEmoji(const std::string& str)
{
    bool returnValue = false;

    int characterNum = Utf8StringSize(str);
    for (int i=0; i<characterNum; ++i)//遍历每个utf8的文字
    {
        std::string subString_utf8 = Utf8SubStr(str, i, 1);
        std::wstring subString = Utf8ToUtf16(subString_utf8); //将一个utf8的文字转换为utf16的文字

        //检测utf16的文字是否为Emoji表情
        const unsigned short hs = subString.at(0);
        // surrogate pair
        if (0xd800 <= hs && hs <= 0xdbff)
        {
            if (subString.length() > 1)
            {
                const unsigned short ls = subString.at(1);
                const int uc = ((hs - 0xd800) * 0x400) + (ls - 0xdc00) + 0x10000;
                if (0x1d000 <= uc && uc <= 0x1f77f)
                {
                    returnValue = true;
                    return true;
                }
            }
        }
        else if (subString.length() > 1)
        {
            const unsigned short ls = subString.at(1);
            if (ls == 0x20e3)
            {
                returnValue = true;
                return true;
            }

        }
        else
        {
            // non surrogate
            if (0x2100 <= hs && hs <= 0x27ff)
            {
                returnValue = true;
                return true;
            }
            else if (0x2B05 <= hs && hs <= 0x2b07)
            {
                returnValue = true;
                return true;
            }
            else if (0x2934 <= hs && hs <= 0x2935)
            {
                returnValue = true;
                return true;
            }
            else if (0x3297 <= hs && hs <= 0x3299)
            {
                returnValue = true;
                return true;
            }
            else if (hs == 0xa9 || hs == 0xae || hs == 0x303d || hs == 0x3030 || hs == 0x2b55 || hs == 0x2b1c || hs == 0x2b1b || hs == 0x2b50)
            {
                returnValue = true;
                return true;
            }
        }
    }
    return returnValue;
}

StringTokenizer::StringTokenizer(const std::string& str, const std::string& separators, int options, bool useMultiSeparators)
{
	if (useMultiSeparators)
    {
        std::string::const_iterator it1 = str.begin();
        std::string::const_iterator it2;
        std::string::const_iterator it3;
        std::string::const_iterator end = str.end();

        while (it1 != end)
        {
            if (options & TOK_TRIM)
            {
                while (it1 != end && Utils::isSpace(*it1)) ++it1;
            }
            it2 = it1;
            while (it2 != end && separators.find(*it2) == std::string::npos) ++it2;
            it3 = it2;
            if (it3 != it1 && (options & TOK_TRIM))
            {
                --it3;
                while (it3 != it1 && Utils::isSpace(*it3)) --it3;
                if (!Utils::isSpace(*it3)) ++it3;
            }
            if (options & TOK_IGNORE_EMPTY)
            {
                if (it3 != it1)
                    _tokens.push_back(std::string(it1, it3));
            }
            else
            {
                _tokens.push_back(std::string(it1, it3));
            }
            it1 = it2;
            if (it1 != end) ++it1;
        }
    }
    else
    {
        const char* src = str.c_str();
        const char* sepstr = separators.c_str();
        int seplen = strlen(sepstr);

        while (*src){
            const char* pos = strstr(src, sepstr);
            if (pos == NULL){
                _tokens.push_back(src);
                break;
            }
            _tokens.push_back(string(src, pos - src));
            src = pos + seplen;
        }
    }

}

StringTokenizer::~StringTokenizer()
{
}

std::vector<int> Utils::lockMsgList;

bool Utils::lockTcpMsg(int msgType)
{
    for (std::vector<int>::iterator iter = lockMsgList.begin(); iter != lockMsgList.end(); iter++) {
        if (msgType == *iter)   return false;
    }
    lockMsgList.push_back(msgType);
    return true;
}

bool Utils::unLockTcpMsg(int msgType)
{
    for (std::vector<int>::iterator iter = lockMsgList.begin(); iter != lockMsgList.end(); iter++)
    {
        if (msgType == *iter)
        {
            lockMsgList.erase(iter);
            return true;
        }
    }
    return false;
}

void Utils::clearLockTcpMsg()
{
    lockMsgList.clear();
}

//int Utils::checkCopyOpen(int _copyID,int _lastCopyID)
//{
//    SceneCfgDef *_sceneCfg = SceneCfg::getCfg(_copyID);
//    if (_sceneCfg != NULL && _sceneCfg->preCopy > _lastCopyID)
//    {
//        return checkCopyOpen(_sceneCfg->preCopy, _lastCopyID);
//    }
//    else
//    {
//        return _copyID;
//    }
//}

//std::string Utils::SceneToString(int _sceneID)
//{
//    char _sceneStr[255] = {0};
//    SceneCfgDef *_sceneCfg = SceneCfg::getCfg(_sceneID);
//    string _sceneType;
//    switch (_sceneCfg->sceneType) {
//        case stTown:
//            _sceneType = "主城";
//            break;
//        case stCopy:
//            _sceneType = "普通副本";
//            break;
//        case stPrintCopy:
//            _sceneType = "图纸副本";
//            break;
//        case stEliteCopy:
//            _sceneType = "精英副本";
//            break;
//        case stDungeon:
//            _sceneType = "地下城";
//            break;
//        case stTeamCopy:
//            _sceneType = "雇佣副本";
//            break;
//        case stPvp:
//            _sceneType = "竞技场";
//            break;
//        case stNewbie:
//            _sceneType = "新手地图";
//            break;
//        case stFriendDunge:
//            _sceneType = "好友地下城";
//            break;
//        default:
//            _sceneType = "其他";
//            break;
//    }
//    sprintf(_sceneStr, "%d_%s_%s",_sceneID,_sceneType.c_str(),(_sceneCfg->name).c_str());
//    return _sceneStr;
//}

std::string Utils::SwordToString(int _sword)
{
    int _mysword = _sword/1000;
    _mysword = _mysword*1000;
    char swordStr[255] = {0};
    sprintf(swordStr, "%d~~%d",_mysword,_mysword+1000);
    return swordStr;
}

std::string Utils::PvpRankToString(int _rank)
{
    if (_rank <= 10)
        return "1~10";
    else if(_rank <= 50)
        return "10~50";
    else if(_rank <= 100)
        return "50~100";
    else if(_rank <= 200)
        return "100~200";
    else if(_rank <= 500)
        return "200~500";
    else if(_rank <= 1000)
        return "500~1k";
    else if(_rank <= 2000)
        return "1k~2k";
    else if(_rank <= 5000)
        return "2k~5k";
    else
        return "5k~?";
}

string Utils::itoa(int val, int radix, char* buf)
{
    assert(radix <= 16 && radix >= 2);

    static const char* symbol = "0123456789ABCDEF";
    char sbuf[128];

    char* pbuf = buf ? buf : sbuf;

    char* startpos = pbuf;
    if (val < 0)
    {
        *startpos++ = '-';
        val = -val;
    }

    char* pos = startpos;
    do
    {
        *pos++ = symbol[val % radix];
        val /= radix;
    }
    while (val);

    *pos-- = 0;

    while (startpos < pos)
    {
        char temp = *startpos;
        *startpos++ = *pos;
        *pos-- = temp;
    }

    return string(pbuf);
}

int Utils::safe_atoi(const char* str, int def)
{
    if (str == NULL || *str == 0){
        return def;
    }

    int sign = 1;
    const char *pos = str;
    if (*pos == '-')
    {
        sign = -1;
        pos++;
    }
    else if (*pos == '+')
    {
        pos++;
    }

    int val = 0;
    while (*pos)
    {
        if (!isdigit(*pos))
        {
            return def;
        }

        val *= 10;
        val += *pos - '0';
        pos++;
    }


    return val * sign;
}

//取大于val的，能整除align的值，align必须时2的幂次方
int Utils::alignNum(int val, uint32_t align)
{
    val = (val + (align - 1)) & (~(align - 1));
    return val;
}


int Utils::numOfOne(int val, int start, int len)
{
    int num = 0;
    for (int i = start; i < start + len; i++)
    {
        if (val & (1 << i))
        {
            num++;
        }
    }
    return num;
}


string strFormat(string& str, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    str = strvFormat(fmt, args);
    va_end(args);
    return str;
}

string strFormat(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    string ret = strvFormat(fmt, args);
    va_end(args);
    return ret;
}

string strvFormat(const char* fmt, va_list ap)
{
    char buf[128];
    va_list tryap;
    va_copy(tryap, ap);
    int n = vsnprintf(buf, sizeof(buf), fmt, tryap);
    if (n < sizeof(buf)){
        return string(buf);
    }

    char* data = new char[n + 1];
    vsnprintf(data, n + 1, fmt, ap);

    string retstr = string(data);
    delete [] data;

    return retstr;
}


std::string Utils::currentDate()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    localtime_r(&now, &tstruct);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);

    return buf;
}

std::string Utils::currentTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    localtime_r(&now, &tstruct);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%X", &tstruct);

    return buf;
}

void printTime(time_t time)
{
    struct tm mkdate;
    localtime_r(&time, &mkdate);
    char buf[128];
    strftime ( buf,80,"Data:\n%Y-%m-%d \nTime:\n%H:%M:%S\n", &mkdate);
    printf("%s", buf);
}

time_t Utils::mktimeFromWday(int wday, int hour, int min)
{
    struct tm date;
    time_t nowtime = time(NULL);
    localtime_r(&nowtime, &date);

    int nowWday = date.tm_wday;
    time_t weektime = 0;

    date.tm_hour = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    nowtime = mktime(&date);
    nowtime -= 3600*24*nowWday;

    weektime = nowtime + 3600 * 24 * wday + 3600 * hour + 60*min;
    return weektime;
}

time_t Utils::mktimeFromDate(int year, int mon, int m_day, int hour, int min)
{
    if (year < 1900 || mon < 1 || m_day < 1 || hour < 0 || min < 0) {
        return 0;
    }

    struct tm date;
    time_t nowtime = time(NULL);
    localtime_r(&nowtime, &date);

    time_t datetime = 0;
    date.tm_year = year - 1900;
    date.tm_mon = mon - 1;

    date.tm_hour = hour;
    date.tm_min = min;
    date.tm_sec = 0;

    datetime = mktime(&date);

    return datetime;
}

time_t Utils::mktimeFromToday(int hour, int min, int sec)
{
    struct tm date;
    time_t nowtime = time(NULL);
    localtime_r(&nowtime, &date);

    time_t datetime = 0;
    date.tm_hour = hour;
    date.tm_min = min;
    date.tm_sec = sec;

    datetime = mktime(&date);

    return datetime;
}

time_t Utils::mktimeTheNextOnhour()
{
    struct tm date;
    time_t nowtime = time(NULL);
    localtime_r(&nowtime, &date);
    
    time_t datetime = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    
    datetime = mktime(&date);
    datetime += 3600;
    
    printTime(datetime);
    
    return datetime;
}

//失败返回-1
int Utils::parseDate(const char* str)
{
    if(NULL == str)
        return -1;
    
    if( strlen(str) == 0)
        return -1 ;
    
    tm  date={0};
    strptime(str, "%Y-%m-%d %H:%M:%S", &date);
    return (int)mktime(&date);
}


bool Utils::sameDate(const char* str)
{
    if(str == NULL)
        return false;

    time_t     now = time(0);
    struct tm  date;
    char       buf[80];
    localtime_r(&now, &date);

    int year ,month , day;
    sscanf(str,"%d-%d-%d",&year,&month,&day);
    if( (date.tm_year + 1900) == year &&
        (date.tm_mon + 1) == month &&
        date.tm_mday == day)
        return true;
    else
        return false;
}

static int arrayTotal(int* elements, int num)
{
	int total = 0;
	for (int i = 0; i < num; i++)
	{
		total += elements[i];
	}

	return total;
}

static int getRand(int* props, int num, int totalprop)
{
	int randnum = rand() % totalprop;
	int index = 0;

	for (;;)
	{
		randnum -= props[index];
		if (randnum < 0)
		{
			break;
		}
		index++;
	}
	return index;
}

int getRand(int* props, int num)
{
	int total = arrayTotal(props, num);

    if(0==total)return 0;//2013.7.23

	return getRand(props, num, total);
}


// 2013-8-27 增加是否重复
void getRands(int *props, int propnum, int getnum, int* getindices, bool isRepeat)
{
	int totalprop = arrayTotal(props, propnum);
    if (0 == totalprop) return;

	for (int i = 0; i < getnum; i++)
	{
		int index = getRand(props, propnum, totalprop);
		getindices[i] = index;
		if (!isRepeat)
        {
            totalprop -= props[index];
            props[index] = 0;
            if (0 == totalprop) return;
        }
	}
}

bool Utils::sameIntVector(const vector<int>& a,const vector<int>& b)
{
    if(a.size() != b.size())
        return false;
    return 0 == memcmp(a.data(), b.data(), sizeof(int)*a.size());
}

string Utils::makeStr(const char * data, ...)
{
    MAKE_VA_STR_DEF(data);
    return strdata;
}

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
std::pair<int,int> checkPageRange(int size,int start ,int num ,int maxNum)
{
    std::pair<int,int> ret;
    ret.first = 0;
    ret.second = 0;

    if(start < 0 || start >= size|| num < 0 )
        return ret;

    if(num > maxNum)
        num = maxNum;

    ret.first = start;
    ret.second = start + num;
    if(ret.second > size)
        ret.second = size;
    return ret;
}

// 配置表id加等级转成技能ID
int makeSkillId(int skillcfgid , int level)
{
    return skillcfgid + level;
}

// 技能ID反算出配置表技能ID与等级 first:skillcfgid,second:level
std::pair<int,int> calSkillCfgIdAndLevel(int skillid)
{
    int skillcfgid = ( skillid / 100 ) * 100;
    int level = skillid % 100;
    return std::make_pair(skillcfgid, level);
}


string xyJsonWrite(const Json::Value & val)
{
	string str = Json::FastWriter().write(val);
	// 移除尾部\n
	str.erase(str.size() - 1);
	return str;
}

bool xyJsonSafeParse(const string &str, Json::Value &value)
{
	Json::Reader reader;
    if (!reader.parse(str, value) || value.type() != Json::objectValue) {
        return false;
    }
	return true;
}


int xyDiffday(time_t first, time_t second)
{
	struct tm firstTm = *localtime(&first);
	firstTm.tm_hour = 0;
	firstTm.tm_min = 0;
	firstTm.tm_sec = 0;
	
	struct tm secondTm = *localtime(&second);
	secondTm.tm_hour = 0;
	secondTm.tm_min = 0;
	secondTm.tm_sec = 0;
	
	int sec = (int)difftime(mktime(&secondTm), mktime(&firstTm));
	return abs(sec) / (24 * 3600);
}

/*
 *用来连接字串,传入char *类型数据
 */
string str_cat(const int &count, ...)
{
	string str;
	va_list args;
	va_start(args, count);
	string tmp;
	for (int i=0; i<count; i++)
	{
		tmp = va_arg(args, char *);
		str.append(tmp);
	}
    va_end(args);
	return str;
}