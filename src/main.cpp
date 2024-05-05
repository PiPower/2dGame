#include "Rendering/window.h"
#include "Rendering/Renderer2D.h"
#include <chrono>
#include "Game/Entity.h"
#include "Game/Camera.h"
#include "Game/GameWorld.h"


using namespace std;
using namespace std::chrono;


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

#if defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Window window(1600, 900, L"test", L"Voxel world");
	Renderer2D renderer(window.GetWindowHWND());
	window.RegisterResizezable(&renderer, Renderer2D::Resize);
	GameWorld game(&renderer, { 5,5 });
	renderer.BindCameraBuffer(game.getCamera()->getWorldTransformAddress());


	TimePoint old = high_resolution_clock::now();
	RenderableResources* resources = game.getRenderableResources();

	while (window.ProcessMessages() == 0)
	{
		renderer.StartRecording();
		renderer.RenderGraphicalObjects(resources->constBufferTable.data(), 
			resources->vbViewTable.data(), resources->ibViewTable.data(), resources->size, INDEX_COUNT);
		renderer.StopRecording();

		game.GameLoop(&window, &renderer);
	}
}


