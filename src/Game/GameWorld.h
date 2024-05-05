#pragma once
#include "Entity.h"
#include "../Rendering/DeviceResources.h"
#include "Camera.h"
#include "Bullet.h"

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
	void processUserInput(Window* window, DeviceResources* device, float dt);
	void testCollisions();
	void playerWallCollision(Entity* entity, Entity** collidableTable, int collidableCount);
	void bulletWallCollision(Bullet* bullet, Entity** collidableTable, int collidableCount);
private:
	Entity* player;
	Camera* camera;
	XMFLOAT2 worldBounds;
	vector<Entity*> worldConstructions;
	vector<Bullet*> bullets;
	RenderableResources renderableResources;
	TimePoint old;
};

