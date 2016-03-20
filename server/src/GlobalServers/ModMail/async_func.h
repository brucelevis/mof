
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
#pragma once

#include "ModMail.h"

//获取邮件列表的结构
struct MailListItem
{
	string mailid;		//邮件的ID  Guid
	string title;		//邮件的标题
	string sender;		//邮件的发送者 //wstring
	string receiver;	//邮件的接受者 //wstring
	int isReaded;		//邮件是否已读
	string send_time;	//邮件的发送时间
	int hasAttach;		//邮件是否有附件
	time_t tm_send_time;
};

//发送邮件异步处理
extern void AsyncProc_send_mail(CMsgTyped* pmsg, sql::Statement* pConn);
//查看邮件列表异步处理
extern void AsyncProc_mail_list(CMsgTyped* pmsg, sql::Statement* pConn);
//阅读邮件异步处理
extern void AsyncProc_read_mail(CMsgTyped* pmsg, sql::Statement* pConn);
//删除邮件异步处理
extern void AsyncProc_delete_mail(CMsgTyped* pmsg, sql::Statement* pConn);
//获取邮件附件异步处理
extern void AsyncProc_mail_attachment(CMsgTyped* pmsg, sql::Statement* pConn);



