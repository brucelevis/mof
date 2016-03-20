#include "base_state.h"

#include "auth.h"

StringArray strSplit(const char* str, const char* sep)
{
    bool table[256];
    memset(table, 0, sizeof(table));
    for (const char* p = sep; *p; p++)
    {
        table[(unsigned char)*p] = 1;
    }
    
    StringArray sa;
    const char* pos = str;
    const char* head = str;
    while (*pos) {
        if (table[(unsigned char)*pos])
        {
            if (head != pos)
            {
                string section(head, pos);
                sa.push_back(section);
            }
            
            head = pos + 1;
        }
        
        pos++;
    }
    
    if (head != pos)
    {
        string section(head, pos);
        sa.push_back(section);
    }
    
    return sa;
}


BaseState* BaseState::create(const char* state)
{
    if (strcmp(state, "auth") == 0)
    {
        AuthState* state = new AuthState;
        return state;
    }
    return NULL;
}