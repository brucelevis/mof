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
#pragma  once
#include <algorithm>
#include <functional>
#include "async_func.h"
#include "MailListCache.h"
#include <mysql_connection.h>
#include "SysMailCache.h"

using namespace sql;
using namespace sql::mysql;


#define GE_MAX_MAIL_COUNT 200

//异步函数调用的发送邮件
bool SendMail(sql::Statement* pConn, const char* owner, const char* Title, const char* Sender, const char* ReceName,
			  int& nErrorCode, const char* MailAttach,const char* MailContent, string& guid_out);

//异步函数调用的获取邮件列表
bool MailList(sql::Statement* pConn, const char* PlayerName, int& nErrorCode, vector<MailListItem*>& maillist);

//异步函数调用的阅读邮件
bool ReadMail(sql::Statement* pConn, const char* PlayerName,
              const char* mail_guid, string& content, string& attach, int& nErrorCode);

//异步函数调用的删除邮件
bool DeleteMail(sql::Statement* pConn, const char* PlayerName,const char* MailId, int& nErrorCode);

//异步函数调用的获取邮件附件
bool GetAttachment(sql::Statement* pConn, const char* mail_guid, string& attach, const char* receiver, int& nErrorCode);

//异步函数调用发送群邮件
bool SendMailAll(sql::Statement* pConn,  const char* Title, int& nErrorCode, const char* MailAttach,const char* MailContent);

// 发送邮件
void AsyncProc_send_mail(CMsgTyped* pmsg, sql::Statement* pConn)
{
	/*
	 * 可能需要判断当前玩家的邮件数量。即使是30天后会自动删除。？
	 */

    if (!pConn || !pmsg) return;

	try
	{
		PersistID connId;
		int receiver = pmsg->IntVal();
		int msgId = pmsg->IntVal();
		connId.nIndex = pmsg->IntVal();
		connId.nIdent = pmsg->IntVal();

		const char* owner = pmsg->StringVal(); //pmsg->WideStrVal();
		const char* Title = pmsg->StringVal();
		const char* Sender = pmsg->StringVal(); //pmsg->WideStrVal();
		const char* ReceName = pmsg->StringVal(); //pmsg->WideStrVal();
		const char* MailAttach = pmsg->StringVal();
		const char* MailContent = pmsg->StringVal();
		const char* params = pmsg->StringVal();
		int send_ret = pmsg->IntVal();

		int nErrorCode = 0;
		string guid_out;

	// 邮件入数据库
        SendMail(pConn,owner, Title, Sender, ReceName, nErrorCode, MailAttach, MailContent, guid_out);

        // 发送回调
        time_t now = time(NULL);
        string send_time = Time_tToString( now );	//邮件的发送时间
        if (nErrorCode == GE_OK)
        {
            // 然后添加到缓存
            MailListItem item;
            item.mailid = guid_out;		//邮件的ID  Guid
            item.title = Title;		//邮件的标题
            item.sender = Sender;		//邮件的发送者 //wstring
            item.receiver = ReceName;	//邮件的接受者 //wstring
            item.isReaded = 0;		//邮件是否已读
            item.send_time = send_time;	//邮件的发送时间
            item.tm_send_time = now;
            item.hasAttach = ( strlen( MailAttach ) > 0 ? 1 : 0 );		//邮件是否有附件

            g_MailCache.AddToCache(owner, &item);
        }

        if (send_ret == 1)
        {
            CMsgTyped ret;
            ret.SetInt(0);
            ret.SetInt(receiver);
            ret.SetInt(msgId);
            ret.SetInt(nErrorCode);
            ret.SetString(guid_out.c_str());
            ret.SetString(MailAttach);
            ret.SetString(Sender);
            ret.SetString(ReceName);
            ret.SetString(Title);
            ret.SetString(send_time.c_str());
            ret.SetString(params);

            g_pGame->SendMessage(connId, &ret);
        }
	}
	catch (CExceptMsg e)
	{

	}
}

