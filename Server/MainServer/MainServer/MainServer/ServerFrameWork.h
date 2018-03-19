#pragma once

//class CIocp;

class CServerFrameWork
{
private:
	SOCKET m_listensock;
	CIocpMgr m_hIocp;
public:
	void Initialize();
	void Accept_Process();

	void Work_Thread();

public:

	void error_display(const char *msg, int err_no);
	void error_display(const char *msg);
	

public:
	CServerFrameWork();
	~CServerFrameWork();

};

