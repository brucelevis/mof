/*-------------------------------------------------------------------------
	created:	2010/09/11  12:07
	filename: 	e:\Project_SVN\Server\Utils\VarTable\VarTable.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

#include "../Public/Var.h"
#include "../MessageQueue/MsgBuf.h"
#include <vector>
#include <map>

#include <ext/hash_map>

using namespace std;
using namespace __gnu_cxx;

struct ptrCmpHash
{
        bool operator()(const char* p1, const char*p2) const
        {
                return strcmp(p1, p2) == 0;
        }
};

struct ptrCmp
{
	bool operator()(const char* s1, const char* s2) const
	{
		if (!s1 || !s2) return false;

		return strcmp( s1, s2 ) < 0;
	}
};

// string hash function
inline unsigned int BKDRHash(const char *str)
{
	unsigned int hash = 0;
	while (*str)
	{
		hash = hash * 131 + (*str++);
	}
	return (hash & 0x7FFFFFFF);
}

enum PROP_VISUAL
{
	PV_HIDE = 0,  // 属性隐藏, 玩家不可见
	PV_PUBLIC,    // 属性公开，玩家和周围玩家可以见
	PV_PRIVATE,   // 属性隐藏，玩家自己可见
};

enum PROP_TAG_BIT
{
	PB_SAVE,           // 0位置， 是否保存
	PB_VISUAL_PUBLIC,  // 1 位置，公开可视
	PB_VISUAL_PRIVATE, // 2 位置，私人可视
	PB_CALLBACK,       // 3 位置，是否绑定回调
};

// 列内容
struct CVarRow
{
public:
	CVarRow(){varList = NULL;}
	~CVarRow(){if (varList) delete [] varList;};

	CVar* varList;
};

// 数据行处理函数 // 返回false 结束遍历
typedef
bool (*LPTRAVERSETABLEFUNC)(size_t index, CVarRow* & row, size_t colCount, void* param);

// 列定义
struct CVarCol
{
public:
	VAR_TYPE type;
	string name;
	bool Equal(CVarCol& another)
	{
		return (type == another.type && stricmp(name.c_str(), another.name.c_str()) == 0);
	}
};

class CVarTable
{
	friend class CPropList;
public:
	CVarTable();
	CVarTable(const char* szTableName, const char* colList, size_t initRows, size_t maxRows);
	~CVarTable();
public:
	// 创建表
	// colList: 列列表，格式如 "int,string,wstring" 则创建一个三列的表
	// initRows 创建时初始化行数
	// maxRows 最多允许的行数
	bool CreateTable(const char* szTableName, const char* colList, size_t initRows, size_t maxRows);

	// 根据新的列定义检查并修复表
	bool RepairTable(const char* colList, size_t maxRows);

	// 复制表
	CVarTable* Clone(const char* newName);

	// 获取名字
	const char* GetName();

	// 获取行数
	size_t GetRowCount();

	// 获取最大行数
	size_t GetMaxRowCount();

	// 设置最大行数
	void SetMaxRowCount(size_t rowCount);

	// 设置保存
	void SetSave(bool bSave);

	// 返回列数量
	size_t GetColCount();

	// 是否保存
	bool GetSave();

	// 插入表(row == -1表示插在尾部), 返回插入后所在index(返回 < 0 表示插入失败)
	int InsertRow(int row, CMsgTyped* pVarLsit = NULL);

	// 更新表
	bool SetRow(int row, CMsgTyped& VarList);

	// 更新表的一列
	bool SetRowInt(int row, int col, int val);
	bool SetRowString(int row, int col, const char* val);
	bool SetRowWideStr(int row, int col, const wchar_t* val);
	bool SetRowFloat(int row, int col, float val);
	bool SetRowBool(int row, int col, bool val);

	// 获取值
	bool GetRow(int row, CMsgTyped& VarList);
	size_t GetRows(int from, int to, CMsgTyped& varList);// from1,to3, 表示返回 1,2,3 3 行
	int GetRowInt(int row, int col);
	const char* GetRowString(int row, int col);
	const wchar_t* GetRowWideStr(int row, int col);
	float GetRowFloat(int row, int col);
	bool GetRowBool(int row, int col);

	// 查找空的行
	bool IsNull(int row);
	int FindFirstEmptyRow();

	// 查找指定行
	int FindRow(int value, size_t col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(float value, size_t col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(const char* value, size_t col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(const wchar_t* value, size_t col = 0, int* results = 0, int* resCnt = NULL);

	// 删除指定index的一行
	void DeleteRow(int index);

	// 删除指定行, 但保留其index;
	void ClearRow(int index);

	// 删除所有行
	void DeleteAllRows();

	// 清除所有行
	void ClearAllRows();

	// 删除所有空行
	bool DeleteAllEmptyRows();

	// 排序(只能对整数，浮点数列排序) col 从0 开始
	bool SortTable(int col, bool asc);

	// 遍历表
	void TraverseTable(LPTRAVERSETABLEFUNC func, void* param);

	// 删除所有
	void Clear();

	// 打印
	void Dump();
	void Dump(char* buffer, size_t bufsize);

	// 序列化
	bool Serialize(CMsgTyped& msg);

	// 反序列化
	bool Deserialize(CMsgTyped& msg);

protected:
	CVarRow* AllocRow();
	bool SetRowValue(CVarRow* pRow, CMsgTyped& var);
	bool GetRowValue(CVarRow* pRow, CMsgTyped& var);
	inline int GetColIndex(const char* colName);
	inline int GetColIndex(const char* colName, vector<CVarCol>& colTile);

private:
	string m_szTableName;
	vector<CVarCol> m_colTile;
	vector<CVarRow*> m_Data;
	size_t m_nMaxRows;
	bool m_bSave;
};

// 找出表中列的index
inline int CVarTable::GetColIndex(const char* colName)
{
	if (!colName) return -1;

	for (size_t i=0; i < m_colTile.size(); i++)
	{
		if (stricmp(m_colTile[i].name.c_str(), colName) == 0)
		{
			return (int)i;
		}
	}

	return -1;
}

// 找出表中列的index
inline int CVarTable::GetColIndex(const char* colName, vector<CVarCol>& colTile)
{
	if (!colName) return -1;

	for (size_t i=0; i < colTile.size(); i++)
	{
		if (stricmp(colTile[i].name.c_str(), colName) == 0)
		{
			return (int)i;
		}
	}

	return -1;
}

// 属性List
// ---------------------------------------------------------------------
/*
class CVarList
{
public:
    CVarList();
    ~CVarList();
public:
    bool GetSave();
    void SetSave(bool bSave);

    // 属性操作
    // 插入属性
    bool InsertInt(const char* name, int val, int index);
    bool InsertString(const char* name, const char* val, int index);
    bool InsertWideStr(const char* name, const wchar_t* val, int index);
    bool InsertFloat(const char* name, float val, int index);
    bool InsertBool(const char* name, bool val, int index);

    // 设置属性
    bool SetInt(const char* name, int val);
    bool SetString(const char* name, const char* val);
    bool SetWideStr(const char* name, const wchar_t* val);
    bool SetFloat(const char* name, float val);
    bool SetBool(const char* name, bool val);

    // 删除属性
    bool DeleteProp(const char* name);

    // 打印
	void Dump();
	void Dump(char* buffer, size_t bufsize);

    // 序列化
	bool Serialize(CMsgTyped& msg);

	// 反序列化
	bool Deserialize(CMsgTyped& msg);

private:
    string m_ListName;
    bool m_bSave;
    list<CVar*> m_varList;
};
*/

