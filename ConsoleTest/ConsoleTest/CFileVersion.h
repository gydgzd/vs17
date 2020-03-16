#pragma once

#include <atlstr.h>


class CFileVersion
{
public:
	CFileVersion();
	~CFileVersion();

	void    Close();
	BOOL    Open(LPCTSTR lpszModuleName);
	CString QueryValue(LPCTSTR lpszValueName, DWORD dwLangCharset = 0);

	CString getFileDescription() { return QueryValue(_T("FileDescription")); };

protected:
	LPBYTE  m_lpVersionData;
	DWORD   m_dwLangCharset;

private:
	void printError_Win(char * msg);
};

