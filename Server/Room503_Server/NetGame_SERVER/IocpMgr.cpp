#include "stdafx.h"
#include "IocpMgr.h"


BOOL CIocpMgr::Initi()
{
	m_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	return (m_hiocp != NULL);
}

BOOL CIocpMgr::GetQCompleteState(LPDWORD pdwNumberOfByte, PULONG_PTR pulCompletionKey, LPOVERLAPPED* pOverlapped, int* error_txt)
{
	BOOL ret = GetQueuedCompletionStatus(m_hiocp, pdwNumberOfByte,
		pulCompletionKey, pOverlapped, INFINITE);
	if (FALSE == ret && NULL != error_txt) {
		*error_txt = WSAGetLastError();
		return FALSE;
	}

	return ret;
}

BOOL CIocpMgr::CreateIO(SOCKET& socket, int id)
{
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket),
		m_hiocp, id, 0);
	return S_OK;
}

BOOL CIocpMgr::CreateIO()
{
	CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	return S_OK;
}

BOOL CIocpMgr::PostQCompleteState(ULONG pKey,IoContextEx* ICE)
{
	BOOL ret=PostQueuedCompletionStatus(m_hiocp,1,pKey,&ICE->over);
	return ret;
}

void CIocpMgr::CloseHD()
{
	CloseHandle(m_hiocp);
}

CIocpMgr::CIocpMgr()
{
}


CIocpMgr::~CIocpMgr()
{

}
