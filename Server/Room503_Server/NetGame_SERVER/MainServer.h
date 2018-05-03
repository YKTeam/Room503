#pragma once

class CIocpMgr;

class CMainServer
{
private:
	float preTime = timeGetTime() *0.001f;
	SOCKET m_listenSock;
	CIocpMgr m_hiocp;

	Client m_tClient[MAX_NPC];

	vector<thread*> m_vThread;
	mutex t_lock;
public:
	void Initi();

	void Accept_Process();
	void Work_Process();
	void Packet_Process(int, UCHAR[]);
	

	void SendPositionPacket(int, int);
	void SendMovePacket(int, int);
	void SendPacket(int, void*);

	void error_display(char *msg, int err_no);

	eBulletDir SetDir(int num) { return(eBulletDir)num; }
public:
	CMainServer();
	~CMainServer();
};
