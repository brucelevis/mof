/*-------------------------------------------------------------------------
	created:	2010/09/27  12:01
	filename:
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

#pragma once
#include "GameType.h"


class IGlobalMessageProc
{
	friend class CGame;
public:
	IGlobalMessageProc(){};
	virtual ~IGlobalMessageProc(){};
protected:
	virtual void OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg){};
};

class IGlobal
{
public:
	IGlobal(){};
	virtual ~IGlobal(){};

	virtual bool SendMessage(int serverid, int modid, CMsgTyped* pmsg) = 0;

	// ��Ϣ����
	virtual void RegisterCallBack(UI32 msgId, IGlobalMessageProc* handle) = 0;
	virtual bool SendMessage(PersistID& id, CMsgTyped* pmsg) = 0;

	// ͨ����ɫid��ģ��id���Ҵ�����. ����0 ��ʾʧ��
	virtual UI32 FindProcessor( UI32 RoleId, int mod) = 0;

	//��ѯÿ�����ӳص�ID,����,����
	virtual bool QueryPoolList( int mod, int* szIdPool, int nMaxSize = 0 ) = 0;

	// �첽������
	// ����������
	virtual bool AddProcessor( int id, const char* szProcessorName, size_t nThreadCount, const char* szADOConnStr) = 0;

	// ���Ҵ����
	virtual CAsyncProcesser* FindProcessor(int nProcessorID) = 0;

	// ���Ҵ����
	virtual CAsyncProcesser* FindProcessor(const char* szProcessorName) = 0;

	// ע�Ṥ��
	virtual bool Call(int nProcessorId, LPASYNCPROCFUNC func, CMsgTyped* pParamList) = 0;

	// ע�Ṥ��
	virtual bool Call(const char* szProcessorName, LPASYNCPROCFUNC func, CMsgTyped* pParamList) = 0;

	// ɾ�������
	virtual bool RemoveProcessor(int nProcessorID) = 0;

	// ɾ�������
	virtual bool RemoveProcessor(const char* szProcessorName) = 0;
};

extern IGlobal* g_pGame;

#define DEF_PUBLIC_MESSAGE(msgid)\
class CGlobalMessageProc##msgid : public IGlobalMessageProc\
{\
	friend class CGame;\
protected:\
	virtual void OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg);\
};\

#define REG_PUBLIC_MESSAGE(msgid)\
	CGlobalMessageProc##msgid *proc##msgid = new CGlobalMessageProc##msgid;\
	g_pGame->RegisterCallBack(msgid, proc##msgid);\


#define PUB_MSG(id) CGlobalMessageProc##id\


