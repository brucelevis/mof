
#pragma once

#include "Utils.h"


void SyncProc();
void SyncSendFile(PersistID conn, const char* filepath);
void SyncSysInfo(PersistID conn);
