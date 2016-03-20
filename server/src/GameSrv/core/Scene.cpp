//
//  Scene.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#include "Scene.h"
#include "Role.h"
#include "math.h"
#include "msg.h"
#include "main.h"
#include "Monster.h"
#include "Constellation.h"
#include "ItemBag.h"
#include "Retinue.h"

/*
 instance------------------------------------------------
 */
Scene::Scene(int _id):mID(_id){
    config();
}

Scene::~Scene()
{
}

void Scene::config(){
//    SceneCfgDef* def = getCfg(mID);
//    assert(def);
//    mType=def->sceneType;
//    if(getType() != stWorldBoss)
//        return;
//    for (int i= 0; i<def->mMonsters.size(); i++) {
//        Boss* m = new Boss();
//        int mid = def->mMonsters[i];
//        MonsterCfgDef* md = MonsterCfg::getCfg(mid);
//        assert(md);
//        m->MonsterInit(md);
//        _objJoin(m);
//        
//        
//    }
}

void Scene::update(float dt)
{
    //if (mType != stWorldBoss) return;
    //std::map<int,Obj*>::iterator it;
    //for (it = mObjs.begin(); it != mObjs.end(); it++)
    //{
        //it->second->update(dt);
    //}
}

void Scene::_objJoin(Obj* r){
    if(mObjs.find(r->getInstID()) == mObjs.end()){
        mObjs[r->getInstID()] = r;
        r->setScene(this);
        //r->setSceneID(getID());
    }
    else{
        //todo log
    }
}

void Scene::objLeave(Obj* r){
    std::map<int,Obj*>::iterator it;
    if((it=mObjs.find(r->getInstID())) != mObjs.end()){
        mObjs.erase(it);
        r->setScene(NULL);
    }
    else{
        //todo log
    }
}


void Scene::broadcast(INetPacket* packet, int roleid)
{
    vector<int> sessions;
    
    std::map<int, Role*>::iterator iter;
    for (iter = mRoles.begin(); iter != mRoles.end(); iter++)
    {
    
        Role* role = iter->second;
        if (role->getInstID() != roleid)
        {
            sessions.push_back(role->getSessionId());
        }
    }
    
    multicastPacket(sessions, packet);
}


void Scene::traverseRoles(TraverseCallback callback, void* param)
{
    std::map<int, Role*>::iterator iter;

    for (iter = mRoles.begin(); iter != mRoles.end(); iter++)
    {
        Role* role = iter->second;
        if (!callback(role, param))
        {
            return;
        }
    }
}

int Scene::getRoleNum()
{
    return mRoles.size();
}

bool Scene::preAddRole(Role* role)
{
    if (getRoleNum() >= GlobalCfg::ReadInt("city_capacity", 20))
    {
        return false;
    }
    return true;
}

int Scene::addPet(Pet *newPet)
{
    if (!newPet) {
        return 0;
    }
    
    PetCfgDef* petCfgDef = PetCfg::getCfg(newPet->petmod);
    if (petCfgDef && petCfgDef->getDisplayInCity() == 0) {
        return 0;
    }
    
    int rank = 1;
    list<Pet*>::iterator iter = mPets.begin();
    for (; iter != mPets.end(); iter++, rank++) {
        Pet* pet = *iter;
        if (pet->mBattle < newPet->mBattle) {
            break;
        } 
    }
    
    mPets.insert(iter, newPet);
    return rank;
}

int Scene::removePet(Pet *remPet)
{
    if (!remPet) {
        return 0;
    }
    
    int rank = 1;
    for (list<Pet*>::iterator iter = mPets.begin(); iter != mPets.end(); iter++, rank++) {
        if (remPet == *iter) {
            mPets.erase(iter);
            return rank;
        }
    }
    
    return 0;
}

int Scene::addRetinue(Retinue *newRetinue)
{
    if (newRetinue == NULL) {
        return 0;
    }
    
    int rank = 1;
    list<Retinue*>::iterator it = mRetinues.begin();
    while (it != mRetinues.end() ) {
        Retinue* retinue = *it;
        if (retinue -> getPropertyIntension() < newRetinue -> getPropertyIntension() ){
            break;
        }
        ++it;
        ++rank;
    }
    mRetinues.insert(it, newRetinue);
    return rank;
}

int Scene::removeRetinue(Retinue *retinue)
{
    if (retinue == NULL) {
        return 0;
    }
    
    int rank = 1;
    for (list<Retinue*>::iterator it = mRetinues.begin(); it != mRetinues.end(); ++it, ++rank) {
        if ( *it == retinue ) {
            mRetinues.erase(it);
            return rank;
        }
    }
    return 0;
}

void Scene::traversePets(Traverser* traverser)
{
    for (list<Pet*>::iterator iter = mPets.begin(); iter != mPets.end(); iter++) {
        
        Pet* pet = *iter;
        if (!traverser->callback(pet))
        {
            return;
        }
    }
}

void Scene::traverseRetinues(Traverser* traverser)
{
    for (list<Retinue*>::iterator iter = mRetinues.begin(); iter != mRetinues.end(); iter++) {
        
        Retinue* retinue = *iter;
        if (!traverser->callback(retinue))
        {
            return;
        }
    }
}

void Scene::addRole(Role* role)
{
    map<int, Role*>::iterator iter = mRoles.find(role->getInstID());
    if (iter != mRoles.end()) {
        return;
    }
    
    _objJoin(role);
    
    mRoles[role->getInstID()] = role;
    role->onEnterScene();
}


void Scene::removeRole(Role *role)
{
    map<int, Role*>::iterator iter = mRoles.find(role->getInstID());
    if (iter == mRoles.end()) {
        return;
    }
    
    role->onLeaveScene();
    mRoles.erase(iter);
    objLeave(role);
}