
// -------------------------------------------------------------------------

#include <wininet.h>

#pragma comment(lib, "wininet.lib")

typedef void (*PROGRESS_FUNC) (int nPercent);

inline BOOL GetLocFileName(const char* szURL, char* buffer, int nBufLen)
{
	if (!szURL || nBufLen == 0)
	{
		return FALSE;
	}

	char szHostName[128];
	char szUrlPath[256];

	URL_COMPONENTS crackedURL;

	ZeroMemory (& crackedURL, sizeof (URL_COMPONENTS));
	crackedURL.dwStructSize     = sizeof (URL_COMPONENTS);
	crackedURL.lpszHostName     = szHostName;
	crackedURL.dwHostNameLength = sizeof(szHostName);
	crackedURL.lpszUrlPath      = szUrlPath;
	crackedURL.dwUrlPathLength  = sizeof(szUrlPath);

	if (!InternetCrackUrl(szURL, strlen(szURL), 0, &crackedURL))
	{
		return FALSE;
	}

	if (nBufLen < strlen(szUrlPath) + 1)
	{
		return FALSE;
	}

	for (size_t i=0; i<strlen(szUrlPath) + 1; i++)
	{
		buffer[i] = szUrlPath[i];
		if (buffer[i] == '/' || buffer[i] == '\\')
		{
			buffer[i] = '@';
		}
	}

	return TRUE;
}

#define progress(n) if (func) func(n);\

inline BOOL OpenURL(const char* szURL, const char* szLocalFile, PROGRESS_FUNC func = NULL)
{
	if (!szURL || !szLocalFile)
		return FALSE;

	char szHostName[128];
	char szUrlPath[256];

	URL_COMPONENTS crackedURL;

	ZeroMemory (& crackedURL, sizeof (URL_COMPONENTS));
	crackedURL.dwStructSize     = sizeof (URL_COMPONENTS);
	crackedURL.lpszHostName     = szHostName;
	crackedURL.dwHostNameLength = sizeof(szHostName);
	crackedURL.lpszUrlPath      = szUrlPath;
	crackedURL.dwUrlPathLength  = sizeof(szUrlPath);

	if (!InternetCrackUrl(szURL, strlen(szURL), 0, &crackedURL))
	{
		return FALSE;
	}

	FILE* file = fopen(szLocalFile, "wb");
	if (!file)
	{
		return FALSE;
	}

	progress(0);

	HINTERNET hInt,hConn,hReq;
	TCHAR  szBuffer[1024];
	DWORD  dwLen;
	BOOL   bResult = false;

	hInt = InternetOpen(   
		"Microsoft Internet Explorer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	int nBytesGet = 0;
	if (hConn = InternetConnect(hInt,crackedURL.lpszHostName,
		crackedURL.nPort,NULL,NULL,INTERNET_SERVICE_HTTP,0,0))   
	{
		if (hReq = HttpOpenRequest(hConn, "GET", crackedURL.lpszUrlPath, NULL, "", NULL, 0, 0))   
		{   
			DWORD dwRetCode = 0;
			DWORD dwSizeOfRq = sizeof(DWORD);
			if (HttpSendRequest(hReq,NULL,0,NULL,0)
				&& HttpQueryInfo(hReq, HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, &dwRetCode, &dwSizeOfRq, NULL)
				&& dwRetCode < 400)
			{
				bResult = true;
			}

			DWORD dwContLen = 0;
			HttpQueryInfo(hReq, HTTP_QUERY_CONTENT_LENGTH, &dwContLen, &dwSizeOfRq, NULL);

			while (bResult)
			{
				bResult = InternetReadFile(hReq, szBuffer, sizeof(szBuffer), &dwLen);
				nBytesGet += dwLen;
				if (dwLen == 0)
				{

					break;
				}
				// 写数据入文件
				fwrite(szBuffer, 1, dwLen, file);

				progress((100 * nBytesGet) / (dwContLen+1));
			}

			InternetCloseHandle(hReq);
		}
	}

	fclose(file);

	if (!bResult)
	{
		::DeleteFile(szLocalFile);
	}

	InternetCloseHandle(hInt);

	progress(100);

	return  bResult;
}

