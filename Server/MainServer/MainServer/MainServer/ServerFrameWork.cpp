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
		m_clients[login_id].cur_packet = 0, m_clients[login_id].pre_packet = 0;
		//////////////////////////////////
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
		IoEx* IoContext;
		int err_no;

		// GetQueuedCompletionSatus
		BOOL ret = m_hIocp.GQCS(&dwSize, &cur_id,
			reinterpret_cast<LPOVERLAPPED*>(&IoContext), err_no);

		if (FALSE == ret) {
			if (64 == err_no)
				error_display("GQCS : ", WSAGetLastError());
		}

		if (0 == dwSize)
			continue;

		if (e_Recv == IoContext->m_eState) {
			unsigned char* buf = m_clients[cur_id].m_IoEx.m_Iobuf;
			unsigned cur_size = m_clients[cur_id].cur_packet;
			unsigned pre_size = m_clients[cur_id].pre_packet;

			while (dwSize < 0) {
				if (0 == cur_size) cur_size = buf[0];

				if (dwSize + pre_size >= cur_size) {
					UCHAR packet[MAX_PACKET];
					memcpy(packet, m_clients[cur_id].packet_buffer, pre_size);
					memcpy(packet + pre_size, buf, cur_size - pre_size);
					Packet_Process(static_cast<int>(cur_id), packet);
					dwSize -= cur_size - pre_size;
					buf += cur_size - pre_size;
					cur_size = 0, pre_size = 0;
				}
				else {
					memcpy(m_clients[cur_id].packet_buffer + pre_size, buf, dwSize);
					pre_size += dwSize;
					dwSize = 0;
				}
			}
			m_clients[cur_id].cur_packet = cur_size;
			m_clients[cur_id].pre_packet = pre_size;
			DWORD dwFlag = 0;
			WSARecv(m_clients[cur_id].client_socket, &m_clients[cur_id].m_IoEx.m_wsabuf, 1, NULL,
				&dwFlag, &m_clients[cur_id].m_IoEx.over, NULL);
		}
		else if (e_Send == IoContext->m_eState)
		{
			if (dwSize != IoContext->m_wsabuf.len) {
				cout << "Send Error in WorkProcess!\n";
				closesocket(m_clients[cur_id].client_socket);
				m_clients[cur_id].connect = false;
			}

			delete IoContext;
		}
		else
		{
			cout << "I dont know GQCS\n";
			while (1);
		}
	}
}

void CServerFrameWork::SendPositionPacket(int client, int object)
{
	sc_position_packet packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;
	packet.x = m_clients[client].x;
	packet.y = m_clients[client].y;

}

void CServerFrameWork::Packet_Process(int id, unsigned char packet[])
{
	switch (packet[1]) {
	case CS_UP:
		m_clients[id].y--;
		break;
	case CS_DOWN:
		m_clients[id].y++;
		break;
	case CS_LEFT:
		m_clients[id].x--;
		break;
	case CS_RIGHT:m_clients[id].x++;
		break;
	default:
		cout << "UnKnown Packet Packet_Process : " << id << endl;
		while (1);
	}

	SendMovePacket(id, id);

	for (int j = 0; j < MAX_USER; ++j) {
		if (j != id) {
			if (m_clients[j].connect) {
				SendMovePacket(j, id);
				SendMovePacket(id, j);
			}

		}
	}

}


// 포지션위치를 계속해서  move시켜줌
void CServerFrameWork::SendMovePacket(int client, int obj)
{
	sc_position_packet packet;
	packet.id = obj;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE;
	packet.x = m_clients[obj].x;
	packet.y = m_clients[obj].y;

	SendPacket(client, &packet);
}

void CServerFrameWork::SendPacket(int client, void* packet)
{
	int cur_size = reinterpret_cast<unsigned char*>(packet)[0];
	IoEx* Io = new IoEx;
	Io->m_eState = e_Send;
	memcpy(Io->m_Iobuf, packet, cur_size);
	ZeroMemory(&Io->over, sizeof(Io->over));
	Io->m_wsabuf.buf = reinterpret_cast<CHAR*> (Io->m_Iobuf);
	Io->m_wsabuf.len = cur_size;

	int ret = WSASend(m_clients[client].client_socket, &Io->m_wsabuf,
		1, NULL, 0, &Io->over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			error_display("Error in SendPacket : ", err_no);
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
	std::wcout << L"에러" << lpMsgBuf << std::endl;
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
