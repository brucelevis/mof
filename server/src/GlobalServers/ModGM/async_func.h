
/********************************************************************
	created:	2010/11/15
	created:	15:11:2010   15:12
	filename: 	e:\SVN\服务器端\Source\ModLogin\async_func.cpp
	file path:	e:\SVN\服务器端\Source\ModLogin
	file base:	async_func
	file ext:	cpp
	author:

	purpose:
*********************************************************************/

#include "ModGM.h"

//发送邮件异步处理
extern void AsyncProc_send_mail(CMsgTyped* pmsg, sql::Statement* pConn);

//异步函数调用发送群邮件
extern void AsyncProc_send_mailAll(CMsgTyped* pmsg, sql::Statement* pConn);

extern void AsyncProp_LoadGmTasks(CMsgTyped* pmsg, sql::Statement* pConn);

extern void AsyncProc_HandleGmTask(CMsgTyped* pmsg, sql::Statement* pConn);

extern void AsyncProc_SaveNotify(CMsgTyped* pmsg, sql::Statement* pConn);
extern void AsyncProc_DeleteNotify(CMsgTyped* pmsg, sql::Statement* pConn);

extern void AsyncProc_SaveBanRole(CMsgTyped* pmsg, sql::Statement* pConn);
extern void AsyncProc_DeleteBanRole(CMsgTyped* pmsg, sql::Statement* pConn);

extern void AsyncProc_SaveBanChat(CMsgTyped* pmsg, sql::Statement* pConn);
extern void AsyncProc_DeleteBanChat(CMsgTyped* pmsg, sql::Statement* pConn);

