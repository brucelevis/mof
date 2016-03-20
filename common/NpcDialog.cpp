//
//  NpcDialog.cpp
//  GameSrv
//
//  Created by cxy on 13-1-28.
//
//

#include "NpcDialog.h"
#include "inifile.h"
#include "Utils.h"
#include <stdlib.h>

std::map<int,NpcDialog*> NpcDialogMgr::_dialogs;

bool NpcDialogMgr::LoadFile(const char* FileName)
{
    IniFile inifile(FileName);
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
        NpcDialog* tmp = new NpcDialog;

        tmp->index = inifile.getValueT(*iter,"index",0);
        tmp->content = inifile.getValue(*iter,"content");

        _dialogs.insert(std::make_pair(tmp->index,tmp));
    }
    return false;
}

NpcDialog* NpcDialogMgr::FindDialog(int index)
{
    std::map<int,NpcDialog*>::iterator fit = _dialogs.find(index);
    if (fit != _dialogs.end()) {
        return fit->second;
    }
    return NULL;
}

void NpcDialogMgr::GetDialogData(NpcDialog *_dialog, std::string &_npcStr,std::map<int, std::string>* _roleStr)
{
    std::string _str = _dialog->content;
    //这是上帝在保佑你!+[0:任务完成]
    if(_str == "") return;
    StringTokenizer reToken(_str,"+[:;]");
    _npcStr = reToken[0];
    for (int i =2; (i+1)<reToken.count(); i=i+2) {
        int _id = atoi(reToken[i].c_str());
        std::string _role = reToken[i+1];
        (*_roleStr)[_id] = _role;
    }
}

//std::list<NpcDialog*> NpcDialogMgr::GetNpcDialogs(int npcid)
//{
//    std::list<NpcDialog*> outlist;
//    for (std::map<int,NpcDialog*>::iterator iter = _dialogs.begin(); iter != _dialogs.end(); ++iter) {
//        if( iter->second->npcId == npcid )
//        {
//            outlist.push_back(iter->second);
//        }
//    }
//    return outlist;
//}

