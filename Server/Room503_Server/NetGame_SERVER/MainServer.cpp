#include "stdafx.h"
#include "MainServer.h"


float preTime = timeGetTime() *0.001f;
float updateTime = timeGetTime()*0.001f;

void CMainServer::SendMovePacket2(int client, int object)
{

	sc_move_packet packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE;
	packet.pos = m_tClient[object].pos;
	packet.anistate = m_tClient[object].anistate;
	packet.world_pos = m_tClient[object].m_WorldPos;
	packet.player_state = m_tClient[object].m_PlayerState;

	////cout << "send ps - " << m_tClient[object].m_PlayerState << endl;
	//cout << packet.world_pos._11 << packet.world_pos._12
	//	<< packet.world_pos._13 << packet.world_pos._14 << endl;
	//cout << packet.world_pos._21 << packet.world_pos._22
	//	<< packet.world_pos._23 << packet.world_pos._24 << endl;
	//cout << packet.world_pos._31 << packet.world_pos._32
	//	<< packet.world_pos._33 << packet.world_pos._34 << endl;
	//cout << packet.world_pos._41 <<' '<< packet.world_pos._42 << ' '
	//	<< packet.world_pos._43 << ' '<< packet.world_pos._44 << endl;

	SendPacket(client, &packet);
	//cout << object << "가 " << client << " 에게 " << packet.pos.x << " " << packet.pos.y << " " << packet.pos.z << endl;
}

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

void CMainServer::Update_Thread()
{
	while (1) {
		if (timeGetTime() * 0.001f - updateTime > 0.05f) 
		{
			updateTime = timeGetTime() * 0.001f;
			for (int i = 0; i < MAX_USER; ++i) {
				if (m_tClient[i].m_bConnect) {
					IoContextEx * Io = new IoContextEx;
					Io->m_itarget = i;
					Io->m_eType = e_UPDATE;
					m_hiocp.PostQCompleteState(i, Io);
				}
			}
		}
	}
}

