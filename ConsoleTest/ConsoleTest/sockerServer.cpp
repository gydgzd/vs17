#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <atlconv.h>      // for T2A ,USES_CONVERSION
#include <tchar.h>

#pragma comment(lib,"ws2_32.lib")
extern int socketServer();
void printError_Win(const char * msg);
DWORD WINAPI clientProc(LPARAM lparam)
{
	SOCKET sockClient = (SOCKET)lparam;
	char recv_buf[10] = "";
	char send_buf[1024] = "hi, welcome";
	while (TRUE)
	{
		memset(recv_buf, 0, sizeof(recv_buf));
		// ���տͻ��˵�һ������   
		int ret = recv(sockClient, recv_buf, sizeof(recv_buf), 0);
		//����Ƿ����ʧ��  
		if (SOCKET_ERROR == ret)
		{
			printf("socket recv failed, %d\n", GetLastError());
			closesocket(sockClient);
			return -1;
		}
		else if (ret > 0)
		{
			recv_buf[ret] = 0x00;
			printf("%s,PID: %d\n", recv_buf,GetCurrentThreadId());
		}
		// 0 ����ͻ��������Ͽ�����  
		else if (ret == 0)
		{
			printf("client close connection,PID: %d\n", GetCurrentThreadId());//GetCurrentProcessId
			closesocket(sockClient);
			return -1;
		}
		// ��������  
		ret = send(sockClient, send_buf, strlen(send_buf), 0);
		//����Ƿ���ʧ��  
		if (SOCKET_ERROR == ret)
		{
			printf("socket send failed\n");
			closesocket(sockClient);
			return -1;
		}
	}
	closesocket(sockClient);
	return 0;
}

int socketServer()
{
	//��ʼ��WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}
	//�����׽���
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !%s", GetLastError());
		return 0;
	}

	//��IP�Ͷ˿�
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8000);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !%s", GetLastError());
	}

	//��ʼ����
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("listen error !%s", GetLastError());
		return 0;
	}

	//ѭ����������
	SOCKET *sClient = new SOCKET();
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	memset(&remoteAddr, 0, nAddrlen);
	char revData[255];
	while (true)
	{
		shutdown(slisten, 2);
		printf("�ȴ�����...\n");
		*sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen); 
		if (*sClient == INVALID_SOCKET)
		{
			printError_Win("accept error !");
			continue;
		}
		TCHAR ip[64] = L"";
		printf("���ܵ�һ�����ӣ�%ls \r\n", InetNtop(AF_INET, &remoteAddr.sin_addr,  ip,64));
		//��������
		//�����߳�Ϊ�ͻ����������շ�  
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)clientProc, (LPVOID)*sClient, 0, 0);
	/*	int ret = recv(sClient, revData, 255, 0);
		if (ret > 0)
		{
			revData[ret] = 0x00;
			printf(revData);
		}

		//��������
		char * sendData = "��ã�TCP client\n";
		send(sClient, sendData, strlen(sendData), 0);
		closesocket(sClient);
		*/
	}

	closesocket(slisten);
	WSACleanup();
	return 0;
}

void printError_Win(const char * msg)
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

	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) && ((*p == '.') || (*p < 33)));

	// Display the message
	USES_CONVERSION;
	printf("ERROR: %s failed with error %d - %s", msg, eNum, T2A((LPCTSTR)lpMsgBuf));

	LocalFree(lpMsgBuf);
}