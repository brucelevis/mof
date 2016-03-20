//
//  basedef.h
//  GameSrv
//
//  Created by prcv on 14-1-1.
//
//

#ifndef shared_basedef_h
#define shared_basedef_h

#define CheckCondition(cond, statement) if (!(cond)) {statement;}
#define foreach(iter, container) for (iter = (container).begin(); iter != (container).end(); ++iter)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);               \
    void operator=(const TypeName&)

typedef bool (*TraverseCallback)(void* traverseobj, void* param);
class Traverser
{
public:
    virtual bool callback(void* traverseobj) {return true;}
};

#endif
