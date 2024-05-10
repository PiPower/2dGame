#include "GameWorld.h"
#include <random>
#include <algorithm>
#include "Bullet.h"
#define DEAD_BULLET -100.0f
#define BULLET_WIPE 50
#define ENEMY_WIPE 40
#define RESPAWN_DELTA_MUL 0.7f
#define RESPAWN_DELTA_CAP 0.05f
#define WEAK_ENEMY 0.2f
#define MIDDLE_ENEMY 0.6f
#define STRONG_ENEMY 0.18f


float bulletCooldown[] = {
	0.01f,
	0.07f
};

float bulletLifetimeTable[] = {
	15.0f,
	2.0f
};

float velocityScalling[] = {
	1.0f,
	0.6f
};


static XMFLOAT2 scaleTable[] = {
	{ 0.01f, 0.01f },
	{ 0.03f, 0.03f }
};

static XMFLOAT4 colorTable[] = {
	{ 0.9f, 0.4f, 0.36f, 1.0f },
	{ 0.3f, 0.8f, 0.5f, 1.0f }
};

using namespace std;
using namespace std::chrono;

GameWorld::GameWorld(DeviceResources* device, XMFLOAT2 worldBounds)
	:
	worldBounds(worldBounds), cooldown(0), totalGameTime(0), 
	removedBullets(0), removedEnemies(0), respawnTime(0.0f), 
	respawnDelta(1.0f), currentBulletType(BulletType::normal_bullet)
{ 
	collisions.reserve(200);
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

}

void GameWorld::GameLoop(Window* window, DeviceResources* device)
{
	const duration<float> frameTime = high_resolution_clock::now() - old;
	old = high_resolution_clock::now();
	float dt = frameTime.count();
	totalGameTime += dt;


	processUserInput(window, device, dt);
	testCollisions(device, window, dt);
	camera->UpdateCamera(*player, dt);
	spawnEnemy(device, dt);
}

Camera* GameWorld::getCamera()
{
	return camera;
}

RenderableResources* GameWorld::getRenderableResources()
{
	return &renderableResources;
}

