//
//  GameText.cpp
//  GameSrv
//
//  Created by prcv on 14-9-26.
//
//

#include "GameWords.h"
#include "Utils.h"

string ItemTag::toString()
{
    string str = strFormat("{\"type\":\"item\", \"id\":\"%d\", \"json\":\"%s\"}", mItem.item, mItem.json.c_str());
    return str;
}


string PetTag::toString()
{
    string str = strFormat("{\"type\"=\"pet\" \"id\"=\"%d\" \"mod\"=\"%d\"}", mPetId, mPetMod);
    return str;
}

string LinkTag::toString()
{
    string str = strFormat("{\"type\"=\"link\" \"href\"=\"%s\" \"desc\"=\"%s\"}", mHref.c_str(), mDesc.c_str());
    return str;
}

string TextTag::toString()
{
    string str = strFormat("{\"type\"=\"text\" \"text\"=\"%s\" \"color\"=\"%d\"}", mText.c_str(), mColor);
    return str;
}

GameWords::~GameWords()
{
    for (int i = 0; i < mTags.size(); i++) {
        WordsTag* tag = mTags[i];
        delete tag;
    }
}

string GameWords::toString()
{
    string ret = "[";
    for (int i = 0; i < mTags.size(); i++) {
        string str = mTags[i]->toString();
        ret.append(str);
    }
    ret.append("]");
    
    return ret;
}

void GameWords::appendItemTag(const ItemGroup& item)
{
    ItemTag* tag = new ItemTag;
    tag->mItem = item;
    appendTag(tag);
}

void GameWords::appendTextTag(const char* text, int color)
{
    TextTag* tag = new TextTag;
    tag->mText = text;
    tag->mColor = color;
    appendTag(tag);
}

void GameWords::appendLinkTag(const char* href, const char* desc)
{
    LinkTag* tag = new LinkTag;
    tag->mHref = href;
    tag->mDesc = desc;
    appendTag(tag);
}

void GameWords::appendPetTag(int petId, int petMod)
{
    PetTag* tag = new PetTag;
    tag->mPetId = petId;
    tag->mPetMod = petMod;
    appendTag(tag);
}

void GameWords::appendTag(WordsTag* tag)
{
    mTags.push_back(tag);
}

