#ifndef __GameSrv__HttpRequestApp__
#define __GameSrv__HttpRequestApp__

#include "HttpRequestConfig.h"


#define HTTP_CUSTOM_MAIL_INFO "custom_mail_info"
#define HTTP_ACTIVATION "activation"
#define HTTP_REGISTER_ORDER "register_order"
#define HTTP_XINYOU_REQUEST_ID "get_xinyou_request_id"
#define HTTP_GOOGLE_PAY "google_pay"
#define HTTP_WINNER_IOS_PAY "winner_ios_pay"
#define HTTP_WINNER_GOOGLE_WALLET_PAY "winner_google_wallet_pay"
#define HTTP_WINNER_FORTUMO_PAY "winner_fortumo_pay"
#define HTTP_WINNER_LOGOUT "winner_Logout"
#define HTTP_WEB_STATE "web_state"
#define HTTP_IP_LIST "ip_list"
#define HTTP_UP_SERVER "upserver"

void httpRequestCbActivation(int roleid, int sessionid, const char *ret);

void initHttpRequestApp();


#endif /* defined(__GameSrv__HttpQuestApp__) */
