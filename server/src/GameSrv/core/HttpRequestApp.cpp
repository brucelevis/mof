
#include "HttpRequestApp.h"
#include "main.h"
#include "Game.h"
#include "json/json.h"
#include "CustomGift.h"
#include "CustomMailApp.h"
#include "WebStateMgr.h"
#include "tinyxml.h"
#include "tinystr.h"
#include "Role.h"
#include "EnumDef.h"
#include "ActivationCodeMgr.h"

using namespace Xml;

void activationDate(ack_activation &ack, Json::Value &value)
{
	try {
		Json::Value obj = value["obj"][(uint)0];
		ack.startDate = obj["startDate"].asString();
		ack.endDate = obj["endDate"].asString();
	} catch(...) {

	}
}

#pragma mark - 激活码获取
void httpRequestCbActivation(int roleid, int sessionid, const char *ret)
{
    ack_activation ack;
    ack.errorcode = enActivationReuslt_UnknowError;
    Json::Value retval;
    do {
        if (!xyJsonSafeParse(ret, retval)) {
            xyerr("激活码 : http ret err，no JSON. %s", ret);
            break;
        }
        int bet = retval["ret"].isNull() ? -1 : retval["ret"].asInt();
        // 状态码有误
        if(bet != 0) {
            switch(bet) {
				case 8000:
					ack.errorcode = enActivationReuslt_Invalid; break;
				case 8001:
					ack.errorcode = enActivationReuslt_Use; break;
				case 8002:
					ack.errorcode = enActivationReuslt_Platform; break;
				case 8003:
					ack.errorcode = enActivationReuslt_ReceiveType; break;
				case 8004:
					ack.errorcode = enActivationReuslt_End;
					activationDate(ack, retval);
					break;
				case 8005:
					ack.errorcode = enActivationReuslt_SelfUse; break;
				case 8006:
					ack.errorcode = enActivationReuslt_Server; break;
				case 8007:
					ack.errorcode = enActivationReuslt_Start;
					activationDate(ack, retval);
					break;
				case 8008:
					ack.errorcode = enActivationReuslt_Os; break;
				default:
					xyerr("激活码 : http ret err, no state. %s", ret); break;
            }
            break;
        }
		// 参数
        int code_id;
        int code_type;
        int code_useType;
		Json::Value obj;
        try {
			obj = retval["obj"][(uint)0];
            code_id = xyJsonAsInt(obj["code_id"]);
            code_type = xyJsonAsInt(obj["type"]);
            code_useType = xyJsonAsInt(obj["useType"]);
        } catch(...) {
			xyerr("激活码 : 获取参数失败", ret);
            break;
        }
		// 检查code_id
		if (code_id <= 0) {
			xyerr("激活码 : 得到的激活码code_id有误%d", code_id);
			break;
		}

        Role *role = SRoleMgr.GetRole(roleid);
        if(NULL == role) {
			// 离线动作
			NewRoleProperty property;
			property.load(roleid);
			property.setActivation(ret);
			property.save();
            break;
        }
		
		// 激活码使用类型为一对多
        if(code_useType == 1) {
            ActivationCodeMgr *acm = role->getActivationCodeMgr();
            ActivationCode ac(code_type, code_id);
            if(!acm->add(ac)) {
                ack.errorcode = enActivationReuslt_ReceiveType;
                break;
            }
            ActivationCodeMgr::save(roleid, ac);
        }
		
		// 组自定义礼包
		CustomGift cg;
		cg.setName(obj["name"].asString());
		cg.setDesc(obj["desc"].asString());
		cg.addCustomGiftItem(obj["items"].asString());
		string giftStr = cg.formatJson();
		
        // 添加到背包, 注意378001是系统表定义的(自义定礼包id), 有特殊的包类型
        ItemArray items;
        GridArray effgrids;
        items.push_back(ItemGroup(378001, 1, giftStr));

        if(role->preAddItems(items, effgrids) != CE_OK){
            xyerr("激活码 : role %d add bag err, %s", roleid, ret);
            ack.errorcode = enActivationReuslt_BagFull;
            break;
        }

//        role->updateBackBag(effgrids, items, true, "激活码");
        role->playerAddItemsAndStore(effgrids, items, "激活码", true);
		// 获取日志
		Xylog log(eLogName_Activation, roleid);
		log<<code_id<<code_type<<giftStr<<ret;
		
        ack.errorcode = enActivationReuslt_Success;
    }while (false);
	
	sendNetPacket(sessionid, &ack);
}

#pragma mark - 申请订单
void httpRequestCbRegisterOrder(int roleid, int sessionid, const char* ret)
{
	ack_register_order ack;
	ack.errorcode = enRegisterOrderReuslt_UnknowError;
    Json::Value retval;
    do {
        if (!xyJsonSafeParse(ret, retval)) {
			ack.errorcode = enRegisterOrderReuslt_Timeout;
            xyerr("申请订单 : http ret err，no JSON.");
            break;
        }
        int bet = retval["ret"].isNull() ?  -1 : retval["ret"].asInt();
        // 状态码有误
        if(bet != 0) {
            switch(bet) {
				case 8102:
					ack.errorcode = enRegisterOrderReuslt_InsertTbErr;
					xyerr("申请订单 : insert tb err.");
					break;
            }
            break;
        }
		try {
            Json::Value obj = retval["obj"][(uint)0];
            ack.orderId = obj["sn"].asString();
			ack.amount = atof(obj["amount"].asString().c_str());
        } catch(...) {
            break;
        }
		// 订单号是空
		if (ack.orderId.empty()) {
			break;
		}
		ack.errorcode = enRegisterOrderReuslt_Success;
    }
	while (false);
		
	sendNetPacket(sessionid, &ack);
}

