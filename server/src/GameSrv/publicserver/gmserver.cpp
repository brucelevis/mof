//
//  gmserver.cpp
//  GameSrv
//
//  Created by prcv on 13-7-23.
//
//

#include "gmserver.h"
#include "MsgBuf.h"
#include "main.h"
#include "psmgr.h"
#include "BroadcastNotify.h"
#include "main.h"
#include "DBRolePropName.h"
#include "Role.h"
#include "GameLog.h"
#include "Paihang.h"
#include "Pvp.h"

extern void appCmdResponse(const string &data);

void GmServer::onResponse(int receiver, int cmdId, CMsgTyped* msg)
{
    switch (cmdId) {
        case GM_BAN_ROLE:
        {
            int roleid = msg->IntVal();
            Role* role = SRoleMgr.GetRole(roleid);
            if (role)
            {
                SRoleMgr.disconnect(role);
            }
            doRedisCmd("hset role:%d %s %d", roleid, "banrole", 1);
            
			// 移除竞技场与排行榜数据
			int delpvprank = msg->IntVal();
			int petid = 0;
			if (delpvprank == 1) {
				RedisResult result(redisCmd("hget role:%d %s", roleid, GetRolePropName(eRoleActivePetId)));
				petid = result.readInt();
				
				SSortMgr.safe_delPetInPaihang(petid);
				SSortMgr.safe_delRoleInPaihang(roleid);
				SPvpMgr.safe_DeleteRoleInpvp(roleid);
			}
			
            Xylog xyLog(eLogName_GmCommand, 0);
            xyLog << "banrole" << roleid << delpvprank << petid;
            break;
        }
        case GM_ALLOW_ROLE:
        {
            int roleid = msg->IntVal();
            doRedisCmd("hset role:%d %s %d", roleid, "banrole", 0);
            
            Xylog xyLog(eLogName_GmCommand, 0);
            xyLog << "allowrole" << roleid;
            
            break;
            
        }
        case GM_BAN_CHAT:
        {
            int roleid = msg->IntVal();
            Role* role = SRoleMgr.GetRole(roleid);
            if (role)
            {
                role->setBanChat(1);
            }
            doRedisCmd("hset role:%d %s %d", roleid, GetRolePropName(eRolePropBanchat), 1);
            break;
        }
        case GM_ALLOW_CHAT:
        {
            int roleid = msg->IntVal();
            Role* role = SRoleMgr.GetRole(roleid);
            if (role)
            {
                role->setBanChat(0);
            }
            doRedisCmd("hset role:%d %s %d", roleid, GetRolePropName(eRolePropBanchat), 0);
            break;
            
        }
        case GM_NOTIFY:
        {
            string content = msg->StringVal();
            broadcastSystem(content);
            break;
        }
        case GM_KICK_ROLE:
        {
            int roleid = msg->IntVal();
            Role* role = SRoleMgr.GetRole(roleid);
            if (role)
            {
                SRoleMgr.disconnect(role);
            }
            break;
        }
        case GM_SET_ROLE_PROP:
        {
            int roleid = msg->IntVal();
            string name = msg->StringVal();
            string value = msg->StringVal();
            
            RedisResult ret(redisCmd("exists role:%d", roleid));
            if (ret.readInt() == 1)
            {
                doRedisCmd("hset role:%d %s %s", roleid, name.c_str(), value.c_str());
            }
            break;
        }
		case GM_APP_CMD :
		{
			string data = msg->StringVal();
			appCmdResponse(data);
            
            Xylog xyLog(eLogName_GmCommand, 0);
            xyLog << "appcmd" << data;
            
			break;
		}
        default:
            break;
    }
}
