// ----------------------------------------------------------------------------
// Desc    :
// History :
// 3/22/2006   15:04  任麒麟(Lovely Kylin) Created.
// ----------------------------------------------------------------------------

#pragma warning(disable:4786)

#include "FileSys.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include "../Public/Inlines.h"

#ifdef LINUX_
    #include <stdarg.h>
    #include "SearchOps.h"
    #include <dirent.h>
#endif

#include <string>
using namespace std;

#pragma warning(disable:4786)
#pragma warning(disable:4996)

#define FOLDER_SEPERATOR        '\\'

void FindFiles(const std::string& szFolder, CompareResult & rst,
				  const std::string& szTmpPath)
{
	std::string tmpPath = szTmpPath;
	if (tmpPath.size() > 0)
	{
		if (tmpPath[tmpPath.size()-1] != '/')
		{
			tmpPath += "/";
		}
	}

	std::string currentPath = tmpPath + "*.*";
	_finddata_t wfd;
	std::string strFile = szFolder + currentPath;
	std::string sConfigPath;

	long hFileFind = _findfirst(strFile.c_str(), &wfd);
	if (hFileFind == -1)
	{
		return;
	}
	do
	{
		if (strcmp(wfd.name, ".") && strcmp(wfd.name, ".."))
		{
			if (wfd.attrib & _A_SUBDIR)
			{
				std::string szSubFolder = tmpPath + wfd.name + "/";
				FindFiles(szFolder, rst, szSubFolder);
			}
			else
			{
				std::string szFileName = wfd.name;
				rst.resize(rst.size() + 1);
				rst.back().m_szFileName = tmpPath + wfd.name;
				rst.back().m_szRelPath = tmpPath;
			}
		}

	} while(_findnext(hFileFind, &wfd));

	_findclose(hFileFind);
}

int IS_DIR(const char* path)
{
    struct stat st;
    lstat(path, &st);
    return S_ISDIR(st.st_mode);
}

//遍历文件夹de递归函数
void List_Files_Core(const char *path, CompareResult & rst, string relpath)
{
    DIR *pdir;
    struct dirent *pdirent;
    char temp[256];
    pdir = opendir(path);
    if ( !pdir ) return;
    while(pdirent = readdir(pdir))
    {
        //跳过"."和".."
        if(strcmp(pdirent->d_name, ".") == 0 || strcmp(pdirent->d_name, "..") == 0)
        {
            continue;
        }

        sprintf(temp, "%s/%s", path, pdirent->d_name);

        //当temp为目录并且recursive为1的时候递归处理子目录
        if (IS_DIR(temp))
        {
            string nrelpath = relpath;
            nrelpath += pdirent->d_name;
            nrelpath += "/";
            List_Files_Core(temp, rst, nrelpath);
        }
        else
        {
            // 文件
            std::string szFileName = pdirent->d_name;
            rst.resize(rst.size() + 1);
            rst.back().m_szFileName = relpath + pdirent->d_name;
            rst.back().m_szRelPath = relpath;
        }
    }
    closedir(pdir);
}

//遍历文件夹的驱动函数
void List_Files(const char *path, CompareResult & rst)
{
    int len;
    char temp[256];

    //去掉末尾的'/'
    len = strlen(path);
    strcpy(temp, path);
    if(temp[len - 1] == '/') temp[len -1] = '\0';

    if (IS_DIR(temp))
    {
        //处理目录
        List_Files_Core(temp, rst, "");
    }
}

