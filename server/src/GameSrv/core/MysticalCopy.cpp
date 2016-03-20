//
//  MysticalCopy.cpp
//  GameSrv
//
//  Created by pireszhi on 13-9-4.
//
//

#include "MysticalCopy.h"
#include "EnumDef.h"
//#include "hander_include.h"
#include "mail_imp.h"
#include "DBRolePropName.h"
#include "GameLog.h"
#include "Activity.h"
#include "Role.h"

extern int sendNormalCopyAward(Role* role,
                               SceneCfgDef* scenecfg,
                               int flopid,
                               int friendId,
                               int& fcindex,
                               vector<int>& drops,
                               RewardStruct& notifyReward,
                               string &itemsResult);

extern void sendNotifyBagFullAndSendMail(Role* role);

bool MysticalCopy::IsOpen()
{
    return SActMgr.IsActive(ae_mystical_copy); 
}

void MysticalCopy::setCopyId(int copyid)
{
    m_Copyid = copyid;
}

void MysticalCopy::OnHeartBeat() // 场景心跳
{
    
}
void MysticalCopy::OnHeartBeat(Obj* obj, OBJ_INFO& info) // 对象心跳
{
    if (!getObjCount()) {
        Destroy();
    }
}
void MysticalCopy::OnEnterRoom(Obj* obj) // 有对象加入场景
{
    
}
void MysticalCopy::OnLeaveRoom(Obj* obj) // 对象离开
{
    SceneCfgDef* scenecfg = SceneCfg::getCfg(m_Copyid);
    
    if (scenecfg && obj->getType() == eRole) {
        
        Role* role = (Role*)obj;
        //role->setCurrSceneId(role->getSceneID());
        role->backToCity();
        
        ack_leave_mysticalcopy ack;
        MysticalResult result;
        if(!getBattleResult(role->getInstID(), result))
        {
            printf("can not find Role result -- mysticalcopy\n");
            return;
        }
        
        do
        {
            if (result.isPass || result.m_killMonsters.size()) {

                stringstream szLogIncrTimesChange;
                stringstream szLogcopyTimesChange;
                
                bool bLogbossAppear = true;
                bool bLogmonsterAppear = true;
                
                //改变前
                szLogIncrTimesChange << role->getIncrMysticalCopyBOSSandGoblinTimes() << ";";
                szLogcopyTimesChange << role->getMysticalCopyTimes() << ";";

                if (this->getBoss().m_Monsterid == 0) bLogbossAppear = false;
                if (this->getMonsters().size() == 0) bLogmonsterAppear = false;
                
                if (role->getIncrMysticalCopyBOSSandGoblinTimes() > 0 )
                {
                    role->setIncrMysticalCopyBOSSandGoblinTimes(role->getIncrMysticalCopyBOSSandGoblinTimes() - 1);
                    role->saveNewProperty();
                }
                else
                {
                    role->setMysticalCopyTimes(role->getMysticalCopyTimes() + 1);
                    role->saveProp(GetRolePropName(eRoleMysterCopyTimes), role->getMysticalCopyTimes());
                }
                
                {
                    string action = Utils::makeStr("passcopy_%d", scenecfg->sceneId);
                    role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesMysticalCopyBOSSandGoblin, action.c_str());
                }
                
                //改变后
                szLogIncrTimesChange << role->getIncrMysticalCopyBOSSandGoblinTimes();
                szLogcopyTimesChange << role->getMysticalCopyTimes();
                
                LogMod::addLogMysticalCopyTimesAndIncrTimes(role, szLogIncrTimesChange.str(), szLogcopyTimesChange.str(), bLogbossAppear, bLogmonsterAppear);
                
            }
            
            //通关扣相应的精力，否则不扣，并发放普通通关奖励
            if (result.isPass) {
                
                role->addFat(-scenecfg->energy_consumption);
                
                int fcindex = 0;
                vector<int> drops;
                RewardStruct notifyReward;
				string awardItems;
                
                ack.errorcode = sendNormalCopyAward(role, scenecfg, m_Copyid, 0,
													fcindex, drops, notifyReward, awardItems);
                
                ack.award.findex = fcindex;
                ack.award.flopid = m_Copyid;
                ack.award.dropindices = drops;
                
                ack.award.gold = notifyReward.reward_gold;
                ack.award.battlepoint = notifyReward.reward_batpoint;
                ack.award.exp = notifyReward.reward_exp;
                ack.award.constell = notifyReward.reward_consval;
                ack.award.petexp = notifyReward.reward_petexp;
				ack.award.awardItems = awardItems;
                ack.award.enchantdust = notifyReward.reward_enchantdust;
                ack.award.enchantgold = notifyReward.reward_enchantgold;
                
                LogMod::addLogPetCampPassRecord(role->getInstID(), m_Copyid, result.battleTime, result.bossid);
                
                if (ack.errorcode) {
                    break;
                }
            }
            
            MysticalCopyDef* mysticalcopy = MysticalCopyCfg::getcfg(m_Copyid);
            
            ItemArray items;
            if (mysticalcopy) {
                for (int i = 0; i < result.m_killMonsters.size(); i++) {
                    
                    MysticalMonsterDef monster = mysticalcopy->getMonsterCfg(result.m_killMonsters[i]);
                    
                    //没通关但杀死了隐藏怪物，按杀死的怪物扣精力
                    if (!result.isPass && monster.m_Monsterid) {
                        role->addFat( -monster.energy_consum);
                    }
                    
                    string award = mysticalcopy->getMonsterKillAward();
                    vector<string> awards = StrSpilt(award, ";");
                    string comefrom = Utils::makeStr("Mysticalcopy Monster_Reward");
                    if(!role->addAwards(awards, items, comefrom.c_str()))
                    {
//                        ack.errorcode = CE_BAG_FULL_AND_SEND_MAIL;
                        
                        //背包满发邮件附件
                        sendMailWithName(0, "", role->getRolename().c_str(), "bag_full", award.c_str());
                        
                        sendNotifyBagFullAndSendMail(role);
                        
//                        sendNetPacket(role->getSessionId(), &ack);
//                        return;
                    }
                    ack.monsterKillAward.push_back(award);
                }
                
                
                
                if(result.bossid)
                {
                    string bossaward = mysticalcopy->bossAward;
                    vector<string> awards = StrSpilt(bossaward, ";");
                    string comefrom = Utils::makeStr("Mysticalcopy Boss_Reward");
                    if(!role->addAwards(awards, items,comefrom.c_str()))
                    {
//                        ack.errorcode = CE_BAG_FULL_AND_SEND_MAIL;
                        sendMailWithName(0, "", role->getRolename().c_str(), "bag_full", bossaward.c_str());
                        
                        sendNotifyBagFullAndSendMail(role);
                        
                        break;
                    }
                    
                    ack.bossAward = bossaward;
                }
            }
        }while(false);
        
        sendNetPacket(role->getSessionId(), &ack);
    }
}

