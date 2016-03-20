//
//  ActivityWorldBossController.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//
// 崩溃恢复思路,暂时不用做,简单的重来就够用了
// 这里判断是否加载崩溃的那只BOSS来进行恢复
// 把BossID 存进数据库，表示还存活，同时把结束时间存进去，BOSS类型也要存一下，当然血量，受到攻击也要存
// 开机时，把全部的没有被杀死的BOSS加载进来，如果过了结束时间，就把他加载进来，然后失败处理
// 如果还没过结束时间，全部加载进来，然后继续杀BOSS
//
// Boss活动控制器
//

#pragma once
#include <string>
#include "Boss.h"


class BossActivityController
{

public:
    void Init();
    
    int getWorldBossId();
    int getGuildBossId(int guildid);

};

extern BossActivityController g_BossActivityController;



//再把handler 再重构一下即可