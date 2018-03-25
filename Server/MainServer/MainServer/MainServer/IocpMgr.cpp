#include "stdafx.h"
#include "IocpMgr.h"


BOOL CIocpMgr::Initialize()
{
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	return (m_hIocp != NULL);
}

BOOL CIocpMgr::GQCS(LPDWORD pdwNumberOfByte, PULONG_PTR pulCompletionKey, LPOVERLAPPED *pOverlapped, int err_txt)
{
	BOOL ret = GetQueuedCompletionStatus(m_hIocp, pdwNumberOfByte,
		pulCompletionKey, pOverlapped, INFINITE);
	if (FALSE == ret && NULL != err_txt) {
		err_txt = WSAGetLastError();
		return FALSE;
	}
	return TRUE;
}

BOOL CIocpMgr::PQCS(ULONG ul_key, WSAOVERLAPPED *over)
{
	BOOL ret = PostQueuedCompletionStatus(m_hIocp, 1, ul_key, over);
	return 0;
}

BOOL CIocpMgr::CreateIOCP(SOCKET& socket, int id)
{
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket),
		m_hIocp, id, 0);
	return S_OK;
}

CIocpMgr::CIocpMgr()
{

}


CIocpMgr::~CIocpMgr()
{
	CloseHandle(m_hIocp);
}
