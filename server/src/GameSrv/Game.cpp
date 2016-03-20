//
//  Game.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#include "Game.h"
#include "../common/inifile.h"
#include "./core/Scene.h"
#include "./core/TokenNode.h"
#include "Role.h"
#include "Defines.h"
#include "Skill.h"
#include "RandName.h"
#include "DBRolePropName.h"
#include "main.h"
#include "Pvp.h"
#include "./core/MultiPlayerRoom.h"

#include "MQ.h"
#include "GuildMgr.h"
#include "cmd_def.h"
#include "process.h"
#include "json/json.h"
#include "GlobalMsg_def.h"
#include "psmgr.h"
#include "ActivityMod.h"
#include "GuildMgr.h"
#include "BossController.h"
#include "lua_helper.h"
#include "HttpRequestApp.h"
#include "AnswerMgr.h"
#include "gift_activity.h"
#include "RolePropVerify.h"
#include "ip_filter.h"
#include "basedef.h"
#include "AntiCheating.h"
#include "QueenBlessMgr.h"
#include "CustomMailApp.h"
#include "RoleMgr.h"
#include "SceneMgr.h"
#include "TreasureFight.h"
#include "GameScript.h"
#include "SyncTeamCopy.h"
#include "WebStateMgr.h"
#include "TwistEggMgr.h"
#include "ChatRoom.h"
#include "MysteriousMgr.h"
#include "RandomEvents.h"

#include "gate_cmdmsg.h"
#include "SessionData.h"
#include "Global.h"
#include "Protocol2Lua.h"
#include "centerClient.h"
#include "worship/worship_system.h"

using namespace std;

message_queue* Game::MQ = NULL;
time_t Game::tick = 0;

bool gmCommand(Role* role, const char* cmd);

bool Game::beforeRun()
{
    redisContext* redisdb = (redisContext*)_thread->getData();
    g_DBMgr.registerDB(_thread->getThreadId(), redisdb);
    
    redisReply* rreply;
    rreply = redisCmd("get gamefreshtime");

    if (rreply->type == REDIS_REPLY_NIL) {
        time_t nowtime = time(NULL);
        struct tm date;
        localtime_r(&nowtime, &date);

        date.tm_hour = 0;
        date.tm_min = 0;
        date.tm_sec = 0;

        time_t nextfreshtime = mktime(&date) + 3600*24;
        char tmp[128];
        sprintf(tmp, "set gamefreshtime %d", (int)nextfreshtime);
        doRedisCmd(tmp);

        mGlobalfreshtime = nextfreshtime;
    }
    else
    {
        mGlobalfreshtime = Utils::safe_atoi(rreply->str);
    }
    freeReplyObject(rreply);
    SceneMgr::init();
    
    
    //公会管理
    SGuildMgr.init();
	
	// 初始HTTP请求回调
	initHttpRequestApp();
    
    // 防作弊
    g_HeatBeatAnitCheating.addSyncClientTimer();
    //
    
    g_RandNameMgr.load(Cfgable::getFullFilePath("names.conf").c_str());
    
    g_VoiceChatMgr.init();

    g_WorshipSystem.init();
    
    return true;
}

void Game::afterRun()
{
    SGuildMgr.update();
    SGuildMgr.release();
}


