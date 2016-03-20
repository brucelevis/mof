//
//  GameText.h
//  GameSrv
//
//  Created by prcv on 14-9-26.
//
//

#ifndef __GameSrv__GameWords__
#define __GameSrv__GameWords__

#include <iostream>
#include <vector>
#include <string>

#include "ItemHelper.h"

using namespace std;

struct WordsTag
{
    virtual string toString() {return "";}
};

class ItemTag : public WordsTag
{
public:
    ItemGroup mItem;
    virtual string toString();
};

class PetTag : public WordsTag
{
public:
    int mPetMod;
    int mPetId;
    virtual string toString();
};

class LinkTag : public WordsTag
{
public:
    string mHref;
    string mDesc;
    virtual string toString();
};

class TextTag : public WordsTag
{
public:
    string mText;
    int    mColor;
    virtual string toString();
};

class GameWords
{
public:
    GameWords() {}
    ~GameWords();
    
    void appendPetTag(int petId, int petMod);
    void appendItemTag(const ItemGroup& item);
    void appendTextTag(const char* text, int color);
    void appendLinkTag(const char* href, const char* desc);
    string toString();
    
private:
    void appendTag(WordsTag* tag);
    vector<WordsTag*> mTags;
};

#endif /* defined(__GameSrv__GameText__) */
