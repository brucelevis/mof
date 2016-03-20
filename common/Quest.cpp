//
//  Quest.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#include "Quest.h"
#include "inifile.h"
#include <vector>
#include "Utils.h"
#include "DataCfg.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>

bool IsCondition_cmd(std::string cmd)
{
    char c[50] = "";
    int id = 0;
    int num = 0;

    if( (sscanf(cmd.c_str(), "%s %d*%d",c,&id,&num) == 3)
       || (sscanf(cmd.c_str(), "%s %d",c,&num) == 2)
       )
    {
        std::string cc(c);
        if ( (cc == "killmonster") ||
            (cc == "entercopy") ||
            (cc == "finishcopy") ||
            (cc == "wincopy") ||
            (cc == "haveitem") ||
            (cc == "lvlup") ||
            (cc == "dialog") ||
            (cc == "upequip")||
            (cc == "studyskill")||
            (cc == "arenabattle")||
            (cc == "copycombo")||
            (cc == "copystar")||
            (cc == "finishelitecopy") ||
            (cc == "constellupg") ||    //星灵升级
            (cc == "studypassiveskill") //学习被动技能
            ||(cc == "enterfriendcopy")                        //完成好友副本
            ||(cc == "enterpetcamp")                        //完成幻兽大本营
            ||(cc == "finishpetadventure")                  //完成幻兽大冒险
            ||(cc == "finishfamoushall")              //通过名人堂
            ||(cc == "fashioncollectmaterial")                    //收集时装碎片
            ||(cc == "petevolution")                        //宠物进化
            )
            return true;
    }

    return false;
}

bool IsGm_cmd(std::string cmd)
{
    char c[30] = "";
    char args[128]  = "";
    if(sscanf(cmd.c_str(), "%s %s",c,args) == 2)
    {
        std::string cc(c);
        if ((cc == "gold") ||
            (cc == "exp") ||
            (cc == "item") )
            return true;
    }
    return false;
}

std::map<int,Quest*> QuestMgr::_Quests;


bool QuestMgr::LoadFile(const char* FileName)
{
    GameInifile inifile(FileName);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("monster_tpltMultiLanguage.ini") == "monster_tpltMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("questsMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("questsMultiLanguage.ini"));
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
        Quest* tmp = new Quest;
        tmp->questId = inifile.getValueT(*iter,"questId",0);
        string name_id = inifile.getValue(*iter,"name_id");
        //lihengjin
        string name_no_mul;
        if(inifile.exists(*iter, "name"))
        {
            name_no_mul = inifile.getValue(*iter, "name");
        }else
        {
            name_no_mul = MLIniFile.getValue(name_id, "desc");
        }
        //tmp->name = MLIniFile.getValue(name_id, "desc");//inifile.getValue(*iter,"name");
        tmp->name = name_no_mul;
        tmp->type = inifile.getValueT(*iter,"type",0);
        tmp->minLvl = inifile.getValueT(*iter,"minLvl",0);
        tmp->maxLvl = inifile.getValueT(*iter,"maxLvl",0);
        tmp->pet_exp = inifile.getValueT(*iter,"pet_exp",0);
        tmp->preQuestId = inifile.getValueT(*iter,"preQuestId",0);
        tmp->getQuestNpc = inifile.getValue(*iter,"getQuestNpc");
        tmp->submitQuestNpc = inifile.getValue(*iter,"submitQuestNpc");
        
        string describe_id = inifile.getValue(*iter,"describe_id");
        
        //lihengjin
        string describe_no_mul;
        if(inifile.exists(*iter, "describe"))
        {
            describe_no_mul = inifile.getValue(*iter,"describe");
        }else
        {
            describe_no_mul = MLIniFile.getValue(describe_id, "desc");
        }
        //tmp->describe = MLIniFile->getValue(describe_id, "desc");//inifile.getValue(*iter,"describe");
        tmp->describe = describe_no_mul;
        tmp->questBeginDialog = inifile.getValue(*iter,"questBeginDialog");
        tmp->questEndDialog = inifile.getValue(*iter,"questEndDialog");
        tmp->getQuestStep = inifile.getValueT(*iter, "getQuestStep",0);
        tmp->finishQuestStep = inifile.getValueT(*iter, "finishQuestStep",0);
        //*****************

        //条件格式 killmonster 1000*10;entercopy 1000*1;finishcopy 1000*1;wincopy 1000*1;
        //haveitem 10001*1;lvlup 10;upequip 10;...etc
        std::string conditions = inifile.getValue(*iter,"conditions");
        tmp->conditions = StrSpilt(conditions,";");


        //奖励格式 gold 500;exp 1763;item 200*10;
        std::string rewards = inifile.getValue(*iter,"rewards");
        tmp->rewards = StrSpilt(rewards,";");

        std::string warrior_rewards = inifile.getValue(*iter,"warrior_rewards");
        tmp->warrior_rewards = StrSpilt(warrior_rewards,";");

        std::string mage_rewards = inifile.getValue(*iter,"mage_rewards");
        tmp->mage_rewards = StrSpilt(mage_rewards,";");

        std::string assassin_rewards = inifile.getValue(*iter,"assassin_rewards");
        tmp->assassin_rewards = StrSpilt(assassin_rewards,";");

        _Quests[tmp->questId] = tmp;
    }
    return true;
}

