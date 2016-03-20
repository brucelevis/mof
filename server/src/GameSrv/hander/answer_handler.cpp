//
//  answer_handler.cpp
//  GameSrv
//
//  Created by nothing on 13-11-26.
//
//

#include "hander_include.h"
#include "Role.h"
#include "daily_schedule_system.h"

hander_msg(req_answerinfo_get, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_answerinfo_get ack;
    ack.errorcode = eAnswerInfoGetResult_UnknowError;
    do
    {
        if(!g_AnswerMgr.is_open())
        {
            ack.errorcode = eAnswerInfoGetResult_NoOpen;
            break;
        }
        AnswerActivityMgr *mgr = role->getAnswerActivityMgr();
        // 处理抽题
        mgr->pumpingTitle();
        // 处理超时题目
        mgr->procTimeout();
        
        ack.question_number = role->getAnswerCurrentNumber();
        ack.question_id = mgr->getQuestion(ack.question_number);
        ack.change_question_count = role->getAnswerChangeCount();
        ack.score = mgr->getTotalScore();
        ack.right_questions_count = mgr->getRightCount();
        ack.endTime = mgr->getEndTime();
        ack.surplus_num = mgr->getSurplusNum();
        ack.isOver = mgr->gameIsOver();
        ack.errorcode = eAnswerInfoGetResult_Success;
    }
    while (false);
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_answer_select, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_answer_select ack;
    ack.errorcode = eAnswerSelectResult_Success;

    
    AnswerActivityMgr *mgr = role->getAnswerActivityMgr();
    

    
    if (mgr->gameIsOver())
    {
        ack.isOver = mgr->gameIsOver();
        sendNetPacket(sessionid, &ack);
        return ;
    }
    
    if (role->getDailyScheduleSystem()) {
        role->getDailyScheduleSystem()->onEvent(kDailyScheduleAnswer, 0, 1);
    }

    
    ack.isTrue = mgr->isRightAnswer(req.answer);
    ack.next_question_id = mgr->getQuestion(role->getAnswerCurrentNumber());
    ack.right_questions_count = mgr->getRightCount();
    ack.score = mgr->getTotalScore();
    ack.endTime = mgr->getEndTime();
    ack.surplus_num = mgr->getSurplusNum();
    ack.isOver = mgr->gameIsOver();
    sendNetPacket(sessionid, &ack);
    
    // 游戏结束
    if (mgr->gameIsOver())
    {
        notify_answer_over notify_ack;
        notify_ack.errorcode = eAnswerNotifyResult_Success;
        if(!mgr->gameOverReward())
        {
            notify_ack.errorcode = eAnswerNotifyResult_BagBull;
        }
        notify_ack.total_score = mgr->getTotalScore();
        notify_ack.total_time = time(NULL) - role->getAnswerPumpingTime();
        notify_ack.right_questions_count = mgr->getRightCount();
        sendNetPacket(sessionid, &notify_ack);
    }
}}


hander_msg(req_answer_questionchange, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_answer_questionchange ack;
    ack.errorcode = eAnswerQuestionChangeResult_UnknowError;
    do
    {
        
        if (role->getAnswerActivityMgr()->gameIsOver())
        {
            ack.errorcode = eAnswerQuestionChangeResult_IsOver;
            break;
        }
        if (role->getVipLvl() <= 0)
        {
            ack.errorcode = eAnswerQuestionChangeResult_NoVip;
            break;
        }
        if (role->getAnswerChangeCount() >= AnswerActivityCfg::getMaxChangeCount())
        {
            ack.errorcode = eAnswerQuestionChangeResult_NoNum;
            break;
        }
        int cost = AnswerActivityCfg::getChangeQuestionCost(role->getAnswerChangeCount());
        if (role->CheckMoneyEnough(cost, eRmbCost, "答题换题") != CE_OK) {
            ack.errorcode = eAnswerQuestionChangeResult_NoCost;
            break;
        }
        // 换题
        ack.question_id = role->getAnswerActivityMgr()->changeQuestion();
        ack.endTime = role->getAnswerActivityMgr()->getEndTime();
        ack.change_question_count = role->getAnswerChangeCount();
        ack.errorcode = eAnswerQuestionChangeResult_Success;
    }
    while (false);
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_answer_totalrewards,req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_answer_totoalrewards ack;
    
    ack.is_reward1 = role->getAnswerCountNumReward();
    ack.day_count1 = role->getAnswerCountNum();
    
    ack.is_reward2 = role->getAnswerContinueNumReward();
    ack.day_count2 = role->getAnswerContinueNum();
    
    ack.is_reward3 = role->getAnswerContinueFunNumReward();
    ack.day_count3 = role->getAnswerContinueFunNum();
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_answer_getrewards, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_answer_getrewards ack;
    /*
     //获取奖励状态
     enum eGetRewardsResult
     {
     eGetRewardsResult_Ok,
     eGetRewardsResult_Fullbag,
     eGetRewardsResult_UnknowError,
     };
     */
    AnswerActivityMgr *mgr = role->getAnswerActivityMgr();
    ack.errorcode = mgr->okToGetReward(req.rewards_type);
    if (ack.errorcode != eGetRewardsResult_Ok)
    {
        sendNetPacket(sessionid, &ack);
        return;
    }

    
    
    sendNetPacket(sessionid, &ack);
}}







