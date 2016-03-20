#include "Obj.h"
#include "Scene.h"
#include "Game.h"
#include "inifile.h"
#include "Scene.h"
#include "Skill.h"
#include "Role.h"

Obj::Obj(ObjType t)
:mState(eIDLE)
{
    mType=t;
    mSkillMod=new SkillMod(this);
    mScene=NULL;
}

Obj::~Obj(){
    delete mSkillMod;
}

SkillOwner Obj::toSkillOwner(){
    SkillOwner ret;
    if (getType() == eMonster) {
        ret = eOwnerMonster;
    } else if (getType() == eRole) {
        Role* role = dynamic_cast<Role*>(this);
        if (role != NULL) {
            int job = role->getJob();
            switch (job) {
                case eWarrior:
                    ret=eOwnerWarrior;
                    break;
                case eMage:
                    ret=eOwnerMage;
                    break;
                case eAssassin:
                    ret=eOwnerAssassin;
                    break;
                default:
                    break;
            }
        }
    }
    return ret;
}

void Obj::OnHeartBeat(int nBeat)
{
    printf("test update time:   %d\n", nBeat);
}

void Obj::update(float dt)
{
    
}