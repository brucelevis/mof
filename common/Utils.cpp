
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
#include "DataCfg.h"

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

bool Utils::isSingleUtf8StringIsEmoji(const std::string& subString_utf8)
{
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
                return true;
            }
        }
    }
    else if (subString.length() > 1)
    {
        const unsigned short ls = subString.at(1);
        if (ls == 0x20e3)
        {
            return true;
        }
        
    }
    else
    {
        // non surrogate
        if (0x2100 <= hs && hs <= 0x27ff)
        {
            return true;
        }
        else if (0x2B05 <= hs && hs <= 0x2b07)
        {
            return true;
        }
        else if (0x2934 <= hs && hs <= 0x2935)
        {
            return true;
        }
        else if (0x3297 <= hs && hs <= 0x3299)
        {
            return true;
        }
        else if (hs == 0xa9 || hs == 0xae || hs == 0x303d || hs == 0x3030 || hs == 0x2b55 || hs == 0x2b1c || hs == 0x2b1b || hs == 0x2b50)
        {
            return true;
        }
    }
    return false;
}


bool Utils::isUtf8StringContainEmoji(const std::string& str)
{
    int characterNum = Utf8StringSize(str);
    for (int i=0; i<characterNum; ++i)//遍历每个utf8的文字
    {
        std::string subString_utf8 = Utf8SubStr(str, i, 1);
        bool returnValue   =  isSingleUtf8StringIsEmoji(subString_utf8);
        if(returnValue == true)
        {
            return true;
        }
    }
    return false;
}

