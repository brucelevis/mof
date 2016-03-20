//
//  RolePropVerify.h
//  GameSrv
//
//  Created by prcv on 13-12-10.
//
//

#ifndef __GameSrv__RolePropVerify__
#define __GameSrv__RolePropVerify__

#include <iostream>
#include "Defines.h"
#include "inifile.h"
#include "GameIniFile.h"
class RolePropVerify
{
    READWRITE(int, mAtkValue, AtkValue)
    READWRITE(int, mDefValue, DefValue)
    READWRITE(int, mHpValue, HpValue)
    READWRITE(int, mHitValue, HitValue)
    READWRITE(int, mDodgeValue, DodgeValue)
    READWRITE(int, mCriValue, CriValue)
    READWRITE(int, mPetBatValue, PetBatValue)
    
    READWRITE(float, mAtkScale, AtkScale)
    READWRITE(float, mDefScale, DefScale)
    READWRITE(float, mHpScale, HpScale)
    READWRITE(float, mHitScale, HitScale)
    READWRITE(float, mDodgeScale, DodgeScale)
    READWRITE(float, mCriScale, CriScale)
    READWRITE(float, mPetBatScale, PetBatScale)
    
public:
    bool init(const char* path)
    {
        mAtkValue = 10000;
        mDefValue = 10000;
        mHitValue = 10000;
        mDodgeValue = 10000;
        mCriValue = 10000;
        mHpValue = 10000;
        mPetBatValue = 10000;
        
        mAtkScale = 2.0f;
        mDefScale = 2.0f;
        mHitScale = 2.0f;
        mDodgeScale = 2.0f;
        mCriScale = 2.0f;
        mHpScale = 2.0f;
        mPetBatScale = 2.0f;
        load(path);
        
        return true;
    }
    
    
    
    bool load(const char* path)
    {
        try
        {
            GameInifile ini(path);
            
            mAtkValue = ini.getValueT("common", "atkvalue", 10000);
            mDefValue = ini.getValueT("common", "defvalue", 10000);
            mHitValue = ini.getValueT("common", "hitvalue", 10000);
            mDodgeValue = ini.getValueT("common", "dodgevalue", 20000);
            mCriValue = ini.getValueT("common", "crivalue", 10000);
            mHpValue = ini.getValueT("common", "hpvalue", 10000);
            mPetBatValue = ini.getValueT("common", "petbatvalue", 10000);
            
            mAtkScale = ini.getValueT("common", "atkrate", 2.0f);
            mDefScale = ini.getValueT("common", "defrate", 2.0f);
            mHitScale = ini.getValueT("common", "hitrate", 2.0f);
            mDodgeScale = ini.getValueT("common", "dodgerate", 2.0f);
            mCriScale = ini.getValueT("common", "crirate", 2.0f);
            mHpScale = ini.getValueT("common", "hprate", 2.0f);
            mPetBatScale = ini.getValueT("common", "petbatrate", 2.0f);
        }
        catch(...)
        {
            
        }
        
        return true;
    }
    
};

extern RolePropVerify g_RolePropVerify;

#endif /* defined(__GameSrv__RolePropVerify__) */