bool SendMail(sql::Statement* pConn, const char* owner, const char* Title, const char* Sender, const char* ReceName,
			  int& nErrorCode, const char* MailAttach,const char* MailContent, string& guid_out)
{
	/*
	 * 这个判断在上层判断
	 * //如果不是系统邮件，需要判断，发送邮件最大数。
	 */

	if (!pConn)
	{
		nErrorCode = GE_CANNOT_ACCESS_DB;
		return false;
	}

	nErrorCode = GE_OK;

	try
	{
		/*objADOStoredProc.AddInputParameter("@Owner", owner);
		objADOStoredProc.AddInputParameter("@Title", Utf8ToWideStr(Title).c_str());
		objADOStoredProc.AddInputParameter("@Sender", Sender);
		objADOStoredProc.AddInputParameter("@ReceiverName", ReceName);
		objADOStoredProc.AddInputParameter("@MailAttachment", (LPSTR)MailAttach);
		objADOStoredProc.AddInputParameter("@MailContent", Utf8ToWideStr(MailContent).c_str());
		objADOStoredProc.AddReturnValue();*/

        MysqlResult res;
        MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
        string sOwner = con->escapeString(owner);
        string sTitle = con->escapeString(Title);
        string sSender = con->escapeString(Sender);
        string sReceName = con->escapeString(ReceName);
        string sMailAttach = con->escapeString(MailAttach);
        string sMailContent = con->escapeString(MailContent);

		string sql = "CALL Proc_SendMail('";
		sql.append(sOwner);
		sql.append("', '");
		sql.append(sTitle);
		sql.append("', '");
		sql.append(sSender);
		sql.append("', '");
		sql.append(sReceName);
		sql.append("', '");
		sql.append(sMailAttach);
		sql.append("', '");
		sql.append(sMailContent);
		sql.append("')");

        res.dataSet = pConn->executeQuery(sql);
        if (res.dataSet && res.dataSet->next())
        {
            guid_out = res.dataSet->getString("__MailId");
        }

        pConn->getMoreResults();

        return true;

	}
    catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

		nErrorCode = GE_CANNOT_ACCESS_DB;
	}

	return false;
}

bool sort_mail_time(MailListItem* m1, MailListItem* m2)
{
    return m1->tm_send_time > m2->tm_send_time;
}

// 获取邮件列表异步处理函数
void AsyncProc_mail_list(CMsgTyped* pmsg, sql::Statement* pConn)
{
    if (!pConn || !pmsg) return;

	try
	{
		PersistID connId;;
		int nErrorCode = 0;
		int IsRead = 1;

		CMsgTyped ret;
		ret.SetInt(0);
		ret.SetInt(pmsg->IntVal()); //receive
		ret.SetInt(pmsg->IntVal()); //id
		connId.nIndex = pmsg->IntVal();
		connId.nIdent = pmsg->IntVal();

		const char* PlayerName = pmsg->StringVal(); //pmsg->WideStrVal();
		int from = pmsg->IntVal();
		int count = pmsg->IntVal();

        // 读取邮件列表
		vector<MailListItem*> mails;

        // 读取普通邮件， 尝试从缓存中读取
        if ( !g_MailCache.FindCache(PlayerName, mails) )
        {
	    // 缓存中无法获取，从数据表格中读取
            if ( MailList(pConn,PlayerName,nErrorCode, mails) && !nErrorCode)
            {
                // 将数据库中的数据，创建新的缓存
                g_MailCache.CreateCache( PlayerName, mails );
            }
        }

        // 读取系统邮件
        if (g_sysMails.SysMailList(PlayerName, mails))
        {
            // 排序
            sort(mails.begin(), mails.end(), sort_mail_time);
        }

        // 用来记录邮件编号
        int readCnt = 0;
        int mailIndex = 0;

		std::vector<MailListItem*>::iterator ItMail = mails.begin();
		ret.SetInt( nErrorCode );
		ret.SetInt( (int)mails.size() );
		ret.SetInt( from );
		ret.SetInt( 0 );

		while( ItMail != mails.end() )
		{
		    mailIndex ++;

		    if (mailIndex >= from && readCnt < count)
            {
                ret.SetString((*ItMail)->mailid.c_str());
                ret.SetString((*ItMail)->title.c_str());
                ret.SetString( (*ItMail)->sender.c_str() ); //SetWideStr
                ret.SetString( (*ItMail)->receiver.c_str()); //SetWideStr
                ret.SetInt((*ItMail)->isReaded);
                ret.SetString((*ItMail)->send_time.c_str());
                ret.SetInt((*ItMail)->hasAttach); //加入判断邮件附件有无

                readCnt ++;
            }

			delete *ItMail;

			ItMail++;
		}

        ret.SetAt(6, readCnt);
		g_pGame->SendMessage( connId, &ret);
	}
	catch (CExceptMsg e)
	{

	}
}