bool Game::init()
{
    /**          Game
                   |
      ---------------------------
     |             |
                 scene_xxx
                   |
      ---------------------------
     |             |
    NPC_xxx       Obj_xxx

     */
// redis:  关于key的一个格式约定介绍下，object-type:id:field。比如user:1000:password
//  ，blog:xxidxx:title
    
    if (!GameScript::globalInit()) {
        log_error("global init game script fail");
        return false;
    }

    CenterClient::globalInit();
    
    return true;
}
bool test = false;
void Game::update(float dt)
{
    Game::tick = time(NULL);

    SceneMgr::updateAll(dt);

    // 活动场景管理器
    ActRoomMGR.Update(dt);
    
    //神秘副本管理器
    g_MysticalCopyMgr.update();
    
    SPSMgr.update(dt);
    
    //语音数据
    g_VoiceChatMgr.update();

    //
    mBeat++;
    mBeat = mBeat%2147483647;

    if (mBeat % FPS == 0) {
        SSessionDataMgr.update();
        SRoleMgr.updateAll(mBeat*FRAME_TIME_MILLSEC/1000);
        g_WorshipSystem.update();
    }
    
    bool isGlobalFresh = false;

    time_t nowtime = Game::tick;
    if ( nowtime  >= mGlobalfreshtime ) {

        struct tm date;
        localtime_r(&nowtime, &date);

        date.tm_hour = 0;
        date.tm_min = 0;
        date.tm_sec = 0;

        time_t nextfreshtime = mktime(&date) + 3600*24;
        char tmp[128];
        sprintf(tmp, "set gamefreshtime %d", (int)nextfreshtime);
        doRedisCmd(tmp);
        mGlobalfreshtime = nextfreshtime;
        isGlobalFresh = true;
    }

    //全局刷新时间，每天12点
    if (isGlobalFresh) {
        SRandomEventsSys.refresh();
        SGuildMgr.spanDayRefresh();
		STotemMon.refresh();
    }

    // handle one message in queue
    skynet_message m;
    while( 0 == skynet_mq_pop(MQ, &m) )
    {
        int msgType = (m.sz >> 24);
        int msgDataLen = m.sz & 0xFFFFFF;
        void* msgData = m.data;

        switch (msgType) {
            case NET_MSG: {
                SessionData* data = SSessionDataMgr.getData(m.session);
                if (data) {
                    data->lasttime = Game::tick;
                }
                
                do {
                    ByteArray byteArray((char*)msgData, msgDataLen);
                    (void)byteArray.read_int();
                    int type = byteArray.read_int();
                    int id = byteArray.read_int();
                    
                    NetPacketHander fun = hander_recvpacketfun(type, id);
                    if (fun) {
                        INetPacket* packet = create_recvpacket(type, id);
                        if (packet == NULL) {
                            break;
                        }
                        
                        try {
                            packet->decode(byteArray);
                        } catch (...) {
                            delete packet;
                            break;
                        }
                        fun(packet, m.session);
                        delete packet;
                    } else {
                        break;
                        LuaPack luapack = getProtoLuaPacker(type, id);
                        if (luapack == NULL) {
                            break;
                        }
                        
                        LuaCall luaCall(GameScript::instance()->getState(), "luaHandleNetMsg");
                        luaCall.addParameter<int>(m.session);
                        luaCall.addParameter(luapack, &byteArray);
                        luaCall.doCall(GameScript::instance()->getErrorHandle());
                    }
                } while (0);
                
                delete (char*)msgData;
                break;
            }
            case CMD_MSG: {
                ICmdMsg *cmdmsg = (ICmdMsg*)msgData;
                if (cmdmsg != NULL) {
                    cmdmsg->handle();
                    destroy_cmd(cmdmsg);
                }
                break;
            }
            case GLOBAL_MSG: {
                IGlobalMsg *msg = (IGlobalMsg*)msgData;
                if (msg != NULL) {
                    msg->handle();
                    destroy_cmd(msg);
                }
                break;
            }
            case PS_MSG: {
                try {
                    SPSMgr.onResponse(m.session, (char*)msgData, msgDataLen);
                } catch(CExceptMsg& except) {
                    log_error(except.GetMsg());
                } catch(...) {

                }
                delete[] (char*)msgData;
                break;
            }
        }
    }
}


//各种活动初始化
void allActiveInit()
{
//    g_friendDungeActMgr.init(ae_friend_dunge);
//    
//    g_PrintCopyActMgr.init(ae_print_copy);
//    g_PetCampActMgr.init(ae_pet_copy);
//    g_PetDunngeActMgr.init(ae_friend_tower);
    GameActMgr::init();
	
    //神秘副本初始化
    g_MysticalCopyMgr.init();
    
    //boss活动控制器
    g_BossActivityController.Init();
    
    g_AnswerMgr.initialize();
    //
    g_QueenBlessMgr.init();
	
	// 定制邮件初始
	g_CustomMailAppMou.init();
	
	// 神秘商店
	g_MysteriousMan.init();
    
    //初始化公会宝藏战
    g_GuildTreasureFightMgr.guildTreasureFightInit();
    
    g_SyncFightingTeamMgr.init(MAX_SYNC_TEAM_COUNT);
	
	// 服务器web状态初始
	g_WebStateMou.init();
	
	// 扭蛋功能初始
	STwistEggMgr.init();
}
