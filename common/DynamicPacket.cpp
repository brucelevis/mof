//
//  DynamicPacket.cpp
//  GameSrv
//
//  Created by xinyou on 14-8-5.
//
//

#include "DynamicPacket.h"
#include "json/json.h"
#include "Utils.h"
#include <ctype.h>

static const char* CYCLE_MSG_BEGIN = "begin_cyclemsg";
static const char* CYCLE_MSG_INT = "def_int";
static const char* CYCLE_MSG_FLOAT = "def_float";
static const char* CYCLE_MSG_STRING = "def_string";
static const char* CYCLE_MSG_INT_ARRAY = "def_int_arr";
static const char* CYCLE_MSG_FLOAT_ARRAY = "def_float_arr";
static const char* CYCLE_MSG_STRING_ARRAY = "def_string_arr";
static const char* CYCLE_MSG_END = "end_cyclemsg";

static const char* PACKET_MSG_BEGIN = "begin_msg";
static const char* PACKET_MSG_ERR = "def_err";
static const char* PACKET_MSG_INT = "def_int";
static const char* PACKET_MSG_INT64 = "def_int64";
static const char* PACKET_MSG_FLOAT = "def_float";
static const char* PACKET_MSG_STRING = "def_string";
static const char* PACKET_MSG_OBJECT = "def_object";
static const char* PACKET_MSG_INT_ARRAY = "def_int_arr";
static const char* PACKET_MSG_INT64_ARRAY = "def_int64_arr";
static const char* PACKET_MSG_FLOAT_ARRAY = "def_float_arr";
static const char* PACKET_MSG_STRING_ARRAY = "def_str_arr";
static const char* PACKET_MSG_OBJECT_ARRAY = "def_object_arr";
static const char* PACKET_MSG_END = "end_msg";


bool DynamicNetPacket::encodeObject(ObjectDef& objectDef, Json::Value& value, ByteArray& byteArray)
{
    for (int i = 0; i < objectDef.mFields.size(); i++) {
        DynamicNetPacket::FieldDef fieldDef = objectDef.mFields[i];
        if (!encodeField(fieldDef, value[fieldDef.mName], byteArray)) {
            return false;
        }
    }
    
    return true;
}



bool DynamicNetPacket::decodeObject(ObjectDef& objectDef, ByteArray& byteArray, Json::Value& value)
{
    for (int i = 0; i < objectDef.mFields.size(); i++) {
        DynamicNetPacket::FieldDef fieldDef = objectDef.mFields[i];
        if (!decodeField(fieldDef, byteArray, value[fieldDef.mName])) {
            return false;
        }
    }
    
    return true;
}


bool DynamicNetPacket::decodeField(FieldDef& field, ByteArray& byteArray, Json::Value& value)
{
    switch (field.mType) {
        case kDynamicFieldInt:
        {
            int val = byteArray.read_int();
            value = Json::Value(val);
            break;
        }
        case kDynamicFieldInt64:
        {
            double val = byteArray.read_int64();
            value = Json::Value(val);
            break;
        }
        case kDynamicFieldFloat:
        {
            float val = byteArray.read_float();
            value = Json::Value(val);
            break;
        }
        case kDynamicFieldString:
        {
            string val = byteArray.read_string();
            value = Json::Value(val.c_str());
            break;
        }
        case kDynamicFieldObject:
        {
            decodeObject(*(field.mObjectDef), byteArray, value);
            break;
        }
        case kDynamicFieldArray:
        {
            DynamicNetPacket::FieldDef subField;
            subField.mType = field.mSubType;
            
            int num = byteArray.read_int();
            for (int i = 0; i < num; i++) {
                Json::Value subValue;
                decodeField(subField, byteArray, subValue);
                value.append(subValue);
            }
            break;
        }
        default:
            break;
    }
    
    return true;
}

