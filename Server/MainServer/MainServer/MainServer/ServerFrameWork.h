#pragma once

//class CIocp;

class CServerFrameWork
{
private:
	SOCKET		m_listensock;
	CIocpMgr	m_hIocp;
	USER*		m_clients;
public:
	void Initialize();
	void Accept_Process();

	void Work_Thread();
	void SendPositionPacket(int, int);

	void Packet_Process(int, unsigned char[]);

	void SendMovePacket(int, int);
	void SendPacket(int, void*);

public:

	void error_display(const char *msg, int err_no);
	void error_display(const char *msg);
	

public:
	CServerFrameWork();
	~CServerFrameWork();

};

