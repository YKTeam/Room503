#pragma once
class CIocpMgr
{
private:
	HANDLE m_hIocp;
public:
	BOOL Initialize();

	//////
	BOOL GQCS(LPDWORD, PULONG_PTR, LPOVERLAPPED*, int);
	BOOL PQCS(ULONG , WSAOVERLAPPED*);

	//////
	BOOL CreateIOCP(SOCKET&, int);

public:
	CIocpMgr();
	~CIocpMgr();
};

