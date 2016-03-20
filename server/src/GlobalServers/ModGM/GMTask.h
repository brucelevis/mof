#ifndef GM_TASK_H
#define GM_TASK_H


#include "json/json.h"

#include <string>
using namespace std;

class CMsgTyped;

class GmTask
{
public:
    virtual string getId() = 0;
    virtual bool create(int serverid, Json::Value& field) = 0;
    virtual bool update(int ts) = 0;
    virtual ~GmTask() {}

    virtual void insertDB() {}
    virtual void updateDB() {}
    virtual void deleteDB() {}
};



#endif
