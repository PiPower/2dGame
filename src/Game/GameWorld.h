#pragma once
#include "Entity.h"
#include "../Rendering/DeviceResources.h"
#include "Camera.h"
#include "Bullet.h"
#include "Enemy.h"

struct RenderableResources
{
	vector<Entity*> resourceOwner;
	vector<D3D12_GPU_VIRTUAL_ADDRESS> constBufferTable;
	vector<D3D12_VERTEX_BUFFER_VIEW*> vbViewTable;
	vector<D3D12_INDEX_BUFFER_VIEW*> ibViewTable;
	int size;
};



class GameWorld
{
public:
	GameWorld(DeviceResources* device, XMFLOAT2 worldBounds);
	void GameLoop(Window* window, DeviceResources* device);
	Camera* getCamera();
	RenderableResources* getRenderableResources();
private:
	void spawnEnemy(DeviceResources* device, float dt);
	void processUserInput(Window* window, DeviceResources* device, float dt);
	void testCollisions(DeviceResources* device, Window* window, float dt);
	void playerWallCollision(Entity* entity, Entity** collidableTable, int collidableCount);
	void bulletWallCollision(Bullet* bullet, Entity** collidableTable, int collidableCount);
	void bulletEnemyCollision(Bullet* bullet, Enemy** collidableTable, int collidableCount);
	void cleanBullet(int i);
	void cleanEnemy(int i);
	void cleanEnemyBullet(int i);
	void RegisterResource(Entity* entity);
	void UnregisterResource(Entity* entity);
	void SpawnBullet(DeviceResources* device, Window* window, float dt);
	void BulletExplode(Bullet* bullet, DeviceResources* device, float dt);
	void UpdateFriendlyBullets(DeviceResources* device, float dt);
	void UpdateEnemyBullets(DeviceResources* device, float dt);
	void UpdateEnemies(DeviceResources* device, float dt);
private:
	float cooldown;
	Entity* player;
	Camera* camera;
	XMFLOAT2 worldBounds;
	vector<Entity*> worldConstructions;
	vector<float> bulletLifetime;
	vector<Bullet*> bullets;
	vector<Enemy*> enemies;
	vector<float> enemyCooldown;
	vector<Bullet*> enemyBullets;
	vector<float> enemyBulletsLifetime;
	RenderableResources renderableResources;
	vector < pair<float, Entity*> > collisions;
	unsigned int removedBullets;
	unsigned int removedEnemyBullets;
	unsigned int removedEnemies;
	float totalGameTime;
	TimePoint old;
	float respawnTime;
	float respawnDelta;
	BulletType currentBulletType;
};

