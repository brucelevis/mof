//
//  FriendMgr.cpp
//  GameSrv
//
//  Created by prcv on 13-4-12.
//
//

#include "FriendMgr.h"
#include "main.h"
#include "Activity.h"
#include "EnumDef.h"
#include "cmd_def.h"
#include "GameLog.h"
#include "redis_helper.h"
#include "Role.h"
#include "Robot.h"

bool FriendMgr::loadFriendList(int roleid)
{
    friends.clear();
    mRobotFriendCount = 0;
    
    // 容错
    updateOldFriendData(roleid);

    RedisResult result(redisCmd("hvals friends:%d", roleid));
    int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i++)
    {
		string str = result.readHash(i, "");
		if (str.empty()) {
			continue;
		}
		StringTokenizer token(str, ",");
		if (token.count() < 2) {
			xyerr("[好友] 存储的数据格式有误%d %s", roleid, str.c_str());
			continue;
		}
		int friendId = Utils::safe_atoi(token[0].c_str(), 0);
		int intimacy = Utils::safe_atoi(token[1].c_str(), 0);
		
		Friend f(friendId, intimacy);
        
        if (friendId == RobotCfg::sFriendId) {// && getObjectType(friendId) == kObjectRobot) {
            mRobotFriendCount++;
            
            Robot* robot = g_RobotMgr.getData(friendId);
            if (robot) {
                f.rolename = robot->mRoleName;
                f.rolelvl = robot->mBattleProp.getLvl();
                f.roletype = robot->mRoleType;
                f.isonline = true;
                f.BattleForce = robot->mBattle;
            }
        } else {
            Role *friendrole = SRoleMgr.GetRole(friendId);
            if (friendrole) {
                f.rolename = friendrole->getRolename();
                f.rolelvl = friendrole->getLvl();
                f.roletype = friendrole->getRoleType();
                f.isonline = true;
                f.BattleForce = friendrole->getBattleForce();
            } else {
                RedisResult roleinfo(redisCmd("hmget role:%d rolename lvl roletype bat", friendId));

                string rolename = roleinfo.readHash(0, "");
                int rolelvl = roleinfo.readHash(1, 0);
                int roletype = roleinfo.readHash(2, 0);
                int battleForce = roleinfo.readHash(3, 0);
                
                if (rolename.empty() || rolelvl == 0) {
                    doRedisCmd("hdel friends:%d fid:%d", roleid, friendId);
                    continue;
                }

                f.rolename = rolename;
                f.rolelvl = rolelvl;
                f.roletype = roletype;
                f.isonline = false;
                f.BattleForce = battleForce;
            }
        }
        friends.insert(std::make_pair(f.mFid, f));
    }
    
    if (mRobotFriendCount == 0 && RobotCfg::sFriendId != 0) {
        Robot* robot = g_RobotMgr.getData(RobotCfg::sFriendId);
        if (robot) {
            Friend f(RobotCfg::sFriendId, 0);
            f.rolename = robot->mRoleName;
            f.rolelvl = robot->mBattleProp.getLvl();
            f.roletype = robot->mRoleType;
            f.isonline = true;
            f.BattleForce = robot->mBattle;
            friends.insert(std::make_pair(f.mFid, f));
            saveFriend(roleid, f);
        }
    }
    
    return true;
}

void FriendMgr::traverse(Traverser *traverser)
{
    map<int, Friend>::iterator iter;
    for (iter = friends.begin(); iter != friends.end(); iter++) {
        Friend* pFriend = &(iter->second);
        if (!traverser->callback(pFriend)) {
            break;
        }
    }
}

void FriendMgr::updateOldFriendData(int roleid)
{
    redisReply* typeResult = redisCmd("type friends:%d", roleid);
    if (typeResult == NULL)
    {
        return;
    }
    string type = typeResult->str;
    freeReplyObject(typeResult);

    if(type == "set")
    {
        redisReply* reply = redisCmd("smembers friends:%d", roleid);
        if (reply == NULL)
        {
            return;
        }
        // del player friends key
        RedisResult delResult(redisCmd("del friends:%d", roleid));
        for (int i = 0; i < reply->elements; i++)
        {
            Friend f(atoi(reply->element[i]->str));
            // save friend
            saveFriend(roleid, f);
        }
        freeReplyObject(reply);
    }
}

bool FriendMgr::addNewFriend(int friendid)
{
    if (friends.find(friendid) != friends.end())
    {
        return true;
    }
    Friend f(friendid);

    friends.insert(std::make_pair(friendid, f));
    return true;
}

