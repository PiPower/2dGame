#include "GameWorld.h"
#include <random>
using namespace std;
using namespace std::chrono;

GameWorld::GameWorld(DeviceResources* device, XMFLOAT2 worldBounds)
	:
	worldBounds(worldBounds)
{

	camera = new Camera(device);
	vector<Entity>* arrayOfObjs = new vector<Entity>();
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<float> dis(0, 1.0);

	worldEntities.push_back({ device, {0.25, 0.25}, {0.05, 0.05} });
	worldEntities.at(0).UpdateColor({ 1,1, 1 , 1 });


	worldEntities.push_back({ device, {0, -worldBounds.y}, {worldBounds.y, 0.1f} });
	worldEntities.at(1).UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });
	
	worldEntities.push_back({ device, {0, worldBounds.y}, {worldBounds.y, 0.1f} });
	worldEntities.at(2).UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });
	
	worldEntities.push_back({ device, {worldBounds.x + 0.1f, 0}, {0.1f, worldBounds.x + 0.1f} });
	worldEntities.at(3).UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });

	worldEntities.push_back({ device, {-worldBounds.x - 0.1f, 0}, {0.1f, worldBounds.x + 0.1f} });
	worldEntities.at(4).UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });

	old = high_resolution_clock::now();
}

void GameWorld::GameLoop(Window* window, DeviceResources* device)
{
	const duration<float> frameTime = high_resolution_clock::now() - old;
	old = high_resolution_clock::now();

	processUserInput(window, frameTime.count());
	testCollisions();
	camera->UpdateCamera(worldEntities[0], frameTime.count());

}

Camera* GameWorld::getCamera()
{
	return camera;
}

vector<Entity>* GameWorld::getWorldEntities()
{
	return &worldEntities;
}

void GameWorld::processUserInput(Window* window, float dt)
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

	worldEntities[playerIndex].UpdateDisplacementVectors({x * dt, y * dt}, {0, 0}, 0);
	camera->ZoomUpdate(zoom, 0.7f, 1.5f);

}

void GameWorld::testCollisions()
{
	vector < pair<float, Entity*> > collisions;
	for (int i = 1; i < worldEntities.size(); i++)
	{
		CollisionDescriptor coll = worldEntities[playerIndex].IsColliding(worldEntities[i]);

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
		CollisionDescriptor coll = worldEntities[playerIndex].IsColliding(*collision.second);
		worldEntities[playerIndex].ResolveCollision(coll);
	}

	worldEntities[playerIndex].UpdatePosition();

}
