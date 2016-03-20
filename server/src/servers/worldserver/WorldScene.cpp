//
//  WorldScene.cpp
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#include "WorldScene.h"
#include "WorldRole.h"
#include "main.h"
#include "msg.h"
#include "basedef.h"
#include "msg.h"
#include "WorldRobot.h"
#include "WorldBase.h"
#include "WorldScheduler.h"
#include "WorldObject.h"
#include "WorldMonster.h"
#include "scene/WorldScenePvp.h"
#include "scene/WorldSceneTeamCopy.h"
#include "scene/WorldSceneTreasureFight.h"
#include "scene/WorldSceneScript.h"
#include "WorldPlayer.h"
#include "WorldRole.h"
#include "WorldRetinue.h"
#include "world_cmdmsg.h"
#include "WorldCreatureAI.h"

#include "DataCfg.h"

extern obj_pos_info getSceneObjectPosInfo(SceneObject* obj);
extern obj_worldrole_info getRoleInfo(WorldRole* role);
extern obj_worldpet_info getPetInfo(WorldPet* pet);
extern obj_worldnpc_info getWorldNpcInfo(WorldMonster* monster);
extern obj_worldretinue_info  getRetinueInfo(WorldRetinue* retinue);

SceneArea* SceneArea::create(const SceneAreaDef& def)
{
    SceneArea* area = new SceneArea;
    
    vector<CCPoint>& landPoses = area->getLandPoses();
    for (int i = 0; i < def.mLandPoints.size(); i++)
    {
        CCPoint pos;
        pos.x = def.mLandPoints[i].x;
        pos.y = def.mLandPoints[i].y;
        landPoses.push_back(pos);
    }
    
    area->setBoundingBox(CCRect(def.mRect.x, def.mRect.y, def.mRect.width, def.mRect.height));
    
    area->mAreaDef = def;
    
    return area;
}


vector<CCPoint>& SceneArea::getLandPoses()
{
    return mLandPoses;
}


CCPoint SceneArea::getLandPos(int index)
{
    if (index > 0 && index <= mLandPoses.size())
    {
        return mLandPoses[index - 1];
    }
    
    int width = mBoundingBox.size.width;
    int height = mBoundingBox.size.height;
    
    int x = ::rand() % width + mBoundingBox.getMinX();
    int y = ::rand() % height + mBoundingBox.getMinY();
    return CCPoint(x, y);
}


extern LuaScriptManager g_LuaScriptMgr;
WorldScene* WorldScene::create(const SceneCfgDef* def, const char* extendProp)
{
    WorldScene* scene = NULL;
    switch (def->sceneType) {
        case stSyncPvp:
            scene = new WorldScenePvp;
            break;
        case stTreasureFight:
            scene = new WorldSceneTreasureFight;
            break;
        case stSyncTeamCopy:
            scene = new WorldSceneTeamCopy;
            break;
        
        // added by jianghan for 脚本场景
        case stScriptScene:
        {
            //scene = new LuaScene(def->scriptName);
            // 临时测试
            string lang = Process::env.getString("lang");
            string luadir = Process::execpath + "/../res/" + lang + "/luaScene/?.lua";
            string luafile = Process::execpath + "/../res/" + lang + "/luaScene/interface.lua";
            
            //string filename = "main.lua";
            lua_State * context = g_LuaScriptMgr.getStateContext(luadir, luafile);
            if( context != NULL )
                scene = new LuaScene(context);
            break;
        }
        // end
        
        default:
            scene = new WorldScene;
            break;
    }
    
    CheckCondition(scene, return NULL);
    if (!scene->init(def, extendProp))
    {
        delete scene;
        scene = NULL;
    }
    return scene;
}

void WorldScene::destroy(WorldScene *scene)
{
    scene->deinit();
    delete scene;
}

WorldScene::WorldScene() : WorldObject(eWorldObjectScene)
{
    mValid = true;
    mActiveArea = -1;
    mStateTime = 0;
    mWaitEndTime = 15000;
    mWaitingEnd = false;
    mInitialized = false;
}

WorldScene::~WorldScene()
{
    // added by jianghan for 资源不要忘记了释放
    //for (SceneArea * area : mAreas) {
    //    delete area;
    //}
    
    for (int i = 0; i < mAreas.size(); i++) {
        SceneArea* area = mAreas[i];
        delete area;
    }
    
    // end add
}


