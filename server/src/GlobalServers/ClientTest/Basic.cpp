/*-------------------------------------------------------------------------
	created:	2010/10/09  13:45
	filename: 	e:\SVN\服务器端\Source\ClientTest\TestBattle.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/
#include "ClientTest.h"

extern CMyAsyncSock g_sock;
extern MsgMgr g_MsgMgr;

#pragma warning(disable: 4267)
#pragma warning(disable: 4305)

extern char recvBuffer[1024*55];
typedef std::vector<std::string> StringArray;


void Split(StringArray& paths, const char* szInput, char div)
{
	// 清空结果
	paths.clear();

	if (!szInput)
	{
		return;
	}

	char* beg = (char*)szInput;
	char* cur = (char*)szInput;
	while (*cur != 0)
	{
		if (*cur == div)
		{
			// 找到一个分隔符
			if (cur - beg > 0)
			{
				std::string s;

				s.resize(cur - beg);
				memcpy(&s[0], beg, cur - beg);
				paths.push_back(s);
			}
			else
			{
				paths.push_back(std::string());
			}

			beg = cur+1;
		}

		cur ++;
	}

	if (cur - beg > 0)
	{
		std::string s;

		s.resize(cur - beg);
		memcpy(&s[0], beg, cur - beg);
		paths.push_back(s);
	}
}

// 发送心跳
void SendHeartBeat()
{
    /*
    UI32 nReserv = msg.IntVal();
    nReceiver = msg.IntVal();
    nCommandId = msg.IntVal();*/

	// 发送网络消息，登陆消息
	CMsgTyped msg;
	msg.SetInt(RESRV);// 1 表示系统消息
	msg.SetInt(RECVER0);// 1 消息表示登陆
	msg.SetInt(GLOBAL_KEEP_ALIVE);

	if (g_sock.SendMessage(&msg))
	{
	}
}

/////////////////////////
class reg_server : public IMsgProc
{
public:
    reg_server()
    {
        cmdId = GLOBAL_REGISTER_SERVER;
        recver = RECVER0;
        name = "reg";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*int nServerId = pmsg->IntVal();
        const char* szServerName = pmsg->StringVal();
        int nPort = pmsg->IntVal();
        int mod = pmsg->IntVal();*/

        out.SetInt( 1001 );
        out.SetString("test server");
        out.SetInt( 5009 );
        out.SetInt( 2 );

    };
    void OnResponse(CMsgTyped* pmsg)
    {
        printf("reg respon");
        pmsg->Dump();
    };
};
reg_server g_reg_server;
/////////////////////////////////


/////////////////////////
class unreg_server : public IMsgProc
{
public:
    unreg_server()
    {
        cmdId = GLOBAL_UNREGISTER_SERVER;
        recver = RECVER0;
        name = "unreg";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*
        int serverid = pmsg->IntVal();
        */

        out.SetInt( 1001 );
    };
    void OnResponse(CMsgTyped* pmsg)
    {
        printf("unreg respon");
        pmsg->Dump();
    };
};
unreg_server g_unreg_server;
/////////////////////////////////


/////////////////////////
class send_mail : public IMsgProc
{
public:
    send_mail()
    {
        cmdId = GM_SEND_MAIL;
        recver = RECVER0;
        name = "send";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*
        	const char* Sender = pmsg->StringVal();
            const char* ReceName = pmsg->StringVal();
            const char* Title = pmsg->StringVal();
            const char* MailContent = pmsg->StringVal();
            const char* MailAttach = pmsg->StringVal();
            const char* params = pmsg->StringVal();
        */

        out.SetString("deeple_send");
        out.SetString("deeple_recv");
        out.SetString("title");
        out.SetString("content");
        out.SetString("attach");
        out.SetString("params for return");
    };
    void OnResponse(CMsgTyped* pmsg)
    {
        printf("send_mail");
        pmsg->Dump();
    };
};
send_mail g_send_mail;
/////////////////////////////////


/////////////////////////
class mails : public IMsgProc
{
public:
    mails()
    {
        cmdId = GM_MAIL_LIST;
        recver = RECVER1;
        name = "mails";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*
        	const char* Sender = pmsg->StringVal();
            const char* ReceName = pmsg->StringVal();
            const char* Title = pmsg->StringVal();
            const char* MailContent = pmsg->StringVal();
            const char* MailAttach = pmsg->StringVal();
            const char* params = pmsg->StringVal();
        */

        if (sa.size() != 3) return;

        out.SetString("deeple_recv");
        out.SetInt( atoi( sa[1].c_str() ) );
        out.SetInt( atoi( sa[2].c_str() ) );
    };
    void OnResponse(CMsgTyped* pmsg)
    {
        printf("mails");
        pmsg->Dump();
    };
};
mails g_mails;
/////////////////////////////////


