//
//  reference.cpp
//  GameSrv
//
//  Created by prcv on 13-9-10.
//
//

#include "reference.h"

#include <assert.h>

RefCountedObject::RefCountedObject() : mCounter(1)
{
    
}

void RefCountedObject::retain()
{
    ++mCounter;
}

void RefCountedObject::release()
{
    assert(mCounter > 0);
    
    --mCounter;
    if (mCounter == 0)
    {
        delete this;
    }
}

int RefCountedObject::referenceCount() const
{
    return mCounter;
}


RefCountedObject::~RefCountedObject()
{
}
    
