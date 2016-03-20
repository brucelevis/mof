//跨服战
#include "msg.h"
#include "NetPacket.h"
#include "sendMessage.h"
#include "serverList.h"
#include "EnumDef.h"
#include "Defines.h"
#include "serverGroupMgr.h"

//#include "serverEntity.h"

hander_msg(req_register_server, req)
{
    ack_register_server ack;
    if (getServerList()->onServerRegister(req.server_id, sessionid)) {
        ack.errorcode = 0;
        log_info("server(" << req.server_id << ") " << req.server_name << " register");
    } else {
        ack.errorcode = 1;
        log_info("server register fail");
    }
    sendNetPacket(sessionid, &ack);
}}

hander_msg(ack_cs_loadPaihangRoleData, ack)
{
    ServerEntity* ret = getServerList()->getServerBySession(sessionid);
    if (NULL == ret) {
        return;
    }
    
    vector<PaihangInfo*> arrObjs;
    
    for (int i = 0; i < ack.data.size(); i++) {
        SortInfo* tmp = new SortInfo;
        tmp->job         = ack.data[i].job;
        tmp->rolename    = ack.data[i].rolename;
        tmp->roleid      = ack.data[i].roleid;
        tmp->battle      = ack.data[i].battle;
        tmp->lvl         = ack.data[i].lvl;
        tmp->recharge    = ack.data[i].recharge;
        tmp->consume     = ack.data[i].consume;
        tmp->serverid    = ack.data[i].serverid;
        arrObjs.push_back(tmp);
    }
    
    ret->getObjDataMgr(ack.type)->loadPaihangData(arrObjs);
    
    CPaihangList* list = ret->getPaihangList(ack.type);
    if (list != NULL) {
        list->onRecvData(ack.total ,arrObjs);
    }
    
}}

hander_msg(ack_cs_loadPaihangPetData, ack)
{
    ServerEntity* ret = getServerList()->getServerBySession(sessionid);
    if (NULL == ret) {
        return;
    }
    
    vector<PaihangInfo*> arrObjs;
    
    for (int i = 0; i < ack.data.size(); i++) {
        SortPetInfo* tmp = new SortPetInfo;
        
        tmp->mastername = ack.data[i].rolename;
        tmp->petid      = ack.data[i].petid;
        tmp->petmod     = ack.data[i].modid;
        tmp->battle     = ack.data[i].battle;
        tmp->growth     = ack.data[i].growth;
        tmp->masterid   = ack.data[i].roleid;
        tmp->serverid    = ack.data[i].serverid;

        arrObjs.push_back(tmp);
    }
    
    ret->getObjDataMgr(ack.type)->loadPaihangData(arrObjs);
    
    CPaihangList* list = ret->getPaihangList(ack.type);
    if (list != NULL) {
        list->onRecvData(ack.total ,arrObjs);
    }
    
}}

