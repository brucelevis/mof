
//注册
begin_msg(req_register,0,1)
def_string(account)
def_string(password)
end_msg()

begin_msg(ack_register,0,2)
def_int(errorcode)
end_msg()


//登录
begin_msg(req_login, 0, 3)
def_string(account)
def_string(password)
end_msg()

begin_msg(ack_login, 0, 4)
def_err(errorcode)
def_string(json)
end_msg()

/****************SDK平台接入*********************/


begin_msg(req_login_91, 0, 15)
def_string(uin)
def_string(sessionid)
end_msg()


begin_msg(ack_login_91, 0, 16)
def_int(errorcode)
end_msg()

begin_msg(req_login_pps, 0, 17)
def_string(sid)
end_msg()

begin_msg(ack_login_pps, 0, 18)
def_int(errorcode)
end_msg()


begin_msg(req_login_tb, 0, 19)
def_string(uin)
def_string(session)
end_msg()

begin_msg(ack_login_tb, 0, 20)
def_int(errorcode)
end_msg()

begin_msg(req_login_xinyou, 0, 21)
def_string(session)
end_msg()

begin_msg(ack_login_xinyou, 0, 22)
def_int(errorcode)
end_msg()


begin_msg(req_login_uc, 0, 25)
def_string(ucsession)
end_msg()


begin_msg(ack_login_uc, 0, 26)
def_int(errorcode)
end_msg()


begin_msg(req_login_360, 0, 34)
def_string(authcode)
end_msg()

begin_msg(ack_login_360, 0, 35)
def_int(errorcode)
def_string(userinfo)
end_msg()

//　畅玩
begin_msg(req_login_cw, 0, 36)
def_string(openId)
def_string(token)
end_msg()

begin_msg(ack_login_cw, 0, 37)
def_int(errorcode)
end_msg()

//　小米
begin_msg(req_login_xm, 0, 38)
def_string(uid)
def_string(session)
end_msg()

begin_msg(ack_login_xm, 0, 39)
def_int(errorcode)
end_msg()

//　碗豆荚
begin_msg(req_login_wdj, 0, 40)
def_string(uid)
def_string(token)
end_msg()

begin_msg(ack_login_wdj, 0, 41)
def_int(errorcode)
end_msg()

//91
begin_msg(req_login_91_android, 0, 42)
def_string(uin)
def_string(sessionid)
end_msg()

begin_msg(ack_login_91_android, 0, 43)
def_int(errorcode)
end_msg()

//uc
begin_msg(req_login_uc_android, 0, 44)
def_string(ucsession)
end_msg()


begin_msg(ack_login_uc_android, 0, 45)
def_int(errorcode)
end_msg()

//快用
begin_msg(req_login_kuaiyong, 0, 46)
def_string(session)
end_msg()

begin_msg(ack_login_kuaiyong, 0, 47)
def_int(errorcode)
end_msg()

//联想
begin_msg(req_login_lenovo, 0, 48)
def_string(session)
end_msg()


begin_msg(ack_login_lenovo, 0, 49)
def_int(errorcode)
end_msg()

//安智
begin_msg(req_login_anzhi, 0, 50)
def_string(account)
def_string(sessionid)
end_msg()

begin_msg(ack_login_anzhi, 0, 51)
def_int(errorcode)
end_msg()

begin_msg(req_login_external, 0, 52)
def_string(channel)
def_string(platform)
def_string(detail)
end_msg()

begin_msg(ack_login_external, 0, 53)
def_int(errorcode)
end_msg()

//oppo
begin_msg(req_login_oppo, 0, 54)
def_string(accesstoken)
def_string(accesssecret)
end_msg()


begin_msg(ack_login_oppo, 0, 55)
def_int(errorcode)
end_msg()

//多酷
begin_msg(req_login_duoku, 0, 56)
def_string(sessionid)
def_string(uid)
end_msg()

begin_msg(ack_login_duoku, 0, 57)
def_int(errorcode)
end_msg()