bool FriendMgr::delFriend(int friendid)
{
    map<int, Friend>::iterator iter = friends.find(friendid);
    if (iter != friends.end())
    {
        friends.erase(iter);
        return true;
    }
    return false;
}

bool FriendMgr::isFriend(int targetid)
{
    return friends.find(targetid) != friends.end();
}

bool FriendMgr::addIntimacy(Friend &f, Role *role, int intimacy)
{
    RoleCfgDef rolecfg = RoleCfg::getCfg(role->getJob(), role->getLvl());
    if(f.mIntimacy == rolecfg.getMaxfriendly()) return false;

    f.mIntimacy += intimacy;

    if(f.mIntimacy >= rolecfg.getMaxfriendly())
    {
        f.mIntimacy = rolecfg.getMaxfriendly();
    }
    return true;
}

void FriendMgr::procAddIntimacyIncr(Role *role, int friendId, int incr)
{
	int roleId = role->getInstID();
	
	notify_edit_intimacy notify_self;
    notify_self.friendid = friendId;
    notify_self.intimacy = addFriendIntimacy(role, friendId, incr);
    sendNetPacket(role->getSessionId(), &notify_self);
    
    //if (getObjectType(friendId) == kObjectRobot) {
    if (friendId == RobotCfg::sFriendId) {
        return;
    }
    
    Role *tarrole = SRoleMgr.GetRole(friendId);
    if (tarrole != NULL)
    {
        notify_edit_intimacy notify_tarrole;
        notify_tarrole.friendid = roleId;
        notify_tarrole.intimacy = tarrole->getFriendMgr()->addFriendIntimacy(tarrole, roleId, incr);
        sendNetPacket(tarrole->getSessionId(), &notify_tarrole);
    }
    else
    {
        editFriendIntimacyForDb(role, friendId, roleId, incr);
    }
}


void FriendMgr::procAddIntimacy(Role *role, int friendId, int copyId)
{
    SceneCfgDef *scenecfg = SceneCfg::getCfg(copyId);
    if (NULL == scenecfg) {
        return;
    }
	procAddIntimacyIncr(role, friendId, scenecfg->incr_friendly);
}

int FriendMgr::addFriendIntimacy(Role *role, int friendid, int intimacy)
{
    map<int, Friend>::iterator it = friends.find(friendid);
    if (it != friends.end())
    {
        if(addIntimacy(it->second, role, intimacy))
        {
            saveFriend(role->getInstID(), it->second);
        }
        return it->second.mIntimacy;
    }
    return 0;
}

void FriendMgr::editFriendIntimacyForDb(Role *role, int roleid, int friendid, int intimacy)
{
    // 容错
    updateOldFriendData(roleid);

    RedisResult result(redisCmd("hmget friends:%d fid:%d", roleid, friendid));
	string str = result.readHash(0, "");
	if (str.empty()) {
		return;
	}
	StringTokenizer token(str, ",");
	if (token.count() < 2) {
		xyerr("[好友] 存储的数据格式有误%d %d %s", roleid, friendid, str.c_str());
		return;
	}
	int friendId = Utils::safe_atoi(token[0].c_str(), 0);
	int fintimacy = Utils::safe_atoi(token[1].c_str(), 0);
	Friend f(friendId, fintimacy);
	
    if(addIntimacy(f, role, intimacy))
    {
        saveFriend(roleid, f);
    }
}

void FriendMgr::saveFriend(int roleid, Friend &f)
{
    vector<int> v;
    v.push_back(f.mFid);
    v.push_back(f.mIntimacy);

    string valueStr = StrJoin(v.begin(), v.end(), ",");

    doRedisCmd("hmset friends:%d fid:%d %s", roleid, f.mFid, valueStr.c_str());
}

void FriendMgr::removeFriend(int roleid, int friendid)
{
    doRedisCmd("hdel friends:%d fid:%d", roleid, friendid);
}

float FriendMgr::getIntimacyCopyRatio(int fid)
{
    Friend *f = getFriend(fid);
    if(NULL == f) return 0;
    FriendRewardDef *frd = FriendRewardCfg::getRankAward(f->mIntimacy);
    if (NULL == frd) return 0;
    return frd->mReward;
}