hander_msg(req_cs_sendRankList, req)
{
    ServerEntity* server = getServerList()->getServerBySession(sessionid);
    if (NULL == server) {
        return;
    }
    int serverid        = server->getServerId();
    int rankListType    = req.type;

    CServerGroupMgr* mgr = getRankListGroupMgrByType(rankListType);
    if (mgr == NULL) {
        return;
    }

    CServerGroup* group = mgr->getServerGroupByServerId(serverid);
    if (group == NULL) {
        return;
    }
    
    vector<PaihangInfo*> ret;
    group->getRankList(ret);
    
    int beginRank   = req.beginRank;
    int endRank     = req.endRank;
    int rank = -1;
    int rankListIsOpened = false;
    
    if (beginRank <= 0 || beginRank >= endRank || beginRank > ret.size()) {
        return;
    }
    
    if (eSortPet == rankListType) {
        ack_getPetPaihangData ack;
        
        if (group->isOpened()) {
            if (endRank > ret.size()) {
                endRank = ret.size();
            }
            log_info("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
            log_info(beginRank << " " << endRank << " " << ret.size());
            for (int i = beginRank; i <= endRank; i++) {
                SortPetInfo* tmp = (SortPetInfo*)ret[i-1];
                
                obj_Petpaihangdata obj;
                obj.battle      = tmp->battle;
                obj.growth      = tmp->growth;
                obj.modid       = tmp->petmod;
                obj.rolename    = tmp->mastername;
                obj.petid       = tmp->petid;
                
                obj.serverid    = tmp->serverid;
                
                int obj_serverid = tmp->serverid;
                
                ServerEntity* obj_server = getServerList()->getServer(obj_serverid);
                string servername = "";
                if (obj_server != NULL) {
                    servername = obj_server->getName();
                }
                obj.servername = servername;
                
                ack.data.push_back(obj);
                printf("%d index: %d, %s: %d, rank: %d, rolename: %s growth: %d, masterid: %d, battle: %d\n",  tmp->serverid,
                       tmp->petid,
                       getKeyByEnum(req.type),
                       tmp->getSortValueByType(req.type),
                       tmp->getSortRankByType(req.type),
                       tmp->mastername.c_str(),
                       tmp->growth,
                       tmp->masterid,
                       tmp->battle);
            }
            log_info("------------------------------------------------------");
            rank = group->getSortRankByObjId(req.petid);
            rankListIsOpened = true;
        }


        ack.beginRank   = beginRank;
        ack.endRank     = endRank;
        ack.paihangType = req.type;
        ack.myPetRank   = rank;
        ack.isCrossService  = true;
        ack.setSession(req.clientSession);
    
        sendNetPacket(sessionid, &ack);
    
    } else {
        ack_getPaihangData ack;
        
        if (group->isOpened()) {
            
            if (endRank > ret.size()) {
                endRank = ret.size();
            }
            
            log_info(beginRank << " " << endRank << " " << ret.size());
            for (int i = beginRank; i <= endRank; i++) {
                SortInfo* tmp = (SortInfo*)ret[i-1];
                obj_paihangdata obj;
                obj.job         = tmp->job;
                obj.rolename    = tmp->rolename;
                obj.roleid      = tmp->roleid;
                obj.battle      = tmp->battle;
                obj.lvl         = tmp->lvl;
                obj.recharge    = tmp->recharge;
                obj.consume     = tmp->consume;
                obj.serverid    = tmp->serverid;
                obj.rank        = tmp->getSortRankByType(rankListType);
                
                int obj_serverid = tmp->serverid;
                
                ServerEntity* obj_server = getServerList()->getServer(obj_serverid);
                string servername = "";
                if (obj_server != NULL) {
                    servername = obj_server->getName();
                }
                obj.servername = servername;
                
                ack.data.push_back(obj);
                printf("%d index: %d, %s: %d, rank: %d, rolename: %s recharge: %d, consume: %d, battle: %d\n",  tmp->serverid,
                                                                                                               tmp->roleid,
                                                                                                               getKeyByEnum(req.type),
                                                                                                               tmp->getSortValueByType(req.type),
                                                                                                               tmp->getSortRankByType(req.type),
                                                                                                               tmp->rolename.c_str(),
                                                                                                               tmp->recharge,
                                                                                                               tmp->consume,
                                                                                                               tmp->battle);
            }
            log_info("------------------------------------------------------");
            rank = group->getSortRankByObjId(req.roleid);
            rankListIsOpened = true;
        }


        ack.beginRank       = req.beginRank;
        ack.endRank         = endRank;
        ack.paihangType     = req.type;
        ack.myRank          = rank;
        ack.isCrossService  = true;
        ack.rankListIsOpened = rankListIsOpened;
        ack.setSession(req.clientSession);

        sendNetPacket(sessionid, &ack);
    }
}}

