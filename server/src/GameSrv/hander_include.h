//
//  hander_include.h
//  GameSrv
//
//  Created by cxy on 13-2-16.
//
//

#ifndef GameSrv_hander_include_h
#define GameSrv_hander_include_h


/* 这里最好不要 include 功能相关的头文件
 * 因为并不是所有的hander.cpp都需要相关功能的代码
 * 相应功能的头文件只添加在发生引用的hander.cpp文件即可
 * 否则会导致编译时间超长
 */

#include "std_includes.h"

#include "msg.h"
#include "main.h"
#include "Defines.h"
#include "Utils.h"
#include "EnumDef.h"
#include "GameLog.h"
#include "BroadcastNotify.h"
#include "Role.h"


//#include "Quest.h"
//#include "Skill.h"
//#include "Paihang.h"
//#include "Monster.h"
//#include "curl_helper.h"
//#include "json/json.h"
//#include "cmd_def.h"
//#include "HttpRequestConfig.h"
//#include "DBRolePropName.h"
//#include "world_cmdmsg.h"
//#include "InviteCodeAct.h"
//#include "SyncTeamCopy.h"
//转换sessionid并检查

#include "Game.h"

#define hand_Sid2Role_check(sessionid,roleid,role) \
    int roleid = SRoleMgr.Session2RoleId(sessionid); \
    Role* role = SRoleMgr.GetRole(roleid);   \
    if(role == NULL){return;}

#define handler_msg hander_msg


class WorldPlayerInfo;
class Role;

//奖励转发放消息协议
void rewardsGridArray2notify_addbagItem(GridArray& effgrids,notify_addbagItem& notify);

//计算任务个数
void CountQuest(std::vector<int>& doingquest,int& outmaincount,int& outbranchcount,int& outdailycount);
//发放日常任务
void SendDailyQuest(Role* role);

int useConsumeItem(ItemCfgDef* cfg, Role* role, int count, string *award = NULL);

string getRandBoxItems(ItemCfgDef* randbox);

extern int sendPetAdventureAward(Role* self, int carryPet, SceneCfgDef* sceneCfg, int flopid, int& fcindex, vector<int>& drops, RewardStruct& notifyReward, bool carryRecommendedPet, string &itemsResult);

extern int sendNormalCopyAward(Role* role,
							   SceneCfgDef* scenecfg,
							   int flopid,
							   int friendId,
							   int& fcindex,
							   vector<int>& drops,
							   RewardStruct& notifyReward,
							   string &itemsResult);

bool checkMonsterExist(int monsterid, Role* role, SceneCfgDef* currScene);

extern WorldPlayerInfo getWorldPlayerInfo(Role* role);

extern bool checkCurrSceneType(int sceneid,const eSceneType &type);

extern bool checkCurrSceneType(SceneCfgDef* scene,const eSceneType &type);

bool checkCanEnterCopy(Role* role, int copyid);

class SendAckEnterCopyHelper
{
public:
    SendAckEnterCopyHelper(Role* role , int copyid)
    {
        assert(role);
        mRole = role;
        mCopyId = copyid;
        mErrorCode = 1;
    }
    
    ~SendAckEnterCopyHelper()
    {
        
    }
    
    bool checkCanEnterCopy(Role* role, int copyid)
    {
        return ::checkCanEnterCopy(role, copyid);
        
//        SceneCfgDef* newscene = SceneCfg::getCfg(copyid);
//        if (newscene == NULL) {
//            return false;
//        }
//        
//        SceneCfgDef* currScene = SceneCfg::getCfg(role->getCurrSceneId());
//        
//        //验证等级
//        if (role->getLvl() < newscene->minLv || role->getVipLvl() < newscene->min_vip) {
//            return false;
//        }
//        //验证精力值
//        if (role->getFat() < newscene->energy_consumption) {
//            return false;
//        }
//        
//        //如果当前是城镇，允许进入任何场景
//        if (currScene->sceneType == stTown) {
//            return true;
//        }   //如果当前是地下城好友地下城，可以进入地下城好友地下城或城镇，但不可以近其他场景
//        else if(currScene->sceneType == stDungeon || currScene->sceneType == stFriendDunge)
//        {
//            if (newscene->sceneType != stTown && currScene->sceneType != newscene->sceneType) {
//                return false;
//            }
//        }
//        else{    //除此之外的，只能进城镇
//            if (newscene->sceneType != stTown) {
//                return false;
//            }
//        }
//        
//        return true;
    }
    
    void setSuccess()
    {
        mErrorCode = 0;
    }
    
protected:
    Role* mRole;
    int mCopyId;
    int mErrorCode;
};


bool canInorgeBattleCheck();

void sendNotifyBagFullAndSendMail(Role* role);

#endif
