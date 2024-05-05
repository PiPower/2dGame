#include "GameWorld.h"
#include <random>
#include "Bullet.h"

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

	player = new Entity({ device, {0.25, 0.25}, {0.05, 0.05} });
	player->UpdateColor({ 1,1, 1 , 1 });


	worldConstructions.push_back(new Entity( { device, {0, -worldBounds.y}, {worldBounds.y, 0.1f} } ));
	worldConstructions.at(0)->UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });

	worldConstructions.push_back( new Entity ({ device, {0, worldBounds.y}, {worldBounds.y, 0.1f} }));
	worldConstructions.at(1)->UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });

	worldConstructions.push_back( new Entity ({ device, {worldBounds.x + 0.1f, 0}, {0.1f, worldBounds.x + 0.1f} }));
	worldConstructions.at(2)->UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });

	worldConstructions.push_back( new Entity ({ device, {-worldBounds.x - 0.1f, 0}, {0.1f, worldBounds.x + 0.1f} }));
	worldConstructions.at(3)->UpdateColor({ dis(gen), dis(gen), dis(gen) , 1 });
;


	renderableResources.size = 5;
	renderableResources.resourceOwner.push_back(player);
	renderableResources.constBufferTable.push_back(player->getConstantBufferVirtualAddress());
	renderableResources.vbViewTable.push_back(player->getVertexBufferView());
	renderableResources.ibViewTable.push_back(player->getIndexBufferView());

	for (int i = 0; i < 4; i++)
	{
		renderableResources.resourceOwner.push_back(worldConstructions[i]);
		renderableResources.constBufferTable.push_back(worldConstructions[i]->getConstantBufferVirtualAddress());
		renderableResources.vbViewTable.push_back(worldConstructions[i]->getVertexBufferView());
		renderableResources.ibViewTable.push_back(worldConstructions[i]->getIndexBufferView());
	}


	old = high_resolution_clock::now();
}

void GameWorld::GameLoop(Window* window, DeviceResources* device)
{
	const duration<float> frameTime = high_resolution_clock::now() - old;
	old = high_resolution_clock::now();

	processUserInput(window, device, frameTime.count());
	testCollisions();
	camera->UpdateCamera(*player, frameTime.count());

}

Camera* GameWorld::getCamera()
{
	return camera;
}

RenderableResources* GameWorld::getRenderableResources()
{
	return &renderableResources;
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
		XMFLOAT2 playerCenter = player->getEntityDescriptor().center;
		float rayDirX = window->GetMousePosXNormalized() + camCenter.x - playerCenter.x;
		float rayDirY = window->GetMousePosYNormalized() + camCenter.y - playerCenter.y;
		float normFactor = sqrtf(rayDirX * rayDirX + rayDirY * rayDirY);

		rayDirX /= normFactor;
		rayDirY /= normFactor;


		Bullet* bullet = new Bullet({ device, {playerCenter.x, playerCenter.y}, {0.01f, 0.01f} });
		bullet->UpdateColor({ 0.9f, 0.4f, 0.36f, 1.0f });
		bullet->UpdateDisplacementVectors({ rayDirX * dt, rayDirY * dt }, { 0,0 }, 0);
		bullets.push_back(bullet);
		renderableResources.resourceOwner.push_back(bullet);
		renderableResources.constBufferTable.push_back(bullet->getConstantBufferVirtualAddress());
		renderableResources.vbViewTable.push_back(bullet->getVertexBufferView());
		renderableResources.ibViewTable.push_back(bullet->getIndexBufferView());
		renderableResources.size++;
	}


	player->UpdateDisplacementVectors({x * dt, y * dt}, {0, 0}, 0);
	camera->ZoomUpdate(zoom, 0.7f, 1.5f);

}

void GameWorld::testCollisions()
{
	playerWallCollision(player, worldConstructions.data(), worldConstructions.size());
	player->UpdatePosition();
	
	for (int i = 0; i < bullets.size(); i++)
	{
		bulletWallCollision(bullets[i], worldConstructions.data(), worldConstructions.size());
		bullets[i]->UpdatePosition(true);
	}
	
}

void GameWorld::playerWallCollision(Entity* entity, Entity** collidableTable, int collidableCount)
{
	vector < pair<float, Entity*> > collisions;
	for (int i = 0; i < collidableCount; i++)
	{
		CollisionDescriptor coll = entity->IsColliding(*collidableTable[i]);

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
		CollisionDescriptor coll = entity->IsColliding(*collision.second);
		entity->ResolveCollision(coll);
	}


}

void GameWorld::bulletWallCollision(Bullet* bullet, Entity** collidableTable, int collidableCount)
{
	vector < pair<float, Entity*> > collisions;
	for (int i = 0; i < collidableCount; i++)
	{
		CollisionDescriptor coll = bullet->IsColliding(*collidableTable[i]);

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
		CollisionDescriptor coll = bullet->IsColliding(*collision.second);
		bullet->ResolveCollision(coll);
	}
}
