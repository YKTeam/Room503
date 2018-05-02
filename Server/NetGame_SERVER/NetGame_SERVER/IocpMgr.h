#pragma once
class CIocpMgr
{
private:
	HANDLE m_hiocp;
public:	
	BOOL Initi();
	
	/////	GQCS
	BOOL GetQCompleteState(LPDWORD,PULONG_PTR,LPOVERLAPPED*, int*);
	/////	CICP
	BOOL CreateIO(SOCKET&,int);
	BOOL CreateIO();

	///// PQCS
	BOOL PostQCompleteState(ULONG pKey, IoContextEx*);


	void CloseHD();
public:
	CIocpMgr();
	~CIocpMgr();
};

