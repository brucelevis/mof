//
//  flopcard.cpp
//  GameSrv
//
//  Created by cxy on 13-3-15.
//
//

#include "flopcard.h"
#include <list>
#include "inifile.h"
#include <stdlib.h>
#include "Utils.h"

#include "ItemHelper.h"

#if defined(CLIENT)
    typedef IniFile GameInifile;
#else
    #include "GameIniFile.h"
#endif

INSTANTIATE_SINGLETON(flopcardMgr);


std::vector<std::string> flopcard::getAwards(int index, const std::vector<int>& drops)
{
   std::vector<std::string> awards;
    if (index <= 0 || index > rewards.size())
    {
        return awards;
    }

    awards = StrSpilt(rewards[index - 1], ";");

    for (int i = 0; i < drops.size(); i++)
    {
        awards.push_back(dropitems[drops[i]-1]);
    }

    return awards;
}

std::string flopcardMgr::RandomCard(int id, int& outindex, vector<int>& drops, std::string& dropStr, float ratio)
{
    std::string rewards = "";
    dropStr = "";
    
    flopcard* flop = Find(id);
    if (flop) {
        int pros = 0;

        for (int i = 0; i < flop->pros.size() && i < flop->rewards.size(); i++) {
            pros += flop->pros[i];
        }

        if (pros) {
            int randnum = rand()%pros;
            pros = 0;
            for (int i = 0; i < flop->pros.size() && i < flop->rewards.size(); i++) {
                pros += flop->pros[i];
                if (randnum < pros) {
                    outindex = i + 1;
                    rewards = flop->rewards[i];
                    break;
                }
            }
        }

        for (int i = 0; i < flop->dropnum; i++)
        {
            float randnum = range_randf(0.0, 1.0);
            if (randnum < flop->dropprops[i] + (flop->dropprops[i] * ratio))
            {
                drops.push_back(i + 1);
                rewards += ";";
                rewards += flop->dropitems[i];
                
                dropStr.append(flop->dropitems[i].c_str());
                dropStr.append(";");
            }
        }

    }

    return rewards;
}


flopcard* flopcardMgr::Find(int id){
    std::map<int,flopcard*>::iterator fit = _flopcards.find(id);
    if (fit != _flopcards.end()) {
        return fit->second;
    }
    return NULL;
}

static vector<string> getItemIds(const char* award)
{
    vector<string> allIds;
    vector<string> awardVec = StrSpilt(award, ";");
    for (int i = 0; i < awardVec.size(); i++) {
        vector<string> awardDesc = StrSpilt(awardVec[i], " ");
        if (awardDesc.size() != 2 || awardDesc[0] != "item") {
            continue;
        }
        
        int pos = awardDesc[1].find('*');
        if (pos == string::npos) {
            continue;
        }
        
        string awardId = awardDesc[1].substr(0, pos);
        allIds.push_back(awardId);
    }
    
    return allIds;
}

bool flopcardMgr::LoadFile(const char* FileName) {
    GameInifile inifile(FileName);
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
        
        
        flopcard* tmp = new flopcard;
        
        set<string>& allIds = tmp->allItemIds;
        
        tmp->id = inifile.getValueT(*iter,"id",0);
        tmp->name = inifile.getValue(*iter,"name");
        tmp->cardnum = inifile.getValueT(*iter, "cardnum", 0);

        for (int i = 0; i < tmp->cardnum; i++) {
            char cardbuf[32];
            char cardpropbuf[32];
            sprintf(cardpropbuf, "pro%d", i + 1);
            sprintf(cardbuf, "rewards%d", i + 1);

            float pro = inifile.getValueT(*iter, cardpropbuf, 0.0);
            std::string rewards = inifile.getValue(*iter, cardbuf, "");

            tmp->pros.push_back(pro);
            tmp->rewards.push_back(rewards);
            
            vector<string> cardItems = getItemIds(rewards.c_str());
            for (int i = 0; i < cardItems.size(); i++) {
                allIds.insert(cardItems[i]);
            }
        }
//        tmp->rewards1 = inifile.getValue(*iter,"rewards1");
//        tmp->pro1 = inifile.getValueT(*iter,"pro1",0);
//        tmp->rewards2 = inifile.getValue(*iter,"rewards2");
//        tmp->pro2 = inifile.getValueT(*iter,"pro2",0);
//        tmp->rewards3 = inifile.getValue(*iter,"rewards3");
//        tmp->pro3 = inifile.getValueT(*iter,"pro3",0);
//        tmp->rewards4 = inifile.getValue(*iter,"rewards4");
//        tmp->pro4 = inifile.getValueT(*iter,"pro4",0);

        tmp->dropnum = inifile.getValueT(*iter, "dropnum", 0);
        for (int i = 0; i < tmp->dropnum; i++)
        {
            char dropbuf[32];
            char droppropbuf[32];
            sprintf(dropbuf, "drop%d", i + 1);
            sprintf(droppropbuf, "dropprop%d", i + 1);

            float dropprop = inifile.getValueT(*iter, droppropbuf, 0.0);
            std::string drop = inifile.getValue(*iter, dropbuf, "");

            tmp->dropitems.push_back(drop);
            tmp->dropprops.push_back(dropprop);
            
            vector<string> dropItems = getItemIds(drop.c_str());
            for (int i = 0; i < dropItems.size(); i++) {
                allIds.insert(dropItems[i]);
            }
        }
        
        _flopcards[tmp->id] = tmp;
    }
    return true;
}
