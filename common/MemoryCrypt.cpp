//
//  MemoryCrypt.cpp
//  mof
//
//  Created by Forest Lin on 14-5-26.
//
//

#include "MemoryCrypt.h"
#if defined CLIENT
#include <cocos2d.h>
#include "AlertDialog.h"
#include "Singleton.h"
#include "Newguidance.h"
#include "msg.h"
#include "Tcp.h"
#endif

#include <assert.h>
#include <string>
using namespace std;

bool EnableMemoryCrypt = true;
int MemoryCryptKey = 516938210;

//内存混淆，采用按位异或的方法：a = b^key; b = a^key;
int MemoryConfuse(void* value)
{
    int temp = *((int*)(value)) ^ *((int*)(&MemoryCryptKey));
    return temp;
}

//检测2个MD5值是否相等，如果不相等说明玩家修改了内存，作弊
void checkMemoryMd5(std::string newMd5, std::string storedMd5, std::string fileName, std::string functionName)
{
    if(newMd5 == storedMd5)
        return;
    else
    {
#if defined CLIENT
        //想服务器发送修改的文件和变量名
        int index = fileName.rfind("/");
        fileName = fileName.substr(index+1, fileName.size());
        CCLOG("fileName:%s, function:%s",fileName.c_str(), functionName.c_str());
        req_client_memory_cheat msg;
        msg.fileName = fileName;
        msg.functionName = functionName;
        Tcp::send(msg);
        CCAssert(false, "");
        
        //弹出作弊提示但不做任何事，让服务器去踢他。以防误杀。
        string show_msg = ML_TAG(66275);
        string tip=ML_TAG(31051);
        AlertDialog::createOneButtonAlert(tip, show_msg,true);
#endif
    }
}

string intToString(int val, int radix, char* buf)
{
    assert(radix <= 16 && radix >= 2);
    
    static const char* symbol = "0123456789ABCDEF";
    char sbuf[128];
    
    char* pbuf = buf ? buf : sbuf;
    
    char* startpos = pbuf;
    if (val < 0)
    {
        *startpos++ = '-';
        val = -val;
    }
    
    char* pos = startpos;
    do
    {
        *pos++ = symbol[val % radix];
        val /= radix;
    }
    while (val);
    
    *pos-- = 0;
    
    while (startpos < pos)
    {
        char temp = *startpos;
        *startpos++ = *pos;
        *pos-- = temp;
    }
    
    return string(pbuf);
}
