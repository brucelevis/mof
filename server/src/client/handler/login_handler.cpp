#include "NetPacket.h"
#include "msg.h"
#include "lgameclient.h"
#include "gameclient.h"
#include "clientmgr.h"

extern string script;

extern Gate gate;

#define READWRITE(type, name, funcname) \
private:\
    type name;\
public:\
    type& get##funcname() {return name;}\
    void set##funcname(const type& val) {name = val;}

class GameRole
{
    READWRITE(int, mRoleId, RoleId)
    READWRITE(int, mInstId, InstId)
    
public:
    
    void doLogic();
};

void GameRole::doLogic()
{
    lua_State* L = luaL_newstate();
    
    luaL_openlibs(L);
    registerGameClient(L);
    string script = "dologic.lua";
    if (!script.empty())
    {
        int errcode = luaL_dofile(L, script.c_str());
        if (errcode != 0)
        {
            printf("%d %s\n", errcode, luaL_checkstring(L, 1));
        }
        
        GameClient* client = new GameClient;
        client->callScript(L, "doLogic");
        delete client;
    }
    lua_close(L);
}

GameRole g_GameRole;

#define tipInfo printf

#define CHECK_CLIENT(sid, client) \
    GameClient* client = NULL;\
    client = ClientMgr::instance()->getClient(sid);\
    if (client == NULL) { \
        return;\
    } \

hander_msg(ack_login, ack)
{
    CHECK_CLIENT(sessionid, client);
    client->getRoles();
}}

hander_msg(ack_getroles, ack)
{
    CHECK_CLIENT(sessionid, client);
    
    if (ack.errorcode != 0) {
        return;
    }
    
    tipInfo("role list\n");
    for (int i = 0; i < ack.roles.size(); i++) {
        tipInfo("%d %s\n", ack.roles[i].roleid, ack.roles[i].rolename.c_str());
    }
    
    if (ack.roles.size() > 0) {
        client->selectRole(0);
    }
}}

hander_msg(ack_select_role, ack)
{
    CHECK_CLIENT(sessionid, client);
    
    if (ack.errorcode != 0) {
        tipInfo("select role fail");
        return;
    }

    client->enterCity(501101, 0);
}}

hander_msg(ack_enter_city, ack)
{
    CHECK_CLIENT(sessionid, client);
    
    if (ack.errorcode != 0) {
        tipInfo("enter scene failed");
        return;
    }
    
    tipInfo("enter scene %d %d\n", ack.cityid, ack.threadid);
    
    client->enterScene(ack.cityid);
    
    gate.create_timer(150000, 0, (IoEvent*)client);
}}

hander_msg(ack_enter_world_scene, ack)
{
    if (ack.errorcode != 0) {
        tipInfo("enter world scene fail");
        return;
    }
    
}}

hander_msg(ack_get_scene_objects, ack)
{
    for (int i = 0; i < ack.roles.size(); i++)
    {
        if (ack.roles[i].roleid == g_GameRole.getRoleId())
        {
            g_GameRole.setInstId(ack.roleposinfos[i].instid);
            break;
        }
    }
    
}}

hander_msg(ack_enter_boss_scene, ack)
{
    CHECK_CLIENT(sessionid, client);
    
    if (ack.err != 0) {
        tipInfo("enter boss room fail with error code %d\n", ack.err);
        return;
    }
    
    tipInfo("enter boss room success\n");
    
    req_attack req;
    req.sceneid = ack.sceneid;
    req.hurts = 1000;
    req.isCir = 1;
    req.objtype = 1;
    req.elaps_seconds = 1;
    client->sendRequest(&req);
}}


hander_msg(notify_sync_skill, notify)
{
    tipInfo("notify sync skill %d %d\n", notify.sourceID, g_GameRole.getInstId());
}}


hander_msg(ack_verify_server_info, ack)
{
    log_info("server id:" << ack.serverid << " name: " << ack.servername.c_str());
}}



hander_msg(ack_ping, ack)
{
    //req_ping req;
    //sendRequest(&req);
 
    printf("get ack ping\n");
}}