void QuestMgr::GetRewords(Quest* _quest,int* _exp,int *_gold,int* _batPoint, int* _petExp, std::map<int,int>* _itemlist )
{
    std::vector<std::string>* _rewords = &(_quest->rewards);
    bool hasBatPoint = false;
    for (std::vector<std::string>::iterator i = _rewords->begin(); i != _rewords->end(); i++)
    {
        StringTokenizer reToken(*i," *");
        if (reToken[0] == "gold")
            *_gold = atoi(reToken[1].c_str()) * atoi(reToken[2].c_str());
        if (reToken[0] == "exp")
            *_exp = atoi(reToken[1].c_str()) * atoi(reToken[2].c_str());
        if ((reToken[0] == "item") && (_itemlist != NULL))
        {
            int _itemID = atoi(reToken[1].c_str());
            (*_itemlist)[_itemID] = atoi(reToken[2].c_str());
        }
        if(reToken[0] == "batpoint")
        {
            hasBatPoint = true;
            *_batPoint = atoi(reToken[1].c_str()) * atoi(reToken[2].c_str());
        }
    }
    if(hasBatPoint == false)
        *_batPoint = 0;
    if(_petExp)
    {
        *_petExp = _quest->pet_exp;
    }
    
}

Quest* QuestMgr::FindQuest(int questid)
{
    std::map<int,Quest*>::iterator fit = _Quests.find(questid);
    if (fit != _Quests.end()) {
        return fit->second;
    }
    return NULL;
}

void QuestMgr::GetQuests(int lvl,std::list<int>& finishQuestIds,std::list<int>* outDailyQuestIds,std::list<int>* outOtherQuestIds)
{
    for (std::map<int,Quest*>::iterator iter = _Quests.begin(); iter != _Quests.end(); iter++) {
        if(iter->second->minLvl <= lvl && lvl<= iter->second->maxLvl)
        {
            std::list<int>::iterator fit =
                std::find(finishQuestIds.begin(), finishQuestIds.end(),iter->second->preQuestId);
            if (fit == finishQuestIds.end()) {
                if (iter->second->type == 3 && outDailyQuestIds)
                {
                    outDailyQuestIds->push_back(iter->second->questId);
                }
                else if (outOtherQuestIds)
                {
                    outOtherQuestIds->push_back(iter->second->questId);
                }
            }
        }
    }
}

void QuestMgr::GetDialog(Quest *_quest, bool isSubmit, int *_npcid, int *_dialogID)
{
    std::string _str;
    if (isSubmit)
        _str = _quest->submitQuestNpc;
    else
        _str = _quest->getQuestNpc;
    StringTokenizer reToken(_str," *");
    if (reToken.count() < 2) {
        return;
    }
    if(_npcid != NULL && reToken[0] != "")
        *_npcid = atoi(reToken[0].c_str());
    if(_dialogID != NULL && reToken[1] != "")
        *_dialogID = atoi(reToken[1].c_str());
}

void QuestMgr::GetConditions(Quest *_quest, std::vector<QuestCondition *> &_conditions){
    _conditions.clear();
    for (std::vector<std::string>::iterator i = (_quest->conditions).begin(); i != (_quest->conditions).end(); i++) {
        QuestCondition *_condition = new QuestCondition;
        StringTokenizer reToken(*i," *");
        _condition->type = reToken[0];
        _condition->target = atoi(reToken[1].c_str());
        _condition->sum = atoi(reToken[2].c_str());
        _conditions.push_back(_condition);
    }
}

