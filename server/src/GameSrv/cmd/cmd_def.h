//
//  cmd.h
//  GameSrv
//  服务器内部消息定义
//  此头文件定义处理类的成员，cmd_handler.cpp重载handle方法
//
//  Created by prcv on 13-5-2.
//
//

#ifndef GAMESRV_CMD_H
#define GAMESRV_CMD_H

#include "cmd_helper.h"

#include <string>
using namespace std;

//连接信息
begin_cmd(connect)
    cmd_member(int, sid)
    cmd_member(string, ip)
    cmd_member(unsigned short, port)
end_cmd()

//客户端断开连接后，网络模块给逻辑模块发送用户退出消息
begin_cmd(disconnect)
    cmd_member(int, sid)
end_cmd()

//发送日常任务
begin_cmd(dailyquest)
    cmd_member(int, roleid)
end_cmd()


//加入广播组
begin_cmd(enterGroup)
    cmd_member(int, groupid)
    cmd_member(int, sid)
end_cmd()

//离开广播组
begin_cmd(leaveGroup)
    cmd_member(int, groupid)
    cmd_member(int, sid)
end_cmd()

//踢出客户端
//sid  sessionid
//type 踢出原因：被新登陆用户挤下线；发生异常；未发送心跳包；强制踢出
begin_cmd(kickSession)
    cmd_member(int, sid)
    cmd_member(int, type)
end_cmd()

/************************************************/
//苹果充值
begin_cmd(recharge_apple)
cmd_member(int, sessionid)
cmd_member(int, reqroleid)
cmd_member(string, rolename)
cmd_member(string, account)
cmd_member(string, receipt_data)
end_cmd()

begin_cmd(recharge_apple_ret)
cmd_member(int, sessionid)
cmd_member(int, reqroleid)
cmd_member(int, rmb)                  //enAppleRechargeRet
cmd_member(string, transaction)
//cmd_member(string, apple_receipt_json)    //成功时带苹果回复，需进一步校验
end_cmd()

//动态配置更新
begin_cmd(config_refresh)
cmd_member(long, mcbl)
cmd_member(void*, param)
end_cmd()

begin_cmd(connectserver)
cmd_member(int, sid)
cmd_member(int, psid)
cmd_member(string, name)
end_cmd()

//HTTP请求
begin_cmd(http_request)
cmd_member(int, roleid)
cmd_member(int, sessionid)
cmd_member(string, fun_name)
cmd_member(string, url)
cmd_member(string, post_param)
cmd_member(bool, is_https)
end_cmd()

begin_cmd(http_request_ret)
cmd_member(int, roleid)
cmd_member(int, sessionid)
cmd_member(string, fun_name)
cmd_member(string, ret)
end_cmd()

begin_cmd(disconnectserver)
cmd_member(int, sid)
end_cmd()

begin_cmd(connectCenterServer)
    cmd_member(int, sid)
end_cmd()

begin_cmd(disconnectCenterServer)

end_cmd()


// 定制邮件
begin_cmd(send_custom_mail)
end_cmd()

begin_cmd(update_server_info)
end_cmd()

/************************************************/
//91苹果平台
begin_cmd(login91)
    cmd_member(int, gsessionid)
    cmd_member(string, uin)
    cmd_member(string, csessionid)
end_cmd()

begin_cmd(login91ret)
    cmd_member(string, account)
    cmd_member(int, sid)
end_cmd()

//91安卓平台
begin_cmd(login91android)
    cmd_member(int, gsessionid)
    cmd_member(string, uin)
    cmd_member(string, csessionid)
end_cmd()

begin_cmd(login91androidret)
    cmd_member(string, account)
    cmd_member(int, sid)
end_cmd()

begin_cmd(loginpps)
    cmd_member(int, gsessionid)
	cmd_member(string, sid)
end_cmd()

begin_cmd(loginppsret)
    cmd_member(string, account)
    cmd_member(int, sid)
end_cmd()

begin_cmd(logintb)
    cmd_member(int, gsessionid)
    cmd_member(string, uin)
    cmd_member(string, csessionid)
end_cmd()

begin_cmd(logintbret)
    cmd_member(string, account)
    cmd_member(string, oldaccount)
    cmd_member(int, sid)
end_cmd()