bool WorldScene::init(const SceneCfgDef* def, const char* extendProp)
{
    mCfg = *def;
    
    for (vector<SceneAreaDef>::iterator iter = mCfg.mAreas.begin();
         iter != mCfg.mAreas.end(); iter++) {
        SceneArea* area = SceneArea::create(*iter);
        mAreas.push_back(area);
    }
    
    mRunTime = 0;
    mBeat = 0;
    mValid = true;
    mExtendProp = extendProp;
    
    onInit();
    
    // added by jianghan for 定时发送各场景的伤害消息
    mDmgNotify = NULL;
    mBeSkilledNotify = NULL;
    mBeAtkedNotify = NULL;
    schedule(schedule_selector(WorldScene::doDamageBroadcast), 100);
    // end add
    
    mInitialized = true;
    return true;
}

void WorldScene::run(uint64_t ms)
{
    mStateTime += ms;
    
    for (int i = 0; i < mEvents.size(); i++) {
        onEvent(mEvents[i].eventId, mEvents[i].lParam, mEvents[i].rParam);
    }
    mEvents.clear();
    
    if (mWaitingEnd) {
        if (mStateTime > mWaitEndTime) {
            setValid(false);
        }
    } else {
        update(ms);
    }
}

void WorldScene::end()
{
    setValid(false);
}

void WorldScene::deinit()
{
    onDeinit();
    clearAll();
}


void WorldScene::pendEnd()
{
    if (mWaitingEnd) {
        return;
    }
    
    mWaitingEnd = true;
    mStateTime = 0;
}


void WorldScene::activeArea(int areaIdx)
{
    assert(areaIdx < mAreas.size());
    
    if (areaIdx == getActiveArea()) {
        return;
    }
    
    setActiveArea(areaIdx);
    
    notify_worldnpc_enter_scene notify;
    
    SceneArea* area = getArea(areaIdx);
    vector<SceneNPCDef>& monsters = area->mAreaDef.mMonsters;
    for (int i = 0; i < monsters.size(); i++) {
        WorldMonster* monster = createMonster(monsters[i].npcId, monsters[i].x, monsters[i].y);
        monster->setAreaId(areaIdx);
        notify.posinfos.push_back(getSceneObjectPosInfo(monster));
        notify.npcinfos.push_back(getWorldNpcInfo(monster));
    }
    
    broadcastPacket(&notify);
}


void WorldScene::clearAll()
{
    //先清除creature，再清除skillaction
    list<WorldPlayer*>::iterator piter = mPlayers.begin();
    list<WorldPlayer*>::iterator piterEnd = mPlayers.end();
    while (piter != piterEnd)
    {
        WorldPlayer* player = *piter;
        piter++;
        
        removePlayer(player);
        
        create_cmd(LeaveWorldSceneRet, leaveWorldSceneRet);
        leaveWorldSceneRet->ret = 0;
        leaveWorldSceneRet->sessionId = player->getSessionId();
        leaveWorldSceneRet->roleId = player->getRole()->mRoleInfo.mRoleId;
        leaveWorldSceneRet->reason = eLeaveWorldSceneDestroyScene;
        leaveWorldSceneRet->sceneId = getInstId();
        leaveWorldSceneRet->sceneMod = mCfg.sceneId;
        leaveWorldSceneRet->extendStr = mExtendProp;
        sendMessageToGame(0, CMD_MSG, leaveWorldSceneRet, 0);
        
        g_WorldPlayerMgr.destroyPlayer(player);
    }
    
    list<WorldObject*>::iterator iter = mObjects.begin();
    list<WorldObject*>::iterator iterEnd = mObjects.end();
    while (iter != iterEnd)
    {
        WorldObject* obj = *iter;
        iter++;
        
        obj->release();
    }
    
    // added by jianghan for 定时发送各场景的伤害消息
    if( mDmgNotify != NULL )
        delete (notify_sync_damage *)mDmgNotify;
    if ( mBeSkilledNotify != NULL )
        delete (notify_sync_beskill *)mBeSkilledNotify;
    if ( mBeAtkedNotify != NULL )
        delete (notify_sync_beatk *)mBeAtkedNotify;
    
    unschedule(schedule_selector(WorldScene::doDamageBroadcast));
    // end add
    
}