void MysticalCopy::CreateMonsters(int copyid, int times)
{
    MysticalCopyCfg::getMysticalMonsters(copyid, times, m_Monsters, m_Boss);
}

//百分百出现隐藏 BOSS 与 哥布林
void MysticalCopy::CreateMonsters(int copyid)
{
    MysticalCopyCfg::getMysticalMonsters(copyid, m_Monsters, m_Boss);
}

const std::vector<MysticalMonsterDef>& MysticalCopy::getMonsters()
{
    return m_Monsters;
}

const MysticalMonsterDef& MysticalCopy::getBoss()
{
    return m_Boss;
}

bool MysticalCopy::getBattleResult(int roleid, MysticalResult& outdata)
{
    std::map<int, MysticalResult>::iterator iter = m_RolesResult.find(roleid);
    if (iter == m_RolesResult.end()) {
        return false;
    }
    
    outdata = iter->second;
    return true;
}

bool MysticalCopy::setBattleResult(int roleid, MysticalResult indata)
{
    std::map<int, MysticalResult>::iterator iter = m_RolesResult.find(roleid);
    if (iter != m_RolesResult.end()) {
        return false;
    }
    
    m_RolesResult.insert(make_pair(roleid, indata));
    return true;
}

bool MysticalCopy::setBattleKillBoss(int roleid, int bossid)
{
    std::map<int, MysticalResult>::iterator iter = m_RolesResult.find(roleid);
    if (iter != m_RolesResult.end()) {
        iter->second.bossid = bossid;
        return true;
    }
    
    return false;
}

bool MysticalCopy::checkBossExist(int bossid)
{
    if (!bossid) {
        return true;
    }
    
    return bossid == m_Boss.m_Monsterid ? true : false;
}

bool MysticalCopy::checkMonsterExist(int monsterid)
{
    if (!monsterid) {
        return true;
    }
    
    int vsize = m_Monsters.size();
    
    for (int i = 0; i < vsize; i++) {
        MysticalMonsterDef &def = m_Monsters[i];
        
        if (def.m_Monsterid == monsterid) {
            return true;
        }
    }
    
    return false;
}