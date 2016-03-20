// GamesScene.cpp : 定义 DLL 应用程序的入口点。
//

#include "main.h"

void RegisterCallBack();
bool InitModule();

extern "C"
{
    bool Initialise(IGlobal* pInterface)
    {
        g_pGame = pInterface;



        if (g_pGame)
        {
            if (!InitModule())
            {
                return false;
            }
            RegisterCallBack();
        }

        return true;
    }
}

