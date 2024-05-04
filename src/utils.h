#pragma once
#include "Rendering/window.h"
#include "Rendering/Renderer2D.h"
#include <chrono>
#include "Entity.h"

void processUserInput(Window* window, Entity& obj, float dt);
void testCollisions(vector<Entity>& arrayOfObjs);