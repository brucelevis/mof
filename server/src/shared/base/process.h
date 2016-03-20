//
//  mof.h
//  GameSrv
//
//  Created by prcv on 13-6-15.
//
//

#ifndef shared_process_h
#define shared_process_h

#include <stdio.h>
#include <string>
#include <map>
#include "Utils.h"
#include "json/json.h"
using namespace std;
//properties

class Properties
{
public:
    void clear()
    {
        mProperties.clear();
    }

    typedef Json::Value ValueType;
    typedef string KeyType;

    int    getInt(const std::string &key, int def = 0);
    bool   setInt(const KeyType& key, int val);

    string getString(const KeyType& key, const string& def = "");
    bool   setString(const KeyType& key, const string& val);

	string getProperty(const std::string &key, const string& def = "");
	bool   setProperty(const KeyType& key, const std::string &val);
	bool   setProperty(const KeyType& key, int val);

    int    getPropertyType(const KeyType& key);

    enum
    {
        eNullValue,
        eIntValue,
        eStringValue,
    };
protected:

	typedef map<KeyType, ValueType> HashType;
	typedef map<KeyType, ValueType>::iterator HashIterType;

	HashType mProperties;			/**< 保存属性的键值对 */
};

class Process
{
public:
    static string execpath;
    static string respath;
    static Properties env;
};

#endif
