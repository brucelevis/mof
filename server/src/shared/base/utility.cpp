//
//  utility.cpp
//  GameSrv
//
//  Created by prcv on 14-2-19.
//
//

#include "utility.h"
#include "uuid/uuid.h"

string util::generateUuid()
{
    uuid_t uuid;
    char str[36];
    
    uuid_generate(uuid);
    uuid_unparse(uuid, str);
    
    return str;
}

string util::bin2Hex(void* data, int len)
{
    static const char *st = "0123456789abcdef";
    string str;
    for (int i = 0; i < len; i++)
    {
        char chHex[2];
        chHex[0] = st[((unsigned char*)data)[i] >> 4];
        chHex[1] = st[((unsigned char*)data)[i] & 0xF];
        str.append(chHex, 2);
    }
    return str;
}


bool math::floatEq(float v1, float v2)
{
    float dv = v1 - v2;
    return dv < 0.000001 && dv > -0.000001;
}

bool math::floatGt(float v1, float v2)
{
    float dv = v1 - v2;
    return dv >= 0.000001;
}

bool math::floatLt(float v1, float v2)
{
    float dv = v1 - v2;
    return dv <= -0.000001;
}

float math::randf()
{
    return ((float)rand()) / RAND_MAX;
}

float math::randfBetween(float min, float max)
{
    assert(floatGt(min, max) == false);
    return min + randf() * (max - min);
}