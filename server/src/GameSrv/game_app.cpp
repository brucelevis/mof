//
//  game_app.cpp
//  GameSrv
//
//  Created by 丁志坚 on 15/1/19.
//
//

#include "game_app.h"
#include "main.h"
#include "io_reactor.h"
#include "GameTable.h"
#include "centerClientIoReactor.h"
#include "Retinue.h"
#include "DataCfg.h"

static std::string GAME_SERVER_RUN_RECORD_FILE="info.dat";


ServerApp* getApp()
{
    static GameApp app;
    return &app;
}


static redisContext* getConnectionDB()
{
    return RedisHelper::createContext(Process::env.getString("db_host").c_str(),
                                      Process::env.getInt("db_port"),
                                      Process::env.getString("db_password").c_str(),
                                      Process::env.getInt("db_index"));
}


static bool getPublicServerState(void* obj, void* param)
{
    vector<pair<string, bool> >* connectState = (vector<pair<string, bool> >*)param;
    PublicServer* server = (PublicServer*)obj;
    pair<string, bool> state;
    state.first = server->name;
    state.second = server->isConnected();
    connectState->push_back(state);
    return true;
}

static void recordServerState()
{
    std::ofstream recordFile;
    recordFile.open((GAME_SERVER_RUN_RECORD_FILE).c_str());
    recordFile<<"version="<<SERVER_VERSION<<std::endl;
    recordFile<<"isSuccess="<<1<<std::endl;
    recordFile<<"lang="<<Process::env.getString("lang")<<std::endl;
    recordFile<<"serverID="<<Process::env.getInt("server_id")<<std::endl;
    recordFile<<"StartTime="<<time(NULL)<<std::endl;
    recordFile.close();
}

GameApp::GameApp() {}
GameApp::~GameApp() {}

int GameApp::init(int argc, char** argv)
{
    //记录游戏启动的情况
    log_info("server version: " << SERVER_VERSION);
    srand(time(NULL));
    
    Process::execpath = os::getAppPath();
    try{
        IniFile inifile(Process::execpath + "/game.ini");
        Process::env.setString("version", SERVER_VERSION);
        Process::env.setInt("server_id", inifile.getValueT("root", "server_id", 0));
        Process::env.setString("server_name", inifile.getValue("root", "server_name", ""));
        Process::env.setString("logpath", inifile.getValue("root", "logpath","./gamelog/"));
        
        Process::env.setString("db_host", inifile.getValue("root","db_host","127.0.0.1"));
        Process::env.setInt("db_port", inifile.getValueT("root","db_port", 6379));
        Process::env.setString("db_password", inifile.getValue("root", "db_password"));
        Process::env.setInt("db_index", inifile.getValueT("root","db_index", 0));
        
        Process::env.setInt("gm_mode", inifile.getValueT("root", "gm_mode", 0));
        Process::env.setInt("testmode", inifile.getValueT("root", "testmode", 0));
        Process::env.setInt("sdkmode", inifile.getValueT("root", "sdkmode", 0));
        Process::env.setInt("onekey_pass", inifile.getValueT("root", "onekey_pass", 0));
        
        //gate
        Json::Value gateConfig;
        {
            string host = inifile.getValue("root", "host", "0.0.0.0");
            unsigned short port = inifile.getValueT("root", "port", 18080);
            int maxConn = inifile.getValueT("root", "maxconn", 8192);
            Json::Value listenerConfig;
            listenerConfig["type"] = "listener";
            listenerConfig["host"] = host;
            listenerConfig["port"] = port;
            listenerConfig["reactor"] = ClientIoReactor::getName();
            listenerConfig["max_conn"] = maxConn;
            gateConfig.append(listenerConfig);
        }

        {
            string host = inifile.getValue("CenterServer", "host");
            unsigned short port = inifile.getValueT("CenterServer", "port", 0);
            if (!host.empty() && port != 0) {
                Json::Value connectorConfig;
                connectorConfig["type"] = "connector";
                connectorConfig["reactor"] = CenterClientIoReactor::getName();
                connectorConfig["host"] = host;
                connectorConfig["port"] = port;
                gateConfig.append(connectorConfig);
            }
        }

        string connectorSections[] = {"GmServer", "LogServer", "MailServer"};
        int connectIds[] = {eGmServer, eLogServer, eMailServer};
        for (int i = 0; i < sizeof(connectorSections) / sizeof(string); i++) {
            string host = inifile.getValue(connectorSections[i], "host");
            unsigned short port = inifile.getValueT(connectorSections[i], "port", 0);
            if (host.empty() || port == 0) {
                continue;
            }
            
            Json::Value connectorConfig;
            connectorConfig["type"] = "connector";
            connectorConfig["reactor"] = PublicServerIoReactor::getName();
            connectorConfig["host"] = host;
            connectorConfig["port"] = port;
            connectorConfig["server_name"] = connectorSections[i];
            connectorConfig["server_id"] = connectIds[i];
            gateConfig.append(connectorConfig);
        }
        Process::env.setProperty("gate", Json::FastWriter().write(gateConfig).c_str());
        
        
        //lang
        Process::env.setString("lang", inifile.getValue("root", "lang", "zh_cn"));
        
        string startTime = inifile.getValue("root", "starttime", "2013-01-01 00:00:00");
        tm starttm;
        memset(&starttm, 0, sizeof(starttm));
        strptime(startTime.c_str(), "%Y-%m-%d %H:%M:%S", &starttm);
        time_t starttime = mktime(&starttm);
        Process::env.setInt("starttime", starttime);
        
        string startTipStr = StrMapCfg::getString("start_tip", "将于{:str:}开启");
        VarStr varStr(startTipStr.c_str());
        varStr << startTime.c_str();
        string startTip = varStr.getStr();
        Process::env.setString("starttip", inifile.getValue("root", "starttip", startTip));
        
        log_info("testmode: " << Process::env.getInt("testmode"));
        log_info("lang: " << Process::env.getString("lang"));
        
    } catch(...) {
        log_error("read game ini fail");
        return -1;
    }
    
    redisContext* redisdb = getConnectionDB();
    if (redisdb == NULL) {
        return -1;
    }
    g_DBMgr.registerDB(pthread_self(), redisdb);
    
    initConfig();
    if (!initDB()) {
        log_error("init database error");
        return -1;
    }
    initGlobalMgr();
    
    Game::MQ = skynet_mq_create(MQ_GAME);
    //GateMod::MQ = skynet_mq_create(MQ_GATE);
    
    mGateMod = new GateMod(this);
    mGateMod->registerIoReactor(ClientIoReactor::getName(),
                                (IoReactorCreator)ClientIoReactor::creator);
    mGateMod->registerIoReactor(PublicServerIoReactor::getName(),
                                (IoReactorCreator)PublicServerIoReactor::creator);
    mGateMod->registerIoReactor(CenterClientIoReactor::getName(),
                                (IoReactorCreator)CenterClientIoReactor::creator);
    
    World::sMQ = skynet_mq_create(0);
    GameLog::MQ = skynet_mq_create(MQ_LOGGER);
    DynamicCfg::MQ = skynet_mq_create(MQ_DYNAMIC_READ_CFG);
    HttpRequest::MQ = skynet_mq_create(MQ_HTTP_REQUEST);
    Global::MQ = skynet_mq_create(MQ_PAIHANG);
    SdkProxy::MQ = skynet_mq_create(MQ_SDK_PROXY);
    
    signalInit();
    
    //游戏进程日志
    string logPath = Process::env.getString("program_log", "log.log");
    log_init(LOG_LEVEL_NULL, logPath.c_str());
    
    return 0;
}

