#include "main.h"

extern void RegisterCallBack();

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

