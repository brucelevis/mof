//
//  AnswerMgr.cpp
//  GameSrv
//
//  Created by nothing on 13-11-20.
//
//

#include "AnswerMgr.h"
#include "Activity.h"
#include "GameLog.h"
#include "main.h"
#include "Utils.h"
#include "GameLog.h"
#include "DBRolePropName.h"
#include "json/json.h"
//#include "hander_include.h"
#include "mail_imp.h"
#include "DynamicConfig.h"
#include "redis_helper.h"
#include "Role.h"
AnswerMgr g_AnswerMgr;


AnswerMgr::AnswerMgr()
{
    _isopen = false;
}

AnswerMgr::~AnswerMgr()
{
}

void AnswerMgr::set_open()
{
    _isopen = true;
    int openedSecs = SActMgr.GetOpenedSecs(ae_daily_answer);
    _itime = time(NULL) - openedSecs;
    //log_info("答题活动开启!距开启时间经过秒:"<<openedSecs<<" 上次活动开启时间:"<<_itime);
}

void AnswerMgr::initialize()
{
    SActMgr.SafeCreateTimer(ae_daily_answer, att_open, 0, activity_start, "");
    SActMgr.SafeCreateTimer(ae_daily_answer, att_close, 0, activity_end, "");
}

void AnswerMgr::activity_start(int actId, int number, string& params)
{
    g_AnswerMgr.set_open();
}

void AnswerMgr::activity_end(int actId, int number, string& params)
{
    g_AnswerMgr.set_close();
}


string QuestionState::formatSaveData()
{
    Json::Value value;
    value["id"] = id;
    value["isAnswered"] = isAnswered;
    value["isRight"] = isRight;
    return xyJsonWrite(value);
}

bool QuestionState::insertData(const string &data)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(data, value) || value.type() != Json::objectValue) {
        return false;
    }
    id = value["id"].asInt();
    isAnswered = value["isAnswered"].asBool();
    isRight = value["isRight"].asBool();
    return true;
}

// 数据库题目超时
static const int ANSWER_TITLE_DB_OFFTIME = 24 * 60 * 60;
static const string ANSWER_TITLE_DB_KEY = "answeractivity";

AnswerActivityMgr::AnswerActivityMgr()
{
}

AnswerActivityMgr::~AnswerActivityMgr()
{
}

void AnswerActivityMgr::load(Role *role)
{
    mRole = role;
    
    RedisResult result(redisCmd("hvals %s:%d", ANSWER_TITLE_DB_KEY.c_str(), mRole->getInstID()));
	int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i++)
    {
        QuestionState qs;
        if (!qs.insertData(result.readHash(i, ""))) {
            continue;
        }
        mQuestions.push_back(qs);
    }
    
    // 离线奖励
    this->offlineReward();
}

void AnswerActivityMgr::save()
{
    doRedisCmd("del %s:%d", ANSWER_TITLE_DB_KEY.c_str(), mRole->getInstID());
    std::vector<QuestionState>::iterator it;
    for (it = mQuestions.begin(); it != mQuestions.end(); it++)
    {
        this->saveQuestionState(*it);
    }
    doRedisCmd("expire %s:%d %d", ANSWER_TITLE_DB_KEY.c_str(), mRole->getInstID(), ANSWER_TITLE_DB_OFFTIME);
}

void AnswerActivityMgr::saveQuestionState(QuestionState &qs)
{
    doRedisCmd("hmset %s:%d %d %s", ANSWER_TITLE_DB_KEY.c_str(), mRole->getInstID(), qs.id, qs.formatSaveData().c_str());
}

void AnswerActivityMgr::pumpingTitle()
{
    int activityDaySw = mRole->getAnswerActivityDaySw();
    // 当天已抽题
    if (activityDaySw > 0) {
        return;
    }
    stringstream titleStr;
    mQuestions.clear();
    vector<int> title = AnswerActivityCfg::getRandomQuestions();
    for (vector<int>::iterator it = title.begin(); it != title.end(); it++)
    {
        QuestionState q(*it);
        mQuestions.push_back(q);
        titleStr<<*it<<",";
    }
    Xylog log(eLogName_AnswerTitle, mRole->getInstID());
    log<<titleStr.str();
    
    int now = time(NULL);
    mRole->setAnswerActivityTime(now);
    mRole->setAnswerCurrentNumber(1);
    mRole->setAnswerStartTime(now);
    mRole->setAnswerPumpingTime(now);
    mRole->setAnswerChangeCount(0);
	mRole->setAnswerActivityDaySw(1);
    // 保存
    this->save();
    mRole->saveNewProperty();
}