begin_cmd(loginxinyou)
    cmd_member(int, gsessionid)
    cmd_member(string, csessionid)
end_cmd()

begin_cmd(loginxinyouret )
    cmd_member(string, account)
    cmd_member(int, sid);
end_cmd()



begin_cmd(loginlenovo)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(loginlenovoret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()


begin_cmd(loginkuaiyong)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(loginkuaiyongret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()



begin_cmd(loginuc)
    cmd_member(int, gsessionid);
    cmd_member(string, csessionid)
end_cmd()

begin_cmd(loginucret)
    cmd_member(string, account)
    cmd_member(int, sid);
end_cmd()



begin_cmd(loginucandroid)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(loginucandroidret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()



begin_cmd(login360)
cmd_member(string, authcode)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login360ret)
cmd_member(int, sid)
cmd_member(int, uid)
cmd_member(string, account)
cmd_member(string, rawinfo)
end_cmd()


// 畅玩登录
begin_cmd(logincw)
	cmd_member(string, openId)
	cmd_member(string, token)
	cmd_member(int, sid)
end_cmd()

begin_cmd(logincwret)
	cmd_member(string, account)
	cmd_member(int, sid);
end_cmd()


// 小米
begin_cmd(loginXm)
	cmd_member(string, uid)
	cmd_member(string, session)
	cmd_member(int, sid)
end_cmd()

begin_cmd(loginXmRet)
	cmd_member(int, sid)
	cmd_member(string, uid)
end_cmd()

// 碗豆荚
begin_cmd(loginWdj)
	cmd_member(string, uid)
	cmd_member(string, token)
	cmd_member(int, sid)
end_cmd()

begin_cmd(loginWdjRet)
	cmd_member(int, sid)
	cmd_member(string, uid)
end_cmd()


// 安智
begin_cmd(loginAnzhi)
    cmd_member(string, csessionid)
    cmd_member(string, account)
    cmd_member(int, gsessionid)
end_cmd()

begin_cmd(loginAnzhiRet)
    cmd_member(int, sid)
    cmd_member(string, account)
end_cmd()


begin_cmd(loginexternal)
    cmd_member(int, gsessionid)
    cmd_member(string, channel)
    cmd_member(string, platform)
    cmd_member(string, detail)
end_cmd()

begin_cmd(loginexternalret)
    cmd_member(int, gsessionid)
    cmd_member(string, channel)
    cmd_member(string, platform)
    cmd_member(string, account)
end_cmd()


begin_cmd(loginoppo)
cmd_member(string, accesstoken)
cmd_member(string, accesssecret)
cmd_member(int, gsessionid)
end_cmd()


begin_cmd(loginopporet)
cmd_member(int, sid)
cmd_member(string, account)
end_cmd()



begin_cmd(loginduoku)
cmd_member(string, csessionid)
cmd_member(string, uid)
cmd_member(int, gsessionid)
end_cmd()


begin_cmd(loginduokuret)
cmd_member(int, sid)
cmd_member(string, account)
end_cmd()

//多乐downjoy
begin_cmd(login_downjoy)
cmd_member(int, gsessionid)
cmd_member(string, mid)
cmd_member(string, token)
end_cmd()

begin_cmd(login_downjoy_ret)
cmd_member(int, sid)
cmd_member(string, account)
end_cmd()

//N多
begin_cmd(login_nduo)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_nduo_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//百度云
begin_cmd(login_baiduyun)
cmd_member(int, gsessionid)
cmd_member(string, auth_code)
end_cmd()

begin_cmd(login_baiduyun_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//搜狗
begin_cmd(login_sougou)
cmd_member(string, user_id)
cmd_member(string, session_key)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_sougou_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//手盟
begin_cmd(login_shoumeng)
cmd_member(string, user_id)
cmd_member(string, session_id)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_shoumeng_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//谷果 
begin_cmd(login_cooguo)
cmd_member(string, userId)
cmd_member(string, Timestamp)
cmd_member(string, sign)
cmd_member(int, gsessionid)   //必须有的
end_cmd()

begin_cmd(login_cooguo_ret)
cmd_member(string, account)
cmd_member(int, sid)          //必须有的
end_cmd()

//兄弟玩
begin_cmd(login_xdwan)
cmd_member(string, userid)
cmd_member(string, tstamp)
cmd_member(string, sign)
cmd_member(int, gsessionid)   //必须有的
end_cmd()

begin_cmd(login_xdwan_ret)
cmd_member(string, account)
cmd_member(int, sid)          //必须有的
end_cmd()

//偶玩
begin_cmd(login_ouwan)
cmd_member(int, gsessionid)
cmd_member(string, openid)
cmd_member(string, timestamp)
cmd_member(string, sign)
end_cmd()

begin_cmd(login_ouwan_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//17173
begin_cmd(login_17173)
cmd_member(int, gsessionid)
cmd_member(string, token)
cmd_member(string, date)
end_cmd()

begin_cmd(login_17173_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//金山
begin_cmd(login_jinshan)
cmd_member(int, gsessionid)
cmd_member(string, uid)
cmd_member(string, user_token)
end_cmd()

begin_cmd(login_jinshan_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//muzhiwan 拇指玩
begin_cmd(login_muzhiwan)
cmd_member(int, gsessionid)
cmd_member(string, token)
end_cmd()

begin_cmd(login_muzhiwan_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//paojiao 泡椒网
begin_cmd(login_paojiao)
cmd_member(int, gsessionid)
cmd_member(string, token)
end_cmd()

begin_cmd(login_paojiao_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//飞流
begin_cmd(login_feiliu)
cmd_member(string, uuid)
cmd_member(string, timestamp)
cmd_member(string, sign)
cmd_member(int, gsessionid)   //必须有的
end_cmd()

begin_cmd(login_feiliu_ret)
cmd_member(string, account)
cmd_member(int, sid)          //必须有的
end_cmd()

//yxduo
begin_cmd(login_yxduo)
cmd_member(string, token)
cmd_member(string, sign)
cmd_member(int, gsessionid)
cmd_member(string, appid)
end_cmd()

begin_cmd(login_yxduo_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//酷狗
begin_cmd(login_kugou)
cmd_member(string, UserName)
cmd_member(string, UnixTime)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_kugou_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//leyogame 掌上乐游
begin_cmd(login_leyogame)
cmd_member(string, ssid)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_leyogame_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//uxin 有信
begin_cmd(login_uxin)
cmd_member(string, openid)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_uxin_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()


begin_cmd(newOrder)
    cmd_member(int, roleid)
    cmd_member(int, rmb)
    cmd_member(string, platform)
    cmd_member(string, transaction)
end_cmd()

//mycard
begin_cmd(loginmycard)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(loginmycardret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//gfan 机锋
begin_cmd(login_gfan)
cmd_member(string, token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_gfan_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()


//bykernel 博雅科诺
begin_cmd(login_bykernel)
cmd_member(string, ssid)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_bykernel_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//efun
begin_cmd(login_efun)
cmd_member(string, timestamp)
cmd_member(string, userid)
cmd_member(string, sign)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_efun_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//android efun
begin_cmd(login_efun_android)
cmd_member(string, timestamp)
cmd_member(string, userid)
cmd_member(string, sign)
cmd_member(int, gsessionid)
cmd_member(string, channel)
end_cmd()

begin_cmd(login_efun_android_ret)
cmd_member(string, account)
cmd_member(int, sid)
cmd_member(string, channel)
end_cmd()

//915
begin_cmd(login_915)
cmd_member(string, session_value)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_915_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//facebook
begin_cmd(login_facebook)
cmd_member(string, access_token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_facebook_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//facebook android
begin_cmd(login_facebook_android)
cmd_member(string, access_token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_facebook_android_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//google
begin_cmd(login_google)
cmd_member(string, access_token)
cmd_member(string, id_token)
cmd_member(string, one_time_code)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_google_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//google android
begin_cmd(login_google_android)
cmd_member(string, access_token)
cmd_member(string, id_token)
cmd_member(string, one_time_code)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_google_android_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//mumayi
begin_cmd(login_mumayi)
cmd_member(string, token)
cmd_member(string, uid)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_mumayi_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//kuwo
begin_cmd(login_kuwo)
cmd_member(string, userid)
cmd_member(string, timestamp)
cmd_member(string, sign)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_kuwo_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//pptv
begin_cmd(login_pptv)
cmd_member(string, ssessionid)
cmd_member(string, username)
cmd_member(int, gsessionid)
end_cmd()
begin_cmd(login_verify)
cmd_member(string, uid)
cmd_member(string, token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_verify_ret)
cmd_member(string, account)
cmd_member(string, game_platform)
cmd_member(int, sid)
end_cmd()

begin_cmd(login_pptv_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//3g
begin_cmd(login_3g)

cmd_member(string, ssid)
cmd_member(string, token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_3g_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//itools
begin_cmd(login_itools)
cmd_member(string, uid)
cmd_member(string, ssessionid)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_itools_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//pipaw
begin_cmd(login_pipaw)
cmd_member(string, username)
cmd_member(string, ssid)
cmd_member(string, time)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_pipaw_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//dianwan
begin_cmd(login_dianwan)
cmd_member(string, ssid)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_dianwan_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//华为
begin_cmd(login_huawei)
cmd_member(string, access_token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_huawei_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//xunlei
begin_cmd(login_xunlei)
cmd_member(string, customerid)
cmd_member(string, customerkey)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_xunlei_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//给力游 支付宝
begin_cmd(login_yxpaialipay)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_yxpaialipay_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//给力游 坚果
begin_cmd(login_yxpaijianguo)
cmd_member(string, userid)
cmd_member(string, token)
cmd_member(int, gsessionid)
cmd_member(string, channel)  //渠道号
end_cmd()

begin_cmd(login_yxpaijianguo_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//绿岸
begin_cmd(login_lvan)
cmd_member(string, uid)
cmd_member(string, token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_lvan_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//
begin_cmd(login_37wan)
cmd_member(string, token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_37wan_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

begin_cmd(login_play168)
cmd_member(string, uid)
cmd_member(string, token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_play168_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()


//快用
begin_cmd(login_kuaiyong2)
cmd_member(string, tokenKey)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_kuaiyong2_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//爱游戏
begin_cmd(login_iyouxi)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_iyouxi_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//taomi
begin_cmd(login_taomi)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
cmd_member(string, package_name)
end_cmd()

begin_cmd(login_taomi_ret )
cmd_member(string, account)
cmd_member(int, sid);
cmd_member(string, package_name)
end_cmd()

//facebook　new
begin_cmd(login_facebook_new)
cmd_member(string, access_token)
cmd_member(int, gsessionid)
cmd_member(string, package_name)
cmd_member(string, partner_name)
end_cmd()

begin_cmd(login_facebook_ret_new)
cmd_member(string, account)
cmd_member(int, sid)
cmd_member(string, package_name)
end_cmd()
 
//facebook android new
begin_cmd(login_facebook_android_new)
cmd_member(string, access_token)
cmd_member(int, gsessionid)
cmd_member(string, package_name)
cmd_member(string, partner_name)
end_cmd()

begin_cmd(login_facebook_android_ret_new)
cmd_member(string, account)
cmd_member(int, sid)
cmd_member(string, package_name)
end_cmd()

//google new
begin_cmd(login_google_new)
cmd_member(string, access_token)
cmd_member(string, id_token)
cmd_member(string, one_time_code)
cmd_member(int, gsessionid)
cmd_member(string, package_name)
cmd_member(string, partner_name)
end_cmd()

begin_cmd(login_google_ret_new)
cmd_member(string, account)
cmd_member(int, sid)
cmd_member(string, package_name)
end_cmd()

//google android new
begin_cmd(login_google_android_new)
cmd_member(string, access_token)
cmd_member(string, id_token)
cmd_member(string, one_time_code)
cmd_member(int, gsessionid)
cmd_member(string, package_name)
cmd_member(string, partner_name)
end_cmd()

begin_cmd(login_google_android_ret_new)
cmd_member(string, account)
cmd_member(int, sid)
cmd_member(string, package_name)
end_cmd()

// 辣椒SC
begin_cmd(login_SC)
cmd_member(int, gsessionid);
cmd_member(string, user_id)
cmd_member(string, login_token)
end_cmd()

begin_cmd(login_SC_ret )
cmd_member(string, account)
cmd_member(int, sid);
cmd_member(int, is_timeout)
end_cmd()

//爱贝
begin_cmd(login_iapppay)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_iapppay_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//　新游
begin_cmd(login_xinyousdk_token)
cmd_member(int, gsessionid)
end_cmd()



begin_cmd(login_xinyousdk_token_ret)
cmd_member(int, sid)
cmd_member(string, token)
end_cmd()

begin_cmd(login_xinyousdk)
cmd_member(int, gsessionid)
cmd_member(string, access_token)
cmd_member(string, platform)
cmd_member(string, openid)
end_cmd()

begin_cmd(login_xinyousdk_ret)
cmd_member(int, sid)
cmd_member(string, account)
cmd_member(string, name)
end_cmd()

// jolo
begin_cmd(login_jolo)
cmd_member(int, gsessionid);
cmd_member(string, account)
cmd_member(string, account_sign)
end_cmd()

begin_cmd(login_jolo_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

// snail game
begin_cmd(login_snailgame)
cmd_member(int, gsessionid);
cmd_member(string, uin)
cmd_member(string, session_id)
end_cmd()

begin_cmd(login_snailgame_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()
//星月
begin_cmd(login_tiantianwan)
cmd_member(string, username)
cmd_member(string, logintime)
cmd_member(string, sign)
cmd_member(int, gsessionid);
end_cmd()



begin_cmd(login_tiantianwan_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

// moqi
begin_cmd(login_moqi)
cmd_member(int, gsessionid);
cmd_member(string, uin)
cmd_member(string, session_key)
end_cmd()


begin_cmd(login_moqi_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//摩掌
begin_cmd(login_mzyx)
cmd_member(string, ssid)
cmd_member(int, gsessionid)
end_cmd()



begin_cmd(login_mzyx_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()


//梦城
begin_cmd(login_dcgame)
cmd_member(string, userid)
cmd_member(string, userkey)
cmd_member(string, at)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_dcgame_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//vivo
begin_cmd(login_vivo)
cmd_member(string, authtoken)
cmd_member(string, openid)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_vivo_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()


// xinyou mycard
begin_cmd(login_xmycard)
cmd_member(int, gsessionid)
cmd_member(string, access_token)
end_cmd()

begin_cmd(login_xmycard_ret)
cmd_member(int, sid)
cmd_member(string, account)
cmd_member(string, name)
end_cmd()

begin_cmd(login_jinli)
cmd_member(int, gsessionid)
cmd_member(string, playid)
cmd_member(string, token)
end_cmd()

begin_cmd(login_jinli_ret)
cmd_member(int, sid)
cmd_member(string, account)
end_cmd()


begin_cmd(login_37wan2)
cmd_member(string, token)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_37wan2_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()


begin_cmd(login_pubgame)
cmd_member(int, gsessionid)
cmd_member(string, playerId)
cmd_member(string, token)
end_cmd()

begin_cmd(login_pubgame_ret)
cmd_member(int, sid)
cmd_member(string, account)
end_cmd()


begin_cmd(login_mopo)
cmd_member(int, gsessionid)
cmd_member(string, skyid)
cmd_member(string, ticked)
cmd_member(string, username)
end_cmd()

begin_cmd(login_mopo_ret)
cmd_member(int, sid)
cmd_member(string, account)
end_cmd()

begin_cmd(login_xyzs)
cmd_member(int, gsessionid)
cmd_member(string, uid)
cmd_member(string, token)
end_cmd()

begin_cmd(login_xyzs_ret)
cmd_member(int, sid)
cmd_member(string, account)
end_cmd()

// 畅玩电信
begin_cmd(logincw2)
cmd_member(string, openId)
cmd_member(string, ip)
cmd_member(string, port)
cmd_member(string, timestamp)
cmd_member(string, sign)
cmd_member(int, sid)
end_cmd()

begin_cmd(logincwret2)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//移动MM
begin_cmd(login_mobileMM)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_mobileMM_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//手游之家  sjwyx(手机玩游戏)
begin_cmd(login_sjwyx)
cmd_member(string, mid);
cmd_member(string, cpAuthtoken)
cmd_member(int, gsessionid);
end_cmd()

begin_cmd(login_sjwyx_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//海马
begin_cmd(login_haima)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_haima_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//i4
begin_cmd(login_i4)
cmd_member(int, gsessionid);
cmd_member(string, userid)
cmd_member(string, token)
end_cmd()

begin_cmd(login_i4_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//u96
begin_cmd(login_u96)
cmd_member(string, user_id)
cmd_member(string, session_id)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(login_u96_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//安锋
begin_cmd(login_anfan)
cmd_member(int, gsessionid)
cmd_member(string, uid)
cmd_member(string, ucid)
cmd_member(string, uuid)
end_cmd()

begin_cmd(login_anfan_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//艺果
begin_cmd(login_pada)
cmd_member(int, gsessionid)
cmd_member(string, appId)
cmd_member(string, roleId)
cmd_member(string, roleToken)
end_cmd()

begin_cmd(login_pada_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//陌陌
begin_cmd(login_momo)
cmd_member(int, gsessionid)
cmd_member(string, uid)
cmd_member(string, token)
end_cmd()

begin_cmd(login_momo_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//offerme2
begin_cmd(login_offerme2)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
cmd_member(string, package_name)
end_cmd()

begin_cmd(login_offerme2_ret)
cmd_member(string, account)
cmd_member(int, sid);
cmd_member(string, package_name)
end_cmd()

//pc
begin_cmd(login_pc)
cmd_member(int, gsessionid);
cmd_member(string, sessionId)
cmd_member(string, uid)
end_cmd()

begin_cmd(login_pc_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//视遥
begin_cmd(login_shiyao)
cmd_member(int, gsessionid)
cmd_member(string, guid)
cmd_member(string, tokenId)
end_cmd()

begin_cmd(login_shiyao_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//新泛联
begin_cmd(login_untx)
cmd_member(int, gsessionid)
cmd_member(string, user_id)
end_cmd()

begin_cmd(login_untx_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//越進
begin_cmd(login_luckystar)
cmd_member(int, gsessionid);
cmd_member(string, accessToken)
cmd_member(string, uid)
end_cmd()

begin_cmd(login_luckystar_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

begin_cmd(login_52fun)
cmd_member(int, gsessionid);
cmd_member(string, checkCode)
cmd_member(string, uid)
end_cmd()

begin_cmd(login_52fun_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

//五彩时空
begin_cmd(login_wcsk)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_wcsk_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

begin_cmd(login_winner)
cmd_member(int, gsessionid);
cmd_member(string, acc_name)
cmd_member(string, password)
cmd_member(string, system)
end_cmd()

begin_cmd(login_winner_ret)
cmd_member(string, account)
cmd_member(string, system)
cmd_member(string, acc_code)
cmd_member(string, ret_code)
cmd_member(string, msg)
cmd_member(int, sid);
end_cmd()

begin_cmd(register_winner)
cmd_member(int, gsessionid);
cmd_member(string, acc_name)
cmd_member(string, password)
cmd_member(string, confirmPassword)
cmd_member(string, system)
end_cmd()

begin_cmd(register_winner_ret)
cmd_member(string, account)
cmd_member(string, acc_code)
cmd_member(string, ret_code)
cmd_member(string, system)
cmd_member(string, msg)
cmd_member(int, sid);
end_cmd()

//E路wifi
begin_cmd(login_elu)
cmd_member(int, gsessionid)
cmd_member(string, username)
cmd_member(string, password)
end_cmd()

begin_cmd(login_elu_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

// 多玩YY
begin_cmd(login_yy)
cmd_member(int, gsessionid)
cmd_member(string, ssid)
cmd_member(string, account)
cmd_member(string, time)
end_cmd()

begin_cmd(login_yy_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

begin_cmd(login_sdk_channel)
cmd_member(int, gsessionid);
cmd_member(string, sdkChannel)
cmd_member(string, verifyStr)
cmd_member(string, system)
cmd_member(string, packageName)
cmd_member(string, partnerName)
end_cmd()

begin_cmd(login_sdk_channel_ret)
cmd_member(string, account)
cmd_member(string, ret)
cmd_member(int, sid);
cmd_member(int, state);
end_cmd()


begin_cmd(login_tencent)
cmd_member(int, gsessionid);
cmd_member(string, openid)
end_cmd()

begin_cmd(login_tencent_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()


begin_cmd(login_tw177)
cmd_member(int, gsessionid);
cmd_member(string, token)
cmd_member(string, userid)
end_cmd()

begin_cmd(login_tw177_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

// 巴士在线
begin_cmd(login_bus)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_bus_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

// 酷派
begin_cmd(login_coolpad)
cmd_member(int, gsessionid);
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_coolpad_ret )
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

begin_cmd(login_gamedog)
cmd_member(int, gsessionid);
cmd_member(string, token)
cmd_member(string, member_id)
end_cmd()

begin_cmd(login_gamedog_ret)
cmd_member(string, account)
cmd_member(int, sid);
end_cmd()

// 爱奇艺
begin_cmd(login_aiqiyi)
cmd_member(int, gsessionid)
cmd_member(string, uid)
cmd_member(string, time)
cmd_member(string, sign)
end_cmd()

begin_cmd(login_aiqiyi_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

// 暴风
begin_cmd(login_baofeng)
cmd_member(int, gsessionid)
cmd_member(string, ticket)
end_cmd()

begin_cmd(login_baofeng_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

//weixin
begin_cmd(login_weixin)
cmd_member(string, access_token)
cmd_member(string, open_id)
cmd_member(string, system)
cmd_member(int, gsessionid)
cmd_member(string, package_name)
cmd_member(string, partner_name)
end_cmd()

begin_cmd(login_weixin_ret)
cmd_member(string, account)
cmd_member(int, sid)
cmd_member(string, package_name)
end_cmd()

// 联通
begin_cmd(login_liantong)
cmd_member(int, gsessionid)
cmd_member(string, access_token)
end_cmd()

begin_cmd(login_liantong_ret)
cmd_member(string, account)
cmd_member(int, sid);
cmd_member(string, uid);
end_cmd()

// 酷派登录
begin_cmd(login_coolpad2)
cmd_member(int, gsessionid)
cmd_member(string, authorization_code)
end_cmd()

begin_cmd(login_coolpad2_ret)
cmd_member(string, account)
cmd_member(int, sid)
cmd_member(string, access_token)
cmd_member(string, openid)
end_cmd()

// 赛众
begin_cmd(login_saizhong)
cmd_member(int, gsessionid)
cmd_member(string, csessionid)
end_cmd()

begin_cmd(login_saizhong_ret)
cmd_member(string, account)
cmd_member(int, sid)
cmd_member(int, uid)
end_cmd()

// 同步推 安卓
begin_cmd(login_tongbutui)
cmd_member(int, gsessionid)
cmd_member(string, session)
cmd_member(string, userid)
end_cmd()

begin_cmd(login_tongbutui_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

// 飓风
begin_cmd(login_iApple)
cmd_member(int, gsessionid)
cmd_member(string, session)
cmd_member(string, userid)
end_cmd()

begin_cmd(login_iApple_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

// 07073数游
begin_cmd(login_shuyou)
    cmd_member(int, gsessionid)
    cmd_member(string, username)
    cmd_member(string, session)
end_cmd()

begin_cmd(login_shuyou_ret)
    cmd_member(string, account)
    cmd_member(int, sid)
end_cmd()

// 傲天手游
begin_cmd(login_aotian)
cmd_member(int, gsessionid)
cmd_member(string, uid)
cmd_member(string, userName)
cmd_member(string, loginSign)
end_cmd()

begin_cmd(login_aotian_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

// 卓动
begin_cmd(login_zhuodong)
cmd_member(int, gsessionid)
cmd_member(string, sessiontoken)
cmd_member(string, gameuid)
end_cmd()

begin_cmd(login_zhuodong_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

// m4399
begin_cmd(login_m4399)
cmd_member(int, gsessionid)
cmd_member(string, token)
cmd_member(string, uid)
end_cmd()

begin_cmd(login_m4399_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

// douwan93
begin_cmd(login_douwan93)
cmd_member(int, gsessionid)
cmd_member(string, token)
cmd_member(string, account_id)
end_cmd()

begin_cmd(login_douwan93_ret)
cmd_member(string, account)
cmd_member(int, sid)
end_cmd()

// 百度91
begin_cmd(loginbaidu91)
cmd_member(string, csessionid)
cmd_member(string, uid)
cmd_member(int, gsessionid)
end_cmd()

begin_cmd(loginbaidu91ret)
cmd_member(int, sid)
cmd_member(string, account)
end_cmd()


/*******************************************************/
#endif
