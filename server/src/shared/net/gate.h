//
//  gate.h
//  GameSrv
//
//  Created by prcv on 13-6-13.
//
//

#ifndef _gate_h
#define _gate_h

#include <string>
#include <list>
#include <vector>
using namespace std;

class Poller;
class IoEvent;
class Socket;
class TcpListener;
class TcpConnector;
class TcpConnection;
class IoEventReactor;


enum IoEventType
{
    IoEventAccept,
    IoEventRead,
    IoEventClose,
    IoEventConnect,
    IoEventConnectFail,
};


class IoEventReactor
{
public:
    virtual void onEvent(int sid, int type, char* data, int len) = 0;
    virtual ~IoEventReactor() {}
};

// sid 分配
// listener   1-255
// connector  257-0xFFFFFF
// connection 0x01000000-0x7FFFFFFF (listenerid << 24 | increment)
class Gate
{
public:
    Gate();
    ~Gate();
    
    int create_server(const char* host, unsigned short port, int maxcon, IoEventReactor* reactor);
    int create_connector(const char* host, unsigned short port, IoEventReactor* reactor);
    void close_connection(int sid);
    void write(int sid, char* data, int size);
    Poller* get_poller() {return _poller;}
    
    Socket* getSocket(int sid);
    
    void processEvents();
    
private:

    Poller* _poller;
    vector<TcpListener*> _listeners;
    vector<TcpConnector*> _connectors;
};

#endif