void W_FindFileInDirectory(W_FileInFolder& ret_folder_info, const char* szDir)
{
	assert(szDir);

	ret_folder_info.m_fileFullNames.clear();
	ret_folder_info.m_filePickNames.clear();
	ret_folder_info.m_subFolderFull.clear();
	ret_folder_info.m_subFolderPick.clear();

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurDir);
	szCurDir[MAX_PATH-1] = 0;

	_finddata_t findData;

	char szFindDir[MAX_PATH];
	sprintf(szFindDir, "%s\\*", szDir);

	long hFind = _findfirst(szFindDir, &findData);

	if (hFind == -1)
	{
		return;
	}

	if ((findData.attrib & _A_SUBDIR) == _A_SUBDIR)
	{
		if (stricmp(findData.name, "..") != 0 && stricmp(findData.name, ".") != 0)
		{
			std::string strFileName(findData.name);
			ret_folder_info.m_subFolderPick.push_back(strFileName);
			sprintf(szFindDir, "%s\\%s", szDir, findData.name);
			strFileName = szFindDir;
			ret_folder_info.m_subFolderFull.push_back(strFileName);
		}
	}
	else
	{
		std::string strFileName(findData.name);
		ret_folder_info.m_filePickNames.push_back(strFileName);
		sprintf(szFindDir, "%s\\%s", szDir, findData.name);
		strFileName = szFindDir;
		ret_folder_info.m_fileFullNames.push_back(strFileName);
	}
	bool bFindNextSucceeded = true;

	for (; bFindNextSucceeded = _findnext(hFind, &findData); )
	{
		if ((findData.attrib & _A_SUBDIR) == _A_SUBDIR)
		{
			if (stricmp(findData.name, "..") != 0 && stricmp(findData.name, ".") != 0)
			{
				std::string strFileName(findData.name);
				ret_folder_info.m_subFolderPick.push_back(strFileName);
				sprintf(szFindDir, "%s\\%s", szDir, findData.name);
				strFileName = szFindDir;
				ret_folder_info.m_subFolderFull.push_back(strFileName);
			}
		}
		else
		{
			std::string strFileName(findData.name);
			ret_folder_info.m_filePickNames.push_back(strFileName);
			sprintf(szFindDir, "%s\\%s", szDir, findData.name);
			strFileName = szFindDir;
			ret_folder_info.m_fileFullNames.push_back(strFileName);
		}
	}

	_findclose(hFind);

	SetCurrentDirectory(szCurDir);
}

void W_Process_Directory(const char* szDir, void (* fn_process)(const char* szFileName))
{
	assert(szDir);
	assert(fn_process);

	W_FileInFolder files;
	W_FindFileInDirectory(files, szDir);

	UINT i;
	for (i = 0; i < files.m_fileFullNames.size(); ++i)
	{
		fn_process(files.m_fileFullNames[i].c_str());
	}
	for (i = 0; i < files.m_subFolderFull.size(); ++i)
	{
		W_Process_Directory(
			files.m_subFolderFull[i].c_str(), fn_process);
	}
}
void W_Process_CurDirectory(void (* fn_process)(const char* szFileName))
{
	assert(fn_process);

	char buff[256];
	GetCurrentDirectory(256, buff);
	buff[255] = 0;

	W_Process_Directory(buff, fn_process);
}

