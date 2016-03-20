
#include "msg.h"
#include "NetPacket.h"
#include "sendMessage.h"
#include "serverList.h"
#include "EnumDef.h"
#include "Defines.h"
#include "csPvp/csPvp.h"
#include "configManager.h"
#include "serverEntity.h"
#include "serverList.h"
#include "centerScript.h"
#include "centerLog.h"

extern CenterLog createCsPvpBattleLog();

int winCsPvpScore(int selfRank, int targetRank)
{
	CMsgTyped param;
	param.SetInt(selfRank);
	param.SetInt(targetRank);
	if (CenterScript::instance()->call("winCsPvpScore", param) != 1) {
		return 0;
	}

	int score = 0;
	try {
		score = param.FloatVal();
	} catch (...) {

	}

	return score;
}

int failCsPvpScore(int selfRank, int targetRank)
{
	CMsgTyped param;
	param.SetInt(selfRank);
	param.SetInt(targetRank);
	if (CenterScript::instance()->call("failCsPvpScore", param) != 1) {
		return 0;
	}

	int score = 0;
	try {
		score = param.FloatVal();
	} catch (...) {

	}

	return score;
}

int finishCsPvpScore(int rank)
{
	CMsgTyped param;
	param.SetInt(rank);
	if (CenterScript::instance()->call("finishCsPvpScore", param) != 1) {
		return 0;
	}

	int score = 0;
	try {
		score = param.FloatVal();
	} catch (...) {

	}

	return score;
}

////
//  战斗中相关
///
handler_msg(req_cs_cspvp_add_role, req)
{
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
	if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	}

	//准备状态不修改竞技场信息
	if (csPvp->getState() != CsPvp::kInBattle) {
	//	return;
	}


	PvpRoleId pvpRoleId = genPvpRoleId(req.server_id, req.role_id);
	PvpRoleRankList* rankList = csPvp->getPvpRoleRankList();
	rankList->updateRoleInfo(pvpRoleId, req.role_name, req.role_type, req.lvl, req.bat);
}}


handler_msg(req_cs_get_cspvp_challenge_data, req)
{
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
    if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	}

	int roleId = req.role_id;
	int serverId = req.server_id;
	PvpRoleId pvpRoleId = genPvpRoleId(serverId, roleId);

	PvpRoleRankList* rankList = csPvp->getPvpRoleRankList();
	PvpRoleInfo* roleInfo = rankList->getRoleInfoByRoleId(pvpRoleId);
	assert(roleInfo);

	int roleRank = roleInfo->mRank;
	vector<PvpRoleInfo*> beChallenges = csPvp->getChallengeData(roleRank);
	ack_cspvp_challengedata ack;
	ack.setSession(req.getSession());
	for (int i = 0; i < beChallenges.size(); i++) {
		PvpRoleInfo* beChallenged = beChallenges[i];

		obj_crossservicewar_role info;
		info.serverid = getPvpRoleServerId(beChallenged->mRoleId);
		info.roleid = getPvpRoleRoleId(beChallenged->mRoleId);
		info.rank = beChallenged->mRank;
		info.rolename = beChallenged->mRoleName;
		info.lvl = beChallenged->mLvl;
		info.roletype = beChallenged->mRoleType;
		ack.roles.push_back(info);
	}

	sendNetPacket(sessionid, &ack);
}}

handler_msg(req_cs_get_cspvp_roleinfo, req)
{
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
    if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	};

	int serverId = req.server_id;
	int roleId = req.role_id;
	PvpRoleId pvpRoleId = genPvpRoleId(serverId, roleId);

	PvpRoleRankList* rankList = csPvp->getPvpRoleRankList();
	PvpRoleInfo* roleInfo = rankList->getRoleInfoByRoleId(pvpRoleId);
	if (roleInfo == NULL) {
		log_error("get role info error");
		return;
	}

	ack_cspvp_roleinfo ack;
	ack.errorcode = 0;
	ack.myrank = roleInfo->mRank;
	ack.score = roleInfo->mScore;
	ack.awardrank = 1;
	ack.collingtime = req.colddown;
	ack.costperminute = req.costperminute;
	ack.setSession(req.getSession());
	sendNetPacket(sessionid, &ack);
}}