//当乐
begin_msg(req_login_downjoy, 0, 58)
def_string(mid)
def_string(token)
end_msg()

begin_msg(ack_login_downjoy, 0, 59)
def_int(errorcode)   //返回在0表示成功
end_msg()

//N多
begin_msg(req_login_nduo, 0, 60)
def_string(session)
end_msg()

begin_msg(ack_login_nduo, 0, 61)
def_int(errorcode)
end_msg()

//百度云
begin_msg(req_login_baiduyun, 0, 62)
def_string(auth_code);
end_msg()

begin_msg(ack_login_baiduyun, 0, 63)
def_int(errorcode)
end_msg()

begin_msg(req_login_baiduyun_sign, 0, 64)
end_msg()

begin_msg(ack_login_baiduyun_sign, 0, 65)
def_string(time)
def_string(sign)
end_msg()


begin_msg(ack_baiduyun_recharge, 0, 67)
def_string(sign)
end_msg()

//搜狗
begin_msg(req_login_sougou, 0, 68)
def_string(user_id)
def_string(session_key)
end_msg()

begin_msg(ack_login_sougou, 0, 69)
def_int(errorcode) //返回0表示验证成功
end_msg()

//偶玩
begin_msg(req_login_ouwan, 0, 70)
def_string(openid)
def_string(timestamp)
def_string(sign)
end_msg()

begin_msg(ack_login_ouwan, 0, 71)
def_int(errorcode) //返回0表示验证成功
end_msg()

//17173
begin_msg(req_login_17173, 0, 72)
def_string(token)
def_string(date) //请按如下格式传递：Fri, 26 Jul 2013 07:13:49 GMT
end_msg()

begin_msg(ack_login_17173, 0, 73)
def_int(errcode)  //返回0表示验证成功
end_msg()

//谷果
begin_msg(req_login_cooguo, 0, 74)
def_string(userId)
def_string(timestamp)
def_string(sign)
end_msg()

begin_msg(ack_login_cooguo, 0, 75)
def_int(errorcode)  //返回0表示验证成功
end_msg()

//金山
begin_msg(req_login_jinshan, 0, 76)
def_string(uid)
def_string(user_token)
end_msg()

begin_msg(ack_login_jinshan, 0, 77)
def_int(errorcode) //返回0表示验证成功
end_msg()

//手盟
begin_msg(req_login_shoumeng, 0, 78)
def_string(user_id)
def_string(session_id)
end_msg()

begin_msg(ack_login_shoumeng, 0, 79)
def_int(errorcode)     //返回0表示验证成功
end_msg()

//兄弟玩
begin_msg(req_login_xdwan, 0, 80)
def_string(userid)
def_string(tstamp)
def_string(sign)
end_msg()

begin_msg(ack_login_xdwan, 0, 81)
def_int(errorcode)   //返回0表示验证成功
end_msg()

//muzhiwan 拇指玩
begin_msg(req_login_muzhiwan, 0, 82)
def_string(token)
end_msg()

begin_msg(ack_login_muzhiwan, 0, 83)
def_int(errorcode) //返回0表示验证成功
end_msg()

//paojiao 泡椒网
begin_msg(req_login_paojiao, 0, 84)
def_string(token)
end_msg()

begin_msg(ack_login_paojiao, 0, 85)
def_int(errorcode)
end_msg()

//feiliu 飞流
begin_msg(req_login_feiliu, 0, 86)
def_string(uuid)
def_string(sign)
def_string(timestamp)
end_msg()

begin_msg(ack_login_feiliu, 0, 87)
def_int(errorcode)   //返回0表示验证成功
end_msg()

//yxduo 游戏多
begin_msg(req_login_yxduo, 0, 88)
def_string(token)
def_string(sign)
//def_string(appid)
end_msg()

begin_msg(ack_login_yxduo, 0, 89)
def_int(errorcode)   //返回0表示验证成功
end_msg()

