//
//  SyncTeamCopy.h
//  GameSrv
//
//  Created by pireszhi on 14-7-16.
//
//

#ifndef __GameSrv__SyncTeamCopy__
#define __GameSrv__SyncTeamCopy__

#include <iostream>
#include "msg.h"
#include "ChatRoom.h"
using namespace std;

class Role;

struct TeamMember
{
    TeamMember():roleid(0),
                 guildid(0),
                 rolename(""),
                 battleForce(0),
                 roleType(0),
                 weaponFasion(0),
                 sessionid(0),
                 isCaptain(false),
                 beReady(false),
                 bodyfashion(0),
                 weaponmodel(0),
                 cityBodyFashion(0),
                 enchantId(0),
                 enchantLvl(0)
    {
        
    }
    int roleid;
    int guildid;
    int rolelvl;
    string rolename;

    int battleForce;
    int roleType;
    int weaponFasion;
    int bodyfashion;
    int weaponmodel;
    int cityBodyFashion;
    int enchantId;
    int enchantLvl;
    
    int sessionid;
    bool isCaptain;
    bool beReady;
};

class FightingTeam
{
    friend class SyncFightingTeamMgr;
public:
    void getFightingTeamMembers(vector<obj_fighting_team_member>& outdata);
    
    vector<TeamMember>& getFightingTeamMembers();
    
    bool isAllMemberReady();
    
    int canBeginBattle();
    
    int canChangeSceneMod(int scenemod);
    
    void changeSceneMod(int scenemod);
    
    bool isMemberFull();
    
    int canJoin(Role* role);
    
    void enterFightingScene();
    
    bool setReady(int roleid, bool ready);
    
    void sendTeamTalking(Role* sender, string content, VoiceInfoItem& voiceInfo);
    
    VoiceInfoItem getTalkingVoiceInfo(int64_t voiceId);
    
    void removeVoiceInfo();
    
protected:
    
    FightingTeam(int captainid, string captainName, int scenemod, int memCount);
    
    bool addMember(Role* newMember, bool iscaptain);
    
    bool leaveTeam(int roleid);
    
    void notifySomeOneJoin(TeamMember& member, int position);
    
    void notifySomeOneLeave(TeamMember& member, int position);
    
    void notifySetNewCaptain(TeamMember& newCaptain, int position);
    
    void notifyMemberState(TeamMember& member, int position);
    
    void sendNotifyToMember(INetPacket* packet, int filteredId = 0);
    
public:
    int mTeamId;
    int mCaptainId;
    int mCurrMemberCount;
    int mSceneMod;
    int mSceneInstId;
    string mCaptainName;
private:
    vector<TeamMember> mMembers;
    list<VoiceInfoItem> mVoiceInfo;
};

#define MAX_SYNC_TEAM_COUNT 4000

class SyncFightingTeamMgr
{
public:
    SyncFightingTeamMgr():mCurIdx(0)
    {
        
    }
    void init(int maxNum)
    {
        mCurNum = 0;
        mCurIdx = 0;
        mMaxNum = maxNum;
        mCapacity = 1;
        while (mCapacity < maxNum)
        {
            mCapacity <<= 1;
        }
        mTeams.resize(mCapacity, NULL);
    }
    //申请创建队伍
    FightingTeam* createTeam(Role* creater, int sceneMod, int memberNum);
    
    FightingTeam* findTeam(int teamid);
    
    //申请加入队伍
    int applyToJoinTeam(Role* applycant, FightingTeam* team);
    
    //退出队伍
    void leaveTeam(Role* role, int teamid);
    
    //进入战斗场景的回调
    bool onTeamEnterSceneToFight(int teamid, int sceneInstId);
    
    //取消准备
    void cancelReadyToFight(Role* role, int teamid);
    
    //踢人
    int kickOutFromTeam(Role* roleBekicked, int teamid);
    
    void sendTeamTalking(Role* sender, string message, VoiceInfoItem& voiceInfo);
    
protected:
    
    //解散队伍
    void dismissTeam(int teamid);
    
    int getGenTeamid()
    {
        assert(mCurNum < mMaxNum);
        
        int idx;
        for (;;)
        {
            mCurIdx &= (mCapacity - 1);
            idx = mCurIdx++;
            
            int slot = idx & (mCapacity - 1);
            if (mTeams[slot] == NULL)
            {
                break;
            }
        }
        
        int instId = idx + 1;
        
        return instId;
    }
    
private:
    vector<FightingTeam*> mTeams;
    int mCurIdx;
    int mCapacity;
    int mCurNum;
    int mMaxNum;
};

extern SyncFightingTeamMgr g_SyncFightingTeamMgr;

struct SyncTeamCopyGuildMemberIncomeState
{
    SyncTeamCopyGuildMemberIncomeState():rolename(""),
                                         incomeTimes(0)
    {
        
    }
    string rolename;
    int incomeTimes;
};

class SyncTeamCopyMgr
{
public:
    
    SyncTeamCopyMgr()
    {
        mTeams.clear();
        mPlayersSessionId.clear();
    }
    
    //获取队伍列表
    void getTeamList(vector<obj_sync_team_info>& teamList);
    
    //在列表添加新的队伍
    bool addNewTeam(int teamid);
    
    bool isTeamInList(int teamid);
    
    //
    void delTeamInList(int teamid);
    
    //打开界面
    void enterActivity(int sessionid);
    
    //离开界面
    void leaveActivity(int sessionid);
    
    bool isOpenActivity(int sessionid);
    
    void notifyTeamInfoChange(int teamid);
    
    void sendNotifyToPalyers(INetPacket* packet);
    
    void fightingResultHandle(int teamid, int result);
private:
    list<int> mTeams;                    //队伍的队长列表
    
    list<int> mPlayersSessionId;                      //打开界面的玩家
};
extern SyncTeamCopyMgr g_SyncTeamCopyMgr;

#endif /* defined(__GameSrv__SyncTeamCopy__) */