bool DynamicNetPacket::encodeField(FieldDef& field, Json::Value& value, ByteArray& byteArray)
{
    switch (field.mType) {
        case kDynamicFieldInt:
        {
            int val = 0;
            if (value.isInt()) {
                val = value.asInt();
            }
            
            byteArray.write_int(val);
            break;
        }
        case kDynamicFieldInt64:
        {
            uint64_t val = 0;
            if (value.isDouble()) {
                val = value.asDouble();
            }
            
            byteArray.write_int64(val);
            break;
        }
        case kDynamicFieldFloat:
        {
            float val = 0.0f;
            if (value.isDouble()) {
                val = value.asDouble();
            }
            byteArray.write_float(val);
            break;
        }
        case kDynamicFieldString:
        {
            string val;
            if (value.isString()) {
                val = value.asString();
            }
            byteArray.write_string(val);
            break;
        }
        case kDynamicFieldObject:
        {
            if (!value.isObject()) {
                if (!encodeObject(*(field.mObjectDef), value, byteArray)) {
                    return false;
                }
            }
            break;
        }
        case kDynamicFieldArray:
        {
            if (!value.isArray()) {
                DynamicNetPacket::FieldDef subField;
                subField.mType = field.mSubType;
                
                int size = value.size();
                byteArray.write_int(size);
                for (int i = 0; i < size; i++) {
                    if (!encodeField(subField, value[i], byteArray)) {
                        return false;
                    }
                }
            } else {
                byteArray.write_int(0);
            }
            break;
        }
        default:
            return false;
    }
    
    return true;
}


bool DynamicNetPacket::encode(Json::Value& fields, ByteArray &byteArray)
{
    byteArray.write_int(mType);
    byteArray.write_int(mId);
    for (int i = 0; i < mFields.size(); i++) {
        Json::Value childValue = fields[mFields[i].mName];
        if (!encodeField(mFields[i], childValue, byteArray)) {
            return false;
        }
    }
    
    return true;
}

bool DynamicNetPacket::decode(ByteArray &byteArray, Json::Value& fields)
{
    for (int i = 0; i < mFields.size(); i++) {
        Json::Value childValue;
        if (!decodeField(mFields[i], byteArray, childValue)) {
            return false;
        }
        fields[mFields[i].mName] = childValue;
    }
    
    return true;
}

void DynamicNetPacket::addField(FieldDef& field)
{
    mFields.push_back(field);
}


bool ProtocolParser::readCStyleComment()
{
    while ( mCurrent != mEnd )
    {
        char c = getNextChar();
        if ( c == '*'  &&  *mCurrent == '/' )
            break;
    }
    return getNextChar() == '/';
}


bool ProtocolParser::readCppStyleComment()
{
    while ( mCurrent != mEnd )
    {
        char c = getNextChar();
        if (  c == '\r'  ||  c == '\n' )
            break;
    }
    return true;
}

bool ProtocolParser::parse(const char *filename)
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return false;
    }
    
    fseek(file, 0, SEEK_END);
    int len = ftell(file);
    char* data = new char[len + 1];
    fseek(file, 0, SEEK_SET);
    fread(data, len, 1, file);
    fclose(file);
    data[len] = 0;
    bool ret = parse(data, data + len);
    delete data;
    return ret;
}

bool ProtocolParser::parse(const char* begin, const char* end)
{
    mBegin = begin;
    mEnd = end;
    mCurrent = begin;
    return parse();
}


bool ProtocolParser::parse()
{
    int state = 0;
    for (;;) {
        string keyword;
        if (!readKeyword(keyword)) {
            return false;
        }
        
        switch (state) {
            case 0:
                if (keyword.empty()) {
                    return true;
                } else if (keyword == getBeginKeyword()) {
                    if (!readBegin()) {
                        return false;
                    }
                    state = 1;
                } else {
                    return false;
                }
                break;
            case 1:
                if (keyword == getEndKeyword()) {
                    if (!readEnd()) {
                        return false;
                    }
                    state = 0;
                } else if (!readMember(keyword)) {
                    return false;
                }
                break;
                
            default:
                break;
        }
    }
    
    return false;
}

bool ProtocolParser::skipComment()
{
    if (*mCurrent++ != '/') {
        return false;
    }
    
    char c = getNextChar();
    
    bool successful = false;
    if ( c == '*' ) {
        successful = readCStyleComment();
    } else if ( c == '/' ) {
        successful = readCppStyleComment();
    }
    
    if ( !successful ) {
        return false;
    }
    
    return true;
}