// added by jianghan for 定时发送各场景的伤害消息
void WorldScene::addSceneDamage(void * pAtkData){
    obj_attackedTarget * pAtk = (obj_attackedTarget * )pAtkData;
    if( mDmgNotify == NULL )
        mDmgNotify = new notify_sync_damage;
    
    ((notify_sync_damage *)mDmgNotify)->attacked.push_back(*pAtk);

    if( ((notify_sync_damage *)mDmgNotify)->attacked.size()>=32 ){
        broadcastPacket((notify_sync_damage *)mDmgNotify);
        ((notify_sync_damage *)mDmgNotify)->attacked.clear();
    }
}

void WorldScene::addSceneBeSkilled(void * pBeSkilledData){
    obj_beSkilled * pBeSkilled = (obj_beSkilled * )pBeSkilledData;
    if( mBeSkilledNotify == NULL )
        mBeSkilledNotify = new notify_sync_beskill;

    ((notify_sync_beskill *)mBeSkilledNotify)->beSkilled.push_back(*pBeSkilled);
    
    if( ((notify_sync_beskill *)mBeSkilledNotify)->beSkilled.size()>=32 ){
        broadcastPacket((notify_sync_beskill *)mBeSkilledNotify);
        ((notify_sync_beskill *)mBeSkilledNotify)->beSkilled.clear();
    }
}

void WorldScene::addSceneBeAtked(void * pBeAtkedData){
    obj_beAttacked * pBeAtked = (obj_beAttacked * )pBeAtkedData;
    if( mBeAtkedNotify == NULL )
        mBeAtkedNotify = new notify_sync_beatk;

    ((notify_sync_beatk *)mBeAtkedNotify)->beAttacked.push_back(*pBeAtked);
    
    if( ((notify_sync_beatk *)mBeAtkedNotify)->beAttacked.size()>=32 ){
        broadcastPacket((notify_sync_beatk *)mBeAtkedNotify);
        ((notify_sync_beatk *)mBeAtkedNotify)->beAttacked.clear();
    }
    
}

void WorldScene::doDamageBroadcast(){
    
    if( mBeAtkedNotify != NULL )
    {
        notify_sync_beatk * pBeAtk = (notify_sync_beatk *)mBeAtkedNotify;
        if( pBeAtk->beAttacked.size() > 0 ){
            broadcastPacket(pBeAtk);
            pBeAtk->beAttacked.clear();
        }
    }
    
    if( mBeSkilledNotify != NULL )
    {
        notify_sync_beskill * pBeSkill = (notify_sync_beskill *)mBeSkilledNotify;
        if( pBeSkill->beSkilled.size() > 0 ){
            broadcastPacket(pBeSkill);
            pBeSkill->beSkilled.clear();
        }
    }
    
    if( mDmgNotify != NULL ){
        notify_sync_damage * pDmg = (notify_sync_damage *)mDmgNotify;
        if( pDmg->attacked.size()>0 ){
            broadcastPacket(pDmg);
            pDmg->attacked.clear();
        }
    }
    
}
// end add


void WorldScene::traverseObjects(TraverseCallback callback, void* param)
{
    list<WorldObject*>::iterator iter;
    for (iter = mObjects.begin(); iter != mObjects.end(); ++iter)
    {
        if (!callback(*iter, param))
        {
            break;
        }
    }
}

WorldObject* WorldScene::findObject(int instId)
{
    WorldObject *obj = NULL;
    
    list<WorldObject*>::iterator iter;
    foreach(iter, mObjects)
    {
        obj = *iter;
        if (obj->getInstId() == instId)
        {
            return obj;
        }
    }
    return obj;
}

void WorldScene::addObject(WorldObject* obj)
{
    mObjects.push_back(obj);
}

void WorldScene::removeObject(WorldObject* obj)
{
    list<WorldObject*>::iterator iter;
    for (iter = mObjects.begin(); iter != mObjects.end();)
    {
        if (*iter == obj)
        {
            iter = mObjects.erase(iter);
            obj->release();
        }
        else
        {
            ++iter;
        }
    }
}

