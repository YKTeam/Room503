
#define WM_SOCKET		WM_USER+1

#define SERVERPORT		4000
#define SERVERIP		"127.0.0.1"

#define BUFSIZE			1024
#define MAX_BUFSIZE		4000

#define MAX_PACKET		255

#define MAX_USER		4
#define MAX_NPC			1000


#define CS_UP		1
#define CS_DOWN		2
#define CS_LEFT		3
#define CS_RIGHT	4
#define CS_NONE		5

#define SC_POS		1
#define SC_MOVE		2
#define SC_REMOVE	3

#define NPC_START	100
#define NPC_END		125

#define NPC_START_XPOS	750
#define NPC_START_YPOS	800

#define NPC_END_XPOS	200

#define WindowXSize 200
#define WindowYSize 600

#define PLAYER_MOVE 20
#define NPC_MOVE	10

#define LEFT_POS	20
#define RIGHT_POS	40
#define UP_POS		16
#define DOWN_POS	32

#define FrameTime	165.f

#pragma pack (push,1)


struct cs_position_packet {
	BYTE size;
	BYTE type;
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4X4 world_pos;
};

//struct cs_position {
//	BYTE size;
//	BYTE type;
//	int id;
//	DirectX::XMFLOAT3 pos;
//};

struct sc_position_packet{
	BYTE size;
	BYTE type;
	WORD id;
	DirectX::XMFLOAT3 pos;
	BYTE anistate;
	DirectX::XMFLOAT4X4 world_pos;
};

struct sc_move_packet {
	BYTE size;
	BYTE type;
	WORD id;
	DirectX::XMFLOAT3 pos;
	BYTE anistate;
	DirectX::XMFLOAT4X4 world_pos;

};


#pragma pack (pop)