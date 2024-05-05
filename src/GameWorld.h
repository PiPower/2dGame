#pragma once
#include "Entity.h"
#include "Rendering/DeviceResources.h"
#include "Camera.h"

class GameWorld
{
public:
	GameWorld(DeviceResources* device, XMFLOAT2 worldBounds);
	void GameLoop(Window* window, DeviceResources* device);
	Camera* getCamera();
	vector<Entity>* getWorldEntities();
private:
	void processUserInput(Window* window, float dt);
	void testCollisions();
private:
	int playerIndex = 0;
	Camera* camera;
	XMFLOAT2 worldBounds;
	vector<Entity> worldEntities;
	TimePoint old;
};