bool WorldScene::addPlayer(WorldPlayer* player)
{
    if (!preAddPlayer(player))
    {
        return false;
    }
    
    mPlayers.push_back(player);
    player->enterScene(this);
    
    WorldRetinue *retinue = player -> getRetinue();
    WorldPet* pet = player->getFirstPet();
    WorldRole *role = player -> getRole();
    
    onAddedPlayer(player);
    
    notify_new_roles notify;
   
    if (pet)
    {
        notify.petposinfos.push_back(getSceneObjectPosInfo(pet));
        notify.pets.push_back(getPetInfo(pet));
    }
        if(role){
        notify.roleposinfos.push_back(getSceneObjectPosInfo(role));
        notify.roles.push_back(getRoleInfo(role));
    }
    
    if(retinue){
        notify.retinues.push_back(getRetinueInfo(retinue));
        notify.retiuneposinfors.push_back(getSceneObjectPosInfo(retinue));
    }
    broadcastPacket(&notify);
    
    return true;
}

void WorldScene::removePlayer(WorldPlayer *player)
{
    beforePlayerLeave(player);
    player->leaveScene();
    list<WorldPlayer*>::iterator iter = mPlayers.begin();
    while (iter != mPlayers.end())
    {
        if (*iter == player)
        {
            mPlayers.erase(iter);
            break;
        }
        
        ++iter;
    }
    afterPlayerLeave();
}

void WorldScene::broadcastPacket(INetPacket* packet, bool shouldReady)
{
    vector<int> sessions;
    for (list<WorldPlayer*>::iterator iter = mPlayers.begin(); mPlayers.end() != iter; iter++)
    {
        WorldPlayer* player = *iter;
        if (shouldReady && !player->getReady())
        {
            continue;
        }
        sessions.push_back(player->getSessionId());
    }
    
    multicastPacket(sessions, packet);
}

void WorldScene::removeInvalidObjects()
{
    list<WorldObject*>::iterator iter = mInvalidObjects.begin();
    list<WorldObject*>::iterator iterEnd = mInvalidObjects.end();
    while (iter != iterEnd)
    {
        WorldObject* obj = *iter;
        ++iter;
        
        obj->release();
    }
    mInvalidObjects.clear();

}

void WorldScene::updateObjects(uint64_t ms)
{
    list<WorldObject*>::iterator iter = mObjects.begin();
    list<WorldObject*>::iterator iterEnd = mObjects.end();
    while (iter != iterEnd)
    {
        WorldObject* obj = *iter;
        if (obj->getValid())
        {
            obj->update(ms);
            ++iter;
        }
        else
        {
            iter = mObjects.erase(iter);
            mInvalidObjects.push_back(obj);
        }
    }
}

void WorldScene::onAddMonster(WorldMonster *monster)
{
    CCRect rect = getArea(0)->getBoundingBox();
    CCPoint point = getRandPos(0);
    check_min(point.y, rect.getMinY() + monster->getBoundingBox().size.height / 2);
    check_max(point.y, rect.getMaxY() - monster->getBoundingBox().size.height / 2);
    monster->setPosition(point);
    monster->setGroup(eCreatureMonsterGroup);
}

void WorldScene::addMonster(WorldMonster* monster)
{
    monster->setValid(true);
    monster->enterScene(this);
    monster->retain();
    onAddMonster(monster);
}

void WorldScene::update(uint64_t ms)
{
    CheckCondition(mValid, return);
    
    WorldObject::update(ms);
    
    updateObjects(ms);
       
    removeInvalidObjects();
    //删除update中需要删除玩家
    handleLeavedPlayers();
}

void WorldScene::handleLeavedPlayers()
{
    for (list<WorldPlayer*>::iterator iter = mLeavedPlayers.begin(); iter != mLeavedPlayers.end(); iter++)
    {
        WorldPlayer* player = *iter;
        removePlayer(player);
    }
    mLeavedPlayers.clear();
}

int WorldScene::getSceneModId()
{
    return mCfg.sceneId;
}
int WorldScene::getSceneType()
{
    return mCfg.sceneType;
}

vector<WorldCreature*> WorldScene::getAttackObjsByArea(SceneObject* srcObj, const CCRect& area)
{
    return getAttackObjsByArea(srcObj, srcObj->getType(), area);
}

