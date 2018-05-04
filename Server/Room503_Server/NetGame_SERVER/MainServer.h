#pragma once
#include <DirectXMath.h>
#include "XMHelper12.h"
#include "GameTimer.h"

class CIocpMgr;
class GameTimer;

class CMainServer
{
private:
	float preTime = timeGetTime() *0.001f;
	SOCKET m_listenSock;
	CIocpMgr m_hiocp;

	Client m_tClient[MAX_NPC];

	vector<thread*> m_vThread;
	mutex m_lock;
public:

	GameTimer m_time;
	
	void Initi();

	void Accept_Process();
	void Work_Process();
	void Packet_Process(int, UCHAR[]);
	void SendPositionPacket(int, int);
	void SendMovePacket(int, int);

	void SendMovePacket2(int, int);
	void SendPacket(int, void*);


	void Update_Thread();
	void Player_Process(int);
	void error_display(char *msg, int err_no);

	eBulletDir SetDir(int num) { return(eBulletDir)num; }

public:
	DirectX::XMFLOAT3 getLook3f(int id) {
		return (DirectX::XMFLOAT3(m_tClient[id].m_WorldPos._31,
			m_tClient[id].m_WorldPos._32,
			m_tClient[id].m_WorldPos._33));
	}
	void SetLook3f(DirectX::XMFLOAT3 f, int id) {
		m_tClient[id].m_WorldPos._31 = f.x;
		m_tClient[id].m_WorldPos._32 = f.y;
		m_tClient[id].m_WorldPos._33 = f.z;
	};

	void SetRight3f(DirectX::XMFLOAT3 f, int id)
	{
		m_tClient[id].m_WorldPos._11 = f.x;
		m_tClient[id].m_WorldPos._12 = f.y;
		m_tClient[id].m_WorldPos._13 = f.z;
	};
	void SetPosition(DirectX::XMFLOAT3 vec3, int id)
	{
		m_tClient[id].m_WorldPos._41 = vec3.x;
		m_tClient[id].m_WorldPos._42 = vec3.y;
		m_tClient[id].m_WorldPos._43 = vec3.z;
	}

	DirectX::XMFLOAT3 GetUp3f(int id)
	{
		return (DirectX::XMFLOAT3(m_tClient[id].m_WorldPos._21, m_tClient[id].m_WorldPos._22, m_tClient[id].m_WorldPos._23));
	}


	DirectX::XMFLOAT3 GetPosition(int id)
	{
		return (DirectX::XMFLOAT3(m_tClient[id].m_WorldPos._41, m_tClient[id].m_WorldPos._42, m_tClient[id].m_WorldPos._43));
	}

public:
	CMainServer();
	~CMainServer();
};