bool ProtocolParser::skipSpaces()
{
    char ch;
    while (mCurrent != mEnd) {
        ch = *mCurrent;
        if (!isspace(ch)) {
            break;
        }
        mCurrent++;
    }
    
    return true;
}

bool ProtocolParser::skipPredefine()
{
    if (*mCurrent != '#') {
        return false;
    }
    
    mCurrent++;
    
    while (mCurrent != mEnd) {
        char c = getNextChar();
        if (  c == '\r'  ||  c == '\n' ) {
            break;
        }
    }
    
    return true;
}

bool ProtocolParser::readToken(char token)
{
    for (;;) {
        char ch = getNextChar();
        if (ch == 0) {
            return false;
        } else if (ch == '/') {
            if (!skipComment()) {
                return false;
            }
        } else if (isspace(ch)) {
            skipSpaces();
        } else {
            return ch == token;
        }
    }
}

bool ProtocolParser::expectToken(char token)
{
    for (;;) {
        char ch = getNextChar();
        if (ch == 0) {
            return false;
        } else if (ch == '/') {
            if (!skipComment()) {
                return false;
            }
        } else if (isspace(ch)) {
            skipSpaces();
        } else {
            return ch == token;
        }
    }
}

bool ProtocolParser::readKeyword(string& keyword)
{
    while (mEnd != mCurrent) {
        char ch = *mCurrent;
        if (ch == 0) {
            return true;
        } else if (ch == '/') {
            if (!skipComment()) {
                return false;
            }
        } else if (isspace(ch)) {
            skipSpaces();
        } else if (ch == '#') {
            skipPredefine();
        } else if (ch == ';') {
            mCurrent++;
        } else if (!isalpha(ch) && !isdigit(ch) && ch != '_') {
            return false;
        } else {
            break;
        }
    }
    
    const char* begin = mCurrent;
    const char* end = mCurrent;
    while (*end) {
        char ch = *end;
        if (!isalpha(ch) && !isdigit(ch) && ch != '_') {
            break;
        }
        end++;
    }
    
    keyword = string(begin, end);
    mCurrent = end;
    return true;
}

CyclemsgParser::CyclemsgParser(DynamicPacketLoader* loader)
{
    mPacketLoader = loader;
}



bool CyclemsgParser::readBegin()
{
    if (!readToken('(')) {
        return false;
    }
    
    string className;
    if (!readKeyword(className)) {
        return false;
    }
    
    if (!readToken(')')) {
        return false;
    }
    
    mObjectDef = new DynamicNetPacket::ObjectDef;
    mObjectDef->mName = className;
    
    return true;
}

bool CyclemsgParser::readMember(const string& keyword)
{
    string memberName;
    if (!readToken('(') || !readKeyword(memberName) || !readToken(')')) {
        return false;
    }
    
    int type = -1;
    int subType = -1;
    if (keyword == CYCLE_MSG_INT) {
        type = kDynamicFieldInt;
    } else if (keyword == CYCLE_MSG_FLOAT) {
        type = kDynamicFieldFloat;
    } else if (keyword == CYCLE_MSG_STRING) {
        type = kDynamicFieldString;
    } else if (keyword == CYCLE_MSG_INT_ARRAY) {
        type = kDynamicFieldArray;
        subType = kDynamicFieldInt;
    } else if (keyword == CYCLE_MSG_FLOAT_ARRAY) {
        type = kDynamicFieldArray;
        subType = kDynamicFieldFloat;
    } else if (keyword == CYCLE_MSG_STRING_ARRAY) {
        type = kDynamicFieldArray;
        subType = kDynamicFieldString;
    }
    
    DynamicNetPacket::FieldDef fieldDef;
    fieldDef.mName = memberName;
    fieldDef.mType = type;
    fieldDef.mSubType = subType;
    mObjectDef->mFields.push_back(fieldDef);
    
    return true;
}

bool CyclemsgParser::readEnd()
{
    if (!readToken('(')) {
        return false;
    }
    
    if (!readToken(')')) {
        return false;
    }
    
    mPacketLoader->addObjectDef(mObjectDef);
    mObjectDef = NULL;
    
    return true;
}