void Utils::separationEmojiFromUtf8String(const std::string& str, vector<string> &StrDatas,vector<int> &isEmojiStr)
{
    int characterNum = Utf8StringSize(str);
    bool lastStrIsEmojiStr= false;
    bool currentStrIsEmojiStr = false;
    for (int i=0; i<characterNum; ++i)//遍历每个utf8的文字
    {
        std::string subString_utf8 = Utf8SubStr(str, i, 1);
        currentStrIsEmojiStr = isSingleUtf8StringIsEmoji(subString_utf8);
        if(i == 0)
        {
           StrDatas.push_back(subString_utf8);
           isEmojiStr.push_back(currentStrIsEmojiStr);
        }
        else
        {
            if(lastStrIsEmojiStr == currentStrIsEmojiStr)
            {
                int index = StrDatas.size() - 1;
                StrDatas[index] += subString_utf8;
            }
            else
            {
                StrDatas.push_back(subString_utf8);
                isEmojiStr.push_back(currentStrIsEmojiStr);
            }
        }
        
        lastStrIsEmojiStr = currentStrIsEmojiStr;
    }
    
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
bool Utils::TcpMsgIsLocked(int msgType)
{
    for (std::vector<int>::iterator iter = lockMsgList.begin(); iter != lockMsgList.end(); iter++)
    {
        if (msgType == *iter)
            return true;
    }
    return false;

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

std::string Utils::JobToString(int _job)
{
    switch (_job) {
        case eWarrior:
            return "剑士";
            break;
        case eMage:
            return "法师";
            break;
        case eAssassin:
            return "刺客";
            break;
        default:
            break;
    }
    return "";
}

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

std::string Utils::EquipPartToString(int part)
{
    switch (part) {
        case kEPTHeadGuard:
            return "头部";
            break;
        case kEPTNecklace:
            return "项链";
            break;
        case kEPTBracelet:
            return "手镯";
            break;
        case kEPTRing:
            return "戒指";
            break;
        case kEPTWeapon:
            return "武器";
            break;
        case kEPTBreastGuard:
            return "胸甲";
            break;
        case  kEPTHandGuard:
            return "护手";
            break;
        case kEPTLegGuard:
            return "护腿";
            break;
        case kEPTFootGuard:
            return "护脚";
            break;
        case kEPTFashion:
            return "时装";
            break;
        default:
            return "其他";
            break;
    }
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
    char       buf[80] = {0};
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

time_t Utils::mktimeFromWday(int wday, int hour, int min, int sec)
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

    weektime = nowtime + 3600 * 24 * wday + 3600 * hour + 60*min + sec;
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

int Utils::getWeekDay(time_t time)
{
    struct tm date;

    localtime_r(&time, &date);
    
    return date.tm_wday;
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

std::string Utils::getPostPram(vector<std::string>* pram)
{
    list<string> pramList;
    for (vector<string>::iterator it = pram->begin(); it != pram->end(); it++)
    {
       if(pramList.size()==0)
           pramList.push_back(*it);
        else
        {
            for (list<string>::iterator iter = pramList.begin();1; iter++)
            {
                if(iter == pramList.end())
                {
                    pramList.push_back((*it));
                    break;
                }
                if(contrastASCII((*it), (*iter)) == '<' || contrastASCII((*it), (*iter)) == '=')
                {
                    pramList.insert(iter, (*it));
                    break;
                }
            }
        }
    }
    string pramString = *(pramList.begin());
    for (list<string>::iterator iter = pramList.begin(); iter != pramList.end(); iter++)
    {
        if (iter == pramList.begin())
            continue;
        pramString = makeStr("%s&%s",pramString.c_str(),(*iter).c_str());
    }
    return pramString;
}

char Utils::contrastASCII(string strAA, string strBB)
{
    const char* strA= strAA.c_str();
    const char* strB = strBB.c_str();
    for (int i = 0; i < strAA.length(); i++)
    {
        if (strA[i] == '=' && strB[i] == '=') // m=    m=
        {
            return '=';
        }
        else if(strA[i] == '=' )   //   m=  mm=
        {
            return '<';
        }
        else if(strB[i] == '=')    // mm=     m=
        {
            return '>';
        }
        else if(strA[i] < strB[i])    //   a=   b=
        {
            return '<';
        }
        else if(strA[i]>strB[i])   //  b=      a=
        {
            return '>';
        }
        else  if(strA[i] == strB[i])  //   aa=  aa=
        {
            continue;
        }
    }
    return '=';
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

int xyJsonAsInt(const Json::Value &val, int defaultValue)
{
	if (val.type() == Json::stringValue) {
		return Utils::safe_atoi(val.asString().c_str());
	}
	if (val.type() == Json::intValue) {
		return val.asInt();
	}
	if (val.type() == Json::booleanValue) {
		return val.asBool() ? 1 : 0;
	}
	return defaultValue;
}

string xyJsonAsString(const Json::Value &val)
{
	if (val.type() == Json::stringValue) {
		return val.asString();
	}
	if (val.type() == Json::intValue) {
		return Utils::itoa(val.asInt());
	}
    if (val.type() == Json::uintValue) {
        stringstream ss;
		ss << val.asUInt();
		return ss.str();
    }
	if (val.type() == Json::realValue) {
		stringstream ss;
		ss << val.asDouble();
		return ss.str();
	}
	return "";
}


int xyDiffday(time_t first, time_t second)
{
	struct tm firstTm;
	localtime_r(&first, &firstTm);
	firstTm.tm_hour = 0;
	firstTm.tm_min = 0;
	firstTm.tm_sec = 0;
	
	struct tm secondTm;
	localtime_r(&second, &secondTm);
	secondTm.tm_hour = 0;
	secondTm.tm_min = 0;
	secondTm.tm_sec = 0;
	
	int sec = (int)difftime(mktime(&secondTm), mktime(&firstTm));
	return abs(sec) / (24 * 3600);
}


/*
 *用来连接字串,传入char *类型数据
 */
string str_cat(const int count, ...)
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

int find_and_replace(string &src, const int count, ...)
{
    va_list args;
    va_start(args, count);
    string tmp;
    int cnt = 0;
    for (int i = 0; i < count; ++i)
    {
        tmp = va_arg(args, char *);
        if (find_first_and_replace(src, "{:str:}", tmp))
        {
            ++cnt;
        }
    }
    va_end(args);
    return cnt;
}

bool find_first_and_replace(string &src, const string &substr, const string &replacestr)
{
    int pos = src.find(substr);
    if (pos == string::npos)
    {
        return false;
    }
    src.replace(pos, substr.size(), replacestr);
    return true;
}

string find_and_replace_all(std::string& in ,const std::string& findstr ,const std::string& replacestr)
{
    if(in.empty() || findstr.empty())
        return in;
      
    int pos=0;
    
    while (  ( pos = in.find(findstr,pos) ) != string::npos )
    {
        in.replace(pos, findstr.size(), replacestr);

        pos += replacestr.size();
    }
    
    return in;
}



void strVecToNumVec(vector<string> &strvec, vector<int> &numvec)
{
    numvec.resize(strvec.size());
    for (int i = 0; i < strvec.size(); ++i)
    {
        numvec[i] = Utils::safe_atoi(strvec[i].c_str());
    }
}

void strToNumVec(const char *str, const char *sep, vector<int> &vec)
{
	StringTokenizer st(str, sep);
	if (st.count() <= 0) {
		return;
	}
    vec.resize(st.count());
	for (int i = 0; i < st.count(); i++) {
		vec[i] = Utils::safe_atoi(st[i].c_str());
	}
}


void strVecToFloatVec(vector<string> &strvec, vector<double> &floatvec)
{
    floatvec.resize(strvec.size());
    for (int i = 0; i < strvec.size(); ++i)
    {
        floatvec[i] = atof(strvec[i].c_str());
    }
}

void strToFloatVec(const char *str, const char *sep, vector<double> &vec)
{
	StringTokenizer st(str, sep);
	if (st.count() <= 0) {
		return;
	}
    vec.resize(st.count());
	for (int i = 0; i < st.count(); i++) {
		vec[i] = atof(st[i].c_str());
	}
}

bool isSameDay(const time_t &time1, const time_t &time2)
{
    return  (0 == xyDiffday(time1, time2));
}


// added by jianghan for 一个概率平均分布的算法模型
/*
 char map[1000];
 void AverageMap(float rate)
 {
 float begin = 0.0f;
 memset( map, '0', 1000 );
 for( int k=0;k<1000;k++ )
 {
 if( begin/(float)(k+1) < rate )
 {
 map[k] = '1';
 begin += 1.0f;
 }
 cout<<map[k];
 }
 cout<<endl;
 }
 */
// 小数点后7位，百万精度级别的概率命中算法
// 10000000 > index >= 0 时可以用来做顺序命中
// 0 > index 可以用来做一定概率的随机命中
bool randomHit(float rate, int& serial)
{
    //cout<<"rate ="<<rate<<",serial ="<<serial<<endl;
    
    if( rate < 0.0000001)
        rate = 0.0000001;
    
    if( serial < 0 )
        serial = rand()%10000000;
    else if( serial > 10000000 )
        serial = serial%10000000;
    
    int rateNow = serial * rate;
    if( rateNow + 1 > ((serial+1))*rate )
    {
        serial ++;
        return false;
    }else{
        serial ++;
        return true;
    }
}
// end add