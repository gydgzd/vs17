#include "stdafx.h"
#include "CFileVersion.h"
#include "winver.h"
#include <assert.h>
#include <iostream>
#pragma comment(lib, "version")  

CFileVersion::CFileVersion()
{
	m_lpVersionData = NULL;
	m_dwLangCharset = 0;
}


CFileVersion::~CFileVersion()
{

}

BOOL CFileVersion::Open(LPCTSTR lpszModuleName)
{
	if (_tcslen(lpszModuleName) <= 0)
		std::cout << "the length of lpszModuleName is 0" << std::endl;

	if (m_lpVersionData == NULL)
	{
		std::cout << "m_lpVersionData is NULL" << std::endl;
	}
	// Get the version information size for allocate the buffer  
	DWORD dwHandle;
	DWORD dwDataSize = ::GetFileVersionInfoSize((LPTSTR)lpszModuleName, &dwHandle);
	if (dwDataSize == 0)
	{
		printError_Win("GetFileVersionInfoSize");
		return FALSE;
	}
	// Allocate buffer and retrieve version information  
	m_lpVersionData = new BYTE[dwDataSize];
	if (!::GetFileVersionInfo((LPTSTR)lpszModuleName, dwHandle, dwDataSize, (void**)m_lpVersionData))
	{
		printError_Win("GetFileVersionInfo");
		Close();
		return FALSE;
	}
	// Retrieve the first language and character-set identifier  
	UINT nQuerySize;
	DWORD* pTransTable;
	if (!::VerQueryValue(m_lpVersionData, TEXT("\\VarFileInfo\\Translation"), (LPVOID *)&pTransTable, &nQuerySize))  // use "\\VarFileInfo\\Translation" will be an error
	{
		printError_Win("VerQueryValue");
		Close();
		return FALSE;
	}
	// Swap the words to have lang-charset in the correct format  
	m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));
	return TRUE;
}

void CFileVersion::Close()
{
	delete[] m_lpVersionData;
	m_lpVersionData = NULL;
	m_dwLangCharset = 0;
}

CString CFileVersion::QueryValue(LPCTSTR lpszValueName, DWORD dwLangCharset)
{
	// Must call Open() first  
	if (m_lpVersionData == NULL)
	{
		std::cout << "m_lpVersionData is NULL" << std::endl;
	}
	if (m_lpVersionData == NULL)
		return (CString)_T("");
	// If no lang-charset specified use default  
	if (dwLangCharset == 0)
		dwLangCharset = m_dwLangCharset;
	// Query version information value  
	UINT nQuerySize;
	LPVOID lpData;
	CString strValue, strBlockName;
	strBlockName.Format(_T("\\StringFileInfo\\%08lx\\%s"), dwLangCharset, lpszValueName);
	if (::VerQueryValue((void **)m_lpVersionData, strBlockName.GetBuffer(0), &lpData, &nQuerySize))
		strValue = (LPCTSTR)lpData;
	else
		printError_Win("VerQueryValue");
	strBlockName.ReleaseBuffer();
	return strValue;
}

 
void CFileVersion::printError_Win(char * msg)
{
	DWORD eNum;
	TCHAR sysMsg[256] = _T("");
	TCHAR* p;
	LPVOID lpMsgBuf;
	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		0, //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf, 0, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;

	while ((*p > 31) || (*p == 9))       // 9(0x09) - 水平制表符   31(0x1f) - 单元分隔符
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

	// Display the message
	USES_CONVERSION;
	printf("ERROR: %s failed with error %d - %s", msg, eNum, T2A((LPCTSTR)lpMsgBuf));
	// LOG(ERROR) << msg << " failed with error " << eNum << " - " << CDevInfo::AnsiToUtf8((LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}