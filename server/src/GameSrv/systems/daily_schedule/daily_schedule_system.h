#ifndef DailySchedule_System_H
#define DailySchedule_System_H

#include "daily_schedule.h"
#include <map>
#include <vector>
#include "RedisHash.h"
using namespace std;

class Role;

class DailyScheduleSystem : public DailyScheduleData
{
public:
    static DailyScheduleSystem* create(Role* owner);
    
    void checkRefresh();
    void refresh(time_t datatime);
    
    bool init(Role* owner);
    void onEvent(int type, int target, int progress);
    
private:
    Role* mOwner;
    
    int mWeekDay;
};

#endif