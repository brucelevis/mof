/*-------------------------------------------------------------------------
	created:	2010/09/16  17:50
	filename: 	e:\Project_SVN\Server\GamesScene\GameScene.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "ModMail.h"
#include "async_func.h"
#include "RoleRegister.h"
#include "MailListCache.h"
#include "SysMailCache.h"

IGlobal* g_pGame = NULL;

extern "C"
{
    void OnHeartBeat(int nBeat)
    {
        // 每小时清理垃圾邮件
        if (nBeat % 3600 == 0)
        {
            g_RoleList.CheckDeleteRubish();
        }

        // 每10秒检测列表缓存， 过期缓存将不存在内存中
        // 每10秒重新加载系统邮件列表， 清除过期邮件
        if (nBeat % 10 == 0)
        {
            g_MailCache.ProcessOutDates();
            g_sysMails.ReLoad();
        }
    }
}

void RegisterCallBack()
{
	REG_PUBLIC_MESSAGE(GM_SEND_MAIL);			 // 发送邮件
	REG_PUBLIC_MESSAGE(GM_MAIL_LIST);			 // 获取邮件列表
	REG_PUBLIC_MESSAGE(GM_READ_MAIL);			 // 查看邮件
	REG_PUBLIC_MESSAGE(GM_DELETE_MAIL);			 // 删除邮件
	REG_PUBLIC_MESSAGE(GM_GET_MAIL_ATTACHMENT);  // 获取邮件附件
	REG_PUBLIC_MESSAGE(GM_REGISTER_PLAYER);      // 邮件系统注册玩家
	REG_PUBLIC_MESSAGE(GM_UNREGPLAYER);          // 邮件系统注销玩家
	REG_PUBLIC_MESSAGE(GM_RENAME_INDEX);

	//这个从db中获取已经有的邮件记录，如果有玩家注册 (暂时改成登陆)，会加一个进去，如果db中没有。
	// 载入位置信息
	g_RoleList.LoadRoleList(g_pGame);

    // 系统邮件载入内存
	g_sysMails.Load();
}

// 发送邮件
void PUB_MSG(GM_SEND_MAIL)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	const char* Sender = pmsg->StringVal();
	const char* ReceName = pmsg->StringVal();
	const char* Title = pmsg->StringVal();
	const char* MailContent = pmsg->StringVal();
	const char* MailAttach = pmsg->StringVal();
	const char* params = pmsg->StringVal();

	// 普通邮件
	if (strlen(ReceName) > 0)
	{
		//if ( !g_RoleList.FindRole(ReceName) )
		//{
			// 返回错误
//			CMsgTyped ret;
//			ret.SetInt(0);
//			ret.SetInt(receiver);
//			ret.SetInt(msgId);
//			ret.SetInt(GE_PLAYER_NOT_FOUND);// 查无此人 300
//			ret.SetString(MailAttach);
//			ret.SetString(Sender);
//			ret.SetString(ReceName);
//			ret.SetString(Title);
//			ret.SetString(params);
//
//			g_pGame->SendMessage(id, &ret);
//
//			return;
//		}

		//// 给发送者写入邮件
		if (strlen(Sender) > 0)
		{
			CMsgTyped SendMail;
			SendMail.SetInt(receiver);
			SendMail.SetInt(msgId);
			SendMail.SetInt((int)id.nIndex);
			SendMail.SetInt((int)id.nIdent);

			SendMail.SetString(Sender);
			SendMail.SetString(Title);
			SendMail.SetString(Sender);
			SendMail.SetString(ReceName);
			SendMail.SetString(MailAttach);
			SendMail.SetString(MailContent);
			SendMail.SetString(params);
			SendMail.SetInt(0); // not send

			g_pGame->Call(g_poolId , AsyncProc_send_mail, &SendMail);
		}

		// 给接收者写入邮件
		{
			CMsgTyped SendMail;
			SendMail.SetInt(receiver);
			SendMail.SetInt(msgId);
			SendMail.SetInt((int)id.nIndex);
			SendMail.SetInt((int)id.nIdent);

			SendMail.SetString(ReceName);
			SendMail.SetString(Title);
			SendMail.SetString(strlen(Sender) > 0 ? Sender : "系统"); //wcslen
			SendMail.SetString(ReceName);
			SendMail.SetString(MailAttach);
			SendMail.SetString(MailContent);
			SendMail.SetString(params);
			SendMail.SetInt(1);//send_back

			// 位置信息
			g_pGame->Call(g_poolId , AsyncProc_send_mail, &SendMail);
		}
	}
}

// 获取邮件列表
void PUB_MSG(GM_MAIL_LIST)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	//_Debug("PUB_MSG(GM_MAIL_LIST)");
	CMsgTyped MailList;

	const char* PlayerName = pmsg->StringVal();

	int from = pmsg->IntVal();
	int count = pmsg->IntVal();
	if (from < 1 || count < 0) return;

	MailList.SetInt( receiver );
	MailList.SetInt(msgId);
	MailList.SetInt((int)id.nIndex);
	MailList.SetInt((int)id.nIdent);
	MailList.SetString(PlayerName);
	MailList.SetInt( from );
	MailList.SetInt( count );

    // 读取邮件列表
	g_pGame->Call(g_poolId, AsyncProc_mail_list, &MailList);

    // 尝试创建角色
	g_RoleList.CreateRole(PlayerName);
}

// 查看邮件
void PUB_MSG(GM_READ_MAIL)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	//_Debug("PUB_MSG(GM_READ_MAIL)");

	CMsgTyped ReadMail;
	const char* MailID = pmsg->StringVal();
	const char* PlayerName = pmsg->StringVal();
	const char* MailTitle = pmsg->StringVal();

	ReadMail.SetInt(receiver);
	ReadMail.SetInt(msgId);
	ReadMail.SetInt((int)id.nIndex);
	ReadMail.SetInt((int)id.nIdent);
	ReadMail.SetString(MailID);
	ReadMail.SetString( PlayerName );
	ReadMail.SetString(MailTitle);

	g_pGame->Call(g_poolId, AsyncProc_read_mail, &ReadMail);

}

// 删除邮件
void PUB_MSG(GM_DELETE_MAIL)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	//_Debug("PUB_MSG(GM_DELETE_MAIL)");

	CMsgTyped DeleteMail;

	const char* MailID = pmsg->StringVal();
	const char* PlayerName = pmsg->StringVal();

	DeleteMail.SetInt(receiver);
	DeleteMail.SetInt(msgId);
	DeleteMail.SetInt((int)id.nIndex);
	DeleteMail.SetInt((int)id.nIdent);
	DeleteMail.SetString(MailID);
	DeleteMail.SetString(PlayerName);

	g_pGame->Call(g_poolId, AsyncProc_delete_mail, &DeleteMail);

}

// 获取邮件附件
void PUB_MSG(GM_GET_MAIL_ATTACHMENT)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	//_Debug("PUB_MSG(GM_GET_MAIL_ATTACHMENT)");

	const char* MailID = pmsg->StringVal();
	const char* PlayerName = pmsg->StringVal();
	const char* MailTitle = pmsg->StringVal();

	CMsgTyped Get_Attachment;

	Get_Attachment.SetInt(receiver);
	Get_Attachment.SetInt(msgId);
	Get_Attachment.SetInt((int)id.nIndex);
	Get_Attachment.SetInt((int)id.nIdent);
	Get_Attachment.SetString(MailID);
	Get_Attachment.SetString(PlayerName);
	Get_Attachment.SetString(MailTitle);

	g_pGame->Call(g_poolId, AsyncProc_mail_attachment, &Get_Attachment);
}

// 邮件系统注册玩家
void PUB_MSG(GM_REGISTER_PLAYER)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{

	//printf("GM_REGISTER_PLAYER \n");

	const char* PlayerName = pmsg->StringVal(); //wchar_t WideStrVal

	// 注册此玩家
	g_RoleList.CreateRole(PlayerName);

}

// 邮件系统注销玩家
void PUB_MSG(GM_UNREGPLAYER)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	//printf("GM_UNREGPLAYER \n");

	const char* PlayerName = pmsg->StringVal(); //wchar_t WideStrVal

	// 注册此玩家
	g_RoleList.DeleteRole(PlayerName);

}

void PUB_MSG(GM_RENAME_INDEX)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
    // 收到重名消息
    const char* playerName = pmsg->StringVal();
    const char* newName = pmsg->StringVal();

    // 各模块重命名玩家
    bool isOk = false;
    if (isOk = g_RoleList.Rename(playerName, newName))
    {
        g_MailCache.Rename(playerName, newName);
        g_sysMails.Rename(playerName, newName);
    }

    // 发送回应
    CMsgTyped ret;
    ret.SetInt(0);
    ret.SetInt(receiver);
    ret.SetInt(msgId);
    ret.SetInt(isOk ? GE_OK : GE_PLAYER_NOT_FOUND);// 查无此人 300
    ret.SetString(playerName);
    ret.SetString(newName);

    g_pGame->SendMessage(id, &ret);
}



