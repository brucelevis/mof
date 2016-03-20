//
//  utility.h
//  GameSrv
//
//  Created by prcv on 14-2-19.
//
//

#ifndef __shared__utility__
#define __shared__utility__

#include <iostream>
#include <string>
#include <assert.h>
#include <stdlib.h>
using namespace std;

namespace util
{
    string generateUuid();
    string bin2Hex(void* data, int len);


    template <class T>
    inline const T &
    check_min (T &val, const T &minval)
    {
        if (val < minval)
        {
            val = minval;
        }

        return val;
    }

    template <class T>
    inline const T &
    check_max (T &val, const T &maxval)
    {
        if (val > maxval)
        {
            val = maxval;
        }

        return val;
    }

    template <class T>
    inline const T &
    check_range (T &val, const T &minval, const T &maxval)
    {
        assert(minval <= maxval);

        if (val < minval)
        {
            val = minval;
        }
        else if (val > maxval)
        {
            val = maxval;
        }

        return val;
    }

    inline int range_rand(int min, int max)
    {
        assert(min <= max);

        if (min == max)
        {
            return min;
        }

        int interval = max - min;
        return min + rand() % (interval + 1);
    }

    inline float randf()
    {
        return ((float)rand()) / RAND_MAX;
    }

    inline float range_randf(float min, float max)
    {
        return min + randf() * (max - min);
    }


};

namespace math
{
    float randf();
    float randfBetween(float min, float max);
    
    bool floatGt(float v1, float v2);
    bool floatLt(float v1, float v2);
    bool floatEq(float v1, float v2);
};

#endif /* defined(__GameSrv__utility__) */
