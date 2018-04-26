#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "GameObject.h"
#include "BlurFilter.h"
#include "SobelFilter.h"
#include "RenderTarget.h"
#include "ShadowMap.h"
#include <tchar.h>

//#include "Scene01.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")



class WaitScene : public D3DApp
{
public:
	WaitScene(HINSTANCE hInstance);
	WaitScene(const WaitScene& rhs) = delete;
	WaitScene& operator=(const WaitScene& rhs) = delete;
	~WaitScene();

	virtual bool Initialize()override;

private:
	virtual void CreateRtvAndDsvDescriptorHeaps()override;
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;
	//virtual void DrawMini(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
	void OnKeyboardInput(const GameTimer& gt);
};