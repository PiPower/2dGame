#include "Rendering/window.h"
#include "Rendering/Renderer2D.h"
#include "Entity.h"


void processUserInput(Window* window, Entity& obj);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

#if defined(_DEBUG)
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Window window(1600, 900, L"test", L"Voxel world");
	Renderer2D renderer(window.GetWindowHWND());
	Entity obj(&renderer, { 0.2, 0.2 }, {0.2, 0.2});

	window.RegisterResizezable(&renderer, Renderer2D::Resize);
	while (window.ProcessMessages() == 0)
	{
		renderer.StartRecording();
		renderer.RenderGraphicalObject(obj);
		renderer.StopRecording();
		processUserInput(&window, obj);
	}
}


void processUserInput(Window* window, Entity& obj)
{
	constexpr float rot_tempo = 0.02;
	float x = 0, y = 0, rot = 0, scale_x = 0, scale_y = 0;


	if (window->IsKeyPressed('W')) y += 0.03;
	if (window->IsKeyPressed('S')) y -= 0.03;
	if (window->IsKeyPressed('D')) x += 0.03;
	if (window->IsKeyPressed('A')) x -= 0.03;
	if (window->IsKeyPressed('Q')) rot -= 0.03;
	if (window->IsKeyPressed('E')) rot += 0.03;
	if (window->IsKeyPressed('Z')) scale_x -= 0.01;
	if (window->IsKeyPressed('X')) scale_x += 0.01;
	if (window->IsKeyPressed('C')) scale_y -= 0.01;
	if (window->IsKeyPressed('V')) scale_y += 0.01;

	obj.UpdatePosition({ x, y }, { scale_x, scale_y }, rot);
}


