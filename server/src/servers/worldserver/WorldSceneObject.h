//
//  WorldSceneObject.h
//  GameSrv
//
//  Created by prcv on 14-4-17.
//
//

#ifndef __GameSrv__WorldSceneObject__
#define __GameSrv__WorldSceneObject__

#include <iostream>
#include "WorldObject.h"
#include "WorldBase.h"


class WorldScene;
class ResCfg;
enum MoveType
{
    eMoveTypeIdle,
    eMoveTypeDest,
    eMoveTypeDirection
};

class SceneObject : public WorldObject
{
public:
    SceneObject(WorldObjectType type);
    virtual ~SceneObject();
    
    virtual void update(uint64_t ms);
    
    bool enterScene(WorldScene* scene);
    void leaveScene();
    
    void move(Vector2& speed);
    void moveTo(float speed, CCPoint& dest);
    void moveDist(Vector2& speed, float dist);
    void stopMove();
    virtual void onStopMove();
    
    
    //overide
    virtual bool preEnterScene(WorldScene* scene);
    virtual void onEnterScene();
    virtual void onLeaveScene();
    
private:
    
    READWRITE(WorldScene*, mScene, Scene)
    READWRITE(int, mAreaId, AreaId)
    READWRITE(int, mTpltID, TpltID)
    READWRITE(bool, mCrossArea, CrossArea)
    
public:
    int getSceneID();
    
    void setPosition(const CCPoint &position);
    //
    //void setBakPosition(const CCPoint &position);
    //void doPositionAdjust();
    
    const CCPoint& getPosition();
    float getPositionX();
    float getPositionY();
    void setPositionX(float x) { mPosition.x = x; }
    void setPositionY(float y) { mPosition.y = y; }
    
    void setSpeed(const Vector2& speed);
    const Vector2& getSpeed();
    float getSpeedX();
    float getSpeedY();
    
    void updatePosition(uint64_t ms);
    
    
    const CCRect& getBoundingBox();
    void setOrient(ObjOrientation orient);
    ObjOrientation getOrient();
    bool isFaceLeft();
    CCPoint getForwardPos(float distance);
    
protected:
    
    ObjOrientation mOrientation;
    CCRect  mBoundingBox;
    ResCfg* mResCfg;
    CCPoint mPosition;
    bool mAdjustOnce;
    Vector2 mSpeed;
    Vector2 mDirection;
    
    
    MoveType mMoveType;
    CCPoint mStartPoint;
    CCPoint mDestination;
    bool reachDest();
    CheckRange mCheckX;
    CheckRange mCheckY;
    
    
    READONLY(int, mResID, ResID);
    
public:
    void reverseOrient();
};


#endif /* defined(__GameSrv__WorldSceneObject__) */
