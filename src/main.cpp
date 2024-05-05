#include "utils.h"
#include "GameWorld.h"
#include <random>
using namespace std;
using namespace std::chrono;


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

#if defined(_DEBUG)
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Window window(1600, 900, L"test", L"Voxel world");
	Renderer2D renderer(window.GetWindowHWND());
	window.RegisterResizezable(&renderer, Renderer2D::Resize);
	GameWorld game(&renderer, { 2,2 });
	renderer.BindCameraBuffer(game.getCamera()->getWorldTransformAddress());


	TimePoint old = high_resolution_clock::now();
	
	while (window.ProcessMessages() == 0)
	{
		renderer.StartRecording();
		renderer.RenderGraphicalObjects(game.getWorldEntities()->data(), game.getWorldEntities()->size());
		renderer.StopRecording();

		game.GameLoop(&window, &renderer);
	}
}


