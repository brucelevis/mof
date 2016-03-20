//
//  WorldSceneTreasureFight.cpp
//  GameSrv
//
//  Created by xinyou on 14-5-12.
//
//

#include "WorldSceneTreasureFight.h"

#include "WorldPlayer.h"
#include "world_cmdmsg.h"
#include "cmd_treasurefight_handler.h"
#include "main.h"
#include "msg.h"
#include "WorldScript.h"
#include "WorldCreatureAI.h"
#include "WorldMonster.h"

extern obj_pos_info getSceneObjectPosInfo(SceneObject* obj);
extern obj_worldrole_info getRoleInfo(WorldRole* role);
extern obj_worldpet_info getPetInfo(WorldPet* pet);
extern obj_worldnpc_info getWorldNpcInfo(WorldMonster* monster);
extern void sendTreasureFightRank(WorldSceneTreasureFight* fightScene);


int WorldSceneTreasureFight::produceScoreByMonster(WorldMonster* monster, int def)
{
    return def;
}

int WorldSceneTreasureFight::produceScoreByRole(WorldRole* role, int def)
{
    int score = def;
    
    try {
        int lvl = role->getLvl();
        CMsgTyped msg;
        msg.SetInt(lvl);
        if (WorldScript::instance()->call("treasurefight_produceScoreByRole", msg) != 1) {
            score = msg.FloatVal();
        }
    } catch (...) {
        
    }
    
    return score;
}

float WorldSceneTreasureFight::getRoleReduce(WorldRole* role, WorldRole* victim, float def)
{
    float reduce = def;
    
    try {
        int victimlvl = victim->getLvl();
        int killerlvl = role->getLvl();
        
        int victimbat = victim->mRoleInfo.mBattle;
        int killerbat = role->mRoleInfo.mBattle;
        
        CMsgTyped msg;
        msg.SetInt(victimlvl);
        msg.SetInt(killerlvl);
        msg.SetInt(victimbat);
        msg.SetInt(killerbat);
        if (WorldScript::instance()->call("treasurefight_getKillRoleReduce", msg) != 1) {
            reduce = msg.FloatVal();
        }
    } catch (...) {
        
    }
    
    return reduce;
}


WorldSceneTreasureFight::WorldSceneTreasureFight() : WorldScene()
{
    mMonsterMod = 0;
    mMonsterNum = 0;
    mMonsterScore = 0;
    mPlayerScoreRate = 1.0f;
    mPlayerAssistRate = 0.2f;
    mMonsterAssistRate = 0.2f;
}

WorldSceneTreasureFight::~WorldSceneTreasureFight()
{
    
}

void WorldSceneTreasureFight::onInit()
{
    int fightCfg = Utils::safe_atoi(mExtendProp.c_str());
    
    TreasureCopyListCfgDef* def = TreasureListCfg::getTreasureCopyCfg(fightCfg);
    if (def != NULL) {
        mPlayerScoreRate = def->playerRate;
        mPlayerAssistRate = def->playerAssistRate;
        mMonsterAssistRate = def->monsterAssistRate;
        mMonsterMod = def->monsterMod;
        mMonsterNum = def->monsterNum;
        mMonsterScore = def->monsterScore;
    }
    
    initMonsters();
    mWaitEndTime = 30000;
    
    setActiveArea(0);
    
    schedule(schedule_selector(WorldSceneTreasureFight::refreshMonsters), 10000);
}

void WorldSceneTreasureFight::onDeinit()
{
    for (int i = 0; i < mMonsters.size(); i++) {
        mMonsters[i]->release();
    }
}

void WorldSceneTreasureFight::onAddMonster(WorldMonster *monster)
{
    CCRect rect = getArea(0)->getBoundingBox();
    CCPoint point = getRandPos(0);
    check_min(point.y, rect.getMinY() + monster->getBoundingBox().size.height / 2);
    check_max(point.y, rect.getMaxY() - monster->getBoundingBox().size.height / 2);
    monster->setPosition(point);
    monster->setIsRecordDamager(true);
    monster->setGroup(eCreatureMonsterGroup);
}

void WorldSceneTreasureFight::onAddedPlayer(WorldPlayer* player)
{
    int guildId = 0;
    int score = 0;
    
    Json::Value value;
    Json::Reader reader;
    reader.parse(player->mExtendProp, value);
    if (value.isObject())
    {
        guildId = value["guild_id"].asInt();
        score = value["score"].asInt();
    }
    
    TreasureFightPlayer* fightPlayer = new TreasureFightPlayer;
    fightPlayer->init(player);
    fightPlayer->mGuildId = guildId;
    fightPlayer->mScore = score;
    mFightPlayers[player->getPlayerId()] = fightPlayer;
    
    uint64_t protectTime = GuildTreasureFightCfg::mCfg.protectTime;
    //float roleHpRate = GuildTreasureFightCfg::mCfg.roleHpRate;
    
    WorldPet* pet = player->getFirstPet();
    WorldRole* role = player->getRole();
    WorldRetinue *retinue = player -> getRetinue();
    
    if (role) {
        role->setGroup(guildId);
        role->setIsRecordDamager(true);
        
        //int maxHp = role->getMaxHp() * roleHpRate;
        //role->setMaxHp(maxHp);
        role->setHp(role->getMaxHp());
        
        role->enterProtectMode(0);
        role->scheduleOnce(schedule_selector(WorldCreature::leaveProtectMode), protectTime);
    }
    
    if (pet) {
        pet->setGroup(role->getGroup());
        pet->setPosition(role->getPosition());
    }
    
    if(retinue){
        retinue->setGroup(role->getGroup());
        retinue->setPosition(role->getPosition());
    }
    
}

