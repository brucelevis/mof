#include "ModCenter.h"
#include <stdio.h>

IGlobal* g_pGame = NULL;

extern "C" {

void 
OnHeartBeat(int nBeat) {
}

}


void 
RegisterCallBack() {
	REG_PUBLIC_MESSAGE(GM_UPLOAD_ROLE_INFO);
}


void 
PUB_MSG(GM_UPLOAD_ROLE_INFO)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg) {
}	