bool GameApp::initGlobalMgr()
{
    g_RobotMgr.load();
    return true;
}

bool GameApp::initConfig()
{
    Process::respath = Process::execpath + "/../res";
    
    string lang = Process::env.getString("lang");
    Cfgable::pathPrefix = Process::respath + "/" + lang + "/config/";

    g_ConfigTableMgr.addSearchPath(Process::respath + "/" + lang + "/system/");
    
    g_ConfigTableMgr.addSearchPath(Cfgable::pathPrefix);
    
    g_ConfigTableMgr.loadAllTable();
    
//    Timestamp _ts;
//    
//    printf("begin pipeRedis\n");
//    _ts.update();
//    
//        MyPipeRedis testPipe(get_DbContext());
//        for (int i = 1; i <= 100000; i++) {
//            
////            testPipe.myRedisAppendCommand("set testStr %d", i);
//        }
//    testPipe.commitImmediately();
//    
//    int64_t endTime = _ts.elapsed();
//    printf("%s\n",result.readStr().c_str());
//    printf("use_sec:%lld\n\n\n", endTime);
    Cfgable::loadAll();
    
//    g_ConfigInterpreter.registerLoaderFunc("scene.ini", SceneCfg::load);
//    g_ConfigInterpreter.callFunc("scene.ini");
    
    RobotCfg::load("robot.ini");
    
    g_IpFilterMgr.load(Process::respath + "/" + lang + "/ip_filter.ini");
    
    g_RolePropVerify.init("role_prop_verify.ini");
    g_WorldConfig.load("world_battle.ini");
    
    // 充值活动
    g_AccumulateConsumeAward.init();
    g_AccumulateRechargeAwardAct.init();
    g_OnceRechargeAwardActivity.init();
    g_act.init();
    g_GiftActivityMgr.init();
    g_AccumulateConsumeFatAward.init();
    
    g_RechargeRankListGroupCfg.load(Cfgable::getFullFilePath("CrossServiceRechargeRankListCfg.ini"), eSortRecharge);
    g_ConsumeRankListGroupCfg.load(Cfgable::getFullFilePath("CrossServiceConsumeRankListCfg.ini"), eSortConsume);
    
    return true;
}


bool GameApp::initDB()
{
    RedisResult result;
    //初始化机器人信息
    string robotInfoKey = "robot:info";
    result.setData(redisCmd("exists %s", robotInfoKey.c_str()));
    if (result.readInt() == 0) {
        RandNameMgr randNameMgr;
        randNameMgr.load(Cfgable::getFullFilePath("robotnames.conf").c_str());
        
        Json::FastWriter writer;
        for (int i = 0; i < RobotCfg::sCfgDatas.size(); i++) {
            
            int startId = RobotCfg::sCfgDatas[i].first.first;
            int endId = RobotCfg::sCfgDatas[i].first.second;
            
            RobotCfgDef* def = RobotCfg::sCfgDatas[i].second;
            
            string robotName = def->name;
            for (int j = startId; j <= endId; j++) {
                
                int robotType = def->roletype ? def->roletype : random() % 6 + 1;
                bool isMale = (robotType - 1) / 3;
                
                string robotName = def->name;
                if (!robotName.empty()) {
                    if (startId != endId) {
                        robotName = strFormat("%s_%d", robotName.c_str(), j);
                    }
                } else {
                    robotName = randNameMgr.getRandName(isMale);
                }
                
                if (robotName.empty()) {
                    robotName = strFormat("<机器人>_%d", j);
                }
                randNameMgr.update(isMale, robotName, false);
                
                Json::Value value;
                value["id"] = j;
                value["role_type"] = robotType;
                value["name"] = robotName;
                value["prop_rate"] = range_randf(-0.1f, 0.1f);
                string robotInfo = writer.write(value);
                doRedisCmd("hset %s %d %s", robotInfoKey.c_str(), j, robotInfo.c_str());
            }
        }
    }
    
    //角色
    result.setData(redisCmd("exists %s", Role::sGenIdKey));
    if (result.readInt() != 0) {
        return true;
    }
    
    int serverid = Process::env.getInt("server_id");
    result.setData(redisCmd("set %s %d", Role::sGenIdKey, (serverid << 20) + 10000));
    
    //宠物
    result.setData(redisCmd("exists %s", Pet::sGenIdKey));
    if (result.readInt() == 0) {
        result.setData(redisCmd("set %s %d", Pet::sGenIdKey, 10000));
    }
    
    //侍魂
    result.setData(redisCmd("exists %s", RetinueMgr::sGenIdKey));
    if (result.readInt() == 0) {
        result.setData(redisCmd("set %s %d", Pet::sGenIdKey, 10000));
    }

    //竞技场排行榜
    result.setData(redisCmd("zcount paihang:jjc -inf +inf"));
    if (result.readInt() == 0) {
        for (int i = 1; i <= 5000; i++) {
            uint64_t robotId = MAKE_OBJ_ID(kObjectRobot, i);
            doRedisCmd("zadd paihang:jjc %d %lld", i, robotId);
        }
    }
    
    return true;
}


int GameApp::main(int argc, char** argv)
{
    log_info("starting game module");
    CheckCondition(game.init(), return 0;)
    gameThread.setData(getConnectionDB());
    gameThread.start(&game);
    
    log_info("starting world module");
    mWorld.start();
    
    //游戏功能日志模块
    log_info("starting logger module");
    logger.init();
    LogThread.start(&logger);
    
    log_info("starting global module");
    global.init();
    Globalthread.setData(getConnectionDB());
    Globalthread.start(&global);
    
    log_info("starting sdk proxy module");
    sdkproxy.init();
    sdkproxy.start();
    
    log_info("starting recharge module");
    mRechargeMod.init();
    mRechargeMod.start();
    
    //启动定时器
    log_info("starting timer module");
    dynamcfgThread.start(&dynamcfg);
    startTimer();
    
    log_info("starting http proxy module");
    httpproxy.init();
    httpproxy.start();
    
    log_info("starting gate module");
    CheckCondition(mGateMod->init(), return 0;)
    //mGateMod.start();
    
    runBackground();
    
    //记录游戏启动的情况
    recordServerState();
    
    
    log_info("starting server ok");
    waitForTerminationRequest();
    return 0;
}

void GameApp::deinit()
{
    log_info("stoping ...\n");
    
    waitTerm();
    //mGateMod.stop();
    
    mRechargeMod.stop();
    
    game.stop();
    gameThread.join();
    redisFree((redisContext*)gameThread.getData());
    
    logger.stop();
    LogThread.join();
    
    global.stop();
    Globalthread.join();
    
    sdkproxy.stop();
    
    dynamcfg.stop();
    dynamcfgThread.join();
    
    httpproxy.stop();
    
    mWorld.stop();
    
    
    g_fameHallMgr.release();
    
    
    
    log_info("stop success ...\n");
    
    Process::env.clear();
}

void GameApp::sendNetMessage(int sid, char* data, int len)
{
    sendMessageToGate(sid, NET_MSG, data, len);
}