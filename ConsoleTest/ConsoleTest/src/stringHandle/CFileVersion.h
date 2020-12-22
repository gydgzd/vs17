#pragma once

#include <atlstr.h>
#include <string>

class CFileVersion
{
public:
	CFileVersion();
	~CFileVersion();

	void    Close();
	BOOL    Open(LPCTSTR lpszModuleName);
	std::string QueryValue(LPCTSTR lpszValueName, DWORD dwLangCharset = 0);

    std::string getFileDescription() { return QueryValue(_T("FileDescription")); };

protected:
	LPBYTE  m_lpVersionData;
	DWORD   m_dwLangCharset;

private:
	void printError_Win(char * msg);
};

