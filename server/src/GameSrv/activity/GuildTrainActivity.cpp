//
//  GuildTrainActivity.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-12-16.
//


#include "GuildTrainActivity.h"
#include "MultiPlayerRoom.h"
#include "GameLog.h"
#include "EnumDef.h"
#include "GuildMgr.h"
#include "GuildRole.h"
#include "mail_imp.h"
#include "DataCfg.h"
#include "daily_schedule_system.h"
#include "Honor.h"
#include "Activity.h"
#include "Role.h"

// 类名不要轻易放，改就要同时改 THIS_ROOM_NAME
#define THIS_ROOM_NAME "GuildTrainAct"

class GuildTrainAct: public BaseActRoom
{
    CloneAble(GuildTrainAct)
public:
    GuildTrainAct();
    bool IsOpen();
    
    virtual void OnLeaveRoom(Obj* obj);
    virtual void OnHeartBeat()
    {
        if (0 == getObjCount()) { // 当房间没人时自动回收房间
            Destroy();
        }
    }
    
    struct Award
    {
        Award()
        {
            contribute = exploit = construction = fortune = 0;
            bless = 0.0f;
        }
        int contribute; // 个人贡献
        int exploit; // 功勋
        int construction; // 公会建设度
        int fortune; // 公会财富
        float bless; // 祝福加成 0.0 ~ 0.3
    };
    
    void getAward(Role* role, Award& out);
    void sendAward(Role* role);
protected:
    
    
public:
    int mLevel; // 级别
    int mWaves; // 守的波数
};


GuildTrainAct::GuildTrainAct()
{
    m_bBroadCastInOut = false;
    m_bSyncMove = false;
    mWaves = 0;
    mLevel = 0;
}


bool GuildTrainAct::IsOpen()
{
    return SActMgr.IsActive(ae_guild_train);
}

void GuildTrainAct::OnLeaveRoom(Obj* obj)
{
    if (obj->getType() == eRole) {
        Role* role = (Role*)obj;
//        role->setGuildTrainTime( role->getGuildTrainTime() + 1 );
//        role->saveNewProperty();
//        sendAward(role);
        
        Xylog log(eLogName_GuildTrainAct,role->getInstID());
        log<<"leave";
    }
}

void GuildTrainAct::getAward(Role* role , Award& out)
{
    int maxWave = ChallengeCfg::getWaveSumforChallengeLvl(mLevel);
    Xylog log(eLogName_GuildTrainActAward,role->getInstID());
    log<<mLevel<<mWaves<<maxWave;
    xycheck( mLevel > 0 && mWaves > 0);
    xycheck( mWaves <= maxWave); // 作弊不发了
    
    for (int i = 1; i <= mWaves; ++i)
    {
        const ChallengeCfgDef* def = ChallengeCfg::getChallengeCfg(mLevel,i);
        if( NULL == def)
            continue;
        
        out.contribute  += def->addContribute; // 个人贡献
        out.exploit += def->addFeats; // 个人功勋
        out.fortune += def->addWealth; // 公会财富
        out.construction += def->addOverBuild; // 公会建设度
    }
    
    // 被祝福加成
    out.bless = role->getGuildBlessed() * ChallengeCfg::getGuildDef().blessedAward;
    out.contribute *= (out.bless + 1);
    out.exploit *= (out.bless + 1);
    out.fortune *= (out.bless + 1);
    out.construction *= (out.bless + 1);
 
    log<<out.bless<<out.contribute<<out.exploit<<out.fortune<<out.construction;
}

void GuildTrainAct::sendAward(Role* role)
{
    Award award;
    getAward(role,award);
    
    string des = GameTextCfg::getString("1044");
    if (des.empty())
    {
        des = "公会试炼";
    }
    
    RoleGuildProperty & property = SRoleGuild(role->getInstID());
    property.addConstrib(award.contribute, "公会试炼");
    property.addExploit(award.exploit, "公会试炼");
    property.save();
    
    
    Guild& guild = SGuildMgr.getRoleGuild(role->getInstID());
    if( ! guild.isNull())
    {
        guild.addFortuneConstruction(award.fortune, award.construction,
                                     role->getRolename().c_str(),des.c_str());
        guild.update();
    }
    
//    stringstream content;
//    content << addContribute << "个人贡献" << addFeats << "功勋"
//    << addWealth << "公会财富" << addOverBuild << "公会建设度";
//    
//    sendMail(0,"系统",role->getRolename().c_str(),"公会试炼奖励",
//             content.str().c_str(),"", "");
}


void on_req_get_guild_train_data(Role* role , req_get_guild_train_data& req)
{
    assert(role);
    int maxtime = ChallengeCfg::getGuildDef().traintNum;
    ack_get_guild_train_data ack;
    ack.leftTimes = maxtime -  role->getGuildTrainTime();
    if(ack.leftTimes < 0)
        ack.leftTimes = 0;
    sendNetPacket(role->getSessionId(), &ack);
}

