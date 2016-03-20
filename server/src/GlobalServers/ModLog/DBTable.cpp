#include "DBTable.h"


const char* g_arrFieldTypeStr[EFT_NUM] =
{
    "bool",
    "int",
    "float",
    "varchar",
    "nvarchar",
    "time"
};

CDBTable::CDBTable(const CDBTable& tb)
{
    m_sTableName = tb.m_sTableName;
    m_nFieldNum = tb.m_nFieldNum;
    m_szInsertSQL = tb.m_szInsertSQL;

    m_vectFieldsName.clear();
    m_vectFieldsType.clear();

    for (int i = 0; i < m_nFieldNum; i++)
    {
        m_vectFieldsName.push_back(tb.GetFieldName(i).c_str());
        m_vectFieldsType.push_back(tb.GetFieldType(i));
    }
}

CDBTable& CDBTable::operator=(const CDBTable& tb)
{
    if (this != &tb)
    {
        m_sTableName = tb.m_sTableName;
        m_nFieldNum = tb.m_nFieldNum;
        m_szInsertSQL = tb.m_szInsertSQL;

        m_vectFieldsName.clear();
        m_vectFieldsType.clear();

        for (int i = 0; i < m_nFieldNum; i++)
        {
            m_vectFieldsName.push_back(tb.GetFieldName(i).c_str());
            m_vectFieldsType.push_back(tb.GetFieldType(i));
        }
    }

    return *this;
}



void CDBTable::FormatInsertSQL()
{
    m_szInsertSQL = "";
    if (m_nFieldNum > 0)
    {
//        m_szInsertSQL.append("INSERT INTO ");
//        m_szInsertSQL.append(m_sTableName);
        m_szInsertSQL.append("(");
        m_szInsertSQL.append(m_vectFieldsName[0]);

        for (int i = 1; i < m_nFieldNum; i++)
        {
            m_szInsertSQL.append(", ");
            m_szInsertSQL.append(m_vectFieldsName[i]);
        }

        m_szInsertSQL.append(") VALUES ");
    }
}