vector<WorldCreature*> WorldScene::getAttackObjsByArea(SceneObject* srcObj, WorldObjectType srcObjType, const CCRect& area)
{
    vector<WorldCreature*> result;
    WorldCreature* targetObj = NULL;
    list<WorldObject*>::iterator it;
    
    WorldCreature* attacker = NULL;
    if (srcObj->isCreature()) {
        attacker = (WorldCreature*)srcObj;
    } else {
        log_warn("getAttackObjsByArea only support creature now");
        return result;
    }
    
    for (it=mObjects.begin(); it!=mObjects.end(); it++)
    {
        if((*it)->isCreature()) {
            targetObj = dynamic_cast<WorldCreature*>(*it);
        } else {
            continue;
        }
        
        // modified by jianghan for 敌我关系判断需要由脚本判断
        if( this->getSceneType() == stScriptScene ){
            bool bCanAtk = ((LuaScene *)this)->lua_isEnemyTarget(attacker, targetObj);
            if (bCanAtk == false ||
                attacker->getAreaId() != targetObj->getAreaId()) {
                continue;
            }
        }else{
            if (attacker->getGroup() == targetObj->getGroup() ||
                attacker->getAreaId() != targetObj->getAreaId() || targetObj -> getType() == eWorldObjectRetinue) {
                continue;
            }
        }
        // end modify
        
        // modified by jianghan
        //targetObj->doPositionAdjust();
        CCRect batkArea = targetObj->calcBeAtkRect();
        if (area.intersectsRect(batkArea) || (targetObj->getType() != eWorldObjectMonster &&
                                              targetObj->intersectsHistoryPosition(area))) {
            result.push_back(targetObj);
        }
    }
    return result;
}

vector<WorldCreature*> WorldScene::getAreaCanAttackObjs(SceneObject* srcObj, WorldObjectType srcObjType)
{
    vector<WorldCreature*> result;
    WorldCreature* targetObj=NULL;
    list<WorldObject*>::iterator it;
    
    WorldCreature* srcCreature = dynamic_cast<WorldCreature*>(srcObj);
    if (NULL == srcCreature) {
        return result;
    }
    
    for (it=mObjects.begin(); it!=mObjects.end(); it++) {
        if((*it)->isCreature()) {
            targetObj = dynamic_cast<WorldCreature*>(*it);
        } else {
            continue;
        }
        
        // modified by jianghan for 敌我关系判断需要由脚本判断
        if( getSceneType() == stScriptScene )
        {
            bool bCanAtk = ((LuaScene *)this)->lua_isEnemyTarget(srcCreature, targetObj);
            if (bCanAtk &&
                srcCreature->getAreaId() == targetObj->getAreaId()) {
                result.push_back(targetObj);
            }
        }else{
            if (targetObj -> getType() == eWorldObjectRetinue){
                continue;
            }
            
            if (srcCreature->getGroup() != targetObj->getGroup() &&
                srcCreature->getAreaId() == targetObj->getAreaId()) {
                result.push_back(targetObj);
            }
        }
        // end modify
    }
    return result;
}

vector<WorldCreature*> WorldScene::getAreaSelfGroup(WorldCreature* srcObj)
{
    vector<WorldCreature*> result;
    list<WorldObject*>::iterator it;

    // modified by jianghan for 敌我关系需要由脚本判断
    if( this->getSceneType() == stScriptScene ){
        for (it=mObjects.begin(); it!=mObjects.end(); it++) {
            WorldCreature *wc = dynamic_cast<WorldCreature*>(*it);
            bool bCanAtk = ((LuaScene *)this)->lua_isEnemyTarget(srcObj, wc);
            if (wc && wc->getInstId() != srcObj->getInstId() &&
                bCanAtk == false &&
                wc->getAreaId() == srcObj->getAreaId()) {
                result.push_back(wc);
            }
        }
    }else{
        for (it=mObjects.begin(); it!=mObjects.end(); it++) {
            WorldCreature *wc = dynamic_cast<WorldCreature*>(*it);
            if (wc && wc->getInstId() != srcObj->getInstId() &&
                wc->getGroup() == srcObj->getGroup() &&
                wc->getAreaId() == srcObj->getAreaId()) {
                result.push_back(wc);
            }
        }
    }

    return result;
}

