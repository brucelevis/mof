//
//  VarStr.h
//  GameSrv
//
//  Created by prcv on 14-9-3.
//
//

#ifndef __GameSrv__VarStr__
#define __GameSrv__VarStr__

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class VarStr
{
public:
    enum SectionType
    {
        eSectionText,
        eSectionVar
    };
    
    struct Section
    {
        int mType;
        string mStr;
    };
    
    VarStr()
    {
    }
    
    VarStr(const char* str)
    {
        compile(str);
    }
    
    bool compile(const char* str)
    {
        mWriteIndex = 0;
        string opStr = str;
        
        string sep = "{:str:}";
        
        std::size_t prev = 0;
        std::size_t pos = 0;
        do {
            pos = opStr.find(sep, prev);
            if (pos == std::string::npos) {
                Section textSection = {eSectionText, opStr.substr(prev)};
                mAllSections.push_back(textSection);
                break;
            }
            
            if (prev != pos) {
                Section textSection = {eSectionText, opStr.substr(prev, pos - prev)};
                mAllSections.push_back(textSection);
            }
            
            Section varSection = {eSectionVar, ""};
            mAllSections.push_back(varSection);
            
            prev = pos + sep.size();
        }
        while (true);
        
        return true;
    }
    
    VarStr& operator<<(const char* str)
    {
        while (mWriteIndex < mAllSections.size()) {
            int handleIndex = mWriteIndex++;
            if (mAllSections[handleIndex].mType == eSectionVar) {
                mAllSections[handleIndex].mStr = str;
                break;
            }
        }
        
        return *this;
    }
    
    string getStr()
    {
        string val;
        for (int i = 0; i < mAllSections.size(); i++) {
            val.append(mAllSections[i].mStr);
        }
        return val;
    }

private:
    int  mWriteIndex;
    vector<Section> mAllSections;
};

#endif /* defined(__GameSrv__VarStr__) */
