#pragma once
#include "Entity.h"

enum class EnemyType
{
	weak,
	middle,
	strong,
	super_strong
};

class Enemy : public Entity
{
public: 
	Enemy(float hp, DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale = { 1,1 }, EnemyType type = EnemyType::weak);
	bool DealDamage(float dmg);
	EnemyType getType();
private:
	float hp;
	EnemyType type;
};

