#pragma once
#include "Entity.h"

class Bullet : public Entity
{
public:
	Bullet(DeviceResources* device, DirectX::XMFLOAT2 origin, DirectX::XMFLOAT2 scale = { 1,1 });
};