void WorldSceneTreasureFight::onCreatureDead(WorldCreature* victim, WorldCreature* killer)
{
    int type = victim->getType();
    switch (type) {
        case eWorldObjectMonster:
        {
            WorldMonster* monster = dynamic_cast<WorldMonster*>(victim);
            if (monster) {
                onMonsterKilled(monster, killer);
            }
            break;
        }
                
        case eWorldObjectPet:
            //do nothing
            break;
                
        case eWorldObjectRole:
        {
            WorldRole* role = dynamic_cast<WorldRole*>(victim);
            if (role) {
                onRoleDead(role, killer);
            }
            break;
        }
        default:
            break;
    }
}

void WorldSceneTreasureFight::removeTreasureFightPlayer(int playerId)
{
    map<int, TreasureFightPlayer*>::iterator iter = mFightPlayers.find(playerId);
    if (iter != mFightPlayers.end()) {
        TreasureFightPlayer* fightPlayer = iter->second;
        mFightPlayers.erase(iter);
        delete fightPlayer;
    }
}

TreasureFightPlayer* WorldSceneTreasureFight::getTreasureFightPlayer(int playerId)
{
    map<int, TreasureFightPlayer*>::iterator iter = mFightPlayers.find(playerId);
    if (iter == mFightPlayers.end()) {
        return NULL;
    }
    
    return iter->second;
}

void WorldSceneTreasureFight::beforePlayerLeave(WorldPlayer* player)
{
    if (getValid() && !mWaitingEnd && !player->mRole->isDead()) {
        
        notify_sync_treasurefight_player_leave notify;
        notify.instid = player->mRole->getInstId();
        
        create_cmd(TreasureFightPlayerLeave, cmd);
        
        int baseScore = produceScoreByRole(player->mRole, player->mRole->getLvl());
        int playerCount = mFightPlayers.size();
        baseScore /= playerCount;
        
        map<int, TreasureFightPlayer*>::iterator iter;
        for (iter = mFightPlayers.begin(); iter != mFightPlayers.end(); iter++) {
            TreasureFightPlayer* otherPlayer = iter->second;
            if (otherPlayer->mPlayer->mRole->getGroup() == player->mRole->getGroup()) {
                continue;
            }
            
            float reduce = getRoleReduce(otherPlayer->mPlayer->mRole, player->mRole, 1.0f);
            int score = baseScore * reduce;
            otherPlayer->mScore += score;
            cmd->scores.push_back(score);
            cmd->ids.push_back(otherPlayer->mPlayer->mRoleId);
            
            obj_treasurecopy_getscore data;
            data.instid = otherPlayer->mPlayer->mRole->getInstId();
            data.score = score;
            notify.datas.push_back(data);
        }
        broadcastPacket(&notify);
        sendMessageToGame(0, CMD_MSG, cmd, 0);
        sendTreasureFightRank(this);
    }
    
    removeTreasureFightPlayer(player->getPlayerId());
    
}

void WorldSceneTreasureFight::getKillAssists(WorldCreature* victim, WorldRole* killer, set<TreasureFightPlayer*>& players)
{
    for (map<int, TreasureFightPlayer*>::iterator iter = mFightPlayers.begin();
         iter != mFightPlayers.end(); iter++) {
        
        TreasureFightPlayer* other = iter->second;
        
        int group = other->mPlayer->getRole()->getGroup();
        if (group == victim->getGroup() || other->mPlayer->mRole == killer) {
            continue;
        }
        
        bool isAssistance = false;
        int roleInstId = other->mPlayer->getRole()->getInstId();
        if (victim->isDamager(roleInstId)) {
            isAssistance = true;;
        } else if (other->mPlayer->getFirstPet()) {
            int petInstId = other->mPlayer->getFirstPet()->getInstId();
            if (victim->isDamager(petInstId)) {
                isAssistance = true;
            }
        }
        if (isAssistance) {
            players.insert(other);
        }
    }

}


