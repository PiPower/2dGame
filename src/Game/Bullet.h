#pragma once
#include "Entity.h"
#include "Enemy.h"

enum class BulletType
{
	normal_bullet
};

class Bullet : public Entity
{
public:
	Bullet(DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale = { 1,1 }, 
														BulletType bulletType = BulletType::normal_bullet);
	void ResolveCollision(CollisionDescriptor& desc);
	void UpdatePosition(bool keepVelocity = false);
	float GetDamage();
	CollisionDescriptor IsCollidingWithEnemy(Enemy* enemy);
private:
	XMFLOAT2 velocityBuffer;
	BulletType bulletType;
};