/*
// 获取系统邮件
bool SysMailList(sql::Statement* pConn, const char* PlayerName, vector<MailListItem*>& maillist)
{
    MysqlResult res;

    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string sOwner = con->escapeString(PlayerName);
	try
	{
		char sql[1024];

		//多加Attachment的判断有无，在SendTime后面
		sprintf(sql, "select * from \
( \
select S.MailID, S.Title, S.SendTime, S.Attachment, T.HasAttachment AS HasAttachment,  T.IsDelete AS IsDelete, T.MailID As existId from \
 SysMailTable AS S \
left join \
(select * from SysMailStat where Owner='%s') AS T \
on T.MailID = S.MailID or T.MailID IS NULL \
) AS RESULT \
where IsDelete = 0 or IsDelete is null", sOwner.c_str() ); // WideStrAsUtf8(PlayerName).c_str()

        res.dataSet = pConn->executeQuery(sql);
        if (res.dataSet)
        {
            while (res.dataSet->next())
            {
                //  MailID Title SendTime Attachment MailContent HasAttachment IsDelete
				MailListItem& mi = *(new MailListItem);

				mi.mailid = res.dataSet->getString("MailID");
				mi.mailid = "sys_" + mi.mailid;
				mi.title = res.dataSet->getString("Title");

				mi.sender = "系统"; //Utf8AsWideStr("系统");
				mi.receiver = PlayerName;

				string existId = res.dataSet->getString("existId");
				mi.isReaded = existId.size() ? 1 : 0;
                mi.send_time = res.dataSet->getString("SendTime");
                string Attachment = res.dataSet->getString("Attachment");

                int hasAttachment = res.dataSet->isNull("HasAttachment") ? 1 : res.dataSet->getInt("HasAttachment");
                mi.hasAttach = hasAttachment && (Attachment.size() ? 1 : 0);

                mi.tm_send_time = sscanftime(mi.send_time.c_str());

				maillist.push_back(&mi);
            }
        }
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }

    return true;
}
*/

bool MailList(sql::Statement* pConn, const char* PlayerName,int& nErrorCode, vector<MailListItem*>& maillist)
{
	if (!pConn)
	{
		nErrorCode = GE_CANNOT_ACCESS_DB;
		return false;
	}

    // 读取普通邮件
	MysqlResult res;

    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string sOwner = con->escapeString(PlayerName);

	try
	{
		char sql[1024];

		//多加Attachment的判断有无，在SendTime后面
		sprintf(sql, "select MailID,Title,Sender,Receiver,SendTime,Attachment,IsReaded from MailTable where Owner = '%s' order by SendTime desc limit 200",
				sOwner.c_str()); // WideStrAsUtf8(PlayerName).c_str()

        res.dataSet = pConn->executeQuery(sql);
		if (res.dataSet)
		{
			while (res.dataSet->next())
			{
				MailListItem &mi = *(new MailListItem);

				mi.mailid = res.dataSet->getString("MailID");
				mi.title = res.dataSet->getString("Title");

				mi.sender = res.dataSet->getString("Sender").c_str(); //Utf8AsWideStr(res.dataSet->getString("Sender").c_str());
				mi.receiver = res.dataSet->getString("Receiver").c_str(); //Utf8AsWideStr(res.dataSet->getString("Receiver").c_str())
				mi.isReaded = res.dataSet->getInt("IsReaded");
                mi.send_time = res.dataSet->getString("SendTime");
                string Attachment = res.dataSet->getString("Attachment");
                mi.hasAttach = Attachment.size() ? 1 : 0;

                mi.tm_send_time = sscanftime(mi.send_time.c_str());

				maillist.push_back(&mi);
			}

			return true;
		}
	}
	catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
	}

	return false;
}

