/* ----------------------------------------------------------------------------
简要描述:
历史记录:
  3/14/2006   17:54  唐飞龙 Created.
-----------------------------------------------------------------------------*/
#ifndef __w_CompareFolder_h_3_14_17_54__
#define __w_CompareFolder_h_3_14_17_54__

#include "zlib/zlib.h"

#include <string>
#include <vector>
#include <assert.h>

#define GZ_MAX_BUFFER_LEN 8192

enum ChangeType
{
	CT_ContentChange = 0,
	CT_Add = 1,
	CT_Del = 2,
};

struct Changed_Struct
{
    std::string         m_szFileName;
    bool                m_bIsFolder;

    ChangeType          m_changeType;
    size_t              m_nBytes;
};

typedef void (*PROGRESS_FUNC) (int nPercent);

typedef std::vector<Changed_Struct> ChangeList;

class CfileSet;
class CCompareFolder
{
public:
    CCompareFolder() {
		m_pFilter = NULL;
		m_Func = NULL;
        ClearMembers();
    };
    ~CCompareFolder() {
        ClearMembers();
    };

    enum  _Compare_type
    {
        CP_Folder_Folder,
        CP_Folder_FileSet,
    };
public:
	// 设置过滤器
	void SetFilter(CfileSet* pSet);

	// 设置进度回调
	void SetProgresFunc(PROGRESS_FUNC func);

    // 设置旧目录
    void SetOldFolder(const char* szOldFolder);

    // 设置新目录
    void SetNewFolder(const char* szNewFolder);

    // 比较两个目录
    bool Compare();

    // 获取结果的数量
    size_t GetResultCount();

    // 获取第i个结果
    const Changed_Struct& GetResult(size_t nIndex) const;

    // 更新旧目录 使之与新目录同步
    bool ApplyChanges();

    // 制作差异包
    bool MakeDiffZip(const char* szDiffZip);

    // 制作新目录的完全包
    bool MakeFullZip(const char* szFullZip);

    //比较两个目录， 将结果存入一个容器
    bool CompareDirectory(
        const char* szLastVersion,
        const char* szCurrentVersion,
        ChangeList& ChangedFiles
        );

    //清空结果
    void ClearMembers();
    bool unpack(
        const char* szPackage,
        const char* szDestLocation,
		bool bForce = true);

    // 遍历文件夹
    static void _findFile(
                const std::string& szFolder,
                ChangeList & rst,
                const std::string& szTmpPath = "");

    // 遍历文件夹, 不递归
    static void _findFile_dont_Rec(
                const std::string& szFolder,
                ChangeList & rst,
                const std::string& szTmpPath = "");
protected:
    // 找到要删除的项目
    bool _findRemove(
            ChangeList& ChangedFile,
            const std::string& szOldDir,
            const std::string& szNewDir,
            const std::string& szTempDir = "");

    // 找到要添加的项目
    bool _findAdd(
            ChangeList& ChangedFile,
            const std::string& szOldDir,
            const std::string& szNewDir,
            const std::string& szTempDir = "");

    // 添加一个文件到压缩文件
    bool AddToZip(
        gzFile gzOut,
        const char* szFolder,
        const char* szFileIn);

    bool AddToZip(
        gzFile gzOut,
        const char*szFolder,
        Changed_Struct *pChangeStruct);

    // 比较文件是否相同
    bool  NeedUpdate(const char* szOldFile,
                     const char* szNewFile);

private:
    ChangeList m_ChangedFiles;
    std::string   m_szOldFolder;
    std::string   m_szNewFolder;
	CfileSet*     m_pFilter;
	PROGRESS_FUNC m_Func;
};

// ----------------------------------------------------------------------------
#endif // #ifndef __w_CompareFolder_h_3_14_17_54__