//kugou 酷狗
begin_msg(req_login_kugou, 0, 90)
def_string(UserName)
def_string(UnixTime)
end_msg()

begin_msg(ack_login_kugou, 0, 91)
def_int(errorcode)     //返回0表示成功
end_msg()

//leyogame 掌上乐游
begin_msg(req_login_leyogame, 0, 92)
def_string(ssid)
end_msg()

begin_msg(ack_login_leyogame, 0, 93)
def_int(errorcode)     //返回0表示成功
end_msg()

//uxin 有信
begin_msg(req_login_uxin, 0, 94)
def_string(openid)
end_msg()

begin_msg(ack_login_uxin, 0, 95)
def_int(errorcode)     //返回0表示成功
end_msg()


//mycard
begin_msg(req_login_mycard, 0, 96)
def_string(session)
end_msg()

begin_msg(ack_login_mycard, 0, 97)
def_int(errorcode)
end_msg()

//gfan 机锋
begin_msg(req_login_gfan, 0, 98)
def_string(token)
end_msg()

begin_msg(ack_login_gfan, 0, 99)
def_int(errorcode)     //返回0表示成功
end_msg()

//bykernel 博雅科诺
begin_msg(req_login_bykernel, 0, 100)
def_string(ssid)
end_msg()

begin_msg(ack_login_bykernel, 0, 101)
def_int(errorcode)    //返回0表示成功
end_msg()

//efun
begin_msg(req_login_efun, 0, 102)
def_string(timestamp)
def_string(userid)
def_string(sign)
end_msg()

begin_msg(ack_login_efun, 0, 103)
def_int(errorcode) //返回0表示成功
end_msg()

//efun android
begin_msg(req_login_efunandroid, 0, 104)
def_string(timestamp)
def_string(userid)
def_string(sign)
def_string(channel)
end_msg()

begin_msg(ack_login_efunandorid, 0, 105)
def_int(errorcode) //返回0表示成功
end_msg()

//915
begin_msg(req_login_915, 0, 106)
def_string(session_value)
end_msg()

begin_msg(ack_login_915, 0, 107)
def_int(errorcode) //返回0表示成功
end_msg()

//facebook
begin_msg(req_login_facebook, 0, 108)
def_string(access_token)
end_msg()

begin_msg(ack_login_facebook, 0, 109)
def_int(errorcode)
end_msg()

//facebook android
begin_msg(req_login_facebook_android, 0, 110)
def_string(access_token)
end_msg()

begin_msg(ack_login_facebook_android, 0, 111)
def_int(errorcode)
end_msg()

//google
begin_msg(req_login_google, 0, 112)
def_string(access_token)
end_msg()

begin_msg(ack_login_google, 0, 113)
def_int(errorcode)
end_msg()

//google android
begin_msg(req_login_google_android, 0, 114)
def_string(access_token)
end_msg()

begin_msg(ack_login_google_android, 0, 115)
def_int(errorcode)
end_msg()

//mumayi 木蚂蚁
begin_msg(req_login_mumayi, 0, 116)
def_string(token)
def_string(uid)
end_msg()

begin_msg(ack_login_mumayi, 0, 117)
def_int(errorcode) //返回0表示成功
end_msg()

//kuwo
begin_msg(req_login_kuwo, 0, 118)
def_string(userid)
def_string(timestamp)
def_string(sign)
end_msg()

begin_msg(ack_login_kuwo, 0, 119)
def_int(errorcode)   //返回0表示成功
end_msg()

//pptv
begin_msg(req_login_pptv, 0, 120)
def_string(ssessionid)
def_string(username)
end_msg()

begin_msg(ack_login_pptv, 0, 121)
def_int(errorcode)   //返回0表示成功
end_msg()

//3G门户
begin_msg(req_login_3g, 0, 122)
def_string(ssid)
def_string(token)
end_msg()

begin_msg(ack_login_3g, 0, 123)
def_int(errorcode) //返回0表示成功
end_msg()

