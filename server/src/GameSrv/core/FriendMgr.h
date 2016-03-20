//
//  FriendMgr.h
//  GameSrv
//
//  Created by prcv on 13-4-12.
//
//  好友管理

#ifndef __GameSrv__FriendMgr__
#define __GameSrv__FriendMgr__

#include <set>
#include <map>
#include <string>
#include <vector>
#include "ItemBag.h"
#include "DataCfg.h"
#include "basedef.h"
using namespace std;

class Role;

struct Friend
{
    // 好友角色ID
    int mFid;
    // 亲密度
    int mIntimacy;
    int BattleForce;
    int roletype;
    int rolelvl;
    bool isonline;
    string rolename;

    Friend(){}
    Friend(int id, int intimacy=0):mFid(id), mIntimacy(intimacy){}
};

class FriendMgr
{
public:
    typedef map<int, Friend> FriendMap;
    typedef map<int, Friend>::iterator FriendMapIter;
    
public:
    bool loadFriendList(int roleid);
    bool addNewFriend(int friendid);
    bool delFriend(int friendid);
    bool isFriend(int targetid); 
    
    void traverse(Traverser* traverser);

    void procAddIntimacy(Role *role, int friendId, int copyId);
	void procAddIntimacyIncr(Role *role, int friendId, int incr);

    static void saveFriend(int roleid, Friend &f);
    static void removeFriend(int roleid, int friendid);

    // 获取副本亲密度加成比例
    float getIntimacyCopyRatio(int fid);
    void intimacyReward(int fid, SceneCfgDef *def, RewardStruct &reward);

	// 获取好友亲密度总和
	int friendIntimacySum();
    int getFrinedIntimacy(int friendid);
	
    map<int, Friend>& getFriends() {return friends;}
    Friend* getFriend(int friendid);
    int getFriendCount() {return friends.size();}
    void getOnlineFriends(std::vector<int>& friendlist);
private:
    static void updateOldFriendData(int roleid);
    static bool addIntimacy(Friend &f, Role *role, int intimacy);
    // 修改好友数据源亲密度
    static void editFriendIntimacyForDb(Role *role, int roleid, int friendid, int intimacy);

    int addFriendIntimacy(Role *role, int friendid, int intimacy);
private:
    map<int, Friend> friends;
    
    int mRobotFriendCount;
};



class TeamCopyMgr
{
public:
    bool IsCopyFinish(int copyid);
    bool IsFriendFighted(int friendid);
    bool loadTeamCopyData(int roleid);

    const set<int>& getFinishCopy();
    const set<int>& getFightedFriendList();

    bool addFightedFriend(int roleid, int friendid);
    bool addFinishCopyid(int roleid, int copyid);

    bool DelCopyFromFinish(int roleid, int copyid);

    void TeamCopyReset(int roleid, int sessionid);
    void clearFightedFriends(int roleid);
private:
    set<int> mFinishCopy;
    set<int> mFightedFriends;
};

#endif /* defined(__GameSrv__FriendMgr__) */
