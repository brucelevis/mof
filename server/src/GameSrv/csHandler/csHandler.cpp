#include "hander_include.h"
#include "centerClient.h"
#include "worship/worship_system.h"
#include "cs_pvp/cs_pvp_battle.h"
#include "Retinue.h"
#include <set>
#include "mail_imp.h"
using namespace std;

extern string getRoleNameByRoleId(int roleid);

float
calcWorshipPlus(int commonTimes, int specialTimes) {
    return CrossServiceWarWorshipCfg::sCommonWorshipPlus * commonTimes +
        CrossServiceWarWorshipCfg::sSpecialWorshipPlus * specialTimes;
}

handler_msg(ack_register_server, ack)
{
    log_info("register to center server " << (ack.errorcode == 0 ? "success" : "fail"));
}}


handler_msg(req_cs_get_role_battle, req)
{
    int roleId = req.target_role_id;
    ack_cs_get_role_battle ack;
    ack.target_server_id = req.target_server_id;
    ack.target_role_id = req.target_role_id;
    ack.src_server_id = req.src_server_id;
    ack.src_role_id = req.src_role_id;
    ack.errorcode = 1;
    ack.custom = req.custom;
    do {
        Role* role = Role::create(roleId, true);
        if (role == NULL) {
            break;
        }
        role->onCalPlayerProp();
        
        ack.worshipPlus = calcWorshipPlus(g_WorshipSystem.getCommonBeWorshipTimes(roleId),
                                          g_WorshipSystem.getSpecialBeWorshipTimes(roleId));
        
        obj_roleBattleProp roleBattle;
        role->getPlayerBattleProp(roleBattle);
        
        Pet* pet = role->mPetMgr->getActivePet();
        obj_petBattleProp petBattle;
        if (pet == NULL) {
            petBattle.petid = 0;
        } else {
            pet->getBattleProp(petBattle);
        }

        Retinue *retinue = role->mRetinueMgr->getActiveRetinue();
        obj_retinueBattleProp retinueBPInfo;
        if(retinue != NULL && retinue->getId() > 0){
            retinue->getBattleProp(retinueBPInfo);
            retinueBPInfo.skills = role->getRetinueMgr()->mSkillMgr->getEquipedSkills();
        }
        ack.retinue_battle = retinueBPInfo;
        
        ack.role_battle = roleBattle;
        ack.pet_battle = petBattle;
        ack.errorcode = CE_OK;
        delete role;
    } while (0);
    
    sendNetPacket(CenterClient::instance()->getSession(), &ack);
}}

handler_msg(ack_cs_get_role_battle, ack)
{
    string custom = ack.custom;
    if (custom == "cspvp") {
        int roleId = ack.src_role_id;
        Role* self = SRoleMgr.GetRole(roleId);
        if (self == NULL) {
            return;
        }
        
        ack_begin_cspvp outAck;
        outAck.errorcode = ack.errorcode;
        if (ack.errorcode == 0) {
            float targetWorshipRate = 1 + ack.worshipPlus;
            outAck.errorcode = 0;
            outAck.sceneid = CSPVP_SCENEID;
            outAck.enemyinfo = ack.role_battle;
            
            outAck.enemyinfo.atk *= targetWorshipRate;
            outAck.enemyinfo.def += 200;
            outAck.enemyinfo.def *= targetWorshipRate;
            outAck.enemyinfo.dodge *= targetWorshipRate;
            outAck.enemyinfo.maxhp *= 10;
            outAck.enemyinfo.maxhp *= targetWorshipRate;
            outAck.enemyinfo.hit *= targetWorshipRate;
            outAck.enemyinfo.cri *= targetWorshipRate;
            
            outAck.enemypetinfo = ack.pet_battle;
            outAck.enemypetinfo.maxHp *= 10;
            outAck.enemyRetinueinfo = ack.retinue_battle;
            
            self->addCsPvpBuf();
            outAck.selfMaxhp = self->mRealBattleProp.getMaxHp();
            outAck.selfatk = self->mRealBattleProp.getAtk();
            outAck.selfdef = self->mRealBattleProp.getDef();
            outAck.selfhit = self->mRealBattleProp.getHit();
            outAck.selfdodge = self->mRealBattleProp.getDodge();
            outAck.selfcri = self->mRealBattleProp.getCri();
            self->getCsPvpState()->beginBattle();
        } else {
            outAck.errorcode = ack.errorcode;
            req_cs_cancel_cspvp req;
            req.role_id = ack.src_role_id;
            req.server_id = ack.src_server_id;
            CenterClient::instance()->sendNetPacket(0, &req);
        }
        self->send(&outAck);
    }
}}

