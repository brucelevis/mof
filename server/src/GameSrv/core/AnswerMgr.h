//
//  AnswerMgr.h
//  GameSrv
//
//  Created by nothing on 13-11-20.
//
// 答题活动控制器
//

#pragma once
#include <string>

#include "Activity.h"
class Role;

class AnswerMgr
{
public:
    AnswerMgr();
    ~AnswerMgr();
    void initialize();
    bool is_open() { return _isopen;}
    void set_open();
    void set_close(){ _isopen = false;}
    int getItime() { return _itime;}
private:
    static void activity_start(int actId, int number, string& params);
    static void activity_end(int actId, int number, string& params);
    
    bool _isopen;
    // 记录抽题的活动开启时间
    int _itime;
};

extern AnswerMgr g_AnswerMgr;

// 答题活动
struct QuestionState
{
    int id; // 题号
    bool isAnswered; // 是否回答
    bool isRight; // 对错
    
    QuestionState(){}
    QuestionState(int title)
    {
        id = title;
        isAnswered = false;
        isRight = false;
    }
    string formatSaveData();
    bool insertData(const string &data);
};

class AnswerActivityMgr
{
public:
    AnswerActivityMgr();
    ~AnswerActivityMgr();
    void load(Role *role);
    // 检查游戏结束
    bool gameIsOver();
    // 获取当前题目倒计时
    int getEndTime();
    // 抽题
    void pumpingTitle();
    // 处理题目超时
    void procTimeout();
    int getQuestion(const int &current_num);
    // 获取回答题的分数
    int getTotalScore();
    
    bool isRightAnswer(const int &role_answer);
    // 改题, 返回改题后的题号
    int changeQuestion();
    
    int getSurplusNum();
    // 游戏结束奖励
    bool gameOverReward();
    int getRightCount();
    
    void weekRefresh();
	void dayRefresh();
    
    int okToGetReward(const int &type);
private:
    void save();
    void saveQuestionState(QuestionState &qs);
    int getQuestionCount();
    void offlineReward();
    bool reward(const string &rewardItems);
    QuestionState * getQuestion();
    void setNextQuestion();
    void countReward();
	bool isFullRight();
    
    Role *mRole;
    std::vector<QuestionState> mQuestions; // 当前抽的题目
    std::vector<int> mHistoryChangeQuestion; //历史换过的题目
};







