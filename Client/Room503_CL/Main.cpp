#include "Scene01.h"
#include "WaitScene.h"

const int gNumFrameResources = 3;

bool gIsCloseApp = false;
bool gIsChangeScene = false;
int gSceneIndex = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// 디버그 빌드에서는 실행시점 메모리 점검 기능을 켠다
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	//MyScene *scene01 = nullptr;// = new MyScene(hInstance);
	WaitScene *waitScene = new WaitScene(hInstance);
	while (1)
	{
		try
		{
			if (gSceneIndex == 0) {
				if (!waitScene->Initialize())
					return 0;
				waitScene->Run();
			}
			else if (gSceneIndex == 1) {
				if (!waitScene->Initialize())
					return 0;
				waitScene->Run();
			}

			if (gIsChangeScene == false)
				return 0;
			else continue;
			break;
		}
		catch (DxException& e)
		{
			MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
			return 0;
		}
	}
	delete waitScene;
}