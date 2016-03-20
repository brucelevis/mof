//
//  Utf8Matcher.cpp
//  GameSrv
//
//  Created by prcv on 14-5-9.
//
//

#include "Utf8Matcher.h"

void Utf8Matcher::appendMatchString(const string& str)
{
    try
    {
        string::const_iterator pos = str.begin();
        string::const_iterator endPos = str.end();
        
        while (pos != endPos)
        {
            uint32_t cp = utf8::next<string::const_iterator>(pos, endPos);
            mFilters.insert(cp);
        }
    }
    catch (...)
    {
        assert("Not utf8 string");
    }
}

bool Utf8Matcher::match(const string& str, bool& isUtf8)
{
    isUtf8 = true;
    try
    {
        string::const_iterator pos = str.begin();
        string::const_iterator endPos = str.end();
        
        while (pos != endPos)
        {
            uint32_t cp = utf8::next<string::const_iterator>(pos, endPos);
            if (mFilters.find(cp) != mFilters.end())
            {
                return true;
            }
        }
    }
    catch(...)
    {
        isUtf8 = false;
        return false;
    }
    
    return false;
}