// 查看邮件异步处理函数
void AsyncProc_read_mail(CMsgTyped* pmsg, sql::Statement* pConn)
{
    if (!pConn || !pmsg) return;

	try
	{
		PersistID connId;

		CMsgTyped ret;
		ret.SetInt(0);
		ret.SetInt(pmsg->IntVal()); //receive
		ret.SetInt(pmsg->IntVal()); //id
		connId.nIndex = pmsg->IntVal();
		connId.nIdent = pmsg->IntVal();

		const char* mail_guid = pmsg->StringVal();
		const char* PlayerName = pmsg->StringVal(); //pmsg->WideStrVal();
		const char* mail_title = pmsg->StringVal();

		string mail_content;
		string attach;
		int nErrorCode = GE_OK;

        // 读取邮件
		if (SysMailMgr::IsSysMail(mail_guid))
        {
            g_sysMails.SysReadMail(pConn, PlayerName, mail_guid, mail_content, attach);
        }
        else
        {
            ReadMail(pConn, PlayerName, mail_guid, mail_content, attach, nErrorCode);
            if (nErrorCode == 0)
            {
                // 设置邮件已读
                g_MailCache.SetReaded(PlayerName, mail_guid);
            }
        }

		ret.SetInt(nErrorCode);
		ret.SetString(mail_guid);
		ret.SetString(mail_content.c_str());
		ret.SetString(attach.c_str());
		ret.SetString(mail_title);

		g_pGame->SendMessage(connId, &ret);

	}
	catch (CExceptMsg e)
	{

	}
}

/*
bool ReadSysMail(sql::Statement* pConn, const char* PlayerName,
                 const char* mail_guid, string& content, string& attach, int& nErrorCode)
{
    const char* mail_guid_fix = mail_guid + 4;

    char sql[1024];
    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string sOwner = con->escapeString(PlayerName);

    sprintf(sql, "select * from \
( \
select S.MailID, S.Title, S.SendTime, S.MailContent, S.Attachment, T.IsDelete AS IsDelete, T.HasAttachment as HasAttachment, T.MailID As existId from \
 SysMailTable AS S \
left join \
(select * from SysMailStat where Owner= '%s') AS T \
on T.MailID = S.MailID or T.MailID IS NULL \
) AS RESULT \
where (IsDelete = 0 or IsDelete is null) AND MailID = '%s'", sOwner.c_str(),  mail_guid_fix); //WideStrAsUtf8(PlayerName)

    MysqlResult res;

	try
	{
        res.dataSet = pConn->executeQuery(sql);
        if (res.dataSet && res.dataSet->next())
        {
            content = res.dataSet->getString("MailContent");
            attach = res.dataSet->getString("Attachment");

            string existid = res.dataSet->getString("existId");

            if (existid.size() && res.dataSet->getInt("HasAttachment") == 0)
            {
                attach = "";
            }

            // 插入记录
            if (existid.size() == 0)
            {
                existid = res.dataSet->getString("MailID");
                string sendtm = res.dataSet->getString("SendTime");
                // insert into SysMailStat values('%s', '%s', '%s', 1, 0);
                sprintf(sql, "insert into SysMailStat values('%s', '%s', '%s', 1, 0)",
                        existid.c_str(), sOwner.c_str(), sendtm.c_str()); //WideStrAsUtf8(PlayerName).c_str()

                pConn->executeUpdate(sql);
            }

            return true;
        }
	}
    catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

        content = "";
        attach = "";
	}

    return false;
}
*/

bool ReadMail(sql::Statement* pConn, const char* PlayerName,
              const char* mail_guid, string& content, string& attach, int& nErrorCode)
{
	if (!pConn )
	{
		nErrorCode = GE_CANNOT_ACCESS_DB;
		return false;
	}

	//nErrorCode = GE_ERROR_ACCOUNT_PASS;

    MysqlResult res;

	try
	{
        char sql[1024];
        sprintf(sql, "CALL Proc_ReadMail('%s','','','0')", mail_guid);

        res.dataSet = pConn->executeQuery(sql);

        if (res.dataSet && res.dataSet->next())
        {
            nErrorCode = res.dataSet->getInt("__result");

            if (nErrorCode == GE_OK)
            {
                content = res.dataSet->getString("__content");
                attach = res.dataSet->getString("__attach");

                pConn->getMoreResults();
                return true;
            }
        }

        pConn->getMoreResults();
	}
    catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

        nErrorCode = GE_CANNOT_ACCESS_DB;
	}

    return false;
}

