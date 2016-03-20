#pragma once

#include "redis_helper.h"

void registerRedisContext(redisContext* context);
void unregisterRedisContext();
redisContext* getRedisContext();
redisContext* createRedisContext();

#define redisCmd(...) (redisReply*)redisCommand(getRedisContext(),__VA_ARGS__)
#define doRedisCmd(...) freeReplyObject(redisCmd(__VA_ARGS__))