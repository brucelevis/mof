//
//  clientmgr.h
//  client
//
//  Created by 丁志坚 on 8/29/14.
//
//

#ifndef __client__clientmgr__
#define __client__clientmgr__

#include <iostream>
#include <map>

#include "gameclient.h"

using namespace std;

class GameClient;

class ClientMgr
{
public:
    map<int, GameClient*> mClients;
    
    
    GameClient* createClient(int sid)
    {
        GameClient* client = GameClient::create(sid);
        addClient(client);
        return client;
    }
    
    void addClient(GameClient* client)
    {
        mClients[client->mSessionId] = client;
    }
    void remClient(int sid)
    {
        map<int, GameClient*>::iterator iter = mClients.find(sid);
        if (iter != mClients.end()) {
            GameClient* client = iter->second;
            delete client;
        }
        
        mClients.erase(iter);
    }
    
    GameClient* getClient(int sid)
    {
        map<int, GameClient*>::iterator iter = mClients.find(sid);
        if (iter != mClients.end()) {
            return iter->second;
        }
        
        return NULL;
    }
    
    static bool globalInit()
    {
        sInstance = new ClientMgr;
        return true;
    }
    
    static ClientMgr* instance()
    {
        return sInstance;
    }
private:
    static ClientMgr* sInstance;
};


#endif /* defined(__client__clientmgr__) */
