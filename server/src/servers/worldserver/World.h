//
//  World.h
//  GameSrv
//
//  Created by prcv on 13-9-17.
//
//

#ifndef GameSrv_world_h
#define GameSrv_world_h


#include <map>

#include "log.h"
#include "Defines.h"

#include "thread.h"
#include "MQ.h"
#include "timestamp.h"
#include "datetime.h"
#include "basedef.h"
#include "os.h"
#include "cmdmsg.h"

#include "WorldNPC.h"
#include "WorldSceneMgr.h"
#include "WorldRole.h"
#include "WorldPet.h"
#include "WorldRobot.h"
#include "WorldObject.h"
#include "WorldMonster.h"
#include "WorldSkillFly.h"
#include "WorldSkillAction.h"
#include "WorldSkill.h"
#include "WorldScript.h"
using namespace std;

class FrameRunnable : public Runnable
{
public:
    FrameRunnable(int fps)
    {
        mRunning = false;
        mUpdateIntervals = NULL;
        setFps(fps);
    }
    
    virtual ~FrameRunnable()
    {
        if (mUpdateIntervals)
        {
            delete mUpdateIntervals;
        }
    }
    
    void setFps(int fps)
    {
        mNextFrame = 0;
        mFps = fps;
        if (mUpdateIntervals)
        {
            delete[] mUpdateIntervals;
        }
        
        mUpdateIntervals = new int[mFps];
        
        int remain = 1000 % mFps;
        int interval = 1000 / mFps;
        
        int i = 0;
        while (i < remain)
        {
            mUpdateIntervals[i++] = interval + 1;
        }
        
        while (i < mFps)
        {
            mUpdateIntervals[i++] = interval;
        }
    }
    
    void run()
    {
        uint64_t lastUpdateTime = Timestamp::now_ms();
        mUpdateTime = 0;
        mRunning = true;
        mStartTime = lastUpdateTime;
        
        int lastFps = 60;
        int runFrames = 0;
        while (mRunning)
        {
            mCurTime = Timestamp::now_ms();
            
            //补帧或者sleep
            if (mUpdateTime > mCurTime)
            {
                int64_t sleepTime = mUpdateTime - mCurTime;
                if (sleepTime > mUpdateIntervals[0]) {
                    sleepTime = mUpdateIntervals[0];
                }
                
                Thread::sleep(sleepTime);
                mCurTime = Timestamp::now_ms();
            }
            else
            {
                int64_t deviation = mCurTime - mUpdateTime;
                if (deviation > mUpdateIntervals[mNextFrame])
                {
                    int remainMs = mCurTime % 1000;
                    mNextFrame = 0;
                    while (remainMs > mUpdateIntervals[mNextFrame])
                    {
                        remainMs -= mUpdateIntervals[mNextFrame++];
                    }
                    
                    mUpdateTime += deviation - remainMs;
                    
                    runFrames = mNextFrame;
                    
                    log_info("CUR TIME " << mCurTime << " UPD TIME " << mUpdateTime);
                }
            }
            
            //上次循环到这次循环间隔
            uint64_t dt = 0;
            if (mCurTime > lastUpdateTime) {
                dt = mCurTime - lastUpdateTime;
            }
            lastUpdateTime = mCurTime;
            //如果循环间隔时间过长，抛出警告。警告线为2倍帧时间
            if (dt > mUpdateIntervals[0] * 2)
            {
                log_warn("frame interval exception " << dt);
            }
            
            update(dt);
            
            
            runFrames++;
            mUpdateTime += mUpdateIntervals[mNextFrame++];
            mNextFrame %= mFps;
            if (mNextFrame == 0)
            {
                if (runFrames != mFps) log_info("Fps is " << runFrames);
                lastFps = runFrames;
                runFrames = 0;
            }
        }

    }
    
    uint64_t getCurrentMs()
    {
        return mCurTime;
    }
    
    int getFps()
    {
        return mFps;
    }
    
    bool start()
    {
        if (init())
        {
            mThread.start(this);
            return true;
        }
        
        return false;
    }
    
    void stop()
    {
        if (mRunning)
        {
            mRunning = false;
            mThread.join();
        }
    }

    virtual bool init() {return true;}
    virtual bool beforeRun() {return true;}
    virtual void update(uint64_t ms) = 0;
    virtual void afterRun() {}
        
protected:
    Thread mThread;
    
    int  mFps;
    int* mUpdateIntervals;
    int  mNextFrame;
    
    
    bool mRunning;
    uint64_t mUpdateTime;
    uint64_t mCurTime;
    
    uint64_t mStartTime;
};

class World : public FrameRunnable
{
public:
    World() : FrameRunnable(30)
    {
    }
    
    bool init()
    {
        FrameRunnable::init();
        
        mBeat = 0;
        
        g_WorldObjectMgr.init(eWorldObjectRole, 10000);
        g_WorldObjectMgr.init(eWorldObjectPet, 10000);
        g_WorldObjectMgr.init(eWorldObjectNPC, 10000);
        g_WorldObjectMgr.init(eWorldObjectScene, 10000);
        g_WorldObjectMgr.init(eWorldObjectRobot, 10000);
        g_WorldObjectMgr.init(eWorldObjectMonster, 1000000);
        g_WorldObjectMgr.init(eWorldObjectSkillFly, 1000000);
        g_WorldObjectMgr.init(eWorldObjectSkillAction, 1000000);
        g_WorldObjectMgr.init(eWorldObjectRetinue, 10000);
        
        WorldScript::globalInit();
        
        SkillEffectMgr::InitEffectTable();
    
        return true;
    }
    
    void update(uint64_t ms)
    {
        sCurTime = mCurTime;
        
        skynet_message msg;
        while (skynet_mq_pop(sMQ, &msg) == 0)
        {
            int msgType = (msg.sz >> 24);
            int msgDataLen = msg.sz & 0xFFFFFF;
            if (msgType == NET_MSG)
            {
                do {
                    ByteArray byteArray((char*)msg.data, msgDataLen);
                    int connId = byteArray.read_int();
                    int type = byteArray.read_int();
                    int id = byteArray.read_int();
                    
                    NetPacketHander fun = hander_recvpacketfun(type, id);
                    if (fun == NULL) {
                        break;
                    }
                    
                    INetPacket* packet = create_recvpacket(type, id);
                    if (packet == NULL) {
                        break;
                    }
                    
                    try {
                        packet->decode(byteArray);
                    } catch (...) {
                        delete packet;
                        break;
                    }
                    fun(packet, msg.session);
                    delete packet;
                    
                } while (0);
                
                delete (char*)msg.data;
            }
            else if (msgType == CMD_MSG)
            {
                ICmdMsg *cmdmsg = (ICmdMsg*)msg.data;
                CheckCondition(cmdmsg, continue)
                cmdmsg->handle();
                delete cmdmsg;
            }
        }
        
        g_WorldSceneMgr.update(ms);
        
        if (mBeat * 1000 < mCurTime - mStartTime)
        {
            mBeat += 10;
#if 1
            for (int i = 1; i < eWorldObjectTypeMax; i++) {
                int num = g_WorldObjectMgr.getObjectNum(i);
                if (num > 0) {
                    //log_info("type " << i << "'s object num is " << num);
                }
            }
#endif
        }
    }
    
    bool beforeRun()
    {
        FrameRunnable::beforeRun();
        
        log_info("world before run");
        return true;
    }
    
    void afterRun()
    {
        log_info("world after run");
        FrameRunnable::afterRun();
    }
    
    
    static message_queue* sMQ;
    static uint64_t       sCurTime;
    
private:
    
    int mFps;
    int mBeat;
};

#endif