// ----------------------------------------------------------------------------
bool W_IsDirectory(const char* szFileName)
{
	if (!W_IsFileExist(szFileName))
	{
		return false;
	}

	long dwOrgAttr = GetFileAttributes(szFileName);
	if (dwOrgAttr & _A_SUBDIR)
	{
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------
void W_CreateDirectory(const char* szFileName)
{
	assert(szFileName);

	size_t nLen = strlen(szFileName);

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurDir);
	szCurDir[MAX_PATH-1] = 0;

	UINT i;
	char buff[256];
	for (i = 0; i < nLen; ++i)
	{
		if (szFileName[i] == FOLDER_SEPERATOR && i > 0)
		{
			memcpy(buff, szFileName, i);
			buff[i] = 0;

			CreateDirectory(buff, NULL);
		}
	}
	if( szFileName[nLen - 1] != FOLDER_SEPERATOR )
	{
		memcpy(buff, szFileName, nLen);
		buff[nLen] = 0;

		CreateDirectory(buff, NULL);
	}


	SetCurrentDirectory(szCurDir);
	return;
}
bool W_IsReadOnly(const char* szFileName)
{
	assert(szFileName);

	if (!W_IsFileExist(szFileName))
	{
		return false;
	}
	long dwOrgAttr = GetFileAttributes(szFileName);
	if (dwOrgAttr & _A_RDONLY)
	{
		return true;
	}
	return false;
}

void W_SetReadOnly(
				   const char* szFileName,
				   bool bReadOnly)
{
	assert(szFileName);
	if (!szFileName) {
		return;
	}

	long dwOrgAttr = GetFileAttributes(szFileName);
	if (!bReadOnly)
	{
		if (!SetFileAttributes(
			szFileName,
			dwOrgAttr & (~_A_RDONLY)))
		{
			return;
		}
	}
	else
	{
		if (!SetFileAttributes(
			szFileName,
			dwOrgAttr | _A_RDONLY))
		{
			return;
		}
	}

	return;
}

__int64 W_GetFileSize(const char* szFileName)
{
	assert(szFileName);
	if (!szFileName) return -1;

	struct stat fileData;
    if (0 == stat(szFileName, &fileData))
    {
       return (__int64)fileData.st_size;
    }
    return -1;
}
bool W_IsFileExist(const char* szFileName)
{
	assert(szFileName);

	_finddata_t fnd_data;
	memset(&fnd_data, 0, sizeof(fnd_data));
	long hdl = _findfirst(szFileName, &fnd_data);
	if (hdl != -1)
	{
		_findclose(hdl);
		hdl = -1;
		return true;
	}
	return false;
}
bool W_ForceDeleteFile(const char* szFileName)
{
	assert(szFileName);

	if (!W_IsFileExist(szFileName))
	{
		return true;
	}

	long dwOrgAttr = GetFileAttributes(szFileName);
	if (dwOrgAttr & _A_RDONLY)
	{
		SetFileAttributes(szFileName, dwOrgAttr & (~_A_RDONLY));
	}

	if( DeleteFile(szFileName) )
	{
		return true;
	}

	return false;
}
bool W_CopyFile(const char* szFromFile, const char* szDestFile)
{
	assert(szFromFile);
	assert(szDestFile);

	if( CopyFile(szFromFile, szDestFile, false))
	{
		return true;
	}
	return false;
}
bool W_ForceCopyFile(const char* szFromFile, const char* szDestFile)
{
	assert(szFromFile);
	assert(szDestFile);

	char szPath[MAX_PATH];
	_splitpath(szDestFile, NULL, szPath, NULL, NULL);
	W_CreateDirectory(szPath);

	if( CopyFile(szFromFile, szDestFile, false))
	{
		return true;
	}
	return false;
}
bool W_RenameFile(const char* szFromFile, const char* szDestFile)
{
	assert(szFromFile);
	assert(szDestFile);

	if (!W_IsFileExist(szFromFile))
	{
		return false;
	}
	if(W_IsFileExist(szDestFile))
	{
		return false;
	}

	long dwOrgAttr = GetFileAttributes(szFromFile);
	if (dwOrgAttr & _A_RDONLY)
	{
		SetFileAttributes(szFromFile, dwOrgAttr & (~_A_RDONLY));
	}

	rename(szFromFile, szDestFile);

	SetFileAttributes(szDestFile, dwOrgAttr);
	return true;
}

bool WOS_IsStreamSame(std::istream& file_data1, std::istream& file_data2)
{
	const int BUF_SIZE = 8192;
	char buffer1[BUF_SIZE];
	char buffer2[BUF_SIZE];

	int iSize1 = BUF_SIZE;
	int iSize2 = BUF_SIZE;

	for( ; iSize1 >= BUF_SIZE; )
	{
		file_data1.read(buffer1, BUF_SIZE);
		file_data2.read(buffer2, BUF_SIZE);

		iSize1 = file_data1.gcount();
		iSize2 = file_data2.gcount();

		if( iSize1 != iSize2 )
		{
			return false;
		}

		if( memcmp(buffer1, buffer2, BUF_SIZE) != 0 )
		{
			return false;
		}
	}

	return true;
}

bool W_IsFileSame(const char* szFile1, const char* szFile2)
{
	assert(szFile1);
	assert(szFile2);

	std::ifstream ifn1, ifn2;

	ifn1.open(szFile1);
	ifn2.open(szFile2);

	return WOS_IsStreamSame(ifn1, ifn2);
}

bool W_MoveFile(const char* szFromFile, const char* szDestFile)
{
	assert(szFromFile);
	assert(szDestFile);

	if( !W_CopyFile(szFromFile, szDestFile) )
	{
		return false;
	}

	return W_ForceDeleteFile(szFromFile);
}

// return true if all copied, return false if some not copy
bool W_CopyTree(const char* szFromFolder, const char* szDestFolder)
{
	bool bAllCopied = true;

	W_FileInFolder folder_info;
	W_FindFileInDirectory(folder_info, szFromFolder);

	CreateDirectory(szDestFolder, NULL);

	UINT i;
	for(i = 0; i< folder_info.m_filePickNames.size(); i++)
	{
		char szTempNameFrom[_MAX_PATH];
		char szTempNameDest[_MAX_PATH];
		sprintf(szTempNameFrom, "%s\\%s", szFromFolder, folder_info.m_filePickNames[i].c_str());
		sprintf(szTempNameDest, "%s\\%s", szDestFolder, folder_info.m_filePickNames[i].c_str());
		if( !CopyFile(szTempNameFrom, szTempNameDest, false) )
		{
			bAllCopied = false;
		}
	}
	for(i = 0; i< folder_info.m_subFolderPick.size(); i++)
	{
		char szTempNameFrom[_MAX_PATH];
		char szTempNameDest[_MAX_PATH];
		sprintf(szTempNameFrom, "%s\\%s", szFromFolder, folder_info.m_subFolderPick[i].c_str());
		sprintf(szTempNameDest, "%s\\%s", szDestFolder, folder_info.m_subFolderPick[i].c_str());
		if( !W_CopyTree(szTempNameFrom, szTempNameDest) )
		{
			bAllCopied = false;
		}
	}

	return bAllCopied;
}

// return true if all moved, return false if some not move
bool W_MoveTree(const char* szFromFolder, const char* szDestFolder)
{
	bool bAllMoved = true;

	W_FileInFolder folder_info;
	W_FindFileInDirectory(folder_info, szFromFolder);

	CreateDirectory(szDestFolder, NULL);

	UINT i;
	for(i = 0; i< folder_info.m_filePickNames.size(); i++)
	{
		char szTempNameFrom[_MAX_PATH];
		char szTempNameDest[_MAX_PATH];
		sprintf(szTempNameFrom, "%s\\%s", szFromFolder, folder_info.m_filePickNames[i].c_str());
		sprintf(szTempNameDest, "%s\\%s", szDestFolder, folder_info.m_filePickNames[i].c_str());
		if( !CopyFile(szTempNameFrom, szTempNameDest, false) )
		{
			bAllMoved = false;
			continue;
		}

		if( !W_ForceDeleteFile(szTempNameFrom) )
		{
			bAllMoved = false;
		}
	}
	for(i = 0; i< folder_info.m_subFolderPick.size(); i++)
	{
		char szTempNameFrom[_MAX_PATH];
		char szTempNameDest[_MAX_PATH];
		sprintf(szTempNameFrom, "%s\\%s", szFromFolder, folder_info.m_subFolderPick[i].c_str());
		sprintf(szTempNameDest, "%s\\%s", szDestFolder, folder_info.m_subFolderPick[i].c_str());
		if( !W_MoveTree(szTempNameFrom, szTempNameDest) )
		{
			bAllMoved = false;
		}
	}

	if( !RemoveDirectory( szFromFolder ) )
	{
		bAllMoved = false;
	}

	return bAllMoved;
}

// return true if all deleted, return false if some not delete
bool W_DelTree(const char* szFromFolder)
{
	assert(szFromFolder);

	bool bClear = true;

	char currentDir[_MAX_PATH];
	GetCurrentDirectory( _MAX_PATH, currentDir );

	if( !SetCurrentDirectory( szFromFolder ) )
		return bClear;

	_finddata_t findData;

	long fHandle = _findfirst( "*",  &findData );

	for( ; _findnext( fHandle, &findData ); )
	{
		if( ( findData.attrib & _A_SUBDIR  ) == _A_SUBDIR )
		{
			if( stricmp(findData.name, "..") != 0 && stricmp(findData.name, ".") != 0)
			{
				if( !W_DelTree( findData.name ) )
				{
					bClear = false;
				}
			}
		}
		else
		{
			if( !W_ForceDeleteFile( findData.name ) )
			{
				bClear = false;
			}
		}
	}

	_findclose( fHandle );

	SetCurrentDirectory("..");

	if( !RemoveDirectory( szFromFolder ) )
	{
		bClear = false;
	}

	SetCurrentDirectory( currentDir );

	return bClear;
}

bool W_GetAppPath(std::string & sAppPath)
{
	char sPath[MAX_PATH];

	GetModuleFileName(NULL,sPath,MAX_PATH);
	sAppPath = sPath;

	size_t n = sAppPath.rfind('/');
	if(n == -1)
		return false;

	sPath[n] = '\0';

	sAppPath = sPath;

	return true;
}

// --------------------------
CIniFile2::CIniFile2()
{

}

CIniFile2::CIniFile2(const char * filename)
{
	if (filename)
	{
		m_szIniFile = filename;
	}
}

CIniFile2::~CIniFile2()
{

}

bool CIniFile2::LoadFromFile()
{
	m_Items.clear();

	FILE * fp = ::fopen(m_szIniFile.c_str(), "rb");

	if (NULL == fp)
	{
		return false;
	}

	::fseek(fp, 0, SEEK_END);

	size_t size = ::ftell(fp);

	::fseek(fp, 0, SEEK_SET);

	char * ReadBuf = new char[size + 2];

	if (::fread(ReadBuf, sizeof(char), size, fp) != size)
	{
		::fclose(fp);
		delete [] ReadBuf;

		return false;
	}

	::fclose(fp);

	ReadBuf[size] = '\r';
	ReadBuf[size + 1] = '\n';

	std::vector<const char *> lines;

	lines.reserve(256);

	size_t count = 0;
	for (size_t i=0; i<size + 2; i++)
	{
		if ( (ReadBuf[i] == '\r') || (ReadBuf[i] == '\n') )
		{
			ReadBuf[i] = 0;
			count = 0;
		}
		else
		{
			if (count == 0)
			{
				lines.push_back(&ReadBuf[i]);
			}
			count++;
		}
	}

	char str[8192 + 2];
	char section_[8192 + 2] = {0};
	char name_[8192 + 2] = {0};

	for (size_t i=0; i < lines.size(); i++)
	{
		// 清除空格
		TrimString(lines[i], str, sizeof(str));
		if (strlen(str) <= 2 || strlen(str) > 8192)
		{
			continue;
		}

		// 1 看这行是否是有效行, 注释无效 / ;
		if ( (str[0] == '/') && (str[1] == '/') )
		{
			continue;
		}

		if (str[0] == ';')
		{
			continue;
		}

		// 看这行是否是section
		size_t nFind1 = -1;
		for (size_t m=0; m < strlen(str); m ++)
		{
			if (str[m] == '[')  //[1]
			{					//012
				nFind1 = m;
			}

			if (str[m] == ']')
			{
				if (nFind1 != -1 && m-nFind1 > 1)
				{
					// 找到一个section
					memcpy(section_, &str[nFind1+1], m - nFind1-1);
					section_[m - nFind1 - 1] = 0;
					continue;
				}
			}
		}

		// 提取内容 1 有效内容 2 无效内容
		const char * val = ::strchr(str, '=');

		if ( (NULL == val) || (val == str))
		{
			continue;
		}

		val ++;

		memcpy(name_, str, val-str-1);
		name_[val-str-1] = 0;

		AddValue(section_, name_, val);
	}

	delete [] ReadBuf;

	return true;
}

void  CIniFile2::AddValue(const char* section, const char* key, const char* val)
{
	char str[8192 + 2];
	ValueItem vi;

	TrimString(section, str, sizeof(str));
	vi.sec = str;

	TrimString(key, str, sizeof(str));
	vi.name = str;

	TrimString(val, str, sizeof(str));
	vi.value = str;

	m_Items.push_back(vi);
}

int CIniFile2::ReadInteger(const char * section, const char * key, int def) const
{
	if (!section || !key)
	{
		return def;
	}

	std::string szValueFound = "";

	for (size_t i=0; i < m_Items.size(); i++)
	{
		if (stricmp(m_Items[i].sec.c_str(), section) == 0
			&& stricmp(m_Items[i].name.c_str(), key) == 0)
		{
			szValueFound = m_Items[i].value;
			break;
		}
	}

	if (szValueFound.size() > 0)
	{
		return atoi(szValueFound.c_str());
	}

	return def;
}

float CIniFile2::ReadFloat(const char * section, const char * key, float def) const
{
	if (!section || !key)
	{
		return def;
	}

	std::string szValueFound = "";

	for (size_t i=0; i < m_Items.size(); i++)
	{
		if (stricmp(m_Items[i].sec.c_str(), section) == 0
			&& stricmp(m_Items[i].name.c_str(), key) == 0)
		{
			szValueFound = m_Items[i].value;
			break;
		}
	}

	if (szValueFound.size() > 0)
	{
		return (float)atof(szValueFound.c_str());
	}

	return def;
}

std::string CIniFile2::ReadString(const char * section,
								  const char * key, const char * def) const
{
	std::string szValueFound = def ? def : "";

	if (!section || !key)
	{
		return szValueFound;
	}

	for (size_t i=0; i < m_Items.size(); i++)
	{
		if (stricmp(m_Items[i].sec.c_str(), section) == 0
			&& stricmp(m_Items[i].name.c_str(), key) == 0)
		{
			szValueFound = m_Items[i].value;
			break;
		}
	}

	return szValueFound;
}

void CIniFile2::ReadSectionValues(const char* section, std::vector<ValueItem>& vec)
{
	vec.clear();

	for (size_t i=0; i < m_Items.size(); i++)
	{
		if (stricmp(m_Items[i].sec.c_str(), section) == 0)
		{
			vec.push_back(m_Items[i]);
		}
	}
}

// END OF FILE
