#ifndef __CCGEMETRY_H__
#define __CCGEMETRY_H__

/**
 * @addtogroup data_structures
 * @{
 */


class CCPoint
{
public:
    float x;
    float y;

public:
    CCPoint();
    CCPoint(float x, float y);
    CCPoint(const CCPoint& other);
    CCPoint& operator= (const CCPoint& other);
    void setPoint(float x, float y);
    bool equals(const CCPoint& target) const;
};

class CCSize
{
public:
    float width;
    float height;

public:
    CCSize();
    CCSize(float width, float height);
    CCSize(const CCSize& other);
    CCSize& operator= (const CCSize& other);
    void setSize(float width, float height);
    bool equals(const CCSize& target) const;
};

class CCRect
{
public:
    CCPoint origin;
    CCSize  size;

public:
    CCRect();    
    CCRect(float x, float y, float width, float height);
    CCRect(const CCRect& other);
    //CCRect(const Rect& other);
    CCRect& operator= (const CCRect& other); 
    void setRect(float x, float y, float width, float height);
    float getMinX() const; /// return the leftmost x-value of current rect
    float getMidX() const; /// return the midpoint x-value of current rect
    float getMaxX() const; /// return the rightmost x-value of current rect
    float getMinY() const; /// return the bottommost y-value of current rect
    float getMidY() const; /// return the midpoint y-value of current rect
    float getMaxY() const; /// return the topmost y-value of current rect
    bool equals(const CCRect& rect) const;   
    bool containsPoint(const CCPoint& point) const;
    bool intersectsRect(const CCRect& rect) const;
};


#define CCPointMake(x, y) CCPoint((float)(x), (float)(y))
#define CCSizeMake(width, height) CCSize((float)(width), (float)(height))
#define CCRectMake(x, y, width, height) CCRect((float)(x), (float)(y), (float)(width), (float)(height))


const CCPoint CCPointZero = CCPointMake(0,0);

/* The "zero" size -- equivalent to CCSizeMake(0, 0). */ 
const CCSize CCSizeZero = CCSizeMake(0,0);

/* The "zero" rectangle -- equivalent to CCRectMake(0, 0, 0, 0). */ 
const CCRect CCRectZero = CCRectMake(0,0,0,0);

// end of data_structure group
/// @}

#define ccp(__X__,__Y__) CCPointMake((float)(__X__), (float)(__Y__))
static inline CCPoint
ccpAdd(const CCPoint& v1, const CCPoint& v2)
{
    return ccp(v1.x + v2.x, v1.y + v2.y);
}

static inline CCPoint
ccpSub(const CCPoint& v1, const CCPoint& v2)
{
    return ccp(v1.x - v2.x, v1.y - v2.y);
}

static inline float
ccpDot(const CCPoint& v1, const CCPoint& v2)
{
    return v1.x*v2.x + v1.y*v2.y;
}

static inline float
ccpLengthSQ(const CCPoint& v)
{
    return ccpDot(v, v);
}

static inline float
ccpDistanceSQ(const CCPoint p1, const CCPoint p2)
{
    return ccpLengthSQ(ccpSub(p1, p2));
}

static inline CCPoint
ccpMult(const CCPoint& v, const float s)
{
    return ccp(v.x*s, v.y*s);
}

#include <math.h>

static inline float
ccpLength(const CCPoint& v)
{
    return sqrtf(ccpLengthSQ(v));
}

static inline float
ccpDistance(const CCPoint& v1, const CCPoint& v2)
{
        return ccpLength(ccpSub(v1, v2));
}

static inline CCPoint
ccpNormalize(const CCPoint& v)
{
    return ccpMult(v, 1.0f/ccpLength(v));
}

inline CCPoint
ccpLerp(const CCPoint& a, const CCPoint& b, float alpha)
{
    return ccpAdd(ccpMult(a, 1.f - alpha), ccpMult(b, alpha));
}

#endif // __CCGEMETRY_H__

