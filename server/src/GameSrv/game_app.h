//
//  game_app.h
//  GameSrv
//
//  Created by 丁志坚 on 15/1/19.
//
//

#ifndef __GameSrv__game_app__
#define __GameSrv__game_app__


#include "thread.h"

#include "NetPacket.h"
#include "Game.h"
#include "hiredis.h"
#include "inifile.h"
#include "hander_include.h"
#include "Scene.h"
#include "cmd_def.h"
#include "gate_mod.h"
#include "application.h"
#include "gate.h"
#include "sdk_inc.h"
#include "Global.h"
#include "GuildMgr.h"
#include "MQ.h"
#include "DynamicConfig.h"
#include "DynamicCfgApp.h"
#include "HttpRequestConfig.h"
#include "recharge_mod.h"
#include "tcp_connection.h"
#include "basedef.h"
#include "io_reactor.h"
#include "process.h"
#include "Utils.h"
#include "lua_helper.h"
#include "os.h"
#include "version.h"
#include "World.h"
#include "RoleExport.h"
#include "Robot.h"
#include "RandName.h"
#include "gift_activity.h"
#include "RolePropVerify.h"
#include "ip_filter.h"
#include "WorldConfig.h"
#include "signal_handler.h"
#include "AccumulateConsumeAwardAct.h"
#include "AccumulateRechargeAwardAct.h"
#include "OnceRechargeAwardAct.h"
#include "RechargeAwardAct.h"
#include "AccumulateFatAwardAct.h"
#include "server_app.h"



class GameApp: public ServerApp
{
public:
    GameApp();
    ~GameApp();
    
    int init(int argc, char** argv);
    
    bool initGlobalMgr();
    
    bool initConfig();
    
    bool initDB();
    
    int main(int argc, char** argv);
    
    void deinit();
    
    void sendNetMessage(int sid, char* data, int len);
private:
    GateMod* mGateMod;
    Game game;
    Thread gameThread;
    SdkProxy sdkproxy;
    GameLog logger;
    Thread LogThread;
    Global global;
    Thread Globalthread;
    
    DynamicCfg dynamcfg;
    Thread dynamcfgThread;
    HttpRequestProxy httpproxy;
    
    RechargeMod mRechargeMod;
    
    World mWorld;
};

ServerApp* getApp();

#endif /* defined(__GameSrv__game_app__) */
