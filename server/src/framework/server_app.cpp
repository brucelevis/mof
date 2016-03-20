#include "server_app.h"
#include "server.h"

void* ServerApp::ServerContext::threadProc(void* param)
{
    ServerContext* context = (ServerContext*)param;
    context->mServer->beforeRun();
    for (;;) {
        if (!context->mRunning) {
            break;
        }
        
        // 0 for success
        skynet_message msg;
        int ret = skynet_mq_pop(context->mMQ, &msg);
        if (ret != 0) {
            //wait();
            usleep(1000);
            continue;
        }
        
        context->mServer->onMessage(&msg);
    }
    context->mServer->afterRun();
    
    return NULL;
}


ServerApp::ServerContext* ServerApp::createServerContext(IServer* server)
{
    ServerContext* context = new ServerContext;
    server->setId(mServers.size());
    context->mServer = server;
    context->mMQ = skynet_mq_create(server->getId());
    context->mThread = new Thread();
    return context;
}

void ServerApp::destroyServerContext(ServerContext* ctx)
{
    delete ctx->mThread;
    skynet_mq_release(ctx->mMQ);
}

int ServerApp::registerServer(IServer* server)
{
    ServerContext* context = createServerContext(server);
    mServers.push_back(context);
    
    return server->getId();
}

void ServerApp::unRegisterServer(IServer *server)
{
    int serverId = server->getId();
    if (serverId >= mServers.size() || serverId < 0) {
        return;
    }
    
    ServerContext* ctx = mServers[serverId];
    
    message_queue* mq = ctx->mMQ;
    Thread* thread = ctx->mThread;
    skynet_mq_release(mq);
    delete thread;
    delete ctx;
}

void ServerApp::runBackground()
{
    for (vector<ServerContext*>::iterator iter = mServers.begin();
         iter !=mServers.end(); iter++) {
        ServerContext* context = (*iter);
        context->mRunning = true;
        context->mThread->start(ServerContext::threadProc, context);
    }
}

void ServerApp::waitTerm()
{
    for (vector<ServerContext*>::iterator iter = mServers.begin();
         iter !=mServers.end(); iter++) {
        ServerContext* context = (*iter);
        context->mRunning = false;
    }
    
    for (vector<ServerContext*>::iterator iter = mServers.begin();
         iter !=mServers.end(); iter++) {
        ServerContext* context = (*iter);
        context->mThread->join();
    }
}

void ServerApp::send(int serverId, skynet_message* message)
{
    if (serverId >= mServers.size() || serverId < 0) {
        return;
    }
    
    ServerContext* ctx = mServers[serverId];
    if (ctx) {
        skynet_mq_push(ctx->mMQ, message);
    }
}

message_queue* ServerApp::getMQ(int serverId)
{
    if (serverId >= mServers.size() || serverId < 0) {
        return NULL;
    }
    
    return mServers[serverId]->mMQ;
}