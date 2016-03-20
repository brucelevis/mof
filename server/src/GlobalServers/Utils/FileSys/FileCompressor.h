/*-----------------------------------------------------------------------------
	created:	2007/06/12  10:39
	file path:	e:\119vss\Toolkits\folderZipper\folderZipper\FileCompressor.h
	author:		Deeple
	purpose:	
	desc:		none
------------------------------------------------------------------------------*/
#pragma once

#pragma warning(disable:4786)

#include <vector>
#include <string>

// ----------------------------------------------------------------------------
struct FileItem
{
	std::string         m_szFileName;
	bool                m_bIsFolder;
	size_t              m_nBytes;
};
// ----------------------------------------------------------------------------
class FileCompressor
{
public:
	FileCompressor(void);
	~FileCompressor(void);
public:
	// 压缩一个文件
	bool CompressFolder(
		const char* szFolder, 
		const char* szZipfile = NULL);

	// 压缩一个目录
	bool CompressFile(
		const char* szFile, 
		const char* szZipfile = NULL);

	// 解压缩文件
	bool UnCompressFile(
		const char* szZipfile,
		const char* szToFolder);
};

void CopyFolder(const char* from, const char* to = 0);
bool RestoreFile(const char* szFilename = "GameOption.ini");
// -----------------------------end of file------------------------------------