void QuestMgr::GetAcceptableQuests(int lvl,std::vector<int>& finishQuestIds,std::vector<int>& doingquestIds,std::vector<int>* outAcceptAbleQuestIds,std::vector<int> * outDailyQuestIds)
{
    for (std::map<int,Quest*>::iterator iter = _Quests.begin(); iter != _Quests.end(); iter++) {
        if(iter->second->minLvl <= lvl && lvl<= iter->second->maxLvl)
        {
             //查找该任务是否已经完成了
            bool Finish = (std::find(finishQuestIds.begin(), finishQuestIds.end()
                                     ,iter->second->questId) != finishQuestIds.end());

            //日常任务可以重复完成
            if(iter->second->type == 3)
            {
                Finish = false;
            }
            if (Finish)
            {
                continue;
            }

            //查找该任务的前置任务是不是已经完成了。
            bool isPreQuestFinish = (std::find(finishQuestIds.begin(), finishQuestIds.end(),
                                               iter->second->preQuestId) != finishQuestIds.end()
                                    )||iter->second->preQuestId == 0;
            if(isPreQuestFinish == false)
            {
                continue;
            }

            //查找该任务是不是已经在身上了
            std::vector<int>::iterator fitdoing =
            std::find(doingquestIds.begin(), doingquestIds.end(), iter->second->questId);
            bool notDoing = false;
            if (fitdoing == doingquestIds.end())
                notDoing = true;
            else
            {
                notDoing = false;
                continue;
            }
            if (!Finish && isPreQuestFinish && notDoing) {
                if (iter->second->type == 3 )
                {
                    if(outDailyQuestIds)
                        outDailyQuestIds->push_back(iter->second->questId);
                }
                else if (outAcceptAbleQuestIds)
                {
                    outAcceptAbleQuestIds->push_back(iter->second->questId);
                }
            }
        }
    }
}

std::list<Quest*> QuestMgr::TypeQuests(const char* type,int id,std::list<int>* findindexs)
{
    std::list<Quest*> outQuests;
    int i = 0;
    for (std::map<int,Quest*>::iterator iter = _Quests.begin(); iter != _Quests.end(); iter++) {
        for (std::vector<std::string>::iterator viter = iter->second->conditions.begin();
             viter != iter->second->conditions.end();
             viter++) {
            char dcmd[50] = "";
            int did = 0;
            sscanf(viter->c_str(), "%s %d",dcmd,&did);
            if((strcmp(type,dcmd)==0) && (id == did || did == -1))
            {
                outQuests.push_back(iter->second);
                if (findindexs)
                    findindexs->push_back(i);
                break;
            }
            ++i;
        }
    }
    return outQuests;
}


void UpdateQuestProgress(std::string upatecmd,std::map<int, std::vector<int> >& QuestProgress)
{
    if (IsCondition_cmd(upatecmd)==false)return;

    char c[50] = "";
    int id = 0;
    int num = 1;


    if((sscanf(upatecmd.c_str(), "%s %d*%d",c,&id,&num) == 3)
       ||(sscanf(upatecmd.c_str(), "%s %d",c,&id) == 2)
       )
    {
        std::list<Quest*> typequests = QuestMgr::TypeQuests(c,id);
        std::list<Quest*>::iterator typequestend = typequests.end();
        for (std::list<Quest*>::iterator iter = typequests.begin(); iter != typequestend; iter++) {
            std::map<int, std::vector<int> >::iterator fqpiter = QuestProgress.find((*iter)->questId);
            if (fqpiter != QuestProgress.end()) {
                for (int vi = 0;vi != (*iter)->conditions.size();vi++) {
                    char findstr[100] = "";
                    sprintf(findstr, "%s %d",c,id);
                    char commonstr[100];
                    sprintf(commonstr, "%s -1", c);
                    std::string condstr = (*iter)->conditions[vi];
                    if (condstr.find(commonstr) != std::string::npos || condstr.find(findstr)!=std::string::npos) {
                        if (strcmp(c, "dialog")==0) {
                            fqpiter->second[vi] = num;
                        }
                        else if((strcmp(c, "copycombo")== 0)||strcmp(c, "copystar") == 0)
                        {
                            //fqpiter->second[vi] = num;
                            int dnum = 1;
                            sscanf((*iter)->conditions[vi].c_str(),"%*s %*d*%d",&dnum);
                            if(num >= dnum)
                                fqpiter->second[vi] = dnum;
                        }
                        else
                        {
                            fqpiter->second[vi] += num;
                            //限制超量
                            int dnum = 1;
                            sscanf((*iter)->conditions[vi].c_str(),"%*s %*d*%d",&dnum);
                            if(fqpiter->second[vi] > dnum)
                                fqpiter->second[vi] = dnum;
                        }
                    }

                }
            }
        }
    }
}



