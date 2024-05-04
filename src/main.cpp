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
	Camera cam(&renderer);
	renderer.BindCameraBuffer(cam.getWorldTransformAddress());
	vector<Entity>* arrayOfObjsPtr = initWorld(&renderer);
	vector<Entity>& arrayOfObjs = *arrayOfObjsPtr;



	TimePoint old = high_resolution_clock::now();
	
	arrayOfObjs[0].UpdateColor({1,1,1,1});
	window.RegisterResizezable(&renderer, Renderer2D::Resize);
	while (window.ProcessMessages() == 0)
	{
		renderer.StartRecording();
		renderer.RenderGraphicalObjects(arrayOfObjs.data(), arrayOfObjs.size());
		renderer.StopRecording();
		const duration<float> frameTime = high_resolution_clock::now() - old;
		old = high_resolution_clock::now();
		processUserInput(&window, arrayOfObjs[0], cam, frameTime.count());
		testCollisions(arrayOfObjs);
		cam.UpdateCamera(arrayOfObjs[0], frameTime.count());
	}
}


void processUserInput(Window* window, Entity& obj, Camera& cam, float dt)
{
	// rotation is currently unsupported


	float x = 0, y = 0, zoom = 0;
	constexpr float dx = 300, dy = 300, dz = 5;


	if (window->IsKeyPressed('W')) y += dy * dt;
	if (window->IsKeyPressed('S')) y -= dy * dt;
	if (window->IsKeyPressed('D')) x += dx * dt;
	if (window->IsKeyPressed('A')) x -= dx * dt;
	if (window->IsKeyPressed('Q')) zoom -= dz * dt;
	if (window->IsKeyPressed('E')) zoom += dz * dt;

	obj.UpdateDisplacementVectors({ x*dt, y*dt }, {0, 0 }, 0);
	cam.ZoomUpdate(zoom);
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


vector<Entity>* initWorld(DeviceResources* renderer)
{
	vector<Entity>* arrayOfObjs = new vector<Entity>();

	arrayOfObjs->push_back({ renderer, {0.25, 0.25}, {0.05, 0.05} });
	for (int i = 0; i < 20; i++)
	{
		arrayOfObjs->push_back({ renderer, {-1.9f + i * 0.2f, -0.6f}, {0.1f, 0.1f} });
	}

	return arrayOfObjs;
}