
#include "stdafx.h"
#include "MainServer.h"



////  B L A

void CMainServer::SendPacket(int client, void* packet)
{
	int cur_size = reinterpret_cast<unsigned char*>(packet)[0];

	IoContextEx* IoEx = new IoContextEx;
	IoEx->m_eType = e_SEND;
	memcpy(IoEx->m_szIoBuf, packet, cur_size);
	ZeroMemory(&IoEx->over, sizeof(IoEx->over));
	IoEx->m_wsabuf.buf = reinterpret_cast<CHAR*> (IoEx->m_szIoBuf);
	IoEx->m_wsabuf.len = cur_size;

	int ret = WSASend(m_tClient[client].m_ClientSocket, &IoEx->m_wsabuf, 1, NULL, 0,
		&IoEx->over, NULL);

	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			error_display("Error in SendPacket : ", err_no);
	}
	//std::cout << "Send Packet [" << output_type << "] To Client : " << client << std::endl;
	//cout << "Send Packet : " << output_type << ", To Client : " << client << endl;


}

void CMainServer::Initi()
{
	wcout.imbue(locale("korean"));

	WSADATA wsadata;
	SOCKADDR_IN server_sockaddr;
	WSAStartup(MAKEWORD(2, 2), &wsadata);


	m_listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ZeroMemory(&server_sockaddr, sizeof(SOCKADDR_IN));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(SERVERPORT);
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;

	::bind(m_listenSock, reinterpret_cast<sockaddr*>(&server_sockaddr), sizeof(SOCKADDR_IN));
	listen(m_listenSock, 5);

	////IO INITI()
	m_hiocp.Initi();

	for (int i = 0; i < MAX_USER; ++i)
		m_tClient[i].m_bConnect = false;
	NPC_Initialize();
	std::cout << "Initi Complete()\n";
}

void CMainServer::NPC_Initialize()
{
	for (int i = NPC_START; i < NPC_END; ++i)
	{
		m_tClient[i].m_bConnect = false;
		m_tClient[i].m_eDir = SetDir(rand() % DIR_END);
		Sleep(10);
		NPC_SetPos(i);
	}
}

void CMainServer::NPC_SetPos(int npc_id)
{
	//m_tClient[npc_id].m_ix = (RIGHT_POS + LEFT_POS) / 2;
	//m_tClient[npc_id].m_iy = (UP_POS + DOWN_POS) / 2;
	switch (m_tClient[npc_id].m_eDir)
	{
	case DIR_LEFT:
		m_tClient[npc_id].m_ix = 80 + rand() % 10;
		m_tClient[npc_id].m_iy = 20 + rand() % 20;
		break;
	case DIR_RIGHT:
		m_tClient[npc_id].m_ix = -10 + rand() % 10;
		m_tClient[npc_id].m_iy = 20 + rand() % 20;
		break;
	case DIR_DOWN:
		m_tClient[npc_id].m_ix = 25 + rand() % 15;
		m_tClient[npc_id].m_iy = -20 + rand() % 15;
		break;
	case DIR_UP:
		m_tClient[npc_id].m_ix = 25 + rand() % 15;
		m_tClient[npc_id].m_iy = 45 + rand() % 15;
		break;
	}
}

void CMainServer::NPC_Move(int id)
{
	switch (m_tClient[id].m_eDir)
	{
	case DIR_LEFT:
		if (m_tClient[id].m_ix > 0)
			m_tClient[id].m_ix -= 1;
		else
			NPC_SetPos(id);
		break;
	case DIR_RIGHT:
		if (m_tClient[id].m_ix < 60)
			m_tClient[id].m_ix += 1;
		else
			NPC_SetPos(id);
		break;
	case DIR_UP:
		if (m_tClient[id].m_iy > 0)
			m_tClient[id].m_iy -= 1;
		else
			NPC_SetPos(id);
		break;
	case DIR_DOWN:
		if (m_tClient[id].m_iy < 45)
			m_tClient[id].m_iy += 1;
		else
			NPC_SetPos(id);
		break;
	}


	for (int i = 0; i < MAX_USER; ++i)
	{
		if (m_tClient[i].m_bConnect)
		{
			m_tClient[i].m_lock.lock();
			SendPositionPacket(i, id);
			//SendMovePacket(i, id);
			m_tClient[i].m_lock.unlock();
		}
	}

}


