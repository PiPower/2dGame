#include "Enemy.h"

Enemy::Enemy(float hp, DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale, EnemyType type)
	:
	Entity(device, origin, scale, 0), hp(hp), type(type)
{
}

bool Enemy::DealDamage(float dmg)
{
	hp -= dmg;
	return hp <= 0;
}

EnemyType Enemy::getType()
{
	return type;
}
