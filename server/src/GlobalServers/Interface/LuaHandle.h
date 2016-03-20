/*-------------------------------------------------------------------------
	created:	2010/08/23  14:11
	filename: 	e:\Project_SVN\Server\ServerDemo\GameObject\GameType.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

#include "Utils.h"

class ILuaState
{
public:
    /*
    ** basic stack manipulation
    */
    virtual int   GetTop() = 0;
    virtual void  SetTop(int idx) = 0;
    virtual void  PushValue(int idx) = 0;
    virtual void  Remove(int idx) = 0;
    virtual void  Insert(int idx) = 0;
    virtual void  Replace(int idx) = 0;
    virtual int   CheckStack(int sz) = 0;

    /*
    ** access functions (stack -> C)
    */

    virtual int            IsNumber(int idx) = 0;
    virtual int            IsString(int idx) = 0;
    virtual int            IsCFunction(int idx) = 0;
    virtual int            IsUserdata(int idx) = 0;
    virtual int            Type(int idx) = 0;
    virtual const char*    TypeName(int tp) = 0;

    virtual int            Equal(int idx1, int idx2) = 0;
    virtual int            RawEqual(int idx1, int idx2) = 0;
    virtual int            LessThan(int idx1, int idx2) = 0;

    virtual lua_Number      ToNumber(int idx) = 0;
    virtual lua_Integer     ToInteger(int idx) = 0;
    virtual int             ToBoolean(int idx) = 0;
    virtual const char*     ToLstring(int idx, size_t *len) = 0;
    virtual size_t          ObjLen(int idx) = 0;
    virtual lua_CFunction   ToCfunction(int idx) = 0;
    virtual void	        ToUserdata(int idx) = 0;
    virtual const void*     ToPointer(int idx) = 0;

    /*
    ** push functions (C -> stack)
    */
    virtual void  PushNil() = 0;
    virtual void  PushNumber(lua_Number n) = 0;
    virtual void  PushInteger(lua_Integer n) = 0;
    virtual void  PushLstring(const char *s, size_t l) = 0;
    virtual void  PushString(const char *s) = 0;
    virtual const char *PushVfstring(const char *fmt, va_list argp) = 0;
    virtual const char *PushFstring(const char *fmt, ...) = 0;
    virtual void  PushCClosure(lua_CFunction fn, int n) = 0;
    virtual void  PushBoolean(int b) = 0;
    virtual void  PushLightUserData(void *p) = 0;

    /*
    ** get functions (Lua -> stack)
    */
    virtual void  GetTable(int idx) = 0;
    virtual void  GetField(int idx, const char *k) = 0;
    virtual void  RawGet(int idx) = 0;
    virtual void  RawGetI(int idx, int n) = 0;
    virtual void  CreateTable(int narr, int nrec) = 0;
    virtual void* NewUserData(size_t sz) = 0;
    virtual int   GetMetaTable(int objindex) = 0;
    virtual void  GetFenv(int idx) = 0;

    /*
    ** set functions (stack -> Lua)
    */
    virtual void  SetTable(int idx) = 0;
    virtual void  SetField(int idx, const char *k) = 0;
    virtual void  RawSet(int idx) = 0;
    virtual void  RawSetI(int idx, int n) = 0;
    virtual int   SetMetaTable(int objindex) = 0;
    virtual int   SetFenv(int idx) = 0;

    /*
    ** ===============================================================
    ** some useful macros
    ** ===============================================================
    */

    virtual void Pop(int index) = 0;
    virtual void NewTable() = 0;
    virtual size_t StrLen(int index) = 0;

    virtual bool IsFunction(int index) = 0;
    virtual bool IsTable(int index) = 0;
    virtual bool IsLightUserData(int index) = 0;
    virtual bool IsNil(int index) = 0;
    virtual bool IsBoolean(int index) = 0;
    virtual bool IsThread(int index) = 0;
    virtual bool IsNone(int index) = 0;
    virtual bool IsNoneOrNil(int index) = 0;

    virtual void SetGlobal(const char* name) = 0;
    virtual void GetGlobal(const char* name) = 0;

    virtual const char* ToString(int index) = 0;

};

// 自定义Lua 回调
typedef int (*LuaFun) (ILuaState *L);
