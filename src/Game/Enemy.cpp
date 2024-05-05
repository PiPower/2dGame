#include "Enemy.h"

Enemy::Enemy(float hp, DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale)
	:
	Entity(device, origin, scale, 0), hp(hp)
{
}

bool Enemy::DealDamage(float dmg)
{
	hp -= dmg;
	return hp <= 0;
}
