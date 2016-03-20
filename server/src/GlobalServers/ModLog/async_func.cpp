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

#include "async_func.h"
// 异步处理函数
//void AsyncProc_Plaayer(CMsgTyped* pmsg, CADODatabase* pDatabase)
void AsyncProc_Plaayer( CMsgTyped* pmsg,  sql::Statement* pConn )
{
	if (!pmsg)
	{
	    _Debug("Null message.\n");
		return;
	}

	if (!pConn)
	{
	    _Debug("Database connection is null.\n");
		return;
	}

	bool bIsSuccess=false;
	string tableTypeName = pmsg->StringVal();
    string sqlStr = pmsg->StringVal();
	try
	{
	    string propFuncName = "CALL Proc_outside_log_tb('";
	    propFuncName.append(tableTypeName);
	    propFuncName.append("')");
        string realTableName = "";
	    MysqlResult res;
	    res.dataSet = pConn->executeQuery(propFuncName.c_str());
	    if(res.dataSet->next())
        {
            realTableName = res.dataSet->getString("tb_name");
        }
        pConn->getMoreResults();
        if(realTableName.empty())
        {
            Log(LOG_ERROR, "can not find realtablename %s", tableTypeName.c_str());
            return;
        }

        string sqlCmd = "";
        sqlCmd.append("INSERT INTO ");
        sqlCmd.append(realTableName);

        sqlCmd.append(sqlStr);
        bIsSuccess = pConn->executeUpdate(sqlCmd);
        if (!bIsSuccess)
        {
            _Debug("executing %s fails.\n", sqlStr.c_str());
        }
	}
	catch(sql::SQLException &e)
	{
        _Debug("error: %s when executing %s.\n", e.what(), sqlStr.c_str());
	}
	catch(...)
	{
        _Debug("unknown exception when executing %s.\n", sqlStr.c_str());
	}
}

string eNum2FieldType(int type)
{
    string typeName = "";
    switch(type)
    {
    case EFT_BOOL:
        {
            typeName = "INT(10)";
            break;
        }
    case EFT_INT:
        {
            typeName = "INT(10)";
            break;
        }
    case EFT_FLOAT:
        {
            typeName = "FLOAT";
            break;
        }
    case EFT_VARCHAR:
        {
            typeName = "VARCHAR(50)";
            break;
        }
    case EFT_NVARCHAR:
        {
            typeName = "NVRCHAR(50)";
            break;
        }
    case EFT_TIME:
        {
            typeName = "DATETIME NOT NULL";
            break;
        }
    default:
        break;
    }
    return typeName;
}
void AsyncProc_CheckTables( CMsgTyped* pmsg,  sql::Statement* pConn )
{
    try
    {
        CDBTable logTable;
        string tablename = pmsg->StringVal();
        int fieldnum = pmsg->IntVal();

        logTable.SetTableName(tablename.c_str());
        logTable.SetFieldNum(fieldnum);
        for(int i = 0; i < fieldnum; i++)
        {
            logTable.SetFieldType(i, pmsg->IntVal());
            string newfieldName = pmsg->StringVal();
            logTable.SetFieldName(i, newfieldName.c_str());
        }

        std::vector<string> db_FieldNames;
        db_FieldNames.clear();
        char buf[512] = "";
        sprintf(buf, "show tables like '%s'", tablename.c_str());

        MysqlResult res;
        res.dataSet = pConn->executeQuery(buf);

        do
        {
            //表存在，检查表结构是否需要更新
            if(res.dataSet->next())
            {
                sprintf(buf, "show columns from %s", tablename.c_str());
                MysqlResult res2;
                res2.dataSet = pConn->executeQuery(buf);
                while(res2.dataSet->next())
                {
                    string name = res2.dataSet->getString("Field");
                    db_FieldNames.push_back(name);
                }

                std::vector<int> newfieldsIndex;
                std::vector<int> notDelIndex;
                newfieldsIndex.clear();
                for(int i = 0; i < fieldnum; i++)
                {
                    int index = 0;
                    for(; index < db_FieldNames.size(); index++)
                    {
                        if(strcmp(db_FieldNames[index].c_str(), logTable.GetFieldName(i).c_str()) == 0)
                        {
                            notDelIndex.push_back(index);
                            break;
                        }
                    }

                    if(index >= db_FieldNames.size())
                    {
                        newfieldsIndex.push_back(i);
                    }
                }

                string sqlDelfields = "alAsyncProc_Plaayerter table ";
                sqlDelfields.append(tablename.c_str());
                sqlDelfields.append(" ");

                for(int i = 0; i < notDelIndex.size(); i++)
                {
                    db_FieldNames[notDelIndex[i]] = "";
                }
                bool needtoDel = false;
                for(int i = 0; i < db_FieldNames.size(); i++)
                {
                    if(!db_FieldNames[i].empty())
                    {
                        if(needtoDel)
                        {
                            sqlDelfields.append(",");
                        }
                        char buf[512] = "";
                        sprintf(buf, "drop column %s", db_FieldNames[i].c_str());
                        sqlDelfields.append(buf);
                        needtoDel = true;
                    }
                }

                if(needtoDel)
                {
                    pConn->executeUpdate(sqlDelfields.c_str());
                }

                if(newfieldsIndex.size() == 0)
                {
                    break;
                }

                string sqlAddfields = "alter table ";
                sqlAddfields.append(tablename.c_str());
                sqlAddfields.append(" ");
                for(int i = 0; i < newfieldsIndex.size(); i++)
                {
                    if(i != 0)
                    {
                        sqlAddfields.append(",");
                    }

                    int type = logTable.GetFieldType(newfieldsIndex[i]);
                    string typeName = eNum2FieldType(type);
                    if(typeName.empty())
                    {
                        Log(LOG_ERROR, "alter tableType error: [%s]", tablename.c_str());
                        return;
                    }
                    char buf[512] = "";
                    sprintf(buf, "add column %s %s", logTable.GetFieldName(newfieldsIndex[i]).c_str(), typeName.c_str());
                    sqlAddfields.append(buf);
                }

                pConn->executeUpdate(sqlAddfields.c_str());
            }
            else    //否则创建该表
            {
                string sqlCreateTable = "create table ";
                sqlCreateTable.append(tablename.c_str());
                sqlCreateTable.append("(");

                for(int i = 0; i < fieldnum; i++)
                {
                    if(i)
                    {
                        sqlCreateTable.append(",");
                    }

                    int type = logTable.GetFieldType(i);
                    string typeName = eNum2FieldType(type);
                    if(typeName.empty())
                    {
                        Log(LOG_ERROR, "create tableType error: [%s]", tablename.c_str());
                        return;
                    }
                    char buf[512] = "";
                    string fielname = logTable.GetFieldName(i);
                    sprintf(buf, "%s %s", fielname.c_str(), typeName.c_str());
                    sqlCreateTable.append(buf);
                }
                sqlCreateTable.append(")");
                sqlCreateTable.append("collate=utf8_general_ci engine=MyISAM");
                pConn->executeUpdate(sqlCreateTable.c_str());
            }
        }while(false);
    }
	catch(sql::SQLException &e)
	{
        _Debug("error: %s when executing.\n", e.what());
	}
	catch(...)
	{
        _Debug("unknown exception when executing.\n");
	}
}

