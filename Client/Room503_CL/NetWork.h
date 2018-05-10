#pragma once
#include "d3dApp.h"
#include "d3dUtil.h"
#include <DirectXMath.h>

#define BUF_SIZE	1024

enum eObj {
	ePlayer, eFriend, eEnd
};

struct PlayerInfo {
	DirectX::XMFLOAT3 pos;
	BYTE ani_state;
	DirectX::XMFLOAT4X4 world;
	int				player_state;
};

struct ItemInfo
{
	DirectX::XMFLOAT3 pos;
};

class NetWork
{
private:

	PlayerInfo mPlayer[eEnd];
	ItemInfo   mItem;

private:
	SOCKET mMainSocket;
	WSABUF mRecvWsaBuf;
	WSABUF mSendWsaBuf;

	char mSendBuf[BUF_SIZE];
	char mRecvBuf[BUF_SIZE];

	DWORD	in_packet_size = 0;

	int		mSaved_PacketSize = 0;

	char	packet_buffer[BUF_SIZE];

	int		mMyid;
	bool	m_Start = false;

public:
	DirectX::XMFLOAT3 GetPlayerPosition() const
	{
		return mPlayer[ePlayer].pos;
	}


	DirectX::XMFLOAT3 GetFriendPosition() const;
	int getAniState(eObj type) const { return mPlayer[type].ani_state; }
	DirectX::XMFLOAT4X4 getWorldPos(eObj type) const { return mPlayer[type].world; }


	int getFriendState() const { return mPlayer[eFriend].player_state; }
	void setFriendState(int n) { mPlayer[eFriend].player_state = n; }


	int getPlayerState() const { return mPlayer[ePlayer].player_state; }
	void setPlayerState(int n) { mPlayer[ePlayer].player_state = n; }


	DirectX::XMFLOAT3 GetItemPosition() { return mItem.pos; }
	void SetItemPosition(DirectX::XMFLOAT3 pos) {
		mItem.pos = pos;
	};

public:
	void ReadPacket(SOCKET);
	void Initialize(HWND&);
	void ProcessPacket(char *);

	void ProcessKeyDown();

	void SendMsg(int, DirectX::XMFLOAT3, DirectX::XMFLOAT4X4);
	void SendItemState(int, DirectX::XMFLOAT3);
private:
	static NetWork* Instance;
public:
	static NetWork* getInstance()
	{
		if (nullptr == Instance)
			Instance = new NetWork();
		return Instance;
	}

	static void Release()
	{
		if (Instance)
			delete Instance;
		Instance = nullptr;
	}

private:
	NetWork();
public:
	~NetWork();
};