bool AnswerActivityMgr::gameIsOver()
{
    if (mRole->getAnswerCurrentNumber() > getQuestionCount()) {
        return true;
    }
    return false;
}

int AnswerActivityMgr::getEndTime()
{
    int tm = time(NULL) - mRole->getAnswerStartTime();
    int temp = AnswerActivityCfg::getAnswerTm() - tm;
    if (temp > 0) {
        return temp;
    }
    return 0;
}

void AnswerActivityMgr::procTimeout()
{
    if (this->getEndTime() > 0) {
        return;
    }
    this->setNextQuestion();
}

int AnswerActivityMgr::getQuestion(const int &current_num)
{
    if (current_num > getQuestionCount() || current_num < 0)
    {
        return 0;
    }
    return mQuestions[current_num - 1].id;
}

QuestionState * AnswerActivityMgr::getQuestion()
{
    int current_num = mRole->getAnswerCurrentNumber();
    if (current_num > getQuestionCount() || current_num < 0)
    {
        return NULL;
    }
    return &mQuestions[current_num - 1];
}

void AnswerActivityMgr::setNextQuestion()
{
    mRole->setAnswerStartTime(time(NULL));
    mRole->setAnswerCurrentNumber(mRole->getAnswerCurrentNumber() + 1);
    mRole->saveNewProperty();
}

bool AnswerActivityMgr::isRightAnswer(const int &role_answer)
{
    QuestionState *qs = this->getQuestion();
    if (NULL == qs) {
        return false;
    }
    
    qs->isAnswered = true;
    qs->isRight = AnswerActivityCfg::isRightAnswer(qs->id, role_answer);
    this->setNextQuestion();
    
    return qs->isRight;
}

int AnswerActivityMgr::changeQuestion()
{
    QuestionState *qs = this->getQuestion();
    if (NULL == qs) {
        return 0;
    }
    vector<int> idVec;
    std::vector<QuestionState>::iterator iter;
    for (iter = mQuestions.begin(); iter != mQuestions.end(); iter++)
    {
        idVec.push_back(iter->id);
    }
    mHistoryChangeQuestion.push_back(qs->id);
    std::copy(mHistoryChangeQuestion.begin(), mHistoryChangeQuestion.end(), back_inserter(idVec));
    
    int questionId = AnswerActivityCfg::chargeQuestion(qs->id, idVec);
    if (0 == questionId)
    {
        xyerr("答题游戏换题失败, 系统数据换不到题");
        return questionId;
    }
    qs->id = questionId;
    mRole->setAnswerStartTime(time(NULL));
    mRole->setAnswerChangeCount(mRole->getAnswerChangeCount() + 1);
    mRole->saveNewProperty();
    return questionId;
}

int AnswerActivityMgr::getQuestionCount()
{
    return mQuestions.size();
}

int AnswerActivityMgr::getTotalScore()
{
    vector<QuestionState>::iterator it;
    int score = 0;
    for (it = mQuestions.begin(); it != mQuestions.end(); it++)
    {
        if (it->isRight)
        {
            score += AnswerActivityCfg::getQuestionScore(it->id);
        }
    }
    return score;
}

int AnswerActivityMgr::getRightCount()
{
    int count = 0;
    for (std::vector<QuestionState>::iterator iter = mQuestions.begin(); iter!=mQuestions.end(); iter++)
    {
        if (iter->isRight)
        {
            ++count;
        }
    }
    return count;
}

void AnswerActivityMgr::offlineReward()
{
    string rewardItems = mRole->getAnswerOfflineItems();
    if (rewardItems.empty())
    {
        return;
    }
    if (!this->reward(rewardItems))
    {
        // 失败， 再次添加动作
        mRole->setAnswerOfflineItems(rewardItems);
        mRole->saveNewProperty();
    }
}

bool AnswerActivityMgr::gameOverReward()
{
    int score = getTotalScore();
    string rewardItems = AnswerActivityCfg::getScoreRewards(score);
    // 游戏结束，奖励写日志
    Xylog log(eLogName_AnswerReward, mRole->getInstID());
    log<<this->getQuestionCount()<<this->getRightCount()<<score<<rewardItems;
    
    // 累计处理
    this->countReward();
    
    return this->reward(rewardItems);
}

