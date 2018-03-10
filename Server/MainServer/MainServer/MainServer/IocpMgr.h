#pragma once
class IocpMgr
{
private:
	HANDLE m_hIocp;
public:
	BOOL Initialize();


public:
	IocpMgr();
	~IocpMgr();
};