//itools
begin_msg(req_login_itools, 0, 124)
def_string(uid)
def_string(ssessionid)
end_msg()

begin_msg(ack_login_itools, 0, 125)
def_int(errorcode)   //返回0表示成功
end_msg()

//使用登录服务器登录验证
begin_msg(req_login_verify, 0, 126)
def_string(uid)
def_string(token)
end_msg()

begin_msg(ack_login_verify, 0, 127)
def_int(errorcode)   //返回0表示成功
end_msg()

//pipaw
begin_msg(req_login_pipaw, 0, 128)
def_string(username)
def_string(time)
def_string(ssid)
end_msg()

begin_msg(ack_login_pipaw, 0, 129)
def_int(errorcode)   //返回0表示成功
end_msg()

//dianwan
begin_msg(req_login_dianwan, 0, 130)
def_string(ssid)
end_msg()

begin_msg(ack_login_dianwan, 0, 131)
def_int(errorcode)   //返回0表示成功
end_msg()

//huawei
begin_msg(req_login_huawei, 0, 132)
def_string(access_token)
end_msg()

begin_msg(ack_login_huawei, 0, 133)
def_int(errorcode)   //返回0表示成功
end_msg()

//xunlei
begin_msg(req_login_xunlei, 0, 134)
def_string(customerid)
def_string(customerkey)
end_msg()

begin_msg(ack_login_xunlei, 0, 135)
def_int(errorcode)    //返回0表示成功
end_msg()

//给力游坚果 yxpai
begin_msg(req_login_yxpaijianguo, 0, 136)
def_string(userid)
def_string(token)
def_string(channel)     // 渠道号
end_msg()

begin_msg(ack_login_yxpaijianguo, 0, 137)
def_int(errorcode)   //返回0表示成功
end_msg()


//给力游支付宝 yxpaialipay
begin_msg(req_login_yxpaialipay, 0, 138)
def_string(session)
end_msg()

begin_msg(ack_login_yxpaialipay, 0, 139)
def_int(errorcode)   //返回0表示成功
end_msg()

//绿岸  lvan
begin_msg(req_login_lvan, 0, 140)
def_string(uid)
def_string(token)
end_msg()

begin_msg(ack_login_lvan, 0, 141)
def_int(errorcode)   //返回0表示成功
end_msg()

//37wan
begin_msg(req_login_37wan, 0, 142)
def_string(token)
end_msg()

begin_msg(ack_login_37wan, 0, 143)
def_int(errorcode)   //返回0表示成功
end_msg()

//play168
begin_msg(req_login_play168, 0, 144)
def_string(uid)
def_string(token)
end_msg()

begin_msg(ack_login_play168, 0, 145)
def_int(errorcode)   //返回0表示成功
end_msg()






//快用登录
begin_msg(req_login_kuaiyong2, 0, 146)
def_string(tokenKey)
end_msg()

begin_msg(ack_login_kuaiyong2, 0, 147)
def_int(errorcode)   //返回0表示成功
end_msg()

//爱游戏
begin_msg(req_login_iyouxi, 0, 148)
def_string(session)
end_msg()

begin_msg(ack_login_iyouxi, 0, 149)
def_int(errorcode)    //返回0表示成功
end_msg()

//taomi
begin_msg(req_login_taomi, 0, 150)
	def_string(session)
	def_string(package_name)
end_msg()

begin_msg(ack_login_taomi, 0, 151)
	def_int(errorcode)
end_msg()

//facebook new
begin_msg(req_login_facebook_new, 0, 152)
	def_string(access_token)
	def_string(package_name)
	def_string(partner_name)	// 合作商 例:taomi,pay168 (注意该参数会用来组建帐号名)
end_msg()



begin_msg(ack_login_facebook_new, 0, 153)
	def_int(errorcode)
end_msg()
 