void CMainServer::Accept_Process()
{
	while (1)
	{
		SOCKADDR_IN Client_Addr;
		ZeroMemory(&Client_Addr, sizeof(SOCKADDR_IN));
		Client_Addr.sin_family = AF_INET;
		Client_Addr.sin_port = htons(SERVERPORT);
		Client_Addr.sin_addr.s_addr = INADDR_ANY;

		int addr_size = sizeof(Client_Addr);
		SOCKET log_client = WSAAccept(m_listenSock, reinterpret_cast<sockaddr*>(&log_client),
			&addr_size, NULL, NULL);

		////확인
		int log_id = -1;
		for (int i = 0; i < MAX_USER; ++i)
			if (false == m_tClient[i].m_bConnect) {
				log_id = i;
				break;
			}

		if (-1 == log_id) {
			cout << "ALL CLIENTS LOGIN ! \n";
			closesocket(log_client);
			continue;
		}

		m_tClient[log_id].m_bConnect = true;
		m_tClient[log_id].m_ClientSocket = log_client;
		m_tClient[log_id].m_iCursize = 0;
		m_tClient[log_id].m_iPredata = 0;
		ZeroMemory(&m_tClient[log_id].m_IoEx, sizeof(m_tClient[log_id].m_IoEx));
		m_tClient[log_id].m_IoEx.m_eType = e_RECV;
		m_tClient[log_id].m_IoEx.m_wsabuf.buf
			= reinterpret_cast<CHAR*> (m_tClient[log_id].m_IoEx.m_szIoBuf);
		m_tClient[log_id].m_IoEx.m_wsabuf.len
			= sizeof(m_tClient[log_id].m_IoEx.m_szIoBuf);
		m_tClient[log_id].m_ix = 0;
		m_tClient[log_id].m_iy = -50;
		m_tClient[log_id].m_iz = 0;

		DWORD dwflag = 0;
		m_hiocp.CreateIO(log_client, log_id);
		WSARecv(log_client, &(m_tClient[log_id].m_IoEx.m_wsabuf), 1,
			NULL, &dwflag, &(m_tClient[log_id].m_IoEx.over), NULL);

		m_tClient[log_id].m_Timer = GetTickCount();

		cout << m_tClient[log_id].m_ix << ", " << m_tClient[log_id].m_iy << ", " << m_tClient[log_id].m_iz << endl;

		SendPositionPacket(log_id, log_id);

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (m_tClient[i].m_bConnect) {
				if (i != log_id)
				{
					SendPositionPacket(log_id, i);
					SendPositionPacket(i, log_id);
				}
			}
		}
	}
}

