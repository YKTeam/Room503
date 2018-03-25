#pragma once

#define WM_SOCKET	WM_USER+1
#define MAX_USER	1000
#define MAIN_PORT	9000
#define SERVER_IP	127.0.0.1
#define MAX_BUF		4000

enum clients_state {
	e_Recv,
	e_Send,
};


#pragma pack (push,1)

typedef struct IoEx {
	WSAOVERLAPPED	over;
	WSABUF			m_wsabuf;
	UCHAR			m_Iobuf[MAX_BUF];

};


typedef struct USER {
	bool			connect;
	int				id;
	int				x;
	int				y;
	clients_state	e_Type;
	IoEx			m_IoEx
};

#pragma pack (pop)