void CMainServer::Player_Process(int id)
{
	const float dt =0.05f;
	//cout << dt << endl;

	if (m_tClient[id].m_PlayerState == CS_LEFT) {
		DirectX::XMFLOAT3 look = Vector3::Normalize(Vector3::Add(XMFLOAT3(1, 0, 0), getLook3f(id)));
		SetLook3f(look, id);
		SetRight3f(Vector3::CrossProduct(GetUp3f(id), getLook3f(id), true), id);
		SetPosition(Vector3::Add(GetPosition(id), Vector3::ScalarProduct(getLook3f(id), -150 * dt, false)), id);
	}

	if (m_tClient[id].m_PlayerState == CS_RIGHT) {
		DirectX::XMFLOAT3 look = Vector3::Normalize(Vector3::Add(XMFLOAT3(-1, 0, 0), getLook3f(id)));
		SetLook3f(look, id);
		SetRight3f(Vector3::CrossProduct(GetUp3f(id), getLook3f(id), true), id);
		SetPosition(Vector3::Add(GetPosition(id), Vector3::ScalarProduct(getLook3f(id), -150 * dt, false)), id);
	}

	if (m_tClient[id].m_PlayerState == CS_UP) {
		DirectX::XMFLOAT3 look = Vector3::Normalize(Vector3::Add(XMFLOAT3(0, 0, -1), getLook3f(id)));
		SetLook3f(look, id);
		SetRight3f(Vector3::CrossProduct(GetUp3f(id), getLook3f(id), true), id);
		SetPosition(Vector3::Add(GetPosition(id), Vector3::ScalarProduct(getLook3f(id), -150 * dt, false)), id);
	}

	if (m_tClient[id].m_PlayerState == CS_DOWN) {
		DirectX::XMFLOAT3 look = Vector3::Normalize(Vector3::Add(XMFLOAT3(0, 0, 1), getLook3f(id)));
		SetLook3f(look, id);
		SetRight3f(Vector3::CrossProduct(GetUp3f(id), getLook3f(id), true), id);
		SetPosition(Vector3::Add(GetPosition(id), Vector3::ScalarProduct(getLook3f(id), -150 * dt, false)), id);
	}

	for (int i = 0; i < MAX_USER; ++i) {
		if (m_tClient[i].m_bConnect && i != id) {
			SendMovePacket2(i, id);
		

			//m_lock.unlock();
		}
	}

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
	m_time.Reset();
	std::cout << "Initi Complete()\n";
	m_time.Tick();
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
		m_tClient[log_id].pos.x = 0;
		m_tClient[log_id].pos.y = -50;
		m_tClient[log_id].pos.z = 0;
		m_tClient[log_id].anistate = 0;
		m_tClient[log_id].m_WorldPos = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f };
		m_tClient[log_id].m_PlayerState = CS_NONE;

		DWORD dwflag = 0;
		m_hiocp.CreateIO(log_client, log_id);
		WSARecv(log_client, &(m_tClient[log_id].m_IoEx.m_wsabuf), 1,
			NULL, &dwflag, &(m_tClient[log_id].m_IoEx.over), NULL);

		m_tClient[log_id].m_Timer = GetTickCount();

		cout << m_tClient[log_id].pos.x << ", " << m_tClient[log_id].pos.y << ", " << m_tClient[log_id].pos.z << endl;

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
		else if (e_UPDATE == IoEx->m_eType) {
			//cout << "ID : " << cur_id << endl;
			m_lock.lock();
			Player_Process(cur_id);
			m_lock.unlock();
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

	//	m_tClient[cur_id].anistate = 1;
	switch (packet[1]) {
	case CS_NONE:
		m_tClient[cur_id].m_PlayerState = CS_NONE;
		m_tClient[cur_id].anistate = 0;
		break;
	case CS_UP:
		m_tClient[cur_id].pos = *(DirectX::XMFLOAT3*)&packet[2];
		m_tClient[cur_id].anistate = 1;
		m_tClient[cur_id].m_PlayerState = CS_UP;
		break;
	case CS_DOWN:
		m_tClient[cur_id].pos = *(DirectX::XMFLOAT3*)&packet[2];
		m_tClient[cur_id].anistate = 1;
		m_tClient[cur_id].m_PlayerState = CS_DOWN;
		break;
	case CS_LEFT:
		m_tClient[cur_id].pos = *(DirectX::XMFLOAT3*)&packet[2];
		m_tClient[cur_id].anistate = 1;
		m_tClient[cur_id].m_PlayerState = CS_LEFT;
		break;
	case CS_RIGHT:
		m_tClient[cur_id].pos = *(DirectX::XMFLOAT3*)&packet[2];
		m_tClient[cur_id].anistate = 1;
		m_tClient[cur_id].m_PlayerState = CS_RIGHT;
		break;

	default:
		cout << "UnKnown Packet Type Cur_ID : " << cur_id << endl;
		while (true);
	}

	m_tClient[cur_id].m_WorldPos = *(DirectX::XMFLOAT4X4*)&packet[2 + sizeof(DirectX::XMFLOAT3)];

	/*cout << m_tClient[cur_id].m_WorldPos._11 << ' ' << m_tClient[cur_id].m_WorldPos._12
		<< m_tClient[cur_id].m_WorldPos._13 << m_tClient[cur_id].m_WorldPos._14 << endl;
	cout << m_tClient[cur_id].m_WorldPos._21 << m_tClient[cur_id].m_WorldPos._22
		<< m_tClient[cur_id].m_WorldPos._23 << m_tClient[cur_id].m_WorldPos._24 << endl;
	cout << m_tClient[cur_id].m_WorldPos._31 << m_tClient[cur_id].m_WorldPos._32
		<< m_tClient[cur_id].m_WorldPos._33 << m_tClient[cur_id].m_WorldPos._34 << endl;
	cout << m_tClient[cur_id].m_WorldPos._41 << m_tClient[cur_id].m_WorldPos._42
		<< m_tClient[cur_id].m_WorldPos._43 << m_tClient[cur_id].m_WorldPos._44 << endl;*/


	//if (timeGetTime() * 0.001f - preTime > 0.2f)
	{
		preTime = timeGetTime() * 0.001f;
		SendMovePacket(cur_id, cur_id);
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
}




void CMainServer::SendPositionPacket(int client, int object)
{
	sc_position_packet packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;
	packet.pos = m_tClient[object].pos;
	packet.anistate = m_tClient[object].anistate;
	packet.world_pos = m_tClient[object].m_WorldPos;
	SendPacket(client, &packet);

	//cout << object << "가 " << client << " 에게 " << packet.pos.x << " " << packet.pos.y << " " << packet.pos.z << endl;


}

void CMainServer::SendMovePacket(int client, int object)
{

	sc_move_packet packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE;
	packet.pos = m_tClient[object].pos;
	packet.anistate = m_tClient[object].anistate;
	packet.world_pos = m_tClient[object].m_WorldPos;
	packet.player_state = m_tClient[object].m_PlayerState;
	
	

	SendPacket(client, &packet);
	//cout << object << "가 " << client << " 에게 " << packet.pos.x << " " << packet.pos.y << " " << packet.pos.z << endl;
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