void UpdateQuestProgress(std::string type, int target, int num, std::map<int, std::vector<int> >& QuestProgress)
{
    std::list<Quest*> typequests = QuestMgr::TypeQuests(type.c_str(), target);
    for (std::list<Quest*>::iterator iter = typequests.begin(); iter != typequests.end(); iter++)
    {
        std::map<int, std::vector<int> >::iterator fqpiter = QuestProgress.find((*iter)->questId);
        if (fqpiter != QuestProgress.end())
        {
            for (int vi = 0;vi != (*iter)->conditions.size();vi++)
            {
                std::string condstr = (*iter)->conditions[vi];
                int condnum;
                int condtarget;
                sscanf((*iter)->conditions[vi].c_str(),"%*s %d*%d", &condtarget, &condnum);

                if (condtarget != target && condtarget != -1)
                {
                    return;
                }

                if (strcmp(type.c_str(), "dialog")==0)
                {
                    fqpiter->second[vi] = num;
                }
                else if((strcmp(type.c_str(), "copycombo")== 0)||strcmp(type.c_str(), "copystar") == 0)
                {
                    //fqpiter->second[vi] = num;
                    int dnum = 1;
                    sscanf((*iter)->conditions[vi].c_str(),"%*s %*d*%d",&dnum);
                    if(num >= dnum)
                        fqpiter->second[vi] = dnum;
                }
                else
                {
                    fqpiter->second[vi] += num;
                    //限制超量
                    if(fqpiter->second[vi] > condnum)
                    {
                        fqpiter->second[vi] = condnum;
                    }
                }

            }
        }
    }
}


void QuestMgr::GetCurCondition(Quest *_quest, std::vector<int> _questProgress, QuestCondition *_condition,int *_curProgress)
{
    //先判断有没有没完成的
    for (int i = 0; i != _quest->conditions.size(); i++)
    {
        StringTokenizer reToken((_quest->conditions)[i]," *");
        int _prograss = atoi(reToken[2].c_str());
        if (_questProgress[i] >= _prograss)
        {
            continue;
        }
        else
        {
            _condition->type = reToken[0];
            _condition->target = atoi(reToken[1].c_str());
            _condition->sum = atoi(reToken[2].c_str());
            if(_curProgress != NULL)
                *_curProgress = _questProgress[i];
            return;
        }
    }
    //没有没完成的，就取已经完成的第一个
    if((_quest->conditions).size() == 0 || _questProgress.size() == 0)
    {
        _condition = NULL;
        return;
    }
    StringTokenizer reToken((_quest->conditions)[0]," *");
    _condition->type = reToken[0];
    _condition->target = atoi(reToken[1].c_str());
    _condition->sum = atoi(reToken[2].c_str());
    if(_curProgress != NULL)
        *_curProgress = _questProgress[0];
}

bool CheckQuestCanFinish(Quest* quest,std::vector<int>& QuestProgress)
{
    for (int i = 0; i != quest->conditions.size(); i++) {
        int progress = 0;
        if( (sscanf(quest->conditions[i].c_str(),"%*s %*d*%d",&progress) == 1)
            ||(sscanf(quest->conditions[i].c_str(),"%*s %d",&progress) == 1)
            )
        {
            if (progress > QuestProgress[i])
                return false;
        }
        else{
            return false;
        }
    }
    return true;
}

std::vector<int> CheckCanFinishQuests(std::map<int, std::vector<int> >& QuestProgress)
{
    std::vector<int> out;
    for (std::map<int, std::vector<int> >::iterator iter = QuestProgress.begin(); iter != QuestProgress.end(); iter++) {
        Quest* quest = QuestMgr::FindQuest(iter->first);
        if (CheckQuestCanFinish(quest,iter->second))
        {
            out.push_back(iter->first);
        }
    }
    return out;
}
