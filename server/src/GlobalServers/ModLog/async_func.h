/********************************************************************
	created:	2010/11/15
	created:	15:11:2010   15:12
	filename: 	e:\SVN\��������\Source\ModLogin\async_func.cpp
	file path:	e:\SVN\��������\Source\ModLogin
	file base:	async_func
	file ext:	cpp
	author:

	purpose:
*********************************************************************/

#include "ModLog.h"

void AsyncProc_Plaayer(CMsgTyped* pmsg, sql::Statement* pConn);
void AsyncProc_CheckTables( CMsgTyped* pmsg,  sql::Statement* pConn );

void AsyncProc_LoadTableStruct( CMsgTyped* pmsg,  sql::Statement* pConn );