handler_msg(ack_cs_begin_cspvp, ack)
{
    int roleId = ack.src_role_id;
    Role* self = SRoleMgr.GetRole(roleId);
    if (self == NULL) {
        return;
    }
    
    if (ack.errorcode) {
        ack_begin_cspvp outAck;
        outAck.errorcode = ack.errorcode;
        self->send(&outAck);
        return;
    }
    
    if (ack.target_server_id == ack.src_server_id) {
        
        ack_begin_cspvp outAck;
        outAck.errorcode = 1;
        int targetId = ack.target_role_id;
        do {
            Role* target = Role::create(targetId, true);
            if (target == NULL) {
                break;
            }
            
            float worshipRate = 1 + calcWorshipPlus(g_WorshipSystem.getCommonBeWorshipTimes(targetId),
                                                    g_WorshipSystem.getSpecialBeWorshipTimes(targetId));
            
            target->onCalPlayerProp();
            obj_roleBattleProp roleBattle;
            target->getPlayerBattleProp(roleBattle);
            roleBattle.maxhp *= 10;
            roleBattle.maxhp *= worshipRate;
            roleBattle.def += 200;
            roleBattle.def *= worshipRate;
            roleBattle.hit *= worshipRate;
            roleBattle.dodge *= worshipRate;
            roleBattle.cri *= worshipRate;
            roleBattle.atk *= worshipRate;
            
            Pet* pet = target->mPetMgr->getActivePet();
            obj_petBattleProp petBattle;
            int petBattleForce = 0;
            if (pet == NULL) {
                petBattle.petid = 0;
            } else {
                pet->getBattleProp(petBattle);
                petBattle.maxHp *= 10;
                petBattleForce = pet->mBattle;
            }
            
            Retinue *retinue = target -> mRetinueMgr -> getActiveRetinue();
            obj_retinueBattleProp retinueBPInfo;
            if(retinue != NULL && retinue -> getId() > 0){
                retinue -> getBattleProp(retinueBPInfo);
                retinueBPInfo.skills = target -> getRetinueMgr() -> mSkillMgr -> getEquipedSkills();
            }
            
            
            outAck.errorcode = 0;
            outAck.sceneid = CSPVP_SCENEID;
            outAck.enemyinfo = roleBattle;
            outAck.enemypetinfo = petBattle;
            outAck.enemyRetinueinfo = retinueBPInfo;
            self->addCsPvpBuf();
            outAck.selfMaxhp = self->mRealBattleProp.getMaxHp();
            outAck.selfatk = self->mRealBattleProp.getAtk();
            outAck.selfdef = self->mRealBattleProp.getDef();
            outAck.selfhit = self->mRealBattleProp.getHit();
            outAck.selfdodge = self->mRealBattleProp.getDodge();
            outAck.selfcri = self->mRealBattleProp.getCri();
            
            self->setEnemyVerifyInfo(target, petBattleForce, petBattle.petid);
            self->setBechallengeBattleForce(target->getBattleForce());
            self->getCsPvpState()->beginBattle();
            delete target;
        } while (0);
        
        self->send(&outAck);
        if (outAck.errorcode) {
            req_cs_cancel_cspvp req;
            req.server_id = ack.src_server_id;
            req.role_id = ack.src_role_id;
            CenterClient::instance()->sendNetPacket(0, &req);
        }

    } else {
        self->getCsPvpState()->getTargetBattle();
        req_cs_get_role_battle req;
        req.src_role_id = ack.src_role_id;
        req.src_server_id = ack.src_server_id;
        req.target_server_id = ack.target_server_id;
        req.target_role_id = ack.target_role_id;
        req.custom = "cspvp";
        CenterClient::instance()->sendNetPacket(0, &req);
    }
}}