void FriendMgr::intimacyReward(int fid, SceneCfgDef *def, RewardStruct &reward)
{
    Friend *f = getFriend(fid);
    if (NULL == f) return;
    FriendRewardDef *frd = FriendRewardCfg::getRankAward(f->mIntimacy);
    if (NULL == frd) return;
    
    reward.reward_exp += ceil(def->copy_exp * frd->mRewardExp);
    reward.reward_gold += ceil(def->copy_gold * frd->mRewardGold);
    reward.reward_batpoint += ceil(def->copy_battlepoint * frd->mRewardBattlePoint);
    reward.reward_petexp += ceil(def->pet_exp * frd->mRewardPetExp);
}

int FriendMgr::friendIntimacySum()
{
	int sum = 0;
	map<int, Friend>::iterator it;
    for (it = friends.begin(); it != friends.end(); it++) {
		sum += it->second.mIntimacy;
	}
	return sum;
}

int FriendMgr::getFrinedIntimacy(int friendid)
{
    map<int, Friend>::iterator it = friends.find(friendid);
    if (it != friends.end()) {
        return it->second.mIntimacy;
    }
    return 0;
}

Friend* FriendMgr::getFriend(int friendid)
{
    map<int, Friend>::iterator it = friends.find(friendid);
    if (it == friends.end())
    {
        return NULL;
    }
    return &it->second;
}

void FriendMgr::getOnlineFriends(std::vector<int>& friendlist)
{
    friendlist.clear();
    map<int, Friend>::iterator iter = friends.begin();
    for (; iter != friends.end(); iter++) {
        Role* role = SRoleMgr.GetRole(iter->first);
        if(role)
        {
            friendlist.push_back(iter->first);
        }
    }
}

bool TeamCopyMgr::loadTeamCopyData(int roleid)
{
    mFinishCopy.clear();
    mFightedFriends.clear();
    char buf[256];
    sprintf(buf, "smembers fightedfriends:%d", roleid);

    redisReply* reply = redisCmd(buf);
    if (reply == NULL)
    {
        return false;
    }

    for (int i = 0; i < reply->elements; i++)
    {
        mFightedFriends.insert(atoi(reply->element[i]->str));
    }
    freeReplyObject(reply);

    snprintf(buf, sizeof(buf), "smembers teamcopy:%d", roleid);

    reply = redisCmd(buf);
    if (reply == NULL)
    {
        return false;
    }

    for (int i = 0; i < reply->elements; i++)
    {
        mFinishCopy.insert(atoi(reply->element[i]->str));
    }

    freeReplyObject(reply);
    return true;
}

bool TeamCopyMgr::IsCopyFinish(int copyid)
{
    if (mFinishCopy.find(copyid) == mFinishCopy.end()) {
        return false;
    }

    return true;
}

bool TeamCopyMgr::IsFriendFighted(int friendid)
{
    if (mFightedFriends.find(friendid) == mFightedFriends.end()) {
        return false;
    }

    return true;
};

const set<int>& TeamCopyMgr::getFinishCopy()
{
    return mFinishCopy;
}

const set<int>& TeamCopyMgr::getFightedFriendList()
{
    return mFightedFriends;
}

bool TeamCopyMgr::addFightedFriend(int roleid, int friendid)
{
    if (mFightedFriends.find(friendid) != mFightedFriends.end()) {
        return false;
    }

    mFightedFriends.insert(friendid);

    char buf[256];
    sprintf(buf, "sadd fightedfriends:%d %d", roleid, friendid);

    doRedisCmd(buf);
    return true;
}

bool TeamCopyMgr::addFinishCopyid(int roleid, int copyid)
{
    if (mFinishCopy.find(copyid) != mFinishCopy.end()) {
        return false;
    }

    mFinishCopy.insert(copyid);

    char buf[256];
    sprintf(buf, "sadd teamcopy:%d %d", roleid, copyid);

    doRedisCmd(buf);
    return true;
}

void TeamCopyMgr::clearFightedFriends(int roleid)
{
    mFightedFriends.clear();
    char buf[256];
    sprintf(buf, "del fightedfriends:%d", roleid);

    doRedisCmd(buf);
}

void TeamCopyMgr::TeamCopyReset(int roleid, int sessionid)
{
    mFinishCopy.clear();
    mFightedFriends.clear();

    notify_sys_reset_teamcopy notify;
    sendNetPacket(sessionid, &notify);
}

bool TeamCopyMgr::DelCopyFromFinish(int roleid, int copyid)
{
    std::set<int>::iterator iter = mFinishCopy.find(copyid);

    if (iter == mFinishCopy.end()) {
        return false;
    }
    else{
        mFinishCopy.erase(iter);
        doRedisCmd("srem teamcopy:%d %d", roleid, copyid);
        return true;
    }

    return false;
}
