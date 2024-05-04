#pragma once
#include "Rendering/window.h"
#include "Rendering/Renderer2D.h"
#include <chrono>
#include "Entity.h"
#include "Camera.h"

void processUserInput(Window* window, Entity& obj, Camera& cam, float dt);
void testCollisions(vector<Entity>& arrayOfObjs);
vector<Entity>* initWorld(DeviceResources* renderer);