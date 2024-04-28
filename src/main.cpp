#include "Rendering/window.h"
#include "Rendering/Renderer2D.h"
#include "GraphicalObject.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

#if defined(_DEBUG)
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Window window(1600, 900, L"test", L"Voxel world");
	Renderer2D renderer(window.GetWindowHWND());
	GraphicalObject obj(&renderer, { 0, 0 }, {0.2, 0.2});

	window.RegisterResizezable(&renderer, Renderer2D::Resize);
	while (window.ProcessMessages() == 0)
	{
		renderer.StartRecording();
		renderer.RenderGraphicalObject(obj);
		renderer.StopRecording();

	}
}


