#ifndef DB_TABLE_H
#define DB_TABLE_H

#include <vector>
#include <string>
#include <stdio.h>
using namespace std;

enum FIELD_TYPE
{
    EFT_BOOL,
    EFT_INT,
    EFT_FLOAT,
    EFT_VARCHAR,
    EFT_NVARCHAR,
    EFT_TIME,
    EFT_NUM
};

extern const char* g_arrFieldTypeStr[EFT_NUM];

class CDBTable
{
private:
    string         m_sTableName;
    int            m_nFieldNum;
    vector<string> m_vectFieldsName;
    vector<int>    m_vectFieldsType;
    string         m_szInsertSQL;
    string         m_sEmptyStr;

    int            m_nSavedDays;

public:

    CDBTable() : m_nFieldNum(-1)
    {
    }
    CDBTable(const CDBTable&);
    CDBTable& operator=(const CDBTable&);

    void SetSavedDays(int days)
    {
        m_nSavedDays = days;
    }

    int GetSavedDays() const
    {
        return m_nSavedDays;
    }

    void SetTableName(const char* tbName)
    {
        m_sTableName = tbName;
    }
    void SetFieldNum(int fdNum)
    {
        if (fdNum <= 0)
        {
            return;
        }
        m_nFieldNum = fdNum;
        m_vectFieldsName.resize(fdNum);
        m_vectFieldsType.resize(fdNum);
    }
    void SetFieldName(int index, const char* fdName)
    {
        m_vectFieldsName[index] = fdName;
    }
    void SetFieldType(int index, int fdType)
    {
        m_vectFieldsType[index] = fdType;
    }
    int GetFieldNum() const
    {
        return m_nFieldNum;
    }
    const string& GetFieldName(int index) const
    {
        if (index < 0 || index > m_nFieldNum)
        {
            return m_sEmptyStr;
        }
        return m_vectFieldsName[index];
    }
    int GetFieldType(int index) const
    {
        if (index < 0 || index > m_nFieldNum)
        {
            return -1;
        }
        return m_vectFieldsType[index];
    }

    void FormatInsertSQL();

    const string& GetTableName() const
    {
        return m_sTableName;
    }

    const string& GetInsertSQL() const
    {
        return m_szInsertSQL;
    }

    void PrintInfo()
    {
        //printf("------------------------------------------------\n");
        printf("table_name: %s.field_num: %d\n", m_sTableName.c_str(), m_nFieldNum);
        /*for (int i = 0; i < m_vectFieldsName.size(); i++)
        {
            int type = m_vectFieldsType[i];
            printf("%-20s  %-20s\n", m_vectFieldsName[i].c_str(), g_arrFieldTypeStr[type]);
        }*/
    }

    void SetColumNames(vector<string> colums)
    {
        m_vectFieldsName = colums;
    }
};

#endif
