#pragma once
#include "Entity.h"
class Enemy : public Entity
{
public: 
	Enemy(float hp, DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale = { 1,1 });
	bool DealDamage(float dmg);
private:
	float hp;
};