#pragma mark - 得到新游的请求id
void httpCbGetXinyouRequest(int roleid, int sessionid, const char* ret)
{
    ack_get_request_id ack;
    if (0 == strcmp(ret, ""))
    {
        ack.errorcode = -1;
    }
    else
    {
        ack.errorcode = 0;
        ack.id = ret;
    }
    sendNetPacket(sessionid, &ack);
}

#pragma mark - 定制邮件信息
void httpCbCustomMailInfo(int roleid, int sessionid, const char* ret)
{
	g_CustomMailAppMou.load(ret);
}

#pragma mark - google 交易
void httpCbGooglePay(int roleid, int sessionid, const char* ret)
{
	ack_google_pay ack;
    Json::Value val;
	if (!xyJsonSafeParse(ret, val)) {
		return;
	}
	if (val["errorcode"].isNull()) {
		return;
	}
	ack.errorcode = xyJsonAsInt(val["errorcode"]);
}

int winnerPayGetRetcode(const char* ret)
{
	TiXmlDocument document;
	document.Parse(ret);
	TiXmlElement* element = document.RootElement();
	if (NULL == element) {
		return 0;
	}
	TiXmlElement* elemRetCode = element->FirstChildElement("ret_code");
	if (NULL == elemRetCode) {
		return 0;
	}
	return Utils::safe_atoi(elemRetCode->GetText());
}

int winnerPayRetcodeProc(int ret_code)
{
	int errorcode = enAppleRecharge_Undefine;
	// 处理错误编号
	switch (ret_code) {
		case 0:
			errorcode = enAppleRecharge_ServerError;
			break;
		case 101:
		case 204:
			errorcode = enAppleRecharge_Success;
			break;
		case 100:
		case 102:	// add point接口返回失败
		case 301:
		case 302:
		case 303:
			errorcode = enAppleRecharge_ServerBusy;
			break;
		default:
			errorcode = enAppleRecharge_Fail;
			break;
	}
	return errorcode;
}

#pragma mark - winner ios 交易
void httpCbWinnerIosPay(int roleid, int sessionid, const char* ret)
{
	Xylog log(eLogName_WinnerIosPayRet, roleid);
	log << ret;
	
	int ret_code = winnerPayGetRetcode(ret);
	log << ret_code;
	
	ack_winner_ios_pay ack;
	ack.errorcode = winnerPayRetcodeProc(ret_code);
	sendNetPacket(sessionid, &ack);
}

#pragma mark - winner google wallet 交易
void httpCbWinnerGoogleWalletPay(int roleid, int sessionid, const char* ret)
{
	Xylog log(eLogName_WinnerGoogleWalletPayRet, roleid);
	log << ret;
	
	int ret_code = winnerPayGetRetcode(ret);
	log << ret_code;
	
	ack_winner_google_wallet_pay ack;
	ack.errorcode = winnerPayRetcodeProc(ret_code);
	sendNetPacket(sessionid, &ack);
}

#pragma mark - winner google wallet 交易
void httpCbWinnerFortumoPay(int roleid, int sessionid, const char* ret)
{
	Xylog log(eLogName_WinnerFortumoPayRet, roleid);
	log << ret;
	
	int ret_code = winnerPayGetRetcode(ret);
	log << ret_code;
	
	ack_winner_fortumo_pay ack;
	ack.errorcode = winnerPayRetcodeProc(ret_code);
	sendNetPacket(sessionid, &ack);
}

#pragma mark - winner logout
void httpCbWinnerLogout(int roleid, int sessionid, const char* ret)
{
	ack_logout_winner ack;
	ack.errorcode = 0;
	sendNetPacket(sessionid, &ack);
}

#pragma mark - web state
void httpCbWebState(int roleid, int sessionid, const char*ret)
{
	g_WebStateMou.load(ret);
}

#pragma mark - ip list
void httpCbIpList(int roleid, int sessionid, const char*ret)
{
	g_WebStateMou.loadIpList(ret);
}

#pragma mark - up server
void httpCbUpServer(int roleid, int sessionid, const char*ret)
{
	
}
/*
    注册回调函数,程序运行时初始
*/
void initHttpRequestApp()
{
    _HttpRequest.addCallBack(HTTP_ACTIVATION, httpRequestCbActivation);
	_HttpRequest.addCallBack(HTTP_REGISTER_ORDER, httpRequestCbRegisterOrder);
    _HttpRequest.addCallBack(HTTP_XINYOU_REQUEST_ID, httpCbGetXinyouRequest);
	_HttpRequest.addCallBack(HTTP_CUSTOM_MAIL_INFO, httpCbCustomMailInfo);
	_HttpRequest.addCallBack(HTTP_GOOGLE_PAY, httpCbGooglePay);
	_HttpRequest.addCallBack(HTTP_WINNER_IOS_PAY, httpCbWinnerIosPay);
	_HttpRequest.addCallBack(HTTP_WINNER_GOOGLE_WALLET_PAY, httpCbWinnerGoogleWalletPay);
	_HttpRequest.addCallBack(HTTP_WINNER_FORTUMO_PAY, httpCbWinnerFortumoPay);
	_HttpRequest.addCallBack(HTTP_WINNER_LOGOUT, httpCbWinnerLogout);
	_HttpRequest.addCallBack(HTTP_WEB_STATE, httpCbWebState);
	_HttpRequest.addCallBack(HTTP_IP_LIST, httpCbIpList);
	_HttpRequest.addCallBack(HTTP_UP_SERVER, httpCbUpServer);
}