handler_msg(req_get_csRoleData_through_cs, req)
{
    int onReqServerid   = req.onReqServerid; //向 这个serverid请求角色数据;
    int roleid          = req.roleid;
    
    ServerEntity* onReqServer = getServerList()->getServer(onReqServerid);
    if (onReqServer == NULL || onReqServer->getState() == ServerEntity::kDisconnect) {
        return;
    }
    req_cs_get_role_Data csReq;
    csReq.roleid           =  roleid;
    csReq.doReqServerid    = req.doReqServerid;
    csReq.clientSession    = req.clientSession;
    
    csReq.onReqServerid = req.onReqServerid;
    sendNetPacket(onReqServer->getSession(), &csReq);
}}

handler_msg(ack_cs_get_role_Data, ack)
{
    int doReqServerid = ack.doReqServerid;
    int clientSession = ack.clientSession;
    
    if (doReqServerid <= 0 || clientSession <= 0 || ack.errorcode != 0) {
        return;
    }
    
    ServerEntity* doReqServer = getServerList()->getServer(doReqServerid);
    if (doReqServer == NULL || doReqServer->getState() == ServerEntity::kDisconnect) {
        return;
    }
    
    ack_get_role_data toReqServerAck;
    toReqServerAck.errorcode            = ack.errorcode;
    toReqServerAck.roleid               = ack.roleid;
    toReqServerAck.onReqServerid        = ack.onReqServerid;
    toReqServerAck.isrobot              = ack.isrobot;
    toReqServerAck.rolename             = ack.rolename;
    toReqServerAck.lvl                  = ack.lvl;
    toReqServerAck.roletype             = ack.roletype;
    toReqServerAck.lastlogin            = ack.lastlogin;
    toReqServerAck.guildname            = ack.guildname;
    toReqServerAck.atk                  = ack.atk;
    toReqServerAck.def                  = ack.def;
    toReqServerAck.hit                  = ack.hit;
    toReqServerAck.dodge                = ack.dodge;
    toReqServerAck.hp                   = ack.hp;
    toReqServerAck.cri                  = ack.cri;
    toReqServerAck.stre                 = ack.stre;
    toReqServerAck.inte                 = ack.inte;
    toReqServerAck.phys                 = ack.phys;
    toReqServerAck.capa                 = ack.capa;
    toReqServerAck.constellid           = ack.constellid;
    toReqServerAck.constellstep         = ack.constellstep;
    toReqServerAck.activepet            = ack.activepet;
    toReqServerAck.equips               = ack.equips;
    toReqServerAck.guildLvl             = ack.guildLvl;
    toReqServerAck.guildposition        = ack.guildposition;
    toReqServerAck.prestige             = ack.prestige;
    toReqServerAck.bodyfashion          = ack.bodyfashion;
    toReqServerAck.weaponfashion        = ack.weaponfashion;
    toReqServerAck.cityBodyFashion      = ack.cityBodyFashion;
    toReqServerAck.fashionList          = ack.fashionList;
    toReqServerAck.playerWardrobeLvl    = ack.playerWardrobeLvl;
    toReqServerAck.guildRank            = ack.guildRank;
    toReqServerAck.enchantId            = ack.enchantId;
    toReqServerAck.enchantLvl           = ack.enchantLvl;
    toReqServerAck.awakeLvl             = ack.awakeLvl;

    toReqServerAck.setSession(clientSession);
    sendNetPacket(doReqServer->getSession(), &toReqServerAck);

}}


