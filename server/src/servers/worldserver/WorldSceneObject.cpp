//
//  WorldSceneObject.cpp
//  GameSrv
//
//  Created by prcv on 14-4-17.
//
//

#include "WorldSceneObject.h"
#include "WorldCreature.h"
#include "WorldScene.h"
#include "msg.h"
#include "DataCfg.h" 

SceneObject::SceneObject(WorldObjectType type) : WorldObject(type), mScene(NULL), mAreaId(0),
    mPosition(0, 0), mSpeed(0, 0), mMoveType(eMoveTypeIdle), mCheckX(NULL), mCheckY(NULL)
{
    setCrossArea(false);
}

SceneObject::~SceneObject()
{
    
}

int SceneObject::getSceneID()
{
    int sceneId = 0;
    if (mScene)
    {
        sceneId = mScene->getSceneModId();
    }
    
    return sceneId;
}

void SceneObject::setPosition(const CCPoint &position)
{
    mPosition = position;
}

/*void SceneObject::setBakPosition(const CCPoint &position)
{
    mBakPosition = position;
    mAdjustOnce = true;
}
void SceneObject::doPositionAdjust(){
    if(mAdjustOnce){
        if( mBakPosition.x != mPosition.x || mBakPosition.y != mPosition.y )
            mPosition = mBakPosition;
        mAdjustOnce = false;
    }
}*/

const CCPoint& SceneObject::getPosition()
{
    return mPosition;
}

const CCRect& SceneObject::getBoundingBox()
{
    if (mResCfg == NULL)
    {
        mBoundingBox.origin = mPosition;
        mBoundingBox.size = CCSize(0, 0);
        return mBoundingBox;
    }
    
    CCPoint origin;
    origin.x = getPositionX() - mResCfg->mBoundingBoxX * 0.5;
    origin.y = getPositionY();
    mBoundingBox.size = CCSize(mResCfg->mBoundingBoxX, mResCfg->mBoundingBoxY);
    mBoundingBox.origin = origin;
    
    return mBoundingBox;
}

void SceneObject::setOrient(ObjOrientation orient)
{
    mOrientation = orient;
}

ObjOrientation SceneObject::getOrient()
{
    return mOrientation;
}

float SceneObject::getPositionX()
{
    return mPosition.x;
}
float SceneObject::getPositionY()
{
    return mPosition.y;
}


void SceneObject::setSpeed(const Vector2& speed)
{
    mSpeed = speed;
}

const Vector2& SceneObject::getSpeed()
{
    return mSpeed;
}


float SceneObject::getSpeedX()
{
    return mSpeed.x;
}
float SceneObject::getSpeedY()
{
    return mSpeed.y;
}

bool SceneObject::reachDest()
{
    bool retX = mCheckX ? mCheckX(mPosition.x, mDestination.x) : true;
    bool retY = mCheckY ? mCheckY(mPosition.y, mDestination.y) : true;
    
    return retX && retY;
}


void SceneObject::updatePosition(uint64_t ms)
{
    if (mScene && mMoveType != eMoveTypeIdle)
    {
        float varX = mSpeed.x * (float)ms / 1000;
        float varY = mSpeed.y * (float)ms / 1000;
        
        mPosition.x += varX;
        mPosition.y += varY;

        if (mMoveType == eMoveTypeDest && reachDest())
        {
            stopMove();
        }
        if (getCrossArea()) {
            CCRect rect = mScene->getBoundingBox(mScene->getActiveArea());
            CCPoint pt = getPosition();
            if (rect.containsPoint(pt)) {
                setCrossArea(false);
                setAreaId(mScene->getActiveArea());
            }
        }
        
        if (getAreaId() == mScene->getActiveArea()) {
            CCRect aabb = mScene->getBoundingBox(getAreaId());
            check_range(mPosition.x, aabb.getMinX(), aabb.getMaxX() - 1);
            check_range(mPosition.y, aabb.getMinY(), aabb.getMaxY() - 1);
        } else if (getAreaId() < mScene->getActiveArea()) {
            CCRect sceneAABB = mScene->getBoundingBox(mScene->getActiveArea());
            CCRect aabb = mScene->getBoundingBox(getAreaId());
            check_range(mPosition.x, aabb.getMinX(), sceneAABB.getMaxX() - 1);
            check_range(mPosition.y, aabb.getMinY(), aabb.getMaxY() - 1);
            setCrossArea(true);
    
        } else {
            log_error("a");
        }
    }
}

void SceneObject::update(uint64_t ms)
{
    updatePosition(ms);
    WorldObject::update(ms);
}


bool SceneObject::enterScene(WorldScene* scene)
{
    scene->addObject(this);
    setScene(scene);
    onEnterScene();
    return true;
}

void SceneObject::leaveScene()
{
    onLeaveScene();
    mScene->removeObject(this);
    setScene(NULL);
}

void SceneObject::moveTo(float speed, CCPoint& dest)
{
    Vector2 moveVec = Vector2(dest.x - mPosition.x, dest.y - mPosition.y);
    if (moveVec.isZero()) {
        stopMove();
        return;
    }
    
    if (FLOAT_GT(moveVec.x, 0.0f)) {
        mCheckX = checkMaxf;
    } else if (FLOAT_LT(moveVec.x, 0.0f)) {
        mCheckX = checkMinf;
    } else  {
        mCheckX = NULL;
    }
    
    if (FLOAT_GT(moveVec.y, 0.0f)) {
        mCheckY = checkMaxf;
    } else if (FLOAT_LT(moveVec.y, 0.0f)) {
        mCheckY = checkMinf;
    } else {
        mCheckY = NULL;
    }
    
    mStartPoint = mPosition;
    mDestination = dest;
    mDirection = moveVec.norm();
    mSpeed = Vector2(mDirection.x * speed, mDirection.y * speed);
    mMoveType = eMoveTypeDest;
}

void SceneObject::stopMove()
{
    mSpeed = Vector2(0.0f, 0.0f);
    mMoveType = eMoveTypeIdle;
    
    onStopMove();
}

void SceneObject::onStopMove()
{
    
}

void SceneObject::move(Vector2& speed)
{
    mSpeed = speed;
    mMoveType = eMoveTypeDirection;
}


bool SceneObject::preEnterScene(WorldScene* scene)
{
    return true;
}
 
void SceneObject::onEnterScene()
{
    
}

void SceneObject::onLeaveScene()
{
    if (mScene->getValid())
    {
        notify_worldobject_leave_scene notify;
        notify.instid = getInstId();
        mScene->broadcastPacket(&notify);
    }
}

bool SceneObject::isFaceLeft()
{
    return mOrientation == eLeft || mOrientation == eLeftDown || mOrientation == eLeftUp;
}

CCPoint SceneObject::getForwardPos(float distance)
{
    if(isFaceLeft())
        return ccpAdd(getPosition(), CCPoint(-distance, 0));
    else
        return ccpAdd(getPosition(), CCPoint(distance, 0));
}


void SceneObject::reverseOrient()
{
    switch (mOrientation) {
        case eLeft:
            setOrient(eRight);
            break;
        case eLeftDown:
            setOrient(eRightDown);
            break;
        case eLeftUp:
            setOrient(eRightUp);
            break;
        case eRight:
            setOrient(eLeft);
            break;
        case eRightDown:
            setOrient(eLeftDown);
            break;
        case eRightUp:
            setOrient(eLeftUp);
            break;
        default:
            break;
    }
}
