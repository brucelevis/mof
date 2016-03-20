//
//  BossFactory.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//

#include "Boss.h"
#include <map>

//谁生成谁销毁，不要让boss给ActSceneManager这个delete了

class BossFactory
{
public:
    BossFactory(){}
    ~BossFactory(){}
        
    Boss* create(enumBossType type);
    void release(int bossid);
    
    Boss* getBoss(int bossid);
    
protected:
    typedef std::map<int,Boss*> BossMap;
    BossMap     mBossMap;
};


extern BossFactory SBossFactory;
