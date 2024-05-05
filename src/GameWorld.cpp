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
	collidable.push_back(1);

	worldEntities.push_back({ device, {0, worldBounds.y}, {worldBounds.y, 0.1f} });
	worldEntities.at(2).UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });
	collidable.push_back(2);

	worldEntities.push_back({ device, {worldBounds.x + 0.1f, 0}, {0.1f, worldBounds.x + 0.1f} });
	worldEntities.at(3).UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });
	collidable.push_back(3);

	worldEntities.push_back({ device, {-worldBounds.x - 0.1f, 0}, {0.1f, worldBounds.x + 0.1f} });
	worldEntities.at(4).UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });
	collidable.push_back(4);



	old = high_resolution_clock::now();
}

void GameWorld::GameLoop(Window* window, DeviceResources* device)
{
	const duration<float> frameTime = high_resolution_clock::now() - old;
	old = high_resolution_clock::now();

	processUserInput(window, device, frameTime.count());
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

void GameWorld::processUserInput(Window* window, DeviceResources* device, float dt)
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

	while (!window->IsMouseEventEmpty())
	{
		Window::MouseEvent mouseEvent = window->ReadMouseEvent();
		if (mouseEvent.Type != Window::MouseEvent::Event::LeftPress)
		{
			continue;
		}

		XMFLOAT2 camCenter = camera->getCameraCenter();
		XMFLOAT2 playerCenter = worldEntities[playerIndex].getEntityDescriptor().center;
		float rayDirX = window->GetMousePosXNormalized() + camCenter.x - playerCenter.x;
		float rayDirY = window->GetMousePosYNormalized() + camCenter.y - playerCenter.y;
		float normFactor = sqrtf(rayDirX * rayDirX + rayDirY * rayDirY);

		rayDirX /= normFactor;
		rayDirY /= normFactor;


		worldEntities.push_back({ device, {playerCenter.x, playerCenter.y}, {0.01f, 0.01f} });
		worldEntities.back().UpdateDisplacementVectors({ rayDirX * dt, rayDirY * dt}, { 0,0 }, 0);
		bulletIndexes.push_back(worldEntities.size() - 1);
	}


	worldEntities[playerIndex].UpdateDisplacementVectors({x * dt, y * dt}, {0, 0}, 0);
	camera->ZoomUpdate(zoom, 0.7f, 1.5f);

}

void GameWorld::testCollisions()
{
	perObjectCollision(0);

	worldEntities[playerIndex].UpdatePosition();
	for (int i = 0; i < bulletIndexes.size(); i++)
	{
		perObjectCollision(bulletIndexes[i]);
		Entity& bullet = worldEntities[bulletIndexes[i]];
		bullet.UpdatePosition(true);
	}
}

void GameWorld::perObjectCollision(unsigned int index)
{
	vector < pair<float, Entity*> > collisions;
	Entity& object = worldEntities[index];
	for (int i = 0; i < collidable.size(); i++)
	{
		CollisionDescriptor coll = object.IsColliding(worldEntities[collidable[i]]);

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
		CollisionDescriptor coll = object.IsColliding(*collision.second);
		object.ResolveCollision(coll);
	}


}
