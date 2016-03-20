#pragma once

#include "../ThreadPool/ThreadPool.h"
#include "./minilzo.h"

#define LZO_BUF_LEN 1024*512
#define LZO_WRKMEN_LEN 16384L
#define LZO_HEAD "use_minilzo_zipped"

#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

#define ZIP_DATA 1

class lzo_item
{
public:
        lzo_item(){  m_res = NULL; m_res_len = 0;   };
        ~lzo_item(){  };
public:
    // 压缩/解压
    void zip( char* input,  int len)
    {
        // 清空结果
        m_res = NULL;
        m_res_len = 0;

        if (!ZIP_DATA) return;

        if (lzo_init() != LZO_E_OK)
        {
            return;
        }

        // 判断输入
        if (!input || len < 1) return;

        unsigned char __LZO_MMODEL * dest = m_buf;
        lzo_uint dest_len = LZO_BUF_LEN;

        if (lzo1x_1_compress((unsigned char*)input, len, dest,&dest_len, m_wrkmem) == LZO_E_OK)
        {
            // 压缩成功
            m_res = (char*)m_buf;
            m_res_len = dest_len;
        }
    }


    void unzip( char* input, int len)
    {
        // 清空结果
        m_res = NULL;
        m_res_len = 0;

        if (!ZIP_DATA) return;

        if (lzo_init() != LZO_E_OK)
        {
            return;
        }

        // 判断输入
        if (!input || len < 1) return;

        //  开始解压
        lzo_uint dest_len = LZO_BUF_LEN ;

        if (lzo1x_decompress_safe((unsigned char*)input, len, m_buf,&dest_len,  NULL) == LZO_E_OK)
        {
            // 压缩成功
            m_res = (char*)m_buf;
            m_res_len = dest_len ;
        }
    }

    // 获取结果
    char* GetResult(){ return m_res;}
    int GetResultLen(){return m_res_len;}

private:
    char* m_res;
    int m_res_len;

    // 1M 的buff 用于存储压缩和解压缩的结果
    unsigned char __LZO_MMODEL m_buf  [ LZO_BUF_LEN ];

    // 工作内存
    HEAP_ALLOC(m_wrkmem, LZO1X_1_MEM_COMPRESS);

public:
    // 临时空间
    char m_temp[LZO_BUF_LEN];
};

class LzoMgr : CriticalObject
{
public:
    LzoMgr()
    {
    }
    ~LzoMgr()
    {
        SMART_LOCK_THIS();

        // 清除内存
        for (size_t i=0; i < m_LzoItems.size(); i++)
        {
            SAFE_DELETE( m_LzoItems[i]  );
        }
        m_LzoItems.clear();
    }

    lzo_item* Alloc()
    {
        SMART_LOCK_THIS();

        for (size_t i=0; i < m_LzoItems.size(); i++)
        {
            // 返回有效的lzo_item
            if (m_LzoItems[i])
            {
                lzo_item* ret = m_LzoItems[i];
                m_LzoItems[i] = NULL;

                return ret;
            }
        }
        return new lzo_item;
    }
    void Reuse( lzo_item* pItem)
    {
        if (!pItem) return;

        SMART_LOCK_THIS();

        for (size_t i=0; i < m_LzoItems.size(); i++)
        {
            // 找一个有效的item
            if (!m_LzoItems[i])
            {
                m_LzoItems[i] = pItem;
                return;
            }
        }

        // 放到尾巴
        m_LzoItems.push_back(pItem);
    }
private:
    vector<lzo_item* > m_LzoItems;
};



