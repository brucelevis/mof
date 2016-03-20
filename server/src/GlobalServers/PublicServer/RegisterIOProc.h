/*-------------------------------------------------------------------------
	created:	2010/09/28  14:11
	filename: 	e:\Project_SVN\Server\GlobalServer\RegisterIOProc.h
	author:		Deeple
	
	purpose:	
---------------------------------------------------------------------------*/
#include "../Interface/ShellHandle.h"

// �����ഴ����
///////////////////////////////////////////////////////////////////////////////
#define CREATE_PROC(msgid)\
class CGlobalMessageProc##msgid : public IGlobalMessageProc\
{\
	friend class CGame;\
protected:\
	virtual void OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg);\
};\

#define REGISTER_CLASS(msgid)\
	CGlobalMessageProc##msgid *proc##msgid = new CGlobalMessageProc##msgid;\
	_GAME->RegisterIOMessageProc(false, msgid, proc##msgid);\


#define IOMSG(id)\
	CGlobalMessageProc##id\


#define CREATE_SYS_PROC(msgid)\
class CGlobalSysMessageProc##msgid : public IGlobalMessageProc\
{\
	friend class CGame;\
protected:\
	virtual void OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg);\
};\

#define REGISTER_SYS_CLASS(msgid)\
	CGlobalSysMessageProc##msgid *sysproc##msgid = new CGlobalSysMessageProc##msgid;\
	_GAME->RegisterIOMessageProc(true, msgid, sysproc##msgid);\

#define SYSIOMSG(id)\
	CGlobalSysMessageProc##id\


/////////////////////////////////////////////////////////////////////////////////

// ���崦����
// ϵͳ
CREATE_SYS_PROC(GLOBAL_REGISTER_SERVER);     // ������ע��
CREATE_SYS_PROC(GLOBAL_UNREGISTER_SERVER);   // ������ע��(������ע��ʱ, 
CREATE_SYS_PROC(GLOBAL_KEEP_ALIVE);          // ��������
