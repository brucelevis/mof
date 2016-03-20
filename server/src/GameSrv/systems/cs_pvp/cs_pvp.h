#pragma once

#include <time.h>

class CsPvpSystem
{
public:
	static bool canWorship();
	static bool canBattle();
	static bool isClose(time_t testTime);
};