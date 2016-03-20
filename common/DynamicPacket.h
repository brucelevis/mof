//
//  DynamicPacket.h
//  GameSrv
//
//  Created by xinyou on 14-8-5.
//
//

/**
 
 
 DynamicPacketLoader loader;
 CyclemsgParser cparser(&loader);
 cparser.parse(Cfgable::getFullFilePath("cyclemsgdefine.h").c_str());
 NetPacketParser nparser(&loader);
 nparser.parse(Cfgable::getFullFilePath("msgdefine.h").c_str());
 
 //name and json to bytearray
 string fieldsStr = "{\"male\":1}";
 Json::Value fields;
 Json::Reader reader;
 reader.parse(fieldsStr, fields);
 ByteArray byteArray;
 loader.encode("req_randname", fields, byteArray);
 
 //bytearray to name and json
 string name;
 Json::Value fields1;
 loader.decode(byteArray, name, fields1);
 
 **/

#ifndef __GameSrv__DynamicPacket__
#define __GameSrv__DynamicPacket__

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "json/json.h"
#include "ByteArray.h"
using namespace std;


enum DynamicFieldType
{
    kDynamicFieldInt,
    kDynamicFieldInt64,
    kDynamicFieldFloat,
    kDynamicFieldString,
    kDynamicFieldObject,
    kDynamicFieldArray,
};



class DynamicNetPacket
{
public:
    struct ObjectDef;
    struct FieldDef;
    
    struct ObjectDef
    {
        string mName;
        vector<FieldDef> mFields;
    };
    
    struct FieldDef
    {
        string mName;
        int mType;
        union {
            int        mSubType;
            ObjectDef* mObjectDef;
        };
    };
    
public:
    DynamicNetPacket(int type, int id, const char* name)
    {
        mType = type;
        mId = id;
        mName = name;
    }
    
    bool encodeObject(ObjectDef& objectDef, Json::Value& value, ByteArray& byteArray);
    bool encodeField(FieldDef& field, Json::Value& value, ByteArray& byteArray);
    
    bool decodeObject(ObjectDef& objectDef, ByteArray& byteArray, Json::Value& value);
    bool decodeField(FieldDef& field, ByteArray& byteArray, Json::Value& value);
    
	bool decode(ByteArray &byteArray, Json::Value& packet);
    bool encode(Json::Value& packet, ByteArray &byteArray);
    
    int getType() {return mType;}
    int getId() {return mId;}
    const char* PacketName() {return mName.c_str();}
    
    
    void addField(FieldDef& def);
    
private:
    int mType;
    int mId;
    string mName;
    vector<FieldDef> mFields;
};

class DynamicPacketLoader;
class ProtocolParser;
class CyclemsgParser;

class ProtocolParser
{
public:
    bool parse(const char* file);
    bool parse(const char* begin, const char* end);
    
protected:
    virtual bool parse();
    
    bool readCStyleComment();
    bool readCppStyleComment();
    char getNextChar()
    {
        if (mCurrent == mEnd) {
            return 0;
        }
        
        return *mCurrent++;
    }
    
    bool skipPredefine();
    bool skipComment();
    bool skipSpaces();
    
    bool readKeyword(string& keyword);
    bool readToken(char token);
    bool expectToken(char token);
    
    const char* mBegin;
    const char* mEnd;
    const char* mCurrent;
    
    virtual const char* getBeginKeyword() { return ""; }
    virtual const char* getEndKeyword() { return ""; }
    
    virtual bool readBegin() {return false;}
    virtual bool readMember(const string& type) {return false;}
    virtual bool readEnd() {return false;}
};


class CyclemsgParser : public ProtocolParser
{
public:
    CyclemsgParser(DynamicPacketLoader* loader);
    
    virtual bool readBegin();
    virtual bool readMember(const string& type);
    virtual bool readEnd();
    
    
    virtual const char* getBeginKeyword();
    virtual const char* getEndKeyword();
    
    DynamicNetPacket::ObjectDef* mObjectDef;
    DynamicPacketLoader* mPacketLoader;
};

class NetPacketParser : public ProtocolParser
{
public:
    NetPacketParser(DynamicPacketLoader* loader);
    
    virtual bool readBegin();
    virtual bool readMember(const string& type);
    virtual bool readEnd();
    
    
    virtual const char* getBeginKeyword();
    virtual const char* getEndKeyword();
    
    DynamicNetPacket* mNetPacket;
    DynamicPacketLoader* mPacketLoader;
};

class DynamicPacketLoader
{
public:
    static DynamicPacketLoader* instance();
    
    void load();
    
    bool encode(const char* name, Json::Value& fields, ByteArray &byteArray);
    bool decode(ByteArray &byteArray, string& name, Json::Value& fields);
    
public:
    void addObjectDef(DynamicNetPacket::ObjectDef* def);
    DynamicNetPacket::ObjectDef* getObjectDef(const char* name);
    
    void addNetPacket(DynamicNetPacket* def);
    DynamicNetPacket* getNetPacket(const char* name);
    DynamicNetPacket* getNetPacket(int type, int id);
    
private:
    
    map<string, DynamicNetPacket::ObjectDef*> mObjectDefs;
    map<string, DynamicNetPacket*> mNetPacketsByName;
    map<int, DynamicNetPacket*>    mNetPacketsByIndex;
    
    static DynamicPacketLoader* sInstace;
};

#endif /* defined(__GameSrv__DynamicPacket__) */