handler_msg(req_cs_get_cspvp_log, req)
{
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
    if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	}

	class LogTraverser : public Traverser
	{
	public:
		LogTraverser(PvpRoleId roleId, int num, ack_cspvp_log* ack) {
			mAck = ack;
			mRoleId = roleId;
            mNum = num;
		}

		virtual bool callback(void* traverseobj) 
		{
			if (mNum <= 0) {
				return false;
			}
			mNum--;

			PvpLog* pvpLog = (PvpLog*)traverseobj;

			obj_crossservicewar_log info;
			if (mRoleId == pvpLog->mRoleId) {
				info.rolename = pvpLog->mTargetName;
				info.iswin = pvpLog->mIsWin;
				info.isbechallenge = 0;
				info.rank = pvpLog->mRoleRank;
				if (pvpLog->mSwapRank) {
					info.changetype = 1;
				}
			} else {
				info.rolename = pvpLog->mRoleName;
				info.iswin = !pvpLog->mIsWin;
				info.isbechallenge = 1;
				if (pvpLog->mSwapRank) {
					info.changetype = 2;
				}
				info.rank = pvpLog->mTargetRank;
			}
			mAck->logs.push_back(info);
			return true;
		}

		ack_cspvp_log* mAck;
		PvpRoleId mRoleId;
		int mNum;
	};

	PvpRoleId pvpRoleId = genPvpRoleId(req.server_id, req.role_id);
	ack_cspvp_log ack;
	ack.setSession(req.getSession());
	LogTraverser traverser(pvpRoleId, req.num, &ack);

	PvpLogMgr* logMgr = csPvp->getPvpLogMgr();
	logMgr->traverseRoleLog(pvpRoleId, &traverser);
	sendNetPacket(sessionid, &ack);
}}


handler_msg(req_cs_begin_cspvp, req)
{
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
    if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	}

	ack_cs_begin_cspvp ack;
	ack.setSession(req.getSession());
	ack.errorcode = 1;
	ack.src_server_id = req.src_server_id;
	ack.src_role_id = req.src_role_id;
	ack.target_server_id = req.target_server_id;
	ack.target_role_id = req.target_role_id;
	do {
		PvpStateMgr* stateMgr = csPvp->getPvpStateMgr();
		PvpRoleId srcId = genPvpRoleId(req.src_server_id, req.src_role_id);
		if (stateMgr->isChallenging(srcId)) {
			break;
		}
		PvpRoleId targetId = genPvpRoleId(req.target_server_id, req.target_role_id);
		if (stateMgr->isBeChallenged(targetId)) {
			break;
		}
		stateMgr->startPvp(srcId, targetId);

		ack.errorcode = 0;
	} while (0);

	sendNetPacket(sessionid, &ack);
}}

handler_msg(req_cs_cancel_cspvp, req)
{
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
    if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	}

	PvpStateMgr* stateMgr = csPvp->getPvpStateMgr();
	PvpRoleId srcId = genPvpRoleId(req.server_id, req.role_id);
	stateMgr->cancelPvp(srcId);
}}