//facebook android  new
begin_msg(req_login_facebook_android_new, 0, 154)
	def_string(access_token)
	def_string(package_name)
	def_string(partner_name)	// 合作商 例:taomi,pay168 (注意该参数会用来组建帐号名)
end_msg()

begin_msg(ack_login_facebook_android_new, 0, 155)
	def_int(errorcode)
end_msg()

//google  new
begin_msg(req_login_google_new, 0, 156)
	def_string(access_token)
	def_string(package_name)
	def_string(partner_name)	// 合作商 例:taomi,pay168 (注意该参数会用来组建帐号名)
end_msg()

begin_msg(ack_login_google_new, 0, 157)
	def_int(errorcode)
end_msg()

//google android  new
begin_msg(req_login_google_android_new, 0, 158)
	def_string(access_token)
	def_string(package_name)
	def_string(partner_name)	// 合作商 例:taomi,pay168 (注意该参数会用来组建帐号名)
end_msg()

begin_msg(ack_login_google_android_new, 0, 159)
def_int(errorcode)
end_msg()

//爱贝
begin_msg(req_login_iapppay, 0, 160)
def_string(session)
end_msg()

begin_msg(ack_login_iapppay, 0, 161)
def_int(errorcode)   //返回0表示成功
end_msg()

//星月 tiantianwan
begin_msg(req_login_tiantianwan, 0, 162)
def_string(username)
def_string(logintime)
def_string(sign)
end_msg()

begin_msg(ack_login_tiantianwan, 0, 163)
def_int(errorcode)   //返回0表示成功
end_msg()

//摩掌 mzyx
begin_msg(req_login_mzyx, 0, 164)
def_string(ssid)
end_msg()

begin_msg(ack_login_mzyx, 0, 165)
def_int(errorcode)   //返回0表示成功
end_msg()

//梦城
begin_msg(req_login_dcgame, 0, 166)
def_string(userid)
def_string(userkey)
def_string(at)
end_msg()

begin_msg(ack_login_dcgame, 0, 167)
def_int(errorcode)   //返回0表示成功
end_msg()

//vivo
begin_msg(req_login_vivo, 0, 168)
def_string(authtoken)
def_string(openid)
end_msg()

begin_msg(ack_login_vivo, 0, 169)
def_int(errorcode)    //返回0表示成功
end_msg()

// 新游
begin_msg(req_login_xinyousdk_token, 0, 170)
end_msg()

begin_msg(ack_login_xinyousdk_token, 0, 171)
	def_string(token)
end_msg()

begin_msg(req_login_xinyousdk, 0, 172)
	def_string(access_token)
    def_string(platform)
    def_string(openid)
end_msg()

begin_msg(ack_login_xinyousdk, 0, 173)
	def_int(errorcode)
	def_string(name)
end_msg()

// 辣椒SC
begin_msg(req_login_SC, 0, 174)
def_string(user_id)
def_string(login_token)
end_msg()

begin_msg(ack_login_SC, 0, 175)
def_int(errorcode)
end_msg()

// jolo 平台
begin_msg(req_login_jolo, 0, 176)
def_string(account)
def_string(account_sign)
end_msg()

begin_msg(ack_login_jolo, 0, 177)
def_int(errorcode)
end_msg()

//snailgame 免商店
begin_msg(req_login_snailgame, 0, 178)
def_string(uin)
def_string(session_id)
end_msg()

begin_msg(ack_login_snailgame, 0, 179)
def_int(errorcode)
end_msg()

//moqi 魔奇
begin_msg(req_login_moqi, 0, 180)
def_string(uin)
def_string(session_key)
end_msg()

begin_msg(ack_login_moqi, 0, 181)
def_int(errorcode)
end_msg()

// xinyou mycard
begin_msg(req_login_xinyou_mycard, 0, 182)
def_string(access_token)
end_msg()

begin_msg(ack_login_xinyou_mycard, 0, 183)
def_int(errorcode)
def_string(name)
end_msg()

begin_msg(req_login_jinli, 0, 184)
def_string(playid)  // uid
def_string(token)
end_msg()