void WorldSceneTreasureFight::onRoleDead(WorldRole *role, WorldCreature *killer)
{
    if (killer->getType() != eWorldObjectPet && killer->getType() != eWorldObjectRole) {
        return;
    }
    
    WorldPlayer* attacker = getPlayerByCreature(killer);
    WorldPlayer* victim = role->mPlayer;
    WorldPet* pet = victim->getFirstPet();
    if (pet && !pet->isDead()) {
        pet->death(NULL);
    }
    
    TreasureFightPlayer* fightAttacker = getTreasureFightPlayer(attacker->getPlayerId());
    
    int baseScore = produceScoreByRole(role, role->getLvl());
    int baseAssistScore = baseScore * mPlayerAssistRate;
    
    float reduce = getRoleReduce(attacker->mRole, role, 1.0f);
    int score = baseScore * reduce * mPlayerScoreRate;
    fightAttacker->mScore += score;
    
    notify_sync_treasurefight_kill_player notify;
    create_cmd(TreasureFightKillPlayer, cmd);
    //击杀
    cmd->victimid = victim->mRoleId;
    cmd->killerid = attacker->mRoleId;
    cmd->killerscore = score;
    notify.victim = role->getInstId();
    notify.killerid = attacker->mRole->getInstId();
    notify.score = score;
    //助攻
    set<TreasureFightPlayer*> assists;
    getKillAssists(role, attacker->mRole, assists);
    for (set<TreasureFightPlayer*>::iterator iter = assists.begin(); iter != assists.end(); iter++) {
        TreasureFightPlayer* fightPlayer = *iter;
        reduce = getRoleReduce(fightPlayer->mPlayer->getRole(), role, 1.0f);
        int assistScore = baseAssistScore * reduce;
        
        fightPlayer->mScore += assistScore;
        
        cmd->assistscores.push_back(assistScore);
        cmd->assistids.push_back(fightPlayer->mPlayer->mRoleId);
        notify.assistscore.push_back(assistScore);
        notify.assists.push_back(fightPlayer->mPlayer->getRole()->getInstId());
    }
    sendMessageToGame(0, CMD_MSG, cmd, 0);
    broadcastPacket(&notify);
    
    //发送积分榜
    sendTreasureFightRank(this);
}

void WorldSceneTreasureFight::onMonsterKilled(WorldMonster* monster, WorldCreature* killer)
{
    monster->setValid(false);
    
    int score = mMonsterScore;
    int assistScore = score * mMonsterAssistRate;
    
    WorldPlayer* attacker = getPlayerByCreature(killer);
    if(attacker){
        TreasureFightPlayer* fightAttacker = getTreasureFightPlayer(attacker->getPlayerId());
        fightAttacker->mScore += score;
        
        notify_sync_treasurefight_kill_npc notify;
        create_cmd(TreasureFightKillNpc, cmd);
        //击杀
        cmd->killerid = attacker->mRoleId;
        cmd->killerscore = score;
        notify.killerid = attacker->mRole->getInstId();
        notify.score = score;
        //助攻
        set<TreasureFightPlayer*> assists;
        getKillAssists(monster, attacker->mRole, assists);
        for (set<TreasureFightPlayer*>::iterator iter = assists.begin(); iter != assists.end(); iter++) {
            TreasureFightPlayer* fightPlayer = *iter;
            fightPlayer->mScore += assistScore;
            cmd->assistscores.push_back(assistScore);
            cmd->assistids.push_back(fightPlayer->mPlayer->mRoleId);
            notify.assistscore.push_back(assistScore);
            notify.assists.push_back(fightPlayer->mPlayer->getRole()->getInstId());
        }
    
        sendMessageToGame(0, CMD_MSG, cmd, 0);
        broadcastPacket(&notify);
        
        sendTreasureFightRank(this);
    }
}

void WorldSceneTreasureFight::initMonsters()
{
    for (int i = 0; i < mMonsterNum; i++) {
        WorldMonster* monster = WorldMonster::create(mMonsterMod);
        if (monster != NULL) {
            addMonster(monster);
            mMonsters.push_back(monster);
        }
    }
}

void WorldSceneTreasureFight::refreshMonsters(uint64_t ms)
{
    notify_worldnpc_enter_scene notify;
    for (int i = 0; i < mMonsters.size(); i++){
        WorldMonster* monster = mMonsters[i];
        if (monster->getHp() > 0) {
            continue;
        }
        monster->restoreLife(NULL);
        addMonster(monster);
        //monster->setIsUnhurtState(true);
        
        //通知进入场景
        notify.posinfos.push_back(getSceneObjectPosInfo(monster));
        notify.npcinfos.push_back(getWorldNpcInfo(monster));
    }
    broadcastPacket(&notify);
}


void WorldSceneTreasureFight::traverseFightPlayer(TraverseCallback cb, void *param)
{
    map<int, TreasureFightPlayer*>::iterator iter = mFightPlayers.begin();
    for (; iter != mFightPlayers.end(); iter++){
        TreasureFightPlayer* fightPlayer = iter->second;
        if (!cb(fightPlayer, param)) {
            break;
        }
    }
}
