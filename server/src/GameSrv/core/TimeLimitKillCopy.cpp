//
//  TimeLimitKillCopy.cpp
//  GameSrv
//
//  Created by prcv on 13-11-25.
//
//

#include "TimeLimitKillCopy.h"
#include "Game.h"
#include "cmd_def.h"
#include "GameLog.h"
#include "Role.h"

extern string getRandBoxItems(ItemCfgDef* randbox);


TimeLimitKillCopy::TimeLimitKillCopy(int copyId)
{
    mCopyId = copyId;
    mGroupIdx = 0;
    mScore = 0;
}

TimeLimitKillCopy* TimeLimitKillCopy::create(int copyId, const CopyCost& cost)
{
    TimeLimitKillCopyCfgDef* cfg = TimeLimitKillCopyCfg::getCfg(copyId);
    if (cfg == NULL)
    {
        return NULL;
    }
    TimeLimitKillCopy* copy = new TimeLimitKillCopy(copyId);
    copy->m_className = TimeLimitKillCopy::getClassName();
    copy->init(*cfg);
    copy->mCopyCost = cost;
    return copy;
}

bool TimeLimitKillCopy::init(TimeLimitKillCopyCfgDef& cfg)
{
    mMonsterIdx = 1;
    mCfg = cfg;
    mEndTime = Game::tick + cfg.mLastTime;
    mNextRefreshTime = Game::tick;
    mGroupIdx = 0;
    mScore = 0;

    
    for (int i = 0; i < mCfg.mGroupMonsters.size(); i++)
    {
        mBufferedUnitCount.push_back(vector<int>());
        mKilledCount.push_back(vector<int>());
        mRemainCount.push_back(vector<int>());
        
        vector<TlkMonsterCfgDef> &monsters = mCfg.mGroupMonsters[mGroupIdx];
        for (int j = 0; j < monsters.size(); j++)
        {
            mBufferedUnitCount[i].push_back(0);
            mKilledCount[i].push_back(0);
            mRemainCount[i].push_back(0);
        }
    }

    return true;
}

bool TimeLimitKillCopy::killMonster(int monsterId)
{
    MonsterIter iter = mMonsters.find(monsterId);
    if (iter == mMonsters.end())
    {
        return false;
    }
    
    int groupIndex = iter->second->mGroupIdx;
    int index = iter->second->mIndex;
    //已杀数量
    mKilledCount[groupIndex][index]++;
    //场上剩余数量
    mRemainCount[groupIndex][index]--;
    
    //缓冲数量
    for (int i = 0; i < mCfg.mGroupMonsters[mGroupIdx].size(); i++)
    {
        mBufferedUnitCount[mGroupIdx][i]++;
    }
    //分数
    TlkMonsterCfgDef& def = mCfg.mGroupMonsters[groupIndex][index];
    mScore += def.mScore;
    //分数达到一定程度，开始刷下一组的怪物
    if (mGroupIdx < mCfg.mGroupScores.size() - 1 && mScore >= mCfg.mGroupScores[mGroupIdx + 1])
    {
        mGroupIdx++;
    }

    delete iter->second;
    mMonsters.erase(iter);
    return true;
}

TimeLimitKillCopy::~TimeLimitKillCopy()
{
    for (MonsterIter iter = mMonsters.begin(); iter != mMonsters.end(); ++iter)
    {
        TlkMonster* monster = iter->second;
        if (monster)
        {
            delete monster;
        }
    }
}

//
//void TimeLimitKillCopy::finish()
//{
//    for (size_t i=0;i<m_objs.size();i++)
//    {
//        // 加入广播列表
//        if (m_objs[i].first && m_objs[i].first->getType() == eRole)
//        {
//            Role* pRole = (Role*)m_objs[i].first;
//            
//            if (!takeCost(pRole))
//            {
//                break;
//            }
//
//            int score = getScore();
//            string awards = getAwards(score);
//            sendAwards(pRole, awards);
//            
//            //ItemArray items;
//            //RewardStruct reward;
//            //vector<string> itemstrs = StrSpilt(awards, ";");
//            //string desc = strFormat("限时击杀副本:%d", getCopyId());
//            //pRole->addAwards(itemstrs, items, desc.c_str());
//
//            pRole->backToCity();
//            ack_leave_tlk_copy ack;
//            ack.awards = awards;
//            ack.errorcode = 0;
//            sendNetPacket(pRole->getSessionId(), &ack);
//        }
//    }
//
//    ClearAll();
//    Destroy();
//}