// 删除邮件异步处理函数
void AsyncProc_delete_mail(CMsgTyped* pmsg, sql::Statement* pConn)
{
    if (!pConn || !pmsg) return;

	try
	{
		PersistID connId;

		CMsgTyped ret;
		ret.SetInt(0);
		ret.SetInt(pmsg->IntVal()); //receive
		ret.SetInt(pmsg->IntVal()); //id
		connId.nIndex = pmsg->IntVal();
		connId.nIdent = pmsg->IntVal();

		const char* MailId = pmsg->StringVal();
		const char* PlayerName = pmsg->StringVal(); //pmsg->WideStrVal()
		int nErrorCode = GE_OK;

        // 删除系统邮件
        if (SysMailMgr::IsSysMail(MailId))
        {
            g_sysMails.SysDeleteMail(pConn, PlayerName, MailId);
        }
        else
        {
            if (DeleteMail(pConn, PlayerName,  MailId, nErrorCode))
            {
                g_MailCache.RemoveFromCache( PlayerName,  MailId );
            }
        }

        ret.SetInt( nErrorCode );
		ret.SetString(MailId);

		g_pGame->SendMessage(connId, &ret);

	}
	catch (CExceptMsg e)
	{

	}
}
/*
bool DeleteSysMail(sql::Statement* pConn, const char* PlayerName,const char* MailId, int& nErrorCode )
{
    const char* mail_guid_fix = MailId + 4;

    // 执行update insert
    // update SysMailState set IsDelete= 1 where MailID=MailID AND Owner=PlayerName;
    // Insert into SysMailState Values('MailID','PlayerName', )

    char sql[1024];
    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string sOwner = con->escapeString(PlayerName);
    sprintf(sql, "update SysMailStat set IsDelete = 1 where MailID='%s' AND Owner='%s'", mail_guid_fix,
    		sOwner.c_str() ); //WideStrAsUtf8(PlayerName).c_str()

    MysqlResult res;

	try
	{
		int n = pConn->executeUpdate(sql);
		if (n == 0)
		{
		    // 插入记录
		    sprintf(sql, "select  SendTime from SysMailTable where MailID='%s'", mail_guid_fix);

            string SendTime;
            res.dataSet = pConn->executeQuery(sql);
            if (res.dataSet && res.dataSet->next())
            {
                SendTime = res.dataSet->getString("SendTime");
            }

            sprintf(sql, "Insert into SysMailStat Values('%s','%s', '%s', 0, 1)", mail_guid_fix,
            		sOwner.c_str() , SendTime.c_str()); //WideStrAsUtf8(PlayerName).c_str()

            pConn->executeUpdate(sql);
		}

		return true;
	}
	catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

        return false;
	}

    return false;
}
*/
bool DeleteMail(sql::Statement* pConn, const char* PlayerName, const char* MailId, int& nErrorCode )
{
    nErrorCode = GE_CANNOT_ACCESS_DB;

	if (!pConn)
	{
		return false;
	}

	try
	{
        MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
        string sOwner = con->escapeString(PlayerName);
		char sql[1024];
		sprintf(sql, "delete from MailTable where Owner = '%s' AND MailID = '%s'",sOwner.c_str(), MailId); // WideStrAsUtf8(PlayerName).c_str()

		pConn->executeUpdate(sql);

		nErrorCode = GE_OK;
	}
	catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

        return false;
	}

	return true;
}

