//
//  WorldBase.h
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#ifndef __GameSrv__WorldBase__
#define __GameSrv__WorldBase__

#include <iostream>
#include "utility.h"
#include "WorldGeometry.h"

inline bool FLOAT_EQ(float v1, float v2)
{
    float dv = v1 - v2;
    return dv < 0.000001 && dv > -0.000001;
}

inline bool FLOAT_GT(float v1, float v2)
{
    float dv = v1 - v2;
    return dv >= 0.000001;
}
inline bool FLOAT_LT(float v1, float v2)
{
    float dv = v1 - v2;
    return dv <= -0.000001;
}

typedef bool (*CheckRange)(float& value, float limit);

inline bool checkMinf(float& value, float min)
{
    if (FLOAT_GT(value, min))
    {
        return false;
    }
    
    value = min;
    return true;
}

inline bool checkMaxf(float& value, float max)
{
    if (FLOAT_LT(value, max))
    {
        return false;
    }
    
    value = max;
    return true;
}



struct Vector2
{
    float x;
    float y;
    
public:
    Vector2()
    {
        
    }
    
    
    Vector2(float fx, float fy)
    {
        x = fx;
        y = fy;
    }
    
    Vector2& operator=(const Vector2& other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }
    
    
    Vector2(const Vector2& other)
    {
        x = other.x;
        y = other.y;
    }
    
    Vector2& operator+=(const Vector2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    
    Vector2 operator+(const Vector2& v1)
    {
        Vector2 ret = *(this);
        ret += v1;
        return ret;
    }
    
    void min(Vector2& v)
    {
        util::check_min(x, v.x);
        util::check_min(y, v.y);
    }
    
    void max(Vector2& v)
    {
        util::check_max(x, v.x);
        util::check_max(y, v.y);
    }
    
    void range(Vector2 minv, Vector2 maxv)
    {
        min(minv);
        max(maxv);
    }
    
    Vector2 norm()
    {
        float len = length();
        
        return Vector2(x / len, y / len);
    }
    
    float length()
    {
        float value = x * x + y * y;
        return sqrt(value);
    }
    
    const Vector2& adjust(float len)
    {
        float rate = len / length();
        
        return multi(rate);
    }
    
    const Vector2& multi(float rate)
    {
        x *= rate;
        y *= rate;
        return *this;
    }
    
    bool isZero()
    {
        return FLOAT_EQ(0.0f, x) && FLOAT_EQ(0.0f, y);
    }
};

float pointDistance(const CCPoint& p1, const CCPoint& p2);

#endif /* defined(__GameSrv__WorldBase__) */
