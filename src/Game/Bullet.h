#pragma once
#include "Entity.h"
#include "Enemy.h"
class Bullet : public Entity
{
public:
	Bullet(DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale = { 1,1 });
	void ResolveCollision(CollisionDescriptor& desc);
	void UpdatePosition(bool keepVelocity = false);
	CollisionDescriptor IsCollidingWithEnemy(Enemy* enemy);
private:
	XMFLOAT2 velocityBuffer;
};
