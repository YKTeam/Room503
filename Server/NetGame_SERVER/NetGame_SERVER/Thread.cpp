#include "stdafx.h"
#include "Thread.h"


void CThread::Aceqept_Process()
{
	while (1)
	{
		SOCKADDR_IN Client_sockAddr;
		ZeroMemory(&Client_sockAddr, sizeof(SOCKADDR_IN));
		Client_sockAddr.sin_family = AF_INET;
		Client_sockAddr.sin_port = htons(SERVERPORT);
		Client_sockAddr.sin_addr.s_addr = INADDR_ANY;

		int addr_size = sizeof(Client_sockAddr);
		SOCKET Accept_Client = WSAAccept()


	}

}

CThread::CThread()
{
}


CThread::~CThread()
{
}
