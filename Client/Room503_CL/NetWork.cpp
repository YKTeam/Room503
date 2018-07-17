#include "NetWork.h"

#include <iostream>

DirectX::XMFLOAT3 NetWork::GetFriendPosition() const
{
	return mPlayer[eFriend].pos;
}

void NetWork::ReadPacket(SOCKET socket)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(mMainSocket, &mRecvWsaBuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		//int err_code = WSAGetLastError();
		//printf("Recv Error [%d]\n", err_code);
		return;
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(mRecvBuf);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + mSaved_PacketSize >= in_packet_size) {
			memcpy(packet_buffer + mSaved_PacketSize, ptr, in_packet_size - mSaved_PacketSize);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - mSaved_PacketSize;
			iobyte -= in_packet_size - mSaved_PacketSize;
			in_packet_size = 0;
			mSaved_PacketSize = 0;
		}
		else {
			memcpy(packet_buffer + mSaved_PacketSize, ptr, iobyte);
			mSaved_PacketSize += iobyte;
			iobyte = 0;
		}
	}
}

void NetWork::Initialize(HWND& hwnd)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	mMainSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);


	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(4000);
	//ServerAddr.sin_addr.s_addr = inet_addr("192.168.35.227");
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//ServerAddr.sin_addr.s_addr = inet_addr("192.168.35.58");
	int ret = WSAConnect(mMainSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr),
		NULL, NULL, NULL, NULL);

	WSAAsyncSelect(mMainSocket, hwnd, WM_USER + 1, FD_CLOSE | FD_READ);

	mSendWsaBuf.buf = mSendBuf;
	mSendWsaBuf.len = BUF_SIZE;

	mRecvWsaBuf.buf = mRecvBuf;
	mRecvWsaBuf.len = BUF_SIZE;

}

void NetWork::ProcessPacket(char *ptr)
{

	switch (ptr[1])
	{
	case SC_POS:
	{
		sc_position_packet * my_packet = reinterpret_cast<sc_position_packet *>(ptr);
		if (!m_Start) {
			mMyid = my_packet->id;
			m_Start = true;
		}

		if (mMyid == my_packet->id) {
			mPlayer[ePlayer].pos = my_packet->pos;
			mPlayer[ePlayer].world = my_packet->world_pos;
			mPlayer[ePlayer].ani_state = my_packet->anistate;
		}
		else {
			mPlayer[eFriend].pos = my_packet->pos;
			mPlayer[eFriend].ani_state = my_packet->anistate;
			mPlayer[eFriend].world = my_packet->world_pos;
		}

		break;
	}
	case SC_MOVE: {

		sc_move_packet * my_packet = reinterpret_cast<sc_move_packet *>(ptr);
		if (mMyid == my_packet->id) {
			mPlayer[ePlayer].pos = my_packet->pos;
			mPlayer[ePlayer].world = my_packet->world_pos;
			mPlayer[ePlayer].ani_state = my_packet->anistate;
			mPlayer[ePlayer].player_state = my_packet->player_state;
		}
		else {
			mPlayer[eFriend].pos = my_packet->pos;
			mPlayer[eFriend].ani_state = my_packet->anistate;
			mPlayer[eFriend].world = my_packet->world_pos;
			mPlayer[eFriend].player_state = my_packet->player_state;
		}
		break;
	}
	case SC_ITEM_ON:
	case SC_ITEM_OFF:
		sc_item_packet * packet = reinterpret_cast<sc_item_packet*>(ptr);
		mItem.pos = packet->pos;
		mItem.type = packet->type;
		mItem.lever = packet->lever;

		if (mItem.pos.y > -150)
			setLever(true);
		break;
	}



}

void NetWork::ProcessKeyDown()
{

}

void NetWork::SendMsg(int value, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4X4 mWorld)
{

	cs_position_packet *packet = reinterpret_cast<cs_position_packet *>(mSendBuf);

	DWORD iobyte;
	packet->pos = pos;
	packet->type = value;
	packet->world_pos = mWorld;

	packet->size = sizeof(cs_position_packet);
	mSendWsaBuf.len = sizeof(cs_position_packet);

	int ret = WSASend(mMainSocket, &mSendWsaBuf, 1, &iobyte, 0, NULL, NULL);

	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void NetWork::SendItemState(int type,BYTE number, DirectX::XMFLOAT3 pos)
{
	cs_item_packet *packet = reinterpret_cast<cs_item_packet *>(mSendBuf);

	DWORD iobyte;
	packet->pos = pos;
	packet->type = type;
	packet->lever = mItem.lever;
	packet->number = number;

	packet->size = sizeof(cs_item_packet);
	mSendWsaBuf.len = sizeof(cs_item_packet);

	int ret = WSASend(mMainSocket, &mSendWsaBuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

void NetWork::SendKeyDown(int keydown)
{
	cs_keydown_packet* packet = reinterpret_cast<cs_keydown_packet*>(mSendBuf);
	DWORD iobyte;
	packet->type = keydown;
	packet->size = sizeof(cs_keydown_packet);
	mSendWsaBuf.len = sizeof(cs_keydown_packet);

	int ret = WSASend(mMainSocket, &mSendWsaBuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}
}

NetWork::NetWork()
{
	m_Start = false;

	for (int i = 0; i < eEnd; ++i) {
		mPlayer[i].pos.x = 0;
		mPlayer[i].pos.y = -50;
		mPlayer[i].pos.z = 0;
	}

	mItem.pos = { 0,0,0 };

}


NetWork::~NetWork()
{
}
