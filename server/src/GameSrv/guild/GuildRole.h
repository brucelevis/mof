#pragma once
#include <map>
#include <string>
#include <vector>
#include "RedisHash.h"
using namespace std;

class RoleGuildProperty : public RoleGuildProp
{
public:
    RoleGuildProperty();
    RoleGuildProperty(int roleid);
    virtual ~RoleGuildProperty(){}
    
    const string& getRoleName() {   return mRoleName;   }
    void setRoleName(const char* roleName)          {   mRoleName = roleName;}
    
    int  getViplvl();
    
    void setViplvl(int val)     {   mViplvl = val;      }
    
    int  getJob() const         {   return mJob;        }
    
    int  getLevel();
    void setLevel(int val)      {   mLevel = val;       }
    
    int  getLastActive();
    void setLastActive(int val) {   mLastActive = val;  }
    
    int  getBattleForce();
    int  getRoleType()          {   return mRoleType;   }
    
    void sendPlayerGuildDetail();
    
	// 加贡献
    bool addConstrib(float val , const char* des);
	// 加功勋
    void addExploit(int val,const char* des);
	
    void clearExploit(const char* des);
    
    bool isNull()               {   return getId() == 0;    }
    bool noGuild()              {   return getGuild()==0;   }
    
	//返回公会技能列表
    void getGuildSkill(vector<int>& skills);
	
protected:
    string              mRoleName;
    int                 mViplvl;            
    int                 mJob;
    int                 mLevel;
    int                 mLastActive;        //最后在线
    int                 mBattleForce;       //战斗力
    int                 mRoleType;          //人物类型
};


class RoleGuildMgr
{
    typedef std::map<int,RoleGuildProperty> RoleGuildPropMap;
public:
    RoleGuildMgr(){}
    ~RoleGuildMgr(){}
    
    RoleGuildProperty& getRoleGuild(int roleid);
    
    void offline(int roleid);               
    
    void saveAllRoleGuildProperty();
    
    
protected:
    RoleGuildPropMap    mRoleGuildProps;
    RoleGuildProperty   mDummy;
};

extern RoleGuildMgr SRoleGuildMgr;
#define SRoleGuild(roleid)  SRoleGuildMgr.getRoleGuild((roleid))

