// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#ifndef CLIENT_TEST_H
#define CLIENT_TEST_H
#include <stdio.h>
#include <iostream>
#include <string>
#include "Utils.h"
#include <pthread.h>
#include <semaphore.h>

#define RESRV 0
#define RECVER0 0
#define RECVER1 19999

enum GLOBAL_MESSAGE
{
	GLOBAL_ADD_PLAYER,           // 注册玩家
	GLOBAL_DELETE_PLAYER,        // 删除玩家
	GLOBAL_LOGIN_PLAYER,        // 玩家登陆（公共服务器要负责数据转移）
	GLOBAL_LOGOUT_PLAYER,       // 玩家登出
	GLOBAL_REGISTER_SERVER,     // 服务器注册
	GLOBAL_UNREGISTER_SERVER,   // 服务器注销(服务器注销时,
	GLOBAL_KEEP_ALIVE,          // 保持连接

    // 邮件服务器
	GM_SEND_MAIL=2000,      // 发送邮件
	GM_MAIL_LIST,			// 获取邮件列表
	GM_READ_MAIL,			// 查看邮件
	GM_DELETE_MAIL,         // 删除邮件
	GM_GET_MAIL_ATTACHMENT, // 获取邮件附件
	GM_REGISTER_PLAYER,     // 注册玩家
	GM_UNREGPLAYER,         // 注销玩家
};

class CMyAsyncSock : public CAsyncSocket
{
protected:
	virtual void OnReceive(CMsgTyped* pmsg);
	virtual void OnConnected()
	{
	    printf("connet server succeed!\n");
	}
};

class IMsgProc
{
public:
    virtual void OnRequest(StringArray& sa, CMsgTyped& out){};
    virtual void OnResponse(CMsgTyped* pmsg){};
    string name;
    UI32 cmdId;
    UI32 recver;
};

extern CMyAsyncSock g_sock;

class MsgMgr
{
public:
    void Reg(IMsgProc* pProc)
    {
        if (!pProc) return;
        if (Find(pProc->name.c_str())) return;
        m_ProcList.push_back( pProc );

        printf("add:  %s\n", pProc->name.c_str());
    }
    IMsgProc* Find(const char* name)
    {
        for(size_t i=0; i < m_ProcList.size(); i++)
        {
            if (strcmp( m_ProcList[i]->name.c_str(), name) == 0) return  m_ProcList[i];
        }
        return NULL;
    }
    IMsgProc* Find(UI32 cmdId)
    {
        for(size_t i=0; i < m_ProcList.size(); i++)
        {
            if (m_ProcList[i]->cmdId == cmdId) return  m_ProcList[i];
        }
        return NULL;
    }
    void OnRequest(StringArray& sa)
    {
        IMsgProc* pProc = Find( sa[0].c_str() );
        if (  pProc )
        {
            CMsgTyped out;
            out.SetInt( 0 );
            out.SetInt( pProc->recver );
            out.SetInt( pProc->cmdId );
            pProc->OnRequest( sa, out );

            // fasong xiaoxi
            g_sock.SendMessage(&out, false);
        }
        else printf("Unknown Request '%s'\n", sa[0].c_str());
    }
    void OnResponse(UI32 cmdId, CMsgTyped* pmsg)
    {
        IMsgProc* pProc = Find( cmdId );
        if (  pProc )
        {
            pProc->OnResponse( pmsg );
        }
        else
        {
           // printf("recv msg <<< ");
           // pmsg->Dump();
        }
    }
    vector<IMsgProc*> m_ProcList;
};

#endif
