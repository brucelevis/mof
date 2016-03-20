//
//  gate.cpp
//  GameSrv
//
//  Created by prcv on 13-6-14.
//
//

#include "gate.h"

#include <string.h>

#include "poller.h"
#include "tcp_listener.h"
#include "tcp_connector.h"
#include "tcp_connection.h"

#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

Gate::Gate()
{
    _poller = Poller::create();
    
    _listeners.push_back(NULL);
    _connectors.push_back(NULL);
}

Gate::~Gate()
{
    for (int i = 1; i < _listeners.size(); i++)
    {
        _listeners[i]->close();
    }
    for (int i = 1; i < _connectors.size(); i++)
    {
        _connectors[i]->close();
    }
    if (_poller)
    {
        delete _poller;
    }
}

int Gate::create_server(const char* host, unsigned short port, int maxcon, IoEventReactor* reactor)
{
    int listenerid = (int)_listeners.size();
    TcpListener* listener = new TcpListener;
    if (!listener->init(this, listenerid, host, port, maxcon))
    {
        return -1;
    }
    listener->set_cb(reactor);
    _listeners.push_back(listener);
    return listenerid;

}

int Gate::create_connector(const char* host, unsigned short port, IoEventReactor* reactor)
{
    int connectorid = 0x0100 + (int)_connectors.size();
    TcpConnector* connector = new TcpConnector;
    connector->init(this, connectorid, host, port);
    connector->set_cb(reactor);
    _connectors.push_back(connector);
    
    return connectorid;
}

void Gate::close_connection(int sid)
{
    int serverid = (sid >> 24);
    TcpListener* listener = _listeners[serverid];
    if (listener){
        listener->close_connection(sid);
    }
}

Socket* Gate::getSocket(int sid)
{
    Socket* sock = NULL;
    do {
        if (sid > 0xFFFF) {
            int listenerid = sid >> 24;
            if (listenerid > _listeners.size()){
                break;
            }
            
            TcpListener* listener = _listeners[listenerid];
            if (listener == NULL) {
                break;
            }
            TcpConnection* connection = listener->get_connection(sid);
            sock = static_cast<Socket*>(connection);
        } else if (sid > 0x0100) {
            int index = sid - 0x0100;
            if (index > _connectors.size()){
                break;
            }
            
            TcpConnector* connector = _connectors[index];
            sock = static_cast<Socket*>(connector);
        }
    } while (0);
    
    return sock;
}

void Gate::write(int sid, char* data, int size)
{
    Socket* sock = getSocket(sid);
    if (sock) {
        sock->write(data, size);
    }
}

void Gate::processEvents()
{
    get_poller()->execute_timers();
    get_poller()->poll();
}
