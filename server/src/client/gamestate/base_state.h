#ifndef CLIENT_GAMESTATE_BASE_STATE_H_
#define CLIENT_GAMESTATE_BASE_STATE_H_

#include <vector>
#include <string>
using namespace std;

typedef vector<string> StringArray;

class BaseState
{
public:
    
    virtual void exec(const char* cmd)
    {
        
    }
    
    virtual void update(float dt)
    {
        
    }
    
    static BaseState* create(const char* state);
};


StringArray strSplit(const char* str, const char* sep);

#endif