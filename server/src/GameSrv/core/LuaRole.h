//
//  LuaRole.h
//  GameSrv
//
//  Created by 丁志坚 on 15/5/14.
//
//

#ifndef __GameSrv__LuaRole__
#define __GameSrv__LuaRole__

#include <stdio.h>
class Role;
class BaseProp;
class BattleProp;

class LuaRole
{
public:
    static LuaRole* create(Role* role);
    
    LuaRole();
    ~LuaRole();
    bool init(Role* role);
    
    int getLuaRef();
    int getRawRef();
    
    void loadData();
    void accuProperty(BaseProp& baseProp, BattleProp& batProp);
    
private:
    int mLuaRef;
    int mRawRef;
    Role* mParent;
};

#endif /* defined(__GameSrv__LuaRole__) */
