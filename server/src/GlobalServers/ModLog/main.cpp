// GamesScene.cpp : ���� DLL Ӧ�ó������ڵ㡣
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

