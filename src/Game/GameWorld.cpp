#include "GameWorld.h"
#include <random>
#include <algorithm>
#include "Bullet.h"
#define COOLDOWN 0.01f
#define BULLET_LIFETIME 15.0f

using namespace std;
using namespace std::chrono;

GameWorld::GameWorld(DeviceResources* device, XMFLOAT2 worldBounds)
	:
	worldBounds(worldBounds), cooldown(0), totalGameTime(0)
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


	RegisterResource(player);

	for (int i = 0; i < 4; i++)
	{
		RegisterResource(worldConstructions[i]);
	}


	old = high_resolution_clock::now();


	spawnEnemy(device);
}

void GameWorld::GameLoop(Window* window, DeviceResources* device)
{
	const duration<float> frameTime = high_resolution_clock::now() - old;
	old = high_resolution_clock::now();
	totalGameTime += frameTime.count();


	processUserInput(window, device, frameTime.count());
	testCollisions(frameTime.count());
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

void GameWorld::spawnEnemy(DeviceResources* device)
{

	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<float> dis(-4.0f, 4.0);
	
	for (int i = 0; i < 300; i++)
	{
		Enemy* enemy = new Enemy({ 1.0f, device, {dis(gen), dis(gen)}, {0.05, 0.05}});
		enemy->UpdateColor({ 0.9f, 0.2f, 0.2f, 1.0f });
		enemies.push_back(enemy);
		RegisterResource(enemy);
	}
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

	cooldown -= dt;
	if (window->IsLeftPressed() && cooldown <= 0)
	{

		XMFLOAT2 mousePositionInWolrd = camera->TransformCoordsToWorldCoords({ window->GetMousePosXNormalized(), window->GetMousePosYNormalized() });
		XMFLOAT2 playerCenter = player->getEntityDescriptor().center;
		float rayDirX = mousePositionInWolrd.x - playerCenter.x;
		float rayDirY = mousePositionInWolrd.y - playerCenter.y;
		float normFactor = sqrtf(rayDirX * rayDirX + rayDirY * rayDirY);

		rayDirX /= normFactor;
		rayDirY /= normFactor;

		Bullet* bullet = new Bullet({ device, {playerCenter.x, playerCenter.y}, {0.01f, 0.01f} });
		bullet->UpdateColor({ 0.9f, 0.4f, 0.36f, 1.0f });
		bullet->UpdateDisplacementVectors({ rayDirX * dt, rayDirY * dt}, { 0,0 }, 0);
		bullets.push_back(bullet);
		RegisterResource(bullet);
		bulletLifetime.push_back(BULLET_LIFETIME);
		cooldown = COOLDOWN;
	}

	player->UpdateDisplacementVectors({x * dt, y * dt}, {0, 0}, 0);
	camera->ZoomUpdate(zoom, 0.5f, 1.5f);

}

void GameWorld::testCollisions(float dt)
{
	playerWallCollision(player, worldConstructions.data(), worldConstructions.size());
	player->UpdatePosition();
	vector<int> cleanupBulletPos;
	vector<int> cleanupEnemyPos;

	for (int i = 0; i < bullets.size(); i++)
	{
		bulletLifetime[i] -= dt;
		if (bulletLifetime[i] <= 0)
		{
			cleanBullet(i);
			cleanupBulletPos.push_back(i);
			continue;
		}

		bulletWallCollision(bullets[i], worldConstructions.data(), worldConstructions.size());
		bulletEnemyCollision(bullets[i], enemies.data(), enemies.size(), cleanupBulletPos, cleanupEnemyPos);
		if (bullets[i] != nullptr)
		{
			bullets[i]->UpdatePosition(true);
		}
	}

	for (int i = 0; i < cleanupBulletPos.size(); i++)
	{
		bullets.erase(bullets.begin() + cleanupBulletPos[i] - i);
		bulletLifetime.erase(bulletLifetime.begin() + cleanupBulletPos[i] - i);
	}

	for (int i = 0; i < cleanupEnemyPos.size(); i++)
	{
		enemies.erase(enemies.begin() + cleanupEnemyPos[i] - i);
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
		if (coll.collisionOccured)
		{
			entity->ResolveCollision(coll);
		}
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
		if (coll.collisionOccured)
		{
			bullet->ResolveCollision(coll);
		}

	}
}

void GameWorld::bulletEnemyCollision(Bullet* bullet, Enemy** collidableTable, int collidableCount, vector<int>& bulletCleanPos, vector<int>& enemyCleanPos)
{
	for (int i = 0; i < collidableCount; i++)
	{
		if (collidableTable[i] == nullptr)
		{
			continue;
		}

		CollisionDescriptor coll = bullet->IsColliding(*collidableTable[i]);
		if (coll.collisionOccured)
		{
			auto bulletIterator = find(bullets.begin(), bullets.end(), bullet);
			int index = distance(bullets.begin(), bulletIterator);
			cleanBullet(index);
			cleanEnemy(i);
			bulletCleanPos.push_back(index);
			enemyCleanPos.push_back(i);
		}
	}

}

void GameWorld::cleanBullet(int i)
{
	UnregisterResource(bullets[i]);
	Bullet* bullet = bullets[i];
	bullets[i] = nullptr;
	delete bullet;
}

void GameWorld::cleanEnemy(int i)
{
	UnregisterResource(enemies[i]);
	Enemy* bullet = enemies[i];
	enemies[i] = nullptr;
	delete bullet;
}

void GameWorld::RegisterResource(Entity* entity)
{

	renderableResources.resourceOwner.push_back(entity);
	renderableResources.constBufferTable.push_back(entity->getConstantBufferVirtualAddress());
	renderableResources.vbViewTable.push_back(entity->getVertexBufferView());
	renderableResources.ibViewTable.push_back(entity->getIndexBufferView());
	renderableResources.size++;
}

void GameWorld::UnregisterResource(Entity* entity)
{
	auto entityIterator = find(renderableResources.resourceOwner.begin(), renderableResources.resourceOwner.end(), entity);
	int index = distance(renderableResources.resourceOwner.begin(), entityIterator);

	renderableResources.resourceOwner.erase(renderableResources.resourceOwner.begin() + index);
	renderableResources.constBufferTable.erase(renderableResources.constBufferTable.begin() + index);
	renderableResources.ibViewTable.erase(renderableResources.ibViewTable.begin() + index);
	renderableResources.vbViewTable.erase(renderableResources.vbViewTable.begin() + index);
	renderableResources.size--;
}
