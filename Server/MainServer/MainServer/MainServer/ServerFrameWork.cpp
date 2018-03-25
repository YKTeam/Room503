#include "stdafx.h"
#include "ServerFrameWork.h"


void CServerFrameWork::Initialize()
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


	// IO Initi
	m_hIocp.Initialize();


}

void CServerFrameWork::Accept_Process()
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

		int login_id = -1;
		for (int i = 0; i < MAX_USER; ++i) {
			if (!m_clients[i].connect)
				break;
		}

		if (-1 == login_id) {
			cout << "ALL CLIENTS LOIN" << endl;
			closesocket(login_client);
			continue;
		}

		m_clients[login_id].connect = true;
		m_clients[login_id].x = 0, m_clients[login_id].y = 0;
		m_clients[login_id].e_Type = e_Recv;
		m_clients[login_id].m_IoEx.m_wsabuf.buf
			= reinterpret_cast<CHAR*>(m_clients[login_id].m_IoEx.m_Iobuf);
		m_clients[login_id].m_IoEx.m_wsabuf.len
			= sizeof(m_clients[login_id].m_IoEx.m_Iobuf);

		DWORD flag = 0;
		// CreateIoCP
		m_hIocp.CreateIOCP(login_client, login_id);

		WSARecv(login_client, &(m_clients[login_id].m_IoEx.m_wsabuf), 1,
			NULL, &flag, &(m_clients[login_id].m_IoEx.over), NULL);

	}
}

void CServerFrameWork::Work_Thread()
{
	while (1)
	{
		DWORD dwSize;
		ULONGLONG cur_id;
		WSAOVERLAPPED over;
		int err_no;

		// GetQueuedCompletionSatus
		BOOL ret = m_hIocp.GQCS(&dwSize, &cur_id,
			reinterpret_cast<LPOVERLAPPED*>(&over), err_no);

		if (FALSE == ret) {
			if (64 == err_no)
				error_display("GQCS : ", WSAGetLastError());
		}

		if (0 == dwSize)
			continue;
	}
}

void CServerFrameWork::error_display(const char * msg, int err_no)
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

void CServerFrameWork::error_display(const char * msg)
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



CServerFrameWork::CServerFrameWork()
{
	m_listensock = NULL;
	m_clients = new USER{ MAX_USER };

	for (int i = 0; i < MAX_USER; ++i)
		m_clients[i].connect = false;
}


CServerFrameWork::~CServerFrameWork()
{
	closesocket(m_listensock);
	m_hIocp.~CIocpMgr();
	WSACleanup();

	delete[] m_clients;
}
