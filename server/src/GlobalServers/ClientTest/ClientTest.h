// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
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
	GLOBAL_ADD_PLAYER,           // ע�����
	GLOBAL_DELETE_PLAYER,        // ɾ�����
	GLOBAL_LOGIN_PLAYER,        // ��ҵ�½������������Ҫ��������ת�ƣ�
	GLOBAL_LOGOUT_PLAYER,       // ��ҵǳ�
	GLOBAL_REGISTER_SERVER,     // ������ע��
	GLOBAL_UNREGISTER_SERVER,   // ������ע��(������ע��ʱ,
	GLOBAL_KEEP_ALIVE,          // ��������

    // �ʼ�������
	GM_SEND_MAIL=2000,      // �����ʼ�
	GM_MAIL_LIST,			// ��ȡ�ʼ��б�
	GM_READ_MAIL,			// �鿴�ʼ�
	GM_DELETE_MAIL,         // ɾ���ʼ�
	GM_GET_MAIL_ATTACHMENT, // ��ȡ�ʼ�����
	GM_REGISTER_PLAYER,     // ע�����
	GM_UNREGPLAYER,         // ע�����
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