begin_msg(ack_login_jinli, 0, 185)
def_int(errorcode)
end_msg()


//37wan2
begin_msg(req_login_37wan2, 0, 186)
def_string(token)
end_msg()

begin_msg(ack_login_37wan2, 0, 187)
def_int(errorcode)   //返回0表示成功
end_msg()

//游戏酒吧
begin_msg(req_login_pubgame, 0, 188)
def_string(token)
def_string(playerId)
end_msg()

begin_msg(ack_login_pubgame, 0, 189)
def_int(errorcode)   //返回0表示成功
end_msg()

//冒泡SDK
begin_msg(req_login_mopo, 0, 190)
def_string(skyid)
def_string(ticked)
def_string(username)
end_msg()

begin_msg(ack_login_mopo, 0, 191)
def_int(errorcode)   //返回0表示成功
end_msg()

//　畅玩电信
begin_msg(req_login_cw2, 0, 192)
def_string(openId)
def_string(ip)
def_string(port)
def_string(timestamp)
def_string(sign)
end_msg()

begin_msg(ack_login_cw2, 0, 193)
def_int(errorcode)
end_msg()

//移动MM mobileMM
begin_msg(req_login_mobileMM, 0, 194)
def_string(session)
end_msg()

begin_msg(ack_login_mobileMM, 0, 195)
def_int(errorcode)    //返回0表示成功
end_msg()

//手游之家 sjwyx
begin_msg(req_login_sjwyx, 0, 196)
def_string(mid)
def_string(cpAuthtoken)
end_msg()

begin_msg(ack_login_sjwyx, 0, 197)
def_int(errorcode)   //返回0表示成功
def_string(key)
end_msg()
//海马
begin_msg(req_login_haima, 0, 198)
def_string(session)
end_msg()

begin_msg(ack_login_haima, 0, 199)
def_int(errorcode)   //返回0表示成功
end_msg()

// i4
begin_msg(req_login_i4, 0, 200)
def_string(userid)
def_string(token)
end_msg()

begin_msg(ack_login_i4, 0, 201)
def_int(errorcode)   //返回0表示成功
end_msg()

//u96
begin_msg(req_login_u96, 0, 202)
def_string(user_id)
def_string(session_id)
end_msg()

begin_msg(ack_login_u96, 0, 203)
def_int(errorcode)   //返回0表示成功
end_msg()

//安锋
begin_msg(req_login_anfan, 0, 204)
def_string(uid)
def_string(ucid)
def_string(uuid)
end_msg()

begin_msg(ack_login_anfan, 0, 205)
def_int(errorcode)   //返回0表示成功
end_msg()

//艺果
begin_msg(req_login_pada, 0, 206)
def_string(appId)
def_string(roleId)
def_string(roleToken)
end_msg()

begin_msg(ack_login_pada, 0, 207)
def_int(errorcode)   //返回0表示成功
end_msg()

//陌陌
begin_msg(req_login_momo, 0, 208)
def_string(uid)
def_string(token)
end_msg()

begin_msg(ack_login_momo, 0, 209)
def_int(errorcode)   //返回0表示成功
end_msg()

//offerme2
begin_msg(req_login_offerme2, 0, 210)
def_string(session)
def_string(package_name)
end_msg()

begin_msg(ack_login_offerme2, 0, 211)
def_int(errorcode)	//返回0表示成功
end_msg()

//pc
begin_msg(req_login_pc, 0, 212)
def_string(uid)
def_string(sessionId)
end_msg()

begin_msg(ack_login_pc, 0, 213)
def_int(errorcode)	//返回0表示成功
def_string(appSecret)
end_msg()

// 视遥
begin_msg(req_login_shiyao, 0, 214)
def_string(guid)
def_string(tokenId)
end_msg()

begin_msg(ack_login_shiyao, 0, 215)
def_int(errorcode)   // 返回0表示成功
end_msg()