// 属性表管理类
class CPropList
{
public:
	CPropList();
	virtual ~CPropList();
	virtual void operator = (const CPropList& another);

public:
	// 添加参数
	bool AddPropInt(const char* name, int nVal = 0, bool save = false, PROP_VISUAL visual = PV_HIDE);
	bool AddPropString(const char* name, const char* szVal = "", bool save = false, PROP_VISUAL visual = PV_HIDE);
	bool AddPropWstr(const char* name, const wchar_t* wsVal = L"", bool save = false, PROP_VISUAL visual = PV_HIDE);
	bool AddPropFloat(const char* name, float fVal = 0.0f, bool save = false, PROP_VISUAL visual = PV_HIDE);
	bool AddPropBool(const char* name, bool bVal = true, bool save = false, PROP_VISUAL visual = PV_HIDE);

	// 设置，获取参数属性
	void SetSave(const char* name, bool bSave = true);
	bool GetSave(const char* name);

    // 设置，获取参数属性
	void SetCallBk(const char* name, void* cb);
	void* FindCallBk(const char* name);

	// 设置可视属性
	void SetVisual(const char* name, PROP_VISUAL visual = PV_PRIVATE);
	PROP_VISUAL GetVisual(const char* name);

	// 获取参数
	int GetPropInt(const char* name, bool* bSucc = NULL, int nDef = 0);
	const char* GetPropString(const char* name, bool* bSucc = NULL, const char* szDef = "");
	const wchar_t* GetPropWstr(const char* name, bool* bSucc = NULL, const wchar_t* wsDef = L"");
	float GetPropFloat(const char* name, bool* bSucc = NULL, float fDef = 0.0f);
	bool GetPropBool(const char* name, bool* bSucc = NULL, bool bDef = false);