bool TimeLimitKillCopy::takeCost(Role* role)
{
    GridArray grids;
    if (mCopyCost.mItems.size() > 0)
    {
        if (!role->getBackBag()->PreDelItems(mCopyCost.mItems, grids))
        {
            return false;
        }
    }
    
    if (role->getFat() < mCopyCost.mOther.reward_fat)
    {
        return false;
    }
    
    role->addFat(-mCopyCost.mOther.reward_fat);
    
//    role->updateBackBag(grids, mCopyCost.mItems, false, "time_limit_kill_copy");
    role->playerDeleteItemsAndStore(grids, mCopyCost.mItems, "time_limit_kill_copy", true);
    
    return true;
}


void TimeLimitKillCopy::handleLeave(Role* role)
{
    Remove(role);
    
    //判断是否该销毁
    int roleCount = getRoleCount();
    if (roleCount == 0)
    {
        Destroy();
    }
}

string TimeLimitKillCopy::handleGetAward(Role* role)
{
    string awards;
    if (takeCost(role)) {
        //从随即礼包里面去物品
        int score = getScore();
        string rpIdStr = getAwards(score);
        int rpId = Utils::safe_atoi(rpIdStr.c_str());
        if (rpId > 0) {
            ItemCfgDef *randbox = ItemCfg::getCfg(rpId);
            awards = getRandBoxItems(randbox);
        }
        
        sendAwards(role, awards);
    }
    
    Remove(role);
    
    //判断是否该销毁
    int roleCount = getRoleCount();
    if (roleCount == 0) {
        Destroy();
    }
    
    return awards;
}


void TimeLimitKillCopy::sendAwards(Role* role, const string& awards)
{
    ItemArray items;
    RewardStruct reward;
    vector<string> itemstrs = StrSpilt(awards, ";");
    string desc = strFormat("限时击杀副本:%d", getCopyId());
    role->addAwards(itemstrs, items, desc.c_str());
}

void TimeLimitKillCopy::OnHeartBeat()
{
    static int beat = 0;
    beat++;

    //时间达到之后，不再刷新怪物
    if (Game::tick > mEndTime)
    {
        //十秒后主动踢出场景
        //if (Game::tick - mEndTime > 10)
        //{
        //    finish();
        //}

        return;
    }

    //刷新怪物
    if (Game::tick >= mNextRefreshTime)
    {
        refreshMonster();
        mNextRefreshTime += mCfg.mRefreshTime;
    }
}

int TimeLimitKillCopy::getRoleCount()
{
    int count = 0;
    for (size_t i=0;i<m_objs.size();i++)
    {
        if (m_objs[i].first && m_objs[i].first->getType() == eRole)
        {
            count++;
        }
    }

    return count;
}

void TimeLimitKillCopy::refreshMonster()
{
    notify_refresh_tlk_monster notify;
    //优先生成高级怪物
    vector<TlkMonsterCfgDef> &monsters = mCfg.mGroupMonsters[mGroupIdx];
    int i = monsters.size();
    if (i == 0)
    {
        return;
    }
    while (--i)
    {
        //判断是否达到生成该怪物的条件，没有的话处理下一种怪物
        if (mBufferedUnitCount[mGroupIdx][i] < monsters[i].mUnitNum)
        {
            continue;
        }

        int newCount = mBufferedUnitCount[mGroupIdx][i] / monsters[i].mUnitNum;
        for (int j = 0; j < newCount; j++)
        {
            int monsterId = mMonsterIdx++;

            TlkMonster* monster = TlkMonster::create(monsterId, monsters[i].mMonsterMod, mGroupIdx, i);
            mMonsters[monsterId] = monster;

            obj_tlk_monster objMonster;
            objMonster.monsterid = monsterId;
            objMonster.mod = monsters[i].mMonsterMod;
            notify.monsters.push_back(objMonster);
        }

        mBufferedUnitCount[mGroupIdx][i] %= monsters[i].mUnitNum;
        mRemainCount[mGroupIdx][i] += newCount;

    }

    //剩余全部生成低级怪物
    int newBaseCount = mCfg.mMaxMonsterNum - getMonsterNum();
    for (int k = 0; k < newBaseCount; k++)
    {
        int monsterId = mMonsterIdx++;

        TlkMonster* monster = TlkMonster::create(monsterId, monsters[0].mMonsterMod, mGroupIdx, 0);
        mMonsters[monsterId] = monster;

        obj_tlk_monster objMonster;
        objMonster.monsterid = monsterId;
        objMonster.mod = monsters[i].mMonsterMod;
        notify.monsters.push_back(objMonster);
    }
    mRemainCount[mGroupIdx][0] += newBaseCount;

    if (notify.monsters.size() > 0)
    {
        BroadCast(&notify);
    }
}
