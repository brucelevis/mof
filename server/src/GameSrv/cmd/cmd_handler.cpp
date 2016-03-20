//
//  cmd.cpp
//  GameSrv
//
//  Created by prcv on 13-5-2.
//
//
#include "cmd_def.h"

#include <cstring>
#include <string>

#include "Role.h"
#include "SessionData.h"
#include "gate.h"
#include "json/json.h"
#include "curl_helper.h"
#include "GameLog.h"
#include "Game.h"
#include "process.h"
#include "msg.h"
#include "DynamicConfig.h"
#include "HttpRequestConfig.h"
#include "psmgr.h"
#include "DynamicConfig.h"
#include "GameScript.h"
#include "sdk_inc.h"
#include "gate_mod.h"
#include "ip_filter.h"
#include "ServerInfoMgr.h"
#include "WebStateMgr.h"
#include "centerClient.h"
using namespace std;

extern void SendDailyQuest(Role* role);

handler_cmd(connect)
{
    bool issuperip = g_IpFilterMgr.isFiltered("super", ip.c_str());
    
    if (!issuperip)
    {
        bool shouldKick = false;
        do
        {
            if (Process::env.getInt("starttime") > Game::tick)
            {
                notify_disconnect_tip tip;
                tip.tip = Process::env.getString("starttip");
                sendNetPacket(sid, &tip);
                
                shouldKick = true;
                break;
            }
            
            if (!g_IpFilterMgr.isFiltered("district_whitelist", ip.c_str()) &&
                g_IpFilterMgr.isFiltered("district_blacklist", ip.c_str()))
            {
                notify_disconnect_tip tip;
                tip.tip = "server is not open to your district";
                sendNetPacket(sid, &tip);
                
                shouldKick = true;
				break;
            }
			
			// 检查web状态,　若非正常状态,　则不让通过
			// 只对配置好的完整服务器信息才有效,　数据库不存在对应服务器信息的,　这里不控制
			if (g_WebStateMou.isState()) {
				// 若是白名单ip则通过
				if (g_WebStateMou.isSafeIp(ip.c_str()))
				{
					break;
				}

				notify_disconnect_tip tip;
                tip.tip = g_WebStateMou.getTips();
                sendNetPacket(sid, &tip);
                
                shouldKick = true;
				break;
			}
        }
        while (0);
        
        if (shouldKick)
        {
            create_cmd(kickSession, kickSession);
            kickSession->sid = sid;
            sendMessageToGate(0, CMD_MSG, kickSession, 0);
            return;
        }
    }
    
    SSessionDataMgr.newData(sid);
    SSessionDataMgr.setProp(sid, "ip", ip);
}

handler_cmd(disconnect)
{
    int roleid = SRoleMgr.Session2RoleId(sid);
    Role* role = SRoleMgr.GetRole(roleid);
    if (role)
    {
        SRoleMgr.destroyRole(role);
        
        log_info("玩家断开连接");
    }

    SSessionDataMgr.delData(sid);
}

handler_cmd(dailyquest)
{
    Role* role = SRoleMgr.GetRole(roleid);
    if (role)
    {
        SendDailyQuest(role);
    }
}

handler_cmd(enterGroup)
{
    SGroupMgr.enterGroup(groupid, sid);
}

handler_cmd(leaveGroup)
{
    SGroupMgr.leaveGroup(groupid, sid);
}

handler_cmd(kickSession)
{
    Gate* gate = ((GateMod*)obj)->getGate();
    gate->close_connection(sid);
}


