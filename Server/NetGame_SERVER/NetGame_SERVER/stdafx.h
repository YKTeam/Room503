// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
#pragma once
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <Windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <iostream>
#include <crtdbg.h>

#include "targetver.h"

#include <stdlib.h>

#include <winsock.h>

#include <tchar.h>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

using namespace std;

#include "protocol.h"
#include "enum.h"
#include "struct.h"
#include "IocpMgr.h"
#include "MainServer.h"


// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