handler_msg(req_getCrossServiceRankListStatus, req)
{
    ServerEntity* server = getServerList()->getServerBySession(sessionid);
    if (server == NULL) {
        return;
    }
    int serverid = server->getServerId();

    vector<int> openStatus;
    vector<int> rankListTypes = req.types;
    
    for (int i = 0; i < rankListTypes.size(); i++) {
        int error = 0;

        do
        {
            CServerGroupMgr* mgr = getRankListGroupMgrByType(rankListTypes[i]);
            if (mgr == NULL) {
                error = -1;
                break;
            }
            
            CServerGroup* group = mgr->getServerGroupByServerId(serverid);
            if (group == NULL) {
                error = -1;
                break;
            }
            if (!group->isOpened()) {
                error = -1;
                break;
            }
            
        }
        while (false);
        
        openStatus.push_back(rankListTypes[i]);
        if (error == 0) {
            openStatus.push_back(1);
        } else {
            openStatus.push_back(0);
        }
    }

    ack_checkCrossServiceRankListOpen ack;
    ack.results = openStatus;
    
    ack.setSession(req.clientSession);
    sendNetPacket(server->getSession(), &ack);
    
}}

hander_msg(req_get_csPetData_through_cs, req)
{
    int onReqServerid   = req.onReqServerid; //向 这个serverid请求角色数据;
    int petid           = req.petid;
    
    ServerEntity* onReqServer = getServerList()->getServer(onReqServerid);
    if (onReqServer == NULL || onReqServer->getState() == ServerEntity::kDisconnect) {
        return;
    }
    req_cs_get_pet_Data csReq;
    csReq.petid           = petid;
    csReq.doReqServerid    = req.doReqServerid;
    csReq.clientSession    = req.clientSession;
    
    csReq.onReqServerid = req.onReqServerid;
    sendNetPacket(onReqServer->getSession(), &csReq);
}}

hander_msg(ack_cs_get_pet_Data, ack)
{
    int doReqServerid = ack.doReqServerid;
    int clientSession = ack.clientSession;
    
    if (doReqServerid <= 0 || clientSession <= 0 || ack.errorcode != 1) {
        return;
    }
    
    ServerEntity* doReqServer = getServerList()->getServer(doReqServerid);
    if (doReqServer == NULL || doReqServer->getState() == ServerEntity::kDisconnect) {
        return;
    }
    
    ack_get_pet_data toReqServerAck;
    toReqServerAck.petinfo          = ack.petinfo;
    toReqServerAck.owner            = ack.owner;
    toReqServerAck.errorcode        = ack.errorcode;
    toReqServerAck.onReqServerid    = ack.onReqServerid;

    toReqServerAck.setSession(clientSession);
    sendNetPacket(doReqServer->getSession(), &toReqServerAck);

}}


hander_msg(req_get_csRoleFashion_through_cs, req)
{
    int onReqServerid   = req.onReqServerid; //向 这个serverid请求角色数据;
    
    ServerEntity* onReqServer = getServerList()->getServer(onReqServerid);
    if (onReqServer == NULL || onReqServer->getState() == ServerEntity::kDisconnect) {
        return;
    }
    
    req_get_csRoleFashion_through_cs toServerReq;
    toServerReq.doReqServerid    = req.doReqServerid;
    toServerReq.clientSession    = req.clientSession;
    toServerReq.onReqServerid    = req.onReqServerid;
    toServerReq.roleFashionInfos = req.roleFashionInfos;
    toServerReq.rankListType     = req.rankListType;
    
    sendNetPacket(onReqServer->getSession(), &toServerReq);

}}

hander_msg(ack_get_csRoleFashion_through_cs, ack)
{
    int doReqServerid = ack.doReqServerid;
    int clientSession = ack.clientSession;
    
    if (doReqServerid <= 0 || clientSession <= 0) {
        return;
    }
    
    ServerEntity* doReqServer = getServerList()->getServer(doReqServerid);
    if (doReqServer == NULL || doReqServer->getState() == ServerEntity::kDisconnect) {
        return;
    }
    
    ack_crossServiceRankListTopThreeFashion toReqServerAck;
    toReqServerAck.roleFashionInfos = ack.roleFashionInfos;
    toReqServerAck.rankListType     = ack.rankListType;
    
    toReqServerAck.setSession(clientSession);
    sendNetPacket(doReqServer->getSession(), &toReqServerAck);
}}