handler_msg(req_cs_cspvp_result, req)
{	
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
	if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	}

	ack_cs_cspvp_result ack;
	ack.setSession(req.getSession());
	ack.errorcode = 1;
	ack.iswin = req.iswin;
	ack.src_role_id = req.src_role_id;
	ack.src_server_id = req.src_server_id;

	int serverId = req.src_server_id;
	int roleId = req.src_role_id;
	do {
		//结束pvp状态
		PvpRoleId pvpRoleId = genPvpRoleId(serverId, roleId);
		PvpStateMgr* stateMgr = csPvp->getPvpStateMgr();
		PvpRoleId beChallengedId = stateMgr->getBeChallengedRole(pvpRoleId);
		if (beChallengedId == 0) {
			break;
		}
		stateMgr->endPvp(pvpRoleId);

		//个人排行处理
		PvpRoleRankList* roleRank = csPvp->getPvpRoleRankList();

		PvpRoleInfo* roleInfo = roleRank->getRoleInfoByRoleId(pvpRoleId);
		PvpRoleInfo* targetInfo = roleRank->getRoleInfoByRoleId(beChallengedId);

		vector<PvpRoleInfo*> saveInfos;
		bool swapRank = false;
		int score = 0;
		if (req.iswin) {
			score = winCsPvpScore(roleInfo->mRank, targetInfo->mRank);
			if (roleInfo->mRank > targetInfo->mRank) {
				saveInfos.push_back(targetInfo);
				roleRank->swapRank(roleInfo, targetInfo);
				ack.isrankup = 1;
				swapRank = true;
			}
		} else {
			score = failCsPvpScore(roleInfo->mRank, targetInfo->mRank);
		}
		roleInfo->mScore += score;
		saveInfos.push_back(roleInfo);
		roleRank->savePvpRoleInfo(saveInfos);

		ack.score = score;
		ack.rank = roleInfo->mRank;

		//服务器排行
		PvpServerRankList* serverRank = csPvp->getPvpServerRankList();
		serverRank->addScore(serverId, score);

		PvpLogMgr* logMgr = csPvp->getPvpLogMgr();
		logMgr->addLog(pvpRoleId, roleInfo->mRoleName, roleInfo->mRank, 
			beChallengedId, targetInfo->mRoleName, targetInfo->mRank, 
			req.iswin, swapRank);
		ack.errorcode = 0;

	} while (0);

	sendNetPacket(sessionid, &ack);
}}


handler_msg(req_cs_cspvp_get_rolerank, req)
{
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
	if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	}

	int startRank = req.start_rank;
	int endRank = req.end_rank;

	ack_get_cspvp_rolerank ack;
	ack.setSession(req.getSession());
	ack.errorcode = 0;
	ack.start_rank = startRank;
	ack.end_rank = endRank;

	PvpRoleRankList* roleRank = csPvp->getPvpRoleRankList();
	for (int i = startRank; i <= endRank; i++) {
		PvpRoleInfo* roleInfo = roleRank->getRoleInfoByRank(i);
		if (roleInfo == NULL) {
			break;
		}

		int serverId = getPvpRoleServerId(roleInfo->mRoleId);
		int roleId = getPvpRoleRoleId(roleInfo->mRoleId);

		ServerEntity* serverEntity = getServerList()->getServer(serverId);
		string serverName = "NULL";
		if (serverEntity) {
			serverName = serverEntity->getName();
		}

		obj_cspvp_rolerank_member info;
    	info.rank = i;
    	info.roleId = roleId;
    	info.rolename = roleInfo->mRoleName;
    	info.servername = serverName;
    	info.serverid = serverId;
    	info.accunum = roleInfo->mScore;
		ack.info.push_back(info);
	}
	sendNetPacket(sessionid, &ack);
}}


handler_msg(req_cs_cspvp_get_serverrank, req)
{
	ServerEntity* server = getServerList()->getServerBySession(sessionid);
	if (server == NULL) {
		return;
	}

	CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
	if (csPvp == NULL) {
		return;
	}

	int startRank = req.start_rank;
	int endRank = req.end_rank;

	ack_get_cspvp_serverrank ack;
	ack.setSession(req.getSession());
	ack.errorcode = 0;
	ack.start_rank = startRank;
	ack.end_rank = endRank;

	PvpServerRankList* serverRank = csPvp->getPvpServerRankList();
	for (int i = startRank; i <= endRank; i++) {
		PvpServerInfo* serverInfo = serverRank->getServerInfoByRank(i);
		if (serverInfo == NULL) {
			break;
		}

		int serverId = serverInfo->mServerId;
		ServerEntity* serverEntity = getServerList()->getServer(serverId);
		string serverName = "NULL";
		if (serverEntity) {
			serverName = serverEntity->getName();
		}

		obj_cspvp_serverrank_member info;
		info.rank = i;
		info.serverid = serverInfo->mServerId;
		info.servername = serverName;
		info.score = serverInfo->mScore;
		ack.info.push_back(info);
	}
	sendNetPacket(sessionid, &ack);
}}