void AsyncProc_LoadTableStruct( CMsgTyped* pmsg,  sql::Statement* pConn )
{
    if (!pmsg)
	{
	    _Debug("Null message.\n");
		return;
	}

	if (!pConn)
	{
	    _Debug("Database connection is null.\n");
		return;
	}

    CLogTableMgr* tableMgr = (CLogTableMgr*)pmsg->GetPointer();
    if(tableMgr == NULL)
    {
        _Debug("Not Find TableMgr\n");
    }

    string sqlStr = "CALL Proc_outside_tb_list_column()";

	try
	{
        MysqlResult res;
        res.dataSet = pConn->executeQuery(sqlStr.c_str());
        vector<pair<string, int> > tableInfo;
        while(res.dataSet->next())
        {
            string tablename = res.dataSet->getString("name");
            int num = res.dataSet->getInt("num");

            if(!tablename.empty() && num)
            {
                tableInfo.push_back(make_pair<string, int>(tablename, num));
            }
        }
        pConn->getMoreResults();

        for(int i = 0; i < tableInfo.size(); i++)
        {
            string tablename = tableInfo[i].first;
            int num = tableInfo[i].second;

            CDBTable table;
            table.SetFieldNum(num);
            table.SetTableName(tablename.c_str());

            char buf[512];
            snprintf(buf, 512,"show columns from %s", tablename.c_str());
            MysqlResult res2;
            res2.dataSet = pConn->executeQuery(buf);

            int columCount = 0;
            while(res2.dataSet->next())
            {
                string colname = res2.dataSet->getString("Field");
                if(columCount >= num)
                {
                    Log(LOG_ERROR, "table colum not right");
                    return;
                }
                table.SetFieldName(columCount, colname.c_str());
                columCount++;
            }
            table.FormatInsertSQL();
            table.SetFieldNum(num);
            table.FormatInsertSQL();
            table.SetTableName(tablename.c_str());

            LogCache* logcache = new LogCache;
            logcache->table = table;
            logcache->sqlstr = table.GetInsertSQL();
            tableMgr->AddLogTable(tablename, logcache);
        }
        pConn->getMoreResults();

        LogCache* tmp = tableMgr->GetLogCache("tb_log");

        tableMgr->setRunning();
	}
	catch(sql::SQLException &e)
	{
        _Debug("error: %s when executing %s.\n", e.what(), sqlStr.c_str());
	}
	catch(...)
	{
        _Debug("unknown exception when executing %s.\n", sqlStr.c_str());
	}
}