void GameWorld::spawnEnemy(DeviceResources* device, float dt)
{
	static random_device rd;
	static mt19937 gen(rd());
	static uniform_real_distribution<float> dis(-2, 2);
	static uniform_real_distribution<float> disEnemyType(0, 1);


	if (totalGameTime - respawnTime >= respawnDelta)
	{
		respawnDelta *= RESPAWN_DELTA_MUL;
		respawnDelta = respawnDelta < RESPAWN_DELTA_CAP ? RESPAWN_DELTA_CAP : respawnDelta;

		respawnTime = totalGameTime;
		float enemyType = disEnemyType(gen);
		Enemy* enemy;
		if (enemyType <=  WEAK_ENEMY)
		{
			enemy = new Enemy({ 1.0f, device, {dis(gen), dis(gen)}, {0.035, 0.035}, EnemyType::weak });
			enemy->UpdateColor({ 0.4f, 0.2f, 0.2f, 1.0f });
		}
		else if (enemyType <= (WEAK_ENEMY + MIDDLE_ENEMY))
		{
			enemy = new Enemy({ 2.0f, device, {dis(gen), dis(gen)}, {0.05, 0.05}, EnemyType::middle });
			enemy->UpdateColor({ 0.6f, 0.2f, 0.2f, 1.0f });
		}
		else if (enemyType <= (WEAK_ENEMY + MIDDLE_ENEMY + STRONG_ENEMY))
		{
			enemy = new Enemy({ 3.0f, device, {dis(gen), dis(gen)}, {0.06, 0.06}, EnemyType::strong });
			enemy->UpdateColor({ 1.0f, 0.2f, 0.2f, 1.0f });
		}
		else
		{
			enemy = new Enemy({ 8.0f, device, {dis(gen), dis(gen)}, {0.08, 0.08}, EnemyType::super_strong });
			enemy->UpdateColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		}


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
	if (window->IsKeyPressed('1')) currentBulletType = BulletType::normal_bullet;
	if (window->IsKeyPressed('2')) currentBulletType = BulletType::spawner;


	cooldown -= dt;
	if (window->IsLeftPressed() && cooldown <= 0)
	{
		SpawnBullet(device, window, dt);
	}


	player->UpdateDisplacementVectors({x * dt, y * dt}, {0, 0}, 0);
	camera->ZoomUpdate(zoom, 0.5f, 1.5f);

}

void GameWorld::testCollisions(DeviceResources* device, Window* window, float dt)
{
	playerWallCollision(player, worldConstructions.data(), worldConstructions.size());
	player->UpdatePosition();

	XMFLOAT2 playerCenter = player->getEntityDescriptor().center;
	XMVECTOR playerCenterVec = XMLoadFloat2(&playerCenter);
	for (int i = 0; i < enemies.size(); i++)
	{
		Enemy* enemy = enemies[i];
		if (enemy == nullptr)
		{
			continue;
		}
		XMFLOAT2 enemyCenter = enemy->getEntityDescriptor().center;
		XMVECTOR enemyCenterVec = XMLoadFloat2(&enemyCenter);

		XMVECTOR dirVec = playerCenterVec - enemyCenterVec;
		dirVec = XMVector2Normalize(dirVec) * dt * velocityScalling[(int)currentBulletType];
		XMFLOAT2 velocity;
		XMStoreFloat2(&velocity, dirVec);

		enemy->UpdateDisplacementVectors(velocity, {0,0}, 0);
	}

	for (int i = 0; i < bullets.size(); i++)
	{

		if (bulletLifetime[i] <= DEAD_BULLET)
		{
			continue;
		}

		bulletLifetime[i] -= dt;
		if (bulletLifetime[i] <= 0)
		{

			if (bullets[i]->GetBulletType() == BulletType::spawner)
			{
				BulletExplode(bullets[i], device, dt);
			}

			cleanBullet(i);
			continue;
		}

		bulletWallCollision(bullets[i], worldConstructions.data(), worldConstructions.size());
		bulletEnemyCollision(bullets[i], enemies.data(), enemies.size());
		if (bullets[i] != nullptr)
		{
			bullets[i]->UpdatePosition(true);
		}
	}

	for (int i = 0; i < enemies.size(); i++)
	{
		Enemy* enemy = enemies[i];
		if (enemy == nullptr)
		{
			continue;
		}
		enemy->UpdatePosition();
	}
	if( removedBullets >= BULLET_WIPE)
	{
		auto newBulletEndIter = remove_if(bullets.begin(), bullets.end(), [](Bullet* bullet) { return bullet == nullptr; });
		bullets.erase(newBulletEndIter, bullets.end());
		auto newLifetimeEndIter = remove_if(bulletLifetime.begin(), bulletLifetime.end(), [](float& lifetime) { return lifetime == DEAD_BULLET; });
		bulletLifetime.erase(newLifetimeEndIter, bulletLifetime.end());

		removedBullets = 0;
	}

	if (removedEnemies >= ENEMY_WIPE)
	{
		auto newEnemiesIter = remove_if(enemies.begin(), enemies.end(), [](Enemy* enemy) { return enemy == nullptr; });
		enemies.erase(newEnemiesIter, enemies.end());
		removedEnemies = 0;
	}

	
}

void GameWorld::playerWallCollision(Entity* entity, Entity** collidableTable, int collidableCount)
{
	collisions.clear();
	for (int i = 0; i < collidableCount; i++)
	{
		CollisionDescriptor coll = entity->IsColliding(collidableTable[i]);

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
		CollisionDescriptor coll = entity->IsColliding(collision.second);
		if (coll.collisionOccured)
		{
			entity->ResolveCollision(coll);
		}
	}


}

void GameWorld::bulletWallCollision(Bullet* bullet, Entity** collidableTable, int collidableCount)
{
	collisions.clear();
	for (int i = 0; i < collidableCount; i++)
	{
		CollisionDescriptor coll = bullet->IsColliding(collidableTable[i]);

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
		CollisionDescriptor coll = bullet->IsColliding(collision.second);
		if (coll.collisionOccured)
		{
			bullet->ResolveCollision(coll);
		}

	}
}

void GameWorld::bulletEnemyCollision(Bullet* bullet, Enemy** collidableTable, int collidableCount)
{
	collisions.clear();
	for (int i = 0; i < collidableCount; i++)
	{
		if (collidableTable[i] == nullptr)
		{
			continue;
		}

		CollisionDescriptor coll = bullet->IsCollidingWithEnemy(collidableTable[i]);
		if (coll.collisionOccured)
		{
			collisions.emplace_back(coll.t_hit, collidableTable[i]);
		}
	}

	if (collisions.size() == 0)
	{
		return;
	}

	sort(collisions.begin(), collisions.end(),
		[](pair<float, Entity*>& obj_1, pair<float, Entity*>& obj_2) {
			return obj_1.first < obj_2.first;
		});


	Enemy* collider = (Enemy * )collisions[0].second;
		
	auto enemytIterator = find(enemies.begin(), enemies.end(), collider);
	int enemyIndex = distance(enemies.begin(), enemytIterator);
	if (collider->DealDamage(bullet->GetDamage()))
	{
		cleanEnemy(enemyIndex);
	}

	auto bulletIterator = find(bullets.begin(), bullets.end(), bullet);
	int index = distance(bullets.begin(), bulletIterator);
	cleanBullet(index);
	
}

void GameWorld::cleanBullet(int i)
{
	UnregisterResource(bullets[i]);
	Bullet* bullet = bullets[i];
	bullets[i] = nullptr;
	bulletLifetime[i] = DEAD_BULLET;
	removedBullets++;
	delete bullet;
}

void GameWorld::cleanEnemy(int i)
{
	UnregisterResource(enemies[i]);
	Enemy* enemy = enemies[i];
	enemies[i] = nullptr;
	removedEnemies++;
	delete enemy;
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

void GameWorld::SpawnBullet(DeviceResources* device, Window* window, float dt)
{
	XMFLOAT2 mousePositionInWolrd = camera->TransformCoordsToWorldCoords({ window->GetMousePosXNormalized(), window->GetMousePosYNormalized() });
	XMFLOAT2 playerCenter = player->getEntityDescriptor().center;
	float rayDirX = mousePositionInWolrd.x - playerCenter.x;
	float rayDirY = mousePositionInWolrd.y - playerCenter.y;
	float normFactor = sqrtf(rayDirX * rayDirX + rayDirY * rayDirY);

	rayDirX /= normFactor;
	rayDirY /= normFactor;

	Bullet* bullet = new Bullet({ device, {playerCenter.x, playerCenter.y}, scaleTable[(int)currentBulletType], currentBulletType });
	bullet->UpdateColor(colorTable[(int)currentBulletType] );
	bullet->UpdateDisplacementVectors({ rayDirX * dt * velocityScalling[(int)currentBulletType], 
										rayDirY * dt * velocityScalling[(int)currentBulletType] }, {0,0}, 0);
	bullets.push_back(bullet);
	RegisterResource(bullet);
	bulletLifetime.push_back(bulletLifetimeTable[(int)currentBulletType]);
	cooldown = bulletCooldown[(int)currentBulletType] ;
}

void GameWorld::BulletExplode(Bullet* bullet, DeviceResources* device, float dt)
{
	
	constexpr int spawnedBulletCount = 5;

	PhysicalDescriptor bulletDesc = bullet->getEntityDescriptor();
	XMFLOAT2 buff = { bulletDesc.width, 0 };
	XMVECTOR offsetVec = XMLoadFloat2(&buff);


	for (float angle = 0; angle < 2 * 3.141592; angle += (2 * 3.141592) / spawnedBulletCount)
	{
		offsetVec = XMVector2Transform(offsetVec, XMMatrixRotationZ(angle));
		XMStoreFloat2(&buff, offsetVec);

		if (abs(bulletDesc.center.x) + buff.x >= worldBounds.x
			|| 
			abs(bulletDesc.center.y) + buff.y >= worldBounds.y)
		{
			continue;
		}

		Bullet* bullet = new Bullet({ device, {bulletDesc.center.x + buff.x , bulletDesc.center.y + buff.y}, { 0.01f, 0.01f } });
		bullet->UpdateColor({ 0.9f, 0.4f, 0.36f, 1.0f });
		XMStoreFloat2(&buff,  XMVector2Normalize( offsetVec) );
		bullet->UpdateDisplacementVectors({ buff.x * dt * velocityScalling[(int)BulletType::normal_bullet],
											buff.y * dt * velocityScalling[(int)BulletType::normal_bullet] }, { 0,0 }, 0);
		bullets.push_back(bullet);
		RegisterResource(bullet);
		bulletLifetime.push_back(bulletLifetimeTable[(int)BulletType::normal_bullet]);


	}
	


}