void CMainServer::Work_Process()
{
	while (1)
	{
		DWORD dwSize;
		ULONGLONG cur_id;
		IoContextEx* IoEx;
		int err_no;
		BOOL ret = m_hiocp.GetQCompleteState(&dwSize, &cur_id,
			reinterpret_cast<LPWSAOVERLAPPED*> (&IoEx), &err_no);

		//cout << endl << "GQCS KEY : " << cur_id << endl;

		if (FALSE == ret) {
			if (64 == err_no)
				//DisconnectClient(cur_id);
				error_display("GQCS : ", WSAGetLastError());
		}

		if (0 == dwSize) {
			//DisconnectClient(cur_id);
			continue;
		}

		if (e_RECV == IoEx->m_eType)
		{
			//cout << "RECV frome Client : " << cur_id;
			//cout << " DATA_SIZE : " << dwSize << endl;
			unsigned char* buf = m_tClient[cur_id].m_IoEx.m_szIoBuf;
			unsigned cur_size = m_tClient[cur_id].m_iCursize;
			unsigned pre_size = m_tClient[cur_id].m_iPredata;
			while (dwSize > 0)
			{
				if (0 == cur_size) cur_size = buf[0];
				if (dwSize + pre_size >= cur_size) {
					UCHAR packet[MAX_PACKET];
					memcpy(packet, m_tClient[cur_id].m_szPacket_buf, pre_size);
					memcpy(packet + pre_size, buf, cur_size - pre_size);
					Packet_Process(static_cast<int>(cur_id), packet);
					dwSize -= cur_size - pre_size;
					buf += cur_size - pre_size;
					cur_size = 0, pre_size = 0;
				}
				else
				{
					memcpy(m_tClient[cur_id].m_szPacket_buf + pre_size, buf, dwSize);
					pre_size += dwSize;
					dwSize = 0;
				}
			}
			m_tClient[cur_id].m_iCursize = cur_size;
			m_tClient[cur_id].m_iPredata = pre_size;
			DWORD dwflag = 0;
			WSARecv(m_tClient[cur_id].m_ClientSocket,
				&m_tClient[cur_id].m_IoEx.m_wsabuf, 1, NULL,
				&dwflag, &m_tClient[cur_id].m_IoEx.over, NULL);
		}
		else if (e_SEND == IoEx->m_eType)
		{
			if (dwSize != IoEx->m_wsabuf.len) {
				cout << " Send Error in WorkProcess!\n";
				closesocket(m_tClient[cur_id].m_ClientSocket);
				m_tClient[cur_id].m_bConnect = false;
			}
			delete IoEx;
		}
		else if (e_AI == IoEx->m_eType)
		{
			NPC_Move(cur_id);
			//		cout << "NPC ID : " << cur_id << endl;
			delete IoEx;
		}
		else
		{
			cout << "Unknown GQCS\n";
			while (true);
		}


	}
}
//// 200 200 600 600
void CMainServer::Packet_Process(int cur_id, UCHAR packet[])
{
	//cout << "ID [" << cur_id << "] Keyboard Input" << endl;

	if (GetTickCount() - m_tClient[cur_id].m_Timer < FrameTime)
		return;
	switch (packet[1]) {
	case CS_UP:
		//if (m_tClient[cur_id].m_iy > UP_POS)
		//	m_tClient[cur_id].m_iy--;
		break;
	case CS_DOWN:
		//if (m_tClient[cur_id].m_iy < DOWN_POS)
		//	m_tClient[cur_id].m_iy++;
		break;
	case CS_LEFT:
		//if (m_tClient[cur_id].m_ix > LEFT_POS)
		//	m_tClient[cur_id].m_ix--;
		break;
	case CS_RIGHT:
		//if (m_tClient[cur_id].m_ix < RIGHT_POS)
		//	m_tClient[cur_id].m_ix++;
		break;
	default:
		cout << "UnKnown Packet Type Cur_ID : " << cur_id << endl;
		while (true);
	}
	m_tClient[cur_id].m_ix = *(float*)&packet[2];
	m_tClient[cur_id].m_iy = *(float*)&packet[2+sizeof(float)];
	m_tClient[cur_id].m_iz = *(float*)&packet[2 + 2*sizeof(float)];

	SendMovePacket(cur_id, cur_id);
	cout << m_tClient[cur_id].m_ix << ", " << m_tClient[cur_id].m_iy << ", " << m_tClient[cur_id].m_iz << endl;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (i != cur_id)
			if (m_tClient[i].m_bConnect) {
				SendMovePacket(i, cur_id);
				SendMovePacket(cur_id, i);
			}
	}
	m_tClient[cur_id].m_Timer = GetTickCount();
}

void CMainServer::Timer_Process()
{
	while (true)
	{
		Sleep(10);
		while (true)
		{
		}
	}
}

void CMainServer::NPC_Process()
{
	while (true)
	{
		auto preTick = chrono::high_resolution_clock::now();
		for (int i = NPC_START; i < NPC_END; ++i)
			for (int j = 0; j < MAX_USER; ++j)
			{
				if (m_tClient[j].m_bConnect)
				{
					IoContextEx* IoEx = new IoContextEx;
					IoEx->m_eType = e_AI;
					m_hiocp.PostQCompleteState(i, &*IoEx);
					break;
				}
			}
		auto lateTick = chrono::high_resolution_clock::now() - preTick;
		int s_time = chrono::duration_cast<chrono::microseconds>(lateTick).count();
		if (FrameTime > s_time) Sleep(FrameTime - s_time);

	}

}



void CMainServer::SendPositionPacket(int client, int object)
{
	sc_position_packet packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;
	packet.x = m_tClient[object].m_ix;
	packet.y = m_tClient[object].m_iy;
	packet.z = m_tClient[object].m_iz;

	SendPacket(client, &packet);
	//cout << object << "가 " << client << " 에게 " << packet.x << " " << packet.y << endl;

}

void CMainServer::SendMovePacket(int client, int object)
{
	sc_move_packet packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE;
	packet.x = m_tClient[object].m_ix;
	packet.y = m_tClient[object].m_iy;
	packet.z = m_tClient[object].m_iz;

	SendPacket(client, &packet);
	//cout << client << " 에게 " << packet.x << " " << packet.y << endl;
}

void CMainServer::error_display(char * msg, int err_no)
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

CMainServer::CMainServer()
{
	m_listenSock = NULL;
	srand(unsigned(time));
}


CMainServer::~CMainServer()
{
	closesocket(m_listenSock);
	m_hiocp.CloseHD();
	WSACleanup();
}