#define return_SendAckEnterGuildTrainRoom( err ) \
do{\
  ack.errorcode = err;  \
  return sendNetPacket(role->getSessionId(), &ack);\
}while(0)

extern bool checkCanEnterCopy(Role* role, int copyid);

void on_req_enter_guild_train_room(Role* role, req_enter_guild_train_room& req)
{
    assert(role);
    
    Guild& guild = SGuildMgr.getRoleGuild(role->getInstID());
    
    xycheck( ! guild.isNull()); // 保证有公会
    
    //防止客户端重复发多条消息过来
    if (0 < role->getPreEnterSceneid()) {
        return;
    }
    
    //判断可否进入，防止同时进入多个副本
    if (!checkCanEnterCopy(role, req.sceneid)) {
        return;
    }
    
    ack_enter_guild_train_room ack;
    ack.inspireAddBattleForce = ChallengeCfg::getGuildDef().inspiredfighting *
         SGuildMgr.getRoleGuild(role->getInstID()).getInspired();
    
    float addbf = ack.inspireAddBattleForce ;
    xyassertf(addbf>=0 ,"%f",addbf);
    
    role->getBriefBatforce(ack.bat);
    
    ack.bat.maxhp *= (1 + addbf);
    ack.bat.atk *= (1 + addbf);
    ack.bat.def *= (1 + addbf);
    ack.bat.hit *= (1 + addbf);
    ack.bat.dodge *= (1 + addbf);
    ack.bat.cri *= (1 + addbf);
    
    
    GuildTrainAct* room = GuildTrainAct::Create();
    xyassert(room);

    send_if(! room->IsOpen(), CE_ACTIVE_IS_NOT_OPEN);
    
    int maxtime = ChallengeCfg::getGuildDef().traintNum;
    send_if(role->getGuildTrainTime() >= maxtime , CE_YOUR_ACTIVITION_TIMES_USEUP);

    //由公会等级决定能不能进入
    ChallengeCfgDef* challengeCfg = ChallengeCfg::getChallengeCfg(req.towerlvl, 1);
    if (challengeCfg == NULL) {
        return;
    }
    
    send_if(guild.getLevel() < challengeCfg->minGuildLvl, CE_GUILD_LEVEL_TOO_LOW);
    
    xycheck( ChallengeCfg::getWaveSumforChallengeLvl(req.towerlvl) > 0);
    
    SceneCfgDef* sceneCfg = SceneCfg::getCfg(req.sceneid);
    xycheck(sceneCfg && sceneCfg->sceneType == stChallenge);
    
    send_if(role->getLvl() < sceneCfg->minLv , CE_YOUR_LVL_TOO_LOW);
    
    send_if( ! room->Add(role, 0, 0, false) , CE_UNKNOWN );
    
    role->setPreEnterSceneid(req.sceneid);
    room->mLevel = req.towerlvl;
    

    Xylog log(eLogName_GuildTrainAct,role->getInstID());
    log<<"enter"<<req.towerlvl;
    
    
    if (role->getDailyScheduleSystem()) {
        role->getDailyScheduleSystem()->onEvent(kDailyScheduleGuildChallenge, 0, 1);
    }
    
    send_if(true,CE_OK);
}

void on_req_leave_guild_train_room(Role* role, req_leave_guild_train_room& req)
{
    assert(role);

    GuildTrainAct* room = (GuildTrainAct*)ActRoomMGR.find(THIS_ROOM_NAME, role);
    xycheck(room);
    
    GuildTrainAct::Award award;
    room->getAward(role, award);
    
    //主动退出或死亡的时候才扣次数
    role->setGuildTrainTime( role->getGuildTrainTime() + 1 );
    role->saveNewProperty();
    room->sendAward(role);
    
    room->Remove(role);
    role->setCurrSceneId(role->getSceneID());
    role->removeGuildInspired();
    
    ack_leave_guild_train_room ack;
    ack.errorcode = CE_OK;
    ack.contribute = award.contribute;
    ack.exploit = award.exploit;
    ack.construction = award.construction;
    ack.fortune = award.fortune;
    ack.bless = award.bless;
    
    role->backToCity();
    sendNetPacket(role->getSessionId(), &ack);
}

void on_req_guild_train_room_next_wave(Role* role, req_guild_train_room_next_wave& req)
{
    assert(role);
    Xylog log(eLogName_GuildTrainAct,role->getInstID());
    log<<req.killwave;
    
    GuildTrainAct* room = (GuildTrainAct*)ActRoomMGR.find(THIS_ROOM_NAME, role);
    xycheck(room);
    xycheck(req.killwave == room->mWaves + 1);
    room->mWaves ++;
}




