//
//  Quest.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#ifndef __GameSrv__Quest__
#define __GameSrv__Quest__

#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <string.h>

struct QuestCondition {
    std::string type;
    int target;
    int sum;
};

class Quest {
public:
    int questId;           //任务id
    std::string name;      //任务名字
    int type;              //任务类型
    int minLvl;          //最小等级
    int maxLvl;         //最大等级
    int preQuestId;        //前置任务
    std::string getQuestNpc; //领取任务npc
    std::string submitQuestNpc; //提交任务npc
    std::vector<std::string> conditions;  //完成条件
    std::string questBeginDialog; //进副本时任务*对话
    std::string questEndDialog; //过副本后任务*对话
    std::vector<std::string> rewards;     //发放奖励
    std::string describe; //任务描述
    std::vector<std::string> warrior_rewards; //剑士奖励
	std::vector<std::string> mage_rewards; //法师奖励
	std::vector<std::string> assassin_rewards; //刺客奖励
    int getQuestStep;
    int finishQuestStep;
    int pet_exp;
};



class QuestMgr {
public:
    static bool LoadFile(const char* FileName);
    static Quest* FindQuest(int questid);

    //玩家等级,完成任务id,执行任务id,[out]日常任务id,[out]其他任务id
    static void GetQuests(int lvl,std::list<int>& finishQuestIds,std::list<int>* outDailyQuestIds = NULL,std::list<int>* outOtherQuestIds=NULL);

    //获取可接任务
    static void GetAcceptableQuests(int lvl,std::vector<int>& finishQuestIds,std::vector<int>& doingquestIds,std::vector<int>* outAcceptAbleQuestIds = NULL,std::vector<int> * outDailyQuestIds=NULL);

    static void GetRewords(Quest* _quest,int* _exp,int *_gold,int* _batPoint, int* _petExp=0, std::map<int,int>* _itemlist = NULL);

    static std::list<Quest*> TypeQuests(const char* type,int id,std::list<int>* findindexs = NULL);

    //获取单个任务npc对话的id ，isSubmit 0：领任务npc，1交任务npc
    static void GetDialog(Quest *_quest,bool isSubmit,int *_npcid = NULL ,int *_dialogID = NULL);

    static void GetCurCondition(Quest *_quest,std::vector<int> _questProgress,QuestCondition* _condition,int *_curProgress = NULL);

    //获取单个任务完成条件
    static  void GetConditions(Quest *_quest,std::vector<QuestCondition* > &_conditions);
private:
    static std::map<int,Quest*> _Quests;
};

bool IsCondition_cmd(std::string cmd);
bool IsGm_cmd(std::string cmd);

//更新任务进度
//updatecmd :{"dialog $id*$select","killmonster $monsterid"}
void UpdateQuestProgress(std::string upatecmd,std::map<int, std::vector<int> >& mQuestProgress);

//检查任务进度完成
bool CheckQuestCanFinish(Quest* quest,std::vector<int>& QuestProgress);

//检查进度已经完成的任务
std::vector<int> CheckCanFinishQuests(std::map<int, std::vector<int> >& mQuestProgress);

#endif /* defined(__GameSrv__Quest__) */
