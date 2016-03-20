// ----------------------------------------------------------------------------
// Desc    :
// History :
// 3/22/2006   15:04  任麒麟(Lovely Kylin) Created.
// ----------------------------------------------------------------------------

#ifndef __w_FileSys_h_3_22_15_04__
#define __w_FileSys_h_3_22_15_04__

#ifndef LINUX_
    #include <windows.h>
    #include <io.h>
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <string.h>
    #include "SearchOps.h"
    #define MAX_PATH    128
#endif
#include "../Public/Macros.h"
#include <assert.h>
#include <vector>
#include <string>
#include "zlib/zlib.h"

#define fix_path(path) 	if (path.size() > 0 && path[path.size()-1] != '/') \
	path += "/"; \

struct W_FileInFolder
{
    std::vector<std::string> m_subFolderPick;
    std::vector<std::string> m_subFolderFull;
    std::vector<std::string> m_filePickNames;
    std::vector<std::string> m_fileFullNames;
};

// 用于遍历ini文件
struct FindResult
{
	std::string         m_szRelPath;
	std::string         m_szFileName;
};

typedef std::vector<FindResult> CompareResult;

// ----------------------------------------------------------------------------
void FindFiles(const std::string& szFolder, CompareResult & rst,
			   const std::string& szTmpPath);

void List_Files(const char *path, CompareResult & rst);

bool W_IsDirectory(const char* szFileName);

void W_FindFileInDirectory(W_FileInFolder& ret_folder_info, const char* szDir);

void W_CreateDirectory(
    const char* szFileName);

void W_Process_Directory(
    const char* szDir,
    void (* fn_process)(const char* szFileName));

void W_Process_CurDirectory(
    void (* fn_process)(const char* szFileName));

bool W_IsReadOnly(const char* szFileName);

void W_SetReadOnly(
    const char* szFileName,
    bool bReadOnly);

__int64 W_GetFileSize(const char* szFileName);

bool W_IsFileExist(const char* szFileName);

bool W_ForceDeleteFile(const char* szFileName);

bool W_CopyFile(
    const char* szFromFile,
    const char* szDestFile);

bool W_ForceCopyFile(
    const char* szFromFile,
    const char* szDestFile);

bool W_RenameFile(
    const char* szFromFile,
    const char* szDestFile);

bool W_MoveFile(
    const char* szFromFile,
    const char* szDestFile);

bool W_IsFileSame(
    const char* szFile1,
    const char* szFile2);

bool W_CopyTree(
    const char* szFromFolder,
    const char* szDestFolder);

bool W_MoveTree(
    const char* szFromFolder,
    const char* szDestFolder);

bool W_DelTree(const char* szFromFolder);

bool W_GetAppPath(std::string & sAppPath);

//-------------------------------------------------------------------------
struct ValueItem
{
	std::string sec;
	std::string name;
	std::string value;
};

class CIniFile2
{
public:
	CIniFile2();
	CIniFile2(const char * filename);
	~CIniFile2();
	bool LoadFromFile();

public:
	int ReadInteger(const char * section, const char * key, int def) const;

	float ReadFloat(const char * section, const char * key, float def) const;

	std::string ReadString(const char * section,
		const char * key, const char * def) const;

	void ReadSectionValues(const char* section, std::vector<ValueItem>& vec);

private:
	void AddValue(const char* section, const char* key, const char* val);

private:
	std::string m_szIniFile;
	std::vector<ValueItem> m_Items;
};


#endif // #ifndef __w_FileSys_h_3_22_15_04__
// END OF FILE