	// 更新参数
	bool SetPropInt(const char* name, int nVal = 0, void* pOperator = NULL, bool sync = true);
	bool SetPropString(const char* name, const char* szVal = "", void* pOperator = NULL, bool sync = true);
	bool SetPropWstr(const char* name, const wchar_t* wsVal = L"", void* pOperator = NULL, bool sync = true);
	bool SetPropFloat(const char* name, float fVal = 0.0f, void* pOperator = NULL, bool sync = true);
	bool SetPropBool(const char* name, bool bVal = true, void* pOperator = NULL, bool sync = true);

	// 更新参数
	bool IncPropInt(const char* name, int inc, int max=0x7FFFFFFF, void* pOperator = NULL, bool sync = true);
	bool IncPropFloat(const char* name, float inc, float max=999999.0f, void* pOperator = NULL, bool sync = true);
	bool DecPropInt(const char* name, int dec, int min=0, void* pOperator = NULL, bool sync = true);
	bool DecPropFloat(const char* name, float dec, float min=0.0f, void* pOperator = NULL, bool sync = true);

	// 删除和查找
	void DeleteProp(const char* name);
	CVar* FindProp(const char* name, VAR_TYPE type = VARTYPE_UNKNOWN);
	void ClearPropList();

public:
	// 属性表操作
	// 创建表
	// colList: 列列表，格式如 "int,string,wstring" 则创建一个三列的表
	// initRows 创建时初始化行数
	// maxRows 最多允许的行数
	bool CreateTable(const char* szTableName,
		const char* colList, size_t initRows, size_t maxRows,
		void* pOperator = NULL);

	// 查找表
	CVarTable* FindTable(const char* szTableName);

	// 删除表
	void DeleteTable(const char* szTableName, void* pOperator = NULL);

	// 获取表数量
	size_t GetTableCount();

	// 获取名字
	const char* GetTableName(int index);

	// 获取当前行数
	size_t GetRowCount(const char* szTableName);

	// 获取最大行数
	size_t GetMaxRowCount(const char* szTableName);

	// 设置最大函数
	void SetMaxRowCount(const char* szTableName, size_t rowCount, void* pOperator = NULL);

	// 设置保存
	void SetSaveTable(const char* szTableName, bool bSave);

	// 返回列数量
	size_t GetColCount(const char* szTableName);

	// 是否保存
	bool GetSaveTable(const char* szTableName);

	// 插入表(row == -1表示插在尾部), 返回插入后所在index(返回 < 0 表示插入失败)
	int InsertRow(const char* szTableName, int row, CMsgTyped* pVarLsit = NULL, void* pOperator = NULL);

	// 更新表
	bool SetRow(const char* szTableName, int row, CMsgTyped& VarList, void* pOperator = NULL);

	// 更新表的一列
	bool SetRowInt(const char* szTableName, int row, int col, int val, void* pOperator = NULL);
	bool SetRowString(const char* szTableName, int row, int col, const char* val, void* pOperator = NULL);
	bool SetRowWideStr(const char* szTableName, int row, int col, const wchar_t* val, void* pOperator = NULL);
	bool SetRowFloat(const char* szTableName, int row, int col, float val, void* pOperator = NULL);
	bool SetRowBool(const char* szTableName, int row, int col, bool val, void* pOperator = NULL);

	bool SetRowInt(const char* szTableName, int row, const char* col, int val, void* pOperator = NULL);
	bool SetRowString(const char* szTableName, int row, const char* col, const char* val, void* pOperator = NULL);
	bool SetRowWideStr(const char* szTableName, int row, const char* col, const wchar_t* val, void* pOperator = NULL);
	bool SetRowFloat(const char* szTableName, int row, const char* col, float val, void* pOperator = NULL);
	bool SetRowBool(const char* szTableName, int row, const char* col, bool val, void* pOperator = NULL);

	// 获取值
	bool GetRow(const char* szTableName, int row, CMsgTyped& VarList);
	size_t GetRows(const char* szTableName, int from, int to, CMsgTyped& varList);// from1,to3, 表示返回 1,2,3 3 行

