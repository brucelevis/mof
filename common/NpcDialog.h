//
//  NpcDialog.h
//  GameSrv
//
//  Created by cxy on 13-1-28.
//
//

#ifndef __GameSrv__NpcDialog__
#define __GameSrv__NpcDialog__

#include <iostream>
#include <map>
#include <list>
#include <string.h>
#include <vector>

class NpcDialog {
public:
    int index; //index
    std::string content; //对话内容
};

class NpcDialogMgr {
public:
    static bool LoadFile(const char* FileName);
    static NpcDialog* FindDialog(int index);
    //static std::list<NpcDialog*> GetNpcDialogs(int npcid);
    //获取某一条dialog里面的数据，
    static void GetDialogData(NpcDialog* _dialog,std::string& _npcStr,std::map<int, std::string>* _roleStr = NULL);
private:
    static std::map<int,NpcDialog*> _dialogs;
};

#endif /* defined(__GameSrv__NpcDialog__) */
