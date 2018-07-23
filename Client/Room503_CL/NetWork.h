#pragma once
#include "d3dApp.h"
#include "d3dUtil.h"
#include <DirectXMath.h>

#define BUF_SIZE	1024

using namespace DirectX;

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
	BYTE type;
	bool lever = false;
};

class NetWork
{
private:

	PlayerInfo mPlayer[eEnd];
	ItemInfo   mItem[3];

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

	bool getLever(int n) { return mItem[n].lever; }
	void setLever(bool value, int n) { mItem[n].lever = value; }
	int getFriendState() const { return mPlayer[eFriend].player_state; }
	void setFriendState(int n) { mPlayer[eFriend].player_state = n; }


	int getPlayerState() const { return mPlayer[ePlayer].player_state; }
	void setPlayerState(int n) { mPlayer[ePlayer].player_state = n; }

	void SetWorldPotision(eObj type, XMFLOAT4X4 world) { mPlayer[type].world = world; }

	DirectX::XMFLOAT3 GetItemPosition(int n) const { return mItem[n].pos; }
	void SetItemPosition(DirectX::XMFLOAT3 pos, int n) {
		mItem[n].pos = pos;
	};

	int getItemState(int n) const { return mItem[n].type; }
	void setItemState(BYTE type,int n) { mItem[n].type = type;}
	
public:
	void ReadPacket(SOCKET);
	void Initialize(HWND&);
	void ProcessPacket(char *);

	void ProcessKeyDown();

	void SendMsg(int, DirectX::XMFLOAT3, DirectX::XMFLOAT4X4);
	void SendItemState(int, int, DirectX::XMFLOAT3);
	void SendKeyDown(int);

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