void AnswerActivityMgr::countReward()
{
	bool isContinue = false;
	int continueNum = 1;
	int fullContinueNum = 0;
    // 天连续
    if (xyDiffday(mRole->getAnswerLastTime(), mRole->getAnswerActivityTime()) == 1)
    {
		continueNum = mRole->getAnswerContinueNum() + 1;
        isContinue = true;
    }
    
	// 满分
	if (isFullRight())
	{
		fullContinueNum = isContinue ? mRole->getAnswerContinueFunNum() + 1 : 1;
	}
	
	mRole->setAnswerCountNum(mRole->getAnswerCountNum() + 1);
	mRole->setAnswerContinueNum(continueNum);
	mRole->setAnswerContinueFunNum(fullContinueNum);
    mRole->setAnswerLastTime(mRole->getAnswerActivityTime());
    mRole->saveNewProperty();
}

bool AnswerActivityMgr::reward(const string &rewardItems)
{
    if (rewardItems.empty())
    {
        return true;
    }
    vector<string> awards = StrSpilt(rewardItems.c_str(), ";");
    ItemArray items;
    if (!mRole->addAwards(awards, items, "答题游戏"))
    {
        // 添加奖励失败，下次登录发放
        mRole->setAnswerOfflineItems(rewardItems);
        mRole->saveNewProperty();
        return false;
    }
    return true;
}

int AnswerActivityMgr::getSurplusNum()
{
    // 当前题目从1开始
    return this->getQuestionCount() - mRole->getAnswerCurrentNumber();
}

void AnswerActivityMgr::weekRefresh()
{
    mRole->setAnswerLastTime(0);
    mRole->setAnswerContinueNum(0);
    mRole->setAnswerContinueNumReward(0);
    mRole->setAnswerCountNum(0);
    mRole->setAnswerCountNumReward(0);
    mRole->setAnswerContinueFunNum(0);
    mRole->setAnswerContinueFunNumReward(0);
    mRole->saveNewProperty();
}

void AnswerActivityMgr::dayRefresh()
{
	mRole->setAnswerActivityDaySw(0);
	mRole->saveNewProperty();
}

bool AnswerActivityMgr::isFullRight()
{
	return this->getRightCount() == this->getQuestionCount() ? true : false;
}




/*
 //获取奖励状态
 enum eGetRewardsResult
 {
 eGetRewardsResult_Ok,
 eGetRewardsResult_NoDay, //天数未达到
 eGetRewardsResult_Fullbag,  //背包满了
 eGetRewardsResult_RewardAlreayGet, //奖励已经领取了
 eGetRewardsResult_UnknowError,
 };
 */
int  AnswerActivityMgr::okToGetReward(const int &type)
{
    TotalawardItem ti = AnswerActivityCfg::getTotalawardItem(type);
    if (ti.totaldays <= 0) {
        return false;
    }
    switch (type)
    {
        case econtinuousRewardType_Question:
            if (mRole->getAnswerCountNum() < ti.totaldays)
            {
                return eGetRewardsResult_NoDay;
            }
            if (mRole->getAnswerCountNumReward())
            {
                return eGetRewardsResult_RewardAlreayGet;
            }
            mRole->setAnswerCountNumReward(1);
            break;
        case econtinuousRewardType_Day:
            if (mRole->getAnswerContinueNum() < ti.totaldays)
            {
                return eGetRewardsResult_NoDay;
            }
            if (mRole->getAnswerContinueNumReward())
            {
                return eGetRewardsResult_RewardAlreayGet;
            }
            mRole->setAnswerContinueNumReward(1);
            break;
        case econtinuousRewardType_FullScore:
            if (mRole->getAnswerContinueFunNum() < ti.totaldays)
            {
                return eGetRewardsResult_NoDay;
            }
            if (mRole->getAnswerContinueFunNumReward())
            {
                return eGetRewardsResult_RewardAlreayGet;
            }
            mRole->setAnswerContinueFunNumReward(1);
            break;
        default:
            return eGetRewardsResult_UnknowError;
    }
    
    // 奖东西
    string rewardItems = ti.awarditem;
    vector<string> awards = StrSpilt(rewardItems.c_str(), ";");
    ItemArray items;
    if (!mRole->addAwards(awards, items, "答题游戏"))
    {
        return eGetRewardsResult_Fullbag;
    }
	
    // 保存领取记录
    mRole->saveNewProperty();
    return eGetRewardsResult_Ok;
}