vector<WorldCreature*> WorldScene::atkObjs(WorldCreature* obj)
{
    CCRect atkRect = obj->calcAtkRect();
    CCRect beAtkRect;
    vector<WorldCreature*> result;
    WorldCreature* targetObj=NULL;
    list<WorldObject*>::iterator it;
    for (it=mObjects.begin(); it!=mObjects.end(); it++)
    {
        if((*it)->isCreature()) {
            targetObj = dynamic_cast<WorldCreature*>(*it);
        } else {
            continue;
        }
        
        bool bCanAtk = false;
        // modified by jianghan for 对于脚本场景，判断是否是敌人这个逻辑应该交给脚本去做
        if( getSceneType() == stScriptScene )
        {
            bCanAtk = ((LuaScene *)this)->lua_isEnemyTarget(obj, targetObj);
        }
        else if (obj->getGroup() != targetObj->getGroup() &&
            obj->getAreaId() == targetObj->getAreaId()) {
            bCanAtk = true;
        }
        
        if( bCanAtk )
        {
            beAtkRect = targetObj->calcBeAtkRect();
            if(atkRect.intersectsRect(beAtkRect)) {
                result.push_back(targetObj);
            }
        }
        // end modify
    }
    return result;
}

bool WorldScene::outOfArea(WorldObject* obj)//如果出界，矫正回
{
    SceneObject* sobj = dynamic_cast<SceneObject*>(obj);
    if (sobj == NULL)
    {
        return false;
    }
    
    float posx = sobj->getPositionX();
    float posy = sobj->getPositionY();
    
    CCRect sceneArea = getBoundingBox(mActiveArea);
    if (posx > sceneArea.getMinX()) {
        sobj->setAreaId(mActiveArea);
    }
    
    CCRect aabb = getBoundingBox(sobj->getAreaId());
    check_range(posx, aabb.getMinX(), aabb.getMaxX() - 1);
    check_range(posy, aabb.getMinY(), aabb.getMaxY() - 1);
    
    sobj->setPosition(CCPoint(posx, posy));
    
    return false;
}

const WorldScene::CreatureHp WorldScene::collectRoleHp() const
{
    WorldScene::CreatureHp ch;

    list<WorldObject*>::const_iterator iter;
    foreach(iter, mObjects)
    {
        if ((*iter)->isCreature()) {
            WorldCreature *obj = dynamic_cast<WorldCreature*>(*iter);
            ch[obj] = obj->getHp();
        }
    }

    return ch;
}

const WorldScene::CreatureHp WorldScene::findHpChangedCreatures(const WorldScene::CreatureHp &ch) const
{
    WorldScene::CreatureHp chr;

    list<WorldObject*>::const_iterator iter;
    foreach(iter, mObjects)
    {
        if (!(*iter)->isCreature()) {
            continue;
        }
        WorldCreature *obj = dynamic_cast<WorldCreature*>(*iter);
        WorldScene::CreatureHp::const_iterator it = ch.find(obj);
        if (it != ch.end() && obj->getHp() != it->second) {
            chr[obj] = obj->getHp();
        }
    }

    return chr;
}


WorldPlayer* WorldScene::getPlayerByCreature(WorldCreature* creature)
{
    WorldPlayer* player = NULL;
    
    int killerType = creature->getType();
    switch (killerType) {
        case eWorldObjectPet:{
            WorldPet* pet = dynamic_cast<WorldPet*>(creature);
            if (pet){
                player = pet->mPlayer;
            }
            break;
        }
        case eWorldObjectRole:{
            WorldRole* role = dynamic_cast<WorldRole*>(creature);
            if (role){
                player = role->mPlayer;
            }
        }
        default:
            break;
    }
    
    return player;
}


WorldMonster* WorldScene::createMonster(int monsterId, int posX, int posY)
{
    WorldMonster* monster = WorldMonster::create(monsterId);
    if (monster != NULL) {
        addMonster(monster);
        monster->setPosition(CCPoint(posX, posY));
    }
    
    return monster;
}

void WorldScene::onEvent(int eventId, long lParam, long rParam)
{

}

void WorldScene::emitEvent(int eventId, long lParam, long rParam)
{
    WorldSceneEvent event;
    event.eventId = eventId;
    event.lParam = lParam;
    event.rParam = rParam;
    mEvents.push_back(event);
}


int WorldScene::getAreaIdByPosition(int x, int y)
{
    Point pt = {x, y};
    for (int i = 0; i < mCfg.mAreas.size(); i++) {
        if (isPointInRect(pt, mCfg.mAreas[i].mRect)) {
            return i;
        }
    }
    return -1;
}