// 越進
begin_msg(req_login_luckystar, 0, 216)
def_string(uid)
def_string(accessToken)
end_msg()

begin_msg(ack_login_luckystar, 0, 217)
def_int(errorcode)   // 返回0表示成功
end_msg()

//新泛联
begin_msg(req_login_untx, 0, 218)
def_string(user_id)
end_msg()

begin_msg(ack_login_untx, 0, 219)
def_int(errorcode)   // 返回0表示成功
end_msg()

begin_msg(req_login_52fun, 0, 220)
def_string(userId)
def_string(checkCode)
end_msg()

begin_msg(ack_login_52fun, 0, 221)
def_int(errorcode)   // 返回0表示成功
end_msg()

//五彩时空
begin_msg(req_login_wcsk, 0, 222)
def_string(session)
end_msg()

begin_msg(ack_login_wcsk, 0, 223)
def_int(errorcode)   // 返回0表示成功
end_msg()

begin_msg(req_login_winner, 0, 224)
def_string(acc_name)
def_string(password)
def_string(system) // ios, android
end_msg()

begin_msg(ack_login_winner, 0, 225)
def_string(acc_code)
def_int(errorcode)   // 返回0表示成功
def_string(ret_code)	// 平台返回的错误号
def_string(msg)
end_msg()

begin_msg(req_register_winner, 0, 226)
def_string(acc_name)
def_string(password)
def_string(confirmPassword)
def_string(system) // ios, android
end_msg()

begin_msg(ack_register_winner, 0, 227)
def_string(acc_code)
def_int(errorcode)   // 返回0表示成功
def_string(ret_code)	// 平台返回的错误号
def_string(msg)
end_msg()

//E路wifi
begin_msg(req_login_elu, 0, 228)
def_string(username)
def_string(password)
end_msg()

begin_msg(ack_login_elu, 0, 229)
def_int(errorcode)   // 返回0表示成功
end_msg()

// 多玩YY
begin_msg(req_login_yy, 0, 230)
def_string(ssid)
def_string(account)
def_string(time)
end_msg()

begin_msg(ack_login_yy, 0, 231)
def_int(errorcode)   // 返回0表示成功
end_msg()

// 巴士在线
begin_msg(req_login_bus, 0, 232)
def_string(session)
end_msg()

begin_msg(ack_login_bus, 0, 233)
def_int(errorcode)   // 返回0表示成功
end_msg()

// 酷派 coolpad
begin_msg(req_login_coolpad, 0, 234)
def_string(session)
end_msg()

begin_msg(ack_login_coolpad, 0, 235)
def_int(errorcode)   // 返回0表示成功
end_msg()

// 爱奇艺
begin_msg(req_login_aiqiyi, 0, 236)
def_string(uid)
def_string(time)
def_string(sign)
end_msg()

begin_msg(ack_login_aiqiyi, 0, 237)
def_int(errorcode)    // 返回0表示成功
end_msg()

// 暴风 baofeng
begin_msg(req_login_baofeng, 0, 238)
def_string(ticket)
end_msg()

begin_msg(ack_login_baofeng, 0, 239)
def_int(errorcode)   // 返回0表示成功
end_msg()




begin_msg(req_login_sdk_channel, 0, 250)
def_string(sdkChannel)		// 验证的渠道,封装好的sdk渠道名.如:channel_uc
def_string(verifyStr)		// 验证需要的参数, json串
def_string(system)			// ios 或 android
def_string(packageName)		// 包渠道名称, 包名
def_string(partnerName)		// 合作商的名称, 现在与包名一至就好, (此参数,用来当帐户的前辍)
end_msg()

begin_msg(ack_login_sdk_channel, 0, 251)
def_int(errorcode)   // 返回0表示成功
def_string(ret)		// 返回的JSON串
end_msg()

begin_msg(req_login_tw177, 0, 252)
def_string(token)
def_string(userid)
end_msg()

