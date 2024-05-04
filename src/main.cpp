#include "utils.h"
using namespace std;
using namespace std::chrono;


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

#if defined(_DEBUG)
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Window window(1600, 900, L"test", L"Voxel world");
	Renderer2D renderer(window.GetWindowHWND());
	
	vector<Entity> arrayOfObjs = {
		{ &renderer, {0.25, 0.25}, {0.2, 0.2}},
		{ &renderer, {0.7, -0.7}, {0.1, 0.1}},
		{ &renderer, {0.5, -0.7}, {0.1, 0.1}},
		{ &renderer, {0.3, -0.7}, {0.1, 0.1}},
		{ &renderer, {0.1, -0.7}, {0.1, 0.1}},
		{ &renderer, {-0.1, -0.7}, {0.1, 0.1}},
	};

	TimePoint old = high_resolution_clock::now();
	arrayOfObjs[0].UpdateColor(false);
	window.RegisterResizezable(&renderer, Renderer2D::Resize);
	while (window.ProcessMessages() == 0)
	{
		renderer.StartRecording();
		renderer.RenderGraphicalObjects(arrayOfObjs.data(), arrayOfObjs.size());
		renderer.StopRecording();
		const duration<float> frameTime = high_resolution_clock::now() - old;
		old = high_resolution_clock::now();
		processUserInput(&window, arrayOfObjs[0], frameTime.count());
		testCollisions(arrayOfObjs);
	}
}


void processUserInput(Window* window, Entity& obj, float dt)
{
	// rotation is currently unsupported


	constexpr float rot_tempo = 0.02;
	float x = 0, y = 0, rot = 0, scale_x = 0, scale_y = 0;
	constexpr float dx = 100, dy = 100, dsx = 0.5, dsy = 0.5, drot = 0.8;

	if (window->IsKeyPressed('W')) y += dy * dt;
	if (window->IsKeyPressed('S')) y -= dy * dt;
	if (window->IsKeyPressed('D')) x += dx * dt;
	if (window->IsKeyPressed('A')) x -= dx * dt;
	//if (window->IsKeyPressed('Q')) rot -= 0.03;
	//if (window->IsKeyPressed('E')) rot += 0.03;
	if (window->IsKeyPressed('Z')) scale_x -= dsx * dt;
	if (window->IsKeyPressed('X')) scale_x += dsx * dt;
	if (window->IsKeyPressed('C')) scale_y -= dsy * dt;
	if (window->IsKeyPressed('V')) scale_y += dsy * dt;

	obj.UpdateDisplacementVectors({ x*dt, y*dt }, { scale_x*dt, scale_y*dt }, rot*dt);
}

void testCollisions(vector<Entity>& arrayOfObjs)
{
	vector < pair<float, Entity*> > collisions;
	for (int i = 1; i < arrayOfObjs.size(); i++)
	{
		CollisionDescriptor coll = arrayOfObjs[0].IsColliding(arrayOfObjs[i]);

		if (coll.collisionOccured)
		{
			collisions.emplace_back(coll.t_hit, coll.collider);
		}

	}

	sort(collisions.begin(), collisions.end(),
		[](pair<float, Entity*>& obj_1, pair<float, Entity*>& obj_2) {
			return obj_1.first < obj_2.first;
		});


	for (pair<float, Entity*>& collision : collisions)
	{
		CollisionDescriptor coll = arrayOfObjs[0].IsColliding(*collision.second);
		arrayOfObjs[0].ResolveCollision(coll);
	}

	arrayOfObjs[0].UpdatePosition();

}


