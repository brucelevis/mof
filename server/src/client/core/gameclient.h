//
//  gameclient.h
//  client
//
//  Created by xinyou on 14-5-28.
//
//

#ifndef __client__gameclient__
#define __client__gameclient__

#include <iostream>

#ifdef __cplusplus
extern "C"{
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

#include "msg.h"
#include "gate.h"
#include "poller.h"

extern void sendRequest(int sessionId, INetPacket* packet);

class GameClient : public IoEvent
{
public:
    static GameClient* create(int sid)
    {
        GameClient* client = new GameClient;
        client->mSessionId = sid;
        return client;
    }
    
    virtual void timer_event(int id)
    {
        enterWorldBossScene();
    }
    
    GameClient()
    {
        printf("gameclient\n");
    }
    ~GameClient()
    {
        printf("~gameclient\n");
    }
    
    void sendRequest(INetPacket* packet)
    {
        ::sendRequest(mSessionId, packet);
    }
    
    int mRoleId;
    int mSessionId;
    
    string mAccount;
    string mPassword;
    
    void login();
    void getRoles()
    {
        req_getroles req;
        sendRequest(&req);
    }
    void selectRole(int roleId)
    {
        req_select_role req;
        req.roleid = roleId;
        sendRequest(&req);
    }
    void getRandName();
    void createRole(const char* roleName);
    
    void enterCity(int cityId, int threadId)
    {
        req_enter_city req;
        req.cityid = cityId;
        req.threadid = threadId;
        sendRequest(&req);
    }
    
    void enterScene(int sceneId)
    {
        req_enter_scene req;
        req.sceneid = sceneId;
        sendRequest(&req);
    }
    
    void enterWorldBossScene()
    {
        req_enter_boss_scene req;
        req.bossType = 0;
        sendRequest(&req);
    }
    
    
    //世界场景
    void getWorldScenes(int sceneMod);
    void enterWorldScene(int sceneMod, int sceneId);
    void getWorldSceneObjects();
    
    void syncAttack(int instId);
    void syncSkill(int skillId, int instId);
    
    
    void callScript(lua_State *L, const char* func);
    
    //聊天
    void worldChat(const char* message);
    
public:
    void onMessage(const char* message);
    
    static bool init();
    static void uninit();
};


#endif /* defined(__client__gameclient__) */
