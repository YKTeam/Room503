#include "stdafx.h"
#include "ServerFrameWork.h"


void ServerFrameWork::Initialize()
{

	// S B L A

	wcout.imbue(locale("korean"));
	WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa)) error_display("WSAStartup error()");


	m_listensock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_listensock == INVALID_SOCKET) error_display("socket()");

	SOCKADDR_IN server_sock;
	ZeroMemory(&server_sock, sizeof(SOCKADDR_IN));
	server_sock.sin_family = AF_INET;
	server_sock.sin_port = htons(MAIN_PORT);
	server_sock.sin_addr.s_addr = INADDR_ANY;

	int retval = bind(m_listensock, reinterpret_cast<sockaddr*>(&server_sock), sizeof(SOCKADDR_IN));

	if (retval == SOCKET_ERROR) error_display("bind()");

	retval = listen(m_listensock, SOMAXCONN);
	if (retval == SOCKET_ERROR) error_display("listen()");


}

void ServerFrameWork::Accept_Process()
{
	while (1)
	{
		SOCKADDR_IN Client_addr;
		ZeroMemory(&Client_addr, sizeof(SOCKADDR_IN));
		Client_addr.sin_family = AF_INET;
		Client_addr.sin_port = htons(MAIN_PORT);
		Client_addr.sin_addr.s_addr = INADDR_ANY;

		int addr_size = sizeof(Client_addr);
		SOCKET login_client = WSAAccept(m_listensock, reinterpret_cast<sockaddr*>(&login_client),
			&addr_size, NULL, NULL);
	}
}

void ServerFrameWork::error_display(const char * msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"¿¡·¯" << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	while (true);
}

void ServerFrameWork::error_display(const char * msg)
{

	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}



ServerFrameWork::ServerFrameWork()
{
}


ServerFrameWork::~ServerFrameWork()
{
}
