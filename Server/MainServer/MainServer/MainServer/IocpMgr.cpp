#include "stdafx.h"
#include "IocpMgr.h"


BOOL IocpMgr::Initialize()
{
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	return (m_hIocp != NULL);
}

IocpMgr::IocpMgr()
{
	
}


IocpMgr::~IocpMgr()
{
}
