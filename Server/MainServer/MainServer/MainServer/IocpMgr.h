#pragma once
class CIocpMgr
{
private:
	HANDLE m_hIocp;
public:
	BOOL Initialize();

	BOOL GQCS(LPDWORD, PULONG_PTR, LPOVERLAPPED*, int);
	BOOL PQCS(ULONG , WSAOVERLAPPED*);

public:
	CIocpMgr();
	~CIocpMgr();
};