handler_cmd(login91ret)
{
    ack_login_91 ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdk91::getName());
        SSessionDataMgr.setProp(sid, "os", "ios");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login91androidret)
{
    ack_login_91_android ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdk91::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(loginppsret)
{
    ack_login_pps ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdkpps::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(logintbret)
{
    ack_login_tb ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        
        //****同步推用户名切换
        //
        RedisResult result(redisCmd("exists player:%s", account.c_str()));
        if (result.readInt() == 0) {
            result.setData(redisCmd("hget player:%s roleids", oldaccount.c_str()));
            
            string roleIdStr = result.readStr();
            if (!roleIdStr.empty()) {
                doRedisCmd("rename player:%s player:%s", oldaccount.c_str(), account.c_str());
                //修改角色的playeraccount
                vector<string> roleIds = StrSpilt(roleIdStr, ";");
                for (int i = 0; i < roleIds.size(); i++) {
                    int roleid = Utils::safe_atoi(roleIds[i].c_str());
                    result.setData(redisCmd("hget role:%d playeraccount", roleid));
                    if (result.readStr() == oldaccount) {
                        doRedisCmd("hset role:%d playeraccount %s", roleid, account.c_str());
                    }
                }
            }
        }
        
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdktb::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(loginxinyouret)
{
    ack_login_xinyou ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(loginkuaiyongret)
{
    ack_login_kuaiyong ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getKuaiyongName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(loginlenovoret)
{
    ack_login_lenovo ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getLenovoName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}



handler_cmd(loginucret)
{
    ack_login_uc ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        string platform = SdkUC::getName();

        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", platform);
        SSessionDataMgr.setProp(sid, "os", "ios");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(loginucandroidret){
    
    ack_login_uc_android ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        string platform = SdkUC::getName();
        
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", platform);
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(logincwret)
{
    ack_login_cw ack;
    ack.errorcode = 1;
	if (!account.empty())
    {
		string platform = SdkCW::getName();
		
        ack.errorcode = 0;
        //根据不同的平台给用户帐号加不同的前缀
        string realaccount = platform;
        realaccount.append("_");
        realaccount.append(account);
        SSessionDataMgr.setProp(sid, "account", realaccount);
        SSessionDataMgr.setProp(sid, "platform", platform);
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
	}
    sendNetPacket(sid, &ack);
}

handler_cmd(login360ret)
{
    ack_login_360 ack;
    ack.errorcode = 1;
    
    if (!account.empty()){
        ack.errorcode = 0;
        ack.userinfo = rawinfo;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdk360::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
        SSessionDataMgr.setProp(sid, "os", "android");
        
    }
    
    sendNetPacket(sid, &ack);
}

handler_cmd(loginXmRet)
{
	ack_login_xm ack;
    ack.errorcode = 1;
	if (!uid.empty()) {
		ack.errorcode = 0;
		string account = strFormat("%s_%s", SdkXm::getName(), uid.c_str());
		SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXm::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
	}
    sendNetPacket(sid, &ack);
}

handler_cmd(loginWdjRet)
{
	ack_login_wdj ack;
    ack.errorcode = 1;
	if (!uid.empty()) {
		ack.errorcode = 0;
		string account = strFormat("%s_%s", SdkWdj::getName(), uid.c_str());
		SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkWdj::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
	}
    sendNetPacket(sid, &ack);
}


handler_cmd(loginAnzhiRet)
{
	ack_login_anzhi ack;
    ack.errorcode = 1;
	if (!account.empty()) {
		ack.errorcode = 0;
		SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkAnZhi::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
	}
    sendNetPacket(sid, &ack);
}


handler_cmd(loginexternalret)
{
    ack_login_external ack;
    ack.errorcode = 1;
	if (!account.empty()) {
		ack.errorcode = 0;
		SSessionDataMgr.setProp(gsessionid, "account", account);
        SSessionDataMgr.setProp(gsessionid, "platform", channel);
        SSessionDataMgr.setProp(gsessionid, "os", platform);
        SSessionDataMgr.setState(gsessionid, kSessionAuth);
	}
    sendNetPacket(gsessionid, &ack);
}


handler_cmd(loginopporet)
{
	ack_login_oppo ack;
    ack.errorcode = 1;
	if (!account.empty()) {
		ack.errorcode = 0;
		SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkOppo::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
	}
    sendNetPacket(sid, &ack);
}

handler_cmd(loginduokuret)
{
	ack_login_duoku ack;
    ack.errorcode = 1;
	if (!account.empty()) {
		ack.errorcode = 0;
		SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkDuoku::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
	}
    sendNetPacket(sid, &ack);
}

//当乐
handler_cmd(login_downjoy_ret)
{
    ack_login_downjoy ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkDownjoy::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//N多
handler_cmd(login_nduo_ret)
{
    ack_login_nduo ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getNduoName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//搜狗
handler_cmd(login_sougou_ret)
{
    ack_login_sougou ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkSougou::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
    
}

//手盟
handler_cmd(login_shoumeng_ret)
{
    ack_login_shoumeng ack;
    ack.errorcode = 1;
    
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkShoumeng::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//谷果 
handler_cmd(login_cooguo_ret)
{
    ack_login_cooguo ack;
    ack.errorcode = 1;
    
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkCooguo::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//兄弟玩
handler_cmd(login_xdwan_ret)
{
    ack_login_xdwan ack;
    ack.errorcode = 1;
    
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXdwan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//金山
handler_cmd(login_jinshan_ret)
{
    ack_login_jinshan ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkJinshan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(newOrder)
{
    Role* role = SRoleMgr.GetRole(roleid);
    Recharge(roleid, role, rmb, platform.c_str(), transaction.c_str());
    if (role)
    {
        notify_recharge_success notify;
        notify.billno = transaction;
        notify.rmb = rmb;
        sendNetPacket(role->getSessionId(), &notify);
    }
}

//百度云
handler_cmd(login_baiduyun_ret)
{
    ack_login_baiduyun ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkBaiduyun::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//偶玩
handler_cmd(login_ouwan_ret)
{
    ack_login_ouwan ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkOuwan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//17173
handler_cmd(login_17173_ret)
{
    ack_login_17173 ack;
    ack.errcode = 1;
    if (!account.empty())
    {
        ack.errcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdk17173::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//酷狗
handler_cmd(login_kugou_ret)
{
    ack_login_kugou ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkKugou::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//苹果网站返回结果处理
handler_cmd(recharge_apple_ret)
{
    Role* role = SRoleMgr.GetRole(reqroleid);
    Recharge(reqroleid, role, rmb, "apple", transaction.c_str());
    if (role)
    {
        notify_recharge_success notify;
        notify.billno = transaction;
        notify.rmb = rmb;
        sendNetPacket(sessionid, &notify);
    }
    
    SSessionDataMgr.setProp(sessionid, "apple_recharge", "");
}



//动态配置更新
handler_cmd(config_refresh)
{
    
    mainThreadRefreshCfg( (TimerMainCb)mcbl,param);
}

handler_cmd(connectserver)
{
    SPSMgr.onConnect(sid, psid, name);
}

handler_cmd(connectCenterServer)
{
    log_info("connect to center server");
    CenterClient::instance()->onConnect(sid);
}

handler_cmd(disconnectCenterServer)
{
    log_info("disconnect center server");
    CenterClient::instance()->onDisconnect();
}


// HTTP请求
handler_cmd(http_request)
{
    _HttpRequest.request(url.c_str(), post_param.c_str(), fun_name.c_str(), roleid, sessionid, is_https);
}

handler_cmd(http_request_ret)
{
    _HttpRequest.handle(fun_name.c_str(), roleid, ret.c_str(), sessionid);
}

handler_cmd(disconnectserver)
{
    SPSMgr.onDisconnect(sid);
}

// 定制邮件
handler_cmd(send_custom_mail)
{
	g_CustomMailAppMou.sendCustomMail();
}


handler_cmd(update_server_info)
{
	updateServerInfo();
}


//muzhiwan
handler_cmd(login_muzhiwan_ret)
{
    ack_login_muzhiwan ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkMuzhiwan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//paojiao
handler_cmd(login_paojiao_ret)
{
    ack_login_paojiao ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkPaojiao::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//feiliu 飞流
handler_cmd(login_feiliu_ret)
{
    ack_login_feiliu ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkFeiliu::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


//8849
handler_cmd(login_yxduo_ret)
{
    ack_login_yxduo ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkYxduo::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//leyogame 掌上乐游
handler_cmd(login_leyogame_ret)
{
    ack_login_leyogame ack;
    ack.errorcode = 1;
    
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkLeyogame::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//uxin 有信
handler_cmd(login_uxin_ret)
{
    ack_login_uxin ack;
    ack.errorcode = 1;
    
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkUxin::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(loginmycardret)
{
    ack_login_mycard ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getMycardName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


//gfan 机锋
handler_cmd(login_gfan_ret)
{
    ack_login_gfan ack;
    ack.errorcode = 1;
    
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkGfan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}



//bykernel 博雅科诺
handler_cmd(login_bykernel_ret)
{
    ack_login_bykernel ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkBykernel::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}


handler_cmd(login_efun_ret)
{
    ack_login_efun ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkEfun::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_efun_android_ret)
{
    ack_login_efunandorid ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkEfun::get_name());
        SSessionDataMgr.setProp(sid, "subchannel", channel);
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(login_915_ret)
{
    ack_login_915 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::get915Name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_facebook_ret)
{
    ack_login_facebook ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkFacebook::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_facebook_android_ret)
{
    ack_login_facebook_android ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkFacebook::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_google_ret)
{
    ack_login_google ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkGoogle::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_google_android_ret)
{
    ack_login_google_android ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkGoogle::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_mumayi_ret)
{
    ack_login_mumayi ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkMumayi::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//kuwo
handler_cmd(login_kuwo_ret)
{
    ack_login_kuwo ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkKuwo::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//pptv
handler_cmd(login_pptv_ret)
{
    ack_login_pptv ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkPPtv::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//3g
handler_cmd(login_3g_ret)
{
    ack_login_3g ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdk3G::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}
handler_cmd(login_verify_ret)
{
    ack_login_verify ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", game_platform);
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//itools
handler_cmd(login_itools_ret)
{
    ack_login_itools ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkItools::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//pipaw
handler_cmd(login_pipaw_ret)
{
    ack_login_pipaw ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkPipaw::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}

//dianwan
handler_cmd(login_dianwan_ret)
{
    ack_login_dianwan ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkDianwan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}

//huawei
handler_cmd(login_huawei_ret)
{
    ack_login_huawei ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkHuawei::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}


//xunlei
handler_cmd(login_xunlei_ret)
{
    ack_login_xunlei ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXunlei::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}

//给力游 支付宝
handler_cmd(login_yxpaialipay_ret)
{
    ack_login_yxpaialipay ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getYxpaialipayName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//给力游 坚果
handler_cmd(login_yxpaijianguo_ret)
{
    ack_login_yxpaijianguo ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkYxpaijianguo::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}

handler_cmd(login_lvan_ret)
{
    ack_login_lvan ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkLvan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}

handler_cmd(login_37wan_ret)
{
    ack_login_37wan ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdk37wan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}


handler_cmd(login_37wan2_ret)
{
    ack_login_37wan2 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdk37wan::get_name(true));
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}

handler_cmd(login_play168_ret)
{
    ack_login_play168 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkPlay168::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    sendNetPacket(sid, &ack);
}
//快用
handler_cmd(login_kuaiyong2_ret)
{
    ack_login_kuaiyong2 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkKuaiyong2::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//爱游戏
handler_cmd(login_iyouxi_ret)
{
    ack_login_iyouxi ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getIyouxiName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(login_taomi_ret)
{
    ack_login_taomi ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", package_name);
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_facebook_ret_new)
{
    ack_login_facebook_new ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", package_name);
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_facebook_android_ret_new)
{
    ack_login_facebook_android_new ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", package_name);
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_google_ret_new)
{
    ack_login_google_new ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", package_name);
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_google_android_ret_new)
{
    ack_login_google_android_new ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", package_name);
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


//爱贝
handler_cmd(login_iapppay_ret)
{
    ack_login_iapppay ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getIapppayName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_xinyousdk_token_ret)
{
    ack_login_xinyousdk_token ack;
	ack.token = token;
    sendNetPacket(sid, &ack);
}

handler_cmd(login_xinyousdk_ret)
{
    ack_login_xinyousdk ack;
    ack.errorcode = 1;
	ack.name = name;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyouSdk::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_SC_ret)
{
    ack_login_SC ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkSC::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    if (is_timeout)
    {
        ack.errorcode = 2;
    }
    
    sendNetPacket(sid, &ack);
}


handler_cmd(login_jolo_ret)
{
    ack_login_jolo ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkJolo::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_snailgame_ret)
{
    ack_login_snailgame ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkSnail::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_moqi_ret)
{
    ack_login_moqi ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkMoqi::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//星月平台
handler_cmd(login_tiantianwan_ret)
{
    ack_login_tiantianwan ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkTiantianwan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//摩掌
handler_cmd(login_mzyx_ret)
{
    ack_login_mzyx ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkMzyx::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
    
}

//梦城
handler_cmd(login_dcgame_ret)
{
    ack_login_dcgame ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkDcgame::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//vivo
handler_cmd(login_vivo_ret)
{
    ack_login_vivo ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkVivo::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_xmycard_ret)
{
    ack_login_xinyou_mycard ack;
    ack.errorcode = 1;
	ack.name = name;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getMycardName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(login_jinli_ret)
{
    ack_login_jinli ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkJinli::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_pubgame_ret)
{
    ack_login_pubgame ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkPubgame::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_mopo_ret)
{
    ack_login_mopo ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkMopo::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_xyzs_ret)
{
    ack_login_xyzs ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXyzs::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(logincwret2)
{
    ack_login_cw ack;
    ack.errorcode = 1;
	if (!account.empty())
    {
		string platform = SdkCW2::getName();
		
        ack.errorcode = 0;
        //根据不同的平台给用户帐号加不同的前缀
        string realaccount = platform;
        realaccount.append("_");
        realaccount.append(account);
        SSessionDataMgr.setProp(sid, "account", realaccount);
        SSessionDataMgr.setProp(sid, "platform", platform);
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
	}
    sendNetPacket(sid, &ack);
}

//移动MM
handler_cmd(login_mobileMM_ret)
{
    ack_login_mobileMM ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getMobileMMName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//手游之家
handler_cmd(login_sjwyx_ret)
{
    ack_login_sjwyx ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkSjwyx::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    
    string cpId = "100097";
    string cpSecretKey = "UNzsF625xMlwy38hVvVO5zmbyVx3Bplc";
    string keyStr = str_cat(3, cpId.c_str(), "#", cpSecretKey.c_str());
    
    MD5 md5(keyStr);
    md5.finalize();
    string key = md5.hexdigest();
    ack.key = key;
    sendNetPacket(sid, &ack);
}

//海马
handler_cmd(login_haima_ret)
{
    ack_login_haima ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getHaimaName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//i4
handler_cmd(login_i4_ret)
{
    ack_login_i4 ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkI4::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//u96
handler_cmd(login_u96_ret)
{
    ack_login_u96 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkU96::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//安锋
handler_cmd(login_anfan_ret)
{
    ack_login_anfan ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkAnfan::get_name());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//艺果
handler_cmd(login_pada_ret)
{
    ack_login_pada ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkPada::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//陌陌
handler_cmd(login_momo_ret)
{
    ack_login_momo ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkMomo::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//offerme2
handler_cmd(login_offerme2_ret)
{
    ack_login_offerme2 ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", package_name);
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//pc
handler_cmd(login_pc_ret)
{
    ack_login_pc ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkPc::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
		ack.appSecret = SdkPc::appSecret;
    }
    sendNetPacket(sid, &ack);
}

//视遥
handler_cmd(login_shiyao_ret)
{
    ack_login_shiyao ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkShiyao::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


//新泛联
handler_cmd(login_untx_ret)
{
    ack_login_untx ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkUntx::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}
// 越進
handler_cmd(login_luckystar_ret)
{
    ack_login_luckystar ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkLuckystar::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_52fun_ret)
{
    ack_login_52fun ack;
    ack.errorcode = 1;
    if (!account.empty())
    {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdk52fun::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

//五彩时空
handler_cmd(login_wcsk_ret)
{
    ack_login_wcsk ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getWcskName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(login_winner_ret)
{
    ack_login_winner ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkWinner::getName());
        SSessionDataMgr.setProp(sid, "os", system);
		SSessionDataMgr.setProp(sid, "acc_code", acc_code);
        SSessionDataMgr.setState(sid, kSessionAuth);
		ack.acc_code = acc_code;
    }
	ack.ret_code = ret_code;
	ack.msg = msg;
    sendNetPacket(sid, &ack);
}

handler_cmd(register_winner_ret)
{
    ack_register_winner ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkWinner::getName());
        SSessionDataMgr.setProp(sid, "os", system);
		SSessionDataMgr.setProp(sid, "acc_code", acc_code);
        SSessionDataMgr.setState(sid, kSessionAuth);
		ack.acc_code = acc_code;
    }
	ack.ret_code = ret_code;
	ack.msg = msg;
    sendNetPacket(sid, &ack);
}

handler_cmd(login_elu_ret)
{
    ack_login_elu ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkElu::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_yy_ret)
{
    ack_login_yy ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkYY::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_sdk_channel_ret)
{
    ack_login_sdk_channel ack;
    ack.errorcode = state;
	ack.ret = ret;
	sendNetPacket(sid, &ack);
}


handler_cmd(login_tw177_ret)
{
    ack_login_tw177 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setProp(sid, "platform", SdkTw177::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


// 巴士在线
handler_cmd(login_bus_ret)
{
    ack_login_bus ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getBusName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 酷派
handler_cmd(login_coolpad_ret)
{
    ack_login_coolpad ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getCoolpadName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_tencent_ret)
{
    ack_login_tencent ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkTencent::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}


handler_cmd(login_gamedog_ret)
{
    ack_login_gamedog ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkGamedog::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 爱奇艺
handler_cmd(login_aiqiyi_ret)
{
    ack_login_aiqiyi ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkAiqiyi::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 暴风
handler_cmd(login_baofeng_ret)
{
    ack_login_baofeng ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkBaofeng::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

handler_cmd(login_weixin_ret)
{
    ack_login_weixin ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", package_name);
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 联通
handler_cmd(login_liantong_ret)
{
    ack_login_liantong ack;
    ack.errorcode = 1;
    ack.uid = "";
    if (!account.empty()) {
        ack.errorcode = 0;
        ack.uid = uid;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkLianTong::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 酷派登录【酷派sdk】
handler_cmd(login_coolpad2_ret)
{
    ack_login_coolpad2 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        ack.access_token = access_token;
        ack.openid = openid;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkCoolpad2::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 赛众
handler_cmd(login_saizhong_ret)
{
    ack_login_saizhong ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        ack.uid = uid;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkXinyou::getSaizhongName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 同步推 安卓
handler_cmd(login_tongbutui_ret)
{
    ack_login_tongbutui ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkTongbutui::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 飓风
handler_cmd(login_iApple_ret)
{
    ack_login_iApple ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkIApple::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 07073数游
handler_cmd(login_shuyou_ret)
{
    ack_login_shuyou ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", "shuyou");
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 傲天手游
handler_cmd(login_aotian_ret)
{
    ack_login_aotian ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkAotian::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 卓动
handler_cmd(login_zhuodong_ret)
{
    ack_login_zhuodong ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkZhuodong::getname());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// m4399
handler_cmd(login_m4399_ret)
{
    ack_login_m4399 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdkm4399::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// douwan93
handler_cmd(login_douwan93_ret)
{
    ack_login_douwan93 ack;
    ack.errorcode = 1;
    if (!account.empty()) {
        ack.errorcode = 0;
        SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", Sdkm4399::getName());
        SSessionDataMgr.setState(sid, kSessionAuth);
    }
    sendNetPacket(sid, &ack);
}

// 百度91
handler_cmd(loginbaidu91ret)
{
	ack_login_baidu91 ack;
    ack.errorcode = 1;
	if (!account.empty()) {
		ack.errorcode = 0;
		SSessionDataMgr.setProp(sid, "account", account);
        SSessionDataMgr.setProp(sid, "platform", SdkBaidu91::getName());
        SSessionDataMgr.setProp(sid, "os", "android");
        SSessionDataMgr.setState(sid, kSessionAuth);
	}
    sendNetPacket(sid, &ack);
}