	int GetRowInt(const char* szTableName, int row, int col);
	const char* GetRowString(const char* szTableName, int row, int col);
	const wchar_t* GetRowWideStr(const char* szTableName, int row, int col);
	float GetRowFloat(const char* szTableName, int row, int col);
	bool GetRowBool(const char* szTableName, int row, int col);

	int GetRowInt(const char* szTableName, int row, const char* col);
	const char* GetRowString(const char* szTableName, int row, const char* col);
	const wchar_t* GetRowWideStr(const char* szTableName, int row, const char* col);
	float GetRowFloat(const char* szTableName, int row, const char* col);
	bool GetRowBool(const char* szTableName, int row, const char* col);

	// 查找空的行
	bool IsNull(const char* szTableName, int row);
	int FindFirstEmptyRow(const char* szTableName);

	// 复制表
	bool CopyTable(const char* szSrcTable, const char* szDestTable, void* pOperator = NULL);

	// 查找指定行
	int FindRow(const char* szTableName, int value, size_t col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(const char* szTableName, float value, size_t col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(const char* szTableName, const char* value, size_t col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(const char* szTableName, const wchar_t* value, size_t col = 0, int* results = 0, int* resCnt = NULL);

	int FindRow(const char* szTableName, int value, const char* col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(const char* szTableName, float value, const char* col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(const char* szTableName, const char* value, const char* col = 0, int* results = 0, int* resCnt = NULL);
	int FindRow(const char* szTableName, const wchar_t* value, const char* col = 0, int* results = 0, int* resCnt = NULL);

	// 删除指定index的一行
	void DeleteRow(const char* szTableName, int index, void* pOperator = NULL);

	// 删除指定行, 但保留其index;
	void ClearRow(const char* szTableName, int index, void* pOperator = NULL);

	// 删除所有行
	bool DeleteAllRows(const char* szTableName, void* pOperator = NULL);

	// 清除所有行
	bool ClearAllRows(const char* szTableName, void* pOperator = NULL);

	// 删除所有空行
	bool DeleteAllEmptyRows(const char* szTableName, void* pOperator = NULL);

	// 排序(只能对整数，浮点数列排序) col 从0 开始
	bool SortTable(const char* szTableName, int col, bool asc, void* pOperator = NULL);
	bool SortTable(const char* szTableName, const char* col, bool asc, void* pOperator = NULL);

	// 遍历表
	bool TraverseTable(const char* szTableName, LPTRAVERSETABLEFUNC func, void* param, void* pOperator = NULL);

	// 删除所有属性表
	void ClearTables();

	// 打印表
	void DumpTable(const char* szTableName, char* buffer = NULL, size_t bufsize = 0);

public:
	bool Serialize(CMsgTyped& msg);
	bool Deserialize(CMsgTyped& msg);
	bool Serialize(string& json, bool styled = true);
	bool Deserialize(string& json);

protected:
	virtual void OnUpdatePropInt(const char* name, int nOldVal, int nNewVal, void* pOperator, bool bSave, PROP_VISUAL pv, void* cb){};
	virtual void OnUpdatePropString(const char* name, const char* szNewVal, void* pOperator, bool bSave, PROP_VISUAL pv, void* cb){};
	virtual void OnUpdatePropWstr(const char* name, const wchar_t* wsNewVal, void* pOperator, bool bSave, PROP_VISUAL pv, void* cb){};
	virtual void OnUpdatePropFloat(const char* name, float fOldVal, float fNewVal, void* pOperator, bool bSave, PROP_VISUAL pv, void* cb){};
	virtual void OnUpdatePropBool(const char* name, bool bOldVal, bool bNewVal, void* pOperator, bool bSave, PROP_VISUAL pv, void* cb){};
	virtual void OnUpdatePropTable(const char* table, bool bSave, void* pOperator){};
	//virtual void OnUpdatePropList(const char* list, bool bSave, void* pOperator){};

protected:
	map<UI32, CVar*> m_proplist;
	map<char*, void*, ptrCmp> * m_callbk;

	vector<CVarTable*> m_tableList;
	//vector<CVarList*> m_listList;
};

extern string VarToString(const CVar* pVar, bool UTF8 = false);
extern PROP_VISUAL GetVisual(const CVar* pVar);
extern int GetVarLevel(const CVar* pVar);
extern bool GetSave(const CVar* v);
extern void SetSave(CVar* v, bool save);
extern bool GetCallBack(const CVar* v);
extern void SetCallBack(CVar* v, bool cb);
extern void DumpVarAlloc();
