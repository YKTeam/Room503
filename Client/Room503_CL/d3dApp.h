
#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "d3dUtil.h"
#include "GameTimer.h"

// 12라이브러리를 링크함
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

enum class Scene : int
{
	Menu = 0,
	Scene01 = 1,
	Scene02 = 2,
	Scene03 = 3,
	Scene04 = 4,
	Scene05 = 5,
	Scene06 = 6,
	Scene07 = 7,
	Count
};

enum class RenderLayer : int
{
	Opaque = 0,
	Grid,
	Menu,
	MenuButton,
	BaseUI,
	MoveUI,
	Scene01_Map,
	Scene02_Map,
	Scene03_Map,
	Scene04_Map,
	Scene05_Map,
	Scene06_Map,
	Scene07_Map,
	Player,
	Friend,
	Shadow,
	Grass,
	SkyBox,
	Enemy,//적
	MoveTile,
	Item,
	Lever,  //무빙 레버
	MapCollision01, //정적인 맵 콜리젼박스 (노가다)
	MapCollision02,
	CollBox, //디버깅용
	Debug,
	Count
};

class D3DApp
{
protected:

    D3DApp(HINSTANCE hInstance);
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();   //D3DApp가 획득한 COM인터페이스들을 해제하고 명령 대기열을 비운다.

public:

    static D3DApp* GetApp();
    
	HINSTANCE AppInst()const; //응용 프로그램 인스턴스 핸들의 복사본을 돌려주는 자명한 접근 함 수
	HWND      MainWnd()const; //주 창 핸들의 복사본을 돌려주는 자명한 접근 함수.
	float     AspectRatio()const; // 후면 버퍼의 종횡비, 즉 높이에 대한 너비의 비율을 돌려준다.

    bool Get4xMsaaState()const; // 4x msaa가 활성화 되잇으면 true 반환
    void Set4xMsaaState(bool value);

	int Run();
 
    virtual bool Initialize(); // 이 메서드는 자원할당, 장면 물체 초기화, 광원설정등 응용 프로그램 고유 초기화 코드를 넣는다
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // 응용 프로그램 주 창의 메시지 처리부 (윈도우프로시저)를 넣는다

protected:
    virtual void CreateRtvAndDsvDescriptorHeaps(); // RTV서술자와 DSV 서술자를 응용 프로그램에 맞게 생성하는 데 쓰임
	virtual void OnResize();  // 응용 프로그램의 창이 WM_SIZE메시지 받을 때 호출 //다렉3d의 속성 갱신해야한다. (특히 후면 버퍼와 깊이-스텐실 버퍼를 다시 생성)
	                          // 후면 버퍼는 ResizeBuffers로 변경 가능, 깊이-스텐실 버퍼는 버퍼를 파괴하고 새 크기에 맞게 다시 생성해야 한다.
	                          // 렌더 대상 뷰와 깊이-스텐실 뷰도 다시 생성할 필요가 있다.
	virtual void Update(const GameTimer& gt)=0; //애니메이션 수행, 카메라이동, 충돌검출, 입력처리
    virtual void Draw(const GameTimer& gt)=0; //현재 프레임을 후면 버퍼에 실제로 그리기 위한 렌더링 명령들을 제출하는 작업을 수행
	                                          //프레임을 다 그린 후에는 IDXGISwapChain::Present메서드 호출해 후면버퍼를 화면에 제시
	//virtual void DrawMini(const GameTimer& gt) = 0;

	// 간편한 마우스 입력 처리를 위한 가상 함수들
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	bool InitMainWindow(); // 응용 프로그램의 주 창을 초기화한다.
	bool InitDirect3D(); // 다렉3d를 초기화한 다
	void CreateCommandObjects(); // 명령 대기열 하나와 명령 목록 할당자 하나, 명령 목록 하나를 생성
    void CreateSwapChain(); 

	void FlushCommandQueue(); //GPU가 명령 대기열에 있는 모든 명령의 처리를 마칠 때까지 CPU가 기다리게 만듬

	ID3D12Resource* CurrentBackBuffer()const; // 교환 사슬의 현재 후면 버퍼에 대한 ID3D12Resource를 돌려준다.
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const; // 현재 후면 버퍼에 대한 RTV를 돌려준다
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const; // 현재 후면 버퍼에 대한 DSV를 돌려준디

	void CalculateFrameStats(); //평균초당 프레임 수 와 평균 프레임당 밀리초를 계산한다

    void LogAdapters(); //시스템의 모든 디스플레이 어댑터를 열거
    void LogAdapterOutputs(IDXGIAdapter* adapter); // 주어진 어댑터와 연관된 모든 출력 열거
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format); // 주어진 출력과 픽셀 형식의 조합이 지원하는 모든 디스플레이 모드를 나열

protected:

    static D3DApp* mApp;

    HINSTANCE mhAppInst = nullptr; // 응용 프로그램 인스턴스 핸들
    HWND      mhMainWnd = nullptr; // 주 창 행들
	bool      mAppPaused = false;  // 응용 프로그램이 일시 정지된 상태인가?
	bool      mMinimized = false;  // 응용 프로그램이 최소화된 상태인가?
	bool      mMaximized = false;  // 응용 프로그램이 최대화된 상태인가?
	bool      mResizing = false;   // 사용자가 크기 조정용 테두리를 끌고 있는 상태인가?
    bool      mFullscreenState = false;// 전체화면 활성화 여부

	// 트루 설정시 4X MSAA (4.1.8).  기본은 false.
    bool      m4xMsaaState = false;    // 4X MSAA 활성화 여부
    UINT      m4xMsaaQuality = 0;      // 4X MSAA 품질 수준

	// 경과 시간과 게임 전체 시간을 측정하는 데 쓰임 (4.4).
	GameTimer mTimer;
	
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;
	
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandListMini;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

    D3D12_VIEWPORT mScreenViewport; 
    D3D12_RECT mScissorRect;

	D3D12_VIEWPORT mScreenViewportMini;
	D3D12_RECT mScissorRectMini;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	//파생 클래스는 자신의 생성자에서 이 멤버 변수들을
	//자신의 목적에 맞는 초기 값들로 설정해야 한다.
	std::wstring mMainWndCaption = L":D";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	int mClientWidth = 1200;
	int mClientHeight = 800;
};