const char* CyclemsgParser::getBeginKeyword()
{
    return CYCLE_MSG_BEGIN;
}

const char* CyclemsgParser::getEndKeyword()
{
    return CYCLE_MSG_END;
}


const char* NetPacketParser::getBeginKeyword()
{
    return PACKET_MSG_BEGIN;
}

const char* NetPacketParser::getEndKeyword()
{
    return PACKET_MSG_END;
}

NetPacketParser::NetPacketParser(DynamicPacketLoader* loader)
{
    mPacketLoader = loader;
}



bool NetPacketParser::readBegin()
{
    if (!readToken('(')) {
        return false;
    }
    
    string className;
    if (!readKeyword(className)) {
        return false;
    }
    
    if (!readToken(',')) {
        return false;
    }
    
    string typeStr;
    if (!readKeyword(typeStr)) {
        return false;
    }
    
    if (!readToken(',')) {
        return false;
    }
    
    string idStr;
    if (!readKeyword(idStr)) {
        return false;
    }
    
    if (!readToken(')')) {
        return false;
    }
    
    int type = Utils::safe_atoi(typeStr.c_str(), -1);
    int id = Utils::safe_atoi(idStr.c_str(), -1);
    if (type == -1 || id == -1) {
        return false;
    }
    
    mNetPacket = new DynamicNetPacket(type, id, className.c_str());
    return true;
}

bool NetPacketParser::readMember(const string& keyword)
{
    int type = -1;
    int subType = -1;
    if (keyword == PACKET_MSG_ERR) {
        type = kDynamicFieldInt;
    } else if (keyword == PACKET_MSG_INT) {
        type = kDynamicFieldInt;
    } else if (keyword == PACKET_MSG_INT64) {
        type = kDynamicFieldInt64;
    } else if (keyword == PACKET_MSG_FLOAT) {
        type = kDynamicFieldFloat;
    } else if (keyword == PACKET_MSG_STRING) {
        type = kDynamicFieldString;
    } else if (keyword == PACKET_MSG_OBJECT) {
        type = kDynamicFieldObject;
    } else if (keyword == PACKET_MSG_INT_ARRAY) {
        type = kDynamicFieldArray;
        subType = kDynamicFieldInt;
    } else if (keyword == PACKET_MSG_INT64_ARRAY) {
        type = kDynamicFieldArray;
        subType = kDynamicFieldInt64;
    } else if (keyword == PACKET_MSG_FLOAT_ARRAY) {
        type = kDynamicFieldArray;
        subType = kDynamicFieldFloat;
    } else if (keyword == PACKET_MSG_STRING_ARRAY) {
        type = kDynamicFieldArray;
        subType = kDynamicFieldString;
    } else if (keyword == PACKET_MSG_OBJECT_ARRAY) {
        type = kDynamicFieldArray;
        subType = kDynamicFieldObject;
    }
    
    DynamicNetPacket::FieldDef fieldDef;
    fieldDef.mType = type;
    fieldDef.mSubType = subType;
    
    if (!readToken('(')) {
        return false;
    }
    
    if (type == kDynamicFieldObject || subType == kDynamicFieldObject) {
        string objTypeName;
        if (!readKeyword(objTypeName) || !readToken(',')) {
            return false;
        }
        
        DynamicNetPacket::ObjectDef* objDef = mPacketLoader->getObjectDef(objTypeName.c_str());
        if (objDef == NULL) {
            return false;
        }
        
        fieldDef.mObjectDef = objDef;
    }
    
    string memberName;
    if (!readKeyword(memberName) || !readToken(')')) {
        return false;
    }
    
    fieldDef.mName = memberName;
    mNetPacket->addField(fieldDef);
    
    return true;
}

bool NetPacketParser::readEnd()
{
    if (!readToken('(')) {
        return false;
    }
    
    if (!readToken(')')) {
        return false;
    }
    
    mPacketLoader->addNetPacket(mNetPacket);
    mNetPacket = NULL;
    
    return true;
}

DynamicPacketLoader* DynamicPacketLoader::sInstace = NULL;

