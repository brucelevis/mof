// GamesScene.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "main.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef _MANAGED
#pragma managed(pop)
#endif

void RegisterCallBack();

extern "C"
{
    bool Initialise(IGlobal* pInterface)
    {
        g_pGame = pInterface;

        if (g_pGame)
        {
            RegisterCallBack();
        }

        return true;
    }

}