// 获取邮件附件异步处理函数
void AsyncProc_mail_attachment(CMsgTyped* pmsg, sql::Statement* pConn)
{
    if (!pConn ||!pmsg) return;

	try
	{
		PersistID connId;

		CMsgTyped ret;
		ret.SetInt(0);
		ret.SetInt(pmsg->IntVal()); //receive
		ret.SetInt(pmsg->IntVal()); //id
		connId.nIndex = pmsg->IntVal();
		connId.nIdent = pmsg->IntVal();

		const char* mail_guid = pmsg->StringVal();
		const char* receiver = pmsg->StringVal(); //pmsg->WideStrVal();
		const char* mail_title = pmsg->StringVal();

		string attach;
		int nErrorCode = GE_OK;

        // 读取系统邮件
		if ( SysMailMgr::IsSysMail(mail_guid) )
        {
            g_sysMails.SysGetAttach(pConn, receiver, mail_guid, attach);
        }
        else
        {
            GetAttachment(pConn, mail_guid, attach, receiver, nErrorCode);
            if (nErrorCode == 0)
            {
                // 设置邮件已读
                g_MailCache.SetAttachNone(receiver, mail_guid);
            }
        }

		ret.SetInt(nErrorCode);
		ret.SetString(mail_guid);
		ret.SetString(attach.c_str());
		ret.SetString(mail_title);

		g_pGame->SendMessage(connId, &ret);
	}
	catch (CExceptMsg e)
	{

	}
}
/*
bool SysMailAttach(sql::Statement* pConn, const char* mail_guid, string& attach, const char* receiver, int& nErrorCode)
{
    const char* mail_guid_fix = mail_guid + 4;

    char sql[1024];
    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string sOwner = con->escapeString(receiver);

    sprintf(sql, "select * from \
( \
select S.MailID, S.Attachment, S.SendTime, T.IsDelete AS IsDelete, T.HasAttachment as HasAttachment, T.MailID As existId from \
 SysMailTable AS S \
left join \
(select * from SysMailStat where Owner='%s') AS T \
on T.MailID = S.MailID or T.MailID IS NULL \
) AS RESULT \
where (IsDelete = 0 or IsDelete is null) AND MailID = '%s'", sOwner.c_str(),  mail_guid_fix); //WideStrAsUtf8(receiver).c_str()

    MysqlResult res;

	try
	{
        res.dataSet = pConn->executeQuery(sql);
        if (res.dataSet && res.dataSet->next())
        {
            attach = res.dataSet->getString("Attachment");

            string existid = res.dataSet->getString("existId");

            if (existid.size() && res.dataSet->getInt("HasAttachment") == 0)
            {
                attach = "";
            }

            // 插入记录
            if (existid.size() == 0)
            {
                existid = res.dataSet->getString("MailID");
                string sendtm = res.dataSet->getString("SendTime");

                sprintf(sql, "insert into SysMailStat values('%s', '%s', '%s', 0, 0)",
                        existid.c_str(),sOwner.c_str() , sendtm.c_str()); //WideStrAsUtf8(receiver).c_str()

                pConn->executeUpdate(sql);
            }
            else
            {
                 sprintf(sql, "update SysMailStat set HasAttachment = 0 where Owner='%s' AND MailID='%s'"
                        , sOwner.c_str(), existid.c_str()); //WideStrAsUtf8(receiver).c_str()

                 pConn->executeUpdate(sql);
            }

            return true;
        }
	}
    catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
        attach = "";
	}

    return false;
}
*/

bool GetAttachment(sql::Statement* pConn, const char* mail_guid, string& attach, const char* receiver, int& nErrorCode)
{
	if (!pConn)
	{
		nErrorCode = GE_CANNOT_ACCESS_DB;
		return false;
	}

	nErrorCode = GE_ERROR_ACCOUNT_PASS;

    MysqlResult res;;
    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string sOwner = con->escapeString(receiver);

	try
	{
	    char sql[1024];
	    sprintf(sql, "CALL Proc_MailAttachment('%s','','%s','0')", mail_guid, sOwner.c_str()); //WideStrAsUtf8(receiver).c_str()

	    res.dataSet = pConn->executeQuery(sql);

	    if (res.dataSet && res.dataSet->next())
	    {
	        nErrorCode = res.dataSet->getInt("__result");

	        if (nErrorCode == GE_OK)
	        {
	            attach = res.dataSet->getString("__attach");

	            pConn->getMoreResults();
	            return true;
	        }
	    }

	    pConn->getMoreResults();
	}
    catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

		nErrorCode = GE_CANNOT_ACCESS_DB;
	}

	return false;
}
