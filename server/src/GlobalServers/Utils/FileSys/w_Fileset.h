// ----------------------------------------------------------------------------
// Desc    :
// History :
// 3/2/2006   17:48  唐飞龙，任麒麟(Lovely Kylin) Created.
// ----------------------------------------------------------------------------

#ifndef __w_Fileset_h_3_2_17_48__
#define __w_Fileset_h_3_2_17_48__

#ifndef LINUX_
#include <windows.h>
#endif  // LINUX_
#include <string>
#include <vector>
#include <stdio.h>

// ----------------------------------------------------------------------------
struct _File_Item
{
    // 文件名（无路径）
    std::string         m_szName;
    // 用户数据
    std::vector<char>   m_UserData;
};

// ----------------------------------------------------------------------------
class CfileSet
{
public:
    CfileSet();
    virtual ~CfileSet();

    void        CleanMembers();
    void        Destroy();

	// 从指定的数据流载入文件列表
	bool        Load(const char* filename);

	// 保存到数据流
	bool        Save(const char* filename);

    // 从指定的数据流载入文件列表
    bool        Load(FILE * stream);

    // 保存到数据流
    bool        Save(FILE * stream);

    // 从文件夹载入
    bool        LoadFromDirectory(const char* szPath);

    // 添加一个文件及其父文件夹(相对根的路径)
    bool        CreateSubFile(
        const char* szRelPath,
        const void* pUserData,
        size_t nBytes);

    // 添加一个文件夹及其父文件夹(相对根的路径)
    bool        CreateSubFolder(const char* szRelPath);

    // 删除一个指定的文件(相对根的路径)
    bool        DeleteSubFile(const char* szRelPath);

    // 删除一个指定的目录(相对根的路径)
    bool        DeleteSubDir(const char* szRelPath);

    // 获取当前目录下文件数量
    size_t      GetFileCount() const;

    // 获取第I个文件
    const char* GetFile(unsigned int i) const;

    // 获取第i个文件的用户数据
    const void* GetFileData(unsigned int i) const;

    // 获取第i个文件用户数据的大小
    size_t      GetFileDataSize(unsigned int i) const;

    // 获取当前目录下的子目录数量
    size_t      GetSubDirCount() const;

    // 获取第i个子目录
    CfileSet&   GetSubDir(unsigned int i);

    // 获取本目录名
    const char* GetDirName() const;

    // 设置本目录名
    void SetDirName(const char* szName);

	// 查找本set下是否有某文件夹
	bool FindSubFolder(const char* szPath);

	// 查找set下是否有某文件
	bool FindSubFile(const char* szFile);

protected:
    // 找子目录
    CfileSet*               _FindSubFolder(const char* szSubFolder);

    CfileSet*               _CreateSubFolder(const char* szSubFolder);

    bool                    _CreateSubFile(
        const char* szFileName,
        const void* pUserData,
        size_t nBytes);

    bool                    _DeleteSubFile(
        const char* szFileName);

    bool                    _DeleteSubDir(
    const char* szDirName);

private:
    // 本文件夹的名字
    std::string             m_szFolderName;
    // 本文件夹下的文件
    std::vector<_File_Item> m_files;
    // 子文件夹们
    std::vector<CfileSet>   m_subFolders;
};

#endif // #ifndef __w_Fileset_h_3_2_17_48__
// END OF FILE