DynamicPacketLoader* DynamicPacketLoader::instance()
{
    if (sInstace == NULL) {
        sInstace = new DynamicPacketLoader;
        sInstace->load();
    }
    
    return sInstace;
}


void DynamicPacketLoader::load()
{
#if CLIENT
//    GameInifile ini(CCFileUtils::sharedFileUtils()->fullPathForFilename("protofiles.ini"));
//    vector<string> cycleMsgKeys;
//    vector<string> msgKeys;
//    ini.getKeys("cyclemsgdefine", inserter(cycleMsgKeys, cycleMsgKeys.begin()));
//    ini.getKeys("msgdefine", inserter(msgKeys, msgKeys.begin()));
//    
//    CyclemsgParser cparser(this);
//    cparser.parse(CCFileUtils::sharedFileUtils()->fullPathForFilename("cyclemsgdefine.h").c_str());
//    //for (int i = 0; i < cycleMsgKeys.size(); i++) {
//    //    string filename = ini.getValue("cyclemsgdefine", cycleMsgKeys[i]);
//    //    cparser.parse(CCFileUtils::sharedFileUtils()->fullPathForFilename(filename.c_str()).c_str());
//    //}
//
//    NetPacketParser nparser(this);
//    nparser.parse(CCFileUtils::sharedFileUtils()->fullPathForFilename("msgdefine.h").c_str());
//    nparser.parse(CCFileUtils::sharedFileUtils()->fullPathForFilename("msgdefine_login.h").c_str());
//    nparser.parse(CCFileUtils::sharedFileUtils()->fullPathForFilename("msgdefine_world.h").c_str());
    //for (int i = 0; i < msgKeys.size(); i++) {
    //    string filename = ini.getValue("msgdefine", cycleMsgKeys[i]);
    //    nparser.parse(CCFileUtils::sharedFileUtils()->fullPathForFilename(filename.c_str()).c_str());
    //}
#else
#endif
}

void DynamicPacketLoader::addObjectDef(DynamicNetPacket::ObjectDef *def)
{
    string name = def->mName;
    map<string, DynamicNetPacket::ObjectDef*>::iterator iter = mObjectDefs.find(name);
    if (iter != mObjectDefs.end()) {
        printf("warning: redefinition of cycle object\n");
    }
    mObjectDefs[name] = def;
}

DynamicNetPacket::ObjectDef* DynamicPacketLoader::getObjectDef(const char *name)
{
    map<string, DynamicNetPacket::ObjectDef*>::iterator iter = mObjectDefs.find(name);
    if (iter != mObjectDefs.end()) {
        return iter->second;
    }
    return NULL;
}

void DynamicPacketLoader::addNetPacket(DynamicNetPacket* def)
{
    string name = def->PacketName();
    int index = (def->getType() << 16) + def->getId();
    mNetPacketsByName[name] = def;
    mNetPacketsByIndex[index] = def;
}

DynamicNetPacket* DynamicPacketLoader::getNetPacket(const char* name)
{
    map<string, DynamicNetPacket*>::iterator iter = mNetPacketsByName.find(name);
    if (iter != mNetPacketsByName.end()) {
        return iter->second;
    }
    
    return NULL;
}

DynamicNetPacket* DynamicPacketLoader::getNetPacket(int type, int id)
{
    int index = (type << 16) + id;
    map<int, DynamicNetPacket*>::iterator iter = mNetPacketsByIndex.find(index);
    if (iter != mNetPacketsByIndex.end()) {
        return iter->second;
    }
    
    return NULL;
}


bool DynamicPacketLoader::encode(const char* name, Json::Value& fields, ByteArray &byteArray)
{
    DynamicNetPacket* packet = getNetPacket(name);
    if (packet != NULL) {
        return packet->encode(fields, byteArray);
    }
    
    return false;
}

bool DynamicPacketLoader::decode(ByteArray &byteArray, string& name, Json::Value& fields)
{
    int type = byteArray.read_int();
    int id = byteArray.read_int();
    
    DynamicNetPacket* packet = getNetPacket(type, id);
    if (packet != NULL) {
        name = packet->PacketName();
        return packet->decode(byteArray, fields);
    }
    
    return false;
}