/////////////////////////
class regp : public IMsgProc
{
public:
    regp()
    {
        cmdId = GM_REGISTER_PLAYER;
        recver = RECVER1;
        name = "regp";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*
        	const char* Sender = pmsg->StringVal();
            const char* ReceName = pmsg->StringVal();
            const char* Title = pmsg->StringVal();
            const char* MailContent = pmsg->StringVal();
            const char* MailAttach = pmsg->StringVal();
            const char* params = pmsg->StringVal();
        */

        if (sa.size() > 1)
        {
            out.SetString(sa[1].c_str());
        }else return;
    };
    void OnResponse(CMsgTyped* pmsg)
    {

        printf("regp");
        pmsg->Dump();
    };
};
regp g_regp;
/////////////////////////////////

/////////////////////////
class unregp : public IMsgProc
{
public:
    unregp()
    {
        cmdId = GM_UNREGPLAYER;
        recver = RECVER1;
        name = "unregp";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*
        	const char* Sender = pmsg->StringVal();
            const char* ReceName = pmsg->StringVal();
            const char* Title = pmsg->StringVal();
            const char* MailContent = pmsg->StringVal();
            const char* MailAttach = pmsg->StringVal();
            const char* params = pmsg->StringVal();
        */

        if (sa.size() > 1)
        {
            out.SetString(sa[1].c_str());
        } else return;
    };
    void OnResponse(CMsgTyped* pmsg)
    {
        printf("unregp");
        pmsg->Dump();
    };
};
unregp g_unregp;
/////////////////////////////////

/////////////////////////
class delmail : public IMsgProc
{
public:
    delmail()
    {
        cmdId = GM_DELETE_MAIL;
        recver = RECVER1;
        name = "delmail";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*
        	const char* Sender = pmsg->StringVal();
            const char* ReceName = pmsg->StringVal();
            const char* Title = pmsg->StringVal();
            const char* MailContent = pmsg->StringVal();
            const char* MailAttach = pmsg->StringVal();
            const char* params = pmsg->StringVal();
        */

        out.SetString("02062656-ee62-11e2-bd04-000c29128d86");
        out.SetString("deeple_recv");
    };
    void OnResponse(CMsgTyped* pmsg)
    {
        printf("delmail");
        pmsg->Dump();
    };
};
delmail g_delmail;
/////////////////////////////////


/////////////////////////
class readmail : public IMsgProc
{
    //
public:
    readmail()
    {
        cmdId = GM_READ_MAIL;
        recver = RECVER1;
        name = "readmail";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*
        	const char* Sender = pmsg->StringVal();
            const char* ReceName = pmsg->StringVal();
            const char* Title = pmsg->StringVal();
            const char* MailContent = pmsg->StringVal();
            const char* MailAttach = pmsg->StringVal();
            const char* params = pmsg->StringVal();
        */
        out.SetString("02062656-ee62-11e2-bd04-000c29128d86");
        out.SetString("deeple_recv");
    };
    void OnResponse(CMsgTyped* pmsg)
    {
        printf("readmail");
        pmsg->Dump();
    };
};
readmail g_readmail;
/////////////////////////////////

/////////////////////////
class attach : public IMsgProc
{
public:
    attach()
    {
        cmdId = GM_GET_MAIL_ATTACHMENT;
        recver = RECVER1;
        name = "attach";

        g_MsgMgr.Reg(this);
    }
    void OnRequest(StringArray& sa, CMsgTyped& out)
    {
        /*
        	const char* Sender = pmsg->StringVal();
            const char* ReceName = pmsg->StringVal();
            const char* Title = pmsg->StringVal();
            const char* MailContent = pmsg->StringVal();
            const char* MailAttach = pmsg->StringVal();
            const char* params = pmsg->StringVal();
        */
        out.SetString("02062656-ee62-11e2-bd04-000c29128d86");
        out.SetString("deeple_recv");
    };
    void OnResponse(CMsgTyped* pmsg)
    {
        printf("attach");
        pmsg->Dump();
    };
};
attach g_attach;
/////////////////////////////////

