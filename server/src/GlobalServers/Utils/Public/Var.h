
#ifndef _VAR_H
#define _VAR_H

#include "Macros.h"

#include <stddef.h>
#include <string.h>

// PERSISTID
// 对象ID数据类型（不可初始化）

class IBaseObj;

#pragma pack(push, 1)

struct PERSISTID
{
	union
	{
		struct
		{
			UI32	nIdent;
			UI32	nSerial;
		};

		// 64位优化
		unsigned __int64 nDummy64;
	};

public:
	bool IsNull() const
	{
		return (0 == nDummy64);
	}

protected:
	PERSISTID() {}
};

#pragma pack(pop)

inline bool operator==(const PERSISTID & source, const PERSISTID & other)
{
	return (source.nDummy64 == other.nDummy64);
}

inline bool operator!=(const PERSISTID & source, const PERSISTID & other)
{
	return (source.nDummy64 != other.nDummy64);
}

// PERSISTID_INIT
// 可初始化的对象ID

struct PERSISTID_INIT: public PERSISTID
{
	PERSISTID_INIT()
	{
		nDummy64 = 0;
	}

	PERSISTID_INIT(UI32 ident, UI32 serial)
	{
		nIdent = ident;
		nSerial = serial;
	}

	PERSISTID_INIT & operator=(const PERSISTID & src)
	{
		nDummy64 = src.nDummy64;
		return *this;
	}
};

// CVar
// 可变数据类型

enum VAR_TYPE
{
	VARTYPE_UNKNOWN,
	VARTYPE_BOOL,
	VARTYPE_INT,
	VARTYPE_FLOAT,
	VARTYPE_STRING,
	VARTYPE_WIDESTR,
	VARTYPE_OBJECT,
	VARTYPE_VOID,
	VARTYPE_INT64,
	VARTYPE_MAX,
};

class CVar
{
    friend class CPropList;
public:
	CVar();
	CVar(const CVar & source);
	explicit CVar(int type);
	CVar(int type, bool value);
	CVar(int type, int value);
	CVar(int type, float value);
	CVar(int type, const char * value);
	CVar(int type, const wchar_t * value);
	~CVar();

	int Type() const;
	int Tag() const;
	const char* Name();
	void SetType(int type);
	void SetTag(int tag);
	void SetName(const char* name);

	bool BoolVal() const;
	int IntVal() const;
	float FloatVal() const;
	const char * StringVal() const;
	const wchar_t * WideStrVal() const;

	void SetBool(bool value);
	void SetInt(int value);
	void SetFloat(float value);
	void SetString(const char * value);
	void SetWideStr(const wchar_t * value);

	CVar & operator=(const CVar & source);

private:
    inline bool IsString() const
    {
        return (m_nType == VARTYPE_STRING || m_nType == VARTYPE_WIDESTR);
    }

private:
	union
	{
		struct
		{
			UI16	m_nType;
			UI16	m_nTag;
		};
		UI32		m_nDummy;
	};

	union
	{
		float		m_fValue;
		int			m_nValue;
		bool		m_bValue;
		char *	m_sValue;
	};

	// ±äÁ¿Ãû
	const char* m_szVarName;
};

inline CVar::CVar()
{
	m_nDummy = 0;
	m_sValue = NULL;
	m_szVarName = NULL;
}

inline CVar::~CVar()
{
    if ( IsString() )
    {
        if (m_sValue ) delete [] m_sValue;
    }
}

inline int CVar::Type() const
{
	return m_nType;
}

inline int CVar::Tag() const
{
	return m_nTag;
}

inline const char* CVar::Name()
{
	return (const char*)m_szVarName;
}

inline void CVar::SetTag(int tag)
{
	m_nTag = tag;
}

inline void CVar::SetName(const char* name)
{
    m_szVarName = name;
}

inline bool CVar::BoolVal() const
{
	if ( VARTYPE_BOOL != m_nType ) return false;

	return m_bValue;
}

inline int CVar::IntVal() const
{
	if ( VARTYPE_INT != m_nType ) return 0;

	return m_nValue;
}

inline float CVar::FloatVal() const
{
	if ( VARTYPE_FLOAT != m_nType ) return 0.0f;

	return m_fValue;
}

inline const char * CVar::StringVal() const
{
	if ( VARTYPE_STRING != m_nType ) return "";

	if (m_sValue != NULL)
	{
		return m_sValue;
	}
	else
	{
		return "";
	}
}

inline const wchar_t * CVar::WideStrVal() const
{
	if ( VARTYPE_WIDESTR != m_nType ) return L"";

	if (m_sValue != NULL)
	{
		return (const wchar_t *)m_sValue;
	}
	else
	{
		return L"";
	}
}

inline void CVar::SetBool(bool value)
{
	if ( VARTYPE_BOOL != m_nType ) return;

	m_bValue = value;
}

inline void CVar::SetInt(int value)
{
	if ( VARTYPE_INT != m_nType ) return;

	m_nValue = value;
}

inline void CVar::SetFloat(float value)
{
	if ( VARTYPE_FLOAT != m_nType ) return;

	m_fValue = value;
}

inline CVar & CVar::operator=(const CVar & source)
{
    // 清理内存
    SetType( source.m_nType );

    //
    m_nDummy = source.m_nDummy;

    // 设置名字
    m_szVarName = source.m_szVarName;

    //  拷贝数据
    m_sValue = source.m_sValue;

    if (m_nType == VARTYPE_STRING)
    {
        m_sValue = NULL;
        SetString( source.StringVal() );
    }
    else if (m_nType == VARTYPE_WIDESTR)
    {
        m_sValue = NULL;
        SetWideStr( source.WideStrVal() );
    }

	return *this;
}


#endif // _VAR_H