begin_msg(ack_login_tw177, 0, 253)
def_int(errorcode)   // 返回0表示成功
end_msg()

begin_msg(req_login_tencent, 0, 254)
def_string(openid)
end_msg()

begin_msg(ack_login_tencent, 0, 255)
def_int(errorcode)   //返回0表示成功
end_msg()


//XY 苹果助手游戏联运SDK
begin_msg(req_login_xyzs, 0, 256)
def_string(uid)
def_string(token)
end_msg()

begin_msg(ack_login_xyzs, 0, 257)
def_int(errorcode)   //返回0表示成功
end_msg()

begin_msg(req_login_gamedog, 0, 258)
def_string(member_id)
def_string(token)
end_msg()

begin_msg(ack_login_gamedog, 0, 259)
def_int(errorcode)	//返回0表示成功
end_msg()

// weixin
begin_msg(req_login_weixin, 0, 260)
def_string(access_token)
def_string(open_id)
def_string(package_name)
def_string(partner_name)	// 合作商 例: (注意该参数会用来组建帐号名)
def_string(system)			// ios or android
end_msg()

begin_msg(ack_login_weixin, 0, 261)
def_int(errorcode)
end_msg()

// 联通
begin_msg(req_login_liantong, 0, 262)
def_string(access_token)
end_msg()

begin_msg(ack_login_liantong, 0, 263)
def_int(errorcode)
def_string(uid)
end_msg()

// 酷派登录 酷派自己的sdk
begin_msg(req_login_coolpad2, 0, 264)
def_string(authorization_code)
end_msg()

begin_msg(ack_login_coolpad2, 0, 265)
def_int(errorcode)   // 返回0表示成功
def_string(access_token)
def_string(openid)
end_msg()

// 赛众
begin_msg(req_login_saizhong, 0, 266)
def_string(session)
end_msg()

begin_msg(ack_login_saizhong, 0, 267)
def_int(uid)
def_int(errorcode)   // 返回0表示成功
end_msg()

// 同步推
begin_msg(req_login_tongbutui, 0, 268)
def_string(session)
def_string(userid)
end_msg()

begin_msg(ack_login_tongbutui, 0, 269)
def_int(errorcode)   // 返回0表示成功
end_msg()

// 飓风互动
begin_msg(req_login_iApple, 0, 270)
def_string(session)
def_string(userid)
end_msg()

begin_msg(ack_login_iApple, 0, 271)
def_int(errorcode)
end_msg()

// 07073数游
begin_msg(req_login_shuyou, 0, 272)
    def_string(username)
    def_string(session)
end_msg()

begin_msg(ack_login_shuyou, 0, 273)
    def_int(errorcode)   // 返回0表示成功
end_msg()

// 傲天手游
begin_msg(req_login_aotian, 0, 274)
def_string(uid)
def_string(userName)
def_string(loginSign)
end_msg()

begin_msg(ack_login_aotian, 0, 275)
def_int(errorcode)   // 返回0表示成功
end_msg()

// 卓动
begin_msg(req_login_zhuodong, 0, 276)
def_string(sessiontoken)
def_string(gameuid)
end_msg()

begin_msg(ack_login_zhuodong, 0, 277)
def_int(errorcode)    // 返回0表示成功
end_msg()

// m4399
begin_msg(req_login_m4399, 0, 278)
def_string(uid)
def_string(token)
end_msg()

begin_msg(ack_login_m4399, 0, 279)
def_int(errorcode)   // 返回0表示成功
end_msg()

// douwan93
begin_msg(req_login_douwan93, 0, 280)
def_string(token)
def_string(account_id)
end_msg()

begin_msg(ack_login_douwan93, 0, 281)
def_int(errorcode)   // 返回0表示成功
end_msg()

//百度91
begin_msg(req_login_baidu91, 0, 282)
def_string(uid)
def_string(sessionid)
end_msg()

begin_msg(ack_login_baidu91, 0, 283)
def_int(errorcode)
end_msg()

/*****************************************************/