handler_msg(ack_cs_cspvp_result, ack)
{
    int roleId = ack.src_role_id;
    Role* self = SRoleMgr.GetRole(roleId);
    if (self == NULL) {
        return;
    }

    self->addPvpPoints(ack.score, "跨服竞技场");

    int cooltime = 0;
    if (self->getVipLvl() < 1) {
        cooltime = PvpCfg::getCoolDownTime(0);
        self->getCsPvpStateData().setBattleColddown(Game::tick + cooltime);
    }
    
    ack_cspvp_result outAck;
    outAck.errorcode = ack.errorcode;
    outAck.cooltime = cooltime;
    outAck.isWin = ack.iswin;
    outAck.score = ack.score;
    outAck.rank = ack.rank;
    self->send(&outAck);

    self->getCsPvpState()->endPvp();
    self->setBechallengeId(0);
    self->backToCity();
    self->clearEnemyVerifyInfo();
    self->save();
}}



handler_msg(notify_cs_cspvp_rolerank_award, notify)
{
    log_info(notify.tostring());
    string sender = "system";
    string roleName = getRoleNameByRoleId(notify.role_id);
    string title = "cspvp rolerank award";
    string content = "content";
    string att = notify.award;
    if (roleName.empty()) {
        return;
    }
    MailFormat *f = MailCfg::getCfg("CsPvpRoleRankAward");
    if (f) {
        vector<string> award = StrSpilt(att, ";");
        ItemArray items;
        RewardStruct reward;
        rewardsCmds2ItemArray(award, items, reward);
        
        vector<string> awardDescArr;
        reward2MailFormat(reward, items, &awardDescArr, NULL);
        string awardDesc = StrJoin(awardDescArr.begin(), awardDescArr.end(), ",");
        
        content = f->content;
        find_and_replace(content, 2, Utils::itoa(notify.rank).c_str(), awardDesc.c_str());
        title = f->title;
        sender = f->sendername;
    }
    if (!sendMail(0, sender.c_str(), roleName.c_str(), title.c_str(), content.c_str(), att.c_str(), "")) {
        StoreOfflineItem(notify.role_id, notify.award.c_str());
    }
}}


handler_msg(notify_cs_cspvp_serverrank_award, notify)
{
    log_info(notify.tostring());

    int weektime = notify.week_time;
    string award = notify.award;
    vector<int> members = g_WorshipSystem.getWorshipMembers(weektime);
    
    string sender = "system";
    string title = "cspvp server rank";
    string content = "content";
    string att = notify.award;
    
    MailFormat *f = MailCfg::getCfg("CsPvpServerRankAward");
    if (f) {
        vector<string> award = StrSpilt(att, ";");
        ItemArray items;
        RewardStruct reward;
        rewardsCmds2ItemArray(award, items, reward);
        
        vector<string> awardDescArr;
        reward2MailFormat(reward, items, &awardDescArr, NULL);
        string awardDesc = StrJoin(awardDescArr.begin(), awardDescArr.end(), ",");
        
        content = f->content;
        find_and_replace(content, 2, Utils::itoa(notify.rank).c_str(), awardDesc.c_str());
        title = f->title;
        sender = f->sendername;
    }
    
    
    for (int i = 0; i < members.size(); i++) {
        
        string roleName = getRoleNameByRoleId(members[i]);
        if (roleName.empty()) {
            continue;
        }
        sendMail(0, sender.c_str(), roleName.c_str(), title.c_str(), content.c_str(), att.c_str(), "");
     }
}}



