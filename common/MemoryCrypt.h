//
// MemoryCrypt.h
//  mof
//
//  Created by Forest Lin on 14-5-26.
//
//  // 内存加密。目前仅支持int，float，bool --linshusen


#ifndef mof_MemoryCrypt_h
#define mof_MemoryCrypt_h

#include "md5.h"

#pragma mark ----------------内存加密混淆的基本变量和函数----------------------------------------------------------

std::string intToString(int val, int radix = 10, char* buf = NULL);

extern bool EnableMemoryCrypt;
extern int MemoryCryptKey;   //内存加密的密钥，游戏开始运行时随机生成

//内存混淆，采用按位异或的方法：a = b^key; b = a^key;
int MemoryConfuse(void* value);

//检测2个MD5值是否相等，如果不相等说明玩家修改了内存，作弊
void checkMemoryMd5(std::string newMd5, std::string storedMd5, std::string fileName, std::string functionName);


#pragma mark ----------------int和float的内存加密----------------------------------------------------------
//内存加密的set函数（先把内存混淆，把混淆后的int值做MD5，记录下来）
//ps:做MD5运算太消耗效率，直接把int转成string即可。---linshusen 2014-11-30
#define setCrypt(varType, varName, varNameMd5, var)         \
if(EnableMemoryCrypt)                                       \
{                                                           \
    int cryptValue = MemoryConfuse(&var);                   \
    varType result = *(varType*)(&cryptValue);              \
    varName = result;                                       \
    varNameMd5 = intToString(cryptValue);              \
}                                                           \
else                                                        \
{                                                           \
    varName = var;                                          \
}

//内存加密的get函数（先验证混淆后的int值是否被修改，然后再还原内存的真实值）
#define getCrypt(varType, varName, varNameMd5)              \
if(EnableMemoryCrypt)                                       \
{                                                           \
    varType tempVarName = varName;                          \
    int cryptValue = *(int*)(&tempVarName);                 \
    std::string valueMd5 = intToString(cryptValue);    \
    checkMemoryMd5(valueMd5, varNameMd5,__FILE__,__FUNCTION__);\
                                                            \
    int realValue = MemoryConfuse(&tempVarName);            \
    varType result = *(varType*)(&realValue);               \
    return result;                                          \
}                                                           \
else                                                        \
{                                                           \
    return varName;                                         \
}


//可读写的内存加密成员变量（目前只支持int,float）
#define ReadWrite_Crypt(varType, varName, funName)              \
private: varType varName;                                       \
private: std::string varName##_md5;                             \
public: void set##funName(varType var)                          \
{                                                               \
    setCrypt(varType, varName, varName##_md5,var);              \
}                                                               \
public: varType get##funName() const                            \
{                                                               \
    getCrypt(varType, varName, varName##_md5);                  \
}

//只读的内存加密成员变量（目前只支持int,float）
#define ReadOnly_Crypt(varType, varName, funName)               \
private: varType varName;                                       \
private: std::string varName##_md5;                             \
private: void set##funName(varType var)                         \
{                                                               \
    setCrypt(varType, varName, varName##_md5,var);              \
}                                                               \
public: varType get##funName() const                            \
{                                                               \
    getCrypt(varType, varName, varName##_md5);                  \
}

//虚函数 可读写的内存加密成员变量（目前只支持int,float）
#define Virtual_ReadWrite_Crypt(varType, varName, funName)      \
private: varType varName;                                       \
private: std::string varName##_md5;                             \
public: virtual void set##funName(varType var)                  \
{                                                               \
    setCrypt(varType, varName, varName##_md5,var);              \
}                                                               \
public: virtual varType get##funName()                          \
{                                                               \
    getCrypt(varType, varName, varName##_md5);                  \
}

//静态可读写的内存加密成员变量(varName和varName##_md5,需要使用者自己在cpp中再定义一次)
#define Static_ReadWrite_Crypt(varType, varName, funName)       \
private: static varType varName;                                \
private: static std::string varName##_md5;                      \
public: static void set##funName(varType var)                   \
{                                                               \
    setCrypt(varType, varName, varName##_md5,var);              \
}                                                               \
public: static varType get##funName()                           \
{                                                               \
    getCrypt(varType, varName, varName##_md5);                  \
}

//静态只读的内存加密成员变量(varName和varName##_md5,需要使用者自己在cpp中再定义一次)
#define Static_ReadOnly_Crypt(varType, varName, funName)        \
private: static varType varName;                                \
private: static std::string varName##_md5;                      \
private: static void set##funName(varType var)                  \
{                                                               \
    setCrypt(varType, varName, varName##_md5,var);              \
}                                                               \
public: static varType get##funName()                           \
{                                                               \
    getCrypt(varType, varName, varName##_md5);                  \
}

#pragma mark ----------------bool的内存加密----------------------------------------------------------
//bool值的加密，首先把bool转换为int，然后照抄int的加密即可
//bool加密的set函数
#define setCryptBool(varName, varNameMd5, var)              \
if(EnableMemoryCrypt)                                       \
{                                                           \
    int boolToIntValue = (int)var;                          \
    int cryptValue = MemoryConfuse(&boolToIntValue);        \
    varName = cryptValue;                                   \
    varNameMd5 = md5(intToString(cryptValue));              \
}                                                           \
else                                                        \
{                                                           \
    varName = (int)var;                                     \
}

//bool加密的get函数
#define getCryptBool(varName, varNameMd5)                   \
if(EnableMemoryCrypt)                                       \
{                                                           \
    int cryptValue = varName;                               \
    std::string valueMd5 = md5(intToString(cryptValue));    \
    checkMemoryMd5(valueMd5, varNameMd5,__FILE__,__FUNCTION__);\
\
    int realValue = MemoryConfuse(&cryptValue);             \
    return (bool)realValue;                                 \
}                                                           \
else                                                        \
{                                                           \
    return (bool)varName;                                   \
}

//可读写的内存加密bool成员变量
#define ReadWriteBool_Crypt(varType, varName, funName)          \
private: int varName;                                           \
private: std::string varName##_md5;                             \
public: void set##funName(bool var)                             \
{                                                               \
    setCryptBool(varName, varName##_md5, var);                  \
}                                                               \
public: bool get##funName()                                     \
{                                                               \
    getCryptBool(varName, varName##_md5);                       \
}

//可读写的内存加密bool成员变量
#define Static_ReadWriteBool_Crypt(varType, varName, funName)   \
private: static int varName;                                    \
private: static std::string varName##_md5;                      \
public: static void set##funName(bool var)                      \
{                                                               \
    setCryptBool(varName, varName##_md5, var);                  \
}                                                               \
public: static bool get##funName()                              \
{                                                               \
    getCryptBool(varName, varName##_md5);                       \
}


#endif
