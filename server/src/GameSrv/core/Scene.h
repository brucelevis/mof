//
//  Scene.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#ifndef __GameSrv__Scene__
#define __GameSrv__Scene__

#include "Defines.h"
#include <map>
#include <vector>
#include <list>
#include "Pet.h"
#include "DataCfg.h"
#include "NetPacket.h"
#include "groupmgr.h"
#include "basedef.h"

class Retinue;

class Obj;class Role;class SceneMgr;
class Scene
{
    READWRITE(int, mID, ID)
    READONLY(int, mType, Type)
    READWRITE(int, mThreadId, ThreadId)
    
public:
    Scene(int _id);
    virtual ~Scene();

    void config();

    void update(float dt);
    void objLeave(Obj* r);
    
    bool preAddRole(Role* role);
    void addRole(Role* role);
    void removeRole(Role* role);

    //vector<Obj*> areaObjs(Obj* obj, float radius,bool noself=true);
    
    void broadcast(INetPacket* packet, int execlusiveid = 0);

    void traverseRoles(TraverseCallback callback, void* param);
    void traversePets(Traverser* callback);
    void traverseRetinues(Traverser* traverser);
    
    int getRoleNum();
    
    
    int addPet(Pet* pet);
    int removePet(Pet* pet);
    int addRetinue(Retinue* retinue);
    int removeRetinue(Retinue* retinue);
    
    
    void sendDisplayedPets();

protected:
    void _objJoin(Obj* r);
    

private:
    std::map<int, Obj*> mObjs; // city players
    
    map<int, Role*> mRoles;
    list<Pet*> mPets;
    list<Retinue*> mRetinues;
    friend class SceneMgr;
};


#endif /* defined(__GameSrv__Scene__) */
